//*****************************************************************************
// File: MsgWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "MsgWin.h"
#include "Input.h"
#include "UIMng.h"
#include <crtdbg.h>
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "GOBoid.h"
#include "ZzzScene.h"
#include "DSPlaySound.h"
#include "wsclientinline.h"
#include "WSclient.h"
#include "UIControls.h"
#include "ZzzOpenglUtil.h"
#include "Utilities/Log/ErrorReport.h"
#include <algorithm>

#if defined(__ANDROID__) || defined(MU_IOS)
#include "sokol_app.h"
#include "MobilePlatform.h"
#include "Platform/PlatformDefs.h"
#endif

#define	MW_OK		0
#define	MW_CANCEL	1

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

namespace
{
constexpr int kDeleteCharGuardDigits = 7;
// Client-side captcha for character delete (shown on 2nd line of MsgWin).
char s_DeleteCharGuardCode[kDeleteCharGuardDigits + 1];

static void GenerateDeleteCharGuardCode()
{
	for (int i = 0; i < kDeleteCharGuardDigits; ++i)
	{
		s_DeleteCharGuardCode[i] = static_cast<char>('0' + (rand() % 10));
	}
	s_DeleteCharGuardCode[kDeleteCharGuardDigits] = '\0';
	// Avoid trivial all-zero code (some servers treat empty resident specially).
	if (strspn(s_DeleteCharGuardCode, "0") == static_cast<size_t>(kDeleteCharGuardDigits))
	{
		s_DeleteCharGuardCode[0] = '1';
	}
}

// Virtual 640x480 hit target for the delete-captcha field: full input sprite slab
// plus padding (fat finger). Matches LoginWin using the whole input box sprite,
// not only the CUITextInputBox text metrics.
static bool DeleteResidentFieldHitVirtual(float vx, float vy, CSprite& sprInput)
{
	const int bx = static_cast<int>(static_cast<float>(sprInput.GetXPos()) / g_fScreenRate_x);
	const int by = static_cast<int>(static_cast<float>(sprInput.GetYPos()) / g_fScreenRate_y);
	const int bw = static_cast<int>(static_cast<float>(sprInput.GetWidth()) / g_fScreenRate_x);
	const int bh = static_cast<int>(static_cast<float>(sprInput.GetHeight()) / g_fScreenRate_y);
	constexpr int kPadH = 14;
	constexpr int kPadV = 20;
	const int hx = bx - kPadH;
	const int hy = by - kPadV;
	const int hw = bw + 2 * kPadH;
	const int hh = bh + 2 * kPadV;
	return vx >= static_cast<float>(hx) && vx < static_cast<float>(hx + hw)
		&& vy >= static_cast<float>(hy) && vy < static_cast<float>(hy + hh);
}
} // namespace

extern int g_iChatInputType;
extern CUITextInputBox* g_pSinglePasswdInputBox;

#if defined(__ANDROID__) || defined(MU_IOS)
bool CMsgWin::AndroidTryFocusDeleteResidentInput(float virtualUiX, float virtualUiY)
{
	if (!m_bShow || m_nMsgCode != MESSAGE_DELETE_CHARACTER_RESIDENT || m_eType != MWT_STR_INPUT)
	{
		return false;
	}
	if (g_iChatInputType != 1 || g_pSinglePasswdInputBox == nullptr)
	{
		return false;
	}
	if (g_pSinglePasswdInputBox->GetState() != UISTATE_NORMAL)
	{
		return false;
	}
	if (!DeleteResidentFieldHitVirtual(virtualUiX, virtualUiY, m_sprInput))
	{
		return false;
	}
	g_pSinglePasswdInputBox->GiveFocus(TRUE);
	return true;
}
#endif

CMsgWin::CMsgWin()
{
}

CMsgWin::~CMsgWin()
{
}

void CMsgWin::Create()
{
	CInput rInput = CInput::Instance();

	CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight());

	m_sprBack.Create(352, 113, BITMAP_MESSAGE_WIN);

	m_sprInput.Create(171, 23, BITMAP_MSG_WIN_INPUT);

	for (int i = 0; i < 2; ++i)
	{
		m_aBtn[i].Create(54, 30, BITMAP_BUTTON + i, 3, 2, 1);
		CWin::RegisterButton(&m_aBtn[i]);
	}

	memset(m_aszMsg[0], 0 ,sizeof(char) * MW_MSG_LINE_MAX * MW_MSG_ROW_MAX);

	m_eType = MWT_NON;
	m_nMsgLine = 0;
	m_nMsgCode = -1;
	m_nGameExit = -1;
	m_dDeltaTickSum = 0.0;
}

