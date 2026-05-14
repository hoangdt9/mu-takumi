//*****************************************************************************
// File: LoginMainWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "LoginMainWin.h"

#include "Input.h"
#include "UIMng.h"

#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "DSPlaySound.h"

#include "ZzzCharacter.h"
#include "wsclientinline.h"

#if defined(__ANDROID__)
#include <unistd.h>
#include "Platform/MobilePlatform.h"
#endif

#ifdef MOVIE_DIRECTSHOW
#include <dshow.h>
#include "MovieScene.h"

extern CMovieScene* g_pMovieScene;
#endif // MOVIE_DIRECTSHOW

extern char* g_lpszMp3[NUM_MUSIC];

CLoginMainWin::CLoginMainWin()
{

}

CLoginMainWin::~CLoginMainWin()
{

}

void CLoginMainWin::Create()
{
	for (int i = 0; i <= LMW_BTN_CREDIT; ++i)
		m_aBtn[i].Create(54, 30, BITMAP_LOG_IN+4 + i, 3, 2, 1);
#if defined(MOVIE_DIRECTSHOW) || defined(__ANDROID__)
	m_aBtn[LMW_BTN_MOVIE].Create(54, 30, BITMAP_LOG_IN+15, 3, 2, 1);
#endif

	CWin::Create(CInput::Instance().GetScreenWidth() - 30 * 2,
		m_aBtn[0].GetHeight(), -2);

	for (int i = 0; i < LMW_BTN_MAX; ++i)
		CWin::RegisterButton(&m_aBtn[i]);

	m_sprDeco.Create(189, 103, BITMAP_LOG_IN+6, 0, NULL, 105, 59);
}

void CLoginMainWin::PreRelease()
{
	m_sprDeco.Release();
}

void CLoginMainWin::SetPosition(int nXCoord, int nYCoord)
{
	CWin::SetPosition(nXCoord, nYCoord);

	m_aBtn[LMW_BTN_MENU].SetPosition(nXCoord, nYCoord);
	m_aBtn[LMW_BTN_CREDIT].SetPosition(
		nXCoord + CWin::GetWidth() - m_aBtn[LMW_BTN_CREDIT].GetWidth(),
		nYCoord);
#if defined(MOVIE_DIRECTSHOW) || defined(__ANDROID__)
	m_aBtn[LMW_BTN_MOVIE].SetPosition(m_aBtn[LMW_BTN_CREDIT].GetXPos()
		- 10 - m_aBtn[LMW_BTN_MOVIE].GetWidth(), nYCoord);
#endif
	m_sprDeco.SetPosition(
		m_aBtn[LMW_BTN_CREDIT].GetXPos(), m_aBtn[LMW_BTN_CREDIT].GetYPos());
}

void CLoginMainWin::Show(bool bShow)
{
	CWin::Show(bShow);

	for (int i = 0; i < LMW_BTN_MAX; ++i)
		m_aBtn[i].Show(bShow);
	m_sprDeco.Show(bShow);
}

bool CLoginMainWin::CursorInWin(int nArea)
{
	if (!CWin::m_bShow)
		return false;

	switch (nArea)
	{
	case WA_MOVE:
		return false;
	}

	return CWin::CursorInWin(nArea);
}

void CLoginMainWin::UpdateWhileActive(double dDeltaTick)
{
	if (m_aBtn[LMW_BTN_MENU].IsClick())
	{
		CUIMng& rUIMng = CUIMng::Instance();
		rUIMng.ShowWin(&rUIMng.m_SysMenuWin);
		rUIMng.SetSysMenuWinShow(true);
	}
	else if (m_aBtn[LMW_BTN_CREDIT].IsClick())
	{
		SendRequestServerList();

		CUIMng& rUIMng = CUIMng::Instance();
		rUIMng.ShowWin(&rUIMng.m_CreditWin);

		::StopMp3(g_lpszMp3[MUSIC_MAIN_THEME]);
		::PlayMp3(g_lpszMp3[MUSIC_MUTHEME]);
	}
#if defined(MOVIE_DIRECTSHOW)
	else if (m_aBtn[LMW_BTN_MOVIE].IsClick())
	{
		g_pMovieScene = new CMovieScene;
		g_pMovieScene->Initialize_DirectShow(g_hWnd, MOVIE_FILE_WMV);
		if(g_pMovieScene->IsFile() == FALSE || g_pMovieScene->IsFailDirectShow() == TRUE)
		{
			g_pMovieScene->Destroy();
			SAFE_DELETE(g_pMovieScene);
			return;	
		}
		::StopMp3(g_lpszMp3[MUSIC_MAIN_THEME]);
		CUIMng& rUIMng = CUIMng::Instance();
		rUIMng.SetMoving(true);
	}
#elif defined(__ANDROID__)
	else if (m_aBtn[LMW_BTN_MOVIE].IsClick())
	{
		CUIMng& rUIMng = CUIMng::Instance();
		if (rUIMng.IsMoving())
		{
			return;
		}
		// Same file as PC DirectShow path (LoginMainWin / MovieScene): MOVIE_FILE_WMV in _define.h
		char pathRel[512];
		{
			const char* src = MOVIE_FILE_WMV;
			size_t o = 0;
			for (; src[o] && o + 1 < sizeof(pathRel); ++o)
			{
				const char c = src[o];
				pathRel[o] = (c == '\\') ? '/' : c;
			}
			pathRel[o] = '\0';
		}
		char resolved[8192];
		if (access(pathRel, F_OK) != 0 || realpath(pathRel, resolved) == nullptr)
		{
			g_ErrorReport.Write(
				"[AndroidLogin] intro movie missing: use same asset as main client (%s).\r\n",
				MOVIE_FILE_WMV);
			return;
		}
		::StopMp3(g_lpszMp3[MUSIC_MAIN_THEME]);
		rUIMng.SetMoving(true);
		MU_AndroidPlayLoginIntroMoviePath(resolved);
	}
#endif
}

void CLoginMainWin::RenderControls()
{
	m_sprDeco.Render();
	CWin::RenderButtons();
}