#include "StdAfx.h"
#include "CB_ViewCharItem.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"

#if(CB_VIEWCHARITEM)
CB_ViewCharItem* gCB_ViewCharItem;

CB_ViewCharItem::CB_ViewCharItem()
{
#if(HAISLOTRING)
	memset(&m_ViewCharItemSlots, 0, sizeof(EQUIPMENT_ITEM) * EQUIPMENT_PENTA);
#else
	memset(&m_ViewCharItemSlots, 0, sizeof(EQUIPMENT_ITEM) * MAX_EQUIPMENT_INDEX);
#endif
	this->ClearData();
}


CB_ViewCharItem::~CB_ViewCharItem()
{
}
void CB_ViewCharItem::SetImgSlot(float x, float y)
{
	m_ViewCharItemSlots[EQUIPMENT_HELPER].x = x + 15;
	m_ViewCharItemSlots[EQUIPMENT_HELPER].y = y + 44;
	m_ViewCharItemSlots[EQUIPMENT_HELPER].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_HELPER].height = 46;
	m_ViewCharItemSlots[EQUIPMENT_HELPER].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_FAIRY;

	m_ViewCharItemSlots[EQUIPMENT_HELM].x = x + 75;
	m_ViewCharItemSlots[EQUIPMENT_HELM].y = y + 44;
	m_ViewCharItemSlots[EQUIPMENT_HELM].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_HELM].height = 46;
	m_ViewCharItemSlots[EQUIPMENT_HELM].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_HELM;

	m_ViewCharItemSlots[EQUIPMENT_WING].x = x + 120;
	m_ViewCharItemSlots[EQUIPMENT_WING].y = y + 44;
	m_ViewCharItemSlots[EQUIPMENT_WING].width = 61;
	m_ViewCharItemSlots[EQUIPMENT_WING].height = 46;
	m_ViewCharItemSlots[EQUIPMENT_WING].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_WING;

	m_ViewCharItemSlots[EQUIPMENT_WEAPON_LEFT].x = x + 135;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_LEFT].y = y + 87;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_LEFT].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_LEFT].height = 66;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_LEFT].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_LEFT;

	m_ViewCharItemSlots[EQUIPMENT_ARMOR].x = x + 75;
	m_ViewCharItemSlots[EQUIPMENT_ARMOR].y = y + 87;
	m_ViewCharItemSlots[EQUIPMENT_ARMOR].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_ARMOR].height = 66;
	m_ViewCharItemSlots[EQUIPMENT_ARMOR].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_ARMOR;

	m_ViewCharItemSlots[EQUIPMENT_WEAPON_RIGHT].x = x + 15;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_RIGHT].y = y + 87;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_RIGHT].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_RIGHT].height = 66;
	m_ViewCharItemSlots[EQUIPMENT_WEAPON_RIGHT].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_RIGHT;

	m_ViewCharItemSlots[EQUIPMENT_GLOVES].x = x + 15;
	m_ViewCharItemSlots[EQUIPMENT_GLOVES].y = y + 150;
	m_ViewCharItemSlots[EQUIPMENT_GLOVES].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_GLOVES].height = 46;
	m_ViewCharItemSlots[EQUIPMENT_GLOVES].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_GLOVES;

	m_ViewCharItemSlots[EQUIPMENT_PANTS].x = x + 75;
	m_ViewCharItemSlots[EQUIPMENT_PANTS].y = y + 150;
	m_ViewCharItemSlots[EQUIPMENT_PANTS].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_PANTS].height = 46;
	m_ViewCharItemSlots[EQUIPMENT_PANTS].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_PANTS;

	m_ViewCharItemSlots[EQUIPMENT_BOOTS].x = x + 135;
	m_ViewCharItemSlots[EQUIPMENT_BOOTS].y = y + 150;
	m_ViewCharItemSlots[EQUIPMENT_BOOTS].width = 46;
	m_ViewCharItemSlots[EQUIPMENT_BOOTS].height = 46;
	m_ViewCharItemSlots[EQUIPMENT_BOOTS].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_BOOT;

	m_ViewCharItemSlots[EQUIPMENT_RING_LEFT].x = x + 114;
	m_ViewCharItemSlots[EQUIPMENT_RING_LEFT].y = y + 150;
	m_ViewCharItemSlots[EQUIPMENT_RING_LEFT].width = 28;
	m_ViewCharItemSlots[EQUIPMENT_RING_LEFT].height = 28;
	m_ViewCharItemSlots[EQUIPMENT_RING_LEFT].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_RING;

	m_ViewCharItemSlots[EQUIPMENT_AMULET].x = x + 54;
	m_ViewCharItemSlots[EQUIPMENT_AMULET].y = y + 87;
	m_ViewCharItemSlots[EQUIPMENT_AMULET].width = 28;
	m_ViewCharItemSlots[EQUIPMENT_AMULET].height = 28;
	m_ViewCharItemSlots[EQUIPMENT_AMULET].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_NECKLACE;

	m_ViewCharItemSlots[EQUIPMENT_RING_RIGHT].x = x + 54;
	m_ViewCharItemSlots[EQUIPMENT_RING_RIGHT].y = y + 150;
	m_ViewCharItemSlots[EQUIPMENT_RING_RIGHT].width = 28;
	m_ViewCharItemSlots[EQUIPMENT_RING_RIGHT].height = 28;
	m_ViewCharItemSlots[EQUIPMENT_RING_RIGHT].dwBgImage = CNewUIMyInventory::IMAGE_INVENTORY_ITEM_RING;