void CMsgWin::PreRelease()
{
	m_sprInput.Release();
	m_sprBack.Release();
}

void CMsgWin::SetPosition(int nXCoord, int nYCoord)
{
	m_sprBack.SetPosition(nXCoord, nYCoord);
	SetCtrlPosition();
}

void CMsgWin::SetCtrlPosition()
{
	int nBaseXPos = m_sprBack.GetXPos();
	int nBtnYPos = m_sprBack.GetYPos() + 72;

	switch (m_eType)
	{
	case MWT_BTN_CANCEL:
		m_aBtn[MW_CANCEL].SetPosition(nBaseXPos + 149, nBtnYPos);
		break;
	case MWT_BTN_OK:
		m_aBtn[MW_OK].SetPosition(nBaseXPos + 149, nBtnYPos);
		break;
	case MWT_BTN_BOTH:
		m_aBtn[MW_OK].SetPosition(nBaseXPos + 98, nBtnYPos);
		m_aBtn[MW_CANCEL].SetPosition(nBaseXPos + 200, nBtnYPos);
		break;
	case MWT_STR_INPUT:
		m_sprInput.SetPosition(nBaseXPos + 32, nBtnYPos + 4);
		m_aBtn[MW_OK].SetPosition(nBaseXPos + 209, nBtnYPos);
		m_aBtn[MW_CANCEL].SetPosition(nBaseXPos + 264, nBtnYPos);
		// ??? ???? ??? ????.
		if (m_nMsgCode == MESSAGE_DELETE_CHARACTER_RESIDENT)
		{
			if (g_iChatInputType == 1)
			{
				const int sprVw = static_cast<int>(static_cast<float>(m_sprInput.GetWidth()) / g_fScreenRate_x);
				const int sprVh = static_cast<int>(static_cast<float>(m_sprInput.GetHeight()) / g_fScreenRate_y);
				const int textW = (std::max)(90, sprVw - 18);
				const int textH = (std::max)(18, sprVh - 10);
				g_pSinglePasswdInputBox->SetPosition(
					int((m_sprInput.GetXPos() + 10) / g_fScreenRate_x),
					int((m_sprInput.GetYPos() + 8) / g_fScreenRate_y));
				g_pSinglePasswdInputBox->SetSize(textW, textH);
			}
		}
		break;
	}
}

void CMsgWin::Show(bool bShow)
{
	CWin::Show(bShow);

	m_sprBack.Show(bShow);

	switch (m_eType)
	{
	case MWT_BTN_CANCEL:
		m_aBtn[MW_OK].Show(false);
		m_aBtn[MW_CANCEL].Show(bShow);
		m_sprInput.Show(false);
		break;
	case MWT_BTN_OK:
		m_aBtn[MW_OK].Show(bShow);
		m_aBtn[MW_CANCEL].Show(false);
		m_sprInput.Show(false);
		break;
	case MWT_BTN_BOTH:
		m_aBtn[MW_OK].Show(bShow);
		m_aBtn[MW_CANCEL].Show(bShow);
		m_sprInput.Show(false);
		break;
	case MWT_STR_INPUT:
		m_aBtn[MW_OK].Show(bShow);
		m_aBtn[MW_CANCEL].Show(bShow);
		m_sprInput.Show(bShow);
		break;
	default:
		m_aBtn[MW_OK].Show(false);
		m_aBtn[MW_CANCEL].Show(false);
		m_sprInput.Show(false);
	}
		
}

bool CMsgWin::CursorInWin(int nArea)
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

