#include "StdAfx.h"
#include "CB_AutoResetInfo.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"


#if(CB_AUTORESETINFO)
CBAutoResetInfo* gCBAutoResetInfo = nullptr;

CBAutoResetInfo::CBAutoResetInfo()
{
	this->ViewInfoResetWindow.Clear();
	//====AutoReset Info========//
	this->TypeReset = 0;
	this->GHRSReset = 0;
	this->GHRSRelife = 0;
	this->LvlReset = 0;
	this->LoadingInfo = 0;
	//==========//

	this->m_LevelRelife = 0;
	this->m_RelifeCong = 0;
	this->m_MaxRelife = 0;
	this->m_LevelSauKhiRL = 0;
	this->m_ResetSauKhiRL = 0;
	this->m_PointRelife = 0;
}


CBAutoResetInfo::~CBAutoResetInfo()
{

}

void CBAutoResetInfo::GCCharacterAutoResetRecv(BYTE* Recv) // OK
{
	PMSG_AUTORESET_INFO_RECV* lpMsg = (PMSG_AUTORESET_INFO_RECV*)Recv;
	this->m_LevelRelife = lpMsg->LevelRelife;
	this->m_RelifeCong = lpMsg->RelifeCong;
	this->m_MaxRelife = lpMsg->MaxRelife;
	this->m_LevelSauKhiRL = lpMsg->LevelSauKhiRL;
	this->m_ResetSauKhiRL = lpMsg->ResetSauKhiRL;
	this->m_PointRelife = lpMsg->PointRelife;


	if (!gCBAutoResetInfo) return;
	if (GetTickCount() < gInterface.Data[eWindowRelife].EventTick + 300) return;
	gInterface.Data[eWindowRelife].EventTick = GetTickCount();
	if (gInterface.Data[eWindowRelife].OnShow)
	{
		gInterface.Data[eWindowRelife].OnShow = 0;
		return;
	}
	gInterface.Data[eWindowRelife].OnShow = 1;
}
void CBAutoResetInfo::DrawWindowRelife()
{
	if (gInterface.CheckWindow(Interface::MoveList) || gInterface.CheckWindow(Interface::CashShop) || gInterface.CheckWindow(Interface::SkillTree) || gInterface.CheckWindow(Interface::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)) ||
		//gCBDauGiaItem.SelectItemDauGia == -1 ||
		GetKeyState(VK_ESCAPE) < 0)
	{
		gInterface.Data[eWindowRelife].OnShow = false;
		return;
	}

	if (!gInterface.Data[eWindowRelife].OnShow)
	{
		return;
	}
	float WindowW = 280;
	float WindowH = 260;
	gInterface.Data[eWindowRelife].Width = WindowW;
	float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
	float StartY = 25.0;
	//if (!gCBUtil.gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowAutoReset, "Auto Reset Info"))
	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowRelife, "Relife Info");
	DWORD Color = eGray100;
	float CuaSoW = WindowW;
	//====================================
	float TextStartY = StartY;

	int ct = 4;
	TextDraw(g_hFont, (StartX + (CuaSoW / 10)), TextStartY + (12 * ct++), 0xEBFFFBFF, 0x0, CuaSoW, 0, 1, "• Bạn đã đạt giới hạn Reset , cần Relife để tiếp tục Reset"); //
	TextDraw(g_hFont, (StartX + (CuaSoW / 10)), TextStartY + (12 * ct++), 0xEBFFFBFF, 0x0, CuaSoW, 0, 1, "• Sau khi Relife bạn sẽ nhận được %d Point", this->m_PointRelife); //
	TextDraw(g_hFont, (StartX + (CuaSoW / 10)), TextStartY + (12 * ct++), 0xEBFFFBFF, 0x0, CuaSoW, 0, 1, "• Reset về %d Level về %d !", this->m_ResetSauKhiRL, this->m_LevelSauKhiRL); //
	ct += 1;
	TextDraw(g_hFont, StartX, TextStartY + (12 * ct++), 0xFFD51CFF, 0x0, CuaSoW, 0, 3, "Yêu cầu Level %d để Relife", this->m_LevelRelife); //
	TextDraw(g_hFont, StartX, TextStartY + (12 * ct++), 0xFFD51CFF, 0x0, CuaSoW, 0, 3, "Bạn sẽ được +%d Relife cho lần tiếp theo", this->m_RelifeCong); //
	TextDraw(g_hFont, StartX, TextStartY + (12 * ct++), 0xFFD51CFF, 0x0, CuaSoW, 0, 3, "Bạn được Relife tối đa %d lần !", this->m_MaxRelife); //



	if (g_pBCustomMenuInfo->DrawButton(StartX + (WindowW / 2) - (120 / 2), StartY + (WindowH - 60), 150, 12, "Thực Hiện Relife", 120) && (GetTickCount() - gInterface.Data[eWindowRelife].EventTick) > 300)
	{
		XULY_CGPACKET pMsg;
		pMsg.header.set(0xD3, 0x36, sizeof(pMsg));
		pMsg.ThaoTac = 3; //
		DataSend((LPBYTE)& pMsg, pMsg.header.size);
	}
}
void CBAutoResetInfo::RecvProtocol(BYTE* Recv)
{
	if (!Recv) return;
	DKRS_COUNTLIST* mRecv = (DKRS_COUNTLIST*)Recv;
	gCBAutoResetInfo->ViewInfoResetWindow.Clear();
	this->LoadingInfo = false;

	gCBAutoResetInfo->ViewInfoResetWindow.Level = mRecv->Level;
	gCBAutoResetInfo->ViewInfoResetWindow.Point = mRecv->Point;
	gCBAutoResetInfo->ViewInfoResetWindow.Zen = mRecv->Zen;
	gCBAutoResetInfo->ViewInfoResetWindow.WC = mRecv->WC;
	gCBAutoResetInfo->ViewInfoResetWindow.WP = mRecv->WP;
	gCBAutoResetInfo->ViewInfoResetWindow.GP = mRecv->GP;
	gCBAutoResetInfo->ViewInfoResetWindow.ResetView = mRecv->ResetView;
	gCBAutoResetInfo->ViewInfoResetWindow.ResetDay = mRecv->ResetDay;
	gCBAutoResetInfo->ViewInfoResetWindow.MaxGHRS = mRecv->MaxGHRS;
	gCBAutoResetInfo->ViewInfoResetWindow.OnResetType = mRecv->OnResetType;
	gCBAutoResetInfo->ViewInfoResetWindow.LoaiReset = mRecv->LoaiReset;
	gCBAutoResetInfo->ViewInfoResetWindow.AutoResetEnable = mRecv->AutoResetEnable;
	if (gCBAutoResetInfo->ViewInfoResetWindow.LoaiReset == 0)gCBAutoResetInfo->ViewInfoResetWindow.Point = 0;
	gCBAutoResetInfo->ViewInfoResetWindow.TypeReset = mRecv->TypeReset;
	gCBAutoResetInfo->ViewInfoResetWindow.CodeReset = mRecv->CodeReset;
	
	//==Get point def
	for (int i = 0; i < 5; i++)
	{
		gCBAutoResetInfo->ViewInfoResetWindow.DefStat[i] = mRecv->DefStat[i];
	}
	gCBAutoResetInfo->ViewInfoResetWindow.m_ItemCheck.clear();
	for (int n = 0; n < mRecv->Count; n++)
	{
		DKRS_ITEMINFO* lpInfo = (DKRS_ITEMINFO*)(((BYTE*)Recv) + sizeof(DKRS_COUNTLIST) + (sizeof(DKRS_ITEMINFO) * n));

		gCBAutoResetInfo->ViewInfoResetWindow.m_ItemCheck.push_back(*lpInfo);
	}
	if (gCBAutoResetInfo->ViewInfoResetWindow.OnResetType > 0)
	{
		if (!gInterface.Data[eWindowAutoReset].OnShow) gInterface.Data[eWindowAutoReset].OnShow = 1;
	}
	this->LoadingInfo = 1;
	//gInterface.DrawMessage(1, "Open Window Auto Reset %d", gCBAutoResetInfo->ViewInfoResetWindow.OnResetType);
}
void CBAutoResetInfo::OpenWindow()
{
	if (!gCBAutoResetInfo) return;
	if (GetTickCount() < gInterface.Data[eWindowAutoReset].EventTick + 300) return;
	gInterface.Data[eWindowAutoReset].EventTick = GetTickCount();
	if (gInterface.Data[eWindowAutoReset].OnShow)
	{
		gInterface.Data[eWindowAutoReset].OnShow = 0;
		return;
	}

	XULY_CGPACKET pMsg;
	pMsg.header.set(0xD3, 0x36, sizeof(pMsg));
	pMsg.ThaoTac = 1; //
	DataSend((LPBYTE)& pMsg, pMsg.header.size);

}
char* CBAutoResetInfo::GetItemName(int ItemType, int Level)
{

	return BGetItemName(ItemType, Level);

}
CUITextInputBox* InputAddPointReset[5] = { nullptr };
CUITextInputBox* InputCodeReset = { nullptr };
char TextInputAddPoint[4][6] = { 0 };
char TextInputCodeReset[11] = { 0 };
char* NameTypeReset[] = { "Giữ Point", "Point*Reset", "Giữ Point + Point" };

