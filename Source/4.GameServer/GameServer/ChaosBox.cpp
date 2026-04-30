// ChaosBox.cpp: implementation of the CChaosBox class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ChaosBox.h"
#include "CastleSiegeSync.h"
#include "CustomWingMix.h"
#include "DSProtocol.h"
#include "ItemBagManager.h"
#include "ItemOptionRate.h"
#include "JewelOfHarmonyOption.h"
#include "JewelOfHarmonyType.h"
#include "Log.h"
#include "LuckyItem.h"
#include "Notice.h"
#include "PentagramSystem.h"
#include "RandomManager.h"
#include "ServerInfo.h"
#include "SetItemType.h"
#include "Shop.h"
#include "SocketItemOption.h"
#include "SocketItemType.h"
#include "Util.h"
#include "CustomMix.h"
#include "Message.h"
#include "MapServerManager.h"
#include "ItemManager.h"

CChaosBox gChaosBox;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChaosBox::CChaosBox() // OK
{
	this->m_SeniorMixLimitDay = 0;

	this->m_SeniorMixLimitMonth = 0;

	this->m_SeniorMixLimitYear = 0;
}

CChaosBox::~CChaosBox() // OK
{

}

void CChaosBox::ChaosBoxInit(LPOBJ lpObj) // OK
{
	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		lpObj->ChaosBox[n].Clear();
		lpObj->ChaosBoxMap[n] = 0xFF;
	}
}

void CChaosBox::ChaosBoxItemDown(LPOBJ lpObj,int slot) // OK
{
	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(n != slot)
		{
			lpObj->ChaosBox[n].Clear();
			lpObj->ChaosBoxMap[n] = 0xFF;
			continue;
		}

		CItem* lpItem = &lpObj->ChaosBox[n];
		
		lpItem->m_Level = 0;

		float dur = (float)gItemManager.GetItemDurability(lpItem->m_Index,lpItem->m_Level,lpItem->IsExcItem(),lpItem->IsSetItem());

		lpItem->m_Durability = dur*(lpItem->m_Durability/lpItem->m_BaseDurability);

		lpItem->Convert(lpItem->m_Index,lpItem->m_Option1,lpItem->m_Option2,lpItem->m_Option3,lpItem->m_NewOption,lpItem->m_SetOption,lpItem->m_JewelOfHarmonyOption,lpItem->m_ItemOptionEx,lpItem->m_SocketOption,lpItem->m_SocketOptionBonus);
		gLog.Output(LOG_CHAOS_MIX, "[PlusItemLevelMix][Failure with Amules Finish][%s][%s] -[Item] - [%s +%d] - [%.0f,%d,%d,%d] ex[%d] set[%d] oEx[%d] ho[%d] sk[%03d,%03d,%03d,%03d,%03d][%d] seri[%08X]"
			, lpObj->Account
			, lpObj->Name
			, gItemManager.GetItemName(lpItem->m_Index)
			, lpItem->m_Level
			, lpItem->m_Durability,
			lpItem->m_Option1,
			lpItem->m_Option2,
			lpItem->m_Option3,
			lpItem->m_NewOption,
			lpItem->m_SetOption,
			lpItem->m_ItemOptionEx,
			lpItem->m_JewelOfHarmonyOption,
			lpItem->m_SocketOption[0],
			lpItem->m_SocketOption[1],
			lpItem->m_SocketOption[2],
			lpItem->m_SocketOption[3],
			lpItem->m_SocketOption[4],
			lpItem->m_SocketOptionBonus,
			lpItem->m_Serial);
	}
}

void CChaosBox::ChaosBoxItemKeep(LPOBJ lpObj,int slot) // OK
{
	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(n != slot)
		{
			lpObj->ChaosBox[n].Clear();
			lpObj->ChaosBoxMap[n] = 0xFF;
			continue;
		}
	}
}

void CChaosBox::ChaosBoxItemSave(LPOBJ lpObj) // OK
{
	if(gObjInventoryCommit(lpObj->Index) != 0)
	{
		for(int n=0;n < CHAOS_BOX_SIZE;n++)
		{
			if(lpObj->ChaosBox[n].IsItem() != 0)
			{
				gItemManager.InventoryInsertItem(lpObj->Index,lpObj->ChaosBox[n]);
			}
		}
	}
}

bool CChaosBox::GetTalismanOfLuckRate(LPOBJ lpObj,int* rate) // OK
{
	int count = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,53)) // Talisman of Luck
		{
			count += (int)lpObj->ChaosBox[n].m_Durability;
		}
	}

	(*rate) += count;

	return ((count>MAX_TALISMAN_OF_LUCK)?0:1);
}

bool CChaosBox::GetElementalTalismanOfLuckRate(LPOBJ lpObj,int* rate) // OK
{
	int count = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,189)) // Elemental Talisman of Luck
		{
			count += (int)lpObj->ChaosBox[n].m_Durability;
		}
	}

	(*rate) += count;

	return ((count>MAX_TALISMAN_OF_LUCK)?0:1);
}

void CChaosBox::ChaosItemMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13) || lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			ItemCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount == 0 || ItemCount == 0)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	if(gServerInfo.m_ChaosItemMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/20000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_ChaosItemMixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = lpObj->ChaosSuccessRate*10000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;
	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemLevel = 0;
		BYTE ItemOption1 = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(2,6),1);

		RandomManager.AddElement(GET_ITEM(4,6),1);

		RandomManager.AddElement(GET_ITEM(5,7),1);

		RandomManager.GetRandomElement(&ItemIndex);

		gItemOptionRate.GetItemOption0(3,&ItemLevel);

		gItemOptionRate.GetItemOption1(3,&ItemOption1);

		gItemOptionRate.GetItemOption2(3,&ItemOption2);

		gItemOptionRate.GetItemOption3(3,&ItemOption3);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,ItemLevel,0,ItemOption1,ItemOption2,ItemOption3,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX, "[Vũ khí chaos][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d) - [Item] - [%s +%d] - [%d,%d,%d]",
			lpObj->Account,
			lpObj->Name,
			lpObj->ChaosSuccessRate,
			lpObj->ChaosMoney,
			gItemManager.GetItemName(ItemIndex),
			ItemLevel,
			ItemOption1,
			ItemOption2,
			ItemOption3);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[ChaosItemMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::DevilSquareMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int EyeCount = 0;
	int EyeLevel = 0;
	int KeyCount = 0;
	int KeyLevel = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,17))
		{
			EyeCount++;
			EyeLevel = lpObj->ChaosBox[n].m_Level;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,18))
		{
			KeyCount++;
			KeyLevel = lpObj->ChaosBox[n].m_Level;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || EyeCount != 1 || KeyCount != 1 || EyeLevel != KeyLevel)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}

		if (EyeLevel > 7 || KeyLevel > 7)
		{
			this->GCChaosMixSend(lpObj->Index, 8, 0);
			return;
		}
	}

	if(EyeLevel > 7 || KeyLevel > 7)
	{
		this->GCChaosMixSend(lpObj->Index,8,0);
		return;
	}

	switch(EyeLevel)
	{
		case 1:
			lpObj->ChaosSuccessRate = gServerInfo.m_DevilSquareMixRate1[lpObj->AccountLevel];
			lpObj->ChaosMoney = 100000;
			break;
		case 2:
			lpObj->ChaosSuccessRate = gServerInfo.m_DevilSquareMixRate2[lpObj->AccountLevel];
			lpObj->ChaosMoney = 200000;
			break;
		case 3:
			lpObj->ChaosSuccessRate = gServerInfo.m_DevilSquareMixRate3[lpObj->AccountLevel];
			lpObj->ChaosMoney = 400000;
			break;
		case 4:
			lpObj->ChaosSuccessRate = gServerInfo.m_DevilSquareMixRate4[lpObj->AccountLevel];
			lpObj->ChaosMoney = 700000;
			break;
		case 5:
			lpObj->ChaosSuccessRate = gServerInfo.m_DevilSquareMixRate5[lpObj->AccountLevel];
			lpObj->ChaosMoney = 1100000;
			break;
		case 6:
			lpObj->ChaosSuccessRate = gServerInfo.m_DevilSquareMixRate6[lpObj->AccountLevel];
			lpObj->ChaosMoney = 1600000;
			break;
		case 7:
			lpObj->ChaosSuccessRate = gServerInfo.m_DevilSquareMixRate7[lpObj->AccountLevel];
			lpObj->ChaosMoney = 2000000;
			break;
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}
	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(14,19),EyeLevel,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[DevilSquareMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[DevilSquareMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::PlusItemLevelMix(LPOBJ lpObj,int type) // OK
{
	int ChaosCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int ItemCount = 0;
	int ItemSlot = 0;
	int ChaosAmulet = 0;
	int ElementalChaosAmulet = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13))
		{
			BlessCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			SoulCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,96))
		{
			ChaosAmulet++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,190))
		{
			ElementalChaosAmulet++;
		}
		else if(lpObj->ChaosBox[n].m_Level == (9+type))
		{
			ItemCount++;
			ItemSlot = n;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || SoulCount < (type + 1) || BlessCount < (type + 1) || ItemCount != 1)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}


	switch(type)
	{
		case 0:
			lpObj->ChaosSuccessRate = gServerInfo.m_PlusItemLevelMixRate1[lpObj->AccountLevel];
			break;
		case 1:
			lpObj->ChaosSuccessRate = gServerInfo.m_PlusItemLevelMixRate2[lpObj->AccountLevel];
			break;
		case 2:
			lpObj->ChaosSuccessRate = gServerInfo.m_PlusItemLevelMixRate3[lpObj->AccountLevel];
			break;
		case 3:
			lpObj->ChaosSuccessRate = gServerInfo.m_PlusItemLevelMixRate4[lpObj->AccountLevel];
			break;
		case 4:
			lpObj->ChaosSuccessRate = gServerInfo.m_PlusItemLevelMixRate5[lpObj->AccountLevel];
			break;
		case 5:
			lpObj->ChaosSuccessRate = gServerInfo.m_PlusItemLevelMixRate6[lpObj->AccountLevel];
			break;
	}

	if(lpObj->ChaosBox[ItemSlot].m_Option2 != 0)
	{
		lpObj->ChaosSuccessRate += gServerInfo.m_AddLuckSuccessRate2[lpObj->AccountLevel];
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (lpObj->ChaosBox[ItemSlot].IsPentagramItem() == 0 && this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
		{
			this->GCChaosMixSend(lpObj->Index, 240, 0);
			return;
		}

		if (lpObj->ChaosBox[ItemSlot].IsPentagramItem() != 0 && this->GetElementalTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
		{
			this->GCChaosMixSend(lpObj->Index, 240, 0);
			return;
		}
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	if (lpObj->ChaosBox[ItemSlot].m_Level >= gServerInfo.m_TatQuayDo)
	{
		gNotice.GCNoticeSend(lpObj->Index, eMessageBox, 0, 0, 0, 0, 0, "Hiện Tại Giới Hạn Đồ Chỉ Đến Level +%d!", gServerInfo.m_TatQuayDo);
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	lpObj->ChaosMoney = 2000000*(type+1);

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		CItem item = lpObj->ChaosBox[ItemSlot];

		item.m_Level++;

		item.m_Durability = (float)gItemManager.GetItemDurability(item.m_Index,item.m_Level,item.IsExcItem(),item.IsSetItem());

		item.m_Durability = (item.m_Durability*lpObj->ChaosBox[ItemSlot].m_Durability)/item.m_BaseDurability;

		this->ChaosBoxInit(lpObj);

		gItemManager.ChaosBoxAddItem(lpObj->Index,item,0);

		this->GCChaosMixSend(lpObj->Index,1,&item);

		gObjCustomLogPlusChaosMix(lpObj,type,item.m_Index);

		ITEM_INFO ItemInfo;
		if (gItemManager.GetInfo(item.m_Index, &ItemInfo) != 0)
		{
			char buff[256];

			wsprintf(buff, gServerInfo.m_AnnounceChaosMixText, lpObj->Name, ItemInfo.Name, item.m_Level);

			if (gServerInfo.m_AnnounceChaosMix == 2)
			{
				gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, buff);
			}
			else if (gServerInfo.m_AnnounceChaosMix == 1)
			{
				GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0, 0, 0, buff);
			}
		}

		gLog.Output(LOG_CHAOS_MIX,"[PlusItemLevelMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		ITEM_INFO ItemInfo;

		CItem item = lpObj->ChaosBox[ItemSlot];

		if(ChaosAmulet == 0 && ElementalChaosAmulet == 0)
		{
			#if(GAMESERVER_UPDATE>=701)

			gPentagramSystem.DelAllPentagramJewelInfo(lpObj,&lpObj->ChaosBox[ItemSlot],0);

			#endif

			this->ChaosBoxInit(lpObj);

			this->GCChaosBoxSend(lpObj,0);

			this->GCChaosMixSend(lpObj->Index,0,0);

			gLog.Output(LOG_CHAOS_MIX,"[PlusItemLevelMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d, ChaosAmulet: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney,(ChaosAmulet+ElementalChaosAmulet));
		}
		else
		{
			this->ChaosBoxItemDown(lpObj,ItemSlot);

			this->GCChaosBoxSend(lpObj,0);

			this->GCChaosMixSend(lpObj->Index,0,0);

			gLog.Output(LOG_CHAOS_MIX,"[PlusItemLevelMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d, ChaosAmulet: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney,(ChaosAmulet+ElementalChaosAmulet));
		}

		char buff2[256];
		wsprintf(buff2, "[%s] Nâng cấp [%s] thất bại !", lpObj->Name, gItemManager.GetItemName(item.m_Index));

		if (gServerInfo.m_AnnounceChaosMix == 2)
		{
			gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, buff2);
		}
		else if (gServerInfo.m_AnnounceChaosMix == 1)
		{
			GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0, 0, 0, buff2);
		}
	}
}

void CChaosBox::DinorantMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int UniriaCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,2) && lpObj->ChaosBox[n].m_Durability == 255)
		{
			UniriaCount++;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || UniriaCount != 10)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}

		lpObj->ChaosSuccessRate = gServerInfo.m_DinorantMixRate[lpObj->AccountLevel];

		if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
		{
			this->GCChaosMixSend(lpObj->Index, 240, 0);
			return;
		}
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 500000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = GET_ITEM(13,3);
		BYTE ItemNewOption = 0;

		gItemOptionRate.GetItemOption4(3,&ItemNewOption);

		gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,255,0,0,0,-1,ItemNewOption,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[DinorantMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[DinorantMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::FruitMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || CreationCount != 1)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_FruitMixRate[lpObj->AccountLevel];

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 3000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		BYTE ItemLevel = GetLargeRand()%5;

		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(13,15),ItemLevel,0,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[FruitMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[FruitMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing2Mix(LPOBJ lpObj,int type) // OK
{
	int ChaosCount = 0;
	int FeatherCount = 0;
	int SleeveCount = 0;
	int WingItemCount = 0;
	int WingItemMoney = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int ItemSlot = 0;
	int TalismanOfWingType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,14) && lpObj->ChaosBox[n].m_Level == 0)
		{
			FeatherCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,14) && lpObj->ChaosBox[n].m_Level == 1)
		{
			SleeveCount++;
		}
		else if((lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,0) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,2)) || lpObj->ChaosBox[n].m_Index == GET_ITEM(12,41))
		{
			WingItemCount++;
			WingItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		else if(lpObj->ChaosBox[n].IsExcItem() != 0 && lpObj->ChaosBox[n].m_Level >= 4)
		{
			ItemCount++;
			ItemSlot = n;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(13,88) && lpObj->ChaosBox[n].m_Index >= GET_ITEM(13,92))
		{
			TalismanOfWingType = lpObj->ChaosBox[n].m_Index-GET_ITEM(13,87);
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || (type == 0 && FeatherCount != 1) || (type == 1 && SleeveCount != 1) || WingItemCount != 1)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	if(gServerInfo.m_Wing2MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (WingItemMoney/4000000)+(ItemMoney/40000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing2MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_Wing2MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90)?90:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}
	
	if(lpObj->ChaosBox[ItemSlot].m_Level >=4 && lpObj->ChaosBox[ItemSlot].m_NewOption >= 1 )
	{
		lpObj->ChaosSuccessRate = lpObj->ChaosSuccessRate + 10;
	}

	lpObj->ChaosMoney = 5000000;
	
	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);
	CItem item = lpObj->ChaosBox[ItemSlot];
	if ((GetLargeRand() % 100) < lpObj->ChaosSuccessRate)
	{
		if(type == 0)
		{
			WORD ItemIndex = 0;
			BYTE ItemLevel = 0;
			BYTE ItemOption1 = 0;
			BYTE ItemOption2 = 0;
			BYTE ItemOption3 = 0;
			BYTE ItemNewOption = 0;

			CRandomManager RandomManager;

			RandomManager.AddElement(GET_ITEM(12,3),1);

			RandomManager.AddElement(GET_ITEM(12,4),1);

			RandomManager.AddElement(GET_ITEM(12,5),1);

			RandomManager.AddElement(GET_ITEM(12,6),1);

			if (gServerInfo.m_DisableWingMixSU != 1)
			{
				RandomManager.AddElement(GET_ITEM(12,42),1);
			}

			RandomManager.GetRandomElement(&ItemIndex);

			switch(TalismanOfWingType)
			{
				case 1:
					ItemIndex = GET_ITEM(12,5);
					break;
				case 2:
					ItemIndex = GET_ITEM(12,4);
					break;
				case 3:
					ItemIndex = GET_ITEM(12,3);
					break;
				case 4:
					if (gServerInfo.m_DisableWingMixSU != 1)
					{
						ItemIndex = GET_ITEM(12,42); // Asa de SU
					}
					else 
					{
						ItemIndex = GET_ITEM(12,5); // Asa de SM
					}
					break;
				case 5:
					ItemIndex = GET_ITEM(12,6);
					break;
			}

			gItemOptionRate.GetItemOption2(4, &ItemLevel);
			gItemOptionRate.GetItemOption2(4, &ItemOption1);
			gItemOptionRate.GetItemOption2(4, &ItemOption2);
			gItemOptionRate.GetItemOption3(4, &ItemOption3);
			gItemOptionRate.GetItemOption4(4, &ItemNewOption);
			gItemOptionRate.MakeNewOption(ItemIndex, ItemNewOption, &ItemNewOption);
			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 0, ItemOption2, ItemOption3, -1, (ItemNewOption + (32 * (GetLargeRand() % 2))), 0, 0, 0, 0, 0xFF, 0);
			gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(902), lpObj->Name, gItemManager.GetItemName(ItemIndex));
			gLog.Output(LOG_CHAOS_MIX, "[Wing2Mix][Thành Công][%s][%s] Item: %s + %d, %d  %d  %d,(Tỉ lệ: %d, Zen: %d) Từ %s+%d,  %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Serial: %08X)"
				, lpObj->Account, lpObj->Name, gItemManager.GetItemName(ItemIndex), ItemLevel, ItemOption1, ItemOption2, ItemOption3 * 4, lpObj->ChaosSuccessRate, lpObj->ChaosMoney
				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, item.m_Serial);
			
		}
		else
		{
			WORD ItemIndex = 0;
			BYTE ItemLevel = 0;
			BYTE ItemOption1 = 0;
			BYTE ItemOption2 = 0;
			BYTE ItemOption3 = 0;
			BYTE ItemNewOption = 0;
			CRandomManager RandomManager;
			#if(GAMESERVER_UPDATE>=601)
			if (gServerInfo.m_DisableWingMixRF != 1)
			{
				RandomManager.AddElement(GET_ITEM(12, 49), 1);
			}
			#endif
			if (gServerInfo.m_DisableWingMixDL != 1)
			{
				RandomManager.AddElement(GET_ITEM(13, 30), 1);
			}
			else
			{
				RandomManager.AddElement(GET_ITEM(12, 3), 1);
			}

			RandomManager.GetRandomElement(&ItemIndex);
			gItemOptionRate.GetItemOption2(4, &ItemLevel);
			gItemOptionRate.GetItemOption2(4, &ItemOption1);
			gItemOptionRate.GetItemOption2(4, &ItemOption2);
			gItemOptionRate.GetItemOption3(4, &ItemOption3);
			gItemOptionRate.GetItemOption4(5, &ItemNewOption);
			gItemOptionRate.MakeNewOption(ItemIndex, ItemNewOption, &ItemNewOption);
			GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 0, ItemOption2, ItemOption3, -1, (ItemNewOption + 32), 0, 0, 0, 0, 0xFF, 0);
			gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(902), lpObj->Name, gItemManager.GetItemName(ItemIndex));
			gLog.Output(LOG_CHAOS_MIX, "[Wing2Mix][Thành Công][%s][%s] Item: %s + %d, %d  %d  %d,(Tỉ lệ: %d, Zen: %d) Từ %s+%d,  %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Serial: %08X)"
				, lpObj->Account, lpObj->Name, gItemManager.GetItemName(ItemIndex), ItemLevel, ItemOption1, ItemOption2, ItemOption3 * 4, lpObj->ChaosSuccessRate, lpObj->ChaosMoney
				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, item.m_Serial);
		}
	}
	else
	{
		this->ChaosBoxInit(lpObj);
		this->GCChaosBoxSend(lpObj, 0);
		this->GCChaosMixSend(lpObj->Index, 0, 0);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(890), lpObj->Name);
			gLog.Output(LOG_CHAOS_MIX, "[Wing2Mix][Xịt][%s/%s]  %s+%d,  %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
				, lpObj->Account
				, lpObj->Name
				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, lpObj->ChaosSuccessRate
				, lpObj->ChaosMoney
				, item.m_Serial);
	}
}

