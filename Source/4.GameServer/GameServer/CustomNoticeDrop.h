#pragma once
#include "StdAfx.h"
#include "user.h"
#include "Protocol.h"



struct ITEM_NOTICE_DROP
{
	int index;
	int level;
};


class CustomNoticeDrop
{
public:
	CustomNoticeDrop(void);
	~CustomNoticeDrop(void);
	void	Init();
	void	Load(char* path);
	void NoticeItem(LPOBJ lpObj, CItem* lpItem);

private:
	int Enable;
	int AllItemExc;
	int TypeShow;

	std::vector<ITEM_NOTICE_DROP> m_ItemNotice;

};

extern CustomNoticeDrop gCustomNoticeDrop;