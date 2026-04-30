// ServerDisplayer.cpp: implementation of the CServerDisplayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerDisplayer.h"
#include "CustomArena.h"
#include "GameMain.h"
#include "Log.h"
#include "Protect.h"
#include "resource.h"
#include "ServerInfo.h"
#include "SocketManager.h"
#include "User.h"
#include "CustomEventTime.h"
CServerDisplayer gServerDisplayer;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServerDisplayer::CServerDisplayer() // OK
{
	
	this->ClearTimeDisplayer();
	this->EventBc = -1;

	for(int n=0;n < MAX_LOG_TEXT_LINE;n++)
	{
		memset(&this->m_log[n],0,sizeof(this->m_log[n]));
	}

	this->m_font = CreateFont(50, 0, 0, 0, FW_DONTCARE, 0, 0, 0, VIETNAMESE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Times");
	this->m_font2 = CreateFont(24, 0, 0, 0, FW_DONTCARE, 0, 0, 0, VIETNAMESE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Times");
	this->m_font3 = CreateFont(15, 0, 0, 0, FW_DONTCARE, 0, 0, 0, VIETNAMESE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");
	this->m_font4 = CreateFont(20, 0, 0, 0, FW_DONTCARE, 0, 0, 0, VIETNAMESE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Times");
	this->m_font5 = CreateFont(15, 0, 0, 0, FW_DONTCARE, 0, 0, 0, VIETNAMESE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");

#if(GAMESERVER_TYPE2 == 0)
	this->m_brush[0] = CreateSolidBrush(RGB(0, 0, 0));
	this->m_brush[1] = CreateSolidBrush(RGB(120, 120, 120));	//rojo
	this->m_brush[2] = CreateSolidBrush(RGB(39, 79, 121));	// 0, 152, 239	//<-
	this->m_brush[3] = CreateSolidBrush(RGB(255, 255, 255));	//semiblack	//<- blanco
	this->m_brush[4] = CreateSolidBrush(RGB(210, 210, 210));	//Black //<- semi blanco
#else
	this->m_brush[0] = CreateSolidBrush(RGB(105, 105, 105));		//<- cuando esta activo
	this->m_brush[1] = CreateSolidBrush(RGB(110, 240, 120));	//<- cuando esta desactivado
	this->m_brush[2] = CreateSolidBrush(RGB(0, 152, 239));		// 0, 152, 239	//<-39, 79, 121
	this->m_brush[3] = CreateSolidBrush(RGB(255, 255, 255));	//semiblack	//<- fondo
	this->m_brush[4] = CreateSolidBrush(RGB(210, 210, 210));	//Black //<- fondo de eventos e informacion
#endif

	strcpy_s(this->m_DisplayerText[0],"STANDBY MODE");
	strcpy_s(this->m_DisplayerText[1],"ACTIVE MODE");
}

CServerDisplayer::~CServerDisplayer() // OK
{
	DeleteObject(this->m_font);
	DeleteObject(this->m_brush[0]);
	DeleteObject(this->m_brush[1]);
	DeleteObject(this->m_brush[2]);
	DeleteObject(this->m_brush[3]);
	DeleteObject(this->m_brush[4]);
}

void CServerDisplayer::Init(HWND hWnd) // OK
{
	PROTECT_START

	this->m_hwnd = hWnd;

	PROTECT_FINAL

	gLog.AddLog(1,"LOG");

	gLog.AddLog(1,"LOG\\LOG_CHAT");

	gLog.AddLog(1,"LOG\\LOG_COMMAND");

	gLog.AddLog(1,"LOG\\LOG_TRADE");

	gLog.AddLog(1,"LOG\\LOG_CONNECT");

	gLog.AddLog(1,"LOG\\LOG_HACK");

	gLog.AddLog(1,"LOG\\LOG_CASH_SHOP");

	gLog.AddLog(1,"LOG\\LOG_CHAOS_MIX");

	gLog.AddLog(1,"LOG\\LOG_ANTIFLOOD");

	gLog.AddLog(1, "LOG\\LOG_BANK_JEWEL");
	gLog.AddLog(1, "LOG\\LOG_THU_MUA");
	gLog.AddLog(1, "LOG\\LOG_ITEMDROPBAG");
	gLog.AddLog(1, "LOG\\LOG_CONGHUONG");
	gLog.AddLog(1, "LOG\\LOG_ITEM_CHARACTER");
	gLog.AddLog(1, "LOG\\LOG_SHOP");
	gLog.AddLog(1, "LOG\\LOG_WCOIN");
	gLog.AddLog(1, "LOG\\LOG_MOCNAP");
	gLog.AddLog(1, "LOG\\LOG_DEBUG");
	gLog.AddLog(1, "LOG\\LOG_ITEM_HomDo");
	gLog.AddLog(1, "LOG\\LOG_TRADEBOT");
}

void CServerDisplayer::Run() // OK
{
	this->LogTextPaint();
	this->PaintName();
	this->SetWindowName();
	this->PaintAllInfo();
	this->PaintOnline();
	//this->PaintPremium();
	this->PaintSeason();
	this->PaintEventTime();
	this->PaintInvasionTime();
	this->PaintCustomArenaTime();
	this->LogTextPaintConnect();
	///this->LogTextPaintGlobalMessage();
}

void CServerDisplayer::SetWindowName() // OK
{
	char buff[256];

	if( Conectar == 0 )
	{
		wsprintf(buff,"[%s] %s (ON: %d) %s",GAMESERVER_VERSION,gServerInfo.m_ServerName, gObjTotalUser,GAMESERVER_CLIENT);
	}
	else
	{
		wsprintf(buff,"[%s] %s (ON: %d) %s   (License Local) %s",GAMESERVER_VERSION,gServerInfo.m_ServerName, gObjTotalUser,GAMESERVER_CLIENT, "https://mumakers.com");
	}

	SetWindowText(this->m_hwnd,buff);

	HWND hWndStatusBar = GetDlgItem(this->m_hwnd, IDC_STATUSBAR);

	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	RECT rect2;

	GetClientRect(hWndStatusBar,&rect2);

	MoveWindow(hWndStatusBar,0,rect.bottom-rect2.bottom+rect2.top,rect.right,rect2.bottom-rect2.top,true);

	int iStatusWidths[] = {190,270,360,450,580, -1};

	char text[256];

	SendMessage(hWndStatusBar, SB_SETPARTS, 6, (LPARAM)iStatusWidths);


	gServerInfo.ProcCheckGHRS();

	wsprintf(text, "%02d/%02d/%04d %02d:%02d:%02d [GHRS %d]", TimeServer->tm_mday, TimeServer->tm_mon, TimeServer->tm_year+1900, TimeServer->tm_hour, TimeServer->tm_min, TimeServer->tm_sec , gServerInfo.GHRSMax);

	SendMessage(hWndStatusBar, SB_SETTEXT, 0,(LPARAM)text);

	wsprintf(text, "OffStore: %d", gObjOffStore);

	SendMessage(hWndStatusBar, SB_SETTEXT, 1,(LPARAM)text);

	wsprintf(text, "OffAttack: %d", gObjOffAttack);

	SendMessage(hWndStatusBar, SB_SETTEXT, 2,(LPARAM)text);

	wsprintf(text, "Bots Buffer: %d", gObjTotalBot);

	SendMessage(hWndStatusBar, SB_SETTEXT, 3,(LPARAM)text);

	wsprintf(text, "Monsters: %d/%d", gObjTotalMonster,MAX_OBJECT_MONSTER);

	SendMessage(hWndStatusBar, SB_SETTEXT, 4,(LPARAM)text);

	SendMessage(hWndStatusBar, SB_SETTEXT, 5,(LPARAM)NULL);

	ShowWindow(hWndStatusBar, SW_SHOW);

}

void CServerDisplayer::PaintAllInfo() // OK activo y desactivado
{
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	long Medida = rect.right - 450;

	Medida = Medida / 3;

	rect.right = rect.right - 450 - Medida - Medida;
	//--
	rect.top = rect.top + 60;

	rect.bottom = rect.top + 25;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font4);
	
	SetTextColor(hdc, RGB(250, 250, 250));

	FillRect(hdc, &rect,this->m_brush[1]);

	if(gJoinServerConnection.CheckState() == 0 || gDataServerConnection.CheckState() == 0)
	{
		DrawText(hdc, GAMESERVER_STATUS, sizeof(GAMESERVER_STATUS), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}
	else
	{
		DrawText(hdc, GAMESERVER_STATUS_MODE, sizeof(GAMESERVER_STATUS_MODE), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	SelectObject(hdc,OldFont);

	SetBkMode(hdc,OldBkMode);

	ReleaseDC(this->m_hwnd,hdc);
}

void CServerDisplayer::PaintOnline() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	long Medida = rect.right - 450;
	long Media = Medida / 2;
	Medida = Medida / 3;

	rect.right = rect.right - 450 - Medida;

	rect.left = Medida + 2;
	//--
	rect.top = rect.top + 60;

	rect.bottom = rect.top + 25;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font4);
	
	SetTextColor(hdc, RGB(250, 250, 250));

	FillRect(hdc, &rect,this->m_brush[1]);

	char text[25];

if( Conectar == 1 )
	if(gServerInfo.m_ServerMaxUserNumber <= 150)
	{
		wsprintf(text, "ONLINE: %d/%d", gObjTotalUser,gServerInfo.m_ServerMaxUserNumber);
	}
	else
	{
		wsprintf(text, "ONLINE: %d/%d", gObjTotalUser,150);
	}
else
{
	wsprintf(text, "ONLINE: %d/%d", gObjTotalUser,gServerInfo.m_ServerMaxUserNumber);
}

	if (gObjTotalUser > 0 )
	{
		TextOut(hdc, Media - 60, rect.top + 2, text, strlen(text));
	}
	else
	{
		TextOut(hdc, Media - 60, rect.top + 2, text, strlen(text));
	}

	SelectObject(hdc,OldFont);

	SetBkMode(hdc,OldBkMode);

	ReleaseDC(this->m_hwnd,hdc);
}

void CServerDisplayer::PaintSeason() // OK Season6
{
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	long Medida = rect.right - 450;

	Medida = Medida / 3;

	rect.right = rect.right - 450;

	rect.left = Medida + Medida + 2;
	//--
	rect.top = rect.top + 60;

	rect.bottom = rect.top + 25;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc, TRANSPARENT);

	HFONT OldFont = (HFONT) SelectObject(hdc, this->m_font4);

	SetTextColor(hdc, RGB(250, 250, 250));
	
	#if(GAMESERVER_NOMBRE == 0)
	FillRect(hdc, &rect, this->m_brush[0]);
	#else
	FillRect(hdc, &rect, this->m_brush[1]);
	#endif

	DrawText(hdc, GAMESERVER_SEASON, sizeof(GAMESERVER_SEASON), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	SelectObject(hdc, OldFont);
	SetBkMode(hdc, OldBkMode);
	ReleaseDC(this->m_hwnd, hdc);
}

void CServerDisplayer::PaintPremium() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	rect.left= rect.right-180;
	rect.top = rect.bottom-65;
	rect.bottom = rect.bottom-20;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font2);
	SetTextColor(hdc,RGB(250,250,250));//this->m_brush[2]
	
#if(GAMESERVER_NOMBRE == 0)
	FillRect(hdc,&rect,this->m_brush[0]);
#else
	FillRect(hdc,&rect,this->m_brush[2]);
#endif
	
if( Conectar == 0 )
{
	TextOut(hdc,rect.right-170,rect.top+14,"PREMIUM",7);
}
else
{
	TextOut(hdc,rect.right-170,rect.top+14,"LOCAL",7);
}
	SelectObject(hdc,OldFont);
	SetBkMode(hdc,OldBkMode);
	ReleaseDC(this->m_hwnd,hdc);
}


void CServerDisplayer::PaintName() // OK nombre
{
	RECT rect;
	GetClientRect(this->m_hwnd,&rect);

	rect.top = 0;

	rect.bottom = 60;

	rect.right= rect.right - 450;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font);

	SetTextColor(hdc,RGB(255,255,255));

	FillRect(hdc,&rect,this->m_brush[2]);

	DrawText(hdc, GAMESERVER_CLIENT, sizeof(GAMESERVER_CLIENT), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	SelectObject(hdc,OldFont);

	SetBkMode(hdc,OldBkMode);
	
	ReleaseDC(this->m_hwnd,hdc);
}

void CServerDisplayer::PaintEventTime() // OK
{
#if(GAMESERVER_TYPE==0)
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	int posX1 = rect.right - 140;
	int posX2 = rect.right - 60;

	rect.left = rect.right - 145;
	rect.right = rect.right - 5;
	rect.top = 0;
	rect.bottom = rect.top + 270;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	FillRect(hdc,&rect,this->m_brush[4]);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font3);

	char text1[20];
	char text2[30];
	int totalseconds;
	int hours;
	int minutes;
	int seconds;
	int days;

	SetTextColor(hdc, RGB(0, 173, 181));
	TextOut(hdc, rect.left + 5, rect.top + 5, "EVENTS:", 7);
	int CountTime = -1;
	for (int i = 0; i < this->CountTimeEventS; i++)
	{
		SetTextColor(hdc, RGB(0, 102, 204));
		if (gCustomEventTime.DataEventTime.empty() || (i + 60) > gCustomEventTime.DataEventTime.size()) break;
		wsprintf(text1, "%d. %s", (i + 60), gCustomEventTime.DataEventTime[i + 60].NameEvent);
		//CountTime = this->EventBc;
		CountTime = *gCustomEventTime.DataEventTime[i + 60].TimeEvent;
		//=
		if (CountTime == -1)
		{
			wsprintf(text2, "OFF");
		}
		else if (CountTime == 0)
		{
			wsprintf(text2, "ON");
		}
		else
		{
			totalseconds = CountTime;
			hours = totalseconds / 3600;
			minutes = (totalseconds / 60) % 60;
			seconds = totalseconds % 60;
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
			if (hours > 23)
			{
				days = hours / 24;
				wsprintf(text2, "%d day(s)+", days);
			}
			else
			{
				wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
			}
		}
		int sizeName = strlen(text1);
		WCHAR text_unicodeName[100];
		int nn = MultiByteToWideChar(CP_UTF8, 0, text1, sizeName, text_unicodeName, 100);
		if (nn > 1)
		{
			TextOutW(hdc, posX1, rect.top + 2 + 15 + (15 * i), text_unicodeName, nn);
	}
		//TextOut(hdc, posX1, rect.top + 2 + 15 + (15 * i), text1, strlen(text1));
		if (CountTime == -1)
		{
			SetTextColor(hdc, RGB(255, 0, 0));
		}
		else if (CountTime == 0)
		{
			SetTextColor(hdc, RGB(0, 190, 0));
		}
		else if (CountTime < 300)
		{
			SetTextColor(hdc, RGB(0, 190, 0));
		}
		else
		{

			SetTextColor(hdc, RGB(0, 0, 0));
		}
		TextOut(hdc, posX2, rect.top + 2 + 15 + (15 * i), text2, strlen(text2));
		if ((rect.top + 2 + 15 + (15 * i)) >= rect.bottom - (20)) goto Exit;
}
Exit:

	SelectObject(hdc,OldFont);
	SetBkMode(hdc,OldBkMode);
	ReleaseDC(this->m_hwnd,hdc);
	
#endif
}

void CServerDisplayer::PaintInvasionTime() // OK
{
#if(GAMESERVER_TYPE==0)
	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	int posX1 = rect.right - 430;// vi tri 1
	int posX2 = rect.right - 200;// vi tri cot 2

	rect.left = rect.right - 450;
	rect.right = rect.right - 150;

	rect.top = 0;
	rect.bottom = rect.top + 435;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	FillRect(hdc,&rect,this->m_brush[4]);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font3);

	char text1[120];
	char text2[30];
	int totalseconds;
	int hours;
	int minutes;
	int seconds;
	int days;

	SetTextColor(hdc, RGB(30, 173, 181));
	TextOut(hdc, rect.left + 100, rect.top + 2, "INVASION:", 9);
	int CountTime = -1;
	for (int i = 0; i < 30; i++)
	{
		if (gCustomEventTime.DataEventTime.empty() || i > gCustomEventTime.DataEventTime.size()) break;
		if (strlen(gCustomEventTime.DataEventTime[i].NameEvent) < 1) continue;
		SetTextColor(hdc, RGB(0, 102, 204));
		wsprintf(text1, "%d. %s", i, gCustomEventTime.DataEventTime[i].NameEvent);
		//CountTime = this->EventBc;
		CountTime = *gCustomEventTime.DataEventTime[i].TimeEvent;
		//=
		if (CountTime == -1)
		{
			wsprintf(text2, "OFF");
		}
		else if (CountTime == 0)
		{
			wsprintf(text2, "ON");
		}
		else
		{
			totalseconds = CountTime;
			hours = totalseconds / 3600;
			minutes = (totalseconds / 60) % 60;
			seconds = totalseconds % 60;
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
			if (hours > 23)
			{
				days = hours / 24;
				wsprintf(text2, "%d day+", days);
			}
			else
			{
				wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
			}
		}
		int sizeName = strlen(text1);
		WCHAR text_unicodeName[120];
		int nn = MultiByteToWideChar(CP_UTF8, 0, text1, sizeName, text_unicodeName, 100);
		if (nn > 1)
		{
			TextOutW(hdc, posX1, rect.top + 2 + 15 + (15 * i), text_unicodeName, nn);
		}
		if (CountTime == -1)
		{
			SetTextColor(hdc, RGB(255, 0, 0));
		}
		else if (CountTime == 0)
		{
			SetTextColor(hdc, RGB(0, 190, 0));
		}
		else if (CountTime < 300)
		{
			SetTextColor(hdc, RGB(0, 190, 0));
		}
		else
		{

			SetTextColor(hdc, RGB(0, 0, 0));
		}
		TextOut(hdc, posX2, rect.top + 2 + 15 + (15 * i), text2, strlen(text2));
		if ((rect.top + 2 + 15 + (15 * i)) >= rect.bottom - (20)) goto Exit;
	}
Exit:
	SelectObject(hdc,OldFont);
	SetBkMode(hdc,OldBkMode);
	ReleaseDC(this->m_hwnd,hdc);
#endif
}

void CServerDisplayer::PaintCustomArenaTime() // OK
{
#if(GAMESERVER_TYPE==0)
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	int posX1 = rect.right - 140;
	int posX2 = rect.right - 60;

	rect.left = rect.right - 145;
	rect.right = rect.right - 5;
	rect.top = 300;
	rect.bottom = rect.top + 200;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	FillRect(hdc,&rect,this->m_brush[4]);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font3);

	char text1[20];
	char text2[30];
	int totalseconds;
	int hours;
	int minutes;
	int seconds;
	int days;

	SetTextColor(hdc,RGB(0, 173, 181));
	TextOut(hdc,rect.left+5,rect.top+2,"CUSTOM ARENA:",13);

	int CountTime = -1;
	for (int i = 0; i < 30; i++)
	{

		if (gCustomEventTime.DataEventTime.empty() || i + 30 > gCustomEventTime.DataEventTime.size()) break;
		if (strlen(gCustomEventTime.DataEventTime[i + 30].NameEvent) < 1) continue;
		SetTextColor(hdc, RGB(0, 102, 204));
		wsprintf(text1, "%d. %s", i + 30, gCustomEventTime.DataEventTime[i + 30].NameEvent);
		//CountTime = this->EventBc;
		CountTime = *gCustomEventTime.DataEventTime[i + 30].TimeEvent;
		//=
		if (CountTime == -1)
		{
			wsprintf(text2, "OFF");
		}
		else if (CountTime == 0)
		{
			wsprintf(text2, "ON");
		}
		else
		{
			totalseconds = CountTime;
			hours = totalseconds / 3600;
			minutes = (totalseconds / 60) % 60;
			seconds = totalseconds % 60;
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
			if (hours > 23)
			{
				days = hours / 24;
				wsprintf(text2, "%d day+", days);
			}
			else
			{
				wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
			}
		}
		int sizeName = strlen(text1);
		WCHAR text_unicodeName[100];
		int nn = MultiByteToWideChar(CP_UTF8, 0, text1, sizeName, text_unicodeName, 100);
		if (nn > 1)
		{
			TextOutW(hdc, posX1, rect.top + 2 + 15 + (15 * i), text_unicodeName, nn);
		}
		if (CountTime == -1)
		{
			SetTextColor(hdc, RGB(255, 0, 0));
		}
		else if (CountTime == 0)
		{
			SetTextColor(hdc, RGB(0, 190, 0));
		}
		else if (CountTime < 300)
		{
			SetTextColor(hdc, RGB(0, 190, 0));
		}
		else
		{

			SetTextColor(hdc, RGB(0, 0, 0));
		}
		TextOut(hdc, posX2, rect.top + 2 + 15 + (15 * i), text2, strlen(text2));
		if ((rect.top + 2 + 15 + (15 * i)) >= rect.bottom - (20)) goto Exit;
	}
Exit:
	SelectObject(hdc,OldFont);
	SetBkMode(hdc,OldBkMode);
	ReleaseDC(this->m_hwnd,hdc);

#else

	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	int posX1 = rect.right - 140;
	int posX2 = rect.right - 60;

	rect.left = rect.right - 145;
	rect.right = rect.right - 5;
	rect.top = 300;
	rect.bottom = rect.top + 200;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	FillRect(hdc,&rect,this->m_brush[4]);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font3);

	char text1[20];
	char text2[30];
	char text3[30];
	char text4[30];
	int totalseconds;
	int hours;
	int minutes;
	int seconds;
	int days;

	SetTextColor(hdc,RGB(0, 173, 181));
	TextOut(hdc,rect.left+5,rect.top + 2,"EVENTS:",7);

	SetTextColor(hdc,RGB(0,102,204));
	wsprintf(text1, "Loren Deep: ");

	if (this->EventCastleDeep == -1)
	{
		wsprintf(text2, "OFF");
	}
	else if (this->EventCastleDeep == 0)
	{
		wsprintf(text2, "ON");
	}
	else
	{
		totalseconds	= this->EventCastleDeep;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;

		if (hours > 23)
		{
			days = hours/24;
			wsprintf(text2, "%d day(s)+", days);
		}
		else
		{
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
		}
	}

	TextOut(hdc, posX1, 30, text1, strlen(text1));
	if (this->EventCastleDeep == -1)
	{
		SetTextColor(hdc,RGB(255,0,0));
	}
	else if (this->EventCastleDeep == 0)
	{
		SetTextColor(hdc,RGB(0,190,0));
	}
	else if (this->EventCastleDeep < 300)
	{
		SetTextColor(hdc,RGB(0,190,0));
	}
	else
	{
		SetTextColor(hdc,RGB(0, 0, 0));
	}
	TextOut(hdc, posX2, 30, text2, strlen(text2));

	SetTextColor(hdc,RGB(0,102,204));
	wsprintf(text1, "CryWolf: ");

	if (this->EventCryWolf == -1)
	{
		wsprintf(text2, "OFF");
	}
	else if (this->EventCryWolf == 0)
	{
		wsprintf(text2, "ON");
	}
	else
	{
		totalseconds	= this->EventCryWolf;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;

		if (hours > 23)
		{
			days = hours/24;
			wsprintf(text2, "%d day(s)+", days);
		}
		else
		{
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
		}
	}

	TextOut(hdc, posX1, 50, text1,strlen(text1));
	if (this->EventCryWolf == -1)
	{
		SetTextColor(hdc,RGB(255,0,0));
	}
	else if (this->EventCryWolf == 0)
	{
		SetTextColor(hdc,RGB(0,190,0));
	}
	else if (this->EventCryWolf < 300)
	{
		SetTextColor(hdc,RGB(0,190,0));
	}
	else
	{
#if(GAMESERVER_NOMBRE == 1)
					SetTextColor(hdc, RGB(0,0,0));
#else
					SetTextColor(hdc, RGB(255,255,255));
#endif
	}
	TextOut(hdc, posX2, 50,text2,strlen(text2));

	SetTextColor(hdc,RGB(0,102,204));
	wsprintf(text1, "Castle Siege: ");

	if (this->EventCs == -1)
	{
		wsprintf(text2, "OFF");
	}
	else if (this->EventCs == 0)
	{
		wsprintf(text2, "Adjust Date");
		wsprintf(text3, " ");
		wsprintf(text4, " ");
	}
	else
	{
		totalseconds	= this->EventCs;
		hours			= totalseconds/3600;
		minutes			= (totalseconds/60) % 60;  
		seconds			= totalseconds % 60;

		if (hours > 23)
		{
			days = hours/24;
			wsprintf(text4, "- Next Stage: %d day(s)+", days);
		}
		else
		{
			wsprintf(text4, "- Next Stage: %02d:%02d:%02d", hours, minutes, seconds);
		}

		if(this->EventCsState == -1)
			wsprintf(text3, "- Stage %d: None", this->EventCsState);
		if(this->EventCsState == 0)
			wsprintf(text3, "- Stage %d: Idle 1", this->EventCsState);
		if(this->EventCsState == 1)
			wsprintf(text3, "- Stage %d: Guild Register", this->EventCsState);
		if(this->EventCsState == 2)
			wsprintf(text3, "- Stage %d: Idle 2", this->EventCsState);
		if(this->EventCsState == 3)
			wsprintf(text3, "- Stage %d: Mark Register", this->EventCsState);
		if(this->EventCsState == 4)
			wsprintf(text3, "- Stage %d: Idle 3", this->EventCsState);
		if(this->EventCsState == 5)
			wsprintf(text3, "- Stage %d: Notify", this->EventCsState);
		if(this->EventCsState == 6)
			wsprintf(text3, "- Stage %d: Ready Siege", this->EventCsState);
		if(this->EventCsState == 7)
			wsprintf(text3, "- Stage %d: Started Siege", this->EventCsState);
		if(this->EventCsState == 8)
			wsprintf(text3, "- Stage %d: Ended Siege", this->EventCsState);
		if(this->EventCsState == 9)
			wsprintf(text3, "- Stage %d: End All", this->EventCsState);
			
		if (this->EventCs)
			wsprintf(text2, "Stage %d", this->EventCsState);
	}

	TextOut(hdc, posX1, 70, text1, strlen(text1));
	if (this->EventCs == -1)
	{
		SetTextColor(hdc,RGB(255,0,0));
	}
	else
	{
#if(GAMESERVER_NOMBRE == 1)
					SetTextColor(hdc, RGB(0,0,0));
#else
					SetTextColor(hdc, RGB(255,255,255));
#endif
	}
	TextOut(hdc, posX2, 70, text2, strlen(text2));

	TextOut(hdc,posX1+5,155,text3,strlen(text3));

	TextOut(hdc,posX1+5,175,text4,strlen(text4));

	SelectObject(hdc,OldFont);
	SetBkMode(hdc,OldBkMode);
	ReleaseDC(this->m_hwnd,hdc);

