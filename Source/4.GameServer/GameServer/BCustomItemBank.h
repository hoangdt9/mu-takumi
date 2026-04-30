#pragma once
#include "User.h"
#include "Protocol.h"
#include "ItemManager.h"
#if (JEWELBANKVER2)
struct DATA_LIST_ITEMBANK
{
	int Active;
	int AutoLoot;
	int ItemIndex;
	int ItemLevel;
	int Cum10Index;
	int Cum10Level;
	int Cum20Index;
	int Cum20Level;
	int Cum30Index;
	int Cum30Level;
	int MaxCountType[4];
};

struct MESSAGE_INFO_ITEMBANK
{
	int Index;
	char Message[256];
};

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//
struct GSSENDDS_COUNT_ITEMBANK
{
	PSWMSG_HEAD header;
	char AccountID[11];
	int aIndex;
	int Count;
};
enum TypeRutGuiBank
{
	eRut1,
	eRut10,
	eRut20,
	eRut30,
	eRutAll,
	eGuiAll,
	eAutoPick,
};
//==== Packer Rut Gui Ngoc
struct REQUEST_RUTGUIBANK
{
	PSBMSG_HEAD header; // C3:F3:03
	BYTE TypeSend;
	short IndexList;
};

struct PSBMSG_JEWELBANK_RECV1
{
	PSBMSG_HEAD header; // C1:F3:F4
	int slot;
};

class BCustomItemBank
{
public:
	BCustomItemBank();
	~BCustomItemBank();
	int Enable;
	void BCustomItemBank::Load(char* FilePath);
	std::vector<DATA_LIST_ITEMBANK> mListItemBank;
	void BCustomItemBank::GSSendDSGetData(int aIndex);
	void BCustomItemBank::DSSetInfoUserBankItem(GSSENDDS_COUNT_ITEMBANK* lpMsg);
	int BCustomItemBank::CheckInfoListItemBank(int ItemIndex, int ItemLevel);
	int BCustomItemBank::CheckInfoListItemBank2(int ItemIndex, int ItemLevel);
	void BCustomItemBank::GSSendDSSaveDataUser(int aIndex);
	void BCustomItemBank::UserSendClientInfo(int aIndex);
	bool BCustomItemBank::CongTruBank(int aIndex, int ItemIndex, int ItemLevel, int Vaule, bool Notice = 1);
	int BCustomItemBank::CheckCountItemBank(int aIndex, int ItemIndex, int ItemLevel);
	bool ClientSendRutOrGui(REQUEST_RUTGUIBANK* lpMsg, int aIndex);
	int BCustomItemBank::GetInventoryItemCountBank(LPOBJ lpObj, int index, int level);
	void JewelBankRecv(PSBMSG_JEWELBANK_RECV1* lpMsg, int aIndex);
	void BCustomItemBank::DeleteInventoryItemCountBank(LPOBJ lpObj, int index, int level, int count);
	void BCustomItemBank::ReloadInfoChar();
	bool BCustomItemBank::CheckItemBankAutoPick(int aIndex, int ItemIndex, int ItemLevel);

	int BCustomItemBank::GetCountItemSlot(int mSlot, int ItemIndex, int ItemLevel);
	int BCustomItemBank::GetSlotIsItem(int ItemIndex, int ItemLevel);
	bool BCustomItemBank::CongBankItemSlot(int aIndex, int ItemIndex, int ItemLevel, bool Notice = 0);
	bool RutNgocSoLuong(REQUEST_RUTGUIBANK* lpMsg, int aIndex);

	void BCustomItemBank::ProcAutoPic();
	void RecvPacket(int aIndex, PMSG_MOVEITEM* aRecv);
private:
	//===Mess
	std::map<int, MESSAGE_INFO_ITEMBANK> m_MessageInfoBP;
	char* GetMessage(int index);
};

extern BCustomItemBank gBCustomItemBank;
#endif