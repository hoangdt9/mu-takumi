// NewUICharacterInfoWindow.cpp: implementation of the CNewUICharacterInfoWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUICharacterInfoWindow.h"
#include "NewUISystem.h"
#include "CharacterManager.h"
#include "CSItemOption.h"
#include "ZzzBMD.h"
#include "ZzzCharacter.h"
#include "UIControls.h"
#include "ZzzInterface.h"
#include "ZzzScene.h"
#include "ZzzInventory.h"
#include "SkillManager.h"
#include "UIJewelHarmony.h"
#include "UIManager.h"
#include "wsclientinline.h"
#include "ServerListManager.h"
#include "CustomMessage.h"
#include "CB_StatsAdvance.h"
#include "Other.h"
#include "NewUICustomMessageBox.h" 
using namespace SEASON3B;
int FixYSetPage = 20;
extern int g_SelectedStatType;
SEASON3B::CNewUICharacterInfoWindow::CNewUICharacterInfoWindow()
{
	m_pNewUIMng = NULL;
	m_iCurrentOpenTab = 0;
	m_Pos.x = m_Pos.y = 0;
}

SEASON3B::CNewUICharacterInfoWindow::~CNewUICharacterInfoWindow()
{
	Release();
}

bool SEASON3B::CNewUICharacterInfoWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if (NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_CHARACTER, this);

	SetPos(x, y);

	LoadImages();

	SetButtonInfo();

	Show(false);

	return true;
}

void SEASON3B::CNewUICharacterInfoWindow::SetButtonInfo()
{
	//=== Table Character
	m_TabBtn.UnRegisterRadioButton();
	std::list<unicode::t_string> ltext;
	ltext.push_back(gOther.Text_ChacracterInfo[0]);

	if (gProtect.m_MainInfo.OnBangChiSo)
	{
		ltext.push_back(gOther.Text_ChacracterInfo[1]);
		m_TabBtn.CreateRadioGroup(2, BITMAP_GUILDINFO_BEGIN, TRUE);
		
	}
	else
	{
		m_TabBtn.CreateRadioGroup(1, BITMAP_GUILDINFO_BEGIN, TRUE);
	}
	m_TabBtn.ChangeRadioText(ltext);
	m_TabBtn.ChangeRadioButtonInfo(true, m_Pos.x + 10.f, m_Pos.y + 23.f, 56, 22);
	m_TabBtn.ChangeFrame(m_iCurrentOpenTab);

	unicode::t_char strText[256];


	m_BtnStat[STAT_STRENGTH].ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_STAT, false);
	m_BtnStat[STAT_STRENGTH].ChangeButtonInfo(m_Pos.x + 160, m_Pos.y + HEIGHT_STRENGTH + 2, 16, 15);

	m_BtnStat[STAT_DEXTERITY].ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_STAT, false);
	m_BtnStat[STAT_DEXTERITY].ChangeButtonInfo(m_Pos.x + 160, m_Pos.y + HEIGHT_DEXTERITY + 2, 16, 15);

	m_BtnStat[STAT_VITALITY].ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_STAT, false);
	m_BtnStat[STAT_VITALITY].ChangeButtonInfo(m_Pos.x + 160, m_Pos.y + HEIGHT_VITALITY + 2, 16, 15);

	m_BtnStat[STAT_ENERGY].ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_STAT, false);
	m_BtnStat[STAT_ENERGY].ChangeButtonInfo(m_Pos.x + 160, m_Pos.y + HEIGHT_ENERGY + 2, 16, 15);

	m_BtnStat[STAT_CHARISMA].ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_STAT, false);
	m_BtnStat[STAT_CHARISMA].ChangeButtonInfo(m_Pos.x + 160, m_Pos.y + HEIGHT_CHARISMA + 2, 16, 15);


	//

	m_BtnPoint[STAT_STRENGTH].ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_STAT_TRU, false);
	m_BtnPoint[STAT_STRENGTH].ChangeButtonInfo(m_Pos.x + 85, m_Pos.y + HEIGHT_STRENGTH + 2, 16, 15);

	m_BtnPoint[STAT_DEXTERITY].ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_STAT_TRU, false);
	m_BtnPoint[STAT_DEXTERITY].ChangeButtonInfo(m_Pos.x + 85, m_Pos.y + HEIGHT_DEXTERITY + 2, 16, 15);

	m_BtnPoint[STAT_VITALITY].ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_STAT_TRU, false);
	m_BtnPoint[STAT_VITALITY].ChangeButtonInfo(m_Pos.x + 85, m_Pos.y + HEIGHT_VITALITY + 2, 16, 15);

	m_BtnPoint[STAT_ENERGY].ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_STAT_TRU, false);
	m_BtnPoint[STAT_ENERGY].ChangeButtonInfo(m_Pos.x + 85, m_Pos.y + HEIGHT_ENERGY + 2, 16, 15);

	m_BtnPoint[STAT_CHARISMA].ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_STAT_TRU, false);
	m_BtnPoint[STAT_CHARISMA].ChangeButtonInfo(m_Pos.x + 85, m_Pos.y + HEIGHT_CHARISMA + 2, 16, 15);

	m_Pos.y = m_Pos.y - FixYSetPage;
	m_BtnExit.ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_EXIT, false);
	m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);
	unicode::_sprintf(strText, GlobalText[927], "C");
	m_BtnExit.ChangeToolTipText(strText, true);
	m_BtnQuest.ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_QUEST, false);
	m_BtnQuest.ChangeButtonInfo(m_Pos.x + 50, m_Pos.y + 392, 36, 29);
	unicode::_sprintf(strText, "%s(%s)", GlobalText[1140], "T");
	m_BtnQuest.ChangeToolTipText(strText, true);
	m_BtnPet.ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_PET, false);
	m_BtnPet.ChangeButtonInfo(m_Pos.x + 87, m_Pos.y + 392, 36, 29);
	m_BtnPet.ChangeToolTipText(GlobalText[1217], true);
#if (OnOffMasterSkill) 
	m_BtnMasterLevel.ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_MASTERLEVEL, false);
	m_BtnMasterLevel.ChangeButtonInfo(m_Pos.x + 124, m_Pos.y + 392, 36, 29);
	m_BtnMasterLevel.ChangeToolTipText(GlobalText[1749], true);
#endif
	m_Pos.y = m_Pos.y + FixYSetPage;
}

void SEASON3B::CNewUICharacterInfoWindow::Release()
{
	UnloadImages();

	if (m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}

}

void SEASON3B::CNewUICharacterInfoWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y + FixYSetPage;
}

bool SEASON3B::CNewUICharacterInfoWindow::UpdateMouseEvent()
{
	if (BtnProcess() == true)
	{
		return false;
	}

	if (CheckMouseIn(m_Pos.x, m_Pos.y, CHAINFO_WINDOW_WIDTH, CHAINFO_WINDOW_HEIGHT))
	{
		return false;
	}

	return true;
}

bool SEASON3B::CNewUICharacterInfoWindow::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

	if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHARACTER);
		return true;
	}

	
	int iBaseClass = gCharacterManager.GetBaseClass(Hero->Class);
	int iCount = 0;
	if (iBaseClass == CLASS_DARK_LORD)
	{
		iCount = 5;
	}
	else
	{
		iCount = 4;
	}

	if (CharacterAttribute->PrintPlayer.ViewPoint > 0)
	{
		for (int i = 0; i < iCount; ++i)
		{
			if (m_BtnStat[i].UpdateMouseEvent() == true)
			{
				g_SelectedStatType = i;
				CreateMessageBox(MSGBOX_LAYOUT_CLASS(CAddStatPointMsgBoxLayout));
				return true;
			}


		}
	}

	for (int x = 0; x < iCount; ++x)
	{
		if (m_BtnPoint[x].UpdateMouseEvent() == true)
		{
			const char* pStatCmd[] = { "str", "agi", "vit", "ene", "cmd" };
			if (x >= 0 && x < _countof(pStatCmd))
			{
				char szCmd[32];
				sprintf(szCmd, "/taydiem %s", pStatCmd[x]);
				SendChat(szCmd);
			}
			return true;
		}
	}

	if (m_BtnExit.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHARACTER);
		return true;
	}

	if (m_BtnQuest.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MYQUEST);
		return true;
	}

	if (m_BtnPet.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_PET);
		return true;
	}
#if (OnOffMasterSkill) 
	if (m_BtnMasterLevel.UpdateMouseEvent() == true)
	{
		if (gCharacterManager.IsMasterLevel(Hero->Class) == true
			//===Bat/Tat Master RF
#ifdef PBG_ADD_NEWCHAR_MONK
			|| gCharacterManager.GetCharacterClass(Hero->Class) == CLASS_TEMPLENIGHT
#endif //PBG_ADD_NEWCHAR_MONK
			)
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MASTER_LEVEL);
		return true;
	}
#endif
	return false;
}

bool SEASON3B::CNewUICharacterInfoWindow::UpdateKeyEvent()
{
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER) == true)
	{
		if (SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHARACTER);
			PlayBuffer(SOUND_CLICK01);

			return false;
		}
	}

	return true;
}

bool SEASON3B::CNewUICharacterInfoWindow::Update()
{
	if (IsVisible())
	{
		int iNumCurOpenTab = m_TabBtn.UpdateMouseEvent();

		if (iNumCurOpenTab == RADIOGROUPEVENT_NONE)
			return true;

		m_iCurrentOpenTab = iNumCurOpenTab;
	}
	return true;
}