#if(HAISLOTRING)
	if (gProtect.m_MainInfo.EnableSlotPet)
	{
		m_ViewCharItemSlots[EQUIPMENT_PET_1].x = x + 54;
		m_ViewCharItemSlots[EQUIPMENT_PET_1].y = y + 62;
		m_ViewCharItemSlots[EQUIPMENT_PET_1].width = 28;
		m_ViewCharItemSlots[EQUIPMENT_PET_1].height = 28;
		m_ViewCharItemSlots[EQUIPMENT_PET_1].dwBgImage = 891552;
	}
#endif

}
void CB_ViewCharItem::ClearData()
{
#if(HAISLOTRING)
	for (int i = 0; i < EQUIPMENT_PENTA; ++i)
#else
	for (int i = 0; i < MAX_EQUIPMENT; ++i)
#endif
	{
		this->CharItemInfo[i].Type = -1;
		this->CharItemInfo[i].Level = 0;
		this->CharItemInfo[i].Option1 = 0;
	}
	this->CharViewName.clear();

	this->m_CharItemInfo.clear();
}

void CB_ViewCharItem::RecvProtocol(BYTE* Recv)
{
	if (!Recv) return;
	PMSG_COUNTLIST_VIEWCHAR* mRecv = (PMSG_COUNTLIST_VIEWCHAR*)Recv;
	this->ClearData();
	this->CharViewName = mRecv->Name;
	for (int n = 0; n < mRecv->Count; n++)
	{

		GETINFOCHAR_DATA lpInfo = *(GETINFOCHAR_DATA*)(((BYTE*)Recv) + sizeof(PMSG_COUNTLIST_VIEWCHAR) + (sizeof(GETINFOCHAR_DATA) * n));
#if(HAISLOTRING)
		if (n > EQUIPMENT_PENTA) break;
#else
		if (n > MAX_EQUIPMENT) break;
#endif
		//infoItemLocal.Item = *((ObjectItem * (__thiscall*)(int, BYTE*)) 0x007E1B10)(*(DWORD*)(((int(*)()) 0x861110)() + 36), lpInfo.Item);
		memcpy(&this->CharItemInfo[n], g_pNewItemMng->CreateItem(lpInfo.Item), sizeof(this->CharItemInfo[n]));
		this->CharItemInfo[n].Durability = lpInfo.Dur;

		if (lpInfo.PeriodTime)
		{
			this->CharItemInfo[n].bPeriodItem = 1;
			this->CharItemInfo[n].lExpireTime = lpInfo.PeriodTime;
		}

		//gInterface.DrawMessage(1, "%d %d",n, this->CharItemInfo[n].Type);
	}
	gInterface.Data[eWindowViewItemChar].OnShow = 1;
	//gInterface.DrawMessage(1, "Recv Info Char %s", this->CharViewName.c_str());
}
void CB_ViewCharItem::SendRequestViewItem(int aIndex)
{
	if (aIndex != -1)
	{
		//gInterface.DrawMessage(1, "SendRequestViewItem %d", aIndex);

		XULY_CGPACKET pMsg;
		pMsg.header.set(0xD3, 0x02, sizeof(pMsg));
		pMsg.ThaoTac = aIndex;
		DataSend((LPBYTE)& pMsg, pMsg.header.size);
	}
}
void CB_ViewCharItem::OpenClose()
{
	if (GetTickCount() - gInterface.Data[eWindowViewItemChar].EventTick > 300)
	{
		gInterface.Data[eWindowViewItemChar].EventTick = GetTickCount();
		if (gInterface.Data[eWindowViewItemChar].OnShow)
		{
			gInterface.Data[eWindowViewItemChar].OnShow = 0;
			return;
		}
		//==Show hoac send packet open
		gInterface.Data[eWindowViewItemChar].OnShow = 1;
	}
}

