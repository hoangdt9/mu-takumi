#if defined(__ANDROID__) || defined(MU_IOS)

#include "MobileChatHud.h"

#include "MobileHud.h"
#include "MobilePlatform.h"

#include "../NewUIChatInputBox.h"
#include "../NewUIChatLogWindow.h"
#include "../NewUISystem.h"
#include "../ZzzInterface.h"
#include "../_enum.h"

#include <SDL.h>

#include <algorithm>
#include <cstdio>
#include <cmath>

extern int DisplayHeightExt;
extern bool MouseLButton;
extern bool MouseLButtonPush;
extern bool MouseLButtonPop;

namespace
{
constexpr const char* kLayoutConfigRelativePath = "Data/Local/android_mobile_chat_layout.cfg";
constexpr int kChatBarWidth = SEASON3B::CNewUIChatInputBox::CHATBOX_WIDTH;
constexpr int kChatPanelWidth = kChatBarWidth;
constexpr int kChatInputHeight = SEASON3B::CNewUIChatInputBox::CHATBOX_HEIGHT;
constexpr int kDefaultInputY = 428;
constexpr int kDefaultPanelX = (640 - kChatBarWidth) / 2;
constexpr int kMinShowingLines = 3;
constexpr int kMaxShowingLines = 12;
constexpr int kDefaultShowingLines = 6;
constexpr float kDefaultAlpha = 0.52f;
constexpr float kLineHeightUi = 15.0f;
constexpr float kLogWndTopBottomEdge = 2.0f;
constexpr float kLogScrollTopBottomPart = 3.0f;
constexpr float kLogResizingBtnHeight = 10.0f;
constexpr float kResizeHandleHitPadY = 10.0f;

struct MobileChatLayoutState
{
    int panelX = kDefaultPanelX;
    int inputY = kDefaultInputY;
    int showingLines = kDefaultShowingLines;
    float alpha = kDefaultAlpha;
    bool collapsed = false;
    bool pinnedVisible = true;
};

MobileChatLayoutState g_mobileChatLayout{};
bool g_mobileChatLayoutLoaded = false;
bool g_mobileChatLinesApplied = false;

enum class ChatUiGesture : int
{
    None = 0,
    Resize,
    Scroll,
    Panel,
};

SDL_FingerID g_chatUiCaptureFingerId = static_cast<SDL_FingerID>(-1);
ChatUiGesture g_chatUiGesture = ChatUiGesture::None;
float g_chatResizeStartUiY = 0.0f;
int g_chatResizeStartLines = kDefaultShowingLines;

Uint32 g_chatResizeLastTapMs = 0;
float g_chatResizeLastTapUiX = 0.0f;
float g_chatResizeLastTapUiY = 0.0f;
constexpr Uint32 kResizeDoubleTapMs = 320;
constexpr float kResizeDoubleTapDist = 24.0f;

std::string BuildLayoutConfigPath()
{
    const std::string dataRoot = MU_MobileGetExternalDataPath();
    if (dataRoot.empty())
    {
        return kLayoutConfigRelativePath;
    }

    if (dataRoot.back() == '/')
    {
        return dataRoot + kLayoutConfigRelativePath;
    }

    return dataRoot + "/" + kLayoutConfigRelativePath;
}

void LoadMobileChatLayout()
{
    if (g_mobileChatLayoutLoaded)
    {
        return;
    }

    g_mobileChatLayoutLoaded = true;

    FILE* file = fopen(BuildLayoutConfigPath().c_str(), "rb");
    if (file == nullptr)
    {
        return;
    }

    int lines = kDefaultShowingLines;
    float alpha = kDefaultAlpha;
    int collapsed = 0;
    if (fscanf(file, "%d %f %d", &lines, &alpha, &collapsed) == 3)
    {
        g_mobileChatLayout.showingLines = std::clamp(lines, kMinShowingLines, kMaxShowingLines);
        g_mobileChatLayout.alpha = std::clamp(alpha, 0.2f, 0.85f);
        g_mobileChatLayout.collapsed = (collapsed != 0);
    }

    fclose(file);
}

void SaveMobileChatLayout()
{
    FILE* file = fopen(BuildLayoutConfigPath().c_str(), "wb");
    if (file == nullptr)
    {
        return;
    }

    fprintf(
        file,
        "%d %.2f %d\n",
        g_mobileChatLayout.showingLines,
        g_mobileChatLayout.alpha,
        g_mobileChatLayout.collapsed ? 1 : 0);
    fclose(file);
}

float MobileChatInputBaseY()
{
    return static_cast<float>(g_mobileChatLayout.inputY + DisplayHeightExt);
}

int GetEffectiveShowingLines()
{
    if (g_mobileChatLayout.collapsed)
    {
        return kMinShowingLines;
    }

    return std::clamp(g_mobileChatLayout.showingLines, kMinShowingLines, kMaxShowingLines);
}

float ComputeLogHeightPx(int showingLines)
{
    return static_cast<float>(showingLines) * kLineHeightUi
        + (kLogScrollTopBottomPart * 2.0f)
        + (kLogWndTopBottomEdge * 2.0f);
}

MobileChatPanelRect BuildPanelRect()
{
    const int lines = GetEffectiveShowingLines();
    const float logH = ComputeLogHeightPx(lines);
    const float inputY = MobileChatInputBaseY();
    const float totalH = logH + static_cast<float>(kChatInputHeight);

    return {
        static_cast<float>(g_mobileChatLayout.panelX),
        inputY - logH,
        static_cast<float>(kChatPanelWidth),
        totalH
    };
}
} // namespace

