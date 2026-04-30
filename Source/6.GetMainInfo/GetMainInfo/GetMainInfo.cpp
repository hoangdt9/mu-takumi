#include "stdafx.h"
#include "..\..\5.Main\source\Utilities\\CCRC32.H"
#include "CustomItem.h"
#include "CustomWing.h"
#include "CustomWIngEffect.h"
#include "DynamicEffect.h"
#include "ThemidaSDK.h"
#include "CustomMessage.h"
#include "CustomJewel.h"
#include "CustomCloak.h"
#include "CustomGloves.h"
#include "CustomCrossBow.h"
#include "CustomItemPosition.h"
#include "CustomMonster.h"
#include "CustomPet.h"
#include "UIMapName.h"
#include "TooltipBuff.h"
#include "CustomNpcName.h"
#include "ItemToolTip.h"
#include "CustomBuyVip.h"
#include "CustomPetEffect.h"
#include "CustomMonsterGlow.h"
#include "MonsterEffect.h"
#include "CustomCommandInfo.h"
#include "CustomDmgColor.h"

struct MAIN_FILE_INFO
{
	DWORD GSPortMin;
	DWORD GSPortMax;

	BYTE LoadAntihack;
	BYTE LauncherType;
	char LauncherName[32];
	char LauncherFile[32];



	char CustomerName[32];
	char IpAddress[32];
	WORD IpAddressPort;
	char ClientVersion[8];
	char ClientSerial[17];
	char WindowName[32];
	char ScreenShotPath[50];
	char ClientName[32];
	DWORD TIME_GMT;
	char PluginName[32];
	char CameraName[32];
	DWORD ClientCRC32;
	DWORD PluginCRC32;
	DWORD CameraCRC32;
	DWORD HelperActiveAlert;
	DWORD HelperActiveLevel;
	DWORD DWMaxAttackSpeed;
	DWORD DKMaxAttackSpeed;
	DWORD FEMaxAttackSpeed;
	DWORD MGMaxAttackSpeed;
	DWORD DLMaxAttackSpeed;
	DWORD SUMaxAttackSpeed;
	DWORD RFMaxAttackSpeed;
	DWORD ReconnectTime;

	DWORD EnableCsSkillsAllMaps;
	DWORD PlayerShowName;
	DWORD PlayerShowGuildLogo;
	DWORD FpsMonitor;
	DWORD FpsLimit;
	DWORD ZoomMin;
	DWORD ZoomMax;
	DWORD Menu[15];

	//===Nap Bank
	DWORD EnableNapBankButton;
	char Bank_NameBank[126];
	char Bank_STK[126];
	char Bank_ChuTK[126];
	char Bank_ChiNhanh[126];
	//===
	BOOL EnableButtonFB;
	char LinkFacbook[128];
	int ViTriFB[2];
	BOOL EnableButtonZL;
	char LinkZalo[128];
	int ViTriZL[2];
	BOOL EnableButtonRS;
	BOOL EnableSlotPet;
	BOOL DisableMix380;
	BYTE RemoveClass;
	int ViTriRS[2];
	BYTE CapDo_Blood;
	BYTE CapDo_Devil;

	//== AutoHP
	BOOL EnableButtonAutoHP;
	int DelayAutoHP;
	int ViTriAutoHP[2];
	int RateHP;
	//== AutoHP
	BOOL EnableButtonAutoPK;
	int ViTriAutoPK[2];

