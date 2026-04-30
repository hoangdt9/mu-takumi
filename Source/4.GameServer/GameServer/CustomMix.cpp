#include "stdafx.h"
#include "Util.h"
#include "ItemManager.h"
#include "Message.h"
#include "User.h"
#include "Path.h"
#include "MemScript.h"
#include "RandomManager.h"
#include "CustomMix.h"
#include "Log.h"


CCustomMix gCustomMix;

void CCustomMix::Load(char* path) {

	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR, path);
		return;
	}

	if (lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	m_BData_Mix.clear();
	m_BData_Mix_Info.clear();
	m_BDataMixNguyenLieu.clear();
	m_BDataMixItemKetQua.clear();

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
				if (section == 0)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					BCUSTOM_MIX info;

					memset(&info, 0, sizeof(info));

					info.Index = lpMemScript->GetNumber();

					info.MixMoney = lpMemScript->GetAsNumber();

					info.MixRate[0] = lpMemScript->GetAsNumber();

					info.MixRate[1] = lpMemScript->GetAsNumber();

					info.MixRate[2] = lpMemScript->GetAsNumber();

					info.MixRate[3] = lpMemScript->GetAsNumber();

					info.CheckItemOption = lpMemScript->GetAsNumber();

					info.CheckItemNewOption = lpMemScript->GetAsNumber();


					this->m_BData_Mix.insert(std::pair<int, BCUSTOM_MIX>(info.Index, info));
				}
				else if (section == 1)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					BCUSTOM_MIX_INFO info1;

					memset(&info1, 0, sizeof(info1));

					info1.Index = lpMemScript->GetNumber();
					info1.Name1 = lpMemScript->GetAsNumber();
					info1.Name2 = lpMemScript->GetAsNumber();
					info1.Name3 = lpMemScript->GetAsNumber();
					info1.Des1 = lpMemScript->GetAsNumber();
					info1.Des2 = lpMemScript->GetAsNumber();
					info1.Des3 = lpMemScript->GetAsNumber();
					info1.Advice1 = lpMemScript->GetAsNumber();
					info1.Advice2 = lpMemScript->GetAsNumber();
					info1.Advice3 = lpMemScript->GetAsNumber();

					this->m_BData_Mix_Info.insert(std::pair<int, BCUSTOM_MIX_INFO>(info1.Index, info1));
				}
				else if (section == 2)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}
					BCUSTOM_MIX_NGUYENLIEU info2;

					memset(&info2, 0, sizeof(info2));

					info2.Index = lpMemScript->GetNumber();
					info2.ItemIndexMin= lpMemScript->GetAsNumber();
					info2.ItemIndexMax= lpMemScript->GetAsNumber();
					info2.Level= lpMemScript->GetAsNumber();
					info2.Skill= lpMemScript->GetAsNumber();
					info2.Luck = lpMemScript->GetAsNumber();
					info2.Op = lpMemScript->GetAsNumber();
					info2.Exl = lpMemScript->GetAsNumber();
					info2.Item380 = lpMemScript->GetAsNumber();
					info2.Set = lpMemScript->GetAsNumber();
					info2.Harmory = lpMemScript->GetAsNumber();
					info2.Socket = lpMemScript->GetAsNumber();
					info2.Count = lpMemScript->GetAsNumber();
					info2.FailDelItem = lpMemScript->GetAsNumber();

					this->m_BDataMixNguyenLieu.push_back(info2);
				}
				else if (section == 3)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					BCUSTOM_MIX_ITEMKETQUA info3;

					memset(&info3, 0, sizeof(info3));
					info3.Index = lpMemScript->GetNumber();
					info3.ItemIndex = lpMemScript->GetAsNumber();
					info3.Level = lpMemScript->GetAsNumber();
					info3.Skill = lpMemScript->GetAsNumber();
					info3.Luck = lpMemScript->GetAsNumber();
					info3.Op = lpMemScript->GetAsNumber();
					info3.Exl = lpMemScript->GetAsNumber();
					info3.Set = lpMemScript->GetAsNumber();
					info3.Socket[0] = lpMemScript->GetAsNumber();
					info3.Socket[1] = lpMemScript->GetAsNumber();
					info3.Socket[2] = lpMemScript->GetAsNumber();
					info3.Socket[3] = lpMemScript->GetAsNumber(); 
					info3.Socket[4] = lpMemScript->GetAsNumber();
					this->m_BDataMixItemKetQua.push_back(info3);

				}
				else
				{
					break;
				}
			}
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}




