#include "stdafx.h"
#include "JoinServerProtocol.h"
#include "..\\..\\Util\\MD5.h"
#include "AccountManager.h"
#include "Log.h"
#include "Protect.h"
#include "QueryManager.h"
#include "ServerManager.h"
#include "SocketManager.h"
#include "Util.h"

namespace
{
	struct JOIN_SECURITY_ACCOUNT_TRACK
	{
		JOIN_SECURITY_ACCOUNT_TRACK()
		{
			this->Reset(0);
		}

		void Reset(DWORD tick)
		{
			this->WindowTick = tick;
			this->FailedCount = 0;
			this->DuplicateCount = 0;
			this->MapMoveFailCount = 0;
			this->RegisterCount = 0;
			this->ResetCount = 0;
			this->LastLogTick = 0;
			memset(this->LastIpAddress,0,sizeof(this->LastIpAddress));
		}

		DWORD WindowTick;
		DWORD FailedCount;
		DWORD DuplicateCount;
		DWORD MapMoveFailCount;
		DWORD RegisterCount;
		DWORD ResetCount;
		DWORD LastLogTick;
		char LastIpAddress[16];
	};

	std::map<std::string,JOIN_SECURITY_ACCOUNT_TRACK> gJoinSecurityAccountTrack;

	char JoinServerSecurityNormalizeChar(char value)
	{
		return (char)CheckAccountCaseSensitive((int)(unsigned char)value);
	}

	BYTE GetJoinServerPacketSub(BYTE* lpMsg,int size)
	{
		if(size < 4)
		{
			return 0xFF;
		}

		return ((lpMsg[0] == 0xC1 || lpMsg[0] == 0xC2) ? lpMsg[3] : 0xFF);
	}

	std::string JoinServerSecurityNormalizeAccount(char* account)
	{
		std::string key(((account == 0) ? "" : account));
		std::transform(key.begin(),key.end(),key.begin(),JoinServerSecurityNormalizeChar);
		return key;
	}

	JOIN_SECURITY_ACCOUNT_TRACK& JoinServerSecurityGetTrack(char* account)
	{
		std::string key = JoinServerSecurityNormalizeAccount(account);
		return gJoinSecurityAccountTrack[key];
	}

	void JoinServerSecurityRefreshTrack(JOIN_SECURITY_ACCOUNT_TRACK& track,DWORD tick,DWORD window)
	{
		if(track.WindowTick == 0 || (tick-track.WindowTick) > window)
		{
			track.Reset(tick);
		}
	}

	void JoinServerSecurityTrackPacket(int index,BYTE head,int size)
	{
		if(SERVER_RANGE(index) == 0)
		{
			return;
		}

		CServerManager* lpServer = &gServerManager[index];

		if(lpServer->CheckState() == 0)
		{
			return;
		}

		DWORD tick = GetTickCount();

		if(lpServer->m_SecurityWindowTick == 0 || (tick-lpServer->m_SecurityWindowTick) > 1000)
		{
			lpServer->m_SecurityWindowTick = tick;
			lpServer->m_SecurityWindowCount = 0;
		}

		if((++lpServer->m_SecurityWindowCount) > 400 && (tick-lpServer->m_SecurityLastLogTick) > 5000)
		{
			gLog.Output(LOG_GENERAL,"[SECURITY][JoinServer] High packet rate (Index: %d, Ip: %s, ServerCode: %d, Count: %u, Head: 0x%02X, Size: %d, OnlineMs: %u)",index,lpServer->m_IpAddr,lpServer->m_ServerCode,lpServer->m_SecurityWindowCount,head,size,(tick-lpServer->m_OnlineTime));
			lpServer->m_SecurityLastLogTick = tick;
		}
	}

