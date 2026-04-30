#pragma once
#include "DSPlaySound.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIButton.h"
#include "UIControls.h"
#include "Defined_Global.h"
#include "MapManager.h"
#include "UIWindows.h"

typedef void(*PUSHEVENT_CALLBACK_LPVOID)(LPVOID);
#define BMAX_CHAT_SIZE_UTF16  (90/2)
#define Color4f(r, b, g, o)			((o << 24) + (g << 16) + (b << 8) + r)
#define pGameWindow				g_hWnd
extern bool StatusAutoReset;
using namespace SEASON3B;
extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayHeight;
extern int DisplayWinMid;
extern int DisplayWinExt;
extern int DisplayWinReal;

#define MAX_WIN_WIDTH		DisplayWin
#define MAX_WIN_HEIGHT		DisplayHeight

struct InterfaceObject
{
	DWORD	ModelID;
	float	Width;
	float	Height;
	float	X;
	float	Y;
	float	MaxX;
	float	MaxY;
	DWORD	EventTick;
	bool	OnClick;
	bool	 OnShow;
	BYTE	Attribute;
	bool	ByClose;
	long	OpenedValue;
	BYTE	Speed;
	char	StatValue[20];
	int		Type;
	//
	bool	FirstLoad;
	float  curX;
	float	curY;
	bool	Hover;
	bool  AllowMove;
	bool  BActiveHiden;
	void OpenClose(int Type = -1)
	{
		this->OnShow ^= 1;
		this->ByClose ^= 1;
		this->Hover ^= 1;
		if (Type != -1) this->Type = Type;
		if (!this->OnShow) this->Type = 0;

	};
	void Close()
	{
		this->OnShow = false;
		this->ByClose = false;
		this->Hover = false;
		this->Type = 0;
		this->AllowMove = true;
		this->BActiveHiden = false;
	};

	void Open()
	{
		this->OnShow = true;
		this->ByClose = false;
	};

	void Open(int Value, int Speed)
	{
		this->OnShow = true;
		this->OpenedValue = Value;
		this->Speed = Speed;
		this->ByClose = false;
		this->FirstLoad = true;
		this->BActiveHiden = false;
	};

	void Close(int Value, int Speed)
	{
		this->OnShow = false;
		this->OpenedValue = Value;
		this->Speed = Speed;
		this->ByClose = false;
		this->Type = 0;
		this->BActiveHiden = false;
	}

	void CloseAnimated(int Speed)
	{
		this->Speed = Speed;
		this->ByClose = true;
		this->Type = 0;
		this->BActiveHiden = false;
	}


};
enum ColorMacro
{
	eTextShop = Color4f(0xFFu, 0xDCu, 0x96u, 0xFFu),
	eTextGuild = Color4f(0xFFu, 0xBDu, 0x19u, 0xFFu),
	eShinyGreen = Color4f(172, 255, 56, 255),
	eShinyGreen100 = Color4f(172, 255, 56, 100),
	eGold = Color4f(255, 189, 25, 255),
	eGold150 = Color4f(255, 189, 25, 150),
	eGold180 = Color4f(255, 189, 25, 180),
	eGold220 = Color4f(255, 189, 25, 220),
	eWhite = Color4f(255, 255, 255, 255),
	eWhite150 = Color4f(255, 255, 255, 150),
	eWhite160 = Color4f(255, 255, 255, 160),
	eWhite170 = Color4f(255, 255, 255, 170),
	eWhite180 = Color4f(255, 255, 255, 180),
	eWhite190 = Color4f(255, 255, 255, 200),
	eOrange = Color4f(255, 105, 25, 255),
	eGray100 = Color4f(211, 211, 211, 100),
	eGray150 = Color4f(50, 50, 50, 150),
	eGray190 = Color4f(50, 50, 50, 190),
	eBlowPink = Color4f(220, 20, 60, 255),
	eBlack = Color4f(0, 0, 0, 255),
	eRed = Color4f(225, 0, 0, 255),
	eRed2 = Color4f(225, 95, 45, 255),
	eExcellent = Color4f(0, 225, 139, 255),
	eAncient = Color4f(1, 223, 119, 255),
	eSocket = Color4f(153, 102, 204, 255),
	eBlue = Color4f(36, 242, 252, 255),
	eBlue250 = Color4f(36, 242, 252, 250),
	eYellow = Color4f(255, 255, 0, 255),
	eYellow170 = Color4f(255, 255, 0, 170),
	eBrown = Color4f(69, 39, 18, 255),
	eColorAdvance = Color4f(109, 25, 192, 255),
	eColorHealth = Color4f(191, 191, 146, 255),
	eExcellentS15 = Color4f(67, 204, 65, 255),
};
enum ObjectID
{
	eFlag01 = 1,
	eTIME,