void CCustomMix::SendInfoCustomMix(int aIndex, int IndexMix)
{
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (gObj[aIndex].Map == 107 || gObj[aIndex].Map == 109)
	{
		return;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 || gObj[aIndex].IsFakeOnline != 0)
	{
		return;
	}

	BYTE send[8192];

	PMSG_BCUSTOM_MIX_INFO pMsg = { 0 };
	// ---
	pMsg.header.set(0xD3, 0x71, 0);

	int size = sizeof(pMsg);


	pMsg.count = 0;

	pMsg.Index = IndexMix;

	if (IndexMix == -1) goto SendClearInfo;

	std::map<int, BCUSTOM_MIX>::iterator it = this->m_BData_Mix.find(IndexMix);

	if (it == this->m_BData_Mix.end())
	{
		LogAdd(LOG_RED, "Khong co du lieu indexmix %d", IndexMix);
		return;
	}

	pMsg.Index = IndexMix;
	pMsg.MixMoney = it->second.MixMoney;
	pMsg.MixRate = it->second.MixRate[gObj[aIndex].AccountLevel];

	std::map<int, BCUSTOM_MIX_INFO>::iterator itInfo = this->m_BData_Mix_Info.find(IndexMix);

	if (itInfo == this->m_BData_Mix_Info.end())
	{
		LogAdd(LOG_RED, "Khong co du lieu info indexmix %d", IndexMix);
		return;
	}
	pMsg.Name1 = itInfo->second.Name1;
	pMsg.Name2 = itInfo->second.Name2;
	pMsg.Name3 = itInfo->second.Name3;
	pMsg.Des1 = itInfo->second.Des1;
	pMsg.Des2 = itInfo->second.Des2;
	pMsg.Des3 = itInfo->second.Des3;
	pMsg.Advice1 = itInfo->second.Advice1;
	pMsg.Advice2 = itInfo->second.Advice2;
	pMsg.Advice3 = itInfo->second.Advice3;



	std::vector<BCUSTOM_MIX_NGUYENLIEU>::iterator iter;
	for (iter = m_BDataMixNguyenLieu.begin(); iter != m_BDataMixNguyenLieu.end(); ++iter)
	{
		if (iter->Index != IndexMix) continue;

		BCUSTOM_MIX_NGUYENLIEU info;
		memset(&info, 0, sizeof(info));
		info.Index = iter->Index;
		info.ItemIndexMin = iter->ItemIndexMin;
		info.ItemIndexMax = iter->ItemIndexMax;
		info.Level = iter->Level;
		info.Skill = iter->Skill;
		info.Luck = iter->Luck;
		info.Op = iter->Op;
		info.Exl = iter->Exl;
		info.Item380 = iter->Item380;
		info.Set = iter->Set;
		info.Harmory = iter->Harmory;
		info.Socket = iter->Socket;
		info.Count = iter->Count;
		info.FailDelItem = iter->FailDelItem;


		if ((size + sizeof(info)) > 8192) break;
		
		pMsg.count++;
		memcpy(&send[size], &info, sizeof(info));
		size += sizeof(info);
	}


SendClearInfo:

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);
	gObj[aIndex].CustomIndexMix = IndexMix;
	//LogAdd(LOG_RED, "SendInfoCustomMix %d -> %s", IndexMix, gObj[aIndex].Name);

}
int CCustomMix::GetChaoMixItemCount(LPOBJ lpObj, int IndexMin, int IndexMax, int MinLevel, int MaxLevel, int MinOpt, int NeedLuck, int NeedSkill, int NeedExl, int Need380, int NeedAnc, int NeedHarmory, int NeedSocket) // OK
{
	int count = 0;

	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
		if (lpObj->ChaosBox[n].IsItem() != 0)
		{
			if (lpObj->ChaosBox[n].m_Index < IndexMin || lpObj->ChaosBox[n].m_Index > IndexMax)
			{
				//LogAdd(LOG_RED, "[GetTraderItemCount]  1");
				continue;
			}
			if (MinLevel != 0 && MinLevel > lpObj->ChaosBox[n].m_Level)
			{
				//LogAdd(LOG_RED, "[GetTraderItemCount]  2 %d", lpObj->ChaosBox[n].m_Index);
				continue;
			}
			//if (MaxLevel != 0 && lpObj->ChaosBox[n].m_Level > MaxLevel)
			//{
			//	LogAdd(LOG_RED, "[GetTraderItemCount]  2 %d", lpObj->ChaosBox[n].m_Index);
			//	continue;
			//}
			if (MinOpt > 0 && MinOpt > lpObj->ChaosBox[n].m_Option3)
			{
				//LogAdd(LOG_RED, "[GetTraderItemCount]  3");
				continue;
			}
			if (NeedSkill != 0 && !lpObj->ChaosBox[n].m_Option1)
			{
				//LogAdd(LOG_RED, "[GetTraderItemCount]  4");
				continue;
			}
			if (NeedLuck != 0 && !lpObj->ChaosBox[n].m_Option2)
			{
				//LogAdd(LOG_RED, "[GetTraderItemCount]  5");
				continue;
			}
			if (NeedExl != 0 && NeedExl < getNumberOfExcOptions(lpObj->ChaosBox[n].m_NewOption))
			{
				//LogAdd(LOG_RED, "[GetTraderItemCount]  6");
				continue;
			}
			if (NeedHarmory != 0 && !lpObj->ChaosBox[n].IsJewelOfHarmonyItem())
			{
				//LogAdd(LOG_RED, "[GetTraderItemCount]  6");
				continue;
			}
			if (Need380 != 0 && !lpObj->ChaosBox[n].Is380Item())
			{
				//LogAdd(LOG_RED, "[GetTraderItemCount]  6");
				continue;
			}
			if (NeedAnc != 0 && !lpObj->ChaosBox[n].IsSetItem())
			{
				//LogAdd(LOG_RED, "[GetTraderItemCount]  7");
				continue;
			}
			if (NeedSocket != 0 && !lpObj->ChaosBox[n].IsSocketItem())
			{
				//LogAdd(LOG_RED, "[GetTraderItemCount]  7");
				continue;
			}
			count += 1;

		}
	}

	return count;
}

