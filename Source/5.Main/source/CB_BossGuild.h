#include "CBInterface.h"
#include "Protocol.h"
class CBBossGuild
{
public:
	struct BOSSGUILD_CGPACKET
	{
		PSBMSG_HEAD header; // C3:F3:03
		BYTE CongVao;

	};
	struct INFOBOSSGUILD_SENDCLIENT
	{
		PSBMSG_HEAD header; // C3:F3:03
		BYTE OpenWindow;
	};
	struct BOSSGUILD_UPDATE
	{
		PBMSG_HEAD2 h;
		int CheDo1;
		int CheDo2;
		int KillBoss;
		int Score;
		int WCoinC;
		int PhuThuyWCoinC;
		int ChienBinhWCoinC;
		int TienNuWCoinC;
		int ThuatSiWCoinC;
		int ThietBinhWCoinC;
		int DauSiWCoinC;
		int ChuaTeWCoinC;
	};

	CBBossGuild();
	virtual ~CBBossGuild();
	void DrawWindow();
	void OpenOnOff();
	void RecvProtocol(BYTE* Recv);
	void BossGuildRecv(BYTE* aRecv);
	void DrawBossGuildMap();
	bool CacheForm;

	int CheDo1;
	int CheDo2;
	int KillBoss;
	int Score;
	int WCoinC;
	int PhuThuyWCoinC;
	int ChienBinhWCoinC;
	int TienNuWCoinC;
	int ThuatSiWCoinC;
	int ThietBinhWCoinC;
	int DauSiWCoinC;
	int ChuaTeWCoinC;

	DWORD BossGuild_TimeConLai;
};

extern CBBossGuild* gCBBossGuild;