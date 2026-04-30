#pragma once
#include "Protocol.h"

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIButton.h"
#include "NewUIScrollBar.h"
#include "NewUITextBox.h"


using namespace SEASON3B;

class CB_BXHTopDmg
{

public:
	struct DATA_BXH_SEND
	{
		char Name[11];
		QWORD damage;
	};
	struct PMSG_COUNT_SEND_DATA
	{
		PSWMSG_HEAD header; // C2:F3:E2
		BYTE count;
		WORD ClassMonter;
	};

	CB_BXHTopDmg();
	~CB_BXHTopDmg();
	void RecvProtocol(BYTE* Recv);

	int ClassMonter;
	std::vector<CB_BXHTopDmg::DATA_BXH_SEND> m_DataTopBXHDMG;
	void DrawWindowMini();
};

extern CB_BXHTopDmg* gCB_BXHTopDmg;