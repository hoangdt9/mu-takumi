//*****************************************************************************
// File: CharMakeWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "CharMakeWin.h"
#include "Input.h"
#include "UIMng.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzOpenglUtil.h"
#include "DSPlaySound.h"
#include "ZzzAI.h"
#include "ZzzScene.h"
#include "UIControls.h"
#include "wsclientinline.h"
#include "Local.h"
#include "CharacterManager.h"

#if defined(__ANDROID__) || defined(MU_IOS)
#include <SDL.h>
#endif

#define	CMW_OK		0
#define	CMW_CANCEL	1

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
extern int g_iChatInputType;
extern CUITextInputBox* g_pSingleTextInputBox;
#if defined(__ANDROID__) || defined(MU_IOS)
extern bool    g_charNameInputActive;
extern wchar_t g_charNameBuf[11];
extern int     g_charNameLen;
#endif

void MoveCharacterCamera(vec3_t Origin,vec3_t Position,vec3_t Angle);

namespace
{
	const char* const kWizardStats[4] = { "28", "20", "25", "10" };
	const char* const kKnightStats[4] = { "18", "18", "15", "30" };
	const char* const kElfStats[4] = { "22", "25", "20", "15" };
	const char* const kDarkStats[4] = { "26", "26", "26", "26" };
	const char* const kDarkLordStats[4] = { "26", "20", "20", "15" };
	const char* const kSummonerStats[4] = { "21", "21", "18", "23" };
#ifdef PBG_ADD_NEWCHAR_MONK
	const char* const kRageFighterStats[4] = { "32", "27", "25", "20" };
#endif //PBG_ADD_NEWCHAR_MONK

	struct CreateCharacterRenderParameters
	{
		bool overrideAngle;
		float angleX;
		float angleY;
		float angleZ;
		float scale;
		float positionOffsetX;
		float positionOffsetZ;
	};

	CreateCharacterRenderParameters GetCreateCharacterRenderParameters(int classType)
	{
		switch (classType)
		{
		case CLASS_KNIGHT:
			return { true, 0.0f, 0.0f, -12.0f, 6.05f, 0.0f, 0.0f };
		case CLASS_WIZARD:
			return { true, 0.0f, 0.0f, -40.0f, 5.9f, 0.0f, 0.0f };
		case CLASS_ELF:
			return { true, 8.0f, 0.0f, 5.0f, 9.1f, 4.8f, 0.0f };
		case CLASS_DARK:
			return { true, 8.0f, 0.0f, -13.0f, 6.0f, 0.0f, 1.8f };
		case CLASS_DARK_LORD:
			return { true, 8.0f, 0.0f, -18.0f, 6.0f, 0.0f, 0.0f };
		case CLASS_SUMMONER:
			return { true, 2.0f, 0.0f, 2.0f, 9.1f, 4.8f, 4.0f };
#ifdef PBG_ADD_NEWCHAR_MONK
		case CLASS_RAGEFIGHTER:
			return { false, 0.0f, 0.0f, 0.0f, 6.0f, 9.8f, -7.5f };
#endif //PBG_ADD_NEWCHAR_MONK
			default:
				return { false, 0.0f, 0.0f, 0.0f, 6.0f, 0.0f, 0.0f };
			}
		}

	const char* const* GetCreateCharacterStats(int classType)
	{
		switch (classType)
		{
		case CLASS_WIZARD:
			return kWizardStats;
		case CLASS_KNIGHT:
			return kKnightStats;
		case CLASS_ELF:
			return kElfStats;
		case CLASS_DARK:
			return kDarkStats;
		case CLASS_DARK_LORD:
			return kDarkLordStats;
		case CLASS_SUMMONER:
			return kSummonerStats;
#ifdef PBG_ADD_NEWCHAR_MONK
		case CLASS_RAGEFIGHTER:
			return kRageFighterStats;
#endif //PBG_ADD_NEWCHAR_MONK
		default:
			return kKnightStats;
		}
	}

#if defined(__ANDROID__) || defined(MU_IOS)
	void CopyAndroidCharNameToAscii(char* output, size_t outputSize)
	{
		if (output == NULL || outputSize == 0)
		{
			return;
		}

		size_t writeIndex = 0;

		for (int readIndex = 0; g_charNameBuf[readIndex] != L'\0' && writeIndex + 1 < outputSize; ++readIndex)
		{
			const wchar_t character = g_charNameBuf[readIndex];
			output[writeIndex++] = (character >= 0x20 && character <= 0x7E) ? static_cast<char>(character) : '?';
		}

		output[writeIndex] = '\0';
	}
#endif
}