int CCustomMix::GetSizeNguyenLieu(int IndexMix)
{
	int SizeOut = 0;
	for (int i = 0; i < this->m_BDataMixNguyenLieu.size(); i++)
	{
		if (IndexMix != this->m_BDataMixNguyenLieu[i].Index) continue;

		SizeOut++;

	}
	return SizeOut;
}

int CCustomMix::CheckItemMixForIndex(LPOBJ lpObj, int IndexMix)
{
	int GetMixNumber = -1;
	bool IsItem = true;
	for (int n = 0; n < CHAOS_BOX_SIZE; n++)
	{
			if (lpObj->ChaosBox[n].IsItem() == 0 || !IsItem)
			{
				continue;
			}
			//LogAdd(LOG_RED, "Slot %d Get Item %s Lv %d %d", n, gItemLevel.GetItemName(lpObj->ChaosBox[n].m_Index, lpObj->ChaosBox[n].m_Level), lpObj->ChaosBox[n].m_Level, GetMixNumber);

			for (int i = 0; i < this->m_BDataMixNguyenLieu.size(); i++)
			{
				if (GetMixNumber != -1 && GetMixNumber != this->m_BDataMixNguyenLieu[i].Index) continue;

				//LogAdd(LOG_RED, "Scan INdex %d (%d)", this->m_BDataMixNguyenLieu[i].Index, GetMixNumber);

				if (lpObj->ChaosBox[n].m_Index < this->m_BDataMixNguyenLieu[i].ItemIndexMin || lpObj->ChaosBox[n].m_Index > this->m_BDataMixNguyenLieu[i].ItemIndexMax)
				{
					//LogAdd(LOG_RED, "CheckItemMixForIndex 1");
					if (GetMixNumber != -1)
					{
						goto SetIndexMix;
					}
					continue;
				}
				if (this->m_BDataMixNguyenLieu[i].Level != 0 && this->m_BDataMixNguyenLieu[i].Level > lpObj->ChaosBox[n].m_Level)
				{
					//LogAdd(LOG_RED, "CheckItemMixForIndex 2");
					if (GetMixNumber != -1)
					{
						goto SetIndexMix;
					}
					continue;
				}
				if (this->m_BDataMixNguyenLieu[i].Op != 0 && this->m_BDataMixNguyenLieu[i].Op > lpObj->ChaosBox[n].m_Option3)
				{
					//LogAdd(LOG_RED, "CheckItemMixForIndex 3");
					if (GetMixNumber != -1)
					{
						goto SetIndexMix;
					}
					continue;
				}
				if (this->m_BDataMixNguyenLieu[i].Skill != 0 && !lpObj->ChaosBox[n].m_Option1)
				{
					//LogAdd(LOG_RED, "CheckItemMixForIndex 4");
					if (GetMixNumber != -1)
					{
						goto SetIndexMix;
					}
					continue;
				}
				if (this->m_BDataMixNguyenLieu[i].Luck != 0 && !lpObj->ChaosBox[n].m_Option2)
				{
					//LogAdd(LOG_RED, "CheckItemMixForIndex 5");
					if (GetMixNumber != -1)
					{
						goto SetIndexMix;
					}
					continue;
				}
				if (this->m_BDataMixNguyenLieu[i].Exl != 0 && this->m_BDataMixNguyenLieu[i].Exl < getNumberOfExcOptions(lpObj->ChaosBox[n].m_NewOption))
				{
					//LogAdd(LOG_RED, "CheckItemMixForIndex 6");
					if (GetMixNumber != -1)
					{
						goto SetIndexMix;
					}
					continue;
				}
				if (this->m_BDataMixNguyenLieu[i].Harmory != 0 && !lpObj->ChaosBox[n].IsJewelOfHarmonyItem())
				{
					//LogAdd(LOG_RED, "CheckItemMixForIndex 7");
					if (GetMixNumber != -1)
					{
						goto SetIndexMix;
					}
					continue;
				}
				if (this->m_BDataMixNguyenLieu[i].Item380 != 0 && !lpObj->ChaosBox[n].Is380Item())
				{
					//LogAdd(LOG_RED, "CheckItemMixForIndex 8");
					if (GetMixNumber != -1)
					{
						goto SetIndexMix;
					}
					continue;
				}
				if (this->m_BDataMixNguyenLieu[i].Set != 0 && !lpObj->ChaosBox[n].IsSetItem())
				{
					//LogAdd(LOG_RED, "CheckItemMixForIndex 9");
					if (GetMixNumber != -1)
					{
						goto SetIndexMix;
					}
					continue;
				}
				if (this->m_BDataMixNguyenLieu[i].Socket != 0 && !lpObj->ChaosBox[n].IsSocketItem())
				{
					//LogAdd(LOG_RED, "CheckItemMixForIndex 10");
					if (GetMixNumber != -1)
					{
						goto SetIndexMix;
					}
					continue;
				}
				
				GetMixNumber = this->m_BDataMixNguyenLieu[i].Index;
				//LogAdd(LOG_BLUE, "OK Item %s Lv %d %d", gItemLevel.GetItemName(lpObj->ChaosBox[n].m_Index, lpObj->ChaosBox[n].m_Level), lpObj->ChaosBox[n].m_Level, GetMixNumber);
				break;

		SetIndexMix:
				if ((i + 1) < this->m_BDataMixNguyenLieu.size())
				{
					GetMixNumber = this->m_BDataMixNguyenLieu[i + 1].Index;
				}
				else
				{
					GetMixNumber = -1;
				}

			}
			if (GetMixNumber==-1) IsItem = false;

	}

	return GetMixNumber;
	
}
void CCustomMix::GetSendInfoMix(LPOBJ lpObj)
{
	int GetCustomIndexMix = this->CheckItemMixForIndex(lpObj, lpObj->CustomIndexMix);
	//int GetIndexMix = this->CheckItemMixForIndex(lpObj, -1);
	//LogAdd(LOG_RED, "GetCustomIndexMix %d", GetCustomIndexMix); //Send Lên GS TT Mix
	//if (GetCustomIndexMix != -1) lpObj->CustomIndexMix = GetCustomIndexMix;
	if(GetCustomIndexMix != lpObj->CustomIndexMix) this->SendInfoCustomMix(lpObj->Index, GetCustomIndexMix);

	
}
BCUSTOM_MIX* CCustomMix::BGetCustomMix(int IndexMix)
{

	BCUSTOM_MIX* value;

	std::map<int, BCUSTOM_MIX>::iterator it = this->m_BData_Mix.find(IndexMix);

	if (it != this->m_BData_Mix.end())
	{
		value = &it->second;

		return value;
	}

	return 0;
}


