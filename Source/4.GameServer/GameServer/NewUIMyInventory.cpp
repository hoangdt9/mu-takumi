#include "stdafx.h"
#if(HAISLOTRING)
#include "GameMain.h"
#include "ItemManager.h"
#include "ItemMove.h"
#include "ObjectManager.h"
#include "RandomManager.h"
#include "DSProtocol.h"
#include "CustomPet.h"
#include "ItemOption.h"
#include "ServerInfo.h"
#include "Attack.h"
#include "Util.h"
#include "Map.h"
#include "NewUIMyInventory.h"
#include "ShopManager.h"

CNewUIMyInventory g_pNewMyInventory;

CNewUIMyInventory::CNewUIMyInventory(void)
{
}

CNewUIMyInventory::~CNewUIMyInventory(void)
{
}

void CNewUIMyInventory::GDPetInventorySaveSend(int aIndex) // OK
{
#if(GAMESERVER_UPDATE>=603)

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->LoadEquipItem == 0)
	{
		return;
	}

	SDHP_PET_INVENTORY_SAVE_SEND pMsg;

	pMsg.header.set(0x27, 0x31, sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account, lpObj->Account, sizeof(pMsg.account));

	memcpy(pMsg.name, lpObj->Name, sizeof(pMsg.name));

	for (int n = 0; n < EQUIP_INVENTORY_SIZE; n++) { gItemManager.DBItemByteConvert(pMsg.PetInventory[n], &lpObj->EquipInventory[n]); }

	gDataServerConnection.DataSend((BYTE*)& pMsg, sizeof(pMsg));

#endif
}

void CNewUIMyInventory::GDPetInventorySend(int aIndex) // OK
{

#if(GAMESERVER_UPDATE>=603)

	if (gObjIsAccountValid(aIndex, gObj[aIndex].Account) == 0)
	{
		return;
	}

	if (gObj[aIndex].LoadEquipItem != 0)
	{
		return;
	}

	SDHP_PET_INVENTORY_SEND pMsg;

	pMsg.header.set(0x27, 0x01, sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account, gObj[aIndex].Account, sizeof(pMsg.account));

	memcpy(pMsg.name, gObj[aIndex].Name, sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)& pMsg, pMsg.header.size);

#endif
}

