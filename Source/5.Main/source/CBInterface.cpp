#include "stdafx.h"
#include "CBInterface.h"
#include "NewUISystem.h"
#include "UIControls.h"
#include "TrayMode.h"
#include "DSPlaySound.h"
#include "ZzzInterface.h"
#include "Other.h"
#include "MenuCustom.h"
#include "Protocol.h"
#include "CustomEventTime.h"
#include "Ranking.h"
#include "WindowClass.h"
#include "CBChoTroi.h"
#include "CB_AutoNapGame.h"
#include "CB_DanhHieu.h"
#if(JEWELBANKVER2)
#include "CB_NewJewelBank.h"
#else
#include "CB_JewelBank.h"
#endif
#include "CB_BossGuild.h"
#include "CB_CTCMini.h"
#include "MapManager.h"
#include "CustomRanking.h"
#include "CB_AutoResetInfo.h"
#include "CBInterfaceVIPChar.h"
#include "VongQuay.h"
#if(CB_VIEWCHARITEM)
#include "CB_ViewCharItem.h"
#endif
#if(CUSTOM_WINDOWLOCKITEM)
#include "CB_LockItem.h"
#endif
#if(DOIMK)
#include "CB_DoiMK.h"
#endif
#include "APICB.h"

#if(CB_CUSTOMMIXINFO)
#include "CB_InfoCustomMix.h"
#endif
#if(CB_NEWQUESTFF)
#include "CB_NewQuest.h"
#endif
#if(CB_HUYDONGEXC)
#include "CB_HuyDongExc.h"
#endif
#if(CB_GETMIXRATE)
#include "CB_GetMixRate.h"
#endif

#include "CB_Mapping.h"
#include "CB_ActiveInvasions.h"
#include "CB_CongHuong.h"
#include "CB_DrawCustomRank.h"
#include "CB_StatsAdvance.h"
#include "CB_BotTrader.h"
#include "CB_BXHTopDmg.h"

#include "MocDonate.h"
#include "Input.h"

namespace
{
	bool IsMobileUiTap()
	{
#if defined(__ANDROID__) || defined(MU_IOS)
		return SEASON3B::IsPress(VK_LBUTTON) || CInput::Instance().IsLBtnDn();
#else
		return ((GetKeyState(VK_LBUTTON) & 0x8000) != 0);
#endif
	}
}


extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayHeight;
extern int DisplayWinMid;
extern int DisplayHeightExt;
extern int DisplayWinExt;
extern int DisplayWinReal;
Interface gInterface;
#if defined(__ANDROID__) || defined(MU_IOS)
DWORD gAndroidSetCursorFocus = 0;
#endif

Interface::Interface() // OK
{
	this->IndexCustomTextture = 0;
	//==MessBox
	this->MsgBoxCaption[0] = 0;
	this->MsgBoxText[0] = 0;
	this->MsgBoxType = 0;
	this->MsgLine = 0;
	ZeroMemory(&Data, sizeof(Data));
	this->CloseAllWindowCustom();
	this->vCaptcha = this->generateCaptcha(4);

	this->NumberMapTime = 255;
	this->MapTime = 0;
	this->TypeStyle = 0;
	this->TextMapTime.clear();

	//=== Ngat Reconnect
	this->m_Disconnect = false;
	this->HidenCustom = false;
#if(SAUDOIITEM)
	gInterface.SAU_ActiveMix = false;
	gInterface.SAU_EffectsMix = false;
	gInterface.SetStateDoiItem = false;
	gInterface.RateSAU = false;
	memset(gInterface.SAU_ItemChinh, 0xFF, sizeof(gInterface.SAU_ItemChinh));
	memset(gInterface.SAU_ItemPhu, 0xFF, sizeof(gInterface.SAU_ItemPhu));
	for (int n = 0; n < 6; n++)
	{
		memset(gInterface.SAU_ItemKetQua[n], 0xFF, sizeof(gInterface.SAU_ItemKetQua[n]));
	}
#endif
}

Interface::~Interface() // OK
{

}


void Interface::DrawMessageBox()
{
	if (!gInterface.Data[eWindowMessageBox].OnShow)
	{
		return;
	}
	float CuaSoW = 220;
	float CuaSoH = 80 + (this->MsgLine * 10);

	float StartX = (MAX_WIN_WIDTH / 2) - (CuaSoW / 2);
	float StartY = 15.0;

	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, CuaSoW, CuaSoH, eWindowMessageBox, this->MsgBoxCaption);
	EnableAlphaTest(true);
	glColor3f(1.0, 1.0, 1.0);
	g_pBCustomMenuInfo->SetBlockCur(TRUE);
	this->MsgLine = TextDraw((HFONT)g_hFont, StartX, StartY + 35, 0xFFFFFFFF, 0x0, CuaSoW, 0, 3, this->MsgBoxText);
	if (gInterface.Data[eWindowMessageBox].Height < (80 + (this->MsgLine * 10))) gInterface.Data[eWindowMessageBox].Height = (float)(80 + (this->MsgLine * 10));

	if (this->MsgBoxType == 0)
	{
		float BtnHeight = 20.0;
		float BtnWidth = 120.0;
		if (g_pBCustomMenuInfo->DrawButton(StartX + CuaSoW / 2 - (BtnWidth / 2), StartY + CuaSoH - BtnHeight - 10, BtnWidth, 12, "Close"))
		{
			gInterface.Data[eWindowMessageBox].OnShow = 0;
		}
	}
	else if (this->MsgBoxType == 1)
	{
		float BtnHeight = 20.0;
		float BtnWidth = 55.0;
		if (g_pBCustomMenuInfo->DrawButton(StartX + CuaSoW / 2 - (BtnWidth + 7.5), StartY + CuaSoH - BtnHeight - 10, BtnWidth, 12, "OK"))
		{
			this->MsgBoxCallback(this);
			this->MsgBoxCallback = 0;
			gInterface.Data[eWindowMessageBox].OnShow = 0;
		}

		if (g_pBCustomMenuInfo->DrawButton(StartX + CuaSoW / 2 + (7.5), StartY + CuaSoH - BtnHeight - 10, BtnWidth, 12, "Close"))
		{
			gInterface.Data[eWindowMessageBox].OnShow = 0;
		}
	}
	DisableAlphaBlend();
	EnableAlphaTest(0);

}

void Interface::OpenMessageBox(char* caption, char* Format, ...)
{
	if (gInterface.Data[eWindowMessageBox].OnShow)
	{
		return;
	}

	char text[1024] = { 0 };
	va_list va;
	va_start(va, Format);
	vsnprintf(text, sizeof(text), (Format != NULL) ? Format : "", va);
	va_end(va);
	this->MsgLine = 0;
	this->MsgBoxCaption[0] = 0;
	this->MsgBoxText[0] = 0;
	snprintf(this->MsgBoxCaption, sizeof(this->MsgBoxCaption), "%s", (caption != NULL) ? caption : "");
	snprintf(this->MsgBoxText, sizeof(this->MsgBoxText), "%s", text);

	this->MsgLine = 1;
	gInterface.Data[eWindowMessageBox].OnShow = true;
	this->MsgBoxType = 0;
	pSetCursorFocus = true;
}

void Interface::OpenMessageBoxOkCancel(PUSHEVENT_CALLBACK_LPVOID pCallbackFunc, char* caption, char* Format, ...)
{
	if (gInterface.Data[eWindowMessageBox].OnShow)
	{
		return;
	}

	char text[1024] = { 0 };
	va_list va;
	va_start(va, Format);
	vsnprintf(text, sizeof(text), (Format != NULL) ? Format : "", va);
	va_end(va);

	this->MsgLine = 0;
	this->MsgBoxCaption[0] = 0;
	this->MsgBoxText[0] = 0;
	snprintf(this->MsgBoxCaption, sizeof(this->MsgBoxCaption), "%s", (caption != NULL) ? caption : "");
	snprintf(this->MsgBoxText, sizeof(this->MsgBoxText), "%s", text);
	this->MsgLine = 1;
	gInterface.Data[eWindowMessageBox].OnShow = true;
	this->MsgBoxType = 1;
	this->MsgBoxCallback = pCallbackFunc;
	pSetCursorFocus = true;
}

void CBRenderText(char* text, int x, int y, int sx, int sy, DWORD color, DWORD backcolor, int sort)
{
	g_pRenderText->SetFont(g_hFont);

	DWORD backuptextcolor = g_pRenderText->GetTextColor();
	DWORD backuptextbackcolor = g_pRenderText->GetBgColor();

	g_pRenderText->SetTextColor(color);
	g_pRenderText->SetBgColor(backcolor);
	g_pRenderText->RenderText(x, y, text, sx, sy, sort);

	g_pRenderText->SetTextColor(backuptextcolor);
	g_pRenderText->SetBgColor(backuptextbackcolor);
}
char* Interface::QNumberFormat(QWORD number)
{
	std::ostringstream stream;
	stream.imbue(std::locale("")); // Định dạng số theo locale
	stream << number; // Xuất số nguyên vào stream
	std::string formattedNumber = stream.str();
	return strdup(formattedNumber.c_str()); // Cấp phát động chuỗi kết quả
}
char* Interface::fNumberFormat(float number)
{
	std::ostringstream stream;
	stream.imbue(std::locale("")); 
	stream << std::fixed << std::setprecision(2) << number;
	std::string formattedNumber = stream.str();
	formattedNumber.erase(formattedNumber.find_last_not_of('0'), std::string::npos);
	return strdup(formattedNumber.c_str());
}

char* Interface::NumberFormat(int Number)
{


	if (Number < 0) { return "0"; }

	char OutPut[15];

	if (Number < 1000) {
		sprintf(OutPut, "%d", Number);
		return strdup(OutPut);
	}
	else if (Number < 1000000) {
		int part1 = Number % 1000;
		int part2 = (Number - part1) / 1000;
		sprintf(OutPut, "%d,%03d", part2, part1);
		return strdup(OutPut);
	}
	else if (Number < 1000000000) {
		int part1 = Number % 1000;
		Number = (Number - part1) / 1000;
		int part2 = Number % 1000;
		Number = (Number - part2) / 1000;
		int part3 = Number % 1000;

		sprintf(OutPut, "%d,%03d,%03d", part3, part2, part1);
		return strdup(OutPut);
	}
	else {
		int part1 = Number % 1000;
		Number = (Number - part1) / 1000;
		int part2 = Number % 1000;
		Number = (Number - part2) / 1000;
		int part3 = Number % 1000;
		int part4 = (Number - part3) / 1000;

		sprintf(OutPut, "%d,%03d,%03d,%03d", part4, part3, part2, part1);
		return strdup(OutPut);
	}
	return "0";
}