void CMsgWin::UpdateWhileShow(double dDeltaTick)
{
#if defined(__ANDROID__) || defined(MU_IOS)
	CInput& rInput = CInput::Instance();
	// Runs even before Active(true) is applied (same frame as PopUp), so the soft
	// keyboard can open on tap without waiting for the next frame.
	if (m_nMsgCode == MESSAGE_DELETE_CHARACTER_RESIDENT && m_eType == MWT_STR_INPUT && rInput.IsLBtnDn())
	{
		// Whole input sprite + padding (same idea as LoginWin hitting m_asprInputBox).
		const bool onPasswd = (g_iChatInputType == 1 && g_pSinglePasswdInputBox != nullptr
			&& g_pSinglePasswdInputBox->GetState() == UISTATE_NORMAL
			&& DeleteResidentFieldHitVirtual(static_cast<float>(MouseX), static_cast<float>(MouseY), m_sprInput));
		if (onPasswd)
		{
			g_pSinglePasswdInputBox->GiveFocus(TRUE);
		}
		else
		{
			const bool onButton = m_aBtn[MW_OK].CursorInObject() || m_aBtn[MW_CANCEL].CursorInObject();
			if (!onButton && AndroidHasFocusedTextInput())
			{
				::SetFocus(nullptr);
			}
		}
	}
	if (m_nMsgCode == MESSAGE_WAIT && CurrentProtocolState == REQUEST_DELETE_CHARACTER)
	{
		m_dDeltaTickSum += dDeltaTick;
		extern bool EnableSocket;
		extern CWsctlc SocketClient;
		if (EnableSocket && SocketClient.GetSocket() != INVALID_SOCKET)
		{
			SocketClient.AndroidSyncPollRecvPending();
			ProtocolCompiler();
		}
		if (m_dDeltaTickSum > 15000.0)
		{
			m_dDeltaTickSum = 0.0;
			CurrentProtocolState = RECEIVE_CHARACTERS_LIST;
			g_ErrorReport.Write(
				"[AndroidLogin] delete character wait timeout (no F3 02 response within 15s)\r\n");
			PopUp(MESSAGE_SERVER_LOST);
		}
	}
#else
	(void)dDeltaTick;
#endif
}

void CMsgWin::UpdateWhileActive(double dDeltaTick)
{
	CInput& rInput = CInput::Instance();

	if (rInput.IsKeyDown(VK_RETURN))
	{
		if (m_eType > MWT_BTN_CANCEL)
		{
			::PlayBuffer(SOUND_CLICK01);
			ManageOKClick();
		}
		else if (m_eType == MWT_BTN_CANCEL)
		{
			::PlayBuffer(SOUND_CLICK01);
			ManageCancelClick();
		}
	}
	else if (rInput.IsKeyDown(VK_ESCAPE))
	{
		if (m_eType == MWT_BTN_OK)
		{
			::PlayBuffer(SOUND_CLICK01);
			ManageOKClick();
		}
		else if (m_eType > MWT_NON)
		{
			::PlayBuffer(SOUND_CLICK01);
			ManageCancelClick();
		}
		CUIMng::Instance().SetSysMenuWinShow(false);
	}
	else if (m_aBtn[MW_OK].IsClick())
		ManageOKClick();
	else if (m_aBtn[MW_CANCEL].IsClick())
		ManageCancelClick();
	else if (m_nMsgCode == MESSAGE_GAME_END_COUNTDOWN)
	{
		if (m_nGameExit != -1) 
		{
			m_dDeltaTickSum += dDeltaTick;
			if (m_dDeltaTickSum > 1000.0)
			{
				m_dDeltaTickSum = 0.0;
				if (--m_nGameExit == 0)
				{
					g_ErrorReport.Write("> Menu - Exit game.");
					g_ErrorReport.WriteCurrentTime();
#if defined(__ANDROID__) || defined(MU_IOS)
					MU_MobileStopTextInput();
					MU_MobileRequestExit();
#else
					::PostMessage(g_hWnd, WM_CLOSE, 0, 0);
#endif
				}
				else
				{
					char szMsg[64];
					::sprintf(szMsg, GlobalText[380], m_nGameExit);
					SetMsg(m_eType, szMsg);
				}
			}
		}
	}
}

