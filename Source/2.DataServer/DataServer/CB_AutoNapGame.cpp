#include "StdAfx.h"
#include "CB_AutoNapGame.h"
#include "Util.h"
#include "QueryManager.h"
#include "SocketManager.h"
#include "DataServerProtocol.h"
#include "CharacterManager.h"
#include "CashShop.h"
#if(CB_AutoBanking)
#include <WinInet.h>
#pragma comment(lib, "wininet.lib")
DWORD TimeCacheGetAPI;
CBAutoNapGame gCBAutoNapGame;
char* DrawTimeUnix(int timestamp)
{
	time_t rawtime = static_cast<time_t>(timestamp);
	struct tm* timeinfo;
	timeinfo = localtime(&rawtime);
	int year = timeinfo->tm_year + 1900;
	int month = timeinfo->tm_mon + 1;
	int day = timeinfo->tm_mday;
	int hour = timeinfo->tm_hour;
	int minute = timeinfo->tm_min;
	char ztemp[255] = { 0 };
	sprintf_s(ztemp, "%02d:%02d %02d/%02d/%02d", hour, minute, day, month, year % 100);
	return strdup(ztemp);
}
char* NumberFormat(int Number)
{


	if (Number < 0) { return "0"; }

	char OutPut[15];

	if (Number < 1000) {
		sprintf(OutPut, "%d", Number);
		return strdup(OutPut);
	}
	else if (Number < 1000000) {
		int part1 = Number % 1000;
		int part2 = (Number - part1) / 1000;
		sprintf(OutPut, "%d,%03d", part2, part1);
		return strdup(OutPut);
	}
	else if (Number < 1000000000) {
		int part1 = Number % 1000;
		Number = (Number - part1) / 1000;
		int part2 = Number % 1000;
		Number = (Number - part2) / 1000;
		int part3 = Number % 1000;

		sprintf(OutPut, "%d,%03d,%03d", part3, part2, part1);
		return strdup(OutPut);
	}
	else {
		int part1 = Number % 1000;
		Number = (Number - part1) / 1000;
		int part2 = Number % 1000;
		Number = (Number - part2) / 1000;
		int part3 = Number % 1000;
		int part4 = (Number - part3) / 1000;

		sprintf(OutPut, "%d,%03d,%03d,%03d", part4, part3, part2, part1);
		return strdup(OutPut);
	}
	return "0";
}

CBAutoNapGame::CBAutoNapGame()
{

	Init();
	gCBAutoNapGame.GetData.clear();
}


CBAutoNapGame::~CBAutoNapGame()
{
	gCBAutoNapGame.GetData.clear();
}
void SendMessGobal(char* message, ...)
{
	char buff[256] = { 0 };

	va_list arg;
	va_start(arg, message);
	vsprintf_s(buff, message, arg);
	va_end(arg);

	int size = strlen(buff);

	size = ((size > 256) ? 256 : size);

	SDHP_GLOBAL_NOTICE_SEND pMsg;

	pMsg.header.set(0x21, sizeof(pMsg));

	pMsg.MapServerGroup = 0;

	pMsg.type = 0;

	pMsg.count = 0;

	pMsg.opacity = 0;

	pMsg.delay = 0;

	pMsg.color = 0;

	pMsg.speed = 0;

	memcpy(pMsg.message, buff, sizeof(pMsg.message));

	for (int n = 0; n < MAX_SERVER; n++)
	{
		if (gServerManager[n].CheckState() != 0)
		{
			gSocketManager.DataSend(n, (BYTE*)& pMsg, pMsg.header.size);
		}
	}
}
void CBAutoNapGame::Init()
{
	this->Enable = 0;
	ZeroMemory(this->APIAutoCheckBank, sizeof(this->APIAutoCheckBank));
	this->TyleNapTien = 0;
	this->GiaTriNapThapNhat = 0;
	this->EnableThongBao = 0;
	this->HuyThanhToan = 0;
	ZeroMemory(this->MsgThongBao, sizeof(this->MsgThongBao));
	this->StatusGetAPI = false;
	gCBAutoNapGame.mDataGETKQNapTien.clear();
}

