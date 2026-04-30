// ItemBag.cpp: implementation of the CItemBag class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemBag.h"
#include "DSProtocol.h"
#include "ItemOptionRate.h"
#include "ItemManager.h"
#include "JewelOfHarmonyOption.h"
#include "Map.h"
#include "MemScript.h"
#include "Protocol.h"
#include "ServerInfo.h"
#include "SetItemType.h"
#include "SocketItemOption.h"
#include "SocketItemType.h"
#include "Util.h"
#include "Notice.h"
#include "RandomManager.h"



CItemBag::CItemBag() // OK
{
	this->Init();
}

CItemBag::~CItemBag() // OK
{

}

void CItemBag::Init() // OK
{
	memset(this->m_EventName,0,sizeof(this->m_EventName));

	this->m_DropZen = 0;
	this->m_ItemDropRate = 0;
	this->m_ItemDropCount = 0;
	this->m_SetItemDropRate = 0;
	this->m_ItemDropType = 0;
	this->m_SendFirework = 0;
	this->m_count = 0;
}

void CItemBag::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->Init();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					strcpy_s(this->m_EventName,lpMemScript->GetString());

					this->m_DropZen = lpMemScript->GetAsNumber();

					this->m_ItemDropRate = lpMemScript->GetAsNumber();

					this->m_ItemDropCount = lpMemScript->GetAsNumber();

					this->m_SetItemDropRate = lpMemScript->GetAsNumber();

					this->m_ItemDropType = lpMemScript->GetAsNumber();

					this->m_SendFirework = lpMemScript->GetAsNumber();
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ITEM_BAG_INFO info;

					memset(&info,0,sizeof(info));

					info.Index = SafeGetItem(GET_ITEM(lpMemScript->GetNumber(),lpMemScript->GetAsNumber()));

					info.MinLevel = lpMemScript->GetAsNumber();

					info.MaxLevel = lpMemScript->GetAsNumber();

					info.Option1 = lpMemScript->GetAsNumber();

					info.Option2 = lpMemScript->GetAsNumber();

					info.Option3 = lpMemScript->GetAsNumber();

					info.NewOption = lpMemScript->GetAsNumber();

					info.SetOption = 0;

					info.SocketOption = 0;

#if(B_RateItemBag)
					info.RateDrop = lpMemScript->GetAsNumber();
#endif

					this->SetInfo(info);
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ITEM_BAG_INFO info;

					memset(&info,0,sizeof(info));

					info.Index = SafeGetItem(GET_ITEM(lpMemScript->GetNumber(),lpMemScript->GetAsNumber()));

					info.MinLevel = lpMemScript->GetAsNumber();

					info.MaxLevel = lpMemScript->GetAsNumber();

					info.Option1 = lpMemScript->GetAsNumber();

					info.Option2 = lpMemScript->GetAsNumber();

					info.Option3 = lpMemScript->GetAsNumber();

					info.NewOption = lpMemScript->GetAsNumber();

					info.SetOption = lpMemScript->GetAsNumber();

					info.SocketOption = lpMemScript->GetAsNumber();

#if(B_RateItemBag)
					info.RateDrop = lpMemScript->GetAsNumber();
#endif

					this->SetInfo(info);
				}
				else
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}
				}
			}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	this->m_ItemBagEx.Load(path);

	delete lpMemScript;
}

void CItemBag::SetInfo(ITEM_BAG_INFO info) // OK
{
	if(this->m_count < 0 || this->m_count >= MAX_BAG_ITEM)
	{
		return;
	}

	this->m_BagInfo[this->m_count++] = info;
}

ITEM_BAG_INFO* CItemBag::GetInfo(int index) // OK
{
	if(index < 0 || index >= this->m_count)
	{
		return 0;
	}

	return &this->m_BagInfo[index];
}

int CItemBag::GetItemLevel(ITEM_BAG_INFO* lpInfo) // OK
{
	if(lpInfo->MinLevel >= lpInfo->MaxLevel)
	{
		return lpInfo->MinLevel;
	}

	return (lpInfo->MinLevel+(GetLargeRand()%((lpInfo->MaxLevel-lpInfo->MinLevel)+1)));
}

