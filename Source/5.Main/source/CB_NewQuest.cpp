#include "StdAfx.h"
#include "CB_NewQuest.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"
#if(CB_NEWQUESTFF)
CB_NewQuest* gCB_NewQuest;

CB_NewQuest::CB_NewQuest()
{
	this->Status = 0;
	this->MaxLevelQuest = 0;
	this->MaxPerPage = 5;
	this->Page = 0;

	for (int i = 0; i < 5; i++)
	{
		this->MonterKill[i] = 0;
	}
}

CB_NewQuest::~CB_NewQuest()
{

}

void CB_NewQuest::RecvInfoMonterKill(BYTE* Recv)
{
	if (!Recv) return;

	PMSG_NQUEST_PROCSEND* lpMsg = (PMSG_NQUEST_PROCSEND*)Recv;
	for (int i = 0; i < 5; i++)
	{
		this->MonterKill[i] = lpMsg->MonterKill[i];
	}
	//gInterface.DrawMessage(1, "RecvInfoMonterKill");
	if (gInterface.Data[eWindowNewQuestMini].OnShow != 1) gInterface.Data[eWindowNewQuestMini].OnShow = 1;
}
void CB_NewQuest::RecvInfo(BYTE* Recv)
{
	if (!Recv) return;

	PMSG_NQUEST_SEND* lpMsg = (PMSG_NQUEST_SEND*) Recv;

	this->Status = lpMsg->Status;
	this->MaxLevelQuest = lpMsg->MaxLevelQuest;

	this->m_DataNQuest.clear();
	for (int n = 0; n < lpMsg->count; n++)
	{
		DATA_NQUEST_REQ* lpInfo = (DATA_NQUEST_REQ*)(((BYTE*)lpMsg) + sizeof(PMSG_NQUEST_SEND) + (sizeof(DATA_NQUEST_REQ) * n));
		this->m_DataNQuest.push_back(*lpInfo);
		//gInterface.DrawMessage(1, "Level %d/%d [%s]", lpInfo->NQuest_LV, this->MaxLevelQuest, lpInfo->NameQuest);
	}
	if (lpMsg->NPCOpen && !this->m_DataNQuest.empty())  gInterface.Data[eWindowNewQuest].OnShow = 1;
	if (gInterface.Data[eWindowNewQuestMini].OnShow != 1 && !this->m_DataNQuest.empty()) gInterface.Data[eWindowNewQuestMini].OnShow = 1;
}