	void JoinServerSecurityTrackServerRequest(int index,BYTE head)
	{
		if(SERVER_RANGE(index) == 0)
		{
			return;
		}

		CServerManager* lpServer = &gServerManager[index];

		if(lpServer->CheckState() == 0)
		{
			return;
		}

		DWORD tick = GetTickCount();

		if(lpServer->m_SecurityRequestWindowTick == 0 || (tick-lpServer->m_SecurityRequestWindowTick) > 10000)
		{
			lpServer->m_SecurityRequestWindowTick = tick;
			lpServer->m_SecurityConnectAccountCount = 0;
			lpServer->m_SecurityMapMoveCount = 0;
			lpServer->m_SecurityRegisterCount = 0;
		}

		DWORD* lpCounter = 0;
		DWORD limit = 0;
		const char* label = "request";

		switch(head)
		{
			case 0x01:
				lpCounter = &lpServer->m_SecurityConnectAccountCount;
				limit = 120;
				label = "connect_account";
				break;
			case 0x03:
			case 0x04:
			case 0x10:
				lpCounter = &lpServer->m_SecurityMapMoveCount;
				limit = 250;
				label = "map_move";
				break;
			case 0x56:
				lpCounter = &lpServer->m_SecurityRegisterCount;
				limit = 40;
				label = "account_edit";
				break;
		}

		if(lpCounter == 0)
		{
			return;
		}

		if((++(*lpCounter)) > limit && (tick-lpServer->m_SecurityLastLogTick) > 5000)
		{
			gLog.Output(LOG_GENERAL,"[SECURITY][JoinServer] Repeated %s burst (Index: %d, Ip: %s, ServerCode: %d, Count: %u, WindowMs: %u)",label,index,lpServer->m_IpAddr,lpServer->m_ServerCode,(*lpCounter),(tick-lpServer->m_SecurityRequestWindowTick));
			lpServer->m_SecurityLastLogTick = tick;
		}
	}

	void JoinServerSecurityLogUnknown(int index,BYTE head,BYTE sub,int size)
	{
		if(SERVER_RANGE(index) == 0)
		{
			return;
		}

		CServerManager* lpServer = &gServerManager[index];
		DWORD tick = GetTickCount();

		lpServer->m_SecurityUnknownCount++;

		if((tick-lpServer->m_SecurityLastUnknownLogTick) < 3000)
		{
			return;
		}

		gLog.Output(LOG_GENERAL,"[SECURITY][JoinServer] Unknown packet (Index: %d, Ip: %s, ServerCode: %d, Head: 0x%02X, Sub: 0x%02X, Size: %d, UnknownCount: %u)",index,lpServer->m_IpAddr,lpServer->m_ServerCode,head,sub,size,lpServer->m_SecurityUnknownCount);
		lpServer->m_SecurityLastUnknownLogTick = tick;
	}

	void JoinServerSecurityTrackAccountFailure(int index,char* account,char* ip,const char* reason,bool immediate)
	{
		if(account == 0 || account[0] == 0)
		{
			return;
		}

		DWORD tick = GetTickCount();
		JOIN_SECURITY_ACCOUNT_TRACK& track = JoinServerSecurityGetTrack(account);

		JoinServerSecurityRefreshTrack(track,tick,60000);

		track.FailedCount++;

		if(ip != 0)
		{
			strcpy_s(track.LastIpAddress,ip);
		}

		if(immediate == 0 && track.FailedCount < 3)
		{
			return;
		}

		if((tick-track.LastLogTick) < 2000)
		{
			return;
		}

		gLog.Output(LOG_ACCOUNT,"[SECURITY][JoinServer] Account failure (Account: %s, IpAddress: %s, GameServerCode: %d, Reason: %s, FailCount: %u)",account,((track.LastIpAddress[0] == 0) ? "N/A" : track.LastIpAddress),gServerManager[index].m_ServerCode,reason,track.FailedCount);
		track.LastLogTick = tick;
	}

	void JoinServerSecurityTrackDuplicate(int index,char* account,char* ip,WORD currentServerCode)
	{
		if(account == 0 || account[0] == 0)
		{
			return;
		}

		DWORD tick = GetTickCount();
		JOIN_SECURITY_ACCOUNT_TRACK& track = JoinServerSecurityGetTrack(account);

		JoinServerSecurityRefreshTrack(track,tick,60000);

		track.DuplicateCount++;

		if(ip != 0)
		{
			strcpy_s(track.LastIpAddress,ip);
		}

		if((tick-track.LastLogTick) < 1000)
		{
			return;
		}

		gLog.Output(LOG_ACCOUNT,"[SECURITY][JoinServer] Duplicate account connect (Account: %s, IpAddress: %s, CurrentServerCode: %d, PreviousServerCode: %d, DuplicateCount: %u)",account,((track.LastIpAddress[0] == 0) ? "N/A" : track.LastIpAddress),gServerManager[index].m_ServerCode,currentServerCode,track.DuplicateCount);
		track.LastLogTick = tick;
	}

