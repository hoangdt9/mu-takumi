// CTCMini.cpp: Evento CTC Mini by CuongBeo
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DSProtocol.h"
#include "GuildClass.h"
#include "Map.h"
#include "MapServerManager.h"
#include "MemScript.h"
#include "Message.h"
#include "ServerInfo.h"
#include "SkillManager.h"
#include "ScheduleManager.h"
#include "Notice.h"
#include "Util.h"
#include "CommandManager.h"
#include "GameMain.h"
#include "Guild.h"
#include "CTCMini.h"
#include "CashShop.h"
#include "Monster.h"
#include "ItemBagManager.h"
#include "Gate.h"
#if(CTCMINI)
CCTCmini gCTCMini;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCTCmini::CCTCmini() // OK
{

	this->m_State = CTC_STATE_BLANK;
	this->m_RemainTime = 0;
	this->m_TargetTime = 0;
	this->m_TickCount = GetTickCount();
	this->MinutesLeft = -1;

	this->m_RemainTimeWinner = 0;

	this->WinnerNumber = -1;
	this->WinnerTime = 0;
	ZeroMemory(this->WinnerName, sizeof(this->WinnerName));
	ZeroMemory(this->WinnerNameOLD, sizeof(this->WinnerNameOLD));

	for (int n = 0; n < 3; n++)
	{
		IndexCong[n] = -1;
		IndexTru[n] = -1;
	}

	ZeroMemory(&this->b_PhaTru, sizeof(this->b_PhaTru));
	ZeroMemory(&this->b_PhaCong, sizeof(this->b_PhaCong));
	ZeroMemory(&this->b_GuildWin, sizeof(this->b_GuildWin));
	ZeroMemory(&this->GateMove, sizeof(this->GateMove));
	this->m_CTCMinitartTime.clear();
	this->Class = -1;
	this->Map = 0;
	this->X = 0;
	this->Y = 0;
	this->GuildMinPlayer = 0;


	this->Clear();
}


void CCTCmini::Clear()
{
	for (int n = 0; n < MAX_CTC_GUILD; n++)
	{
		this->Guild[n].Reset();
	}

	for (int n = 0; n < MAX_CTC_CHAR; n++)
	{
		this->Char[n].Reset();
	}

	this->m_RemainTimeWinner = 0;

	this->WinnerNumber = -1;


	//SetEntranceZone(); //Set dia hinh
	for (int n = 0; n < 3; n++)
	{
		//gObjDel(IndexCong[n]);
		IndexCong[n] = -1;
		//gObjDel(IndexTru[n]);
		IndexTru[n] = -1;
	}
	this->m_Active = 0;

}
CCTCmini::~CCTCmini() // OK
{

}

void CCTCmini::Init() // OK
{
	if (this->m_Enabled == 0)
	{
		this->SetState(CTC_STATE_BLANK);
	}
	else
	{
		this->SetState(CTC_STATE_EMPTY);
	}
}
void CCTCmini::Load(char* path) // OK
{
	this->m_CTCMinitartTime.clear();
	ZeroMemory(this->WinnerNameOLD, sizeof(this->WinnerNameOLD));
	this->Clear();

	this->m_Enabled = 0;
	this->m_EventTime = 0;
	this->m_WarningTime = 0;

	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(path);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", path, res.description());
		return;
	}

	//--
	pugi::xml_node oCTCMini = file.child("CTCMini");
	//--
	pugi::xml_node eCTCMini = oCTCMini.child("EventTime");
	for (pugi::xml_node rCTCMini = eCTCMini.child("Time"); rCTCMini; rCTCMini = rCTCMini.next_sibling()) {
		CTC_START_TIME info;
		info.Year = rCTCMini.attribute("Year").as_int();
		info.Month = rCTCMini.attribute("Month").as_int();
		info.Day = rCTCMini.attribute("Day").as_int();
		info.DayOfWeek = rCTCMini.attribute("DayOfWeek").as_int();
		info.Hour = rCTCMini.attribute("Hour").as_int();
		info.Minute = rCTCMini.attribute("Minute").as_int();
		info.Second = rCTCMini.attribute("Second").as_int();
		this->m_CTCMinitartTime.push_back(info);
	}
	//--
	pugi::xml_node eNPCTalk = oCTCMini.child("NPCTalk");
	for (pugi::xml_node rNPCTalk = eNPCTalk.child("NPC"); rNPCTalk; rNPCTalk = rNPCTalk.next_sibling()) {
		this->Class = rNPCTalk.attribute("Class").as_int();
		this->Map = rNPCTalk.attribute("Map").as_int();
		this->X = rNPCTalk.attribute("X").as_int();
		this->Y = rNPCTalk.attribute("Y").as_int();
	}
	//--
	pugi::xml_node eCauHinhTime = oCTCMini.child("CauHinhTime");
	for (pugi::xml_node rCTCConfig = eCauHinhTime.child("Config"); rCTCConfig; rCTCConfig = rCTCConfig.next_sibling())
	{
		this->m_Enabled = rCTCConfig.attribute("Enabled").as_int();
		this->m_WarningTime = rCTCConfig.attribute("TimeChuanBi").as_int();
		this->m_EventTime = rCTCConfig.attribute("TimeSuKien").as_int();
		this->WinnerTime = rCTCConfig.attribute("SoTruCanChiem").as_int();
		this->GuildMinPlayer = rCTCConfig.attribute("GuildMinPlayer").as_int();
		//LogAdd(LOG_EVENT,"Debug  m_Enabled %d m_WarningTime %d m_EventTime %d WinnerTime %d", this->m_Enabled,this->m_WarningTime,this->m_EventTime,this->WinnerTime);
	}
	//====
	pugi::xml_node eCauHinhGate = oCTCMini.child("CauHinhGate");
	for (pugi::xml_node rGateConfig = eCauHinhGate.child("Config"); rGateConfig; rGateConfig = rGateConfig.next_sibling())
	{
		this->GateMove[0] = rGateConfig.attribute("Gate1").as_int();
		this->GateMove[1] = rGateConfig.attribute("Gate2").as_int();
		this->GateMove[2] = rGateConfig.attribute("Gate3").as_int();
		this->GateMove[3] = rGateConfig.attribute("Gate4").as_int();
		this->GateMove[4] = rGateConfig.attribute("Gate5").as_int();
		//LogAdd(LOG_EVENT,"Debug  Gate1 %d, Gate2 %d, Gate3 %d, Gate4 %d, Gate5 %d,",this->GateMove[0],this->GateMove[1],this->GateMove[2],this->GateMove[3],this->GateMove[4]);
	}
	//--
	pugi::xml_node ePhaCong = oCTCMini.child("PhaCong");
	for (pugi::xml_node rPhaCong = ePhaCong.child("Config"); rPhaCong; rPhaCong = rPhaCong.next_sibling()) {

		b_PhaCong.WCoin = rPhaCong.attribute("WCoin").as_int();
		b_PhaCong.WCoinP = rPhaCong.attribute("WCoinP").as_int();
		b_PhaCong.GobinP = rPhaCong.attribute("GobinP").as_int();
		b_PhaCong.Ruud = rPhaCong.attribute("Ruud").as_int();
		b_PhaCong.IndexBonus = rPhaCong.attribute("BonusAllGuild").as_int();
	}
	//===
	pugi::xml_node ePhaTru = oCTCMini.child("PhaTru");
	for (pugi::xml_node rPhaTru = ePhaTru.child("Config"); rPhaTru; rPhaTru = rPhaTru.next_sibling()) {

		b_PhaTru.WCoin = rPhaTru.attribute("WCoin").as_int();
		b_PhaTru.WCoinP = rPhaTru.attribute("WCoinP").as_int();
		b_PhaTru.GobinP = rPhaTru.attribute("GobinP").as_int();
		b_PhaTru.Ruud = rPhaTru.attribute("Ruud").as_int();
		b_PhaTru.IndexBonus = rPhaTru.attribute("BonusAllGuild").as_int();
	}
	//===
	pugi::xml_node eGuidWin = oCTCMini.child("GuidWin");
	for (pugi::xml_node rGuildWin = eGuidWin.child("Config"); rGuildWin; rGuildWin = rGuildWin.next_sibling()) {

		b_GuildWin.WCoin = rGuildWin.attribute("WCoin").as_int();
		b_GuildWin.WCoinP = rGuildWin.attribute("WCoinP").as_int();
		b_GuildWin.GobinP = rGuildWin.attribute("GobinP").as_int();
		b_GuildWin.Ruud = rGuildWin.attribute("Ruud").as_int();
		b_GuildWin.IndexBonus = rGuildWin.attribute("IndexItemBag").as_int();
	}
	//==============================
	GetPrivateProfileString("GuildWinOLD", "GuildName", "", this->WinnerNameOLD, sizeof(this->WinnerNameOLD), "..\\Data\\Event\\CTCMini\\GuildWin.ini");
	//LogAdd(LOG_EVENT,"Debug  GuildWinOLD %s",this->WinnerNameOLD);
}