CCharMakeWin::CCharMakeWin()
{
}

CCharMakeWin::~CCharMakeWin()
{
}

void CCharMakeWin::Create()
{
	CInput rInput = CInput::Instance();
	CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight());


	m_winBack.Create(454, 406, -2);


	m_asprBack[CMW_SPR_INPUT].Create(346, 38, BITMAP_LOG_IN);

	m_asprBack[CMW_SPR_STAT].Create(108, 80);

	m_asprBack[CMW_SPR_DESC].Create(454, 51);

	int i;
	for (i = CMW_SPR_STAT; i < CMW_SPR_MAX; ++i)
	{
		m_asprBack[i].SetAlpha(143);
		m_asprBack[i].SetColor(0, 0, 0);
	}

	DWORD adwJobBtnClr[8] =
	{
		CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, CLRDW_GRAY,
		CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, CLRDW_GRAY
	};

	int nText;
	for (i = 0; i < MAX_CLASS; ++i)
	{
#if (BTYPECUSTOMSS< 5)
		if (i >= 5) continue;
#endif
		m_abtnJob[i].Create(108, 26, BITMAP_LOG_IN+1, 4, 2, 1, 0, 3, 3, 3, 0);
#ifdef PBG_ADD_NEWCHAR_MONK
		int _btn_classname[MAX_CLASS] = {20, 21, 22, 23, 24, 1687, 3150};
#else //PBG_ADD_NEWCHAR_MONK
		int _btn_classname[MAX_CLASS] = {20, 21, 22, 23, 24, 1687};
#endif //PBG_ADD_NEWCHAR_MONK
		nText = _btn_classname[i];
		m_abtnJob[i].SetText(GlobalText[nText], adwJobBtnClr);
		CWin::RegisterButton(&m_abtnJob[i]);
	}

	for (i = 0; i < 2; ++i)
	{
		m_aBtn[i].Create(54, 30, BITMAP_BUTTON + i, 3, 2, 1);
		CWin::RegisterButton(&m_aBtn[i]);
	}

	::memset(m_aszJobDesc, 0,
		sizeof(char) * CMW_DESC_LINE_MAX * CMW_DESC_ROW_MAX);
	m_nDescLine = 0;

	m_nSelJob = CLASS_KNIGHT;
	m_abtnJob[m_nSelJob].SetCheck(true);

	UpdateDisplay();
}

void CCharMakeWin::PreRelease()
{
	for (int i = 0; i < CMW_SPR_MAX; ++i)
		m_asprBack[i].Release();
	m_winBack.Release();
}

