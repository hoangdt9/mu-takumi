// NewUIBCustomMenu.cpp: implementation of the CNewUIBCustomMenuInfo class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIBCustomMenu.h"
#include "NewUISystem.h"
#include "wsclientinline.h"
#include "MapManager.h"
#include "SkillManager.h"
#include "CharacterManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzAI.h"
#include "Util.h"
#include "CustomJewel.h"
#include "TrayMode.h"
#include "CBInterface.h"
#include "Input.h"

using namespace SEASON3B;
extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayHeight;
extern int DisplayWinMid;
extern int DisplayWinExt;
extern int DisplayWinReal;
using namespace SEASON3B;

DWORD BLockMouseMoveClick = 0;

DWORD BButtonClickTime = 0;

extern int TargetX, TargetY;
extern MovementSkill g_MovementSkill;
extern int SelectedCharacter;

namespace
{
	bool IsPrimaryUiTap()
	{
#if defined(__ANDROID__) || defined(MU_IOS)
		return SEASON3B::IsPress(VK_LBUTTON) || CInput::Instance().IsLBtnDn();
#else
		return ((GetKeyState(VK_LBUTTON) & 0x8000) != 0);
#endif
	}
}


CNewUIBCustomMenuInfo::CNewUIBCustomMenuInfo()
{
#ifdef __ANDROID__
	OutputDebugStringA("UI_BCUSTOM_CTOR: begin");
	g_ErrorReport.Write("[UI_BCUSTOM_CTOR] begin\r\n");
#endif
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
	BLockButtonMoveWindow = false;
	BLockButtonHover = false;
	BLockMessageBox = false;
	AutoCtrlPK = false;
	AutoHP = false;
#ifdef __ANDROID__
	OutputDebugStringA("UI_BCUSTOM_CTOR: end");
	g_ErrorReport.Write("[UI_BCUSTOM_CTOR] end\r\n");
#endif
}

CNewUIBCustomMenuInfo::~CNewUIBCustomMenuInfo()
{
	Release();
}

