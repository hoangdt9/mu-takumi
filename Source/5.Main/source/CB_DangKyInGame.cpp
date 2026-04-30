#include "StdAfx.h"
#include "CB_DangKyInGame.h"
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

CB_DangKyInGame* gCB_DangKyInGame;

namespace
{
	const DWORD kRegisterTextColor = 0xFFFFFFFF;
	const float kRegisterWindowWidth = 262.0f;
	const float kRegisterWindowHeight = 250.0f;
	const float kInputWidth = 110.0f;
	const float kInputSpacing = 20.0f;

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
	gInterface.vCaptcha = gInterface.generateCaptcha(4);
}

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

		char captchaText[8] = { 0 };
		CInputCaptCha->GetText(captchaText, sizeof(captchaText));
		std::string captchaInput(captchaText);

		if (!gInterface.check_Captcha(gInterface.vCaptcha, captchaInput))
		{
			gInterface.OpenMessageBox("Error", "Invalid Captcha");
			return;
		}

		RequsetDKTK();
	};

	RenderRegisterText(g_hFontBold, startX + 20.0f, startY + 48.0f, kRegisterWindowWidth - 40.0f, 14.0f, 3, "Tai khoan chi duoc su dung");
	RenderRegisterText(g_hFontBold, startX + 20.0f, startY + 62.0f, kRegisterWindowWidth - 40.0f, 14.0f, 3, "cac ky tu 0-9, a-z");

	startY += 30.0f;

	for (int i = 0; i < TYPE_INPUT_DKTK::eMaxINPUT; ++i)
	{
		RenderRegisterText(g_hFontBold, startX + 18.0f, startY + 48.0f, 96.0f, 16.0f, 1, labels[i]);
		gInterface.DrawBarForm((startX + 120.0f) - 3.0f, (startY + 50.0f) - 3.0f, kInputWidth, 16.0f, 0.0f, 0.0f, 0.0f, 1.0f);

		g_pBCustomMenuInfo->RenderInputBox(
			startX + 120.0f,
			startY + 50.0f,
			kInputWidth,
			14.0f,
			"",
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
		CInputCaptCha->SetTabTarget(NULL);
	}

	gInterface.RenderCaptchaNumber(captchaX, captchaY, CInputCaptCha, gInterface.vCaptcha.c_str());

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

	gInterface.DrawMessageBox();
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

	gInterface.vCaptcha = gInterface.generateCaptcha(4);
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

	switch (lpMsg->ThaoTac)
	{
	case CB_DangKyInGame::eDangKyThanhCong:
		{
			gInterface.OpenMessageBox("Ket Qua", "Dang ky thanh cong\nID : %s", szID);
			CUIMng& rUIMng = CUIMng::Instance();
			rUIMng.m_LoginWin.GetIDInputBox()->SetText(szID);
			rUIMng.m_LoginWin.GetPassInputBox()->SetText(szPass);
			gInterface.Data[eWindow_DangKyInGame].Close();
			Clear();
		}
		break;

	case CB_DangKyInGame::eTaiKhoanDaTonTai:
		gInterface.OpenMessageBox("Ket Qua", "ID %s da ton tai", szID);
		break;

	case CB_DangKyInGame::eDuLieuNhapKhongDung:
		gInterface.OpenMessageBox("Ket Qua", "Thong tin nhap khong hop le");
		break;

	default:
		break;
	}
}
