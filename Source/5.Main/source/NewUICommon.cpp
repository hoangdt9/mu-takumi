// NewUICommon.cpp: implementation of the CNewUICommon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUICommon.h"
#include "NewUIRenderNumber.h"
#include "NewUISystem.h"
#include "NewUICommonMessageBox.h"
#include "ZzzTexture.h"
#include "ZzzOpenglUtil.h"
extern float g_fScreenRate_x;	// ¡Ø
extern float g_fScreenRate_y;
extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayWinMid;
extern int DisplayWinExt;
extern int DisplayWinReal;
extern int MouseX, MouseY;
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
extern bool g_bWndActive;
#endif	// ASG_FIX_ACTIVATE_APP_INPUT

bool SEASON3B::CreateOkMessageBox(const unicode::t_string& strMsg, DWORD dwColor, float fPriority)
{
	CNewUICommonMessageBox* pMsgBox = g_MessageBox->NewMessageBox(MSGBOX_CLASS(CNewUICommonMessageBox));
	if(pMsgBox)
	{
		return pMsgBox->Create(MSGBOX_COMMON_TYPE_OK, strMsg, dwColor);
	}
	return false;
}

int SEASON3B::IsPurchaseShop() 
{ 
	if( g_pMyShopInventory->IsVisible() ) 
	{
		return 1;
	}
	else if( g_pPurchaseShopInventory->IsVisible() ) 
	{
		return 2;
	}

	return -1;
}

bool SEASON3B::CheckMouseIn(int x, int y, int width, int height)
{
	if (MouseX >= x && MouseX < x + width && MouseY >= y && MouseY < y + height && GetForegroundWindow() == g_hWnd)
	{

		return true;
	}

	return false;
}

void SEASON3B::RenderImage(GLuint uiImageType, float x, float y, float width, float height)
{

	BITMAP_t *pImage = &Bitmaps[uiImageType];

	float u, v, uw, vh;

	u = 0.5f / (float)pImage->Width;
	v = 0.5f / (float)pImage->Height;
	uw = (width - 0.5f) / (float)pImage->Width;
	vh = (height - 0.5f) / (float)pImage->Height;

	RenderBitmap(uiImageType, x, y, width, height, u, v, uw - u, vh - v);	
}
//===BCUstom
int SEASON3B::TextDraw(HFONT font, int PosX, int PosY, DWORD color, DWORD bkcolor, int Width, int Height, BYTE Align, LPCTSTR Text, ...) 
{
	va_list args;
	va_start(args, Text);
	TCHAR Buff[256];
	_vsntprintf(Buff, sizeof(Buff) / sizeof(TCHAR), Text, args);
	va_end(args);

	int LineCount = 0;

	char* Line = strtok(Buff, "\n##");
	DWORD BKTextColor = g_pRenderText->GetTextColor();
	DWORD BKBGColor = g_pRenderText->GetBgColor();
	g_pRenderText->SetFont(font);
	g_pRenderText->SetTextColor(GetRed(color), GetGreen(color), GetBlue(color), GetAlpha(color));
	g_pRenderText->SetBgColor(GetRed(bkcolor), GetGreen(bkcolor), GetBlue(bkcolor), GetAlpha(bkcolor));
	EnableAlphaTest();
	glColor3f(1.0, 1.0, 1.0);

	while (Line != NULL)
	{
		g_pRenderText->RenderText(PosX, PosY, Line, Width, Height, Align, 0);
		PosY += 10;
		Line = strtok(NULL, "\n##");
		LineCount++;
	}
	EnableAlphaTest(false);
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(BKTextColor);
	g_pRenderText->SetBgColor(BKBGColor);
	return LineCount;
}

void SEASON3B::DrawFrame(GLuint ModelID, float X, float Y, float W, float H, float SourceX, float SourceY, float SourceW, float SourceH, float ScaleW, float ScaleH, bool ScaleUp)
{
	float v1, v2;

	if (W) {
		W /= ScaleW;
		v1 = ScaleW / (SourceW / W);
	}
	else {
		v1 = ScaleW;
	}

	if (H) {
		H /= ScaleH;
		v2 = ScaleH / (SourceH / H);
	}
	else {
		v2 = ScaleH;
	}
	RenderBitmap(ModelID, X, Y, W, H, SourceX / SourceW, SourceY / SourceH, v1, v2, ScaleUp, 1, 0.0);
}


void SEASON3B::RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv,float uw, float vh, DWORD color)
{
	RenderColorBitmap(uiImageType, x, y, width, height, su, sv, uw, vh, color);
}

void SEASON3B::RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv)
{
	BITMAP_t *pImage = &Bitmaps[uiImageType];

	float u, v, uw, vh;
	u = ((su + 0.5f) / (float)pImage->Width);
	v = ((sv + 0.5f) / (float)pImage->Height);
	uw = (width - 0.5f) / (float)pImage->Width - (0.5f / (float)pImage->Width);
	vh = (height - 0.5f) / (float)pImage->Height - (0.5f / (float)pImage->Height);

	RenderBitmap(uiImageType, x, y, width, height, u, v, uw, vh);
}

