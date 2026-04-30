#include "stdafx.h"
#include "ConnectServerProtocol.h"
#include "ClientManager.h"
#include "Log.h"
#include "Protect.h"
#include "ServerList.h"
#include "SocketManager.h"
#include "Util.h"

namespace
{
	BYTE GetConnectServerPacketSub(BYTE* lpMsg,int size)
	{
		if(size < 4)
		{
			return 0xFF;
		}

		return ((lpMsg[0] == 0xC1 || lpMsg[0] == 0xC2) ? lpMsg[3] : 0xFF);
	}

	void ConnectServerSecurityTrackPacket(int index,BYTE head,BYTE sub,int size)
	{
		if(CLIENT_RANGE(index) == 0)
		{
			return;
		}

		CClientManager* lpClient = &gClientManager[index];

		if(lpClient->CheckState() == 0)
		{
			return;
		}

		DWORD tick = GetTickCount();

		if(lpClient->m_SecurityWindowTick == 0 || (tick-lpClient->m_SecurityWindowTick) > 1000)
		{
			lpClient->m_SecurityWindowTick = tick;
			lpClient->m_SecurityWindowCount = 0;
		}

		if((++lpClient->m_SecurityWindowCount) > 30 && (tick-lpClient->m_SecurityLastLogTick) > 5000)
		{
			LogAdd(LOG_RED,"[SECURITY][ConnectServer] High packet rate (Index: %d, Ip: %s, Count: %u, Head: 0x%02X, Sub: 0x%02X, Size: %d, OnlineMs: %u)",index,lpClient->m_IpAddr,lpClient->m_SecurityWindowCount,head,sub,size,(tick-lpClient->m_OnlineTime));
			lpClient->m_SecurityLastLogTick = tick;
		}
	}

	void ConnectServerSecurityTrackRequest(int index,bool serverInfo)
	{
		if(CLIENT_RANGE(index) == 0)
		{
			return;
		}

		CClientManager* lpClient = &gClientManager[index];

		if(lpClient->CheckState() == 0)
		{
			return;
		}

		DWORD tick = GetTickCount();

		if(lpClient->m_SecurityRequestWindowTick == 0 || (tick-lpClient->m_SecurityRequestWindowTick) > 10000)
		{
			lpClient->m_SecurityRequestWindowTick = tick;
			lpClient->m_SecurityServerInfoCount = 0;
			lpClient->m_SecurityServerListCount = 0;
		}

		DWORD* lpCounter = ((serverInfo != 0) ? &lpClient->m_SecurityServerInfoCount : &lpClient->m_SecurityServerListCount);
		DWORD limit = ((serverInfo != 0) ? 8 : 12);

		if((++(*lpCounter)) > limit && (tick-lpClient->m_SecurityLastLogTick) > 5000)
		{
			LogAdd(LOG_RED,"[SECURITY][ConnectServer] Repeated %s request burst (Index: %d, Ip: %s, Count: %u, WindowMs: %u)",((serverInfo != 0) ? "server_info" : "server_list"),index,lpClient->m_IpAddr,(*lpCounter),(tick-lpClient->m_SecurityRequestWindowTick));
			lpClient->m_SecurityLastLogTick = tick;
		}
	}

	void ConnectServerSecurityLogUnknown(int index,BYTE head,BYTE sub,int size)
	{
		if(CLIENT_RANGE(index) == 0)
		{
			return;
		}

		CClientManager* lpClient = &gClientManager[index];

		DWORD tick = GetTickCount();

		lpClient->m_SecurityUnknownCount++;

		if((tick-lpClient->m_SecurityLastUnknownLogTick) < 3000)
		{
			return;
		}

		LogAdd(LOG_RED,"[SECURITY][ConnectServer] Unknown packet (Index: %d, Ip: %s, Head: 0x%02X, Sub: 0x%02X, Size: %d, UnknownCount: %u)",index,lpClient->m_IpAddr,head,sub,size,lpClient->m_SecurityUnknownCount);
		lpClient->m_SecurityLastUnknownLogTick = tick;
	}
}

void ConnectServerProtocolCore(int index,BYTE head,BYTE* lpMsg,int size) // OK
{
	PROTECT_START

	gClientManager[index].m_PacketTime = GetTickCount();

	BYTE sub = GetConnectServerPacketSub(lpMsg,size);

	ConnectServerSecurityTrackPacket(index,head,sub,size);

	switch(head)
	{
		case 0xF4:
			switch(lpMsg[3])
			{
				case 0x03:
					ConnectServerSecurityTrackRequest(index,1);
					CCServerInfoRecv((PMSG_SERVER_INFO_RECV*)lpMsg,index);
					break;
				case 0x06:
					ConnectServerSecurityTrackRequest(index,0);
					CCServerListRecv((PMSG_SERVER_LIST_RECV*)lpMsg,index);
					break;
				default:
					ConnectServerSecurityLogUnknown(index,head,sub,size);
					break;
			}
			break;
		default:
			ConnectServerSecurityLogUnknown(index,head,sub,size);
			break;
	}

	PROTECT_FINAL
}

void CCServerInfoRecv(PMSG_SERVER_INFO_RECV* lpMsg,int index) // OK
{
	if(gServerList.CheckJoinServerState() == 0)
	{
		return;
	}

	SERVER_LIST_INFO* lpServerListInfo = gServerList.GetServerListInfo(lpMsg->ServerCode);

	if(lpServerListInfo == 0)
	{
		return;
	}

	if(lpServerListInfo->ServerShow == 0 || lpServerListInfo->ServerState == 0)
	{
		return;
	}

	PMSG_SERVER_INFO_SEND pMsg;

	pMsg.header.set(0xF4,0x03,sizeof(pMsg));

	memcpy(pMsg.ServerAddress,lpServerListInfo->ServerAddress,sizeof(pMsg.ServerAddress));

	pMsg.ServerPort = lpServerListInfo->ServerPort;

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
}

void CCServerListRecv(PMSG_SERVER_LIST_RECV* lpMsg,int index) // OK
{
	BYTE send[2048];

	PMSG_SERVER_LIST_SEND pMsg;

	pMsg.header.set(0xF4,0x06,0);

	int size = sizeof(pMsg);

	int count = gServerList.GenerateServerList(send,&size);

	pMsg.count[0] = SET_NUMBERHB(count);

	pMsg.count[1] = SET_NUMBERLB(count);

	pMsg.header.size[0] = SET_NUMBERHB(size);

	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	gSocketManager.DataSend(index,send,size);
}

void CCServerInitSend(int index,int result) // OK
{
	PMSG_SERVER_INIT_SEND pMsg;

	pMsg.header.set(0x00,sizeof(pMsg));

	pMsg.result = result;

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
}
