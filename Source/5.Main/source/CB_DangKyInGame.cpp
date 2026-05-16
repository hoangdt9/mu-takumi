#include "StdAfx.h"
#include "CB_DangKyInGame.h"
#include "TakumiUserNotify.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"
#include "UIMng.h"
#include "Utilities/Log/ErrorReport.h"
#if defined(__ANDROID__) || defined(MU_IOS)
#include "Platform/PlatformDefs.h"
#include "Platform/MobilePlatform.h"
#include "Input.h"
#endif

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

CB_DangKyInGame* gCB_DangKyInGame;

namespace
{
#if defined(__ANDROID__)
	DWORD ReadRegisterResultCode(const BYTE* packet)
	{
		if (packet == NULL)
		{
			return 0;
		}

		if (packet[0] == 0xC1 && packet[1] >= 8)
		{
			DWORD result = 0;
			memcpy(&result, packet + 4, sizeof(result));
			return result;
		}

		return reinterpret_cast<const XULY_CGPACKET*>(packet)->ThaoTac;
	}
#endif
	const DWORD kRegisterTextColor = 0xFFFFFFFF;
	const float kRegisterWindowWidth = 262.0f;
	const float kRegisterWindowHeight = 250.0f;
	const float kInputWidth = 110.0f;
	const float kInputSpacing = 20.0f;

#if defined(__ANDROID__) || defined(MU_IOS)
	bool CursorInRegisterRectVirt(float virtX, float virtY, float virtW, float virtH)
	{
		const POINT pt = CInput::Instance().GetCursorPos();
		const float x = virtX * g_fScreenRate_x;
		const float y = virtY * g_fScreenRate_y;
		const float w = virtW * g_fScreenRate_x;
		const float h = virtH * g_fScreenRate_y;
		return static_cast<float>(pt.x) >= x
			&& static_cast<float>(pt.x) < x + w
			&& static_cast<float>(pt.y) >= y
			&& static_cast<float>(pt.y) < y + h;
	}

	bool CursorInTextInputBox(CUITextInputBox* input)
	{
		if (input == NULL)
		{
			return false;
		}

		return CursorInRegisterRectVirt(
			static_cast<float>(input->GetPosition_x()),
			static_cast<float>(input->GetPosition_y()),
			static_cast<float>(input->GetWidth()),
			static_cast<float>(input->GetHeight()));
	}

	void GetRegisterWindowVirt(float& startX, float& startY)
	{
		if (gInterface.Data[eWindow_DangKyInGame].FirstLoad)
		{
			startX = gInterface.Data[eWindow_DangKyInGame].X;
			startY = gInterface.Data[eWindow_DangKyInGame].Y;
		}
		else
		{
			startX = (MAX_WIN_WIDTH / 2) - (kRegisterWindowWidth / 2);
			startY = 150.0f;
		}
	}

	void ComputeRegisterButtonVirt(float& buttonX, float& buttonY)
	{
		float startX = 0.0f;
		float startY = 0.0f;
		GetRegisterWindowVirt(startX, startY);
		startY += 30.0f;
		startY += 4.0f * kInputSpacing;
		startY += 20.0f + 30.0f;
		buttonX = startX + 100.0f;
		buttonY = startY + 45.0f;
	}

	void DismissRegisterTextInput()
	{
		::SetFocus(nullptr);
		MU_MobileStopTextInput();
	}
#endif

	const char* ResolveRegisterLabel(const char* text, const char* fallback)
	{
		if (text == NULL || text[0] == '\0' || strcmp(text, "Null") == 0 || strcmp(text, "NULL") == 0)
		{
			return fallback;
		}

		return text;
	}

	void RenderRegisterText(HFONT font, float boxX, float boxY, float boxW, float boxH, int align, const char* text)
	{
		if (text == NULL || text[0] == '\0')
		{
			return;
		}

		DWORD backupTextColor = g_pRenderText->GetTextColor();
		DWORD backupBgColor = g_pRenderText->GetBgColor();

		gInterface.DrawBarForm(boxX, boxY, boxW, boxH, 0.0f, 0.0f, 0.0f, 0.60f);
		g_pRenderText->SetFont(font != NULL ? font : g_hFont);
		g_pRenderText->SetShadowText(0);
		g_pRenderText->SetTextColor(kRegisterTextColor);
		g_pRenderText->SetBgColor(0);
		g_pRenderText->RenderText((int)boxX, (int)boxY + 1, text, (int)boxW, 0, align);
		g_pRenderText->SetFont(g_hFont);
		g_pRenderText->SetTextColor(backupTextColor);
		g_pRenderText->SetBgColor(backupBgColor);
	}