	//==Img
	eButtonMXH,
	eButtonNext,
	eButtonPrevious,
	eDanhImg,
	eDanhImgEnd = eDanhImg+50,
	eUICTCImg,
	eUICTCImgEnd = eUICTCImg+8,

	IMG_31617,
	IMG_31618,
	IMG_31764,
	IMG_31765,
#if MaNap
	IMG_4046,
#endif
#if(CB_DANGKYINGAME)
	IMG_71620,
#endif
	IMG_HPBarFrame,
	IMG_HPBarValue,

	//=WindowMenu
	eBeginWindowCustom, //bat dau cac object window
	eMuHelperConfig,
	eMenu_MAIN,
	eMenu_OPTION,
	eMenu,
	eMenuBG,
	eWindowEventTime,
	eVip_MAIN,
	eMAIN_RANKING,
	eWindowChangeClass,
	eWindowMessageBox,

	eWindowChotroi,
	eWindowNhapPass,

	eWindowQRCode,
	eWindowAutoBaking,

	eWindowDanhHieu,
	eWindowJewelBank,

	eBossGuildWindow,

#if CTCMINI
	eCTCMiniTime,
	eCTCMiniWindow,
	eOTHERCHAR_POINT,
	eCHAR_POINT,
#endif
	eStatus_KepChuot,

#if(JEWELBANKVER2)
	eWindowJewelBankRut,
#endif
	eRankPANEL_MAIN,

#if(CB_DANGKYINGAME)
	eWindow_DangKyInGame,
#endif

#if(CB_AUTORESETINFO)
	eWindowAutoReset,
	eWindowRelife,
#endif
#if(CB_VIP_CHAR)
	eWindowVip,
	eWindowVipItem,
#endif
#if(CB_VIEWCHARITEM)
	eWindowViewItemChar,
#endif

#if(CUSTOM_WINDOWLOCKITEM)
	eWindowLockItem,
#endif
#if(DOIMK)
	eWindowDoiMK,
#endif
#if(CB_NEWQUESTFF)
	eWindowNewQuest,
	eWindowNewQuestMini,
#endif
#if(CB_HUYDONGEXC)
	eWindowHuyDongExc,
#endif
#if(CB_BXHDMG)
	eWindowBXHDmg,
#endif
	eActiveInvasionsMain,
	eWindowVongQuay,
	eTickCount,
	eWindow_CongHuong,

	eWindowMocNap,
	eWindowMocNapList,
	eWindowListItemTrader,
	eWindowDrawCustomRank,

#if(SAUDOIITEM)
		eWindowDoiItem,
#endif
	eEndWindowCustom, //ket thuc


	EndObjectCustom,
};

class Interface
{
public:
	struct RECV_CUSTOMTIMECOUNT
	{
		PSBMSG_HEAD header; // C3:F3:03
		BYTE MapNumber;
		BYTE TypeStyle;
		DWORD TimeCount;
		char TextName[60];
	};

	struct DATA_NOTICEPK
	{
		int Delay;
		DWORD	Color;
		DWORD	Time;
		char	Mess[90];
	};