void SEASON3B::CNewUICharacterInfoWindow::RenderFrame()
{
	RenderImage(IMAGE_CHAINFO_BACK, m_Pos.x, m_Pos.y - FixYSetPage, 190.f, 429.f);
	RenderImage(IMAGE_CHAINFO_TOP, m_Pos.x, m_Pos.y - FixYSetPage, 190.f, 64.f);
	RenderImage(IMAGE_CHAINFO_LEFT, m_Pos.x, (m_Pos.y - FixYSetPage) + 64, 21.f, 320.f);
	RenderImage(IMAGE_CHAINFO_RIGHT, m_Pos.x + 190 - 21, (m_Pos.y - FixYSetPage) + 64, 21.f, 320.f);
	RenderImage(IMAGE_CHAINFO_BOTTOM, m_Pos.x, (m_Pos.y - FixYSetPage) + 429 - 45, 190.f, 45.f);
	if (m_iCurrentOpenTab == 0)
	{
		glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
		RenderColor(m_Pos.x + 12, m_Pos.y + 48, 160, 66);
		EndRenderColor();
		RenderImage(IMAGE_CHAINFO_TABLE_TOP_LEFT, m_Pos.x + 12, m_Pos.y + 48, 14, 14);
		RenderImage(IMAGE_CHAINFO_TABLE_TOP_RIGHT, m_Pos.x + 12 + 165 - 14, m_Pos.y + 48, 14, 14);
		RenderImage(IMAGE_CHAINFO_TABLE_BOTTOM_LEFT, m_Pos.x + 12, m_Pos.y + 119 - 14, 14, 14);
		RenderImage(IMAGE_CHAINFO_TABLE_BOTTOM_RIGHT, m_Pos.x + 12 + 165 - 14, m_Pos.y + 119 - 14, 14, 14);

		const int tableTopLeft = m_Pos.x + 12 + 14;
		const int tableTopWidth = 165 - (14 * 2);
		if (tableTopWidth > 0)
		{
			RenderImage(IMAGE_CHAINFO_TABLE_TOP_PIXEL, tableTopLeft, m_Pos.y + 48, tableTopWidth, 14);
			RenderImage(IMAGE_CHAINFO_TABLE_BOTTOM_PIXEL, tableTopLeft, m_Pos.y + 119 - 14, tableTopWidth, 14);
		}

		const int tableMidLeft = m_Pos.x + 14;
		const int tableMidWidth = 12 + 165 - 18;
		if (tableMidWidth > 0)
		{
			RenderImage(IMAGE_CHAINFO_TABLE_BOTTOM_PIXEL, tableMidLeft, m_Pos.y + 48 + 12, tableMidWidth, 14);
		}

		const int tableSideTop = m_Pos.y + 48 + 14;
		const int tableSideHeight = 119 - 48 - (14 * 2);
		if (tableSideHeight > 0)
		{
			RenderImage(IMAGE_CHAINFO_TABLE_LEFT_PIXEL, m_Pos.x + 12, tableSideTop, 14, tableSideHeight);
			RenderImage(IMAGE_CHAINFO_TABLE_RIGHT_PIXEL, m_Pos.x + 12 + 165 - 14, tableSideTop, 14, tableSideHeight);
		}
		RenderImage(IMAGE_CHAINFO_TEXTBOX, m_Pos.x + 11, m_Pos.y + HEIGHT_STRENGTH, 170.f, 21.f);
		RenderImage(IMAGE_CHAINFO_TEXTBOX, m_Pos.x + 11, m_Pos.y + HEIGHT_DEXTERITY, 170.f, 21.f);
		RenderImage(IMAGE_CHAINFO_TEXTBOX, m_Pos.x + 11, m_Pos.y + HEIGHT_VITALITY, 170.f, 21.f);
		RenderImage(IMAGE_CHAINFO_TEXTBOX, m_Pos.x + 11, m_Pos.y + HEIGHT_ENERGY, 170.f, 21.f);
		if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD)
		{
			RenderImage(IMAGE_CHAINFO_TEXTBOX, m_Pos.x + 11, m_Pos.y + HEIGHT_CHARISMA, 170.f, 21.f);
		}
	}
	if (m_iCurrentOpenTab == 1)
	{
		if (gCB_StatsAdvance) gCB_StatsAdvance->RenderInfo(m_Pos.x, m_Pos.y);
	}
}

bool SEASON3B::CNewUICharacterInfoWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	RenderFrame();
	RenderTexts();
	RenderButtons();
	DisableAlphaBlend();
	return true;
}

void SEASON3B::CNewUICharacterInfoWindow::RenderTexts()
{
	RenderSubjectTexts();
	if (m_iCurrentOpenTab == 0)
	{
		RenderTableTexts();
		RenderAttribute();
	}
}

void SEASON3B::CNewUICharacterInfoWindow::RenderSubjectTexts()
{
	char strID[256];
	sprintf(strID, "%s", CharacterAttribute->Name);
	unicode::t_char strClassName[256];
	unicode::_sprintf(strClassName, "(%s)", gCharacterManager.GetCharacterClassText(CharacterAttribute->Class));

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(20, 20, 20, 20);
	SetPlayerColor(Hero->PK);
	g_pRenderText->RenderText(m_Pos.x, (m_Pos.y - FixYSetPage) + 12, strID, 190, 0, RT3_SORT_CENTER);

	unicode::t_char strServerName[MAX_TEXT_LENGTH];

	const char* apszGlobalText[4]
		= { GlobalText[461], GlobalText[460], GlobalText[3130], GlobalText[3131] };
	sprintf(strServerName, apszGlobalText[g_ServerListManager->GetNonPVPInfo()],
		g_ServerListManager->GetSelectServerName(), g_ServerListManager->GetSelectServerIndex());

	float fAlpha = sinf(WorldTime * 0.001f) + 1.f;
	g_pRenderText->SetTextColor(255, 255, 255, 127 * (2.f - fAlpha));
	g_pRenderText->RenderText(m_Pos.x, (m_Pos.y - FixYSetPage) + 27, strClassName, 190, 0, RT3_SORT_CENTER);
	g_pRenderText->SetTextColor(255, 255, 255, 127 * fAlpha);
	g_pRenderText->RenderText(m_Pos.x, (m_Pos.y - FixYSetPage) + 27, strServerName, 190, 0, RT3_SORT_CENTER);
}