int CItemBag::GetItemNewOption(ITEM_BAG_INFO* lpInfo) // OK
{
	int count = 0;

	int NewOption = 0;

	while(true)
	{
		if(count >= lpInfo->NewOption || count >= MAX_EXC_OPTION)
		{
			break;
		}

		int value = 1 << (GetLargeRand()%MAX_EXC_OPTION);

		if((NewOption & value) == 0)
		{
			NewOption |= value;
			count++;
		}
	}

	return NewOption;
}

int CItemBag::GetItemNewOption_New(ITEM_BAG_INFO* lpInfo, int Slot) // OK
{
	int count = 0;

	BYTE NewOption = 0;

	BYTE value = 0;
	
	while (true)
	{
		CRandomManager* BRandomManager = new CRandomManager;
		int CheckMaxCount = lpInfo->NewOption;
		
		if (Slot >= 2 && Slot <= 6)  //Trang Bi
		{
			if (gServerInfo.NewItemDrop[0] > 0) BRandomManager->AddElement(1, gServerInfo.NewItemDrop[0]);
			if (gServerInfo.NewItemDrop[1] > 0) BRandomManager->AddElement(2, gServerInfo.NewItemDrop[1]);
			if (gServerInfo.NewItemDrop[2] > 0) BRandomManager->AddElement(4, gServerInfo.NewItemDrop[2]);
			if (gServerInfo.NewItemDrop[3] > 0) BRandomManager->AddElement(8, gServerInfo.NewItemDrop[3]);
			if (gServerInfo.NewItemDrop[4] > 0) BRandomManager->AddElement(16, gServerInfo.NewItemDrop[4]);
			if (gServerInfo.NewItemDrop[5] > 0) BRandomManager->AddElement(32, gServerInfo.NewItemDrop[5]);
		}
		else if (Slot >= 0 && Slot <= 1) 			 //Vu Khi
		{
			if (gServerInfo.NewVKDrop[0] > 0)BRandomManager->AddElement(1, gServerInfo.NewVKDrop[0]);
			if (gServerInfo.NewVKDrop[1] > 0)BRandomManager->AddElement(2, gServerInfo.NewVKDrop[1]);
			if (gServerInfo.NewVKDrop[2] > 0)BRandomManager->AddElement(4, gServerInfo.NewVKDrop[2]);
			if (gServerInfo.NewVKDrop[3] > 0)BRandomManager->AddElement(8, gServerInfo.NewVKDrop[3]);
			if (gServerInfo.NewVKDrop[4] > 0)BRandomManager->AddElement(16, gServerInfo.NewVKDrop[4]); 
			if (gServerInfo.NewVKDrop[5] > 0)BRandomManager->AddElement(32, gServerInfo.NewVKDrop[5]); 
		}
		else if (Slot >= 9 && Slot <= 10) 	  //Day Chuyen Ring
		{
			if (gServerInfo.NewPhuKienDrop[0] > 0) BRandomManager->AddElement(1, gServerInfo.NewPhuKienDrop[0]);
			if (gServerInfo.NewPhuKienDrop[1] > 0) BRandomManager->AddElement(2, gServerInfo.NewPhuKienDrop[1]);
			if (gServerInfo.NewPhuKienDrop[2] > 0) BRandomManager->AddElement(4, gServerInfo.NewPhuKienDrop[2]);
			if (gServerInfo.NewPhuKienDrop[3] > 0) BRandomManager->AddElement(8, gServerInfo.NewPhuKienDrop[3]);
			if (gServerInfo.NewPhuKienDrop[4] > 0) BRandomManager->AddElement(16, gServerInfo.NewPhuKienDrop[4]);
			if (gServerInfo.NewPhuKienDrop[5] > 0) BRandomManager->AddElement(32, gServerInfo.NewPhuKienDrop[5]);
		}
		//else
		//{
		//	BRandomManager->AddElement(1, 100); 
		//	
		//	BRandomManager->AddElement(2, 100);
		//
		//	BRandomManager->AddElement(4, 100);
		//	
		//	BRandomManager->AddElement(8, 100);
		//
		//	BRandomManager->AddElement(16, 100);
		//	
		//	BRandomManager->AddElement(32, 100);
		//	
		//}
		//====
		int GetCountRate = BRandomManager->GetCount();

		BRandomManager->GetRandomElement(&value);
		if ((NewOption & value) == 0)
		{
			NewOption |= value;
			count++;
			//LogAdd(LOG_RED,"GetItemNewOption1 %d ~ %d (Type %d) (%d <> %d)",NewOption,count,Slot, CheckMaxCount, GetCountRate);
		}
		if (CheckMaxCount > GetCountRate)  CheckMaxCount = GetCountRate;

		delete BRandomManager;

		if (count >= CheckMaxCount || count >= MAX_EXC_OPTION)
		{
			break;
		}
	}
	

	return NewOption;
}
int CItemBag::GetItemSetOption(ITEM_BAG_INFO* lpInfo) // OK
{
	if(lpInfo->SetOption == 0)
	{
		return 0;
	}

	int SetOption = 0;

	for(int n=0;n < MAX_SET_ITEM_OPTION_INDEX;n++)
	{
		if(gSetItemType.GetSetItemOptionIndex(lpInfo->Index,n) <= 0)
		{
			continue;
		}

		SetOption = ((SetOption==0)?(n+1):(((GetLargeRand()%2)==0)?(n+1):SetOption));
	}

	if(SetOption == 0)
	{
		return 0;
	}

	if((GetLargeRand()%100) < 80)
	{
		SetOption |= 4;
	}
	else
	{
		SetOption |= 8;
	}

	return SetOption;
}