void SEASON3B::RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv, DWORD color)
{
	BITMAP_t *pImage = &Bitmaps[uiImageType];

	float u, v, uw, vh;
	u = ((su + 0.5f) / (float)pImage->Width);
	v = ((sv + 0.5f) / (float)pImage->Height);
	uw = (width - 0.5f) / (float)pImage->Width - (0.5f / (float)pImage->Width);
	vh = (height - 0.5f) / (float)pImage->Height - (0.5f / (float)pImage->Height);

	RenderColorBitmap(uiImageType, x, y, width, height, u, v, uw, vh, color);	
}

float SEASON3B::RenderNumber(float x, float y, int iNum, float fScale)
{
	return g_RenderNumber->RenderNumber(x, y, iNum, fScale);
}

bool SEASON3B::IsNone(int iVirtKey)
{ 
	return g_pNewKeyInput->IsNone(iVirtKey); 
}

bool SEASON3B::IsRelease(int iVirtKey)
{ 
	return g_pNewKeyInput->IsRelease(iVirtKey); 
}

bool SEASON3B::IsPress(int iVirtKey)
{ 
	return g_pNewKeyInput->IsPress(iVirtKey); 
}

bool SEASON3B::IsRepeat(int iVirtKey)
{ 
	return g_pNewKeyInput->IsRepeat(iVirtKey); 
}

SEASON3B::CNewKeyInput::CNewKeyInput()
{
	Init();
}

SEASON3B::CNewKeyInput::~CNewKeyInput()
{

}

void SEASON3B::CNewKeyInput::Init()
{
	memset(&m_pInputInfo, 0, sizeof(INPUTSTATEINFO)*256);
}

SEASON3B::CNewKeyInput* SEASON3B::CNewKeyInput::GetInstance()
{
	static SEASON3B::CNewKeyInput s_Instance;
	return &s_Instance;
}

void SEASON3B::CNewKeyInput::ScanAsyncKeyState()
{
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
	if (!g_bWndActive)
		return;
#endif	// ASG_FIX_ACTIVATE_APP_INPUT

	for(int key=0; key < 256; key++)
	{
		if(HIBYTE(GetAsyncKeyState(key)) & 0x80)
		{
			if(m_pInputInfo[key].byKeyState == KEY_NONE || m_pInputInfo[key].byKeyState == KEY_RELEASE)
			{
				// press event (key was up before but down now)
				m_pInputInfo[key].byKeyState = KEY_PRESS;
			}
			else if(m_pInputInfo[key].byKeyState == KEY_PRESS)
			{
				// drag event (key is still down)
				m_pInputInfo[key].byKeyState = KEY_REPEAT;
			}
		}
		else // Key is not currently pressed
		{
			if(m_pInputInfo[key].byKeyState == KEY_REPEAT || m_pInputInfo[key].byKeyState == KEY_PRESS)
			{
				// release event (key was down before but up now)
				m_pInputInfo[key].byKeyState = KEY_RELEASE;
			}
			else if(m_pInputInfo[key].byKeyState == KEY_RELEASE)
			{
				m_pInputInfo[key].byKeyState = KEY_NONE;
			}
		}
	}

	if( IsPress(VK_RETURN) && IsEnterPressed() == false ) {
		m_pInputInfo[VK_RETURN].byKeyState = KEY_NONE;
	}
	SetEnterPressed( false );
}

bool SEASON3B::CNewKeyInput::IsNone(int iVirtKey)
{
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
	if (!g_bWndActive)
		return false;
#endif	// ASG_FIX_ACTIVATE_APP_INPUT
	return (m_pInputInfo[iVirtKey].byKeyState == KEY_NONE) ? true : false;
}

bool SEASON3B::CNewKeyInput::IsRelease(int iVirtKey)
{
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
	if (!g_bWndActive)
		return false;
#endif	// ASG_FIX_ACTIVATE_APP_INPUT
	return (m_pInputInfo[iVirtKey].byKeyState == KEY_RELEASE) ? true : false;
}

bool SEASON3B::CNewKeyInput::IsPress(int iVirtKey)
{
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
	if (!g_bWndActive)
		return false;
#endif	// ASG_FIX_ACTIVATE_APP_INPUT
	return (m_pInputInfo[iVirtKey].byKeyState == KEY_PRESS) ? true : false;	
}

bool SEASON3B::CNewKeyInput::IsRepeat(int iVirtKey)
{
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
	if (!g_bWndActive)
		return false;
#endif	// ASG_FIX_ACTIVATE_APP_INPUT
	return (m_pInputInfo[iVirtKey].byKeyState == KEY_REPEAT) ? true : false;
}

void SEASON3B::CNewKeyInput::SetKeyState(int iVirtKey, KEY_STATE KeyState)
{
	m_pInputInfo[iVirtKey].byKeyState = KeyState;
}