void SEASON3B::CNewUICharacterInfoWindow::RenderTableTexts()
{
	unicode::t_char strLevel[128];
	unicode::t_char strExtra[32];
	unicode::t_char strExp[128];
	unicode::t_char strPoint[128];

	//if(gCharacterManager.IsMasterLevel(CharacterAttribute->Class) == true)
	//{
	//	unicode::_sprintf(strLevel, GlobalText[1745]);
	//	unicode::_sprintf(strExp, "----------");
	//}
	//else
	{
		//unicode::_sprintf(strLevel, GlobalText[200], CharacterAttribute->Level);
		//unicode::_sprintf(strExtra, " / RS %d ", CharacterAttribute->PrintPlayer.ViewReset);
		unicode::_sprintf(strLevel, gCustomMessage.GetMessageA(0), CharacterAttribute->Level, CharacterAttribute->PrintPlayer.ViewReset, CharacterAttribute->PrintPlayer.ViewMasterReset);
		//strcat(strLevel, strExtra);

		unicode::_sprintf(strExp, GlobalText[201], CharacterAttribute->Experience, CharacterAttribute->NextExperince);
	}
	if (CharacterAttribute->Level > 9)
	{
		int iMinus, iMaxMinus;
		if (CharacterAttribute->wMinusPoint == 0)
		{
			iMinus = 0;
		}
		else
		{
			iMinus = -CharacterAttribute->wMinusPoint;
		}

		iMaxMinus = -CharacterAttribute->wMaxMinusPoint;

		unicode::_sprintf(strPoint, "%s %d/%d | %s %d/%d",
			GlobalText[1412], CharacterAttribute->AddPoint, CharacterAttribute->MaxAddPoint,
			GlobalText[1903], iMinus, iMaxMinus);
	}
	else
	{
		wsprintf(strPoint, "%s %d/%d | %s %d/%d", GlobalText[1412], 0, 0, GlobalText[1903], 0, 0);
	}

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(230, 230, 0, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->RenderText(m_Pos.x + 18, m_Pos.y + 58, strLevel);

	if (CharacterAttribute->PrintPlayer.ViewPoint > 0)
	{
		unicode::t_char strLevelUpPoint[128];

		if (gCharacterManager.IsMasterLevel(CharacterAttribute->Class) == false || CharacterAttribute->PrintPlayer.ViewPoint > 0)
		{
			unicode::_sprintf(strLevelUpPoint, GlobalText[217], CharacterAttribute->PrintPlayer.ViewPoint);
		}
		else
			unicode::_sprintf(strLevelUpPoint, "");
		g_pRenderText->SetFont(g_hFontBold);
		g_pRenderText->SetTextColor(255, 138, 0, 255);
		g_pRenderText->SetBgColor(0, 0, 0, 0);
		g_pRenderText->RenderText(m_Pos.x + 18, m_Pos.y + 88, strLevelUpPoint);  //chinh cot point
		//g_pRenderText->RenderText(m_Pos.x + 115, m_Pos.y + 58, strLevelUpPoint);  //chinh cot point
	}

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->RenderText(m_Pos.x + 18, m_Pos.y + 75, strExp);

	int iAddPoint, iMinusPoint;

	if (CharacterAttribute->AddPoint <= 10)
	{
		iAddPoint = 100;
	}
	else
	{
		int iTemp = 0;
		if (CharacterAttribute->MaxAddPoint != 0)
		{
			iTemp = (CharacterAttribute->AddPoint * 100) / CharacterAttribute->MaxAddPoint;
		}
		if (iTemp <= 10)
		{
			iAddPoint = 70;
		}
		else if (iTemp > 10 && iTemp <= 30)
		{
			iAddPoint = 60;
		}
		else if (iTemp > 30 && iTemp <= 50)
		{
			iAddPoint = 50;
		}
		else if (iTemp > 50)
		{
			iAddPoint = 40;
		}
	}

	if (CharacterAttribute->wMinusPoint <= 10)
	{
		iMinusPoint = 100;
	}
	else
	{
		int iTemp = 0;
		if (CharacterAttribute->wMaxMinusPoint != 0)
		{
			iTemp = (CharacterAttribute->wMinusPoint * 100) / CharacterAttribute->wMaxMinusPoint;
		}

		if (iTemp <= 10)
		{
			iMinusPoint = 70;
		}
		else if (iTemp > 10 && iTemp <= 30)
		{
			iMinusPoint = 60;
		}
		else if (iTemp > 30 && iTemp <= 50)
		{
			iMinusPoint = 50;
		}
		else if (iTemp > 50)
		{
			iMinusPoint = 40;
		}
	}


	unicode::t_char strPointProbability[128];
	unicode::_sprintf(strPointProbability, GlobalText[1907], iAddPoint, iMinusPoint);
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(76, 197, 254, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->RenderText(m_Pos.x + 18, m_Pos.y + 88, strPointProbability);

	g_pRenderText->SetTextColor(76, 197, 254, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->RenderText(m_Pos.x + 18, m_Pos.y + 101, strPoint);
}

void SEASON3B::CNewUICharacterInfoWindow::RenderAttribute()
{
	g_pRenderText->SetFont(g_hFontBold);

	DWORD wStrength;

	wStrength = CharacterAttribute->PrintPlayer.ViewStrength + CharacterAttribute->PrintPlayer.ViewAddStrength;

	if (g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion1))
	{
		g_pRenderText->SetTextColor(255, 120, 0, 255);
	}
	else
		if (CharacterAttribute->PrintPlayer.ViewAddStrength)
		{
			g_pRenderText->SetTextColor(100, 150, 255, 255);
		}
		else
		{
			g_pRenderText->SetTextColor(230, 230, 0, 255);
		}

	unicode::t_char strStrength[32];
	unicode::_sprintf(strStrength, "%d", wStrength);

	g_pRenderText->SetBgColor(0);
	g_pRenderText->RenderText(m_Pos.x + 12, m_Pos.y + HEIGHT_STRENGTH + 6, GlobalText[166], 74, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(m_Pos.x + 86, m_Pos.y + HEIGHT_STRENGTH + 6, strStrength, 86, 0, RT3_SORT_CENTER);

	unicode::t_char strAttakMamage[256];
	int iAttackDamageMin = 0;
	int iAttackDamageMax = 0;

	bool bAttackDamage = GetAttackDamage(&iAttackDamageMin, &iAttackDamageMax);

	int Add_Dex = 0;
	int Add_Rat = 0;
	int Add_Dfe = 0;
	int Add_Att_Max = 0;
	int Add_Att_Min = 0;
	int Add_Dfe_Rat = 0;
	int Add_Ch_Dfe = 0;
	int Add_Mana_Max = 0;
	int Add_Mana_Min = 0;

	//MasTer Skill S5
	/*
	for (int i = 0; i < MAX_MAGIC; ++i)
	{
		if (CharacterAttribute->Skill[i] >= AT_SKILL_TOMAN_ATTACKUP && CharacterAttribute->Skill[i] < AT_SKILL_TOMAN_ATTACKUP + 5)
		{
			Add_Dex = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
		}
		else
			if (CharacterAttribute->Skill[i] >= AT_SKILL_ATTACK_RATEUP && CharacterAttribute->Skill[i] < AT_SKILL_ATTACK_RATEUP + 5)
			{
				Add_Rat = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
			}
			else
				if (CharacterAttribute->Skill[i] >= AT_SKILL_TOMAN_DEFENCEUP && CharacterAttribute->Skill[i] < AT_SKILL_TOMAN_DEFENCEUP + 5)
				{
					Add_Dfe = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
				}
				else
					if (CharacterAttribute->Skill[i] >= AT_SKILL_DEF_UP && CharacterAttribute->Skill[i] < AT_SKILL_DEF_UP + 5)
					{
						Add_Ch_Dfe = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
					}
		//		else
		//		if(CharacterAttribute->Skill[i] >= AT_SKILL_MAX_HP_UP && CharacterAttribute->Skill[i] < AT_SKILL_MAX_HP_UP + 5)
		//		{
		//			Add_Life = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
		//		}
		//		else
		//		if(CharacterAttribute->Skill[i] >= AT_SKILL_MAX_AG_UP && CharacterAttribute->Skill[i] < AT_SKILL_MAX_AG_UP + 5)
		//		{
		//			Add_Ag = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
		//		}
					else
						if (CharacterAttribute->Skill[i] >= AT_SKILL_MAX_ATTACKRATE_UP && CharacterAttribute->Skill[i] < AT_SKILL_MAX_ATTACKRATE_UP + 5)
						{
							Add_Att_Max = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
						}
						else
							if (CharacterAttribute->Skill[i] >= AT_SKILL_MAX_ATT_MAGIC_UP && CharacterAttribute->Skill[i] < AT_SKILL_MAX_ATT_MAGIC_UP + 5)
							{
								Add_Mana_Max = Add_Att_Max = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
							}
							else
								if (CharacterAttribute->Skill[i] >= AT_SKILL_MIN_ATT_MAGIC_UP && CharacterAttribute->Skill[i] < AT_SKILL_MIN_ATT_MAGIC_UP + 5)
								{
									Add_Mana_Min = Add_Att_Min = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
								}
								else
									if (CharacterAttribute->Skill[i] >= AT_SKILL_MAX_MANA_UP && CharacterAttribute->Skill[i] < AT_SKILL_MAX_MANA_UP + 5)
									{
										Add_Mana_Max = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
									}
									else
										if (CharacterAttribute->Skill[i] >= AT_SKILL_MIN_MANA_UP && CharacterAttribute->Skill[i] < AT_SKILL_MIN_MANA_UP + 5)
										{
											Add_Mana_Min = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
										}
										else
											if (CharacterAttribute->Skill[i] >= AT_SKILL_MIN_ATTACKRATE_UP && CharacterAttribute->Skill[i] < AT_SKILL_MIN_ATTACKRATE_UP + 5)
											{
												Add_Att_Min = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
											}
	}
	*/

	ITEM* pWeaponRight = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
	ITEM* pWeaponLeft = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];

	int iAttackRating = CharacterAttribute->AttackRating + Add_Rat;
	int iAttackRatingPK = CharacterAttribute->AttackRatingPK + Add_Dex;
	iAttackDamageMax += Add_Att_Max;
	iAttackDamageMin += Add_Att_Min;

	if (g_isCharacterBuff((&Hero->Object), eBuff_AddAG))
	{
		DWORD wDexterity = CharacterAttribute->PrintPlayer.ViewDexterity + CharacterAttribute->PrintPlayer.ViewAddDexterity;
		iAttackRating += wDexterity;
		iAttackRatingPK += wDexterity;
		if (PartyNumber >= 3)
		{
			int iPlusRating = (wDexterity * ((PartyNumber - 2) * 0.01f));
			iAttackRating += iPlusRating;
			iAttackRatingPK = iPlusRating;
		}
	}
	if (g_isCharacterBuff((&Hero->Object), eBuff_HelpNpc))
	{
		int iTemp = 0;
		if (CharacterAttribute->Level > 180)
		{
			iTemp = (180 / 3) + 45;
		}
		else
		{
			iTemp = (CharacterAttribute->Level / 3) + 45;
		}

		iAttackDamageMin += iTemp;
		iAttackDamageMax += iTemp;
	}

	if (g_isCharacterBuff((&Hero->Object), eBuff_Berserker))
	{
		int nTemp = CharacterAttribute->PrintPlayer.ViewStrength + CharacterAttribute->PrintPlayer.ViewAddStrength
			+ CharacterAttribute->PrintPlayer.ViewDexterity + CharacterAttribute->PrintPlayer.ViewAddDexterity;
		float fTemp = int(CharacterAttribute->PrintPlayer.ViewEnergy / 30) / 100.f;
		iAttackDamageMin += nTemp / 7 * fTemp;
		iAttackDamageMax += nTemp / 4 * fTemp;
	}

	int iMinIndex = 0, iMaxIndex = 0, iMagicalIndex = 0;

	StrengthenCapability SC_r, SC_l;

	int rlevel = (pWeaponRight->Level >> 3) & 15;

	if (rlevel >= pWeaponRight->Jewel_Of_Harmony_OptionLevel)
	{
		g_pUIJewelHarmonyinfo->GetStrengthenCapability(&SC_r, pWeaponRight, 1);

		if (SC_r.SI_isSP)
		{
			iMinIndex = SC_r.SI_SP.SI_minattackpower;
			iMaxIndex = SC_r.SI_SP.SI_maxattackpower;
			iMagicalIndex = SC_r.SI_SP.SI_magicalpower;
		}
	}

	int llevel = (pWeaponLeft->Level >> 3) & 15;

	if (llevel >= pWeaponLeft->Jewel_Of_Harmony_OptionLevel)
	{
		g_pUIJewelHarmonyinfo->GetStrengthenCapability(&SC_l, pWeaponLeft, 1);

		if (SC_l.SI_isSP)
		{
			iMinIndex += SC_l.SI_SP.SI_minattackpower;
			iMaxIndex += SC_l.SI_SP.SI_maxattackpower;
			iMagicalIndex += SC_l.SI_SP.SI_magicalpower;
		}
	}

	int iDefenseRate = 0, iAttackPowerRate = 0;

	StrengthenCapability rightinfo, leftinfo;

	int iRightLevel = (pWeaponRight->Level >> 3) & 15;

	if (iRightLevel >= pWeaponRight->Jewel_Of_Harmony_OptionLevel)
	{
		g_pUIJewelHarmonyinfo->GetStrengthenCapability(&rightinfo, pWeaponRight, 1);
	}

	int iLeftLevel = (pWeaponLeft->Level >> 3) & 15;

	if (iLeftLevel >= pWeaponLeft->Jewel_Of_Harmony_OptionLevel)
	{
		g_pUIJewelHarmonyinfo->GetStrengthenCapability(&leftinfo, pWeaponLeft, 1);
	}

	if (rightinfo.SI_isSP)
	{
		iAttackPowerRate += rightinfo.SI_SP.SI_attackpowerRate;
	}
	if (leftinfo.SI_isSP)
	{
		iAttackPowerRate += leftinfo.SI_SP.SI_attackpowerRate;
	}

	for (int k = EQUIPMENT_WEAPON_LEFT; k < MAX_EQUIPMENT; ++k)
	{
		StrengthenCapability defenseinfo;

		ITEM* pItem = &CharacterMachine->Equipment[k];

		int eqlevel = (pItem->Level >> 3) & 15;

		if (eqlevel >= pItem->Jewel_Of_Harmony_OptionLevel)
		{
			g_pUIJewelHarmonyinfo->GetStrengthenCapability(&defenseinfo, pItem, 2);
		}

		if (defenseinfo.SI_isSD)
		{
			iDefenseRate += defenseinfo.SI_SD.SI_defenseRate;
		}
	}

	int itemoption380Attack = 0;
	int itemoption380Defense = 0;

	for (int j = 0; j < MAX_EQUIPMENT; ++j)
	{
		bool is380item = CharacterMachine->Equipment[j].option_380;
		int i380type = CharacterMachine->Equipment[j].Type;

		if (is380item && i380type > -1 && i380type < MAX_ITEM)
		{
			ITEM_ADD_OPTION item380option;

			item380option = g_pItemAddOptioninfo->GetItemAddOtioninfo(CharacterMachine->Equipment[j].Type);

			if (item380option.m_byOption1 == 1)
			{
				itemoption380Attack += item380option.m_byValue1;
			}

			if (item380option.m_byOption2 == 1)
			{
				itemoption380Attack += item380option.m_byValue2;
			}

			if (item380option.m_byOption1 == 3)
			{
				itemoption380Defense += item380option.m_byValue1;
			}

			if (item380option.m_byOption2 == 3)
			{
				itemoption380Defense += item380option.m_byValue2;
			}
		}
	}

	ITEM* pItemRingLeft, * pItemRingRight;

	pItemRingLeft = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
	pItemRingRight = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
	if (pItemRingLeft && pItemRingRight)
	{
		int iNonExpiredLRingType = -1;
		int iNonExpiredRRingType = -1;
		if (!pItemRingLeft->bPeriodItem || !pItemRingLeft->bExpiredPeriod)
		{
			iNonExpiredLRingType = pItemRingLeft->Type;
		}
		if (!pItemRingRight->bPeriodItem || !pItemRingRight->bExpiredPeriod)
		{
			iNonExpiredRRingType = pItemRingRight->Type;
		}

		int maxIAttackDamageMin = 0;
		int maxIAttackDamageMax = 0;
		if (iNonExpiredLRingType == ITEM_HELPER + 41 || iNonExpiredRRingType == ITEM_HELPER + 41)
		{
			maxIAttackDamageMin = max(maxIAttackDamageMin, 20);
			maxIAttackDamageMax = max(maxIAttackDamageMax, 20);
		}
		if (iNonExpiredLRingType == ITEM_HELPER + 76 || iNonExpiredRRingType == ITEM_HELPER + 76)
		{
			maxIAttackDamageMin = max(maxIAttackDamageMin, 30);
			maxIAttackDamageMax = max(maxIAttackDamageMax, 30);
		}
		if (iNonExpiredLRingType == ITEM_HELPER + 122 || iNonExpiredRRingType == ITEM_HELPER + 122)
		{
			maxIAttackDamageMin = max(maxIAttackDamageMin, 40);
			maxIAttackDamageMax = max(maxIAttackDamageMax, 40);
		}

		iAttackDamageMin += maxIAttackDamageMin;
		iAttackDamageMax += maxIAttackDamageMax;
	}

	ITEM* pItemHelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
	if (pItemHelper)
	{
		if (pItemHelper->Type == ITEM_HELPER + 37 && pItemHelper->Option1 == 0x04)
		{
			WORD wLevel = CharacterAttribute->Level;
			iAttackDamageMin += (wLevel / 12);
			iAttackDamageMax += (wLevel / 12);
		}
		if (pItemHelper->Type == ITEM_HELPER + 64)
		{
			if (false == pItemHelper->bExpiredPeriod)
			{
				iAttackDamageMin += int(float(iAttackDamageMin) * 0.4f);
				iAttackDamageMax += int(float(iAttackDamageMax) * 0.4f);
			}
		}
		if (pItemHelper->Type == ITEM_HELPER + 123)
		{
			if (false == pItemHelper->bExpiredPeriod)
			{
				iAttackDamageMin += int(float(iAttackDamageMin) * 0.2f);
				iAttackDamageMax += int(float(iAttackDamageMax) * 0.2f);
			}
		}
		if (pItemHelper->Type == ITEM_HELPER + 1)
		{
			iAttackDamageMin += int(float(iAttackDamageMin) * 0.3f);
			iAttackDamageMax += int(float(iAttackDamageMax) * 0.3f);
		}
	}

	if (iAttackRating > 0)
	{
		//if (iAttackDamageMin + iMinIndex >= iAttackDamageMax + iMaxIndex)
		//{
		//	unicode::_sprintf(strAttakMamage, GlobalText[203], iAttackDamageMax + iMaxIndex, iAttackDamageMax + iMaxIndex, iAttackRating);
		//}
		//else
		//{
		//	unicode::_sprintf(strAttakMamage, GlobalText[203], iAttackDamageMin + iMinIndex, iAttackDamageMax + iMaxIndex, iAttackRating);
		//}
		DWORD cDamageMin = 0;
		DWORD cDamageMax = 0;
		GetViewPhisyDamage(&cDamageMin, &cDamageMax);
		iAttackDamageMin = cDamageMin;
		iAttackDamageMax = cDamageMax;

		iAttackRating = CharacterAttribute->PrintPlayer.ViewAttackSuccessRate;

		unicode::_sprintf(strAttakMamage, GlobalText[203], iAttackDamageMin, iAttackDamageMax, iAttackRating);
	}
	else
	{
		//if (iAttackDamageMin + iMinIndex >= iAttackDamageMax + iMaxIndex)
		//{
		//	unicode::_sprintf(strAttakMamage, GlobalText[204], iAttackDamageMax + iMaxIndex, iAttackDamageMax + iMaxIndex);
		//}
		//else
		//{
		//	unicode::_sprintf(strAttakMamage, GlobalText[204], iAttackDamageMin + iMinIndex, iAttackDamageMax + iMaxIndex);
		//}
		DWORD cDamageMin = 0;
		DWORD cDamageMax = 0;
		GetViewPhisyDamage(&cDamageMin, &cDamageMax);
		iAttackDamageMin = cDamageMin;
		iAttackDamageMax = cDamageMax;
		unicode::_sprintf(strAttakMamage, GlobalText[204], iAttackDamageMin, iAttackDamageMax);
	}

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

	if (bAttackDamage)
	{
		g_pRenderText->SetTextColor(100, 150, 255, 255);
	}
	else
	{
		g_pRenderText->SetTextColor(255, 255, 255, 255);
	}

	if (g_isCharacterBuff((&Hero->Object), eBuff_Hellowin2))
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

	if (g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll3))
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

	if (g_isCharacterBuff((&Hero->Object), eBuff_CherryBlossom_Petal))
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

	int iY = HEIGHT_STRENGTH + 25;
	g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + HEIGHT_STRENGTH + 25, strAttakMamage);

	if (iAttackRatingPK > 0)
	{
		if (itemoption380Attack != 0 || iAttackPowerRate != 0)
		{
			unicode::_sprintf(strAttakMamage, GlobalText[2109], CharacterAttribute->PrintPlayer.ViewAttackSuccessRatePvP);
		}
		else
		{
			unicode::_sprintf(strAttakMamage, GlobalText[2044], CharacterAttribute->PrintPlayer.ViewAttackSuccessRatePvP);
		}

		iY += 13;
		g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strAttakMamage);
	}

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0);

	if (g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion2))
	{
		g_pRenderText->SetTextColor(255, 120, 0, 255);
	}
	else
		if (CharacterAttribute->AddDexterity)
		{
			g_pRenderText->SetTextColor(100, 150, 255, 255);
		}
		else
		{
			g_pRenderText->SetTextColor(230, 230, 0, 255);
		}

	unicode::t_char strDexterity[32];
	DWORD wDexterity = CharacterAttribute->PrintPlayer.ViewDexterity + CharacterAttribute->AddDexterity;
	unicode::_sprintf(strDexterity, "%d", wDexterity);
	g_pRenderText->RenderText(m_Pos.x + 12, m_Pos.y + HEIGHT_DEXTERITY + 6, GlobalText[1702], 74, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(m_Pos.x + 86, m_Pos.y + HEIGHT_DEXTERITY + 6, strDexterity, 86, 0, RT3_SORT_CENTER);

	bool bDexSuccess = true;
	int iBaseClass = gCharacterManager.GetBaseClass(Hero->Class);

	for (int i = EQUIPMENT_HELM; i <= EQUIPMENT_BOOTS; ++i)
	{
		if (iBaseClass == CLASS_DARK)
		{
			if ((CharacterMachine->Equipment[i].Type == -1 && (i != EQUIPMENT_HELM && iBaseClass == CLASS_DARK))
				|| (CharacterMachine->Equipment[i].Type != -1 && CharacterMachine->Equipment[i].Durability <= 0))
			{
				bDexSuccess = false;
				break;
			}
		}
#ifdef PBG_ADD_NEWCHAR_MONK
		else if (iBaseClass == CLASS_RAGEFIGHTER)
		{
			if ((CharacterMachine->Equipment[i].Type == -1 && (i != EQUIPMENT_GLOVES && iBaseClass == CLASS_RAGEFIGHTER))
				|| (CharacterMachine->Equipment[i].Type != -1 && CharacterMachine->Equipment[i].Durability <= 0))
			{
				bDexSuccess = false;
				break;
			}
		}
#endif //PBG_ADD_NEWCHAR_MONK
		else
		{
			if ((CharacterMachine->Equipment[i].Type == -1) ||
				(CharacterMachine->Equipment[i].Type != -1 && CharacterMachine->Equipment[i].Durability <= 0))
			{
				bDexSuccess = false;
				break;
			}
		}
	}

	if (bDexSuccess)
	{
		int iType;
		if (iBaseClass == CLASS_DARK)
		{

			iType = CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type;

			if (
				(iType != ITEM_ARMOR + 15)
				&& (iType != ITEM_ARMOR + 20)
				&& (iType != ITEM_ARMOR + 23)
				&& (iType != ITEM_ARMOR + 32)
				&& (iType != ITEM_ARMOR + 37)
				&& (iType != ITEM_ARMOR + 47)
				&& (iType != ITEM_ARMOR + 48)
				)
			{
				bDexSuccess = false;
			}

			iType = iType % MAX_ITEM_INDEX;
		}
		else
		{
			iType = CharacterMachine->Equipment[EQUIPMENT_HELM].Type % MAX_ITEM_INDEX;
		}

		if (bDexSuccess)
		{
			for (int i = EQUIPMENT_ARMOR; i <= EQUIPMENT_BOOTS; ++i)
			{
#ifdef PBG_ADD_NEWCHAR_MONK
				if (iBaseClass == CLASS_RAGEFIGHTER && i == EQUIPMENT_GLOVES)
					continue;
#endif //PBG_ADD_NEWCHAR_MONK
				if (iType != CharacterMachine->Equipment[i].Type % MAX_ITEM_INDEX)
				{
					bDexSuccess = false;
					break;
				}
			}
		}
	}

	int t_adjdef = CharacterAttribute->Defense + Add_Ch_Dfe;

	if (g_isCharacterBuff((&Hero->Object), eBuff_HelpNpc))
	{
		if (CharacterAttribute->Level > 180)
		{
			t_adjdef += 180 / 5 + 50;
		}
		else
		{
			t_adjdef += (CharacterAttribute->Level / 5 + 50);
		}
	}

	if (g_isCharacterBuff((&Hero->Object), eBuff_Berserker))
	{
		int nTemp = CharacterAttribute->PrintPlayer.ViewDexterity + CharacterAttribute->PrintPlayer.ViewAddDexterity;
		float fTemp = (40 - int(CharacterAttribute->PrintPlayer.ViewEnergy / 60)) / 100.f;
		fTemp = MAX(fTemp, 0.1f);
		t_adjdef -= nTemp / 3 * fTemp;
	}

	int maxdefense = 0;

	for (int j = 0; j < MAX_EQUIPMENT; ++j)
	{
		int TempLevel = (CharacterMachine->Equipment[j].Level >> 3) & 15;
		if (TempLevel >= CharacterMachine->Equipment[j].Jewel_Of_Harmony_OptionLevel)
		{
			StrengthenCapability SC;

			g_pUIJewelHarmonyinfo->GetStrengthenCapability(&SC, &CharacterMachine->Equipment[j], 2);

			if (SC.SI_isSD)
			{
				maxdefense += SC.SI_SD.SI_defense;
			}
		}
	}

	int iChangeRingAddDefense = 0;

	pItemRingLeft = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
	pItemRingRight = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
	if (pItemRingLeft->Type == ITEM_HELPER + 39 || pItemRingRight->Type == ITEM_HELPER + 39)
	{
		iChangeRingAddDefense = (t_adjdef + maxdefense) / 10;
	}

	if (Hero->Helper.Type == MODEL_HELPER + 80)
		iChangeRingAddDefense += 50;

	if (Hero->Helper.Type == MODEL_HELPER + 106)
		iChangeRingAddDefense += 50;

	unicode::t_char strBlocking[256];

#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	int nAdd_FulBlocking = 0;
	if (
		g_isCharacterBuff((&Hero->Object), eBuff_Def_up_Ourforces)
		|| g_isCharacterBuff((&Hero->Object), EFFECT_GREATER_DEFENSE_SUCCESS_RATE_IMPROVED)
		|| g_isCharacterBuff((&Hero->Object), EFFECT_GREATER_DEFENSE_SUCCESS_RATE_ENHANCED)

		)
	{
#ifdef PBG_MOD_RAGEFIGHTERSOUND
		int _AddStat = (10 + (float)(CharacterAttribute->PrintPlayer.ViewEnergy - 80) / 10);
#else //PBG_MOD_RAGEFIGHTERSOUND
		int _AddStat = (10 + (float)(CharacterAttribute->PrintPlayer.ViewEnergy - 160) / 50);
#endif //PBG_MOD_RAGEFIGHTERSOUND

		if (_AddStat > 100)
			_AddStat = 100;

		_AddStat = CharacterAttribute->SuccessfulBlocking * _AddStat / 100;
		nAdd_FulBlocking += _AddStat;
	}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	if (bDexSuccess)
	{
		// memorylock
		if (CharacterAttribute->SuccessfulBlocking > 0)
		{
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			//if (nAdd_FulBlocking)
			//{
			//	unicode::_sprintf(strBlocking, GlobalText[206], t_adjdef + maxdefense + iChangeRingAddDefense,
			//		CharacterAttribute->SuccessfulBlocking, (CharacterAttribute->SuccessfulBlocking) / 10 + nAdd_FulBlocking);
			//}
			//else
			//{
			//	unicode::_sprintf(strBlocking, GlobalText[206], t_adjdef + maxdefense + iChangeRingAddDefense,
			//		CharacterAttribute->SuccessfulBlocking, (CharacterAttribute->SuccessfulBlocking) / 10);
			//}
			unicode::_sprintf(strBlocking, GlobalText[206], 208,
				CharacterAttribute->PrintPlayer.ViewDefense, CharacterAttribute->PrintPlayer.ViewDefenseSuccessRate);
#else //PBG_ADD_NEWCHAR_MONK_SKILL
			unicode::_sprintf(strBlocking, GlobalText[206], t_adjdef + maxdefense + iChangeRingAddDefense, CharacterAttribute->SuccessfulBlocking, (CharacterAttribute->SuccessfulBlocking) / 10);
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
		}
		else
		{
			unicode::_sprintf(strBlocking, GlobalText[207], 209, CharacterAttribute->PrintPlayer.ViewDefense);
		}
	}
	else
	{
		if (CharacterAttribute->SuccessfulBlocking > 0)
		{
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			if (nAdd_FulBlocking)
			{
				//unicode::_sprintf(strBlocking, GlobalText[206], t_adjdef + maxdefense + iChangeRingAddDefense, CharacterAttribute->SuccessfulBlocking, nAdd_FulBlocking);
				unicode::_sprintf(strBlocking, GlobalText[206], 208,
					CharacterAttribute->PrintPlayer.ViewDefense, CharacterAttribute->PrintPlayer.ViewDefenseSuccessRate);
			}
			else
			{
				unicode::_sprintf(strBlocking, GlobalText[208], CharacterAttribute->PrintPlayer.ViewDefense, CharacterAttribute->PrintPlayer.ViewDefenseSuccessRate);
			}
#else //PBG_ADD_NEWCHAR_MONK_SKILL
			// 208 "방어력(율): %d (%d)"
			unicode::_sprintf(strBlocking, GlobalText[208],
				t_adjdef + maxdefense + iChangeRingAddDefense,
				CharacterAttribute->SuccessfulBlocking
			);
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
		}
		else
		{
			// 209
			unicode::_sprintf(strBlocking, GlobalText[209], CharacterAttribute->PrintPlayer.ViewDefense);
		}
	}

	iY = HEIGHT_DEXTERITY + 24;
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);

	if (g_isCharacterBuff((&Hero->Object), eBuff_Hellowin3))
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}
	if (g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll2))
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	if (g_isCharacterBuff((&Hero->Object), eBuff_Def_up_Ourforces)
		|| g_isCharacterBuff((&Hero->Object), EFFECT_GREATER_DEFENSE_SUCCESS_RATE_IMPROVED)
		|| g_isCharacterBuff((&Hero->Object), EFFECT_GREATER_DEFENSE_SUCCESS_RATE_ENHANCED))
	{
		g_pRenderText->SetTextColor(100, 150, 255, 255);
	}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	g_pRenderText->SetBgColor(0);
	g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strBlocking);

	WORD wAttackSpeed = CLASS_WIZARD == iBaseClass || CLASS_SUMMONER == iBaseClass
		? CharacterAttribute->MagicSpeed : CharacterAttribute->AttackSpeed;

	unicode::_sprintf(strBlocking, GlobalText[64], CharacterAttribute->PrintPlayer.ViewPhysiSpeed);
	iY += 13;

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);

	if (g_isCharacterBuff((&Hero->Object), eBuff_Hellowin1))
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

	if (g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll1))
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

	ITEM* phelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
	if (phelper->Durability != 0 &&
		(phelper->Type == ITEM_HELPER + 64 || phelper->Type == ITEM_HELPER + 123))
	{
		if (IsRequireEquipItem(phelper))
		{
			if (false == pItemHelper->bExpiredPeriod)
			{
				g_pRenderText->SetTextColor(255, 0, 240, 255);
			}
		}
	}
	g_pRenderText->SetBgColor(0);
	g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strBlocking);

	if (itemoption380Defense != 0 || iDefenseRate != 0)
	{
		unicode::_sprintf(strBlocking, GlobalText[2110], CharacterAttribute->PrintPlayer.ViewDefenseSuccessRatePvP);
	}
	else
	{
		unicode::_sprintf(strBlocking, GlobalText[2045], CharacterAttribute->PrintPlayer.ViewDefenseSuccessRatePvP);
	}

	iY += 13;
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strBlocking);


	g_pRenderText->SetFont(g_hFontBold);

	DWORD wVitality = CharacterAttribute->PrintPlayer.ViewVitality + CharacterAttribute->PrintPlayer.ViewAddVitality;

	if (g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion3))
	{
		g_pRenderText->SetTextColor(255, 120, 0, 255);
	}
	else
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		if (
			g_isCharacterBuff((&Hero->Object), eBuff_Hp_up_Ourforces)
			|| g_isCharacterBuff((&Hero->Object), EFFECT_FITNESS_IMPROVED)
			)
		{
#ifdef PBG_MOD_RAGEFIGHTERSOUND
			CharacterMachine->CalculateAll();
			wVitality = CharacterAttribute->PrintPlayer.ViewVitality + CharacterAttribute->PrintPlayer.ViewAddVitality;
#else //PBG_MOD_RAGEFIGHTERSOUND
			WORD _AddStat = (WORD)(30 + (WORD)((CharacterAttribute->PrintPlayer.ViewEnergy - 380) / 10));
			if (_AddStat > 200)
			{
				_AddStat = 200;
			}
			wVitality = CharacterAttribute->PrintPlayer.ViewVitality + _AddStat;
#endif //PBG_MOD_RAGEFIGHTERSOUND
			g_pRenderText->SetTextColor(100, 150, 255, 255);
		}
		else
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
			if (CharacterAttribute->PrintPlayer.ViewAddVitality)
			{
				g_pRenderText->SetTextColor(100, 150, 255, 255);
			}
			else
			{
				g_pRenderText->SetTextColor(230, 230, 0, 255);
			}

	unicode::t_char strVitality[256];
	unicode::_sprintf(strVitality, "%d", wVitality);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->RenderText(m_Pos.x + 12, m_Pos.y + HEIGHT_VITALITY + 6, GlobalText[169], 74, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(m_Pos.x + 86, m_Pos.y + HEIGHT_VITALITY + 6, strVitality, 86, 0, RT3_SORT_CENTER);

	g_pRenderText->SetFont(g_hFont);


	//if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
	//{
	//	unicode::_sprintf(strVitality, GlobalText[211], CharacterAttribute->PrintPlayer.ViewCurHP, Master_Level_Data.wMaxLife);
	//}
	//else
	{
		unicode::_sprintf(strVitality, GlobalText[211], CharacterAttribute->PrintPlayer.ViewCurHP, CharacterAttribute->PrintPlayer.ViewMaxHP);
	}

	g_pRenderText->SetTextColor(255, 255, 255, 255);

	if (g_isCharacterBuff((&Hero->Object), eBuff_Hellowin4))
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

	if (g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll5))
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

	if (g_isCharacterBuff((&Hero->Object), eBuff_CherryBlossom_RiceCake))
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

	if (phelper->Durability != 0 && phelper->Type == ITEM_HELPER + 65)
	{
		if (IsRequireEquipItem(phelper))
		{
			if (false == pItemHelper->bExpiredPeriod)
			{
				g_pRenderText->SetTextColor(255, 0, 240, 255);
			}
		}
	}
	g_pRenderText->SetBgColor(0);
	iY = HEIGHT_VITALITY + 24;
	g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strVitality);