void CChaosBox::BloodCastleMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ScrollCount = 0;
	int ScrollLevel = 0;
	int BoneCount = 0;
	int BoneLevel = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,16))
		{
			ScrollCount++;
			ScrollLevel = lpObj->ChaosBox[n].m_Level;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,17))
		{
			BoneCount++;
			BoneLevel = lpObj->ChaosBox[n].m_Level;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || ScrollCount != 1 || BoneCount != 1 || ScrollLevel != BoneLevel)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}

		if (ScrollLevel > 8 || BoneLevel > 8)
		{
			this->GCChaosMixSend(lpObj->Index, 8, 0);
			return;
		}
	}

	switch(ScrollLevel)
	{
		case 1:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate1[lpObj->AccountLevel];
			lpObj->ChaosMoney = 50000;
			break;
		case 2:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate2[lpObj->AccountLevel];
			lpObj->ChaosMoney = 80000;
			break;
		case 3:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate3[lpObj->AccountLevel];
			lpObj->ChaosMoney = 150000;
			break;
		case 4:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate4[lpObj->AccountLevel];
			lpObj->ChaosMoney = 250000;
			break;
		case 5:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate5[lpObj->AccountLevel];
			lpObj->ChaosMoney = 400000;
			break;
		case 6:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate6[lpObj->AccountLevel];
			lpObj->ChaosMoney = 600000;
			break;
		case 7:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate7[lpObj->AccountLevel];
			lpObj->ChaosMoney = 850000;
			break;
		case 8:
			lpObj->ChaosSuccessRate = gServerInfo.m_BloodCastleMixRate8[lpObj->AccountLevel];
			lpObj->ChaosMoney = 1050000;
			break;
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) <= lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(13,18),ScrollLevel,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[BloodCastleMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[BloodCastleMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::Wing1Mix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ChaosItem = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int TalismanOfWingType = 0;
	int ItemSlot = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13) || lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			ItemCount++;
			lpObj->ChaosBox[n].OldValue();
			ItemMoney += lpObj->ChaosBox[n].m_OldBuyMoney;
		}
		else if((lpObj->ChaosBox[n].m_Index == GET_ITEM(2,6) || lpObj->ChaosBox[n].m_Index == GET_ITEM(4,6) || lpObj->ChaosBox[n].m_Index == GET_ITEM(5,7)) && lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ChaosItem++;
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemSlot = n;
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(13,83) && lpObj->ChaosBox[n].m_Index >= GET_ITEM(13,86))
		{
			TalismanOfWingType = lpObj->ChaosBox[n].m_Index-GET_ITEM(13,82);
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount == 0 || ChaosItem == 0)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	if(gServerInfo.m_Wing1MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/20000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing1MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = lpObj->ChaosSuccessRate*10000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}
	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	CItem item = lpObj->ChaosBox[ItemSlot];
	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemLevel = 0;
		BYTE ItemOption1 = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,0),1);

		RandomManager.AddElement(GET_ITEM(12,1),1);

		RandomManager.AddElement(GET_ITEM(12,2),1);

		if (gServerInfo.m_DisableWingMixSU != 1)
		{
			RandomManager.AddElement(GET_ITEM(12,41),1);
		}

		RandomManager.GetRandomElement(&ItemIndex);

		switch(TalismanOfWingType)
		{
			case 1:
				ItemIndex = GET_ITEM(12,2);
				break;
			case 2:
				ItemIndex = GET_ITEM(12,1);
				break;
			case 3:
				ItemIndex = GET_ITEM(12,0);
				break;
			case 4:
				if (gServerInfo.m_DisableWingMixSU != 1)
				{
					ItemIndex = GET_ITEM(12,41); //asa de SU
				}
				else
				{
					ItemIndex = GET_ITEM(12,0); // Asa de SM
				}
				break;
		}

		gItemOptionRate.GetItemOption0(5, &ItemLevel);
		gItemOptionRate.GetItemOption1(5, &ItemOption1);
		gItemOptionRate.GetItemOption2(5, &ItemOption2);
		gItemOptionRate.GetItemOption3(5, &ItemOption3);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, ItemLevel, 0, ItemOption1, ItemOption2, ItemOption3, -1, 0, 0, 0, 0, 0, 0xFF, 0);

		//GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 0, ItemOption2, ItemOption3, -1, 0, 0, 0, 0, 0, 0xFF, 0);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(902), lpObj->Name, gItemManager.GetItemName(ItemIndex));		

		gLog.Output(LOG_CHAOS_MIX, "[Wing1Mix][Thành Công][%s][%s] Item: %s + %d, %d  %d  %d,(Tỉ lệ: %d, Zen: %d) Từ %s+%d,  %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Serial: %08X)"
			, lpObj->Account, lpObj->Name, gItemManager.GetItemName(ItemIndex), ItemLevel, ItemOption1, ItemOption2, ItemOption3 * 4, lpObj->ChaosSuccessRate, lpObj->ChaosMoney
			, gItemManager.GetItemName(item.m_Index)
			, item.m_Level
			, item.m_Option1
			, item.m_Option2
			, item.m_Option3 * 4
			, item.m_NewOption
			, item.m_SetOption
			, item.m_JewelOfHarmonyOption
			, item.m_ItemOptionEx
			, item.m_SocketOption[0]
			, item.m_SocketOption[1]
			, item.m_SocketOption[2]
			, item.m_SocketOption[3]
			, item.m_SocketOption[4]
			, item.m_Serial);

	}
	else
	{

		this->ChaosBoxInit(lpObj);
		this->GCChaosBoxSend(lpObj, 0);
		this->GCChaosMixSend(lpObj->Index, 0, 0);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(889), lpObj->Name);
		gLog.Output(LOG_CHAOS_MIX, "[Wing1Mix][Xịt][%s/%s]  %s+%d,  %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
			, lpObj->Account
			, lpObj->Name
			, gItemManager.GetItemName(item.m_Index)
			, item.m_Level
			, item.m_Option1
			, item.m_Option2
			, item.m_Option3 * 4
			, item.m_NewOption
			, item.m_SetOption
			, item.m_JewelOfHarmonyOption
			, item.m_ItemOptionEx
			, item.m_SocketOption[0]
			, item.m_SocketOption[1]
			, item.m_SocketOption[2]
			, item.m_SocketOption[3]
			, item.m_SocketOption[4]
			, lpObj->ChaosSuccessRate
			, lpObj->ChaosMoney
			, item.m_Serial);
	}
}

void CChaosBox::PetMix(LPOBJ lpObj,int type) // OK
{
	int ChaosCount = 0;
	int SoulOfDarkHorseCount = 0;
	int SoulOfDarkSpiritCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int CreationCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,31) && lpObj->ChaosBox[n].m_Level == 0)
		{
			SoulOfDarkHorseCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,31) && lpObj->ChaosBox[n].m_Level == 1)
		{
			SoulOfDarkSpiritCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13))
		{
			BlessCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			SoulCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || (type == 0 && SoulOfDarkHorseCount != 1) || (type == 1 && SoulOfDarkSpiritCount != 1) || BlessCount != (5 - (type * 3)) || SoulCount != (5 - (type * 3)) || CreationCount != 1)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_PetMixRate[lpObj->AccountLevel];

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 5000000-(4000000*type);

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index, 0xFE, 0, 0, (GET_ITEM(13, 4) + type), 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0xFF, 0);
		
		gLog.Output(LOG_CHAOS_MIX, "[PetMix][Thành Công][%s][%s] - (Item: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name,gItemManager.GetItemName((GET_ITEM(13, 4) + type)), lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		
	}
	else
	{
		this->ChaosBoxInit(lpObj);
		this->GCChaosBoxSend(lpObj, 1);
		this->GCChaosMixSend(lpObj->Index, 0, 0);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(894), lpObj->Name);
		
		gLog.Output(LOG_CHAOS_MIX, "[PetMix][Xịt][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, type, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	
	}
}

void CChaosBox::SiegePotionMix(LPOBJ lpObj,int type) // OK
{
	int BlessCount = 0;
	int SoulCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13))
		{
			BlessCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			SoulCount++;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if ((type == 0 && (BlessCount == 0 || BlessCount > 25)) || (type == 1 && (SoulCount == 0 || SoulCount > 25)))
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	lpObj->ChaosMoney = 100000-(50000*type);

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(14,7),type,((BlessCount+SoulCount)*10),0,0,0,-1,0,0,0,0,0,0xFF,0);

	gLog.Output(LOG_CHAOS_MIX,"[SiegePotionMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
}

void CChaosBox::LifeStoneMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int BlessCount = 0;
	int SoulCount = 0;
	int GuardianCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13))
		{
			BlessCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			SoulCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,31))
		{
			GuardianCount++;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || BlessCount != 5 || SoulCount != 5 || GuardianCount != 1)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	lpObj->ChaosMoney = 5000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(13,11),1,0,0,0,0,-1,0,0,0,0,0,0xFF,0);

	gLog.Output(LOG_CHAOS_MIX,"[LifeStoneMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
}