void Interface::DrawBarForm(float PosX, float PosY, float Width, float Height, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	EnableAlphaTest(true);
	glColor4f((GLfloat)red, (GLfloat)green, (GLfloat)blue, (float)alpha);
	RenderColor(PosX, PosY, Width, Height, 0.0f, 0);
	EndRenderColor();
	glColor3f(1.0, 1.0, 1.0);
	EnableAlphaTest(false);
}

int Interface::DrawFormat(DWORD Color, int PosX, int PosY, int Width, int Align, LPCSTR Text, ...)
{
	char Buff[2048];
	int BuffLen = sizeof(Buff) - 1;
	ZeroMemory(Buff, BuffLen);

	va_list args;
	va_start(args, Text);
	int Len = vsprintf_s(Buff, BuffLen, Text, args);
	va_end(args);

	int LineCount = 0;

	char* Line = strtok(Buff, "\n");

	while (Line != NULL)
	{
		CBRenderText(Line, PosX, PosY, Width, 0, Color, 0, Align);

		PosY += 10;
		Line = strtok(NULL, "\n");
	}

	return PosY;
}
bool Interface::CheckWindow(int WindowID)
{
	return g_pNewUISystem->IsVisible(WindowID);
}
bool Interface::UseHotKey()
{
	if (GetForegroundWindow() != g_hWnd || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX))
	{
		return false;
	}
	if (SEASON3B::IsPress(VK_END))
	{
		gInterface.Data[eStatus_KepChuot].OnShow ^= 1;
		return true;
	}
	if (SEASON3B::IsPress(VK_F12))
	{
		gTrayMode.Toggle();
		return true;
	}
	//===ANtilag
	if (SEASON3B::IsPress(VK_F6))
	{

		g_pNewUISystem->GetUI_NewOptionWindow()->OnOffGrap[CNewUIOptionWindow::eGlowEffect] ^= 1;
		g_pNewUISystem->GetUI_NewOptionWindow()->OnOffGrap[CNewUIOptionWindow::eEffectDynamic] ^= 1;
		g_pNewUISystem->GetUI_NewOptionWindow()->OnOffGrap[CNewUIOptionWindow::eEffectStatic] ^= 1;
		g_pNewUISystem->GetUI_NewOptionWindow()->OnOffGrap[CNewUIOptionWindow::eExcellentEffect] ^= 1;
		g_RenderEff = !g_RenderEff;
		CreateNotice("[F6] Antilag I", 1);
		PlayBuffer(SOUND_CLICK01);
		return true;
	}
	if (SEASON3B::IsPress(VK_F7))
	{

		g_pNewUISystem->GetUI_NewOptionWindow()->OnOffGrap[CNewUIOptionWindow::eBMDImg] ^= 1;
		g_pNewUISystem->GetUI_NewOptionWindow()->OnOffGrap[CNewUIOptionWindow::eBMDWeapons] ^= 1;
		g_pNewUISystem->GetUI_NewOptionWindow()->OnOffGrap[CNewUIOptionWindow::eBMDWings] ^= 1;
		g_pNewUISystem->GetUI_NewOptionWindow()->OnOffGrap[CNewUIOptionWindow::eBMDMonter] ^= 1;
		g_pNewUISystem->GetUI_NewOptionWindow()->OnOffGrap[CNewUIOptionWindow::eBMDZen] ^= 1;
		CreateNotice("[F7] Antilag II", 1);
		PlayBuffer(SOUND_CLICK01);
		return true;
	}
	//===Event Time
	if (gProtect.m_MainInfo.Menu[eButtonEventTime])
	{
		if (SEASON3B::IsPress('H'))
		{
			gInterface.Data[eWindowEventTime].OpenClose();
			if (gInterface.Data[eWindowEventTime].OnShow)
			{
				gCustomEventTime.ClearCustomEventTime();

				gCustomEventTime.OpenTestWindow();
			}
			PlayBuffer(SOUND_CLICK01);
			return true;
		}
	}

	if (SEASON3B::IsPress(VK_F5))
	{
		gInterface.Data[eMenu_MAIN].OnShow ^= 1;
		PlayBuffer(SOUND_CLICK01);
		return true;
	}

	//===Ranking
	if (gProtect.m_MainInfo.Menu[eButtonRanking])
	{
		if (SEASON3B::IsPress(VK_F8))
		{
			gCustomRanking->OpenWindow();
			//gRanking.OpenOnOff();
			PlayBuffer(SOUND_CLICK01);
			return true;
		}
	}
	//===JewelBank
	if (gProtect.m_MainInfo.Menu[eButtonJewelBank])
	{
		if (SEASON3B::IsPress('J'))
		{
#if(JEWELBANKVER2)
			gCB_NewJewelBank->OpenOnOff();
#else
			gCBJewelBank.OpenOnOff();
#endif
			PlayBuffer(SOUND_CLICK01);
		}
	}
	if (SEASON3B::IsPress(VK_F10))
	{
		Camera3DEnable ^= 1;
		Camera3DSet = true;
		if (Camera3DEnable)
		{
			CreateNotice("On Camera 3D", 1);
		}
		else
		{
			CreateNotice("Off Camera 3D", 1);
		}
		PlayBuffer(SOUND_CLICK01);
		return true;
	}
	if (SEASON3B::IsPress(VK_F11))
	{
		if (SceneFlag > CHARACTER_SCENE)
		{
			CameraFOV = 40.f;
		}
		else
		{
			CameraFOV = 35.f;
		}
		CameraAngle[2] = -45.f;
		Camera3DPosY = 0.f;
		Camera3DPosZ = 0.f;
		Camera3DSet = false;
		CreateNotice("Reset Camera 3D", 1);
		PlayBuffer(SOUND_CLICK01);
		return true;
	}

	return false;
}
int Interface::DrawMessage(int Mode, LPCSTR Text, ...)
{
	char Buff[2048];
	int BuffLen = sizeof(Buff);
	ZeroMemory(Buff, BuffLen);
	// ----
	va_list args;
	va_start(args, Text);
	int Len = vsprintf_s(Buff, BuffLen, Text, args);
	va_end(args);
	// ----
	CreateNotice(Buff, Mode);
	return 1;
}
//===Close All Window CUstom
void Interface::CloseAllWindowCustom()
{
	for(int i = eBeginWindowCustom; i < eEndWindowCustom; i++) gInterface.Data[i].Close(); //Close All window
}
//== Xử Lý Send Get Item
bool Interface::ActionSendItemGS(ITEM* ItemSell, int Slot)
{
#if(CongHuongV2)
	if (gCB_CongHuong->SendItem(ItemSell, Slot, GetKeyState(VK_RBUTTON))) return 1;
#endif
#if(CB_HUYDONGEXC)
	if (gCBHuyDongExc->SendItem(ItemSell, Slot, GetKeyState(VK_RBUTTON))) return 1;
#endif
#if(CB_VIP_CHAR)
	if (gBInterfaceVIPChar->SendItem(ItemSell, Slot, TRUE)) return 1;
#endif
#if(CUSTOM_BCHOTROI)
	if (gCBChotroi.SendItemRaoBan(ItemSell, Slot, GetKeyState(VK_RBUTTON))) return 1;
#endif
#if(JEWELBANKVER2)
	if (gCB_NewJewelBank->SendItem(ItemSell, Slot, GetKeyState(VK_RBUTTON))) return 1;
#else
	if (gCBJewelBank.SendItem(ItemSell, Slot, GetKeyState(VK_RBUTTON))) return 1;
#endif
	if (ChangeSSendItem(ItemSell, Slot, GetKeyState(VK_RBUTTON))) return 1;
	return 0;
}
void Interface::Init()
{
#ifdef __ANDROID__
#define ANDROID_IF_STAGE(msg) do { OutputDebugStringA("UI_IFACE: " msg); g_ErrorReport.Write("[UI_IFACE] " msg "\r\n"); } while(0)
#else
#define ANDROID_IF_STAGE(msg) ((void)0)
#endif
	ANDROID_IF_STAGE("init begin");
	gCB_BXHTopDmg = new CB_BXHTopDmg;
	ANDROID_IF_STAGE("BXHTopDmg ok");

	gCB_BotTrader = new CB_BotTrader;
	ANDROID_IF_STAGE("BotTrader ok");

	gCB_StatsAdvance = new CB_StatsAdvance;
	ANDROID_IF_STAGE("StatsAdvance ok");

	gCB_DrawCustomRank = new CB_DrawCustomRank;
	ANDROID_IF_STAGE("DrawCustomRank ok");
#if(CB_GETMIXRATE)
	gCB_GetMixRate = new CB_GetMixRate;
	ANDROID_IF_STAGE("GetMixRate ok");
#endif
#if(CongHuongV2)
	gCB_CongHuong = new CB_CongHuong;
	ANDROID_IF_STAGE("CongHuong ok");
#endif
#if(CB_ActiveInvasionsD)
	gCB_ActiveInvasions = new CB_ActiveInvasions;
	ANDROID_IF_STAGE("ActiveInvasions ok");
#endif
#if(JEWELBANKVER2)
	gCB_NewJewelBank = new CB_NewJewelBank;
	ANDROID_IF_STAGE("NewJewelBank ok");
#endif
	gCustomRanking = new CustomRanking;
	ANDROID_IF_STAGE("CustomRanking ok");
#if(CB_AUTORESETINFO)
	gCBAutoResetInfo = new CBAutoResetInfo;
	ANDROID_IF_STAGE("AutoResetInfo ok");
#endif
#if(CB_VIP_CHAR)
	gBInterfaceVIPChar = new CBInterfaceVIPChar;
	ANDROID_IF_STAGE("VIPChar ok");
#endif
#if(CB_VIEWCHARITEM)
	gCB_ViewCharItem = new CB_ViewCharItem;
	ANDROID_IF_STAGE("ViewCharItem ok");
#endif
#if(CUSTOM_WINDOWLOCKITEM)
	gCB_LockItem = new CB_LockItem;
	ANDROID_IF_STAGE("LockItem ok");
#endif
#if(DOIMK)
	gCB_DoiMK = new CB_DoiMK;
	ANDROID_IF_STAGE("DoiMK ok");
#endif
#if(CB_CUSTOMMIXINFO)
	gCB_InfoCustomMix = new CB_InfoCustomMix;
	ANDROID_IF_STAGE("InfoCustomMix ok");
#endif
#if(CB_NEWQUESTFF)
	gCB_NewQuest = new CB_NewQuest;
	ANDROID_IF_STAGE("NewQuest ok");
#endif
#if(BOSS_GUILD)
	gCBBossGuild = new CBBossGuild;
	ANDROID_IF_STAGE("BossGuild ok");
#endif
#if(CB_HUYDONGEXC)
	gCBHuyDongExc = new CBHuyDongExc;
	ANDROID_IF_STAGE("HuyDongExc ok");
#endif
	gCB_Mapping = new CB_Mapping;
	ANDROID_IF_STAGE("Mapping ok");


#undef ANDROID_IF_STAGE
}