#ifdef PBG_ADD_NEWCHAR_MONK
	if (iBaseClass == CLASS_RAGEFIGHTER)
	{
		iY += 13;
		//물리공격력
		unicode::_sprintf(strVitality, GlobalText[3155], CharacterAttribute->PrintPlayer.ViewRFDamageMultiplierA);
		g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strVitality);
	}
#endif //PBG_ADD_NEWCHAR_MONK

	g_pRenderText->SetFont(g_hFontBold);

	DWORD wEnergy = CharacterAttribute->PrintPlayer.ViewEnergy + CharacterAttribute->PrintPlayer.ViewAddEnergy;

	if (g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion4))
	{
		g_pRenderText->SetTextColor(255, 120, 0, 255);
	}
	else
		if (CharacterAttribute->PrintPlayer.ViewAddEnergy)
		{
			g_pRenderText->SetTextColor(100, 150, 255, 255);
		}
		else
		{
			g_pRenderText->SetTextColor(230, 230, 0, 255);
		}

	unicode::t_char strEnergy[256];
	unicode::_sprintf(strEnergy, "%d", wEnergy);

	g_pRenderText->SetBgColor(0);
	g_pRenderText->RenderText(m_Pos.x + 12, m_Pos.y + HEIGHT_ENERGY + 6, GlobalText[168], 74, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(m_Pos.x + 86, m_Pos.y + HEIGHT_ENERGY + 6, strEnergy, 86, 0, RT3_SORT_CENTER);

	g_pRenderText->SetFont(g_hFont);

	//if(gCharacterManager.IsMasterLevel( Hero->Class ) == true )
	if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
	{
		unicode::_sprintf(strEnergy, GlobalText[213], CharacterAttribute->PrintPlayer.ViewCurMP, CharacterAttribute->PrintPlayer.ViewMaxMP);
	}
	else
	{
		unicode::_sprintf(strEnergy, GlobalText[213], CharacterAttribute->PrintPlayer.ViewCurMP, CharacterAttribute->PrintPlayer.ViewMaxMP);
	}

	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(255, 255, 255, 255);

	if (g_isCharacterBuff((&Hero->Object), eBuff_Hellowin5))
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

	if (g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll6))
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

	if (g_isCharacterBuff((&Hero->Object), eBuff_CherryBlossom_Liguor))
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

	iY = HEIGHT_ENERGY + 24;

	g_pRenderText->RenderText(m_Pos.x + 18, m_Pos.y + iY, strEnergy);

	if (iBaseClass == CLASS_WIZARD || iBaseClass == CLASS_DARK || iBaseClass == CLASS_SUMMONER)
	{
		int Level = (pWeaponRight->Level >> 3) & 15;
		int iMagicDamageMin;
		int iMagicDamageMax;

		gCharacterManager.GetMagicSkillDamage(CharacterAttribute->Skill[Hero->CurrentSkill], &iMagicDamageMin, &iMagicDamageMax);

		int iMagicDamageMinInitial = iMagicDamageMin;
		int iMagicDamageMaxInitial = iMagicDamageMax;

		iMagicDamageMin += Add_Mana_Min;
		iMagicDamageMax += Add_Mana_Max;

		if (CharacterAttribute->Ability & ABILITY_PLUS_DAMAGE)
		{
			iMagicDamageMin += 10;
			iMagicDamageMax += 10;
		}

		int maxMg = 0;

		for (int j = 0; j < MAX_EQUIPMENT; ++j)
		{
			int TempLevel = (CharacterMachine->Equipment[j].Level >> 3) & 15;

			if (TempLevel >= CharacterMachine->Equipment[j].Jewel_Of_Harmony_OptionLevel)
			{
				StrengthenCapability SC;
				g_pUIJewelHarmonyinfo->GetStrengthenCapability(&SC, &CharacterMachine->Equipment[j], 1);

				if (SC.SI_isSP)
				{
					maxMg += SC.SI_SP.SI_magicalpower;
				}
			}
		}

		pItemRingLeft = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
		pItemRingRight = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];


		int iNonExpiredLRingType = -1;
		int iNonExpiredRRingType = -1;

		if (!pItemRingLeft->bPeriodItem || !pItemRingLeft->bExpiredPeriod)
		{
			iNonExpiredLRingType = pItemRingLeft->Type;
		}
		if (!pItemRingRight->bPeriodItem || !pItemRingRight->bExpiredPeriod)
		{
			iNonExpiredRRingType = pItemRingRight->Type;
		}

		int maxIMagicDamageMin = 0;
		int maxIMagicDamageMax = 0;

		if (iNonExpiredLRingType == ITEM_HELPER + 41 || iNonExpiredRRingType == ITEM_HELPER + 41)
		{
			maxIMagicDamageMin = max(maxIMagicDamageMin, 20);
			maxIMagicDamageMax = max(maxIMagicDamageMax, 20);
		}
		if (iNonExpiredLRingType == ITEM_HELPER + 76 || iNonExpiredRRingType == ITEM_HELPER + 76)
		{
			maxIMagicDamageMin = max(maxIMagicDamageMin, 30);
			maxIMagicDamageMax = max(maxIMagicDamageMax, 30);
		}
		if (iNonExpiredLRingType == ITEM_HELPER + 122 || iNonExpiredRRingType == ITEM_HELPER + 122)
		{
			maxIMagicDamageMin = max(maxIMagicDamageMin, 40);
			maxIMagicDamageMax = max(maxIMagicDamageMax, 40);
		}

		iMagicDamageMin += maxIMagicDamageMin;
		iMagicDamageMax += maxIMagicDamageMax;

		pItemHelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
		if (pItemHelper)
		{
			if (pItemHelper->Type == ITEM_HELPER + 37 && pItemHelper->Option1 == 0x04)
			{
				WORD wLevel = CharacterAttribute->Level;
				iMagicDamageMin += (wLevel / 25);
				iMagicDamageMax += (wLevel / 25);
			}

			if (pItemHelper->Type == ITEM_HELPER + 64)
			{
				if (false == pItemHelper->bExpiredPeriod)
				{
					iMagicDamageMin += int(float(iMagicDamageMin) * 0.4f);
					iMagicDamageMax += int(float(iMagicDamageMax) * 0.4f);
				}

			}
			if (pItemHelper->Type == ITEM_HELPER + 123)
			{
				if (false == pItemHelper->bExpiredPeriod)
				{
					iMagicDamageMin += int(float(iMagicDamageMin) * 0.2f);
					iMagicDamageMax += int(float(iMagicDamageMax) * 0.2f);
				}
			}
			if (pItemHelper->Type == ITEM_HELPER + 1)
			{
				iMagicDamageMin += int(float(iMagicDamageMin) * 0.3f);
				iMagicDamageMax += int(float(iMagicDamageMax) * 0.3f);
			}
		}

		if (g_isCharacterBuff((&Hero->Object), eBuff_Berserker))
		{
			int nTemp = CharacterAttribute->PrintPlayer.ViewEnergy + CharacterAttribute->PrintPlayer.ViewAddEnergy;
			float fTemp = int(CharacterAttribute->PrintPlayer.ViewEnergy / 30) / 100.f;
			iMagicDamageMin += nTemp / 9 * fTemp;
			iMagicDamageMax += nTemp / 4 * fTemp;
		}

		if ((pWeaponRight->Type >= MODEL_STAFF - MODEL_ITEM
			&& pWeaponRight->Type < (MODEL_STAFF + MAX_ITEM_INDEX - MODEL_ITEM))
			|| pWeaponRight->Type == (MODEL_SWORD + 31 - MODEL_ITEM)
			|| pWeaponRight->Type == (MODEL_SWORD + 23 - MODEL_ITEM)
			|| pWeaponRight->Type == (MODEL_SWORD + 25 - MODEL_ITEM)
			|| pWeaponRight->Type == (MODEL_SWORD + 21 - MODEL_ITEM)
			|| pWeaponRight->Type == (MODEL_SWORD + 28 - MODEL_ITEM)
			)
		{
			//float magicPercent = (float)(pWeaponRight->MagicPower) / 100;

			//ITEM_ATTRIBUTE* p = &ItemAttribute[pWeaponRight->Type];
			//float   percent = CalcDurabilityPercent(pWeaponRight->Durability, p->MagicDur, pWeaponRight->Level, pWeaponRight->Option1, pWeaponRight->ExtOption);

			//magicPercent = magicPercent - magicPercent * percent;
			//unicode::_sprintf(strEnergy, GlobalText[215], iMagicDamageMin + maxMg, iMagicDamageMax + maxMg, (int)((iMagicDamageMaxInitial + maxMg) * magicPercent));
			DWORD DamageMin = 0;
			DWORD DamageMax = 0;
			DWORD DamageRate = 0;
			GetViewMagicDamage(&DamageMin, &DamageMax, &DamageRate);
			unicode::_sprintf(strEnergy, GlobalText[215], DamageMin, DamageMax, DamageRate);

		}
		else
		{
			DWORD DamageMin = 0;
			DWORD DamageMax = 0;
			GetViewMagicDamage(&DamageMin, &DamageMax);
			unicode::_sprintf(strEnergy, GlobalText[216], DamageMin, DamageMax);
		}

		iY += 13;
		g_pRenderText->SetBgColor(0);
		g_pRenderText->SetTextColor(255, 255, 255, 255);

		if (g_isCharacterBuff((&Hero->Object), eBuff_Hellowin2))
		{
			g_pRenderText->SetTextColor(255, 0, 240, 255);
		}

		if (g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll4))
		{
			g_pRenderText->SetTextColor(255, 0, 240, 255);
		}

		if (g_isCharacterBuff((&Hero->Object), eBuff_CherryBlossom_Petal))
		{
			g_pRenderText->SetTextColor(255, 0, 240, 255);
		}
		g_pRenderText->RenderText(m_Pos.x + 18, m_Pos.y + iY, strEnergy);
	}

	if (iBaseClass == CLASS_SUMMONER)
	{
		int iCurseDamageMin = 0;
		int iCurseDamageMax = 0;

		gCharacterManager.GetCurseSkillDamage(CharacterAttribute->Skill[Hero->CurrentSkill], &iCurseDamageMin, &iCurseDamageMax);

		if (g_isCharacterBuff((&Hero->Object), eBuff_Berserker))
		{
			int nTemp = CharacterAttribute->PrintPlayer.ViewEnergy + CharacterAttribute->PrintPlayer.ViewAddEnergy;
			float fTemp = int(CharacterAttribute->PrintPlayer.ViewEnergy / 30) / 100.f;
			iCurseDamageMin += nTemp / 9 * fTemp;
			iCurseDamageMax += nTemp / 4 * fTemp;
		}

		int iNonExpiredLRingType = -1;
		int iNonExpiredRRingType = -1;

		if (!pItemRingLeft->bPeriodItem || !pItemRingLeft->bExpiredPeriod)
		{
			iNonExpiredLRingType = pItemRingLeft->Type;
		}
		if (!pItemRingRight->bPeriodItem || !pItemRingRight->bExpiredPeriod)
		{
			iNonExpiredRRingType = pItemRingRight->Type;
		}

		int maxICurseDamageMin = 0;
		int maxICurseDamageMax = 0;

		if (iNonExpiredLRingType == ITEM_HELPER + 76 || iNonExpiredRRingType == ITEM_HELPER + 76)
		{
			maxICurseDamageMin = max(maxICurseDamageMin, 30);
			maxICurseDamageMax = max(maxICurseDamageMax, 30);
		}
		if (iNonExpiredLRingType == ITEM_HELPER + 122 || iNonExpiredRRingType == ITEM_HELPER + 122)
		{
			maxICurseDamageMin = max(maxICurseDamageMin, 40);
			maxICurseDamageMax = max(maxICurseDamageMax, 40);
		}

		iCurseDamageMin += maxICurseDamageMin;
		iCurseDamageMax += maxICurseDamageMax;

		pItemHelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
		if (pItemHelper)
		{
			if (pItemHelper->Type == ITEM_HELPER + 64)
			{
				if (false == pItemHelper->bExpiredPeriod)
				{
					iCurseDamageMin += int(float(iCurseDamageMin) * 0.4f);
					iCurseDamageMax += int(float(iCurseDamageMax) * 0.4f);
				}
			}
			if (pItemHelper->Type == ITEM_HELPER + 123)
			{
				if (false == pItemHelper->bExpiredPeriod)
				{
					iCurseDamageMin += int(float(iCurseDamageMin) * 0.2f);
					iCurseDamageMax += int(float(iCurseDamageMax) * 0.2f);
				}
			}
		}

		if (ITEM_STAFF + 21 <= pWeaponLeft->Type && pWeaponLeft->Type <= ITEM_STAFF + 29)
		{
			//float fCursePercent = (float)(pWeaponLeft->MagicPower) / 100;

			//ITEM_ATTRIBUTE* p = &ItemAttribute[pWeaponLeft->Type];
			//float fPercent = ::CalcDurabilityPercent(pWeaponLeft->Durability,
			//	p->MagicDur, pWeaponLeft->Level, pWeaponLeft->Option1,
			//	pWeaponLeft->ExtOption);

			//fCursePercent -= fCursePercent * fPercent;

			//unicode::_sprintf(strEnergy, GlobalText[1693],
			//	iCurseDamageMin, iCurseDamageMax,
			//	(int)((iCurseDamageMax)* fCursePercent));

			DWORD DmgRate = 0;
			DWORD cDamageMin = 0;
			DWORD cDamageMax = 0;
			GetViewCurseDamage(&cDamageMin, &cDamageMax, &DmgRate);
			iCurseDamageMin = cDamageMin;
			iCurseDamageMax = cDamageMax;

			unicode::_sprintf(strEnergy, GlobalText[1693],
				iCurseDamageMin, iCurseDamageMax,
				(int)(DmgRate));

		}
		else
		{
			DWORD cDamageMin = 0;
			DWORD cDamageMax = 0;
			GetViewCurseDamage(&cDamageMin, &cDamageMax);
			iCurseDamageMin = cDamageMin;
			iCurseDamageMax = cDamageMax;
			unicode::_sprintf(strEnergy, GlobalText[1694],
				iCurseDamageMin, iCurseDamageMax);
		}

		iY += 13;
		g_pRenderText->RenderText(m_Pos.x + 18, m_Pos.y + iY, strEnergy);
	}

	iY += 13;
	if (iBaseClass == CLASS_KNIGHT)
	{
		unicode::_sprintf(strEnergy, GlobalText[582], CharacterAttribute->PrintPlayer.ViewDamageMultiplier);
		g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strEnergy);
	}
	if (iBaseClass == CLASS_DARK)
	{
		unicode::_sprintf(strEnergy, GlobalText[582], CharacterAttribute->PrintPlayer.ViewDamageMultiplier);
		g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strEnergy);
	}
	if (iBaseClass == CLASS_DARK_LORD)
	{
		unicode::_sprintf(strEnergy, GlobalText[582], CharacterAttribute->PrintPlayer.ViewDamageMultiplier);
		g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strEnergy);
	}