void CCTCmini::MainProc() // OK
{
	DWORD elapsed = GetTickCount() - this->m_TickCount;

	if (elapsed < 1000)
	{
		return;
	}

	this->m_TickCount = GetTickCount();

	this->m_RemainTime = (int)difftime(this->m_TargetTime, time(0));


	if (this->m_Enabled == 0)
	{
		if (gServerDisplayer.EventCTCMini != -1)
		{
			gServerDisplayer.EventCTCMini = -1;
		}
	}
	else
	{
		if (this->m_State == CTC_STATE_EMPTY)
		{
			gServerDisplayer.EventCTCMini = this->m_RemainTime;
		}
		else
		{
			if (gServerDisplayer.EventCTCMini != 0)
			{
				gServerDisplayer.EventCTCMini = 0;
			}
		}
	}

	//LogAdd(LOG_EVENT,"Debug  MainProc %d (State %d)",this->m_RemainTime,this->m_State);

	switch (this->m_State)
	{
	case CTC_STATE_BLANK:
		this->ProcState_BLANK();
		break;
	case CTC_STATE_EMPTY:
		this->ProcState_EMPTY();
		break;
	case CTC_STATE_START:
		this->ProcState_START();
		break;
	}

	//==Tien hanh tong ket va trao thuong
	if (this->m_RemainTimeWinner > 0)
	{
		if (this->WinnerNumber >= 0)
		{
			if (this->m_RemainTimeWinner == 40)
			{
				GCServerMsgStringSendToMap(MapCTC, 0, "[Công Thành chiến Mini] Chuẩn bị tiến hành trao thưởng!");
			}


			if (this->m_RemainTimeWinner == 15) //Trao thuong cho Guild
			{
				GUILD_INFO_STRUCT* lpGuildInfo = gGuildClass.SearchGuild_Number(this->WinnerNumber);

				if (lpGuildInfo == 0)
				{

				}
				else
				{
					LogAdd(LOG_EVENT, "[CTC Mini] Guild Winner: %s", lpGuildInfo->Name);
					gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "[Công Thành chiến Mini] Guild [%s] đã chiến thắng!", lpGuildInfo->Name);
					gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "Các thành viên tham gia sẽ được thưởng %d WC, %d WP, %d GP, %d Ruud", b_GuildWin.WCoin, b_GuildWin.WCoinP, b_GuildWin.GobinP, b_GuildWin.Ruud);

					for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
					{
						if (gObjIsConnectedGP(n) != 0 && gObj[n].Type == OBJECT_USER)
						{
							if (strcmp(gObj[n].GuildName, lpGuildInfo->Name) == 0)
							{
								LogAdd(LOG_EVENT, "[CTC Mini] Bonus Guild Member Reward: %s", gObj[n].Name);
								gCashShop.GDCashShopAddPointSaveSend(gObj[n].Index, 0, b_GuildWin.WCoin, b_GuildWin.WCoinP, b_GuildWin.GobinP, b_GuildWin.Ruud);
								gCashShop.CGCashShopPointRecv(gObj[n].Index);

								if (b_GuildWin.IndexBonus > 0)
								{
									gItemBagManager.DropItemByIndexItemBag(b_GuildWin.IndexBonus, &gObj[n]); //Gui Item cho NV
								}
							}
						}
					}

				}

			}
			this->m_RemainTimeWinner--;

			if (this->m_RemainTimeWinner <= 0)
			{
				GCServerMsgStringSendToMap(MapCTC, 0, "[Công Thành chiến Mini] Đã kết thúc !");
				LogAdd(LOG_EVENT, "[CTC Mini] Finish 1");
				for (int n = 0; n < MAX_CTC_CHAR; n++)
				{
					if (this->Char[n].Index == -1)
					{
						continue;
					}
					gObjMoveGate(this->Char[n].Index, 17); //Move tat ca cac thanh nien ve 
				}
				this->Clear();
			}

		}
		else
		{
			LogAdd(LOG_EVENT, "[CTC Mini] Finish 2");
			GCServerMsgStringSendToMap(MapCTC, 0, "[Công Thành chiến Mini] Đã kết thúc không có Guild chiến thắng !");
			this->m_RemainTimeWinner = 0;
			this->Clear();
		}


	}
}

