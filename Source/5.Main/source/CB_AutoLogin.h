#pragma once
#include "Protocol.h"
#if(CB_AUTOLOGINWIN)
#define MAX_ACCOUNT_SAVE					5
class CB_AutoLogin
{
public:
	struct AUTOLOGIN_ACCOUNT
	{
		char ID[11];
		char PW[11];
		int	 index;
	};
	CB_AutoLogin();
	~CB_AutoLogin();
	void SaveData(char* szID, char* szPass);
	void DrawInfo(int XPos, int YPos);
	void ReadConfigs();
	AUTOLOGIN_ACCOUNT	saved_acc[MAX_ACCOUNT_SAVE];
	int	 totalSavedAcc;
	void SetShowListAccount(bool show);
	void SetSelectedAccount(int Index);
	void RemoveAccount(int Index);

	bool showListAccount;
	DWORD TickCount;
	int  selectedAccount;
};

extern CB_AutoLogin* gCB_AutoLogin;
#endif