#ifdef PBG_ADD_NEWCHAR_MONK
	if (iBaseClass == CLASS_RAGEFIGHTER)
	{
		//마법공격력
		unicode::_sprintf(strEnergy, GlobalText[3156], CharacterAttribute->PrintPlayer.ViewRFDamageMultiplierB);
		g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strEnergy);
		iY += 13;
		//범위공격력
		unicode::_sprintf(strEnergy, GlobalText[3157], CharacterAttribute->PrintPlayer.ViewRFDamageMultiplierC);
		g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strEnergy);
	}
#endif //PBG_ADD_NEWCHAR_MONK

	if (iBaseClass == CLASS_DARK_LORD)
	{
		g_pRenderText->SetFont(g_hFontBold);
		g_pRenderText->SetBgColor(0);

		DWORD wCharisma;

		wCharisma = CharacterAttribute->PrintPlayer.ViewLeadership + CharacterAttribute->PrintPlayer.ViewAddLeadership;

		if (g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion5))
		{
			g_pRenderText->SetTextColor(255, 120, 0, 255);
		}
		else

			if (CharacterAttribute->PrintPlayer.ViewAddLeadership)
			{
				g_pRenderText->SetTextColor(100, 150, 255, 255);
			}
			else
			{
				g_pRenderText->SetTextColor(230, 230, 0, 255);
			}

		unicode::t_char strCharisma[256];
		unicode::_sprintf(strCharisma, "%d", wCharisma);
		g_pRenderText->RenderText(m_Pos.x + 12, m_Pos.y + HEIGHT_CHARISMA + 6, GlobalText[1900], 74, 0, RT3_SORT_CENTER);
		g_pRenderText->RenderText(m_Pos.x + 86, m_Pos.y + HEIGHT_CHARISMA + 6, strCharisma, 86, 0, RT3_SORT_CENTER);
	}
}

