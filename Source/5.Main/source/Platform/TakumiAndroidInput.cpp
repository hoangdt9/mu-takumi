#ifdef __ANDROID__

#include "stdafx.h"
#include "Platform/TakumiAndroidInput.h"
#include "NewUISystem.h"
#include "NewUIMyInventory.h"
#include "NewUIInventoryCtrl.h"
#include "NewUICommon.h"
#include "ZzzOpenglUtil.h"
#include "MobilePlatform.h"
#include "Utilities/Log/ErrorReport.h"

#include <algorithm>
#include <cmath>

#include <android/log.h>

extern CErrorReport g_ErrorReport;
#define TAKUMI_INV_LOGI(...) __android_log_print(ANDROID_LOG_INFO, "TakumiInvUse", __VA_ARGS__)
#define TAKUMI_INV_LOGW(...) __android_log_print(ANDROID_LOG_WARN, "TakumiInvUse", __VA_ARGS__)

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

} // namespace

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

#endif // __ANDROID__
