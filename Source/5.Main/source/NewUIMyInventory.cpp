// NewUIMyInventory.cpp: implementation of the CNewUIMyInventory class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIMyInventory.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "CSChaosCastle.h"
#include "GOBoid.h"
#include "ZzzEffect.h"
#include "GIPetManager.h"
#include "CSParts.h"
#include "UIJewelHarmony.h"
#include "CDirection.h"
#include "GMCryWolf1st.h"
#include "GMCryingWolf2nd.h"
#include "ZzzInventory.h"
#include "wsclientinline.h"
#include "MixMgr.h"
#include "ZzzLodTerrain.h"
#include "CSQuest.h"
#include "UIGuildInfo.h"
#include "UIManager.h"
#include "CSItemOption.h"
#include "MapManager.h"
#include "w_PetProcess.h"
#include "SocketSystem.h"
#include "w_CursedTemple.h"
#include "PortalMgr.h"
#ifdef CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
#include "Event.h"
#endif // CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
#include "GMUnitedMarketPlace.h"
#include "ChangeRingManager.h"
#include "MonkSystem.h"
#include "CharacterManager.h"
#include "Protocol.h"
#include "CBInterface.h"
#include "Util.h"
#include "CUIController.h"
#include "MenuCustom.h"
extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayWinMid;
extern int DisplayWinExt;
extern int DisplayWinReal;
extern int DisplayHeightExt;
using namespace SEASON3B;

CNewUIMyInventory::CNewUIMyInventory()
{
	m_pNewUIMng = nullptr;
	m_pNewUI3DRenderMng = nullptr;
	m_pNewInventoryCtrl = nullptr;
	m_Pos.x = m_Pos.y = 0;

	memset(&m_EquipmentSlots, 0, sizeof(EQUIPMENT_ITEM) * MAX_EQUIPMENT_INDEX);
#if(HAISLOTRING)
	memset(&m_EquipmentSlotsMuun, 0, sizeof(EQUIPMENT_ITEM)*16);
	m_iPointedSlotMuun = -1;
#endif
	m_iPointedSlot = -1;

	m_MyShopMode = MYSHOP_MODE_OPEN;
	m_RepairMode = REPAIR_MODE_OFF;
	m_dwStandbyItemKey = 0;

	m_bRepairEnableLevel = false;
	m_bMyShopOpen = false;
}

CNewUIMyInventory::~CNewUIMyInventory()
{
	Release();
}

bool CNewUIMyInventory::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng, int x, int y)
{
	if (nullptr == pNewUIMng || nullptr == pNewUI3DRenderMng || nullptr == g_pNewItemMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(INTERFACE_INVENTORY, this);

	m_pNewUI3DRenderMng = pNewUI3DRenderMng;
	m_pNewUI3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

	m_pNewInventoryCtrl = new CNewUIInventoryCtrl;
	if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::INVENTORY, m_pNewUI3DRenderMng, g_pNewItemMng, this, x + 15, y + 200, 8, 8, MAX_EQUIPMENT))
	{
		SAFE_DELETE(m_pNewInventoryCtrl);
		return false;
	}

	SetPos(x, y);
	LoadImages();
	SetEquipmentSlotInfo();
	SetButtonInfo();
	Show(false);
	return true;
}

void CNewUIMyInventory::Release()
{
	if (m_pNewUI3DRenderMng)
		m_pNewUI3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);

	UnequipAllItems();
	DeleteAllItems();

	UnloadImages();

	SAFE_DELETE(m_pNewInventoryCtrl);

	if (m_pNewUI3DRenderMng)
	{
		m_pNewUI3DRenderMng->Remove3DRenderObj(this);
		m_pNewUI3DRenderMng = nullptr;
	}
	if (m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = nullptr;
	}
}

bool CNewUIMyInventory::EquipItem(int iIndex, BYTE* pbyItemPacket)
{
#if(HAISLOTRING)
	if (iIndex >= 12 && iIndex < 16 && g_pNewItemMng && CharacterMachine)
	{
		ITEM* pTargetItemSlotNew = &CharacterMachine->EquipmentMuun[iIndex];
		if (pTargetItemSlotNew->Type > 0)
		{
			UnequipItem(iIndex);
		}
		ITEM* pTempItemNew = g_pNewItemMng->CreateItem(pbyItemPacket);
		if (nullptr != pTempItemNew)
		{
			pTempItemNew->lineal_pos = iIndex;
			pTempItemNew->ex_src_type = ITEM_EX_SRC_EQUIPMENT2;
			if (iIndex == 12)
			{
				int CacheSlot = Hero->m_PetInfo[PET_TYPE_END].m_dwPetType;
				//char Text[100];
				//sprintf(Text, "EquipItem Muun %d %d", pTempItemNew->Type + 1171, Hero->m_PetInfo[PET_TYPE_END].m_dwPetType);
				//CreateNotice(Text, 1);
				//DeleteBug(&Hero->Object);
				//*(WORD*)(Hero + 592) = pTempItemNew->Type + 1171;
				Hero->m_PetInfo[PET_TYPE_END].m_dwPetType = pTempItemNew->Type + 1171;
				if(CacheSlot == 66706) CreateEffectEquipping(pTempItemNew->Type, Hero, &Hero->Object);

			}
			memcpy(pTargetItemSlotNew, pTempItemNew, sizeof(ITEM));
			g_pNewItemMng->DeleteItem(pTempItemNew);
		}
		return true;
	}
#endif
	if (iIndex >= 0 && iIndex < MAX_EQUIPMENT_INDEX && g_pNewItemMng && CharacterMachine)
	{
		ITEM* pTargetItemSlot = &CharacterMachine->Equipment[iIndex];
		if (pTargetItemSlot->Type > 0)
		{
			UnequipItem(iIndex);
		}

		ITEM* pTempItem = g_pNewItemMng->CreateItem(pbyItemPacket);

		if (nullptr == pTempItem)
			//if(NULL == pTempItem || false == IsEquipable(iIndex, pTempItem))
		{
			return false;
		}

		if (pTempItem->Type == ITEM_HELPER + 4)
			SendRequestPetInfo(PET_TYPE_DARK_HORSE, 0, iIndex);

		if (pTempItem->Type == ITEM_HELPER + 5)
		{
			CreatePetDarkSpirit(Hero);
			SendRequestPetInfo(PET_TYPE_DARK_SPIRIT, 0, iIndex);
		}

		pTempItem->lineal_pos = iIndex;
		pTempItem->ex_src_type = ITEM_EX_SRC_EQUIPMENT;
		memcpy(pTargetItemSlot, pTempItem, sizeof(ITEM));
		g_pNewItemMng->DeleteItem(pTempItem);

		CreateEquippingEffect(pTargetItemSlot);
	}
//#if(HAISLOTRING)
//	if(Hero->m_PetInfo[PET_TYPE_END].m_dwPetType != 0xFFFF)
//	{
//		CreateEffectEquipping(Hero->m_PetInfo[PET_TYPE_END].m_dwPetType-1171, Hero, &Hero->Object);
//	}
//#endif

	return false;
}
void CNewUIMyInventory::UnequipItem(int iIndex)
{
#if(HAISLOTRING)
	if (iIndex >= 12 && iIndex < 16 && g_pNewItemMng && CharacterMachine)
	{
		ITEM* pEquippedItem = &CharacterMachine->EquipmentMuun[iIndex];
		pEquippedItem->Type = -1;
		pEquippedItem->Level = 0;
		pEquippedItem->Number = -1;
		pEquippedItem->Option1 = 0;
		pEquippedItem->ExtOption = 0;
		pEquippedItem->SocketCount = 0;
		for (int i = 0; i < MAX_SOCKETS; ++i)
		{
			pEquippedItem->SocketSeedID[i] = SOCKET_EMPTY;
			pEquippedItem->SocketSphereLv[i] = 0;
		}
		pEquippedItem->SocketSeedSetOption = 0;
		if (iIndex == 12)
		{
			DeleteBug(&Hero->Object);
			Hero->m_PetInfo[PET_TYPE_END].m_dwPetType = 0xFFFF;
		}
		return;
	}
#endif

	if (iIndex >= 0 && iIndex < MAX_EQUIPMENT_INDEX && g_pNewItemMng && CharacterMachine)
	{
		ITEM* pEquippedItem = &CharacterMachine->Equipment[iIndex];

		if (pEquippedItem && pEquippedItem->Type != -1)
		{
			if (pEquippedItem->Type == ITEM_HELPER + 4)
			{
				Hero->InitPetInfo(PET_TYPE_DARK_HORSE);
			}
			else if (pEquippedItem->Type == ITEM_HELPER + 5)
			{
				DeletePet(Hero);
				Hero->InitPetInfo(PET_TYPE_DARK_SPIRIT);
			}

			if (pEquippedItem->Type != ITEM_HELPER + 5)
				DeleteEquippingEffectBug(pEquippedItem);

			pEquippedItem->Type = -1;
			pEquippedItem->Level = 0;
			pEquippedItem->Number = -1;
			pEquippedItem->Option1 = 0;
			pEquippedItem->ExtOption = 0;
			pEquippedItem->SocketCount = 0;
			for (int i = 0; i < MAX_SOCKETS; ++i)
			{
				pEquippedItem->SocketSeedID[i] = SOCKET_EMPTY;
				pEquippedItem->SocketSphereLv[i] = 0;
			}
			pEquippedItem->SocketSeedSetOption = 0;
			DeleteEquippingEffect();
		}
	}

#if(HAISLOTRING)
	if (Hero->m_PetInfo[PET_TYPE_END].m_dwPetType != 0xFFFF && iIndex == 8)
	{
		CreateEffectEquipping(Hero->m_PetInfo[PET_TYPE_END].m_dwPetType - 1171, Hero, &Hero->Object);
	}
#endif
}
void CNewUIMyInventory::UnequipAllItems()
{
	if (CharacterMachine)
	{
		for (int i = 0; i < MAX_EQUIPMENT_INDEX; i++)
		{
			UnequipItem(i);
		}
	}
}

bool CNewUIMyInventory::IsEquipable(int iIndex, ITEM* pItem)
{
	if (pItem == nullptr)
		return false;

	const ITEM_ATTRIBUTE * pItemAttr = &ItemAttribute[pItem->Type];
	bool bEquipable = false;
	if (pItemAttr->RequireClass[gCharacterManager.GetBaseClass(Hero->Class)])
		bEquipable = true;

	else if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK && pItemAttr->RequireClass[CLASS_WIZARD]
		&& pItemAttr->RequireClass[CLASS_KNIGHT])
		bEquipable = true;

	const BYTE byFirstClass = gCharacterManager.GetBaseClass(Hero->Class);
	const BYTE byStepClass = gCharacterManager.GetStepClass(Hero->Class);
	if (pItemAttr->RequireClass[byFirstClass] > byStepClass)
	{
		return false;
	}

	if (bEquipable == false)
		return false;

	bEquipable = false;
	if (pItemAttr->m_byItemSlot == iIndex)
		bEquipable = true;

	else if (pItemAttr->m_byItemSlot == EQUIPMENT_WEAPON_RIGHT && iIndex == EQUIPMENT_WEAPON_LEFT)
	{
		if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK
			|| gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
		{
			if (!pItemAttr->TwoHand)
				bEquipable = true;
#ifdef PBG_FIX_EQUIP_TWOHANDSWORD
			else
			{
				bEquipable = false;
				return false;
			}
#endif //PBG_FIX_EQUIP_TWOHANDSWORD
		}
		else if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_SUMMONER &&
			!(pItem->Type >= ITEM_STAFF && pItem->Type <= ITEM_STAFF + MAX_ITEM_INDEX))
			bEquipable = true;
	}
	else if (pItemAttr->m_byItemSlot == EQUIPMENT_RING_RIGHT && iIndex == EQUIPMENT_RING_LEFT)
		bEquipable = true;

	if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF)
	{
		//ITEM *r = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
		const ITEM* l = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
		if (iIndex == EQUIPMENT_WEAPON_RIGHT && l->Type != ITEM_BOW + 7 //CustomArrow
			&& (l->Type >= ITEM_BOW && l->Type < ITEM_BOW + MAX_ITEM_INDEX))
		{
			if (pItem->Type != ITEM_BOW + 15)//CustomArrow
				bEquipable = false;
		}
	}

	if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
	{
		//== Fix RF Mang gloves
		//if (iIndex == EQUIPMENT_GLOVES)
		//	bEquipable = false;
		
		if (pItemAttr->m_byItemSlot == EQUIPMENT_WEAPON_RIGHT)
			bEquipable = g_CMonkSystem.RageEquipmentWeapon(iIndex, pItem->Type);
	}

	if (bEquipable == false)
		return false;

	const WORD wStrength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
	const WORD wDexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
	const WORD wEnergy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
	const WORD wVitality = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
	const WORD wCharisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
	const WORD wLevel = CharacterAttribute->Level;

	const int iItemLevel = (pItem->Level >> 3) & 15;

	int iDecNeedStrength = 0, iDecNeedDex = 0;

	extern JewelHarmonyInfo * g_pUIJewelHarmonyinfo;
	if (iItemLevel >= pItem->Jewel_Of_Harmony_OptionLevel)
	{
		StrengthenCapability SC;
		g_pUIJewelHarmonyinfo->GetStrengthenCapability(&SC, pItem, 0);

		if (SC.SI_isNB)
		{
			iDecNeedStrength = SC.SI_NB.SI_force;
			iDecNeedDex = SC.SI_NB.SI_activity;
		}
	}
	if (pItem->SocketCount > 0)
	{
		for (int i = 0; i < pItem->SocketCount; ++i)
		{
			if (pItem->SocketSeedID[i] == 38)
			{
				const int iReqStrengthDown = g_SocketItemMgr.GetSocketOptionValue(pItem, i);
				iDecNeedStrength += iReqStrengthDown;
			}
			else if (pItem->SocketSeedID[i] == 39)
			{
				const int iReqDexterityDown = g_SocketItemMgr.GetSocketOptionValue(pItem, i);
				iDecNeedDex += iReqDexterityDown;
			}
		}
	}

	if (pItem->RequireStrength - iDecNeedStrength > wStrength)
		return false;
	if (pItem->RequireDexterity - iDecNeedDex > wDexterity)
		return false;
	if (pItem->RequireEnergy > wEnergy)
		return false;
	if (pItem->RequireVitality > wVitality)
		return false;
	if (pItem->RequireCharisma > wCharisma)
		return false;
	if (pItem->RequireLevel > wLevel)
		return false;

	if (pItem->Type == ITEM_HELPER + 5)
	{
		const PET_INFO* pPetInfo = GetPetInfo(pItem);
		const WORD wRequireCharisma = (185 + (pPetInfo->m_wLevel * 15));
		if (wRequireCharisma > wCharisma)
			return false;
	}

	if (gMapManager.WorldActive == WD_7ATLANSE && (pItem->Type >= ITEM_HELPER + 2 && pItem->Type <= ITEM_HELPER + 3))
	{
		return false;
	}
	if (pItem->Type == ITEM_HELPER + 2 && gMapManager.WorldActive == WD_10HEAVEN)
	{
		return false;
	}
	if (pItem->Type == ITEM_HELPER + 2 && g_Direction.m_CKanturu.IsMayaScene())
	{
		return false;
	}
	if (gMapManager.InChaosCastle() || (Get_State_Only_Elf()
		&& g_isCharacterBuff((&Hero->Object), eBuff_CrywolfHeroContracted)))
	{
		if ((pItem->Type >= ITEM_HELPER + 2 && pItem->Type <= ITEM_HELPER + 5) || pItem->Type == ITEM_HELPER + 37 || gCustomPet2.GetInfoPetType(pItem->Type) == 5 || gCustomPet2.GetInfoPetType(pItem->Type) == 6)
			return false;
	}
	else if ((pItem->Type >= ITEM_HELPER + 2 && pItem->Type <= ITEM_HELPER + 4 || pItem->Type == ITEM_HELPER + 37 || gCustomPet2.GetInfoPetType(pItem->Type) == 5 || gCustomPet2.GetInfoPetType(pItem->Type) == 6)
		&& Hero->Object.CurrentAction >= PLAYER_SIT1 && Hero->Object.CurrentAction <= PLAYER_SIT_FEMALE2)
	{
		return false;
	}

	return bEquipable;
}

