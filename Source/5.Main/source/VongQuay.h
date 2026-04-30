#pragma once
// CustomHealthBar.h: interface for the CCustomHealthBar class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "Protocol.h"




class CVongQuay
{
public:


	struct PMSG_VONGQUAY_SEND
	{
		PSWMSG_HEAD header; // C2:F3:E2
		BYTE count;
	};

	struct ListVongQuaySend
	{
		int IndexVongQuay;
		char Name[30];
	};

	struct PMSG_YCVONGQUAY_SEND
	{
		PSWMSG_HEAD header; // C2:F3:E2
		BYTE count;
		int IndexYC;
		int CountItem;
		int WCYC;
		int WPYC;
		int GPYC;
	};

	struct LISTITEMVONGQUAY_SENDINFO
	{
		float SizeBMD;
		float PosX;
		float PosY;
		short Index;
		BYTE Dur;
		BYTE Item[12];
		int  PeriodTime;
	};

	//==Struct Client
	struct INFO_VONGQUAY_LOCAL_ITEM
	{
		float SizeBMD;
		float PosX;
		float PosY;
		short Index;
		ITEM* Item;
	};

	struct XULY_CGPACKET_VONGQUAY
	{
		PSBMSG_HEAD header; // C3:F3:03
		DWORD StartRoll;
		DWORD IndexWin;
	};


	struct XULY_CGPACKET_SOLAN
	{
		PSBMSG_HEAD header; // C3:F3:03
		DWORD ThaoTac;
		DWORD SoLan;
	};

	CVongQuay();
	virtual ~CVongQuay();
	void Init();
	void DrawWindowVQ();
	void OpenVongQuay();
	int StartRollSau;
	int IndexItemSau;
	int IndexYC;
	int CountItem;
	int WCYC;
	int WPYC;
	int GPYC;
	std::vector<INFO_VONGQUAY_LOCAL_ITEM> ListItemVongQuay;
	std::vector<ListVongQuaySend> DanhSachVongQuay;
	void GetListVQ(BYTE* Recv);
	void RecvListItemVQ(BYTE* Recv);
	void GetInfoVQ(BYTE* Recv);
private:
	
};

extern CVongQuay gVongQuay;