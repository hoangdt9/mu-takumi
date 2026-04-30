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
	void Load(char * path);
	void SetInfo(CUSTOM_DMG_COLOR info);
public:
	CUSTOM_DMG_COLOR m_CustomDmgColor[MAX_CUSTOM_DMG];
}; extern CustomDmgColor gCustomDmgColor;