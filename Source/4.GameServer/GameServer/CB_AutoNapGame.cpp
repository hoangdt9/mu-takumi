#include "stdafx.h"
#include "stdafx.h"
#include "GameMain.h"
#include "Notice.h"
#include "Util.h"
#include "DSProtocol.h"
#include "CB_AutoNapGame.h"
#include "Protocol.h"
#if(CB_AutoBanking)
CBAutoNapGame gCBAutoNapGame;

CBAutoNapGame::CBAutoNapGame()
{
	Init();
}


CBAutoNapGame::~CBAutoNapGame()
{
}
void CBAutoNapGame::Init()
{
	this->Enable = 0;
	this->TyleNapTien = 0;
	this->GiaTriNapThapNhat = 0;
}
void CBAutoNapGame::RecvConfigNapThe(BYTE* recv)
{
	if (!recv) return;
	RuleAutoNapConfig* lpMsg = (RuleAutoNapConfig*)recv;
	this->Enable = 1;
	this->TyleNapTien = lpMsg->TyleNapTien;
	this->GiaTriNapThapNhat = lpMsg->GiaTriNapThapNhat;
	LogAdd(LOG_BLUE, "[AutoNap] Enable %d, Tyle %d, MinNap %d", this->Enable, this->TyleNapTien, this->GiaTriNapThapNhat);
}
void CBAutoNapGame::GetListThanhToan(int aIndex)
{
	if (!this->Enable)
	{
		return;
	}
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return;
	}
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJECT_USER || lpObj->m_OfflineMode != 0)
	{
		return;
	}
	SDHP_GSGETDATANAPTHE pMsg;

	pMsg.h.set(0xD3, 0x7B, sizeof(pMsg));

	pMsg.aIndex = aIndex;

	memcpy(pMsg.Account, lpObj->Account, sizeof(pMsg.Account));

	gDataServerConnection.DataSend((BYTE*)& pMsg, pMsg.h.size);
}
void CBAutoNapGame::RecvListThanhToan(BYTE* recv)
{
	if (!recv) return;
	LISTCOUNDAUTONAP* lpMsg = (LISTCOUNDAUTONAP*)recv;
	if (!this->Enable)
	{
		return;
	}
	if (gObjIsConnected(lpMsg->aIndex) == false)
	{
		return;
	}
	// ----
	BYTE send[4092];

	LISTCOUNDAUTONAP_CLIENT pMsg;

	pMsg.header.set(0xD3, 0x7B, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;
	pMsg.aIndex = lpMsg->aIndex;
	pMsg.TyleNapTien = this->TyleNapTien;
	pMsg.GiaTriNapThapNhat = this->GiaTriNapThapNhat;

	for (int n = 0; n < lpMsg->count; n++)
	{
		DataLichSuNapThe* lpInfo = (DataLichSuNapThe*)(((BYTE*)lpMsg) + sizeof(LISTCOUNDAUTONAP) + (sizeof(DataLichSuNapThe) * n));
		DataLichSuNapThe lpInfoSend = { 0 };
		lpInfoSend = *lpInfo;
		memcpy(&send[size], &lpInfoSend, sizeof(lpInfoSend));
		size += sizeof(lpInfoSend);
		pMsg.count++;

	}
	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(lpMsg->aIndex, send, size);
}
void CBAutoNapGame::SendNapTienBank(BYTE * recv, int aIndex)
{
	if (!recv) return;
	XULY_CGPACKET* lpMsg = (XULY_CGPACKET*)recv;

	if (!this->Enable)
	{
		return;
	}
	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	LPOBJ lpObj = &gObj[aIndex];
	if (lpMsg->ThaoTac < this->GiaTriNapThapNhat)
	{
		gNotice.GCNoticeSend(aIndex, 255, 0, 0, 0, 0, 0, "Vui lòng nạp ít nhất %d", this->GiaTriNapThapNhat);//
		return;
	}

	SEND_RECV_NAPBANK pMsg;

	pMsg.header.set(0xD3, 0x7C, sizeof(pMsg));
	memcpy(pMsg.Account, lpObj->Account, sizeof(pMsg.Account));
	memcpy(pMsg.CharName, lpObj->Name, sizeof(pMsg.CharName));
	pMsg.TienNap = lpMsg->ThaoTac;
	pMsg.aIndex = aIndex;
	gDataServerConnection.DataSend((BYTE*)& pMsg, pMsg.header.size);
	gNotice.GCNoticeSend(aIndex, 255, 0, 0, 0, 0, 0, "Bạn vừa tạo lệnh nạp với giá trị %d VND", pMsg.TienNap);//

}
#endif