bool CNewUIMyInventory::InsertItem(int iIndex, BYTE * pbyItemPacket) const
{
	if (m_pNewInventoryCtrl)
	{
		return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
	}

	return false;
}
void CNewUIMyInventory::DeleteItem(int iIndex) const
{
	if (m_pNewInventoryCtrl)
	{
		ITEM* pItem = m_pNewInventoryCtrl->FindItem(iIndex);
		if (pItem != nullptr)
		{
			m_pNewInventoryCtrl->RemoveItem(pItem);
		}
		else
		{
			CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
			if (pPickedItem)
			{
				if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
				{
					if (pPickedItem->GetSourceLinealPos() == iIndex)
					{
						CNewUIInventoryCtrl::DeletePickedItem();
					}
				}
			}
		}
	}
}
void CNewUIMyInventory::DeleteAllItems() const
{
	if (m_pNewInventoryCtrl)
		m_pNewInventoryCtrl->RemoveAllItems();
}

void CNewUIMyInventory::SetPos(int x, int y)
{
	x = x + DisplayWinCDepthBox;
	m_Pos.x = x;
	m_Pos.y = y;

	SetEquipmentSlotInfo();

	m_pNewInventoryCtrl->SetPos(x + 15, y + 200);
	m_BtnExit.SetPos(m_Pos.x + 13, m_Pos.y + 391);
	m_BtnRepair.SetPos(m_Pos.x + 50, m_Pos.y + 391);
	m_BtnMyShop.SetPos(m_Pos.x + 87, m_Pos.y + 391);
	m_BtnExpand.SetPos(m_Pos.x + 87 + 37, m_Pos.y + 391);
}

const POINT & CNewUIMyInventory::GetPos() const
{
	return m_Pos;
}

CNewUIMyInventory::REPAIR_MODE CNewUIMyInventory::GetRepairMode() const
{
	return m_RepairMode;
}

void CNewUIMyInventory::SetRepairMode(bool bRepair)
{
	if (bRepair)
	{
		m_RepairMode = REPAIR_MODE_ON;
		if (m_pNewInventoryCtrl)
		{
			m_pNewInventoryCtrl->SetRepairMode(true);
		}
	}
	else
	{
		m_RepairMode = REPAIR_MODE_OFF;
		if (m_pNewInventoryCtrl)
		{
			m_pNewInventoryCtrl->SetRepairMode(false);
		}
	}
}

bool CNewUIMyInventory::UpdateMouseEvent()
{
	if (m_pNewInventoryCtrl && !m_pNewInventoryCtrl->UpdateMouseEvent())
		return false;

	if (true == EquipmentWindowProcess())
		return false;
	if (true == InventoryProcess())
		return false;

	if (true == BtnProcess())
		return false;

	CNewUIPickedItem * pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
	if (pPickedItem && IsPress(VK_LBUTTON) && CheckMouseIn(0, 0, GetScreenWidth(), (GetWindowsY() - 51)))
	{
		if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) == true
			|| g_pNewUISystem->IsVisible(INTERFACE_TRADE) == true
			|| g_pNewUISystem->IsVisible(INTERFACE_DEVILSQUARE) == true
			|| g_pNewUISystem->IsVisible(INTERFACE_BLOODCASTLE) == true
			|| g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY) == true
			|| g_pNewUISystem->IsVisible(INTERFACE_STORAGE) == true
			|| g_pNewUISystem->IsVisible(INTERFACE_MYSHOP_INVENTORY) == true
#ifdef LEM_ADD_LUCKYITEM
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) == true
#endif // LEM_ADD_LUCKYITEM
			|| g_pNewUISystem->IsVisible(INTERFACE_PURCHASESHOP_INVENTORY) == true)
		{
			ResetMouseLButton();
			return false;
		}

		ITEM* pItemObj = pPickedItem->GetItem();
		if (pItemObj && pItemObj->Jewel_Of_Harmony_Option != 0)
		{
			g_pChatListBox->AddText("", GlobalText[2217], TYPE_ERROR_MESSAGE);

			ResetMouseLButton();
			return false;
		}
		if (pItemObj && IsHighValueItem(pItemObj) == true)
		{
			g_pChatListBox->AddText("", GlobalText[269], TYPE_ERROR_MESSAGE);
			CNewUIInventoryCtrl::BackupPickedItem();

			ResetMouseLButton();
			return false;
		}
		if (pItemObj && IsDropBan(pItemObj))
		{
			g_pChatListBox->AddText("", GlobalText[1915], TYPE_ERROR_MESSAGE);
			CNewUIInventoryCtrl::BackupPickedItem();

			ResetMouseLButton();
			return false;
		}
		if (pItemObj && pItemObj->Type == ITEM_POTION + 28 && gMapManager.IsCursedTemple() == true)
		{
			ResetMouseLButton();
			return false;
		}
		RenderTerrain(true);
		if (RenderTerrainTile(SelectXF, SelectYF, (int)SelectXF, (int)SelectYF, 1.f, 1, true))
		{
			const int iSourceIndex = pPickedItem->GetSourceLinealPos();
			const int tx = (int)(CollisionPosition[0] / TERRAIN_SCALE);
			const int ty = (int)(CollisionPosition[1] / TERRAIN_SCALE);
			//gInterface.DrawMessage(1, "0 Drop %d (%d %d)", iSourceIndex, tx, ty);
			if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl || g_pMyInventoryExt->GetOwnerOf(pPickedItem))
			{
				if (Hero->Dead == false)
				{
					SendRequestDropItem(iSourceIndex, tx, ty);
					//gInterface.DrawMessage(1, "1 Drop %d (%d %d)", iSourceIndex, tx, ty);
				}
			}
			else if (pItemObj && pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
			{
				SendRequestDropItem(iSourceIndex, tx, ty);
				///gInterface.DrawMessage(1, "2 Drop %d (%d %d)", iSourceIndex, tx, ty);
			}
			MouseUpdateTime = 0;
			MouseUpdateTimeMax = 6;

			ResetMouseLButton();
			return false;
		}
	}

	

	if (CheckMouseIn(m_Pos.x, m_Pos.y + 20, INVENTORY_WIDTH * 0.5f, 15) == true)
	{
		g_csItemOption.SetViewOptionList(true);
	}
	else
	{
		g_csItemOption.SetViewOptionList(false);
	}

	if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT))
	{
		if (IsPress(VK_RBUTTON))
		{
			ResetMouseRButton();
			return false;
		}

		if (IsNone(VK_LBUTTON) == false)
		{
			return false;
		}
	}

	return true;
}

bool CNewUIMyInventory::UpdateKeyEvent()
{
	if (!g_pNewUISystem->IsVisible(INTERFACE_INVENTORY))
	{
		return true;
	}

	if (IsPress(VK_ESCAPE) == true)
	{
		if (g_pNPCShop->IsSellingItem() == false)
		{
			g_pNewUISystem->Hide(INTERFACE_INVENTORY);
			PlayBuffer(SOUND_CLICK01);
		}
		return false;
	}

	if (IsPress('L') == true)
	{
		if (m_bRepairEnableLevel == true && g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) == false
			&& g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY) == false
#ifdef LEM_ADD_LUCKYITEM
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) == false
#endif // LEM_ADD_LUCKYITEM
			)
		{
			ToggleRepairMode();

			return false;
		}
	}

	if (CanOpenMyShopInterface() == true && IsPress('S'))
	{
		if (m_bMyShopOpen)
		{
			if (m_MyShopMode == MYSHOP_MODE_OPEN)
			{
				ChangeMyShopButtonStateClose();
			}
			else if (m_MyShopMode == MYSHOP_MODE_CLOSE)
			{
				ChangeMyShopButtonStateOpen();
			}
			g_pNewUISystem->Toggle(INTERFACE_MYSHOP_INVENTORY);
			PlayBuffer(SOUND_CLICK01);
		}
		return false;
	}

	if (IsPress('K'))
	{
		g_pNewUISystem->Toggle(INTERFACE_ExpandInventory);
		PlayBuffer(SOUND_CLICK01);

		return false;
	}

	if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT) == false)
	{
		return true;
	}

	if (IsRepeat(VK_CONTROL))
	{
		int iHotKey = -1;
		if (IsPress('Q'))
		{
			iHotKey = HOTKEY_Q;
		}
		else if (IsPress('W'))
		{
			iHotKey = HOTKEY_W;
		}
		else if (IsPress('E'))
		{
			iHotKey = HOTKEY_E;
		}
		else if (IsPress('R'))
		{
			iHotKey = HOTKEY_R;
		}

		if (iHotKey != -1)
		{
			const ITEM* pItem = m_pNewInventoryCtrl->FindItemAtPt(MouseX, MouseY);
			if (pItem == nullptr)
			{
				return false;
			}

			if (CanRegisterItemHotKey(pItem->Type) == true)
			{
				const int iItemLevel = (pItem->Level >> 3) & 15;
				g_pMainFrame->SetItemHotKey(iHotKey, pItem->Type, iItemLevel);
				return false;
			}
		}
	}

	return true;
}

bool CNewUIMyInventory::Update()
{
	if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
	{
		return false;
	}

	if (IsVisible())
	{
		m_iPointedSlot = -1;
		for (int i = 0; i < MAX_EQUIPMENT_INDEX; i++)
		{
			if (CheckMouseIn(m_EquipmentSlots[i].x + 1, m_EquipmentSlots[i].y,
				m_EquipmentSlots[i].width - 4, m_EquipmentSlots[i].height - 4))
			{
				m_iPointedSlot = i;
				m_iPointedSlotMuun = 0;
				break;
			}
		}
#if(HAISLOTRING)
		for (int i = 0; i < 16; i++)
		{
			if (CheckMouseIn(m_EquipmentSlotsMuun[i].x + 1, m_EquipmentSlotsMuun[i].y,
				m_EquipmentSlotsMuun[i].width - 4, m_EquipmentSlotsMuun[i].height - 4))
			{
				m_iPointedSlot = i;
				m_iPointedSlotMuun = 1;
				break;
			}
		}
#endif
	}
	return true;
}

bool CNewUIMyInventory::Render()
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	RenderFrame();
	RenderInventoryDetails();
	RenderSetOption();
	RenderSocketOption();
	RenderButtons();

	if (m_pNewInventoryCtrl)
		m_pNewInventoryCtrl->Render();

	RenderEquippedItem();
	DisableAlphaBlend();
	return true;
}

void CNewUIMyInventory::RenderSetOption()
{
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	if (g_csItemOption.GetSetOptionANum() > 0 || g_csItemOption.GetSetOptionBNum() > 0)
	{
		g_pRenderText->SetTextColor(255, 204, 25, 255);
	}
	else
	{
		g_pRenderText->SetTextColor(100, 100, 100, 255);
	}

	unicode::t_char strText[128];
	unicode::_sprintf(strText, "[%s]", GlobalText[989]);
	g_pRenderText->RenderText(m_Pos.x + INVENTORY_WIDTH * 0.2f, m_Pos.y + 25, strText, INVENTORY_WIDTH * 0.3f, 0, RT3_SORT_CENTER);

	if (g_csItemOption.IsViewOptionList() == true)
	{
		m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, -1, ITEM_SET_OPTION);
	}
}

void CNewUIMyInventory::RenderSocketOption()
{
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

	if (g_SocketItemMgr.IsSocketSetOptionEnabled())
	{
		g_pRenderText->SetTextColor(255, 204, 25, 255);
	}
	else
	{
		g_pRenderText->SetTextColor(100, 100, 100, 255);
	}

	unicode::t_char strText[128];
	unicode::_sprintf(strText, "[%s]", GlobalText[2651]);
	g_pRenderText->RenderText(m_Pos.x + INVENTORY_WIDTH * 0.5f, m_Pos.y + 25, strText, INVENTORY_WIDTH * 0.3f, 0, RT3_SORT_CENTER);

	if (CheckMouseIn(m_Pos.x + INVENTORY_WIDTH * 0.5f, m_Pos.y + 20, INVENTORY_WIDTH * 0.5f, 15) == true)
	{
		m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, -1, ITEM_SOCKET_SET_OPTION);
	}
}

