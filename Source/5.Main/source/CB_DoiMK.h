#pragma once
#include "Protocol.h"
#if(DOIMK)
class CB_DoiMK
{

public:
	//=== CHange Pass
	struct CSENDGS_DOIMK
	{
		PSBMSG_HEAD Head; // C3:F3:03
		BYTE	TYPE;
		char PASS_OLD[11];
		char PASS_NEW[11];
	};
	CB_DoiMK();
	~CB_DoiMK();
	void DrawWindow();
	void OpenWindow();
	DWORD TimeSend;
};

extern CB_DoiMK* gCB_DoiMK;
#endif