void CCharMakeWin::SetPosition(int nXCoord, int nYCoord)
{
	int nBaseY = nYCoord + 131;
	int nBaseX = nXCoord + 346;

	int nBtnHeight = m_abtnJob[0].GetHeight();

	m_winBack.SetPosition(nXCoord, nYCoord);

	if (gProtect.m_MainInfo.RemoveClass == 1)
	{
		nBaseY = nYCoord + (246 - nBtnHeight);
		m_asprBack[CMW_SPR_STAT].SetPosition(nBaseX, nBaseY - 107);

		for (int i = 0; i < 4; ++i)
		{
			m_abtnJob[i].SetPosition(nBaseX, nBaseY + i * nBtnHeight);
		}
	}
	else if (gProtect.m_MainInfo.RemoveClass == 2)
	{
		nBaseY += (nBtnHeight + nBtnHeight);
		m_asprBack[CMW_SPR_STAT].SetPosition(nBaseX, nBaseY - 107);

		for (int i = 0; i < 3; ++i)
		{
			m_abtnJob[i].SetPosition(nBaseX, nBaseY + i * nBtnHeight);
		}

		nBaseY = nYCoord + 246;

		for (int i = 3; i <= CLASS_DARK_LORD; ++i)
			m_abtnJob[i].SetPosition(nBaseX, nBaseY + (i - 2) * nBtnHeight);
	}
	else if (gProtect.m_MainInfo.RemoveClass == 3)
	{
		nBaseY += nBtnHeight;
		m_asprBack[CMW_SPR_STAT].SetPosition(nBaseX, nBaseY - 107);

		for (int i = 0; i < 3; ++i)
			m_abtnJob[i].SetPosition(nBaseX, nBaseY + i * nBtnHeight);

		nBaseY = nYCoord + 246;
		m_abtnJob[CLASS_SUMMONER].SetPosition(nBaseX, nBaseY);

		for (int i = 3; i <= CLASS_DARK_LORD; ++i)
			m_abtnJob[i].SetPosition(nBaseX, nBaseY + (i - 2) * nBtnHeight);
	}
	else
	{
		m_asprBack[CMW_SPR_STAT].SetPosition(nBaseX, nYCoord + 24);

		for (int i = 0; i < 3; ++i)
			m_abtnJob[i].SetPosition(nBaseX, nBaseY + i * nBtnHeight);

		m_abtnJob[CLASS_SUMMONER].SetPosition(nBaseX, nBaseY + 3 * nBtnHeight);

		nBaseY = nYCoord + 246;
		m_abtnJob[CLASS_RAGEFIGHTER].SetPosition(nBaseX, nBaseY);

		for (int i = 3; i <= CLASS_DARK_LORD; ++i)
			m_abtnJob[i].SetPosition(nBaseX, nBaseY + (i - 2) * nBtnHeight);
	}

	nBaseY = nYCoord + 325;
	m_aBtn[CMW_OK].SetPosition(nBaseX, nBaseY);
	m_aBtn[CMW_CANCEL].SetPosition(nXCoord + 400, nBaseY);

	m_asprBack[CMW_SPR_INPUT].SetPosition(nXCoord, nYCoord + 317);

	if (g_iChatInputType == 1)
	{
		g_pSingleTextInputBox->SetPosition(
			int((m_asprBack[CMW_SPR_INPUT].GetXPos() + 78) / g_fScreenRate_x),
			int((m_asprBack[CMW_SPR_INPUT].GetYPos() + 21) / g_fScreenRate_y));
	}

	m_asprBack[CMW_SPR_DESC].SetPosition(nXCoord, nYCoord + 355);
}


void CCharMakeWin::Show(bool bShow)
{
	CWin::Show(bShow);

	int i;
	for (i = 0; i < CMW_SPR_MAX; ++i)
		m_asprBack[i].Show(bShow);

	for (i = 0; i < MAX_CLASS; ++i)
		m_abtnJob[i].Show(bShow);
	for (i = 0; i < 2; ++i)
		m_aBtn[i].Show(bShow);

	if (bShow)
	{
		InputTextWidth = 73;
		ClearInput();
		InputEnable = true;
		InputNumber = 1;
		InputTextMax[0] = MAX_ID_SIZE;
		if (g_iChatInputType == 1)
		{
#if defined(__ANDROID__) || defined(MU_IOS)
			g_charNameLen = 0;
			g_charNameBuf[0] = L'\0';
			g_charNameInputActive = true;
			if (g_pSingleTextInputBox != NULL)
			{
				g_pSingleTextInputBox->SetText(NULL);
				g_pSingleTextInputBox->SetState(UISTATE_HIDE);
			}
			MU_MobileStartTextInput();
#else
			g_pSingleTextInputBox->SetState(UISTATE_NORMAL);
			g_pSingleTextInputBox->SetOption(UIOPTION_NULL);
			g_pSingleTextInputBox->SetBackColor(0, 0, 0, 0);
			g_pSingleTextInputBox->SetTextLimit(10);
			g_pSingleTextInputBox->GiveFocus();
#endif
		}
	}
	else
	{
		if (g_iChatInputType == 1)
		{
#if defined(__ANDROID__) || defined(MU_IOS)
			g_charNameInputActive = false;
			g_charNameLen = 0;
			g_charNameBuf[0] = L'\0';
			MU_MobileStopTextInput();
			if (g_pSingleTextInputBox != NULL)
			{
				g_pSingleTextInputBox->SetState(UISTATE_HIDE);
			}
#else
			g_pSingleTextInputBox->SetText(NULL);
			g_pSingleTextInputBox->SetState(UISTATE_HIDE);
#endif
		}
	}
}