#endif

}

void CServerDisplayer::LogTextPaint() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	FillRect(hdc,&rect,this->m_brush[3]);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font3);

	int line = MAX_LOG_TEXT_LINE;

	int count = (((this->m_count-1)>=0)?(this->m_count-1):(MAX_LOG_TEXT_LINE-1));

	for(int n=0;n < MAX_LOG_TEXT_LINE;n++)
	{
		switch(this->m_log[count].color)
		{
			case LOG_BLACK:
#if(GAMESERVER_NOMBRE == 1)
			SetTextColor(hdc, RGB(0, 0, 0));
#else
					SetTextColor(hdc,RGB(255,255,255));
#endif
				break;
			case LOG_RED:
				SetTextColor(hdc,RGB(239,0,0));
				break;
			case LOG_GREEN:
				SetTextColor(hdc,RGB(0,255,0));
				break;
			case LOG_BLUE:
				SetTextColor(hdc,RGB(0, 152, 239));
				break;
			case LOG_BOT:
				SetTextColor(hdc,RGB(255,0,64));
				break;
			case LOG_USER:
				SetTextColor(hdc,RGB(254,154,46));
				break;
			case LOG_EVENT:
				SetTextColor(hdc,RGB(0,102,204));
				break;
			case LOG_ALERT:
				SetTextColor(hdc,RGB(0, 173, 181));
				break;
		}

		int size = strlen(this->m_log[count].text);

		WCHAR text_unicode[100];

		int nn = MultiByteToWideChar(CP_UTF8, 0, this->m_log[count].text, size, text_unicode, 100);

		if (nn > 1)
		{
			TextOutW(hdc, 0, (37 + (line * 15)), text_unicode, nn);
			line--;
		}

		//if(size > 1)
		//
		//	
		//{
		//	TextOut(hdc,0,(37+(line*15)),this->m_log[count].text,size);
		//	line--;
		//}

		count = (((--count)>=0)?count:(MAX_LOG_TEXT_LINE-1));
	}

	SelectObject(hdc,OldFont);
	ReleaseDC(this->m_hwnd,hdc);
}

