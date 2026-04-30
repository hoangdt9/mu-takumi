#pragma once

// System Include
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#define MAIN_LANGUAGE 0x00E8C5C4
#define MAIN_CONNECTION_STATUS 0x08793704
#define MAIN_SCREEN_STATE 0x00E609E8
#define MAIN_CHARACTER_STRUCT 0x08128AC8
#define MAIN_VIEWPORT_STRUCT 0x07BC4F04
#define MAIN_PACKET_SERIAL 0x08793700
#define MAIN_HELPER_STRUCT 0x00E8CB7C
#define MAIN_FONT_SIZE 0x081C0380
#define MAIN_RESOLUTION 0x00E8C240
#define MAIN_RESOLUTION_X 0x00E61E58
#define MAIN_RESOLUTION_Y 0x00E61E5C
#define MAIN_PARTY_MEMBER_COUNT 0x081F6B6C

#define ProtocolCore ((BOOL(*)(DWORD,BYTE*,DWORD,DWORD))0x00663B20)
#define DrawInterfaceText ((void(*)(char*,int,int,int,int,int,int,int))0x007D04D0)

#define MAX_CUSTOM_COMMAND					100
#define MAX_CUSTOM_MESSAGE					500
#define MAX_CUSTOM_JEWEL						25
#define MAX_POSITION_ITEM						1000
#define MaxLineItemToolTip						3000
#define MAX_CUSTOM_ITEM						2000
#define MAX_CUSTOM_GLOVES					50
#define MAX_CUSTOM_BOW						50
#define MAX_CUSTOM_WING						100
#define MAX_CUSTOMCLOAK						20
#define MAX_CUSTOMCEFFECT					200
#define MAX_DYNAMIC_WING_EFFECT		5000
#define MAX_CUSTOM_WING_EFFECT			5000
//==Monter
#define MAX_CUSTOMMONSTER					100
#define MAX_BOSSCLASS							100
#define MAX_CUSTOM_NPCNAME				100
#define MAX_CUSTOM_MONSTERGLOW		100
#define MAX_EFFECT_MONSTER					1500
//===Pet
#define MAX_PET_ITEM								300
#define MAX_CUSTOPETEFFECT				   3000 //Effect
#define MAX_RENDER_MESH					   1000 //Glow Color
//==CustomMap
#define MAX_MAP					200
#define WD_81KARUTAN2		81
#define WD_ENDMAP				350