void CNewUIBCustomMenuInfo::DrawInfoBox(float PosX, float PosY, float Width, float Height, DWORD bkcolor, int Arg6  , int Arg7)
{
	float Red = (float)((bkcolor & 0xff000000) >> 24) / 255.0f;
	float Green = (float)((bkcolor & 0x00ff0000) >> 16) / 255.0f;
	float Blue = (float)((bkcolor & 0x0000ff00) >> 8) / 255.0f;
	float Alpha = (float)(bkcolor & 0x000000ff) / 255.0f;
	
	if (bkcolor)
	{
		EnableAlphaTest();
		glColor4f(Red, Green, Blue, Alpha);
		RenderColor(PosX, PosY, Width + 6.5, Height + 5, 0, Arg6); //central
		EndRenderColor();
	}
	float RightX = PosX - 2.0f;
	float LeftX = (PosX + Width) - 2.0f;
	float TopY = PosY - 2.0f;
	float DownY = (PosY + Height) - 2.0f;
	bool horizontal = true;
	bool vertical = true;
	//--
	float IniciarX = PosX - 2.0f;
	float w = 14.0f;
	//--
	float IniciarY = PosY - 2.0f;
	float H = 14.0f;


	float x = PosX-4;
	float y = PosY-3.1;
	float width = Width+16;
	float height = Height+15.1;
	RenderImage(IMAGE_INFO_TOP_LEFT, x, y, 14.0, 14.0);
	RenderImage(IMAGE_INFO_TOP_RIGHT, (x + width) - 14.f, y, 14.0, 14.0);
	RenderImage(IMAGE_INFO_BOTTOM_LEFT, x, (y + height) - 14.f, 14.0, 14.0);
	RenderImage(IMAGE_INFO_BOTTOM_RIGHT, (x + width) - 14.f, (y + height) - 14.f, 14.0, 14.0);
	RenderImage(IMAGE_INFO_TOP_PIXEL, x + 6.f, y, (width - 12.f), 14.0);
	RenderImage(IMAGE_INFO_RIGHT_PIXEL, (x + width) - 14.f, y + 6.f, 14.0, (height - 14.f));
	RenderImage(IMAGE_INFO_BOTTOM_PIXEL, x + 6.f, (y + height) - 14.f, (width - 12.f), 14.0);
	RenderImage(IMAGE_INFO_LEFT_PIXEL, x, (y + 6.f), 14.0, (height - 14.f));
	//EnableAlphaTest(true);
	//glColor3f(1.0, 1.0, 1.0);

	//while (vertical || horizontal)
	//{
	//	if (vertical)
	//	{
	//		IniciarX += 14.0f;

	//		if ((IniciarX + w) > LeftX)
	//		{
	//			vertical = false;
	//			w = LeftX - IniciarX;
	//		}
	//		RenderImage(IMAGE_INFO_TOP_PIXEL, IniciarX, TopY, w, 14.0f); //Ngang

	//		RenderImage(IMAGE_INFO_BOTTOM_PIXEL, IniciarX, DownY, w, 14.0f); //Ngang 2
	//	}
	//	//-- abajo
	//	if (horizontal)
	//	{
	//		IniciarY += 14.0f;

	//		if ((IniciarY + H) > DownY)
	//		{
	//			horizontal = false;
	//			H = DownY - IniciarY;
	//		}

	//		RenderImage(IMAGE_INFO_RIGHT_PIXEL, LeftX, IniciarY, 14.0f, H); //Doc 2

	//		RenderImage(IMAGE_INFO_LEFT_PIXEL, RightX, IniciarY, 14.0f, H); // Doc 1

	//		
	//	}
	//}

	//RenderImage(IMAGE_INFO_TOP_LEFT, RightX, TopY, 14.0f, 14.0f); //Goc tren ben trai

	//RenderImage(IMAGE_INFO_TOP_RIGHT, LeftX, TopY, 14.0f, 14.0f); //Goc tren ben phai

	//RenderImage(IMAGE_INFO_BOTTOM_LEFT, RightX, DownY, 14.0f, 14.0f);  //Goc duoi trai

	//RenderImage(IMAGE_INFO_BOTTOM_RIGHT, LeftX, DownY, 14.0f, 14.0f);  //Goc Duoi phai

	//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	//EnableAlphaTest(0);
}
bool CNewUIBCustomMenuInfo::DrawWindowBG(GUIObj* gui, LPCSTR Text, ...) {
	static float v1;
	static float v2;
	static float v3;
	static float v4;
	static float v5;
	static float v6;
	static float v7;
	static float v8;
	static float v9;
	static float v10;

	//
	if (!gui->onShow)
	{
		return false;
	}
	if (GetKeyState(VK_ESCAPE) < 0)
	{

		gui->onShow = false;
	}
	if (gui->X == -1)
	{
		float ConverX = (DisplayWin / 2) - (gui->Width / 2);
		gui->X = ConverX;
		gui->Default_X = ConverX;
		gui->Max_X = ConverX + gui->Width;
		return false;
	}
	//g_ConsoleDebug->Write(MCD_ERROR, "SHOWMENUI !!");
	char Buff[2048];
	int BuffLen = sizeof(Buff) - 1;
	ZeroMemory(Buff, BuffLen);
	// ----
	va_list args;
	va_start(args, Text);
	int Len = vsprintf_s(Buff, BuffLen, Text, args);
	va_end(args);

	//textSize tS = this->getTextSize(Buff);


	if (SEASON3B::CheckMouseIn(gui->X, gui->Y, gui->Width, gui->Height) == 1)
	{
		BLockMouseMoveClick = GetTickCount() + 500;
	}
	if (gui->allowDrag)
	{
		if (GetKeyState(VK_LBUTTON) & 0x8000 && !BLockButtonHover && !BLockMessageBox && !BLockButtonMoveWindow)
		{

			if (SEASON3B::CheckMouseIn(gui->X, gui->Y, gui->Width, 30))
			{
				if (!gui->dragging) {
					gui->dragging = true;
					gui->curX = MouseX;
					gui->curY = MouseY;
				}
				//g_pBCustomMenuInfo->DrawMessage(1, "Check Window : %f~%f Max(%f/%f)", gui->X, gui->Y, gui->Max_X, gui->Max_Y);
			}
			if (gui->dragging)
			{
				float CalcX = gui->X + MouseX - gui->curX;
				float CalcY = gui->Y + MouseY - gui->curY;

				gui->X += MouseX - gui->curX;
				gui->Y += MouseY - gui->curY;
				gui->curX = MouseX;
				gui->curY = MouseY;
				gui->Max_X = gui->X + gui->Width;
				gui->Max_Y = gui->Y + gui->Height;

				if (CalcX > DisplayWin - gui->Width)
				{
					gui->X = DisplayWin - gui->Width;
				}
				else if (CalcX < 0)
				{
					gui->X = 0;
				}
				else if (CalcY < 0)
				{
					gui->Y = 0;
				}
				else if (CalcY > 350)
				{
					gui->Y = 350;
				}
			}
		}
		else {
			if (gui->dragging) {
				gui->dragging = false;
			}
		}
	}

	//v1 = (1024 / gui->Width) / pWinWidthReal;
	//v2 = (1024 / gui->Height) / pWinHeightReal;
	//v3 = gui->Height / 1024 * 130;
	//v4 = gui->Y + v3 - tS.resize_height - 5 * pWinHeightReal;
	//
	//nInterface.DrawFrame(0xF1007, gui->X, gui->Y, 1024, 1024, 0, 0, 1024, 1024, v1, v2, 0);

	//SIZE TextSize = { 0, 0 };
	//g_pRenderText->SetFont(g_hFont);
	//g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), Buff, lstrlen(Buff), &TextSize);
	//TextDraw(g_hFont, gui->X, v4, 0xffffffff, 0x0, gui->Width, TextSize.cy, 3, Buff);


	float StartX = gui->X;
	float StartY = gui->Y;
	float CuaSoW = gui->Width;
	float CuaSoH = gui->Height;

	float ScaleW = (225 / CuaSoW) / g_fScreenRate_x;
	float ScaleH = (225 / CuaSoH) / g_fScreenRate_y;

	EnableAlphaTest();

	DrawFrame(IMAGE_BCUSTOM_WINDOW_31322, StartX, StartY, 225, 225, 0.0, 0.0, 270, 225, ScaleW, ScaleH); //khung goc ben trai tren

	//==== NGang Tren va Duoi
	for (int i = StartX + 10; i < (StartX + CuaSoW) - 10; i += 10)
	{
		RenderImage(IMAGE_BCUSTOM_WINDOW_31353, i, StartY, 10.0, 64.0, 60.0, 0); //khung goc ben trai tren
		RenderImage(IMAGE_BCUSTOM_WINDOW_31357, i, (StartY + CuaSoH) - 45.0, 10.0, 45.0, 60.0, 0); //khung goc ben trai tren
	}

	//=== Nen va thanh doc 2 trai phai
	for (int doc = StartY + 10; doc < (StartY + CuaSoH) - 10; doc += 10)
	{
		RenderImage(IMAGE_BCUSTOM_WINDOW_31355, StartX, doc, 21.0, 10, 0, 0); //Khung giua ben trai
		RenderImage(IMAGE_BCUSTOM_WINDOW_31356, (StartX + CuaSoW) - 21.0, doc, 21.0, 10, 0, 0); //Khung giua ben phai
	}
	RenderImage(IMAGE_BCUSTOM_WINDOW_31353, StartX, StartY, 60.0, 64.0, 0, 0); //khung goc ben trai tren
	RenderImage(IMAGE_BCUSTOM_WINDOW_31353, (StartX + CuaSoW) - 60.0, StartY, 60.0, 64.0, 130.0, 0); //khung goc ben phai tren

	RenderImage(IMAGE_BCUSTOM_WINDOW_31357, StartX, (StartY + CuaSoH) - 45.0, 60.0, 45.0, 0, 0); //khung goc ben trai duoi
	RenderImage(IMAGE_BCUSTOM_WINDOW_31357, (StartX + CuaSoW) - 60.0, (StartY + CuaSoH) - 45.0, 60.0, 45.0, 130.0, 0); //khung goc ben phai duoi

	//======Close
	int CloseX = StartX + CuaSoW - 33.0f;
	int CloseY = StartY;
	RenderImage(IMAGE_BCUSTOM_WINDOW_32453, CloseX, CloseY, 36.0f, 29.0f);
	if (SEASON3B::CheckMouseIn(CloseX, CloseY, 36, 36) == 1)
	{
		if (GetKeyState(VK_LBUTTON) & 0x8000 && GetTickCount() - gui->lastActionTime > 300 && !BLockButtonHover && !BLockMessageBox)
		{
			PlayBuffer(25, 0, 0);
			glColor3f(1.0f, 0.0f, 0.0f);
			BLockMouseMoveClick = GetTickCount() + 500;
			gui->Clear();
		}
		RenderImage(IMAGE_BCUSTOM_WINDOW_32453, CloseX, CloseY, 36, 29, 0, 0, RGBA(255, 204, 20, 130));
		RenderTipText(CloseX + 40, CloseY + 15, "Close");
	}
	//================================================
	TextDraw(g_hFontBold, StartX, StartY + 7, 0xFFFFFFFF, 0x0, CuaSoW, 0, 3, Buff); //Buoc Vao
	
	EnableAlphaTest(false);
	return SEASON3B::CheckMouseIn(gui->X, gui->Y, gui->Width, gui->Height);
}


