//*****************************************************************************
// Desc: implementation of the CServerSelWin class.
//*****************************************************************************

#include "stdafx.h"
#include "ServerSelWin.h"
#if defined(__ANDROID__)
#include "Platform/MobilePlatform.h"
#endif
#include "Input.h"
#include "UIMng.h"
#include "local.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "wsclientinline.h"
#include "UIControls.h"
#include "GameCensorship.h"
#include "ServerListManager.h"

#define	SSW_GAP_WIDTH	28
#define	SSW_GAP_HEIGHT	5
#define	SSW_GB_POS_X	16
#define	SSW_GB_POS_Y	19

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
extern int MouseWheel;
extern int MouseX;
extern int MouseY;
extern bool MouseLButton;

using namespace SEASON3A;

CServerSelWin::CServerSelWin()
{
	m_bAutoEnterFirstServer = false;
}

CServerSelWin::~CServerSelWin()
{

}

void CServerSelWin::Create()
{
	CWin::Create(0, 0, -2);

	m_iSelectServerBtnIndex = -1;
	m_iServerScrollOffset = 0;
	m_pSelectServerGroup = NULL;
	m_bAutoEnterFirstServer = false;

	int i;

	for (i = 0; i < SSW_SERVER_G_MAX; ++i)
	{
		m_aServerGroupBtn[i].Create(SERVER_GROUP_BTN_WIDTH, SERVER_GROUP_BTN_HEIGHT, BITMAP_LOG_IN, 4, 2, 1, -1, 3);
		CWin::RegisterButton(&m_aServerGroupBtn[i]);
	}

	for (i = 0; i < SSW_SERVER_MAX; ++i)
	{
		m_aServerBtn[i].Create(SERVER_BTN_WIDTH, SERVER_BTN_HEIGHT, BITMAP_LOG_IN+1, 3, 2, 1);
		CWin::RegisterButton(&m_aServerBtn[i]);
		m_aServerGauge[i].Create(160, 4, BITMAP_LOG_IN+2);				
	}

	SImgInfo aiiDeco[2] = 
	{
		{ BITMAP_LOG_IN+3, 0, 0, 68, 95 },
		{ BITMAP_LOG_IN+3, 68, 0, 68, 95 }
	};
	m_aBtnDeco[0].Create(&aiiDeco[0], 8, 19);
	m_aBtnDeco[1].Create(&aiiDeco[1], 60, 19);

	SImgInfo aiiArrow[2] = 
	{
		{ BITMAP_LOG_IN+3, 136, 0, 23, 29 },
		{ BITMAP_LOG_IN+3, 136, 30, 23, 29 }
	};
	m_aArrowDeco[0].Create(&aiiArrow[0], 1, 2);
	m_aArrowDeco[1].Create(&aiiArrow[1], 23, 2);
	
	SImgInfo aiiDescBg[WE_BG_MAX] = 
	{
		{ BITMAP_LOG_IN+11, 0, 0, 4, 4 },
		{ BITMAP_LOG_IN+12, 0, 0, 512, 6 },
		{ BITMAP_LOG_IN+12, 0, 6, 512, 6 },
		{ BITMAP_LOG_IN+13, 0, 0, 3, 4 },
		{ BITMAP_LOG_IN+13, 3, 0, 3, 4 }
	};
	m_winDescription.Create(aiiDescBg, 1, 10);
	m_winDescription.SetLine(10);

	CWin::SetSize((SERVER_GROUP_BTN_WIDTH + SSW_GAP_WIDTH) * 2 +SERVER_BTN_WIDTH,SERVER_BTN_HEIGHT*SSW_SERVER_MAX + SSW_GAP_HEIGHT*2 + SERVER_GROUP_BTN_HEIGHT+m_winDescription.GetHeight());
}

void CServerSelWin::PreRelease()
{
	int i;

	for (i = 0; i < SSW_SERVER_MAX; ++i)
	{
		m_aServerGauge[i].Release();
	}
	
	for (i = 0; i < 2; ++i)
	{
		m_aBtnDeco[i].Release();
		m_aArrowDeco[i].Release();
	}
	
	m_winDescription.Release();
}

