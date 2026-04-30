#pragma once
#include "Protocol.h"
#if(CB_VIEWCHARITEM)
class CB_ViewCharItem
{

public:
	struct PMSG_COUNTLIST_VIEWCHAR
	{
		PSWMSG_HEAD header;
		int Count;
		char Name[11];
	};
	struct GETINFOCHAR_DATA
	{
		BYTE Dur;
		BYTE Item[12];
		int  PeriodTime;
	};

	CB_ViewCharItem();
	~CB_ViewCharItem();
	void OpenClose();
	void DrawWindow();
	void SendRequestViewItem(int aIndex);
	void RecvProtocol(BYTE* Recv);

private:


	typedef struct tagEQUIPMENT_ITEM
	{
		int x, y;
		int width, height;
		DWORD dwBgImage;
	} EQUIPMENT_ITEM;
#if(HAISLOTRING)
	EQUIPMENT_ITEM m_ViewCharItemSlots[EQUIPMENT_PENTA];
	ITEM			CharItemInfo[EQUIPMENT_PENTA];
	std::vector<GETINFOCHAR_DATA> m_CharItemInfo;
#else
	EQUIPMENT_ITEM m_ViewCharItemSlots[MAX_EQUIPMENT];
	ITEM			CharItemInfo[MAX_EQUIPMENT];
#endif
	std::string		CharViewName;
	void ClearData();
	void SetImgSlot(float x, float y);
	
};

extern CB_ViewCharItem* gCB_ViewCharItem;
#endif