void MU_MobileChatHudInit()
{
    LoadMobileChatLayout();
    g_mobileChatLinesApplied = false;
}

void MU_MobileChatHudSetShowingLines(int showingLines)
{
    LoadMobileChatLayout();
    g_mobileChatLayout.showingLines = std::clamp(showingLines, kMinShowingLines, kMaxShowingLines);
    g_mobileChatLayout.collapsed = false;
    SaveMobileChatLayout();
}

void MU_MobileChatHudSyncLayout()
{
    if (!MU_MobileIsModernMobileHudEnabled())
    {
        return;
    }

    LoadMobileChatLayout();

    if (g_pNewUISystem == nullptr || g_pChatInputBox == nullptr || g_pChatListBox == nullptr)
    {
        return;
    }

    g_mobileChatLayout.panelX = kDefaultPanelX;
    g_mobileChatLayout.inputY = kDefaultInputY;

    const int panelX = g_mobileChatLayout.panelX;
    const int inputY = g_mobileChatLayout.inputY + DisplayHeightExt;

    g_pChatInputBox->SetWndPos(panelX, inputY);
    g_pChatInputBox->RelayoutMobilePanel();

    g_pChatListBox->SetPosition(panelX, inputY);
    if (!g_mobileChatLinesApplied)
    {
        g_pChatListBox->SetNumberOfShowingLines(GetEffectiveShowingLines());
        g_mobileChatLinesApplied = true;
    }
    g_pChatListBox->SetBackAlpha(g_mobileChatLayout.alpha);
    g_pChatListBox->ShowFrame();
    g_pChatListBox->ShowChatLog(false);
    g_pChatListBox->UpdateWndSize();
    g_pChatListBox->UpdateScrollPos();

    if (g_mobileChatLayout.pinnedVisible
        && !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX))
    {
        g_pNewUISystem->Show(SEASON3B::INTERFACE_CHATINPUTBOX);
    }
}

MobileChatPanelRect MU_MobileGetChatPanelRect()
{
    LoadMobileChatLayout();
    return BuildPanelRect();
}

MobileChatPanelRect MU_MobileGetChatResizeHandleRect()
{
    LoadMobileChatLayout();

    const int lines = GetEffectiveShowingLines();
    const float logH = ComputeLogHeightPx(lines);
    const float inputY = MobileChatInputBaseY();
    const float x = static_cast<float>(g_mobileChatLayout.panelX);

    return {
        x,
        inputY - logH - kLogResizingBtnHeight,
        static_cast<float>(kChatPanelWidth),
        kLogResizingBtnHeight
    };
}

bool MU_MobileHitTestChatPanel(float uiX, float uiY)
{
    if (!MU_MobileIsModernMobileHudEnabled())
    {
        return false;
    }

    return MU_MobileHitTestChatResizeHandle(uiX, uiY)
        || MU_MobileHitTestChatLogArea(uiX, uiY)
        || MU_MobileHitTestChatInputBar(uiX, uiY);
}

