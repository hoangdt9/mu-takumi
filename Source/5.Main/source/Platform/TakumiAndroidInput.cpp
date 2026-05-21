#ifdef __ANDROID__

#include "stdafx.h"
#include "Platform/TakumiAndroidInput.h"
#include "NewUISystem.h"
#include "NewUIMyInventory.h"
#include "NewUIInventoryCtrl.h"
#include "NewUICommon.h"
#include "NewUIMessageBox.h"
#include "ZzzOpenglUtil.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzInfomation.h"
#include "ZzzScene.h"
#include "ZzzLodTerrain.h"
#include "ZzzObject.h"
#include "MobilePlatform.h"
#include "NewUIMainFrameWindow.h"
#include "SkillManager.h"
#include "Utilities/Log/ErrorReport.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include <android/log.h>

extern CErrorReport g_ErrorReport;
extern MovementSkill g_MovementSkill;
#define TAKUMI_INV_LOGI(...) __android_log_print(ANDROID_LOG_INFO, "TakumiInvUse", __VA_ARGS__)
#define TAKUMI_INV_LOGW(...) __android_log_print(ANDROID_LOG_WARN, "TakumiInvUse", __VA_ARGS__)
#define TAKUMI_SKILL_LOGI(...) __android_log_print(ANDROID_LOG_INFO, "TakumiSkillAtk", __VA_ARGS__)
#define TAKUMI_SKILL_LOGW(...) __android_log_print(ANDROID_LOG_WARN, "TakumiSkillAtk", __VA_ARGS__)

namespace
{
constexpr int kInventoryPanelWidth = 190;
constexpr int kInventoryPanelHeight = 429;
constexpr uint32_t kInventoryUseLongPressMs = 480;
constexpr uint32_t kInventoryDoubleTapMaxMs = 420;
constexpr float kInventoryDoubleTapMaxDistUi = 28.0f;
constexpr float kInventoryLongPressCancelMoveUi = 48.0f;

struct InventoryTouchState
{
    SDL_FingerID finger = static_cast<SDL_FingerID>(-1);
    uint32_t downMs = 0;
    float downUiX = 0.0f;
    float downUiY = 0.0f;
};

InventoryTouchState g_inventoryTouch{};
bool g_inventoryUsePressPending = false;
bool g_inventoryLongPressFired = false;

uint32_t g_inventoryLastShortTapMs = 0;
float g_inventoryLastShortTapUiX = 0.0f;
float g_inventoryLastShortTapUiY = 0.0f;

constexpr uint32_t kWorldSkillLongPressMs = 480;
constexpr uint32_t kWorldSkillDoubleTapMaxMs = 420;
constexpr uint32_t kWorldSkillChannelIntervalMs = 120;
constexpr float kWorldSkillLongPressCancelMoveUi = 48.0f;
constexpr float kWorldSkillDoubleTapMaxDistUi = 28.0f;

struct WorldSkillTouchState
{
    SDL_FingerID finger = static_cast<SDL_FingerID>(-1);
    uint32_t downMs = 0;
    float downUiX = 0.0f;
    float downUiY = 0.0f;
    int aimMouseX = 320;
    int aimMouseY = 240;
};

bool ShouldSkipPrimeHeroStopForConcurrentJoystick()
{
    return MU_AndroidIsVirtualJoystickDrivingMouse() || MU_AndroidIsVirtualJoystickEngaged();
}

WorldSkillTouchState g_worldSkillTouch{};
bool g_worldSkillLongPressFired = false;
bool g_worldSkillTouchConsumed = false;

uint32_t g_worldSkillLastShortTapMs = 0;
float g_worldSkillLastShortTapUiX = 0.0f;
float g_worldSkillLastShortTapUiY = 0.0f;

uint32_t g_worldSkillPendingMeleeMs = 0;
bool g_worldSkillChannelHold = false;
bool g_worldSkillChannelLatched = false;
uint32_t g_worldSkillLastChannelTickMs = 0;

struct ScopedWorldSkillAimMouse
{
    int savedX = 0;
    int savedY = 0;
    bool active = false;

    ScopedWorldSkillAimMouse()
    {
        // Latched auto-channel must not reuse attack-button UI coords (wrong facing → server hits=0).
        if (g_worldSkillChannelLatched)
        {
            return;
        }

        // Finger id is cleared on touch-up before deferred melee; downMs stays until the next gesture.
        if (g_worldSkillTouch.downMs == 0)
        {
            return;
        }

        savedX = MouseX;
        savedY = MouseY;
        MouseX = g_worldSkillTouch.aimMouseX;
        MouseY = g_worldSkillTouch.aimMouseY;
        active = true;
    }

    ~ScopedWorldSkillAimMouse()
    {
        if (!active)
        {
            return;
        }

        MouseX = savedX;
        MouseY = savedY;
    }
};

void TouchToVirtualUi(const SDL_TouchFingerEvent& touch, float& outX, float& outY)
{
    const float nx = std::clamp(touch.x, 0.0f, 1.0f);
    const float ny = std::clamp(touch.y, 0.0f, 1.0f);
    outX = nx * 640.0f;
    outY = ny * 480.0f;
}

bool IsInventoryUiOpenForUse()
{
    return g_pNewUISystem != nullptr
        && g_pMyInventory != nullptr
        && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY)
        && g_pMyInventory->GetRepairMode() == SEASON3B::CNewUIMyInventory::REPAIR_MODE_OFF;
}