void CCTCmini::ProcState_BLANK() // OK
{

}

void CCTCmini::ProcState_EMPTY() // OK
{

	if (this->m_RemainTime > 0 && this->m_RemainTime <= (this->m_WarningTime * 60))
	{
		int minutes = this->m_RemainTime / 60;

		if ((this->m_RemainTime % 60) == 0)
		{
			minutes--;
		}

		if (this->MinutesLeft != minutes)
		{
			this->MinutesLeft = minutes;

			gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "[Công Thành chiến Mini] Bắt đầu sau %d phút !", (MinutesLeft + 1));
		}
		if (this->m_Active == 0) { this->m_Active = 1; }
	}

	if (this->m_RemainTime > 0 && this->m_RemainTime <= 5)
	{
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "[Công Thành chiến Mini] Bắt đầu sau %d giây !", m_RemainTime);
	}

	if (this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "[Công Thành chiến Mini] Sự kiện bắt đầu !"); //Bat Dau Su Kien

		this->SetState(CTC_STATE_START);
	}
}

void CCTCmini::ProcState_START() // OK
{

	if (this->m_RemainTime > 0 && this->m_RemainTime <= 300)
	{
		int minutes = this->m_RemainTime / 60;

		if (this->MinutesLeft != minutes)
		{
			this->MinutesLeft = minutes;

			gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "[Công Thành chiến Mini] Sẽ kết thúc sau %d phút !", (MinutesLeft + 1));
		}
	}

	if (this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "[Công Thành chiến Mini] Đã kết thúc !");
		this->SetGuildTop(); //Set Guild Chien Thang
		this->m_RemainTimeWinner = 40; //Thoi Gian dem nguoc trao thuong
		this->m_Active = 0;
		this->SetState(CTC_STATE_EMPTY);
		SetEntranceZone(); //Set dia hinh
		for (int n = 0; n < 3; n++)
		{
			if (gObj[IndexCong[n]].Live) {
				gObjDel(IndexCong[n]);
			}
			if (gObj[IndexTru[n]].Live) {
				gObjDel(IndexTru[n]);
			}
		}
	}
	//Check Vi Tri Tru
	for (int ttb = 0; ttb < 3; ttb++) //Them Cong
	{
		if (IndexTru[ttb] != -1 && gObj[IndexTru[ttb]].Live)
		{
			int px = ThemTru[ttb][0];
			int py = ThemTru[ttb][1];
			if (gObj[IndexTru[ttb]].X != px || gObj[IndexTru[ttb]].Y != py)
			{
				gObj[IndexTru[ttb]].X = px;
				gObj[IndexTru[ttb]].Y = py;
			}

		}
	}
	//=======
}

void CCTCmini::SetState(int state) // OK
{
	this->m_State = state;

	switch (this->m_State)
	{
	case CTC_STATE_BLANK:
		this->SetState_BLANK();
		break;
	case CTC_STATE_EMPTY:
		this->SetState_EMPTY();
		break;
	case CTC_STATE_START:
		this->SetState_START();
		break;
	}
}

void CCTCmini::SetState_BLANK() // OK
{

}

void CCTCmini::SetState_EMPTY() // OK
{
	this->CheckSync();
}

void CCTCmini::SetState_START() // OK
{
	this->m_Active = 2;

	this->m_RemainTime = this->m_EventTime * 60;

	this->m_TargetTime = (int)(time(0) + this->m_RemainTime);

	this->TestCTCMini(); //Khoi dong event

	this->SendKillCTCMini(0, 0);

	LogAdd(LOG_EVENT, "[CTC Mini] Bắt Đầu Event Công Thành Chiến Mini");

}

void CCTCmini::CheckSync() // OK
{

	if (this->m_CTCMinitartTime.empty() != 0)
	{
		this->SetState(CTC_STATE_BLANK);
		return;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for (std::vector<CTC_START_TIME>::iterator it = this->m_CTCMinitartTime.begin(); it != this->m_CTCMinitartTime.end(); it++)
	{
		ScheduleManager.AddSchedule(it->Year, it->Month, it->Day, it->Hour, it->Minute, it->Second, it->DayOfWeek);
	}

	if (ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		this->SetState(CTC_STATE_BLANK);
		return;
	}

	this->m_RemainTime = (int)difftime(ScheduleTime.GetTime(), time(0));

	this->m_TargetTime = (int)ScheduleTime.GetTime();

}
void CCTCmini::CGPacketCTCMini(CTCMINI_CGPACKET* aRecv, int aIndex)
{
	if (!this->m_Enabled)
	{
		return;
	}
	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}
	LPOBJ lpObj = &gObj[aIndex];

	if ((GetTickCount() - lpObj->ClickClientSend) < 2000) {
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "Bạn thao tác quá nhanh vui lòng chờ đợi !");
		return;
	}

	GUILD_INFO_STRUCT* lpGuildInfo = lpObj->Guild;

	if (this->m_Active == 0 || this->m_Active == 2) //Verifica se o evento está em andamento
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "Chưa đến thời gian sự kiện");
		return;
	}

	if (lpGuildInfo == NULL)
	{

		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "[Công Thành chiến Mini] Không thấy thông tin Guild của bạn");
		return;
	}

	if (lpObj->GuildNumber == 0) //Verifica se o personagem faz parte de uma guild
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "[Công Thành chiến Mini] Bạn chưa vào Guild nên không thể tham gia !");
		return;
	}

	if (lpGuildInfo->TotalCount < this->GuildMinPlayer) //So Thanh Vien Toi Thieu Tham Gia
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "[Công Thành chiến Mini] Guild bạn không đủ người than gia !");
		return;
	}

	int CongVao = aRecv->CongVao;
	this->SetChar(lpObj->Index, CongVao);
	gObjMoveGate(lpObj->Index, this->GateMove[CongVao]);
	this->SendKillCTCMini(lpObj->Index, 1);
	gEffectManager.ClearAllEffect(lpObj); //Xoá Buff

}