void Interface::Clear()
{
	SAFE_DELETE(gCB_BXHTopDmg);

	SAFE_DELETE(gCB_BotTrader);

	SAFE_DELETE(gCB_StatsAdvance);

	SAFE_DELETE(gCB_Mapping);

	SAFE_DELETE(gCB_DrawCustomRank);

#if(CB_GETMIXRATE)
	SAFE_DELETE(gCB_GetMixRate);
#endif
#if(CB_HUYDONGEXC)
	SAFE_DELETE(gCBHuyDongExc);
#endif
#if(CB_NEWQUESTFF)
	SAFE_DELETE(gCB_NewQuest);
#endif
#if(CB_CUSTOMMIXINFO)
	SAFE_DELETE(gCB_InfoCustomMix);
#endif

#if(DOIMK)
	SAFE_DELETE(gCB_DoiMK);
#endif
#if(CB_AUTORESETINFO)
	SAFE_DELETE(gCBAutoResetInfo);
#endif
#if(JEWELBANKVER2)
	SAFE_DELETE(gCB_NewJewelBank);
#endif
	SAFE_DELETE(gCustomRanking);
#if(CB_VIP_CHAR)
	SAFE_DELETE(gBInterfaceVIPChar);
#endif
#if(CB_VIEWCHARITEM)
	SAFE_DELETE(gCB_ViewCharItem);
#endif
#if(CUSTOM_WINDOWLOCKITEM)
	SAFE_DELETE(gCB_LockItem);
#endif

#if(BOSS_GUILD)
	SAFE_DELETE(gCBBossGuild);
#endif
#if(CB_ActiveInvasionsD)
	SAFE_DELETE(gCB_ActiveInvasions);
#endif
#if(CongHuongV2)
	SAFE_DELETE(gCB_CongHuong);
#endif
}
//===Draw Window
void Interface::Work()
{
	if (gCB_BXHTopDmg) gCB_BXHTopDmg->DrawWindowMini();//

	if (gCB_DrawCustomRank) gCB_DrawCustomRank->Draw();
#if(SAUDOIITEM)
	Interface::DrawDoiItem();
#endif
	//===Kep Chuot Tu dong
	if (gInterface.Data[eStatus_KepChuot].OnShow)
	{
		gInterface.DrawFormat(eGold, 120, 45, 200, 1, "Tự động đánh bật");
		gInterface.DrawFormat(eGold, 120, 60, 100, 1, "END: Tắt Auto");
		MouseRButton = 1;
	}
	if (gProtect.m_MainInfo.LoadAntihack)
	{
		if (gCB_Mapping) gCB_Mapping->Update();
	}
#if(CongHuongV2)
	if (gCB_CongHuong) gCB_CongHuong->DrawWindowCongHuong();
#endif
#if(CB_ActiveInvasionsD)
	if (gCB_ActiveInvasions) gCB_ActiveInvasions->DrawWindow();
#endif

#if(CB_HUYDONGEXC)
	if (gCBHuyDongExc) gCBHuyDongExc->DrawWindowHuyExc();
#endif
#if(CB_NEWQUESTFF)
	if (gCB_NewQuest) gCB_NewQuest->DrawWindow();
#endif
#if(DOIMK)
	if (gCB_DoiMK) gCB_DoiMK->DrawWindow();
#endif
#if(CUSTOM_WINDOWLOCKITEM)
	if(gCB_LockItem) gCB_LockItem->DrawWindow();
#endif
	gInterface.DrawNoticePKSystem();
#if(CB_VIEWCHARITEM)
	if (gCB_ViewCharItem) gCB_ViewCharItem->DrawWindow();
#endif
#if(CB_VIP_CHAR)
	if(gBInterfaceVIPChar) gBInterfaceVIPChar->Draw();
#endif
#if(CB_AUTORESETINFO) 
	if (gCBAutoResetInfo) gCBAutoResetInfo->DrawWindow();
#endif
#if(CB_ANTIHACKGGNEW)
	gAPICB.Work();
#endif
	gCustomMenu.Draw();
	gCustomEventTime.DrawEventTimePanelWindow();
	gInterface.DrawWindowMuaVIP();
	gCustomRanking->DrawRankPanelWindow();
	//gRanking.Draw();
	WindowClass.Draw();
#if(CUSTOM_BCHOTROI)
	gCBChotroi.BDrawWindowChoTroi();
#endif
	gCBAutoNapGame.DrawWindowAutoBanking();
#if DANH_HIEU_NEW
	gCBDanhHieu.DrawWindow();
#endif
	gVongQuay.DrawWindowVQ();
#if(JEWELBANKVER2)
	if (gCB_NewJewelBank)
	{
		gCB_NewJewelBank->DrawWindow();
		gCB_NewJewelBank->DrawNhapSoNgoc();
	}
#else
	gCBJewelBank.DrawWindow();
#endif
	if (gCBBossGuild) gCBBossGuild->DrawWindow();
	gCBCTCMini.DrawWindow();

	gMocDonate.DrawWindow();
	gMocDonate.DrawXemMocNap();

	//=== Luôn để cuối cùng !!
	gInterface.DrawTimeUseMap();
	gInterface.DrawInfoMini();
	gInterface.DrawMessageBox();

}
//==Load Img
void Interface::BLoadImage()
{
	g_pBCustomMenuInfo->BindObject(eMenu, "Interface\\Menu2.tga", 16, 12, -1, -1);
	g_pBCustomMenuInfo->BindObject(eMenuBG, "Interface\\Menu_BG.tga", 36, 18, 174, 0);
	g_pBCustomMenuInfo->BindObject(eButtonMXH, "Custom\\Interface\\buttonex.tga");
	g_pBCustomMenuInfo->BindObject(eButtonNext, "Interface\\Next.tga");
	g_pBCustomMenuInfo->BindObject(eButtonPrevious, "Interface\\Previous.tga");
#if DANH_HIEU_NEW
	char StringDirDH[128] = { 0 };
	for (int st = 0; st < 50; st++)
	{
		wsprintf(StringDirDH, "Custom\\DanhHieu\\%d.tga", st+1);
		g_pBCustomMenuInfo->BindObject(eDanhImg+st, StringDirDH);
	}
#endif

	g_pBCustomMenuInfo->BindObject(IMG_31617, "Interface\\newui_SW_MiniMap_Bt_clearness.jpg");
	g_pBCustomMenuInfo->BindObject(IMG_31618, "Interface\\newui_SW_Minimap_Bt_group.tga");
	g_pBCustomMenuInfo->BindObject(IMG_31764, "Interface\\MacroUI\\MacroUI_Gauge_Frame.tga");
	g_pBCustomMenuInfo->BindObject(IMG_31765, "Interface\\MacroUI\\MacroUI_Gauge_Bar_(B).jpg");
#if MaNap
	LoadBitmap("Custom\\Interface\\Donate.tga", IMG_4046, GL_NEAREST, GL_CLAMP_TO_EDGE);
#endif
#if(CB_DANGKYINGAME)
	g_pBCustomMenuInfo->BindObject(IMG_71620, "Custom\\Interface\\newui_button_register.tga");
#endif


	g_pBCustomMenuInfo->BindObject(IMG_HPBarFrame, "Custom\\HPBarFrame.tga");
	g_pBCustomMenuInfo->BindObject(IMG_HPBarValue, "Custom\\HPBarValue.jpg");

	gCBCTCMini.LoadImg();
}
std::string Interface::generateCaptcha(int n)
{
	time_t t;
	srand((unsigned)time(&t));
	char* required_chars = "0123456789";
	std::string captcha = "";
	while (n--)
		captcha.push_back(required_chars[rand() % sizeof(required_chars)]);
	return captcha;
}

bool Interface::check_Captcha(std::string& captcha, std::string& user_input)
{
	return captcha.compare(user_input) == 0;
}
void Interface::RenderCaptchaNumber(float PosX, float PosY, CUITextInputBox* a6, LPCSTR Text, ...)
{
	char Buff[2048];
	int BuffLen = sizeof(Buff) - 1;
	ZeroMemory(Buff, BuffLen);
	// ----
	va_list args;
	va_start(args, Text);
	int Len = vsprintf_s(Buff, BuffLen, Text, args);
	va_end(args);


	gInterface.DrawBarForm(PosX, PosY + 3.5, 120, 17, 0.0, 0.0, 0.0, 1.0);
	gInterface.DrawBarForm(PosX + 2, PosY + 4, 50, 15, 1.0, 0.2167, 0.0, 1.0);
	TextDraw((HFONT)g_hFontBold, PosX + 2, PosY + 4 + 2, 0xFFFFFFB8, 0x0, 50, 0, 3, Buff); //Ma Xac Nhan
	a6->SetPosition(PosX + 60, PosY + 6.5);
	a6->Render();
	a6->DoAction();

	if (SEASON3B::CheckMouseIn(PosX - 5, PosY - 5, 120, 30) == 1)
	{
		if (IsMobileUiTap())
		{
			a6->GiveFocus(1);
			PlayBuffer(25, 0, 0);
		}
	}
	//gCentral.DrawStringInput(PosX + 60, PosY + 5, 50, 13, 3, a6, 0x57575750, 0x0, 0xFFFFFFFF);
}


