//*****************************************************************************
// File: MsgWin.h
//*****************************************************************************
#pragma once

#include "Win.h"
#include "Button.h"

#define	MW_MSG_LINE_MAX		2
#define	MW_MSG_ROW_MAX		52

class CMsgWin : public CWin  
{
protected:
	enum MSG_WIN_TYPE
	{
		MWT_NON,
		MWT_BTN_CANCEL,
		MWT_BTN_OK,
		MWT_BTN_BOTH,
		MWT_STR_INPUT,
	};

	CSprite			m_sprBack;
	CSprite			m_sprInput;
	CButton			m_aBtn[2];
	char			m_aszMsg[MW_MSG_LINE_MAX][MW_MSG_ROW_MAX];
	int				m_nMsgLine;
	int				m_nMsgCode;
	MSG_WIN_TYPE	m_eType;
	short			m_nGameExit;
	double			m_dDeltaTickSum;

public:
	CMsgWin();
	virtual ~CMsgWin();
	void Create();
	void SetPosition(int nXCoord, int nYCoord) override;
	void Show(bool bShow) override;
	bool CursorInWin(int nArea) override;
	void PopUp(int nMsgCode, char* pszMsg = NULL);
#if defined(__ANDROID__) || defined(MU_IOS)
	// TouchToVirtualUi / FocusVirtualChatInputAt space — must run from android_main before virtual pad consumes the finger.
	bool AndroidTryFocusDeleteResidentInput(float virtualUiX, float virtualUiY);
#endif

protected:
	void PreRelease() override;
	void UpdateWhileShow(double dDeltaTick) override;
	void UpdateWhileActive(double dDeltaTick) override;
	void RenderControls() override;
	void SetCtrlPosition();
	void SetMsg(MSG_WIN_TYPE eType, LPCTSTR lpszMsg, LPCTSTR lpszMsg2 = NULL);
	void ManageOKClick();
	void ManageCancelClick();
	void InitResidentNumInput();
	void RequestDeleteCharacter();
};