void CMsgWin::RenderControls()
{
	m_sprBack.Render();

	int nTextPosX, nTextPosY;

	g_pRenderText->SetFont(g_hFixFont);
	g_pRenderText->SetTextColor(CLRDW_WHITE);
	g_pRenderText->SetBgColor(0);

	if (1 == m_nMsgLine)
	{
		nTextPosX = int(m_sprBack.GetXPos() / g_fScreenRate_x);
		if (MWT_NON != m_eType)
			nTextPosY = int((m_sprBack.GetYPos() + 38) / g_fScreenRate_y);
		else
			nTextPosY = int((m_sprBack.GetYPos() + 54) / g_fScreenRate_y);
		g_pRenderText->RenderText(nTextPosX, nTextPosY, m_aszMsg[0],
			m_sprBack.GetWidth() / g_fScreenRate_x, 0, RT3_SORT_CENTER);
	}
	else if (2 == m_nMsgLine)
	{
		nTextPosX = int((m_sprBack.GetXPos() + 25) / g_fScreenRate_x);
		if (MWT_NON != m_eType)
			nTextPosY = int((m_sprBack.GetYPos() + 32) / g_fScreenRate_y);
		else
			nTextPosY = int((m_sprBack.GetYPos() + 44) / g_fScreenRate_y);
		g_pRenderText->RenderText(nTextPosX, nTextPosY, m_aszMsg[0]);

		if (MWT_NON != m_eType)
			nTextPosY = int((m_sprBack.GetYPos() + 51) / g_fScreenRate_y);
		else
			nTextPosY = int((m_sprBack.GetYPos() + 66) / g_fScreenRate_y);
		g_pRenderText->RenderText(nTextPosX, nTextPosY, m_aszMsg[1]);
	}

	m_sprInput.Render();

	if (m_nMsgCode == MESSAGE_DELETE_CHARACTER_RESIDENT)
	{
		if (g_iChatInputType == 1)
			g_pSinglePasswdInputBox->Render();
		else if (g_iChatInputType == 0)
		{
			InputTextWidth = 100;
			::RenderInputText(
				int((m_sprInput.GetXPos() + 10) / g_fScreenRate_x),
				int((m_sprInput.GetYPos() + 8) / g_fScreenRate_y), 0, 0);
			InputTextWidth = 256;
		}
	}

	CWin::RenderButtons();
}

void CMsgWin::SetMsg(MSG_WIN_TYPE eType, LPCTSTR lpszMsg, LPCTSTR lpszMsg2)
{
	_ASSERT(lpszMsg);

	m_eType = eType;

	SetCtrlPosition();

	if (NULL == lpszMsg2)
	{
		m_nMsgLine = ::SeparateTextIntoLines(
			(char*)lpszMsg, m_aszMsg[0], MW_MSG_LINE_MAX, MW_MSG_ROW_MAX);
	}
	else
	{
		// m_aszMsg rows are MW_MSG_ROW_MAX (52) bytes. Do not use strncpy_s(..., _TRUNCATE) on Android:
		// _TRUNCATE is MSVC-specific; Bionic treats it as a huge count and FORTIFY aborts.
		snprintf(m_aszMsg[0], sizeof(m_aszMsg[0]), "%s", lpszMsg);
		snprintf(m_aszMsg[1], sizeof(m_aszMsg[1]), "%s", lpszMsg2);
		m_nMsgLine = 2;
	}
}