bool::CItemBag::GetRandomItemDropLocation(int map,int* ox,int* oy,int tx,int ty,int count) // OK
{
	int x = (*ox);
	int y = (*oy);

	tx = ((tx<1)?1:tx);
	ty = ((ty<1)?1:ty);

	for(int n=0;n < count;n++)
	{
		(*ox) = ((GetLargeRand()%(tx+1))*((GetLargeRand()%2==0)?-1:1))+x;
		(*oy) = ((GetLargeRand()%(ty+1))*((GetLargeRand()%2==0)?-1:1))+y;

		if(gMap[map].CheckAttr((*ox),(*oy),4) == 0 && gMap[map].CheckAttr((*ox),(*oy),8) == 0)
		{
			return 1;
		}
	}

	return 0;
}

bool CItemBag::GetItem(LPOBJ lpObj,CItem* lpItem) // OK
{
	if(this->m_ItemBagEx.m_ItemBagInfo.empty() == 0)
	{
		return this->m_ItemBagEx.GetItem(lpObj,lpItem);
	}

	if(this->m_count == 0)
	{
		return 0;
	}

#if(B_RateItemBag)
	CRandomManager RandomManager;
	for (int i = 0; i < this->m_count; i++)
	{
		if (this->m_BagInfo[i].RateDrop > 0)
		{
			RandomManager.AddElement((int)(&(this->m_BagInfo[i])), this->m_BagInfo[i].RateDrop);
		}
	}
	ITEM_BAG_INFO* lpInfo = 0;
	if (RandomManager.GetRandomElement((int*)& lpInfo) == 0)
	{
		return 0;
	}
	//LogAdd(LOG_RED, "GetItem Rate %d", lpInfo->RateDrop);
#else
	ITEM_BAG_INFO* lpInfo = this->GetInfo(GetLargeRand() % this->m_count);
#endif

	if(lpInfo == 0)
	{
		return 0;
	}

	int level = this->GetItemLevel(lpInfo);
	int Option1 = lpInfo->Option1;	//Skill
	int Option2 = lpInfo->Option2;	//Luck
	int Option3 = lpInfo->Option3;	//Op
	//=== Phan Loai Itme Drop
	int NewOption = 0;
	//LogAdd(LOG_RED,"DEBUG %d  m_CheckOptionsDrop %d, %d", lpInfo->Index,gServerInfo.m_CheckOptionsDrop,gItemManager.GetItemSlotByIndex(lpInfo->Index));
	if (gServerInfo.m_CheckOptionsDrop && gItemManager.GetItemSlotByIndex(lpInfo->Index) != -1)
	{
		int Slot = gItemManager.GetItemSlotByIndex(lpInfo->Index);
		NewOption = this->GetItemNewOption_New(lpInfo, Slot);
	}
	else {
		NewOption = this->GetItemNewOption(lpInfo);
	}
	int SetOption = this->GetItemSetOption(lpInfo);

	if (this->m_ItemDropType != 0)
	{
		if (Option1 != 0)
		{
			Option1 = GetLargeRand() % 2;
		}

		if (Option2 != 0)
		{

			Option2 = GetLargeRand() % 2;

		}

		Option3 = 1 + (GetLargeRand() % gServerInfo.m_MaxItemOption);

		if ((GetLargeRand() % 100) < (Option3 * 3))
		{
			Option3 = (gServerInfo.m_MaxItemOption + 1) - Option3;
		}
		else
		{
			Option3 = 0;
		}

		if (NewOption != 0)
		{
			Option1 = 1;
		}

	}
	if (gServerInfo.m_CheckOptionsDrop)
	{
		if (Option1 != 0)
		{
			if (GetLargeRand() % 100 < gServerInfo.m_SkillPecent)
			{
				Option1 = 1;
			}
			else
			{
				Option1 = 0;
			}
		}
		if (Option2 != 0)
		{
			if (GetLargeRand() % 100 < gServerInfo.m_LuckPecent)
			{
				Option2 = 1;
			}
			else
			{
				Option2 = 0;
			}
		}
		if (Option3 != 0)
		{
			Option3 = 1 + (GetLargeRand() % Option3);

			if ((GetLargeRand() % 100) < gServerInfo.m_OptionPecent)
			{
				Option3 = (Option3 + 1) - Option3;
			}
			else
			{
				Option3 = 0;
			}
		}
		//LogAdd(LOG_RED,"DEBUG OPt Life %d",Option3);
	}

	BYTE SocketOption[MAX_SOCKET_OPTION] = { SOCKET_ITEM_OPTION_NONE,SOCKET_ITEM_OPTION_NONE,SOCKET_ITEM_OPTION_NONE,SOCKET_ITEM_OPTION_NONE,SOCKET_ITEM_OPTION_NONE };

	if (gSocketItemType.CheckSocketItemType(lpInfo->Index) != 0)
	{
		if (lpInfo->SocketOption == 0)
		{
			gItemOptionRate.GetItemOption6(0, &SocketOption[0]);
			gItemOptionRate.MakeSocketOption(lpInfo->Index, SocketOption[0], &SocketOption[0]);
		}
		else
		{
			for (int n = 0; n < lpInfo->SocketOption; n++)
			{
				SocketOption[n] = 0xFE;
			}
		}
	}

	lpItem->m_Level = level;

	lpItem->Convert(lpInfo->Index, Option1, Option2, Option3, NewOption, SetOption, 0, 0, SocketOption, 0xFF);
	return 1;
}

