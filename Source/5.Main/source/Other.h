#pragma once
#define TEXTVN_FILE "./Data/Custom/TextVN.dat"

class Other
{
public:
	Other();
	virtual ~Other();
	void Load();
	char Text_Chung[150][120];


	char Text_Event[50][120];
	char Text_Button[50][120];
	char TextVN_CHOTROI[50][120];
	char TextVN_NAPGAME[50][120];
	char Text_InfoDW[50][120];
	char Text_InfoDK[50][120];
	char Text_InfoELF[50][120];
	char Text_InfoMG[50][120];
	char Text_InfoDL[50][120];
	char Text_InfoSUM[50][120];
	char Text_InfoRF[50][120];
	char Text_VIPCHAR[50][120];
	char TextVN_ArenaPvP[50][120];
	char Text_ChangeItem[50][120];
	char Text_ChacracterInfo[50][120];

#if(CongHuongV2)	
	char TextVN_CongHuongTrangBi[50][120];
#endif
	char Text_HuyDong[50][120];

};

extern Other	gOther;