void CB_ViewCharItem::DrawWindow()
{
	//=== Test Open Window
	//if (GetKeyState(VK_F3) & 0x4000 && GetTickCount() > gInterface.Data[eWindowViewItemChar].EventTick + 300)
	//{
	//	this->OpenClose(); // << Ham Goi Window
	//	gInterface.Data[eWindowViewItemChar].EventTick = GetTickCount();
	//}

	if (gInterface.CheckWindow(Interface::MoveList) || gInterface.CheckWindow(Interface::ObjWindow::CashShop) || gInterface.CheckWindow(Interface::ObjWindow::SkillTree) || gInterface.CheckWindow(Interface::ObjWindow::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)))
	{
		gInterface.Data[eWindowViewItemChar].OnShow = false;
		return;
	}

	if (!gInterface.Data[eWindowViewItemChar].OnShow)
	{
		return;
	}
	float CuaSoW = 190;
	float CuaSoH = 220;
	float StartX = 10;
	float StartY = 30;

	static bool SetPosCache = false;
	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, CuaSoW, CuaSoH, eWindowViewItemChar, "[%s]", this->CharViewName.c_str());
	if (gInterface.Data[eWindowViewItemChar].OnClick || !SetPosCache)
	{
		this->SetImgSlot(StartX, StartY);
		SetPosCache = true;
	}
	int NumberInfo = -1;
#if(HAISLOTRING)
	for (int i = 0; i < EQUIPMENT_PENTA; i++)
#else
	for (int i = 0; i < MAX_EQUIPMENT_INDEX; i++)
#endif
	{
		//if (i == EQUIPMENT_HELM)
		//{
		//	if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK)
		//	{
		//		continue;
		//	}
		//}
		//if ((i == EQUIPMENT_GLOVES) && (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER))
		//	continue;

		EnableAlphaTest();

		RenderImage(m_ViewCharItemSlots[i].dwBgImage, m_ViewCharItemSlots[i].x, m_ViewCharItemSlots[i].y,
			m_ViewCharItemSlots[i].width, m_ViewCharItemSlots[i].height);


		DisableAlphaBlend();

		//==Render Item 3d
		if (CharItemInfo[i].Type != 0x1FFF)
		{
			float y = 0.f;
			if (i == EQUIPMENT_ARMOR)
			{
				y = m_ViewCharItemSlots[i].y - 10.f;
			}
			else
			{
				y = m_ViewCharItemSlots[i].y;
			}

			glColor4f(1.f, 1.f, 1.f, 1.f);
			g_pNewUISystem->RenderItem3DFree(
				m_ViewCharItemSlots[i].x + 1,
				y,
				m_ViewCharItemSlots[i].width - 4,
				m_ViewCharItemSlots[i].height - 4,
				CharItemInfo[i].Type,
				CharItemInfo[i].Level,
				CharItemInfo[i].Option1,
				CharItemInfo[i].ExtOption,
				false, 1.0, false);
			

		}
		if (SEASON3B::CheckMouseIn(m_ViewCharItemSlots[i].x, m_ViewCharItemSlots[i].y,
			m_ViewCharItemSlots[i].width, m_ViewCharItemSlots[i].height))
		{
			NumberInfo = i;
		}
	}

	if (NumberInfo != -1 /*&& !gInterface.Data[eWindowChotroiRaoBan].Hover*/)
	{

		RenderItemInfo(MouseX + 75, MouseY, &CharItemInfo[NumberInfo], 0, 0, false, false);
	}
}
#endif