	enum ObjWindow // -> Season 6.3
	{
		FriendList = 1,
		MoveList = 2,
		Party = 3,
		Quest = 4,
		NPC_Devin = 5,
		Guild = 6,
		Trade = 7,
		Warehouse = 8,
		ChaosBox = 9,
		CommandWindow = 10,
		PetInfo = 11,
		Shop = 12,
		Inventory = 13,
		Store = 14,
		OtherStore = 15,
		Character = 16,
		DevilSquare = 19,
		BloodCastle = 20,
		CreateGuild = 21,
		GuardNPC = 22,
		SeniorNPC = 23,
		GuardNPC2 = 24,
		CastleGateSwitch = 25,
		CatapultNPC = 26,
		CrywolfGate = 29,
		IllusionTemple = 30,
		HeroList = 32,
		ChatWindow = 33,
		FastMenu = 34,
		Options = 35,
		Help = 36,
		FastDial = 39,
		SkillTree = 57,
		GoldenArcher1 = 58,
		GoldenArcher2 = 59,
		LuckyCoin1 = 60,
		LuckyCoin2 = 61,
		NPC_Duel = 62,
		NPC_Titus = 63,
		CashShop = 65,
		Lugard = 66,
		QuestList1 = 68,
		QuestList2 = 69,
		Jerint = 70,
		FullMap = 72,
		NPC_Dialog = 73,
		GensInfo = 74,
		NPC_Julia = 75,
		NPC_ChaosMix = 76,
		ExpandInventory = 77,
		ExpandWarehouse = 78,
		MuHelper = 79,
	};
	Interface();
	virtual ~Interface();
	void CloseAllWindowCustom();
	int DrawMessage(int Mode, LPCSTR Text, ...);
	bool CheckWindow(int WindowID);
	int DrawFormat(DWORD Color, int PosX, int PosY, int Width, int Align, LPCSTR Text, ...);
	char* NumberFormat(int Vaule);
	char* fNumberFormat(float number);
	char* QNumberFormat(QWORD number);
	void	    DrawBarForm(float PosX, float PosY, float Width, float Height, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	InterfaceObject Data[EndObjectCustom];
	void Init();
	void Clear();
	void Work();
	void BLoadImage();
	int IndexCustomTextture;
	bool UseHotKey();
	bool ActionSendItemGS(ITEM* ItemSell, int Slot);
	//==Mess Box
	char MsgBoxCaption[128];
	char MsgBoxText[1024];
	int MsgBoxType;
	int MsgLine;
	PUSHEVENT_CALLBACK_LPVOID MsgBoxCallback;
	void DrawMessageBox();
	void OpenMessageBox(char* caption, char* Format, ...);
	void OpenMessageBoxOkCancel(PUSHEVENT_CALLBACK_LPVOID pCallbackFunc, char* caption, char* Format, ...);
	//====
	void		RenderCaptchaNumber(float PosX, float PosY, CUITextInputBox* a6, LPCSTR Text, ...);
	bool		check_Captcha(std::string& captcha, std::string& user_input);
	std::string generateCaptcha(int n);
	std::string vCaptcha;

	void DrawInfoMini();
	void DrawWindowMuaVIP();
	void BRenderTimeCountDown(float x, float y, float w, int MatchTime);
	//==Render Time Count Down
	std::string TextMapTime;
	BYTE NumberMapTime;
	BYTE TypeStyle;
	DWORD MapTime;
	void RecvInfoTime(BYTE* Recv);
	void DrawTimeUseMap();
	int GetTimeGMT();
	SIZE getTextSize(HFONT fontsize, char* a1);
	bool m_Disconnect;
	bool HidenCustom;
	//==

	void DrawNoticePKSystem();
	std::vector<DATA_NOTICEPK> m_DataNoticePK;

	void DrawInfoCommand();
#if(SAUDOIITEM)
	bool SAU_ActiveMix;
	bool SAU_EffectsMix;
	bool SetStateDoiItem;
	BYTE SAU_ItemChinh[16];
	BYTE SAU_ItemPhu[3][16];
	BYTE SAU_ItemKetQua[6][16];
	int RateSAU;
	void DrawDoiItem();
	bool ChangeSSendItem(ITEM* ItemSell, int Slot, bool KeyClick);
#endif
	int RenderHPBarNew(CHARACTER* c);
};

extern Interface gInterface;

#define GetImgRender(x)				(gInterface.Data[x].ModelID)
