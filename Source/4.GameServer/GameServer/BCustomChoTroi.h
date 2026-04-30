#pragma once
#include "StdAfx.h"
#include "Protocol.h"
#include "DSProtocol.h"
#include "User.h"
#include "OfflineMode.h"
#include "ItemManager.h"
#if(CUSTOM_BCHOTROI)
#define MARKET_ITEM_BUFFER		16
#define MARKET_ITEM_MAX			100
#define MARKET_NAME_LEN			11
#define MARKET_DEBUG			1
#define MARKET_PRICE_DEV		1
#define MARKET_FILTER_DEV		1
#define MARKET_NAME_DEV			1
#define MARKET_PAGE_DEV			1
#define MAX_ITEM_LEVEL	15
#define DBI_GET_LEVEL(x)  ( ((x)>>3)& MAX_ITEM_LEVEL )
#define DBI_GET_SKILL(x)  ( (x) >> 7 )
#define DBI_GET_LUCK(x)  ( ((x) >> 2) & 0x01)
#define DBI_GET_OPTION(x)  ( ( (x) ) & 0x03 )
#define DBI_GET_OPTION16(x) ( (x) & 0x40  )
#define DBI_GET_DINO_OPTION(x)  ( DBI_GET_OPTION16((x)) >> 4 )
#define DBI_GET_NOPTION(x)  ( (x) & 0x3F  )
#define DBI_GET_380OPTION(x) ( ((x) & 0x08) << 4  )

enum
{
	eMarketPriceWC = 1,
	eMarketPriceWP = 2,
	eMarketPriceGP = 3,
	eMarketPriceB = 4,
	eMarketPriceS = 5,
	eMarketPriceC = 6,
};



struct PMSG_REQ_MARKET_SELL
{
	PSBMSG_HEAD h;
	int Result;
	int ItemPos;
	int ItemPriceType;
	int ItemPrice;
	int ItemDay;
	int Pass;
};

struct VAT_DATA_INFO
{
	int TypeItem;
	int WCRate;
	int WPRate;
	int GPRate;
	int BRate;
	int SRate;
	int CRate;
};

struct DATA_ITEMBLOCK
{
	int IndexMin;
	int IndexMax;
	int LvMin;
	int LvMax;
	int Luck;
	int Skill;
	int Opt;
	int ExcOpt;
};

struct CUSTOM_CHOTROI_MESSAGE
{
	int Index;
	char Message[256];
};

struct SDHP_REQ_MARKET_SELL
{
	PBMSG_HEAD2 h;
	int Result;
	int aIndex;
	char Account[11];
	char Name[11];
	int PriceType;
	int Price;
	int ItemPos;
	BYTE ItemData[MARKET_ITEM_BUFFER];
	int TypeItem;
	int ItemDay;
	int Pass;
};
//===Get List Cho Troi
struct PMSG_REQ_MARKET_ITEM //Client Send GS
{
	PSBMSG_HEAD h;
	int Result;
	int PriceType;
	int PageNumber;
	int GetTypeItem;
};
struct PMSG_REQ_MARKET_BUY
{
	PSBMSG_HEAD h;
	int Result;
	int ID;
	int Pass;
};
struct SDHP_REQ_MARKET_ITEM //GS Send DS
{
	PSBMSG_HEAD h;
	int Result;
	int aIndex;
	int PriceType;
	int GetTypeItem;
	char Account[11];
	int PageNumber;
	int TypeHSD;
};
struct MARKET_DATA
{
	int ID;
	char Name[MARKET_NAME_LEN];
	BYTE Item[MARKET_ITEM_BUFFER];
	int PriceType;
	int Price;
	int TypeItem;
	int TimeItemRaoBan;
	int Pass;
};
//==GS TO DS
struct SDHP_REQ_MARKET_BUY
{
	PSBMSG_HEAD h;
	int Result;
	int aIndex;
	int ID;
};
//==DS Send GS
struct SDHP_ANS_MARKET_BUY
{
	PSBMSG_HEAD h;
	int Result;
	int aIndex;
	int ID;
};
struct SDHP_REQ_MARKET_STATUS
{
	PSBMSG_HEAD h;
	int Result;
	int aIndex;
	char Account[11];
};
struct SDHP_ANS_MARKET_STATUS
{
	BYTE ItemData[MARKET_ITEM_BUFFER];
	char Account[11];
	int PriceType;
	int PriceValue;
	int TypeItem;
};
struct BCSEV_COUNTLIST
{
	PSWMSG_HEAD header;
	int Count;
	int Type;
};
class BCustomChoTroi
{
public:
	int TypeHSD;
	void Load(char* FilePath);
	int Enable;
	int OnCointType;
	std::map<int, VAT_DATA_INFO> m_DataVAT;
	std::vector<DATA_ITEMBLOCK> m_DataItemBlock;
	std::vector<MARKET_DATA> m_ListDataChoTroi[MAX_OBJECT];
	bool CheckItemBlockSend(CItem* lpItem);
	void ClientSendItemRaoBan(int aIndex, PMSG_MOVEITEM* lpMsg);
	void RollBack(int aIndex, XULY_CGPACKET* lpMsg);
	void CGReqItemSell(PMSG_REQ_MARKET_SELL* lpMsg, int aIndex);
	void CGReqItemList(PMSG_REQ_MARKET_ITEM* lpMsg, int aIndex);
	void DGAnsItemList(CBCUSTOM_LOAD_COUNT* lpMsg);
	void CGOpenListChoTroi(int aIndex);
	void SendListUser(int Index);

	//==BUY
	bool GetCheckMoney(int aIndex, int PriceType, int PriceValue);
	void CGReqItemBuy(PMSG_REQ_MARKET_BUY* lpMsg, int aIndex);
	void DGAnsItemBuy(SDHP_ANS_MARKET_BUY* lpMsg);

	void GDReqItemStatus(int aIndex);;
	void DGAnsItemStatus(CBCUSTOM_LOAD_COUNT* lpMsg);
	int GetRateTaxTypeItem(int TypeItem, int TypeCoin);


	//===Mess
	std::map<int, CUSTOM_CHOTROI_MESSAGE> m_MessageInfoBP;
	char* GetMessage(int index);
	void GuiThuThongBao(LPOBJ lpTarget, char* Subject, char* message, ...);
private:
};

extern BCustomChoTroi gBCustomChoTroi;
#endif