//=== Draw Menu Custom
void Interface::DrawInfoMini()
{
	if (gInterface.CheckWindow(ObjWindow::CashShop)
		|| gInterface.CheckWindow(ObjWindow::FriendList)
		|| gInterface.CheckWindow(ObjWindow::MoveList)
		|| gInterface.CheckWindow(ObjWindow::Party)
		|| gInterface.CheckWindow(ObjWindow::Quest)
		|| gInterface.CheckWindow(ObjWindow::NPC_Devin)
		|| gInterface.CheckWindow(ObjWindow::Guild)
		|| gInterface.CheckWindow(ObjWindow::Trade)
		|| gInterface.CheckWindow(ObjWindow::Warehouse)
		|| gInterface.CheckWindow(ObjWindow::ChaosBox)
		|| gInterface.CheckWindow(ObjWindow::CommandWindow)
		|| gInterface.CheckWindow(ObjWindow::PetInfo)
		|| gInterface.CheckWindow(ObjWindow::Shop)
		|| gInterface.CheckWindow(ObjWindow::Inventory)
		|| gInterface.CheckWindow(ObjWindow::Store)
		|| gInterface.CheckWindow(ObjWindow::OtherStore)
		|| gInterface.CheckWindow(ObjWindow::Character)
		|| gInterface.CheckWindow(ObjWindow::DevilSquare)
		|| gInterface.CheckWindow(ObjWindow::BloodCastle)
		|| gInterface.CheckWindow(ObjWindow::CreateGuild)
		|| gInterface.CheckWindow(ObjWindow::GuardNPC)
		|| gInterface.CheckWindow(ObjWindow::SeniorNPC)
		|| gInterface.CheckWindow(ObjWindow::GuardNPC2)
		|| gInterface.CheckWindow(ObjWindow::CastleGateSwitch)
		|| gInterface.CheckWindow(ObjWindow::CatapultNPC)
		|| gInterface.CheckWindow(ObjWindow::CrywolfGate)
		|| gInterface.CheckWindow(ObjWindow::IllusionTemple)
		|| gInterface.CheckWindow(ObjWindow::HeroList)
		|| gInterface.CheckWindow(ObjWindow::ChatWindow)
		|| gInterface.CheckWindow(ObjWindow::FastMenu)
		|| gInterface.CheckWindow(ObjWindow::Options)
		|| gInterface.CheckWindow(ObjWindow::Help)
		|| gInterface.CheckWindow(ObjWindow::FastDial)
		|| gInterface.CheckWindow(ObjWindow::SkillTree)
		|| gInterface.CheckWindow(ObjWindow::GoldenArcher1)
		|| gInterface.CheckWindow(ObjWindow::GoldenArcher2)
		|| gInterface.CheckWindow(ObjWindow::LuckyCoin1)
		|| gInterface.CheckWindow(ObjWindow::LuckyCoin2)
		|| gInterface.CheckWindow(ObjWindow::NPC_Duel)
		|| gInterface.CheckWindow(ObjWindow::NPC_Titus)
		|| gInterface.CheckWindow(ObjWindow::CashShop)
		|| gInterface.CheckWindow(ObjWindow::Lugard)
		|| gInterface.CheckWindow(ObjWindow::QuestList1)
		|| gInterface.CheckWindow(ObjWindow::QuestList2)
		|| gInterface.CheckWindow(ObjWindow::Jerint)
		|| gInterface.CheckWindow(ObjWindow::FullMap)
		|| gInterface.CheckWindow(ObjWindow::NPC_Dialog)
		|| gInterface.CheckWindow(ObjWindow::GensInfo)
		|| gInterface.CheckWindow(ObjWindow::NPC_Julia)
		|| gInterface.CheckWindow(ObjWindow::NPC_ChaosMix)
		|| gInterface.CheckWindow(ObjWindow::ExpandInventory)
		|| gInterface.CheckWindow(ObjWindow::ExpandWarehouse)
		|| gInterface.HidenCustom
		|| gInterface.CheckWindow(ObjWindow::MuHelper))
	{
		return;
	}

	float SizeButtonW = 26;
	float SizeButtonH = 26;
	float StartX = 5;
	float StartY = DisplayHeight - SizeButtonW - 50;
	EnableAlphaTest(true);
	glColor3f(1.0, 1.0, 1.0);



	if (gProtect.m_MainInfo.EnableButtonFB)
	{
		if (SEASON3B::CheckMouseIn(gProtect.m_MainInfo.ViTriFB[0], gProtect.m_MainInfo.ViTriFB[1]+ DisplayHeightExt, SizeButtonW, SizeButtonH) && GetForegroundWindow() == g_hWnd)
		{
			glColor4f(0.0, 0.5, 0.5, 1.0);
			RenderBitmap(gInterface.Data[eButtonMXH].ModelID, gProtect.m_MainInfo.ViTriFB[0], gProtect.m_MainInfo.ViTriFB[1]+ DisplayHeightExt, SizeButtonW, SizeButtonH, 0.0, 0.0, 0.37, 0.78, 1, 1, 0);//FB
			glColor4f(1.0, 1.0, 1.0, 1.0);
			g_pBCustomMenuInfo->SetBlockCur();

			if (GetTickCount() - gInterface.Data[eTIME].EventTick > 500) //Click
			{
				if (GetKeyState(VK_LBUTTON) & 0x8000)
				{
					ShellExecute(0, 0, gProtect.m_MainInfo.LinkFacbook, 0, 0, SW_SHOW);
					gInterface.Data[eTIME].EventTick = GetTickCount();
					PlayBuffer(25, 0, 0);
				}
			}
		}
		else
		{
			RenderBitmap(gInterface.Data[eButtonMXH].ModelID, gProtect.m_MainInfo.ViTriFB[0], gProtect.m_MainInfo.ViTriFB[1] + DisplayHeightExt, SizeButtonW, SizeButtonH, 0.0, 0.0, 0.37, 0.78, 1, 1, 0);//FB
		}
	}
	if (gProtect.m_MainInfo.EnableButtonZL)
	{
		if (SEASON3B::CheckMouseIn(gProtect.m_MainInfo.ViTriZL[0], gProtect.m_MainInfo.ViTriZL[1] + DisplayHeightExt, SizeButtonW, SizeButtonH) && GetForegroundWindow() == pGameWindow)
		{
			glColor4f(0.0, 0.5, 0.5, 1.0);
			RenderBitmap(gInterface.Data[eButtonMXH].ModelID, gProtect.m_MainInfo.ViTriZL[0], gProtect.m_MainInfo.ViTriZL[1] + DisplayHeightExt, SizeButtonW, SizeButtonH, 0.37, 0.0, 0.37, 0.78, 1, 1, 0);//ZL
			glColor4f(1.0, 1.0, 1.0, 1.0);
			g_pBCustomMenuInfo->SetBlockCur();

			if (GetTickCount() - gInterface.Data[eTIME].EventTick > 500) //Click
			{
				if (GetKeyState(VK_LBUTTON) & 0x8000)
				{
					ShellExecute(0, 0, gProtect.m_MainInfo.LinkZalo, 0, 0, SW_SHOW);
					gInterface.Data[eTIME].EventTick = GetTickCount();
					PlayBuffer(25, 0, 0);
				}
			}
		}
		else
		{
			RenderBitmap(gInterface.Data[eButtonMXH].ModelID, gProtect.m_MainInfo.ViTriZL[0], gProtect.m_MainInfo.ViTriZL[1] + DisplayHeightExt, SizeButtonW, SizeButtonH, 0.37, 0.0, 0.37, 0.78, 1, 1, 0);//ZL
		}
	}
	if (gProtect.m_MainInfo.EnableButtonRS)
	{
		//=== Button Auto Reset
		//if (g_pBCustomMenuInfo->DrawButtonGUI(32339, gProtect.m_MainInfo.ViTriRS[0], gProtect.m_MainInfo.ViTriRS[1] + DisplayHeightExt, 25, 25, 3)) 
		if (g_pBCustomMenuInfo->DrawButton(gProtect.m_MainInfo.ViTriRS[0], gProtect.m_MainInfo.ViTriRS[1] + DisplayHeightExt, 100, 12, " ", 55))
		{
			gCBAutoResetInfo->OpenWindow();
		}
		TextDraw((HFONT)g_hFontBold, gProtect.m_MainInfo.ViTriRS[0], gProtect.m_MainInfo.ViTriRS[1] + DisplayHeightExt + 6, 0xFFFFFFFF, 0x0, 55, 0, 3, "Auto Reset");
		//if (SEASON3B::CheckMouseIn(StartX + 2, StartY, 25, 25) == 1)
		if (SEASON3B::CheckMouseIn(gProtect.m_MainInfo.ViTriRS[0], gProtect.m_MainInfo.ViTriRS[1] + DisplayHeightExt, 55, 12) == 1)
		{
			RenderTipText(MouseX + 30, MouseY, gCustomMessage.GetMessage(25));
		}
	}

	if (gProtect.m_MainInfo.EnableButtonAutoHP)
	{
		//Auto HP
		if (g_pBCustomMenuInfo->DrawButton(gProtect.m_MainInfo.ViTriAutoHP[0], gProtect.m_MainInfo.ViTriAutoHP[1] + DisplayHeightExt, 100, 12, " ", 55))
		{
			g_pBCustomMenuInfo->AutoHP ^= 1;
		}
		if (g_pBCustomMenuInfo->AutoHP)
		{
			TextDraw((HFONT)g_hFontBold, gProtect.m_MainInfo.ViTriAutoHP[0], gProtect.m_MainInfo.ViTriAutoHP[1] + DisplayHeightExt + 6, 0xFFC738FF, 0x0, 55, 0, 3, "HP[ON]");
		}
		else
		{
			TextDraw((HFONT)g_hFontBold, gProtect.m_MainInfo.ViTriAutoHP[0], gProtect.m_MainInfo.ViTriAutoHP[1] + DisplayHeightExt + 6, 0xFFFFFFFF, 0x0, 55, 0, 3, "HP[OFF]");
		}
	}

	if (gProtect.m_MainInfo.EnableButtonAutoPK)
	{
		//Auto HP và PK
		//if (g_pBCustomMenuInfo->DrawButtonGUI(32339, gProtect.m_MainInfo.ViTriAutoPK[0], gProtect.m_MainInfo.ViTriAutoPK[1] + DisplayHeightExt, 25, 25, 3)) 
		if (g_pBCustomMenuInfo->DrawButton(gProtect.m_MainInfo.ViTriAutoPK[0], gProtect.m_MainInfo.ViTriAutoPK[1] + DisplayHeightExt, 100, 12, " ", 55))
		{
			g_pBCustomMenuInfo->AutoHP ^= 1;
			g_pBCustomMenuInfo->AutoCtrlPK ^= 1;
		}
		if (g_pBCustomMenuInfo->AutoCtrlPK)
		{
			TextDraw((HFONT)g_hFontBold, gProtect.m_MainInfo.ViTriAutoPK[0], gProtect.m_MainInfo.ViTriAutoPK[1] + DisplayHeightExt + 6, 0xFFC738FF, 0x0, 55, 0, 3, "HP và PK [ON]");
		}
		else
		{
			TextDraw((HFONT)g_hFontBold, gProtect.m_MainInfo.ViTriAutoPK[0], gProtect.m_MainInfo.ViTriAutoPK[1] + DisplayHeightExt + 6, 0xFFFFFFFF, 0x0, 55, 0, 3, "HP và PK [OFF]");
		}
	}

	//=====================================
	glColor3f(1.0f, 1.0f, 1.0f); //Xoa mau
	DisableAlphaBlend();
	EnableAlphaTest(0);

}

