#pragma once
#include "Protocol.h"
#if(CUSTOM_WINDOWLOCKITEM)
class CB_LockItem
{

public:
	struct CGPACKET_LOCKWINDOW
	{
		PSBMSG_HEAD header; // C3:F3:03
		DWORD Status;
		DWORD InPass;

	};
	CB_LockItem();
	~CB_LockItem();
	void DrawWindow();
	bool StatusLock;

	void OpenWindowLock();
	void SendLockUnLock(int Pass);
	void RecvGSSatusLockWindow(CGPACKET_LOCKWINDOW* lpMsg);
};

extern CB_LockItem* gCB_LockItem;
#endif