void SEASON3B::CNewUICharacterInfoWindow::RenderButtons()
{
	int iBaseClass = gCharacterManager.GetBaseClass(Hero->Class);
	int iCount = 0;
	if (iBaseClass == CLASS_DARK_LORD)
	{
		iCount = 5;
	}
	else
	{
		iCount = 4;
	}
	if (m_iCurrentOpenTab == 0)
	{
		if (CharacterAttribute->PrintPlayer.ViewPoint > 0)
		{
			for (int i = 0; i < iCount; ++i)
			{
				m_BtnStat[i].Render();
			}
		}

		for (int i = 0; i < iCount; ++i)
		{
			m_BtnPoint[i].Render();
		}
	}
	m_BtnExit.Render();
	m_BtnQuest.Render();
	m_BtnPet.Render();
#if (OnOffMasterSkill) 
	m_BtnMasterLevel.Render();
#endif

	m_TabBtn.Render();
}

float SEASON3B::CNewUICharacterInfoWindow::GetLayerDepth()
{
	return 5.1f;
}

void SEASON3B::CNewUICharacterInfoWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_CHAINFO_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back04.tga", IMAGE_CHAINFO_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_CHAINFO_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_CHAINFO_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_CHAINFO_BOTTOM, GL_LINEAR);

	LoadBitmap("Interface\\newui_item_table01(L).tga", IMAGE_CHAINFO_TABLE_TOP_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_table01(R).tga", IMAGE_CHAINFO_TABLE_TOP_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_table02(L).tga", IMAGE_CHAINFO_TABLE_BOTTOM_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_table02(R).tga", IMAGE_CHAINFO_TABLE_BOTTOM_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_table03(Up).tga", IMAGE_CHAINFO_TABLE_TOP_PIXEL, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_table03(Dw).tga", IMAGE_CHAINFO_TABLE_BOTTOM_PIXEL, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_table03(L).tga", IMAGE_CHAINFO_TABLE_LEFT_PIXEL, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_table03(R).tga", IMAGE_CHAINFO_TABLE_RIGHT_PIXEL, GL_LINEAR);

	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_CHAINFO_BTN_EXIT, GL_LINEAR);

	LoadBitmap("Interface\\newui_cha_textbox02.tga", IMAGE_CHAINFO_TEXTBOX, GL_LINEAR);
	LoadBitmap("Interface\\newui_chainfo_btn_level.tga", IMAGE_CHAINFO_BTN_STAT, GL_LINEAR);
	LoadBitmap("Interface\\newui_chainfo_btn_quest.tga", IMAGE_CHAINFO_BTN_QUEST, GL_LINEAR);
	LoadBitmap("Interface\\newui_chainfo_btn_pet.tga", IMAGE_CHAINFO_BTN_PET, GL_LINEAR);
	LoadBitmap("Interface\\newui_chainfo_btn_master.tga", IMAGE_CHAINFO_BTN_MASTERLEVEL, GL_LINEAR);

	LoadBitmap("Interface\\MacroUI\\MacroUI_RangeMinus.tga", IMAGE_CHAINFO_BTN_STAT_TRU, GL_LINEAR);
}