void CCTCmini::SendKillCTCMini(int aIndex, int Type) //Gui thong tin ve CLient
{
	CTCMINIKILL_SENDCLIENT pMsg;
	pMsg.header.set(0xF3, 0x33, sizeof(pMsg));
	if (Type == 1) {
		//=== Send 1 Client===//
		if (gObjIsConnectedGP(aIndex) == 0)
		{
			return;
		}

		LPOBJ lpObj = &gObj[aIndex];
		pMsg.TimeCTCMini = this->m_RemainTime;
		//==============
		memset(&pMsg.GNameTru1, 0, sizeof(pMsg.GNameTru1));
		memset(&pMsg.GNameTru2, 0, sizeof(pMsg.GNameTru2));
		memset(&pMsg.GNameTru3, 0, sizeof(pMsg.GNameTru3));
		//==============
		for (int b = 0; b < 3; b++) //Them Cong
		{
			if (IndexTru[b] != -1 && gObj[IndexTru[b]].GuildNumber == lpObj->GuildNumber) {
				pMsg.Tru[b] = 1;
			}
			else if (IndexTru[b] != -1 && gObj[IndexTru[b]].GuildNumber != 0 && gObj[IndexTru[b]].GuildNumber != lpObj->GuildNumber) {
				pMsg.Tru[b] = 2;
			}
			else {
				pMsg.Tru[b] = 0;
			}
			//=======================
			if (IndexCong[b] != -1 && !gObj[IndexCong[b]].Live) {
				pMsg.Cong[b] = 1;
			}
			else {
				pMsg.Cong[b] = 0;
			}
		}
		if (strlen(gObj[IndexTru[0]].GuildName) > 1) {
			memcpy(pMsg.GNameTru1, gObj[IndexTru[0]].GuildName, 9); //MAX_GUILD_NAME define
		}
		if (strlen(gObj[IndexTru[1]].GuildName) > 1) {
			memcpy(pMsg.GNameTru2, gObj[IndexTru[1]].GuildName, 9); //MAX_GUILD_NAME define
		}
		if (strlen(gObj[IndexTru[2]].GuildName) > 1) {
			memcpy(pMsg.GNameTru3, gObj[IndexTru[2]].GuildName, 9); //MAX_GUILD_NAME define
		}
		DataSend(lpObj->Index, (BYTE*)&pMsg, pMsg.header.size);
	}
	else {
		//=== Send All ==//
		for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
		{
			if (gObjIsConnectedGP(n) != 0 && gObj[n].Map == MapCTC)
			{
				pMsg.TimeCTCMini = this->m_RemainTime;
				memset(&pMsg.GNameTru1, 0, sizeof(pMsg.GNameTru1));
				memset(&pMsg.GNameTru2, 0, sizeof(pMsg.GNameTru2));
				memset(&pMsg.GNameTru3, 0, sizeof(pMsg.GNameTru3));
				for (int b = 0; b < 3; b++) //Them Cong
				{
					//========================
					if (IndexTru[b] != -1 && gObj[IndexTru[b]].GuildNumber != 0 && gObj[IndexTru[b]].GuildNumber == gObj[n].GuildNumber) {
						pMsg.Tru[b] = 1;
					}
					else if (IndexTru[b] != -1 && gObj[IndexTru[b]].GuildNumber != 0 && gObj[IndexTru[b]].GuildNumber != gObj[n].GuildNumber) {
						pMsg.Tru[b] = 2;
					}
					else {
						pMsg.Tru[b] = 0;
					}
					//=======================
					if (IndexCong[b] != -1 && !gObj[IndexCong[b]].Live) {
						pMsg.Cong[b] = 1;
					}
					else {
						pMsg.Cong[b] = 0;
					}
				}
				if (strlen(gObj[IndexTru[0]].GuildName) > 1) {
					memcpy(pMsg.GNameTru1, gObj[IndexTru[0]].GuildName, 9); //MAX_GUILD_NAME define
				}
				if (strlen(gObj[IndexTru[1]].GuildName) > 1) {
					memcpy(pMsg.GNameTru2, gObj[IndexTru[1]].GuildName, 9); //MAX_GUILD_NAME define
				}
				if (strlen(gObj[IndexTru[2]].GuildName) > 1) {
					memcpy(pMsg.GNameTru3, gObj[IndexTru[2]].GuildName, 9); //MAX_GUILD_NAME define
				}
				DataSend(n, (BYTE*)&pMsg, pMsg.header.size);
			}
		}
	}
}

void SendInfoCTCMini(int aIndex)
{
	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	GUILD_INFO_STRUCT* lpGuildInfo = gGuildClass.SearchGuild(gCTCMini.WinnerNameOLD);


	INFOCTCMINI_SENDCLIENT pMsg;
	pMsg.header.set(0xF3, 0x32, sizeof(pMsg));
	pMsg.OpenWindow = 1;
	pMsg.GuildWinOld = 0;

	if (lpGuildInfo != 0)
	{
		if (lpGuildInfo->Number == lpObj->GuildNumber) {
			pMsg.GuildWinOld = 1;
		}
	}
	DataSend(lpObj->Index, (BYTE*)&pMsg, pMsg.header.size);
}