void CNewUIMyInventory::Render3D()
{
	for (int i = 0; i < MAX_EQUIPMENT_INDEX; i++)
	{
		const ITEM* pEquippedItem = &CharacterMachine->Equipment[i];
		if (pEquippedItem->Type >= 0)
		{
			float y = 0.f;
			if (i == EQUIPMENT_ARMOR)
			{
				y = m_EquipmentSlots[i].y - 10.f;
			}
			else
			{
				y = m_EquipmentSlots[i].y;
			}

			glColor4f(1.f, 1.f, 1.f, 1.f);
			RenderItem3D(
				m_EquipmentSlots[i].x + 1,
				y,
				m_EquipmentSlots[i].width - 4,
				m_EquipmentSlots[i].height - 4,
				pEquippedItem->Type,
				pEquippedItem->Level,
				pEquippedItem->Option1,
				pEquippedItem->ExtOption,
				false);
		}
	}
#if(HAISLOTRING)
	for (int ii = 0; ii < 16; ii++)
	{
		const ITEM* pEquippedItemMuun = &CharacterMachine->EquipmentMuun[ii];
		if (pEquippedItemMuun->Type >= 0)
		{
			float y = 0.f;
			y = m_EquipmentSlotsMuun[ii].y;

			glColor4f(1.f, 1.f, 1.f, 1.f);
			RenderItem3D(
				m_EquipmentSlotsMuun[ii].x + 1,
				y,
				m_EquipmentSlotsMuun[ii].width - 4,
				m_EquipmentSlotsMuun[ii].height - 4,
				pEquippedItemMuun->Type,
				pEquippedItemMuun->Level,
				pEquippedItemMuun->Option1,
				pEquippedItemMuun->ExtOption,
				false);
		}
	}
#endif
}

bool CNewUIMyInventory::IsVisible() const
{
	return CNewUIObj::IsVisible();
}

void CNewUIMyInventory::OpenningProcess()
{
	SetRepairMode(false);

	m_MyShopMode = MYSHOP_MODE_OPEN;
	ChangeMyShopButtonStateOpen();

	const WORD wLevel = CharacterAttribute->Level;

	if (wLevel >= 50)
	{
		m_bRepairEnableLevel = true;
	}
	else
	{
		m_bRepairEnableLevel = false;
	}

	if (wLevel >= 6)
	{
		m_bMyShopOpen = true;
	}
	else
	{
		m_bMyShopOpen = false;
	}

	if (g_QuestMng.IsIndexInCurQuestIndexList(0x1000F))
	{
		if (g_QuestMng.IsEPRequestRewardState(0x1000F))
		{
			SendSatisfyQuestRequestFromClient(0x1000F);
			g_QuestMng.SetEPRequestRewardState(0x1000F, false);
		}
	}
}

void CNewUIMyInventory::ClosingProcess()
{
	m_pNewInventoryCtrl->BackupPickedItem();
	RepairEnable = 0;
	SetRepairMode(false);
}

float CNewUIMyInventory::GetLayerDepth()
{
	return 4.54f;
}

CNewUIInventoryCtrl* CNewUIMyInventory::GetInventoryCtrl() const
{
	return m_pNewInventoryCtrl;
}

ITEM* CNewUIMyInventory::FindItem(int iLinealPos) const
{
	ITEM* GetItem = nullptr;

	if (iLinealPos >= 76)
	{
		GetItem = g_pMyInventoryExt->FindItem(iLinealPos);
	}
	else
	{
		if (m_pNewInventoryCtrl)
		{
			GetItem = m_pNewInventoryCtrl->FindItem(iLinealPos);
		}
	}
	return GetItem;
}

ITEM* CNewUIMyInventory::FindItemByKey(DWORD dwKey) const
{
	if (m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->FindItemByKey(dwKey);
	return nullptr;
}

int CNewUIMyInventory::FindItemIndex(short int siType, int iLevel, bool InvExt) const
{
	//==Inv Ext
	if (InvExt)
	{
		int IndexPos = -1;
		IndexPos = g_pMyInventoryExt->FindItemIndex(siType, iLevel);
		if (IndexPos != -1)
		{
			return IndexPos;
		}
	}
	if (m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->FindItemIndex(siType, iLevel);
	return -1;
}

int CNewUIMyInventory::FindItemReverseIndex(short sType, int iLevel) const
{
	if (m_pNewInventoryCtrl)
	{
		return m_pNewInventoryCtrl->FindItemReverseIndex(sType, iLevel);
	}

	return -1;
}

int CNewUIMyInventory::FindEmptySlot(IN int cx, IN int cy) const
{
	int Result = -1;
	if (m_pNewInventoryCtrl)
	{
		Result = m_pNewInventoryCtrl->FindEmptySlot(cx, cy);
		if (Result == -1)
		{
			Result = m_pNewInventoryCtrl->FindEmptySlot(cx, cy, true);
		}
	}

	return Result;
}

int CNewUIMyInventory::FindEmptySlot(ITEM * pItem) const
{
	const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
	if (m_pNewInventoryCtrl)
	{
		return m_pNewInventoryCtrl->FindEmptySlot(pItemAttr->Width, pItemAttr->Height);
	}

	return -1;
}

void CNewUIMyInventory::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
	if (pClass)
	{
		auto* pMyInventory = (CNewUIMyInventory*)(pClass);

		if (dwParamB == ITEM_SET_OPTION)
		{
			g_csItemOption.RenderSetOptionList(pMyInventory->GetPos().x, pMyInventory->GetPos().y);
		}
		else if (dwParamB == ITEM_SOCKET_SET_OPTION)
		{
			g_SocketItemMgr.RenderToolTipForSocketSetOption(pMyInventory->GetPos().x, pMyInventory->GetPos().y);
		}
		else
		{
			pMyInventory->RenderItemToolTip(dwParamA);
		}
	}
}

void CNewUIMyInventory::SetStandbyItemKey(DWORD dwItemKey)
{
	m_dwStandbyItemKey = dwItemKey;
}

DWORD CNewUIMyInventory::GetStandbyItemKey() const
{
	return m_dwStandbyItemKey;
}

int CNewUIMyInventory::GetStandbyItemIndex() const
{
	int Result = -1;
	if (ITEM * pItem = GetStandbyItem())
	{
		//==Inv Ext
		for (int iInv = 0; iInv < CharacterAttribute->InventoryExtensions; iInv++)
		{
			Result = g_pMyInventoryExt->GetInventoryCtrl(iInv)->GetIndexByItem(pItem);
			if (Result != -1)
			{
				return Result;
			}
		}
		return m_pNewInventoryCtrl->GetIndexByItem(pItem);
	}
	return Result;
}

ITEM* CNewUIMyInventory::GetStandbyItem() const
{
	ITEM* GetItem = nullptr;
	//==Inv Ext
	for (int iInv = 0; iInv < CharacterAttribute->InventoryExtensions; iInv++)
	{
		GetItem = g_pMyInventoryExt->GetInventoryCtrl(iInv)->FindItemByKey(m_dwStandbyItemKey);
		if (GetItem)
		{
			return GetItem;
		}
	}

	if (m_pNewInventoryCtrl)
	{
		GetItem = m_pNewInventoryCtrl->FindItemByKey(m_dwStandbyItemKey);
	}

	return GetItem;
}

void CNewUIMyInventory::CreateEquippingEffect(ITEM * pItem)
{
	SetCharacterClass(Hero);
	OBJECT* pHeroObject = &Hero->Object;
	if (false == gMapManager.InChaosCastle())
	{
		switch (pItem->Type)
		{
		case ITEM_HELPER:
			CreateBug(MODEL_HELPER, pHeroObject->Position, pHeroObject);
			break;
		case ITEM_HELPER + 2:
			CreateBug(MODEL_UNICON, pHeroObject->Position, pHeroObject);
			if (!Hero->SafeZone)
				CreateEffect(BITMAP_MAGIC + 1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
			break;
		case ITEM_HELPER + 3:
			CreateBug(MODEL_PEGASUS, pHeroObject->Position, pHeroObject);
			if (!Hero->SafeZone)
				CreateEffect(BITMAP_MAGIC + 1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
			break;
		case ITEM_HELPER + 4:
			CreateBug(MODEL_DARK_HORSE, pHeroObject->Position, pHeroObject);
			if (!Hero->SafeZone)
				CreateEffect(BITMAP_MAGIC + 1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
			break;
		case ITEM_HELPER + 37:
			Hero->Helper.Option1 = pItem->Option1;
			if (pItem->Option1 == 0x01)
			{
				CreateBug(MODEL_FENRIR_BLACK, pHeroObject->Position, pHeroObject);
			}
			else if (pItem->Option1 == 0x02)
			{
				CreateBug(MODEL_FENRIR_BLUE, pHeroObject->Position, pHeroObject);
			}
			else if (pItem->Option1 == 0x04)
			{
				CreateBug(MODEL_FENRIR_GOLD, pHeroObject->Position, pHeroObject);
			}
			else
			{
				CreateBug(MODEL_FENRIR_RED, pHeroObject->Position, pHeroObject);
			}

			if (!Hero->SafeZone)
			{
				CreateEffect(BITMAP_MAGIC + 1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
			}
			break;
		case ITEM_HELPER + 64:
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER + 64, pHeroObject->Position, Hero);
			break;
		case ITEM_HELPER + 65:
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER + 65, pHeroObject->Position, Hero);
			break;
		case ITEM_HELPER + 67:
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER + 67, pHeroObject->Position, Hero);
			break;
		case ITEM_HELPER + 80:
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER + 80, pHeroObject->Position, Hero);
			break;
		case ITEM_HELPER + 106:
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER + 106, pHeroObject->Position, Hero);
			break;
		case ITEM_HELPER + 123:
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER + 123, pHeroObject->Position, Hero);
			break;
		}
		//===Custom Pet
		if (gCustomPet2.CheckCustomPetByItem(pItem->Type))
		{
			CreateBug(pItem->Type + 1171, pHeroObject->Position, pHeroObject);
		}

	}
	if (Hero->EtcPart <= 0 || Hero->EtcPart > 3)
	{
		if (pItem->Type == ITEM_HELPER + 20 && (pItem->Level >> 3) == 3)
		{
			DeleteParts(Hero);
			Hero->EtcPart = PARTS_LION;
		}
	}
	if (pItem->Type==ITEM_WING+39 || pItem->Type==ITEM_HELPER+30 || 
		pItem->Type == ITEM_WING+130 ||
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		(pItem->Type >= ITEM_WING+49 && pItem->Type <= ITEM_WING+50) ||
		(pItem->Type == ITEM_WING+135) ||
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		gCustomWing.CheckCustomWingByModelType(pItem->Type, 1) ||
		pItem->Type==ITEM_WING+40)
	{
		DeleteCloth(Hero, &Hero->Object);
	}
}

void CNewUIMyInventory::DeleteEquippingEffectBug(ITEM * pItem)
{

	if (ThePetProcess().IsPet(pItem->Type) == true)
	{
		ThePetProcess().DeletePet(Hero, pItem->Type);
	}

	switch (pItem->Type)
	{
	case ITEM_HELPER + 30:
	case ITEM_WING + 39:
	case ITEM_WING + 40:
	case ITEM_WING + 130:
	case ITEM_WING + 49:
	case ITEM_WING + 50:
	case ITEM_WING + 135:
		DeleteCloth(Hero, &Hero->Object);
		return;
	}
	if (gCustomWing.CheckCustomWingByModelType(pItem->Type, 1))
	{
		DeleteCloth(Hero, &Hero->Object);
		return;
	}
	if (IsBug(pItem) == true)
	{
		DeleteBug(&Hero->Object);
	}
}

void CNewUIMyInventory::DeleteEquippingEffect()
{
	if (Hero->EtcPart < PARTS_ATTACK_TEAM_MARK)
	{
		DeleteParts(Hero);
		if (Hero->EtcPart > 3)
		{
			Hero->EtcPart = 0;
		}
	}

	SetCharacterClass(Hero);
}

void CNewUIMyInventory::SetEquipmentSlotInfo()
{
	m_EquipmentSlots[EQUIPMENT_HELPER].x = m_Pos.x + 15;
	m_EquipmentSlots[EQUIPMENT_HELPER].y = m_Pos.y + 44;
	m_EquipmentSlots[EQUIPMENT_HELPER].width = 46;
	m_EquipmentSlots[EQUIPMENT_HELPER].height = 46;
	m_EquipmentSlots[EQUIPMENT_HELPER].dwBgImage = IMAGE_INVENTORY_ITEM_FAIRY;

	m_EquipmentSlots[EQUIPMENT_HELM].x = m_Pos.x + 75;
	m_EquipmentSlots[EQUIPMENT_HELM].y = m_Pos.y + 44;
	m_EquipmentSlots[EQUIPMENT_HELM].width = 46;
	m_EquipmentSlots[EQUIPMENT_HELM].height = 46;
	m_EquipmentSlots[EQUIPMENT_HELM].dwBgImage = IMAGE_INVENTORY_ITEM_HELM;

	m_EquipmentSlots[EQUIPMENT_WING].x = m_Pos.x + 120;
	m_EquipmentSlots[EQUIPMENT_WING].y = m_Pos.y + 44;
	m_EquipmentSlots[EQUIPMENT_WING].width = 61;
	m_EquipmentSlots[EQUIPMENT_WING].height = 46;
	m_EquipmentSlots[EQUIPMENT_WING].dwBgImage = IMAGE_INVENTORY_ITEM_WING;

	m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].x = m_Pos.x + 135;
	m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].y = m_Pos.y + 87;
	m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].width = 46;
	m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].height = 66;
	m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].dwBgImage = IMAGE_INVENTORY_ITEM_LEFT;

	m_EquipmentSlots[EQUIPMENT_ARMOR].x = m_Pos.x + 75;
	m_EquipmentSlots[EQUIPMENT_ARMOR].y = m_Pos.y + 87;
	m_EquipmentSlots[EQUIPMENT_ARMOR].width = 46;
	m_EquipmentSlots[EQUIPMENT_ARMOR].height = 66;
	m_EquipmentSlots[EQUIPMENT_ARMOR].dwBgImage = IMAGE_INVENTORY_ITEM_ARMOR;

	m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].x = m_Pos.x + 15;
	m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].y = m_Pos.y + 87;
	m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].width = 46;
	m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].height = 66;
	m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].dwBgImage = IMAGE_INVENTORY_ITEM_RIGHT;

	m_EquipmentSlots[EQUIPMENT_GLOVES].x = m_Pos.x + 15;
	m_EquipmentSlots[EQUIPMENT_GLOVES].y = m_Pos.y + 150;
	m_EquipmentSlots[EQUIPMENT_GLOVES].width = 46;
	m_EquipmentSlots[EQUIPMENT_GLOVES].height = 46;
	m_EquipmentSlots[EQUIPMENT_GLOVES].dwBgImage = IMAGE_INVENTORY_ITEM_GLOVES;

	m_EquipmentSlots[EQUIPMENT_PANTS].x = m_Pos.x + 75;
	m_EquipmentSlots[EQUIPMENT_PANTS].y = m_Pos.y + 150;
	m_EquipmentSlots[EQUIPMENT_PANTS].width = 46;
	m_EquipmentSlots[EQUIPMENT_PANTS].height = 46;
	m_EquipmentSlots[EQUIPMENT_PANTS].dwBgImage = IMAGE_INVENTORY_ITEM_PANTS;

	m_EquipmentSlots[EQUIPMENT_BOOTS].x = m_Pos.x + 135;
	m_EquipmentSlots[EQUIPMENT_BOOTS].y = m_Pos.y + 150;
	m_EquipmentSlots[EQUIPMENT_BOOTS].width = 46;
	m_EquipmentSlots[EQUIPMENT_BOOTS].height = 46;
	m_EquipmentSlots[EQUIPMENT_BOOTS].dwBgImage = IMAGE_INVENTORY_ITEM_BOOT;

	m_EquipmentSlots[EQUIPMENT_RING_LEFT].x = m_Pos.x + 114;
	m_EquipmentSlots[EQUIPMENT_RING_LEFT].y = m_Pos.y + 150;
	m_EquipmentSlots[EQUIPMENT_RING_LEFT].width = 28;
	m_EquipmentSlots[EQUIPMENT_RING_LEFT].height = 28;
	m_EquipmentSlots[EQUIPMENT_RING_LEFT].dwBgImage = IMAGE_INVENTORY_ITEM_RING;

	m_EquipmentSlots[EQUIPMENT_AMULET].x = m_Pos.x + 54;
	m_EquipmentSlots[EQUIPMENT_AMULET].y = m_Pos.y + 87;
	m_EquipmentSlots[EQUIPMENT_AMULET].width = 28;
	m_EquipmentSlots[EQUIPMENT_AMULET].height = 28;
	m_EquipmentSlots[EQUIPMENT_AMULET].dwBgImage = IMAGE_INVENTORY_ITEM_NECKLACE;

	m_EquipmentSlots[EQUIPMENT_RING_RIGHT].x = m_Pos.x + 54;
	m_EquipmentSlots[EQUIPMENT_RING_RIGHT].y = m_Pos.y + 150;
	m_EquipmentSlots[EQUIPMENT_RING_RIGHT].width = 28;
	m_EquipmentSlots[EQUIPMENT_RING_RIGHT].height = 28;
	m_EquipmentSlots[EQUIPMENT_RING_RIGHT].dwBgImage = IMAGE_INVENTORY_ITEM_RING;

