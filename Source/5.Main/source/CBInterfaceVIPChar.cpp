#include "StdAfx.h"
#include "CBInterfaceVIPChar.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"
#include "ZzzTexture.h"
#include "MemScript.h"


#if(CB_VIP_CHAR)
CBInterfaceVIPChar* gBInterfaceVIPChar = nullptr;

#ifdef __ANDROID__
#define ANDROID_VIP_STAGE(msg) do { OutputDebugStringA("UI_VIP: " msg); g_ErrorReport.Write("[UI_VIP] " msg "\r\n"); } while(0)
#else
#define ANDROID_VIP_STAGE(msg) ((void)0)
#endif


CBInterfaceVIPChar::CBInterfaceVIPChar()
{
	ANDROID_VIP_STAGE("ctor begin");
	this->m_VIPCharInfoList.clear();
	FillMemory(&this->m_VIPCharInfo, sizeof(this->m_VIPCharInfo), 0);
	this->LoadData();
	ANDROID_VIP_STAGE("ctor end");
}


CBInterfaceVIPChar::~CBInterfaceVIPChar()
{
}

void CBInterfaceVIPChar::ErrorMessageBox(char* message, ...) // OK
{
	char buff[256];

	memset(buff, 0, sizeof(buff));

	va_list arg;
	va_start(arg, message);
	vsnprintf(buff, sizeof(buff), (message != NULL) ? message : "", arg);
	va_end(arg);

#ifdef __ANDROID__
	OutputDebugStringA(buff);
	g_ErrorReport.Write("[UI_VIP_ERROR] %s\r\n", buff);
	return;
#else
	MessageBox(0, buff, "Error", MB_OK | MB_ICONERROR);

	ExitProcess(0);
#endif
}
void CBInterfaceVIPChar::LoadData()
{
	ANDROID_VIP_STAGE("load begin");
	this->m_VipCharRank.clear();
	char* path = "Data\\Custom\\VIPCharRank.txt";

#ifdef __ANDROID__
	FILE* file = fopen("Data/Custom/VIPCharRank.txt", "rb");

	if (file == NULL)
	{
		CBInterfaceVIPChar::ErrorMessageBox("Open VIPCharRank failed: %s", "Data/Custom/VIPCharRank.txt");
		return;
	}

	bool sectionReady = false;
	char line[1024];

	while (fgets(line, sizeof(line), file) != NULL)
	{
		char* cursor = line;

		if ((unsigned char)cursor[0] == 0xEF &&
			(unsigned char)cursor[1] == 0xBB &&
			(unsigned char)cursor[2] == 0xBF)
		{
			cursor += 3;
		}

		while (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n')
		{
			cursor++;
		}

		if (cursor[0] == 0)
		{
			continue;
		}

		if (cursor[0] == '/' && cursor[1] == '/')
		{
			continue;
		}

		char* lineEnd = cursor + strlen(cursor);

		while (lineEnd > cursor &&
			(lineEnd[-1] == '\r' || lineEnd[-1] == '\n' || lineEnd[-1] == ' ' || lineEnd[-1] == '\t'))
		{
			*--lineEnd = 0;
		}

		if (sectionReady == false)
		{
			if (strcmp(cursor, "2") == 0)
			{
				sectionReady = true;
			}

			continue;
		}

		if (_stricmp(cursor, "end") == 0)
		{
			break;
		}

		CUSTOMVIPCHAR_DATA infoVIPChar;
		char textDir[256] = { 0 };
		char tempHex[32] = { 0 };
		unsigned int colorValue = 0;
		float sizeW = 0.0f;
		float height = 0.0f;
		float posX = 0.0f;
		float posY = 0.0f;

		memset(&infoVIPChar, 0, sizeof(infoVIPChar));

		if (sscanf(cursor, "%d %d \"%255[^\"]\" \"%31[^\"]\" %f %f %f %f",
			&infoVIPChar.Index,
			&infoVIPChar.TypeRank,
			textDir,
			tempHex,
			&sizeW,
			&height,
			&posX,
			&posY) != 8)
		{
			continue;
		}

		strncpy_s(infoVIPChar.TextDir, textDir, sizeof(infoVIPChar.TextDir));
		sscanf(tempHex, "%X", &colorValue);
		infoVIPChar.ColorText = colorValue;
		infoVIPChar.SizeW = sizeW;
		infoVIPChar.H = height;
		infoVIPChar.X = posX;
		infoVIPChar.Y = posY;
		infoVIPChar.CacheRank = 0;
		this->m_VipCharRank.insert(std::pair<int, CUSTOMVIPCHAR_DATA>(infoVIPChar.Index, infoVIPChar));

		if (infoVIPChar.TypeRank == 2)
		{
			LoadBitmap(infoVIPChar.TextDir, 150000 + infoVIPChar.Index, 0x2601, 0x2900, 1, 0);
		}
	}

	fclose(file);
	ANDROID_VIP_STAGE("load done");
	return;
#endif

	CMemScript* lpMemScript = new CMemScript;


	if (lpMemScript == 0)
	{
		CBInterfaceVIPChar::ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR, path);
		return;
	}

	if (lpMemScript->SetBuffer(path) == 0)
	{
		CBInterfaceVIPChar::ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}
	ANDROID_VIP_STAGE("setbuffer ok");
	try
	{
		while (true)
		{
			if (lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while (true)
			{
				if (section == 2)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}
					CUSTOMVIPCHAR_DATA infoVIPChar;
					memset(&infoVIPChar, 0, sizeof(infoVIPChar));
					infoVIPChar.Index = lpMemScript->GetNumber();
					infoVIPChar.TypeRank = lpMemScript->GetAsNumber();
					strncpy_s(infoVIPChar.TextDir, lpMemScript->GetAsString(), sizeof(infoVIPChar.TextDir));
					//==ColorDWord
					char TempHex[11];
					strcpy_s(TempHex, lpMemScript->GetAsString());
					sscanf_s(TempHex, "%X", &infoVIPChar.ColorText);
					infoVIPChar.SizeW = lpMemScript->GetAsFloatNumber();
					infoVIPChar.H = lpMemScript->GetAsFloatNumber();
					infoVIPChar.X = lpMemScript->GetAsFloatNumber();
					infoVIPChar.Y = lpMemScript->GetAsFloatNumber();
					infoVIPChar.CacheRank = 0;
					this->m_VipCharRank.insert(std::pair<int, CUSTOMVIPCHAR_DATA>(infoVIPChar.Index, infoVIPChar));
					if (infoVIPChar.TypeRank == 2)
					{
						char path[MAX_PATH] = { 0 };

						wsprintf(path, "%s", infoVIPChar.TextDir);
						LoadBitmap(path, 150000 + infoVIPChar.Index, 0x2601, 0x2900, 1, 0);
					}
				}
			}
		}
	}
	catch (...)
	{
		CBInterfaceVIPChar::ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
	ANDROID_VIP_STAGE("load done");


}
char* VipOptionText[6] =
{
	gOther.Text_VIPCHAR[8],//84
	gOther.Text_VIPCHAR[9],//83
	gOther.Text_VIPCHAR[10],//91
	gOther.Text_VIPCHAR[11], //102
	gOther.Text_VIPCHAR[12], //117
	gOther.Text_VIPCHAR[13], //92
};