bool MU_MobileHitTestChatInputBar(float uiX, float uiY)
{
    if (!MU_MobileIsModernMobileHudEnabled())
    {
        return false;
    }

    LoadMobileChatLayout();
    const float x = static_cast<float>(g_mobileChatLayout.panelX);
    const float y = MobileChatInputBaseY();
    return uiX >= x
        && uiX <= (x + static_cast<float>(kChatBarWidth))
        && uiY >= y
        && uiY <= (y + static_cast<float>(kChatInputHeight));
}

bool MU_MobileHitTestChatLogArea(float uiX, float uiY)
{
    if (!MU_MobileIsModernMobileHudEnabled())
    {
        return false;
    }

    LoadMobileChatLayout();

    const MobileChatPanelRect panel = MU_MobileGetChatPanelRect();
    const MobileChatPanelRect handle = MU_MobileGetChatResizeHandleRect();
    const float logTop = handle.y + handle.h;
    const float logBottom = MobileChatInputBaseY();

    if (logBottom <= logTop)
    {
        return false;
    }

    return uiX >= panel.x
        && uiX <= (panel.x + panel.w)
        && uiY >= logTop
        && uiY < logBottom;
}

bool MU_MobileHitTestChatResizeHandle(float uiX, float uiY)
{
    if (!MU_MobileIsModernMobileHudEnabled())
    {
        return false;
    }

    const MobileChatPanelRect handle = MU_MobileGetChatResizeHandleRect();
    return uiX >= handle.x
        && uiX <= (handle.x + handle.w)
        && uiY >= (handle.y - kResizeHandleHitPadY)
        && uiY <= (handle.y + handle.h + kResizeHandleHitPadY);
}

void ApplyChatUiCaptureFlags()
{
    MouseOnWindow = true;
    MouseLButtonPop = false;
    MouseLButtonPush = true;
    MouseLButton = true;
}

void ReleaseChatUiCapture(SDL_FingerID fingerId)
{
    if (g_chatUiCaptureFingerId != fingerId)
    {
        return;
    }

    if (g_chatUiGesture == ChatUiGesture::Scroll && g_pChatListBox != nullptr)
    {
        g_pChatListBox->EndScrollDrag();
    }
    else if (g_chatUiGesture == ChatUiGesture::Resize)
    {
        SaveMobileChatLayout();
    }

    g_chatUiCaptureFingerId = static_cast<SDL_FingerID>(-1);
    g_chatUiGesture = ChatUiGesture::None;

    MouseLButton = false;
    MouseLButtonPush = false;
    MouseLButtonPop = false;
}

bool MU_MobileIsChatUiCapturing()
{
    return g_chatUiCaptureFingerId != static_cast<SDL_FingerID>(-1);
}

bool MU_MobileIsChatUiFingerCaptured(SDL_FingerID fingerId)
{
    return g_chatUiCaptureFingerId == fingerId;
}

bool MU_MobileHandleChatResizeFingerDownAt(float uiX, float uiY, SDL_FingerID fingerId)
{
    return MU_MobileHandleChatUiFingerDownAt(uiX, uiY, fingerId)
        && g_chatUiGesture == ChatUiGesture::Resize;
}

bool MU_MobileHandleChatResizeFingerMotionAt(float uiY, SDL_FingerID fingerId)
{
    return MU_MobileHandleChatUiFingerMotionAt(0.0f, uiY, fingerId)
        && g_chatUiGesture == ChatUiGesture::Resize;
}

bool MU_MobileHandleChatResizeFingerUp(SDL_FingerID fingerId)
{
    if (!MU_MobileIsChatUiFingerCaptured(fingerId))
    {
        return false;
    }

    ReleaseChatUiCapture(fingerId);
    return true;
}