#if(HAISLOTRING)
	if (gProtect.m_MainInfo.EnableSlotPet)
	{
		m_EquipmentSlotsMuun[EQUIPMENT_PET_1].x = m_Pos.x + 54;
		m_EquipmentSlotsMuun[EQUIPMENT_PET_1].y = m_Pos.y + 62;
		m_EquipmentSlotsMuun[EQUIPMENT_PET_1].width = 28;
		m_EquipmentSlotsMuun[EQUIPMENT_PET_1].height = 28;
		m_EquipmentSlotsMuun[EQUIPMENT_PET_1].dwBgImage = 891552;
	}
#endif
}

void CNewUIMyInventory::SetButtonInfo()
{
	m_BtnExit.ChangeButtonImgState(true, IMAGE_INVENTORY_EXIT_BTN, false);
	m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 391, 36, 29);
	m_BtnExit.ChangeToolTipText(GlobalText[225], true);

	m_BtnRepair.ChangeButtonImgState(true, IMAGE_INVENTORY_REPAIR_BTN, false);
	m_BtnRepair.ChangeButtonInfo(m_Pos.x + 50, m_Pos.y + 391, 36, 29);
	m_BtnRepair.ChangeToolTipText(GlobalText[233], true);

	m_BtnMyShop.ChangeButtonImgState(true, IMAGE_INVENTORY_MYSHOP_OPEN_BTN, false);
	m_BtnMyShop.ChangeButtonInfo(m_Pos.x + 87, m_Pos.y + 391, 36, 29);
	m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);

	m_BtnExpand.ChangeButtonImgState(true, IMAGE_INVENTORY_EXPAND_BTN, false);
	m_BtnExpand.ChangeButtonInfo(m_Pos.x + 87 + 37, m_Pos.y + 391, 36, 29);
	m_BtnExpand.ChangeToolTipText(GlobalText[3322], true);
}

void CNewUIMyInventory::LoadImages() const
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_INVENTORY_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_INVENTORY_BACK_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back04.tga", IMAGE_INVENTORY_BACK_TOP2, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_INVENTORY_BACK_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_INVENTORY_BACK_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_INVENTORY_BACK_BOTTOM, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_boots.tga", IMAGE_INVENTORY_ITEM_BOOT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_cap.tga", IMAGE_INVENTORY_ITEM_HELM, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_fairy.tga", IMAGE_INVENTORY_ITEM_FAIRY, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_wing.tga", IMAGE_INVENTORY_ITEM_WING, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_weapon(L).tga", IMAGE_INVENTORY_ITEM_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_weapon(R).tga", IMAGE_INVENTORY_ITEM_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_upper.tga", IMAGE_INVENTORY_ITEM_ARMOR, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_gloves.tga", IMAGE_INVENTORY_ITEM_GLOVES, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_lower.tga", IMAGE_INVENTORY_ITEM_PANTS, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_ring.tga", IMAGE_INVENTORY_ITEM_RING, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_necklace.tga", IMAGE_INVENTORY_ITEM_NECKLACE, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_money.tga", IMAGE_INVENTORY_MONEY, GL_LINEAR);
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_INVENTORY_EXIT_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_repair_00.tga", IMAGE_INVENTORY_REPAIR_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_expansion_btn.tga", IMAGE_INVENTORY_EXPAND_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_Bt_openshop.tga", IMAGE_INVENTORY_MYSHOP_OPEN_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_Bt_closeshop.tga", IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, GL_LINEAR);
#if(HAISLOTRING)
	LoadBitmap("Interface\\newui_item_earring.tga", 891551, GL_LINEAR, GL_CLAMP, 1, 0);
	LoadBitmap("Interface\\newui_item_muunv2.tga", 891552, GL_LINEAR, GL_CLAMP, 1, 0);
	LoadBitmap("Interface\\newui_item_earring1.tga", 891553, GL_LINEAR, GL_CLAMP, 1, 0);
	LoadBitmap("Interface\\newui_item_muunv21.tga", 891554, GL_LINEAR, GL_CLAMP, 1, 0);
#endif
}

void CNewUIMyInventory::UnloadImages()
{
	DeleteBitmap(IMAGE_INVENTORY_MYSHOP_CLOSE_BTN);
	DeleteBitmap(IMAGE_INVENTORY_MYSHOP_OPEN_BTN);
	DeleteBitmap(IMAGE_INVENTORY_REPAIR_BTN);
	DeleteBitmap(IMAGE_INVENTORY_EXIT_BTN);
	DeleteBitmap(IMAGE_INVENTORY_MONEY);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_NECKLACE);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_RING);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_PANTS);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_GLOVES);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_ARMOR);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_RIGHT);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_LEFT);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_WING);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_FAIRY);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_HELM);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_BOOT);
	DeleteBitmap(IMAGE_INVENTORY_BACK_BOTTOM);
	DeleteBitmap(IMAGE_INVENTORY_BACK_RIGHT);
	DeleteBitmap(IMAGE_INVENTORY_BACK_LEFT);
	DeleteBitmap(IMAGE_INVENTORY_BACK_TOP2);
	DeleteBitmap(IMAGE_INVENTORY_BACK_TOP);
	DeleteBitmap(IMAGE_INVENTORY_BACK);
	DeleteBitmap(IMAGE_INVENTORY_EXPAND_BTN);
#if(HAISLOTRING)
	DeleteBitmap(891551);
	DeleteBitmap(891552);
	DeleteBitmap(891553);
	DeleteBitmap(891554);
#endif
}

void CNewUIMyInventory::RenderFrame() const
{
	const auto x = static_cast<float>(m_Pos.x);
	const auto y = static_cast<float>(m_Pos.y);

	RenderImage(IMAGE_INVENTORY_BACK, x, y, INVENTORY_WIDTH, INVENTORY_HEIGHT);
	RenderImage(IMAGE_INVENTORY_BACK_TOP2, x, y, 190.f, 64.f);
	RenderImage(IMAGE_INVENTORY_BACK_LEFT, x, y + 64, 21.f, 320.f);
	RenderImage(IMAGE_INVENTORY_BACK_RIGHT, x + INVENTORY_WIDTH - 21, y + 64, 21.f, 320.f);
	RenderImage(IMAGE_INVENTORY_BACK_BOTTOM, x, y + INVENTORY_HEIGHT - 45, 190.f, 45.f);
}

void CNewUIMyInventory::RenderEquippedItem()
{

	

	for (int i = 0; i < MAX_EQUIPMENT_INDEX; i++)
	{
		if (i == EQUIPMENT_HELM)
		{
			if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK)
			{
				continue;
			}
		}
		if ((i == EQUIPMENT_GLOVES) && (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER))
			continue;

		EnableAlphaTest();

		RenderImage(m_EquipmentSlots[i].dwBgImage, m_EquipmentSlots[i].x, m_EquipmentSlots[i].y,
			m_EquipmentSlots[i].width, m_EquipmentSlots[i].height);


		DisableAlphaBlend();

		ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[i];


		if (pEquipmentItemSlot->Type != -1)
		{
			ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pEquipmentItemSlot->Type];
			const int iLevel = (pEquipmentItemSlot->Level >> 3) & 15;
			const int iMaxDurability = calcMaxDurability(pEquipmentItemSlot, pItemAttr, iLevel);

			// ¿ë»ç/Àü»çÀÇ¹ÝÁö ¿¹¿ÜÃ³¸®
			if (i == EQUIPMENT_RING_LEFT || i == EQUIPMENT_RING_RIGHT)
			{
				if (pEquipmentItemSlot->Type == ITEM_HELPER + 20 && iLevel == 1
					|| iLevel == 2)
				{
					continue;
				}
			}

			if ((pEquipmentItemSlot->bPeriodItem == true) && (pEquipmentItemSlot->bExpiredPeriod == false))
				continue;

			if (pEquipmentItemSlot->Durability <= 0)
				glColor4f(1.f, 0.f, 0.f, 0.25f);
			else if (pEquipmentItemSlot->Durability <= (iMaxDurability * 0.2f))
				glColor4f(1.f, 0.15f, 0.f, 0.25f);
			else if (pEquipmentItemSlot->Durability <= (iMaxDurability * 0.3f))
				glColor4f(1.f, 0.5f, 0.f, 0.25f);
			else if (pEquipmentItemSlot->Durability <= (iMaxDurability * 0.5f))
				glColor4f(1.f, 1.f, 0.f, 0.25f);
			else if (IsEquipable(i, pEquipmentItemSlot) == false)
				glColor4f(1.f, 0.f, 0.f, 0.25f);
			else
			{
				continue;
			}

			EnableAlphaTest();
			RenderColor(m_EquipmentSlots[i].x + 1, m_EquipmentSlots[i].y, m_EquipmentSlots[i].width - 4, m_EquipmentSlots[i].height - 4);
			EndRenderColor();
		}
	}

#if(HAISLOTRING)
	for (int i = 0; i < EQUIPMENT_PENTA; i++)
	{
		if (i == EQUIPMENT_HELM)
		{
			if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK)
			{
				continue;
			}
		}
		if ((i == EQUIPMENT_GLOVES) && (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER))
			continue;

		EnableAlphaTest();

		RenderImage(m_EquipmentSlotsMuun[i].dwBgImage, m_EquipmentSlotsMuun[i].x, m_EquipmentSlotsMuun[i].y,
			m_EquipmentSlotsMuun[i].width, m_EquipmentSlotsMuun[i].height);


		DisableAlphaBlend();

		ITEM * pEquipmentItemSlot = &CharacterMachine->EquipmentMuun[i];


		if (pEquipmentItemSlot->Type != -1)
		{
			ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pEquipmentItemSlot->Type];
			const int iLevel = (pEquipmentItemSlot->Level >> 3) & 15;
			const int iMaxDurability = calcMaxDurability(pEquipmentItemSlot, pItemAttr, iLevel);

			// ¿ë»ç/Àü»çÀÇ¹ÝÁö ¿¹¿ÜÃ³¸®
			if (i == EQUIPMENT_RING_LEFT || i == EQUIPMENT_RING_RIGHT)
			{
				if (pEquipmentItemSlot->Type == ITEM_HELPER + 20 && iLevel == 1
					|| iLevel == 2)
				{
					continue;
				}
			}

			if ((pEquipmentItemSlot->bPeriodItem == true) && (pEquipmentItemSlot->bExpiredPeriod == false))
				continue;

			if (pEquipmentItemSlot->Durability <= 0)
				glColor4f(1.f, 0.f, 0.f, 0.25f);
			else if (pEquipmentItemSlot->Durability <= (iMaxDurability * 0.2f))
				glColor4f(1.f, 0.15f, 0.f, 0.25f);
			else if (pEquipmentItemSlot->Durability <= (iMaxDurability * 0.3f))
				glColor4f(1.f, 0.5f, 0.f, 0.25f);
			else if (pEquipmentItemSlot->Durability <= (iMaxDurability * 0.5f))
				glColor4f(1.f, 1.f, 0.f, 0.25f);
			else if (IsEquipable(i, pEquipmentItemSlot) == false)
				glColor4f(1.f, 0.f, 0.f, 0.25f);
			else
			{
				continue;
			}

			EnableAlphaTest();
			RenderColor(m_EquipmentSlotsMuun[i].x + 1, m_EquipmentSlotsMuun[i].y, m_EquipmentSlotsMuun[i].width - 4, m_EquipmentSlotsMuun[i].height - 4);
			EndRenderColor();
		}
	}
