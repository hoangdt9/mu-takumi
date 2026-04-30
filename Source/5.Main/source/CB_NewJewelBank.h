#pragma once
#include "Protocol.h"
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
struct USER_BANK_DATA
{
	int ItemIndex;
	int ItemLevel;
	int ItemCount;
	int AutoPick;
	void Clear()
	{
		ItemIndex = -1;
		ItemLevel = 0;
		ItemCount = 0;
		AutoPick = 0;
	}
};
//==== Packer Rut Gui Ngoc
struct REQUEST_RUTGUIBANK
{
	PSBMSG_HEAD header; // C3:F3:03
	BYTE TypeSend;
	short IndexList;
};
class CB_NewJewelBank
{

public:
	CB_NewJewelBank();
	~CB_NewJewelBank();
	void OpenOnOff();
	bool SendItem(ITEM* ItemSell, int Slot, bool KeyClick);
	std::vector<USER_BANK_DATA> mCListItemBank;
	void DrawNhapSoNgoc();
	void DrawWindow();
};

extern CB_NewJewelBank* gCB_NewJewelBank;