int ExpVipChar3 = 0;

//#include "Character.h"

int Vip = 0;
static void NutLui(LPVOID pClass)
{
	if (GetTickCount() < gInterface.Data[eRankPANEL_MAIN].EventTick + 300) return;
	gInterface.Data[eRankPANEL_MAIN].EventTick = GetTickCount();

	if (Vip > 0)
	{
		Vip--;
	}

}
static void NutTien(LPVOID pClass)
{
	if (GetTickCount() < gInterface.Data[eRankPANEL_MAIN].EventTick + 300) return;
	gInterface.Data[eRankPANEL_MAIN].EventTick = GetTickCount();

	if (Vip < gBInterfaceVIPChar->m_VipCharRank.size())
	{
		Vip++;
	}

}


void DrawWindowVipChar()
{

	if (gInterface.CheckWindow(Interface::MoveList) || gInterface.CheckWindow(Interface::CashShop) || gInterface.CheckWindow(Interface::SkillTree) || gInterface.CheckWindow(Interface::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)))
	{
		gInterface.Data[eWindowVip].OnShow = false;

		return;
	}



	if (!gInterface.Data[eWindowVip].OnShow)
	{
		ExpVipChar3 = 0;
		Vip = 0;
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


	int LevelVipChar = gBInterfaceVIPChar->m_VIPCharInfo.LevelVIP + Vip;//gCustomRankUser.m_Data[gObjUser.lpViewPlayer->aIndex].mLvVIPChar+1;

	std::map<int, CBInterfaceVIPChar::CUSTOMVIPCHAR_DATA>::iterator it = gBInterfaceVIPChar->m_VipCharRank.find(LevelVipChar);

	EnableAlphaTest(1);


	float WindowW = 220;
	float WindowH = 260;

	float StartX = 5;
	float StartY = 30;

		g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowVip, gOther.Text_VIPCHAR[0]);


		float WKhungInfo = WindowW - 28;
		g_pBCustomMenuInfo->DrawInfoBox(StartX + 10, StartY + 35, WKhungInfo, 110, 0x00000096, 0, 0);

		if (it == gBInterfaceVIPChar->m_VipCharRank.end())
		{
			TextDraw((HFONT)g_hFont, StartX, StartY + (WindowH / 2) - 7, 0xFFFFFFFF, 0xFF0000A5, WindowW, 0, 3, gOther.Text_VIPCHAR[1]);
			return;
		}


		float BarW = 165;
		float BarH = 10;
		float BarX = (StartX + (WindowW / 2)) - (BarW / 2);
		float BarY = StartY + 120;
		float RankVipY = (StartY + 30) + (70 / 2);
		const BYTE state[3] = { 0, 1, 2 };

		UIController.Button(32346, (BarX + 7) + (BarW - 10) - 20, (StartY + 75), 20, 22, 20, 22, 3, state, NutTien);
		UIController.Button(32345, (BarX + 7) - (BarW - 10) + 146, (StartY + 75), 20, 22, 20, 22, 3, state, NutLui);

		//=== Rank Text Or Img
		if (it->second.TypeRank == 1)
		{
			int SizeTextFont = it->second.SizeW;
			TextDraw((HFONT)g_hFont, BarX, RankVipY - (SizeTextFont / 2), it->second.ColorText, 0x0, BarW, 0, 3, it->second.TextDir);//
		}
		else if (it->second.TypeRank == 2)
		{
			EnableAlphaTest(true);
			glColor3f(1.0, 1.0, 1.0);
			RenderBitmap(150000 + LevelVipChar, BarX + (BarW / 2) - (it->second.SizeW / 2), RankVipY - (it->second.H / 2), it->second.SizeW, it->second.H, 0.0, 0.0, 1.0, 1.0, 1, 1, 0.0);
			glColor3f(1.0f, 1.0f, 1.0f); //Xoa mau
			EnableAlphaTest(0);
		}
		//===Draw Info Rank
		//gCentral.gDrawButtonGUI(BITMAP_INTERFACE_EX+20, (BarX + 7) + (BarW - 10), (StartY + 40), 15, 15, 1);//?
		RenderImage(BITMAP_INTERFACE_EX+20, (BarX + 7) + (BarW - 10), (StartY + 40), 15, 15);
		if (SEASON3B::CheckMouseIn((BarX + 7) + (BarW - 10), (StartY + 40), 15, 15) == 1)
		{
			float CountText = 0;
			for (int n = 0; n < 6; n++)
			{
				if (gBInterfaceVIPChar->m_VIPCharInfo.TypeOp[n] == -1) { continue; }
				//gInterface.DrawMessage(1, "%d",gBInterfaceVIPChar->m_VIPCharInfo.TypeOp[n]);
				TextDraw((HFONT)g_hFont, MouseX + 30, MouseY + (13 * CountText), 0xFFBA19FF, 0x000000A5, 0, 0, 1, VipOptionText[n], gInterface.NumberFormat(gBInterfaceVIPChar->m_VIPCharInfo.ValueOp[n]));
				CountText++;
			}

		}
		//===Exp Bar
		int ExpDangCo = gBInterfaceVIPChar->m_VIPCharInfo.UserPointVIP;
		int ExpYeuCauUp = gBInterfaceVIPChar->m_VIPCharInfo.PointVIP;
		int PhamTramExp = 0;
		if (ExpYeuCauUp != 0)
		{
			PhamTramExp = (ExpDangCo * 100) / ExpYeuCauUp;
		}
		int TyLeTGA = ((BarW - 5) * (PhamTramExp > 100 ? 100 : PhamTramExp)) / 100;
		if (TyLeTGA > (BarW - 5)) { TyLeTGA = (BarW - 5); }
		RenderBitmap(gInterface.Data[IMG_31764].ModelID, BarX, BarY, BarW, BarH, 0.0, 0.0, 0.607, 1.0, 1, 1, 0.0);
		//===Tang/Giam Process
		if (ExpVipChar3 < TyLeTGA)
		{
			ExpVipChar3++;
		}
		else if (ExpVipChar3 > TyLeTGA)
		{
			ExpVipChar3--;
		}
		RenderBitmap(gInterface.Data[IMG_31765].ModelID, BarX + 3, BarY + 3, ExpVipChar3, BarH - 6, 0.0, 0.0, 0.5, 1.0, 1, 1, 0.0);
		//=== Exp Text
		TextDraw((HFONT)g_hFontBold, BarX, BarY - 25, 0xFFFFFFFF, 0xB59A0096, BarW, 0, 3, gOther.Text_VIPCHAR[2], LevelVipChar);//
		HFONT CHFontMini = CreateFontA(12, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3u, 0, "Arial");
		TextDraw((HFONT)g_hFont, BarX, BarY - 10, 0xFF8103FF, 0x0, BarW, 0, 1, gOther.Text_VIPCHAR[3]);//
		TextDraw(CHFontMini, BarX, BarY + 2, 0xFFFFFFFF, 0x0, BarW, 0, 3, "%s/%s", gInterface.NumberFormat(ExpDangCo), gInterface.NumberFormat(ExpYeuCauUp));//
		//===
		float TextWCoin = BarW / 3;
		TextDraw((HFONT)g_hFont, BarX, BarY + 12, 0xFF8103FF, 0x0, (TextWCoin / 2), 0, 3, "WC");//
		TextDraw((HFONT)g_hFont, BarX + (TextWCoin / 2), BarY + 12, 0xFFFFFFFF, 0x0, (TextWCoin / 2), 0, 3, "%s", gInterface.NumberFormat(gBInterfaceVIPChar->m_VIPCharInfo.Coin[0]));//
		TextDraw((HFONT)g_hFont, BarX + TextWCoin, BarY + 12, 0xFF8103FF, 0x0, (TextWCoin / 2), 0, 3, "WP");//
		TextDraw((HFONT)g_hFont, BarX + TextWCoin + (TextWCoin / 2), BarY + 12, 0xFFFFFFFF, 0x0, (TextWCoin / 2), 0, 3, "%s", gInterface.NumberFormat(gBInterfaceVIPChar->m_VIPCharInfo.Coin[1]));//
		TextDraw((HFONT)g_hFont, BarX + (TextWCoin * 2), BarY + 12, 0xFF8103FF, 0x0, (TextWCoin / 2), 0, 3, "GP");//
		TextDraw((HFONT)g_hFont, BarX + (TextWCoin * 2) + (TextWCoin / 2), BarY + 12, 0xFFFFFFFF, 0x0, (TextWCoin / 2), 0, 3, "%s", gInterface.NumberFormat(gBInterfaceVIPChar->m_VIPCharInfo.Coin[2]));//

		//================
		float SizeButton = 100;
		float ButtonX = (StartX + (WindowW / 2)) - (SizeButton / 2);
		float ButtonY = (StartY + WindowH) - 40;

		TextDraw((HFONT)g_hFont, StartX, ButtonY - 55, 0xC06EFFFF, 0x000000A5, WindowW, 0, 3, gOther.Text_VIPCHAR[4]);
		//===Draw Button Show List Item Point
		DeleteObject(CHFontMini);

		if (g_pBCustomMenuInfo->DrawButton(ButtonX, ButtonY - 30, SizeButton, 12, gOther.Text_VIPCHAR[5]))
		{
			XULY_CGPACKET pMsg;
			pMsg.header.set(0xD3, 0x24, sizeof(pMsg));
			pMsg.ThaoTac = 1; //
			DataSend((LPBYTE)& pMsg, pMsg.header.size);
			//gBInterfaceVIPChar->WindowViPCharList->onShow ^= 1;
		}

		//===Draw Button Thang Cap VIP
		if (g_pBCustomMenuInfo->DrawButton(ButtonX, ButtonY, SizeButton, 12, gOther.Text_VIPCHAR[6]))
		{
			XULY_CGPACKET pMsg;
			pMsg.header.set(0xD3, 0x23, sizeof(pMsg));
			pMsg.ThaoTac = 1; //
			DataSend((LPBYTE)& pMsg, pMsg.header.size);
		}


		if (g_pBCustomMenuInfo->DrawButton(ButtonX + 120, ButtonY, 60, 12, "R", 20))
		{
			gBInterfaceVIPChar->LoadData();
		}



	
	EnableAlphaTest(false);
}