void CServerSelWin::SetPosition(int nXCoord, int nYCoord)
{
	CWin::SetPosition(nXCoord, nYCoord);

	int nServerGBtnWidth = m_aServerGroupBtn[0].GetWidth();
	int nServerGBtnHeight = m_aServerGroupBtn[0].GetHeight();
	int nServerBtnWidth = m_aServerBtn[0].GetWidth();
	int nServerBtnHeight = m_aServerBtn[0].GetHeight();
	int nDescGgHeight = m_winDescription.GetHeight();
	int nBtnPosY;
	int i;
	
	int nServerGBtnBasePosY = nYCoord + CWin::GetHeight() - (nServerGBtnHeight * 11 + SSW_GAP_HEIGHT * 2 + nDescGgHeight);
	int nRServerGBtnPosX = nXCoord + nServerGBtnWidth + nServerBtnWidth + (SSW_GAP_WIDTH * 2);
	
	int icntServreGroup = 0;
	m_aServerGroupBtn[icntServreGroup++].SetPosition( nXCoord + (CWin::GetWidth() - nServerGBtnWidth) / 2, nYCoord + CWin::GetHeight() - nServerGBtnHeight - SSW_GAP_HEIGHT - nDescGgHeight);

	for (i=0 ; i<SSW_LEFT_SERVER_G_MAX; i++)
	{
		nBtnPosY = nServerGBtnBasePosY + nServerGBtnHeight * i;	
		m_aServerGroupBtn[icntServreGroup++].SetPosition(nXCoord, nBtnPosY);
	}

	for (i=0 ; i<SSW_RIGHT_SERVER_G_MAX; i++)
	{
		nBtnPosY = nServerGBtnBasePosY + nServerGBtnHeight * i;
		m_aServerGroupBtn[icntServreGroup++].SetPosition(nRServerGBtnPosX, nBtnPosY);
	}

	m_winDescription.SetPosition( nXCoord - ((m_winDescription.GetWidth() - CWin::GetWidth()) / 2),	nYCoord + CWin::GetHeight() - m_winDescription.GetHeight());	

	m_aBtnDeco[0].SetPosition(m_aServerGroupBtn[1].GetXPos(), m_aServerGroupBtn[1].GetYPos());
	m_aBtnDeco[1].SetPosition(m_aServerGroupBtn[SSW_LEFT_SERVER_G_MAX+1].GetXPos() + SERVER_GROUP_BTN_WIDTH,m_aServerGroupBtn[SSW_LEFT_SERVER_G_MAX+1].GetYPos());

	int a = m_aServerGroupBtn[1].GetXPos();
}

void CServerSelWin::SetServerBtnPosition()
{
	if( m_iSelectServerBtnIndex == -1 )
		return;

	// Middle column (subs): to the right of the *left-column* group button — use [1] width, not [0]
	// (center row uses index 0; its width can differ and misplaces subs off-screen).
	const int nServerBtnPosX = m_aServerGroupBtn[1].GetXPos() + m_aServerGroupBtn[1].GetWidth() + SSW_GAP_WIDTH;

	const int nServerBtnHeight = m_aServerBtn[0].GetHeight();

	const int nYAnchorIndex = (m_iSelectServerBtnIndex == 0) ? 1 : m_iSelectServerBtnIndex;
	const int nLServerGBtnTop = m_aServerGroupBtn[nYAnchorIndex].GetYPos();

	const int maxScroll = (m_icntServer > SSW_VISIBLE_SERVER_MAX) ? (m_icntServer - SSW_VISIBLE_SERVER_MAX) : 0;
	if (m_iServerScrollOffset < 0)
	{
		m_iServerScrollOffset = 0;
	}
	else if (m_iServerScrollOffset > maxScroll)
	{
		m_iServerScrollOffset = maxScroll;
	}

	const int visibleCount = (m_icntServer > SSW_VISIBLE_SERVER_MAX)
		? SSW_VISIBLE_SERVER_MAX
		: m_icntServer;

	int nServerBtnBasePosY = nLServerGBtnTop;
	if (m_icntServer <= SSW_VISIBLE_SERVER_MAX)
	{
		const int nLServerGBtnHeightSum = m_aServerGroupBtn[1].GetHeight() * 10;
		const int nServerBtnHeightSum = nServerBtnHeight * m_icntServer;
		if (nLServerGBtnHeightSum <= nServerBtnHeightSum)
		{
			nServerBtnBasePosY = nLServerGBtnTop - (nServerBtnHeightSum - nLServerGBtnHeightSum);
		}
	}

	for (int i = 0; i < visibleCount; ++i)
	{
		m_aServerBtn[i].SetPosition(nServerBtnPosX, nServerBtnBasePosY + nServerBtnHeight * i);
		m_aServerGauge[i].SetPosition(
			m_aServerBtn[i].GetXPos() + SSW_GB_POS_X,
			m_aServerBtn[i].GetYPos() + SSW_GB_POS_Y);
	}

	SyncVisibleServerSlots();
}