bool IsMouseOverInventoryPanel()
{
    if (!IsInventoryUiOpenForUse())
    {
        return false;
    }

    const POINT& pos = g_pMyInventory->GetPos();
    return SEASON3B::CheckMouseIn(pos.x, pos.y, kInventoryPanelWidth, kInventoryPanelHeight);
}

void SyncAndroidRightButtonKeyPress()
{
    if (g_pNewKeyInput != nullptr)
    {
        g_pNewKeyInput->SetKeyState(VK_RBUTTON, SEASON3B::CNewKeyInput::KEY_PRESS);
    }
}

bool AndroidTryUseItemNow()
{
    if (g_pMyInventory == nullptr)
    {
        return false;
    }

    if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem() != nullptr)
    {
        SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
        TAKUMI_INV_LOGI("cleared picked item before use");
    }

    const bool used = g_pMyInventory->AndroidTryUseItemUnderCursor();
    if (used)
    {
        TAKUMI_INV_LOGI("AndroidTryUseItemUnderCursor ok Mouse=%d,%d", MouseX, MouseY);
        g_ErrorReport.Write("[InvUse] use item ok Mouse=%d,%d", MouseX, MouseY);
    }
    else
    {
        TAKUMI_INV_LOGW(
            "AndroidTryUseItemUnderCursor failed Mouse=%d,%d overPanel=%d",
            MouseX,
            MouseY,
            IsMouseOverInventoryPanel() ? 1 : 0);
        g_ErrorReport.Write(
            "[InvUse] use failed Mouse=%d,%d overPanel=%d",
            MouseX,
            MouseY,
            IsMouseOverInventoryPanel() ? 1 : 0);
    }

    return used;
}

void FireInventoryLongPressUse()
{
    if (g_inventoryLongPressFired)
    {
        return;
    }

    g_inventoryLongPressFired = true;
    TakumiAndroid_PulseRightClick();
    AndroidTryUseItemNow();
    g_inventoryUsePressPending = false;
    MouseLButtonPush = false;
    MouseLButtonPop = false;
    MouseLButton = false;
}

bool IsWorldSkillGestureScene()
{
    return SceneFlag == MAIN_SCENE
        && Hero != nullptr
        && CharacterAttribute != nullptr
        && Hero->Dead <= 0;
}

bool TakumiAndroid_IsHudBlockingWorldGesture(float uiX, float uiY)
{
    if (SceneFlag != MAIN_SCENE || Hero == nullptr)
    {
        return false;
    }

    if (g_MessageBox != nullptr && !g_MessageBox->IsEmpty())
    {
        return true;
    }

    if (IsInventoryUiOpenForUse() && IsMouseOverInventoryPanel())
    {
        return true;
    }

    // Virtual 640×480 — legacy HUD chrome (joystick / attack / skill row).
    if (uiX < 220.0f && uiY > 290.0f)
    {
        return true;
    }

    if (uiY > 395.0f && uiX > 460.0f)
    {
        return true;
    }

    if (uiY > 418.0f && uiX > 180.0f && uiX < 540.0f)
    {
        return true;
    }

    return false;
}

bool IsWorldSkillGestureBlockedByUiAt(const float uiX, const float uiY)
{
    if (g_MessageBox != nullptr && !g_MessageBox->IsEmpty())
    {
        return true;
    }

    if (IsInventoryUiOpenForUse() && IsMouseOverInventoryPanel())
    {
        return true;
    }

    if (TakumiAndroid_IsHudBlockingWorldGesture(uiX, uiY))
    {
        return true;
    }

    return false;
}

int SkillTypeAtIndex(const int skillIndex)
{
    if (skillIndex < 0 || skillIndex >= MAX_MAGIC)
    {
        return 0;
    }

    return CharacterAttribute->Skill[skillIndex];
}

int ResolveActiveHotbarSkillIndex()
{
    if (!IsWorldSkillGestureScene())
    {
        return -1;
    }

    if (Hero->CurrentSkill >= 0 && Hero->CurrentSkill < MAX_MAGIC)
    {
        const int skillType = SkillTypeAtIndex(Hero->CurrentSkill);
        if (skillType > 0 && skillType < MAX_SKILLS)
        {
            return Hero->CurrentSkill;
        }
    }

    if (g_pSkillList != nullptr)
    {
        for (int hotKey = 0; hotKey < 10; ++hotKey)
        {
            const int skillIndex = g_pSkillList->GetHotKey(hotKey);
            const int skillType = SkillTypeAtIndex(skillIndex);
            if (skillType > 0 && skillType < MAX_SKILLS)
            {
                return skillIndex;
            }
        }
    }

    return -1;
}

bool HasActiveHotbarSkill()
{
    return ResolveActiveHotbarSkillIndex() >= 0;
}

bool IsSupportOrSelfSkillType(const int skillType)
{
    return IsCorrectSkillType_Buff(skillType) == TRUE
        || IsCorrectSkillType_FrendlySkill(skillType) == TRUE;
}

// IsDirectionChannelSkillType — shared with PC path in ZzzInterface.cpp

void PrimeHeroForSkillCast()
{
    if (Hero == nullptr || ShouldSkipPrimeHeroStopForConcurrentJoystick())
    {
        return;
    }

    LetHeroStop(Hero, TRUE);
    Hero->Movement = false;
    SetPlayerStop(Hero);
}

void SetupMovementSkillForCast(const int skillIndex)
{
    ZeroMemory(&g_MovementSkill, sizeof(g_MovementSkill));
    g_MovementSkill.m_bMagic = TRUE;
    g_MovementSkill.m_iSkill = skillIndex;

    if (CheckAttack())
    {
        g_MovementSkill.m_iTarget = SelectedCharacter;
    }
    else
    {
        g_MovementSkill.m_iTarget = -1;
    }
}