void MoveWindows(int eNumWindow)
{
	if (GetKeyState(VK_LBUTTON) & 0x8000 && GetTickCount() - gInterface.Data[eNumWindow].EventTick > 300)
	{
		if (SEASON3B::CheckMouseIn(gInterface.Data[eNumWindow].X, gInterface.Data[eNumWindow].Y, gInterface.Data[eNumWindow].Width - 70, 30))
		{
			if (!gInterface.Data[eNumWindow].OnClick) {
				gInterface.Data[eNumWindow].OnClick = true;
				gInterface.Data[eNumWindow].curX = MouseX;
				gInterface.Data[eNumWindow].curY = MouseY;
			}
		}
		if (gInterface.Data[eNumWindow].OnClick)
		{
			g_pBCustomMenuInfo->BLockButtonMoveWindow = true;
			gInterface.Data[eNumWindow].X += MouseX - gInterface.Data[eNumWindow].curX;
			gInterface.Data[eNumWindow].Y += MouseY - gInterface.Data[eNumWindow].curY;
			gInterface.Data[eNumWindow].curX = MouseX;
			gInterface.Data[eNumWindow].curY = MouseY;
			//===
			if (gInterface.Data[eNumWindow].X < 0)
				gInterface.Data[eNumWindow].X = 0;
			if (gInterface.Data[eNumWindow].Y < 0)
				gInterface.Data[eNumWindow].Y = 0;

			if ((gInterface.Data[eNumWindow].X + gInterface.Data[eNumWindow].Width) > DisplayWin)
				gInterface.Data[eNumWindow].X = DisplayWin - (gInterface.Data[eNumWindow].Width);
			if ((gInterface.Data[eNumWindow].Y + gInterface.Data[eNumWindow].Height + 46) > DisplayHeight)
				gInterface.Data[eNumWindow].Y = DisplayHeight - (gInterface.Data[eNumWindow].Height + 46);
		}
	}
	else {
		if (gInterface.Data[eNumWindow].OnClick)
		{
			gInterface.Data[eNumWindow].OnClick = false;
			g_pBCustomMenuInfo->BLockButtonMoveWindow = false;
		}
	}
}
extern void SetLineColor(int iType, float fAlphaRate = 1.0f);
bool CNewUIBCustomMenuInfo::DrawWindowCustomMini(float* StartX, float* StartY, float CuaSoW, float CuaSoH, int eNumWindow, LPCSTR Text, ...)
{
	//return 0;
	static char Buff[90];
	int BuffLen = sizeof(Buff) - 1;
	ZeroMemory(Buff, BuffLen);
	// ----
	va_list args;
	va_start(args, Text);
	int Len = vsprintf_s(Buff, BuffLen, Text, args);
	va_end(args);

	if (!gInterface.Data[eNumWindow].OnShow)
	{
		return 0;
	}
	if (!gInterface.Data[eNumWindow].FirstLoad)
	{
		gInterface.Data[eNumWindow].Width = CuaSoW;
		gInterface.Data[eNumWindow].Height = CuaSoH;
		gInterface.Data[eNumWindow].X = *StartX;
		gInterface.Data[eNumWindow].Y = *StartY;
		gInterface.Data[eNumWindow].FirstLoad = true;
	}
	else
	{
		(*StartX) = gInterface.Data[eNumWindow].X;
		(*StartY) = gInterface.Data[eNumWindow].Y;
	}


	if (gInterface.Data[eNumWindow].AllowMove)
	{
		MoveWindows(eNumWindow);
	}


	if (SEASON3B::CheckMouseIn(gInterface.Data[eNumWindow].X, gInterface.Data[eNumWindow].Y, gInterface.Data[eNumWindow].Width, CuaSoH) == 1)
	{
		g_pBCustomMenuInfo->SetBlockCur(TRUE);
		gInterface.Data[eNumWindow].Hover = true;
	}
	else
	{
		gInterface.Data[eNumWindow].Hover = false;
	}
	float ScaleW = (225 / gInterface.Data[eNumWindow].Width) / g_fScreenRate_x;
	float ScaleH = (225 / gInterface.Data[eNumWindow].Height) / g_fScreenRate_y;

	EnableAlphaTest();
	
	//==Khung Title
	RenderBitmap(BITMAP_INTERFACE_EX + 8, (float)gInterface.Data[eNumWindow].X, (float)gInterface.Data[eNumWindow].Y+5, (float)gInterface.Data[eNumWindow].Width, (float)20, 0.f, 0.f, 4.f / 4.f, 15.f / 16.f);
	SetLineColor(2);
	//BDrawOutLine((float)gInterface.Data[eNumWindow].X - 3, (float)gInterface.Data[eNumWindow].Y +2, CuaSoW + 6, CuaSoH);
	if (gInterface.Data[eNumWindow].BActiveHiden)
	{
		BDrawOutLine((float)gInterface.Data[eNumWindow].X - 3, (float)gInterface.Data[eNumWindow].Y + 2, gInterface.Data[eNumWindow].Width + 6, 23);
	}
	else
	{
		BDrawOutLine((float)gInterface.Data[eNumWindow].X - 3, (float)gInterface.Data[eNumWindow].Y + 2, gInterface.Data[eNumWindow].Width + 6, gInterface.Data[eNumWindow].Height);
	}

	EndRenderColor();

	if (SEASON3B::CheckMouseIn((float)((gInterface.Data[eNumWindow].X + 3) + CuaSoW) - 17, (float)gInterface.Data[eNumWindow].Y + 10, 9.0, 9.0) == 1)
	{
		glColor4f(0.60000002, 0.60000002, 0.60000002, 1.0);
		if ((SEASON3B::IsPress(VK_LBUTTON) && GetTickCount() - gInterface.Data[eNumWindow].EventTick > 300 && !BLockButtonHover && !BLockMessageBox && !BLockButtonMoveWindow) || SEASON3B::IsPress(VK_ESCAPE))
		{
			PlayBuffer(25, 0, 0);
			gInterface.Data[eNumWindow].OnShow ^= 1;
			gInterface.Data[eNumWindow].EventTick = GetTickCount();

		}
	}
	else
	{
		glColor4f(1.0, 1.0, 1.0, 1.0);
	}
	RenderBitmap(BITMAP_INTERFACE_EX + 10, (float)((gInterface.Data[eNumWindow].X + 3) + gInterface.Data[eNumWindow].Width) - 17, (float)gInterface.Data[eNumWindow].Y+10, 9.0, 9.0, 0.f, 9.f / 32.f, 9.f / 32.f, 9.f / 32.f, 1, 1, 0.0);

	if (!gInterface.Data[eNumWindow].BActiveHiden)
	{
		if (SEASON3B::CheckMouseIn((float)((gInterface.Data[eNumWindow].X + 3) + gInterface.Data[eNumWindow].Width) - 17 - (11 * 1), (float)gInterface.Data[eNumWindow].Y + 10, 9.0, 9.0) == 1)
		{
			glColor4f(0.60000002, 0.60000002, 0.60000002, 1.0);
			if (SEASON3B::IsPress(VK_LBUTTON) && GetTickCount() - gInterface.Data[eNumWindow].EventTick > 300)
			{
				PlayBuffer(25, 0, 0);
				gInterface.Data[eNumWindow].BActiveHiden ^= 1;
				gInterface.Data[eNumWindow].EventTick = GetTickCount();
			}
		}
		else
		{
			glColor4f(1.0, 1.0, 1.0, 1.0);
		}
		//==Hide
		RenderBitmap(BITMAP_INTERFACE_EX + 10, (float)((gInterface.Data[eNumWindow].X + 3) + gInterface.Data[eNumWindow].Width) - 17 - (11 * 1), (float)gInterface.Data[eNumWindow].Y + 10, 9.0, 9.0, 0.0, 0.0, 9.f / 32.f, 9.f / 32.f, 1, 1, 0.0);
	}
	else
	{
		if (SEASON3B::CheckMouseIn((float)((gInterface.Data[eNumWindow].X + 3) + gInterface.Data[eNumWindow].Width) - 17 - (11 * 1), (float)gInterface.Data[eNumWindow].Y + 10, 9.0, 9.0) == 1)
		{
			glColor4f(0.60000002, 0.60000002, 0.60000002, 1.0);
			if (SEASON3B::IsPress(VK_LBUTTON) && GetTickCount() - gInterface.Data[eNumWindow].EventTick > 300)
			{
				PlayBuffer(25, 0, 0);
				gInterface.Data[eNumWindow].BActiveHiden ^= 1;
				gInterface.Data[eNumWindow].EventTick = GetTickCount();
			}
		}
		else
		{
			glColor4f(1.0, 1.0, 1.0, 1.0);
		}
		//===
		//==Show
		RenderBitmap(BITMAP_INTERFACE_EX + 10, (float)((gInterface.Data[eNumWindow].X + 3) + CuaSoW) - 17 - (11 * 1), (float)gInterface.Data[eNumWindow].Y + 10, 9.0, 9.0, 9.f / 32.f, 0.0, 9.f / 32.f, 9.f / 32.f, 1, 1, 0.0);
	}

	TextDraw(g_hFontBold, gInterface.Data[eNumWindow].X, gInterface.Data[eNumWindow].Y + 10, 0xFFFFFFFF, 0x0, gInterface.Data[eNumWindow].Width, 0, 3, Buff); //Buoc Vao
	//DisableAlphaBlend();
	EnableAlphaTest(false);
	return 1;
}