bool CCharMakeWin::CursorInWin(int nArea)
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

void CCharMakeWin::UpdateDisplay()
{
	int i;

	const int _SecondClassCnt = 3;

	for(i=0; i<=(MAX_CLASS-1); ++i)
	{
		m_abtnJob[i].SetEnable(true); 
	}

#ifdef PBG_ADD_CHARACTERCARD
	for(i=0; i<CLASS_CHARACTERCARD_TOTALCNT; ++i)
	{
		if(!g_CharCardEnable.bCharacterEnable[i])
			m_abtnJob[i+CLASS_DARK].SetEnable(false);
	}
#else //PBG_ADD_CHARACTERCARD
	m_abtnJob[CLASS_SUMMONER].SetEnable(true);
#endif //PBG_ADD_CHARACTERCARD


	if (m_nSelJob == CLASS_DARK_LORD)
		m_asprBack[CMW_SPR_STAT].SetSize(0, 96, Y);
	else
		m_asprBack[CMW_SPR_STAT].SetSize(0, 80, Y);

	int nText = m_nSelJob == CLASS_SUMMONER ? 1690 : 1705 + m_nSelJob;
#ifdef PBG_ADD_NEWCHAR_MONK
	if(m_nSelJob == CLASS_RAGEFIGHTER)
		nText = 3152;
#endif //PBG_ADD_NEWCHAR_MONK
	m_nDescLine = ::SeparateTextIntoLines(GlobalText[nText], m_aszJobDesc[0],CMW_DESC_LINE_MAX, CMW_DESC_ROW_MAX);

	SelectCreateCharacter();
}

void CCharMakeWin::UpdateWhileActive(double dDeltaTick)
{
	int i, j;
	{
		for (i = 0; i < MAX_CLASS; ++i)
		{
			if (m_abtnJob[i].IsClick())
			{
				for (j = 0; j < MAX_CLASS; ++j)
					m_abtnJob[j].SetCheck(false);
				m_abtnJob[i].SetCheck(true);

				if (m_nSelJob == i)
					break;

				m_nSelJob = i;
				UpdateDisplay();
				break;
			}
		}
	}

	{
		if (g_iChatInputType == 1 && MouseLButtonPush)
		{
			const int inputX = int((m_asprBack[CMW_SPR_INPUT].GetXPos() + 78) / g_fScreenRate_x);
			const int inputY = int((m_asprBack[CMW_SPR_INPUT].GetYPos() + 21) / g_fScreenRate_y);
#if defined(__ANDROID__) || defined(MU_IOS)
			if (::CheckMouseIn(inputX - 10, inputY - 6, 220, 28))
			{
				g_charNameInputActive = true;
				MU_MobileStartTextInput();
			}
#else
			if (::CheckMouseIn(inputX - 10, inputY - 6, 220, 28))
			{
				g_pSingleTextInputBox->GiveFocus();
			}
#endif
		}

		if(m_aBtn[CMW_OK].IsClick())
		{
			RequestCreateCharacter();
		}
		else if(m_aBtn[CMW_CANCEL].IsClick())
		{
			CUIMng::Instance().HideWin(this);
		}
		else if (CInput::Instance().IsKeyDown(VK_RETURN))
		{
			::PlayBuffer(SOUND_CLICK01);
			RequestCreateCharacter();
		}
		else if (CInput::Instance().IsKeyDown(VK_ESCAPE))
		{
			::PlayBuffer(SOUND_CLICK01);
			CUIMng::Instance().HideWin(this);
			CUIMng::Instance().SetSysMenuWinShow(false);
		}
	}
	UpdateCreateCharacter();
}

