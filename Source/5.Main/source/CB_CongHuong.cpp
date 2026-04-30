#include "StdAfx.h"
#include "CB_CongHuong.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"

#if(CongHuongV2)
CB_CongHuong* gCB_CongHuong;

CB_CongHuong::CB_CongHuong() 
{

}

CB_CongHuong::~CB_CongHuong()
{
}
void CB_CongHuong::CH_RecvInfoGSSend(PMSG_GSSendInfoClient* pMsg)
{
	if (pMsg->mCH_IndexItem[0] != -1)
	{
		gCB_CongHuong->CH_EffectMixOn = 0;
		gCB_CongHuong->CH_TyLeThanhCong = pMsg->mCH_TyLeThanhCong;
		gCB_CongHuong->CH_ResetMix = pMsg->mCH_ResetMix;
		gCB_CongHuong->CH_TypeMix = pMsg->mCH_TypeMix;
		gCB_CongHuong->CH_CheckLvItem = pMsg->mCH_CheckLvItem;;
		for (int n = 0; n < 3; n++)
		{
			gCB_CongHuong->CH_IndexItem[n] = pMsg->mCH_IndexItem[n];
			gCB_CongHuong->CH_CoinMix[n] = pMsg->mCH_CoinMix[n];
			memset(gCB_CongHuong->CH_InfoItem[n], 0x0, sizeof(gCB_CongHuong->CH_InfoItem[n]));
			memcpy(gCB_CongHuong->CH_InfoItem[n], pMsg->mCH_InfoItem[n], sizeof(gCB_CongHuong->CH_InfoItem[n]));
		}
	}
	else {
		//gCB_CongHuong->m_ScrollPos = 0;

		gCB_CongHuong->CH_EffectMixOn = 0;
		gCB_CongHuong->CH_TyLeThanhCong = -1;
		gCB_CongHuong->CH_ResetMix = -1;
		gCB_CongHuong->CH_TypeMix = -1;
		gCB_CongHuong->CH_CheckLvItem = 0;
		for (int n = 0; n < 3; n++)
		{
			gCB_CongHuong->CH_IndexItem[n] = -1;
			gCB_CongHuong->CH_CoinMix[n] = -1;
			ZeroMemory(gCB_CongHuong->CH_InfoItem[n], sizeof(gCB_CongHuong->CH_InfoItem[n]));
		}
	}
}
void CB_CongHuong::XoaDataCongHuong()
{
	if (!gCB_CongHuong) return;

	//gCB_CongHuong->m_ScrollPos = 0;

	gCB_CongHuong->CH_EffectMixOn = 0;
	gCB_CongHuong->CH_TyLeThanhCong = -1;
	gCB_CongHuong->CH_ResetMix = -1;
	gCB_CongHuong->CH_TypeMix = -1;
	gCB_CongHuong->CH_CheckLvItem = 0;
	for (int n = 0; n < 3; n++)
	{
		gCB_CongHuong->CH_IndexItem[n] = -1;
		gCB_CongHuong->CH_CoinMix[n] = -1;
		ZeroMemory(gCB_CongHuong->CH_InfoItem[n], sizeof(gCB_CongHuong->CH_InfoItem[n]));
	}
	//==== Del Cache Item
	XULY_CGPACKET pMsg;
	pMsg.header.set(0xFA, 0x16, sizeof(pMsg));
	pMsg.ThaoTac = 0; //RollBack Item
	DataSend((LPBYTE)& pMsg, pMsg.header.size);
	//pDrawMessage("XoaDataCongHuong",1);
}
int CB_CongHuong::GetCountPhoiCongHuong(int mIndexItem, int Type) // OK
{
	int GetCount = 0;
	if (mIndexItem == -1 || mIndexItem > MAX_ITEM)
	{
		return GetCount;

	}

	ITEM_ATTRIBUTE* itemgoc = &ItemAttribute[mIndexItem];

	int CatType = (gCB_CongHuong->CH_IndexItem[0] / 512);

	if (itemgoc->Name[0] != NULL)
	{
		for (int i = 0; i < MAX_ITEM; i++)
		{
			int CatGet = (i / 512);
			if (CatType == CatGet)
			{
				ITEM_ATTRIBUTE* ItemGet = &ItemAttribute[i];
				if (ItemGet->Name[0] != NULL)
				{
					if (ItemGet->Level >= (itemgoc->Level) && gCB_CongHuong->CH_TypeMix == 1)
					{
						GetCount++;
					}
					else if (Type == 0)
					{
						GetCount++;
					}
					else if (Type == 2 && mIndexItem == i)
					{
						GetCount++;
					}
					else {
						continue;
					}
				}
			}

		}
	}

	return GetCount;

}
//int mViTriScroll = 0;
void CB_CongHuong::DrawWindowCongHuong()
{
	if (!gInterface.Data[eWindow_CongHuong].OnShow)
	{
		if (gCB_CongHuong->CH_IndexItem[0] != -1 || gCB_CongHuong->CH_IndexItem[1] != -1 || gCB_CongHuong->CH_IndexItem[2] != -1)
		{
			gCB_CongHuong->XoaDataCongHuong();
		}
		return;
	}
	if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY))
	{
		g_pNewUISystem->Show(SEASON3B::INTERFACE_INVENTORY);
	}

	float CuaSoW = 190; //340
	float CuaSoH = 345;

	float StartX = 95;
	float StartY = 45.0;
	//int GetInfoLoadItem = 0;

	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, CuaSoW, CuaSoH, eWindow_CongHuong, gOther.TextVN_CongHuongTrangBi[0]);
	//=== Draw Cong Huong
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 7, StartY + 35, 170, 130, 0x00000096, 0, 0);
	TextDraw(g_hFont, StartX + 7, StartY + 37, 0xFFFFFFFF, 0x0, 170, 0, 3, gOther.TextVN_CongHuongTrangBi[1]);// "(Dung Chuot Phai De Thao Tac)"); //
	//==== Khung INV
	g_pBCustomMenuInfo->gItemBoxInv(StartX + 12, StartY + 55, 4, 2, (gCB_CongHuong->CH_IndexItem[0] > -1) ? 1 : 0); //Item Chinh
	g_pBCustomMenuInfo->gItemBoxInv(StartX + 12 + 50, StartY + 55, 4, 2, (gCB_CongHuong->CH_IndexItem[1] > -1) ? 1 : 0); //Phoi
	SEASON3B::RenderImage(SEASON3B::CNewUIMasterSkillTree::IMAGE_MASTER_INTERFACE + 6, StartX + 12 + 50 + 45, StartY + 90, 15.0, 7.0, 13.0, 0); //-->>
	g_pBCustomMenuInfo->gItemBoxInv(StartX + 12 + 50 + 45 + 20, StartY + 55, 4, 2, (gCB_CongHuong->CH_IndexItem[2] > -1) ? 1 : 0); //Xem Truoc
	//====
	TextDraw(g_hFont, StartX + 12, StartY + 55 + (21 * 4), 0xFFFFFFFF, 0x0, 45, 0, 3, gOther.TextVN_CongHuongTrangBi[2]);//"Item Chinh"); //
	TextDraw(g_hFont, StartX + 12, StartY + 55 + (21 * 4) + 10, ((gCB_CongHuong->CH_IndexItem[0] > -1) ? 0x2B8EFFFF : 0xDE1616FF), 0x0, 45, 0, 3, "%d/1", (gCB_CongHuong->CH_IndexItem[0] > -1) ? 1 : 0); //
	if (gCB_CongHuong->CH_IndexItem[0] != -1)
	{
		ITEM* Item_1 = g_pNewItemMng->CreateItem(gCB_CongHuong->CH_InfoItem[0]);
		//===Draw Img Item
		g_pNewUISystem->RenderItem3DFree(StartX + 12, StartY + 22, (21 * 2), (21 * 4), Item_1->Type, Item_1->Level, Item_1->Option1, Item_1->ExtOption, 0, 1.0);//BMD MOdel
	}
	//===Draw Img Item
	if (gCB_CongHuong->CH_IndexItem[1] != -1)
	{
		ITEM* Item_2 = g_pNewItemMng->CreateItem(gCB_CongHuong->CH_InfoItem[1]);
		//===Draw Img Item
		g_pNewUISystem->RenderItem3DFree(StartX + 12 + 50, StartY + 22, (21 * 2), (21 * 4), Item_2->Type, Item_2->Level, Item_2->Option1, Item_2->ExtOption, 0, 1.0);
	}
	TextDraw(g_hFont, StartX + 12 + 50, StartY + 55 + (21 * 4), 0xFFFFFFFF, 0x0, 45, 0, 3, gOther.TextVN_CongHuongTrangBi[3]);//"Item Phoi"); //
	TextDraw(g_hFont, StartX + 12 + 50, StartY + 55 + (21 * 4) + 10, ((gCB_CongHuong->CH_IndexItem[1] > -1) ? 0x2B8EFFFF : 0xDE1616FF), 0x0, 45, 0, 3, "%d/1", (gCB_CongHuong->CH_IndexItem[1] > -1) ? 1 : 0); //


	TextDraw(g_hFont, StartX + 12 + 50 + 45 + 20, StartY + 55 + (21 * 4), 0xFFFFFFFF, 0x0, 45, 0, 3, gOther.TextVN_CongHuongTrangBi[4]);//"Xem Truoc"); //
	//===Draw Img Item
	if (gCB_CongHuong->CH_IndexItem[2] != -1)
	{
		ITEM* Item_3 = g_pNewItemMng->CreateItem(gCB_CongHuong->CH_InfoItem[2]);
		
		//===Draw Img Item
		g_pNewUISystem->RenderItem3DFree(StartX + 12 + 50 + 45 + 20, StartY + 22, (21 * 2), (21 * 4), Item_3->Type, Item_3->Level, Item_3->Option1, Item_3->ExtOption, 0, 1.0);
	}


	//== Draw List Phoi
	//g_pBCustomMenuInfo->DrawInfoBox(StartX + 190, StartY + 35, 120, 295, 0x00000096, 0, 0);
	//TextDraw(g_hFontBold, StartX + 195, StartY + 40, 0xFFBA19FF, 0x0, 90, 0, 1, gOther.TextVN_CongHuongTrangBi[12]);//"Danh Sach Phoi:"); //
	//int iMaxScrollCount = 0;
	//int CountItem = 0;
	//if (gCB_CongHuong->CH_IndexItem[0] != -1 && gCB_CongHuong->CH_IndexItem[0] < MAX_ITEM)
	//{
	//
	//	ITEM_ATTRIBUTE* itemgoc = &ItemAttribute[gCB_CongHuong->CH_IndexItem[0]];
	//
	//	int CatType = (gCB_CongHuong->CH_IndexItem[0] / 512);
	//
	//	if (itemgoc->Name[0] != NULL)
	//	{
	//		for (int i = 0; i < MAX_ITEM; i++)
	//		{
	//			int CatGet = (i / 512);
	//			if (CatType == CatGet)
	//			{
	//				ITEM_ATTRIBUTE* ItemGet = &ItemAttribute[i];
	//				if (ItemGet->Name[0] != NULL)
	//				{
	//					if (ItemGet->Level >= (itemgoc->Level) && gCB_CongHuong->CH_TypeMix == 1)
	//					{
	//						iMaxScrollCount++;
	//					}
	//					else if (gCB_CongHuong->CH_TypeMix == 0)
	//					{
	//						iMaxScrollCount++;
	//					}
	//					else if (gCB_CongHuong->CH_TypeMix == 2 && gCB_CongHuong->CH_IndexItem[0] == i)
	//					{
	//						iMaxScrollCount++;
	//					}
	//					else {
	//						continue;
	//					}
	//					if (iMaxScrollCount < gCB_CongHuong->m_ScrollPos)
	//					{
	//						continue;
	//					}
	//					//TextDraw(g_hFont, StartX + 195, StartY + 55 + (13 * CountItem), 0x2B8EFFFF, 0x0, 120, 0, 1, "- %s", ItemGet->Name); //
	//					CountItem++;
	//				}
	//			}
	//			if (CountItem >= 21)
	//			{
	//				break;
	//			}
	//		}
	//	}
	//
	//}
	//else
	//{
	//	TextDraw(g_hFont, StartX + 195, StartY + 55 + (13 * CountItem), 0xDE1616FF, 0x0, 120, 0, 1, gOther.TextVN_CongHuongTrangBi[13]); //
	//}
	//float flScrollDiv = 0;
	//
	//if (gCB_CongHuong->GetCountPhoiCongHuong(gCB_CongHuong->CH_IndexItem[0], gCB_CongHuong->CH_TypeMix) > 0)
	//{
	//	flScrollDiv = (10 * 30) / gCB_CongHuong->GetCountPhoiCongHuong(gCB_CongHuong->CH_IndexItem[0], gCB_CongHuong->CH_TypeMix);	//
	//}
	//SEASON3B::RenderImage(CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLLBAR_ON, StartX + 310, StartY + 55 + (gCB_CongHuong->m_ScrollPos * flScrollDiv), 15, 30, 0, 0); //Scroll
	//mViTriScroll = (int)(gCB_CongHuong->m_ScrollPos * flScrollDiv);
	//if (SEASON3B::CheckMouseIn(StartX + 310, StartY + 55 + (gCB_CongHuong->m_ScrollPos * flScrollDiv), 15, 30) == 1)
	//{
		//if (SEASON3B::IsPress(VK_LBUTTON) && GetTickCount() - gInterface.Data[eWindow_CongHuong].EventTick > 300)
		//{
		//
		//	PlayBuffer(25, 0, 0);
		//	glColor3f(1.0f, 0.0f, 0.0f);
		//	gInterface.Data[eWindow_CongHuong].EventTick = GetTickCount();
		//
		//}
		//else if (SEASON3B::IsRepeat(VK_LBUTTON))
		//{
		//	int mSrollY = (int)(StartY + 55 + (gCB_CongHuong->m_ScrollPos * flScrollDiv) + 15);
		//	if ((MouseY - mSrollY) > 5 && gCB_CongHuong->m_ScrollPos < gCB_CongHuong->GetCountPhoiCongHuong(gCB_CongHuong->CH_IndexItem[0], gCB_CongHuong->CH_TypeMix) && mViTriScroll < 245)
		//	{
		//		gCB_CongHuong->m_ScrollPos++;
		//		//ScorllY = pCursorY;
		//	}
		//	else if ((mSrollY - MouseY) > 5 && gCB_CongHuong->m_ScrollPos > 0)
		//	{
		//		gCB_CongHuong->m_ScrollPos--;
		//		//ScorllY = pCursorY;
		//	}
		//
		//}

		//pDrawColorButton(0x7A29, StartX + 310, StartY + 55 + (gInterface.m_ScrollPos * flScrollDiv), 15, 30, 0, 0, pMakeColor(255, 204, 20, 130));
		//gInterface.DrawToolTip(gObjUser.m_CursorX + 20, gObjUser.m_CursorY, "Scroll");
	//}

	//================================================

	//== Draw Info Mix CH
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 7, StartY + 46 + 130, 170, 100, 0x00000096, 0, 0);
	TextDraw(g_hFont, StartX + 15, StartY + 180, 0xFFFFFFFF, 0x0, (170 / 2), 0, 1, gOther.TextVN_CongHuongTrangBi[5]);//"Ty le thanh cong:"); //
	if (gCB_CongHuong->CH_TyLeThanhCong == -1)
	{
		TextDraw(g_hFont, StartX + 20 + (170 / 2), StartY + 180, 0xDE1616FF, 0x0, 0, 0, 1, gOther.TextVN_CongHuongTrangBi[13]);//"Chua Xac Dinh"); //
	}
	else
	{
		TextDraw(g_hFont, StartX + 20 + (170 / 2), StartY + 180, 0x19FF9FFF, 0x0, 0, 0, 1, "%d %%", gCB_CongHuong->CH_TyLeThanhCong);//
	}
	//==================
	TextDraw(g_hFont, StartX + 15, StartY + 180 + 13, 0xFFFFFFFF, 0x0, (170 / 2), 0, 1, gOther.TextVN_CongHuongTrangBi[6]);//"Yeu cau Reset:"); //
	if (gCB_CongHuong->CH_ResetMix == -1)
	{
		TextDraw(g_hFont, StartX + 20 + (170 / 2), StartY + 180 + 13, 0xDE1616FF, 0x0, 0, 0, 1, gOther.TextVN_CongHuongTrangBi[13]);//"Chua Xac Dinh"); //
	}
	else
	{
		TextDraw(g_hFont, StartX + 20 + (170 / 2), StartY + 180 + 13, 0x19FF9FFF, 0x0, 0, 0, 1, "%d (RS)", gCB_CongHuong->CH_ResetMix);//
	}
	//=======================
	TextDraw(g_hFont, StartX + 15, StartY + 180 + 26, 0xFFFFFFFF, 0x0, (170 / 2), 0, 1, gOther.TextVN_CongHuongTrangBi[7]);// "Kieu Cong Huong:"); //
	if (gCB_CongHuong->CH_TypeMix == -1)
	{
		TextDraw(g_hFont, StartX + 20 + (170 / 2), StartY + 180 + 26, 0xDE1616FF, 0x0, 0, 0, 1, gOther.TextVN_CongHuongTrangBi[13]);//"Chua Xac Dinh"); //
	}
	else
	{
		TextDraw(g_hFont, StartX + 20 + (170 / 2), StartY + 180 + 26, 0x19FF9FFF, 0x0, 0, 0, 1, "%s", gOther.TextVN_CongHuongTrangBi[14 + gCB_CongHuong->CH_TypeMix]);//
	}
	//======================================================
	TextDraw(g_hFont, StartX + 15, StartY + 225, 0xFFFFFFFF, 0x0, (170 / 2), 0, 1, gOther.TextVN_CongHuongTrangBi[8]);//"Chi Phi:"); //
	//===============
	TextDraw(g_hFont, StartX + 15, StartY + 225 + 13, 0xFFBA19FF, 0x0, (170 / 2), 0, 1, gOther.TextVN_CongHuongTrangBi[9]);//"- WCoin"); //
	if (gCB_CongHuong->CH_CoinMix[0] == -1)
	{
		TextDraw(g_hFont, StartX + 20 + (170 / 2), StartY + 225 + 13, 0xDE1616FF, 0x0, 0, 0, 1, gOther.TextVN_CongHuongTrangBi[13]); //
	}
	else {
		TextDraw(g_hFont, StartX + 20 + (170 / 2), StartY + 225 + 13, 0x19FF9FFF, 0x0, 0, 0, 1, "%s", gInterface.NumberFormat(gCB_CongHuong->CH_CoinMix[0])); //
	}
	//==============
	TextDraw(g_hFont, StartX + 15, StartY + 225 + (13 * 2), 0xFFBA19FF, 0x0, (170 / 2), 0, 1, gOther.TextVN_CongHuongTrangBi[10]);//"- WCoinP"); //
	if (gCB_CongHuong->CH_CoinMix[1] == -1)
	{
		TextDraw(g_hFont, StartX + 20 + (170 / 2), StartY + 225 + (13 * 2), 0xDE1616FF, 0x0, 0, 0, 1, gOther.TextVN_CongHuongTrangBi[13]); //
	}
	else {
		TextDraw(g_hFont, StartX + 20 + (170 / 2), StartY + 225 + (13 * 2), 0x19FF9FFF, 0x0, 0, 0, 1, "%s", gInterface.NumberFormat(gCB_CongHuong->CH_CoinMix[1])); //
	}

	TextDraw(g_hFont, StartX + 15, StartY + 225 + (13 * 3), 0xFFBA19FF, 0x0, (170 / 2), 0, 1, gOther.TextVN_CongHuongTrangBi[11]);//"- GobinP"); //
	//==============
	TextDraw(g_hFont, StartX + 15, StartY + 225 + (13 * 2), 0xFFBA19FF, 0x0, (170 / 2), 0, 1, gOther.TextVN_CongHuongTrangBi[10]);//"- WCoinP"); //
	if (gCB_CongHuong->CH_CoinMix[2] == -1)
	{
		TextDraw(g_hFont, StartX + 20 + (170 / 2), StartY + 225 + (13 * 3), 0xDE1616FF, 0x0, 0, 0, 1, gOther.TextVN_CongHuongTrangBi[13]); //
	}
	else {
		TextDraw(g_hFont, StartX + 20 + (170 / 2), StartY + 225 + (13 * 3), 0x19FF9FFF, 0x0, 0, 0, 1, "%s", gInterface.NumberFormat(gCB_CongHuong->CH_CoinMix[2])); //
	}
	//== Button Mix
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 7, StartY + 50 + 140 + 100, 170, 40, 0x00000096, 0, 0);
	SEASON3B::RenderImage(CNewUIMixInventory::IMAGE_MIXINVENTORY_MIXBTN, StartX + 7 + (170 / 2) - 15, StartY + 55 + 140 + 100, 44.0, 35.0, 0, 0); //ButtonMix
	if (SEASON3B::CheckMouseIn(StartX + 7 + (170 / 2) - 15, StartY + 55 + 140 + 100, 44.0, 35.0) == 1 && gCB_CongHuong->CH_IndexItem[2] != -1)
	{
		if (GetTickCount() - gInterface.Data[eWindow_CongHuong].EventTick > 300)
		{
			if (GetKeyState(1) & 0x8000)
			{
				PlayBuffer(25, 0, 0);
				glColor3f(1.0f, 0.0f, 0.0f);
				//gCentral.LockMouseMoveClick = GetTickCount();
				gInterface.Data[eWindow_CongHuong].EventTick = GetTickCount();
				gCB_CongHuong->CH_EffectMixOn ^= 1;
				//==== Send Mix
				XULY_CGPACKET pMsg;
				pMsg.header.set(0xFA, 0x17, sizeof(pMsg));
				pMsg.ThaoTac = 1; //RollBack Item
				DataSend((LPBYTE)& pMsg, pMsg.header.size);
			}
		}
		//pDrawColorButton(31419, StartX + 7 + (170 / 2) - 15, StartY + 55 + 140 + 100, 44.0, 35.0, 0, 0, pMakeColor(255, 204, 20, 130));
		//gInterface.DrawToolTip(gObjUser.m_CursorX + 20, gObjUser.m_CursorY, "Mix Item");
	}
	if (gCB_CongHuong->CH_EffectMixOn)
	{
		RenderMixEffect(1, StartX + 12 + 50 + 45 + 20, StartY + 55,3,5);
	}
	//== Draw Info Item In mouse over

	//=== Item Chinh
	if (SEASON3B::CheckMouseIn(StartX + 12, StartY + 55, 2 * 21, 4 * 21) == 1 && gCB_CongHuong->CH_IndexItem[0] != -1)
	{
		if (GetTickCount() - gInterface.Data[eWindow_CongHuong].EventTick > 300)
		{
			if (GetKeyState(VK_RBUTTON) & 0x8000)
			{
				PlayBuffer(25, 0, 0);
				gInterface.Data[eWindow_CongHuong].EventTick = GetTickCount();
				XULY_CGPACKET pMsg;
				pMsg.header.set(0xFA, 0x16, sizeof(pMsg));
				pMsg.ThaoTac = 0; //RollBack Item
				DataSend((LPBYTE)& pMsg, pMsg.header.size);
			}
		}
		//B.CalcX = (int)StartX + 130;
		//B.CalcY = (int)StartY + 55;
		//GetInfoLoadItem = BPConvertByteItem(*(DWORD*)(GetInstance() + 36), gCB_CongHuong->CH_InfoItem[0]);
		//BPDrawInfoItem(*(DWORD*)(GetInstance() + 308), B, GetInfoLoadItem, 0, 0, 0);
		ITEM* CTItem = g_pNewItemMng->CreateItem(gCB_CongHuong->CH_InfoItem[0]);
		RenderItemInfo(MouseX + 75, MouseY, CTItem, 0, 0, false, false);

	}
	//=== Phoi Item
	if (SEASON3B::CheckMouseIn(StartX + 12 + 50, StartY + 55, 2 * 21, 4 * 21) == 1 && gCB_CongHuong->CH_IndexItem[1] != -1)
	{
		if (GetTickCount() - gInterface.Data[eWindow_CongHuong].EventTick > 300)
		{
			if (GetKeyState(VK_RBUTTON) & 0x8000)
			{
				PlayBuffer(25, 0, 0);
				gInterface.Data[eWindow_CongHuong].EventTick = GetTickCount();
				XULY_CGPACKET pMsg;
				pMsg.header.set(0xFA, 0x16, sizeof(pMsg));
				pMsg.ThaoTac = 2; //RollBack Item
				DataSend((LPBYTE)& pMsg, pMsg.header.size);
			}
		}
		//B.X = (int)StartX + 130 + 50;
		//B.Y = (int)StartY + 55;
		//GetInfoLoadItem = BPConvertByteItem(*(DWORD*)(GetInstance() + 36), gCB_CongHuong->CH_InfoItem[1]);
		//BPDrawInfoItem(*(DWORD*)(GetInstance() + 308), B, GetInfoLoadItem, 0, 0, 0);
		ITEM* CTItem = g_pNewItemMng->CreateItem(gCB_CongHuong->CH_InfoItem[1]);
		RenderItemInfo(MouseX + 75, MouseY, CTItem, 0, 0, false, false);

	}
	//== Xem Truoc
	if (SEASON3B::CheckMouseIn(StartX + 12 + 50 + 45 + 20, StartY + 55, 2 * 21, 4 * 21) == 1 && gCB_CongHuong->CH_IndexItem[2] != -1)
	{
		//B.X = (int)StartX + 130 + 50 + 45 + 20;
		//B.Y = (int)StartY + 55;
		//GetInfoLoadItem = BPConvertByteItem(*(DWORD*)(GetInstance() + 36), gCB_CongHuong->CH_InfoItem[2]);

		//BPDrawInfoItem(*(DWORD*)(GetInstance() + 308), B, GetInfoLoadItem, 0, 0, 0);
		ITEM* CTItem = g_pNewItemMng->CreateItem(gCB_CongHuong->CH_InfoItem[2]);
		RenderItemInfo(MouseX + 75, MouseY, CTItem, 0, 0, false, false);
	}
	EndRenderColor();
	glColor4f(1.0, 1.0, 1.0, 1.0);
	EnableAlphaTest(0);
}

bool CB_CongHuong::SendItem(ITEM* ItemSell, int Slot, bool KeyClick)
{
	if (KeyClick && gInterface.Data[eWindow_CongHuong].OnShow && GetTickCount() > gInterface.Data[eWindow_CongHuong].EventTick)// Send Item
	{
		if (ItemSell->Type < 0) { return 0; }

		PMSG_ITEM_MOVE_RECV pMsg = { 0 };

		pMsg.h.set(0xFA, 0x15, sizeof(pMsg));
		pMsg.Target = -1;
		pMsg.sFlag = 0;
		pMsg.tFlag = 0;
		pMsg.Target = 0;
		pMsg.Source = Slot;
		::PlayBuffer(SOUND_GET_ITEM01);
		DataSend((BYTE*)& pMsg, pMsg.h.size);
		gInterface.Data[eWindow_CongHuong].EventTick = GetTickCount() + 1000;
		return 1;
	}

	return 0;
}
#endif