void CChaosBox::SeniorMix(LPOBJ lpObj) // OK
{
	if(gCastleSiegeSync.CheckCastleOwnerMember(lpObj->Index) == 0 || lpObj->GuildStatus != 0x80)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	SYSTEMTIME SystemTime;

	GetSystemTime(&SystemTime);

	if(this->m_SeniorMixLimitDay == SystemTime.wDay && this->m_SeniorMixLimitMonth == SystemTime.wMonth && this->m_SeniorMixLimitYear == SystemTime.wYear)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	int SoulPack10 = 0;
	int BlessPack10 = 0;
	int GuardianCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,30))
		{
			BlessPack10 += lpObj->ChaosBox[n].m_Level+1;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,31))
		{
			SoulPack10 += lpObj->ChaosBox[n].m_Level+1;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,31))
		{
			GuardianCount++;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (SoulPack10 != 3 || BlessPack10 != 3 || GuardianCount != 30)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	lpObj->ChaosMoney = 1000000000;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	CItem item;

	if(gItemBagManager.GetItemBySpecialValue(ITEM_BAG_SENIOR_MIX,lpObj,&item) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	this->m_SeniorMixLimitDay = SystemTime.wDay;

	this->m_SeniorMixLimitMonth = SystemTime.wMonth;

	this->m_SeniorMixLimitYear = SystemTime.wYear;

	GDCreateItemSend(lpObj->Index,0xFF,0,0,item.m_Index,(BYTE)item.m_Level,0,(BYTE)item.m_Option1,(BYTE)item.m_Option2,(BYTE)item.m_Option3,-1,item.m_NewOption,item.m_SetOption,item.m_JewelOfHarmonyOption,item.m_ItemOptionEx,item.m_SocketOption,item.m_SocketOptionBonus,0);

	gLog.Output(LOG_CHAOS_MIX,"[SeniorMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
}

void CChaosBox::PieceOfHornMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int SplinterOfArmorCount = 0;
	int BlessOfGuardianCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,32))
		{
			SplinterOfArmorCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,33))
		{
			BlessOfGuardianCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || SplinterOfArmorCount != 20 || BlessOfGuardianCount != 20)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_PieceOfHornMixRate[lpObj->AccountLevel];

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(13,35),0,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[PieceOfHornMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[PieceOfHornMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::BrokenHornMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ClawOfBeastCount = 0;
	int PieceOfHornCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,34))
		{
			ClawOfBeastCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,35))
		{
			PieceOfHornCount++;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || ClawOfBeastCount != 10 || PieceOfHornCount != 5)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_BrokenHornMixRate[lpObj->AccountLevel];

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(13, 36), 0, 1, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0xFF, 0);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "%s đã nâng cấp thành công Sừng Gãy", lpObj->Name);
		gLog.Output(LOG_CHAOS_MIX, "[Sừng Gãy][Thành Công][%s][%s] -Item: %s(ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name,gItemManager.GetItemName(GET_ITEM(13,37)), lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);
		this->GCChaosBoxSend(lpObj, 0);
		this->GCChaosMixSend(lpObj->Index, 0, 0);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(895), lpObj->Name);
		gLog.Output(LOG_CHAOS_MIX, "[Sừng Gãy][Xịt][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
		
	}
}

void CChaosBox::HornOfFenrirMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int BrokenHornCount = 0;
	int LifeCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,36))
		{
			BrokenHornCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,16))
		{
			LifeCount++;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || BrokenHornCount != 1 || LifeCount != 3)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}

	}
	lpObj->ChaosSuccessRate = gServerInfo.m_HornOfFenrirMixRate[lpObj->AccountLevel];

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 10000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(13, 37), 0, 255, 1, 0, 0, -1, 0, 0, 0, 0, 0, 0xFF, 0);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "%s nâng cấp thành công Sói Tinh", lpObj->Name);
		gLog.Output(LOG_CHAOS_MIX, "[Sói Tinh][Thành Công][%s][%s] -Item: %s(ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name,gItemManager.GetItemName(GET_ITEM(13, 37)), lpObj->ChaosSuccessRate, lpObj->ChaosMoney);

	}
	else
	{
		this->ChaosBoxInit(lpObj);
		this->GCChaosBoxSend(lpObj, 0);
		this->GCChaosMixSend(lpObj->Index, 0, 0);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(896), lpObj->Name);	
		gLog.Output(LOG_CHAOS_MIX, "[HornOfFenrirMix][Xịt][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);
	}
}

void CChaosBox::HornOfFenrirUpgradeMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int HornOfFenrirCount = 0;
	int LifeCount = 0;
	int WeaponCount = 0;
	int WeaponMoney = 0;
	int ArmorCount = 0;
	int ArmorMoney = 0;
	int ItemSlot = 0;
	if (gServerInfo.m_TatBatSoiTCPT != 0)
	{
		for (int n = 0;n < CHAOS_BOX_SIZE;n++)
		{
			if (lpObj->ChaosBox[n].IsItem() == 0)
			{
				continue;
			}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,37))
		{
			HornOfFenrirCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,16))
		{
			LifeCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(0,0) && lpObj->ChaosBox[n].m_Index < GET_ITEM(6,0) && lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			WeaponCount++;
			WeaponMoney = lpObj->ChaosBox[n].m_BuyMoney;ItemSlot=n;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(6,0) && lpObj->ChaosBox[n].m_Index < GET_ITEM(12,0) && lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ArmorCount++;
			ArmorMoney = lpObj->ChaosBox[n].m_BuyMoney;ItemSlot=n;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || HornOfFenrirCount != 1 || LifeCount != 5 || (WeaponCount == 0 && ArmorCount == 0) || (WeaponCount != 0 && ArmorCount != 0))
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	if(gServerInfo.m_HornOfFenrirUpgradeMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (WeaponMoney+ArmorMoney)/10000;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_HornOfFenrirUpgradeMixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_HornOfFenrirUpgradeMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>80)?80:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 10000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);
	
	CItem item = lpObj->ChaosBox[ItemSlot];
	if ((GetLargeRand() % 100) < lpObj->ChaosSuccessRate)
	{
		BYTE ItemNewOption = ((WeaponCount == 0) ? ((ArmorCount == 0) ? 0 : 2) : 1);
		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(13, 37), 0, 255, 1, 0, 0, -1, ItemNewOption, 0, 0, 0, 0, 0xFF, 0);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "%s đã nâng cấp thành công Sói Tấn Công", lpObj->Name);
		gLog.Output(LOG_CHAOS_MIX, "[Nâng Sói][Thành Công][%s/%s] Item: %s  Từ: %s+%d,  %d %d %d %d, Thần: %d, HC %03d 380: %03d, SK %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
				, lpObj->Account
				, lpObj->Name
				,gItemManager.GetItemName(GET_ITEM(13, 37)) 
				
				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, lpObj->ChaosSuccessRate
				, lpObj->ChaosMoney				
				, item.m_Serial);	

	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(896), lpObj->Name);
		gLog.Output(LOG_CHAOS_MIX, "[Nâng Sói][Xịt][%s/%s] Item: %s+%d  %d %d %d %d, Thần: %d, GC: %03d, 380: %03d, SK: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
				, lpObj->Account
				, lpObj->Name
				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, lpObj->ChaosSuccessRate
				, lpObj->ChaosMoney				
				, item.m_Serial);		

	}
}
}

void CChaosBox::HornOfFenrirGoldMix(LPOBJ lpObj) // OK
{
	int FenrirThuong = 0;
	int FenrirTC = 0;
	int FenrirPT = 0;
	if (gServerInfo.m_TatBatSoiHK != 0)
	{
		for (int n = 0; n < CHAOS_BOX_SIZE; n++)
		{
			if (lpObj->ChaosBox[n].IsItem() == 0)
			{
				continue;
			}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(13, 37) && lpObj->ChaosBox[n].m_NewOption == 0)
		{
			FenrirThuong++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(13, 37) && lpObj->ChaosBox[n].m_NewOption == 1)
		{
			FenrirTC++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(13, 37) && lpObj->ChaosBox[n].m_NewOption == 2)
		{
			FenrirPT++;
		}
	}
	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{ 
	if (FenrirThuong != 1 || FenrirTC != 1 || FenrirPT != 1)
		//if(FenrirThuong != 3  )
	{
		this->GCChaosMixSend(lpObj->Index, 7, 0);
		return;
	}
	}
	lpObj->ChaosSuccessRate = gServerInfo.m_HornOfFenrirGoldMixRate[lpObj->AccountLevel];

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate > 100) ? 100 : lpObj->ChaosSuccessRate);
	lpObj->ChaosMoney = 10000000;
	int TaxMoney = (lpObj->ChaosMoney * gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;
	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;


	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}
	lpObj->Money -= lpObj->ChaosMoney;
	GCMoneySend(lpObj->Index, lpObj->Money);
	gCastleSiegeSync.AddTributeMoney(TaxMoney);
	int ItemSlot = 0;
	CItem item = lpObj->ChaosBox[ItemSlot];

	if ((GetLargeRand() % 100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(13, 37), 0, 255, 1, 0, 0, -1, 4, 0, 0, 0, 0, 0xFF, 0);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "%s nâng cấp thành công Sói Tinh Hoàng Kim", lpObj->Name);
		gLog.Output(LOG_CHAOS_MIX, "[Sói Hoàng Kim][Thành Công][%s][%s] -Item: %s  (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, gItemManager.GetItemName(GET_ITEM(13, 37)), lpObj->ChaosSuccessRate, lpObj->ChaosMoney);

	}
	else
	{
		this->ChaosBoxInit(lpObj);
		this->GCChaosBoxSend(lpObj, 0);
		this->GCChaosMixSend(lpObj->Index, 0, 0);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "%s nâng cấp Xịt Sói Tinh Hoàng Kim", lpObj->Name);
		gLog.Output(LOG_CHAOS_MIX, "[Sói Hoàng Kim][Xịt][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);

		}
	}
}


void CChaosBox::ShieldPotionMix(LPOBJ lpObj, int type) // OK
{
	int LargeHealingPotionCount = 0;
	int SmallCompoundPotionCount = 0;
	int MediumCompoundPotionCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,3))
		{
			LargeHealingPotionCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,38))
		{
			SmallCompoundPotionCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,39))
		{
			MediumCompoundPotionCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if ((type == 0 && LargeHealingPotionCount != 3) || (type == 1 && SmallCompoundPotionCount != 3) || (type == 2 && MediumCompoundPotionCount != 3))
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	switch(type)
	{
		case 0:
			lpObj->ChaosSuccessRate = gServerInfo.m_ShieldPotionMixRate1[lpObj->AccountLevel];
			break;
		case 1:
			lpObj->ChaosSuccessRate = gServerInfo.m_ShieldPotionMixRate2[lpObj->AccountLevel];
			break;
		case 2:
			lpObj->ChaosSuccessRate = gServerInfo.m_ShieldPotionMixRate3[lpObj->AccountLevel];
			break;
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = ((type==0)?100000:(500000*type));

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;


	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,(GET_ITEM(14,35)+type),0,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[ShieldPotionMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[ShieldPotionMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::JewelOfHarmonyItemPurityMix(LPOBJ lpObj) // OK
{
	int GemStoneCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,41))
		{
			GemStoneCount++;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (GemStoneCount != 1)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_JewelOfHarmonyItemPurityMixRate[lpObj->AccountLevel];

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(14,42),0,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[JewelOfHarmonyItemPurityMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[JewelOfHarmonyItemPurityMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::JewelOfHarmonyItemSmeltMix(LPOBJ lpObj) // OK
{
	int ItemCount = 0;
	int ItemSlot = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(gJewelOfHarmonyType.CheckJewelOfHarmonyItemType(&lpObj->ChaosBox[n]) != 0 && lpObj->ChaosBox[n].IsSetItem() == 0 && lpObj->ChaosBox[n].IsJewelOfHarmonyItem() == 0)
		{
			ItemCount++;
			ItemSlot = n;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ItemCount != 1)
		{
			gNotice.GCNoticeSend(lpObj->Index, eMessageBox, 0, 0, 0, 0, 0, "Vật phẩm không phù hợp !");
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	if(lpObj->ChaosBox[ItemSlot].IsExcItem() == 0)
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_JewelOfHarmonyItemSmeltMixRate1[lpObj->AccountLevel];
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_JewelOfHarmonyItemSmeltMixRate2[lpObj->AccountLevel];
	}

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = ((lpObj->ChaosBox[ItemSlot].IsExcItem()==0)?GET_ITEM(14,43):GET_ITEM(14,44));

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 1, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0xFF, 0);
		CItem item = lpObj->ChaosBox[ItemSlot];
	gLog.Output(LOG_CHAOS_MIX, "[Đá Tinh Luyện][Thành Công][%s/%s] Từ: %s+%d,  %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
				, lpObj->Account
				, lpObj->Name
				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, lpObj->ChaosSuccessRate
				, lpObj->ChaosMoney				
				, item.m_Serial);		

	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);
		CItem item = lpObj->ChaosBox[ItemSlot];
				gLog.Output(LOG_CHAOS_MIX, "[Đá Tinh Luyện][Thất bại][%s/%s] Từ: %s+%d,  %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
				, lpObj->Account
				, lpObj->Name
				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, lpObj->ChaosSuccessRate
				, lpObj->ChaosMoney				
				, item.m_Serial);			

	}
}

void CChaosBox::JewelOfHarmonyItemRestoreMix(LPOBJ lpObj) // OK
{
	int ItemCount = 0;
	int ItemSlot = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].IsJewelOfHarmonyItem() != 0)
		{
			ItemCount++;
			ItemSlot = n;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ItemCount != 1)
		{
			gNotice.GCNoticeSend(lpObj->Index, eMessageBox, 0, 0, 0, 0, 0, "Vật phẩm không phù hợp !");
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	lpObj->ChaosMoney = gJewelOfHarmonyOption.GetJewelOfHarmonyItemRestoreMoney(&lpObj->ChaosBox[ItemSlot]);

	if(lpObj->ChaosMoney == -1)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	CItem item = lpObj->ChaosBox[ItemSlot];

	item.m_JewelOfHarmonyOption = JEWEL_OF_HARMONY_OPTION_NONE;

	this->ChaosBoxInit(lpObj);
	gItemManager.ChaosBoxAddItem(lpObj->Index, item, 0);
	this->GCChaosMixSend(lpObj->Index, 1, &item);
	gLog.Output(LOG_CHAOS_MIX, "[Xóa gia cường][Thành Công][%s/%s] Item: %s+%d, %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
				, lpObj->Account
				, lpObj->Name
				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, lpObj->ChaosSuccessRate
				, lpObj->ChaosMoney				
				, item.m_Serial);	

}

void CChaosBox::Item380Mix(LPOBJ lpObj) // OK
{
	int GuardianCount = 0;
	int HarmonyCount = 0;
	int ItemCount = 0;
	int ItemSlot = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,31))
		{
			GuardianCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,42))
		{
			HarmonyCount++;
		}
		else if(lpObj->ChaosBox[n].Is380Item() == 0 && lpObj->ChaosBox[n].m_Level >= 4 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemSlot = n;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (GuardianCount != 1 || HarmonyCount != 1 || ItemCount != 1)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	if(lpObj->ChaosBox[ItemSlot].m_Level >= 7)
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Item380MixRate2[lpObj->AccountLevel];
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Item380MixRate1[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 10000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);


	CItem item = lpObj->ChaosBox[ItemSlot];

	if ((GetLargeRand() % 100) < lpObj->ChaosSuccessRate)
	{
		CItem item = lpObj->ChaosBox[ItemSlot];

		item.m_ItemOptionEx |= 0x80;

		this->ChaosBoxInit(lpObj);
		gItemManager.ChaosBoxAddItem(lpObj->Index, item, 0);
		this->GCChaosMixSend(lpObj->Index, 1, &item);
		gLog.Output(LOG_CHAOS_MIX, "[Item380Mix][Thành Công][%s/%s]  %s+%d,  %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
				, lpObj->Account
				, lpObj->Name
				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, lpObj->ChaosSuccessRate
				, lpObj->ChaosMoney				
				, item.m_Serial);		

	}
	else
	{
		this->ChaosBoxItemKeep(lpObj, ItemSlot);
		this->GCChaosBoxSend(lpObj, 0);
		this->GCChaosMixSend(lpObj->Index, 0, 0);
	gLog.Output(LOG_CHAOS_MIX, "[Item380Mix][Xịt][%s/%s] ITEM: %s+%d,  %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
				, lpObj->Account
				, lpObj->Name
				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, lpObj->ChaosSuccessRate
				, lpObj->ChaosMoney				
				, item.m_Serial);		

	}
}