void PulseAndroidSkillAttack(const bool holdRightButton)
{
    MouseLButtonPush = false;
    MouseLButtonPop = false;
    MouseLButton = false;
    MouseRButtonPop = false;
    MouseRButtonPush = true;
    MouseRButton = holdRightButton;
    SyncAndroidRightButtonKeyPress();
    Attack(Hero);
    if (!holdRightButton)
    {
        MouseRButtonPush = false;
        MouseRButton = false;
    }
}

bool TrySelectNearestAttackableMonster();

void StopWorldSkillChannel()
{
    g_worldSkillChannelHold = false;
    g_worldSkillChannelLatched = false;
    MouseRButtonPush = false;
    MouseRButton = false;
    MouseRButtonPop = false;
    Attacking = -1;
}

int GetHeroCharacterIndexLocal()
{
    if (Hero == nullptr || CharactersClient == nullptr)
    {
        return -1;
    }

    if (Hero < &CharactersClient[0] || Hero >= (&CharactersClient[0] + MAX_CHARACTERS_CLIENT))
    {
        return -1;
    }

    return static_cast<int>(Hero - &CharactersClient[0]);
}

bool TrySelectNearestAttackableMonster()
{
    if (!IsWorldSkillGestureScene() || CharactersClient == nullptr || Hero == nullptr)
    {
        return false;
    }

    const int heroIndex = GetHeroCharacterIndexLocal();
    int bestIndex = -1;
    float bestDist2 = 0.0f;

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
    {
        if (i == heroIndex)
        {
            continue;
        }

        CHARACTER* candidate = &CharactersClient[i];
        if (candidate->Dead > 0 || !candidate->Object.Visible)
        {
            continue;
        }

        if (candidate->Object.Kind != KIND_MONSTER && candidate->Object.Kind != KIND_EDIT)
        {
            continue;
        }

        const float dx = static_cast<float>(candidate->Object.Position[0] - Hero->Object.Position[0]);
        const float dy = static_cast<float>(candidate->Object.Position[1] - Hero->Object.Position[1]);
        const float dist2 = (dx * dx) + (dy * dy);
        if (bestIndex < 0 || dist2 < bestDist2)
        {
            bestIndex = i;
            bestDist2 = dist2;
        }
    }

    if (bestIndex < 0)
    {
        return false;
    }

    const int previousTarget = SelectedCharacter;
    SelectedCharacter = bestIndex;
    if (!CheckAttack())
    {
        SelectedCharacter = previousTarget;
        return false;
    }

    return true;
}

bool RefreshAttackableTargetAtMouse()
{
    if (!IsWorldSkillGestureScene())
    {
        return false;
    }

    const int previousTarget = SelectedCharacter;
    int candidate = SelectCharacter(KIND_MONSTER | KIND_EDIT);
    if (candidate == -1)
    {
        candidate = SelectCharacter(KIND_PLAYER);
    }

    if (candidate < 0)
    {
        SelectedCharacter = previousTarget;
        return false;
    }

    SelectedCharacter = candidate;
    if (!CheckAttack())
    {
        SelectedCharacter = previousTarget;
        return false;
    }

    return true;
}

enum class WorldSkillTargetKind : int
{
    None = 0,
    AttackCharacter,
    FriendlyPlayer,
    FriendlyNpc,
    Self,
    Ground,
};

bool PrepareWorldSkillTarget(int skillType, WorldSkillTargetKind& outKind)
{
    outKind = WorldSkillTargetKind::None;

    if (!IsWorldSkillGestureScene())
    {
        return false;
    }

    SelectObjects();

    if (IsDirectionChannelSkillType(skillType))
    {
        if (!TakumiAndroid_ResolveDirectionChannelTarget(Hero))
        {
            return false;
        }

        outKind = WorldSkillTargetKind::Ground;
        return true;
    }

    if (RefreshAttackableTargetAtMouse())
    {
        outKind = WorldSkillTargetKind::AttackCharacter;
        return true;
    }

    const bool friendlySkill = IsSupportOrSelfSkillType(skillType);

    if (friendlySkill)
    {
        const int playerTarget = SelectCharacter(KIND_PLAYER);
        if (playerTarget >= 0)
        {
            SelectedCharacter = playerTarget;
            outKind = WorldSkillTargetKind::FriendlyPlayer;
            return true;
        }

        const int npcTarget = SelectCharacter(KIND_NPC);
        if (npcTarget >= 0)
        {
            SelectedCharacter = npcTarget;
            SelectedNpc = npcTarget;
            outKind = WorldSkillTargetKind::FriendlyNpc;
            return true;
        }

        const int heroIndex = GetHeroCharacterIndexLocal();
        if (heroIndex >= 0)
        {
            SelectedCharacter = heroIndex;
            outKind = WorldSkillTargetKind::Self;
            return true;
        }
    }

    if (!friendlySkill && TrySelectNearestAttackableMonster())
    {
        outKind = WorldSkillTargetKind::AttackCharacter;
        return true;
    }

    const int previousTarget = SelectedCharacter;
    SelectedCharacter = -1;
    if (!CheckTarget(Hero))
    {
        SelectedCharacter = previousTarget;
        return false;
    }

    outKind = WorldSkillTargetKind::Ground;
    return true;
}