void Interface::DrawWindowMuaVIP()
{
	if (!gInterface.Data[eVip_MAIN].OnShow)
	{
		return;
	}

	if (gProtect.m_MainInfo.Menu[eButtonVipShop] != 1)
	{
		gInterface.Data[eVip_MAIN].OnShow = false;
		return;
	}

	if (gInterface.CheckWindow(ObjWindow::MoveList) || gInterface.CheckWindow(ObjWindow::CashShop) || gInterface.CheckWindow(ObjWindow::SkillTree) || gInterface.CheckWindow(ObjWindow::FullMap)
		|| (gInterface.CheckWindow(Inventory)
			&& gInterface.CheckWindow(ExpandInventory)
			&& gInterface.CheckWindow(Store))
		|| (gInterface.CheckWindow(Inventory)
			&& gInterface.CheckWindow(Warehouse)
			&& gInterface.CheckWindow(ExpandWarehouse)))
	{
		gInterface.Data[eVip_MAIN].OnShow = false;
		return;
	}

	// ----
	float MainWidth = 230.0;
	float MainHeight = 270;
	
	float StartY = 40.0;
	float StartX = (MAX_WIN_WIDTH / 2) - (MainWidth / 2);
	float MainCenter = StartX + (MainWidth / 3);
	float ButtonX = MainCenter - (float)(29.0 / 2);
	// ----

	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, MainWidth, MainHeight, eVip_MAIN, gCustomMessage.GetMessage(5));
	float StartBody = StartY;

	int LineY = gInterface.DrawFormat(eWhite, (int)StartX + 30, (int)StartBody + 35, 210, 1, gCustomMessage.GetMessage(6), CharacterAttribute->Name);
	LineY = gInterface.DrawFormat(eGold, (int)StartX + 30, (int)LineY, 210, 1, gCustomMessage.GetMessage(14));
	LineY = gInterface.DrawFormat(eWhite, (int)StartX + 30, (int)LineY, 210, 1, gCustomMessage.GetMessage(7));
	LineY = gInterface.DrawFormat(eWhite, (int)StartX + 30, (int)LineY, 210, 1, gCustomMessage.GetMessage(8));

	// ----
	//gInterface.DrawGUI(eVip_DIV, StartX, StartBody + 70);
	gInterface.DrawFormat(eGold, (int)StartX + 30, (int)StartBody + 95 - 2, 210, 1, "Plan");
	gInterface.DrawFormat(eGold, (int)StartX + 80, (int)StartBody + 95 - 2, 210, 1, "Exp");
	gInterface.DrawFormat(eGold, (int)StartX + 130, (int)StartBody + 95 - 2, 210, 1, "Drop");
	gInterface.DrawFormat(eGold, (int)StartX + 170, (int)StartBody + 95 - 2, 210, 1, "Time");
	// ----
	int HoverSelect = -1;
	for (int i = 0; i < MAX_CUSTOM_BUYVIP; i++)
	{
		if (gCustomBuyVip.m_CustomBuyVipInfo[i].Index == -1) continue;

		gInterface.DrawFormat(eWhite, (int)StartX + 30, (int)StartBody + 110 + (12 * i) - 5, 210, 1, gCustomBuyVip.m_CustomBuyVipInfo[i].VipName);
		// ----
		gInterface.DrawFormat(eWhite, (int)StartX + 80, (int)StartBody + 110 + (12 * i) - 5, 210, 1, "+%d%%", gCustomBuyVip.m_CustomBuyVipInfo[i].Exp);
		// ----
		gInterface.DrawFormat(eWhite, (int)StartX + 130, (int)StartBody + 110 + (12 * i) - 5, 210, 1, "+%d%%", gCustomBuyVip.m_CustomBuyVipInfo[i].Drop);
		// ----
		gInterface.DrawFormat(eWhite, (int)StartX + 170, (int)StartBody + 110 + (12 * i) - 5, 210, 1, "%d day(s)", gCustomBuyVip.m_CustomBuyVipInfo[i].Days);

		RenderImage(CNewUIWindowMenu::IMAGE_WINDOW_MENU_LINE, StartX + 30, StartBody + 110 + (12 * i) + 9 - 5, 82.0, 2.0);
		RenderImage(CNewUIWindowMenu::IMAGE_WINDOW_MENU_LINE, StartX + 30 + 82, StartBody + 110 + (12 * i) + 9 - 5, 82.0, 2.0);

		//==Button VIP
		if (g_pBCustomMenuInfo->DrawButton((int)StartX + 60, (int)StartBody + 160 + (25 * i) - 5, 120, 12, gCustomBuyVip.m_CustomBuyVipInfo[i].VipName))
		{
			gCustomBuyVip.BuyStatus(i+1);
		}
		if (SEASON3B::CheckMouseIn((int)StartX + 60, (int)StartBody + 160 + (25 * i) - 5, 120, 25) == 1)
		{
			gInterface.DrawFormat(eGold, (int)StartX + 30, (int)StartBody + 142, 210, 1, gCustomMessage.GetMessage(10));
			gInterface.DrawFormat(eGold, (int)StartX + 70, (int)StartBody + 142, 210, 1, gCustomMessage.GetMessage(11), gCustomBuyVip.m_CustomBuyVipInfo[i].Coin1);
			gInterface.DrawFormat(eGold, (int)StartX + 120, (int)StartBody + 142, 210, 1, gCustomMessage.GetMessage(12), gCustomBuyVip.m_CustomBuyVipInfo[i].Coin2);
			gInterface.DrawFormat(eGold, (int)StartX + 160, (int)StartBody + 142, 210, 1, gCustomMessage.GetMessage(13), gCustomBuyVip.m_CustomBuyVipInfo[i].Coin3);
		}
	}

}


void Interface::BRenderTimeCountDown(float x, float y, float w, int MatchTime)
{
	char lpszStr[256];
	char lpszStrS[10];
	int iMinute = MatchTime / 60;
	DWORD ColorText = 0xE6FCF7FF;
	wsprintf(lpszStr, " %.2d :", iMinute);

	int iSecondTime = MatchTime - (iMinute * 60);
	if (iSecondTime >= 0)
	{
		wsprintf(lpszStrS, " %.2d", iSecondTime);
		strcat(lpszStr, lpszStrS);
	}

	if (iMinute < 5)
	{
		ColorText = 0xFF2121FF;
	}
	if (iMinute < 15)
	{
		wsprintf(lpszStrS, ": %.2d", ((int)WorldTime % 60));
		strcat(lpszStr, lpszStrS);
	}
	HFONT FontTextTime = CreateFontA(18, 0, 0, 0, 700, 0, 0, 0, 0x1, 0, 0, 3u, 0, "Arial");
	TextDraw(FontTextTime, x, y, ColorText, 0x0, w, 0, 3, lpszStr); //
	DeleteObject(FontTextTime);

}


void Interface::RecvInfoTime(BYTE* Recv)
{
	if (!Recv) return;
	RECV_CUSTOMTIMECOUNT* lpMsg = (RECV_CUSTOMTIMECOUNT*)Recv;
	gInterface.NumberMapTime = lpMsg->MapNumber;
	gInterface.MapTime = GetTimeGMT() + lpMsg->TimeCount;
	gInterface.TextMapTime = lpMsg->TextName;
	gInterface.TypeStyle = lpMsg->TypeStyle;
}