int PointReAdd = 0;

void WindowDieuKienResetInfo(int X, int Y, int W)
{
	float StartX = X;
	float StartY = Y;
	float WindowW = W;
	StartY += 35;
	//TextDraw((HFONT)g_hFontBold, StartX + 10, StartY, 0xEB8213FF, 0x0, WindowW, 0, 1, "**Coin:");
	TextDraw((HFONT)g_hFontBold, StartX + 10, StartY, 0xEB8213FF, 0x0, WindowW, 0, 1, "- Nguyên Liệu Yêu Cầu:");
	StartY += 15;

	float XText = 25;
	float BGW = 35;
	float BGH = 12;
	float cStartX = StartX + 10;
	float kkW = BGW + XText + 10;
	int GetCountLine = 0;
	if (gCBAutoResetInfo->ViewInfoResetWindow.Zen)
	{
		gInterface.DrawBarForm(cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0), StartY + (GetCountLine > 1 ? 15 : 0), BGW + XText, BGH, 1, 0.2667, 0, 0.8);
		TextDraw((HFONT)g_hFontBold, cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0), StartY + (GetCountLine > 1 ? 15 : 0) + 1.5, 0xFFD000A9, 0x0, XText, 0, 3, "Zen");
		gInterface.DrawBarForm(cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0) + XText, StartY + (GetCountLine > 1 ? 15 : 0), BGW, BGH, 0.0, 0.0, 0.0, 0.8);
		TextDraw((HFONT)g_hFont, cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0) + XText, StartY + (GetCountLine > 1 ? 15 : 0) + 1.5, 0xFFFFFFFF, 0x0, BGW, 0, 3, "%s", gInterface.NumberFormat(gCBAutoResetInfo->ViewInfoResetWindow.Zen));
		GetCountLine++;
	}

	if (gCBAutoResetInfo->ViewInfoResetWindow.WC)
	{
		gInterface.DrawBarForm(cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0), StartY + (GetCountLine > 1 ? 15 : 0), BGW + XText, BGH, 1, 0.2667, 0, 0.8);
		TextDraw((HFONT)g_hFontBold, cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0), StartY + (GetCountLine > 1 ? 15 : 0) + 1.5, 0xFFD000A9, 0x0, XText, 0, 3, "WC");
		gInterface.DrawBarForm(cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0) + XText, StartY + (GetCountLine > 1 ? 15 : 0), BGW, BGH, 0.0, 0.0, 0.0, 0.8);
		TextDraw((HFONT)g_hFont, cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0) + XText, StartY + (GetCountLine > 1 ? 15 : 0) + 1.5, 0xFFFFFFFF, 0x0, BGW, 0, 3, "%s", gInterface.NumberFormat(gCBAutoResetInfo->ViewInfoResetWindow.WC));
		GetCountLine++;
	}
	if (gCBAutoResetInfo->ViewInfoResetWindow.WP)
	{
		gInterface.DrawBarForm(cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0), StartY + (GetCountLine > 1 ? 15 : 0), BGW + XText, BGH, 1, 0.2667, 0, 0.8);
		TextDraw((HFONT)g_hFontBold, cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0), StartY + (GetCountLine > 1 ? 15 : 0) + 1.5, 0xFFD000A9, 0x0, XText, 0, 3, "WP");
		gInterface.DrawBarForm(cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0) + XText, StartY + (GetCountLine > 1 ? 15 : 0), BGW, BGH, 0.0, 0.0, 0.0, 0.8);
		TextDraw((HFONT)g_hFont, cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0) + XText, StartY + (GetCountLine > 1 ? 15 : 0) + 1.5, 0xFFFFFFFF, 0x0, BGW, 0, 3, "%s", gInterface.NumberFormat(gCBAutoResetInfo->ViewInfoResetWindow.WP));
		GetCountLine++;
	}
	if (gCBAutoResetInfo->ViewInfoResetWindow.GP)
	{
		gInterface.DrawBarForm(cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0), StartY + (GetCountLine > 1 ? 15 : 0), BGW + XText, BGH, 1, 0.2667, 0, 0.8);
		TextDraw((HFONT)g_hFontBold, cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0), StartY + (GetCountLine > 1 ? 15 : 0) + 1.5, 0xFFD000A9, 0x0, XText, 0, 3, "GP");
		gInterface.DrawBarForm(cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0) + XText, StartY + (GetCountLine > 1 ? 15 : 0), BGW, BGH, 0.0, 0.0, 0.0, 0.8);
		TextDraw((HFONT)g_hFont, cStartX + ((GetCountLine == 1 || GetCountLine == 3) ? kkW : 0) + XText, StartY + (GetCountLine > 1 ? 15 : 0) + 1.5, 0xFFFFFFFF, 0x0, BGW, 0, 3, "%s", gInterface.NumberFormat(gCBAutoResetInfo->ViewInfoResetWindow.GP));
		GetCountLine++;
	}
	if (!gCBAutoResetInfo->ViewInfoResetWindow.m_ItemCheck.empty())
	{
		StartY += 35;

		TextDraw((HFONT)g_hFontBold, StartX + 10, StartY, 0xEB8213FF, 0x0, WindowW, 0, 1, "**Vật Phẩm:");
		//===Draw List
		StartY += 15;
		float InfoListX = StartX + 10;
		float InfoListY = StartY;
		float InfoListW = W - 20;
		int Count = 0;
		DWORD ColorSelect = 0x0;
		for (int n = 0; n < gCBAutoResetInfo->ViewInfoResetWindow.m_ItemCheck.size(); n++)
		{


			ITEM* CTItem = g_pNewItemMng->CreateItem(gCBAutoResetInfo->ViewInfoResetWindow.m_ItemCheck[n].Item);
			int CTItemIndex = CTItem->Type;

			//BYTE m_Skill = (gCBAutoResetInfo->ViewInfoResetWindow.m_ItemCheck[n].Item[1] / 128) & 1;
			//BYTE m_Luck = (gCBAutoResetInfo->ViewInfoResetWindow.m_ItemCheck[n].Item[1] / 4) & 1;
			//BYTE m_Opt = (gCBAutoResetInfo->ViewInfoResetWindow.m_ItemCheck[n].Item[1] & 3) + ((gCBAutoResetInfo->ViewInfoResetWindow.m_ItemCheck[n].Item[7] & 64) / 16);
			if (Count >= 5) break;
			if (SEASON3B::CheckMouseIn(InfoListX + 5, (InfoListY + 1) + (Count * 12), InfoListW + 15, 13) == 1)
			{
				ColorSelect = 0xABABAB64;
				//	gPostInterface.item_post_ = CTItem;

				//gInterface.DrawItemToolTipText(CTItem, *(int*)0x879340C, *(int*)0x8793410 + 25);
				//glColor3f(1, 1, 1);
				//pSetBlend(false);
				RenderItemInfo(MouseX + 75, MouseY, CTItem, 0, 0, false, false);
			}
			else
			{
				ColorSelect = 0x0;
			}


			//TextDraw((HFONT)g_hFont, InfoListX + 5, (InfoListY + 5) + (Count * 12), (gCBAutoResetInfo->ViewInfoResetWindow.m_ItemCheck[n].Status == 1) ? 0xFFDD00FF : 0xFC4023FF, ColorSelect, InfoListW + 15, 0, 1, "[x%d] %s", gCBAutoResetInfo->ViewInfoResetWindow.m_ItemCheck[n].SL, gCBAutoResetInfo->GetItemName(CTItemIndex, CTItem->Level));//
			TextDraw((HFONT)g_hFont, InfoListX + 5, (InfoListY + 5) + (Count * 12), (gCBAutoResetInfo->ViewInfoResetWindow.m_ItemCheck[n].Status == 1) ? 0xFFDD00FF : 0xFC4023FF, ColorSelect, InfoListW + 15, 0, 1, "[x%d] %s", gCBAutoResetInfo->ViewInfoResetWindow.m_ItemCheck[n].SL, gCBAutoResetInfo->GetItemName(CTItemIndex, CTItem->Level));//
			Count++;

		}
	}
}
void CBAutoResetInfo::DrawWindow()
{

	if (!gCBAutoResetInfo) return;

	this->DrawWindowRelife();

	//if (GetKeyState(VK_F3) & 0x4000)
	//{
	//	gCBAutoResetInfo->OpenWindow();
	//}
	if (gInterface.CheckWindow(Interface::MoveList) || gInterface.CheckWindow(Interface::CashShop) || gInterface.CheckWindow(Interface::SkillTree) || gInterface.CheckWindow(Interface::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)) ||
		//gCBDauGiaItem.SelectItemDauGia == -1 ||
		GetKeyState(VK_ESCAPE) < 0)
	{
		gInterface.Data[eWindowAutoReset].OnShow = false;
		return;
	}

	if (!gInterface.Data[eWindowAutoReset].OnShow || this->LoadingInfo == 0 || gCBAutoResetInfo->ViewInfoResetWindow.LoaiReset < 0 || gCBAutoResetInfo->ViewInfoResetWindow.LoaiReset > 2)
	{
		return;
	}
	float WindowW = 320;
	float WindowH = 290;
	gInterface.Data[eWindowAutoReset].Width = WindowW;
	float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
	float StartY = 25.0;
	//if (!gCBUtil.gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowAutoReset, "Auto Reset Info"))
	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowAutoReset, "Auto Reset Info");



	//===Info
	float StartInfoBoxX = StartX + 7;
	float StartInfoBoxY = StartY + 35;
	float WBox = 140;
	float HBox = 65;

	g_pBCustomMenuInfo->DrawInfoBox(StartInfoBoxX, StartInfoBoxY, WBox - 2, HBox, 0x00000096, 0, 0);
	float TextX = StartInfoBoxX;
	float TextY = StartInfoBoxY + 3;
	TextDraw((HFONT)g_hFontBold, TextX, TextY, 0xEB8213FF, 0x0, WBox, 0, 3, "[%s] Reset Tiếp Theo: %d", CharacterAttribute->Name, gCBAutoResetInfo->ViewInfoResetWindow.ResetView + 1);
	TextY += 7;
	TextDraw((HFONT)g_hFontBold, TextX, TextY, 0xFFFFFFFF, 0x0, WBox + 2, 0, 3, "-------------------------------------------------------");
	TextY += 12;
	TextDraw((HFONT)g_hFont, TextX + 3, TextY, 0xFFFFFFFF, 0x0, WBox, 0, 1, "- Level Yêu Cầu:");
	TextDraw((HFONT)g_hFontBold, TextX, TextY, 0xEBE313FF, 0x0, WBox, 0, 4, "%d/%d", CharacterAttribute->Level, gCBAutoResetInfo->ViewInfoResetWindow.Level);
	TextY += 12;
	TextDraw((HFONT)g_hFont, TextX + 3, TextY, 0xFFFFFFFF, 0x0, WBox, 0, 1, "- GHRS:");
	TextDraw((HFONT)g_hFontBold, TextX, TextY, 0xEBE313FF, 0x0, WBox, 0, 4, "%d/%d", gCBAutoResetInfo->ViewInfoResetWindow.ResetDay, gCBAutoResetInfo->ViewInfoResetWindow.MaxGHRS);
	TextY += 12;
	TextDraw((HFONT)g_hFont, TextX + 3, TextY, 0xFFFFFFFF, 0x0, WBox, 0, 1, "- Dạng Reset:");
	TextDraw((HFONT)g_hFontBold, TextX, TextY, 0xE11FFFFF, 0x0, WBox, 0, 4, NameTypeReset[gCBAutoResetInfo->ViewInfoResetWindow.LoaiReset]);
	TextY += 12;
	TextDraw((HFONT)g_hFont, TextX + 3, TextY, 0xFFFFFFFF, 0x0, WBox, 0, 1, "- Điểm nhận Reset:");
	TextDraw((HFONT)g_hFontBold, TextX, TextY, 0x1FFFC7FF, 0x0, WBox, 0, 4, "%s", gInterface.NumberFormat(gCBAutoResetInfo->ViewInfoResetWindow.Point));
	//TextY += 14;
	//TextDraw((HFONT)g_hFont, TextX + 3, TextY + 1, 0xB1EB13FF, 0x0, WBox, 0, 1, "*** Nguyên Liệu Yêu Cầu:");
	//==Show Bang Nguyen Lieu
	//if (g_pBCustomMenuInfo->DrawButton(TextX + WBox - 50, TextY, 80, 11, "XEM", 50) && (GetTickCount() - gInterface.Data[eWindowAutoReset].EventTick) > 300)
	//{
	////	gInterface.WindowDieuKienResetInfo->onShow ^= 1;
	//}

	float InfoMidX = TextX + WBox + 10;
	float InfoMidY = StartInfoBoxY + 3;
	//===Check Type Reset
	if (gCBAutoResetInfo->ViewInfoResetWindow.OnResetType & 1)
	{
		if (g_pBCustomMenuInfo->RenderCheckBox(InfoMidX + 5, InfoMidY, gCBAutoResetInfo->ViewInfoResetWindow.TypeReset & 1 ? 0x13EBDCFF : 0xFFFFFFFF, gCBAutoResetInfo->ViewInfoResetWindow.TypeReset & 1 ? TRUE : FALSE, "Reset Thường"))
		{
			XULY_CGPACKET pMsg;
			pMsg.header.set(0xD3, 0x36, sizeof(pMsg));
			pMsg.ThaoTac = 1; //
			DataSend((LPBYTE)& pMsg, pMsg.header.size);
		}
	}
	if (gCBAutoResetInfo->ViewInfoResetWindow.OnResetType & 2)
	{
		if (g_pBCustomMenuInfo->RenderCheckBox(InfoMidX + (WBox / 2) + 10, InfoMidY, gCBAutoResetInfo->ViewInfoResetWindow.TypeReset & 2 ? 0x13EBDCFF : 0xFFFFFFFF, gCBAutoResetInfo->ViewInfoResetWindow.TypeReset & 2 ? TRUE : FALSE, "Reset VIP"))
		{
			XULY_CGPACKET pMsg;
			pMsg.header.set(0xD3, 0x36, sizeof(pMsg));
			pMsg.ThaoTac = 2; //
			DataSend((LPBYTE)& pMsg, pMsg.header.size);
		}
	}
	InfoMidY += 17;
	if (gCBAutoResetInfo->ViewInfoResetWindow.LoaiReset > 0)
	{
		TextDraw((HFONT)g_hFont, InfoMidX + 3, InfoMidY + 1, 0xEB8213FF, 0x0, WBox, 0, 1, "Cộng Điểm Reset");
		TextDraw((HFONT)g_hFont, InfoMidX + 3 + (WBox / 2) + 10, InfoMidY + 1, 0x13EBDCFF, 0x0, WBox, 0, 1, "Điểm dư: %s", gInterface.NumberFormat(gCBAutoResetInfo->ViewInfoResetWindow.Point - PointReAdd));
		//===Add Point
		char* TypeName[5] = { strdup(GlobalText[166]), strdup(GlobalText[167]),strdup(GlobalText[169]),strdup(GlobalText[168]),strdup(GlobalText[939]) };
		//float NTCoinY = InfoMidY + 20;
		InfoMidY += 20;
		float XText = 75;
		float BGW = 55;
		float BGH = 12;
		int Type = 0;
		float NTCoinX = (InfoMidX + (80) - (XText + BGW) / 2);
		int CaclPointAdd = 0;
		for (int n = 0; n < 5; n++)
		{
			if ((CharacterAttribute->Class != CLASS_DARK_LORD && CharacterAttribute->Class != CLASS_LORDEMPEROR) && n == 4)
			{
				continue;
			}

			gInterface.DrawBarForm(NTCoinX, InfoMidY, BGW + XText, BGH, 0.29, 0.2767, 0.2581, 0.8);
			TextDraw((HFONT)g_hFont, NTCoinX, InfoMidY + 1.5, 0xFFFFFFFF, 0x0, XText - 20, 0, 3, TypeName[Type]);
			if (gCBAutoResetInfo->ViewInfoResetWindow.LoaiReset == 1)
			{
				//===Point defaut char
				gInterface.DrawBarForm(NTCoinX + XText - 20, InfoMidY, 20, BGH, 0.95, 0.6135, 0.1425, 0.8);
				TextDraw((HFONT)g_hFont, NTCoinX + XText - 20, InfoMidY + 1.5, 0xFFFFFFFF, 0x0, 20, 0, 3, "%d+", gCBAutoResetInfo->ViewInfoResetWindow.DefStat[n]);
			}
			//===input point
			gInterface.DrawBarForm(NTCoinX + XText, InfoMidY, BGW, BGH, 1.0, 1.0, 1.0, 0.8);
			if (!InputAddPointReset[Type])
			{
				InputAddPointReset[Type] = new CUITextInputBox;
				InputAddPointReset[Type]->Init(pGameWindow, BGW, BGH, 5);
				InputAddPointReset[Type]->SetBackColor(0, 0, 0, 0);
				InputAddPointReset[Type]->SetTextColor(255, 0, 0, 0);
				InputAddPointReset[Type]->SetFont((HFONT)g_hFont);
				InputAddPointReset[Type]->SetState(UISTATE_NORMAL);
				InputAddPointReset[Type]->SetOption(UIOPTION_NUMBERONLY);
				InputAddPointReset[Type]->SetPosition(NTCoinX + XText, InfoMidY + 1.5);
				InputAddPointReset[Type]->SetText("0");

			}
			else
			{
				InputAddPointReset[Type]->SetPosition(NTCoinX + XText, InfoMidY + 1.5);

				InputAddPointReset[Type]->Render();

				InputAddPointReset[Type]->GetText(TextInputAddPoint[Type], sizeof(TextInputAddPoint[Type]));

				if (atoi(TextInputAddPoint[Type]) > gCBAutoResetInfo->ViewInfoResetWindow.Point - CaclPointAdd)
				{
					char NumberFix[10] = { 0 };
					sprintf(NumberFix, "%d", (gCBAutoResetInfo->ViewInfoResetWindow.Point - CaclPointAdd));
					InputAddPointReset[Type]->SetText(NumberFix);
				}
				else
				{
					CaclPointAdd += atoi(TextInputAddPoint[Type]);
				}

				if (SEASON3B::CheckMouseIn(NTCoinX + XText, InfoMidY + 1.5, BGW, BGH) == 1)
				{

					if (GetKeyState(VK_LBUTTON) & 0x8000)
					{
						InputAddPointReset[Type]->GiveFocus(1);
						PlayBuffer(25, 0, 0);
					}
				}
			}
			Type++;
			InfoMidY += 20;
		}
		PointReAdd = CaclPointAdd;
		//===
	}
	int GetCountLine = TextDraw((HFONT)g_hFont, InfoMidX + 3, InfoMidY + 1, 0xFFFFFFFA9, 0x0, WBox, 0, 1, "*Lưu Ý: hãy cộng điểm cho lần Reset tiếp# theo hình thức này sẽ không có điểm danh# vọng reset");
	InfoMidY += GetCountLine * 12;
	float ButtonW = 80;
	if (gCBAutoResetInfo->ViewInfoResetWindow.AutoResetEnable)
	{
		//==OK Reset
		InfoMidY += 25;
		if (g_pBCustomMenuInfo->DrawButton(StartX + (WindowW / 4) - (ButtonW / 2), StartY + (WindowH - 40), 150, 12, "<Tắt Auto Reset>", ButtonW) && (GetTickCount() - gInterface.Data[eWindowAutoReset].EventTick) > 300)
		{
			gInterface.Data[eWindowAutoReset].EventTick = GetTickCount();
			RESETCODE_SEND pMsg;
			pMsg.header.set(0xD3, 0x37, sizeof(pMsg));
			pMsg.TypeReset = -1;

			for (int i = 0; i < 5; i++)
			{
				pMsg.AutoResetStats[i] = 0;
			}
			//===Code
			memset(&pMsg.CodeReset, 0, sizeof(pMsg.CodeReset));
			DataSend((LPBYTE)& pMsg, pMsg.header.size);
		}
	}
	//==OK Reset
	InfoMidY += 25;

	if (g_pBCustomMenuInfo->DrawButton(StartX + (WindowW / 2) + (ButtonW / 2), StartY + (WindowH - 40), 150, 12, "Thực Hiện Reset", ButtonW) && (GetTickCount() - gInterface.Data[eWindowAutoReset].EventTick) > 300)
	{
		gInterface.Data[eWindowAutoReset].EventTick = GetTickCount();
		RESETCODE_SEND pMsg;
		pMsg.header.set(0xD3, 0x37, sizeof(pMsg));
		pMsg.TypeReset = gCBAutoResetInfo->ViewInfoResetWindow.TypeReset;
		for (int i = 0; i < 5; i++)
		{
			pMsg.AutoResetStats[i] = atoi(TextInputAddPoint[i]);
		}
		//===Code
		memset(&pMsg.CodeReset, 0, sizeof(pMsg.CodeReset));
		memcpy(&pMsg.CodeReset, TextInputCodeReset, sizeof(pMsg.CodeReset));
		DataSend((LPBYTE)& pMsg, pMsg.header.size);
	}
	WindowDieuKienResetInfo(TextX, StartY + 75, WBox);


}
#endif