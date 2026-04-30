#include "stdafx.h"
#include "Protect.h"
#include "Utilities/CCRC32.H"
#include "Util.h"
#include "MainLoad.h"
#include "BuffIcon.h"
#include "TrayMode.h"
#include "APICB.h"
MainLoad gMainLoad;

MainLoad::MainLoad()
{
	
}
MainLoad::~MainLoad()
{

}
DWORD StartAddress(void* lpThreadParameter)
{
	HANDLE v1;
	HANDLE v2;

	while (TRUE)
	{
		::Sleep(60000);

		v1 = GetCurrentProcess();
		SetProcessWorkingSetSize(v1, 0xFFFFFFFF, 0xFFFFFFFF);

		v2 = GetCurrentProcess();
		SetThreadPriority(v2, -2);
	}

	return 0;
}

bool MainLoad::Load()
{
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)StartAddress, 0, 0, 0);

	if (gProtect.ReadMainFile(".\\Data\\Local\\CBGetMain.bin") == 0)
	{
		MessageBox(0, "Config corrupt! ReadMainFile", "Error", MB_OK | MB_ICONERROR);
		ExitProcess(0);
		return 0;
	}
	if (gProtect.ReadTextFile(".\\Data\\Local\\CBTextInfo.bin") == 0)
	{
		MessageBox(0, "Config corrupt! ReadTextFile", "Error", MB_OK | MB_ICONERROR);
		ExitProcess(0);
		return 0;
	}
	gProtect.LoadEncDec();

	gProtect.CheckPluginFile();
	gProtect.CheckLauncher();
	gProtect.CheckInstance();

	//=== Set IP Serrial 
	szServerIpAddress = gProtect.m_MainInfo.IpAddress;
	g_ServerPort = gProtect.m_MainInfo.IpAddressPort;
	memcpy(Serial, gProtect.m_MainInfo.ClientSerial, sizeof(Serial));
	Version[0] = (BYTE)gProtect.m_MainInfo.ClientVersion[0] + 1;
	Version[1] = (BYTE)gProtect.m_MainInfo.ClientVersion[2] + 2;
	Version[2] = (BYTE)gProtect.m_MainInfo.ClientVersion[3] + 3;
	Version[3] = (BYTE)gProtect.m_MainInfo.ClientVersion[5] + 4;
	Version[4] = (BYTE)gProtect.m_MainInfo.ClientVersion[6] + 5;

	if (gProtect.m_MainInfo.LoadAntihack)
	{
		
		
		

	}
	
	gCustomMessage.LoadEng(gProtect.m_MainInfo.EngCustomMessageInfo);
	gCustomMessage.LoadVtm(gProtect.m_MainInfo.VtmCustomMessageInfo);

	gCustomBattleGloves.Load(gProtect.m_MainInfo.CustomGloves);
	gCustomJewel.Load(gProtect.m_MainInfo.CustomJewelInfo);
	gCustomWing.Load(gProtect.m_MainInfo.CustomWingInfo);
	gCustomItem.Load(gProtect.m_MainInfo.CustomItemInfo);
	gCustomItem.LoadRingPen(gProtect.m_MainInfo.CustomRingPenInfo);

	gCloak.Load(gProtect.m_MainInfo.m_CustomCloak);
	gCloak.LoadCEffect(gProtect.m_MainInfo.m_CustomCEffect);

	gCustomWingEffect.Load(gProtect.m_MainInfo.CustomWingEffectInfo);
	gDynamicWingEffect.Load(gProtect.m_MainInfo.DynamicWingEffectInfo);


	gCustomBow.Load(gProtect.m_MainInfo.CustomBowInfo);
	ItemTRSData.Load(gProtect.m_MainInfo.CustomPosition);

	gCustomMonster.Load(gProtect.m_MainInfo.CustomMonsters);
	gCustomMonster.LoadBossClass(gProtect.m_MainInfo.CustomBossClass);
	gNPCName.Load(gProtect.m_MainInfo.CustomNPCName);
	gCustomMonsterGlow.LoadGlow(gProtect.m_MainInfo.m_CustomMonsterGlow);
	gCustomMonsterGlow.LoadBrightness(gProtect.m_MainInfo.m_CustomMonsterbrightness);

	gCustomMap.OpenScritp(gProtect.m_MainInfo.m_MapInfo);
	gCustomPet2.Load(gProtect.m_MainInfo.CustomPetInfo);
	gCustomCEffectPet.Load(gProtect.m_MainInfo.m_PetCEffectBMD);
	gCustomCEffectPet.LoadGlow(gProtect.m_MainInfo.RenderMeshPet);
	gggJCEffectMonster.Load(gProtect.m_MainInfo.m_CustomMonsterEffect);
	//=============

	//==Text FIle
	gIconBuff.LoadEng(gProtect.m_TextInfo.m_TooltipTRSDataEng);
	gIconBuff.LoadVTM(gProtect.m_TextInfo.m_TooltipTRSDataVTM);

	GInfo.loadnInformation(gProtect.m_TextInfo.m_TRSTooltipData);
	GInfo.loadnInformationSet(gProtect.m_TextInfo.m_TRSTooltipSetData);
	GInfo.loadnText(gProtect.m_TextInfo.m_TRSTooltipText);

	gCustomBuyVip.Load(gProtect.m_MainInfo.CustomBuyVipInfo); 

#if (CB_ANTIHACKGGNEW)
	gAPICB.Init();
#endif
	SetTargetFps(gProtect.m_MainInfo.FpsLimit);

	gCustomCommandInfo.Load(gProtect.m_MainInfo.CustomCommandInfo);
	gCustomDmgColor.Load(gProtect.m_MainInfo.CustomDmgColor); //Dmg Color
	return 1;
}