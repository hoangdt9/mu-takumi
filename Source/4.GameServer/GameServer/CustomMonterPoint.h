#pragma once
#include "User.h"
#include "Protocol.h"
#include "ItemManager.h"
class CustomMonterPoint
{
public:
	struct DATA_MONTER_POINT
	{
		int ClassMin;
		int ClassMax;
		int PointAdd;
	};
	struct SDHP_MONTERPOINT_SAVE_SEND
	{
		PBMSG_HEAD header; // C1:39
		WORD index;
		char account[11];
		char name[11];
		DWORD PointAdd;
	};


	CustomMonterPoint();
	~CustomMonterPoint();
	void Load(char* FilePath);

	int m_Enable;
	int m_Notice;
	std::string m_Text;

	std::vector< DATA_MONTER_POINT> m_DataMonterPoint;
	void CustomMonterPoint::MonsterDieProc(LPOBJ lpObj, LPOBJ lpTarget);
	void CustomMonterPoint::GDAddPointSaveSend(int aIndex, int AddPoint);
};

extern CustomMonterPoint gCustomMonterPoint;