int Interface::GetTimeGMT()
{
	time_t rawTime;
	struct tm* timeInfo;

	time(&rawTime);
	timeInfo = gmtime(&rawTime);

	timeInfo->tm_hour += (gProtect.m_MainInfo.TIME_GMT);
	return mktime(timeInfo);
}
SIZE Interface::getTextSize(HFONT fontsize, char* a1)
{
	SelectObject(g_pRenderText->GetFontDC(), fontsize);
	SIZE TextSize = { 0, 0 };
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), a1, strlen(a1), &TextSize);
	return TextSize;
}
void Interface::DrawTimeUseMap()
{

	if (gInterface.CheckWindow(ObjWindow::CashShop)
		|| gInterface.CheckWindow(ObjWindow::FriendList)
		|| gInterface.CheckWindow(ObjWindow::MoveList)
		|| gInterface.CheckWindow(ObjWindow::Party)
		|| gInterface.CheckWindow(ObjWindow::Quest)
		|| gInterface.CheckWindow(ObjWindow::NPC_Devin)
		|| gInterface.CheckWindow(ObjWindow::Guild)
		|| gInterface.CheckWindow(ObjWindow::Trade)
		|| gInterface.CheckWindow(ObjWindow::Warehouse)
		|| gInterface.CheckWindow(ObjWindow::ChaosBox)
		|| gInterface.CheckWindow(ObjWindow::CommandWindow)
		|| gInterface.CheckWindow(ObjWindow::PetInfo)
		|| gInterface.CheckWindow(ObjWindow::Shop)
		|| gInterface.CheckWindow(ObjWindow::Inventory)
		|| gInterface.CheckWindow(ObjWindow::Store)
		|| gInterface.CheckWindow(ObjWindow::OtherStore)
		|| gInterface.CheckWindow(ObjWindow::Character)
		|| gInterface.CheckWindow(ObjWindow::DevilSquare)
		|| gInterface.CheckWindow(ObjWindow::BloodCastle)
		|| gInterface.CheckWindow(ObjWindow::CreateGuild)
		|| gInterface.CheckWindow(ObjWindow::GuardNPC)
		|| gInterface.CheckWindow(ObjWindow::SeniorNPC)
		|| gInterface.CheckWindow(ObjWindow::GuardNPC2)
		|| gInterface.CheckWindow(ObjWindow::CastleGateSwitch)
		|| gInterface.CheckWindow(ObjWindow::CatapultNPC)
		|| gInterface.CheckWindow(ObjWindow::CrywolfGate)
		|| gInterface.CheckWindow(ObjWindow::IllusionTemple)
		|| gInterface.CheckWindow(ObjWindow::HeroList)
		|| gInterface.CheckWindow(ObjWindow::ChatWindow)
		|| gInterface.CheckWindow(ObjWindow::FastMenu)
		|| gInterface.CheckWindow(ObjWindow::Options)
		|| gInterface.CheckWindow(ObjWindow::Help)
		|| gInterface.CheckWindow(ObjWindow::FastDial)
		|| gInterface.CheckWindow(ObjWindow::SkillTree)
		|| gInterface.CheckWindow(ObjWindow::GoldenArcher1)
		|| gInterface.CheckWindow(ObjWindow::GoldenArcher2)
		|| gInterface.CheckWindow(ObjWindow::LuckyCoin1)
		|| gInterface.CheckWindow(ObjWindow::LuckyCoin2)
		|| gInterface.CheckWindow(ObjWindow::NPC_Duel)
		|| gInterface.CheckWindow(ObjWindow::NPC_Titus)
		|| gInterface.CheckWindow(ObjWindow::CashShop)
		|| gInterface.CheckWindow(ObjWindow::Lugard)
		|| gInterface.CheckWindow(ObjWindow::QuestList1)
		|| gInterface.CheckWindow(ObjWindow::QuestList2)
		|| gInterface.CheckWindow(ObjWindow::Jerint)
		|| gInterface.CheckWindow(ObjWindow::FullMap)
		|| gInterface.CheckWindow(ObjWindow::NPC_Dialog)
		|| gInterface.CheckWindow(ObjWindow::GensInfo)
		|| gInterface.CheckWindow(ObjWindow::NPC_Julia)
		|| gInterface.CheckWindow(ObjWindow::NPC_ChaosMix)
		|| gInterface.CheckWindow(ObjWindow::ExpandInventory)
		|| gInterface.CheckWindow(ObjWindow::ExpandWarehouse)
		|| gInterface.CheckWindow(ObjWindow::MuHelper))
	{
		return;
	}
	if (gInterface.NumberMapTime != gMapManager.WorldActive)
	{
		gInterface.NumberMapTime = 255;
		gInterface.MapTime = 0;
		gInterface.TypeStyle = 0;
		return;
	}
	if ((gInterface.MapTime - GetTimeGMT() < 1))
	{
		gInterface.NumberMapTime = 255;
		gInterface.MapTime = 0;
		gInterface.TypeStyle = 0;
		return;
	}
	//zText1.Draw(102, 170, 290, 5, 15, false);
	if (gInterface.TypeStyle == 1)//Center
	{
		float InfoW = 120;
		float InfoH = 10;
		float InfoX = (MAX_WIN_WIDTH / 2) - (InfoW / 2);
		float InfoY = MAX_WIN_HEIGHT - (InfoH + 65);
		//gCBUtil.gInfoBox(InfoX, InfoY, InfoW, InfoH, 0x00000096, 0, 0); //1vs3

		HFONT FontTextTime = CreateFontA(16, 0, 0, 0, 700, 0, 0, 0, 0x1, 0, 0, 3u, 0, "Arial");
		SIZE GetFontSizeName = getTextSize(FontTextTime, strdup(gInterface.TextMapTime.c_str()));
		if (!gInterface.TextMapTime.empty())
		{
			TextDraw(FontTextTime, InfoX, InfoY + 5, 0xFFAA00FF, 0x0, 0, 0, 1, gInterface.TextMapTime.c_str()); //Buoc Vao
		}
		else
		{
			TextDraw(FontTextTime, InfoX, InfoY + 5, 0xFFAA00FF, 0x0, 0, 0, 1, "Timer Countdown"); //Buoc Vao
		}
		DeleteObject(FontTextTime);
		BRenderTimeCountDown(InfoX + (((float)GetFontSizeName.cx / *(float*)0xE7C3D4) - 10), InfoY + 5, InfoW + 10, gInterface.MapTime - GetTimeGMT());
	}
	else if (gInterface.TypeStyle == 2) //Right
	{
		float InfoW = 120;
		float InfoH = 30;
		float InfoX = MAX_WIN_WIDTH - InfoW - 20;
		float InfoY = MAX_WIN_HEIGHT - (InfoH + 65);
		g_pBCustomMenuInfo->DrawInfoBox(InfoX, InfoY, InfoW, InfoH, 0x00000096, 0); //1vs3
		HFONT FontTextTime = CreateFontA(16, 0, 0, 0, 700, 0, 0, 0, 0x1, 0, 0, 3u, 0, "Arial");
		if (!gInterface.TextMapTime.empty())
		{
			TextDraw(FontTextTime, InfoX, InfoY + 5, 0xFFAA00FF, 0x0, InfoW + 10, 0, 3, gInterface.TextMapTime.c_str()); //Buoc Vao
		}
		else
		{
			TextDraw(FontTextTime, InfoX, InfoY + 5, 0xFFAA00FF, 0x0, InfoW + 10, 0, 3, "Timer Countdown"); //Buoc Vao
		}
		DeleteObject(FontTextTime);
		BRenderTimeCountDown(InfoX, (InfoY + InfoH) - 7, InfoW + 10, gInterface.MapTime - GetTimeGMT());
	}
	else //left
	{
		float InfoW = 120;
		float InfoH = 30;
		float InfoX = 5;
		float InfoY = MAX_WIN_HEIGHT - (InfoH + 65);
		g_pBCustomMenuInfo->DrawInfoBox(InfoX, InfoY, InfoW, InfoH, 0x00000096, 0); //1vs3
		HFONT FontTextTime = CreateFontA(16, 0, 0, 0, 700, 0, 0, 0, 0x1, 0, 0, 3u, 0, "Arial");
		if (!gInterface.TextMapTime.empty())
		{
			TextDraw(FontTextTime, InfoX, InfoY + 5, 0xFFAA00FF, 0x0, InfoW + 10, 0, 3, gInterface.TextMapTime.c_str()); //Buoc Vao
		}
		else
		{
			TextDraw(FontTextTime, InfoX, InfoY + 5, 0xFFAA00FF, 0x0, InfoW + 10, 0, 3, "Timer Countdown"); //Buoc Vao
		}
		DeleteObject(FontTextTime);
		BRenderTimeCountDown(InfoX, (InfoY + InfoH) - 7, InfoW + 10, gInterface.MapTime - GetTimeGMT());
	}
}


void Interface::DrawInfoCommand()
{
	float MainWidth = 240.0;
	float MainHeight = 175.0;
	float StartBody = ((MAX_WIN_HEIGHT - 51) / 2) - (MainHeight / 2);
	float StartX = MAX_WIN_WIDTH-(MainWidth + 42 +20);
	float MainCenter = StartX + (MainWidth / 3);
	float ButtonX = MainCenter - (float)(29.0 / 2);
	// ----
	DWORD Color = eGray100;

	g_pBCustomMenuInfo->DrawInfoBox(StartX, StartBody + 82, MainWidth + 42, MainHeight, 0x00000096, 0);

	if ((gCustomCommandInfo.Page == 0 && gCustomCommandInfo.Count > 25) || (gCustomCommandInfo.Page == 1 && gCustomCommandInfo.Count > 50) || (gCustomCommandInfo.Page == 2 && gCustomCommandInfo.Count > 75))
	{
		//gInterface.DrawGUI(eNextCommand, StartX + MainWidth - 8, StartBody + 80);
	}


	if (gCustomCommandInfo.Page == 1 || gCustomCommandInfo.Page == 2 || gCustomCommandInfo.Page == 3)
	{
		//gInterface.DrawGUI(ePrevCommand, StartX - 20, StartBody + 80);
	}


	for (int i = (25 * gCustomCommandInfo.Page) + 0; i < (25 * gCustomCommandInfo.Page) + 25; i++)
	{
		if (gCustomCommandInfo.m_CustomCommandInfo[i].Index == -1)
		{
			continue;
		}

		if (gCustomCommandInfo.m_CustomCommandInfo[i].Color == 1)
		{
			Color = eGold;
		}
		else if (gCustomCommandInfo.m_CustomCommandInfo[i].Color == 2)
		{
			Color = eWhite;
		}
		else if (gCustomCommandInfo.m_CustomCommandInfo[i].Color == 3)
		{
			Color = eOrange;
		}
		else if (gCustomCommandInfo.m_CustomCommandInfo[i].Color == 4)
		{
			Color = eRed2;
		}
		else if (gCustomCommandInfo.m_CustomCommandInfo[i].Color == 5)
		{
			Color = eExcellent;
		}
		else if (gCustomCommandInfo.m_CustomCommandInfo[i].Color == 6)
		{
			Color = eYellow;
		}
		else if (gCustomCommandInfo.m_CustomCommandInfo[i].Color == 7)
		{
			Color = eSocket;
		}
		else
		{
			Color = eGold;
		}

		int x = i;

		if (gCustomCommandInfo.Page > 0)
		{
			x = x - (25 * gCustomCommandInfo.Page);
		}

		gInterface.DrawFormat(Color, (int)StartX + 18, (int)StartBody + 92 + (x * 10), 100, 1, gCustomCommandInfo.m_CustomCommandInfo[i].Command);
		gInterface.DrawFormat(Color, (int)StartX + 130, (int)StartBody + 92 + (x * 10), 180, 1, gCustomCommandInfo.m_CustomCommandInfo[i].infoCommand);

	}
}

