#include "stdafx.h"
#include "CustomCommandInfo.h"


CCustomCommandInfo gCustomCommandInfo;
// -------------------------------------------------------------------------------

CCustomCommandInfo::CCustomCommandInfo()
{
	this->Init();
}
// -------------------------------------------------------------------------------

CCustomCommandInfo::~CCustomCommandInfo()
{
	// ----
}
// -------------------------------------------------------------------------------

void CCustomCommandInfo::Init() // OK
{
	for (int n = 0;n < MAX_CUSTOM_COMMAND;n++)
	{
		this->m_CustomCommandInfo[n].Index = -1;
	}
	this->Count = 0;
	this->Page = 0;
}

void CCustomCommandInfo::Load(CUSTOM_COMMAND_INFO* info) // OK
{

	for (int n = 0;n < MAX_CUSTOM_COMMAND;n++)
	{
		this->SetInfo(info[n]);
	}
}

void CCustomCommandInfo::SetInfo(CUSTOM_COMMAND_INFO info) // OK
{
	if (info.Index < 0 || info.Index >= MAX_CUSTOM_COMMAND)
	{
		return;
	}
	this->Count = info.Index + 1;
	this->m_CustomCommandInfo[info.Index] = info;
}

CUSTOM_COMMAND_INFO* CCustomCommandInfo::GetInfo(int index) // OK
{
	if (index < 0 || index >= MAX_CUSTOM_COMMAND)
	{
		return 0;
	}

	if (this->m_CustomCommandInfo[index].Index != index)
	{
		return 0;
	}

	return &this->m_CustomCommandInfo[index];
}