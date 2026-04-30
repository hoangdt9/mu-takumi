#pragma once
#include "DefaultClassInfo.h"
#include "User.h"
#include "Protocol.h"
#include "SkillManager.h"
#define eMessageBox				255
#if(MOCNAP)
struct MESSAGE_INFO_MocNap
{
	int Index;
	char Message[256];
};
struct DATA_CBITEMMOCNAP
{
	float SizeBMD;
	int Count;
	int IndexItem;
	int LvItem;
	int Dur;
	int Skill;
	int Luck;
	int Opt;
	int Exc;
	int Anc;
	int SK[MAX_SOCKET_OPTION];
	int SKBonus;
	int HSD;
	int Class[MAX_CLASS];
};
struct DATA_CBMONAP
{
	int IndexMocNap;
	int GiaTriNap;
	int WC;
	int WP;
	int GP;
	int Ruud;
	std::vector<DATA_CBITEMMOCNAP> ListItemNhan;
};
//===List Moc Nap
struct PMSG_CBMONAP_SEND
{
	PSWMSG_HEAD header; // C2:F3:E2
	BYTE count;
	int NhanMocNap;
	int TongNap;
};

struct ListMocNapSend
{
	int IndexMocNap;
	int GiaTriNap;
};
//===List THuowng
struct PMSG_CBLISTTHUONG_SEND
{
	PSWMSG_HEAD header; // C2:F3:E2
	BYTE count;
	int WC;
	int WP;
	int GP;
	int Ruud;
};
struct LISTITEMMOCNAP_SENDINFO
{
	float SizeBMD;
	int Count;
	short Index;
	BYTE Dur;
	BYTE Item[12];
	int  PeriodTime;
};
class gBMocNap
{
public:
	gBMocNap();
	virtual ~gBMocNap();
	void LoadFileXML(char* FilePath);
	void gBMocNap::UserSendClientInfo(int aIndex);
	void gBMocNap::SendListNhanThuong(int aIndex, int MocNap);
	void gBMocNap::NhanThuongMocNap(int aIndex, int MocNap);
private:
	//===Mess
	int Enable;
	int Firework;
	int Notice;
	std::map<int, MESSAGE_INFO_MocNap> m_MessageInfoBP;
	std::map<int, DATA_CBMONAP> m_DataMocNap;
	char* GetMessage(int index);
};

extern gBMocNap gMocNap;

#endif