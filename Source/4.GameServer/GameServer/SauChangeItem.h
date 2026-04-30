#pragma once
#include "StdAfx.h"
#include "Protocol.h"
#include "DSProtocol.h"
#include "User.h"
#include "OfflineMode.h"
#include "ItemManager.h"
#if(SAUDOIITEM)
#define eMessageBox				255
struct DOIITEM_MSG
{
	int Index;
	char Message[256];
};
struct NPC_DATA_DOIITEM
{
	int NPCClass;
	int NPCMap;
	int NPCX;
	int NPCY;
	int NPCDir;
	void Clear()
	{
		NPCClass = -1;
		NPCMap = 0;
		NPCX = 0;
		NPCY = 0;
		NPCDir = 0;
	}
};
struct ITEM_DOI_DATA
{
	int IndexItem;
	int Level;
	int Skill;
	int Luck;
	int Option;
	int Exc;
};


struct ITEM_KQ_DATA
{
	int IndexItem;
	int IndexItem1;
	int IndexItem2;
	int IndexItem3;
	int IndexItem4;
	int IndexItem5;
};

struct GROUPDOIITEM_DATA
{
	int Index;
	int Notice;
	char Name[90];
	int Wcoin;
	ITEM_DOI_DATA ItemChinh;
	std::vector<ITEM_DOI_DATA> ItemPhu;
	ITEM_KQ_DATA ItemKetQua;

};
struct INFOITEM_DOIITEM_CLIENT
{
	PSWMSG_HEAD header;
	BYTE ActiveMix;
	BYTE ItemChinh[16];
	BYTE ItemPhu[3][16];
	BYTE ItemKetQua[6][16];
	int Rate;
};

class SauChangeItem
{
public:
	SauChangeItem();
	virtual ~SauChangeItem();
	int Enable;
	NPC_DATA_DOIITEM* mNPCData;
	void LoadConfig(char* FilePath);
	bool SauChangeItem::Dialog(LPOBJ lpObj, LPOBJ lpNpc);
	std::map<int, GROUPDOIITEM_DATA> m_GroupDoiItemData;

	void SauChangeItem::SetStateInterface(int aIndex, int Type = 0);
	void SauChangeItem::ProcItemSend(int aIndex, int SlotItem);
	void SauChangeItem::SendInfoItemCache(int aIndex);
	void SauChangeItem::BackItem(int aIndex, int BackSlot);

	void SetNPC();
private:
	//===Mess
	std::map<int, DOIITEM_MSG> m_MessageInfoBP;
	char* GetMessage(int index);

};

extern SauChangeItem gSauChangeItem;



#endif