void CMsgWin::PopUp(int nMsgCode, char* pszMsg)
{
	CUIMng& rUIMng = CUIMng::Instance();
	LPCTSTR lpszMsg = NULL, lpszMsg2 = NULL;
	MSG_WIN_TYPE eType = MWT_BTN_OK;
	m_nMsgCode = nMsgCode;
	char szTempMsg[128];

	switch (m_nMsgCode)
	{
	case MESSAGE_FREE_MSG_NOT_BTN:
		lpszMsg = pszMsg;
		eType = MWT_NON;
		break;
	case MESSAGE_GAME_END_COUNTDOWN:
		m_nGameExit = 5;
		::sprintf(szTempMsg, GlobalText[380], m_nGameExit);
		lpszMsg = szTempMsg;
		eType = MWT_NON;
		break;
	case MESSAGE_WAIT:
		lpszMsg = GlobalText[471];
		eType = MWT_NON;
		m_dDeltaTickSum = 0.0;
		break;
	case MESSAGE_SERVER_BUSY:
	case RECEIVE_LOG_IN_FAIL_SERVER_BUSY:
		lpszMsg = GlobalText[416];
		break;
	case RECEIVE_JOIN_SERVER_WAITING:
		rUIMng.ShowWin(&rUIMng.m_ServerSelWin);
		lpszMsg = GlobalText[416];
		break;
	case MESSAGE_SERVER_LOST:
		lpszMsg = GlobalText[402];
		break;
	case MESSAGE_VERSION:
	case RECEIVE_LOG_IN_FAIL_VERSION:
		lpszMsg = GlobalText[405];
		lpszMsg2 = GlobalText[406];
		break;
	case MESSAGE_INPUT_ID:
		lpszMsg = GlobalText[403];
		break;
	case MESSAGE_INPUT_PASSWORD:
		lpszMsg = GlobalText[404];
		break;
	case RECEIVE_LOG_IN_FAIL_ID:
		lpszMsg = GlobalText[414];
		break;
	case RECEIVE_LOG_IN_FAIL_PASSWORD:
		lpszMsg = GlobalText[407];
		break;
	case RECEIVE_LOG_IN_FAIL_ID_CONNECTED:
		lpszMsg = GlobalText[415];
		break;
	case RECEIVE_LOG_IN_FAIL_ID_BLOCK:
	case MESSAGE_DELETE_CHARACTER_ID_BLOCK:
		lpszMsg = GlobalText[417];
		break;
	case RECEIVE_LOG_IN_FAIL_CONNECT:
		lpszMsg = GlobalText[408];
		break;
	case RECEIVE_LOG_IN_FAIL_ERROR:
		lpszMsg = GlobalText[409];
		break;
	case RECEIVE_LOG_IN_FAIL_NO_PAYMENT_INFO:
		lpszMsg = GlobalText[433];
		break;
	case RECEIVE_LOG_IN_FAIL_USER_TIME1:
		lpszMsg = GlobalText[410];
		break;
	case RECEIVE_LOG_IN_FAIL_USER_TIME2:
		lpszMsg = GlobalText[411];
		break;
	case RECEIVE_LOG_IN_FAIL_PC_TIME1:
		lpszMsg = GlobalText[412];
		break;
	case RECEIVE_LOG_IN_FAIL_PC_TIME2:
		lpszMsg = GlobalText[413];
		break;
	case RECEIVE_LOG_IN_FAIL_ONLY_OVER_15:
		lpszMsg = GlobalText[435];
		break;
	case RECEIVE_LOG_IN_FAIL_CHARGED_CHANNEL:
		lpszMsg = GlobalText[3118];
		break;
	case RECEIVE_LOG_IN_FAIL_POINT_DATE:
		lpszMsg = GlobalText[597];
		break;
	case RECEIVE_LOG_IN_FAIL_POINT_HOUR:
		lpszMsg = GlobalText[598];
		break;
	case RECEIVE_LOG_IN_FAIL_INVALID_IP:
		lpszMsg = GlobalText[599];
		break;
	case MESSAGE_DELETE_CHARACTER_GUILDWARNING:
		lpszMsg = GlobalText[1654];
		break;
	case MESSAGE_DELETE_CHARACTER_WARNING:
		sprintf(szTempMsg, GlobalText[1711], CHAR_DEL_LIMIT_LV);
		lpszMsg = szTempMsg;
		break;
	case MESSAGE_DELETE_CHARACTER_CONFIRM:
		sprintf(szTempMsg, GlobalText[1712], CharactersClient[SelectedHero].ID);
		lpszMsg = szTempMsg;
		eType = MWT_BTN_BOTH;
		break;
	case MESSAGE_DELETE_CHARACTER_RESIDENT:
		GenerateDeleteCharGuardCode();
		// ASCII-only for NDK; must fit in m_aszMsg[][MW_MSG_ROW_MAX] (52) when SetMsg copies two lines.
		sprintf(
			szTempMsg,
			"Vui long nhap %d so bao mat ben duoi trung ma hien thi.",
			kDeleteCharGuardDigits);
		lpszMsg = szTempMsg;
		lpszMsg2 = s_DeleteCharGuardCode;
		eType = MWT_STR_INPUT;
		break;
	case MESSAGE_DELETE_CHARACTER_ITEM_BLOCK:
		lpszMsg = GlobalText[439];
		break;
	case MESSAGE_STORAGE_RESIDENTWRONG:
		lpszMsg = GlobalText[401];
		break;
	case MESSAGE_DELETE_CHARACTER_SUCCESS:
		CharactersClient[SelectedHero].Object.Live = false;
		DeleteBug(&CharactersClient[SelectedHero].Object);
		SelectedHero = -1;
		rUIMng.m_CharSelMainWin.UpdateDisplay();
		rUIMng.m_CharInfoBalloonMng.UpdateDisplay();
		lpszMsg = GlobalText[1714];
		break;
	case MESSAGE_BLOCKED_CHARACTER:
		lpszMsg = GlobalText[434];
		break;
	case MESSAGE_MIN_LENGTH:
		lpszMsg = GlobalText[390];
		break;
	case MESSAGE_ID_SPACE_ERROR:
		lpszMsg = GlobalText[1715];
		break;
	case MESSAGE_SPECIAL_NAME:
		lpszMsg = GlobalText[391];
		break;
	case RECEIVE_CREATE_CHARACTER_FAIL:
		rUIMng.ShowWin(&rUIMng.m_CharMakeWin);
		lpszMsg = GlobalText[1716];
		break;
	case RECEIVE_CREATE_CHARACTER_FAIL2:
		rUIMng.ShowWin(&rUIMng.m_CharMakeWin);
		lpszMsg = GlobalText[396];
		break;
	default:
		m_nMsgCode = -1;
		return;
	}

	SetMsg(eType, lpszMsg, lpszMsg2);
	rUIMng.ShowWin(this);

	if (m_nMsgCode == MESSAGE_DELETE_CHARACTER_RESIDENT)
	{
		InitResidentNumInput();
	}
}

