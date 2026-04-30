#pragma once
#include "Protocol.h"
class CB_Mapping
{

public:

	CB_Mapping();
	virtual ~CB_Mapping();
	void Update();
	void Init();
	DWORD MainNumber;
	bool CheckConnectDLL;

	int CacheIndex;

};
extern CB_Mapping* gCB_Mapping;