void CServerDisplayer::LogTextPaintConnect() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd,&rect);

	rect.left	= rect.right - 450;
	rect.right	= rect.right-150;
	rect.top = rect.bottom-245;
	rect.bottom = rect.bottom-20;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc,TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font3);

	FillRect(hdc,&rect,this->m_brush[4]);

	SetTextColor(hdc,RGB(0, 173, 181));
	TextOut(hdc,rect.left+5,rect.top+2,"CONNECTION LOG:",15);

	int line = MAX_LOGCONNECT_TEXT_LINE;

	int count = (((this->m_countConnect-1)>=0)?(this->m_countConnect-1):(MAX_LOGCONNECT_TEXT_LINE-1));

	for(int n=0;n < MAX_LOGCONNECT_TEXT_LINE;n++)
	{
		switch(this->m_logConnect[count].color)
		{
			case LOG_BLACK:
#if(GAMESERVER_NOMBRE == 1)
					SetTextColor(hdc,RGB(0,0,0));
#else
					SetTextColor(hdc,RGB(255,255,255));
#endif
				break;
			case LOG_RED:
				SetTextColor(hdc,RGB(255,0,0));
				break;
			case LOG_GREEN:
				SetTextColor(hdc,RGB(0,190,0));
				break;
			case LOG_BLUE:
				SetTextColor(hdc,RGB(0,0,255));
				break;
			case LOG_BOT:
				SetTextColor(hdc,RGB(255,0,64));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_USER:
				SetTextColor(hdc,RGB(254,154,46));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_EVENT:
				SetTextColor(hdc,RGB(0,102,204));
				SetBkMode(hdc,TRANSPARENT);
				break;
			case LOG_ALERT:
				SetTextColor(hdc,RGB(0, 173, 181));
				SetBkMode(hdc,TRANSPARENT);
				break;
		}

		int size = strlen(this->m_logConnect[count].text);

		if(size > 1)
		{
			TextOut(hdc,rect.left+10,(rect.top+5+(line*15)),this->m_logConnect[count].text,size);
			line--;
		}

		count = (((--count)>=0)?count:(MAX_LOGCONNECT_TEXT_LINE-1));
	}

	SelectObject(hdc,OldFont);
	SetBkMode(hdc,OldBkMode);
	ReleaseDC(this->m_hwnd,hdc);
}