bool CCTCmini::Dialog(int aIndex, int aNpcIndex) //Click NPC Vao CTC
{
	if (!this->m_Enabled)
	{
		return false;
	}
	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return false;
	}
	LPOBJ lpUser = &gObj[aIndex];
	LPOBJ lpNpc = &gObj[aNpcIndex];
	if (lpNpc->Class == this->Class
		&& lpNpc->Map == this->Map
		&& lpNpc->X == this->X
		&& lpNpc->Y == this->Y)
	{
		//=== Gui Info ve CLient
		SendInfoCTCMini(aIndex);
		return true;
	}
	// ----
	return false;
}



void CCTCmini::AddGuild(int gIndex) // Set Guild tham gia Su Kien
{

	LPOBJ lpObj = &gObj[gIndex];
	for (int n = 0; n < MAX_CTC_GUILD; n++)
	{
		if (this->Guild[n].Number >= 0)
		{
			continue;
		}

		this->Guild[n].Number = lpObj->GuildNumber;
		this->Guild[n].Owner = 0;
		memcpy(this->Guild[n].GuildOwner, lpObj->GuildName, sizeof(this->Guild[n].GuildOwner));
		return;
	}
}

bool CCTCmini::GetGuild(int gIndex) // OK
{
	for (int n = 0; n < MAX_CTC_GUILD; n++)
	{
		if (this->Guild[n].Number == gIndex)
		{
			return 1;
		}
	}
	return 0;
}

void CCTCmini::SetGuildTop() // Set Guild Chien Thang
{
	for (int n = 0; n < MAX_CTC_GUILD; n++)
	{
		if (this->Guild[n].Owner == 1)
		{
			this->WinnerNumber = this->Guild[n].Number;
			memcpy(this->WinnerName, this->Guild[n].GuildOwner, sizeof(this->WinnerName));
			WritePrivateProfileStringA("GuildWinOLD", "GuildName", this->WinnerName, "..\\Data\\Event\\CTCMini\\GuildWin.ini");
		}
	}
}


void CCTCmini::SetChar(int cIndex, int CongVao) // OK
{

	if (this->GetChar(cIndex) == 0)
	{
		this->AddChar(cIndex, CongVao);
	}

}

void CCTCmini::AddChar(int cIndex, int CongVao) // OK
{
	for (int n = 0; n < MAX_CTC_CHAR; n++)
	{
		if (this->Char[n].Index >= 0)
		{
			continue;
		}

		this->Char[n].Index = cIndex;
		memcpy(this->Char[n].Name, gObj[cIndex].Name, sizeof(gObj[cIndex].Name));
		this->Char[n].CongVao = CongVao;
		LogAdd(LOG_RED, "Nhân Vật [%s] Vào Cổng [%d]", this->Char[n].Name, CongVao);
		return;
	}
}

int CCTCmini::GetCongVao(int cIndex) // OK
{
	for (int n = 0; n < MAX_CTC_CHAR; n++)
	{
		if (this->Char[n].Index == cIndex)
		{
			return this->Char[n].CongVao;
		}
	}
	return 0;
}

bool CCTCmini::GetChar(int cIndex) // OK
{
	for (int n = 0; n < MAX_CTC_CHAR; n++)
	{
		if (this->Char[n].Index == cIndex)
		{
			return 1;
		}
	}
	return 0;
}

void CCTCmini::CheckChar() // OK
{
	for (int n = 0; n < MAX_CTC_CHAR; n++)
	{
		if (OBJECT_RANGE(this->Char[n].Index) == 0)
		{
			continue;
		}
		if (strcmp(this->Char[n].Name, gObj[this->Char[n].Index].Name) == 0)
		{
			this->Char[n].Reset();
			return;
		}
	}
	return;
}


//=================================================================================================================