	CUSTOM_MESSAGE_INFO EngCustomMessageInfo[MAX_CUSTOM_MESSAGE];
	CUSTOM_MESSAGE_INFO VtmCustomMessageInfo[MAX_CUSTOM_MESSAGE];
	CUSTOM_JEWEL_INFO CustomJewelInfo[MAX_CUSTOM_JEWEL];
	CUSTOM_WING_INFO CustomWingInfo[MAX_CUSTOM_WING];
	CUSTOMCLOAK_DATA m_CustomCloak[MAX_CUSTOMCLOAK];
	CEFFECT_DATA m_CustomCEffect[MAX_CUSTOMCEFFECT];
	CUSTOM_ITEM_INFO CustomItemInfo[MAX_CUSTOM_ITEM];
	CUSTOM_RINGPEN_INFO CustomRingPenInfo[MAX_CUSTOM_ITEM];
	CUSTOM_WING_EFFECT_INFO CustomWingEffectInfo[MAX_CUSTOM_WING_EFFECT];
	DYNAMIC_WING_EFFECT_INFO DynamicWingEffectInfo[MAX_DYNAMIC_WING_EFFECT];
	CUSTOM_BOW_INFO CustomBowInfo[MAX_CUSTOM_BOW];
	CUSTOM_RF_GLOVES CustomGloves[MAX_CUSTOM_GLOVES];
	CUSTOM_POSITION_INFO CustomPosition[MAX_POSITION_ITEM];
	DWORD IsVersion;
	BOOL  ShowInfoItemDrop;

	CUSTOMMONSTER_DATA CustomMonsters[MAX_CUSTOMMONSTER];
	BOSSCLASS_DATA	   CustomBossClass[MAX_BOSSCLASS]; //Boss Class
	NPCNAME_DATA CustomNPCName[MAX_CUSTOM_NPCNAME];

	CUSTOM_PET_INFO CustomPetInfo[MAX_PET_ITEM];
	PET_EFFECT_CUSTOM m_PetCEffectBMD[MAX_CUSTOPETEFFECT];
	RENDER_MESH RenderMeshPet[MAX_RENDER_MESH];

	PathMap m_MapInfo[MAX_MAP];

	CUSTOM_BUYVIP_INFO CustomBuyVipInfo[MAX_CUSTOM_BUYVIP];
	CUSTOM_MONSTERGLOW_INFO m_CustomMonsterGlow[MAX_CUSTOM_MONSTERGLOW];
	CUSTOM_GLOW_INFO m_CustomMonsterbrightness[MAX_CUSTOM_MONSTERGLOW];
	xMonsterEffect m_CustomMonsterEffect[MAX_EFFECT_MONSTER];

	bool ShowCommandInfo;
	CUSTOM_COMMAND_INFO CustomCommandInfo[MAX_CUSTOM_COMMAND];
	BYTE HidenPetIsClass[7];
	DWORD HidenPetDelay;
	CUSTOM_DMG_COLOR   CustomDmgColor[MAX_CUSTOM_DMG]; //Dmg Color

	BYTE			OnBangChiSo;
	BYTE			OnHealBar;

};
struct TEXT_FILE_INFO
{
	RenderTooltipBuff m_TooltipTRSDataEng[256];
	RenderTooltipBuff m_TooltipTRSDataVTM[256];

	nInformation m_TRSTooltipData[MaxLineItemToolTip];
	nInformation m_TRSTooltipSetData[MaxLineItemToolTip];
	nText m_TRSTooltipText[MaxLineItemToolTip];
};

