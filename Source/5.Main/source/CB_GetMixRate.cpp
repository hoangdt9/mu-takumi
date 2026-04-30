#include "StdAfx.h"
#include "CB_GetMixRate.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"
#include "MixMgr.h"

#if(CB_GETMIXRATE)
CB_GetMixRate* gCB_GetMixRate;

CB_GetMixRate::CB_GetMixRate()
{
	this->RealMix = -1;
	g_MixRecipeMgr.RealRate = -1;
	g_MixRecipeMgr.RealZen = -1;
}

CB_GetMixRate::~CB_GetMixRate()
{
}

void CB_GetMixRate::SetInfoMixID(int MixID)
{
	if (this->RealMix != -1 && MixID == 0)
	{
		this->RealMix = -1;
		g_MixRecipeMgr.RealRate = -1;
		g_MixRecipeMgr.RealZen = -1;
	}
	if (MixID != this->RealMix && MixID != 0)
	{
		//gInterface.DrawMessage(1, "SetInfoMixID() %d", MixID);
		this->RealMix = MixID;
		this->CGSendMixInfo();
		g_MixRecipeMgr.RealRate = -1;
		g_MixRecipeMgr.RealZen = -1;
	}

	//gInterface.DrawMessage(1, "SetInfoMixID %d %d", MixID, g_MixRecipeMgr.GetMixInventoryType());


}
void CB_GetMixRate::CGSendMixInfo()
{
	PMSG_CHAOX_MIX_GET_RATE_SEND pRequest;

	pRequest.header.set(0x88, sizeof(pRequest));
	pRequest.MixID = this->RealMix;
	pRequest.MixInfo = g_MixRecipeMgr.GetMixSubType();
	DataSend((LPBYTE)& pRequest, pRequest.header.size);
}
void CB_GetMixRate::GCRecvMixInfo(BYTE* lpMsg, int size)
{
	if (sizeof(PMSG_CHAOX_MIX_GET_RATE_RECV) != size)
	{
		return;
	}
	PMSG_CHAOX_MIX_GET_RATE_RECV* Data = (PMSG_CHAOX_MIX_GET_RATE_RECV*)lpMsg;
	g_MixRecipeMgr.RealRate = Data->Rate;
	g_MixRecipeMgr.RealZen = Data->Zen;
}

#endif