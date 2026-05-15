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
#if defined(__ANDROID__)
	// Movie is auto-played as full-screen background; bitmap often missing in data packs (white box).
	m_aBtn[LMW_BTN_MOVIE].Show(false);
#endif
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
#if defined(__ANDROID__)
	// Show(true) above re-enables every child; keep movie control hidden (no texture in many data packs).
	m_aBtn[LMW_BTN_MOVIE].Show(false);
#endif
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
		// WMV first (same as legacy Android path / PC asset layout). Some data packs ship a broken or
		// empty MU.mp4 placeholder; preferring MP4 first made MediaPlayer fail after that change.
		static const char* const kMovieCandidates[] = {
			MOVIE_FILE_WMV,
			MOVIE_FILE_MP4,
		};
		char resolved[8192];
		bool found = false;
		for (size_t ci = 0; ci < sizeof(kMovieCandidates) / sizeof(kMovieCandidates[0]); ++ci)
		{
			char pathRel[512];
			{
				const char* src = kMovieCandidates[ci];
				size_t o = 0;
				for (; src[o] && o + 1 < sizeof(pathRel); ++o)
				{
					const char c = src[o];
					pathRel[o] = (c == '\\') ? '/' : c;
				}
				pathRel[o] = '\0';
			}
			if (access(pathRel, F_OK) != 0)
			{
				continue;
			}
			if (realpath(pathRel, resolved) != nullptr)
			{
				found = true;
				break;
			}
			// realpath can fail on some Android mounts even when access() succeeds; MediaPlayer still
			// accepts the path the game uses (cwd is Data root after early chdir).
			if (strlen(pathRel) < sizeof(resolved))
			{
				memcpy(resolved, pathRel, strlen(pathRel) + 1);
				found = true;
				break;
			}
		}
		if (!found)
		{
			g_ErrorReport.Write(
				"[AndroidLogin] intro movie missing: %s or %s under Data/Movie (cwd=Data root).\r\n",
				MOVIE_FILE_WMV,
				MOVIE_FILE_MP4);
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