void* BCacheInfoItemDD;
DWORD CacheTimeShowInfoItem = 0;
bool Interface::ChangeSSendItem(ITEM* ItemSell, int Slot, bool KeyClick)
{
	if (KeyClick && gInterface.Data[eWindowDoiItem].OnShow)// Send Item
	{
		if (ItemSell->Type < 0) { return 0; }

		PMSG_ITEM_MOVE_RECV pMsg = { 0 };

		pMsg.h.set(0xD3, 0x6B, sizeof(pMsg));
		pMsg.Target = -1;
		pMsg.sFlag = 0;
		pMsg.tFlag = 0;
		pMsg.Target = 0;
		pMsg.Source = Slot;
		::PlayBuffer(SOUND_GET_ITEM01);
		DataSend((BYTE*)& pMsg, pMsg.h.size);
		return 1;
	}

	return 0;
}
void sfasfDrawInfoItemSet()
{

	if (BCacheInfoItemDD && CacheTimeShowInfoItem > GetTickCount())
	{
		RenderItemInfo(MouseX + 75, MouseY, (ITEM*)BCacheInfoItemDD, 0, 0, false, false);
	}
	else if (BCacheInfoItemDD)
	{
		BCacheInfoItemDD = nullptr;
	}


}
void BackDoiItem(int Type)
{
	if ((GetTickCount() - gInterface.Data[eWindowDoiItem].EventTick) > 300)
	{

		XULY_CGPACKET pMsg;
		pMsg.header.set(0xD3, 0x6C, sizeof(pMsg));
		pMsg.ThaoTac = Type; //
		DataSend((LPBYTE)& pMsg, pMsg.header.size);
		gInterface.Data[eWindowDoiItem].EventTick = GetTickCount();
	}
}
int DoiItem = -1;
void Interface::DrawDoiItem()
{
	if (!gInterface.Data[eWindowDoiItem].OnShow)
	{
		if (gInterface.SetStateDoiItem == 1)
		{
			XULY_CGPACKET pMsg;
			pMsg.header.set(0xD3, 0x6A, sizeof(pMsg));
			pMsg.ThaoTac = 111; //
			DataSend((LPBYTE)& pMsg, pMsg.header.size);
			gInterface.SetStateDoiItem = false;
			gInterface.SAU_EffectsMix = false;
			gInterface.SAU_ActiveMix = false;
			gInterface.RateSAU = false;
			DoiItem = -1;
		}
		return;
	}
	if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY))
	{
		g_pNewUISystem->Show(SEASON3B::INTERFACE_INVENTORY);
	}
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHARACTER);
	}

	float StartX = 10;
	float StartY = 20;
	float WindowW = 350;
	float WindowH = 400;
	int CheckItemOK = 0;

	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowDoiItem, gOther.Text_ChangeItem[0]);



		float BoxItemWH = 35;
		//===Item Chinh
		float PosXGroup1 = StartX + 20;
		float PosYGroup1 = StartY + 40;
		g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 - 10, PosYGroup1 - 5, WindowW - 30, 240, 0x00000096, 0, 0);

		TextDraw((HFONT)g_hFontBold, PosXGroup1, PosYGroup1 + 5, 0xEBA000FF, 0x0, 0, 0, 1, gOther.Text_ChangeItem[1]);//
		//===Box Item Chinh
		g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2), PosYGroup1 + 20, BoxItemWH, BoxItemWH, 0x00000096, 0, 0); // Chinh

		ITEM* CTItemChinh = g_pNewItemMng->CreateItem(gInterface.SAU_ItemChinh);
		if (CTItemChinh->Type != 0x1FFF)
		{
			g_pNewUISystem->RenderItem3DFree((PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2) + 7), (PosYGroup1 + 20) + 7, BoxItemWH, BoxItemWH, CTItemChinh->Type, CTItemChinh->Level, CTItemChinh->Option1, CTItemChinh->ExtOption, 0, 1.4, FALSE);//BMD MOdel
			if (SEASON3B::CheckMouseIn(PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2), PosYGroup1 + 20, 50, 50) == 1)
			{
				BCacheInfoItemDD = CTItemChinh;
				CacheTimeShowInfoItem = GetTickCount() + 50;
				if (GetKeyState(VK_RBUTTON) & 0x8000)
				{
					BackDoiItem(0);
				}
			}
		}
		//pDrawColorButton(71520, PosXGroup1, PosYGroup1 + 75, (WindowW - 40), 1, 0, 0, pMakeColor(158, 158, 158, 130));//-- Divisor

		//===Item Phu
		float PosYGroup2 = PosYGroup1 + 85;
		TextDraw((HFONT)g_hFontBold, PosXGroup1, PosYGroup2, 0xEBA000FF, 0x0, 0, 0, 1, gOther.Text_ChangeItem[2]);//
		//===Box Item Phu
		if (DoiItem == 1)
		{
			g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 + 50, PosYGroup2 + 20, BoxItemWH, BoxItemWH, 0x26E0A596, 0, 0); //Phu 1
		}
		g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 + 50, PosYGroup2 + 20, BoxItemWH, BoxItemWH, 0x00000096, 0, 0); //Phu 1
		//====
		//ObjectItem* CTItemPhu1 = &*(ObjectItem*)(BPConvertByteItem(*(DWORD*)(GetInstance() + 36), gInterface.SAU_ItemKetQua[0]));
		ITEM* CTItemPhu1 = g_pNewItemMng->CreateItem(gInterface.SAU_ItemKetQua[0]);
		if (CTItemPhu1->Type != 0x1FFF)
		{

			g_pNewUISystem->RenderItem3DFree((PosXGroup1 + 50) + 7, (PosYGroup2 + 20) + 7, BoxItemWH, BoxItemWH, CTItemPhu1->Type, CTItemPhu1->Level, CTItemPhu1->Option1, CTItemPhu1->ExtOption, 0, 1.4, FALSE);//BMD MOdel

			if (SEASON3B::CheckMouseIn((PosXGroup1 + 50), PosYGroup2 + 20, 50, 50) == 1)
			{
				BCacheInfoItemDD = CTItemPhu1;
				CacheTimeShowInfoItem = GetTickCount() + 50;
				if (GetKeyState(VK_LBUTTON) & 0x8000)
				{
					DoiItem = 1;
					//BackDoiItem(1);
				}
			}
		}
		//====
		if (DoiItem == 2)
		{
			g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2), PosYGroup2 + 20, BoxItemWH, BoxItemWH, 0x26E0A596, 0, 0); //Phu 2
		}
		g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2), PosYGroup2 + 20, BoxItemWH, BoxItemWH, 0x00000096, 0, 0); //Phu 2
		//====
		//ObjectItem * CTItemPhu2 = &*(ObjectItem*)(BPConvertByteItem(*(DWORD*)(GetInstance() + 36), gInterface.SAU_ItemKetQua[1]));
		ITEM* CTItemPhu2 = g_pNewItemMng->CreateItem(gInterface.SAU_ItemKetQua[1]);
		if (CTItemPhu2->Type != 0x1FFF)
		{

			g_pNewUISystem->RenderItem3DFree((PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2)) + 7, (PosYGroup2 + 20) + 7, BoxItemWH, BoxItemWH, CTItemPhu2->Type, CTItemPhu2->Level, CTItemPhu2->Option1, CTItemPhu2->ExtOption, 0, 1.4, FALSE);//BMD MOdel
			if (SEASON3B::CheckMouseIn((PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2)), PosYGroup2 + 20, 50, 50) == 1)
			{
				BCacheInfoItemDD = CTItemPhu2;
				CacheTimeShowInfoItem = GetTickCount() + 50;
				if (GetKeyState(VK_LBUTTON) & 0x8000)
				{
					DoiItem = 2;
					//BackDoiItem(2);
				}
			}
		}
		//====
		if (DoiItem == 3)
		{
			g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 + ((WindowW - 40) / 2) + 70, PosYGroup2 + 20, BoxItemWH, BoxItemWH, 0x26E0A596, 0, 0); //Phu 3
		}
		g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 + ((WindowW - 40) / 2) + 70, PosYGroup2 + 20, BoxItemWH, BoxItemWH, 0x00000096, 0, 0); //Phu 3
		//====
		//ObjectItem * CTItemPhu3 = &*(ObjectItem*)(BPConvertByteItem(*(DWORD*)(GetInstance() + 36), gInterface.SAU_ItemKetQua[2]));
		ITEM* CTItemPhu3 = g_pNewItemMng->CreateItem(gInterface.SAU_ItemKetQua[2]);
		if (CTItemPhu3->Type != 0x1FFF)
		{

			g_pNewUISystem->RenderItem3DFree((PosXGroup1 + ((WindowW - 40) / 2) + 70) + 5, (PosYGroup2 + 20) + 7, BoxItemWH, BoxItemWH, CTItemPhu3->Type, CTItemPhu3->Level, CTItemPhu3->Option1, CTItemPhu3->ExtOption, 0, 1.4,FALSE);//BMD MOdel
			if (SEASON3B::CheckMouseIn(PosXGroup1 + ((WindowW - 40) / 2) + 70, PosYGroup2 + 20, 50, 50) == 1)
			{
				BCacheInfoItemDD = CTItemPhu3;
				CacheTimeShowInfoItem = GetTickCount() + 50;
				if (GetKeyState(VK_LBUTTON) & 0x8000)
				{
					DoiItem = 3;
					//BackDoiItem(3);
				}
			}
		}
		//====

		//pDrawColorButton(71520, PosXGroup1, PosYGroup2 + 75, (WindowW - 40), 1, 0, 0, pMakeColor(158, 158, 158, 130));//-- Divisor	

		//===Item Ket Qua
		float PosYGroup3 = PosYGroup2 + 85;
		//TextDraw((HFONT)g_hFontBold, PosXGroup1, PosYGroup3, 0xEBA000FF, 0x0, 0, 0, 1, gOther.Text_Event[43]);//
		//===Box Item KetQua
	//	g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2), PosYGroup3 + 20, BoxItemWH, BoxItemWH, 0x00000096, 0, 0); //
		//====

		if (DoiItem == 4)
		{
			g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 + 50, PosYGroup3 + 20, BoxItemWH, BoxItemWH, 0x26E0A596, 0, 0); //Phu 4
		}
		g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 + 50, PosYGroup3 + 20, BoxItemWH, BoxItemWH, 0x00000096, 0, 0); //Phu 4

		//ObjectItem* CTItemKQ1 = &*(ObjectItem*)(BPConvertByteItem(*(DWORD*)(GetInstance() + 36), gInterface.SAU_ItemKetQua[3]));
		ITEM* CTItemKQ1 = g_pNewItemMng->CreateItem(gInterface.SAU_ItemKetQua[3]);
		if (CTItemKQ1->Type != 0x1FFF)
		{

			g_pNewUISystem->RenderItem3DFree((PosXGroup1 + 50) + 7, (PosYGroup3 + 20) + 7, BoxItemWH, BoxItemWH, CTItemKQ1->Type, CTItemKQ1->Level, CTItemKQ1->Option1, CTItemKQ1->ExtOption, 0, 1.4,FALSE);//BMD MOdel
			if (SEASON3B::CheckMouseIn((PosXGroup1 + 50), PosYGroup3 + 20, 50, 50) == 1)
			{
				BCacheInfoItemDD = CTItemKQ1;
				CacheTimeShowInfoItem = GetTickCount() + 50;
				if (GetKeyState(VK_LBUTTON) & 0x8000)
				{
					DoiItem = 4;
					//BackDoiItem(4);
				}
			}
		}
		//====
		if (DoiItem == 5)
		{
			g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2), PosYGroup3 + 20, BoxItemWH, BoxItemWH, 0x26E0A596, 0, 0); //Phu 5
		}
		g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2), PosYGroup3 + 20, BoxItemWH, BoxItemWH, 0x00000096, 0, 0); //Phu 5
		//====
		//ObjectItem * CTItemKQ2 = &*(ObjectItem*)(BPConvertByteItem(*(DWORD*)(GetInstance() + 36), gInterface.SAU_ItemKetQua[4]));
		ITEM* CTItemKQ2 = g_pNewItemMng->CreateItem(gInterface.SAU_ItemKetQua[4]);
		if (CTItemPhu2->Type != 0x1FFF)
		{

			g_pNewUISystem->RenderItem3DFree((PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2)) + 7, (PosYGroup3 + 20) + 7, BoxItemWH, BoxItemWH, CTItemKQ2->Type, CTItemKQ2->Level, CTItemKQ2->Option1, CTItemKQ2->ExtOption, 0, 1.4,FALSE);//BMD MOdel
			if (SEASON3B::CheckMouseIn((PosXGroup1 + ((WindowW - 40) / 2) - (BoxItemWH / 2)), PosYGroup3 + 20, 50, 50) == 1)
			{
				BCacheInfoItemDD = CTItemKQ2;
				CacheTimeShowInfoItem = GetTickCount() + 50;
				if (GetKeyState(VK_LBUTTON) & 0x8000)
				{
					DoiItem = 5;
					//BackDoiItem(5);
				}
			}
		}
		//====
		if (DoiItem == 6)
		{
			g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 + ((WindowW - 40) / 2) + 70, PosYGroup3 + 20, BoxItemWH, BoxItemWH, 0x26E0A596, 0, 0); //Phu 6
		}

		g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 + ((WindowW - 40) / 2) + 70, PosYGroup3 + 20, BoxItemWH, BoxItemWH, 0x00000096, 0, 0); //Phu 6
		//====
		//ObjectItem * CTItemKQ3 = &*(ObjectItem*)(BPConvertByteItem(*(DWORD*)(GetInstance() + 36), gInterface.SAU_ItemKetQua[5]));
		ITEM* CTItemKQ3 = g_pNewItemMng->CreateItem(gInterface.SAU_ItemKetQua[5]);
		if (CTItemKQ3->Type != 0x1FFF)
		{

			g_pNewUISystem->RenderItem3DFree((PosXGroup1 + ((WindowW - 40) / 2) + 70) + 5, (PosYGroup3 + 20) + 7, BoxItemWH, BoxItemWH, CTItemKQ3->Type, CTItemKQ3->Level, CTItemKQ3->Option1, CTItemKQ3->ExtOption, 0, 1.4, FALSE);//BMD MOdel
			if (SEASON3B::CheckMouseIn(PosXGroup1 + ((WindowW - 40) / 2) + 70, PosYGroup3 + 20, 50, 50) == 1)
			{
				BCacheInfoItemDD = CTItemKQ3;
				CacheTimeShowInfoItem = GetTickCount() + 50;
				if (GetKeyState(VK_LBUTTON) & 0x8000)
				{
					DoiItem = 6;
					//BackDoiItem(6);
				}
			}
		}
		//====




		//====
		//pDrawColorButton(71520, PosXGroup1, PosYGroup3 + 75, (WindowW - 40), 1, 0, 0, pMakeColor(158, 158, 158, 130));//-- Divisor
		g_pBCustomMenuInfo->DrawInfoBox(PosXGroup1 - 10, PosYGroup3 + 80, WindowW - 30, 90, 0x00000096, 0, 0);
		//===Info Text
		float PosYTextInfo = PosYGroup3 + 85;
		int CountText = 0;
		TextDraw((HFONT)g_hFontBold, PosXGroup1, PosYTextInfo + (10 * CountText++), 0xEBA000FF, 0x0, 0, 0, 1, gOther.Text_ChangeItem[3]);//

		TextDraw((HFONT)g_hFontBold, PosXGroup1, PosYTextInfo + (20 * CountText++), 0xEBA000FF, 0x0, 0, 0, 1, gOther.Text_ChangeItem[4]);//

		TextDraw((HFONT)g_hFontBold, PosXGroup1, PosYTextInfo + (20 * CountText++), 0xEBA000FF, 0x0, 0, 0, 1, "WCoin");//

		TextDraw((HFONT)g_hFontBold, PosXGroup1 + 110, PosYTextInfo + (20 * 2), 0x62FF00FF, 0x0, 0, 0, 1, "%s", gInterface.NumberFormat(gInterface.RateSAU));//
		float ButtonW = 115;
		float ButtonH = 25;
		float ButtonX = PosXGroup1 + ((WindowW - 40) / 2) - (ButtonW / 2);
		float ButtonY = (StartY + WindowH) - 20 - ButtonH;
		//if (gElemental.gDrawButton(ButtonX, ButtonY, ButtonW, 12, "Dung Luyện")) //Dung Luyen

		if (g_pBCustomMenuInfo->DrawButton(ButtonX, ButtonY, ButtonW, 12, gOther.Text_ChangeItem[5]))
		{
			PlayBuffer(25, 0, 0);
			BackDoiItem(DoiItem);
			DoiItem = -1;
			gInterface.Data[eWindowDoiItem].OnShow ^= 1;
		}
		sfasfDrawInfoItemSet();
}