void CChaosBox::IllusionTempleMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int ScrollCount = 0;
	int ScrollLevel = 0;
	int PotionCount = 0;
	int PotionLevel = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,49))
		{
			ScrollCount++;
			ScrollLevel = lpObj->ChaosBox[n].m_Level;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,50))
		{
			PotionCount++;
			PotionLevel = lpObj->ChaosBox[n].m_Level;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || ScrollCount != 1 || PotionCount != 1 || ScrollLevel != PotionLevel)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}

		if (ScrollLevel > 6 || PotionLevel > 6)
		{
			this->GCChaosMixSend(lpObj->Index, 8, 0);
			return;
		}
	}

	switch(ScrollLevel)
	{
		case 1:
			lpObj->ChaosSuccessRate = gServerInfo.m_IllusionTempleMixRate1[lpObj->AccountLevel];
			lpObj->ChaosMoney = 3000000;
			break;
		case 2:
			lpObj->ChaosSuccessRate = gServerInfo.m_IllusionTempleMixRate2[lpObj->AccountLevel];
			lpObj->ChaosMoney = 5000000;
			break;
		case 3:
			lpObj->ChaosSuccessRate = gServerInfo.m_IllusionTempleMixRate3[lpObj->AccountLevel];
			lpObj->ChaosMoney = 7000000;
			break;
		case 4:
			lpObj->ChaosSuccessRate = gServerInfo.m_IllusionTempleMixRate4[lpObj->AccountLevel];
			lpObj->ChaosMoney = 9000000;
			break;
		case 5:
			lpObj->ChaosSuccessRate = gServerInfo.m_IllusionTempleMixRate5[lpObj->AccountLevel];
			lpObj->ChaosMoney = 11000000;
			break;
		case 6:
			lpObj->ChaosSuccessRate = gServerInfo.m_IllusionTempleMixRate6[lpObj->AccountLevel];
			lpObj->ChaosMoney = 13000000;
			break;
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(13,51),ScrollLevel,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[IllusionTempleMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[IllusionTempleMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::FeatherOfCondorMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;
	int SoulPack10 = 0;
	int WingCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int ItemSlot = 0;
	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,31) && lpObj->ChaosBox[n].m_Level == 0)
		{
			SoulPack10++;
		}
		else if(((lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,3) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,6)) || lpObj->ChaosBox[n].m_Index == GET_ITEM(12,42) || lpObj->ChaosBox[n].m_Index == GET_ITEM(12,49) || lpObj->ChaosBox[n].m_Index == GET_ITEM(13,30)) && (lpObj->ChaosBox[n].m_Level >= 9 && lpObj->ChaosBox[n].m_Option3 >= 1))
		{
			WingCount++;
			ItemSlot = n;
		}
		else if(lpObj->ChaosBox[n].IsSetItem() != 0 && lpObj->ChaosBox[n].m_Level >= 7 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || CreationCount != 1 || SoulPack10 != 1 || WingCount != 1 || ItemCount == 0)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	if(gServerInfo.m_FeatherOfCondorMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/300000)+1;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_FeatherOfCondorMixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_FeatherOfCondorMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>60)?60:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = lpObj->ChaosSuccessRate*200000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);
	
	CItem item = lpObj->ChaosBox[ItemSlot];
	if ((GetLargeRand() % 100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, GET_ITEM(13, 53), 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0xFF, 0);
		
		gLog.Output(LOG_CHAOS_MIX, "[Lông Vũ 3][Thành Công][%s/%s]Từ: %s+%d,  %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
				, lpObj->Account
				, lpObj->Name
				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, lpObj->ChaosSuccessRate
				, lpObj->ChaosMoney				
				, item.m_Serial);		

	}
	else
	{
		this->ChaosBoxInit(lpObj);
		this->GCChaosBoxSend(lpObj, 0);
		this->GCChaosMixSend(lpObj->Index, 0, 0);
				gLog.Output(LOG_CHAOS_MIX, "[FeatherOfCondorMix][Thất bại][%s/%s] Từ: %s+%d,  %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
				, lpObj->Account
				, lpObj->Name
				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, lpObj->ChaosSuccessRate
				, lpObj->ChaosMoney				
				, item.m_Serial);

	}
}

void CChaosBox::Wing3Mix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;
	int SoulPack10 = 0;
	int BlessPack10 = 0;
	int FlameCount = 0;
	int FeatherCount = 0;
	int ItemCount = 0;
	int ItemMoney = 0;
	int ItemSlot = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,31) && lpObj->ChaosBox[n].m_Level == 0)
		{
			SoulPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,30) && lpObj->ChaosBox[n].m_Level == 0)
		{
			BlessPack10++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,52))
		{
			FlameCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,53))
		{
			FeatherCount++;
		}
		else if(lpObj->ChaosBox[n].IsExcItem() != 0 && lpObj->ChaosBox[n].m_Level >= 9 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			ItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
			ItemSlot = n;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || CreationCount != 1 || SoulPack10 != 1 || BlessPack10 != 1 || FlameCount != 1 || FeatherCount != 1 || ItemCount == 0)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/3000000) + 1;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_Wing3MixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_Wing3MixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40)?40:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = lpObj->ChaosSuccessRate*200000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	
	CItem item = lpObj->ChaosBox[ItemSlot];

	if ((GetLargeRand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;
		BYTE ItemNewOption = 0;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,36),1);

		RandomManager.AddElement(GET_ITEM(12,37),1);

		RandomManager.AddElement(GET_ITEM(12,38),1);

		RandomManager.AddElement(GET_ITEM(12,39),1);

		for (int i=0; i < gCustomWingMix.m_count; i++)
		{
			RandomManager.AddElement(GET_ITEM(gCustomWingMix.m_Data[i].m_Category,gCustomWingMix.m_Data[i].m_Index),1);
		}

		if (gServerInfo.m_DisableWingMixDL != 1)
		{
			RandomManager.AddElement(GET_ITEM(12,40),1);
		}
		
		if (gServerInfo.m_DisableWingMixSU != 1)
		{
			RandomManager.AddElement(GET_ITEM(12,43),1);
		}

		#if(GAMESERVER_UPDATE>=601)

		if (gServerInfo.m_DisableWingMixRF != 1)
		{
			RandomManager.AddElement(GET_ITEM(12,50),1);
		}

		#endif

		RandomManager.GetRandomElement(&ItemIndex);
		gItemOptionRate.GetItemOption2(6, &ItemOption2);
		gItemOptionRate.GetItemOption3(6, &ItemOption3);
		gItemOptionRate.GetItemOption4(6, &ItemNewOption);
		gItemOptionRate.MakeNewOption(ItemIndex, ItemNewOption, &ItemNewOption);
		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 0, ItemOption2, ItemOption3, -1, (ItemNewOption + (16 * (GetLargeRand() % 3))), 0, 0, 0, 0, 0xFF, 0);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(902), lpObj->Name, gItemManager.GetItemName(ItemIndex));

		gLog.Output(LOG_CHAOS_MIX, "[Wing3Mix][Thành Công][%s/%s] Tạo Thành: %s, Luck: %d, Op: %d, Hoàn hảo: %d,  Từ: %s+%d,  %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
				, lpObj->Account
				, lpObj->Name
				,gItemManager.GetItemName(ItemIndex),ItemOption2,ItemOption3,ItemNewOption

				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, lpObj->ChaosSuccessRate
				, lpObj->ChaosMoney				
				, item.m_Serial);		

	}
	else
	{
		this->ChaosBoxInit(lpObj);
		this->GCChaosBoxSend(lpObj, 0);
		this->GCChaosMixSend(lpObj->Index, 0, 0);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(891), lpObj->Name);

		gLog.Output(LOG_CHAOS_MIX, "[Wing3Mix][Xịt][%s/%s] Từ: %s+%d,  %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
				, lpObj->Account
				, lpObj->Name
				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, lpObj->ChaosSuccessRate
				, lpObj->ChaosMoney				
				, item.m_Serial);		

	}
}


void CChaosBox::CustomItemMix(LPOBJ lpObj, int HeadCode) // OK
{
	if (lpObj->CustomIndexMix == -1)
	{
		return;
	}

	BCUSTOM_MIX* CustomMix = gCustomMix.BGetCustomMix(HeadCode);

	if (CustomMix != 0)
	{
		int OptionExcelente1[6] = { 1, 2, 4, 8, 16, 32 };

		int OptionExcelente2[15] = { 3, 5, 6, 9, 10, 12, 17, 18, 20, 24, 33, 34, 36, 40, 48 };

		int OptionExcelente3[20] = { 7, 11, 13, 14, 19, 21, 22, 25, 26, 28, 35, 37, 38, 41, 42, 44, 49, 50, 52, 56 };

		int OptionExcelente4[15] = { 15, 23, 27, 29, 30, 39, 43, 45, 46, 51, 53, 54, 57, 58, 60 };

		int OptionExcelente5[6] = { 31, 47, 55, 59, 61, 62 };

		int GetTongNguyenLieu = gCustomMix.GetSizeNguyenLieu(HeadCode);

		int NguyenLieuDatVao = 0;

		for (int i = 0; i < gCustomMix.m_BDataMixNguyenLieu.size(); i++)
		{
			if (HeadCode != gCustomMix.m_BDataMixNguyenLieu[i].Index) continue;

			int Count = gCustomMix.GetChaoMixItemCount(lpObj, gCustomMix.m_BDataMixNguyenLieu[i].ItemIndexMin, gCustomMix.m_BDataMixNguyenLieu[i].ItemIndexMax
				, gCustomMix.m_BDataMixNguyenLieu[i].Level, gCustomMix.m_BDataMixNguyenLieu[i].Level, gCustomMix.m_BDataMixNguyenLieu[i].Op, gCustomMix.m_BDataMixNguyenLieu[i].Luck, gCustomMix.m_BDataMixNguyenLieu[i].Skill,
				gCustomMix.m_BDataMixNguyenLieu[i].Exl, gCustomMix.m_BDataMixNguyenLieu[i].Item380, gCustomMix.m_BDataMixNguyenLieu[i].Set, gCustomMix.m_BDataMixNguyenLieu[i].Harmory, gCustomMix.m_BDataMixNguyenLieu[i].Socket);

			if (Count >= gCustomMix.m_BDataMixNguyenLieu[i].Count)
			{
				NguyenLieuDatVao++;
			}

		}


		if (GetTongNguyenLieu > NguyenLieuDatVao) // Khong du nguyen lieu Mix
		{
			//LogAdd(LOG_RED, "Khong du Nguyen Lieu %d <> %d", GetTongNguyenLieu , NguyenLieuDatVao);
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}

		if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
		{
			//LogAdd(LOG_RED, "GetTalismanOfLuckRate %d", lpObj->ChaosSuccessRate);
			this->GCChaosMixSend(lpObj->Index, 240, 0);
			return;
		}

		lpObj->ChaosSuccessRate = CustomMix->MixRate[lpObj->AccountLevel];

		lpObj->ChaosMoney = CustomMix->MixMoney;

		int TaxMoney = (lpObj->ChaosMoney * gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

		lpObj->ChaosMoney += TaxMoney;

		if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
		{
			this->GCChaosMixSend(lpObj->Index, 2, 0);
			return;
		}

		lpObj->Money -= lpObj->ChaosMoney;

		GCMoneySend(lpObj->Index, lpObj->Money);

		gCastleSiegeSync.AddTributeMoney(TaxMoney);


		int NumberData = 0;
		WORD ItemIndex = 0;
		BYTE ItemLevel = 0;
		BYTE ItemSkill = 0;
		BYTE ItemLuck = 0;
		BYTE ItemOption = 0;
		BYTE ItemExcellent = 0;
		BYTE ItemSetAncestral = 0;

		if ((GetLargeRand() % 100) < lpObj->ChaosSuccessRate)
		{
			BYTE ItemSocketOption[MAX_SOCKET_OPTION] = { SOCKET_ITEM_OPTION_NONE,SOCKET_ITEM_OPTION_NONE,SOCKET_ITEM_OPTION_NONE,SOCKET_ITEM_OPTION_NONE,SOCKET_ITEM_OPTION_NONE };

			BCUSTOM_MIX_ITEMKETQUA* ItemCreate = nullptr;
			if (gCustomMix.m_BDataMixItemKetQua.size() > 1)
			{
				CRandomManager RandomManager = gCustomMix.BRandomManager(HeadCode);

				RandomManager.GetRandomElement(&NumberData);

				ItemCreate = &gCustomMix.m_BDataMixItemKetQua[NumberData];
			}
			else
			{
				ItemCreate = &gCustomMix.m_BDataMixItemKetQua[0];
			}

			//LogAdd(LOG_RED, "ItemCreate %d", NumberData);
			if (!ItemCreate)
			{
				LogAdd(LOG_RED, "Khong co item kq %d", HeadCode);
				return;
			}
			ItemIndex = ItemCreate->ItemIndex;

			if (CustomMix->CheckItemNewOption != 0)
			{
				gItemOptionRate.GetItemOption4(CustomMix->CheckItemNewOption, &ItemExcellent);
			}
			else
			{
				ItemExcellent = ItemCreate->Exl;
				//switch (ItemCreate->Exl)
				//{
				//case 1:
				//	ItemExcellent = OptionExcelente1[rand() % 6];
				//	break;
				//case 2:
				//	ItemExcellent = OptionExcelente2[rand() % 15];
				//	break;
				//case 3:
				//	ItemExcellent = OptionExcelente3[rand() % 20];
				//	break;
				//case 4:
				//	ItemExcellent = OptionExcelente4[rand() % 15];
				//	break;
				//case 5:
				//	ItemExcellent = OptionExcelente5[rand() % 6];
				//	break;
				//case 6:
				//	ItemExcellent = 63;
				//	break;
				//default:
				//	ItemExcellent = 0;
				//	break;
				//}
			}

			if (CustomMix->CheckItemOption != 0)
			{
				gItemOptionRate.GetItemOption3(CustomMix->CheckItemOption, &ItemOption);
			}
			else
			{
				if (ItemCreate->Op > 7)
				{
					ItemOption = 7;
				}
				else if (ItemCreate->Op < 0)
				{
					ItemOption = 0;
				}
				else
				{
					ItemOption = ItemCreate->Op;
				}
			}


			int qtd = gSocketItemType.GetSocketItemMaxSocket(ItemIndex);

			ItemSocketOption[0] = (BYTE)((qtd > 0) ? ((ItemCreate->Socket[0] != 255) ? ItemCreate->Socket[0] : 255) : SOCKET_ITEM_OPTION_NONE);
			ItemSocketOption[1] = (BYTE)((qtd > 1) ? ((ItemCreate->Socket[1] != 255) ? ItemCreate->Socket[1] : 255) : SOCKET_ITEM_OPTION_NONE);
			ItemSocketOption[2] = (BYTE)((qtd > 2) ? ((ItemCreate->Socket[2] != 255) ? ItemCreate->Socket[2] : 255) : SOCKET_ITEM_OPTION_NONE);
			ItemSocketOption[3] = (BYTE)((qtd > 3) ? ((ItemCreate->Socket[3] != 255) ? ItemCreate->Socket[3] : 255) : SOCKET_ITEM_OPTION_NONE);
			ItemSocketOption[4] = (BYTE)((qtd > 4) ? ((ItemCreate->Socket[4] != 255) ? ItemCreate->Socket[4] : 255) : SOCKET_ITEM_OPTION_NONE);

			GDCreateItemSend(lpObj->Index,
				0xFF, 0, 0, ItemIndex,
				(ItemCreate->Level != -1) ? ItemCreate->Level : rand() % 6, 0,
				((ItemCreate->Skill != 0) ? gItemManager.CheckItemSkill(ItemIndex) : 0),
				(ItemCreate->Luck != -1) ? 1 : rand() % 2,
				ItemOption,
				-1, ItemExcellent,
				(ItemCreate->Set != -1) ? ItemCreate->Set : rand() % 2,
				0, 0, ItemSocketOption, 0xFF, 0);


			ITEM_INFO ItemInfo;
			gItemManager.GetInfo(ItemIndex, &ItemInfo);
			char thongbaomix[256];
			wsprintf(thongbaomix, "[%s] Nâng cấp [%s] thành công !", lpObj->Name, ItemInfo.Name);
			if (gServerInfo.m_AnnounceChaosMix == 2 && gServerInfo.m_CustommixThanhCong == 1)
			{
				gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, thongbaomix);
			}
			else if (gServerInfo.m_AnnounceChaosMix == 1 && gServerInfo.m_CustommixThanhCong == 1)
			{
				GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0, 0, 0, thongbaomix);
			}

			//gLog.Output(LOG_CHAOS_MIX, "[CustomMix][Success][%s][%s] [%d] - (ChaosSuccessRate: %d, ChaosMoney: %d) HeadCode %d", lpObj->Account, lpObj->Name, ItemIndex, lpObj->ChaosSuccessRate, lpObj->ChaosMoney, HeadCode);
			gLog.Output(LOG_CHAOS_MIX, "CustomMix[%s/%s]HeadCode %d - ID:[%d]:Chế tạo %s+%d,  %d %d  %d, %d,  %03d, %03d, %03d, %03d, %03d, Rate: %d%% )"
				, lpObj->Account
				, lpObj->Name, HeadCode,ItemIndex,
				gItemManager.GetItemName(ItemIndex), (ItemCreate->Level != -1) ? ItemCreate->Level : rand() % 6,
				(ItemCreate->Luck != -1) ? 1 : rand() % 2,

				ItemOption, ItemExcellent, ItemSetAncestral, ItemSocketOption[0], ItemSocketOption[1], ItemSocketOption[2], ItemSocketOption[3], ItemSocketOption[4]

				, lpObj->ChaosSuccessRate);
		}
		else
		{
			//this->ChaosBoxInit(lpObj);
			for (int n = 0; n < CHAOS_BOX_SIZE; n++)
			{
				if (!lpObj->ChaosBox[n].IsItem()) continue;

				int FailDelItem = gCustomMix.GetFailDelItem(lpObj->ChaosBox[n].m_Index, HeadCode);
				//LogAdd(LOG_RED, "GetFailDelItem [%d] %d", lpObj->ChaosBox[n].m_Index, FailDelItem);
				switch (FailDelItem)
				{
				case 0:
				{
					continue;
				}
				break;
				case 2:
				{
					CItem* lpItem = &lpObj->ChaosBox[n];

					lpItem->m_Level = 0;

					float dur = (float)gItemManager.GetItemDurability(lpItem->m_Index, lpItem->m_Level, lpItem->IsExcItem(), lpItem->IsSetItem());

					lpItem->m_Durability = dur * (lpItem->m_Durability / lpItem->m_BaseDurability);

					lpItem->Convert(lpItem->m_Index, lpItem->m_Option1, lpItem->m_Option2, lpItem->m_Option3, lpItem->m_NewOption, lpItem->m_SetOption, lpItem->m_JewelOfHarmonyOption, lpItem->m_ItemOptionEx, lpItem->m_SocketOption, lpItem->m_SocketOptionBonus);
				}
				break;
				default:
					lpObj->ChaosBox[n].Clear();
					lpObj->ChaosBoxMap[n] = 0xFF;
					break;
				}

			}

			this->GCChaosBoxSend(lpObj, 0);

			this->GCChaosMixSend(lpObj->Index, 0, 0);

			ITEM_INFO ItemInfo;
			gItemManager.GetInfo(ItemIndex, &ItemInfo);

			gLog.Output(LOG_CHAOS_MIX, "[CustomMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d) HeadCode %d", lpObj->Account, lpObj->Name, lpObj->ChaosSuccessRate, lpObj->ChaosMoney, HeadCode);
		}
	}
	else
	{
		gLog.Output(LOG_CHAOS_MIX, "[CustomMix][Failure][%s][%s] - CustomMix No Existente HeadCode %d", lpObj->Account, lpObj->Account, lpObj->Name, HeadCode);
	}
}

