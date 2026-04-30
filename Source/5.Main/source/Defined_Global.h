
#ifdef _DEBUG
	#define KWAK_FIX_ALT_KEYDOWN_MENU_BLOCK
#endif // _DEBUG
#ifndef ENCRYPT_STATE
#define ENCRYPT_STATE 1
#endif

//===========Custom Config ========================//

#define GET_ITEMOPT_LEVEL(x)		((x >> 3) & 0xF)
#define SET_ITEMOPT_LEVEL(x)		(x << 3)
#define ITEM_INTER					1171
#define ITEM(x, y)					((x * 512) + y)
#define GET_ITEM(x, y)					((x * 512) + y)
#define ITEM2(x, y)					((x * 512) + y + ITEM_INTER)
//===Config Size
#define TOOLTIP_LEVEL_SIZE							128
#define TOOLTIP_TEXT_SIZE							512
#define TOOLTIP_SIZE									60
#define USE_ITEMTOOLTIP_SS6						1



//================
#define MAX_CUSTOM_COMMAND 100
#define MAX_CUSTOM_MESSAGE							500
#define MAX_CUSTOM_JEWEL							25
#define MAX_POSITION_ITEM							1000
#define MaxLineItemToolTip							3000
#define MAX_CUSTOM_ITEM								2000
#define MAX_CUSTOM_GLOVES							50
#define MAX_CUSTOM_BOW								50
#define MAX_CUSTOM_WING								100
#define MAX_CUSTOMCLOAK								20
#define MAX_CUSTOMCEFFECT							200
#define MAX_DYNAMIC_WING_EFFECT						5000
#define MAX_CUSTOM_WING_EFFECT						5000
//===Monter
#define MAX_CUSTOMMONSTER							100
#define MAX_BOSSCLASS								100
#define MAX_CUSTOM_NPCNAME							100
#define MAX_CUSTOM_MONSTERGLOW						100
#define MAX_EFFECT_MONSTER							1500
//===Pet
#define MAX_PET_ITEM								300
#define MAX_CUSTOPETEFFECT							3000 //Effect
#define MAX_RENDER_MESH								1000 //Glow Color
//==CustomMap
#define MAX_MAP										200
#define WD_ENDMAP									350
#define PI											3.14159265
//===

//======
#define GetGPUUse								1 //Test GPU
#define BTYPECUSTOMSS							6  //Phien Ban Season
#define NEW_MASTER_SKILL_TREE					1	//Su Dung Master Skill


#define ENABLE_EDIT
#define ENABLE_EDIT2

#define DEBUG									0
#define DEBUG_NOTICE							0
#define UseStackLog								1
#define OnOffMasterSkill						1	// Cho phep Dung  SkillTree
#define OnOffPopupMess							0	// Hien Popup Mess Check File
#define WIDE_SCREEN								1	//WIde
#define UseNewMacroUI							0 //Su dung khung toa do mini map moi
#define ShadowText								0 //Name Text Moi
#define ShowLogoGuild							1
#define FixDmgQWORD								0

#define UseReconnect							1
#define HAISLOTRING								1
#define RANKING_NEW								1
#define CUSTOM_BCHOTROI							1
#define CB_AutoBanking							1
#define DANH_HIEU_NEW							1
#define BOSS_GUILD								1
#define CTCMINI									1
#define CB_ANTIHACKGGNEW						0
#define JEWELBANKVER2							1	
#define CUSTOM_BRANKINGNEW						1
#define CB_DANGKYINGAME							1
#define CB_SHOWWCOIN_V							0
#define CB_SHOWCHOTROI_V						0
#define CB_AUTORESETINFO						1
#define CB_VIP_CHAR								1
#define CB_VIEWCHARITEM							1

#define CB_CUSTOMOFFTRADE						1
#define CB_OFFMUHELPER							1
#define CB_GETMIXRATE							1
#define CB_CUSTOMMIXINFO						1
#define CB_BOTTRADER							1
#define DOIMK									1
#define CUSTOM_WINDOWLOCKITEM					1
#define CB_NEWQUESTFF							0
#define CB_HUYDONGEXC							1
#define CB_ActiveInvasionsD						1
#define CB_AUTOLOGINWIN							1
#define CongHuongV2								1
#define CB_ATTACK_HIDEN_PET						1		//
#define CB_ATTACK_HIDEN_WING					1		//
#define SAUDOIITEM								1
#define CB_HPBARNAME							1
#define CB_BXHDMG								1
#define MaNap								1
#define CB_NUMBERCHECKSUM						114749 //Mã CheckSum GS<->Client (giá trị là số không chữ) không dùng thì Set 0


//==Key EncDec Client <--> Server
#define BEncDecKey1				(BYTE)0xF1
#define BEncDecKey2				(BYTE)0x1A