bool FireWorldSkillChannelTick(const char* reason)
{
    if (!IsWorldSkillGestureScene())
    {
        return false;
    }

    const int skillIndex = ResolveActiveHotbarSkillIndex();
    if (skillIndex < 0)
    {
        return false;
    }

    const int skillType = SkillTypeAtIndex(skillIndex);
    if (!IsDirectionChannelSkillType(skillType))
    {
        return false;
    }

    const uint32_t nowMs = MU_MobileGetTicks();
    if ((nowMs - g_worldSkillLastChannelTickMs) < kWorldSkillChannelIntervalMs)
    {
        return true;
    }

    g_worldSkillLastChannelTickMs = nowMs;

    const ScopedWorldSkillAimMouse aimGuard;

    const int previousSkillIndex = Hero->CurrentSkill;
    Hero->CurrentSkill = static_cast<BYTE>(skillIndex);

    WorldSkillTargetKind targetKind = WorldSkillTargetKind::None;
    if (!PrepareWorldSkillTarget(skillType, targetKind))
    {
        Hero->CurrentSkill = static_cast<BYTE>(previousSkillIndex);
        return false;
    }

    // Do not stop the hero between channel ticks — SetPlayerStop() cancels cast animation.
    SetupMovementSkillForCast(skillIndex);
    const bool castOk = CastDirectionChannelSkill(Hero, skillType, static_cast<BYTE>(skillIndex));

    Hero->CurrentSkill = static_cast<BYTE>(previousSkillIndex);
    TAKUMI_SKILL_LOGI(
        "skill channel tick reason=%s skill=%d type=%d cast=%d attacking=%d skillId=%d atkTime=%d",
        reason != nullptr ? reason : "?",
        skillIndex,
        skillType,
        castOk ? 1 : 0,
        Attacking,
        Hero->Skill,
        Hero->AttackTime);
    g_ErrorReport.Write(
        "[SkillAtk] channel reason=%s skill=%d type=%d cast=%d attacking=%d",
        reason != nullptr ? reason : "?",
        skillIndex,
        skillType,
        castOk ? 1 : 0,
        Attacking);
    return castOk;
}

bool FireWorldPickUpItem(const char* reason)
{
    if (!IsWorldSkillGestureScene() || IsWorldSkillGestureBlockedByUiAt(g_worldSkillTouch.downUiX, g_worldSkillTouch.downUiY))
    {
        return false;
    }

    SelectObjects();
    if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem() != nullptr)
    {
        return false;
    }

    if (SelectedItem < 0)
    {
        SelectedItem = SelectItem();
    }

    if (SelectedItem < 0)
    {
        return false;
    }

    SelectedCharacter = -1;
    Attacking = -1;

    g_worldSkillLongPressFired = true;
    g_worldSkillTouchConsumed = true;

    MouseRButtonPush = false;
    MouseRButton = false;
    MouseRButtonPop = false;
    MouseLButtonPop = false;
    MouseLButtonPush = true;
    MouseLButton = true;

    MoveHero();

    MouseLButtonPush = false;
    MouseLButton = false;

    TAKUMI_SKILL_LOGI(
        "pickup item ok reason=%s item=%d Mouse=%d,%d",
        reason != nullptr ? reason : "?",
        SelectedItem,
        MouseX,
        MouseY);
    g_ErrorReport.Write("[SkillAtk] pickup item=%d", SelectedItem);
    return true;
}

bool FireWorldOperateObject(const char* reason)
{
    if (!IsWorldSkillGestureScene() || IsWorldSkillGestureBlockedByUiAt(g_worldSkillTouch.downUiX, g_worldSkillTouch.downUiY))
    {
        return false;
    }

    SelectObjects();
    if (SelectedOperate < 0)
    {
        SelectedOperate = SelectOperate();
    }

    if (SelectedOperate < 0)
    {
        return false;
    }

    SelectedCharacter = -1;
    Attacking = -1;

    g_worldSkillLongPressFired = true;
    g_worldSkillTouchConsumed = true;

    MouseRButtonPush = false;
    MouseRButton = false;
    MouseRButtonPop = false;
    MouseLButtonPop = false;
    MouseLButtonPush = true;
    MouseLButton = true;

    MoveHero();

    MouseLButtonPush = false;
    MouseLButton = false;

    TAKUMI_SKILL_LOGI(
        "operate ok reason=%s operate=%d Mouse=%d,%d",
        reason != nullptr ? reason : "?",
        SelectedOperate,
        MouseX,
        MouseY);
    g_ErrorReport.Write("[SkillAtk] operate=%d", SelectedOperate);
    return true;
}

bool FireWorldNpcTalk(const char* reason)
{
    if (!IsWorldSkillGestureScene() || IsWorldSkillGestureBlockedByUiAt(g_worldSkillTouch.downUiX, g_worldSkillTouch.downUiY))
    {
        return false;
    }

    SelectObjects();
    if (SelectedNpc < 0)
    {
        const int npcTarget = SelectCharacter(KIND_NPC);
        if (npcTarget >= 0)
        {
            SelectedNpc = npcTarget;
        }
    }

    if (SelectedNpc < 0)
    {
        return false;
    }

    g_worldSkillLongPressFired = true;
    g_worldSkillTouchConsumed = true;

    MouseRButtonPush = false;
    MouseRButton = false;
    MouseRButtonPop = false;

    MouseLButtonPop = false;
    MouseLButtonPush = true;
    MouseLButton = true;

    MoveHero();

    MouseLButtonPush = false;
    MouseLButton = false;

    TAKUMI_SKILL_LOGI(
        "npc talk ok reason=%s npc=%d Mouse=%d,%d",
        reason != nullptr ? reason : "?",
        SelectedNpc,
        MouseX,
        MouseY);
    g_ErrorReport.Write("[SkillAtk] npc talk npc=%d", SelectedNpc);
    return true;
}