void CBAutoNapGame::LoadConfig()
{
	Init();
	char* PathConfig = ".\\ConfigAutoNap.ini";
	this->Enable = GetPrivateProfileInt("AutoNapBank", "Enable", 0, PathConfig);
	this->GetLogUpdateCoin = GetPrivateProfileInt("AutoNapBank", "GetLogUpdateCoin", 1, PathConfig);
	this->TypeDB = GetPrivateProfileInt("AutoNapBank", "TypeDB", 0, PathConfig);
	GetPrivateProfileString("AutoNapBank", "APIAutoCheckBank", "", this->APIAutoCheckBank, sizeof(this->APIAutoCheckBank), PathConfig);
	this->TyleNapTien = GetPrivateProfileInt("AutoNapBank", "TyleNapTien", 0, PathConfig);
	this->GiaTriNapThapNhat = GetPrivateProfileInt("AutoNapBank", "GiaTriNapThapNhat", 0, PathConfig);
	this->HuyThanhToan = GetPrivateProfileInt("AutoNapBank", "HuyThanhToan", 0, PathConfig);
	this->EnableThongBao = GetPrivateProfileInt("AutoNapBank", "EnableThongBao", 0, PathConfig);
	GetPrivateProfileString("AutoNapBank", "MsgThongBao", "", this->MsgThongBao, sizeof(this->MsgThongBao), PathConfig);
	gCBAutoNapGame.SendConfigToGS(-1);
	TimeCacheGetAPI = GetTickCount() + (1000 * 15);
	LogAdd(LOG_BLUE, "[AutoNapGame] Load Config OK (%d)", this->Enable);
}

