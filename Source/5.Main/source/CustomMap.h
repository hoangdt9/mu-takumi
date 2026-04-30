#pragma once
typedef struct
{
	int world;
	char MapName[128];
	char TgaMapName[128];
	char lpszMp3[128];
}PathMap;

typedef std::map<int, std::string> ImgPathMap;
typedef std::map<int, PathMap> listMap;
class CCustomMap
{
public:
	// ----
	//void	Init();
	//void	ReadList(char * File);
	void	Load();
	enum SHOW_STATE { HIDE, FADEIN, SHOW, FADEOUT };
	char* GetNameMap(int world);
	char* GetMapName(int iMap);
	PathMap* GetMapInfo(int world);
	void OpenScritp(PathMap* thisa);

	listMap MapPath;
	// ----
}; extern CCustomMap gCustomMap;