#endif

	if (CNewUIInventoryCtrl::GetPickedItem() && m_iPointedSlot != -1)
	{
		ITEM* pItemObj = CNewUIInventoryCtrl::GetPickedItem()->GetItem();
#if(HAISLOTRING)
		const ITEM* pEquipmentItemSlot = (m_iPointedSlot < 12) ? &CharacterMachine->Equipment[m_iPointedSlot] : &CharacterMachine->EquipmentMuun[m_iPointedSlot];
		if (pItemObj && (pEquipmentItemSlot->Type != -1 || false == IsEquipable(m_iPointedSlot, pItemObj))
			&& !((gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER) && (m_iPointedSlot == EQUIPMENT_GLOVES)))
		{
			glColor4f(0.9f, 0.1f, 0.1f, 0.4f);
			EnableAlphaTest();
			if ((m_iPointedSlot < 12))
			{
				RenderColor(m_EquipmentSlots[m_iPointedSlot].x + 1, m_EquipmentSlots[m_iPointedSlot].y,
					m_EquipmentSlots[m_iPointedSlot].width - 4, m_EquipmentSlots[m_iPointedSlot].height - 4);
			}
			else
			{
				RenderColor(m_EquipmentSlotsMuun[m_iPointedSlot].x + 1, m_EquipmentSlotsMuun[m_iPointedSlot].y,
					m_EquipmentSlotsMuun[m_iPointedSlot].width - 4, m_EquipmentSlotsMuun[m_iPointedSlot].height - 4);
			}
			EndRenderColor();
		}

#else
		const ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[m_iPointedSlot];
		if (pItemObj && (pEquipmentItemSlot->Type != -1 || false == IsEquipable(m_iPointedSlot, pItemObj))
			&& !((gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER) && (m_iPointedSlot == EQUIPMENT_GLOVES)))
		{
			glColor4f(0.9f, 0.1f, 0.1f, 0.4f);
			EnableAlphaTest();
			RenderColor(m_EquipmentSlots[m_iPointedSlot].x + 1, m_EquipmentSlots[m_iPointedSlot].y,
				m_EquipmentSlots[m_iPointedSlot].width - 4, m_EquipmentSlots[m_iPointedSlot].height - 4);
			EndRenderColor();
		}
#endif
	}

	if (m_iPointedSlot != -1 && m_pNewUI3DRenderMng)
	{
		m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, m_iPointedSlot, 0);
	}
}

void CNewUIMyInventory::RenderButtons()
{
	EnableAlphaTest();

	if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) == false
		&& g_pNewUISystem->IsVisible(INTERFACE_TRADE) == false
		&& g_pNewUISystem->IsVisible(INTERFACE_DEVILSQUARE) == false
		&& g_pNewUISystem->IsVisible(INTERFACE_BLOODCASTLE) == false
		&& g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY) == false
#ifdef LEM_ADD_LUCKYITEM
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) == false
#endif // LEM_ADD_LUCKYITEM
		&& g_pNewUISystem->IsVisible(INTERFACE_STORAGE) == false)
	{
		if (m_bRepairEnableLevel == true)
		{
			m_BtnRepair.Render();
		}
		if (m_bMyShopOpen == true)
		{
			m_BtnMyShop.Render();
		}
	}
	m_BtnExit.Render();
	m_BtnExpand.Render();

	DisableAlphaBlend();
}

void CNewUIMyInventory::RenderInventoryDetails() const
{
	EnableAlphaTest();

#if(!CB_SHOWWCOIN_V)
	
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 12, GlobalText[223], INVENTORY_WIDTH, 0, RT3_SORT_CENTER);

	RenderImage(IMAGE_INVENTORY_MONEY, m_Pos.x + 11, m_Pos.y + 364, 170.f, 26.f);

	const DWORD dwZen = CharacterMachine->Gold;

	unicode::t_char Text[256] = { 0, };
	ConvertGold(dwZen, Text);

	g_pRenderText->SetTextColor(getGoldColor(dwZen));
	g_pRenderText->RenderText((int)m_Pos.x + 50, (int)m_Pos.y + 371, Text);

	g_pRenderText->SetFont(g_hFont);


#else

	

	
	gInterface.DrawBarForm((int)m_Pos.x + 10, (int)m_Pos.y + 369, 170, 16, 0.0, 0.0, 0.0, 1.0);

	TextDraw(g_hFontBold, (int)m_Pos.x + 15, (int)m_Pos.y + 371, 0xFFFFFFFF, 0x0, 100, 0, 1, "Zen");
	TextDraw(g_hFontBold, (int)m_Pos.x + 15+85, (int)m_Pos.y + 371, 0xFFFFFFFF, 0x0, 100, 0, 1, "WC");

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 12, GlobalText[223], INVENTORY_WIDTH, 0, RT3_SORT_CENTER);

	//RenderImage(IMAGE_INVENTORY_MONEY, m_Pos.x + 11, m_Pos.y + 364, 170.f, 26.f);
	

	const DWORD dwZen = CharacterMachine->Gold;

	unicode::t_char Text[256] = { 0, };
	ConvertGold(dwZen, Text);

	g_pRenderText->SetTextColor(getGoldColor(dwZen));
	
	g_pRenderText->RenderText((int)m_Pos.x + 35, (int)m_Pos.y + 371, Text);

	//==WCoin
	const DWORD dwWC = CharacterAttribute->PrintPlayer.Coin1;

	unicode::t_char TextWC[256] = { 0, };
	ConvertGold(dwWC, TextWC);

	g_pRenderText->SetTextColor(getGoldColor(dwWC));

	g_pRenderText->RenderText((int)m_Pos.x + 40+85, (int)m_Pos.y + 371, TextWC);


	g_pRenderText->SetFont(g_hFont);

	
#endif

#if(CB_SHOWCHOTROI_V)
	const BYTE state[3] = { 0, 1, 2 };
	float bStartX = m_Pos.x + 125;
	float bStartY = m_Pos.y + 450;
	if (gProtect.m_MainInfo.BatTatChoTroi == 1)
	{
		if (g_pBCustomMenuInfo->DrawButtonGUI(gInterface.Data[IMG_31617].ModelID, bStartX, bStartY, 38, 23, 3))
		{

			gCustomMenu.ActionButton(eButtonMarKet);
		}
		TextDraw((HFONT)g_hFontBold, bStartX, bStartY + 5, 0xFFC738FF, 0x0, 39, 0, 3, "Chợ");
	}
#endif

	DisableAlphaBlend();
}

bool CNewUIMyInventory::EquipmentWindowProcess()
{
	if (m_iPointedSlot != -1 && IsRelease(VK_LBUTTON))
	{
		if (CNewUIPickedItem * pPickedItem = CNewUIInventoryCtrl::GetPickedItem())
		{
			ITEM* pItemObj = pPickedItem->GetItem();
			const int iSourceIndex = pPickedItem->GetSourceLinealPos();
			const int iTargetIndex = m_iPointedSlot;
			if (pItemObj->bPeriodItem && pItemObj->bExpiredPeriod)
			{
				g_pChatListBox->AddText("", GlobalText[2285], SEASON3B::TYPE_ERROR_MESSAGE);
				CNewUIInventoryCtrl::BackupPickedItem();

				ResetMouseLButton();
				return false;
			}
#if(HAISLOTRING)
			ITEM* pEquipmentItemSlot = (iTargetIndex < 12) ? &CharacterMachine->Equipment[iTargetIndex] : &CharacterMachine->EquipmentMuun[iTargetIndex];
#else
			ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[iTargetIndex];
#endif
			
			if (pEquipmentItemSlot && pEquipmentItemSlot->Type != -1)
			{
				return true;
			}

			if (g_ChangeRingMgr->CheckChangeRing(pPickedItem->GetItem()->Type))
			{
				ITEM* pItemRingLeft = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
				ITEM* pItemRingRight = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];

				if (g_ChangeRingMgr->CheckChangeRing(pItemRingLeft->Type) || g_ChangeRingMgr->CheckChangeRing(pItemRingRight->Type))
				{
					g_pChatListBox->AddText("", GlobalText[2285], TYPE_ERROR_MESSAGE);
					CNewUIInventoryCtrl::BackupPickedItem();

					ResetMouseLButton();
					return false;
				}
			}
#if(HAISLOTRING)
			if (IsEquipable(iTargetIndex, pItemObj)) //tháo, dời item tren inv
			{
				const STORAGE_TYPE sourceType = pPickedItem->GetSourceStorageType();
				//gInterface.DrawMessage(1, "0ex_src_type %d", pItemObj->ex_src_type);
				if (sourceType == STORAGE_TYPE::INVENTORY && iSourceIndex == iTargetIndex && m_iPointedSlotMuun == 0)
				{
					//gInterface.DrawMessage(1, "1ex_src_type %d", pItemObj->ex_src_type);
					CNewUIInventoryCtrl::BackupPickedItem();
				}
				else
				{
					//gInterface.DrawMessage(1, "2ex_src_type sourceType %d %d %d %d", sourceType, pItemObj->ex_src_type, pPickedItem->GetItem()->bySelectedSlotIndex, iTargetIndex);
					SendRequestEquipmentItem((int)sourceType, iSourceIndex, pItemObj, (iTargetIndex < 12 ? (int)(STORAGE_TYPE::INVENTORY) : 40), iTargetIndex);
					//SendRequestEquipmentItem((int)sourceType, iSourceIndex, pItemObj, (int)(STORAGE_TYPE::INVENTORY), iTargetIndex);
					return true;
				}
			}
#else
			if (IsEquipable(iTargetIndex, pItemObj)) //tháo, dời item tren inv
			{
				const STORAGE_TYPE sourceType = pPickedItem->GetSourceStorageType();
		
				if (sourceType == STORAGE_TYPE::INVENTORY && iSourceIndex == iTargetIndex )
				{
					CNewUIInventoryCtrl::BackupPickedItem();
				}
				else
				{
					SendRequestEquipmentItem((int)sourceType, iSourceIndex, pItemObj, (int)(STORAGE_TYPE::INVENTORY), iTargetIndex);
					return true;
				}
		}

#endif
			
		}
		else // pPickedItem == NULL
		{
			if (GetRepairMode() == REPAIR_MODE_ON)
			{
				ITEM* pEquippedItem = &CharacterMachine->Equipment[m_iPointedSlot];

				if (pEquippedItem == NULL)
				{
					return true;
				}

				if (IsRepairBan(pEquippedItem) == true)
				{
					return true;
				}

				if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) && g_pNPCShop->IsRepairShop())
				{
					SendRequestRepair(m_iPointedSlot, 0);
				}
				else if (m_bRepairEnableLevel == true)
				{
					SendRequestRepair(m_iPointedSlot, 1);
				}

				return true;
			}

#if(HAISLOTRING)
			ITEM* pEquippedItem = (m_iPointedSlot < 12) ? &CharacterMachine->Equipment[m_iPointedSlot] : &CharacterMachine->EquipmentMuun[m_iPointedSlot];
#else
			ITEM* pEquippedItem = &CharacterMachine->Equipment[m_iPointedSlot];
#endif
			//ITEM* pEquippedItem = &CharacterMachine->Equipment[m_iPointedSlot];
			if (pEquippedItem->Type >= 0)
			{
				if (gMapManager.WorldActive == WD_10HEAVEN)
				{
					const ITEM* pEquippedPetItem = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
					bool bPicked = true;

					if (m_iPointedSlot == EQUIPMENT_HELPER || m_iPointedSlot == EQUIPMENT_WING)
					{
						if (((m_iPointedSlot == EQUIPMENT_HELPER) && !gCharacterManager.IsEquipedWing()))
						{
							bPicked = false;
						}
						else if (((m_iPointedSlot == EQUIPMENT_WING) && !((pEquippedPetItem->Type == ITEM_HELPER + 3) || (pEquippedPetItem->Type == ITEM_HELPER + 4) || (pEquippedPetItem->Type == ITEM_HELPER + 37) || gCustomPet2.GetInfoPetType(pEquippedPetItem->Type) == 5 || gCustomPet2.GetInfoPetType(pEquippedPetItem->Type) == 6))
							)
						{
							bPicked = false;
						}
					}

					if (bPicked == true)
					{
						if (CNewUIInventoryCtrl::CreatePickedItem(nullptr, pEquippedItem))
						{
							UnequipItem(m_iPointedSlot);
						}
					}
				}
				else
				{
					if (CNewUIInventoryCtrl::CreatePickedItem(nullptr, pEquippedItem))
					{
						UnequipItem(m_iPointedSlot);
					}
				}
			}
		}
	}

	if (IsRelease(VK_RBUTTON))
	{
		const CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

		const int iSourceIndex = m_iPointedSlot;
		if (GetRepairMode() != REPAIR_MODE_ON && EquipmentItem == false
			&& pPickedItem == nullptr
			&& iSourceIndex != -1)
		{
			ResetMouseRButton();
#if(HAISLOTRING)
			//gInterface.DrawMessage(1, "IsRelease(VK_RBUTTON) m_iPointedSlot %d %d", m_iPointedSlot, m_iPointedSlotMuun);

			if (m_iPointedSlotMuun)
			{
				ITEM* pEquippedItem = &CharacterMachine->EquipmentMuun[iSourceIndex];

				if (pEquippedItem->Type >= 0)
				{
					const int emptySlotIndex = FindEmptySlot(pEquippedItem);

					if (emptySlotIndex != -1)
					{
						// This code looks tricky... it simulates a pick up and click on the inventory slot.
						// God knows what happens, when this request to the server goes wrong.
						if (CNewUIInventoryCtrl::CreatePickedItem(nullptr, pEquippedItem))
						{
							CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
							UnequipItem(iSourceIndex);
							pPickedItem->HidePickedItem();
						}
						//gInterface.DrawMessage(1, "IsRelease(VK_RBUTTON) sourceType %d %d %d %d", iSourceIndex, pEquippedItem->ex_src_type, pEquippedItem->bySelectedSlotIndex, emptySlotIndex);

						SendRequestEquipmentItem(40, iSourceIndex, pEquippedItem, (int)STORAGE_TYPE::INVENTORY, emptySlotIndex);
						return true;
					}
				}
			}
			else
			{

				ITEM* pEquippedItem = &CharacterMachine->Equipment[iSourceIndex];

				if (pEquippedItem->Type >= 0)
				{
					const int emptySlotIndex = FindEmptySlot(pEquippedItem);

					if (emptySlotIndex != -1)
					{
						// This code looks tricky... it simulates a pick up and click on the inventory slot.
						// God knows what happens, when this request to the server goes wrong.
						if (CNewUIInventoryCtrl::CreatePickedItem(nullptr, pEquippedItem))
						{
							CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
							UnequipItem(iSourceIndex);
							pPickedItem->HidePickedItem();
						}
						//gInterface.DrawMessage(1, "IsRelease(VK_RBUTTON) sourceType %d %d %d %d", iSourceIndex, pEquippedItem->ex_src_type, pEquippedItem->bySelectedSlotIndex, emptySlotIndex);

						SendRequestEquipmentItem((int)STORAGE_TYPE::INVENTORY, iSourceIndex, pEquippedItem, (int)STORAGE_TYPE::INVENTORY, emptySlotIndex);
						return true;
					}
				}
			}
#else
			ITEM* pEquippedItem = &CharacterMachine->Equipment[iSourceIndex];

			if (pEquippedItem->Type >= 0)
			{
				const int emptySlotIndex = FindEmptySlot(pEquippedItem);

				if (emptySlotIndex != -1)
				{
					// This code looks tricky... it simulates a pick up and click on the inventory slot.
					// God knows what happens, when this request to the server goes wrong.
					if (CNewUIInventoryCtrl::CreatePickedItem(nullptr, pEquippedItem))
					{
						CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
						UnequipItem(iSourceIndex);
						pPickedItem->HidePickedItem();
					}
					SendRequestEquipmentItem((int)STORAGE_TYPE::INVENTORY, iSourceIndex, pEquippedItem, (int)STORAGE_TYPE::INVENTORY, emptySlotIndex);
					return true;
				}
			}
#endif
		}
	}

	return false;
}

