#pragma once
#if DANH_HIEU_NEW
#include "CBInterface.h"
#include "Protocol.h"
class CBDanhHieu
{
public:
	struct BUY_DANH_HIEU_REQ
	{
		PSBMSG_HEAD h;
		int			Number;
	};
	struct DANHHIEU_UPDATE
	{
		PSWMSG_HEAD h;
		int rCoint;
		int rCount;
		int rType;
		int rIndex;
		int SatThuong;
		int SatThuongX2;
		int PhongThu;
		int HP;
		int SD;
		int GST;
		int DANH_HIEU;

	};
	CBDanhHieu();
	virtual ~CBDanhHieu();
	void DrawWindow();
	void OpenWindow();
	void RecvProtocol(BYTE* lpMsg);
	DANHHIEU_UPDATE m_DataDanhHieu;
	int TRANG_DANH_HIEU;
	DWORD ClickTick;
	bool HIENTHIDANH_HIEU;
	void BUY_DANH_HIEU(int Number);
};

extern CBDanhHieu gCBDanhHieu;
#endif