bool FireWorldMeleeAttack(const char* reason)
{
    if (!IsWorldSkillGestureScene())
    {
        return false;
    }

    const ScopedWorldSkillAimMouse aimGuard;

    if (!RefreshAttackableTargetAtMouse())
    {
        return false;
    }

    if (MU_AndroidIsVirtualJoystickDrivingMouse())
    {
        MU_Android_StopJoystickMovementForCombat();
    }

    StopWorldSkillChannel();
    g_worldSkillPendingMeleeMs = 0;

    g_worldSkillTouchConsumed = true;

    MouseRButtonPush = false;
    MouseRButton = false;
    MouseRButtonPop = false;
    MouseLButtonPop = false;
    MouseLButtonPush = true;
    MouseLButton = true;

    Attack(Hero);

    MouseLButtonPush = false;
    MouseLButton = false;

    TAKUMI_SKILL_LOGI(
        "melee attack ok reason=%s target=%d Mouse=%d,%d",
        reason != nullptr ? reason : "?",
        SelectedCharacter,
        MouseX,
        MouseY);
    g_ErrorReport.Write(
        "[SkillAtk] melee ok reason=%s target=%d",
        reason != nullptr ? reason : "?",
        SelectedCharacter);
    g_worldSkillTouch.downMs = 0;
    return true;
}

bool FireWorldSkillAttack(const char* reason)
{
    if (g_worldSkillLongPressFired)
    {
        return true;
    }

    if (!IsWorldSkillGestureScene() || IsWorldSkillGestureBlockedByUiAt(g_worldSkillTouch.downUiX, g_worldSkillTouch.downUiY))
    {
        TAKUMI_SKILL_LOGW("skill attack blocked scene/ui reason=%s", reason != nullptr ? reason : "?");
        return false;
    }

    g_worldSkillPendingMeleeMs = 0;

    const int skillIndex = ResolveActiveHotbarSkillIndex();
    if (skillIndex < 0)
    {
        return false;
    }

    const int skillType = SkillTypeAtIndex(skillIndex);
    if (skillType <= 0 || skillType >= MAX_SKILLS)
    {
        return false;
    }

    const int previousSkillIndex = Hero->CurrentSkill;
    Hero->CurrentSkill = static_cast<BYTE>(skillIndex);

    WorldSkillTargetKind targetKind = WorldSkillTargetKind::None;
    if (!PrepareWorldSkillTarget(skillType, targetKind))
    {
        Hero->CurrentSkill = static_cast<BYTE>(previousSkillIndex);
        TAKUMI_SKILL_LOGW(
            "skill attack no target reason=%s skillIndex=%d skillType=%d",
            reason != nullptr ? reason : "?",
            skillIndex,
            skillType);
        return false;
    }

    const int target = SelectedCharacter;
    const bool channelSkill = IsDirectionChannelSkillType(skillType);

    const bool latchAutoChannel =
        channelSkill && reason != nullptr && strcmp(reason, "double-tap") == 0;
    if (latchAutoChannel)
    {
        g_worldSkillChannelLatched = true;
    }

    const ScopedWorldSkillAimMouse aimGuard;
    PrimeHeroForSkillCast();
    SetupMovementSkillForCast(skillIndex);

    bool castOk = false;
    if (channelSkill)
    {
        castOk = CastDirectionChannelSkill(Hero, skillType, static_cast<BYTE>(skillIndex));
    }
    else
    {
        PulseAndroidSkillAttack(false);
        castOk = (Attacking == 2);
    }

    if (!castOk)
    {
        if (latchAutoChannel)
        {
            g_worldSkillChannelLatched = false;
        }

        Hero->CurrentSkill = static_cast<BYTE>(previousSkillIndex);
        TAKUMI_SKILL_LOGW(
            "skill attack fail reason=%s skillIndex=%d skillType=%d",
            reason != nullptr ? reason : "?",
            skillIndex,
            skillType);
        g_ErrorReport.Write(
            "[SkillAtk] fail reason=%s skill=%d type=%d",
            reason != nullptr ? reason : "?",
            skillIndex,
            skillType);
        return false;
    }

    Hero->CurrentSkill = static_cast<BYTE>(previousSkillIndex);

    g_worldSkillLongPressFired = true;
    g_worldSkillTouchConsumed = true;
    g_worldSkillLastChannelTickMs = MU_MobileGetTicks();

    if (channelSkill)
    {
        if (!latchAutoChannel)
        {
            g_worldSkillChannelHold = true;
        }
    }

    TAKUMI_SKILL_LOGI(
        "skill attack ok reason=%s skillIndex=%d skillType=%d kind=%d target=%d channel=%d attacking=%d heroSkill=%d atkTime=%d",
        reason != nullptr ? reason : "?",
        skillIndex,
        skillType,
        static_cast<int>(targetKind),
        target,
        channelSkill ? 1 : 0,
        Attacking,
        Hero->Skill,
        Hero->AttackTime);
    g_ErrorReport.Write(
        "[SkillAtk] ok reason=%s skill=%d type=%d kind=%d target=%d channel=%d attacking=%d heroSkill=%d",
        reason != nullptr ? reason : "?",
        skillIndex,
        skillType,
        static_cast<int>(targetKind),
        target,
        channelSkill ? 1 : 0,
        Attacking,
        Hero->Skill);

    return true;
}

