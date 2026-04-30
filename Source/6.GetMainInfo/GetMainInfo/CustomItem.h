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
	void Init();
	void Load(char* path);
	void SetInfo(CUSTOM_ITEM_INFO info);

	void LoadRingPen(char* path);
	void SetInfoRingPen(CUSTOM_RINGPEN_INFO info);
public:
	CUSTOM_ITEM_INFO m_CustomItemInfo[MAX_CUSTOM_ITEM];
	CUSTOM_RINGPEN_INFO m_CustomRingPenInfo[MAX_CUSTOM_ITEM];
};

extern CCustomItem gCustomItem;