void CNewUIMyInventory::DGPetInventoryRecv(SDHP_PET_INVENTORY_RECV* lpMsg) // OK
{
#if(GAMESERVER_UPDATE>=603)

	if (gObjIsAccountValid(lpMsg->index, lpMsg->account) == 0)
	{
		LogAdd(LOG_RED, "[DGPetInventoryRecv] Invalid Account [%d](%s)", lpMsg->index, lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	/*if(lpObj->LoadEquipItem != 0)
	{
		return;
	}*/

	lpObj->LoadEquipItem = 1;

	memset(lpObj->EquipInventory, 0xFF, EQUIP_INVENTORY_SIZE);

	for (int n = 0; n < EQUIP_INVENTORY_SIZE; n++)
	{
		CItem item;

		lpObj->EquipInventory[n].Clear();

		if (gItemManager.ConvertItemByte(&item, lpMsg->PetInventory[n]) != 0) { this->PetInventoryAddItem(lpObj->Index, item, n); }
	}

	gObjectManager.CharacterMakePreviewCharSet(lpObj->Index);
	this->GCPetItemListSend(lpObj->Index);
#endif
}

void CNewUIMyInventory::GCPetItemListSend(int aIndex) // OK
{
#if(GAMESERVER_UPDATE>=603)

	LPOBJ lpObj = &gObj[aIndex];

	BYTE send[4096];

	PMSG_PET_ITEM_LIST_SEND pMsg;

	pMsg.header.setE(0x4E, 0x0A, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	PMSG_PET_ITEM_LIST info;

	for (int n = 12; n < EQUIP_INVENTORY_SIZE; n++)
	{
		if (lpObj->EquipInventory[n].IsItem() != 0 && lpObj->EquipInventory[n].m_ItemExist != 0)
		{
			info.slot = n;

			gItemManager.ItemByteConvert(info.ItemInfo, lpObj->EquipInventory[n]);

			memcpy(&send[size], &info, sizeof(info));
			size += sizeof(info);

			pMsg.count++;
		}
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);

#endif
}

void CNewUIMyInventory::GCPetItemDurSend(int aIndex, BYTE slot, BYTE dur) // OK
{
#if(GAMESERVER_UPDATE>=603)

	PMSG_PET_ITEM_DUR_SEND pMsg;

	pMsg.header.set(0x4E, 0x0B, sizeof(pMsg));

	pMsg.slot = slot;

	pMsg.dur = dur;

	DataSend(aIndex, (BYTE*)& pMsg, pMsg.header.size);

#endif
}

void CNewUIMyInventory::GCPetItemDeleteSend(int aIndex, BYTE slot, BYTE flag) // OK
{
#if(GAMESERVER_UPDATE>=603)

	PMSG_PET_ITEM_DELETE_SEND pMsg;

	pMsg.header.set(0x4E, 0x0C, sizeof(pMsg));

	pMsg.slot = slot;

	pMsg.flag = flag;

	DataSend(aIndex, (BYTE*)& pMsg, pMsg.header.size);

#endif
}

void CNewUIMyInventory::GCPetItemChangeSend(int aIndex, BYTE slot) // OK
{
#if(GAMESERVER_UPDATE>=603)

	LPOBJ lpObj = &gObj[aIndex];

	PMSG_PET_ITEM_CHANGE_SEND pMsg;

	pMsg.header.set(0x4E, 0x0D, sizeof(pMsg));

	pMsg.index[0] = SET_NUMBERHB(aIndex);
	pMsg.index[1] = SET_NUMBERLB(aIndex);

	gItemManager.ItemByteConvert(pMsg.ItemInfo, lpObj->EquipInventory[slot]);

	pMsg.ItemInfo[1] = slot * 16;
	pMsg.ItemInfo[1] |= ((lpObj->EquipInventory[slot].m_Level - 1) / 2) & 0x0F;

	DataSend(aIndex, (BYTE*)& pMsg, pMsg.header.size);
	MsgSendV2(lpObj, (BYTE*)& pMsg, pMsg.header.size);

	gObjViewportListProtocolCreate(lpObj); //Fix

#endif
}

void CNewUIMyInventory::MainProc() // OK
{
#if(GAMESERVER_UPDATE>=603)

	for (int b = OBJECT_START_USER; b < MAX_OBJECT; b++)
	{
		if (gObjIsConnectedGP(b) == 0)
		{
			continue;
		}

		LPOBJ lpObj = &gObj[b];

		for (int n = 0; n < EQUIP_INVENTORY_SIZE; n++)
		{
			if (lpObj->EquipInventory[n].IsItem() != 0)
			{
				//===Delete Item Dur or Time
				if (lpObj->EquipInventory[n].m_Durability < 1 ||
					lpObj->EquipInventory[n].m_PeriodicItemTime <= ((int)time(0))
					&& lpObj->EquipInventory[n].m_LoadPeriodicItem != 0
					)
				{
					lpObj->EquipInventoryStatus[n] = 0;

					g_pNewMyInventory.PetInventoryDelItem(lpObj, n);

					g_pNewMyInventory.GCPetItemChangeSend(lpObj->Index, n);
					g_pNewMyInventory.GCPetItemDeleteSend(lpObj->Index, n, 1);
				}
				else
				{
					if (n == EQUIPMENT_ERRING_RIGHT || n == EQUIPMENT_ERRING_LEFT || n == EQUIPMENT_PET_1 || n == EQUIPMENT_PENTA)
					{
						if (lpObj->EquipInventoryStatus[n - EQUIPMENT_PET_1] != 0)
						{
							lpObj->EquipInventoryStatus[n - EQUIPMENT_PET_1] = 0;
							//gObjectManager.CharacterCalcAttribute(lpObj->Index);
						}
						else if (lpObj->EquipInventoryStatus[n - EQUIPMENT_PET_1] == 0)
						{
							lpObj->EquipInventoryStatus[n - EQUIPMENT_PET_1] = 1;
							//gObjectManager.CharacterCalcAttribute(lpObj->Index);
						}


					}
				}

			}
		}
	}
#endif
}

void CNewUIMyInventory::CalcItemCommonOption(LPOBJ lpObj, bool flag) // OK
{
#if(GAMESERVER_UPDATE>=603)

	for (int n = 0; n < EQUIP_INVENTORY_SIZE; n++)
	{
		if (lpObj->EquipInventory[n].IsItem() != 0 && n >= MAX_EQUIPMENT_INDEX && lpObj->EquipInventory[n].m_Durability > 0)
		{
			if (flag != 0) { continue; }

			gItemOption.InsertOption(lpObj, &lpObj->EquipInventory[n], flag);
		}
	}
#endif
}

void CNewUIMyInventory::EquipSprtite(LPOBJ lpObj, int damage) // OK
{
	return; //ko mat mau
#if(GAMESERVER_UPDATE>=603)

	if (lpObj->Type != OBJECT_USER)
	{
		return;
	}

	if (gEffectManager.GetEffect(lpObj, EFFECT_TALISMAN_OF_PROTECTION) != 0)
	{
		return;
	}

	float DurabilityValue = (1.0f / gServerInfo.m_GuardianDurabilityRate) * 100;

	DurabilityValue = (DurabilityValue / lpObj->GuardianDurabilityRate) * 100;

	for (int n = 0; n < EQUIP_INVENTORY_SIZE; n++)
	{
		//===Delete Item Dur or Time
		if (lpObj->EquipInventory[n].IsItem() != 0 && lpObj->EquipInventory[n].m_IsPeriodicItem == 0)
		{
			if (n == EQUIPMENT_PET_1)
			{
				//if (gCustomPet.PetBatTu(lpObj->EquipInventory[n].m_Index))
				//{
				//	return;
				//}
				float DurCalc = (damage * (1.0f * DurabilityValue)) / 100;
				if (DurCalc > lpObj->EquipInventory[n].m_Durability)
				{
					lpObj->EquipInventory[n].m_Durability = 0;
				}
				else
				{
					lpObj->EquipInventory[n].m_Durability -= (damage * (1.0f * DurabilityValue)) / 100;
				}
				gItemManager.GCItemDurSend(lpObj->Index, EQUIPMENT_PET_1, (BYTE)lpObj->EquipInventory[n].m_Durability, 0);
				this->GCPetItemDurSend(lpObj->Index, n, (BYTE)lpObj->EquipInventory[n].m_Durability);
				//	LogAdd(LOG_RED, "EquipSprtite Test %s Dur %d", lpObj->Name, (BYTE)lpObj->EquipInventory[n].m_Durability);
			}
			else
			{
				bool result = lpObj->EquipInventory[n].PendantDurabilityDown(lpObj->Index, ((gMap[lpObj->Map].CheckAttr(lpObj->X, lpObj->Y, 1) == 0) ? 1 : 0));

				if (result != 0)
				{
					this->GCPetItemDurSend(lpObj->Index, n, (BYTE)lpObj->EquipInventory[n].m_Durability);
				}
			}

		}
	}
#endif
}

void CNewUIMyInventory::PetInventoryItemSet(int aIndex, int slot, BYTE type)
{
#if(GAMESERVER_UPDATE>=603)

	if (slot < 0 || slot >= EQUIP_INVENTORY_SIZE)
	{
		return;
	}

	ITEM_INFO ItemInfo;

	if (gItemManager.GetInfo(gObj[aIndex].EquipInventory[slot].m_Index, &ItemInfo) == 0)
	{
		return;
	}

	gObj[aIndex].EquipInventoryMap[slot] = type;

#endif
}

BYTE CNewUIMyInventory::PetInventoryAddItem(int aIndex, CItem item, int slot) // OK
{
#if(GAMESERVER_UPDATE>=603)

	if (slot < 0 || slot >= EQUIP_INVENTORY_SIZE)
	{
		return 0xFF;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->EquipInventory[slot].IsItem() != 0 || item.IsItem() == 0)
	{
		return 0xFF;
	}

	ITEM_INFO ItemInfo;

	if (gItemManager.GetInfo(item.m_Index, &ItemInfo) == 0)
	{
		return 0xFF;
	}

	lpObj->EquipInventory[slot] = item;

	this->PetInventoryItemSet(aIndex, slot, 1);

	gObjectManager.CharacterCalcAttribute(lpObj->Index);
	return slot;

#else

	return 0xFF;

#endif
}

void CNewUIMyInventory::PetInventoryDelItem(LPOBJ lpObj, int slot) // OK
{
	this->PetInventoryItemSet(lpObj->Index, slot, 0xFF);
	lpObj->EquipInventory[slot].Clear();
	gObjectManager.CharacterCalcAttribute(lpObj->Index);
}


void CNewUIMyInventory::CGItemSellRecv(PMSG_EQUIP_ITEM_SELL_RECV * lpMsg, int aIndex) // OK
{
#if(GAMESERVER_UPDATE>=603)

	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_ITEM_SELL_SEND pMsg;

	pMsg.header.set(0x33, sizeof(pMsg));

	pMsg.result = 0;

	pMsg.money = 0;

	if (lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_SHOP)
	{
		DataSend(aIndex, (BYTE*)& pMsg, pMsg.header.size);
		return;
	}

	if (SHOP_RANGE(lpObj->TargetShopNumber) == 0)
	{
		DataSend(aIndex, (BYTE*)& pMsg, pMsg.header.size);
		return;
	}

	gObjFixInventoryPointer(aIndex);

	if (lpObj->Transaction == 1)
	{
		DataSend(aIndex, (BYTE*)& pMsg, pMsg.header.size);
		return;
	}

	if (EQUIP_INVENTORY_RANGE(lpMsg->slot) == 0)
	{
		DataSend(aIndex, (BYTE*)& pMsg, pMsg.header.size);
		return;
	}

	if (lpObj->Transaction == 1)
	{
		DataSend(aIndex, (BYTE*)& pMsg, pMsg.header.size);
		return;
	}

	CItem* lpItem = &lpObj->EquipInventory[lpMsg->slot];

	if (lpItem->IsItem() == 0)
	{
		DataSend(aIndex, (BYTE*)& pMsg, pMsg.header.size);
		return;
	}

	if (gItemMove.CheckItemMoveAllowSell(lpItem->m_Index) == 0)
	{
		DataSend(aIndex, (BYTE*)& pMsg, pMsg.header.size);
		return;
	}

	if (lpItem->m_Index == GET_ITEM(13, 4) || lpItem->m_Index == GET_ITEM(13, 5)) // Dark Horse,Dark Reaven
	{
		lpItem->PetValue();
	}
	else
	{
		lpItem->Value();
	}

	DWORD value = (DWORD)lpItem->m_SellMoney;

	if (lpItem->Sell > 0)
	{
		if (lpItem->Coin1 > 0)
		{
			GDSetCoinSend(lpObj->Index, value, 0, 0, "CommonShopSell");
		}
		else if (lpItem->Coin2 > 0)
		{
			GDSetCoinSend(lpObj->Index, 0, value, 0, "CommonShopSell");
		}
		else if (lpItem->Coin3 > 0)
		{
			GDSetCoinSend(lpObj->Index, 0, 0, value, "CommonShopSell");
		}
		else
		{
			if (gObjCheckMaxMoney(aIndex, value) == 0)
			{
				lpObj->Money = MAX_MONEY;
			}
			else
			{
				lpObj->Money += value;
			}
		}
	}
	else
	{
		if (gObjCheckMaxMoney(aIndex, value) == 0)
		{
			lpObj->Money = MAX_MONEY;
		}
		else
		{
			lpObj->Money += value;
		}
	}

	this->PetInventoryDelItem(lpObj, lpMsg->slot);

	pMsg.result = 1;

	pMsg.money = lpObj->Money;

	DataSend(aIndex, (BYTE*)& pMsg, pMsg.header.size);

	if (EQUIP_INVENTORY_RANGE(lpMsg->slot) != 0)
	{
		gObjectManager.CharacterMakePreviewCharSet(lpObj->Index);
		this->GCPetItemChangeSend(lpObj->Index, lpMsg->slot);
		lpObj->EquipInventoryStatus[lpMsg->slot] = 0;
	}

#endif
}

#endif