void CCharMakeWin::RequestCreateCharacter()
{
	if (g_iChatInputType == 1)
	{
#if defined(__ANDROID__) || defined(MU_IOS)
		CopyAndroidCharNameToAscii(InputText[0], MAX_ID_SIZE + 1);
#else
		g_pSingleTextInputBox->GetText(InputText[0]);
#endif
	}

	CUIMng& rUIMng = CUIMng::Instance();

	if (::strlen(InputText[0]) < 4)
		rUIMng.PopUpMsgWin(MESSAGE_MIN_LENGTH);
    else if(::CheckName())
		rUIMng.PopUpMsgWin(MESSAGE_ID_SPACE_ERROR);
	else if(CheckSpecialText(InputText[0]))
		rUIMng.PopUpMsgWin(MESSAGE_SPECIAL_NAME);
	else
	{
		SendRequestCreateCharacter(InputText[0], CharacterView.Class, CharacterView.Skin);
		rUIMng.HideWin(this);
		rUIMng.PopUpMsgWin(MESSAGE_WAIT);
	}
}

void CCharMakeWin::RenderControls()
{
	RenderCreateCharacter();
	::EnableAlphaTest();

	int i;
	for (i = 0; i < CMW_SPR_MAX; ++i)
	{
		m_asprBack[i].Render();
	}
	CWin::RenderButtons();
	g_pRenderText->SetFont(g_hFixFont);
	g_pRenderText->SetTextColor(CLRDW_WHITE);
	g_pRenderText->SetBgColor(0);

	const char* const* apszStat = GetCreateCharacterStats(m_nSelJob);
	int nStatBaseX = m_asprBack[CMW_SPR_STAT].GetXPos() + 22;
	int nStatY;
	for (i = 0; i < 4; ++i)
	{
		nStatY = int((m_asprBack[CMW_SPR_STAT].GetYPos() + 10 + i * 17)
			/ g_fScreenRate_y);

		g_pRenderText->SetTextColor(CLRDW_ORANGE);
		g_pRenderText->RenderText(int((nStatBaseX + 54) / g_fScreenRate_x), nStatY,
			apszStat[i]);
		g_pRenderText->SetTextColor(CLRDW_WHITE);
		g_pRenderText->RenderText(int(nStatBaseX / g_fScreenRate_x), nStatY,
			GlobalText[1701 + i]);
	}

	if (m_nSelJob == CLASS_DARK_LORD)
	{
		nStatY = int((m_asprBack[CMW_SPR_STAT].GetYPos() + 10 + 4 * 17)	/ g_fScreenRate_y);

		g_pRenderText->SetTextColor(CLRDW_ORANGE);
		g_pRenderText->RenderText(int((nStatBaseX + 54) / g_fScreenRate_x), nStatY, "25");
		g_pRenderText->SetTextColor(CLRDW_WHITE);
		g_pRenderText->RenderText(int(nStatBaseX / g_fScreenRate_x), nStatY, GlobalText[1738]);
	}

	{
		for (i = 0; i < m_nDescLine; ++i)
		{
			g_pRenderText->RenderText(int((m_asprBack[CMW_SPR_DESC].GetXPos() + 10) / g_fScreenRate_x),
				int((m_asprBack[CMW_SPR_DESC].GetYPos() + 12 + i * 19)
				/ g_fScreenRate_y), m_aszJobDesc[i]);
		}
	}

	g_pRenderText->SetFont(g_hFont);
	
	if (g_iChatInputType == 1)
	{
#if defined(__ANDROID__) || defined(MU_IOS)
		char inputText[MAX_ID_SIZE + 1] = { 0 };
		const int textX = int((m_asprBack[CMW_SPR_INPUT].GetXPos() + 78) / g_fScreenRate_x);
		const int textY = int((m_asprBack[CMW_SPR_INPUT].GetYPos() + 21) / g_fScreenRate_y);
		CopyAndroidCharNameToAscii(inputText, sizeof(inputText));
		g_pRenderText->SetTextColor(CLRDW_WHITE);
		g_pRenderText->SetBgColor(0);
		if (inputText[0] != '\0')
		{
			g_pRenderText->RenderText(textX, textY, inputText);
		}
		if (g_charNameInputActive != false && (MU_MobileGetTicks() / 500u) % 2u == 0u)
		{
			SIZE textSize = { 0, 0 };
			if (inputText[0] != '\0')
			{
				g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), inputText, strlen(inputText), &textSize);
			}
			g_pRenderText->RenderText(textX + int(textSize.cx / g_fScreenRate_x), textY, "|");
		}