void Interface::DrawNoticePKSystem()
{
	if (gInterface.CheckWindow(ObjWindow::CashShop)
		|| gInterface.CheckWindow(ObjWindow::FriendList)
		|| gInterface.CheckWindow(ObjWindow::MoveList)
		|| gInterface.CheckWindow(ObjWindow::Party)
		|| gInterface.CheckWindow(ObjWindow::Quest)
		|| gInterface.CheckWindow(ObjWindow::NPC_Devin)
		|| gInterface.CheckWindow(ObjWindow::Guild)
		|| gInterface.CheckWindow(ObjWindow::Trade)
		|| gInterface.CheckWindow(ObjWindow::Warehouse)
		|| gInterface.CheckWindow(ObjWindow::ChaosBox)
		|| gInterface.CheckWindow(ObjWindow::CommandWindow)
		|| gInterface.CheckWindow(ObjWindow::PetInfo)
		|| gInterface.CheckWindow(ObjWindow::Shop)
		|| gInterface.CheckWindow(ObjWindow::Inventory)
		|| gInterface.CheckWindow(ObjWindow::Store)
		|| gInterface.CheckWindow(ObjWindow::OtherStore)
		|| gInterface.CheckWindow(ObjWindow::Character)
		|| gInterface.CheckWindow(ObjWindow::DevilSquare)
		|| gInterface.CheckWindow(ObjWindow::BloodCastle)
		|| gInterface.CheckWindow(ObjWindow::CreateGuild)
		|| gInterface.CheckWindow(ObjWindow::GuardNPC)
		|| gInterface.CheckWindow(ObjWindow::SeniorNPC)
		|| gInterface.CheckWindow(ObjWindow::GuardNPC2)
		|| gInterface.CheckWindow(ObjWindow::CastleGateSwitch)
		|| gInterface.CheckWindow(ObjWindow::CatapultNPC)
		|| gInterface.CheckWindow(ObjWindow::CrywolfGate)
		|| gInterface.CheckWindow(ObjWindow::IllusionTemple)
		|| gInterface.CheckWindow(ObjWindow::HeroList)
		|| gInterface.CheckWindow(ObjWindow::ChatWindow)
		|| gInterface.CheckWindow(ObjWindow::FastMenu)
		|| gInterface.CheckWindow(ObjWindow::Options)
		|| gInterface.CheckWindow(ObjWindow::Help)
		|| gInterface.CheckWindow(ObjWindow::FastDial)
		|| gInterface.CheckWindow(ObjWindow::SkillTree)
		|| gInterface.CheckWindow(ObjWindow::GoldenArcher1)
		|| gInterface.CheckWindow(ObjWindow::GoldenArcher2)
		|| gInterface.CheckWindow(ObjWindow::LuckyCoin1)
		|| gInterface.CheckWindow(ObjWindow::LuckyCoin2)
		|| gInterface.CheckWindow(ObjWindow::NPC_Duel)
		|| gInterface.CheckWindow(ObjWindow::NPC_Titus)
		|| gInterface.CheckWindow(ObjWindow::CashShop)
		|| gInterface.CheckWindow(ObjWindow::Lugard)
		|| gInterface.CheckWindow(ObjWindow::QuestList1)
		|| gInterface.CheckWindow(ObjWindow::QuestList2)
		|| gInterface.CheckWindow(ObjWindow::Jerint)
		|| gInterface.CheckWindow(ObjWindow::FullMap)
		|| gInterface.CheckWindow(ObjWindow::NPC_Dialog)
		|| gInterface.CheckWindow(ObjWindow::GensInfo)
		|| gInterface.CheckWindow(ObjWindow::NPC_Julia)
		|| gInterface.CheckWindow(ObjWindow::NPC_ChaosMix)
		|| gInterface.CheckWindow(ObjWindow::ExpandInventory)
		|| gInterface.CheckWindow(ObjWindow::ExpandWarehouse)
		|| gInterface.CheckWindow(ObjWindow::MuHelper))
	{
		return;
	}

	if (this->m_DataNoticePK.empty()) return;

	float dodaitext = 230;
	for (int i = 0; i < this->m_DataNoticePK.size(); i++)
	{
		float StartXText = MAX_WIN_WIDTH - dodaitext + this->m_DataNoticePK[i].Delay;
		EnableAlphaTest(true);
		glColor3f(1.0, 1.0, 1.0);
		TextDraw((HFONT)g_hFontBold, StartXText, PartyNumber * 25 + 30 + 15 * i, this->m_DataNoticePK[i].Color, 0x00000000A5, 0, 0, 3, this->m_DataNoticePK[i].Mess);
		//
		DisableAlphaBlend();
		EnableAlphaTest(0);

		if (this->m_DataNoticePK[i].Time <= GetTickCount())
		{
			if (StartXText > MAX_WIN_WIDTH)
			{
				this->m_DataNoticePK.erase(this->m_DataNoticePK.begin() + i);
			}
			else
			{
				this->m_DataNoticePK[i].Delay += 5;
			}
		}
	}
}
int Interface::RenderHPBarNew(CHARACTER* c)
{
	if (gProtect.m_MainInfo.OnHealBar != 2)  return 0;

	OBJECT* o = &c->Object;
	if (!c || c == Hero || !o->Live || c->InfoHealBar.Life < 1) return 0;

	float StartX = DisplayWinMid;
	float StartY = 30;
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	float WFame = 250;
	float HFame = 14;
	//199,15
	RenderImage(gInterface.Data[IMG_HPBarFrame].ModelID, StartX-(WFame/2), StartY, WFame, HFame,0.0,0.0,0.778,0.9);
	//169,10
	float WFameV = 211;
	float WFameVRate = WFameV / 100;
	float HFameV = 5;
	RenderImage(gInterface.Data[IMG_HPBarValue].ModelID, StartX-(WFameV/2), StartY+4, WFameVRate * c->InfoHealBar.Life, HFameV,0.0,0.0,0.6,0.6);

	char    TextLife[100];
	char    TextLV[100];
	char    SD[100];
	char    Name[100];
	//sprintf(Name, "%s [Lv %d]", c->ID, c->InfoHealBar.Level);
	sprintf(Name, "%s", c->ID);
	sprintf(TextLife, "%d%%", c->InfoHealBar.Life);
	sprintf(TextLV, "Lv %d", c->InfoHealBar.Level);
	sprintf(SD, "%d%%", c->InfoHealBar.SD);


	g_pRenderText->SetTextColor(255, 230, 210, 255);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetShadowText(2);
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->RenderText(StartX - (WFame / 2), StartY-10, Name, WFame, 0, RT3_SORT_CENTER);
	g_pRenderText->SetFont(g_hFontMini);
	g_pRenderText->RenderText(StartX - (WFame / 2), StartY+3, TextLife, WFame, 0, RT3_SORT_CENTER);

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetShadowText(0); //clear shadow
	DisableAlphaBlend();

	return 1;
}