bool CCTCmini::CheckPlayerTarget(LPOBJ lpObj, LPOBJ lpTarget, int Type) // OK
{
	if (Type == 1) //Attack
	{
		if (lpObj->Map == MapCTC)
		{
			if (lpObj->GuildNumber == 0 && lpObj->Type == OBJECT_USER)
			{
				gObjMoveGate(lpObj->Index, 17); //Move tat ca cac thanh nien ve 
				return 1;
			}
			if (lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_MONSTER)
			{
				if (lpObj->GuildNumber == lpTarget->GuildNumber && lpObj->GuildNumber != 0)
				{
					return 1;
				}
			}
		}
	}
	else //View PK
	{
		if (lpObj->Map == MapCTC)
		{
			if (lpTarget->GuildNumber != lpObj->GuildNumber)
			{
				return 1;
			}
		}
	}
	return 0;
}
void CCTCmini::SetEntranceZone() // OK
{
#if(GAMESERVER_UPDATE>=501)

	for (int x = XoaDuongVao[0][0]; x <= XoaDuongVao[0][2]; x++)
	{
		for (int y = XoaDuongVao[0][1]; y <= XoaDuongVao[0][3]; y++)
		{
			gMap[MapCTC].SetAttr(x, y, 16);
		}
	}

	for (int x = XoaDuongVao[1][0]; x <= XoaDuongVao[1][2]; x++)
	{
		for (int y = XoaDuongVao[1][1]; y <= XoaDuongVao[1][3]; y++)
		{
			gMap[MapCTC].SetAttr(x, y, 16);
		}
	}

	for (int x = XoaDuongVao[2][0]; x <= XoaDuongVao[2][2]; x++)
	{
		for (int y = XoaDuongVao[2][1]; y <= XoaDuongVao[2][3]; y++)
		{
			gMap[MapCTC].SetAttr(x, y, 16);
		}
	}

	for (int x = XoaDuongVao[3][0]; x <= XoaDuongVao[3][2]; x++)
	{
		for (int y = XoaDuongVao[3][1]; y <= XoaDuongVao[3][3]; y++)
		{
			gMap[MapCTC].SetAttr(x, y, 16);
		}
	}

	for (int x = XoaDuongVao[4][0]; x <= XoaDuongVao[4][2]; x++)
	{
		for (int y = XoaDuongVao[4][1]; y <= XoaDuongVao[4][3]; y++)
		{
			gMap[MapCTC].SetAttr(x, y, 16);
		}
	}
	//===========Xoa Tru
	for (int x = XoaTru[0][0]; x <= XoaTru[0][2]; x++)
	{
		for (int y = XoaTru[0][1]; y <= XoaTru[4][3]; y++)
		{
			gMap[MapCTC].SetAttr(x, y, 6);
		}
	}
	for (int x = XoaTru[1][0]; x <= XoaTru[1][2]; x++)
	{
		for (int y = XoaTru[1][1]; y <= XoaTru[1][3]; y++)
		{
			gMap[MapCTC].SetAttr(x, y, 6);
		}
	}
	for (int x = XoaTru[2][0]; x <= XoaTru[2][2]; x++)
	{
		for (int y = XoaTru[2][1]; y <= XoaTru[2][3]; y++)
		{
			gMap[MapCTC].SetAttr(x, y, 6);
		}
	}
	//================Xoa Cong
	for (int x = XoaCong[0][0]; x <= XoaCong[0][2]; x++)
	{
		for (int y = XoaCong[0][1]; y <= XoaCong[0][3]; y++)
		{
			gMap[MapCTC].SetAttr(x, y, 16);
		}
	}
	for (int x = XoaCong[1][0]; x <= XoaCong[1][2]; x++)
	{
		for (int y = XoaCong[1][1]; y <= XoaCong[1][3]; y++)
		{
			gMap[MapCTC].SetAttr(x, y, 16);
		}
	}
	for (int x = XoaCong[2][0]; x <= XoaCong[2][2]; x++)
	{
		for (int y = XoaTru[2][1]; y <= XoaCong[2][3]; y++)
		{
			gMap[MapCTC].SetAttr(x, y, 16);
		}
	}



	for (int n = OBJ_STARTUSERINDEX; n < MAX_OBJECT; n++)
	{
		if (gObj[n].Type == OBJECT_USER && gObj[n].Map == 98) {
			GCMapAttrSend(n, 0, 16, 0, 1, (PMSG_MAP_ATTR*)XoaDuongVao[0]);
			GCMapAttrSend(n, 0, 16, 0, 1, (PMSG_MAP_ATTR*)XoaDuongVao[1]);
			GCMapAttrSend(n, 0, 16, 0, 1, (PMSG_MAP_ATTR*)XoaDuongVao[2]);
			GCMapAttrSend(n, 0, 16, 0, 1, (PMSG_MAP_ATTR*)XoaDuongVao[3]);
			GCMapAttrSend(n, 0, 16, 0, 1, (PMSG_MAP_ATTR*)XoaDuongVao[4]);

			GCMapAttrSend(n, 0, 6, 0, 1, (PMSG_MAP_ATTR*)XoaTru[0]);
			GCMapAttrSend(n, 0, 6, 0, 1, (PMSG_MAP_ATTR*)XoaTru[1]);
			GCMapAttrSend(n, 0, 6, 0, 1, (PMSG_MAP_ATTR*)XoaTru[2]);

			GCMapAttrSend(n, 0, 16, 0, 1, (PMSG_MAP_ATTR*)XoaCong[0]);
			GCMapAttrSend(n, 0, 16, 0, 1, (PMSG_MAP_ATTR*)XoaCong[1]);
			GCMapAttrSend(n, 0, 16, 0, 1, (PMSG_MAP_ATTR*)XoaCong[2]);
		}

	}


#endif
}

void CCTCmini::DelEntranceZone() // OK
{
#if(GAMESERVER_UPDATE>=501)

	for (int x = XoaDuongVao[0][0]; x <= XoaDuongVao[0][2]; x++)
	{
		for (int y = XoaDuongVao[0][1]; y <= XoaDuongVao[0][3]; y++)
		{
			gMap[MapCTC].DelAttr(x, y, 16);
		}
	}

	for (int x = XoaDuongVao[1][0]; x <= XoaDuongVao[1][2]; x++)
	{
		for (int y = XoaDuongVao[1][1]; y <= XoaDuongVao[1][3]; y++)
		{
			gMap[MapCTC].DelAttr(x, y, 16);
		}
	}

	for (int x = XoaDuongVao[2][0]; x <= XoaDuongVao[2][2]; x++)
	{
		for (int y = XoaDuongVao[2][1]; y <= XoaDuongVao[2][3]; y++)
		{
			gMap[MapCTC].DelAttr(x, y, 16);
		}
	}

	for (int x = XoaDuongVao[3][0]; x <= XoaDuongVao[3][2]; x++)
	{
		for (int y = XoaDuongVao[3][1]; y <= XoaDuongVao[3][3]; y++)
		{
			gMap[MapCTC].DelAttr(x, y, 16);
		}
	}

	for (int x = XoaDuongVao[4][0]; x <= XoaDuongVao[4][2]; x++)
	{
		for (int y = XoaDuongVao[4][1]; y <= XoaDuongVao[4][3]; y++)
		{
			gMap[MapCTC].DelAttr(x, y, 16);
		}
	}
	//===========Xoa Tru
	for (int x = XoaTru[0][0]; x <= XoaTru[0][2]; x++)
	{
		for (int y = XoaTru[0][1]; y <= XoaTru[0][3]; y++)
		{
			gMap[MapCTC].DelAttr(x, y, 6);
		}
	}
	for (int x = XoaTru[1][0]; x <= XoaTru[1][2]; x++)
	{
		for (int y = XoaTru[1][1]; y <= XoaTru[1][3]; y++)
		{
			gMap[MapCTC].DelAttr(x, y, 6);
		}
	}
	for (int x = XoaTru[2][0]; x <= XoaTru[2][2]; x++)
	{
		for (int y = XoaTru[2][1]; y <= XoaTru[2][3]; y++)
		{
			gMap[MapCTC].DelAttr(x, y, 6);
		}
	}



	for (int n = OBJ_STARTUSERINDEX; n < MAX_OBJECT; n++)
	{
		if (gObj[n].Type == OBJECT_USER && gObj[n].Map == 98) {
			GCMapAttrSend(n, 0, 16, 1, 1, (PMSG_MAP_ATTR*)XoaDuongVao[0]);
			GCMapAttrSend(n, 0, 16, 1, 1, (PMSG_MAP_ATTR*)XoaDuongVao[1]);
			GCMapAttrSend(n, 0, 16, 1, 1, (PMSG_MAP_ATTR*)XoaDuongVao[2]);
			GCMapAttrSend(n, 0, 16, 1, 1, (PMSG_MAP_ATTR*)XoaDuongVao[3]);
			GCMapAttrSend(n, 0, 16, 1, 1, (PMSG_MAP_ATTR*)XoaDuongVao[4]);

			GCMapAttrSend(n, 0, 6, 1, 1, (PMSG_MAP_ATTR*)XoaTru[0]);
			GCMapAttrSend(n, 0, 6, 1, 1, (PMSG_MAP_ATTR*)XoaTru[1]);
			GCMapAttrSend(n, 0, 6, 1, 1, (PMSG_MAP_ATTR*)XoaTru[2]);
		}

	}


#endif
}