void GetMainInfo()
{
	MAIN_FILE_INFO info;

	memset(&info, 0, sizeof(info));

	info.LoadAntihack = GetPrivateProfileInt("AntihackConfig", "LoadAntihack", 0, ".\\MainInfo.ini");
	info.GSPortMin = GetPrivateProfileInt("AntihackConfig", "GSPortMin", 55901, ".\\MainInfo.ini");
	info.GSPortMax = GetPrivateProfileInt("AntihackConfig", "GSPortMax", 55950, ".\\MainInfo.ini");

	info.LauncherType = GetPrivateProfileInt("MainInfo", "LauncherType", 0, ".\\MainInfo.ini");
	GetPrivateProfileString("MainInfo", "LauncherName", "", info.LauncherName, sizeof(info.LauncherName), ".\\MainInfo.ini");
	GetPrivateProfileString("MainInfo", "LauncherFile", "", info.LauncherFile, sizeof(info.LauncherFile), ".\\MainInfo.ini");


	info.IpAddressPort = GetPrivateProfileInt("MainInfo", "IpAddressPort", 63000, ".\\MainInfo.ini");

	GetPrivateProfileString("MainInfo", "CustomerName", "", info.CustomerName, sizeof(info.CustomerName), ".\\MainInfo.ini");

	GetPrivateProfileString("MainInfo", "IpAddress", "", info.IpAddress, sizeof(info.IpAddress), ".\\MainInfo.ini");

	GetPrivateProfileString("MainInfo", "ClientVersion", "", info.ClientVersion, sizeof(info.ClientVersion), ".\\MainInfo.ini");

	GetPrivateProfileString("MainInfo", "ClientSerial", "", info.ClientSerial, sizeof(info.ClientSerial), ".\\MainInfo.ini");

	GetPrivateProfileString("MainInfo", "WindowName", "", info.WindowName, sizeof(info.WindowName), ".\\MainInfo.ini");

	GetPrivateProfileString("MainInfo", "ScreenShotPath", "", info.ScreenShotPath, sizeof(info.ScreenShotPath), ".\\MainInfo.ini");

	GetPrivateProfileString("MainInfo", "ClientName", "", info.ClientName, sizeof(info.ClientName), ".\\MainInfo.ini");

	GetPrivateProfileString("MainInfo", "PluginName", "", info.PluginName, sizeof(info.PluginName), ".\\MainInfo.ini");

	GetPrivateProfileString("MainInfo", "CameraName", "", info.CameraName, sizeof(info.CameraName), ".\\MainInfo.ini");

	info.TIME_GMT = GetPrivateProfileInt("MainInfo", "GTM", 0, ".\\MainInfo.ini");

	info.HelperActiveAlert = GetPrivateProfileInt("HelperInfo", "HelperActiveAlert", 0, ".\\MainInfo.ini");

	info.HelperActiveLevel = GetPrivateProfileInt("HelperInfo", "HelperActiveLevel", 80, ".\\MainInfo.ini");

	info.DWMaxAttackSpeed = GetPrivateProfileInt("CharacterInfo", "DWMaxAttackSpeed", 65535, ".\\MainInfo.ini");

	info.DKMaxAttackSpeed = GetPrivateProfileInt("CharacterInfo", "DKMaxAttackSpeed", 65535, ".\\MainInfo.ini");

	info.FEMaxAttackSpeed = GetPrivateProfileInt("CharacterInfo", "FEMaxAttackSpeed", 65535, ".\\MainInfo.ini");

	info.MGMaxAttackSpeed = GetPrivateProfileInt("CharacterInfo", "MGMaxAttackSpeed", 65535, ".\\MainInfo.ini");

	info.DLMaxAttackSpeed = GetPrivateProfileInt("CharacterInfo", "DLMaxAttackSpeed", 65535, ".\\MainInfo.ini");

	info.SUMaxAttackSpeed = GetPrivateProfileInt("CharacterInfo", "SUMaxAttackSpeed", 65535, ".\\MainInfo.ini");

	info.RFMaxAttackSpeed = GetPrivateProfileInt("CharacterInfo", "RFMaxAttackSpeed", 65535, ".\\MainInfo.ini");

	info.ReconnectTime = GetPrivateProfileInt("ReconnectInfo", "ReconnectTime", 0, ".\\MainInfo.ini");

	info.IsVersion = GetPrivateProfileInt("MainInfo", "IsSeason", 0, ".\\MainInfo.ini");

	info.ShowInfoItemDrop = GetPrivateProfileInt("MainInfo", "ShowInfoItemDrop", 0, ".\\MainInfo.ini");

	info.EnableCsSkillsAllMaps = GetPrivateProfileInt("Custom", "EnableCsSkillsAllMaps", 0, ".\\MainInfo.ini");
	info.PlayerShowGuildLogo = GetPrivateProfileInt("Custom", "PlayerShowGuildLogo", 0, ".\\MainInfo.ini");

	info.ShowCommandInfo = GetPrivateProfileInt("Custom", "ShowCommandInfo", 0, ".\\MainInfo.ini");

	info.FpsMonitor = GetPrivateProfileInt("Custom", "FpsMonitor", 0, ".\\MainInfo.ini");
	info.FpsLimit = GetPrivateProfileInt("Custom", "FpsLimit", 0, ".\\MainInfo.ini");
	info.ZoomMin = GetPrivateProfileInt("Custom", "ZoomMin", 15, ".\\MainInfo.ini");
	info.ZoomMax = GetPrivateProfileInt("Custom", "ZoomMax", 55, ".\\MainInfo.ini");

	info.HidenPetIsClass[0] = GetPrivateProfileInt("Custom", "HidenPetIsDW", 0, ".\\MainInfo.ini");
	info.HidenPetIsClass[1] = GetPrivateProfileInt("Custom", "HidenPetIsDK", 0, ".\\MainInfo.ini");
	info.HidenPetIsClass[2] = GetPrivateProfileInt("Custom", "HidenPetIsELF", 0, ".\\MainInfo.ini");
	info.HidenPetIsClass[3] = GetPrivateProfileInt("Custom", "HidenPetIsMG", 0, ".\\MainInfo.ini");
	info.HidenPetIsClass[4] = GetPrivateProfileInt("Custom", "HidenPetIsDL", 0, ".\\MainInfo.ini");
	info.HidenPetIsClass[5] = GetPrivateProfileInt("Custom", "HidenPetIsSUM", 0, ".\\MainInfo.ini");
	info.HidenPetIsClass[6] = GetPrivateProfileInt("Custom", "HidenPetIsRF", 0, ".\\MainInfo.ini");
	info.HidenPetDelay = GetPrivateProfileInt("Custom", "HidenPetDelay", 5000, ".\\MainInfo.ini");

	info.OnBangChiSo = GetPrivateProfileInt("Custom", "OnBangChiSo", 0, ".\\MainInfo.ini");
	info.OnHealBar = GetPrivateProfileInt("Custom", "OnHealBar", 0, ".\\MainInfo.ini");

	//==Button Menu
	char GetFotText[55] = { 0 };
	for (int st = 0; st < 15; st++)
	{
		wsprintf(GetFotText, "Menu%d", st);
		info.Menu[st] = GetPrivateProfileInt("Custom", GetFotText, 0, ".\\MainInfo.ini");
	}
	//===Nap Bank
	info.EnableNapBankButton = GetPrivateProfileInt("Custom", "EnableNapBankButton", 0, ".\\MainInfo.ini");
	GetPrivateProfileString("Custom", "Bank_NameBank", "Bank_NameBank", info.Bank_NameBank, sizeof(info.Bank_NameBank), ".\\MainInfo.ini");
	GetPrivateProfileString("Custom", "Bank_STK", "Bank_STK", info.Bank_STK, sizeof(info.Bank_STK), ".\\MainInfo.ini");
	GetPrivateProfileString("Custom", "Bank_ChuTK", "Bank_ChuTK", info.Bank_ChuTK, sizeof(info.Bank_ChuTK), ".\\MainInfo.ini");
	GetPrivateProfileString("Custom", "Bank_ChiNhanh", "Bank_ChiNhanh", info.Bank_ChiNhanh, sizeof(info.Bank_ChiNhanh), ".\\MainInfo.ini");
	//=====
	info.EnableButtonFB = GetPrivateProfileInt("Custom", "EnableButtonFB", 0, ".\\MainInfo.ini");
	GetPrivateProfileString("Custom", "LinkFacbook", "LinkFacbook", info.LinkFacbook, sizeof(info.LinkFacbook), ".\\MainInfo.ini");
	info.ViTriFB[0] = GetPrivateProfileInt("Custom", "ViTriFB_X", 0, ".\\MainInfo.ini");
	info.ViTriFB[1] = GetPrivateProfileInt("Custom", "ViTriFB_Y", 0, ".\\MainInfo.ini");

	info.EnableButtonZL = GetPrivateProfileInt("Custom", "EnableButtonZL", 0, ".\\MainInfo.ini");
	GetPrivateProfileString("Custom", "LinkZalo", "LinkZalo", info.LinkZalo, sizeof(info.LinkZalo), ".\\MainInfo.ini");
	info.ViTriZL[0] = GetPrivateProfileInt("Custom", "ViTriZL_X", 0, ".\\MainInfo.ini");
	info.ViTriZL[1] = GetPrivateProfileInt("Custom", "ViTriZL_Y", 0, ".\\MainInfo.ini");

	info.EnableButtonRS = GetPrivateProfileInt("Custom", "EnableButtonRS", 0, ".\\MainInfo.ini");
	info.EnableSlotPet = GetPrivateProfileInt("Custom", "EnableSlotPet", 0, ".\\MainInfo.ini");
	info.DisableMix380 = GetPrivateProfileInt("Custom", "DisableMix380", 0, ".\\MainInfo.ini");
	
	info.RemoveClass = GetPrivateProfileInt("Custom", "RemoveClass", 0, ".\\MainInfo.ini");
	info.CapDo_Blood = GetPrivateProfileInt("Custom", "MaxLevelBlood", 0, ".\\MainInfo.ini");
	info.CapDo_Devil = GetPrivateProfileInt("Custom", "MaxLevelDevil", 0, ".\\MainInfo.ini");

	info.ViTriRS[0] = GetPrivateProfileInt("Custom", "ViTriRS_X", 0, ".\\MainInfo.ini");
	info.ViTriRS[1] = GetPrivateProfileInt("Custom", "ViTriRS_Y", 0, ".\\MainInfo.ini");

	info.EnableButtonAutoHP = GetPrivateProfileInt("Custom", "EnableButtonAutoHP", 0, ".\\MainInfo.ini");
	info.DelayAutoHP = GetPrivateProfileInt("Custom", "DelayAutoHP", 0, ".\\MainInfo.ini");
	info.ViTriAutoHP[0] = GetPrivateProfileInt("Custom", "ViTriAutoHP_X", 0, ".\\MainInfo.ini");
	info.ViTriAutoHP[1] = GetPrivateProfileInt("Custom", "ViTriAutoHP_Y", 0, ".\\MainInfo.ini");
	info.RateHP = GetPrivateProfileInt("Custom", "RateHP", 0, ".\\MainInfo.ini");
	info.EnableButtonAutoPK = GetPrivateProfileInt("Custom", "EnableButtonAutoPK", 0, ".\\MainInfo.ini");
	info.ViTriAutoPK[0] = GetPrivateProfileInt("Custom", "ViTriAutoPK_X", 0, ".\\MainInfo.ini");
	info.ViTriAutoPK[1] = GetPrivateProfileInt("Custom", "ViTriAutoPK_Y", 0, ".\\MainInfo.ini");
	//===Effect

	gCustomWingEffect.Load("EffectManager\\CustomEffectStatic.txt");
	gDynamicWingEffect.Load("EffectManager\\CustomEffectDynamic.txt");
	gCloak.LoadCEffect("EffectManager\\CEffect.txt");
	gCustomMonsterGlow.Load("EffectManager\\CustomMonsterGlow.txt");
	memcpy(info.m_CustomMonsterGlow, gCustomMonsterGlow.m_CustomGlowInfo, sizeof(info.m_CustomMonsterGlow));
	memcpy(info.m_CustomMonsterbrightness, gCustomMonsterGlow.m_CustombrightnessInfo, sizeof(info.m_CustomMonsterbrightness));
	JCEffectMonster.Load("EffectManager\\CustomMonsterEffect.txt");
	memcpy(info.m_CustomMonsterEffect, JCEffectMonster.m_MonsterEffect, sizeof(info.m_CustomMonsterEffect));
	//===Item
	gCloak.Load("ItemManager\\CustomCape.txt");
	gCustomGloves.Load("ItemManager\\CustomClaws.txt");
	gCustomCrossBow.Load("ItemManager\\CustomCrossbow.txt");
	gCustomItem.Load("ItemManager\\CustomItem.txt");
	gCustomItem.LoadRingPen("ItemManager\\CustomRingPen.txt");
	gCustomPosition.Load("ItemManager\\CustomItemPosition.txt");
	gCustomJewel.Load("ItemManager\\CustomJewel.txt");
	gCustomWing.Load("ItemManager\\CustomWing.txt");

	gCustomMessage.Load("CommonManager\\CustomMessage.txt");

	gCustomCommandInfo.Load("CommonManager\\CustomCommandInfo.txt");
	memcpy(info.CustomCommandInfo, gCustomCommandInfo.m_CustomCommandInfo, sizeof(info.CustomCommandInfo));

	gMonsters.Load("CustomManager\\CustomMonster.txt");
	memcpy(info.CustomMonsters, gMonsters.m_Monsters, sizeof(info.CustomMonsters));
	memcpy(info.CustomBossClass, gMonsters.m_BossClass, sizeof(info.CustomBossClass)); //Custom Boss Class
	
	gNPCName.Load("CustomManager/CustomNPCName.txt");
	memcpy(info.CustomNPCName, gNPCName.m_CustomNpcName, sizeof(info.CustomNPCName));

	//==Pet
	gCustomPet2.Load("CustomManager\\CustomPet.txt"); //CustomPets
	memcpy(info.CustomPetInfo, gCustomPet2.m_CustomPetInfo, sizeof(info.CustomPetInfo)); //Custom Pets
	gCustomCEffectPet.Load("EffectManager\\CustomPetEffect.txt");
	memcpy(info.m_PetCEffectBMD, gCustomCEffectPet.m_PetCEffect, sizeof(info.m_PetCEffectBMD));
	gCustomCEffectPet.LoadGlow("EffectManager\\CustomPetGlow.txt");
	memcpy(info.RenderMeshPet, gCustomCEffectPet.m_RenderMeshPet, sizeof(info.RenderMeshPet));

	gCustomDmgColor.Load("CustomManager\\CustomDmgColor.txt");
	memcpy(info.CustomDmgColor, gCustomDmgColor.m_CustomDmgColor, sizeof(info.CustomDmgColor)); //==Dmg Color

	g_pUIMapName.OpenScritp("CustomManager\\CustomMapList.txt");
	memcpy(info.m_MapInfo, g_pUIMapName.m_MapInfo, sizeof(info.m_MapInfo));

	gCustomBuyVip.Load("CommonManager/CustomBuyVip.txt");
	memcpy(info.CustomBuyVipInfo, gCustomBuyVip.m_CustomBuyVipInfo, sizeof(info.CustomBuyVipInfo));


	memcpy(info.EngCustomMessageInfo, gCustomMessage.m_EngCustomMessageInfo, sizeof(info.EngCustomMessageInfo));

	memcpy(info.VtmCustomMessageInfo, gCustomMessage.m_VtmCustomMessageInfo, sizeof(info.VtmCustomMessageInfo));
	//
	memcpy(info.CustomJewelInfo, gCustomJewel.m_CustomJewelInfo, sizeof(info.CustomJewelInfo));
	//
	memcpy(info.CustomWingInfo, gCustomWing.m_CustomWingInfo, sizeof(info.CustomWingInfo));
	//
	memcpy(info.m_CustomCloak, gCloak.m_Cloak, sizeof(info.m_CustomCloak));
	//
	memcpy(info.m_CustomCEffect, gCloak.m_CEffect, sizeof(info.m_CustomCEffect));
	//
	memcpy(info.CustomItemInfo, gCustomItem.m_CustomItemInfo, sizeof(info.CustomItemInfo));

	memcpy(info.CustomRingPenInfo, gCustomItem.m_CustomRingPenInfo, sizeof(info.CustomRingPenInfo));
	//
	memcpy(info.CustomWingEffectInfo, gCustomWingEffect.m_CustomWingEffectInfo, sizeof(info.CustomWingEffectInfo));
	//
	memcpy(info.DynamicWingEffectInfo, gDynamicWingEffect.m_DynamicWingEffectInfo, sizeof(info.DynamicWingEffectInfo));
	//
	memcpy(info.CustomGloves, gCustomGloves.m_CustomGlovesInfo, sizeof(info.CustomGloves));
	//
	memcpy(info.CustomBowInfo, gCustomCrossBow.m_CustomBowInfo, sizeof(info.CustomBowInfo));
	//
	memcpy(info.CustomPosition, gCustomPosition.m_CustomPositionInfo, sizeof(info.CustomPosition));

	CCRC32 CRC32;

	if (CRC32.FileCRC(info.ClientName, &info.ClientCRC32, 1024) == 0)
	{
		info.ClientCRC32 = 0;
	}

	if (CRC32.FileCRC(info.PluginName, &info.PluginCRC32, 1024) == 0)
	{
		info.PluginCRC32 = 0;
	}

	if (CRC32.FileCRC(info.CameraName, &info.CameraCRC32, 1024) == 0)
	{
		info.CameraCRC32 = 0;
	}

	for (int n = 0; n <  sizeof(MAIN_FILE_INFO); n++)
	{
		((BYTE*)& info)[n] ^= (BYTE)(0xCA ^ LOBYTE(n));
		((BYTE*)& info)[n] += (BYTE)(0x95 ^ HIBYTE(n));
	}


	HANDLE file = CreateFile("..\\ClientFile\\Data\\Local\\CBGetMain.bin", GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0);

	if (file == INVALID_HANDLE_VALUE)
	{
		return;
	}

	DWORD OutSize = 0;

	if (WriteFile(file, &info, sizeof(MAIN_FILE_INFO), &OutSize, 0) == 0)
	{
		CloseHandle(file);
		return;
	}

	CloseHandle(file);
}

