#pragma once

#include "CBInterface.h"
#include "Protocol.h"
class CBAutoNapGame
{
public:
	struct DataLichSuNapThe
	{
		int TienNap;
		char Checking[30];
		int Status;
	};
	struct LISTCOUNDAUTONAP_CLIENT
	{
		PSWMSG_HEAD header;
		int count;
		int aIndex;
		DWORD TyleNapTien;
		DWORD GiaTriNapThapNhat;
	};

	CBAutoNapGame();
	virtual ~CBAutoNapGame();
	void OpenWindowBanking();
	void DrawQRCodeThanhToan();
	void DrawWindowAutoBanking();
	void RecvDataListThanhToan(BYTE* recv);
	std::vector<DataLichSuNapThe> mCBDataLichSuNapThe;
	DWORD mTyleNapTien;
	DWORD mGiaTriNapThapNhat;
private:
	bool ShowQRAfterInput;
	void ClearData();
};

extern CBAutoNapGame gCBAutoNapGame;
