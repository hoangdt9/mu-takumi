#pragma once


typedef struct
{
	int world;
	char MapName[128];
	char TgaMapName[128];
	char lpszMp3[128];
}PathMap;

class CUIMapName
{
public:
	CUIMapName(void);
	virtual ~CUIMapName(void);
	void OpenScritp(char* path);

	PathMap m_MapInfo[MAX_MAP];
};
extern CUIMapName g_pUIMapName;