void CCTCmini::XoaTruChiDinh(BYTE n) // OK
{
	for (int x = XoaCong[n][0]; x <= XoaCong[n][2]; x++)
	{
		for (int y = XoaCong[n][1]; y <= XoaCong[n][3]; y++)
		{
			gMap[MapCTC].DelAttr(x, y, 6);
		}
	}
	for (int u = OBJ_STARTUSERINDEX; u < MAX_OBJECT; u++)
	{
		if (gObj[u].Type == OBJECT_USER && gObj[u].Map == 98)
		{
			GCMapAttrSend(u, 0, 16, 1, 1, (PMSG_MAP_ATTR*)XoaCong[n]);
		}

	}

}
void CCTCmini::TestCTCMini() // OK
{
	SetEntranceZone(); //Set dia hinh
	DelEntranceZone(); //Xoa dia hinh

	//=============== Them Cong Vao Event
	for (int n = 0; n < 3; n++) //Them Cong
	{
		IndexCong[n] = gObjAddMonster(MapCTC);

		if (OBJECT_RANGE(IndexCong[n]) == 0)
		{
			return;
		}

		LPOBJ lpMonster = &gObj[IndexCong[n]];

		int px = ThemCong[n][0];
		int py = ThemCong[n][1];
		lpMonster->PosNum = -1;
		lpMonster->X = px;
		lpMonster->Y = py;
		lpMonster->TX = px;
		lpMonster->TY = py;
		lpMonster->OldX = px;
		lpMonster->OldY = py;
		lpMonster->StartX = px;
		lpMonster->StartY = py;
		lpMonster->Dir = 1;
		lpMonster->Map = MapCTC;
		lpMonster->MonsterDeleteTime = GetTickCount() + 1800000;

		if (gObjSetMonster(IndexCong[n], ClassCong) == 0)
		{
			gObjDel(IndexCong[n]);
			return;
		}
	}
	//=============== Them Tru Vao Event
	for (int n = 0; n < 3; n++) //Them Tru
	{
		IndexTru[n] = gObjAddMonster(MapCTC);

		if (OBJECT_RANGE(IndexTru[n]) == 0)
		{
			return;
		}

		LPOBJ lpMonster = &gObj[IndexTru[n]];

		int px = ThemTru[n][0];
		int py = ThemTru[n][1];
		lpMonster->PosNum = -1;
		lpMonster->X = px;
		lpMonster->Y = py;
		lpMonster->TX = px;
		lpMonster->TY = py;
		lpMonster->OldX = px;
		lpMonster->OldY = py;
		lpMonster->StartX = px;
		lpMonster->StartY = py;
		lpMonster->Dir = 1;
		lpMonster->Map = MapCTC;
		lpMonster->MonsterDeleteTime = GetTickCount() + 1800000;

		if (gObjSetMonster(IndexTru[n], ClassTru) == 0)
		{
			gObjDel(IndexTru[n]);
			return;
		}
	}

}
void CCTCmini::ThemTruGuild(int n, int GuidNumber) // OK
{
	IndexTru[n] = gObjAddMonster(MapCTC);

	if (OBJECT_RANGE(IndexTru[n]) == 0)
	{
		return;
	}

	LPOBJ lpMonster = &gObj[IndexTru[n]];
	GUILD_INFO_STRUCT* lpGuildInfo = gGuildClass.SearchGuild_Number(GuidNumber);
	int px = ThemTru[n][0];
	int py = ThemTru[n][1];
	lpMonster->PosNum = -1;
	lpMonster->X = px;
	lpMonster->Y = py;
	lpMonster->TX = px;
	lpMonster->TY = py;
	lpMonster->OldX = px;
	lpMonster->OldY = py;
	lpMonster->StartX = px;
	lpMonster->StartY = py;
	lpMonster->Dir = 1;
	lpMonster->Map = MapCTC;
	lpMonster->GuildNumber = GuidNumber;
	memcpy(lpMonster->GuildName, lpGuildInfo->Name, 9); //MAX_GUILD_NAME define
	lpMonster->MonsterDeleteTime = GetTickCount() + 1800000;

	if (gObjSetMonster(IndexTru[n], ClassTru) == 0)
	{
		gObjDel(IndexTru[n]);
		return;
	}
}
int CCTCmini::SoTruDaChiem(int GuildNumber, int aIndex) // OK 
{
	int TruDaChiem = 0;
	for (int n = 0; n < 3; n++) //Them Tru
	{
		if (gObj[IndexTru[n]].GuildNumber == GuildNumber) {
			TruDaChiem++;

		}
	}
	if (this->GetGuild(GuildNumber) == 0)
	{
		this->AddGuild(aIndex);
	}

	if (TruDaChiem >= 2) //this->WinnerTime
	{

		for (int n = 0; n < MAX_CTC_GUILD; n++)
		{
			if (Guild[n].Number == GuildNumber)
			{
				this->Guild[n].Owner = 1; //Set Guild Da Chime duoc 2 tru 
				GCServerMsgStringSendToMap(MapCTC, 0, "[Công Thành chiến Mini] Guild %s đã chiếm đóng thành công 2 trụ!", this->Guild[n].GuildOwner);
			}
			else
			{
				this->Guild[n].Owner = 0;
			}
		}

	}

	return TruDaChiem;
}
bool CCTCmini::MonsterDie(int aIndexMonster, int aIndexUser)
{
	bool Result = false;

	if (!this->m_Enabled)
	{
		return false;

	}

	LPOBJ lpUser = &gObj[aIndexUser];
	LPOBJ lpMonster = &gObj[aIndexMonster];

	if ((lpMonster->Class == ClassTru || lpMonster->Class == ClassCong) && lpMonster->Map == MapCTC)
	{
		for (int n = 0; n < 3; n++) //Check Kill Tru
		{
			//=== Check Pha Tru
			if (IndexTru[n] == lpMonster->Index)
			{
				gObjDel(IndexTru[n]);
				IndexTru[n] = -1;
				this->ThemTruGuild(n, lpUser->GuildNumber); //Tao lai Tru theo Guild Number
				GCServerMsgStringSendToMap(MapCTC, 0, "[Công Thành chiến Mini] Guild %s đã chiếm %d Trụ!", lpUser->GuildName, this->SoTruDaChiem(lpUser->GuildNumber, lpUser->Index));

				LogAdd(LOG_EVENT, "[CTC Mini] Bonus Phá Trụ %s", lpUser->Name);

				//gCashShop.GDCashShopAddPointSaveSend(lpUser->Index,0, b_PhaTru.WCoin, b_PhaTru.WCoinP, b_PhaTru.GobinP , b_PhaTru.Ruud);
				GDSetCoinSend(lpUser->Index, +(b_PhaTru.WCoin), +(b_PhaTru.WCoinP), +(b_PhaTru.GobinP), "CTC Mini");
				gCashShop.CGCashShopPointRecv(lpUser->Index);

				if (b_PhaTru.IndexBonus == 1)
				{
					for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
					{
						if (gObjIsConnectedGP(n) != 0 && gObj[n].Type == OBJECT_USER && gObj[n].Index != lpUser->Index && gObj[n].Map == MapCTC)
						{
							if (strcmp(gObj[n].GuildName, lpUser->Name) == 0)
							{
								LogAdd(LOG_EVENT, "[CTC Mini] Bonus b_PhaTru Member Reward: %s", gObj[n].Name);

								GDSetCoinSend(lpUser->Index, +(b_PhaTru.WCoin), +(b_PhaTru.WCoinP), +(b_PhaTru.GobinP), "CTC Mini");
								gCashShop.CGCashShopPointRecv(gObj[n].Index);

							}
						}
					}
				}
				this->SendKillCTCMini(0, 0);
				Result = true;

			}
			//==== Check Pha Cong
			if (IndexCong[n] == lpMonster->Index)
			{
				gObjDel(IndexCong[n]);
				this->XoaTruChiDinh(n);
				GCServerMsgStringSendToMap(MapCTC, 0, "[Công Thành chiến Mini] Guild %s đã phá cổng %d!", lpUser->GuildName, n + 1);

				//LogAdd(LOG_EVENT, "[CTC Mini] Bonus Phá Cổng %s", lpUser->Name);

				//gCashShop.GDCashShopAddPointSaveSend(lpUser->Index,0, b_PhaCong.WCoin, b_PhaCong.WCoinP, b_PhaCong.GobinP , b_PhaCong.Ruud);

				GDSetCoinSend(lpUser->Index, +(b_PhaCong.WCoin), +(b_PhaCong.WCoinP), +(b_PhaCong.GobinP), "CTC Mini");

				gCashShop.CGCashShopPointRecv(gObj[n].Index);

				if (b_PhaCong.IndexBonus == 1) {
					for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
					{
						if (gObjIsConnectedGP(n) != 0 && gObj[n].Type == OBJECT_USER && gObj[n].Index != lpUser->Index && gObj[n].Map == MapCTC)
						{
							if (strcmp(gObj[n].GuildName, lpUser->Name) == 0)
							{
								LogAdd(LOG_EVENT, "[CTC Mini] Bonus b_PhaCong Member Reward: %s", gObj[n].Name);

								GDSetCoinSend(lpUser->Index, +(b_PhaCong.WCoin), +(b_PhaCong.WCoinP), +(b_PhaCong.GobinP), "CTC Mini");

								gCashShop.CGCashShopPointRecv(gObj[n].Index);
							}
						}
					}
				}
				this->SendKillCTCMini(0, 0);
				Result = true;
			}
			//============================
		}
	}

	return Result;
}