#else
		g_pSingleTextInputBox->Render();
#endif
	}
	else if (g_iChatInputType == 0)
		::RenderInputText(
			int((m_asprBack[CMW_SPR_INPUT].GetXPos() + 78) / g_fScreenRate_x),
			int((m_asprBack[CMW_SPR_INPUT].GetYPos() + 21) / g_fScreenRate_y),
			0);
}

void CCharMakeWin::SelectCreateCharacter()
{
	CharacterView.Class = m_nSelJob;
	CreateCharacterPointer(&CharacterView,MODEL_FACE+CharacterView.Class,0,0);
	CharacterView.Object.Kind = 0;
	SetAction(&CharacterView.Object,1);
}

void CCharMakeWin::UpdateCreateCharacter()
{
	if (!CharacterAnimation(&CharacterView, &CharacterView.Object))
		SetAction(&CharacterView.Object,0);
}
extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayHeight;
extern int DisplayWinMid;
extern int DisplayHeightExt;
extern int DisplayWinExt;
extern int DisplayWinReal;
void CCharMakeWin::RenderCreateCharacter()
{
	OBJECT *o = &CharacterView.Object;
	vec3_t Position, Angle;
	vec3_t savedAngle;
	const float savedScale = o->Scale;

 	Vector(1.0f,1.0f,1.0f,o->Light);
	Vector(10,-500.f,48.f,Position);
	Vector(-90.f,0.f,0.f,Angle);
    CameraFOV = 10.f;
	MoveCharacterCamera(CharacterView.Object.Position,Position,Angle);

	const int vpX = m_winBack.GetXPos()/g_fScreenRate_x;
	const int vpY = m_winBack.GetYPos()/g_fScreenRate_y;
	const int vpW = 410/g_fScreenRate_x;
	const int vpH = 335/g_fScreenRate_y;

	BeginOpengl(vpX, vpY, vpW, vpH);

#if(WIDE_SCREEN)
	const int scissorX = vpX*WindowWidth/(WindowWidth/g_fScreenRate_x);
	const int scissorY = vpY*WindowHeight/(WindowHeight/g_fScreenRate_y);
	const int scissorW = vpW*WindowWidth/(WindowWidth/g_fScreenRate_x);
	const int scissorH = vpH*WindowHeight/(WindowHeight/g_fScreenRate_y);
#else
	const int scissorX = vpX * WindowWidth / 640;
	const int scissorY = vpY * WindowHeight / 480;
	const int scissorW = vpW * WindowWidth / 640;
	const int scissorH = vpH * WindowHeight / 480;
#endif

	glEnable(GL_SCISSOR_TEST);
	glScissor(scissorX, WindowHeight - (scissorY + scissorH), scissorW, scissorH);

	const CreateCharacterRenderParameters params = GetCreateCharacterRenderParameters(CharacterView.Class);
	const float savedPosX = CharacterView.Object.Position[0];
	const float savedPosZ = CharacterView.Object.Position[2];

	VectorCopy(o->Angle, savedAngle);

	if (params.overrideAngle != false)
	{
		Vector(params.angleX, params.angleY, params.angleZ, o->Angle);
	}

	o->Scale = params.scale;

	if (params.positionOffsetX != 0.0f)
	{
		CharacterView.Object.Position[0] += params.positionOffsetX;
	}

	if (params.positionOffsetZ != 0.0f)
	{
		CharacterView.Object.Position[2] += params.positionOffsetZ;
	}

	RenderCharacter(&CharacterView,o);

	CharacterView.Object.Position[0] = savedPosX;
	CharacterView.Object.Position[2] = savedPosZ;
	VectorCopy(savedAngle, o->Angle);
	o->Scale = savedScale;

	glDisable(GL_SCISSOR_TEST);

	glViewport2(0,0,WindowWidth,WindowHeight);

	EndOpengl();
}
