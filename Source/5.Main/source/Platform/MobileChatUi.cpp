#if defined(__ANDROID__) || defined(MU_IOS)

#include "../stdafx.h"
#include "MobileChatUi.h"

#include "MobileChatHud.h"
#include "MobileHud.h"

#include "../UIControls.h"
#include "../ZzzOpenglUtil.h"

extern HFONT g_hFont;

void MU_MobileChatUiRenderResizeHandle()
{
    if (!MU_MobileIsModernMobileHudEnabled())
    {
        return;
    }

    const MobileChatPanelRect handle = MU_MobileGetChatResizeHandleRect();

    EnableAlphaTest();
    glDisable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.05f, 0.08f, 0.14f, 0.72f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(handle.x, handle.y);
    glVertex2f(handle.x + handle.w, handle.y);
    glVertex2f(handle.x + handle.w, handle.y + handle.h);
    glVertex2f(handle.x, handle.y + handle.h);
    glEnd();

    const float cx = handle.x + handle.w * 0.5f;
    const float cy = handle.y + handle.h * 0.5f;
    const float lineW = handle.w * 0.22f;
    glColor4f(0.85f, 0.90f, 1.0f, 0.85f);
    glLineWidth(2.0f);
    for (int i = -1; i <= 1; ++i)
    {
        const float ly = cy + static_cast<float>(i) * (handle.h * 0.18f);
        glBegin(GL_LINES);
        glVertex2f(cx - lineW * 0.5f, ly);
        glVertex2f(cx + lineW * 0.5f, ly);
        glEnd();
    }
    glLineWidth(1.0f);

    if (g_hFont != nullptr && g_pRenderText != nullptr)
    {
        g_pRenderText->SetFont(g_hFont);
        g_pRenderText->SetBgColor(0);
        g_pRenderText->SetTextColor(200, 220, 255, 220);
        g_pRenderText->RenderText(
            static_cast<int>(handle.x + handle.w * 0.5f),
            static_cast<int>(handle.y + 2.0f),
            _T("Chat"),
            0,
            0,
            RT3_WRITE_CENTER);
    }

    glEnable(GL_TEXTURE_2D);
    glColor4f(1.f, 1.f, 1.f, 1.f);
}

#endif