	// Same virtual coords as DrawBarForm / RenderCaptchaNumber — do not use RenderInputBox here
	// (it divides Pos by g_fScreenRate and shrinks W/H, which misaligns or hides typed text on mobile).
	void RenderRegisterFieldInput(
		float posX,
		float posY,
		CUITextInputBox*& input,
		UIOPTIONS option,
		int maxChar,
		bool isPassword)
	{
		const int fieldW = static_cast<int>(kInputWidth);
		const int fieldH = 14;

		if (input == NULL)
		{
			input = new CUITextInputBox;
			input->Init(g_hWnd, fieldW, fieldH, maxChar, isPassword);
			input->SetTextColor(255, 255, 255, 255);
			input->SetBackColor(255, 0, 0, 0);
			input->SetFont(g_hFont);
			input->SetState(UISTATE_NORMAL);
			input->SetOption(option);
		}
		else
		{
			input->SetSize(fieldW, fieldH);
		}

		input->SetPosition(static_cast<int>(posX), static_cast<int>(posY));
		input->Render();
		input->DoAction();
	}
}

CB_DangKyInGame::CB_DangKyInGame()
{
	for (int i = 0; i < TYPE_INPUT_DKTK::eMaxINPUT; ++i)
	{
		CInputData[i] = NULL;
	}

	CInputCaptCha = NULL;
	TimeSendRegTK = GetTickCount();
	OpenDKTK = false;
	m_ExpectedCaptcha.clear();
}

CB_DangKyInGame::~CB_DangKyInGame()
{
	Clear();
}

void CB_DangKyInGame::Clear()
{
	for (int i = 0; i < TYPE_INPUT_DKTK::eMaxINPUT; ++i)
	{
		SAFE_DELETE(CInputData[i]);
	}

	SAFE_DELETE(CInputCaptCha);

	TimeSendRegTK = GetTickCount();
	OpenDKTK = false;
	m_ExpectedCaptcha.clear();
}

void CB_DangKyInGame::PrepareCloseRegisterWindow()
{
	auto releaseInput = [](CUITextInputBox* input)
	{
		if (input == NULL)
		{
			return;
		}

		if (input->HaveFocus())
		{
#if defined(__ANDROID__) || defined(MU_IOS)
			::SetFocus(nullptr);
			MU_MobileStopTextInput();
#else
			SetFocus(g_hWnd);
#endif
		}

		input->SetTabTarget(NULL);
		input->SetState(UISTATE_HIDE);
	};

	for (int i = 0; i < TYPE_INPUT_DKTK::eMaxINPUT; ++i)
	{
		releaseInput(CInputData[i]);
	}

	releaseInput(CInputCaptCha);
}

void CB_DangKyInGame::OpenOnOff()
{
	if (GetTickCount() - gInterface.Data[eWindow_DangKyInGame].EventTick <= 300)
	{
		return;
	}

	gInterface.Data[eWindow_DangKyInGame].EventTick = GetTickCount();

	if (gInterface.Data[eWindow_DangKyInGame].OnShow)
	{
		gInterface.Data[eWindow_DangKyInGame].Close();
		Clear();
		return;
	}

	gInterface.Data[eWindow_DangKyInGame].Open();
	OpenDKTK = true;
	m_ExpectedCaptcha = gInterface.generateCaptcha(4);
	gInterface.vCaptcha = m_ExpectedCaptcha;
}