void SEASON3B::CNewUICharacterInfoWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_CHAINFO_BTN_MASTERLEVEL);
	DeleteBitmap(IMAGE_CHAINFO_BTN_PET);
	DeleteBitmap(IMAGE_CHAINFO_BTN_QUEST);
	DeleteBitmap(IMAGE_CHAINFO_BTN_STAT);
	DeleteBitmap(IMAGE_CHAINFO_TEXTBOX);

	DeleteBitmap(IMAGE_CHAINFO_BTN_EXIT);

	DeleteBitmap(IMAGE_CHAINFO_BOTTOM);
	DeleteBitmap(IMAGE_CHAINFO_RIGHT);
	DeleteBitmap(IMAGE_CHAINFO_LEFT);
	DeleteBitmap(IMAGE_CHAINFO_TOP);
	DeleteBitmap(IMAGE_CHAINFO_BACK);

	DeleteBitmap(IMAGE_CHAINFO_TABLE_RIGHT_PIXEL);
	DeleteBitmap(IMAGE_CHAINFO_TABLE_LEFT_PIXEL);
	DeleteBitmap(IMAGE_CHAINFO_TABLE_BOTTOM_PIXEL);
	DeleteBitmap(IMAGE_CHAINFO_TABLE_TOP_PIXEL);
	DeleteBitmap(IMAGE_CHAINFO_TABLE_BOTTOM_RIGHT);
	DeleteBitmap(IMAGE_CHAINFO_TABLE_BOTTOM_LEFT);
	DeleteBitmap(IMAGE_CHAINFO_TABLE_TOP_RIGHT);
	DeleteBitmap(IMAGE_CHAINFO_TABLE_TOP_LEFT);
	DeleteBitmap(IMAGE_CHAINFO_BTN_STAT_TRU);
}