bool CItemBag::DropItem(LPOBJ lpObj,int map,int x,int y) // OK
{
	if(this->m_ItemBagEx.m_ItemBagInfo.empty() == 0)
	{
		return this->m_ItemBagEx.DropItem(lpObj,map,x,y);
	}

	for(int n=0;n < this->m_ItemDropCount;n++)
	{
		int px = x;
		int py = y;

		if(this->m_ItemDropCount > 1 || (gMap[map].CheckAttr(px,py,4) != 0 || gMap[map].CheckAttr(px,py,8) != 0))
		{
			if(this->GetRandomItemDropLocation(map,&px,&py,2,2,10) == 0)
			{
				px = lpObj->X;
				py = lpObj->Y;
			}
		}

		if((GetLargeRand()%100) >= this->m_ItemDropRate)
		{
			gMap[map].MoneyItemDrop(this->m_DropZen,px,py);
			continue;
		}

		if((GetLargeRand()%10000) < this->m_SetItemDropRate)
		{
			gSetItemType.MakeRandomSetItem(lpObj->Index,map,px,py);
			continue;
		}

		CItem item;

		if(this->GetItem(lpObj,&item) != 0)
		{
			GDCreateItemSend(lpObj->Index,
				map,px,py,item.m_Index,
				(BYTE)item.m_Level,0,
				item.m_Option1,
				item.m_Option2,
				item.m_Option3,
				lpObj->Index,
				item.m_NewOption,
				item.m_SetOption,
				item.m_JewelOfHarmonyOption,
				item.m_ItemOptionEx,
				item.m_SocketOption,
				item.m_SocketOptionBonus,0);
			continue;
		}
	}

	if(this->m_SendFirework != 0)
	{
		GCFireworksSend(lpObj,lpObj->X,lpObj->Y);
	}

	return 1;
}