bool CNewUIMyInventory::RepairItemAtMousePoint(CNewUIInventoryCtrl * targetControl)
{
	ITEM* pItem = targetControl->FindItemAtPt(MouseX, MouseY);
	if (pItem == nullptr)
	{
		return true;
	}

	if (IsRepairBan(pItem) == true)
	{
		return true;
	}

	int iIndex = targetControl->GetIndex(pItem->x, pItem->y);

	if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) && g_pNPCShop->IsRepairShop())
	{
		SendRequestRepair(iIndex, 0);
	}
	else
	{
		SendRequestRepair(iIndex, 1);
	}

	return true;
}

bool CNewUIMyInventory::ApplyJewels(CNewUIInventoryCtrl * targetControl, CNewUIPickedItem * pPickedItem, ITEM * pPickItem, const int iSourceIndex, const int iTargetIndex)
{
	auto pItem = targetControl->FindItem(iTargetIndex);
	if (!pItem)
	{
		return false;
	}

	const int	iType = pItem->Type;
	const int	iLevel = (pItem->Level >> 3) & 15;
	const int	iDurability = pItem->Durability;

	bool bSuccess = true;

	if (iType > ITEM_WING + 6
		&& iType != ITEM_HELPER + 30
		&& !(iType >= ITEM_WING + 36 && iType <= ITEM_WING + 43)
		&& !gCustomWing.CheckCustomWingByItem(iType)//=== Check Custom Wing 
		&& !(ITEM_WING + 130 <= iType && iType <= ITEM_WING + 134)
		&& !(iType >= ITEM_WING + 49 && iType <= ITEM_WING + 50)
		&& (iType != ITEM_WING + 135))
	{
		bSuccess = false;
	}

	//if (iType == ITEM_BOW + 7 || iType == ITEM_BOW + 15)
	//{
	//	bSuccess = false;
	//}

	//if ((pPickItem->Type == ITEM_POTION + 13 && iLevel >= 6) || (pPickItem->Type == ITEM_POTION + 14 && iLevel >= 9))
	//{
	//	bSuccess = false;
	//}
	if (gCustomJewel.CheckCustomJewelByItem(pPickItem->Type))
	{
		bSuccess = gCustomJewel.CheckCustomJewelApplyItem(pPickItem->Type, iType);
		//gInterface.DrawMessage(1, "Check %d -> %d %d", pPickItem->Type, iType, bSuccess);
	}
	if (pPickItem->Type == ITEM_POTION + 13 && iType == ITEM_HELPER + 37 && iDurability != 255)
	{
		CFenrirRepairMsgBox* pMsgBox = nullptr;
		CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CFenrirRepairMsgBoxLayout), &pMsgBox);
		pMsgBox->SetSourceIndex(iSourceIndex);

		auto iIndex = targetControl->GetIndex(pItem->x, pItem->y);
		pMsgBox->SetTargetIndex(iIndex);

		pPickedItem->HidePickedItem();
		return true;
	}

	if (pPickItem->Type == ITEM_POTION + 42)
	{
		if (g_SocketItemMgr.IsSocketItem(pItem))
		{
			bSuccess = false;
		}
		else
			if (pItem->Jewel_Of_Harmony_Option != 0)
			{
				bSuccess = false;
			}
			else
			{
				const StrengthenItem strengthitem = g_pUIJewelHarmonyinfo->GetItemType(static_cast<int>(pItem->Type));

				if (strengthitem == SI_None)
				{
					bSuccess = false;
				}
			}
	}

	if (pPickItem->Type == ITEM_POTION + 43 || pPickItem->Type == ITEM_POTION + 44)
	{
		if (g_SocketItemMgr.IsSocketItem(pItem))
		{
			bSuccess = false;
		}
		else if (pItem->Jewel_Of_Harmony_Option == 0)
		{
			bSuccess = false;
		}
	}

#ifdef LEM_ADD_LUCKYITEM
	if (Check_LuckyItem(pItem->Type))
	{
		bSuccess = false;
		if (pPickItem->Type == ITEM_POTION + 161)
		{
			if (pItem->Jewel_Of_Harmony_Option == 0)	bSuccess = true;
		}
		else if (pPickItem->Type == ITEM_POTION + 160)
		{
			if (pItem->Durability > 0)					bSuccess = true;
		}
	}
#endif // LEM_ADD_LUCKYITEM

	if (bSuccess)
	{
		const int targetIndex = targetControl->GetIndexByItem(pItem);
		SendRequestUse(iSourceIndex, targetIndex);
		PlayBuffer(SOUND_GET_ITEM01);
		return true;
	}

	return false;
}

bool CNewUIMyInventory::TryStackItems(CNewUIInventoryCtrl * targetControl, ITEM * pPickItem, const int iSourceIndex, const int iTargetIndex)
{
	if (ITEM * pItem = targetControl->FindItem(iTargetIndex))
	{
		if (targetControl->AreItemsStackable(pPickItem, pItem))
		{
			SendRequestEquipmentItem((int)STORAGE_TYPE::INVENTORY, iSourceIndex, pPickItem, (int)STORAGE_TYPE::INVENTORY, iTargetIndex);
			return true;
		}
	}

	return false;
}

bool CNewUIMyInventory::TryConsumeItem(CNewUIInventoryCtrl * targetControl, ITEM * pItem, const int iIndex)
{
	if (pItem == nullptr)
	{
		return false;
	}
	
	if (pItem->Type == ITEM_POTION + 10)
	{
		if (!Teleport)
		{
			SendRequestUse(iIndex, 0);
			return true;
		}
	}

	const auto isApple = pItem->Type == ITEM_POTION + 0;
	const auto isPotion =
		(pItem->Type >= ITEM_POTION + 0 && pItem->Type <= ITEM_POTION + 9)
		|| (pItem->Type >= ITEM_POTION + 35 && pItem->Type <= ITEM_POTION + 40);

	if (isApple || isPotion
		|| (pItem->Type == ITEM_POTION + 20 && ((pItem->Level >> 3) & 15) == 0)
		|| (pItem->Type >= ITEM_POTION + 46 && pItem->Type <= ITEM_POTION + 50)
		|| (pItem->Type == ITEM_POTION + 11 && ((pItem->Level >> 3) & 15) == 14)
		|| (pItem->Type >= ITEM_POTION + 70 && pItem->Type <= ITEM_POTION + 71)
		|| (pItem->Type >= ITEM_POTION + 72 && pItem->Type <= ITEM_POTION + 77)
		|| pItem->Type == ITEM_HELPER + 60
		|| pItem->Type == ITEM_POTION + 94
		|| (pItem->Type >= ITEM_POTION + 85 && pItem->Type <= ITEM_POTION + 87)
		|| (pItem->Type >= ITEM_POTION + 97 && pItem->Type <= ITEM_POTION + 98)
		|| pItem->Type == ITEM_HELPER + 81
		|| pItem->Type == ITEM_HELPER + 82
		|| pItem->Type == ITEM_POTION + 133
		|| pItem->Type == ITEM_POTION + 162
		|| pItem->Type == ITEM_POTION + 163
		|| pItem->Type >= ITEM_ETC
		)
	{
		SendRequestUse(iIndex, 0);
		if (isApple)
		{
			PlayBuffer(SOUND_EAT_APPLE01);
		}
		else if (isPotion)
		{
			PlayBuffer(SOUND_DRINK01);
		}

		return true;
	}

	if ((pItem->Type >= ITEM_POTION + 78 && pItem->Type <= ITEM_POTION + 82))
	{
		std::list<eBuffState> secretPotionbufflist;
		secretPotionbufflist.push_back(eBuff_SecretPotion1);
		secretPotionbufflist.push_back(eBuff_SecretPotion2);
		secretPotionbufflist.push_back(eBuff_SecretPotion3);
		secretPotionbufflist.push_back(eBuff_SecretPotion4);
		secretPotionbufflist.push_back(eBuff_SecretPotion5);

		if (g_isCharacterBufflist((&Hero->Object), secretPotionbufflist) == eBuffNone) {
			SendRequestUse(iIndex, 0);
			return true;
		}

		CreateOkMessageBox(GlobalText[2530], RGBA(255, 30, 0, 255));
		return false;
	}

	if ((pItem->Type >= ITEM_HELPER + 54 && pItem->Type <= ITEM_HELPER + 57) || (pItem->Type == ITEM_HELPER + 58 && gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD))
	{
		bool result = true;
		WORD point[5] = { 0, };

		point[0] = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
		point[1] = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
		point[2] = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
		point[3] = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
		point[4] = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;

		char nStat[MAX_CLASS][5] =
		{
			18, 18, 15, 30,	0,
			28, 20, 25, 10,	0,
			22, 25, 20, 15,	0,
			26, 26, 26, 26,	0,
			26, 20, 20, 15, 25,
			21, 21, 18, 23,	0,
			32, 27, 25, 20, 0,
		};

		const auto attributeType = pItem->Type - (ITEM_HELPER + 54);
		const int characterClass = gCharacterManager.GetBaseClass(Hero->Class);
		point[attributeType] -= nStat[characterClass][attributeType];
		if (point[attributeType] < (pItem->Durability * 10))
		{
			g_pMyInventory->SetStandbyItemKey(pItem->Key);
			CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUsePartChargeFruitMsgBoxLayout));
			return false;
		}

		SendRequestUse(iIndex, 0);
		return true;
	}

	if (pItem->Type == ITEM_HELPER + 58 && gCharacterManager.GetBaseClass(Hero->Class) != CLASS_DARK_LORD)
	{
		CreateOkMessageBox(GlobalText[1905]);
		return true;
	}

	if (pItem->Type == ITEM_HELPER + 29)
	{
		if (IsUnitedMarketPlace())
		{
			char	szOutputText[512];
			sprintf(szOutputText, "%s %s", GlobalText[3014], GlobalText[3015]);

			CreateOkMessageBox(szOutputText);
			return true;
		}

		if (Hero->SafeZone == false)
		{
			CreateOkMessageBox(GlobalText[2330]);
			return false;
		}

		SendRequestEventZoneOpenTime(4, ((pItem->Level >> 3) & 15));
		g_pMyInventory->SetStandbyItemKey(pItem->Key);
		return true;
	}

	if (pItem->Type == ITEM_HELPER + 46)
	{
		const BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_DEVILSQUARE);
		SendRequestEventZoneOpenTime(1, byPossibleLevel);
		return false;
	}

	if (pItem->Type == ITEM_HELPER + 47)
	{
		const BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_BLOODCASTLE);
		SendRequestEventZoneOpenTime(2, byPossibleLevel);
		return false;
	}

	if (pItem->Type == ITEM_HELPER + 48)
	{
		if (Hero->SafeZone || gMapManager.InHellas())
		{
			g_pChatListBox->AddText("", GlobalText[1238], TYPE_ERROR_MESSAGE);
			return false;
		}

		SendRequestUse(iIndex, 0);
		return true;
	}

	if (pItem->Type == ITEM_HELPER + 61)
	{
		const BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_CURSEDTEMPLE);
		SendRequestEventZoneOpenTime(5, byPossibleLevel);
		return true;
	}

	if (pItem->Type == ITEM_HELPER + 121)
	{
		if (Hero->SafeZone == false)
		{
			CreateOkMessageBox(GlobalText[2330]);
			return false;
		}

		SendRequestEventZoneOpenTime(4, ((pItem->Level >> 3) & 15));
		g_pMyInventory->SetStandbyItemKey(pItem->Key);
		return true;
	}

	if (pItem->Type == ITEM_HELPER + 51)
	{
		SendRequestEventZoneOpenTime(5, ((pItem->Level >> 3) & 15));
		return true;
	}

	if (pItem->Type == ITEM_POTION + 19)
	{
		SendRequestEventZoneOpenTime(1, ((pItem->Level >> 3) & 15));
		return true;
	}

	if (pItem->Type == ITEM_HELPER + 18)
	{
		if (pItem->Level == 0)
		{
			g_pChatListBox->AddText("", GlobalText[2089], TYPE_ERROR_MESSAGE);
		}
		else
		{
			SendRequestEventZoneOpenTime(2, ((pItem->Level >> 3) & 15) - 1);
		}

		return true;
	}

	if ((pItem->Type >= ITEM_ETC + 0 && pItem->Type < ITEM_ETC + MAX_ITEM_INDEX)
		|| (pItem->Type >= ITEM_WING + 7 && pItem->Type <= ITEM_WING + 14)
		|| (pItem->Type >= ITEM_WING + 16 && pItem->Type <= ITEM_WING + 19)
		|| (pItem->Type == ITEM_WING + 20)
		|| (pItem->Type >= ITEM_WING + 21 && pItem->Type <= ITEM_WING + 24)
		|| (pItem->Type == ITEM_WING + 35)
		|| (pItem->Type == ITEM_WING + 44)
		|| (pItem->Type == ITEM_WING + 47)
		|| (pItem->Type == ITEM_WING + 46)
		|| (pItem->Type == ITEM_WING + 45)
		|| (pItem->Type == ITEM_WING + 48)
		|| (pItem->Type == ITEM_ETC + 29)
		|| (pItem->Type == ITEM_ETC + 28)
		)
	{
		bool bReadBookGem = true;

		if ((pItem->Type == ITEM_ETC + 18)
			|| (pItem->Type == ITEM_ETC + 28)
			|| pItem->Type == ITEM_WING + 45
			|| (pItem->Type == ITEM_WING + 46)
			|| (pItem->Type == ITEM_WING + 44)
			)
		{
			if (g_csQuest.getQuestState2(QUEST_CHANGE_UP_3) != QUEST_END)

				bReadBookGem = false;
		}
		if (pItem->Type == ITEM_WING + 48)
		{
			const int Level = CharacterAttribute->Level;
			if (Level < 220)
				bReadBookGem = false;
		}
		if (bReadBookGem)
		{
			WORD Strength, Energy;
			Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
			Energy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;

			if (CharacterAttribute->Level >= ItemAttribute[pItem->Type].RequireLevel &&
				Energy >= pItem->RequireEnergy &&
				Strength >= pItem->RequireStrength)
			{
				SendRequestUse(iIndex, 0);
			}

			return true;
		}

		return false;
	}

	if (pItem->Type == ITEM_HELPER + 15)
	{
		const int Level = CharacterAttribute->Level;

		if (Level >= 10)
		{
			bool bEquipmentEmpty = true;
			for (int i = 0; i < MAX_EQUIPMENT; i++)
			{
				if (CharacterMachine->Equipment[i].Type != -1)
				{
					bEquipmentEmpty = false;
				}
			}
			const int Class = CharacterAttribute->Class;

			if (bEquipmentEmpty == true)
			{
				if (pItem->Level == 32)
				{
					if (gCharacterManager.GetBaseClass(Class) != CLASS_DARK_LORD)
					{
						CreateOkMessageBox(GlobalText[1905]);
						return true;
					}
				}
				g_pMyInventory->SetStandbyItemKey(pItem->Key);
				CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseFruitMsgBoxLayout));
				return true;
			}

			CreateOkMessageBox(GlobalText[1909]);
			return true;
		}

		CreateOkMessageBox(GlobalText[749]);
		return true;
	}

	if (pItem->Type == ITEM_HELPER + 11)
	{
		bool bUse = false;
		const int  Level = (pItem->Level >> 3) & 15;
		switch (Level)
		{
		case 0:
			bUse = true;
			break;
		case 1:
			if (Hero->GuildStatus != G_MASTER)
				bUse = true;
			break;
		}
		if (bUse)
		{
			SendRequestUse(iIndex, 0);
			return true;
		}

		return false;
	}

	if (pItem->Type == ITEM_HELPER + 69)
	{
		if (g_PortalMgr.IsRevivePositionSaved())
		{
			if (g_PortalMgr.IsPortalUsable())
			{
				g_pMyInventory->SetStandbyItemKey(pItem->Key);
				CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseReviveCharmMsgBoxLayout));
			}
			else
			{
				CreateOkMessageBox(GlobalText[2608]);
			}
		}

		return false;
	}

	if (pItem->Type == ITEM_HELPER + 70)
	{
		if (g_PortalMgr.IsPortalUsable())
		{
			if (pItem->Durability == 2)
			{
				if (g_PortalMgr.IsPortalPositionSaved())
				{
					CreateOkMessageBox(GlobalText[2610]);
				}
				else
				{
					g_pMyInventory->SetStandbyItemKey(pItem->Key);
					CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUsePortalCharmMsgBoxLayout));
				}
			}
			else if (pItem->Durability == 1)
			{
				g_pMyInventory->SetStandbyItemKey(pItem->Key);
				CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CReturnPortalCharmMsgBoxLayout));
			}
		}
		else
		{
			CreateOkMessageBox(GlobalText[2608]);
		}

		return false;
	}

	if (pItem->Type == ITEM_HELPER + 66)
	{
		g_pMyInventory->SetStandbyItemKey(pItem->Key);
		CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseSantaInvitationMsgBoxLayout));
	}

	return false;
}