#if(WIDE_SCREEN)
extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
#define GetWindowsX()								(WindowWidth  / g_fScreenRate_x)
#define GetWindowsY()								(WindowHeight / g_fScreenRate_y)
#define setPosCenterX(x)							((GetWindowsX() / 2.f) - (x / 2.f))
#define setPosCenterY(x)							((GetWindowsY() / 2.f) - (x / 2.f) - 51.f)
#define setPosRight(x)								(GetWindowsX() - 640.f + x )
#define setPosDown(x)								(GetWindowsY() - 480.f + x )
#define Position_In_The_Left(x)						(((GetWindowsX) - x ))
#define pos_left(x)									(((float)x))

#else
#define GetWindowsX()								640
#define GetWindowsY()								480
#define setPosCenterX(x)							((GetWindowsX() / 2.f) - (x / 2.f))
#define setPosCenterY(x)							((GetWindowsY() / 2.f) - (x / 2.f) - 51.f)
#define setPosRight(x)								(GetWindowsX() - 640.f + x )
#define setPosDown(x)								(GetWindowsY() - 480.f + x )
#endif

//#define BEncDecKey1				(BYTE)0xE1
//#define BEncDecKey2				(BYTE)0xFA
//=======================

#define RGBTOGLFLOAT(x) ((float)x/255.0f)
#define FLOATTORGB(x)	((float)x*255)
#define ABS(x) (((x) < 0) ? (-(x)) : (x))

//#define NEW_PROTOCOL_SYSTEM

#define LEM_ADD_LUCKYITEM
#define PBG_ADD_NEWCHAR_MONK
#define PBG_ADD_NEWCHAR_MONK_ITEM
#define PBG_ADD_NEWCHAR_MONK_ANI
#define PBG_ADD_NEWCHAR_MONK_SKILL
#define PBG_MOD_RAGEFIGHTERSOUND

#define LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
#define LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#define LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2

#define ASG_ADD_MAP_KARUTAN
#define ASG_ADD_KARUTAN_NPC
#define ASG_ADD_KARUTAN_MONSTERS


//==Test
#define LJW_FIX_MANY_FLAG_DISAPPEARED_PROBREM
#define PARTS_NONE                          0x0


#if (DEBUG==1)
#define ENABLE_EDIT
#define ENABLE_EDIT2
//#define DEBUG_BITMAP_CACHE
#define DEBUG_NOTICE 1	
#endif // _DEBUG

#ifdef FOR_WORK
#if (DEBUG==1)
#define DEBUG_NOTICE	1
#define CSK_LH_DEBUG_CONSOLE
#ifdef CSK_LH_DEBUG_CONSOLE
#define CONSOLE_DEBUG
#endif // CSK_LH_DEBUG_CONSOLE

#define CSK_DEBUG_MAP_ATTRIBUTE

#define CSK_DEBUG_RENDER_BOUNDINGBOX

#define CSK_DEBUG_MAP_PATHFINDING

#endif // _DEBUG
#endif // FOR_WORK
//==========
#define PJH_NEW_SERVER_SELECT_MAP

#define ASG_ADD_GENS_SYSTEM
#ifdef ASG_ADD_GENS_SYSTEM
	#define ASG_ADD_INFLUENCE_GROUND_EFFECT
	#define ASG_ADD_GENS_MARK
	#define PBG_MOD_STRIFE_GENSMARKRENDER
	#define PBG_ADD_GENSRANKING
#endif	// ASG_ADD_GENS_SYSTEM

#ifndef __ANDROID__
#define KJH_PBG_ADD_INGAMESHOP_SYSTEM
#else
#define PBG_ADD_NAMETOPMSGBOX
#define KJH_ADD_PERIOD_ITEM_SYSTEM
#define PBG_ADD_INGAMESHOPMSGBOX
#define PBG_ADD_ITEMRESIZE
#define PBG_ADD_CHARACTERCARD
#endif
#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
	#define PBG_ADD_INGAMESHOP_UI_MAINFRAME
	#define PBG_ADD_INGAMESHOP_UI_ITEMSHOP
	#define PBG_ADD_NAMETOPMSGBOX
	#define KJH_ADD_INGAMESHOP_UI_SYSTEM
	#define KJH_ADD_PERIOD_ITEM_SYSTEM
	#define PBG_ADD_INGAMESHOPMSGBOX
	#define PBG_ADD_ITEMRESIZE
	#define KJH_MOD_SHOP_SCRIPT_DOWNLOAD
	//^#define PBG_ADD_CHARACTERSLOT
	#define PBG_ADD_CHARACTERCARD
	//^#define LDK_ADD_INGAMESHOP_LIMIT_MOVE_WINDOW
#endif //KJH_PBG_ADD_INGAMESHOP_SYSTEM

#define PJH_ADD_PANDA_PET
#define PJH_ADD_PANDA_CHANGERING

#define ASG_ADD_TIME_LIMIT_QUEST_NPC
