#include "stdafx.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "ServerDisplayer.h"
#include "Util.h"
#include "Message.h"
#include "User.h"
#include "Path.h"
#include "ServerInfo.h"
#include "CustomEventTime.h"
#include "MemScript.h"
#include "Notice.h"
#include "InvasionManager.h"
#include "CustomArena.h"
CCustomEventTime gCustomEventTime;

CCustomEventTime::CCustomEventTime() // OK
{
	//gCustomEventTime.CountTime = 0;
	gCustomEventTime.DataEventTime.clear();
	this->LoadDataTime = 0;
	this->m_SendClientDataEventTime.clear();
	this->m_Enable = 0;
}


CCustomEventTime::~CCustomEventTime() // OK
{

}
void CCustomEventTime::LoadData()
{
	for (int i = 0; i < 30; i++)
	{
		//if (strlen(gInvasionManager.m_InvasionInfo[i].AlertMessage) < 1) continue;
		gCustomEventTime.AddDataEventsTime(gInvasionManager.m_InvasionInfo[i].AlertMessage, &gServerDisplayer.EventInvasion[i]);
	}
	for (int ii = 0; ii < 30; ii++)
	{
		//if (strlen(gCustomArena.GetArenaName(ii)) < 1) continue;
		gCustomEventTime.AddDataEventsTime(gCustomArena.GetArenaName(ii), &gServerDisplayer.EventCustomArena[ii]);
	}
}
void CCustomEventTime::AddDataEventsTime(char* NameEvent, int* Time)
{
	//if (this->CountTime > MAX_EVENTS_TIME) return;
	//memcpy(&this->DataEventTime[this->CountTime].NameEvent, NameEvent, sizeof(this->DataEventTime[this->CountTime].NameEvent));
	//this->DataEventTime[this->CountTime].time = Time;
	//this->CountTime++;
	BCUSTOM_EVENTTIME_DATA info;
	memcpy(&info.NameEvent, NameEvent, sizeof(info.NameEvent));
	info.TimeEvent = Time;
	gCustomEventTime.DataEventTime.push_back(info);
}
void CCustomEventTime::LoadFileXML(char* FilePath)
{
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}
	//--
	pugi::xml_node CustomEventTime = file.child("CustomEventTime");
	this->m_Enable = CustomEventTime.attribute("Enable").as_int();

	this->m_SendClientDataEventTime.clear();
	for (pugi::xml_node Event = CustomEventTime.child("Event"); Event; Event = Event.next_sibling())
	{
		DATA_CUSTOMEVENTTIME info;
		info.NumberEvent = Event.attribute("NumberEvent").as_int();
		strcpy_s(info.NameEvent, Event.attribute("NameEvent").as_string());
		strcpy_s(info.DesString, Event.attribute("DesString").as_string());
		info.NumberGate = Event.attribute("NumberGate").as_int();
		this->m_SendClientDataEventTime.push_back(info);
	}
}
int MaxPerPage = 20;
void CCustomEventTime::GCReqEventTime(int Index, PMSG_CUSTOM_EVENTTIME_RECV* lpMsg)
{

#if (GAMESERVER_CLIENTE_UPDATE >= 2)

	if (gServerInfo.m_CustomEventTimeSwitch == 0)
	{
		return;
	}

	if (gObjIsConnected(Index) == false)
	{
		return;
	}
	if (!this->m_Enable)
	{
		gNotice.GCNoticeSend(Index, eMessageBox, 0, 0, 0, 0, 0, "Chuc nang khong duoc su dung");//
		return;
	}
	int GetPage = lpMsg->Page;
	BYTE send[8192];

	PMSG_CUSTOM_EVENTTIME_SEND pMsg;

	pMsg.header.set(0xF3, 0xE8, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;
	pMsg.MaxList = gCustomEventTime.m_SendClientDataEventTime.size();

	CUSTOM_EVENTTIME_DATA info;

	for (int n = (GetPage * MaxPerPage); n < gCustomEventTime.m_SendClientDataEventTime.size(); n++)
	{
		if (pMsg.count >= MaxPerPage) break;
		info.index = n;
		if (gCustomEventTime.m_SendClientDataEventTime[n].NumberEvent >= 0 && gCustomEventTime.m_SendClientDataEventTime[n].NumberEvent < gCustomEventTime.DataEventTime.size())
		{
			info.time = *gCustomEventTime.DataEventTime[gCustomEventTime.m_SendClientDataEventTime[n].NumberEvent].TimeEvent;
		}
		else
		{
			info.time = -1;
		}

		info.NumberGate = gCustomEventTime.m_SendClientDataEventTime[n].NumberGate;
		memcpy(&info.NameEvent, gCustomEventTime.m_SendClientDataEventTime[n].NameEvent, sizeof(info.NameEvent));
		memcpy(&info.DesString, gCustomEventTime.m_SendClientDataEventTime[n].DesString, sizeof(info.DesString));
		if ((size + sizeof(info)) >= 8192) break;
		
		memcpy(&send[size], &info, sizeof(info));
		size += sizeof(info);
		pMsg.count++;
	}
	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(Index, send, size);

#endif
	return;
}