// TODO: This whole logic (and possibly others) should be moved into a 'controller' class or similar.
bool CNewUIMyInventory::HandleInventoryActions(CNewUIInventoryCtrl * targetControl)
{
	CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
	if (pPickedItem && IsRelease(VK_LBUTTON))
	{
		ITEM* pPickItem = pPickedItem->GetItem();
		if (pPickItem == nullptr)
		{
			return true;
		}

		const int iSourceIndex = pPickedItem->GetSourceLinealPos();
		const int iTargetIndex = pPickedItem->GetTargetLinealPos(targetControl);
		//gInterface.DrawMessage(1, "HandleInventoryActions %d -> %d %d", iSourceIndex, iTargetIndex, g_pMyInventoryExt->GetInventoryCtrl());
		if (pPickedItem->GetOwnerInventory() == targetControl
			|| g_pMyInventoryExt->GetInventoryCtrl()) // Movement between Inventory (and within extensions)
		{
			
			// Apply Jewels:
			if ((pPickItem->Type == ITEM_POTION + 13
				|| gCustomJewel.CheckCustomJewelByItem(pPickItem->Type)
				|| pPickItem->Type == ITEM_POTION + 14
				|| pPickItem->Type == ITEM_POTION + 16
				|| pPickItem->Type == ITEM_POTION + 42
				|| pPickItem->Type == ITEM_POTION + 43
				|| pPickItem->Type == ITEM_POTION + 44
				|| pPickItem->Type == ITEM_POTION + 160
				|| pPickItem->Type == ITEM_POTION + 161
				) && ApplyJewels(targetControl, pPickedItem, pPickItem, iSourceIndex, iTargetIndex))
			{
				return true;
			}

			if (iTargetIndex != -1 && TryStackItems(targetControl, pPickItem, iSourceIndex, iTargetIndex))
			{
				return true;
			}
		}
		else if (pPickedItem->GetOwnerInventory() == g_pMyShopInventory->GetInventoryCtrl())
		{

			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			int iTargetIndex = pPickedItem->GetTargetLinealPos(g_pMyInventory->GetInventoryCtrl());

			if (iTargetIndex != -1 && g_pMyInventory->GetInventoryCtrl()->CanMove(iTargetIndex, pPickItem))
			{
				SendRequestEquipmentItem(REQUEST_EQUIPMENT_MYSHOP, MAX_MY_INVENTORY_EX_INDEX + iSourceIndex,
					pPickItem, REQUEST_EQUIPMENT_INVENTORY, iTargetIndex);

				//g_ConsoleDebug->Write(MCD_ERROR, "@SendRequestEquipmentItem Shop -> Inv %d %d ", MAX_MY_INVENTORY_EX_INDEX + iSourceIndex, iTargetIndex);
				return true;
			}
		}
		// If nothing of above applied, we try to move the item.
		if (iTargetIndex >= 0 && targetControl->CanMove(iTargetIndex, pPickItem))
		{
			const auto sourceStorageType = pPickedItem->GetSourceStorageType();
			const auto targetStorageType = targetControl->GetStorageType();
			if (iTargetIndex != iSourceIndex)
			{
				//return 1;
				return SendRequestEquipmentItem((int)sourceStorageType, iSourceIndex,
					pPickItem, (int)targetStorageType, iTargetIndex);
			}
			else
			{
#if(HAISLOTRING)
				if (pPickItem->ex_src_type == ITEM_EX_SRC_EQUIPMENT2)
				{
					return SendRequestEquipmentItem(40, iSourceIndex,
						pPickItem, (int)targetStorageType, iTargetIndex);
				}
#endif				

				CNewUIInventoryCtrl::BackupPickedItem();
			}
		}
	}

	else if (g_pMyInventory->GetRepairMode() == REPAIR_MODE_OFF && IsPress(VK_RBUTTON))
	{
		// handle right click (item usage etc.)
		g_pMyInventory->ResetMouseRButton();

		if (g_pNewUISystem->IsVisible(INTERFACE_STORAGE))
		{
			g_pStorageInventory->ProcessMyInvenItemAutoMove();
			return 1;
		}
		if (g_pNewUISystem->IsVisible(INTERFACE_TRADE))
		{
			g_pTrade->ProcessMyInvenItemAutoMove();
			return 1;
		}
		if (g_pNewUISystem->IsVisible(INTERFACE_INVENTORY)
			&& !g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP)
			&& !g_pNewUISystem->IsVisible(INTERFACE_TRADE)
			&& !g_pNewUISystem->IsVisible(INTERFACE_DEVILSQUARE)
			&& !g_pNewUISystem->IsVisible(INTERFACE_BLOODCASTLE)
#ifdef LEM_ADD_LUCKYITEM
			&& !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
#endif // LEM_ADD_LUCKYITEM
			&& !g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY))
		{
			ITEM* pItem = targetControl->FindItemAtPt(MouseX, MouseY);
			if (!pItem)
			{
				return false;
			}

			const int iIndex = targetControl->GetIndexByItem(pItem);
			//===Custom Send Item
			if (gInterface.ActionSendItemGS(pItem, iIndex))
			{
				return 1;
			}
			////===Draw Test
			//char m_szTextBuff[128] = { 0 };
			//sprintf(m_szTextBuff, "PostItem  %d %d Slot (%d)", MouseX, MouseY, iIndex);
			//g_pChatListBox->AddText("", m_szTextBuff, SEASON3B::TYPE_ERROR_MESSAGE);

			if (GetKeyState(VK_CONTROL) & 0x8000) //Send Post Item //FIX CHUỘT POST
			{
				if (GetTickCount() - gInterface.Data[eTIME].EventTick > 3000) //Click
				{
					gInterface.Data[eTIME].EventTick = GetTickCount();
					PMSG_POSTITEM iMsg = { '\0' };
					strcpy_s(iMsg.chatmsg, (char*)BGetItemName(pItem->Type, pItem->Level));
					memcpy_s(&iMsg.chatid[0], 10, (char*)CharacterAttribute->Name, 10);
					memcpy_s(&iMsg.item_data[0], 107, pItem, 107);
					iMsg.h.set(0x78, sizeof(iMsg));
					BDataSend((LPBYTE)&iMsg, iMsg.h.size);
					return 1;	
				}
			}

			if (iIndex >= 0 && TryConsumeItem(targetControl, pItem, iIndex))
			{
				return true;
			}
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
			else if (g_pMyInventory->IsInvenItem(pItem->Type))
			{
#ifdef LJH_FIX_APP_SHUTDOWN_WEQUIPPING_INVENITEM_WITH_CLICKING_MOUSELBTN
				if (MouseLButton || MouseLButtonPop || MouseLButtonPush)
					return false;
#endif //LJH_FIX_APP_SHUTDOWN_WEQUIPPING_INVENITEM_WITH_CLICKING_MOUSELBTN
				if (pItem->Durability == 0)
					return false;

				int iChangeInvenItemStatus = 0;
				(pItem->Durability == 255) ? iChangeInvenItemStatus = 254 : iChangeInvenItemStatus = 255;

				SendRequestEquippingInventoryItem(iIndex, iChangeInvenItemStatus);
			}
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
			//-- Equip item
			if (!EquipmentItem) //hai slot pet
			{
				const int iSrcIndex = iIndex;

				const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];

				int nDstIndex = pItemAttr->m_byItemSlot;

				if (nDstIndex >= 0 && nDstIndex < 13)
				{
					const ITEM* pEquipment = &CharacterMachine->Equipment[nDstIndex];

					if (pEquipment && pEquipment->Type != -1)
					{
						if (nDstIndex == EQUIPMENT_WEAPON_RIGHT)
						{
							if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK
								|| gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
							{
								if (!pItemAttr->TwoHand)
								{
									nDstIndex = EQUIPMENT_WEAPON_LEFT;
									pEquipment = &CharacterMachine->Equipment[nDstIndex];

									if (!(pEquipment && pEquipment->Type != -1))
									{
										goto LABEL_32;
									}
								}
							}
						}
						else if (nDstIndex == EQUIPMENT_RING_RIGHT)
						{
							nDstIndex = EQUIPMENT_RING_LEFT;
							pEquipment = &CharacterMachine->Equipment[nDstIndex];

							if (!(pEquipment && pEquipment->Type != -1))
							{
								goto LABEL_32;
							}
						}

						return true;
					}
				LABEL_32: // wtf...
					if (g_pMyInventory->IsEquipable(nDstIndex, pItem))
					{
						//gInterface.DrawMessage(1, "g_pMyInventory->IsEquipable iSrcIndex %d %d ", iSrcIndex, nDstIndex);

						g_pMyInventory->m_iPointedSlot = iSrcIndex;
						g_pMyInventory->BMoveItemNew(MOVE_CHAR_TO_INV);
						//if (CNewUIInventoryCtrl::CreatePickedItem(nullptr, pItem))
						//{
						//	targetControl->RemoveItem(pItem);

						//	const auto pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
						//	pPickedItem->HidePickedItem();
						//}

						//SendRequestEquipmentItem((int)STORAGE_TYPE::INVENTORY, iSrcIndex, pItem, (nDstIndex <12 ?(int)STORAGE_TYPE::INVENTORY:40), nDstIndex);
					}
				}

				return true;
			}
		}

		else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY))
		{
			g_pMyInventory->BMoveItemNew(MOVE_INV_TO_MIX);
		}
		return false;
	}

	if (g_pMyInventory->GetRepairMode() == REPAIR_MODE_ON && IsPress(VK_LBUTTON))
	{
		// Repair stuff

		return RepairItemAtMousePoint(targetControl);
	}

	return false;
}

bool CNewUIMyInventory::InventoryProcess() const
{
	if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT) == false)
	{
		return false;
	}

	if (m_pNewInventoryCtrl == nullptr)
	{
		return false;
	}

	return HandleInventoryActions(m_pNewInventoryCtrl);
}

bool CNewUIMyInventory::BtnProcess()
{
	const POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

	if (IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide(INTERFACE_INVENTORY);
		return true;
	}
	if (m_BtnExit.UpdateMouseEvent())
	{
		if (g_pNewUISystem->IsVisible(INTERFACE_MYSHOP_INVENTORY))
		{
			g_pNewUISystem->Hide(INTERFACE_MYSHOP_INVENTORY);
		}
		g_pNewUISystem->Hide(INTERFACE_INVENTORY);
		return true;
	}

	if (m_BtnExpand.UpdateMouseEvent())
	{
		g_pNewUISystem->Toggle(INTERFACE_ExpandInventory);
		return true;
	}

	if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP) == false
		&& g_pNewUISystem->IsVisible(INTERFACE_TRADE) == false
		&& g_pNewUISystem->IsVisible(INTERFACE_DEVILSQUARE) == false
		&& g_pNewUISystem->IsVisible(INTERFACE_BLOODCASTLE) == false
#ifdef LEM_ADD_LUCKYITEM
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) == false
#endif // LEM_ADD_LUCKYITEM
		&& g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY) == false
		&& g_pNewUISystem->IsVisible(INTERFACE_STORAGE) == false)
	{
		if (m_bRepairEnableLevel == true && m_BtnRepair.UpdateMouseEvent() == true)
		{
			ToggleRepairMode();
			return true;
		}

		if (m_bMyShopOpen == true && m_BtnMyShop.UpdateMouseEvent() == true)
		{
			if (m_MyShopMode == MYSHOP_MODE_OPEN)
			{
				ChangeMyShopButtonStateClose();
				g_pNewUISystem->Show(INTERFACE_MYSHOP_INVENTORY);
			}
			else if (m_MyShopMode == MYSHOP_MODE_CLOSE)
			{
				ChangeMyShopButtonStateOpen();
				g_pNewUISystem->Hide(INTERFACE_MYSHOP_INVENTORY);
				g_pNewUISystem->Hide(INTERFACE_PURCHASESHOP_INVENTORY);
			}

			return true;
		}
	}

	return false;
}