bool CNewUIBCustomMenuInfo::gDrawWindowCustom(float* StartX, float* StartY, float CuaSoW, float CuaSoH, int eNumWindow, LPCSTR Text, ...)
{
	//return 0;
	static char Buff[90];
	int BuffLen = sizeof(Buff) - 1;
	ZeroMemory(Buff, BuffLen);
	// ----
	va_list args;
	va_start(args, Text);
	int Len = vsprintf_s(Buff, BuffLen, Text, args);
	va_end(args);

	if (!gInterface.Data[eNumWindow].OnShow)
	{
		return 0;
	}
	if (!gInterface.Data[eNumWindow].FirstLoad)
	{
		gInterface.Data[eNumWindow].Width = CuaSoW;
		gInterface.Data[eNumWindow].Height = CuaSoH;
		gInterface.Data[eNumWindow].X = *StartX;
		gInterface.Data[eNumWindow].Y = *StartY;
		gInterface.Data[eNumWindow].FirstLoad = true;
	}
	else
	{
		(*StartX) = gInterface.Data[eNumWindow].X;
		(*StartY) = gInterface.Data[eNumWindow].Y;
	}


	if (gInterface.Data[eNumWindow].AllowMove)
	{
		MoveWindows(eNumWindow);
	}
	

	if (SEASON3B::CheckMouseIn(gInterface.Data[eNumWindow].X, gInterface.Data[eNumWindow].Y, CuaSoW, CuaSoH) == 1)
	{
		g_pBCustomMenuInfo->SetBlockCur(TRUE);
		gInterface.Data[eNumWindow].Hover = true;
	}
	else
	{
		gInterface.Data[eNumWindow].Hover = false;
	}
	float ScaleW = (225 / CuaSoW) / g_fScreenRate_x;
	float ScaleH = (225 / CuaSoH) / g_fScreenRate_y;

	EnableAlphaTest();
	//glColor3f(1.0, 1.0, 1.0);

	//DrawFrame(IMAGE_BCUSTOM_WINDOW_31322, gInterface.Data[eNumWindow].X, gInterface.Data[eNumWindow].Y, 225, 225, 0.0, 0.0, 270, 225, ScaleW, ScaleH); //khung goc ben trai tren
	gInterface.DrawBarForm(gInterface.Data[eNumWindow].X, gInterface.Data[eNumWindow].Y+5, CuaSoW, CuaSoH-10, 0.1, 0.1, 0.1, 0.9);
	//==== NGang Tren va Duoi
	for (int i = gInterface.Data[eNumWindow].X + 10; i < (gInterface.Data[eNumWindow].X + CuaSoW) - 10; i += 10)
	{
		RenderImage(IMAGE_BCUSTOM_WINDOW_31353, i, gInterface.Data[eNumWindow].Y, 10.0, 64.0, 60.0, 0); //khung goc ben trai tren
		RenderImage(IMAGE_BCUSTOM_WINDOW_31357, i, (gInterface.Data[eNumWindow].Y + CuaSoH) - 45.0, 10.0, 45.0, 60.0, 0); //khung goc ben trai tren
	}

	//=== Nen va thanh doc 2 trai phai
	for (int doc = gInterface.Data[eNumWindow].Y + 10; doc < (gInterface.Data[eNumWindow].Y + CuaSoH) - 10; doc += 10)
	{
		RenderImage(IMAGE_BCUSTOM_WINDOW_31355, gInterface.Data[eNumWindow].X, doc, 21.0, 10, 0, 0); //Khung giua ben trai
		RenderImage(IMAGE_BCUSTOM_WINDOW_31356, (gInterface.Data[eNumWindow].X + CuaSoW) - 21.0, doc, 21.0, 10, 0, 0); //Khung giua ben phai
	}




	RenderImage(IMAGE_BCUSTOM_WINDOW_31353, gInterface.Data[eNumWindow].X, gInterface.Data[eNumWindow].Y, 60.0, 64.0, 0, 0); //khung goc ben trai tren
	RenderImage(IMAGE_BCUSTOM_WINDOW_31353, (gInterface.Data[eNumWindow].X + CuaSoW) - 60.0, gInterface.Data[eNumWindow].Y, 60.0, 64.0, 130.0, 0); //khung goc ben phai tren

	RenderImage(IMAGE_BCUSTOM_WINDOW_31357, gInterface.Data[eNumWindow].X, (gInterface.Data[eNumWindow].Y + CuaSoH) - 45.0, 60.0, 45.0, 0, 0); //khung goc ben trai duoi
	RenderImage(IMAGE_BCUSTOM_WINDOW_31357, (gInterface.Data[eNumWindow].X + CuaSoW) - 60.0, (gInterface.Data[eNumWindow].Y + CuaSoH) - 45.0, 60.0, 45.0, 130.0, 0); //khung goc ben phai duoi

	//======Close
	int CloseX = gInterface.Data[eNumWindow].X + CuaSoW - 33.0f;
	int CloseY = gInterface.Data[eNumWindow].Y;
	RenderImage(IMAGE_BCUSTOM_WINDOW_32453, CloseX, CloseY, 36.0f, 29.0f);
	if (SEASON3B::CheckMouseIn(CloseX, CloseY, 36, 36) == 1)
	{
		const bool closePressed = IsPrimaryUiTap();
		if (closePressed && GetTickCount() - gInterface.Data[eNumWindow].EventTick > 300 && !BLockButtonHover && !BLockMessageBox && !BLockButtonMoveWindow)
		{
			PlayBuffer(25, 0, 0);
			glColor3f(1.0f, 0.0f, 0.0f);
			BLockMouseMoveClick = GetTickCount() + 500;
			gInterface.Data[eNumWindow].EventTick = GetTickCount();
			gInterface.Data[eNumWindow].Close();
		}
		RenderImage(IMAGE_BCUSTOM_WINDOW_32453, CloseX, CloseY, 36, 29, 0, 0, RGBA(255, 204, 20, 130));
		RenderTipText(CloseX + 40, CloseY + 15, "Close");
	}
	//================================================
	TextDraw(g_hFontBold, gInterface.Data[eNumWindow].X, gInterface.Data[eNumWindow].Y + 10, 0xFFFFFFFF, 0x0, CuaSoW, 0, 3, Buff); //Buoc Vao
	//DisableAlphaBlend();
	EnableAlphaTest(false);
	return 1;
}