void CServerSelWin::SyncVisibleServerSlots()
{
	if (m_pSelectServerGroup == NULL || m_icntServer < 1)
	{
		return;
	}

	DWORD adwServerBtnClr[4][4] =
	{
		{ CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, 0 },
		{ CLRDW_YELLOW, CLRDW_YELLOW, CLRDW_BR_YELLOW, 0 },
		{ CLRDW_ORANGE, CLRDW_ORANGE, CLRDW_BR_ORANGE, 0 },
		{ CLRDW_ORANGE, CLRDW_ORANGE, CLRDW_BR_ORANGE, 0 },
	};

	const int visibleCount = (m_icntServer > SSW_VISIBLE_SERVER_MAX)
		? SSW_VISIBLE_SERVER_MAX
		: m_icntServer;

	for (int slot = 0; slot < visibleCount; ++slot)
	{
		const int serverIndex = m_iServerScrollOffset + slot;
		CServerInfo* pServerInfo = m_pSelectServerGroup->GetServerInfo(serverIndex);
		if (pServerInfo == NULL)
		{
			continue;
		}

		m_aServerBtn[slot].SetText(pServerInfo->m_bName, adwServerBtnClr[pServerInfo->m_byNonPvP]);
		m_aServerGauge[slot].SetValue(pServerInfo->m_iPercent, 100);
	}
}

void CServerSelWin::SetArrowSpritePosition()
{
	if( m_iSelectServerBtnIndex == -1 )
		return;
	
	if((m_iSelectServerBtnIndex >= 0 ) && (m_iSelectServerBtnIndex <= SSW_LEFT_SERVER_G_MAX))
	{
		m_aArrowDeco[0].SetPosition(m_aServerGroupBtn[m_iSelectServerBtnIndex].GetXPos() + SERVER_GROUP_BTN_WIDTH,m_aServerGroupBtn[m_iSelectServerBtnIndex].GetYPos());
	}
	else if((m_iSelectServerBtnIndex > SSW_LEFT_SERVER_G_MAX ) && (m_iSelectServerBtnIndex < SSW_SERVER_G_MAX))
	{
		m_aArrowDeco[1].SetPosition(m_aServerGroupBtn[m_iSelectServerBtnIndex].GetXPos(),m_aServerGroupBtn[m_iSelectServerBtnIndex].GetYPos());
	}
}