void GetTextInfo()
{
	//===Text Info
	TEXT_FILE_INFO info;
	memset(&info, 0, sizeof(info));
	gCBuffIcon.Load("CommonManager\\CustomBuffEffect_ENG.txt", 0);
	gCBuffIcon.Load("CommonManager\\CustomBuffEffect_VTM.txt", 1);
	memcpy(info.m_TooltipTRSDataEng, gCBuffIcon.m_TooltipENG, sizeof(info.m_TooltipTRSDataEng));
	memcpy(info.m_TooltipTRSDataVTM, gCBuffIcon.m_TooltipVTM, sizeof(info.m_TooltipTRSDataVTM));

	GInfo.Load("ItemManager\\CustomItemToolTip.txt");
	GInfo.Load2("ItemManager\\CustomTextTooltip.txt");

	memcpy(info.m_TRSTooltipData, GInfo.n_TRSTooltipData, sizeof(info.m_TRSTooltipData));
	memcpy(info.m_TRSTooltipSetData, GInfo.n_TRSTooltipSetData, sizeof(info.m_TRSTooltipSetData));
	memcpy(info.m_TRSTooltipText, GInfo.n_TRSTooltipText, sizeof(info.m_TRSTooltipText));

	for (int n = 0; n < sizeof(TEXT_FILE_INFO); n++)
	{
		((BYTE*)& info)[n] ^= (BYTE)(0xCA ^ LOBYTE(n));
		((BYTE*)& info)[n] += (BYTE)(0x95 ^ HIBYTE(n));
	}


	HANDLE file = CreateFile("..\\ClientFile\\Data\\Local\\CBTextInfo.bin", GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0);

	if (file == INVALID_HANDLE_VALUE)
	{
		return;
	}

	DWORD OutSize = 0;

	if (WriteFile(file, &info, sizeof(TEXT_FILE_INFO), &OutSize, 0) == 0)
	{
		CloseHandle(file);
		return;
	}

	CloseHandle(file);
}
#include "Message.h"
struct ANTIHACK_CLIENT_INFO
{
	char CustomerName[32];
	char IpAddress[32];
	WORD ServerPort;
	char ServerName[32];
	char ClientName[32];
	char PluginName[32];
	char VerifyName[32];
	char RegistryHWID[32];
	DWORD ClientCRC32;
	DWORD PluginCRC32;
	DWORD VerifyCRC32;