void CChaosBox::ChaosCardMix(LPOBJ lpObj,int type) // OK
{
	int ChaosCardCount = 0;
	int ChaosCardGoldCount = 0;
	int ChaosCardRareCount = 0;
	int ChaosCardMiniCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,54))
		{
			type = 1;
			ChaosCardCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,92))
		{
			type = 2;
			ChaosCardGoldCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,93))
		{
			type = 3;
			ChaosCardRareCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,95))
		{
			type = 4;
			ChaosCardMiniCount++;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (type == 0 || (type == 1 && ChaosCardCount != 1) || (type == 2 && ChaosCardGoldCount != 1) || (type == 3 && ChaosCardRareCount != 1) || (type == 4 && ChaosCardMiniCount != 1))
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	CItem item;

	if(gItemBagManager.GetItemBySpecialValue((ITEM_BAG_CHAOS_CARD_MIX1+(type-1)),lpObj,&item) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	GDCreateItemSend(lpObj->Index,0xFF,0,0,item.m_Index,(BYTE)item.m_Level,0,(BYTE)item.m_Option1,(BYTE)item.m_Option2,(BYTE)item.m_Option3,-1,item.m_NewOption,item.m_SetOption,item.m_JewelOfHarmonyOption,item.m_ItemOptionEx,item.m_SocketOption,item.m_SocketOptionBonus,0);

	gLog.Output(LOG_CHAOS_MIX,"[ChaosCardMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
}

void CChaosBox::CherryBlossomMix(LPOBJ lpObj,int type) // OK
{
	int WhiteCherryBlossomCount = 0;
	int RedCherryBlossomCount = 0;
	int GoldenCherryBlossomCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,88))
		{
			type = 1;
			WhiteCherryBlossomCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,89))
		{
			type = 2;
			RedCherryBlossomCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,90))
		{
			type = 3;
			GoldenCherryBlossomCount += (int)lpObj->ChaosBox[n].m_Durability;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (type == 0 || (type == 1 && WhiteCherryBlossomCount != 10) || (type == 2 && RedCherryBlossomCount != 30) || (type == 3 && GoldenCherryBlossomCount != 255))
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}
	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	CItem item;

	if(gItemBagManager.GetItemBySpecialValue((ITEM_BAG_CHERRY_BLOSSOM_MIX1+(type-1)),lpObj,&item) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	GDCreateItemSend(lpObj->Index,0xFF,0,0,item.m_Index,(BYTE)item.m_Level,0,(BYTE)item.m_Option1,(BYTE)item.m_Option2,(BYTE)item.m_Option3,-1,item.m_NewOption,item.m_SetOption,item.m_JewelOfHarmonyOption,item.m_ItemOptionEx,item.m_SocketOption,item.m_SocketOptionBonus,0);

	gLog.Output(LOG_CHAOS_MIX,"[CherryBlossomMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
}

void CChaosBox::SocketItemCreateSeedMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;
	int HarmonyCount = 0;
	int ExcItemCount = 0;
	int SetItemCount = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,42))
		{
			HarmonyCount++;
		}
		else if(lpObj->ChaosBox[n].IsExcItem() != 0 && lpObj->ChaosBox[n].m_Level >= 4 && ExcItemCount == 0)
		{
			ExcItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		else if(lpObj->ChaosBox[n].IsSetItem() != 0 && lpObj->ChaosBox[n].m_Level >= 4 && SetItemCount == 0)
		{
			SetItemCount++;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || CreationCount != 1 || HarmonyCount != 1 || ExcItemCount != 1 || SetItemCount != 1)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	if(gServerInfo.m_SocketItemCreateSeedMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/2000000)+80;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SocketItemCreateSeedMixRate[lpObj->AccountLevel];
	}

	if(gServerInfo.m_SocketItemCreateSeedMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90)?90:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		int SocketOptionType = 1+(GetLargeRand()%MAX_SOCKET_ITEM_OPTION_TYPE);

		int SocketOptionIndex = gSocketItemOption.GetSocketItemRandomOption(SocketOptionType);

		int SocketOptionValue = gSocketItemOption.GetSocketItemSeedOption(SocketOptionIndex,SocketOptionType);

		if(SocketOptionIndex == -1 || SocketOptionValue == -1)
		{
			this->GCChaosMixSend(lpObj->Index,0,0);
			return;
		}

		GDCreateItemSend(lpObj->Index,0xFF,0,0,(GET_ITEM(12,60)+(SocketOptionType-1)),SocketOptionValue,0,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketItemCreateSeedMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketItemCreateSeedMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::SocketItemCreateSeedSphereMix(LPOBJ lpObj) // OK
{
	int ChaosCount = 0;
	int CreationCount = 0;
	int SeedCount = 0;
	int SeedType = 0;
	int SeedLevel = 0;
	int SphereCount = 0;
	int SphereType = 0;
	int ItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,60) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,66))
		{
			SeedCount++;
			SeedType = lpObj->ChaosBox[n].m_Index-GET_ITEM(12,60);
			SeedLevel = lpObj->ChaosBox[n].m_Level;
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,70) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,74))
		{
			SphereCount++;
			SphereType = lpObj->ChaosBox[n].m_Index-GET_ITEM(12,70);
			ItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || CreationCount != 1 || SeedCount != 1 || SphereCount != 1)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	if(gServerInfo.m_SocketItemCreateSeedSphereMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = (ItemMoney/200000)+80;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SocketItemCreateSeedSphereMixRate[lpObj->AccountLevel];
	}

	if(gServerInfo.m_SocketItemCreateSeedSphereMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>90)?90:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000;
	
	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;
	
	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,(GET_ITEM(12,100)+(SphereType*6)+SeedType),SeedLevel,0,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketItemCreateSeedSphereMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketItemCreateSeedSphereMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
}

void CChaosBox::SocketItemMountSeedSphereMix(LPOBJ lpObj,BYTE info) // OK
{
	if(info >= MAX_SOCKET_ITEM_OPTION_TABLE)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	int ChaosCount = 0;
	int CreationCount = 0;
	int SeedSphereCount = 0;
	int SeedSphereType = 0;
	int SeedSphereLevel = 0;
	int SeedSphereOption = 0;
	int ItemCount = 0;
	int ItemSlot = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,22))
		{
			CreationCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(12,100) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12,129))
		{
			SeedSphereCount++;
			SeedSphereType = (lpObj->ChaosBox[n].m_Index-GET_ITEM(12,100))%MAX_SOCKET_ITEM_OPTION_TYPE;
			SeedSphereLevel = (lpObj->ChaosBox[n].m_Index-GET_ITEM(12,100))/MAX_SOCKET_ITEM_OPTION_TYPE;
			SeedSphereOption = lpObj->ChaosBox[n].m_Level;
		}
		else if(gSocketItemType.CheckSocketItemType(lpObj->ChaosBox[n].m_Index) != 0 && lpObj->ChaosBox[n].m_SocketOption[info] == SOCKET_ITEM_OPTION_EMPTY)
		{
			ItemCount++;
			ItemSlot = n;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{

		if (ChaosCount != 1 || CreationCount != 1 || SeedSphereCount != 1 || ItemCount != 1)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}

		//limite socket
		if (info >= gSocketItemType.GetSocketItemMaxSocket(lpObj->ChaosBox[ItemSlot].m_Index))
		{
			this->GCChaosMixSend(lpObj->Index, 0, 0);
			return;
		}
	}

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	//limite socket
	if(info >= gSocketItemType.GetSocketItemMaxSocket(lpObj->ChaosBox[ItemSlot].m_Index))
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	lpObj->ChaosMoney = 1000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;



	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	BYTE SocketOption = gSocketItemOption.GetSocketItemOption(&lpObj->ChaosBox[ItemSlot],(SeedSphereType+1),SeedSphereOption,SeedSphereLevel);

	if(SocketOption == SOCKET_ITEM_OPTION_NONE)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	CItem item = lpObj->ChaosBox[ItemSlot];

	item.m_SocketOption[info] = SocketOption;

	item.m_SocketOptionBonus = gSocketItemOption.GetSocketItemBonusOption(&item);

	this->ChaosBoxInit(lpObj);

	gItemManager.ChaosBoxAddItem(lpObj->Index,item,0);

	this->GCChaosMixSend(lpObj->Index,1,&item);

	gLog.Output(LOG_CHAOS_MIX, "[Khảm Ngọc][Thành Công][%s/%s] Item: %s+%d,  %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
				, lpObj->Account
				, lpObj->Name
				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, lpObj->ChaosSuccessRate
				, lpObj->ChaosMoney				
				, item.m_Serial);
	
}

void CChaosBox::SocketItemUnMountSeedSphereMix(LPOBJ lpObj,BYTE info) // OK
{
	if(info >= MAX_SOCKET_ITEM_OPTION_TABLE)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	int ItemCount = 0;
	int ItemSlot = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_SocketOption[info] != SOCKET_ITEM_OPTION_NONE && lpObj->ChaosBox[n].m_SocketOption[info] != SOCKET_ITEM_OPTION_EMPTY)
		{
			ItemCount++;
			ItemSlot = n;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ItemCount != 1)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	lpObj->ChaosMoney = 1000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	CItem item = lpObj->ChaosBox[ItemSlot];

	item.m_SocketOption[info] = SOCKET_ITEM_OPTION_EMPTY;

	item.m_SocketOptionBonus = gSocketItemOption.GetSocketItemBonusOption(&item);

	this->ChaosBoxInit(lpObj);
	gItemManager.ChaosBoxAddItem(lpObj->Index, item, 0);
	this->GCChaosMixSend(lpObj->Index, 1, &item);
			gLog.Output(LOG_CHAOS_MIX, "[Tháo Ngọc][Thành Công][%s/%s] Item: %s+%d,  %d %d %d %d %d, HC: %03d, 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
				, lpObj->Account
				, lpObj->Name
				, gItemManager.GetItemName(item.m_Index)
				, item.m_Level
				, item.m_Option1
				, item.m_Option2
				, item.m_Option3 * 4
				, item.m_NewOption
				, item.m_SetOption
				, item.m_JewelOfHarmonyOption
				, item.m_ItemOptionEx
				, item.m_SocketOption[0]
				, item.m_SocketOption[1]
				, item.m_SocketOption[2]
				, item.m_SocketOption[3]
				, item.m_SocketOption[4]
				, lpObj->ChaosSuccessRate
				, lpObj->ChaosMoney				
				, item.m_Serial);
}

void CChaosBox::ImperialGuardianMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	int DimensionalPart1Count = 0;
	int DimensionalPart2Count = 0;
	int DimensionalPart3Count = 0;
	int DimensionalPart4Count = 0;
	int DimensionalPart5Count = 0;
	int DimensionalPart6Count = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,103))
		{
			DimensionalPart1Count++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,104))
		{
			DimensionalPart2Count++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,105))
		{
			DimensionalPart3Count++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,106))
		{
			DimensionalPart4Count++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,107))
		{
			DimensionalPart5Count++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,108))
		{
			DimensionalPart6Count++;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (DimensionalPart1Count != 1 && DimensionalPart2Count != 1 && DimensionalPart3Count != 1 && DimensionalPart4Count != 1 && DimensionalPart5Count != 1 && DimensionalPart6Count != 1)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}
	lpObj->ChaosMoney = 1000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(14,109),0,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

	gLog.Output(LOG_CHAOS_MIX,"[ImperialGuardianMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);

	#endif
}


void CChaosBox::ChestMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	int GoldKeyCount = 0;
	int GoldChestCount = 0;
	int SilverKeyCount = 0;
	int SilverChestCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,112))
		{
			SilverKeyCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,113))
		{
			GoldKeyCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,121))
		{
			GoldChestCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,122))
		{
			SilverChestCount++;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if ((GoldKeyCount != 1 && SilverKeyCount != 1) || (GoldKeyCount == 1 && GoldChestCount != 1) || (SilverKeyCount == 1 && SilverChestCount != 1))
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}
	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	GDCreateItemSend(lpObj->Index,0xFF,0,0,(GET_ITEM(14,123)+SilverKeyCount),0,0,0,0,0,-1,0,0,0,0,0,0xFF,0);

	gLog.Output(LOG_CHAOS_MIX,"[ChestMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);

	#endif
}