void CServerSelWin::UpdateDisplay()
{
	m_pSelectServerGroup = NULL;
	m_icntServerGroup = 0;
	m_icntServer = 0;
	m_icntLeftServerGroup = 0;
	m_icntRightServerGroup = 0;
	m_bTestServerBtn = false;

	DWORD adwServerGBtnClr[BTN_IMG_MAX] =
	{
		CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, 0,
		CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, 0
	};

	DWORD adwServerBtnClr[4][4] =
	{
		{ CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, 0 },
		{ CLRDW_YELLOW, CLRDW_YELLOW, CLRDW_BR_YELLOW, 0 },
		{ CLRDW_ORANGE, CLRDW_ORANGE, CLRDW_BR_ORANGE, 0 },
		{ CLRDW_ORANGE, CLRDW_ORANGE, CLRDW_BR_ORANGE, 0 },
	};

	m_icntServerGroup = g_ServerListManager->GetServerGroupSize();

	if( m_icntServerGroup < 1 )
		return;

	CServerGroup* pServerGroup = NULL;

	g_ServerListManager->SetFirst();

	while( g_ServerListManager->GetNext(pServerGroup) )
	{
		if( pServerGroup->m_iWidthPos == CServerGroup::SBP_CENTER )
		{
			if( m_bTestServerBtn == true )
				continue;

			m_aServerGroupBtn[0].SetText(pServerGroup->m_szName, adwServerGBtnClr);
			pServerGroup->m_iBtnPos = 0;
			m_bTestServerBtn = true;
		}
		else if( pServerGroup->m_iWidthPos == CServerGroup::SBP_LEFT )
		{
			if( m_icntLeftServerGroup >= SSW_LEFT_SERVER_G_MAX )
				continue;

			m_aServerGroupBtn[m_icntLeftServerGroup+1].SetText(pServerGroup->m_szName, adwServerGBtnClr);
			pServerGroup->m_iBtnPos = m_icntLeftServerGroup+1;

			m_icntLeftServerGroup++;
		}
		else if( pServerGroup->m_iWidthPos == CServerGroup::SBP_RIGHT )
		{
			if( m_icntRightServerGroup >= SSW_RIGHT_SERVER_G_MAX )
				continue;

			m_aServerGroupBtn[SSW_LEFT_SERVER_G_MAX+m_icntRightServerGroup+1].SetText(pServerGroup->m_szName, adwServerGBtnClr);
			pServerGroup->m_iBtnPos = SSW_LEFT_SERVER_G_MAX+m_icntRightServerGroup+1;

			m_icntRightServerGroup++;
		}	
	}

	ShowServerGBtns();
	ShowDecoSprite();

#if defined(__ANDROID__) || defined(MU_IOS)
	if (m_iSelectServerBtnIndex == -1)
	{
		if (m_bTestServerBtn == true)
		{
			m_iSelectServerBtnIndex = 0;
		}
		else if (m_icntLeftServerGroup > 0)
		{
			m_iSelectServerBtnIndex = 1;
		}
		else if (m_icntRightServerGroup > 0)
		{
			m_iSelectServerBtnIndex = SSW_LEFT_SERVER_G_MAX + 1;
		}

		if (m_iSelectServerBtnIndex != -1)
		{
			m_aServerGroupBtn[m_iSelectServerBtnIndex].SetCheck(true);
		}
	}
#endif

	memset(m_szDescription, 0, sizeof(char) * SSW_DESC_LINE_MAX * SSW_DESC_ROW_MAX);
	
	if( m_iSelectServerBtnIndex != -1)
	{
		m_pSelectServerGroup = g_ServerListManager->GetServerGroupByBtnPos(m_iSelectServerBtnIndex);
	}

	if( m_pSelectServerGroup == NULL )
		return;

	m_iServerScrollOffset = 0;
	m_icntServer = m_pSelectServerGroup->GetServerSize();

	if( m_icntServer < 1)
		return;

	CServerInfo* pServerInfo = NULL;

	m_pSelectServerGroup->SetFirst();

	int icntServer = 0;
	while(m_pSelectServerGroup->GetNext(pServerInfo))
	{
		m_aServerBtn[icntServer].SetText(pServerInfo->m_bName, adwServerBtnClr[pServerInfo->m_byNonPvP]);
		m_aServerGauge[icntServer].SetValue(pServerInfo->m_iPercent, 100);
		icntServer++;
	}

	::SeparateTextIntoLines(m_pSelectServerGroup->m_szDescription, m_szDescription[0], SSW_DESC_LINE_MAX, SSW_DESC_ROW_MAX);

	SetArrowSpritePosition();
	SetServerBtnPosition();
	ShowArrowSprite();
	ShowServerBtns();

}