	BYTE ShowMessageBox;
	DWORD GSPortMin;
	DWORD GSPortMax;
	MESSAGE_INFO EngMessageInfo[MAX_MESSAGE];
	MESSAGE_INFO PorMessageInfo[MAX_MESSAGE];
	MESSAGE_INFO SpnMessageInfo[MAX_MESSAGE];
};
void GetAntiInfo()
{
	ANTIHACK_CLIENT_INFO info;

	memset(&info, 0, sizeof(info));

	GetPrivateProfileString("MainInfo", "CustomerName", "", info.CustomerName, sizeof(info.CustomerName), ".\\MainInfo.ini");
	GetPrivateProfileString("MainInfo", "IpAddress", "", info.IpAddress, sizeof(info.IpAddress), ".\\MainInfo.ini");

	info.ServerPort = GetPrivateProfileInt("AntihackConfig", "AntihackServerPort", 0, ".\\MainInfo.ini");
	info.ShowMessageBox = GetPrivateProfileInt("AntihackConfig", "ShowMessageBox", 0, ".\\MainInfo.ini");
	info.GSPortMin = GetPrivateProfileInt("AntihackConfig", "GSPortMin", 55901, ".\\MainInfo.ini");
	info.GSPortMax = GetPrivateProfileInt("AntihackConfig", "GSPortMax", 55950, ".\\MainInfo.ini");
	GetPrivateProfileString("AntihackConfig", "AntihackServerName", "", info.ServerName, sizeof(info.ServerName), ".\\MainInfo.ini");

	strcpy_s(info.RegistryHWID, "SYSTEM\\HardwareConfig\\Current");

	strcpy_s(info.VerifyName, "..\\Data\\Local\\Main.json");

	gMessage.Load("CommonManager\\Message.txt");

	memcpy(info.EngMessageInfo, gMessage.m_EngMessageInfo, sizeof(info.EngMessageInfo));

	memcpy(info.PorMessageInfo, gMessage.m_PorMessageInfo, sizeof(info.PorMessageInfo));

	memcpy(info.SpnMessageInfo, gMessage.m_SpnMessageInfo, sizeof(info.SpnMessageInfo));

	CCRC32 CRC32;

	if (CRC32.FileCRC(info.ClientName, &info.ClientCRC32, 1024) == 0)
	{
		info.ClientCRC32 = 0;
	}

	if (CRC32.FileCRC(info.PluginName, &info.PluginCRC32, 1024) == 0)
	{
		info.PluginCRC32 = 0;
	}

	for (int n = 0; n < sizeof(ANTIHACK_CLIENT_INFO); n++)
	{
		((BYTE*)& info)[n] ^= 0xF5;
		((BYTE*)& info)[n] -= 0x7D;
	}

	HANDLE file = CreateFile("..\\ClientFile\\Data\\Local\\License.json", GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0);

	if (file == INVALID_HANDLE_VALUE)
	{
		printf("GetAntiInfo1 [OK] \n");
		return;
	}

	DWORD OutSize = 0;

	if (WriteFile(file, &info, sizeof(ANTIHACK_CLIENT_INFO), &OutSize, 0) == 0)
	{
		CloseHandle(file);
		printf("GetAntiInfo2 [OK] \n");
		return;
	}
	printf("GetAntiInfo [OK] \n");
	CloseHandle(file);
}

int _tmain(int argc, _TCHAR* argv[]) // OK
{
	CLEAR_START

	ENCODE_START

	CreateDirectory("..\\ClientFile\\Data", 0);
	CreateDirectory("..\\ClientFile\\Data\\Local", 0);


	GetMainInfo(); //Tao getMain
	GetTextInfo();//Tao Text Info
	GetAntiInfo();
	ENCODE_END

	CLEAR_END
	printf("GetFile [OK] Coppy ClientFile to ClientGame \n");
	system("pause");
	return 0;
}