#if defined(__ANDROID__) || defined(MU_IOS)
void CB_DangKyInGame::UpdateMobileInput()
{
	if (!gInterface.Data[eWindow_DangKyInGame].OnShow)
	{
		return;
	}

	if (!CInput::Instance().IsLBtnDn())
	{
		return;
	}

	float winX = 0.0f;
	float winY = 0.0f;
	GetRegisterWindowVirt(winX, winY);

	if (!CursorInRegisterRectVirt(winX, winY, kRegisterWindowWidth, kRegisterWindowHeight))
	{
		if (AndroidHasFocusedTextInput())
		{
			DismissRegisterTextInput();
		}
		return;
	}

	float fieldY = winY + 30.0f;
	for (int i = 0; i < TYPE_INPUT_DKTK::eMaxINPUT; ++i)
	{
		const float fieldX = winX + 120.0f;
		const float fieldYTop = fieldY + 50.0f;
		if (CursorInTextInputBox(CInputData[i])
			|| CursorInRegisterRectVirt(fieldX, fieldYTop, kInputWidth, 14.0f))
		{
			if (CInputData[i] != NULL)
			{
				CInputData[i]->GiveFocus(TRUE);
			}
			return;
		}
		fieldY += kInputSpacing;
	}

	if (CursorInTextInputBox(CInputCaptCha)
		|| CursorInRegisterRectVirt(winX + 105.0f, fieldY + 45.0f, 120.0f, 30.0f))
	{
		if (CInputCaptCha != NULL)
		{
			CInputCaptCha->GiveFocus(TRUE);
		}
		return;
	}

	float buttonX = 0.0f;
	float buttonY = 0.0f;
	ComputeRegisterButtonVirt(buttonX, buttonY);
	const bool onRegisterButton = CursorInRegisterRectVirt(buttonX, buttonY, 100.0f, 12.0f);
	if (!onRegisterButton && AndroidHasFocusedTextInput())
	{
		DismissRegisterTextInput();
	}
}
#endif