void CMsgWin::ManageOKClick()
{
	CUIMng& rUIMng = CUIMng::Instance();

	if (m_nMsgCode == MESSAGE_DELETE_CHARACTER_RESIDENT)
	{
		char typed[kDeleteCharGuardDigits + 8] = {};
		if (g_iChatInputType == 1)
		{
			g_pSinglePasswdInputBox->GetText(typed);
		}
		else
		{
			strncpy_s(typed, sizeof(typed), InputText[0], _TRUNCATE);
		}
		// Match legacy behaviour: do not send delete with empty / partial resident.
		if (strlen(typed) != static_cast<size_t>(kDeleteCharGuardDigits)
			|| strcmp(typed, s_DeleteCharGuardCode) != 0)
		{
			::PlayBuffer(SOUND_CLICK01);
			CUIMng::Instance().PopUpMsgWin(MESSAGE_STORAGE_RESIDENTWRONG);
			return;
		}
	}

	rUIMng.HideWin(this);

	switch (m_nMsgCode)
	{
	case RECEIVE_LOG_IN_FAIL_VERSION:
#if defined(__ANDROID__) || defined(MU_IOS)
		MU_MobileStopTextInput();
		MU_MobileRequestExit();
#else
		::PostMessage(g_hWnd, WM_CLOSE, 0, 0);
#endif
		break;
	case MESSAGE_SERVER_LOST:
#if defined(__ANDROID__) || defined(MU_IOS)
		MU_MobileStopTextInput();
		MU_MobileRequestExit();
#else
		::PostMessage(g_hWnd, WM_CLOSE, 0, 0);
#endif
		break;
	case MESSAGE_VERSION:
	case RECEIVE_LOG_IN_FAIL_ERROR:
	case MESSAGE_INPUT_ID:
	case RECEIVE_LOG_IN_FAIL_ID:
	case RECEIVE_LOG_IN_FAIL_ID_CONNECTED:
	case RECEIVE_LOG_IN_FAIL_SERVER_BUSY:
	case RECEIVE_LOG_IN_FAIL_ID_BLOCK:
	case RECEIVE_LOG_IN_FAIL_CONNECT:
	case RECEIVE_LOG_IN_FAIL_NO_PAYMENT_INFO:
	case RECEIVE_LOG_IN_FAIL_USER_TIME1:
	case RECEIVE_LOG_IN_FAIL_USER_TIME2:
	case RECEIVE_LOG_IN_FAIL_PC_TIME1:
	case RECEIVE_LOG_IN_FAIL_PC_TIME2:
	case RECEIVE_LOG_IN_FAIL_ONLY_OVER_15:
	case RECEIVE_LOG_IN_FAIL_POINT_DATE:
	case RECEIVE_LOG_IN_FAIL_POINT_HOUR:
	case RECEIVE_LOG_IN_FAIL_INVALID_IP:
	case RECEIVE_LOG_IN_FAIL_CHARGED_CHANNEL:
		rUIMng.ShowWin(&rUIMng.m_LoginWin);
		CUIMng::Instance().m_LoginWin.GetIDInputBox()->GiveFocus(TRUE);
		CurrentProtocolState = RECEIVE_JOIN_SERVER_SUCCESS;
		break;
	case MESSAGE_INPUT_PASSWORD:
	case RECEIVE_LOG_IN_FAIL_PASSWORD:
		rUIMng.ShowWin(&rUIMng.m_LoginWin);
		CUIMng::Instance().m_LoginWin.GetPassInputBox()->GiveFocus(TRUE);
		CurrentProtocolState = RECEIVE_JOIN_SERVER_SUCCESS;
		break;
	case MESSAGE_DELETE_CHARACTER_CONFIRM:
		PopUp(MESSAGE_DELETE_CHARACTER_RESIDENT);
		break;
	case MESSAGE_DELETE_CHARACTER_RESIDENT:
	{
		char resident20[21] = {};
		if (g_iChatInputType == 1)
		{
			g_pSinglePasswdInputBox->GetText(resident20);
		}
		else
		{
			strncpy_s(resident20, sizeof(resident20), InputText[0], _TRUNCATE);
		}
		RequestDeleteCharacter(resident20);
		PopUp(MESSAGE_WAIT);
		break;
	}
	}
}