bool CNewUIBCustomMenuInfo::DrawButtonGUI(int IDGUID, float PosX, float PosY, float SizeW, float SizeH, int TypeButton)
{
	EnableAlphaTest(true);
	bool mreturn = false;
	//=====================Button 1

	if (TypeButton == 3)
	{
		if (SEASON3B::CheckMouseIn(PosX, PosY, SizeW, SizeH) == 1)
		{
			BLockMouseMoveClick = GetTickCount() + 500;

			if ((GetKeyState(VK_LBUTTON) & 0x8000) && GetTickCount() - gInterface.Data[eTIME].EventTick > 500 && !BLockButtonHover && !BLockMessageBox && !BLockButtonMoveWindow)
			{
				gInterface.Data[eTIME].EventTick = GetTickCount();
				PlayBuffer(25, 0, 0);
				mreturn = true;
				RenderImage(IDGUID, PosX, PosY, SizeW, SizeH, 0.0, SizeH * 2);
			}
			else {
				RenderImage(IDGUID, PosX, PosY, SizeW, SizeH, 0.0, SizeH * 1);
			}

		}
		else {
			RenderImage(IDGUID, PosX, PosY, SizeW, SizeH, 0.0, 0.0);
		}
	}
	else if (TypeButton == 2)
	{
		if (SEASON3B::CheckMouseIn(PosX, PosY, SizeW, SizeH) == 1)
		{
			BLockMouseMoveClick = GetTickCount() + 500;

			if ((GetKeyState(VK_LBUTTON) & 0x8000) && GetTickCount() - gInterface.Data[eTIME].EventTick > 500 && !BLockButtonHover && !BLockMessageBox && !BLockButtonMoveWindow)
			{
				gInterface.Data[eTIME].EventTick = GetTickCount();
				PlayBuffer(25, 0, 0);
				mreturn = true;
				RenderImage(IDGUID, PosX, PosY, SizeW, SizeH, 0.0, SizeH * 1);
			}
			else {
				RenderImage(IDGUID, PosX, PosY, SizeW, SizeH, 0.0, 0.0);
			}

		}
		else {
			RenderImage(IDGUID, PosX, PosY, SizeW, SizeH, 0.0, 0.0);
		}
	}

	else
	{
		if (SEASON3B::CheckMouseIn(PosX, PosY, SizeW, SizeH) == 1)
		{
			BLockMouseMoveClick = GetTickCount() + 500;
			if (GetTickCount() - gInterface.Data[eTIME].EventTick > 500 && !BLockButtonHover && !BLockMessageBox && !BLockButtonMoveWindow) //Click
			{
				if ((GetKeyState(VK_LBUTTON) & 0x8000))
				{
					gInterface.Data[eTIME].EventTick = GetTickCount();
					PlayBuffer(25, 0, 0);
					mreturn = true;
				}
			}
			RenderImage(IDGUID, PosX, PosY, SizeW, SizeH, 0, 0, RGBA(255, 228, 107, 255));
		}
		else
		{
			RenderImage(IDGUID, PosX, PosY, SizeW, SizeH);
		}
	}

	EnableAlphaTest(0);
	return mreturn;
}
bool CNewUIBCustomMenuInfo::DrawButton(float PosX, float PosY, float SizeButton, int FontSize, char* ButtonText, float mSizeButtonW, bool IsEnter)
{
	EnableAlphaTest();
	//============Button
	float SizeButtonW = SizeButton;
	float SizeButtonH = (SizeButton * 20) / 100;

	if (mSizeButtonW != 0)
	{
		SizeButtonW = mSizeButtonW;
	}
	float StartX = PosX;
	float StartY = PosY;
	bool mreturn = false;
	float ScaleX = 0.0, ScaleY = 0.227;
	//=====================Button 1
	if (SEASON3B::CheckMouseIn(StartX, StartY, SizeButtonW, SizeButtonH) == 1 && !BLockButtonHover && !BLockMessageBox && !BLockButtonMoveWindow)
	{
		BLockMouseMoveClick = GetTickCount() + 500;
		ScaleX = ScaleY;
		if (GetTickCount() - BButtonClickTime > 500) //Click
		{
			if (IsPrimaryUiTap())
			{

				ScaleX = ScaleY * 2;
				BButtonClickTime = GetTickCount();
				PlayBuffer(25, 0, 0);
				mreturn = true;
			}
		}
	}
	if ((SEASON3B::IsPress(VK_RETURN) && IsEnter) && !BLockButtonHover && !BLockMessageBox && !BLockButtonMoveWindow)
	{
		if (GetTickCount() - BButtonClickTime > 500) //Click
		{
			//g_pBCustomMenuInfo->CloseWindow(ChatWindow);
			BButtonClickTime = GetTickCount();
			PlayBuffer(25, 0, 0);
			mreturn = true;
		}

	}
	//31563
	//RenderBitmap(31563, StartX, StartY, SizeButtonW, SizeButtonH, 0.0, 0.0, 0.64999998, 0.80000001, 1, 1, 0);

	RenderBitmap(IMAGE_BCUSTOM_WINDOW_31326, StartX, StartY, SizeButtonW, SizeButtonH, 0.0, ScaleX, 0.830, ScaleY, 1, 1, 0);
	TextDraw(g_hFontBold, StartX, StartY + (SizeButtonH / 2) - (FontSize / 2.5), 0xE6FCF7FF, 0x0, SizeButtonW, 0, 3, ButtonText); //Buoc Vao
	//=====================================
	EnableAlphaTest(false);
	return mreturn;
}
bool CNewUIBCustomMenuInfo::RenderCheckBoxMini(float PosX, float PosY, DWORD TextColor, bool Checkis, LPCSTR Text, ...)
{
	bool mreturn = false;
	char Buff[2048];
	int BuffLen = sizeof(Buff) - 1;
	ZeroMemory(Buff, BuffLen);
	// ----
	va_list args;
	va_start(args, Text);
	int Len = vsprintf_s(Buff, BuffLen, Text, args);
	va_end(args);

	if (Checkis)
	{
		//RenderImage(BITMAP_INTERFACE_CUSTOM + 6, PosX, PosY, 15.0, 15.0, 0.0, 0.0);
		RenderBitmap(BITMAP_INTERFACE_CUSTOM + 6, PosX, PosY, 15 / g_fScreenRate_x, 15 / g_fScreenRate_y, 0.01, 0.02999999747, 0.8799999952, 0.3700000346, 1, 1, 0);
	}
	else
	{
		//RenderImage(BITMAP_INTERFACE_CUSTOM + 6, PosX, PosY, 15.0, 15.0, 0.0, 15.0);
		RenderBitmap(BITMAP_INTERFACE_CUSTOM + 6, PosX, PosY, 15 / g_fScreenRate_x, 15 / g_fScreenRate_y, 0.01, 0.49999999747, 0.8799999952, 0.3700000346, 1, 1, 0);
	}
	TextDraw(g_hFont, PosX + (20 / g_fScreenRate_x), PosY + 3, TextColor, 0x0, 0, 0, 1, Text);

	if (SEASON3B::CheckMouseIn(PosX, PosY, 15, 15) == 1)
	{
		if ((GetKeyState(VK_LBUTTON) & 0x8000) && GetTickCount() - gInterface.Data[eTIME].EventTick > 500)
		{
			gInterface.Data[eTIME].EventTick = GetTickCount();
			PlayBuffer(25, 0, 0);
			BButtonClickTime = GetTickCount();
			mreturn = true;
		}
	}
	return mreturn;
}
bool CNewUIBCustomMenuInfo::RenderCheckBox(float PosX, float PosY, DWORD TextColor, bool Checkis, LPCSTR Text, ...)
{
	bool mreturn = false;
	char Buff[2048];
	int BuffLen = sizeof(Buff) - 1;
	ZeroMemory(Buff, BuffLen);
	// ----
	va_list args;
	va_start(args, Text);
	int Len = vsprintf_s(Buff, BuffLen, Text, args);
	va_end(args);

	if (Checkis)
	{
		RenderImage(BITMAP_INTERFACE_CUSTOM + 6, PosX, PosY, 15.0, 15.0, 0.0, 0.0);
	}
	else
	{
		RenderImage(BITMAP_INTERFACE_CUSTOM + 6, PosX, PosY, 15.0, 15.0, 0.0, 15.0);
	}
	TextDraw(g_hFont, PosX + 20, PosY + 3, TextColor, 0x0, 0, 0, 1, Text);

	if (SEASON3B::CheckMouseIn(PosX, PosY, 15, 15) == 1)
	{
		if ((GetKeyState(VK_LBUTTON) & 0x8000) && GetTickCount() - gInterface.Data[eTIME].EventTick > 500)
		{
			gInterface.Data[eTIME].EventTick = GetTickCount();
			PlayBuffer(25, 0, 0);
			BButtonClickTime = GetTickCount();
			mreturn = true;
		}
	}
	return mreturn;
}
bool CNewUIBCustomMenuInfo::RenderCheckOption(float PosX, float PosY, DWORD TextColor, bool Checkis, LPCSTR Text, ...)
{
	bool mreturn = false;
	char Buff[2048];
	int BuffLen = sizeof(Buff) - 1;
	ZeroMemory(Buff, BuffLen);
	// ----
	va_list args;
	va_start(args, Text);
	int Len = vsprintf_s(Buff, BuffLen, Text, args);
	va_end(args);

	if (Checkis)
	{
		RenderImage(BITMAP_INTERFACE_CUSTOM + 7, PosX, PosY, 15.0, 15.0, 0.0, 0.0);
	}
	else
	{
		RenderImage(BITMAP_INTERFACE_CUSTOM + 7, PosX, PosY, 15.0, 15.0, 0.0, 15.0);
	}
	TextDraw(g_hFont, PosX + 20, PosY + 3, TextColor, 0x0, 0, 0, 1, Text);

	if (SEASON3B::CheckMouseIn(PosX, PosY, 15, 15) == 1)
	{
		if ((GetKeyState(VK_LBUTTON) & 0x8000) && GetTickCount() - gInterface.Data[eTIME].EventTick > 500)
		{
			gInterface.Data[eTIME].EventTick = GetTickCount();
			PlayBuffer(25, 0, 0);
			BButtonClickTime = GetTickCount();
			mreturn = true;
		}
	}
	return mreturn;
}
void CNewUIBCustomMenuInfo::RenderGroupBox(int iPosX, int iPosY, int iWidth, int iHeight, int iTitleWidth/* =60 */, int iTitleHeight/* =20  */)
{
	EnableAlphaTest();

	glColor4f(0.f, 0.f, 0.f, 0.9f);
	RenderColor(float(iPosX + 3), float(iPosY + 2), float(iTitleWidth - 8), float(iTitleHeight));
	glColor4f(0.f, 0.f, 0.f, 0.6f);
	RenderColor(float(iPosX + 3), float(iPosY + 2 + iTitleHeight), float(iWidth - 7), float(iHeight - iTitleHeight - 7));

	EndRenderColor();

	RenderImage(IMAGE_B_TABLE_TOP_LEFT, iPosX, iPosY, 14, 14);
	RenderImage(IMAGE_B_TABLE_TOP_RIGHT, iPosX + iTitleWidth - 14, iPosY, 14, 14);
	RenderImage(IMAGE_B_TABLE_TOP_RIGHT, iPosX + iWidth - 14, iPosY + iTitleHeight, 14, 14);
	RenderImage(IMAGE_B_TABLE_BOTTOM_LEFT, iPosX, iPosY + iHeight - 14, 14, 14);
	RenderImage(IMAGE_B_TABLE_BOTTOM_RIGHT, iPosX + iWidth - 14, iPosY + iHeight - 14, 14, 14);

	RenderImage(IMAGE_B_TABLE_TOP_PIXEL, iPosX + 6, iPosY, iTitleWidth - 12, 14);
	RenderImage(IMAGE_B_TABLE_RIGHT_PIXEL, iPosX + iTitleWidth - 14, iPosY + 6, 14, iTitleHeight - 6);
	RenderImage(IMAGE_B_TABLE_TOP_PIXEL, iPosX + iTitleWidth - 5, iPosY + iTitleHeight, iWidth - iTitleWidth - 6, 14);
	RenderImage(IMAGE_B_TABLE_RIGHT_PIXEL, iPosX + iWidth - 14, iPosY + iTitleHeight + 6, 14, iHeight - iTitleHeight - 14);
	RenderImage(IMAGE_B_TABLE_BOTTOM_PIXEL, iPosX + 6, iPosY + iHeight - 14, iWidth - 12, 14);
	RenderImage(IMAGE_B_TABLE_LEFT_PIXEL, iPosX, iPosY + 6, 14, iHeight - 14);
}
void CNewUIBCustomMenuInfo::RederBarOptionW(int iPosX, int iPosY, int* Vaule)
{
	if (CheckMouseIn(iPosX-8, iPosY, 124 + 8, 16))
	{
		int iOldValue = *Vaule;
		if (SEASON3B::IsRepeat(VK_LBUTTON))
		{
			int x = MouseX - (iPosX);
			if (x < 0)
			{
				*Vaule = 0;
			}
			else
			{
				float fValue = (10.f * x) / 124.f;
				*Vaule = (int)fValue + 1;
			}
		}
	}
	int SetVaule = *Vaule;
	RenderImage(IMAGE_OPTION_B_BACK, iPosX, iPosY, 124.f, 16.f);
	if (SetVaule > 0)
	{
		RenderImage(IMAGE_OPTION_B_COLOR, iPosX, iPosY, 124.f * 0.1f * (SetVaule), 16.f);
	}
}
bool CNewUIBCustomMenuInfo::RenderInputBox(float PosX, float PosY, float W, float H, char* TextSet, CUITextInputBox*& Input, UIOPTIONS UiOption,int MaxChar, bool isPass)
{
	// Input Box
	if (!Input)
	{
		Input = new CUITextInputBox;
		Input->Init(g_hWnd, W, H, MaxChar, isPass);
		Input->SetTextColor(255, 255, 230, 230);
		Input->SetBackColor(0, 0, 0, 25);
		Input->SetFont(g_hFont);
		Input->SetState(UISTATE_NORMAL);
		Input->SetOption(UiOption);
		Input->SetText(TextSet);
	}
	Input->SetPosition(PosX, PosY);
	Input->Render();
	Input->DoAction();
	return 1;
}
//---------------------------------------------------------------------------------------------
// Create
bool CNewUIBCustomMenuInfo::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
#ifdef __ANDROID__
	OutputDebugStringA("UI_BCUSTOM: create begin");
	g_ErrorReport.Write("[UI_BCUSTOM] create begin\r\n");