CNewUIScrollBar* WindowViPCharListScrollBar = NULL;
int VipCharListMaxPerPage = 15;
bool UpdateMaxPosVIP = false;

void DrawWindowVipCharList()
{
	if (gInterface.CheckWindow(Interface::MoveList) || gInterface.CheckWindow(Interface::CashShop) || gInterface.CheckWindow(Interface::SkillTree) || gInterface.CheckWindow(Interface::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)))
	{
		gInterface.Data[eWindowVipItem].OnShow = false;

		return;
	}

	if (!gInterface.Data[eWindowVipItem].OnShow)
	{
		if (WindowViPCharListScrollBar)
		{
			WindowViPCharListScrollBar = NULL;

		}
		return;
	}


	EnableAlphaTest(true);


	float WindowW = 220;
	float WindowH = 260;

	float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2) + 221;
	float StartY = ((MAX_WIN_HEIGHT - 51) / 2) - (WindowW / 2);

	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowVipItem, gOther.Text_VIPCHAR[5]);


		gInterface.Data[eWindowVipItem].X = gInterface.Data[eWindowVip].X + gInterface.Data[eWindowVip].Width;
		gInterface.Data[eWindowVipItem].Y = gInterface.Data[eWindowVip].Y;
		StartX = gInterface.Data[eWindowVip].X + gInterface.Data[eWindowVip].Width;
		StartY = gInterface.Data[eWindowVip].Y;

		//==RenderText
		float TextX = StartX + 20;
		float TextY = StartY + 60;
		float TextW = WindowW - 60;

		//===ScrollBar
		int DataListItem = gBInterfaceVIPChar->m_VIPCharInfoList.size();

		if (WindowViPCharListScrollBar == NULL)
		{
			WindowViPCharListScrollBar = new CNewUIScrollBar();
			//WindowViPCharListScrollBar->Create(StartX, StartY, gBInterfaceVIPChar->WindowViPCharList->Height - 80);
			WindowViPCharListScrollBar->Create(StartX, StartY, gInterface.Data[eWindowVipItem].Height - 80);
			if (WindowViPCharListScrollBar)
			{
				WindowViPCharListScrollBar->SetMaxPos((DataListItem <= VipCharListMaxPerPage) ? 0 : (DataListItem / VipCharListMaxPerPage) - 1);
				WindowViPCharListScrollBar->SetPos(StartX + (gInterface.Data[eWindowVipItem].Width - 20), StartY + 60);
			}
		}
		if (WindowViPCharListScrollBar)
		{
			if (UpdateMaxPosVIP)
			{
				WindowViPCharListScrollBar->SetMaxPos((DataListItem <= VipCharListMaxPerPage) ? 0 : (DataListItem / VipCharListMaxPerPage) - 1);
				UpdateMaxPosVIP = false;
			}
			if (gInterface.Data[eWindowVipItem].OnClick || gInterface.Data[eWindowVip].OnClick)
			{

				WindowViPCharListScrollBar->SetPos(StartX + (gInterface.Data[eWindowVipItem].Width - 20), StartY + 60);
				WindowViPCharListScrollBar->SetCurPos(0);
			}
			WindowViPCharListScrollBar->MouseWheelWindow = SEASON3B::CheckMouseIn(TextX, TextY, WindowW, WindowH);
			WindowViPCharListScrollBar->Render();
			WindowViPCharListScrollBar->UpdateMouseEvent();
			WindowViPCharListScrollBar->Update();
		}
		//===ScrollBar END
		TextDraw((HFONT)g_hFontBold, StartX, TextY - 30, 0xC06EFFFF, 0x000000A5, WindowW, 0, 3, "%s Point", gInterface.NumberFormat(gBInterfaceVIPChar->m_VIPCharInfo.UserPointVIP));//
		TextDraw((HFONT)g_hFontBold, StartX, TextY - 20, 0xC06EFFFF, 0x000000A5, WindowW, 0, 3, gOther.Text_VIPCHAR[14]);//
		int Count = 0;
		int VipCharListPage = WindowViPCharListScrollBar->GetCurPos();
		for (int n = (VipCharListPage * VipCharListMaxPerPage); n < DataListItem; n++)
		{
			if (Count >= VipCharListMaxPerPage) break;
			TextDraw((HFONT)g_hFontBold, TextX, TextY + (Count * 12), 0xFFDD00FF, 0x0, TextW, 0, 1, "%s", BGetItemName(gBInterfaceVIPChar->m_VIPCharInfoList[n].IndexItem, SET_ITEMOPT_LEVEL((gBInterfaceVIPChar->m_VIPCharInfoList[n].Level == 255) ? 0 : gBInterfaceVIPChar->m_VIPCharInfoList[n].Level)));//
			TextDraw((HFONT)g_hFontBold, TextX, TextY + (Count * 12), 0x00FFD0FF, 0x0, TextW, 0, 4, "%s Point", gInterface.NumberFormat(gBInterfaceVIPChar->m_VIPCharInfoList[n].PointVIP));//
			Count++;

		}

	EnableAlphaTest(false);
}
void CBInterfaceVIPChar::CGSendOpenWinwdowVIP()
{
	if (gInterface.Data[eWindowVip].OnShow)
	{
		gInterface.Data[eWindowVip].OnShow = 0;
	}
	else
	{
		if (GetTickCount() > gInterface.Data[eWindowVip].EventTick + 300)
		{
			XULY_CGPACKET pMsg;
			pMsg.header.set(0xD3, 0x25, sizeof(pMsg));
			pMsg.ThaoTac = 1; //
			DataSend((LPBYTE)& pMsg, pMsg.header.size);
			gInterface.Data[eWindowVip].EventTick = GetTickCount();
		}
	}
}