void CChaosBox::SummonScrollMix(LPOBJ lpObj,int type) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	int SummonScroll1Count = 0;
	int SummonScroll2Count = 0;
	int SummonScroll3Count = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,210))
		{
			type = 1;
			SummonScroll1Count++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,211))
		{
			type = 2;
			SummonScroll2Count++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,212))
		{
			type = 3;
			SummonScroll3Count++;
		}
	}

	if(type == 0 || (type == 1 && SummonScroll1Count != 2) || (type == 2 && (SummonScroll1Count != 1 || SummonScroll2Count != 1)) || (type == 3 && (SummonScroll1Count != 1 || SummonScroll3Count != 1)))
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	switch(type)
	{
		case 1:
			lpObj->ChaosSuccessRate = gServerInfo.m_SummonScrollMixRate1[lpObj->AccountLevel];
			lpObj->ChaosMoney = 100000;
			break;
		case 2:
			lpObj->ChaosSuccessRate = gServerInfo.m_SummonScrollMixRate2[lpObj->AccountLevel];
			lpObj->ChaosMoney = 200000;
			break;
		case 3:
			lpObj->ChaosSuccessRate = gServerInfo.m_SummonScrollMixRate3[lpObj->AccountLevel];
			lpObj->ChaosMoney = 300000;
			break;
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,(GET_ITEM(14,210)+type),0,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SummonScrollMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[SummonScrollMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	#endif
}

void CChaosBox::LuckyItemCreateMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	int LuckyTicketCount = 0;
	int LuckyTicketGroup = 0;
	int LuckyTicketSection = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,135))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 0;
			LuckyTicketSection = 8;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,136))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 0;
			LuckyTicketSection = 9;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,137))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 0;
			LuckyTicketSection = 7;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,138))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 0;
			LuckyTicketSection = 10;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,139))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 0;
			LuckyTicketSection = 11;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,140))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 1;
			LuckyTicketSection = 8;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,141))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 1;
			LuckyTicketSection = 9;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,142))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 1;
			LuckyTicketSection = 7;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,143))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 1;
			LuckyTicketSection = 10;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(13,144))
		{
			LuckyTicketCount++;
			LuckyTicketGroup = 1;
			LuckyTicketSection = 11;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (LuckyTicketCount != 1)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	WORD ItemIndex = 0;
	BYTE ItemLevel = 0;
	BYTE ItemOption1 = 0;
	BYTE ItemOption2 = 0;
	BYTE ItemOption3 = 0;
	BYTE ItemNewOption = 0;
	BYTE ItemSetOption = 0;
	BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

	if(gLuckyItem.GetLuckyItemIndex(lpObj,LuckyTicketSection,LuckyTicketGroup,&ItemIndex) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,0,0);
		return;
	}

	gLuckyItem.GetLuckyItemOption0(ItemIndex,&ItemLevel);

	gLuckyItem.GetLuckyItemOption1(ItemIndex,&ItemOption1);

	gLuckyItem.GetLuckyItemOption2(ItemIndex,&ItemOption2);

	gLuckyItem.GetLuckyItemOption3(ItemIndex,&ItemOption3);

	gLuckyItem.GetLuckyItemOption4(ItemIndex,&ItemNewOption);

	gLuckyItem.GetLuckyItemOption5(ItemIndex,&ItemSetOption);

	gLuckyItem.GetLuckyItemOption6(ItemIndex,&ItemSocketOption[0]);

	gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

	gItemOptionRate.MakeSetOption(ItemIndex,ItemSetOption,&ItemSetOption);

	gItemOptionRate.MakeSocketOption(ItemIndex,ItemSocketOption[0],&ItemSocketOption[0]);

	GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,ItemLevel,255,ItemOption1,ItemOption2,ItemOption3,-1,ItemNewOption,ItemSetOption,0,0,ItemSocketOption,0xFF,0);

	gLog.Output(LOG_CHAOS_MIX,"[LuckyItemCreateMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);

	#endif
}

void CChaosBox::LuckyItemRefineMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	int LuckyItemCount = 0;
	int LuckyItemSlot = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].IsLuckyItem() != 0)
		{
			LuckyItemCount++;
			LuckyItemSlot = n;
		}
	}

	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (LuckyItemCount != 1)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	if(lpObj->ChaosBox[LuckyItemSlot].m_Durability == 255)
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_LuckyItemRefineMixRate1[lpObj->AccountLevel];
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_LuckyItemRefineMixRate2[lpObj->AccountLevel];
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		GDCreateItemSend(lpObj->Index,0xFF,0,0,GET_ITEM(14,160),0,1,0,0,0,-1,0,0,0,0,0,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[LuckyItemRefineMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[LuckyItemRefineMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	#endif
}

void CChaosBox::MonsterWingMix(LPOBJ lpObj) // OK
{

	int ChaosCount = 0;
	int CreationCount = 0;
	int MaterialType = 0;
	int WingItemCount = 0;
	int WingItemMoney = 0;
	int WingItemType = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 15))
		{
			ChaosCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 22))
		{
			CreationCount++;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 170))
		{
			MaterialType = 1;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 171))
		{
			MaterialType = 2;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 172))
		{
			MaterialType = 3;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(14, 173))
		{
			MaterialType = 4;
		}
		else if ((lpObj->ChaosBox[n].m_Index >= GET_ITEM(12, 3) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(12, 6)) || lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 42))
		{
			WingItemCount++;
			WingItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
			WingItemType = 0;
		}
		else if (lpObj->ChaosBox[n].m_Index == GET_ITEM(12, 49) || lpObj->ChaosBox[n].m_Index == GET_ITEM(13, 30))
		{
			WingItemCount++;
			WingItemMoney += lpObj->ChaosBox[n].m_BuyMoney;
			WingItemType = 1;
		}
	}
	if (lpObj->ChaosLockShowRate != 100) //CB_GETMIXRATE
	{
		if (ChaosCount != 1 || CreationCount != 1 || MaterialType == 0 || WingItemCount != 1)
		{
			this->GCChaosMixSend(lpObj->Index, 7, 0);
			return;
		}
	}

	if (gServerInfo.m_MonsterWingMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = WingItemMoney / ((WingItemType == 0) ? 9000000 : 500000);
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_MonsterWingMixRate[lpObj->AccountLevel];
	}

	if (this->GetTalismanOfLuckRate(lpObj, &lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index, 240, 0);
		return;
	}

	if (gServerInfo.m_MonsterWingMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate > 60) ? 60 : lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate > 100) ? 100 : lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = lpObj->ChaosSuccessRate * 100000;

	int TaxMoney = (lpObj->ChaosMoney * gCastleSiegeSync.GetTaxRateChaos(lpObj->Index)) / 100;

	lpObj->ChaosMoney += TaxMoney;
	//CB_GETMIXRATE
	if (lpObj->ChaosLockShowRate == 100)  return;

	if (lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index, 2, 0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index, lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	int ItemSlot = 0;
	CItem item = lpObj->ChaosBox[ItemSlot];

	if ((GetLargeRand() % 100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemOption3 = 0;
		BYTE ItemNewOption = 0;

		switch (MaterialType)
		{
		case 1:
			ItemIndex = GET_ITEM(12, 155);
			break;
		case 2:
			ItemIndex = GET_ITEM(12, 150);
			break;
		case 3:
			ItemIndex = GET_ITEM(12, 151);
			break;
		case 4:
			ItemIndex = GET_ITEM(12, 152);
			break;
		}

		gItemOptionRate.GetItemOption2(7, &ItemOption2);

		gItemOptionRate.GetItemOption3(7, &ItemOption3);

		gItemOptionRate.GetItemOption4(7, &ItemNewOption);

		gItemOptionRate.MakeNewOption(ItemIndex, ItemNewOption, &ItemNewOption);

		GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, ItemIndex, 0, 0, 0, ItemOption2, ItemOption3, -1, (ItemNewOption + (16 * (GetLargeRand() % 2))), 0, 0, 0, 0, 0xFF, 0);
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(902), lpObj->Name, gItemManager.GetItemName(ItemIndex));

		gLog.Output(LOG_CHAOS_MIX, "[Wing2.5Mix][Thành Công][Account: %s, Name: %s] Item: %s, Level: %d, Skill: %d, Luck: %d, Tính năng: %d, Hoàn hảo: %d, Thần: %d, Hamorny: %03d, Item 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
			, lpObj->Account
			, lpObj->Name
			, gItemManager.GetItemName(item.m_Index)
			, item.m_Level
			, item.m_Option1
			, item.m_Option2
			, item.m_Option3 * 4
			, item.m_NewOption
			, item.m_SetOption
			, item.m_JewelOfHarmonyOption
			, item.m_ItemOptionEx
			, item.m_SocketOption[0]
			, item.m_SocketOption[1]
			, item.m_SocketOption[2]
			, item.m_SocketOption[3]
			, item.m_SocketOption[4]
			, lpObj->ChaosSuccessRate
			, lpObj->ChaosMoney
			, item.m_Serial);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj, 0);

		this->GCChaosMixSend(lpObj->Index, 0, 0);

		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(903), lpObj->Name);

		gLog.Output(LOG_CHAOS_MIX, "[Wing2.5Mix][Thất Bại][Account: %s, Name: %s] Item: %s, Level: %d, Skill: %d, Luck: %d, Tính năng: %d, Hoàn hảo: %d, Thần: %d, Hamorny: %03d, Item 380: %03d, Socket: %03d, %03d, %03d, %03d, %03d, Rate: %d%%, Money: %d, Serial: %08X)"
			, lpObj->Account
			, lpObj->Name
			, gItemManager.GetItemName(item.m_Index)
			, item.m_Level
			, item.m_Option1
			, item.m_Option2
			, item.m_Option3 * 4
			, item.m_NewOption
			, item.m_SetOption
			, item.m_JewelOfHarmonyOption
			, item.m_ItemOptionEx
			, item.m_SocketOption[0]
			, item.m_SocketOption[1]
			, item.m_SocketOption[2]
			, item.m_SocketOption[3]
			, item.m_SocketOption[4]
			, lpObj->ChaosSuccessRate
			, lpObj->ChaosMoney
			, item.m_Serial); 
	}
}


void CChaosBox::SocketWeaponMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int IceWalkerManeCount = 0;
	int MammothBundleOfFurCount = 0;
	int GiantIcicleCount = 0;
	int CoolutinPoisonCount = 0;
	int IronKnightHeartCount = 0;
	int DarkMammothHornCount = 0;
	int DarkGiantAxeCount = 0;
	int DarkCoolutinBloodCount = 0;
	int DarkIronKnightSwordCount = 0;
	int CombinationNoteType = 0;
	int SocketItemCount = 0;
	int SocketItemMoney = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,180))
		{
			IceWalkerManeCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,181))
		{
			MammothBundleOfFurCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,182))
		{
			GiantIcicleCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,183))
		{
			CoolutinPoisonCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,184))
		{
			IronKnightHeartCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,185))
		{
			DarkMammothHornCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,186))
		{
			DarkGiantAxeCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,187))
		{
			DarkCoolutinBloodCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,188))
		{
			DarkIronKnightSwordCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index >= GET_ITEM(14,191) && lpObj->ChaosBox[n].m_Index <= GET_ITEM(14,202))
		{
			CombinationNoteType = lpObj->ChaosBox[n].m_Index-GET_ITEM(14,190);
		}
		else if(gSocketItemType.CheckSocketItemType(lpObj->ChaosBox[n].m_Index) != 0 && lpObj->ChaosBox[n].m_Level >= 7 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			SocketItemCount++;
			SocketItemMoney += lpObj->ChaosBox[n].m_Value;
		}
	}

	if(CombinationNoteType == 0 || SocketItemCount == 0 || (CombinationNoteType == 1 && (IceWalkerManeCount != 2 || DarkIronKnightSwordCount != 1)) || (CombinationNoteType == 2 && (MammothBundleOfFurCount != 2 || DarkMammothHornCount != 1)) || (CombinationNoteType == 3 && (GiantIcicleCount != 2 || DarkIronKnightSwordCount != 1)) || (CombinationNoteType == 4 && (CoolutinPoisonCount != 2 || DarkMammothHornCount != 1)) || (CombinationNoteType == 5 && (GiantIcicleCount != 2 || DarkGiantAxeCount != 1)) || (CombinationNoteType == 6 && (IceWalkerManeCount != 2 || DarkMammothHornCount != 1)) || (CombinationNoteType == 7 && (IronKnightHeartCount != 2 || DarkCoolutinBloodCount != 1)) || (CombinationNoteType == 8 && (GiantIcicleCount != 2 || DarkMammothHornCount != 1)) || (CombinationNoteType == 9 && (IronKnightHeartCount != 2 || DarkGiantAxeCount != 1)) || (CombinationNoteType == 10 && (IronKnightHeartCount != 2 || DarkIronKnightSwordCount != 1)) || (CombinationNoteType == 11 && (MammothBundleOfFurCount != 2 || DarkCoolutinBloodCount != 1)) || (CombinationNoteType == 12 && (IronKnightHeartCount != 2 || DarkMammothHornCount != 1)))
	{
		this->GCChaosMixSend(lpObj->Index,7,0);
		return;
	}

	if(gServerInfo.m_SocketWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = SocketItemMoney/360000;
	}
	else
	{
		lpObj->ChaosSuccessRate = gServerInfo.m_SocketWeaponMixRate[lpObj->AccountLevel];
	}

	if(this->GetTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	if(gServerInfo.m_SocketWeaponMixRate[lpObj->AccountLevel] == -1)
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>40)?40:lpObj->ChaosSuccessRate);
	}
	else
	{
		lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);
	}

	lpObj->ChaosMoney = 1000000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,2,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE ItemOption2 = 0;
		BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

		switch(CombinationNoteType)
		{
			case 1:
				ItemIndex = GET_ITEM(0,29);
				break;
			case 2:
				ItemIndex = GET_ITEM(0,36);
				break;
			case 3:
				ItemIndex = GET_ITEM(0,37);
				break;
			case 4:
				ItemIndex = GET_ITEM(3,12);
				break;
			case 5:
				ItemIndex = GET_ITEM(2,20);
				break;
			case 6:
				ItemIndex = GET_ITEM(4,25);
				break;
			case 7:
				ItemIndex = GET_ITEM(4,26);
				break;
			case 8:
				ItemIndex = GET_ITEM(5,35);
				break;
			case 9:
				ItemIndex = GET_ITEM(5,37);
				break;
			case 10:
				ItemIndex = GET_ITEM(0,30);
				break;
			case 11:
				ItemIndex = GET_ITEM(2,19);
				break;
			case 12:
				ItemIndex = GET_ITEM(5,32);
				break;
		}

		gItemOptionRate.GetItemOption2(8,&ItemOption2);

		gItemOptionRate.GetItemOption6(((gItemManager.GetItemTwoHand(ItemIndex)==0)?6:7),&ItemSocketOption[0]);

		gItemOptionRate.MakeSocketOption(ItemIndex,ItemSocketOption[0],&ItemSocketOption[0]);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,1,ItemOption2,0,-1,0,0,0,0,ItemSocketOption,0xFF,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketWeaponMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,0,0);

		gLog.Output(LOG_CHAOS_MIX,"[SocketWeaponMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	#endif
}

void CChaosBox::PentagramMithrilMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int ChaosCount = 0;
	int MithrilFragmentBunchCount = 0;
	int MithrilFragmentBunchType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,148))
		{
			MithrilFragmentBunchCount++;
			MithrilFragmentBunchType = lpObj->ChaosBox[n].m_SocketOptionBonus;
		}
	}

	if(ChaosCount != 1 || MithrilFragmentBunchCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,250,0);
		return;
	}

	if(CHECK_RANGE((MithrilFragmentBunchType-1),MAX_PENTAGRAM_ELEMENTAL_ATTRIBUTE) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,248,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_PentagramMithrilMixRate[lpObj->AccountLevel];

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 100000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,249,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = GET_ITEM(12,145);
		BYTE SocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};
		BYTE SocketOptionBonus = MithrilFragmentBunchType;

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,0,0,-1,0,0,0,0,SocketOption,SocketOptionBonus,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramMithrilMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,226,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramMithrilMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	#endif
}