//void CServerDisplayer::LogTextPaintGlobalMessage() // OK
//{
//	RECT rect;
//
//	GetClientRect(this->m_hwnd, &rect);
//
//	rect.left	= rect.right - 450;
//	rect.right	= rect.right-150;
//	rect.top = 295;
//	rect.bottom = 440;
//
//	HDC hdc = GetDC(this->m_hwnd);
//
//	int OldBkMode = SetBkMode(hdc, TRANSPARENT);
//
//	HFONT OldFont = (HFONT)SelectObject(hdc,this->m_font3);
//
//	FillRect(hdc,&rect,this->m_brush[4]);
//
//	SetTextColor(hdc,RGB(0, 173, 181));
//
//	TextOut(hdc,rect.left+5,rect.top+2,"GLOBAL MESSAGE:",15);
//
//	int line = MAX_LOGGLOBAL_TEXT_LINE;
//
//	int count = (((this->m_countGlobal-1)>=0)?(this->m_countGlobal-1):(MAX_LOGGLOBAL_TEXT_LINE-1));
//
//	for(int n=0;n < MAX_LOGGLOBAL_TEXT_LINE;n++)
//	{
//
//		SetTextColor(hdc,RGB(0,190,0));
//
//		int size = strlen(this->m_logGlobal[count].text);
//
//		if(size > 1)
//		{
//			TextOut(hdc,rect.left+10,(rect.top+5+(line*15)),this->m_logGlobal[count].text,size);
//			line--;
//		}
//
//		count = (((--count)>=0)?count:(MAX_LOGGLOBAL_TEXT_LINE-1));
//	}
//
//	SelectObject(hdc,OldFont);
//	SetBkMode(hdc,OldBkMode);
//	ReleaseDC(this->m_hwnd,hdc);
//}