void CB_NewQuest::RecvRewardList(BYTE* Recv)
{
	if (!Recv) return;
	PMSG_NQUEST_SEND* lpMsg = (PMSG_NQUEST_SEND*)Recv;
	this->m_DataRewardList.clear();
	for (int n = 0; n < lpMsg->count; n++)
	{
		DATA_NQUEST_REWARD_LOCAL* lpInfo = (DATA_NQUEST_REWARD_LOCAL*)(((BYTE*)lpMsg) + sizeof(PMSG_NQUEST_SEND) + (sizeof(DATA_NQUEST_REWARD_LOCAL) * n));

		DATA_NQUEST_REWARD_LOCAL_C InfoTT = { 0 };
		InfoTT.Count = lpInfo->Count;
		InfoTT.Item = g_pNewItemMng->CreateItem(lpInfo->Item);

		this->m_DataRewardList.push_back(InfoTT);


		//gInterface.DrawMessage(1, "(%d) %d", n, this->m_DataRewardList[n]->Type);
	}
	//gInterface.DrawMessage(1, "RecvRewardList");
}
void CB_NewQuest::DrawWindowMini()
{
	if (gInterface.CheckWindow(Interface::MoveList) 
		|| gInterface.CheckWindow(Interface::ObjWindow::CreateGuild)
		|| gInterface.CheckWindow(Interface::ObjWindow::BloodCastle)
		|| gInterface.CheckWindow(Interface::ObjWindow::DevilSquare)
		|| gInterface.CheckWindow(Interface::ObjWindow::Character)
		|| gInterface.CheckWindow(Interface::ObjWindow::FastMenu)
		|| gInterface.CheckWindow(Interface::ObjWindow::CashShop)
		|| gInterface.CheckWindow(Interface::ObjWindow::SkillTree) || gInterface.CheckWindow(Interface::ObjWindow::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)) || gInterface.CheckWindow(Interface::Inventory))
	{
		
		return;
	}

	if (!gInterface.Data[eWindowNewQuestMini].OnShow || this->m_DataNQuest.empty() || this->Status == eNone)
	{
		return;
	}
	float WindowW = 150;
	float WindowH = 60;
	float StartX = (MAX_WIN_WIDTH - WindowW) - (10);
	float StartY = (MAX_WIN_HEIGHT - 135);


	g_pBCustomMenuInfo->DrawWindowCustomMini(&StartX, &StartY, WindowW, WindowH, eWindowNewQuestMini, "Nhiệm Vụ %d", this->Status);

	if (gInterface.Data[eWindowNewQuestMini].BActiveHiden) return;

	int InfoX = StartX + 5;
	int InfoY = StartY + 30;
	int HText = 12;
	int CountText = 0;
	TextDraw((HFONT)g_hFont, InfoX, InfoY + (HText * CountText++), 0xFFC421FF, 0x0, WindowW - 30, 0, 1, "%s (%d)", this->m_DataNQuest[0].NameQuest, this->m_DataNQuest[0].NQuest_LV);
	DWORD ColorText = 0xFFFFFFFF;
	if (this->Status == eDaDuYeuCau)
	{
		ColorText = 0x79FF54FF;
	}
	else
	{
		ColorText = 0xFFFFFFFF;
	}
	if (this->m_DataNQuest[0].MonterClass[0] != -1) TextDraw((HFONT)g_hFont, InfoX + 10, InfoY + (HText * CountText++), ColorText, 0x0, WindowW - 30, 0, 1, "Diệt [%s] (%d/%d)", getMonsterName(this->m_DataNQuest[0].MonterClass[0]), this->MonterKill[0], this->m_DataNQuest[0].NQuest_C[0]);
	if (this->m_DataNQuest[0].MonterClass[1] != -1) TextDraw((HFONT)g_hFont, InfoX + 10, InfoY + (HText * CountText++), ColorText, 0x0, WindowW - 30, 0, 1, "Diệt [%s] (%d/%d)", getMonsterName(this->m_DataNQuest[0].MonterClass[1]), this->MonterKill[1], this->m_DataNQuest[0].NQuest_C[1]);
	if (this->m_DataNQuest[0].MonterClass[2] != -1) TextDraw((HFONT)g_hFont, InfoX + 10, InfoY + (HText * CountText++), ColorText, 0x0, WindowW - 30, 0, 1, "Diệt [%s] (%d/%d)", getMonsterName(this->m_DataNQuest[0].MonterClass[2]), this->MonterKill[2], this->m_DataNQuest[0].NQuest_C[2]);
	if (this->m_DataNQuest[0].MonterClass[3] != -1) TextDraw((HFONT)g_hFont, InfoX + 10, InfoY + (HText * CountText++), ColorText, 0x0, WindowW - 30, 0, 1, "Diệt [%s] (%d/%d)", getMonsterName(this->m_DataNQuest[0].MonterClass[3]), this->MonterKill[3], this->m_DataNQuest[0].NQuest_C[3]);
	if (this->m_DataNQuest[0].MonterClass[4] != -1) TextDraw((HFONT)g_hFont, InfoX + 10, InfoY + (HText * CountText++), ColorText, 0x0, WindowW - 30, 0, 1, "Diệt [%s] (%d/%d)", getMonsterName(this->m_DataNQuest[0].MonterClass[4]), this->MonterKill[4], this->m_DataNQuest[0].NQuest_C[4]);

	if ((40 + (HText * CountText)) > (gInterface.Data[eWindowNewQuestMini].Height))
	{
		gInterface.Data[eWindowNewQuestMini].Height += 10;
	}
}