bool CB_DangKyInGame::RenderWindow(int X, int Y)
{
	(void)X;
	(void)Y;

	if (!gInterface.Data[eWindow_DangKyInGame].OnShow || gCB_DangKyInGame == NULL)
	{
		if (OpenDKTK)
		{
			Clear();
		}

		return false;
	}

	OpenDKTK = true;

	const char* labels[TYPE_INPUT_DKTK::eMaxINPUT] =
	{
		"TAI KHOAN :",
		"MAT KHAU :",
		"7 SO BAO MAT :",
		"SO DIEN THOAI :"
	};

	const int inputOptions[TYPE_INPUT_DKTK::eMaxINPUT] =
	{
		UIOPTION_NOLOCALIZEDCHARACTERS,
		UIOPTION_NOLOCALIZEDCHARACTERS,
		UIOPTION_NUMBERONLY,
		UIOPTION_NUMBERONLY
	};

	const int maxInput[TYPE_INPUT_DKTK::eMaxINPUT] =
	{
		MAX_ID_SIZE,
		MAX_PASSWORD_SIZE,
		7,
		11
	};

	float startX = (MAX_WIN_WIDTH / 2) - (kRegisterWindowWidth / 2);
	float startY = 150.0f;
	gInterface.Data[eWindow_DangKyInGame].AllowMove = false;

	g_pBCustomMenuInfo->gDrawWindowCustom(
		&startX,
		&startY,
		kRegisterWindowWidth,
		kRegisterWindowHeight,
		eWindow_DangKyInGame,
		"Dang Ky Tai Khoan");

	if (!gInterface.Data[eWindow_DangKyInGame].OnShow)
	{
		Clear();
		return false;
	}

	auto submitRegister = [&]()
	{
		if (CInputCaptCha == NULL)
		{
			return;
		}

		CInputCaptCha->DoAction(TRUE);
		char captchaText[8] = { 0 };
		CInputCaptCha->GetText(captchaText, sizeof(captchaText));
		std::string captchaInput(captchaText);
		std::string expectedCaptcha = m_ExpectedCaptcha;

		if (!gInterface.check_Captcha(expectedCaptcha, captchaInput))
		{
			TakumiUserNotify_ShowError("Invalid Captcha");
			return;
		}

		RequsetDKTK();
	};

	RenderRegisterText(g_hFontBold, startX + 20.0f, startY + 48.0f, kRegisterWindowWidth - 40.0f, 14.0f, 3, "Tai khoan chi duoc su dung");
	RenderRegisterText(g_hFontBold, startX + 20.0f, startY + 62.0f, kRegisterWindowWidth - 40.0f, 14.0f, 3, "cac ky tu 0-9, a-z");

	startY += 30.0f;

	for (int i = 0; i < TYPE_INPUT_DKTK::eMaxINPUT; ++i)
	{
		const float fieldX = startX + 120.0f;
		const float fieldY = startY + 50.0f;

		RenderRegisterText(g_hFontBold, startX + 18.0f, startY + 48.0f, 96.0f, 16.0f, 1, labels[i]);
		gInterface.DrawBarForm(fieldX - 3.0f, fieldY - 3.0f, kInputWidth, 16.0f, 0.0f, 0.0f, 0.0f, 1.0f);

		RenderRegisterFieldInput(
			fieldX,
			fieldY,
			CInputData[i],
			(UIOPTIONS)inputOptions[i],
			maxInput[i],
			(i == Pass ? TRUE : FALSE));

		if (CInputData[i] != NULL)
		{
			CInputData[i]->SetTextColor(255, 255, 255, 255);
			CInputData[i]->SetBackColor(255, 0, 0, 0);
		}

		startY += kInputSpacing;
	}

	startY += 20.0f;

	const float captchaX = startX + 105.0f;
	const float captchaY = startY + 45.0f;
	RenderRegisterText(
		(HFONT)g_hFontBold,
		startX + 18.0f,
		startY + 48.0f,
		96.0f,
		16.0f,
		1,
		ResolveRegisterLabel(gOther.TextVN_NAPGAME[13], "Captcha :"));

	if (CInputCaptCha == NULL)
	{
		CInputCaptCha = new CUITextInputBox;
		CInputCaptCha->Init(g_hWnd, 40, 14, 4);
		CInputCaptCha->SetBackColor(0, 0, 0, 0);
		CInputCaptCha->SetTextColor(255, 255, 255, 255);
		CInputCaptCha->SetFont((HFONT)g_hFont);
		CInputCaptCha->SetState(UISTATE_NORMAL);
		CInputCaptCha->SetOption(UIOPTION_NUMBERONLY);
	}

	if (CInputData[Account] != NULL && CInputData[Pass] != NULL)
	{
		CInputData[Account]->SetTabTarget(CInputData[Pass]);
	}

	if (CInputData[Pass] != NULL && CInputData[Snonumber] != NULL)
	{
		CInputData[Pass]->SetTabTarget(CInputData[Snonumber]);
	}

	if (CInputData[Snonumber] != NULL && CInputData[Phone] != NULL)
	{
		CInputData[Snonumber]->SetTabTarget(CInputData[Phone]);
	}

	if (CInputData[Phone] != NULL && CInputCaptCha != NULL)
	{
		CInputData[Phone]->SetTabTarget(CInputCaptCha);
	}

	if (CInputCaptCha != NULL)
	{
#if defined(__ANDROID__) || defined(MU_IOS)
		CInputCaptCha->SetTabTarget(NULL);
#else
		CInputCaptCha->SetTabTarget(CInputData[Account]);
#endif
	}

	gInterface.RenderCaptchaNumber(captchaX, captchaY, CInputCaptCha, m_ExpectedCaptcha.c_str());

	startY += 30.0f;

	if (g_pBCustomMenuInfo->DrawButton(
		startX + 100.0f,
		startY + 45.0f,
		100.0f,
		12,
		const_cast<char*>(ResolveRegisterLabel(gOther.TextVN_NAPGAME[14], "Dang Ky")),
		80.0f,
		true))
	{
		submitRegister();
	}
#if defined(__ANDROID__) || defined(MU_IOS)
	else if (CInputCaptCha != NULL && CInputCaptCha->HaveFocus() && SEASON3B::IsPress(VK_RETURN))
	{
		submitRegister();
	}
#endif

	return true;
}