void CBAutoNapGame::SendConfigToGS(int uIndex)
{
	if (!this->Enable) return;
	RuleAutoNapConfig pMsg;

	pMsg.header.set(0xD3, 0x7A, sizeof(pMsg));
	pMsg.TyleNapTien = this->TyleNapTien;
	pMsg.GiaTriNapThapNhat = this->GiaTriNapThapNhat;
	if (uIndex == -1)
	{
		for (int n = 0; n < MAX_SERVER; n++)
		{
			if (gServerManager[n].CheckState() != 0)
			{
				gSocketManager.DataSend(n, (BYTE*)& pMsg, sizeof(pMsg));
				LogAdd(LOG_BLUE, "[AutoNapGame] Send Data -> GS [%d]", n);
			}
		}
	}
	else
	{
		gSocketManager.DataSend(uIndex, (BYTE*)& pMsg, sizeof(pMsg));
		LogAdd(LOG_BLUE, "[AutoNapGame] Send Data -> GS [%d]", uIndex);
	}
}
void CBAutoNapGame::RecvNapTienBank(BYTE * recv, int gsIndex)//7C
{
	if (!recv) return;
	SEND_RECV_NAPBANK* lpMsg = (SEND_RECV_NAPBANK*)recv;
	int TimeGet = (int)time(0);
	if (this->TypeDB == 1)
	{
		gQueryManager.ExecQuery("INSERT INTO CardPhone(acc, name, card_type, menhgia, card_num, card_num_md5, card_serial, timenap) VALUES('%s', '%s', '%s', '%d', '%s', '%s', '%s', '%d')",
			lpMsg->Account, lpMsg->CharName, "ATM", lpMsg->TienNap, "CardNum", "CardNum", "CardSerial", TimeGet);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.ExecQuery("INSERT INTO DataNapGame (Account, Name, TienNap, Checking) VALUES ('%s', '%s', %d, %d)", lpMsg->Account, lpMsg->CharName, lpMsg->TienNap, TimeGet);
		gQueryManager.Close();
	}
	SendListThanhToan(lpMsg->Account, lpMsg->aIndex, gsIndex);
}
void CBAutoNapGame::SendListThanhToan(char* Account, int aIndex, int gsIndex)
{
	BYTE send[LIMITSIZELIST];

	LISTCOUNDAUTONAP pMsg;

	pMsg.header.set(0xD3, 0x7B, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	pMsg.aIndex = aIndex;
	if (TypeDB == 1)
	{
		if (gQueryManager.ExecQuery("Select Top 8 * From CardPhone Where acc='%s' ORDER BY timenap DESC", Account) != 0)
		{
			while (gQueryManager.Fetch() != SQL_NO_DATA)
			{
				DataLichSuNapThe info;

				//gQueryManager.GetAsString("Checking", info.Checking, sizeof(info.Checking));
				int mChecking = gQueryManager.GetAsInteger("timenap");
				sprintf(info.Checking, "%d", mChecking);
				info.TienNap = gQueryManager.GetAsInteger("menhgia");
				info.Status = gQueryManager.GetAsInteger("status");
				if (info.Status > 0) info.Status -= 1;
				if (size + sizeof(info) > LIMITSIZELIST) break;
				memcpy(&send[size], &info, sizeof(info));
				size += sizeof(info);
				pMsg.count++;
			}
		}
	}
	else
	{
		if (gQueryManager.ExecQuery("Select Top 8 * From DataNapGame Where Account='%s' ORDER BY Checking DESC", Account) != 0)
		{
			while (gQueryManager.Fetch() != SQL_NO_DATA)
			{
				DataLichSuNapThe info;

				//gQueryManager.GetAsString("Checking", info.Checking, sizeof(info.Checking));
				int mChecking = gQueryManager.GetAsInteger("Checking");
				sprintf(info.Checking, "%d", mChecking);
				info.TienNap = gQueryManager.GetAsInteger("TienNap");
				info.Status = gQueryManager.GetAsInteger("Status");
				if (size + sizeof(info) > LIMITSIZELIST) break;
				memcpy(&send[size], &info, sizeof(info));
				size += sizeof(info);
				pMsg.count++;
			}
		}
	}
	gQueryManager.Close();
	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(gsIndex, send, size);

}
void CBAutoNapGame::UserGetLichSuNap(BYTE * recv, int gsIndex)
{
	if (!recv) return;
	SDHP_GSGETDATANAPTHE* lpMsg = (SDHP_GSGETDATANAPTHE*)recv;
	SendListThanhToan(lpMsg->Account, lpMsg->aIndex, gsIndex);
}
int GetServerGSIndex(WORD GameServerCode)
{
	int gsIndex = -1;
	for (int n = 0; n < MAX_SERVER; n++)
	{
		if (gServerManager[n].CheckState() != 0)
		{
			if ((WORD)gServerManager[n].m_ServerCode == (WORD)GameServerCode)
			{
				gsIndex = n;
			}
		}
	}
	return gsIndex;
}
//===


bool StatusGetAPI = false;
char* GetAPIDataString(char* Url, ...)
{
	StatusGetAPI = true;
	char szUrl[1024];

	va_list arg;
	va_start(arg, Url);
	vsprintf_s(szUrl, Url, arg);
	va_end(arg);

	HINTERNET hOpen = NULL;
	HINTERNET hFile = NULL;

	DWORD dataSize = 0;
	DWORD dwBytesRead = 0;
	DWORD dwTimeOut = 2000; // Timeout in milliseconds

	hOpen = InternetOpenA("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/98.0.4758.102 Safari/537.36 Edg/98.0.1108.62", NULL, NULL, NULL, NULL);
	if (!hOpen) return "ERROR";

	hFile = InternetOpenUrlA(hOpen, szUrl, NULL, NULL, INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, NULL);
	bool b = InternetSetOption(hFile, INTERNET_OPTION_RECEIVE_TIMEOUT, &dwTimeOut, sizeof(dwTimeOut));

	if (!hFile || !b) {
		InternetCloseHandle(hOpen);
		StatusGetAPI = false;
		return "ERROR";
	}

	char buffer[2000];
	if (InternetReadFile(hFile, buffer, _countof(buffer), &dwBytesRead)) {
		buffer[dwBytesRead] = '\0';
	}
	else
	{
		StatusGetAPI = false;
		return "ERROR";
	}


	InternetCloseHandle(hFile);
	InternetCloseHandle(hOpen);
	char* ret = buffer;
	StatusGetAPI = false;
	return ret;
}
//==

DWORD WINAPI TCheckKQNapBank(LPVOID lpThreadParameter)
{
	if (gCBAutoNapGame.StatusGetAPI) return 0;
	gCBAutoNapGame.StatusGetAPI = true;

	LogAdd(LOG_BLUE, "[AutoNapGame] Run Checking !!");

	gCBAutoNapGame.mDataGETKQNapTien.clear();
	if (gCBAutoNapGame.TypeDB == 1)
	{
		if (gQueryManager.ExecQuery("Select * From CardPhone Where Status = 0") != 0)
		{
			while (gQueryManager.Fetch() != SQL_NO_DATA)
			{
				DataGETKQNapTien info;
				info.STT = gQueryManager.GetAsInteger("stt");
				gQueryManager.GetAsString("acc", info.Account, sizeof(info.Account));
				gQueryManager.GetAsString("Name", info.Name, sizeof(info.Name));
				info.TienNap = gQueryManager.GetAsInteger("menhgia");
				info.Status = gQueryManager.GetAsInteger("status");
				int mChecking = gQueryManager.GetAsInteger("timenap");
				sprintf(info.Checking, "%d", mChecking);
				gCBAutoNapGame.mDataGETKQNapTien.push_back(info);
			}
		}
		gQueryManager.Close();
	}
	else
	{
		if (gQueryManager.ExecQuery("Select * From DataNapGame Where Status = 0") != 0)
		{
			while (gQueryManager.Fetch() != SQL_NO_DATA)
			{
				DataGETKQNapTien info;
				info.STT = gQueryManager.GetAsInteger("STT");
				gQueryManager.GetAsString("Account", info.Account, sizeof(info.Account));
				gQueryManager.GetAsString("Name", info.Name, sizeof(info.Name));
				info.TienNap = gQueryManager.GetAsInteger("TienNap");
				info.Status = gQueryManager.GetAsInteger("Status");
				int mChecking = gQueryManager.GetAsInteger("Checking");
				sprintf(info.Checking, "%d", mChecking);
				gCBAutoNapGame.mDataGETKQNapTien.push_back(info);
			}
		}
		gQueryManager.Close();
	}


	if (gCBAutoNapGame.mDataGETKQNapTien.empty())
	{
		goto Exit;
		return 0;
	}

	for (int n = 0; n < gCBAutoNapGame.mDataGETKQNapTien.size(); n++)
	{
		//===Sleep
		while (true)
		{
			if (StatusGetAPI == true)
			{
				Sleep(10);
			}
			else
			{
				break;
			}
		}
		//===
		if (gCBAutoNapGame.mDataGETKQNapTien[n].STT == -1 || strlen(gCBAutoNapGame.mDataGETKQNapTien[n].Account) < 1 || strlen(gCBAutoNapGame.mDataGETKQNapTien[n].Name) < 1) continue;
		gCBAutoNapGame.GetData.clear();
		gCBAutoNapGame.GetData = GetAPIDataString("https://script.google.com/macros/s/%s/exec?NameCheck=%s", gCBAutoNapGame.APIAutoCheckBank, gCBAutoNapGame.mDataGETKQNapTien[n].Checking);
		if (gCBAutoNapGame.GetData.empty())
		{
			continue;
		}
		//LogAdd(LOG_RED, "GetData %s", gCBAutoNapGame.GetData.c_str());
		int KQNapThe = atoi(gCBAutoNapGame.GetData.c_str());
		if (KQNapThe < 1)
		{
			if (gCBAutoNapGame.HuyThanhToan != 0)
			{
				int TimeGet = (int)time(0);
				int CheckTimeLenh = atoi(gCBAutoNapGame.mDataGETKQNapTien[n].Checking) + (60 * gCBAutoNapGame.HuyThanhToan);
				if (CheckTimeLenh < TimeGet)
				{
					if (gCBAutoNapGame.TypeDB == 1)
					{
						gQueryManager.ExecQuery("Update CardPhone set status='3',timeduyet='%d' where stt='%d'", (int)(time(0)), gCBAutoNapGame.mDataGETKQNapTien[n].STT);
						gQueryManager.Fetch();
						gQueryManager.Close();
					}
					else
					{
						gQueryManager.ExecQuery("Update DataNapGame set Status='2' where Account='%s' and Name='%s'and STT='%d' and Checking='%s'",
							gCBAutoNapGame.mDataGETKQNapTien[n].Account,
							gCBAutoNapGame.mDataGETKQNapTien[n].Name,
							gCBAutoNapGame.mDataGETKQNapTien[n].STT,
							gCBAutoNapGame.mDataGETKQNapTien[n].Checking);
						gQueryManager.Fetch();
						gQueryManager.Close();
					}
				}
			}
			continue;
		}
		//===Update Lai GIa Tri Nap va ghi nhan kq nap the
		//float TinhToanGiaTriNap = float((KQNapThe * gCBAutoNapGame.TyleNapTien) / 100);
		int TinhToanGiaTriNap = static_cast<int>(KQNapThe * static_cast<double>(gCBAutoNapGame.TyleNapTien) / 100.0);
		if (TinhToanGiaTriNap < 0 || TinhToanGiaTriNap > INT_MAX)
		{
			continue;
		}
		if (gCBAutoNapGame.TypeDB == 1)
		{
			//==Set trang thai the nap sang the dung
			gCBAutoNapGame.mDataGETKQNapTien[n].TienNap = (int)KQNapThe;
			gQueryManager.ExecQuery("Update CardPhone Set status=2,menhgia='%d' Where stt=%d",
				gCBAutoNapGame.mDataGETKQNapTien[n].TienNap,
				gCBAutoNapGame.mDataGETKQNapTien[n].STT);
			gQueryManager.Fetch();
			gQueryManager.Close();
			//===get gcoin truoc khi cong
			gQueryManager.ExecQuery("select gcoin from MEMB_INFO Where memb___id='%s'", gCBAutoNapGame.mDataGETKQNapTien[n].Account);
			gQueryManager.Fetch();
			int CoinTruoc = gQueryManager.GetAsInteger("gcoin");
			gQueryManager.Close();
			//===Cong gcoin
			gQueryManager.ExecQuery("Update MEMB_INFO set gcoin=%d Where memb___id='%s'", (int)(CoinTruoc + TinhToanGiaTriNap), gCBAutoNapGame.mDataGETKQNapTien[n].Account);
			gQueryManager.Fetch();
			gQueryManager.Close();
			if (gCBAutoNapGame.GetLogUpdateCoin)
			{
				LogAdd(LOG_RED, "//=====================================================//");
				LogAdd(LOG_RED, "CoinTruoc = %d, CoinSau = %d", CoinTruoc, (int)(CoinTruoc + TinhToanGiaTriNap));
				LogAdd(LOG_RED, "Update MEMB_INFO set gcoin=%d Where memb___id='%s'", (int)(CoinTruoc + TinhToanGiaTriNap), gCBAutoNapGame.mDataGETKQNapTien[n].Account);
				LogAdd(LOG_RED, "//=====================================================//");
			}

			//===update card
			gQueryManager.ExecQuery("Update CardPhone set addvpoint=1,timeduyet=%d Where stt=%d", (int)(time(0)), gCBAutoNapGame.mDataGETKQNapTien[n].STT);
			gQueryManager.Fetch();
			gQueryManager.Close();
			//==Update Char Tren GS
			CHARACTER_INFO CharacterInfo;
			int CheckAccountOnline = gCharacterManager.GetAccountInfo(&CharacterInfo, gCBAutoNapGame.mDataGETKQNapTien[n].Account);
			if (CheckAccountOnline) //Online
			{
				int mGSIndex = GetServerGSIndex(CharacterInfo.GameServerCode);
				//===Update List thanh toan
				gCBAutoNapGame.SendListThanhToan(gCBAutoNapGame.mDataGETKQNapTien[n].Account, CharacterInfo.UserIndex, mGSIndex);
			}
		}
		else
		{
			gCBAutoNapGame.mDataGETKQNapTien[n].TienNap = (int)KQNapThe;
			gCBAutoNapGame.mDataGETKQNapTien[n].Status = 1;
			gQueryManager.ExecQuery("Update DataNapGame set TienNap='%d',Status='1' where Account='%s' and Name='%s'and STT='%d'",
				gCBAutoNapGame.mDataGETKQNapTien[n].TienNap,
				gCBAutoNapGame.mDataGETKQNapTien[n].Account,
				gCBAutoNapGame.mDataGETKQNapTien[n].Name,
				gCBAutoNapGame.mDataGETKQNapTien[n].STT);
			gQueryManager.Fetch();
			gQueryManager.Close();

			//===Cong WCoin
			gQueryManager.ExecQuery("EXEC WZ_SetCoin '%s','%s','%d','%d','%d'", gCBAutoNapGame.mDataGETKQNapTien[n].Account, gCBAutoNapGame.mDataGETKQNapTien[n].Name, (int)TinhToanGiaTriNap, 0, 0);
			if (gCBAutoNapGame.GetLogUpdateCoin)LogAdd(LOG_RED, "EXEC WZ_SetCoin '%s','%s','%d','%d','%d'", gCBAutoNapGame.mDataGETKQNapTien[n].Account, gCBAutoNapGame.mDataGETKQNapTien[n].Name, (int)TinhToanGiaTriNap, 0, 0);
			gQueryManager.Fetch();
			gQueryManager.Close();

			//==Update Char Tren GS
			CHARACTER_INFO CharacterInfo;
			int CheckAccountOnline = gCharacterManager.GetAccountInfo(&CharacterInfo, gCBAutoNapGame.mDataGETKQNapTien[n].Account);

			if (CheckAccountOnline) //Online
			{
				int mGSIndex = GetServerGSIndex(CharacterInfo.GameServerCode);
				//==Update Coin
				SDHP_CASH_SHOP_POINT_RECV pMsg;

				pMsg.header.set(0x18, 0x00, sizeof(pMsg));

				pMsg.index = CharacterInfo.UserIndex;

				memcpy(pMsg.account, gCBAutoNapGame.mDataGETKQNapTien[n].Account, sizeof(pMsg.account));

				gCashShop.GDCashShopPointRecv(&pMsg, mGSIndex);
				//===Update List thanh toan
				gCBAutoNapGame.SendListThanhToan(gCBAutoNapGame.mDataGETKQNapTien[n].Account, CharacterInfo.UserIndex, mGSIndex);


			}
		}
		//==Thong Bao
		if (gCBAutoNapGame.EnableThongBao)
		{
			SendMessGobal(gCBAutoNapGame.MsgThongBao, gCBAutoNapGame.mDataGETKQNapTien[n].Name, NumberFormat(gCBAutoNapGame.mDataGETKQNapTien[n].TienNap));
		}
		LogAdd(LOG_BLUE, "[AutoNapGame] [%s] da nap thanh cong (%s) Nhan duoc (%s) VND MaNap [%s]", gCBAutoNapGame.mDataGETKQNapTien[n].Name, NumberFormat(gCBAutoNapGame.mDataGETKQNapTien[n].TienNap), NumberFormat(TinhToanGiaTriNap), gCBAutoNapGame.mDataGETKQNapTien[n].Checking);
		Sleep(100);
	}
Exit:
	gCBAutoNapGame.StatusGetAPI = false;
	return 0;

}
void CBAutoNapGame::GetKQNapTien(int Type)
{
	if (!this->Enable) return;
	if (gCBAutoNapGame.StatusGetAPI || (TimeCacheGetAPI > GetTickCount() && Type == 0)) return;

	TimeCacheGetAPI = GetTickCount() + (1000 * 60) * 5;
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TCheckKQNapBank, NULL, 0, NULL);
}
#endif