#endif
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_BCUSTOMMENU, this );
#ifdef __ANDROID__
	OutputDebugStringA("UI_BCUSTOM: add ui obj ok");
	g_ErrorReport.Write("[UI_BCUSTOM] add ui obj ok\r\n");
#endif

	SetPos(x, y);
#ifdef __ANDROID__
	OutputDebugStringA("UI_BCUSTOM: set pos ok");
	g_ErrorReport.Write("[UI_BCUSTOM] set pos ok\r\n");
#endif
	LoadImages();
#ifdef __ANDROID__
	OutputDebugStringA("UI_BCUSTOM: load images ok");
	g_ErrorReport.Write("[UI_BCUSTOM] load images ok\r\n");
#endif
	Show( true );
#ifdef __ANDROID__
	OutputDebugStringA("UI_BCUSTOM: show ok");
	g_ErrorReport.Write("[UI_BCUSTOM] show ok\r\n");
#endif

	//this->WindowMenuCustom = new GUIObj(-1, 50, 220, 295, true);
	gInterface.Init();
#ifdef __ANDROID__
	OutputDebugStringA("UI_BCUSTOM: gInterface.Init ok");
	g_ErrorReport.Write("[UI_BCUSTOM] gInterface.Init ok\r\n");
#endif
	return true;
}