void SEASON3B::CNewUICharacterInfoWindow::OpenningProcess()
{
	ResetEquipmentLevel();
#if (OnOffMasterSkill) 
	if (gCharacterManager.IsMasterLevel(Hero->Class) == true
#ifdef PBG_ADD_NEWCHAR_MONK
		|| gCharacterManager.GetCharacterClass(Hero->Class) == CLASS_TEMPLENIGHT
#endif //PBG_ADD_NEWCHAR_MONK
		)
	{
		m_BtnMasterLevel.UnLock();
		m_BtnMasterLevel.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
		m_BtnMasterLevel.ChangeTextColor(RGBA(255, 255, 255, 255));
	}
	else
	{
		m_BtnMasterLevel.Lock();
		m_BtnMasterLevel.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
		m_BtnMasterLevel.ChangeTextColor(RGBA(100, 100, 100, 255));
	}
#endif
	g_csItemOption.init();

	if (CharacterMachine->IsZeroDurability())
	{
		CharacterMachine->CalculateAll();
	}

	if (g_QuestMng.IsIndexInCurQuestIndexList(0x10009))
	{
		if (g_QuestMng.IsEPRequestRewardState(0x10009))
		{
			g_pMyQuestInfoWindow->UnselectQuestList();
			SendSatisfyQuestRequestFromClient(0x10009);
			g_QuestMng.SetEPRequestRewardState(0x10009, false);
		}
	}
}

void SEASON3B::CNewUICharacterInfoWindow::ResetEquipmentLevel()
{
	ITEM* pItem = CharacterMachine->Equipment;
	Hero->Weapon[0].Level = (pItem[EQUIPMENT_WEAPON_RIGHT].Level >> 3) & 15;
	Hero->Weapon[1].Level = (pItem[EQUIPMENT_WEAPON_LEFT].Level >> 3) & 15;
	Hero->BodyPart[BODYPART_HELM].Level = (pItem[EQUIPMENT_HELM].Level >> 3) & 15;
	Hero->BodyPart[BODYPART_ARMOR].Level = (pItem[EQUIPMENT_ARMOR].Level >> 3) & 15;
	Hero->BodyPart[BODYPART_PANTS].Level = (pItem[EQUIPMENT_PANTS].Level >> 3) & 15;
	Hero->BodyPart[BODYPART_GLOVES].Level = (pItem[EQUIPMENT_GLOVES].Level >> 3) & 15;
	Hero->BodyPart[BODYPART_BOOTS].Level = (pItem[EQUIPMENT_BOOTS].Level >> 3) & 15;

	CheckFullSet(Hero);
}