void CNewUIMyInventory::RenderItemToolTip(int iSlotIndex) const
{
#if(HAISLOTRING)
	if (m_iPointedSlot != -1)
	{
		ITEM* pEquipmentItemSlot = (iSlotIndex <12) ? &CharacterMachine->Equipment[iSlotIndex] : &CharacterMachine->EquipmentMuun[iSlotIndex];

		if (iSlotIndex < 12)
		{
			if (pEquipmentItemSlot->Type != -1)
			{
				const int iTargetX = m_EquipmentSlots[iSlotIndex].x + m_EquipmentSlots[iSlotIndex].width / 2;
				const int iTargetY = m_EquipmentSlots[iSlotIndex].y + m_EquipmentSlots[iSlotIndex].height / 2;

				pEquipmentItemSlot->bySelectedSlotIndex = iSlotIndex;

				if (m_RepairMode == REPAIR_MODE_OFF)
				{
					RenderItemInfo(iTargetX, iTargetY, pEquipmentItemSlot, false);
				}
				else
				{
					RenderRepairInfo(iTargetX, iTargetY, pEquipmentItemSlot, false);
				}
			}
		}
		else
		{
			if (pEquipmentItemSlot->Type != -1)
			{
				const int iTargetX = m_EquipmentSlotsMuun[iSlotIndex].x + m_EquipmentSlotsMuun[iSlotIndex].width / 2;
				const int iTargetY = m_EquipmentSlotsMuun[iSlotIndex].y + m_EquipmentSlotsMuun[iSlotIndex].height / 2;

				pEquipmentItemSlot->bySelectedSlotIndex = iSlotIndex;

				if (m_RepairMode == REPAIR_MODE_OFF)
				{
					RenderItemInfo(iTargetX, iTargetY, pEquipmentItemSlot, false);
				}
				else
				{
					RenderRepairInfo(iTargetX, iTargetY, pEquipmentItemSlot, false);
				}
			}
		}
	  


	}
#else
	if (m_iPointedSlot != -1)
	{
		ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[iSlotIndex];
		if (pEquipmentItemSlot->Type != -1)
		{
			const int iTargetX = m_EquipmentSlots[iSlotIndex].x + m_EquipmentSlots[iSlotIndex].width / 2;
			const int iTargetY = m_EquipmentSlots[iSlotIndex].y + m_EquipmentSlots[iSlotIndex].height / 2;

			pEquipmentItemSlot->bySelectedSlotIndex = iSlotIndex;

			if (m_RepairMode == REPAIR_MODE_OFF)
			{
				RenderItemInfo(iTargetX, iTargetY, pEquipmentItemSlot, false);
			}
			else
			{
				RenderRepairInfo(iTargetX, iTargetY, pEquipmentItemSlot, false);
			}
		}
	}
#endif
}

bool CNewUIMyInventory::CanRegisterItemHotKey(int iType)
{
	switch (iType)
	{
	case ITEM_POTION + 0:
	case ITEM_POTION + 1:
	case ITEM_POTION + 2:
	case ITEM_POTION + 3:
	case ITEM_POTION + 4:
	case ITEM_POTION + 5:
	case ITEM_POTION + 6:
	case ITEM_POTION + 7:
	case ITEM_POTION + 8:
	case ITEM_POTION + 9:
	case ITEM_POTION + 10:
	case ITEM_POTION + 20:
	case ITEM_POTION + 35:
	case ITEM_POTION + 36:
	case ITEM_POTION + 37:
	case ITEM_POTION + 38:
	case ITEM_POTION + 39:
	case ITEM_POTION + 40:
	case ITEM_POTION + 46:
	case ITEM_POTION + 47:
	case ITEM_POTION + 48:
	case ITEM_POTION + 49:
	case ITEM_POTION + 50:
	case ITEM_POTION + 70:
	case ITEM_POTION + 71:
	case ITEM_POTION + 78:
	case ITEM_POTION + 79:
	case ITEM_POTION + 80:
	case ITEM_POTION + 81:
	case ITEM_POTION + 82:
	case ITEM_POTION + 94:
	case ITEM_POTION + 85:
	case ITEM_POTION + 86:
	case ITEM_POTION + 87:
	case ITEM_POTION + 133:
		return true;
	}

	return false;
}

bool CNewUIMyInventory::CanOpenMyShopInterface()
{
	if (g_pNewUISystem->IsVisible(INTERFACE_NPCSHOP)
		|| g_pNewUISystem->IsVisible(INTERFACE_STORAGE)
		|| g_pNewUISystem->IsVisible(INTERFACE_MIXINVENTORY)
#ifdef LEM_ADD_LUCKYITEM
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
#endif // LEM_ADD_LUCKYITEM
		|| g_pNewUISystem->IsVisible(INTERFACE_TRADE)
		|| gMapManager.IsCursedTemple()
		)
	{
		return false;
	}
	return true;
}

bool CNewUIMyInventory::IsRepairEnableLevel() const
{
	return m_bRepairEnableLevel;
}

void CNewUIMyInventory::SetRepairEnableLevel(bool bOver)
{
	m_bRepairEnableLevel = bOver;
}

void CNewUIMyInventory::ChangeMyShopButtonStateOpen()
{
	m_MyShopMode = MYSHOP_MODE_OPEN;
	m_BtnMyShop.UnRegisterButtonState();
	m_BtnMyShop.RegisterButtonState(BUTTON_STATE_UP, IMAGE_INVENTORY_MYSHOP_OPEN_BTN, 0);
	m_BtnMyShop.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_INVENTORY_MYSHOP_OPEN_BTN, 1);
	m_BtnMyShop.ChangeImgIndex(IMAGE_INVENTORY_MYSHOP_OPEN_BTN, 0);
	m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);
}

void CNewUIMyInventory::ChangeMyShopButtonStateClose()
{
	m_MyShopMode = MYSHOP_MODE_CLOSE;
	m_BtnMyShop.UnRegisterButtonState();
	m_BtnMyShop.RegisterButtonState(BUTTON_STATE_UP, IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, 0);
	m_BtnMyShop.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, 1);
	m_BtnMyShop.ChangeImgIndex(IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, 0);
	m_BtnMyShop.ChangeToolTipText(GlobalText[1127], true);
}

void CNewUIMyInventory::LockMyShopButtonOpen()
{
	m_BtnMyShop.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
	m_BtnMyShop.ChangeTextColor(RGBA(100, 100, 100, 255));
	m_BtnMyShop.Lock();
	// 1125 "°³ÀÎ»óÁ¡¿­±â(S)"
	m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);
}

void CNewUIMyInventory::UnlockMyShopButtonOpen()
{
	m_BtnMyShop.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnMyShop.ChangeTextColor(RGBA(255, 255, 255, 255));
	m_BtnMyShop.UnLock();
	// 1125 "°³ÀÎ»óÁ¡¿­±â(S)"
	m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);
}

void CNewUIMyInventory::ToggleRepairMode()
{
	//. Åä±Û ¼ö¸®¸ðµå
	if (m_RepairMode == REPAIR_MODE_OFF)
	{
		SetRepairMode(true);
	}
	else if (m_RepairMode == REPAIR_MODE_ON)
	{
		SetRepairMode(false);
	}
}

bool CNewUIMyInventory::IsItem(short int siType, bool bcheckPick) const
{
	if (bcheckPick == true)
	{
		const CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

		if (pPickedItem)
		{
			const ITEM* pItemObj = pPickedItem->GetItem();

			if (pItemObj->Type == siType) return true;
		}
	}

	const ITEM* pholyitemObj = m_pNewInventoryCtrl->FindTypeItem(siType);

	if (pholyitemObj) return true;

	return false;
}

int CNewUIMyInventory::GetNumItemByKey(DWORD dwItemKey) const
{
	return m_pNewInventoryCtrl->GetNumItemByKey(dwItemKey);
}

int CNewUIMyInventory::GetNumItemByType(short sItemType) const
{
	return m_pNewInventoryCtrl->GetNumItemByType(sItemType);
}
bool CNewUIMyInventory::CheckIsToolTip() const
{
	return m_pNewInventoryCtrl->CheckIsToolTip();
}
BYTE CNewUIMyInventory::GetDurabilityPointedItem() const
{
	const ITEM* pItem = nullptr;

	if (m_iPointedSlot != -1)
	{
		pItem = &CharacterMachine->Equipment[m_iPointedSlot];
		const BYTE byDurability = pItem->Durability;

		return byDurability;
	}

	pItem = m_pNewInventoryCtrl->FindItemPointedSquareIndex();
	if (pItem != nullptr)
	{
		const BYTE byDurability = pItem->Durability;
		return byDurability;
	}

	return 0;
}

int CNewUIMyInventory::GetPointedItemIndex() const
{
	if (m_iPointedSlot != -1)
	{
		return m_iPointedSlot;
	}

	return m_pNewInventoryCtrl->GetPointedSquareIndex();
}
int CNewUIMyInventory::FindHPItemIndex() const
{
	for (int i = ITEM_POTION; i < ITEM_POTION + 4; i++)
	{
		const int iIndex = FindItemReverseIndex(i);
		if (iIndex != -1)
		{
			return iIndex;
		}
	}

	return -1;
}
int CNewUIMyInventory::FindManaItemIndex() const
{
	for (int i = ITEM_POTION + 6; i >= ITEM_POTION + 5; i--)
	{
		const int iIndex = FindItemReverseIndex(i);
		if (iIndex != -1)
		{
			return iIndex;
		}
	}

	return -1;
}

void CNewUIMyInventory::ResetMouseLButton()
{
	MouseLButton = false;
	MouseLButtonPop = false;
	MouseLButtonPush = false;
}

void CNewUIMyInventory::ResetMouseRButton()
{
	MouseRButton = false;
	MouseRButtonPop = false;
	MouseRButtonPush = false;
}

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
BOOL SEASON3B::CNewUIMyInventory::IsInvenItem(const short sType)
{
	BOOL bInvenItem = FALSE;

	if (FALSE
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
		|| (sType == ITEM_HELPER + 128 || sType == ITEM_HELPER + 129 || sType == ITEM_HELPER + 134)
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
		|| (sType >= ITEM_HELPER + 130 && sType <= ITEM_HELPER + 133)
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
		)
		bInvenItem = TRUE;

	return bInvenItem;
}
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY

void SEASON3B::CNewUIMyInventory::BMoveItemNew(int Type)
{
	if (g_pPickedItem)
		if (g_pPickedItem->GetItem())
			return;
	if (Type < MOVE_INV_TO_MIX || Type >MOVE_CHAR_TO_INV) { return; }
	this->SlotAutoMove = -1;
	////===Draw Test
	//char m_szTextBuff[128] = { 0 };
	//sprintf(m_szTextBuff, "BMoveItemNew %d %d %d Slot (%d)",  MouseX, MouseY, Type, m_iPointedSlot);
	//g_pChatListBox->AddText("", m_szTextBuff, SEASON3B::TYPE_ERROR_MESSAGE);
	//===


	PMSG_ITEM_MOVE_RECV pMsg = { 0 };

	pMsg.h.set(0x79, 0x01, sizeof(pMsg));
	pMsg.Target = -1;

	if (Type == MOVE_CHAR_TO_INV)
	{
		pMsg.sFlag = 0;
		pMsg.tFlag = 0;
		pMsg.Target = 0;
		pMsg.Source = m_iPointedSlot;
		::PlayBuffer(SOUND_GET_ITEM01);
		BDataSend((BYTE*)& pMsg, pMsg.h.size);
		return;
	}


	CNewUIInventoryCtrl* pMyInvenCtrl = NULL;
	if (Type == MOVE_INV_TO_MIX || Type == MOVE_INV_TO_CHAR)
	{
		pMyInvenCtrl = g_pMyInventoryExt->GetInventoryCtrl();

		if (!pMyInvenCtrl)
		{
			pMyInvenCtrl = g_pMyInventory->GetInventoryCtrl();
		}
	}
	else if (Type == MOVE_MIX_TO_INV)
	{
		pMyInvenCtrl = g_pMixInventory->GetInventoryCtrl();
	}
	//g_pChatListBox->AddText("", "Bmove Check 1", SEASON3B::TYPE_ERROR_MESSAGE);

	if (!pMyInvenCtrl) return;

	ITEM* pItemObj = pMyInvenCtrl->FindItemAtPt(MouseX, MouseY);
	//g_pChatListBox->AddText("", "Bmove Check 2", SEASON3B::TYPE_ERROR_MESSAGE);
	if (pItemObj)
	{
		if (pItemObj->Type == ITEM_HELPER + 20)
			return;

		//int nSrcIndex = pItemObj->y * pMyInvenCtrl->GetNumberOfColumn() + pItemObj->x;
		int nSrcIndex = pMyInvenCtrl->GetIndexByItem(pItemObj);
		if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY) && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY))
		{
			pMsg.sFlag = MOVE_INV_TO_MIX == Type ? 0 : g_MixRecipeMgr.GetMixInventoryEquipmentIndex();
			pMsg.tFlag = MOVE_INV_TO_MIX == Type ? g_MixRecipeMgr.GetMixInventoryEquipmentIndex() : 0;
			pMsg.Target = 0;
			//pMsg.Source = (MOVE_INV_TO_MIX == Type ? MAX_EQUIPMENT_INDEX : 0) + nSrcIndex;
			pMsg.Source = nSrcIndex;
		}
		else
		{
			pMsg.sFlag = 0;
			pMsg.tFlag = 0;
			pMsg.Target = 0;
			pMsg.Source = MAX_EQUIPMENT_INDEX + nSrcIndex;
		}
		if (Type == MOVE_MIX_TO_INV) this->SlotAutoMove = pMsg.Source;


		::PlayBuffer(SOUND_GET_ITEM01);
		BDataSend((BYTE*)& pMsg, pMsg.h.size);

		//sprintf(m_szTextBuff, "pMyInvenCtrl Send %d Src %d %d", pItemObj->Type, pMyInvenCtrl->GetIndexByItem(pItemObj), pMsg.Source);
		//g_pChatListBox->AddText("", m_szTextBuff, SEASON3B::TYPE_ERROR_MESSAGE);
	}
}