void CServerDisplayer::LogAddText(eLogColor color,char* text,int size) // OK
{
	PROTECT_START

	size = ((size>=MAX_LOG_TEXT_SIZE)?(MAX_LOG_TEXT_SIZE-1):size);

	memset(&this->m_log[this->m_count].text,0,sizeof(this->m_log[this->m_count].text));

	memcpy(&this->m_log[this->m_count].text,text,size);

	this->m_log[this->m_count].color = color;

	this->m_count = (((++this->m_count)>=MAX_LOG_TEXT_LINE)?0:this->m_count);

	PROTECT_FINAL

	gLog.Output(LOG_GENERAL,"%s",&text[9]);
}

void CServerDisplayer::LogAddTextConnect(eLogColor color,char* text,int size) // OK
{
	PROTECT_START

	size = ((size>=MAX_LOGCONNECT_TEXT_SIZE)?(MAX_LOGCONNECT_TEXT_SIZE-1):size);

	memset(&this->m_logConnect[this->m_countConnect].text,0,sizeof(this->m_logConnect[this->m_countConnect].text));

	memcpy(&this->m_logConnect[this->m_countConnect].text,text,size);

	this->m_logConnect[this->m_countConnect].color = color;

	this->m_countConnect = (((++this->m_countConnect)>=MAX_LOGCONNECT_TEXT_LINE)?0:this->m_countConnect);

	PROTECT_FINAL

	gLog.Output(LOG_GENERAL,"%s",&text[9]);
}

