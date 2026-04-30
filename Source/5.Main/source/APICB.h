#pragma once

#if defined(__ANDROID__) || defined(MU_IOS)
#define APICB_API
#else
#pragma comment(lib, "APICB.lib")
#define APICB_API __declspec(dllimport)
#endif
//==Callback
typedef void (*DATA_SEND)(BYTE* lpMsg, DWORD size);
APICB_API void SetAPIDATA_SEND(DATA_SEND callback);

APICB_API void CBAnihack_Work();
APICB_API void CBAnihack_Init();
APICB_API void CBAnihack_Attack();
APICB_API void CBAnihack_Recv(BYTE* Recv);

extern APICB_API DWORD  API_TypeMain;
extern APICB_API DWORD  API_CharLevel;
extern APICB_API DWORD  API_CharWeaponFirstSlot;
extern APICB_API DWORD  API_CharWeaponSecondSlot;
extern APICB_API DWORD  API_CharHelmSlot;
extern APICB_API DWORD  API_CharArmorSlot;
extern APICB_API DWORD  API_CharPantsSlot;
extern APICB_API DWORD  API_CharGlovesSlot;
extern APICB_API DWORD  API_CharBootsSlot;
extern APICB_API DWORD  API_CharWingsSlot;
extern APICB_API DWORD  API_CharPetSlot;
extern APICB_API DWORD  API_CharAnimationID;
extern APICB_API DWORD  API_CharActiveMagic;

extern APICB_API DWORD* API_GameState;
extern APICB_API DWORD* API_FrameValue;
extern APICB_API DWORD* API_SpeedValue;
extern APICB_API DWORD* API_MainTickCount;
extern APICB_API DWORD* API_SyncTickCount;
extern APICB_API DWORD* API_CountModifier;
extern APICB_API DWORD* API_DelayModifier;
extern APICB_API DWORD* API_HasteModifier;
extern APICB_API DWORD* API_SleepModifier;
extern APICB_API DWORD* API_SpeedModifier1;
extern APICB_API DWORD* API_SpeedModifier2;
extern APICB_API DWORD* API_ModelModifier1;
extern APICB_API DWORD* API_ModelModifier2;
extern APICB_API DWORD* API_ModelModifier3;
extern APICB_API DWORD* API_ViewPoint;
extern APICB_API DWORD* API_ViewStrength;
extern APICB_API DWORD* API_ViewDexterity;
extern APICB_API DWORD* API_ViewVitality;
extern APICB_API DWORD* API_ViewEnergy;
extern APICB_API DWORD* API_ViewLeadership;
extern APICB_API DWORD* API_ViewAddStrength;
extern APICB_API DWORD* API_ViewAddDexterity;
extern APICB_API DWORD* API_ViewAddVitality;
extern APICB_API DWORD* API_ViewAddEnergy;
extern APICB_API DWORD* API_ViewAddLeadership;
extern APICB_API DWORD* API_ViewPhysiSpeed;
extern APICB_API DWORD* API_ViewMagicSpeed;

class APICB
{
public:
	APICB();
	virtual ~APICB();
	void Work();
	void Init();
	void Recv(BYTE* Recv);
	void Attack();
};

extern APICB gAPICB;