bool CB_DangKyInGame::RequsetDKTK()
{
	if (CInputData[Account] == NULL
		|| CInputData[Pass] == NULL
		|| CInputData[Snonumber] == NULL
		|| CInputData[Phone] == NULL)
	{
		return false;
	}

	char szID[MAX_ID_SIZE + 1] = { 0 };
	char szPass[MAX_PASSWORD_SIZE + 1] = { 0 };
	char szSno[7 + 1] = { 0 };
	char szSDT[11 + 1] = { 0 };

	CInputData[Account]->GetText(szID, MAX_ID_SIZE + 1);
	CInputData[Pass]->GetText(szPass, MAX_PASSWORD_SIZE + 1);
	CInputData[Snonumber]->GetText(szSno, sizeof(szSno));
	CInputData[Phone]->GetText(szSDT, sizeof(szSDT));

	if (TimeSendRegTK > GetTickCount())
	{
		gInterface.OpenMessageBox("Error", "Thao tac cham lai");
		return false;
	}

	if (strlen(szID) < 1)
	{
		gInterface.OpenMessageBox("Error", "Vui long nhap tai khoan");
		return false;
	}

	if (strlen(szPass) < 1)
	{
		gInterface.OpenMessageBox("Error", "Vui long nhap mat khau");
		return false;
	}

	if (strlen(szSno) < 7)
	{
		gInterface.OpenMessageBox("Error", "Vui long nhap 7 so bao mat");
		return false;
	}

	if (strlen(szSDT) < 10)
	{
		gInterface.OpenMessageBox("Error", "Vui long nhap so dien thoai");
		return false;
	}

	if (!CheckChuoiKyTuDacBiet(szID) || !CheckChuoiKyTuDacBiet(szPass))
	{
		gInterface.OpenMessageBox("Error", "Tai khoan hoac mat khau co ky tu khong hop le");
		return false;
	}

	PMSG_REGISTER_MAIN_SEND pMsg;
	pMsg.header.set(0xD3, 0x05, sizeof(pMsg));
	pMsg.TypeSend = 0x01;

	memset(pMsg.account, 0, sizeof(pMsg.account));
	memset(pMsg.password, 0, sizeof(pMsg.password));
	memset(pMsg.numcode, 0, sizeof(pMsg.numcode));
	memset(pMsg.sodienthoai, 0, sizeof(pMsg.sodienthoai));

	memcpy(pMsg.account, szID, min(sizeof(pMsg.account) - 1, strlen(szID)));
	memcpy(pMsg.password, szPass, min(sizeof(pMsg.password) - 1, strlen(szPass)));
	memcpy(pMsg.numcode, szSno, min(sizeof(pMsg.numcode) - 1, strlen(szSno)));
	memcpy(pMsg.sodienthoai, szSDT, min(sizeof(pMsg.sodienthoai) - 1, strlen(szSDT)));

	DataSend((LPBYTE)&pMsg, pMsg.header.size);

	m_ExpectedCaptcha = gInterface.generateCaptcha(4);
	gInterface.vCaptcha = m_ExpectedCaptcha;
	TimeSendRegTK = GetTickCount() + 5000;

	return true;
}

void CB_DangKyInGame::RecvKQRegInGame(XULY_CGPACKET* lpMsg)
{
	if (lpMsg == NULL)
	{
		return;
	}

	char szID[MAX_ID_SIZE + 1] = { 0 };
	char szPass[MAX_PASSWORD_SIZE + 1] = { 0 };

	if (CInputData[Account] != NULL)
	{
		CInputData[Account]->GetText(szID, sizeof(szID));
	}

	if (CInputData[Pass] != NULL)
	{
		CInputData[Pass]->GetText(szPass, sizeof(szPass));
	}

#if defined(__ANDROID__)
	const DWORD thaoTac = ReadRegisterResultCode(reinterpret_cast<const BYTE*>(lpMsg));
#else
	const DWORD thaoTac = lpMsg->ThaoTac;
#endif

	switch (thaoTac)
	{
	case CB_DangKyInGame::eDangKyThanhCong:
		{
			CUIMng& rUIMng = CUIMng::Instance();
			if (CUITextInputBox* idBox = rUIMng.m_LoginWin.GetIDInputBox())
			{
				idBox->SetText(szID);
			}

			if (CUITextInputBox* passBox = rUIMng.m_LoginWin.GetPassInputBox())
			{
				passBox->SetText(szPass);
			}

			PrepareCloseRegisterWindow();
			gInterface.Data[eWindow_DangKyInGame].Close();
			// Defer SAFE_DELETE of register inputs until RenderWindow sees OnShow==false
			// (avoids SIGABRT when focused CUITextInputBox HWND is destroyed mid-frame).
			OpenDKTK = true;

			TakumiUserNotify_ShowInfo("Ket Qua", "Dang ky thanh cong\nTai khoan: %s", szID);
			g_ErrorReport.Write("[DangKyInGame] notify ok account=%s\r\n", szID);
		}
		break;

	case CB_DangKyInGame::eTaiKhoanDaTonTai:
		gInterface.OpenMessageBox("Error", "ID %s da ton tai", szID);
		g_ErrorReport.Write("[DangKyInGame] account exists account=%s\r\n", szID);
		break;

	case CB_DangKyInGame::eDuLieuNhapKhongDung:
		gInterface.OpenMessageBox("Error", "Thong tin nhap khong hop le");
		g_ErrorReport.Write("[DangKyInGame] invalid input\r\n");
		break;

	default:
		break;
	}
}