void CServerSelWin::Show(bool bShow)
{
	CWin::Show(bShow);

	if (bShow == false)
	{
		for (int i = 0; i < SSW_SERVER_G_MAX; ++i)
		{
			m_aServerGroupBtn[i].Show(false);
		}

		for (int i = 0; i < SSW_SERVER_MAX; ++i)
		{
			m_aServerBtn[i].Show(false);
			m_aServerGauge[i].Show(false);
		}

		for (int i = 0; i < 2; ++i)
		{
			m_aBtnDeco[i].Show(false);
			m_aArrowDeco[i].Show(false);
		}

		m_winDescription.Show(false);
		return;
	}

	ShowServerGBtns();
	ShowDecoSprite();
	ShowArrowSprite();
	ShowServerBtns();
}

void CServerSelWin::ResetAutoEnterFirstServer()
{
	m_bAutoEnterFirstServer = false;
}

bool CServerSelWin::ConnectServerButtonIndex(int iIndex)
{
	if (m_pSelectServerGroup == NULL)
	{
		return false;
	}

	CServerInfo* pServerInfo = m_pSelectServerGroup->GetServerInfo(iIndex);

	if (pServerInfo == NULL)
	{
		return false;
	}

	if (pServerInfo->m_iPercent >= 100)
	{
		if (pServerInfo->m_iPercent < 128)
		{
			CUIMng::Instance().PopUpMsgWin(MESSAGE_SERVER_BUSY);
		}

		return false;
	}

	CUIMng::Instance().HideWin(this);
	SendRequestServerAddress(pServerInfo->m_iConnectIndex);
#if defined(__ANDROID__)
	g_ErrorReport.Write(
		"[TakumiLoginBg] sent C1 F4 03 connectIndex=%d fd=%d\r\n",
		pServerInfo->m_iConnectIndex,
		static_cast<int>(SocketClient.GetSocket()));
#endif

	int iCensorshipIndex = CGameCensorship::STATE_12;

	if (m_pSelectServerGroup->m_bPvPServer == true)
	{
		iCensorshipIndex = CGameCensorship::STATE_18;
	}
	else if (0x01 & pServerInfo->m_byNonPvP)
	{
		iCensorshipIndex = CGameCensorship::STATE_15;
	}

	bool bTestServer = false;
	if (m_pSelectServerGroup->m_iSequence == 0)
	{
		bTestServer = true;
	}

	g_ServerListManager->SetSelectServerInfo(
		m_pSelectServerGroup->m_szName,
		pServerInfo->m_iIndex,
		pServerInfo->m_iConnectIndex,
		iCensorshipIndex,
		pServerInfo->m_byNonPvP,
		bTestServer);

#if defined(__ANDROID__)
	MU_AndroidNotifyServerSubPickStarted();
#endif

	return true;
}

void CServerSelWin::ShowServerGBtns()
{
	int i;

	if( m_bTestServerBtn == true )
	{
		m_aServerGroupBtn[0].Show(CWin::m_bShow);
	}
	else
	{
		m_aServerGroupBtn[0].Show(false);
	}

	for (i = 1 ; i < m_icntLeftServerGroup+1 ; i++)
	{
		m_aServerGroupBtn[i].Show(CWin::m_bShow);
	}
	for (; i < SSW_LEFT_SERVER_G_MAX; ++i)
	{
		m_aServerGroupBtn[i].Show(false);
	}

	for (i = SSW_LEFT_SERVER_G_MAX+1 ; i < SSW_RIGHT_SERVER_G_MAX+1+m_icntRightServerGroup ; i++)
	{
		m_aServerGroupBtn[i].Show(CWin::m_bShow);
	}
	for (; i < SSW_SERVER_G_MAX; i++)
	{
		m_aServerGroupBtn[i].Show(false);
	}
}