void CChaosBox::PentagramElixirMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int ChaosCount = 0;
	int ElixirFragmentBunchCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,15))
		{
			ChaosCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,149))
		{
			ElixirFragmentBunchCount++;
		}
	}

	if(ChaosCount != 1 || ElixirFragmentBunchCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,250,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_PentagramElixirMixRate[lpObj->AccountLevel];

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 100000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,249,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = GET_ITEM(12,147);
		BYTE SocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};
		BYTE SocketOptionBonus = 0xFF;

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,0,0,-1,0,0,0,0,SocketOption,SocketOptionBonus,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramElixirMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,226,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramElixirMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	#endif
}

void CChaosBox::PentagramJewelMix(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int BlessCount = 0;
	int MithrilCount = 0;
	int MithrilType = 0;
	int ElixirCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13))
		{
			BlessCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,145))
		{
			MithrilCount++;
			MithrilType = lpObj->ChaosBox[n].m_SocketOptionBonus;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,147))
		{
			ElixirCount++;
		}
	}

	if(BlessCount != 1 || MithrilCount != 1 || ElixirCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,250,0);
		return;
	}

	if(CHECK_RANGE((MithrilType-1),MAX_PENTAGRAM_ELEMENTAL_ATTRIBUTE) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,251,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_PentagramJewelMixRate[lpObj->AccountLevel];

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 100000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,249,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE SocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};
		BYTE SocketOptionBonus = MithrilType | 16;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,221),4000);

		RandomManager.AddElement(GET_ITEM(12,231),3000);

		RandomManager.AddElement(GET_ITEM(12,241),1500);

		RandomManager.AddElement(GET_ITEM(12,251),1000);

		RandomManager.AddElement(GET_ITEM(12,261),500);

		RandomManager.GetRandomElement(&ItemIndex);

		if(gPentagramSystem.GetPentagramRandomJewelOption(ItemIndex,1,&SocketOption[0]) == 0)
		{
			this->GCChaosMixSend(lpObj->Index,248,0);
			return;
		}

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,0,0,-1,0,0,0,0,SocketOption,SocketOptionBonus,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramJewelMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,226,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramJewelMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	#endif
}

void CChaosBox::PentagramDecompositeMix(LPOBJ lpObj,int type) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int HarmonyCount = 0;
	int PentagramItemType = 0;
	int PentagramItemSlot = 0;
	int PentagramDecompositeType = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,42))
		{
			HarmonyCount++;
		}
		if(lpObj->ChaosBox[n].IsPentagramItem() != 0)
		{
			PentagramItemType = lpObj->ChaosBox[n].m_SocketOptionBonus%16;
			PentagramItemSlot = n;
			PentagramDecompositeType = lpObj->ChaosBox[n].m_Index-GET_ITEM(12,199);
		}
	}

	if(HarmonyCount != 1 || PentagramDecompositeType == 0 || (PentagramDecompositeType != (type+1)))
	{
		this->GCChaosMixSend(lpObj->Index,250,0);
		return;
	}

	if(CHECK_RANGE((PentagramItemType-1),MAX_PENTAGRAM_ELEMENTAL_ATTRIBUTE) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,251,0);
		return;
	}

	lpObj->ChaosSuccessRate = gServerInfo.m_PentagramDecompositeMixRate[lpObj->AccountLevel];

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = 100000;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,249,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	gPentagramSystem.DelAllPentagramJewelInfo(lpObj,&lpObj->ChaosBox[PentagramItemSlot],0);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		WORD ItemIndex = 0;
		BYTE SocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};
		BYTE SocketOptionBonus = 0xFF;

		CRandomManager RandomManager;

		RandomManager.AddElement(GET_ITEM(12,144),5000);

		RandomManager.AddElement(GET_ITEM(12,145),1500);

		RandomManager.AddElement(GET_ITEM(12,146),2500);

		RandomManager.AddElement(GET_ITEM(12,147),500);

		RandomManager.AddElement(GET_ITEM(12,150),500);

		RandomManager.GetRandomElement(&ItemIndex);

		SocketOptionBonus = ((ItemIndex==GET_ITEM(12,144))?PentagramItemType:SocketOptionBonus);

		SocketOptionBonus = ((ItemIndex==GET_ITEM(12,145))?PentagramItemType:SocketOptionBonus);

		GDCreateItemSend(lpObj->Index,0xFF,0,0,ItemIndex,0,0,0,0,0,-1,0,0,0,0,SocketOption,SocketOptionBonus,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramDecompositeMix][Success][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,226,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramDecompositeMix][Failure][%s][%s] - (Type: %d, ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,type,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	#endif
}

void CChaosBox::PentagramJewelUpgradeLevelMix(LPOBJ lpObj,BYTE info) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int BlessCount = 0;
	int JewelCombinationFrameCount = 0;
	int PentagramJewelCount = 0;
	int PentagramJewelSlot = 0;
	int ElementalChaosAmulet = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13))
		{
			BlessCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,150))
		{
			JewelCombinationFrameCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,190))
		{
			ElementalChaosAmulet++;
		}
		else if(lpObj->ChaosBox[n].IsPentagramJewel() != 0)
		{
			PentagramJewelCount++;
			PentagramJewelSlot = n;
		}
	}

	if(BlessCount != 1 || JewelCombinationFrameCount != 1 || PentagramJewelCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,250,0);
		return;
	}

	if(CHECK_RANGE(((lpObj->ChaosBox[PentagramJewelSlot].m_SocketOptionBonus/16)-1),MAX_PENTAGRAM_JEWEL_RANK) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,252,0);
		return;
	}

	if(CHECK_RANGE(lpObj->ChaosBox[PentagramJewelSlot].m_Level,(MAX_PENTAGRAM_JEWEL_LEVEL-1)) == 0 || lpObj->ChaosBox[PentagramJewelSlot].m_Level != (info-1))
	{
		this->GCChaosMixSend(lpObj->Index,252,0);
		return;
	}

	PENTAGRAM_JEWEL_UPGRADE_LEVEL_INFO PentagramJewelUpgradeLevelInfo;

	if(gPentagramSystem.GetPentagramJewelUpgradeLevelInfo((info-1),&PentagramJewelUpgradeLevelInfo) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,248,0);
		return;
	}

	lpObj->ChaosSuccessRate = PentagramJewelUpgradeLevelInfo.MixRate[((lpObj->ChaosBox[PentagramJewelSlot].m_SocketOptionBonus/16)-1)]/100;

	if(this->GetElementalTalismanOfLuckRate(lpObj,&lpObj->ChaosSuccessRate) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,240,0);
		return;
	}

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = PentagramJewelUpgradeLevelInfo.MixMoney;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,249,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		CItem item = lpObj->ChaosBox[PentagramJewelSlot];

		item.m_Level++;

		item.m_SocketOption[((item.m_SocketOptionBonus/16)-1)] += 16;

		this->ChaosBoxInit(lpObj);

		gItemManager.ChaosBoxAddItem(lpObj->Index,item,0);

		this->GCChaosMixSend(lpObj->Index,1,&item);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramJewelUpgradeLevelMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		if(ElementalChaosAmulet == 0)
		{
			this->ChaosBoxInit(lpObj);

			this->GCChaosBoxSend(lpObj,0);

			this->GCChaosMixSend(lpObj->Index,225,0);

			gLog.Output(LOG_CHAOS_MIX,"[PentagramJewelUpgradeLevelMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d, ChaosAmulet: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney,ElementalChaosAmulet);
		}
		else
		{
			this->ChaosBoxItemDown(lpObj,PentagramJewelSlot);

			this->GCChaosBoxSend(lpObj,0);

			this->GCChaosMixSend(lpObj->Index,225,0);

			gLog.Output(LOG_CHAOS_MIX,"[PentagramJewelUpgradeLevelMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d, ChaosAmulet: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney,ElementalChaosAmulet);
		}
	}

	#endif
}

void CChaosBox::PentagramJewelUpgradeRankMix(LPOBJ lpObj,BYTE info) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int BlessCount = 0;
	int SoulCount = 0;
	int SpiritPowderCount = 0;
	int PentagramJewelCount = 0;
	int PentagramJewelSlot = 0;
	int SetItemCount = 0;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,13))
		{
			BlessCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(14,14))
		{
			SoulCount++;
		}
		else if(lpObj->ChaosBox[n].m_Index == GET_ITEM(12,151))
		{
			SpiritPowderCount++;
		}
		else if(lpObj->ChaosBox[n].IsPentagramJewel() != 0 && lpObj->ChaosBox[n].m_Level >= 7)
		{
			PentagramJewelCount++;
			PentagramJewelSlot = n;
		}
		else if(lpObj->ChaosBox[n].IsSetItem() != 0 && lpObj->ChaosBox[n].m_Level >= 7 && lpObj->ChaosBox[n].m_Option3 >= 1)
		{
			SetItemCount++;
		}
	}

	if(BlessCount != (info-1) || SoulCount != (info-1) || SpiritPowderCount != (info-1) || PentagramJewelCount != 1 || SetItemCount != 1)
	{
		this->GCChaosMixSend(lpObj->Index,250,0);
		return;
	}

	if(CHECK_RANGE(((lpObj->ChaosBox[PentagramJewelSlot].m_SocketOptionBonus/16)-1),(MAX_PENTAGRAM_JEWEL_RANK-1)) == 0 || (lpObj->ChaosBox[PentagramJewelSlot].m_SocketOptionBonus/16) != (info-1))
	{
		this->GCChaosMixSend(lpObj->Index,252,0);
		return;
	}

	PENTAGRAM_JEWEL_UPGRADE_RANK_INFO PentagramJewelUpgradeRankInfo;

	if(gPentagramSystem.GetPentagramJewelUpgradeRankInfo((info-1),&PentagramJewelUpgradeRankInfo) == 0)
	{
		this->GCChaosMixSend(lpObj->Index,248,0);
		return;
	}

	lpObj->ChaosSuccessRate = PentagramJewelUpgradeRankInfo.MixRate/100;

	lpObj->ChaosSuccessRate = ((lpObj->ChaosSuccessRate>100)?100:lpObj->ChaosSuccessRate);

	lpObj->ChaosMoney = PentagramJewelUpgradeRankInfo.MixMoney;

	int TaxMoney = (lpObj->ChaosMoney*gCastleSiegeSync.GetTaxRateChaos(lpObj->Index))/100;

	lpObj->ChaosMoney += TaxMoney;

	if(lpObj->Money < ((DWORD)lpObj->ChaosMoney))
	{
		this->GCChaosMixSend(lpObj->Index,249,0);
		return;
	}

	lpObj->Money -= lpObj->ChaosMoney;

	GCMoneySend(lpObj->Index,lpObj->Money);

	gCastleSiegeSync.AddTributeMoney(TaxMoney);

	if((GetLargeRand()%100) < lpObj->ChaosSuccessRate)
	{
		CItem item = lpObj->ChaosBox[PentagramJewelSlot];

		item.m_Level = 0;

		item.m_SocketOptionBonus += 16;

		if(gPentagramSystem.GetPentagramRandomJewelOption(item.m_Index,(item.m_SocketOptionBonus/16),&item.m_SocketOption[((item.m_SocketOptionBonus/16)-1)]) == 0)
		{
			this->GCChaosMixSend(lpObj->Index,248,0);
			return;
		}

		this->ChaosBoxInit(lpObj);

		gItemManager.ChaosBoxAddItem(lpObj->Index,item,0);

		this->GCChaosMixSend(lpObj->Index,1,&item);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramJewelUpgradeRankMix][Success][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}
	else
	{
		this->ChaosBoxInit(lpObj);

		this->GCChaosBoxSend(lpObj,0);

		this->GCChaosMixSend(lpObj->Index,225,0);

		gLog.Output(LOG_CHAOS_MIX,"[PentagramJewelUpgradeRankMix][Failure][%s][%s] - (ChaosSuccessRate: %d, ChaosMoney: %d)",lpObj->Account,lpObj->Name,lpObj->ChaosSuccessRate,lpObj->ChaosMoney);
	}

	#endif
}