void CNewUIBCustomMenuInfo::Release()
{
	UnloadImages();

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
	gInterface.Clear();
}

void CNewUIBCustomMenuInfo::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}
void CNewUIBCustomMenuInfo::SetBlockCur(bool Delay)
{
	if (Delay)
	{
		BLockMouseMoveClick = GetTickCount() + 300;
	}
	else
	{
		BLockMouseMoveClick = 0;
	}
	
}
bool CNewUIBCustomMenuInfo::BtnProcess()
{
	if (BLockMouseMoveClick > GetTickCount())
	{
		return true;

	}
	else 
	{
		BLockMouseMoveClick = 0;
	}
	return false;
}

bool CNewUIBCustomMenuInfo::UpdateMouseEvent()
{
	if( true == BtnProcess() )
		return false;

	
	return true;
}

bool CNewUIBCustomMenuInfo::UpdateKeyEvent()
{
	return true;
}

bool CNewUIBCustomMenuInfo::Update()
{
	return true;
}
bool CNewUIBCustomMenuInfo::UseHotKey()
{
	//if (gInterface.UseHotKey()) return false;
	return true;
}


DWORD SleepTimeHP = 0;
bool CNewUIBCustomMenuInfo::Render() //Work()
{
	gInterface.Work();
	//==AutoHP
	if (this->AutoHP)
	{
		BYTE RateHP = 100;
		if (CharacterAttribute->PrintPlayer.ViewMaxHP > 0)
		{
			RateHP = (BYTE)((CharacterAttribute->PrintPlayer.ViewCurHP * 100) / (CharacterAttribute->PrintPlayer.ViewMaxHP));
		}
		if (RateHP <= gProtect.m_MainInfo.RateHP)
		{
			if (GetTickCount() > SleepTimeHP)
			{
				int Index = g_pMyInventory->FindHPItemIndex();
				if (Index != -1)
				{
					SendRequestUse(Index, 0);
				}
				SleepTimeHP = GetTickCount() + gProtect.m_MainInfo.DelayAutoHP;
			}
		}
	}
	return true;
}

float CNewUIBCustomMenuInfo::GetLayerDepth()
{
	return 9.3f;
}

void CNewUIBCustomMenuInfo::OpenningProcess()
{
	
}

