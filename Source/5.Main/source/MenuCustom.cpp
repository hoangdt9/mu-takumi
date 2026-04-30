#include "stdafx.h"
#include "MenuCustom.h"
#include "Util.h"
#include "Protect.h"
#include "CBInterface.h"
#include "NewUISystem.h"
#include "Other.h"
#include "CustomEventTime.h"
#include "Ranking.h"
#include "WindowClass.h"
#include "CBChoTroi.h"
#include "CB_AutoNapGame.h"
#include "CB_DanhHieu.h"
#include "CB_JewelBank.h"
#include "CustomRanking.h"
#include "CBInterfaceVIPChar.h"
#include "CB_HuyDongExc.h"
#include "CB_DoiMK.h"
#include "MocDonate.h"
#include "CB_LockItem.h"
#include "VongQuay.h"

cCustomMenu gCustomMenu;

cCustomMenu::cCustomMenu()
{

}

cCustomMenu::~cCustomMenu()
{
}


void cCustomMenu::GetCountButton()
{

}
void cCustomMenu::ActionButton(int TypeButton)
{
	switch (TypeButton)
	{
	case eButtonEventTime: //Time Event
	{
		gInterface.Data[eWindowEventTime].OpenClose();
		if (gInterface.Data[eWindowEventTime].OnShow)
		{
			gCustomEventTime.ClearCustomEventTime();

			gCustomEventTime.OpenTestWindow();
		}
	}
	break;
	case eButtonVipShop: //VIP
	{
		gInterface.Data[eVip_MAIN].OpenClose();
	}
	break;
	case eButtonRanking: //Ranking
	{
		gCustomRanking->OpenWindow();
		//gRanking.OpenOnOff();
	}
	break;
	case eButtonChangeClass: //Change Class
	{
		WindowClass.SetVisible(true);
	}
	break;
#if(CUSTOM_BCHOTROI)
	case eButtonMarKet: //cho troi
	{
		gCBChotroi.GetOpenChoTroiWinDow();
	}
	break;
#endif
#if(DANH_HIEU_NEW == 1)
	case eButtonDanhHieu: //danh hieu
	{
		gCBDanhHieu.OpenWindow();
	}
	break;
#endif
#if(CB_AutoBanking)
	case eButtonNapGame: //nut nap game
	{
		gCBAutoNapGame.OpenWindowBanking();
	}
	break;
#endif
	case eButtonJewelBank: //nut jewelbank
	{
		gCBJewelBank.OpenOnOff();
	}
	break;
#if(CB_VIP_CHAR)
	case eButtonVipChar: //
	{
		if (gBInterfaceVIPChar) gBInterfaceVIPChar->CGSendOpenWinwdowVIP();
	}
	break;
#endif
#if(CB_HUYDONGEXC)
	case eButtonHuyDongExc: //nut jewelbank
	{
		gCBHuyDongExc->OpenWindow();
	}
	break;
#endif
#if(DOIMK)
	case eButtonDoiMatKhau:
	{
		gCB_DoiMK->OpenWindow();
	}
	break;
#endif
	case eButtonMocNap:
	{
		gMocDonate.OpenWindowMocNap();
	}
	break;
	
	case eButtonLockItem:
	{
		gCB_LockItem->OpenWindowLock();
	}
	break;


	case eButtonVQ:
	{
		gVongQuay.OpenVongQuay();
	}
	break;

	default:
		break;
	}
	gInterface.Data[eMenu_MAIN].OnShow = 0;
}

void cCustomMenu::Draw()
{

	if (!gInterface.Data[eMenu_MAIN].OnShow) return;

	float MainWidth = 230;
	float MainHeight = 290;
	float StartY = ((DisplayHeight - 51) / 2) - (MainHeight / 2);
	float StartX = (DisplayWin / 2) - (MainWidth / 2);

	//--
	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, MainWidth, MainHeight, eMenu_MAIN, "Menu");

	gInterface.DrawFormat(eGold, (int)StartX + 10, (int)StartY + 35, 210, 3, "%s : %s", gCustomMessage.GetMessage(50), gInterface.NumberFormat(CharacterMachine->Gold));

	gInterface.DrawFormat(eWhite, (int)StartX + 25, (int)StartY + 55, 40, 1, gCustomMessage.GetMessage(51));
	gInterface.DrawFormat(eGold, (int)StartX + 50, (int)StartY + 55, 40, 1, "%s", gInterface.NumberFormat(CharacterAttribute->PrintPlayer.Coin1));

	gInterface.DrawFormat(eWhite, (int)StartX + 90, (int)StartY + 55, 40, 1, gCustomMessage.GetMessage(52));
	gInterface.DrawFormat(eGold, (int)StartX + 115, (int)StartY + 55, 40, 1, "%s", gInterface.NumberFormat(CharacterAttribute->PrintPlayer.Coin2));

	gInterface.DrawFormat(eWhite, (int)StartX + 153, (int)StartY + 55, 40, 1, gCustomMessage.GetMessage(53));
	gInterface.DrawFormat(eGold, (int)StartX + 176, (int)StartY + 55, 40, 1, "%s", gInterface.NumberFormat(CharacterAttribute->PrintPlayer.Coin3));


	int BtCount = 0;
	int BtStartX = StartX + 8;
	int BtStartY = StartY + 78;
	int BtDistance = 33;
	float BtnW = (MainWidth - 17) / 2;
	float KhoangCachX = BtnW + 3;
	float KhoangCachY = 32;
	int cX = 0;
	int cY = 0;
	for (int i = 0; i < 15; i++)
	{
		if (!gProtect.m_MainInfo.Menu[i]) continue;

		if (g_pBCustomMenuInfo->DrawButton(BtStartX + (cX * KhoangCachX), BtStartY + (cY * KhoangCachY), 170, 12, gOther.Text_Button[i], BtnW))
		{
			this->ActionButton(i);
		}
		cX++;
		if (cX > 1)
		{
			cX = 0;
			cY++;
		}

	}

}