bool MU_MobileHandleChatUiFingerDownAt(float uiX, float uiY, SDL_FingerID fingerId)
{
    if (!MU_MobileIsModernMobileHudEnabled())
    {
        return false;
    }

    MU_MobileChatHudSyncLayout();

    if (MU_MobileHitTestChatResizeHandle(uiX, uiY))
    {
        const Uint32 now = SDL_GetTicks();
        const float dx = uiX - g_chatResizeLastTapUiX;
        const float dy = uiY - g_chatResizeLastTapUiY;
        const float dist2 = dx * dx + dy * dy;
        if ((now - g_chatResizeLastTapMs) <= kResizeDoubleTapMs
            && dist2 <= (kResizeDoubleTapDist * kResizeDoubleTapDist))
        {
            MU_MobileToggleChatPanelCollapsed();
            g_chatResizeLastTapMs = 0;
            return true;
        }

        g_chatResizeLastTapMs = now;
        g_chatResizeLastTapUiX = uiX;
        g_chatResizeLastTapUiY = uiY;

        g_chatUiCaptureFingerId = fingerId;
        g_chatUiGesture = ChatUiGesture::Resize;
        g_chatResizeStartUiY = uiY;
        g_chatResizeStartLines = g_mobileChatLayout.showingLines;
        ApplyChatUiCaptureFlags();
        return true;
    }

    if (g_pChatListBox != nullptr
        && g_pChatListBox->IsShowFrame()
        && g_pChatListBox->HitTestScrollBar(uiX, uiY))
    {
        g_chatUiCaptureFingerId = fingerId;
        g_chatUiGesture = ChatUiGesture::Scroll;
        g_pChatListBox->BeginScrollDragAt(static_cast<int>(uiY));
        g_pChatListBox->UpdateScrollDragAt(static_cast<int>(uiY));
        ApplyChatUiCaptureFlags();
        return true;
    }

    if (!MU_MobileHitTestChatPanel(uiX, uiY))
    {
        return false;
    }

    g_chatUiCaptureFingerId = fingerId;
    g_chatUiGesture = ChatUiGesture::Panel;
    ApplyChatUiCaptureFlags();
    return true;
}

bool MU_MobileHandleChatUiFingerMotionAt(float uiX, float uiY, SDL_FingerID fingerId)
{
    if (g_chatUiCaptureFingerId != fingerId)
    {
        return false;
    }

    MouseOnWindow = true;
    MouseX = static_cast<int>(uiX);
    MouseY = static_cast<int>(uiY);

    if (g_chatUiGesture == ChatUiGesture::Resize)
    {
        const float delta = g_chatResizeStartUiY - uiY;
        const int lineDelta = static_cast<int>(std::lround(delta / kLineHeightUi));
        const int newLines = std::clamp(
            g_chatResizeStartLines + lineDelta,
            kMinShowingLines,
            kMaxShowingLines);
        g_mobileChatLayout.showingLines = newLines;
        g_mobileChatLayout.collapsed = false;
        if (g_pChatListBox != nullptr)
        {
            g_pChatListBox->SetNumberOfShowingLines(newLines);
        }
        MU_MobileChatHudSyncLayout();
        return true;
    }

    if (g_chatUiGesture == ChatUiGesture::Scroll && g_pChatListBox != nullptr)
    {
        g_pChatListBox->UpdateScrollDragAt(static_cast<int>(uiY));
        return true;
    }

    return true;
}

bool MU_MobileHandleChatUiFingerUp(SDL_FingerID fingerId)
{
    if (!MU_MobileIsChatUiFingerCaptured(fingerId))
    {
        return false;
    }

    ReleaseChatUiCapture(fingerId);
    return true;
}

void MU_MobileToggleChatPanelCollapsed()
{
    LoadMobileChatLayout();
    g_mobileChatLayout.collapsed = !g_mobileChatLayout.collapsed;
    SaveMobileChatLayout();
    if (g_pChatListBox != nullptr)
    {
        g_pChatListBox->SetNumberOfShowingLines(GetEffectiveShowingLines());
    }
    MU_MobileChatHudSyncLayout();
}

void MU_MobileChatHudRestoreLegacyLayout()
{
    if (g_pChatInputBox == nullptr || g_pChatListBox == nullptr)
    {
        return;
    }

    g_mobileChatLinesApplied = false;

    const int legacyY = (480 - 51 - 47) + DisplayHeightExt;
    g_pChatInputBox->SetWndPos(0, legacyY);
    g_pChatInputBox->RelayoutMobilePanel();
    g_pChatListBox->SetPosition(0, legacyY);
    g_pChatListBox->SetNumberOfShowingLines(6);
    g_pChatListBox->SetBackAlpha(0.6f);
    g_pChatListBox->UpdateWndSize();
    g_pChatListBox->UpdateScrollPos();
}

#endif