void CServerSelWin::ShowDecoSprite()
{
	if( m_icntLeftServerGroup > 0 )
	{
		m_aBtnDeco[0].Show(CWin::m_bShow);
	}
	else
	{
		m_aBtnDeco[0].Show(false);
	}


	if( m_icntRightServerGroup > 0 )
	{
		m_aBtnDeco[1].Show(CWin::m_bShow);
	}
	else
	{
		m_aBtnDeco[1].Show(false);
	}
}

void CServerSelWin::ShowArrowSprite()
{
	if((m_iSelectServerBtnIndex >= 0 ) && (m_iSelectServerBtnIndex <= SSW_LEFT_SERVER_G_MAX))
	{
		m_aArrowDeco[0].Show(CWin::m_bShow);
		m_aArrowDeco[1].Show(false);
	}
	else if((m_iSelectServerBtnIndex > SSW_LEFT_SERVER_G_MAX ) && (m_iSelectServerBtnIndex < SSW_SERVER_G_MAX))
	{
		m_aArrowDeco[0].Show(false);
		m_aArrowDeco[1].Show(CWin::m_bShow);
	}
	else
	{
		m_aArrowDeco[0].Show(false);
		m_aArrowDeco[1].Show(false);
	}
}

void CServerSelWin::ShowServerBtns()
{
	if (m_iSelectServerBtnIndex == -1)
	{
		m_winDescription.Show(false);
		return;
	}
	
	const int visibleCount = (m_icntServer > SSW_VISIBLE_SERVER_MAX)
		? SSW_VISIBLE_SERVER_MAX
		: m_icntServer;

	int i;
	for (i = 0; i < visibleCount; ++i)
	{
		m_aServerBtn[i].Show(CWin::m_bShow);
		m_aServerGauge[i].Show(CWin::m_bShow);
	}
	for ( ; i < SSW_SERVER_MAX; i++)
	{
		m_aServerBtn[i].Show(false);
		m_aServerGauge[i].Show(false);
	}

	m_winDescription.Show(CWin::m_bShow);	
}

bool CServerSelWin::CursorInWin(int nArea)
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

void CServerSelWin::UpdateWhileActive(double dDeltaTick)
{
	int i;

	for( i=0 ; i<SSW_SERVER_G_MAX ; i++ )
	{
		if (m_aServerGroupBtn[i].IsClick())
		{
			if(m_iSelectServerBtnIndex != -1 )
			{
				m_aServerGroupBtn[m_iSelectServerBtnIndex].SetCheck(false);
			}

			m_aServerGroupBtn[i].SetCheck(true);
			m_iSelectServerBtnIndex = i;
			m_iServerScrollOffset = 0;

			SendRequestServerList();
		}
	}

	if( m_pSelectServerGroup == NULL )
		return ;

	const int visibleCount = (m_icntServer > SSW_VISIBLE_SERVER_MAX)
		? SSW_VISIBLE_SERVER_MAX
		: m_icntServer;

	if (m_icntServer > SSW_VISIBLE_SERVER_MAX && visibleCount > 0)
	{
		const int listLeft = m_aServerBtn[0].GetXPos();
		const int listRight = listLeft + SERVER_BTN_WIDTH + 12;
		const int listTop = m_aServerBtn[0].GetYPos();
		const int listBottom = m_aServerBtn[visibleCount - 1].GetYPos() + SERVER_BTN_HEIGHT;
		const bool pointerInList = MouseX >= listLeft && MouseX <= listRight
			&& MouseY >= listTop && MouseY <= listBottom;

		static int s_dragScrollLastY = -1;
		if (MouseLButton && pointerInList)
		{
			if (s_dragScrollLastY >= 0)
			{
				const int deltaY = MouseY - s_dragScrollLastY;
				if (deltaY >= 18)
				{
					--m_iServerScrollOffset;
					if (m_iServerScrollOffset < 0)
					{
						m_iServerScrollOffset = 0;
					}
					SetServerBtnPosition();
					ShowServerBtns();
					s_dragScrollLastY = MouseY;
				}
				else if (deltaY <= -18)
				{
					++m_iServerScrollOffset;
					const int maxScroll = m_icntServer - SSW_VISIBLE_SERVER_MAX;
					if (m_iServerScrollOffset > maxScroll)
					{
						m_iServerScrollOffset = maxScroll;
					}
					SetServerBtnPosition();
					ShowServerBtns();
					s_dragScrollLastY = MouseY;
				}
			}
			else
			{
				s_dragScrollLastY = MouseY;
			}
		}
		else
		{
			s_dragScrollLastY = -1;
		}

		if (MouseWheel > 0)
		{
			--m_iServerScrollOffset;
			if (m_iServerScrollOffset < 0)
			{
				m_iServerScrollOffset = 0;
			}
			SetServerBtnPosition();
			ShowServerBtns();
			MouseWheel = 0;
		}
		else if (MouseWheel < 0)
		{
			++m_iServerScrollOffset;
			const int maxScroll = m_icntServer - SSW_VISIBLE_SERVER_MAX;
			if (m_iServerScrollOffset > maxScroll)
			{
				m_iServerScrollOffset = maxScroll;
			}
			SetServerBtnPosition();
			ShowServerBtns();
			MouseWheel = 0;
		}
	}

	for( i=0 ; i<visibleCount ; i++ )
	{
		if (m_aServerBtn[i].IsClick())
		{
			ConnectServerButtonIndex(m_iServerScrollOffset + i);
			break;
		}
	}
}

