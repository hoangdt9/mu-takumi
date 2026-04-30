#include "stdafx.h"
#include "CB_Mapping.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "Protocol.h"
#include "ZzzLodTerrain.h"
#include "CustomRankUser.h"
#include "MapManager.h"


extern char LogInID[MAX_ID_SIZE + 1];
CB_Mapping* gCB_Mapping = nullptr;
HANDLE FileMappingHandle;
CB_Mapping::CB_Mapping()
{
	this->Init();
	this->CacheIndex = -1;
	this->CheckConnectDLL = 0;
}


CB_Mapping::~CB_Mapping()
{

}

void CB_Mapping::Init()
{

}
extern bool LogOut;
void CB_Mapping::Update()
{
	static DWORD CacheTime = 0;

	if (LogOut)
	{
		this->CheckConnectDLL = 0;
	}

	if (CacheTime > GetTickCount() || this->CheckConnectDLL) return;

#if !defined(__ANDROID__)
	CBGAMEGUARD::PublicClass::SetOffsetMemory((size_t)&LogInID, 4);
	CBGAMEGUARD::PublicClass::SetOffsetMemory((size_t)&CharacterAttribute, 5);
	CBGAMEGUARD::PublicClass::SetOffsetMemory((size_t)&g_hWnd, 6);
	CBGAMEGUARD::PublicClass::SetOffsetMemory((size_t)&this->CheckConnectDLL, 9);
#endif



	//gInterface.DrawMessage(1, "UpdateMapping %s %d" , LogInID, this->CheckConnectDLL);
	CacheTime = GetTickCount() + 1000;

	//if (!this->CheckConnectDLL)
	//{
	//	static int Count = 0;

	//	if (Count >= 30)
	//	{
	//		ExitProcess(0);
	//	}
	//	Count++;
	//}
}