bool FireWorldSecondaryAction(const char* reason)
{
    if (g_worldSkillLongPressFired)
    {
        return true;
    }

    if (!IsWorldSkillGestureScene() || IsWorldSkillGestureBlockedByUiAt(g_worldSkillTouch.downUiX, g_worldSkillTouch.downUiY))
    {
        return false;
    }

    if (HasActiveHotbarSkill() && FireWorldSkillAttack(reason))
    {
        return true;
    }

    if (FireWorldPickUpItem(reason))
    {
        return true;
    }

    if (FireWorldOperateObject(reason))
    {
        return true;
    }

    if (FireWorldNpcTalk(reason))
    {
        return true;
    }

    return false;
}

} // namespace

bool TakumiAndroid_ResolveDirectionChannelTarget(CHARACTER* c)
{
    if (c == nullptr)
    {
        return false;
    }

    const int previousTarget = SelectedCharacter;
    if (g_worldSkillChannelLatched && TrySelectNearestAttackableMonster())
    {
        const bool ok = CheckTarget(c);
        if (!ok)
        {
            SelectedCharacter = previousTarget;
        }
        return ok;
    }

    SelectedCharacter = -1;
    const bool ok = CheckTarget(c);
    if (!ok)
    {
        SelectedCharacter = previousTarget;
    }
    return ok;
}

bool TakumiAndroid_HasActiveWorldSkillGesture()
{
    return g_worldSkillTouch.finger != static_cast<SDL_FingerID>(-1);
}

void TakumiAndroid_DisarmSkillMouseForMovement()
{
    // Joystick uses a separate finger: do not cancel an in-progress world long-press / channel.
    if (TakumiAndroid_HasActiveWorldSkillGesture())
    {
        return;
    }

    // Finger is up but tap-melee may still be in the double-tap wait window.
    if (g_worldSkillPendingMeleeMs != 0)
    {
        return;
    }

    StopWorldSkillChannel();

    g_worldSkillTouch.finger = static_cast<SDL_FingerID>(-1);
    g_worldSkillTouch.downMs = 0;
    g_worldSkillLongPressFired = false;
    g_worldSkillTouchConsumed = false;
    g_worldSkillLastShortTapMs = 0;
    g_worldSkillLastChannelTickMs = 0;

    if (Hero != nullptr)
    {
        Hero->AttackTime = 0;
        Hero->SkillSuccess = false;
    }

    Attacking = -1;

    ZeroMemory(&g_MovementSkill, sizeof(g_MovementSkill));

    MouseRButtonPop = false;
    MouseRButtonPush = false;
    MouseRButton = false;
    MouseRButtonPress = 0;
}

bool TakumiAndroid_IsWorldSkillFinger(const SDL_FingerID fingerId)
{
    return g_worldSkillTouch.finger != static_cast<SDL_FingerID>(-1)
        && g_worldSkillTouch.finger == fingerId;
}

bool TakumiAndroid_PeekInventoryUsePress()
{
    return g_inventoryUsePressPending;
}

void TakumiAndroid_CancelInventoryUsePress()
{
    g_inventoryUsePressPending = false;
}

bool TakumiAndroid_ConsumeInventoryUsePress()
{
    if (!g_inventoryUsePressPending)
    {
        return false;
    }

    g_inventoryUsePressPending = false;
    TAKUMI_INV_LOGI("consume inventory use press (Mouse=%d,%d)", MouseX, MouseY);
    return true;
}

void TakumiAndroid_PulseRightClick()
{
    MouseRButtonPop = false;
    MouseRButtonPush = true;
    MouseRButton = true;
    g_inventoryUsePressPending = true;
    SyncAndroidRightButtonKeyPress();
    TAKUMI_INV_LOGI("pulse RMB use (Mouse=%d,%d)", MouseX, MouseY);
}

void TakumiAndroid_ProcessWorldSkillFrame()
{
    if (g_worldSkillTouch.finger != static_cast<SDL_FingerID>(-1))
    {
        const uint32_t nowMs = MU_MobileGetTicks();
        const uint32_t heldMs =
            (g_worldSkillTouch.downMs > 0) ? (nowMs - g_worldSkillTouch.downMs) : 0;

        if (!g_worldSkillLongPressFired && heldMs >= kWorldSkillLongPressMs)
        {
            FireWorldSecondaryAction("long-press-frame");
        }
        else if (g_worldSkillChannelHold)
        {
            FireWorldSkillChannelTick("hold-frame");
        }
    }

    if (g_worldSkillChannelLatched)
    {
        FireWorldSkillChannelTick("latched");
    }

    if (g_worldSkillPendingMeleeMs == 0)
    {
        return;
    }

    const uint32_t nowMs = MU_MobileGetTicks();
    if ((nowMs - g_worldSkillPendingMeleeMs) <= kWorldSkillDoubleTapMaxMs)
    {
        return;
    }

    g_worldSkillPendingMeleeMs = 0;
    g_worldSkillTouch.downMs = 0;
    FireWorldMeleeAttack("tap-delayed");
}

void TakumiAndroid_ProcessInventoryUseFrame()
{
    if (!g_inventoryUsePressPending)
    {
        return;
    }

    if (!IsInventoryUiOpenForUse())
    {
        TAKUMI_INV_LOGW("pending use dropped: inventory not open/repair mode");
        g_inventoryUsePressPending = false;
        return;
    }

    if (!IsMouseOverInventoryPanel())
    {
        TAKUMI_INV_LOGW(
            "pending use wait: cursor outside bag Mouse=%d,%d",
            MouseX,
            MouseY);
        return;
    }

    if (AndroidTryUseItemNow())
    {
        g_inventoryUsePressPending = false;
    }
}