void CServerSelWin::RenderControls()
{
	int i = 0;
	
	g_pRenderText->SetFont(g_hFixFont);
	g_pRenderText->SetTextColor(CLRDW_WHITE);
	g_pRenderText->SetBgColor(0);
	
	CWin::RenderButtons();
	
	if( m_pSelectServerGroup != NULL )
	{
		const int visibleCount = (m_icntServer > SSW_VISIBLE_SERVER_MAX)
			? SSW_VISIBLE_SERVER_MAX
			: m_icntServer;

		for (i = 0; i < visibleCount; ++i)
		{
			m_aServerGauge[i].Render();
		}

		if (m_icntServer > SSW_VISIBLE_SERVER_MAX && visibleCount > 0)
		{
			const int trackX = m_aServerBtn[0].GetXPos() + SERVER_BTN_WIDTH + 4;
			const int trackTop = m_aServerBtn[0].GetYPos();
			const int trackBottom = m_aServerBtn[visibleCount - 1].GetYPos() + SERVER_BTN_HEIGHT;
			const int trackH = trackBottom - trackTop;
			if (trackH > 0)
			{
				const float scrollRate = static_cast<float>(m_iServerScrollOffset)
					/ static_cast<float>(m_icntServer - SSW_VISIBLE_SERVER_MAX);
				const int thumbH = (trackH * SSW_VISIBLE_SERVER_MAX) / m_icntServer;
				const int thumbY = trackTop + static_cast<int>((trackH - thumbH) * scrollRate);

				EnableAlphaTest();
				glColor4f(1.0f, 1.0f, 1.0f, 0.35f);
				RenderColor(static_cast<float>(trackX), static_cast<float>(trackTop), 4.0f, static_cast<float>(trackH));
				glColor4f(1.0f, 0.85f, 0.2f, 0.9f);
				RenderColor(static_cast<float>(trackX), static_cast<float>(thumbY), 4.0f, static_cast<float>(thumbH));
				EndRenderColor();
				DisableAlphaBlend();
			}
		}

		if( m_pSelectServerGroup->m_bPvPServer == true )
		{
			g_pRenderText->SetTextColor(ARGB(255, 255, 255, 255));
			g_pRenderText->RenderText(90, 164 - 60, GlobalText[565]);
			g_pRenderText->RenderText(90, 164 - 45, GlobalText[566]);
			g_pRenderText->RenderText(90, 164 - 30, GlobalText[567]);
		}
	}
}
