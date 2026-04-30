#include "CBInterface.h"
#include "Protocol.h"
class CBCTCMini
{
public:

	struct CTCMINI_CGPACKET
	{
		PSBMSG_HEAD header; // C3:F3:03
		BYTE CongVao;

	};

	struct INFOCTCMINI_SENDCLIENT
	{
		PSBMSG_HEAD header; // C3:F3:03
		BYTE OpenWindow;
		BYTE GuildWinOld;
	};

	struct INFOTVT_SENDCLIENT
	{
		PSBMSG_HEAD header; // C3:F3:03
		BYTE OpenWindow;
	};

	struct CTCMINIKILL_SENDCLIENT
	{
		PSBMSG_HEAD header; // C3:F3:03
		int	TimeCTCMini;
		DWORD ShowAnh;
		BYTE Cong[3];
		BYTE Tru[3];


	};
	CBCTCMini();
	virtual ~CBCTCMini();
	void DrawWindow();
	void OpenOnOff();
	void RecvProtocol(BYTE* Recv);
	void LoadImg();
	void GCCTCMiniShowWindow(BYTE* Recv);
	void GCCTCMiniKill(BYTE* Recv);
	void DrawCTCMiNiMap();
};

extern CBCTCMini gCBCTCMini;