CRandomManager CCustomMix::BRandomManager(int IndexMix)
{
	CRandomManager RandomManager;

	for (int n = 0 ; n < this->m_BDataMixItemKetQua.size(); n++)
	{
		if (IndexMix != this->m_BDataMixItemKetQua[n].Index) continue;

		RandomManager.AddElement(n, 1);
	}

	return RandomManager;
}

int CCustomMix::GetFailDelItem(int ItemIndex,int IndexMix)
{

	for (int n = 0; n < this->m_BDataMixNguyenLieu.size(); n++)
	{
		if (IndexMix != this->m_BDataMixNguyenLieu[n].Index) continue;
		if (this->m_BDataMixNguyenLieu[n].ItemIndexMin > ItemIndex || ItemIndex > this->m_BDataMixNguyenLieu[n].ItemIndexMax)
		{
			//LogAdd(LOG_RED, "[GetTraderItemCount]  1");
			continue;
		}
		return this->m_BDataMixNguyenLieu[n].FailDelItem;
	}


	return 1;
}

CUSTOM_MIX * CCustomMix::GetCustomMix(int IndexMix){

	CUSTOM_MIX * value;

	std::map<int, CUSTOM_MIX>::iterator it = this->m_Data_Mix.find(IndexMix);

	if(it != this->m_Data_Mix.end())
	{
		value = &it->second;

		return value;
	}

	return 0;
}