bool TakumiAndroid_HandleInventoryTouchDown(const SDL_TouchFingerEvent& touch)
{
    if (!IsInventoryUiOpenForUse())
    {
        return false;
    }

    if (!IsMouseOverInventoryPanel())
    {
        return false;
    }

    float uiX = 0.0f;
    float uiY = 0.0f;
    TouchToVirtualUi(touch, uiX, uiY);
    g_inventoryTouch.finger = touch.fingerId;
    g_inventoryTouch.downMs = MU_MobileGetTicks();
    g_inventoryTouch.downUiX = uiX;
    g_inventoryTouch.downUiY = uiY;
    g_inventoryLongPressFired = false;
    TAKUMI_INV_LOGI(
        "touch down finger=%lld ui=(%.0f,%.0f) Mouse=(%d,%d)",
        static_cast<long long>(touch.fingerId),
        uiX,
        uiY,
        MouseX,
        MouseY);
    return false;
}

bool TakumiAndroid_HandleInventoryTouchMove(const SDL_TouchFingerEvent& touch)
{
    if (g_inventoryTouch.finger != touch.fingerId)
    {
        return false;
    }

    float uiX = 0.0f;
    float uiY = 0.0f;
    TouchToVirtualUi(touch, uiX, uiY);
    const float dx = uiX - g_inventoryTouch.downUiX;
    const float dy = uiY - g_inventoryTouch.downUiY;
    if ((dx * dx) + (dy * dy) > (kInventoryLongPressCancelMoveUi * kInventoryLongPressCancelMoveUi))
    {
        TAKUMI_INV_LOGI("long-press cancelled (moved %.0f ui)", std::sqrt((dx * dx) + (dy * dy)));
        g_inventoryTouch.finger = static_cast<SDL_FingerID>(-1);
        return false;
    }

    if (!g_inventoryLongPressFired && IsMouseOverInventoryPanel())
    {
        const uint32_t nowMs = MU_MobileGetTicks();
        const uint32_t heldMs =
            (g_inventoryTouch.downMs > 0) ? (nowMs - g_inventoryTouch.downMs) : 0;
        if (heldMs >= kInventoryUseLongPressMs)
        {
            TAKUMI_INV_LOGI("long-press use (motion) heldMs=%u Mouse=(%d,%d)", heldMs, MouseX, MouseY);
            g_ErrorReport.Write("[InvUse] long-press use heldMs=%u", heldMs);
            FireInventoryLongPressUse();
            return true;
        }
    }

    return false;
}

bool TakumiAndroid_HandleInventoryTouchUp(const SDL_TouchFingerEvent& touch)
{
    if (g_inventoryTouch.finger != touch.fingerId)
    {
        return false;
    }

    const uint32_t nowMs = MU_MobileGetTicks();
    const uint32_t heldMs = (g_inventoryTouch.downMs > 0) ? (nowMs - g_inventoryTouch.downMs) : 0;
    g_inventoryTouch.finger = static_cast<SDL_FingerID>(-1);

    if (!IsMouseOverInventoryPanel())
    {
        TAKUMI_INV_LOGW("touch up outside inventory Mouse=(%d,%d)", MouseX, MouseY);
        return false;
    }

    float uiX = 0.0f;
    float uiY = 0.0f;
    TouchToVirtualUi(touch, uiX, uiY);

    if (!g_inventoryLongPressFired && heldMs >= kInventoryUseLongPressMs)
    {
        TAKUMI_INV_LOGI("long-press use heldMs=%u ui=(%.0f,%.0f) Mouse=(%d,%d)", heldMs, uiX, uiY, MouseX, MouseY);
        g_ErrorReport.Write("[InvUse] long-press use (up) heldMs=%u", heldMs);
        FireInventoryLongPressUse();
        return true;
    }

    if (g_inventoryLongPressFired)
    {
        return true;
    }

    const float dx = uiX - g_inventoryLastShortTapUiX;
    const float dy = uiY - g_inventoryLastShortTapUiY;
    const bool doubleTap =
        g_inventoryLastShortTapMs > 0
        && (nowMs - g_inventoryLastShortTapMs) <= kInventoryDoubleTapMaxMs
        && (dx * dx) + (dy * dy) <= (kInventoryDoubleTapMaxDistUi * kInventoryDoubleTapMaxDistUi);

    g_inventoryLastShortTapMs = nowMs;
    g_inventoryLastShortTapUiX = uiX;
    g_inventoryLastShortTapUiY = uiY;

    if (doubleTap)
    {
        g_inventoryLastShortTapMs = 0;
        TAKUMI_INV_LOGI("double-tap use heldMs=%u ui=(%.0f,%.0f) Mouse=(%d,%d)", heldMs, uiX, uiY, MouseX, MouseY);
        g_ErrorReport.Write("[InvUse] double-tap use");
        FireInventoryLongPressUse();
        return true;
    }

    TAKUMI_INV_LOGI("tap end (no use) heldMs=%u ui=(%.0f,%.0f)", heldMs, uiX, uiY);
    return false;
}