void CChaosBox::CGChaosMixRecv(PMSG_CHAOS_MIX_RECV* lpMsg, int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if (lpObj->ChaosLock != 0 ||lpObj->ChaosLockShowRate != 0)
	{
		return;
	}

	if (lpObj->PShopOpen != 0)
	{
		return;
	}

	lpObj->ChaosLock = 1;

	lpObj->ChaosMoney = 0;

	lpObj->ChaosSuccessRate = 0;

	lpObj->IsChaosMixCompleted = 1;

	int HeadCodetype = MAKE_NUMBERW(lpMsg->type[0], lpMsg->type[1]);

	gLog.Output(LOG_CHAOS_MIX, "[CHECK ITEM GOBIN [%s]-[%s]", lpObj->Account, lpObj->Name);
	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() != 0)
		{
			gLog.Output(LOG_CHAOS_MIX, "[Item] - [%s +%d] - [DB[%.0f]|Skill[%d] | Luck[%d], Op[%d]] Exl[%d] SET[%d] oEx[%d] Harmony[%d] Socket[%03d,%03d,%03d,%03d,%03d][%d] Seri[%08X]",
				gItemManager.GetItemName(lpObj->ChaosBox[n].m_Index),
				lpObj->ChaosBox[n].m_Level,
				lpObj->ChaosBox[n].m_Durability,
				lpObj->ChaosBox[n].m_Option1,
				lpObj->ChaosBox[n].m_Option2,
				lpObj->ChaosBox[n].m_Option3,
				lpObj->ChaosBox[n].m_NewOption,
				lpObj->ChaosBox[n].m_SetOption,
				lpObj->ChaosBox[n].m_ItemOptionEx,
				lpObj->ChaosBox[n].m_JewelOfHarmonyOption,
				lpObj->ChaosBox[n].m_SocketOption[0],
				lpObj->ChaosBox[n].m_SocketOption[1],
				lpObj->ChaosBox[n].m_SocketOption[2],
				lpObj->ChaosBox[n].m_SocketOption[3],
				lpObj->ChaosBox[n].m_SocketOption[4],
				lpObj->ChaosBox[n].m_SocketOptionBonus,
				lpObj->ChaosBox[n].m_Serial);
		}
	}
	gLog.Output(LOG_CHAOS_MIX, "[==============[%s][%s]===========", lpObj->Account, lpObj->Name);

	LogAdd(LOG_RED, "CGChaosMixRecv Type %d %d Cs(%d)", HeadCodetype, lpMsg->info, lpObj->CustomIndexMix);

	switch(HeadCodetype)
	{
		case CHAOS_MIX_CHAOS_ITEM:
			this->ChaosItemMix(lpObj);
			break;
		case CHAOS_MIX_DEVIL_SQUARE:
			this->DevilSquareMix(lpObj);
			break;
		case CHAOS_MIX_PLUS_ITEM_LEVEL1:
			this->PlusItemLevelMix(lpObj,0);
			break;
		case CHAOS_MIX_PLUS_ITEM_LEVEL2:
			this->PlusItemLevelMix(lpObj,1);
			break;
		case CHAOS_MIX_DINORANT:
			this->DinorantMix(lpObj);
			break;
		case CHAOS_MIX_FRUIT:
			this->FruitMix(lpObj);
			break;
		case CHAOS_MIX_WING1:
			this->Wing2Mix(lpObj,0);
			break;
		case CHAOS_MIX_BLOOD_CASTLE:
			this->BloodCastleMix(lpObj);
			break;
		case CHAOS_MIX_WING2:
			this->Wing1Mix(lpObj);
			break;
		case CHAOS_MIX_PET1:
			this->PetMix(lpObj,0);
			break;
		case CHAOS_MIX_PET2:
			this->PetMix(lpObj,1);
			break;
		case CHAOS_MIX_SIEGE_POTION1:
			this->SiegePotionMix(lpObj,0);
			break;
		case CHAOS_MIX_SIEGE_POTION2:
			this->SiegePotionMix(lpObj,1);
			break;
		case CHAOS_MIX_LIFE_STONE:
			this->LifeStoneMix(lpObj);
			break;
		case CHAOS_MIX_SENIOR:
			this->SeniorMix(lpObj);
			break;
		case CHAOS_MIX_PLUS_ITEM_LEVEL3:
			this->PlusItemLevelMix(lpObj,2);
			break;
		case CHAOS_MIX_PLUS_ITEM_LEVEL4:
			this->PlusItemLevelMix(lpObj,3);
			break;
		case CHAOS_MIX_WING3:
			this->Wing2Mix(lpObj,1);
			break;
		case CHAOS_MIX_PIECE_OF_HORN:
			this->PieceOfHornMix(lpObj);
			break;
		case CHAOS_MIX_BROKEN_HORN:
			this->BrokenHornMix(lpObj);
			break;
		case CHAOS_MIX_HORN_OF_FENRIR:
			this->HornOfFenrirMix(lpObj);
			break;
		case CHAOS_MIX_HORN_OF_FENRIR_UPGRADE:
			this->HornOfFenrirUpgradeMix(lpObj);
			break;
		case CHAOS_MIX_SHIELD_POTION1:
			this->ShieldPotionMix(lpObj,0);
			break;
		case CHAOS_MIX_SHIELD_POTION2:
			this->ShieldPotionMix(lpObj,1);
			break;
		case CHAOS_MIX_SHIELD_POTION3:
			this->ShieldPotionMix(lpObj,2);
			break;
		case CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_PURITY:
			this->JewelOfHarmonyItemPurityMix(lpObj);
			break;
		case CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_SMELT:
			this->JewelOfHarmonyItemSmeltMix(lpObj);
			break;
		case CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_RESTORE:
			this->JewelOfHarmonyItemRestoreMix(lpObj);
			break;
		case CHAOS_MIX_ITEM_380:
			this->Item380Mix(lpObj);
			break;
		case CHAOS_MIX_ILLUSION_TEMPLE:
			this->IllusionTempleMix(lpObj);
			break;
		case CHAOS_MIX_FEATHER_OF_CONDOR:
			this->FeatherOfCondorMix(lpObj);
			break;
		case CHAOS_MIX_WING4:
			this->Wing3Mix(lpObj);
			break;
		case CHAOS_MIX_CHAOS_CARD:
			this->ChaosCardMix(lpObj,0);
			break;
		case CHAOS_MIX_CHERRY_BLOSSOM:
			this->CherryBlossomMix(lpObj,0);
			break;
		case CHAOS_MIX_SOCKET_ITEM_CREATE_SEED:
			this->SocketItemCreateSeedMix(lpObj);
			break;
		case CHAOS_MIX_SOCKET_ITEM_CREATE_SEED_SPHERE:
			this->SocketItemCreateSeedSphereMix(lpObj);
			break;
		case CHAOS_MIX_SOCKET_ITEM_MOUNT_SEED_SPHERE:
			this->SocketItemMountSeedSphereMix(lpObj,lpMsg->info);
			break;
		case CHAOS_MIX_SOCKET_ITEM_UN_MOUNT_SEED_SPHERE:
			this->SocketItemUnMountSeedSphereMix(lpObj,lpMsg->info);
			break;
		case CHAOS_MIX_IMPERIAL_GUARDIAN:
			this->ImperialGuardianMix(lpObj);
			break;
		case CHAOS_MIX_CHEST:
			this->ChestMix(lpObj);
			break;
		case CHAOS_MIX_SUMMON_SCROLL:
			this->SummonScrollMix(lpObj,0);
			break;
		case CHAOS_MIX_PLUS_ITEM_LEVEL5:
			this->PlusItemLevelMix(lpObj,4);
			break;
		case CHAOS_MIX_PLUS_ITEM_LEVEL6:
			this->PlusItemLevelMix(lpObj,5);
			break;
		case CHAOS_MIX_LUCKY_ITEM_CREATE:
			this->LuckyItemCreateMix(lpObj);
			break;
		case CHAOS_MIX_LUCKY_ITEM_REFINE:
			this->LuckyItemRefineMix(lpObj);
			break;
		case CHAOS_MIX_MONSTER_WING:
			this->MonsterWingMix(lpObj);
			break;
		case CHAOS_MIX_SOCKET_WEAPON:
			this->SocketWeaponMix(lpObj);
			break;
		case CHAOS_MIX_FENRIR_GOLD:
			this->HornOfFenrirGoldMix(lpObj);
			break;
		default:
			this->CustomItemMix(lpObj, HeadCodetype);
			break;
	}
}
void CChaosBox::CGChaosMixCloseRecv(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_CHAOS_BOX)
	{
		return;
	}

	lpObj->Interface.use = 0;
	lpObj->Interface.type = INTERFACE_NONE;
	lpObj->Interface.state = 0;

	this->ChaosBoxInit(lpObj);

	gObjInventoryCommit(aIndex);

	lpObj->ChaosLock = 0;

	lpObj->IsChaosMixCompleted = 0;

	lpObj->IsCastleNPCUpgradeCompleted = 0;

	PBMSG_HEAD pMsg;

	pMsg.set(0x87,sizeof(pMsg));

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.size);
}

void CChaosBox::GCChaosBoxSend(LPOBJ lpObj,BYTE type) // OK
{
	if((type == 0 && (lpObj->Interface.type != INTERFACE_CHAOS_BOX || lpObj->Interface.state == 1)) || (type != 0 && (lpObj->Interface.type != INTERFACE_TRAINER || lpObj->Interface.state == 1)))
	{
		return;
	}

	BYTE send[2048];

	PMSG_SHOP_ITEM_LIST_SEND pMsg;

	pMsg.header.set(0x31,0);

	int size = sizeof(pMsg);

	pMsg.type = ((type==0)?3:5);

	pMsg.count = 0;

	PMSG_SHOP_ITEM_LIST info;

	for(int n=0;n < CHAOS_BOX_SIZE;n++)
	{
		if(lpObj->ChaosBox[n].IsItem() == 0)
		{
			continue;
		}

		info.slot = n;

		gItemManager.ItemByteConvert(info.ItemInfo,lpObj->ChaosBox[n]);

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	DataSend(lpObj->Index,send,size);
}

void CChaosBox::GCChaosMixSend(int aIndex,BYTE result,CItem* lpItem) // OK
{
	PMSG_CHAOS_MIX_SEND pMsg;

	pMsg.header.set(0x86,sizeof(pMsg));

	pMsg.result = result;

	if(lpItem == 0)
	{
		memset(pMsg.ItemInfo,0xFF,sizeof(pMsg.ItemInfo));
	}
	else
	{
		gItemManager.ItemByteConvert(pMsg.ItemInfo,(*lpItem));
	}

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	gObj[aIndex].ChaosLock = 0;
}



#if (CB_GETMIXRATE)
struct PMSG_CHAOX_MIX_GET_RATE_RECV
{
	PBMSG_HEAD header;
	int MixID;
	int MixInfo;
};

struct PMSG_CHAOX_MIX_GET_RATE_SEND
{
	PBMSG_HEAD header; // C1:86
	int Rate;
	int Zen;
};

void CChaosBox::CGChaosMixInfo(BYTE* lpMsg, int aIndex, int size)
{

	if (size != sizeof(PMSG_CHAOX_MIX_GET_RATE_RECV)) {
		return;
	}

	if (gServerInfo.m_UseZenRateReal == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if (lpObj->Interface.type == INTERFACE_TRAINER) //Fix tỷ lệ Quạ
	{
		goto Jump;
	}

	if (lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_CHAOS_BOX || lpObj->ChaosLockShowRate != 0)
	{
		return;
	}
Jump:

	PMSG_CHAOX_MIX_GET_RATE_RECV* Data = (PMSG_CHAOX_MIX_GET_RATE_RECV*)lpMsg;

	int ChaosMixId = Data->MixID;
	int ChaosMixInfo = Data->MixInfo;

	lpObj->ChaosLockShowRate = 100;  //Set trang thai get Rate

	lpObj->ChaosMoney = 0;

	lpObj->ChaosSuccessRate = -1;

	switch (ChaosMixId)
	{
	case CHAOS_MIX_CHAOS_ITEM:
		this->ChaosItemMix(lpObj);
		break;
	case CHAOS_MIX_DEVIL_SQUARE:
		this->DevilSquareMix(lpObj);
		break;
	case CHAOS_MIX_PLUS_ITEM_LEVEL1:
		this->PlusItemLevelMix(lpObj, 0);
		break;
	case CHAOS_MIX_PLUS_ITEM_LEVEL2:
		this->PlusItemLevelMix(lpObj, 1);
		break;
	case CHAOS_MIX_DINORANT:
		this->DinorantMix(lpObj);
		break;
	case CHAOS_MIX_FRUIT:
		this->FruitMix(lpObj);
		break;
	case CHAOS_MIX_WING1:
		this->Wing2Mix(lpObj, 0);
		break;
	case CHAOS_MIX_BLOOD_CASTLE:
		this->BloodCastleMix(lpObj);
		break;
	case CHAOS_MIX_WING2:
		this->Wing1Mix(lpObj);
		break;
	case CHAOS_MIX_PET1:
		this->PetMix(lpObj, 0);
		break;
	case CHAOS_MIX_PET2:
		this->PetMix(lpObj, 1);
		break;
	case CHAOS_MIX_SIEGE_POTION1:
		this->SiegePotionMix(lpObj, 0);
		break;
	case CHAOS_MIX_SIEGE_POTION2:
		this->SiegePotionMix(lpObj, 1);
		break;
	case CHAOS_MIX_LIFE_STONE:
		this->LifeStoneMix(lpObj);
		break;
	case CHAOS_MIX_SENIOR:
		this->SeniorMix(lpObj);
		break;
	case CHAOS_MIX_PLUS_ITEM_LEVEL3:
		this->PlusItemLevelMix(lpObj, 2);
		break;
	case CHAOS_MIX_PLUS_ITEM_LEVEL4:
		this->PlusItemLevelMix(lpObj, 3);
		break;
	case CHAOS_MIX_WING3:
		this->Wing2Mix(lpObj, 1);
		break;
	case CHAOS_MIX_PIECE_OF_HORN:
		this->PieceOfHornMix(lpObj);
		break;
	case CHAOS_MIX_BROKEN_HORN:
		this->BrokenHornMix(lpObj);
		break;
	case CHAOS_MIX_HORN_OF_FENRIR:
		this->HornOfFenrirMix(lpObj);
		break;
	case CHAOS_MIX_HORN_OF_FENRIR_UPGRADE:
		this->HornOfFenrirUpgradeMix(lpObj);
		break;
	case CHAOS_MIX_SHIELD_POTION1:
		this->ShieldPotionMix(lpObj, 0);
		break;
	case CHAOS_MIX_SHIELD_POTION2:
		this->ShieldPotionMix(lpObj, 1);
		break;
	case CHAOS_MIX_SHIELD_POTION3:
		this->ShieldPotionMix(lpObj, 2);
		break;
	case CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_PURITY:
		this->JewelOfHarmonyItemPurityMix(lpObj);
		break;
	case CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_SMELT:
		this->JewelOfHarmonyItemSmeltMix(lpObj);
		break;
	case CHAOS_MIX_JEWEL_OF_HARMONY_ITEM_RESTORE:
		this->JewelOfHarmonyItemRestoreMix(lpObj);
		break;
	case CHAOS_MIX_ITEM_380:
		this->Item380Mix(lpObj);
		break;
	case CHAOS_MIX_ILLUSION_TEMPLE:
		this->IllusionTempleMix(lpObj);
		break;
	case CHAOS_MIX_FEATHER_OF_CONDOR:
		this->FeatherOfCondorMix(lpObj);
		break;
	case CHAOS_MIX_WING4:
		this->Wing3Mix(lpObj);
		break;
	case CHAOS_MIX_CHAOS_CARD:
		this->ChaosCardMix(lpObj, 0);
		break;
	case CHAOS_MIX_CHERRY_BLOSSOM:
		this->CherryBlossomMix(lpObj, 0);
		break;
	case CHAOS_MIX_SOCKET_ITEM_CREATE_SEED:
		this->SocketItemCreateSeedMix(lpObj);
		break;
	case CHAOS_MIX_SOCKET_ITEM_CREATE_SEED_SPHERE:
		this->SocketItemCreateSeedSphereMix(lpObj);
		break;
	case CHAOS_MIX_SOCKET_ITEM_MOUNT_SEED_SPHERE:
		this->SocketItemMountSeedSphereMix(lpObj, ChaosMixInfo);
		break;
	case CHAOS_MIX_SOCKET_ITEM_UN_MOUNT_SEED_SPHERE:
		this->SocketItemUnMountSeedSphereMix(lpObj, ChaosMixInfo);
		break;
	case CHAOS_MIX_IMPERIAL_GUARDIAN:
		this->ImperialGuardianMix(lpObj);
		break;
	case CHAOS_MIX_CHEST:
		this->ChestMix(lpObj);
		break;
	case CHAOS_MIX_SUMMON_SCROLL:
		this->SummonScrollMix(lpObj, 0);
		break;
	case CHAOS_MIX_PLUS_ITEM_LEVEL5:
		this->PlusItemLevelMix(lpObj, 4);
		break;
	case CHAOS_MIX_PLUS_ITEM_LEVEL6:
		this->PlusItemLevelMix(lpObj, 5);
		break;
	case CHAOS_MIX_LUCKY_ITEM_CREATE:
		this->LuckyItemCreateMix(lpObj);
		break;
	case CHAOS_MIX_LUCKY_ITEM_REFINE:
		this->LuckyItemRefineMix(lpObj);
		break;
	case CHAOS_MIX_MONSTER_WING:
		this->MonsterWingMix(lpObj);
		break;
	case CHAOS_MIX_SOCKET_WEAPON:
		this->SocketWeaponMix(lpObj);
		break;
	case CHAOS_MIX_FENRIR_GOLD:
		this->HornOfFenrirGoldMix(lpObj);
		break;
	}

	LogAdd(LOG_RED, "CGChaosMixInfo ChaosMixId %d Info(%d) Rate %d Zen %d", ChaosMixId, ChaosMixInfo, lpObj->ChaosSuccessRate, lpObj->ChaosMoney);

	if (this->MixRateShow)
	{
		std::map<int, DATA_CUSTOMRATEMIX>::iterator it = this->m_DataMixRateShow.find(ChaosMixId);
		if (it != this->m_DataMixRateShow.end())
		{
			lpObj->ChaosSuccessRate = it->second.RateShow;
		}
	}

	lpObj->ChaosLockShowRate = 0;
	if (lpObj->ChaosSuccessRate != -1)
	{
		PMSG_CHAOX_MIX_GET_RATE_SEND pMsg;

		pMsg.header.set(0x88, sizeof(pMsg));

		pMsg.Rate = min(lpObj->ChaosSuccessRate, 100.f); //max 100%
		pMsg.Zen = lpObj->ChaosMoney;

		DataSend(aIndex, (BYTE*)& pMsg, pMsg.header.size);
	}
}
void CChaosBox::LoadMixRateShow(char* FilePath)
{
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}
	//--
	pugi::xml_node CustomRateMix = file.child("CustomRateMix");
	this->MixRateShow = CustomRateMix.attribute("Enable").as_int();


	this->m_DataMixRateShow.clear();
	if (!this->MixRateShow) return;
	for (pugi::xml_node ListMix = CustomRateMix.child("Mix"); ListMix; ListMix = ListMix.next_sibling())
	{
		DATA_CUSTOMRATEMIX info;
		info.CodeMix = ListMix.attribute("CodeMix").as_int();
		info.RateShow = ListMix.attribute("RateShow").as_int();

		this->m_DataMixRateShow.insert(std::pair<int, DATA_CUSTOMRATEMIX>(info.CodeMix, info));
	}
}

#endif

