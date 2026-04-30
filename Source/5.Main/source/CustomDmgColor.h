#pragma once

#define MAX_CUSTOM_DMG	15

struct CUSTOM_DMG_COLOR
{
	int Index;
	int TypeIndex;
	BYTE Red;
	BYTE Green;
	BYTE Blue;
};
class CustomDmgColor
{
public:
	CustomDmgColor();
	virtual ~CustomDmgColor();
	void Init();
	void Load(CUSTOM_DMG_COLOR* info);
	void SetInfo(CUSTOM_DMG_COLOR info);
	CUSTOM_DMG_COLOR* GetInfo(int index);
	CUSTOM_DMG_COLOR* GetInfoTypeIndex(int TypeIndex);

public:
	//std::map<int, CUSTOM_DMG_COLOR> m_CustomDmgColor;
	CUSTOM_DMG_COLOR m_CustomDmgColor[MAX_CUSTOM_DMG];
}; extern CustomDmgColor gCustomDmgColor;