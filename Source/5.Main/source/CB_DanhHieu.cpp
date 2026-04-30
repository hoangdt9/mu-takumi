#include "StdAfx.h"
#include "CB_DanhHieu.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ExternalObject/curl/curl.h"
#if DANH_HIEU_NEW
#define MAX_TRANG_DANH_HIEU    5  // OK
CBDanhHieu gCBDanhHieu;

CBDanhHieu::CBDanhHieu()
{
	TRANG_DANH_HIEU = 1;
	//--------------------------------------------------------------
	HIENTHIDANH_HIEU = false;

	ClickTick = 0;
}

CBDanhHieu::~CBDanhHieu()
{

}
void CBDanhHieu::RecvProtocol(BYTE* lpMsg)
{
	if (!lpMsg) return;
	DANHHIEU_UPDATE* aRecv = (DANHHIEU_UPDATE*)lpMsg;

	m_DataDanhHieu.rCoint = aRecv->rCoint;

	m_DataDanhHieu.rCount = aRecv->rCount;
	m_DataDanhHieu.rType = aRecv->rType;
	m_DataDanhHieu.rIndex = aRecv->rIndex;

	m_DataDanhHieu.SatThuong = aRecv->SatThuong;
	m_DataDanhHieu.SatThuongX2 = aRecv->SatThuongX2;
	m_DataDanhHieu.PhongThu = aRecv->PhongThu;
	m_DataDanhHieu.HP = aRecv->HP;
	m_DataDanhHieu.SD = aRecv->SD;
	m_DataDanhHieu.GST = aRecv->GST;

	m_DataDanhHieu.DANH_HIEU = aRecv->DANH_HIEU;
}
void CBDanhHieu::OpenWindow()
{
	if (GetTickCount() - gInterface.Data[eWindowDanhHieu].EventTick > 300)
	{
		gInterface.Data[eWindowDanhHieu].EventTick = GetTickCount();
		if (gInterface.Data[eWindowDanhHieu].OnShow)
		{
			gInterface.Data[eWindowDanhHieu].OnShow = 0;
			return;
		}
		gInterface.Data[eWindowDanhHieu].OnShow = 1;
		this->BUY_DANH_HIEU(2);

	}
}
void CBDanhHieu::BUY_DANH_HIEU(int Number)
{
	BUY_DANH_HIEU_REQ pMsg;
	pMsg.h.set(0xD3, 0x03, sizeof(pMsg)); // Gửi Xuống Server Protocol.cpp
	pMsg.Number = Number;
	DataSend((BYTE*)& pMsg, pMsg.h.size);
}
//----
void CBDanhHieu::DrawWindow()
{
	//if (GetKeyState(VK_F4) & 0x4000)
	//{
	//	gCBDanhHieu.OpenWindow();
	//}

	if (gInterface.CheckWindow(Interface::ObjWindow::MoveList) || gInterface.CheckWindow(Interface::ObjWindow::CashShop) || gInterface.CheckWindow(Interface::ObjWindow::SkillTree) || gInterface.CheckWindow(Interface::ObjWindow::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)))
	{
		gInterface.Data[eWindowDanhHieu].OnShow = false;
		return;
	}

	if (!gInterface.Data[eWindowDanhHieu].OnShow)
	{
		return;
	}


	float WindowW = 480;
	float WindowH = 380;
	float StartY = ((MAX_WIN_HEIGHT - 51) / 2) - (WindowH / 2);
	float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
	//-------------------------------------------------------------------------------
	float MainCenter = StartX + (WindowW / 3);
	float ButtonX = MainCenter - (29.0 / 2);

	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowDanhHieu, gOther.Text_Chung[29]);

	StartX -= 70;
	StartY -= 30;
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 91, StartY + 79, 180, 90, 0x00000096, 0);
	//gCItemSetOption.ItemTooltipS15(StartX + 91, StartY + 79, 190, 100, 0.0, 0);
	gInterface.DrawFormat(eShinyGreen, StartX + 91 + 80, StartY + 85, 210, 1, gOther.Text_Chung[30]);
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 91, StartY + 79 + 100, 180, 200, 0x00000096, 0);
	//gCItemSetOption.ItemTooltipS15(StartX + 91, StartY + 79 + 100, 210, 210, 0.0, 0);

	gInterface.DrawFormat(eShinyGreen, StartX + 91 + 65, StartY + 85 + 100, 210, 1, gOther.Text_Chung[31]);

	gInterface.DrawFormat(eSocket, StartX + 110, StartY + 200, 210, 1, gOther.Text_Chung[32]);
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 91 + 190, StartY + 79, 245, 300, 0x00000096, 0);
	//gCItemSetOption.ItemTooltipS15(StartX + 91 + 190, StartY + 79, 255, 310, 0.0, 0);

	g_pBCustomMenuInfo->DrawInfoBox(StartX + 291, StartY + 95, 100, 40, 0x00000096, 0);
	//gCItemSetOption.ItemTooltipS15(StartX + 291, StartY + 95, 110, 50, 0.0, 0);
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 291, StartY + 150, 100, 40, 0x00000096, 0);
	//gCItemSetOption.ItemTooltipS15(StartX + 291, StartY + 150, 110, 50, 0.0, 0);
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 291, StartY + 205, 100, 40, 0x00000096, 0);
	//gCItemSetOption.ItemTooltipS15(StartX + 291, StartY + 205, 110, 50, 0.0, 0);
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 291, StartY + 260, 100, 40, 0x00000096, 0);
	//gCItemSetOption.ItemTooltipS15(StartX + 291, StartY + 260, 110, 50, 0.0, 0);
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 291, StartY + 315, 100, 40, 0x00000096, 0);
	//gCItemSetOption.ItemTooltipS15(StartX + 291, StartY + 315, 110, 50, 0.0, 0);
	//------------------------------------------------------------------------------------------------------
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 291 + 120, StartY + 95, 100, 40, 0x00000096, 0);
	//gCItemSetOption.ItemTooltipS15(StartX + 291, StartY + 95, 110, 50, 0.0, 0);
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 291 + 120, StartY + 150, 100, 40, 0x00000096, 0);
	//gCItemSetOption.ItemTooltipS15(StartX + 291, StartY + 150, 110, 50, 0.0, 0);
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 291 + 120, StartY + 205, 100, 40, 0x00000096, 0);
	//gCItemSetOption.ItemTooltipS15(StartX + 291, StartY + 205, 110, 50, 0.0, 0);
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 291 + 120, StartY + 260, 100, 40, 0x00000096, 0);
	//gCItemSetOption.ItemTooltipS15(StartX + 291, StartY + 260, 110, 50, 0.0, 0);
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 291 + 120, StartY + 315, 100, 40, 0x00000096, 0);
	//gCItemSetOption.ItemTooltipS15(StartX + 291, StartY + 315, 110, 50, 0.0, 0);

	//-----------------------------------------------------------------------------------------------------------------------------------------------------
	if (g_pBCustomMenuInfo->DrawButton(StartX + 291 + 35, StartY + 366, 55, 12, "<<"))
	{

		if (1 < TRANG_DANH_HIEU && TRANG_DANH_HIEU <= MAX_TRANG_DANH_HIEU)
		{
			TRANG_DANH_HIEU--;
		}

	}

	if (g_pBCustomMenuInfo->DrawButton(StartX + 291 + 150, StartY + 366, 55, 12, ">>"))
	{

		if (1 <= TRANG_DANH_HIEU && TRANG_DANH_HIEU < MAX_TRANG_DANH_HIEU)
		{
			TRANG_DANH_HIEU++;
		}

	}
	gInterface.DrawFormat(eGold, StartX + 291 + 105, StartY + 373, 210, 1, gOther.Text_Chung[34], TRANG_DANH_HIEU);


	if (g_pBCustomMenuInfo->DrawButton(StartX + 135, StartY + 341, 115, 12, gOther.Text_Chung[35]))
	{

		this->BUY_DANH_HIEU(1);
		if (m_DataDanhHieu.DANH_HIEU == 99)
		{
			TRANG_DANH_HIEU = 2;
		}

		if (m_DataDanhHieu.DANH_HIEU == 199)
		{
			TRANG_DANH_HIEU = 3;
		}

		if (m_DataDanhHieu.DANH_HIEU == 299)
		{
			TRANG_DANH_HIEU = 4;
		}

		if (m_DataDanhHieu.DANH_HIEU == 399)
		{
			TRANG_DANH_HIEU = 5;
		}

	}

	gInterface.DrawFormat(eWhite, StartX + 105, StartY + 105, 210, 1, gOther.Text_Chung[36]);


	//---- wcoin nâng cấp

	int v1 = m_DataDanhHieu.DANH_HIEU % 10;
	int v2 = (m_DataDanhHieu.DANH_HIEU - (v1)) / 10;

	gInterface.DrawFormat(eYellow, StartX + 145, StartY + 105, 210, 1, "%s WCoinC", gInterface.NumberFormat(m_DataDanhHieu.rCoint));
	gInterface.DrawFormat(eYellow, StartX + 145, StartY + 125, 210, 1, ": x%d (viên)", m_DataDanhHieu.rCount);

	g_pNewUISystem->RenderItem3DFree(StartX + 135, StartY + 105, 0, 0, GET_ITEM(m_DataDanhHieu.rType, m_DataDanhHieu.rIndex), 0, 1, 0, 0);


	gInterface.DrawFormat(eYellow, StartX + 110, StartY + 220, 300, 1, gOther.Text_Chung[37], m_DataDanhHieu.SatThuong);
	gInterface.DrawFormat(eYellow, StartX + 110, StartY + 235, 300, 1, gOther.Text_Chung[38], m_DataDanhHieu.SatThuongX2);
	gInterface.DrawFormat(eYellow, StartX + 110, StartY + 250, 300, 1, gOther.Text_Chung[39], m_DataDanhHieu.PhongThu);
	gInterface.DrawFormat(eYellow, StartX + 110, StartY + 265, 300, 1, gOther.Text_Chung[40], m_DataDanhHieu.HP);
	gInterface.DrawFormat(eYellow, StartX + 110, StartY + 280, 300, 1, gOther.Text_Chung[41], m_DataDanhHieu.SD);
	gInterface.DrawFormat(eYellow, StartX + 110, StartY + 295, 300, 1, gOther.Text_Chung[42], m_DataDanhHieu.GST);

	gInterface.DrawFormat(eSocket, StartX + 215, StartY + 105, 210, 1, "Cấp %d", v2);
	EnableAlphaTest(true);
	glColor3f(1.0, 1.0, 1.0);
	switch (TRANG_DANH_HIEU)
	{
	case 1:

		if (v2 >= 1)
		{
			//pDrawMapObject(531145, 1, StartX -115, StartY + 125, 135, 0, 0, 0, 0, 0, 33, 1.0, 1.0, 0); // doan ve anh thanh cong
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 2)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 3)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);

		}


		if (v2 >= 4)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 5)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);

		}


		if (v2 >= 6)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 7)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 8)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 9)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 10)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		for (int zI = 0; zI < 5; zI++)
		{

			RenderBitmap(gInterface.Data[eDanhImg+ zI].ModelID, StartX + 294, StartY + 95 + 55 * zI, 92, 40, 0, 0, 26.f / 40.f, 29.f / 40.f, 1, 1, v2 == (zI + 1) ? 0.0 : 0.35);

			RenderBitmap(gInterface.Data[eDanhImg+5 + zI].ModelID, StartX + 294 + 120, StartY + 95 + 55 * zI, 146, 45, 0, 0, 1, 1, 1, 1, v2 == (zI + 6) ? 0.0 : 0.35);
		}

		if (v2 < 1)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 1)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 2)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 3)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 4)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 5)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 6)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 7)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 8)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 9)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		break;
	case 2:
		if (v2 >= 11)
		{
			//pDrawMapObject(531145, 1, StartX -115, StartY + 125, 135, 0, 0, 0, 0, 0, 33, 1.0, 1.0, 0); // doan ve anh thanh cong
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 12)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 13)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);

		}


		if (v2 >= 14)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 15)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);

		}


		if (v2 >= 16)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 17)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 18)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 19)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 20)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}

		for (int zI = 0; zI < 5; zI++)
		{

			RenderBitmap(gInterface.Data[eDanhImg+10 + zI ].ModelID, StartX + 294, StartY + 95 + 55 * zI, 146, 45, 0, 0, 1, 1, 1, 1, v2 == (zI + 11) ? 0.0 : 0.35);

			RenderBitmap(gInterface.Data[eDanhImg+15 + zI].ModelID, StartX + 294 + 120, StartY + 95 + 55 * zI, 146, 45, 0, 0, 1, 1, 1, 1, v2 == (zI + 16) ? 0.0 : 0.35);
		}

		if (v2 == 10)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 11)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 12)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 13)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 14)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 15)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 16)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 17)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 18)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 19)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		break;
	case 3:
		if (v2 >= 21)
		{
			//pDrawMapObject(531145, 1, StartX -115, StartY + 125, 135, 0, 0, 0, 0, 0, 33, 1.0, 1.0, 0); // doan ve anh thanh cong
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 22)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 23)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);

		}


		if (v2 >= 24)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 25)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);

		}


		if (v2 >= 26)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 27)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 28)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 29)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 30)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		for (int zI = 0; zI < 5; zI++)
		{

			RenderBitmap(gInterface.Data[eDanhImg+20 + zI].ModelID, StartX + 294, StartY + 95 + 55 * zI, 146, 45, 0, 0, 1, 1, 1, 1, v2 == (zI + 21) ? 0.0 : 0.35);

			RenderBitmap(gInterface.Data[eDanhImg+25 + zI].ModelID, StartX + 294 + 120, StartY + 95 + 55 * zI, 146, 45, 0, 0, 1, 1, 1, 1, v2 == (zI + 26) ? 0.0 : 0.35);
		}

		if (v2 == 20)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 21)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 22)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 23)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 24)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 25)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 26)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 27)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 28)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 29)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		break;
	case 4:
		if (v2 >= 31)
		{
			//pDrawMapObject(531145, 1, StartX -115, StartY + 125, 135, 0, 0, 0, 0, 0, 33, 1.0, 1.0, 0); // doan ve anh thanh cong
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 32)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 33)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);

		}


		if (v2 >= 34)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 35)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);

		}


		if (v2 >= 36)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 37)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 38)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 39)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 40)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		for (int zI = 0; zI < 5; zI++)
		{

			RenderBitmap(gInterface.Data[eDanhImg+30 + zI].ModelID, StartX + 294, StartY + 95 + 55 * zI, 146, 45, 0, 0, 1, 1, 1, 1, v2 == (zI + 31) ? 0.0 : 0.35);

			RenderBitmap(gInterface.Data[eDanhImg+35 + zI ].ModelID, StartX + 294 + 120, StartY + 95 + 55 * zI, 146, 45, 0, 0, 1, 1, 1, 1, v2 == (zI + 36) ? 0.0 : 0.35);
		}

		if (v2 == 30)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 31)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 32)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 33)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 34)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 35)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 36)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 37)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 38)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 39)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		break;
	case 5:


		if (v2 >= 41)
		{
			//pDrawMapObject(531145, 1, StartX -115, StartY + 125, 135, 0, 0, 0, 0, 0, 33, 1.0, 1.0, 0); // doan ve anh thanh cong
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 42)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 43)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);

		}


		if (v2 >= 44)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 45)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);

		}


		if (v2 >= 46)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 47)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 48)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 49)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		if (v2 >= 50)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, gOther.Text_Chung[44]);
		}


		for (int zI = 0; zI < 5; zI++)
		{

			RenderBitmap(gInterface.Data[eDanhImg + 40 + zI].ModelID, StartX + 294, StartY + 95 + 55 * zI, 146, 45, 0, 0, 1, 1, 1, 1, v2 == (zI + 41) ? 0.0 : 0.35);

			RenderBitmap(gInterface.Data[eDanhImg + 45 + zI].ModelID, StartX + 294 + 120, StartY + 95 + 55 * zI, 146, 45, 0, 0, 1, 1, 1, 1, v2 == (zI + 46) ? 0.0 : 0.35);
		}

		if (v2 == 40)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 41)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 42)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 43)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 44)
		{
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 45)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 125, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 46)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 175, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 47)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 225, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 48)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 275, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		if (v2 == 49)
		{
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0x0, 0xcccccc80, 100, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0x0, 0x22ff2280, 10 * v1, 7, 0, " ");
			TextDraw((HFONT)g_hFont, StartX + 294 + 120, StartY + 325, 0xffffffff, 0x0, 100, 15, 3, "%d%%", v1 * 10);
		}
		break;
	case 6:
		break;
	case 7:
		break;
	case 8:
		break;
	case 9:
		break;
	case 10:
		break;
	default:
		break;
	}
	glColor3f(1.0f, 1.0f, 1.0f); //Xoa mau
	EnableAlphaTest(0);
}
#endif