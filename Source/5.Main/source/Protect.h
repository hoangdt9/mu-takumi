// Protect.h: interface for the CProtect class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "CustomItem.h"
#include "CustomJewel.h"
#include "CustomMessage.h"
#include "CustomWing.h"
#include "CustomCloak.h"
#include "StaticEffect.h"
#include "DynamicEffect.h"
#include "CustomGloves.h"
#include "CustomBow.h"
#include "ItemTRSData.h"
#include "CustomMonster.h"
#include "CustomPet.h"
#include "CustomMap.h"
#include "CustomNpcName.h"
#include "ItemTooltip.h"
#include "CustomBuyVip.h"
#include "CustomPetEffect.h"
#include "CustomMonsterGlow.h"
#include "MonsterEffect.h"
#include "CustomCommandInfo.h"
#include "CustomDmgColor.h"

struct RenderTooltipBuff
{
	int Index;
	int IndexBuff;
	int Arg1;
	int Arg2;
	int Arg3;
	char NameBuff[128];
	int Arg4;
	int Arg5;
	int Arg6;
	char InfoBuff[128];
};
struct TEXT_FILE_INFO
{
	RenderTooltipBuff m_TooltipTRSDataEng[256];
	RenderTooltipBuff m_TooltipTRSDataVTM[256];
	nInformation m_TRSTooltipData[MaxLineItemToolTip];
	nInformation m_TRSTooltipSetData[MaxLineItemToolTip];
	nText m_TRSTooltipText[MaxLineItemToolTip];
};

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
	//== AutoPK
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
	BOOL ShowInfoItemDrop;

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
	jCEffectMonster::xMonsterEffectBBBBB m_CustomMonsterEffect[MAX_EFFECT_MONSTER];

	bool ShowCommandInfo;
	CUSTOM_COMMAND_INFO CustomCommandInfo[MAX_CUSTOM_COMMAND];

	BYTE HidenPetIsClass[7];
	DWORD HidenPetDelay;
	CUSTOM_DMG_COLOR   CustomDmgColor[MAX_CUSTOM_DMG]; //Dmg Color

	BYTE			OnBangChiSo;
	BYTE			OnHealBar;
};

class CProtect
{
public:
	CProtect();
	virtual ~CProtect();
	bool ReadMainFile(char* name);
	bool ReadTextFile(char* name);
	void CheckLauncher();
	void CheckInstance();
	void CheckClientFile();
	void CheckPluginFile();
	void CheckCameraFile();
	void DecryptData(BYTE* lpMsg, int size);
	void EncryptData(BYTE* lpMsg, int size);
	bool CheckSocketPort(SOCKET s);
public:
	MAIN_FILE_INFO m_MainInfo;
	TEXT_FILE_INFO m_TextInfo;
	DWORD m_ClientFileCRC;
	void LoadEncDec();
};

extern CProtect gProtect;