void CBInterfaceVIPChar::GCSetInfoVIPChar(BPMSG_CUSTOM_VIPCHAR_SEND * lpMsg)
{

	m_VIPCharInfo.UserPointVIP = lpMsg->UserPointVIP;

	if (lpMsg->TypeSend == 0 || lpMsg->TypeSend == 2)
	{
		m_VIPCharInfo.LevelVIP = lpMsg->LevelVIP;
		m_VIPCharInfo.PointVIP = lpMsg->PointVIP;
		//==Coin
		m_VIPCharInfo.Coin[0] = lpMsg->Coin[0];
		m_VIPCharInfo.Coin[1] = lpMsg->Coin[1];
		m_VIPCharInfo.Coin[2] = lpMsg->Coin[2];
	}

	for (int n = 0; n < 6; n++)
	{
		m_VIPCharInfo.TypeOp[n] = lpMsg->TypeOp[n];
		m_VIPCharInfo.ValueOp[n] = lpMsg->ValueOp[n];
	}
	if (lpMsg->TypeSend == 0)
	{
		gInterface.Data[eWindowVip].OnShow ^= 1;
	}
}

void CBInterfaceVIPChar::GCSetInfoVIPCharList(CSEV_COUNTLIST * lpMsg) // OK
{
	m_VIPCharInfoList.clear();

	for (int n = 0; n < lpMsg->Count; n++)
	{
		DATA_ITEMVIPPOINT* lpInfo = (DATA_ITEMVIPPOINT*)(((BYTE*)lpMsg) + sizeof(CSEV_COUNTLIST) + (sizeof(DATA_ITEMVIPPOINT) * n));

		m_VIPCharInfoList.push_back(*lpInfo);
	}

	gInterface.Data[eWindowVipItem].OnShow ^= 1;
}