bool CCustomMix::istItemBagMix(int IndexMix, CItem boxItem){
	
	for(std::map<int,CUSTOM_BAG_MIX>::iterator it = this->m_Data_Bag.begin();it != this->m_Data_Bag.end();it++)
	{
		if(it->second.m_Index == IndexMix && it->second.m_ItemMix == boxItem.m_Index
			&& it->second.m_ItemLevel == boxItem.m_Level)
		{
			return true;
		}
	}

	return false;
}
//--
bool CCustomMix::istItemMix(int IndexMix, int group, CItem boxItem){

	for(std::map<int, CUSTOM_ITEM_MIX>::iterator it = this->m_Data_Item.begin(); it != this->m_Data_Item.end(); it++)
	{
		if(it->second.m_Index == IndexMix && it->second.m_Group == group && boxItem.m_Index >= it->second.m_ItemIndexMin && boxItem.m_Index <= it->second.m_ItemIndexMax)
		{

			if(it->second.m_ItemLevel != -1 && it->second.m_SetACC != 0)
			{
				if(boxItem.m_Level < it->second.m_ItemLevel)
				{
					return false;
				}
			}

			if(it->second.m_Skill != -1 && it->second.m_SetACC != 0)
			{
				if(it->second.m_Skill != boxItem.m_Option1)
				{
					return false;
				}
			}

			if(it->second.m_Luck != -1 && it->second.m_SetACC != 0)
			{
				if(it->second.m_Luck != boxItem.m_Option2)
				{
					return false;
				}
			}
			if(it->second.m_Opcion != -1 && it->second.m_SetACC != 0)
			{
				if(it->second.m_Opcion > boxItem.m_Option3)
				{
					return false;
				}
			}

			if(it->second.m_Excelent != -1 && it->second.m_SetACC != 0)
			{
				if(it->second.m_Excelent != boxItem.m_NewOption)
				{
					return false;
				}
			}

			if(it->second.m_SetACC != -1 && it->second.m_SetACC != 0)
			{
				if(it->second.m_SetACC != boxItem.m_SetOption)
				{
					return false;
				}
			}

			return true;
		}
	}

	return false;
}

int CCustomMix::GetCountItemBagMix(int IndexMix){

	int itemcount = 0;

	for(std::map<int,CUSTOM_BAG_MIX>::iterator it = this->m_Data_Bag.begin();it != this->m_Data_Bag.end();it++)
	{
		if( it->second.m_Index == IndexMix )
		{
			itemcount += it->second.m_Count;
		}
	}
	return itemcount;
}

int CCustomMix::GetCountItemMix(int IndexMix,int Group){

	int itemcount = 0;

	for(std::map<int,CUSTOM_ITEM_MIX>::iterator it = this->m_Data_Item.begin();it != this->m_Data_Item.end();it++)
	{
		if(it->second.m_Index == IndexMix && it->second.m_Group == Group)
		{
			itemcount += it->second.m_Count;
		}	
	}

	return itemcount;
}

CRandomManager CCustomMix::RandomManager(int IndexMix, int group)
{
	CRandomManager RandomManager;

	std::map<int, CUSTOM_ITEM_MIX_RESULT>::iterator it = this->DataResult[IndexMix].m_DataResult.find(group);

	if(it != this->DataResult[IndexMix].m_DataResult.end())
	{
		for(int n = it->second.m_ItemIndexMin; n <= it->second.m_ItemIndexMax ; n++)
		{

			RandomManager.AddElement(n, 1);
		}
	}
			
	return RandomManager;
}
//---
CUSTOM_ITEM_MIX_RESULT* CCustomMix::GetItemResult(int IndexMix, int group, int ItemIndex)
{
	std::map<int, CUSTOM_ITEM_MIX_RESULT>::iterator it = this->DataResult[IndexMix].m_DataResult.find(group);

	if(it != this->DataResult[IndexMix].m_DataResult.end())
	{
		if(ItemIndex >= it->second.m_ItemIndexMin && ItemIndex <= it->second.m_ItemIndexMax)
		{
			return &it->second;
		}
	}

	return 0;
}