	void JoinServerSecurityTrackMapMoveFailure(int index,char* account,const char* reason)
	{
		if(account == 0 || account[0] == 0)
		{
			return;
		}

		DWORD tick = GetTickCount();
		JOIN_SECURITY_ACCOUNT_TRACK& track = JoinServerSecurityGetTrack(account);

		JoinServerSecurityRefreshTrack(track,tick,60000);

		track.MapMoveFailCount++;

		if((tick-track.LastLogTick) < 2000 && track.MapMoveFailCount < 3)
		{
			return;
		}

		gLog.Output(LOG_ACCOUNT,"[SECURITY][JoinServer] Map move anomaly (Account: %s, LastIpAddress: %s, GameServerCode: %d, Reason: %s, Count: %u)",account,((track.LastIpAddress[0] == 0) ? "N/A" : track.LastIpAddress),gServerManager[index].m_ServerCode,reason,track.MapMoveFailCount);
		track.LastLogTick = tick;
	}

	void JoinServerSecurityTrackRegistration(int index,char* account,int type,int result)
	{
		if(account == 0 || account[0] == 0)
		{
			return;
		}

		DWORD tick = GetTickCount();
		JOIN_SECURITY_ACCOUNT_TRACK& track = JoinServerSecurityGetTrack(account);

		JoinServerSecurityRefreshTrack(track,tick,300000);

		DWORD* lpCounter = ((type == 0x01) ? &track.RegisterCount : &track.ResetCount);
		const char* label = ((type == 0x01) ? "register" : "reset_password");

		(*lpCounter)++;

		if(result == 1 && (*lpCounter) < 5)
		{
			return;
		}

		if(result != 1 && (*lpCounter) < 3)
		{
			return;
		}

		if((tick-track.LastLogTick) < 2000)
		{
			return;
		}

		gLog.Output(LOG_ACCOUNT,"[SECURITY][JoinServer] Account %s burst (Account: %s, GameServerCode: %d, Result: %d, Count: %u)",label,account,gServerManager[index].m_ServerCode,result,(*lpCounter));
		track.LastLogTick = tick;
	}
}

void JoinServerProtocolCore(int index,BYTE head,BYTE* lpMsg,int size) // OK
{
	PROTECT_START

	gServerManager[index].m_PacketTime = GetTickCount();

	BYTE sub = GetJoinServerPacketSub(lpMsg,size);

	JoinServerSecurityTrackPacket(index,head,size);
	JoinServerSecurityTrackServerRequest(index,head);

	switch(head)
	{
		case 0x00:
			GJServerInfoRecv((SDHP_SERVER_INFO_RECV*)lpMsg,index);
			break;
		case 0x01:
			GJConnectAccountRecv((SDHP_CONNECT_ACCOUNT_RECV*)lpMsg,index);
			break;
		case 0x02:
			GJDisconnectAccountRecv((SDHP_DISCONNECT_ACCOUNT_RECV*)lpMsg,index);
			break;
		case 0x03:
			GJMapServerMoveRecv((SDHP_MAP_SERVER_MOVE_RECV*)lpMsg,index);
			break;
		case 0x04:
			GJMapServerMoveAuthRecv((SDHP_MAP_SERVER_MOVE_AUTH_RECV*)lpMsg,index);
			break;
		case 0x05:
			GJAccountLevelRecv((SDHP_ACCOUNT_LEVEL_RECV*)lpMsg,index);
			break;
		case 0x06:
			GJAccountLevelRecv2((SDHP_ACCOUNT_LEVEL_RECV*)lpMsg,index);
			break;
		case 0x10:
			GJMapServerMoveCancelRecv((SDHP_MAP_SERVER_MOVE_CANCEL_RECV*)lpMsg,index);
			break;
		case 0x11:
			GJAccountLevelSaveRecv((SDHP_ACCOUNT_LEVEL_SAVE_RECV*)lpMsg,index);
			break;
		case 0x12:
			GJAccountLockSaveRecv((SDHP_LOCK_SAVE_RECV*)lpMsg,index);
			break;
		case 0x20:
			GJServerUserInfoRecv((SDHP_SERVER_USER_INFO_RECV*)lpMsg,index);
			break;
		case 0x30:
			GJExternalDisconnectAccountRecv((SDHP_EXTERNAL_DISCONNECT_ACCOUNT_RECV*)lpMsg,index);
			break;
		case 0x56:
			GJRegistroMainRecv((SDHP_REGISTRO_GS_SEND_JS*)lpMsg, index);
			break;
		default:
			JoinServerSecurityLogUnknown(index,head,sub,size);
			break;
	}

	PROTECT_FINAL
}

void GJServerInfoRecv(SDHP_SERVER_INFO_RECV* lpMsg,int index) // OK
{
	SDHP_SERVER_INFO_SEND pMsg;

	pMsg.header.set(0x00,sizeof(pMsg));

	pMsg.result = 1;

	pMsg.ItemCount = 0;

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	gServerManager[index].SetServerInfo(lpMsg->ServerName,lpMsg->ServerPort,lpMsg->ServerCode);
}