bool TakumiAndroid_HandleWorldSkillTouchDown(const SDL_TouchFingerEvent& touch)
{
    float uiX = 0.0f;
    float uiY = 0.0f;
    TouchToVirtualUi(touch, uiX, uiY);

    if (!IsWorldSkillGestureScene() || IsWorldSkillGestureBlockedByUiAt(uiX, uiY))
    {
        return false;
    }

    g_worldSkillTouch.finger = touch.fingerId;
    g_worldSkillTouch.downMs = MU_MobileGetTicks();
    g_worldSkillTouch.downUiX = uiX;
    g_worldSkillTouch.downUiY = uiY;
    g_worldSkillTouch.aimMouseX = std::clamp(static_cast<int>(uiX), 0, 640);
    g_worldSkillTouch.aimMouseY = std::clamp(static_cast<int>(uiY), 0, 480);
    g_worldSkillLongPressFired = false;
    g_worldSkillTouchConsumed = false;
    g_worldSkillPendingMeleeMs = 0;
    g_worldSkillChannelHold = false;
    g_worldSkillChannelLatched = false;
    TAKUMI_SKILL_LOGI(
        "touch down finger=%lld ui=(%.0f,%.0f) Mouse=(%d,%d)",
        static_cast<long long>(touch.fingerId),
        uiX,
        uiY,
        MouseX,
        MouseY);
    // Do not consume the touch: SDL path still sets MouseLButton for PC click-to-walk on empty map.
    return false;
}

bool TakumiAndroid_HandleWorldSkillTouchMove(const SDL_TouchFingerEvent& touch)
{
    if (g_worldSkillTouch.finger != touch.fingerId)
    {
        return false;
    }

    float uiX = 0.0f;
    float uiY = 0.0f;
    TouchToVirtualUi(touch, uiX, uiY);
    const float dx = uiX - g_worldSkillTouch.downUiX;
    const float dy = uiY - g_worldSkillTouch.downUiY;
    if ((dx * dx) + (dy * dy) > (kWorldSkillLongPressCancelMoveUi * kWorldSkillLongPressCancelMoveUi))
    {
        TAKUMI_SKILL_LOGI("long-press cancelled (moved %.0f ui)", std::sqrt((dx * dx) + (dy * dy)));
        g_worldSkillTouch.finger = static_cast<SDL_FingerID>(-1);
        return false;
    }

    if (!g_worldSkillLongPressFired)
    {
        const uint32_t nowMs = MU_MobileGetTicks();
        const uint32_t heldMs =
            (g_worldSkillTouch.downMs > 0) ? (nowMs - g_worldSkillTouch.downMs) : 0;
        if (heldMs >= kWorldSkillLongPressMs)
        {
            FireWorldSecondaryAction("long-press");
        }
    }
    else if (g_worldSkillChannelHold)
    {
        FireWorldSkillChannelTick("hold");
    }

    return g_worldSkillTouchConsumed;
}

bool TakumiAndroid_HandleWorldSkillTouchUp(const SDL_TouchFingerEvent& touch)
{
    if (g_worldSkillTouch.finger != touch.fingerId)
    {
        return false;
    }

    const uint32_t nowMs = MU_MobileGetTicks();
    const uint32_t heldMs = (g_worldSkillTouch.downMs > 0) ? (nowMs - g_worldSkillTouch.downMs) : 0;
    g_worldSkillTouch.finger = static_cast<SDL_FingerID>(-1);

    float uiX = 0.0f;
    float uiY = 0.0f;
    TouchToVirtualUi(touch, uiX, uiY);

    if (!g_worldSkillLongPressFired && heldMs >= kWorldSkillLongPressMs)
    {
        FireWorldSecondaryAction("long-press-up");
    }

    if (g_worldSkillTouchConsumed)
    {
        if (g_worldSkillChannelHold || g_worldSkillChannelLatched)
        {
            StopWorldSkillChannel();
        }
        return true;
    }

    if (g_worldSkillChannelLatched)
    {
        StopWorldSkillChannel();
    }

    const float dx = uiX - g_worldSkillLastShortTapUiX;
    const float dy = uiY - g_worldSkillLastShortTapUiY;
    const bool doubleTap =
        g_worldSkillLastShortTapMs > 0
        && (nowMs - g_worldSkillLastShortTapMs) <= kWorldSkillDoubleTapMaxMs
        && (dx * dx) + (dy * dy) <= (kWorldSkillDoubleTapMaxDistUi * kWorldSkillDoubleTapMaxDistUi);

    g_worldSkillLastShortTapMs = nowMs;
    g_worldSkillLastShortTapUiX = uiX;
    g_worldSkillLastShortTapUiY = uiY;

    if (doubleTap)
    {
        g_worldSkillLastShortTapMs = 0;
        g_worldSkillPendingMeleeMs = 0;
        if (FireWorldSecondaryAction("double-tap"))
        {
            return true;
        }
    }

    if (heldMs < kWorldSkillLongPressMs && !g_worldSkillLongPressFired)
    {
        if (g_worldSkillChannelLatched)
        {
            StopWorldSkillChannel();
            TAKUMI_SKILL_LOGI("tap end channel latch off heldMs=%u", heldMs);
            return false;
        }

        if (FireWorldMeleeAttack("tap"))
        {
            g_worldSkillTouch.downMs = 0;
            return true;
        }

        // No attackable target: let SDL finger-up emit MouseLButtonPop (PC LMB click-to-walk / game attack path).
        g_worldSkillTouch.downMs = 0;
        TAKUMI_SKILL_LOGI("tap end passthrough ui=(%.0f,%.0f) heldMs=%u", uiX, uiY, heldMs);
        return false;
    }

    TAKUMI_SKILL_LOGI("tap end (no skill) heldMs=%u ui=(%.0f,%.0f)", heldMs, uiX, uiY);
    g_ErrorReport.Write("[SkillAtk] tap end no action heldMs=%u", heldMs);
    return false;
}

#endif // __ANDROID__
