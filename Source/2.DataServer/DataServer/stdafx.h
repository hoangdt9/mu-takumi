#pragma once

#define WIN32_LEAN_AND_MEAN

#define _WIN32_WINNT _WIN32_WINNT_WINXP

#define DATASERVER_VERSION "DS"
#define GAMESERVER_TYPE2 0

#define GAMESERVER_NOMBRE 1

#if(GAMESERVER_NOMBRE == 0)
	#define DATASERVER_CLIENT "VIERJA&HIGHLEVEL"
#elif(GAMESERVER_NOMBRE == 1)
	#define DATASERVER_CLIENT "DataServer - xKEN"
#else
#define DATASERVER_CLIENT "REAL GAMING"
#endif



#ifndef DATASERVER_UPDATE
#define DATASERVER_UPDATE 803
#endif

#ifndef PROTECT_STATE
#define PROTECT_STATE 1
#endif

// System Include
#include <windows.h>
#include <winsock2.h>
#include <commctrl.h>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <process.h>
#include <iostream>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>
#include <math.h>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <Rpc.h>
#include <dbghelp.h>
#include <Psapi.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"Rpcrt4.lib")
#pragma comment(lib,"dbghelp.lib")
#pragma comment(lib,"Psapi.lib")

extern char CustomerName[32];
extern char CustomerHardwareId[36];

typedef unsigned __int64 QWORD;

#define MOCNAP					1
#define DANH_HIEU_NEW			1
#define CUSTOM_BCHOTROI			1
#define CB_AutoBanking			1
#define JEWELBANKVER2			1
#define CUSTOM_BRANKINGNEW		1
#define CB_VIP_CHAR				1
#define HAISLOTRING				1
#define DOIMK					1
#define CB_NEWQUESTFF			0
