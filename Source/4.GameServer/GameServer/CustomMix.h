// ---
#include "stdafx.h"
#include "RandomManager.h"
// ---
#define MAX_MIX 5000
// ---
struct CUSTOM_MIX
{
	int  m_Index;
	int  m_MixMoney;
	int  m_MixRate_AL0;
	int  m_MixRate_AL1;
	int  m_MixRate_AL2;
	int  m_MixRate_AL3;
	int  m_CountGroup;
	int  m_CountItem;
	int m_OpcionExeRand;
//Index		MixMoney	MixRate_AL0		MixRate_AL1		MixRate_AL2		MixRate_AL3		CountGroup	CountItem
};

struct CUSTOM_BAG_MIX
{
	int index;
	int  m_Index;
	int  m_ItemMix;
	int  m_ItemLevel;
	int  m_Count;
//Index	ItemBag	level	Count
};

struct CUSTOM_ITEM_MIX
{
	int index;
	int  m_Index;
	int  m_Group;
	int  m_Count;
	int  m_ItemIndexMin;
	int  m_ItemIndexMax;
	int  m_ItemLevel;
	int  m_Skill;
	int  m_Luck;
	int  m_Opcion;
	int  m_Excelent;
	int  m_SetACC;
//Index		Group		Count	ItemIndex	ItemIndex	MinItemLevel	Skill	Luck	MinAdicional	Exe		ItemSetOption
};
// ---
struct CUSTOM_ITEM_MIX_RESULT
{
	int index;
	int  m_Index;
	int  m_Group;
	int  m_ItemIndexMin;
	int  m_ItemIndexMax;
	int  m_ItemLevel;
	int  m_Skill;
	int  m_Luck;
	int  m_Opcion;
	int  m_Excelent;
	int  m_SetACC;
	int  Socket1;
	int  Socket2;
	int  Socket3;
	int  Socket4;
	int  Socket5;
//Index		Group	ItemIndexMin	ItemIndexMax	ItemLevel	Skill	Luck	MinAdicional	Exe		ItemSetOption	ItemClass
};

struct DataResult
{
	int MixNum;
	std::map<int, CUSTOM_ITEM_MIX_RESULT> m_DataResult;
};
//===Custom
struct BCUSTOM_MIX
{
	int  Index;
	int  MixMoney;
	int  MixRate[4];
	int  CheckItemOption;
	int  CheckItemNewOption;
	//0
	//Index	MixMoney	MixRate_AL0	MixRate_AL1	MixRate_AL2	MixRate_AL3	CheckItemOption	CheckItemNewOption
};
struct BCUSTOM_MIX_INFO
{
	int  Index;
	int  Name1;
	int  Name2;
	int  Name3;
	int  Des1;
	int  Des2;
	int  Des3;
	int  Advice1;
	int  Advice2;
	int  Advice3;
	//1
	//Index	Name1	Name2	Name3	Des1	Des2	Des3	Advice1	Advice2	Advice3
};
struct BCUSTOM_MIX_NGUYENLIEU
{
	int  Index;
	int  ItemIndexMin;
	int  ItemIndexMax;
	int  Level;
	int  Skill;
	int  Luck;
	int  Op;
	int  Exl;
	int  Item380;
	int  Set;
	int  Harmory;
	int  Socket;
	int  Count;
	int  FailDelItem;
	//2
	//Index	ItemIndexMin	ItemIndexMax	Level   Skill   Luck	Op	Exl	Item380	Set	Harmory	Socket	Count	FailDelItem
};
struct BCUSTOM_MIX_ITEMKETQUA
{
	int  Index;
	int  ItemIndex;
	int  Level;
	int  Skill;
	int  Luck;
	int  Op;
	int  Exl;
	int  Set;
	int  Socket[5];
	//3
	//Index	ItemIndex	Level   Skill   Luck	Op	Exl	Set	Socket1	Socket2	Socket3	Socket4	Socket5
};

struct PMSG_BCUSTOM_MIX_INFO
{
	PSWMSG_HEAD header; // C2:F3:E2
	BYTE count;
	int  Index;
	int  MixMoney;
	int  MixRate;
	int  Name1;
	int  Name2;
	int  Name3;
	int  Des1;
	int  Des2;
	int  Des3;
	int  Advice1;
	int  Advice2;
	int  Advice3;
};

class CCustomMix
{
public:
	int m_count1;
	int m_count2;
	int m_count3;
	int m_count4;

	void Load(char* path);
	int GetCountItemMix(int IndexMix,int counGroup);
	CRandomManager RandomManager(int IndexMix,int counGroup);
	CUSTOM_MIX* GetCustomMix(int IndexMix);
	CUSTOM_ITEM_MIX_RESULT* GetItemResult(int IndexMix,int group,int ItemIndex);
	int GetCountItemBagMix(int IndexMix);
	bool istItemMix(int IndexMix, int group, CItem boxItem);
	bool istItemBagMix(int IndexMix, CItem boxItem);

	std::map<int, CUSTOM_MIX> m_Data_Mix;
	std::map<int, CUSTOM_BAG_MIX> m_Data_Bag;
	std::map<int, CUSTOM_ITEM_MIX> m_Data_Item;
	DataResult DataResult[254];

	//====
	std::map<int, BCUSTOM_MIX> m_BData_Mix;
	std::map<int, BCUSTOM_MIX_INFO> m_BData_Mix_Info;
	std::vector<BCUSTOM_MIX_NGUYENLIEU> m_BDataMixNguyenLieu;
	std::vector<BCUSTOM_MIX_ITEMKETQUA> m_BDataMixItemKetQua;

	int CCustomMix::CheckItemMixForIndex(LPOBJ lpObj, int IndexMix);
	void CCustomMix::GetSendInfoMix(LPOBJ lpObj);
	void CCustomMix::SendInfoCustomMix(int aIndex, int IndexMix);

	int CCustomMix::GetChaoMixItemCount(LPOBJ lpObj, int IndexMin, int IndexMax, int MinLevel, int MaxLevel, int MinOpt, int NeedLuck, int NeedSkill, int NeedExl,int Need380, int NeedAnc, int NeedHarmory, int NeedSocket);
	int CCustomMix::GetSizeNguyenLieu(int IndexMix);
	BCUSTOM_MIX* CCustomMix::BGetCustomMix(int IndexMix);
	CRandomManager CCustomMix::BRandomManager(int IndexMix);
	int CCustomMix::GetFailDelItem(int ItemIndex, int IndexMix);
	
};

extern CCustomMix gCustomMix;
// ---