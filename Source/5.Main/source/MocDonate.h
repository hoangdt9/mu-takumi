#include "CBInterface.h"
#include "Protocol.h"
#define max(a,b)            (((a) > (b)) ? (a) : (b))
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

struct INFO_LOCAL_ITEM
{
	float SizeBMD;
	int Count;
	short Index;
	BYTE Item[12];
};
struct CBLISTPHANTHUONGMOCNAP_CLIENT
{
	int WC;
	int WP;
	int GP;
	int Ruud;
	std::vector<INFO_LOCAL_ITEM> ListItemMocNap;

	void Clear()
	{
		WC = 0;
		WP = 0;
		GP = 0;
		Ruud = 0;
		ListItemMocNap.clear();
	}
};
struct CBINFOMOCNAP_CLIENT
{
	int NhanMocNap;
	int TongNap;
	std::vector<ListMocNapSend> DanhSachMocNap;
	void Clear()
	{
		NhanMocNap = 0;
		TongNap = 0;
		DanhSachMocNap.clear();
	}
};


class MocDonate
{
public:
	
	MocDonate();
	virtual ~MocDonate();
	void DrawWindow();
	void DrawXemMocNap();
	void OpenWindowMocNap();
	void GetListDonate(BYTE* Recv);
	void RecvListPhanThuong(BYTE* Recv);

	CBINFOMOCNAP_CLIENT mDataMocNapClient;
	CBLISTPHANTHUONGMOCNAP_CLIENT mDataListItemMocNapClient;
};

extern MocDonate gMocDonate;