void CNewUIBCustomMenuInfo::ClosingProcess()
{
	
}
void CNewUIBCustomMenuInfo::BindObject(short ObjectID, char* Patch, float Width, float Height, float X, float Y ,int Type)
{
	int ModelID = BITMAP_NONAMED_TEXTURES_NEW_LOAD + gInterface.IndexCustomTextture++;
	LoadBitmap(Patch, ModelID, GL_LINEAR, Type, 1, 0);
	gInterface.Data[ObjectID].EventTick = 0;
	gInterface.Data[ObjectID].OnClick = false;
	gInterface.Data[ObjectID].OnShow = false;
	gInterface.Data[ObjectID].ModelID = ModelID;
	gInterface.Data[ObjectID].Width = Width;
	gInterface.Data[ObjectID].Height = Height;
	gInterface.Data[ObjectID].X = X;
	gInterface.Data[ObjectID].Y = Y;
	gInterface.Data[ObjectID].MaxX = X + Width;
	gInterface.Data[ObjectID].MaxY = Y + Height;
	gInterface.Data[ObjectID].Attribute = 0;
}
void CNewUIBCustomMenuInfo::DrawGUI(short ObjectID, float PosX, float PosY, bool SetPos)
{
	if(gInterface.Data[ObjectID].X == -1 || gInterface.Data[ObjectID].Y == -1 )
	{
		gInterface.Data[ObjectID].X = PosX;
		gInterface.Data[ObjectID].Y = PosY;
		gInterface.Data[ObjectID].MaxX = PosX +gInterface.Data[ObjectID].Width;
		gInterface.Data[ObjectID].MaxY = PosY +gInterface.Data[ObjectID].Height;
	}

	RenderImage(gInterface.Data[ObjectID].ModelID, PosX, PosY,gInterface.Data[ObjectID].Width, gInterface.Data[ObjectID].Height);
}
void CNewUIBCustomMenuInfo::gItemBoxInv(int X, int Y, int Doc, int Ngang, int Use)
{
	float v6; // [esp+0h] [ebp-34h]
	float v7; // [esp+4h] [ebp-30h]
	GLfloat red; // [esp+8h] [ebp-2Ch]
	GLfloat green; // [esp+Ch] [ebp-28h]
	int j; // [esp+2Ch] [ebp-8h]
	int i; // [esp+30h] [ebp-4h]

	for (i = 0; i < Doc; ++i)
	{
		for (j = 0; j < Ngang; ++j)
		{
			EnableAlphaTest();
			if (Use)
			{
				//glColor4f(1.0, 0.69999999, 0.0, 0.60000002);
				glColor4f(0.30000001, 0.5, 0.5, 0.60000002);
			}
			else
			{
				glColor4f(0.3, 0.3, 0.3, 0.60000002);
			}
			v7 = (float)(Y + 20 * i);
			v6 = (float)(X + 20 * j);
			RenderColor(LODWORD(v6), LODWORD(v7), 20.0, 20.0, 0.0, 0);
			EnableAlphaTest(true);
			glColor3f(1.0, 1.0, 1.0);
			green = (float)(Y + 20 * i);
			red = (float)(X + 20 * j);
			RenderImage(SEASON3B::CNewUIInventoryCtrl::IMAGE_ITEM_SQUARE, LODWORD(red), LODWORD(green), 21.0, 21.0);
			EnableAlphaTest(0);
		}
	}
}
void CNewUIBCustomMenuInfo::LoadImages()
{
	//MAx IMAGE_HERO_POSITION_INFO_BASE_WINDOW +5;
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_BCUSTOM_WINDOW_31322, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_BCUSTOM_WINDOW_31353, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_BCUSTOM_WINDOW_31357, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_BCUSTOM_WINDOW_31355, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_BCUSTOM_WINDOW_31356, GL_LINEAR);
	LoadBitmap("Interface\\mini_map_ui_cancel.tga", IMAGE_BCUSTOM_WINDOW_32453, GL_LINEAR);
	LoadBitmap("Interface\\newui_btn_empty.tga", IMAGE_BCUSTOM_WINDOW_31326, GL_LINEAR);
	LoadBitmap("Custom\\Maps\\CRadar.tga", BITMAP_INTERFACE_CUSTOM + 2, GL_LINEAR);
	LoadBitmap("Custom\\Maps\\map_radar.tga", BITMAP_INTERFACE_CUSTOM + 4, GL_LINEAR);
	LoadBitmap("Custom\\Maps\\background-tcp.tga", BITMAP_INTERFACE_CUSTOM + 5, GL_LINEAR);
	LoadBitmap("Interface\\newui_option_check.tga", BITMAP_INTERFACE_CUSTOM+6, GL_LINEAR);
	LoadBitmap("Interface\\MacroUI\\MacroUI_OptionButton.tga", BITMAP_INTERFACE_CUSTOM+7, GL_LINEAR);

	LoadBitmap("Interface\\newui_option_volume01.tga", IMAGE_OPTION_B_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_option_volume02.tga", IMAGE_OPTION_B_COLOR, GL_LINEAR);

	LoadBitmap("Interface\\newui_item_table01(L).tga", IMAGE_B_TABLE_TOP_LEFT);
	LoadBitmap("Interface\\newui_item_table01(R).tga", IMAGE_B_TABLE_TOP_RIGHT);
	LoadBitmap("Interface\\newui_item_table02(L).tga", IMAGE_B_TABLE_BOTTOM_LEFT);
	LoadBitmap("Interface\\newui_item_table02(R).tga", IMAGE_B_TABLE_BOTTOM_RIGHT);
	LoadBitmap("Interface\\newui_item_table03(Up).tga", IMAGE_B_TABLE_TOP_PIXEL);
	LoadBitmap("Interface\\newui_item_table03(Dw).tga", IMAGE_B_TABLE_BOTTOM_PIXEL);
	LoadBitmap("Interface\\newui_item_table03(L).tga", IMAGE_B_TABLE_LEFT_PIXEL);
	LoadBitmap("Interface\\newui_item_table03(R).tga", IMAGE_B_TABLE_RIGHT_PIXEL);
	//==Custom
	gInterface.BLoadImage(); //load img

}

void CNewUIBCustomMenuInfo::UnloadImages()
{
	DeleteBitmap(IMAGE_BCUSTOM_WINDOW_31322);
	DeleteBitmap(IMAGE_BCUSTOM_WINDOW_31353);
	DeleteBitmap(IMAGE_BCUSTOM_WINDOW_31357);
	DeleteBitmap(IMAGE_BCUSTOM_WINDOW_31355);
	DeleteBitmap(IMAGE_BCUSTOM_WINDOW_31356);
	DeleteBitmap(IMAGE_BCUSTOM_WINDOW_32453);
	DeleteBitmap(IMAGE_BCUSTOM_WINDOW_31326);
	DeleteBitmap(IMAGE_BCUSTOM_WINDOW);
	DeleteBitmap(BITMAP_INTERFACE_CUSTOM + 2);
	DeleteBitmap(BITMAP_INTERFACE_CUSTOM + 4);
	DeleteBitmap(BITMAP_INTERFACE_CUSTOM + 5);
	DeleteBitmap(BITMAP_INTERFACE_CUSTOM + 6);

	DeleteBitmap(IMAGE_B_TABLE_RIGHT_PIXEL);
	DeleteBitmap(IMAGE_B_TABLE_LEFT_PIXEL);
	DeleteBitmap(IMAGE_B_TABLE_BOTTOM_PIXEL);
	DeleteBitmap(IMAGE_B_TABLE_TOP_PIXEL);
	DeleteBitmap(IMAGE_B_TABLE_BOTTOM_RIGHT);
	DeleteBitmap(IMAGE_B_TABLE_BOTTOM_LEFT);
	DeleteBitmap(IMAGE_B_TABLE_TOP_RIGHT);
	DeleteBitmap(IMAGE_B_TABLE_TOP_LEFT);

	DeleteBitmap(IMAGE_OPTION_B_BACK);
	DeleteBitmap(IMAGE_OPTION_B_COLOR);
	//Clear IMG
	for (int i = 0; i < gInterface.IndexCustomTextture; i++)
	{
		DeleteBitmap(BITMAP_NONAMED_TEXTURES_NEW_LOAD + i);
	}
}