bool CItemBag::DropInventoryItem(LPOBJ lpObj,int map,int x,int y) // OK
{
	if(this->m_ItemBagEx.m_ItemBagInfo.empty() == 0)
	{
		return this->m_ItemBagEx.DropItem(lpObj, map, x, y);
	}

	for(int n = 0 ; n < this->m_ItemDropCount ; n++ )
	{
		int px = x;
		int py = y;

		if(this->m_ItemDropCount > 1 || (gMap[map].CheckAttr(px, py, 4) != 0 || gMap[map].CheckAttr(px, py, 8) != 0))
		{
			if(this->GetRandomItemDropLocation(map, &px, &py, 2, 2, 10) == 0)
			{
				px = lpObj->X;
				py = lpObj->Y;
			}
		}

		if(( GetLargeRand() % 100 ) >= this->m_ItemDropRate )
		{
			if ( !gObjCheckMaxMoney(lpObj->Index, this->m_DropZen) )
			{
				if (lpObj->Money < MAX_MONEY)
				{
					lpObj->Money = MAX_MONEY;
					continue;
				}
			}
			else
			{
				lpObj->Money += this->m_DropZen;
			}
			continue;
		}

		if(( GetLargeRand() % 10000 ) < this->m_SetItemDropRate )
		{
			gSetItemType.MakeRandomSetItem(lpObj->Index, map, px, py);
			continue;
		}

		CItem item;
		ITEM_INFO ItemInfo;

		if(this->GetItem(lpObj, &item) != 0)
		{
			if( gItemManager.GetInfo(item.m_Index,&ItemInfo) == 0 )
			{
				continue;
			}

			int crear = 0;

			int MaxY = ( gItemManager.GetInventoryMaxValue( lpObj )-INVENTORY_WEAR_SIZE ) / 8;

			for( int y = 0 ; y < MaxY; y++ )
			{
				for( int x = 0 ; x < 8; x++ )
				{
					if( lpObj->InventoryMap[((y*8)+x)] == 0xFF )
					{
						BYTE slot = gItemManager.InventoryRectCheck(lpObj->Index, x, y, ItemInfo.Width, ItemInfo.Height);

						if(slot != 0xFF)
						{
							if( crear == 0 )
							{
							GDCreateItemSend(lpObj->Index,
								0xEB,0,0,item.m_Index,
								(BYTE)item.m_Level,0,
								item.m_Option1,
								item.m_Option2,
								item.m_Option3,
								lpObj->Index,
								item.m_NewOption,
								item.m_SetOption,
								item.m_JewelOfHarmonyOption,
								item.m_ItemOptionEx,
								item.m_SocketOption,
								item.m_SocketOptionBonus,0);
								//--
								crear = 1;
							}
						}
					}
				}
			}
			
			if ( crear == 0 )
			{
				return 0;
			}

			continue;
		}
	}

	return 1;
}