//gBInterfaceVIPChar->Draw();
void CBInterfaceVIPChar::Draw()
{
	//=== Test Open Window
	//if (GetKeyState(VK_F4) & 0x4000 && GetTickCount() > gInterface.Data[eWindowVip].EventTick + 300)
	//{
	//	CGSendOpenWinwdowVIP(); // << Ham Goi Window
	//	gInterface.Data[eWindowVip].EventTick = GetTickCount();
	//}
	//===VIp Char 
	DrawWindowVipChar();
	DrawWindowVipCharList();
}
bool CBInterfaceVIPChar::SendItem(ITEM* ItemSell, int Slot, bool KeyClick)
{
	if (KeyClick && gInterface.Data[eWindowVip].OnShow && GetTickCount() > gInterface.Data[eWindowVipItem].EventTick)// Send Item
	{
		if (ItemSell->Type < 0) { return 0; }

		PMSG_ITEM_MOVE_RECV pMsg = { 0 };

		pMsg.h.set(0xD3, 0x22, sizeof(pMsg));
		pMsg.Target = -1;
		pMsg.sFlag = 0;
		pMsg.tFlag = 0;
		pMsg.Target = 0;
		pMsg.Source = Slot;
		::PlayBuffer(SOUND_GET_ITEM01);
		DataSend((BYTE*)& pMsg, pMsg.h.size);
		gInterface.Data[eWindowVipItem].EventTick = GetTickCount() + 1000;
		return 1;
	}

	return 0;
}
#endif
