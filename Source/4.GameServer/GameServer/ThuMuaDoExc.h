
#include "User.h"

#if(BOTHUMUADORAC == 1)

#define MAX_BOTTHUMUA	30
#define MAX_BOTTHUMUASKILLS	5


struct BotThuMuaBodyItems
{
	int num;
	int level;
	int opt;
	int Enabled;
};

struct DoiItem
{
	int Index;
	int IndexBot;
	int OnlyVip;
	int LoaiTien;
	int MinCoin;
	int MaxCoin;
	int SeriCheck;
	int ItemExc;
	int ItemAnc;
	int ItemBag;
};

struct DoiItemList
{
	int Index;
	int IndexBot;
	int ItemIndex;
	int LoaiTien;
	int MinCoin;
	int MaxCoin;
	int SeriCheck;
	int ItemBag;
};

struct MESSAGE_INFO_TM
{
	int Index;
	char Message[256];
};


struct botThuMuaStruct
{
	int Class;
	char Name[11];
	BYTE Map;
	BYTE X;
	BYTE Y;
	BYTE Dir;
	int OnlyVip;
	int ActiveGensFamily;
	int ContributionGens;
	int GensFamily;
	int Enabled;
	int Zen;
	int WCoinCMin;
	int WCoinCMax;
	int WCoinP;
	int ChangeColorName;
	int index;
	BOOL OnlySameType;
	BOOL OnlyLowerIndex;
	BOOL AcceptAncient;
	WORD MaxLevel;
	BYTE MaxExc;
	BotThuMuaBodyItems body[9];
	BYTE SkillCount;
	BYTE Rate;
	BYTE Type;

	BOOL AllowLevel;
	BOOL AllowOpt;
	BOOL AllowLuck;
	BOOL AllowSkill;
	BOOL AllowExc;
	BOOL AllowFFFF;
};

class BotThuMuaer
{
public:
	void Read(char * path);
	void MakeBot();
	BOOL IsInTrade(int aIndex);
	BOOL TradeOpen(int index, int nindex);
	void TradeOk(int aIndex);
	void TradeCancel(int aIndex);
	char* GetMessage(int index);
	short Max_Normal_Level;
	BYTE gObjInventoryInsertItemPos(int aIndex, CItem item, int pos, BOOL RequestCheck);

	DoiItem* GetInfoDoiItem(int Numbot);
	DoiItemList* GetInfoDoiItemList(int Numbot, int ItemIndex);

	bool  GetCoinDoiItem(int Numbot, int* Coin);
private:
	int GetBotIndex(int aIndex);
	bool Enabled;
	int	getNumberOfExcOptions(int checksum);
	bool AllowExc(BYTE BotNum, BYTE ExcOpt);
	BOOL XuLyItemThuMua(int aIndex, int BotNum);
	botThuMuaStruct bot[MAX_BOTTHUMUA];

	std::map<int, DoiItem> m_DoiItem;
	std::map<int, DoiItemList> m_DoiItemList;
	std::map<int, MESSAGE_INFO_TM> m_MessageInfoTM;
	DWORD CacheSerial[1000];
};

extern  BotThuMuaer BotThuMua;

#endif