void CB_NewQuest::DrawWindow()
{
	this->DrawWindowMini();

	if (gInterface.CheckWindow(Interface::MoveList) || gInterface.CheckWindow(Interface::ObjWindow::CashShop) || gInterface.CheckWindow(Interface::ObjWindow::SkillTree) || gInterface.CheckWindow(Interface::ObjWindow::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)))
	{
		gInterface.Data[eWindowNewQuest].OnShow = false;
		return;
	}

	if (!gInterface.Data[eWindowNewQuest].OnShow || this->m_DataNQuest.empty())
	{
		this->Page = 0;
		return;
	}
	float WindowW = 290;
	float WindowH = 380;
	float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
	float StartY = 30;
	
	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowNewQuest, "Hệ Thống Nhiệm Vụ");
	//g_pBCustomMenuInfo->DrawWindowCustomMini(&StartX, &StartY, WindowW, WindowH, eWindowNewQuest, "Hệ Thống Nhiệm Vụ");

	int InfoX = StartX + 15;
	int InfoY = StartY + 40;
	int HText = 12;
	int CountText = 0;
	//0x62FF00FF = Xanh La
	TextDraw((HFONT)g_hFontBold, InfoX, InfoY+(HText* CountText++), 0xFFC421FF, 0x0, WindowW - 30, 0, 3, "Thông Tin Cần Biết");
	CountText += 1;
	TextDraw((HFONT)g_hFont, InfoX, InfoY+(HText* CountText++), 0xFFFFFFFF, 0x0, WindowW-30, 0, 1, "- Đến các ngôi làng của lục địa Mu và tiêu diệt lũ quái vật cứng đầu");
	TextDraw((HFONT)g_hFont, InfoX, InfoY+(HText* CountText++), 0xFFFFFFFF, 0x0, WindowW-30, 0, 1, "- Bạn cần cống nạp một số vật phẩm để thực hiện nhiệm vụ");
	TextDraw((HFONT)g_hFont, InfoX, InfoY+(HText* CountText++), 0xFFFFFFFF, 0x0, WindowW-30, 0, 1, "- Sau khi hoàn thành bạn sẽ nhận được 1 số phần thưởng");

	if(CountText < 8) CountText = 8;
	if (this->Status != eNone)
	{
		TextDraw((HFONT)g_hFontBold, InfoX, InfoY + (HText * CountText++), 0xFFC421FF, 0x0, WindowW - 30, 0, 3, "Thông Tin Nhiệm Vụ");
	}
	else
	{
		TextDraw((HFONT)g_hFontBold, InfoX, InfoY + (HText * CountText++), 0xFFC421FF, 0x0, WindowW - 30, 0, 3, "Nhiệm Vụ Mới");
	}
	CountText += 1;
	TextDraw((HFONT)g_hFontBold, InfoX + 80, InfoY + (HText * CountText), 0x62FF00FF, 0x0, WindowW - 30, 0, 1, this->m_DataNQuest[0].NameQuest);
	TextDraw((HFONT)g_hFont, InfoX + 10, InfoY + (HText * CountText++), 0xFFFFFFFF, 0x0, WindowW - 30, 0, 1, "Nhiệm Vụ: ");
	

	TextDraw((HFONT)g_hFontBold, InfoX + 80, InfoY + (HText * CountText), 0xFF904FFF, 0x0, WindowW - 30, 0, 1, "%02d/%02d", this->m_DataNQuest[0].NQuest_LV,this->MaxLevelQuest);
	TextDraw((HFONT)g_hFont, InfoX + 10, InfoY + (HText * CountText++), 0xFFFFFFFF, 0x0, WindowW - 30, 0, 1, "Cấp độ: ");

	if (this->Status != eNone)
	{
		if(this->m_DataNQuest[0].MonterClass[0] != -1) TextDraw((HFONT)g_hFontBold, InfoX + 80, InfoY + (HText * CountText), 0xFFFFFFFF, 0x0, WindowW - 30, 0, 1, "Tiêu diệt quái vật [%s] (%d/%d)", getMonsterName(this->m_DataNQuest[0].MonterClass[0]), this->MonterKill[0], this->m_DataNQuest[0].NQuest_C[0]);
		TextDraw((HFONT)g_hFont, InfoX + 10, InfoY + (HText * CountText++), 0xFFFFFFFF, 0x0, WindowW - 30, 0, 1, "Thực hiện: ");

		if (this->m_DataNQuest[0].MonterClass[1] != -1) TextDraw((HFONT)g_hFontBold, InfoX + 80, InfoY + (HText * CountText++), 0xFFFFFFFF, 0x0, WindowW - 30, 0, 1, "Tiêu diệt quái vật [%s] (%d/%d)", getMonsterName(this->m_DataNQuest[0].MonterClass[1]), this->MonterKill[1], this->m_DataNQuest[0].NQuest_C[1]);
		if (this->m_DataNQuest[0].MonterClass[2] != -1) TextDraw((HFONT)g_hFontBold, InfoX + 80, InfoY + (HText * CountText++), 0xFFFFFFFF, 0x0, WindowW - 30, 0, 1, "Tiêu diệt quái vật [%s] (%d/%d)", getMonsterName(this->m_DataNQuest[0].MonterClass[2]), this->MonterKill[2], this->m_DataNQuest[0].NQuest_C[2]);
		if (this->m_DataNQuest[0].MonterClass[3] != -1) TextDraw((HFONT)g_hFontBold, InfoX + 80, InfoY + (HText * CountText++), 0xFFFFFFFF, 0x0, WindowW - 30, 0, 1, "Tiêu diệt quái vật [%s] (%d/%d)", getMonsterName(this->m_DataNQuest[0].MonterClass[3]), this->MonterKill[3], this->m_DataNQuest[0].NQuest_C[3]);
		if (this->m_DataNQuest[0].MonterClass[4] != -1) TextDraw((HFONT)g_hFontBold, InfoX + 80, InfoY + (HText * CountText++), 0xFFFFFFFF, 0x0, WindowW - 30, 0, 1, "Tiêu diệt quái vật [%s] (%d/%d)", getMonsterName(this->m_DataNQuest[0].MonterClass[4]), this->MonterKill[4], this->m_DataNQuest[0].NQuest_C[4]);
	}
	else
	{
		TextDraw((HFONT)g_hFontBold, InfoX + 80, InfoY + (HText * CountText), 0xFFFFFFFF, 0x0, WindowW - 30, 0, 1, "Cấp độ: %d", this->m_DataNQuest[0].ReqLevel);
		TextDraw((HFONT)g_hFont, InfoX + 10, InfoY + (HText * CountText++), 0xFFFFFFFF, 0x0, WindowW - 30, 0, 1, "Yêu cầu: ");
		if(this->m_DataNQuest[0].ReqBless > 0)   TextDraw((HFONT)g_hFontBold, InfoX + 80, InfoY + (HText * CountText++), 0xFFFFFFFF, 0x0, WindowW - 30, 0, 1, "Bless: %d", this->m_DataNQuest[0].ReqBless);
		if (this->m_DataNQuest[0].ReqSoul > 0)   TextDraw((HFONT)g_hFontBold, InfoX + 80, InfoY + (HText * CountText++), 0xFFFFFFFF, 0x0, WindowW - 30, 0, 1, "Soul: %d", this->m_DataNQuest[0].ReqSoul);
		if (this->m_DataNQuest[0].ReqLife > 0)   TextDraw((HFONT)g_hFontBold, InfoX + 80, InfoY + (HText * CountText++), 0xFFFFFFFF, 0x0, WindowW - 30, 0, 1, "Life: %d", this->m_DataNQuest[0].ReqLife);
		if (this->m_DataNQuest[0].ReqChaos > 0)  TextDraw((HFONT)g_hFontBold, InfoX + 80, InfoY + (HText * CountText++), 0xFFFFFFFF, 0x0, WindowW - 30, 0, 1, "Chaos: %d", this->m_DataNQuest[0].ReqChaos);
		if (this->m_DataNQuest[0].ReqCre > 0)    TextDraw((HFONT)g_hFontBold, InfoX + 80, InfoY + (HText * CountText++), 0xFFFFFFFF, 0x0, WindowW - 30, 0, 1, "Cre: %d", this->m_DataNQuest[0].ReqCre);
	}
	if (CountText < 20) CountText = 18;
	TextDraw((HFONT)g_hFontBold, InfoX, InfoY + (HText * CountText++), 0xFFC421FF, 0x0, WindowW - 30, 0, 3, "Phần Thưởng");
	CountText += 1;

	InfoY = InfoY+(HText * CountText);

	int CountItem = 0;
	int  ShowInfoItem = -1;
	for (int iT = (this->Page*this->MaxPerPage); iT < this->m_DataRewardList.size(); ++iT)
	{
		if (SEASON3B::CheckMouseIn(InfoX + 25 + (43 * CountItem), InfoY, 42, 42))
		{
			ShowInfoItem = iT;
		}

		gInterface.DrawBarForm(InfoX + 25 + (43* CountItem), InfoY, 42, 42, 0.47, 0.2726, 0.1739, 0.7);
		g_pNewUISystem->RenderItem3DFree(InfoX + 25 + (43 * CountItem), InfoY-5, 42, 42, this->m_DataRewardList[iT].Item->Type, this->m_DataRewardList[iT].Item->Level, this->m_DataRewardList[iT].Item->Option1, this->m_DataRewardList[iT].Item->ExtOption, 0, 1.4,FALSE);//BMD MOdel
		TextDraw((HFONT)g_hFontMini, InfoX + 25 + (43 * CountItem), InfoY+34, 0xFFFFFFA6, 0x0, 42, 0, 4, "x%02d", this->m_DataRewardList[iT].Count);//
		CountItem++;
		if (CountItem >= this->MaxPerPage) break;
	}
	if (this->m_DataRewardList.size() > this->MaxPerPage)
	{
		if (g_pBCustomMenuInfo->DrawButtonGUI(32345, InfoX, InfoY + 9, 20, 22, 3))
		{
			if (this->Page > 0) this->Page--;
		}
		if (g_pBCustomMenuInfo->DrawButtonGUI(32346, InfoX + (WindowW - 45), InfoY + 9, 20, 22, 3))
		{
			if (this->Page < this->m_DataRewardList.size()) this->Page++;
		}
		
	}
	static char* TextButton[] = { "Nhận Nhiệm Vụ" ,"Đang Thực Hiện", "Nhận Thưởng" };
	if (g_pBCustomMenuInfo->DrawButton(StartX+((WindowW/2)-60), StartY+(WindowH-50), 150, 12, TextButton[this->Status], 120, true))
	{
		XULY_CGPACKET pMsg;
		pMsg.header.set(0xD3, 0x75, sizeof(pMsg));
		pMsg.ThaoTac = 1154; //
		DataSend((LPBYTE)& pMsg, pMsg.header.size);
	}

	if (ShowInfoItem != -1 /*&& !gInterface.Data[eWindowChotroiRaoBan].Hover*/)
	{
		//int BGetInfoLoadItem = BPConvertByteItem(*(DWORD*)(GetInstance() + 36), gInterface.m_DataChoTroi[ShowInfoItem].Item);
		//JCCoord B;
		//B.X = (int)pCursorX + 75;
		//B.Y = (int)pCursorY;
		//BPDrawInfoItem(*(DWORD*)(GetInstance() + 308), B, BGetInfoLoadItem, 0, 0, 0);
		RenderItemInfo(MouseX + 75, MouseY, this->m_DataRewardList[ShowInfoItem].Item, 0, 0, false, false);
	}
}
#endif