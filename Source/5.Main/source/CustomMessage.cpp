#include "stdafx.h"
#include "CustomMessage.h"

CCustomMessage gCustomMessage;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomMessage::CCustomMessage() // OK
{
	this->Init();
}

CCustomMessage::~CCustomMessage() // OK
{

}

void CCustomMessage::Init() // OK
{
	memset(this->m_DefaultMessage,0,sizeof(this->m_DefaultMessage));

	for(int n=0;n < MAX_CUSTOM_MESSAGE;n++)
	{
		this->m_EngCustomMessageInfo[n].Index = -1;
		this->m_VtmCustomMessageInfo[n].Index = -1;
	}
}

void CCustomMessage::LoadEng(CUSTOM_MESSAGE_INFO* info) // OK
{
	for(int n=0;n < MAX_CUSTOM_MESSAGE;n++)
	{
		this->SetInfoEng(info[n]);
	}
}

void CCustomMessage::LoadVtm(CUSTOM_MESSAGE_INFO* info) // OK
{
	for(int n=0;n < MAX_CUSTOM_MESSAGE;n++)
	{
		this->SetInfoVtm(info[n]);
	}
}



void CCustomMessage::SetInfoEng(CUSTOM_MESSAGE_INFO info) // OK
{
	if(info.Index < 0 || info.Index >= MAX_CUSTOM_MESSAGE)
	{
		return;
	}

	this->m_EngCustomMessageInfo[info.Index] = info;
}

void CCustomMessage::SetInfoVtm(CUSTOM_MESSAGE_INFO info) // OK
{
	if(info.Index < 0 || info.Index >= MAX_CUSTOM_MESSAGE)
	{
		return;
	}
	//g_Console.AddMessage(3, "Loadd Mess VTM %d %s", info.Index, info.Message);
	this->m_VtmCustomMessageInfo[info.Index] = info;
}


CUSTOM_MESSAGE_INFO* CCustomMessage::GetInfoEng(int index) // OK
{
	if(index < 0 || index >= MAX_CUSTOM_MESSAGE)
	{
		wsprintf(this->m_DefaultMessage,"Message %d out of bound!",index);
		return 0;
	}

	if(this->m_EngCustomMessageInfo[index].Index != index)
	{
		wsprintf(this->m_DefaultMessage,"Could not find message %d!",index);
		return 0;
	}

	return &this->m_EngCustomMessageInfo[index];
}

CUSTOM_MESSAGE_INFO* CCustomMessage::GetInfoVtm(int index) // OK
{
	if(index < 0 || index >= MAX_CUSTOM_MESSAGE)
	{
		wsprintf(this->m_DefaultMessage,"Message %d out of bound!",index);
		return 0;
	}

	if(this->m_VtmCustomMessageInfo[index].Index != index)
	{
		wsprintf(this->m_DefaultMessage,"Could not find message %d!",index);
		return 0;
	}

	return &this->m_VtmCustomMessageInfo[index];
}


char* CCustomMessage::GetMessage(int index) // OK
{
	if(strcmp(g_strSelectedML.c_str(),"Eng") == 0)
	{
		CUSTOM_MESSAGE_INFO* lpInfo = this->GetInfoEng(index);
		if(lpInfo==0)
		{
			return this->m_DefaultMessage;
		}
		return lpInfo->Message;
	}

	if(strcmp(g_strSelectedML.c_str(),"Vtm") == 0)
	{
		CUSTOM_MESSAGE_INFO* lpInfo = this->GetInfoVtm(index);
		if(lpInfo==0)
		{
			return this->m_DefaultMessage;
		}
		return lpInfo->Message;
	}

	CUSTOM_MESSAGE_INFO* lpInfo = this->GetInfoEng(index);
	if(lpInfo==0){
		return this->m_DefaultMessage;
	}
	return lpInfo->Message;
}

char * CCustomMessage::GetMessageB(int index) // Text.bmd
{
	return "NULL";
	//return pGetTextLine(pTextLineThis,index);
}