bool CCTCmini::GetUserRespawnLocation(LPOBJ lpObj, int* gate, int* map, int* x, int* y, int* dir, int* level) // OK
{

	if (this->m_Enabled == 0)
	{
		return 0;
	}

	if (this->GetChar(lpObj->Index) == 0)
	{
		return 0;
	}

	int CongVao = this->GetCongVao(lpObj->Index);

	if (gGate.GetGate(this->GateMove[CongVao], gate, map, x, y, dir, level) != 0)
	{
		return 1;
	}

	return 0;
}


void CCTCmini::StartCTCMini()
{
	time_t theTime = time(NULL);
	struct tm* aTime = localtime(&theTime);

	int hour = aTime->tm_hour;
	int minute = aTime->tm_min + 2;


	if (minute >= 60)
	{
		hour++;
		minute = minute - 60;
	}

	CTC_START_TIME info;

	info.Year = -1;

	info.Month = -1;

	info.Day = -1;

	info.DayOfWeek = -1;

	info.Hour = hour;

	info.Minute = minute;

	info.Second = 0;

	this->m_CTCMinitartTime.push_back(info);

	LogAdd(LOG_EVENT, "[CTC Mini] Bắt Đầu At %2d:%2d:00", hour, minute);

	this->Init();
}


#endif