// TrayMode.cpp: implementation of the CTrayMode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <ShellAPI.h>
#include "TrayMode.h"
#include "Protect.h"
#include "CharacterManager.h"
#include "resource.h"

HWND            Window = NULL;
CTrayMode gTrayMode;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTrayMode::CTrayMode() // OK
{
}

void CTrayMode::Init(HINSTANCE hins) // OK
{
	this->m_TrayIcon = LoadIcon(hins, (LPCTSTR)IDI_ICON1);

	this->m_MainWndProc = 0;
}

void CTrayMode::Toggle() // OK
{
	//Window = FindWindow(gProtect.m_MainInfo.WindowName, NULL);
	Window = g_hWnd;

	if (this->GetMainWndProc() != 0)
	{
		if (IsWindowVisible(Window) == 0)
		{
			ShowWindow(Window, SW_SHOW);

			this->ShowNotify(0);
		}
		else
		{
			ShowWindow(Window, SW_HIDE);

			this->ShowNotify(1);
		}
	}
}

LONG CTrayMode::GetMainWndProc() // OK
{
	//Window = FindWindow(gProtect.m_MainInfo.WindowName, NULL);
	Window = g_hWnd;

	return ((this->m_MainWndProc == 0) ? ((LONG)(this->m_MainWndProc = (WNDPROC)SetWindowLong(Window, GWL_WNDPROC, (LONG)CTrayMode::TrayModeWndProc))) : ((LONG)this->m_MainWndProc));
}

void CTrayMode::ShowNotify(bool mode) // OK
{
	//Window = FindWindow(gProtect.m_MainInfo.WindowName, NULL);
	Window = g_hWnd;

	NOTIFYICONDATA nid;

	memset(&nid, 0, sizeof(nid));

	nid.cbSize = sizeof(NOTIFYICONDATA);

	nid.hWnd = Window;

	nid.uID = WM_TRAY_MODE_ICON;

	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

	nid.uCallbackMessage = WM_TRAY_MODE_MESSAGE;

	nid.hIcon = this->m_TrayIcon;

	if (SceneFlag == 5)
	{
		wsprintf(nid.szTip, "Char: (%s)\nResets: (%d)\nLevel: (%d)", CharacterAttribute->Name, CharacterAttribute->PrintPlayer.ViewReset, CharacterAttribute->Level);
	}
	else
	{
		wsprintf(nid.szTip, "MuOnline");
	}

	Shell_NotifyIcon(((mode == 0) ? NIM_DELETE : NIM_ADD), &nid);
}

LRESULT CALLBACK CTrayMode::TrayModeWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) // OK
{
	switch (message)
	{
	case WM_TRAY_MODE_MESSAGE:
		switch (lParam)
		{
		case WM_LBUTTONDBLCLK:
			gTrayMode.Toggle();
			break;
		default:
			break;
		}
	default:
		break;
	}

	return CallWindowProc((WNDPROC)gTrayMode.GetMainWndProc(), hWnd, message, wParam, lParam);
}