void CMsgWin::ManageCancelClick()
{
	CUIMng& rUIMng = CUIMng::Instance();
	if (m_nMsgCode == MESSAGE_DELETE_CHARACTER_RESIDENT && g_iChatInputType == 1)
	{
		g_pSinglePasswdInputBox->SetText(NULL);
		g_pSinglePasswdInputBox->SetState(UISTATE_HIDE);
	}
#if defined(__ANDROID__) || defined(MU_IOS)
	if (m_nMsgCode == MESSAGE_DELETE_CHARACTER_RESIDENT)
	{
		MU_MobileStopTextInput();
	}
#endif
	m_nMsgCode = -1;
	rUIMng.HideWin(this);
}

void CMsgWin::InitResidentNumInput()
{
	::ClearInput();
	InputEnable = true;
	InputNumber = 1;
	InputTextMax[0] = kDeleteCharGuardDigits;
	InputTextHide[0] = 1;

	if (g_iChatInputType == 1)
	{
		g_pSinglePasswdInputBox->SetState(UISTATE_NORMAL);
		g_pSinglePasswdInputBox->SetOption(UIOPTION_NUMBERONLY);
		g_pSinglePasswdInputBox->SetBackColor(0, 0, 0, 0);
		g_pSinglePasswdInputBox->SetTextLimit(kDeleteCharGuardDigits);
#if !defined(__ANDROID__) && !defined(MU_IOS)
		g_pSinglePasswdInputBox->GiveFocus(TRUE);
#endif
	}
}

void CMsgWin::RequestDeleteCharacter(char* resident20)
{
	if (g_iChatInputType == 1)
	{
		g_pSinglePasswdInputBox->SetText(NULL);
		g_pSinglePasswdInputBox->SetState(UISTATE_HIDE);
	}
	InputEnable = false;
	if (SelectedHero < 0 || SelectedHero >= 5)
	{
		g_ErrorReport.Write("[AndroidLogin] delete character aborted — invalid SelectedHero=%d\r\n", SelectedHero);
		return;
	}
#if defined(__ANDROID__) || defined(MU_IOS)
	g_ErrorReport.Write(
		"[AndroidLogin] SendRequestDeleteCharacter hero=%s slot=%d\r\n",
		CharactersClient[SelectedHero].ID,
		SelectedHero);
#endif
	SendRequestDeleteCharacter(CharactersClient[SelectedHero].ID, resident20);
}