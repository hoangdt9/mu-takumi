#pragma once

#define WIN32_LEAN_AND_MEAN

#define _WIN32_WINNT _WIN32_WINNT_WINXP

#define GAMESERVER_STATUS "STANDBY MODE"

#define GAMESERVER_STATUS_MODE "ACTIVE MODE"

#define GAMESERVER_HOST 1

#define ISPREMIUN 1

extern int Conectar;

#define GAMESERVER_NOMBRE 1

#if(GAMESERVER_NOMBRE == 0)
	#define GAMESERVER_CLIENT "VIERJA&HIGHLEVEL"
#elif(GAMESERVER_NOMBRE == 1)
	#define GAMESERVER_CLIENT "GameServer xKEN"
#else
#define GAMESERVER_CLIENT "REAL GAMING"
#endif

#define UPDATE_GAMING 15
#define GAMESERVER_CLIENTE_UPDATE 17
#define GAMESERVER_CLIENTE_PREMIUM 0


#if(GAMESERVER_TYPE==0)
#define GAMESERVER_VERSION "GS"
#else
#define GAMESERVER_VERSION "GSCS"
#endif

#ifndef GAMESERVER_TYPE
#define GAMESERVER_TYPE 0
#endif

#ifndef GAMESERVER_EXTRA
#define GAMESERVER_EXTRA 1
#endif

#ifndef GAMESERVER_UPDATE
#define GAMESERVER_UPDATE 803
#endif

#if(GAMESERVER_UPDATE==401)
#define GAMESERVER_SEASON "SEASON 4"
#endif

#if(GAMESERVER_UPDATE==603)
#define GAMESERVER_SEASON "SEASON 6"
#endif

#if(GAMESERVER_UPDATE==803)
#define GAMESERVER_SEASON "SEASON 8"
#endif

#ifndef GAMESERVER_LANGUAGE
#define GAMESERVER_LANGUAGE 1
#endif

#ifndef PROTECT_STATE
#define PROTECT_STATE 1
#endif

#ifndef ENCRYPT_STATE
#define ENCRYPT_STATE 1
#endif

#if(PROTECT_STATE==0)
#define GAMESERVER_NAME "Free"
#else
#define GAMESERVER_NAME "Premium"
#endif

#define CUSTOM_PET					1
#define NEW_FENRIR					1
#define OFFLINE_MODE				1
#define ADD_OFFMODE_PICKUP			0
//MC bot
//----------------------------
//	 ZG-Defines			     -
//	 1:ON  (CPP AND HEADER!) -
//	 2:OFF (CPP AND HEADER!) -
//----------------------------

#ifndef POINT_CUSTOM 
#define POINT_CUSTOM 1 // ShopPointEx
#endif


#define PC_POINT			 2 //Agregar algun npc o bot con PCP
#define HAPPY_HOURS			 2 //[80%]->Falta Arreglar Los Vip!
#define MASTER_HOURS		 2
#define ACHERON_GUARDIAN	 2

#define BOT_BUFFER			 1

#define ARCA_WAR			 2	// Mix nomas!!
#define GOLDEN_ARCHER		 2	// 


//-- Need All Types
#define PG_CUSTOM			 1
#define ALLBOTSSTRUC		 1

//FakeOnline_EMU
#define USE_FAKE_ONLINE		TRUE	

enum TypeNoticeCustom
{
	//eMessagePK = 254,
	eMessagePK = 0xD3 + 1,
	eMessageBox = 255,
};

enum Jewelry
{
	JEWELRY_CHAOS = 0,
	JEWELRY_BLESS = 1,
	JEWELRY_SOUL = 2,
	JEWELRY_LIFE = 3,
	JEWELRY_CREATION = 4,
	JEWELRY_STONE = 5,
	JEWELRY_GUARDYAN = 6,
	JEWELRY_HARMONY = 7,
	JEWELRY_LOWER = 8,
	JEWELRY_HIGHER = 9,
};
enum TYPE_CACHEITEM
{
	CACHE_ITEMCHOTROI,
};
#define FIXBUGCOIN					1
#define DANH_HIEU_NEW				1 
#define CUSTOM_BCHOTROI				1
#define CB_AutoBanking				1
#define JEWELBANKVER2				1
#define CUSTOM_BRANKINGNEW			1
#define CB_DANGKYINGAME				1
#define CB_AUTORESETINFO			1
#define CB_VIP_CHAR					1
#define HAISLOTRING					1
#define FIXTELE						1
#define DOIMK						1
#define CTCMINI						1
#define CUSTOM_WINDOWLOCKITEM		1
#define CB_VIEWCHARITEM				1
#define CB_CUSTOMMIXINFO			1
#define CB_NEWQUESTFF				0
#define CB_HUYDONGEXC				1
#define BOSS_GUILD					1
#define B_RateItemBag				1
#define BOTHUMUADORAC				1
#define CB_BOTTRADE					1
#define CB_ActiveInvasionsD			1
#define CongHuongV2					1
#define LAGBOSS						1
#define CB_ATTACK_HIDEN_PET			1
#define CB_ATTACK_HIDEN_WING		1
#define SAUDOIITEM					1
#define CB_GETMIXRATE				1
#define CB_BXHDMG					1
#define CB_NUMBERCHECKSUM			114749	//Mã CheckSum GS<->Client (giá trị là số không chữ) không dùng thì Set 0

#define SET_DEMO					1 //Che do tesst het han 1/6/2025

#define MOCNAP						1
#define BsvEvent					1

#include <windows.h>
#include <winsock2.h>
#include <mswSock.h>
#include <commctrl.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <map>
#include <vector>
#include <queue>
#include <random>
#include <Rpc.h>
#include <algorithm>
#include <string>
#include <atltime.h>
#include <dbghelp.h>
#include <Psapi.h>

#include "pugixml.hpp"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Rpcrt4.lib")
#pragma comment(lib,"dbghelp.lib")
#pragma comment(lib,"Psapi.lib")

#if(GAMESERVER_UPDATE>=701)
#if(NDEBUG==0)
#pragma comment(lib,"..\\..\\Util\\cryptopp\\Debug\\cryptlib.lib")
#else
#pragma comment(lib,"..\\..\\Util\\cryptopp\\Release\\cryptlib.lib")
#endif
#pragma comment(lib,"..\\..\\Util\\mapm\\mapm.lib")
#endif

typedef char chr;

typedef float flt;

typedef short shrt;

typedef unsigned __int64 QWORD;