void CServerDisplayer::LogAddTextGlobal(eLogColor color,char* text,int size) // OK
{
	PROTECT_START

	size = ((size>=MAX_LOGGLOBAL_TEXT_SIZE)?(MAX_LOGGLOBAL_TEXT_SIZE-1):size);

	memset(&this->m_logGlobal[this->m_countGlobal].text,0,sizeof(this->m_logGlobal[this->m_countGlobal].text));

	memcpy(&this->m_logGlobal[this->m_countGlobal].text,text,size);

	this->m_logGlobal[this->m_countGlobal].color = color;

	this->m_countGlobal = (((++this->m_countGlobal)>=MAX_LOGGLOBAL_TEXT_LINE)?0:this->m_countGlobal);

	PROTECT_FINAL
}
void CServerDisplayer::ClearTimeDisplayer()
{
	this->CountTimeEventS = 0;
	gServerDisplayer.EventBc = -1;
	gServerDisplayer.EventDs = -1;
	gServerDisplayer.EventCc = -1;
	gServerDisplayer.EventIt = -1;
	gServerDisplayer.EventCustomLottery = -1;
	gServerDisplayer.EventCustomQuiz = -1;
	gServerDisplayer.EventCustomBonus = -1;
	gServerDisplayer.EventKing = -1;
	gServerDisplayer.EventTvT = -1;
	gServerDisplayer.EventMoss = -1;
	gServerDisplayer.EventBossGuild = -1;
	gServerDisplayer.EventCTCMini = -1;
	gServerDisplayer.EventLoanChien = -1;

}
void CServerDisplayer::InitTimeEvent()
{
	gCustomEventTime.DataEventTime.clear();
	gCustomEventTime.LoadDataTime = 0;
	gCustomEventTime.LoadData();
	//=== add data Event time
	gCustomEventTime.AddDataEventsTime("Blood Castle", &this->EventBc); this->CountTimeEventS++;
	gCustomEventTime.AddDataEventsTime("Devil Square ", &this->EventDs); this->CountTimeEventS++;
	gCustomEventTime.AddDataEventsTime("Chaos Castle ", &this->EventCc); this->CountTimeEventS++;
	gCustomEventTime.AddDataEventsTime("Illus Temple ", &this->EventIt); this->CountTimeEventS++;
	gCustomEventTime.AddDataEventsTime("Online Lottery ", &this->EventCustomLottery); this->CountTimeEventS++;
	gCustomEventTime.AddDataEventsTime("Custom Quiz", &this->EventCustomQuiz); this->CountTimeEventS++;
	gCustomEventTime.AddDataEventsTime("Custom Bonus", &this->EventCustomBonus); this->CountTimeEventS++;
	gCustomEventTime.AddDataEventsTime("Custom Drop", &this->EventDrop); this->CountTimeEventS++;
	gCustomEventTime.AddDataEventsTime("King Of Mu", &this->EventKing); this->CountTimeEventS++;
	gCustomEventTime.AddDataEventsTime("Tvt Event", &this->EventTvT); this->CountTimeEventS++;
	gCustomEventTime.AddDataEventsTime("Moss Merch", &this->EventMoss); this->CountTimeEventS++;
	gCustomEventTime.AddDataEventsTime("Boss Guild", &this->EventBossGuild); this->CountTimeEventS++;
	gCustomEventTime.AddDataEventsTime("CTC Mini", &this->EventCTCMini); this->CountTimeEventS++;
	gCustomEventTime.AddDataEventsTime("Loan Chien", &this->EventLoanChien); this->CountTimeEventS++;

}