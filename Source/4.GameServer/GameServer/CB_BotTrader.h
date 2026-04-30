#pragma once
#include "Protocol.h"
#include "Item.h"

class CB_BotTrader
{
public:
	struct DATA_ITEM_BOT
	{
		int SlotItem;
		int TypeItem;
		int IndexItem;
		int LevelItem;
		int OptionItem;
	};
	struct DATA_CREATE_BOT
	{
		int IndexBot;
		int Class;
		int Enable;
		int OnylVip;
		char BotName[11];
		int Map;
		int CoordX;
		int CoordY;
		int Dir;
		int CustomShowInfo;
		std::vector<DATA_ITEM_BOT> m_DataItemBot;
		int MixIndexMin;
		int MixIndexMax;
		int Cache_aIndex;
	};

	struct DATA_NEED_ITEM
	{
		int Type;
		int Index;
		int MinLevel;
		int MaxLevel;
		int MinOpt;
		int MaxOpt;
		int Luck;
		int Skill;
		int NeedExc;
		int Durability;
		int KeepLevel;
		int KeepOption;
		int KeepItemMixFail;
	};

	struct DATA_RESULT
	{
		
		int Type;
		int Index;
		int LevelMin;
		int LevelMax;
		int OptMin;
		int OptMax;
		int Luck;
		int Skill;
		int Exc;
		int Durability;
	};

	struct DATA_MIX_DATA
	{
		int MixIndex;
		char  MixName[55];
		int SuccessRate;
		int PriceType;
		int PriceCount;
		int FailMes;
		int SuccessMes;
		std::vector<DATA_NEED_ITEM> mDataNeedItem;
		std::vector<DATA_RESULT> mDataResultItem;
	};


	struct MESSAGE_INFO
	{
		int Index;
		char Message[256];
	};

	CB_BotTrader();
	~CB_BotTrader();

	int Enabled;

	char* GetMessage(int index);
	std::map<int, MESSAGE_INFO> m_MessageInfo;

public:
	//==Packet
	struct LIST_MIX_TRADER
	{
		char NameMix[60];
		int MixIndex;
	};
	enum  TYPE_SEND_RECV
	{
		eSendRecvListDataMix,
		eSendRecvListItemMix,
	};
	struct DATA_ITEMSEND
	{
		short IndexItem;
		int MinLevel;
		int MaxLevel;
		int MinOpt;
		int MaxOpt;
		int Luck;
		int Skill;
		int NeedExc;
		int Durability;
	};
	struct PMSG_CUSTOM_LIST
	{
		PSWMSG_HEAD header;
		int TypeSend;
		int count;

		int TypeCoin;
		int Coin;
		int Rate;
		int KeepLevel;
		int KeepOption;
		int KeepItemMixFail;
	};

	void LoadXML(char* FilePath);


	void ClearObjBot();
	std::map<int, CB_BotTrader::DATA_CREATE_BOT> m_DataCreateBot;
	std::map<int, CB_BotTrader::DATA_MIX_DATA> m_DataMix;

	void MakeBot();
	DATA_CREATE_BOT* GetBotIndex(int aIndex);
	BOOL IsInTrade(int aIndex);
	BOOL TradeOpen(int index, int nindex);
	void TradeOk(int aIndex);
	void TradeCancel(int aIndex);
	BYTE gObjInventoryInsertItemPos(int aIndex, CItem item, int pos, BOOL RequestCheck);
	BOOL XuLyItemTrade(int aIndex, int BotNum);
	int GetItemCountMixIndex(int MixIndex);
	int ItemCount(int aIndex);
	BYTE BoxExcOptions(int maxOptions);
	void SendDataIsTrade(int aIndex, int IndexBot, int TypeSend, int MixIndex = -1);
};

extern CB_BotTrader	gCB_BotTrader;