#include "user.h"
#include "Protocol.h"
#pragma once

// -------------------------------------------------------------------------------
struct BUY_DANH_HIEU_REQ
{
	PSBMSG_HEAD h;
	int			Number;
};

struct DANH_HIEU_INFO
{
	int	rCoin;

	int	minlv;
	int	maxlv;

	int	gemcount;
	int rType;
	int rIndex;

	int SatThuong;
	int SatThuongX2;
	int PhongThu;
	int HP;
	int SD;
	int GST;

};


// -------------------------------------------------------------------------------

void RegDataSend(int aIndex);

class DanhHieu
{
public:
	// ----
	DanhHieu();
	virtual ~DanhHieu();
	// ----
	void		BUY_DANH_HIEU(LPOBJ lpObj, BUY_DANH_HIEU_REQ* aRecv);
	// ----
	void		DANH_HIEU_OPTION(LPOBJ lpObj, bool flag);
	// ----
	void	    Load();
	void	Init();
	void	Load1(char* path);
	int MONSTER_ID_01;

	int GetCoint(LPOBJ lpObj);

	int GetCount(LPOBJ lpObj);
	int GetType(LPOBJ lpObj);
	int GetIndex(LPOBJ lpObj);

	int GetMaxLv(LPOBJ lpObj);
	// ---
	int SatThuong(LPOBJ lpObj);
	int SatThuongX2(LPOBJ lpObj);
	int PhongThu(LPOBJ lpObj);
	int HP(LPOBJ lpObj);
	int SD(LPOBJ lpObj);
	int GST(LPOBJ lpObj);
	// ----
	char TitleCheck[64];
	char TitleCheck1[64];
	char TitleCheck2[64];
	char TitleCheck3[64];



	DANH_HIEU_INFO m_DanhHieu;
	void DanhHieu::TinhToanChiSoCapDoDanhHieu(LPOBJ lpObj);

	int Enable;
	int TypeCalc;
private:
	std::vector<DANH_HIEU_INFO> m_DanhHieuInfo;
}; extern DanhHieu gDanhHieu;


// -------------------------------------------------------------------------------