void GJConnectAccountRecv(SDHP_CONNECT_ACCOUNT_RECV* lpMsg,int index) // OK
{
	SDHP_CONNECT_ACCOUNT_SEND pMsg;

	pMsg.header.set(0x01,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.result = 1;

	if(CheckTextSyntax(lpMsg->account,sizeof(lpMsg->account)) == 0)
	{
		pMsg.result = 2;
		JoinServerSecurityTrackAccountFailure(index,lpMsg->account,lpMsg->IpAddress,"invalid_account_syntax",1);
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

    if(gAccountManager.GetAccountCount() >= 
#if    PROTECT_STATE
        gJoinServerMaxAccount[gProtect.m_AuthInfo.PackageType][gProtect.m_AuthInfo.PlanType]
#else
        MAX_ACCOUNT
#endif
        )
	//if(gAccountManager.GetAccountCount() >= gJoinServerMaxAccount[gProtect.m_AuthInfo.PackageType][gProtect.m_AuthInfo.PlanType])
	{
		pMsg.result = 4;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(MD5Encryption == 0)
	{
		if(gQueryManager.ExecQuery("SELECT memb__pwd FROM MEMB_INFO WHERE memb___id='%s' COLLATE Latin1_General_BIN",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			pMsg.result = 2;
			JoinServerSecurityTrackAccountFailure(index,lpMsg->account,lpMsg->IpAddress,"account_not_found",0);
			gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		char password[11] = {0};

		gQueryManager.GetAsString("memb__pwd",password,sizeof(password));

		if(strcmp(lpMsg->password,password) != 0 && strcmp(lpMsg->password,GlobalPassword) != 0)
		{
			gQueryManager.Close();
			pMsg.result = 0;
			JoinServerSecurityTrackAccountFailure(index,lpMsg->account,lpMsg->IpAddress,"password_mismatch",0);
			gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		gQueryManager.Close();

		if(strcmp(lpMsg->password,GlobalPassword) == 0)
		{
			LogAdd(LOG_RED,"Account %s logged with global password!",lpMsg->account);
			JoinServerSecurityTrackAccountFailure(index,lpMsg->account,lpMsg->IpAddress,"global_password_used",1);
		}
	}
	else
	{
		if(gQueryManager.ExecQuery("SELECT memb__pwd FROM MEMB_INFO WHERE memb___id='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			pMsg.result = 2;
			JoinServerSecurityTrackAccountFailure(index,lpMsg->account,lpMsg->IpAddress,"account_not_found",0);
			gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		BYTE password[16] = {0};

		gQueryManager.GetAsBinary("memb__pwd",password,sizeof(password));

		MD5 MD5Hash;

		if(MD5Hash.MD5_CheckValue(lpMsg->password,(char*)password,MakeAccountKey(lpMsg->account)) == 0 && strcmp(lpMsg->password,GlobalPassword) != 0)
		{
			gQueryManager.Close();
			pMsg.result = 0;
			JoinServerSecurityTrackAccountFailure(index,lpMsg->account,lpMsg->IpAddress,"password_mismatch",0);
			gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		gQueryManager.Close();

		if(strcmp(lpMsg->password,GlobalPassword) == 0)
		{
			LogAdd(LOG_RED,"Account %s logged with global password!",lpMsg->account);
			JoinServerSecurityTrackAccountFailure(index,lpMsg->account,lpMsg->IpAddress,"global_password_used",1);
		}
	}

	if(gQueryManager.ExecQuery("EXEC WZ_DesblocAccount '%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		pMsg.result = 2;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gQueryManager.ExecQuery("SELECT sno__numb,bloc_code FROM MEMB_INFO WHERE memb___id='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		pMsg.result = 2;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	gQueryManager.GetAsString("sno__numb",pMsg.PersonalCode,sizeof(pMsg.PersonalCode));

	pMsg.BlockCode = (BYTE)gQueryManager.GetAsInteger("bloc_code");

	gQueryManager.Close();

	if(gQueryManager.ExecQuery("EXEC WZ_GetAccountLevel '%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		pMsg.result = 2;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	pMsg.AccountLevel = gQueryManager.GetAsInteger("AccountLevel");

	gQueryManager.GetAsString("AccountExpireDate",pMsg.AccountExpireDate,sizeof(pMsg.AccountExpireDate));

	gQueryManager.Close();

	if(gQueryManager.ExecQuery("SELECT top 1 Lock FROM MEMB_INFO WHERE memb___id='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		pMsg.Lock = 0;
	}
	else
	{
		pMsg.Lock = gQueryManager.GetAsInteger("Lock");
		gQueryManager.Close();
	}

	ACCOUNT_INFO AccountInfo;

	if(gAccountManager.GetAccountInfo(&AccountInfo,lpMsg->account) != 0)
	{
		pMsg.result = 3;
		JoinServerSecurityTrackDuplicate(index,lpMsg->account,lpMsg->IpAddress,AccountInfo.GameServerCode);
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		JGAccountAlreadyConnectedSend(AccountInfo.GameServerCode,AccountInfo.UserIndex,AccountInfo.Account);
		return;
	}

	gQueryManager.ExecQuery("EXEC WZ_CONNECT_MEMB '%s','%s','%s'",lpMsg->account,gServerManager[index].m_ServerName,lpMsg->IpAddress);

	gQueryManager.Close();

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	strcpy_s(AccountInfo.Account,lpMsg->account);

	strcpy_s(AccountInfo.IpAddress,lpMsg->IpAddress);

	AccountInfo.UserIndex = lpMsg->index;
	AccountInfo.GameServerCode = gServerManager[index].m_ServerCode;
	AccountInfo.MapServerMove = 0;
	AccountInfo.MapServerMoveTime = 0;
	AccountInfo.LastServerCode = -1;
	AccountInfo.NextServerCode = -1;
	AccountInfo.Map = 0;
	AccountInfo.X = 0;
	AccountInfo.Y = 0;
	AccountInfo.AuthCode1 = 0;
	AccountInfo.AuthCode2 = 0;
	AccountInfo.AuthCode3 = 0;
	AccountInfo.AuthCode4 = 0;

	gAccountManager.InsertAccountInfo(AccountInfo);

	gLog.Output(LOG_ACCOUNT,"[AccountInfo] Account connected (Account: %s, IpAddress: %s, GameServerCode: %d)",AccountInfo.Account,AccountInfo.IpAddress,AccountInfo.GameServerCode);
}

void GJDisconnectAccountRecv(SDHP_DISCONNECT_ACCOUNT_RECV* lpMsg,int index) // OK
{
	SDHP_DISCONNECT_ACCOUNT_SEND pMsg;

	pMsg.header.set(0x02,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.result = 1;

	ACCOUNT_INFO AccountInfo;

	if(gAccountManager.GetAccountInfo(&AccountInfo,lpMsg->account) == 0)
	{
		pMsg.result = 0;
		JoinServerSecurityTrackMapMoveFailure(index,lpMsg->account,"disconnect_account_missing");
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(AccountInfo.UserIndex != lpMsg->index)
	{
		pMsg.result = 0;
		JoinServerSecurityTrackMapMoveFailure(index,lpMsg->account,"disconnect_index_mismatch");
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(AccountInfo.GameServerCode != gServerManager[index].m_ServerCode)
	{
		pMsg.result = 0;
		JoinServerSecurityTrackMapMoveFailure(index,lpMsg->account,"disconnect_servercode_mismatch");
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(AccountInfo.MapServerMove != 0 && (GetTickCount()-AccountInfo.MapServerMoveTime) < 30000)
	{
		pMsg.result = 0;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	gQueryManager.ExecQuery("EXEC WZ_DISCONNECT_MEMB '%s'",lpMsg->account);

	gQueryManager.Close();

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	gAccountManager.RemoveAccountInfo(AccountInfo);

	gLog.Output(LOG_ACCOUNT,"[AccountInfo] Account disconnected (Account: %s, IpAddress: %s, GameServerCode: %d)",AccountInfo.Account,AccountInfo.IpAddress,AccountInfo.GameServerCode);
}

void GJMapServerMoveRecv(SDHP_MAP_SERVER_MOVE_RECV* lpMsg,int index) // OK
{
	SDHP_MAP_SERVER_MOVE_SEND pMsg;

	pMsg.header.set(0x03,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 1;

	ACCOUNT_INFO AccountInfo;

	if(gAccountManager.GetAccountInfo(&AccountInfo,lpMsg->account) == 0)
	{
		pMsg.result = 0;
		JoinServerSecurityTrackMapMoveFailure(index,lpMsg->account,"map_move_account_missing");
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(AccountInfo.UserIndex != lpMsg->index)
	{
		pMsg.result = 0;
		JoinServerSecurityTrackMapMoveFailure(index,lpMsg->account,"map_move_index_mismatch");
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(AccountInfo.GameServerCode != gServerManager[index].m_ServerCode)
	{
		pMsg.result = 0;
		JoinServerSecurityTrackMapMoveFailure(index,lpMsg->account,"map_move_servercode_mismatch");
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(AccountInfo.MapServerMove != 0)
	{
		pMsg.result = 0;
		JoinServerSecurityTrackMapMoveFailure(index,lpMsg->account,"map_move_reentry");
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	CServerManager* lpServerManager = FindServerByCode(lpMsg->NextServerCode);

	if(lpServerManager == 0)
	{
		pMsg.result = 0;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpServerManager->m_CurUserCount >= lpServerManager->m_MaxUserCount)
	{
		pMsg.result = 0;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	pMsg.GameServerCode = lpMsg->GameServerCode;

	pMsg.NextServerCode = lpMsg->NextServerCode;

	pMsg.map = lpMsg->map;

	pMsg.x = lpMsg->x;

	pMsg.y = lpMsg->y;

	pMsg.AuthCode1 = GetTickCount();

	pMsg.AuthCode2 = GetTickCount()%10000;

	pMsg.AuthCode3 = GetTickCount()%777;

	pMsg.AuthCode4 = GetTickCount()%8911;

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	AccountInfo.MapServerMove = 1;
	AccountInfo.MapServerMoveTime = GetTickCount();
	AccountInfo.LastServerCode = pMsg.GameServerCode;
	AccountInfo.NextServerCode = pMsg.NextServerCode;
	AccountInfo.Map = pMsg.map;
	AccountInfo.X = pMsg.x;
	AccountInfo.Y = pMsg.y;
	AccountInfo.AuthCode1 = pMsg.AuthCode1;
	AccountInfo.AuthCode2 = pMsg.AuthCode2;
	AccountInfo.AuthCode3 = pMsg.AuthCode3;
	AccountInfo.AuthCode4 = pMsg.AuthCode4;

	gAccountManager.InsertAccountInfo(AccountInfo);
}

void GJMapServerMoveAuthRecv(SDHP_MAP_SERVER_MOVE_AUTH_RECV* lpMsg,int index) // OK
{
	SDHP_MAP_SERVER_MOVE_AUTH_SEND pMsg;

	pMsg.header.set(0x04,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.result = 1;

	ACCOUNT_INFO AccountInfo;

	if(gAccountManager.GetAccountInfo(&AccountInfo,lpMsg->account) == 0)
	{
		pMsg.result = 4;
		JoinServerSecurityTrackMapMoveFailure(index,lpMsg->account,"map_move_auth_missing");
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(AccountInfo.MapServerMove == 0)
	{
		pMsg.result = 4;
		JoinServerSecurityTrackMapMoveFailure(index,lpMsg->account,"map_move_auth_without_request");
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpMsg->AuthCode1 != AccountInfo.AuthCode1 || lpMsg->AuthCode2 != AccountInfo.AuthCode2 || lpMsg->AuthCode3 != AccountInfo.AuthCode3 || lpMsg->AuthCode4 != AccountInfo.AuthCode4)
	{
		pMsg.result = 4;
		JoinServerSecurityTrackMapMoveFailure(index,lpMsg->account,"map_move_authcode_mismatch");
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gQueryManager.ExecQuery("SELECT sno__numb,bloc_code FROM MEMB_INFO WHERE memb___id='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		pMsg.result = 2;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	gQueryManager.GetAsString("sno__numb",pMsg.PersonalCode,sizeof(pMsg.PersonalCode));

	pMsg.BlockCode = (BYTE)gQueryManager.GetAsInteger("bloc_code");

	gQueryManager.Close();

	pMsg.LastServerCode = AccountInfo.LastServerCode;

	pMsg.map = AccountInfo.Map;

	pMsg.x = AccountInfo.X;

	pMsg.y = AccountInfo.Y;

	if(gQueryManager.ExecQuery("EXEC WZ_GetAccountLevel '%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		pMsg.result = 2;
		gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	pMsg.AccountLevel = gQueryManager.GetAsInteger("AccountLevel");

	gQueryManager.GetAsString("AccountExpireDate",pMsg.AccountExpireDate,sizeof(pMsg.AccountExpireDate));

	gQueryManager.Close();

	if(gQueryManager.ExecQuery("SELECT top 1 Lock FROM MEMB_INFO WHERE memb___id='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		pMsg.Lock = 0;
	}
	else
	{
		pMsg.Lock = gQueryManager.GetAsInteger("Lock");
		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);

	AccountInfo.UserIndex = lpMsg->index;
	AccountInfo.GameServerCode = gServerManager[index].m_ServerCode;
	AccountInfo.MapServerMove = 0;
	AccountInfo.MapServerMoveTime = 0;
	AccountInfo.LastServerCode = -1;
	AccountInfo.NextServerCode = -1;
	AccountInfo.Map = 0;
	AccountInfo.X = 0;
	AccountInfo.Y = 0;
	AccountInfo.AuthCode1 = 0;
	AccountInfo.AuthCode2 = 0;
	AccountInfo.AuthCode3 = 0;
	AccountInfo.AuthCode4 = 0;

	gAccountManager.InsertAccountInfo(AccountInfo);
}

void GJAccountLevelRecv(SDHP_ACCOUNT_LEVEL_RECV* lpMsg,int index) // OK
{
	ACCOUNT_INFO AccountInfo;

	if(gAccountManager.GetAccountInfo(&AccountInfo,lpMsg->account) == 0)
	{
		return;
	}

	SDHP_ACCOUNT_LEVEL_SEND pMsg;

	pMsg.header.set(0x05,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	if(gQueryManager.ExecQuery("EXEC WZ_GetAccountLevel '%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.AccountLevel = 0;
	}
	else
	{
		pMsg.AccountLevel = gQueryManager.GetAsInteger("AccountLevel");

		gQueryManager.GetAsString("AccountExpireDate",pMsg.AccountExpireDate,sizeof(pMsg.AccountExpireDate));

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
}


void GJAccountLevelRecv2(SDHP_ACCOUNT_LEVEL_RECV* lpMsg,int index) // OK
{

	SDHP_ACCOUNT_LEVEL_SEND pMsg;

	pMsg.header.set(0x06,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	if(gQueryManager.ExecQuery("EXEC WZ_GetAccountLevel '%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.AccountLevel = 0;

		memcpy(pMsg.account,"null",sizeof(pMsg.account));
	}
	else
	{
		pMsg.AccountLevel = gQueryManager.GetAsInteger("AccountLevel");

		gQueryManager.GetAsString("AccountExpireDate",pMsg.AccountExpireDate,sizeof(pMsg.AccountExpireDate));

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.header.size);
}

void GJMapServerMoveCancelRecv(SDHP_MAP_SERVER_MOVE_CANCEL_RECV* lpMsg,int index) // OK
{
	ACCOUNT_INFO AccountInfo;

	if(gAccountManager.GetAccountInfo(&AccountInfo,lpMsg->account) == 0)
	{
		return;
	}

	AccountInfo.MapServerMove = 0;
	AccountInfo.MapServerMoveTime = 0;
	AccountInfo.LastServerCode = -1;
	AccountInfo.NextServerCode = -1;
	AccountInfo.Map = 0;
	AccountInfo.X = 0;
	AccountInfo.Y = 0;
	AccountInfo.AuthCode1 = 0;
	AccountInfo.AuthCode2 = 0;
	AccountInfo.AuthCode3 = 0;
	AccountInfo.AuthCode4 = 0;

	gAccountManager.InsertAccountInfo(AccountInfo);
}

void GJAccountLevelSaveRecv(SDHP_ACCOUNT_LEVEL_SAVE_RECV* lpMsg,int index) // OK
{
	gQueryManager.ExecQuery("EXEC WZ_SetAccountLevel '%s','%d','%d'",lpMsg->account,lpMsg->AccountLevel,lpMsg->AccountExpireTime);
	gQueryManager.Fetch();
	gQueryManager.Close();
}

void GJAccountLockSaveRecv(SDHP_LOCK_SAVE_RECV* lpMsg,int index) // OK
{
	gQueryManager.ExecQuery("UPDATE memb_info SET Lock = %d WHERE memb___id = '%s'",lpMsg->Lock,lpMsg->account);
	gQueryManager.Fetch();
	gQueryManager.Close();
}

void GJServerUserInfoRecv(SDHP_SERVER_USER_INFO_RECV* lpMsg,int index) // OK
{
	gServerManager[index].m_CurUserCount = lpMsg->CurUserCount;

	gServerManager[index].m_MaxUserCount = lpMsg->MaxUserCount;
}

void GJExternalDisconnectAccountRecv(SDHP_EXTERNAL_DISCONNECT_ACCOUNT_RECV* lpMsg,int index) // OK
{
	ACCOUNT_INFO AccountInfo;

	if(gAccountManager.GetAccountInfo(&AccountInfo,lpMsg->account) == 0)
	{
		return;
	}

	JGExternalDisconnectAccountSend(AccountInfo.GameServerCode,AccountInfo.UserIndex,AccountInfo.Account);
}

void JGExternalDisconnectAccountSend(int GameServerCode,int UserIndex,char* account) // OK
{
	CServerManager* lpServerManager = FindServerByCode(GameServerCode);

	if(lpServerManager == 0)
	{
		return;
	}

	SDHP_DISCONNECT_ACCOUNT_SEND pMsg;

	pMsg.header.set(0x02,sizeof(pMsg));

	pMsg.index = UserIndex;

	memcpy(pMsg.account,account,sizeof(pMsg.account));

	pMsg.result = 0;

	gSocketManager.DataSend(lpServerManager->m_index,(BYTE*)&pMsg,pMsg.header.size);
}

void JGAccountAlreadyConnectedSend(int GameServerCode,int UserIndex,char* account) // OK
{
	CServerManager* lpServerManager = FindServerByCode(GameServerCode);

	if(lpServerManager == 0)
	{
		return;
	}

	SDHP_ACCOUNT_ALREADY_CONNECTED_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.index = UserIndex;

	memcpy(pMsg.account,account,sizeof(pMsg.account));

	gSocketManager.DataSend(lpServerManager->m_index,(BYTE*)&pMsg,pMsg.header.size);
}



void GJRegistroMainRecv(SDHP_REGISTRO_GS_SEND_JS* lpMsg, int index) // OK
{
	//=== Send KQ VE GS De Bao Ve Client
	SDHP_SERVER_INFO_SEND pMsg;

	pMsg.header.set(0x56, sizeof(pMsg));

	pMsg.result = 0;

	pMsg.ItemCount = lpMsg->aIndexUser;

	if (CheckTextSyntax(lpMsg->account, sizeof(lpMsg->account)) == 0)
	{
		JoinServerSecurityTrackRegistration(index,lpMsg->account,lpMsg->TypeSend,0);
		gSocketManager.DataSend(index, (BYTE*)& pMsg, pMsg.header.size);
		return;
	}
	//==Get MD5 Hash Pass Web
	MD5 MD5Hash;
	std::string PassMD5 = lpMsg->password;
	if (lpMsg->TypeSend == 0x01) //===Dang Ky Tai Khoan
	{
		if (gQueryManager.ExecQuery("SELECT * FROM MEMB_INFO WHERE memb___id='%s'", lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();


			if (gQueryManager.ExecQuery("INSERT INTO dbo.MEMB_INFO(memb___id, memb__pwd, memb_name, sno__numb, tel__numb, mail_chek, bloc_code, ctl1_code) VALUES ('%s', '%s', '%s', '%s', '%s', 1, 0, 1)",
				lpMsg->account,
				lpMsg->password,
				"JoinServer",
				lpMsg->numcode,
				lpMsg->sodienthoai) == TRUE)
			{
				gQueryManager.Close();
				pMsg.result = 1;
				JoinServerSecurityTrackRegistration(index,lpMsg->account,lpMsg->TypeSend,pMsg.result);
				gLog.Output(LOG_ACCOUNT, "[AccountInfo] Account created (Account: %s, Password: %s )", lpMsg->account, lpMsg->password);
			}
			gQueryManager.Close();
		}
		else {
			pMsg.result = 2;
			JoinServerSecurityTrackRegistration(index,lpMsg->account,lpMsg->TypeSend,pMsg.result);
		}
		gQueryManager.Close();
	}
	else if (lpMsg->TypeSend == 0x0B) //=== Dat Lai Mat Khau
	{
		if (gQueryManager.ExecQuery("SELECT * FROM MEMB_INFO WHERE memb___id='%s' AND  sno__numb ='556114%s' AND phon_numb='%s'", lpMsg->account, lpMsg->numcode, lpMsg->sodienthoai) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			pMsg.result = 12;
			JoinServerSecurityTrackRegistration(index,lpMsg->account,lpMsg->TypeSend,pMsg.result);
		}
		else
		{
			gQueryManager.Close();
			if (gQueryManager.ExecQuery("UPDATE MEMB_INFO set memb__pwd='%s',memb__pwdmd5='%s' where memb___id='%s' AND  sno__numb ='556114%s' AND phon_numb='%s' ", lpMsg->password, strdup(PassMD5.c_str()), lpMsg->account, lpMsg->numcode, lpMsg->sodienthoai) == TRUE && gQueryManager.Fetch() != SQL_NO_DATA)
			{
				gQueryManager.Close();

				pMsg.result = 11;
				JoinServerSecurityTrackRegistration(index,lpMsg->account,lpMsg->TypeSend,pMsg.result);

				gLog.Output(LOG_ACCOUNT, "[DatLaiPass] Set Pass  (Account: %s, Password: %s )", lpMsg->account, lpMsg->password);
			}
			gQueryManager.Close();
		}

	}

	gSocketManager.DataSend(index, (BYTE*)& pMsg, pMsg.header.size);
}
