// CustomItem.h: interface for the CCustomItem class.
//
//////////////////////////////////////////////////////////////////////

#pragma once



struct CUSTOM_ITEM_INFO
{
	int Index;
	int ItemIndex;
	int ColorR;
	int ColorG;
	int ColorB;
	char ModelName[32];
};
struct CUSTOM_RINGPEN_INFO
{
	int Index;
	int ItemIndex;
	int Type;
};
class CCustomItem
{
public:
	CCustomItem();
	virtual ~CCustomItem();
	void Load(CUSTOM_ITEM_INFO* info);
	CUSTOM_ITEM_INFO* GetInfoByItem(int ItemIndex);
	BOOL GetCustomItemColor(int ItemIndex, float* ItemColor);
	int IsCustomItem(int ItemIndex);

	void LoadRingPen(CUSTOM_RINGPEN_INFO* info);
	int IsCustomRingPenType(int ItemIndex);

public:
	std::map<int, CUSTOM_ITEM_INFO> m_CustomItemInfo;
	std::map<int, CUSTOM_RINGPEN_INFO> m_CustomRingPenInfo;
};

extern CCustomItem gCustomItem;
