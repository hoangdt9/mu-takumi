//////////////////////////////////////////////////////////////////////
// NewUIMainFrameWindow.cpp: implementation of the CNewUIMainFrameWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIMainFrameWindow.h"	// self
#include "Utilities/Log/TakumiAndroidUiPerf.h"
#include "Platform/TakumiAndroidHud.h"
#include "Platform/TakumiAndroidInput.h"
#include "NewUIOptionWindow.h"
#include "NewUISystem.h"
#include "UIBaseDef.h"
#include "DSPlaySound.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "wsclientinline.h"
#include "WSclient.h"
#include "CSItemOption.h"
#include "CSChaosCastle.h"
#include "MapManager.h"
#include "CharacterManager.h"
#include "SkillManager.h"
#include "GMDoppelGanger1.h"
#include "GMDoppelGanger2.h"
#include "GMDoppelGanger3.h"
#include "GMDoppelGanger4.h"
#include "./Time/CTimCheck.h"
#if defined(__ANDROID__) || defined(MU_IOS)
#include "Platform/MobileHud.h"
#endif
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
#include "MonkSystem.h"
#endif //PBG_ADD_NEWCHAR_MONK_SKILL

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
#include "GameShop/InGameShopSystem.h"
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#include "CBInterface.h"
#include "CustomEventTime.h"
#include "CustomRanking.h"
#include "ZzzOpenData.h"

#if defined(__ANDROID__)
#include "Utilities/Log/ErrorReport.h"

/// Log once per hovered picker cell (slot or pet cmd) to verify tooltip vs icon data path.
static void TakumiLogSkillPickerHoverCell(int renderInfoType)
{
	static int s_lastHoverType = -9999;
	if (renderInfoType == s_lastHoverType)
	{
		return;
	}
	s_lastHoverType = renderInfoType;

	if (CharacterAttribute == NULL)
	{
		return;
	}

	if (renderInfoType >= AT_PET_COMMAND_DEFAULT && renderInfoType < AT_PET_COMMAND_END)
	{
		g_ErrorReport.Write("[SkillPicker] hover petCmd=%d\r\n", renderInfoType);
		return;
	}

	if (renderInfoType < 0 || renderInfoType >= MAX_MAGIC)
	{
		g_ErrorReport.Write("[SkillPicker] hover badIndex=%d\r\n", renderInfoType);
		return;
	}

	const WORD skillType = CharacterAttribute->Skill[renderInfoType];
	const BYTE useType = SkillAttribute[skillType].SkillUseType;
	const int magicIcon = SkillAttribute[skillType].Magic_Icon;

	char nameBuf[160] = {};
	int mana = 0;
	int dist = 0;
	int skillMana = 0;
	if (skillType > 0)
	{
		gSkillManager.GetSkillInformation(skillType, 1, nameBuf, &mana, &dist, &skillMana);
	}

	g_ErrorReport.Write(
		"[SkillPicker] hover magicSlot=%d wireType=%u useType=%u Magic_Icon=%d GetSkillInfo=%s\r\n",
		renderInfoType,
		(unsigned)skillType,
		(unsigned)useType,
		magicIcon,
		nameBuf);
}
#endif // __ANDROID__

extern float g_fScreenRate_x;

static void TakumiResolveMainFrameVitals(DWORD& lifeMax, DWORD& life, DWORD& manaMax, DWORD& mana)
{
	DWORD curAg = 0;
	DWORD maxAg = 0;
	TakumiGetHudVitals(life, lifeMax, mana, manaMax, curAg, maxAg);
	(void)curAg;
	(void)maxAg;
}
extern float g_fScreenRate_y;
extern int  MouseUpdateTime;
extern int  MouseUpdateTimeMax;
extern int SelectedCharacter;
extern int Attacking;
extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayHeight;
extern int DisplayWinMid;
extern int DisplayHeightExt;
extern int DisplayWinExt;
extern int DisplayWinReal;

static void GetLegacyCurrentSkillSlotRect(float& x, float& y, float& width, float& height)
{
	float fixX = 0.0f;
	if (gProtect.m_MainInfo.IsVersion == 1)
	{
		fixX = 75.0f;
	}

	x = 385.f - fixX + DisplayWinExt;
	y = 431.f + DisplayHeightExt;
	width = 32.f;
	height = 38.f;
}

static void GetLegacySkillPickerCellRect(int order, float& x, float& y, float& width, float& height)
{
	float fixX = 0.0f;
	if (gProtect.m_MainInfo.IsVersion == 1)
	{
		fixX = 75.0f;
	}

	const float fOrigX = 385.f - fixX + DisplayWinExt;
	width = 32.f;
	height = 38.f;
	y = 390.f + DisplayHeightExt;

	if (order >= 18)
	{
		y -= height;
	}

	if (order < 14)
	{
		const int iRemainder = order % 2;
		const int iQuotient = order / 2;

		if (iRemainder == 0)
		{
			x = fOrigX + iQuotient * width;
		}
		else
		{
			x = fOrigX - (iQuotient + 1) * width;
		}
	}
	else if (order < 18)
	{
		x = fOrigX - (8 * width) - ((order - 14) * width);
	}
	else
	{
		x = fOrigX - (12 * width) + ((order - 17) * width);
	}
}

/// Keeps picker render, cached layout, mouse hit-testing, and tooltips on the same slot order.
static bool LegacyPickerIncludeMagicArrayIndex(int magicArrayIndex)
{
	if (CharacterAttribute == NULL)
	{
		return false;
	}

	const WORD iSkillType = CharacterAttribute->Skill[magicArrayIndex];

	if (iSkillType >= AT_PET_COMMAND_DEFAULT && iSkillType < AT_PET_COMMAND_END)
	{
		return false;
	}

	if (iSkillType == 0 || (iSkillType >= AT_SKILL_STUN && iSkillType <= AT_SKILL_REMOVAL_BUFF))
	{
		return false;
	}

	const BYTE bySkillUseType = SkillAttribute[iSkillType].SkillUseType;

	if (bySkillUseType == SKILL_USE_TYPE_MASTER || bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
	{
		return false;
	}

	return true;
}

static bool HitLegacyPickerPetCommandRow(float uiX, float uiY, OUT int* outCommand)
{
	if (outCommand == NULL || Hero == NULL || Hero->m_pPet == NULL)
	{
		return false;
	}

	float fixX = 0.0f;
	if (gProtect.m_MainInfo.IsVersion == 1)
	{
		fixX = 75.f;
	}

	float x = 353.f - fixX + DisplayWinExt;
	const float y = 352.f + DisplayHeightExt;
	const float width = 32.f;
	const float height = 38.f;

	for (int i = AT_PET_COMMAND_DEFAULT; i < AT_PET_COMMAND_END; ++i)
	{
		if (uiX >= x && uiX <= (x + width) && uiY >= y && uiY <= (y + height))
		{
			*outCommand = i;
			return true;
		}
		x += width;
	}

	return false;
}

static bool GetLegacyPickerPetCommandCellRect(int commandIndex, OUT float& x, OUT float& y, OUT float& w, OUT float& h)
{
	if (commandIndex < AT_PET_COMMAND_DEFAULT || commandIndex >= AT_PET_COMMAND_END)
	{
		return false;
	}

	float fixX = 0.0f;
	if (gProtect.m_MainInfo.IsVersion == 1)
	{
		fixX = 75.f;
	}

	x = 353.f - fixX + DisplayWinExt;
	y = 352.f + DisplayHeightExt;
	w = 32.f;
	h = 38.f;
	x += static_cast<float>(commandIndex - AT_PET_COMMAND_DEFAULT) * w;

	return true;
}

static bool IsRenderableSkillSlotIndex(int skillIndex)
{
	if (Hero == NULL || CharacterAttribute == NULL)
	{
		return false;
	}

	if (skillIndex >= AT_PET_COMMAND_DEFAULT && skillIndex < AT_PET_COMMAND_END)
	{
		return Hero->m_pPet != NULL;
	}

	if (skillIndex < 0 || skillIndex >= MAX_MAGIC)
	{
		return false;
	}

	const WORD skillType = CharacterAttribute->Skill[skillIndex];
	if (skillType == 0)
	{
		return false;
	}

	if (skillType >= AT_SKILL_STUN && skillType <= AT_SKILL_REMOVAL_BUFF)
	{
		return false;
	}

	return true;
}

static int GetPrimarySkillSlotIndex()
{
	if (g_pMainFrame == NULL)
	{
		return -1;
	}

	return g_pMainFrame->GetSkillHotKey(0);
}

static bool ShouldDrawCurrentSkillIcon()
{
	const int primarySkillIndex = GetPrimarySkillSlotIndex();
	return IsRenderableSkillSlotIndex(primarySkillIndex);
}

static bool ShouldHighlightCurrentSkillSlot()
{
	if (ShouldDrawCurrentSkillIcon())
	{
		return true;
	}

	return false;
}

static WORD GetCurrentSkillTypeForPrior()
{
	if (Hero == NULL || CharacterAttribute == NULL)
	{
		return 0;
	}

	if (Hero->CurrentSkill >= AT_PET_COMMAND_DEFAULT && Hero->CurrentSkill < AT_PET_COMMAND_END)
	{
		return static_cast<WORD>(Hero->CurrentSkill);
	}

	if (Hero->CurrentSkill >= 0 && Hero->CurrentSkill < MAX_MAGIC)
	{
		return CharacterAttribute->Skill[Hero->CurrentSkill];
	}

	return 0;
}

static void ApplySelectedSkillIndex(int skillIndex)
{
	if (Hero == NULL || g_pSkillList == NULL)
	{
		return;
	}

	if (!IsRenderableSkillSlotIndex(skillIndex))
	{
		return;
	}

	g_pSkillList->SetHeroPriorSkill(GetCurrentSkillTypeForPrior());
	Hero->CurrentSkill = static_cast<BYTE>(skillIndex);
	g_pMainFrame->SetSkillHotKey(0, skillIndex);
}

#if defined(__ANDROID__) || defined(MU_IOS)
bool AndroidTriggerHotKeySkillTap(int hotKeySkillIndex);
#endif

SEASON3B::CNewUIMainFrameWindow::CNewUIMainFrameWindow()
{
	m_bExpEffect = false;
	m_dwExpEffectTime = 0;
	m_dwPreExp = 0;
	m_dwGetExp = 0;
	m_bButtonBlink = false;
	ButtonSS2.x = 0;
	ButtonSS2.y = 0;
	ButtonSS2W = 0;
	ButtonSS2H = 0;
}

SEASON3B::CNewUIMainFrameWindow::~CNewUIMainFrameWindow()
{
	Release();
}

void SEASON3B::CNewUIMainFrameWindow::LoadImages()
{
	if (gProtect.m_MainInfo.IsVersion == 1) //Skin Ss2
	{
		LoadBitmap("Custom\\InterfaceS2\\Menu_left.jpg", IMAGE_MENU_1, GL_LINEAR);
		//LoadBitmap("Custom\\Interface\\MenuS2_left.jpg", IMAGE_MENU_1, GL_LINEAR);
		LoadBitmap("Custom\\InterfaceS2\\Menu_middle.jpg", IMAGE_MENU_2, GL_LINEAR);
		LoadBitmap("Custom\\InterfaceS2\\Menu_right.jpg", IMAGE_MENU_3, GL_LINEAR);
		LoadBitmap("Custom\\InterfaceS2\\Menu_Blue.jpg", IMAGE_GAUGE_BLUE, GL_LINEAR);
		LoadBitmap("Custom\\InterfaceS2\\Menu_Green.jpg", IMAGE_GAUGE_GREEN, GL_LINEAR);
		LoadBitmap("Custom\\InterfaceS2\\Menu_Red.jpg", IMAGE_GAUGE_RED, GL_LINEAR);
		LoadBitmap("Custom\\InterfaceS2\\Menu_AG.jpg", IMAGE_GAUGE_AG, GL_LINEAR);
		LoadBitmap("Custom\\InterfaceS2\\Menu_SD.jpg", IMAGE_GAUGE_SD, GL_LINEAR);

		LoadBitmap("Custom\\InterfaceS2\\DragonRight.tga", IMAGE_eDragonLeft, GL_LINEAR);
		LoadBitmap("Custom\\InterfaceS2\\DragonLeft.tga", IMAGE_eDragonRight, GL_LINEAR);

		LoadBitmap("Custom\\InterfaceS2\\Boton\\Menu_Inventory.jpg", IMAGE_iNewInventory, GL_LINEAR);
		LoadBitmap("Custom\\InterfaceS2\\Boton\\Menu_Character.jpg", IMAGE_iNewCharacter, GL_LINEAR);
		LoadBitmap("Custom\\InterfaceS2\\Boton\\Menu_Party.jpg", IMAGE_iNewParty, GL_LINEAR);
		LoadBitmap("Custom\\InterfaceS2\\Boton\\Menu_friend.jpg", IMAGE_iNewWinpush, GL_LINEAR);
		LoadBitmap("Custom\\InterfaceS2\\Boton\\Menu_guild.jpg", IMAGE_iNewGuild, GL_LINEAR);
		LoadBitmap("Custom\\InterfaceS2\\Boton\\Menu_CashShop.jpg", IMAGE_iNewCashShop, GL_LINEAR);

		//LoadBitmap("Custom\\InterfaceS2\\MenuS2_Button.jpg", IMAGE_MENUS2_BUTTON, GL_LINEAR);
	}
	else
	{
		LoadBitmap("Interface\\newui_menu_blue.jpg", IMAGE_GAUGE_BLUE, GL_LINEAR);
		LoadBitmap("Interface\\newui_menu_green.jpg", IMAGE_GAUGE_GREEN, GL_LINEAR);
		LoadBitmap("Interface\\newui_menu_red.jpg", IMAGE_GAUGE_RED, GL_LINEAR);
		LoadBitmap("Interface\\newui_menu01.jpg", IMAGE_MENU_1, GL_LINEAR);
		LoadBitmap("Interface\\newui_menu02.jpg", IMAGE_MENU_2, GL_LINEAR);
		LoadBitmap("Interface\\partCharge1\\newui_menu03.jpg", IMAGE_MENU_3, GL_LINEAR);
		LoadBitmap("Interface\\newui_menu_ag.jpg", IMAGE_GAUGE_AG, GL_LINEAR);
		LoadBitmap("Interface\\newui_menu_sd.jpg", IMAGE_GAUGE_SD, GL_LINEAR);
	}

	LoadBitmap("Interface\\newui_menu02-03.jpg", IMAGE_MENU_2_1, GL_LINEAR);

	LoadBitmap("Interface\\newui_exbar.jpg", IMAGE_GAUGE_EXBAR, GL_LINEAR);
	LoadBitmap("Interface\\Exbar_Master.jpg", IMAGE_MASTER_GAUGE_BAR, GL_LINEAR);
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
	LoadBitmap("Interface\\partCharge1\\newui_menu_Bt05.jpg", IMAGE_MENU_BTN_CSHOP, GL_LINEAR, GL_CLAMP_TO_EDGE);
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
	LoadBitmap("Interface\\partCharge1\\newui_menu_Bt01.jpg", IMAGE_MENU_BTN_CHAINFO, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\partCharge1\\newui_menu_Bt02.jpg", IMAGE_MENU_BTN_MYINVEN, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\partCharge1\\newui_menu_Bt03.jpg", IMAGE_MENU_BTN_FRIEND, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\partCharge1\\newui_menu_Bt04.jpg", IMAGE_MENU_BTN_WINDOW, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\partCharge1\\Decor.tga", IMAGE_DECOR_WIDE, GL_LINEAR, GL_CLAMP_TO_EDGE);

}

void SEASON3B::CNewUIMainFrameWindow::UnloadImages()
{
	//DeleteBitmap(IMAGE_MENU_1);
	//DeleteBitmap(IMAGE_MENU_2);
	//DeleteBitmap(IMAGE_MENU_3);
	//DeleteBitmap(IMAGE_MENU_2_1);
	//DeleteBitmap(IMAGE_GAUGE_BLUE);
	//DeleteBitmap(IMAGE_GAUGE_GREEN);
	//DeleteBitmap(IMAGE_GAUGE_RED);
	//DeleteBitmap(IMAGE_GAUGE_AG);
	//DeleteBitmap(IMAGE_GAUGE_SD);
	//DeleteBitmap(IMAGE_GAUGE_EXBAR);
	//DeleteBitmap(IMAGE_MENU_BTN_CHAINFO);
	//DeleteBitmap(IMAGE_MENU_BTN_MYINVEN);
	//DeleteBitmap(IMAGE_MENU_BTN_FRIEND);
	//DeleteBitmap(IMAGE_MENU_BTN_WINDOW);
	//DeleteBitmap(IMAGE_DECOR_WIDE);
	//DeleteBitmap(IMAGE_MENUS2_BUTTON);
	for (int i = BITMAP_INTERFACE_NEW_MAINFRAME_BEGIN; i < BITMAP_INTERFACE_NEW_MAINFRAME_END; i++)
	{
		DeleteBitmap(i);
	}
}

bool SEASON3B::CNewUIMainFrameWindow::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng)
{
	if (NULL == pNewUIMng || NULL == pNewUI3DRenderMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MAINFRAME, this);

	m_pNewUI3DRenderMng = pNewUI3DRenderMng;
	m_pNewUI3DRenderMng->Add3DRenderObj(this, ITEMHOTKEYNUMBER_CAMERA_Z_ORDER);

	LoadImages();

	SetButtonInfo();

	Show(true);

	return true;
}

void SEASON3B::CNewUIMainFrameWindow::SetButtonInfo()
{
	int x_Next = 489 + DisplayWinExt;
	int y_Next = (DisplayHeight)-51;
	int x_Add = 30;
	int y_Add = 41;
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
	m_BtnCShop.ChangeTextBackColor(RGBA(255, 255, 255, 0));
	m_BtnCShop.ChangeButtonImgState(true, IMAGE_MENU_BTN_CSHOP, true);
	m_BtnCShop.ChangeButtonInfo(x_Next, y_Next, x_Add, y_Add);
	x_Next += x_Add;
	m_BtnCShop.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnCShop.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
	m_BtnCShop.ChangeToolTipText(GlobalText[2277], true);
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME

	m_BtnChaInfo.ChangeTextBackColor(RGBA(255, 255, 255, 0));
	m_BtnChaInfo.ChangeButtonImgState(true, IMAGE_MENU_BTN_CHAINFO, true);
	m_BtnChaInfo.ChangeButtonInfo(x_Next, y_Next, x_Add, y_Add);
	x_Next += x_Add;
	m_BtnChaInfo.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnChaInfo.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
	m_BtnChaInfo.ChangeToolTipText(GlobalText[362], true);

	m_BtnMyInven.ChangeTextBackColor(RGBA(255, 255, 255, 0));
	m_BtnMyInven.ChangeButtonImgState(true, IMAGE_MENU_BTN_MYINVEN, true);
	m_BtnMyInven.ChangeButtonInfo(x_Next, y_Next, x_Add, y_Add);
	x_Next += x_Add;
	m_BtnMyInven.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnMyInven.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
	m_BtnMyInven.ChangeToolTipText(GlobalText[363], true);

	m_BtnFriend.ChangeTextBackColor(RGBA(255, 255, 255, 0));
	m_BtnFriend.ChangeButtonImgState(true, IMAGE_MENU_BTN_FRIEND, true);
	m_BtnFriend.ChangeButtonInfo(x_Next, y_Next, x_Add, y_Add);
	x_Next += x_Add;
	m_BtnFriend.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnFriend.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
	m_BtnFriend.ChangeToolTipText(GlobalText[1043], true);

	m_BtnWindow.ChangeTextBackColor(RGBA(255, 255, 255, 0));
	m_BtnWindow.ChangeButtonImgState(true, IMAGE_MENU_BTN_WINDOW, true);
	m_BtnWindow.ChangeButtonInfo(x_Next, y_Next, x_Add, y_Add);
	m_BtnWindow.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnWindow.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
	m_BtnWindow.ChangeToolTipText(GlobalText[1744], true);


	if (gProtect.m_MainInfo.IsVersion == 1) //Skin Ss2
	{
		//== Set Pos Button SS2
		ButtonSS2.x = 87 + DisplayWinExt;
		ButtonSS2.y = DisplayHeight - 28;
		ButtonSS2W = 25;
		ButtonSS2H = 25;

		m_BtnFriend.ChangeButtonInfo(-1, -1, x_Add, y_Add);
		m_BtnWindow.ChangeButtonInfo(-1, -1, x_Add, y_Add);
		m_BtnMyInven.ChangeButtonInfo(-1, -1, x_Add, y_Add);
		m_BtnChaInfo.ChangeButtonInfo(-1, -1, x_Add, y_Add);
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
		m_BtnCShop.ChangeButtonInfo(-1, -1, x_Add, y_Add);
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
	}

}

void SEASON3B::CNewUIMainFrameWindow::Release()
{
	UnloadImages();

	if (m_pNewUI3DRenderMng)
	{
		m_pNewUI3DRenderMng->Remove3DRenderObj(this);
		m_pNewUI3DRenderMng = NULL;
	}

	if (m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

//=== Render Skin SS2

void SEASON3B::CNewUIMainFrameWindow::RenderLifeManaSS2()
{
	DWORD wLifeMax, wLife, wManaMax, wMana;

	TakumiResolveMainFrameVitals(wLifeMax, wLife, wManaMax, wMana);


	if (wLifeMax > 0)
	{
		if (wLife > 0 && (wLife / (float)wLifeMax) < 0.2f)
		{
			PlayBuffer(SOUND_HEART);
		}
	}

	float fLife = 0.f;
	float fMana = 0.f;

	if (wLifeMax > 0)
	{
		fLife = (wLifeMax - wLife) / (float)wLifeMax;
	}
	if (wManaMax > 0)
	{
		fMana = (wManaMax - wMana) / (float)wManaMax;
	}

	float width, height;
	float x, y;
	float fY, fH, fV;

	// life
	width = 52;
	height = 51;
	x = 98.0f + DisplayWinExt;
	y = DisplayHeight - 50.f;

	fY = y + (fLife * height);
	fH = height - (fLife * height);
	fV = fLife;
	if (g_isCharacterBuff((&Hero->Object), eDeBuff_Poison))
	{
		RenderBitmap(IMAGE_GAUGE_GREEN, x, fY, width, fH, 0.f, fV * height / 64.f, width / 64.f, (1.0f - fV) * height / 64.f);
	}
	else
	{
		RenderBitmap(IMAGE_GAUGE_RED, x, fY, width, fH, 0.f, fV * height / 64.f, width / 64.f, (1.0f - fV) * height / 64.f);
	}

	SEASON3B::RenderNumber(x + 25, (470 + DisplayHeightExt) - 18, wLife);

	char strTipText[256];
	if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		sprintf(strTipText, GlobalText[358], wLife, wLifeMax);
		RenderTipText((int)x, (int)(413 + DisplayHeightExt), strTipText);
	}

	// mana
	width = 52;
	height = 51;
	x = 488.7f + DisplayWinExt;
	y = DisplayHeight - 50.5f;

	fY = y + (fMana * height);
	fH = height - (fMana * height);
	fV = fMana;
	RenderBitmap(IMAGE_GAUGE_BLUE, x, fY, width, fH, 0.f, fV * height / 64.f, width / 64.f, (1.0f - fV) * height / 64.f);

	SEASON3B::RenderNumber(x + 30, (470 + DisplayHeightExt) - 18, wMana);

	// mana
	if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		sprintf(strTipText, GlobalText[359], wMana, wManaMax);
		RenderTipText((int)x, (int)(413 + DisplayHeightExt), strTipText);
	}
}

void SEASON3B::CNewUIMainFrameWindow::RenderGuageAGSS2()
{
	float x, y, width, height;
	float fY, fH, fV;

	DWORD dwMaxSkillMana, dwSkillMana;

	//if(gCharacterManager.IsMasterLevel(Hero->Class) == true)
	//if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
	//{
	//
	//	dwMaxSkillMana = max(1, Master_Level_Data.wMaxBP);
	//	dwSkillMana = min(dwMaxSkillMana, CharacterAttribute->PrintPlayer.ViewCurBP);
	//}
	//else
	{
		dwMaxSkillMana = max(1, CharacterAttribute->PrintPlayer.ViewMaxBP);
		dwSkillMana = min(dwMaxSkillMana, CharacterAttribute->PrintPlayer.ViewCurBP);
	}

	float fSkillMana = 0.0f;

	if (dwMaxSkillMana > 0)
	{
		fSkillMana = (dwMaxSkillMana - dwSkillMana) / (float)dwMaxSkillMana;
	}

	width = 14;
	height = 36;
	x = 551.5f + DisplayWinExt;
	y = DisplayHeight - (480 - 435);

	fY = y + (fSkillMana * height);
	fH = height - (fSkillMana * height);
	fV = fSkillMana;

	RenderBitmap(IMAGE_GAUGE_AG, x, fY, width, fH, 0.f, fV * height / 64.f, width / 32.f, (1.0f - fV) * height / 64.f);
	SEASON3B::RenderNumber(x + 10, (480 + DisplayHeightExt) - 18, (int)dwSkillMana);

	if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		char strTipText[256];

		sprintf(strTipText, GlobalText[214], dwSkillMana, dwMaxSkillMana);
		RenderTipText((int)x - 20, (int)(418 + DisplayHeightExt), strTipText);
	}
}
bool SetPostButtonAuto = false;
void SEASON3B::CNewUIMainFrameWindow::RenderGuageSDSS2()
{
	float x, y, width, height;
	float fY, fH, fV;
	DWORD wMaxShield, wShield;

	////Master_Level_Data.wMaxShield
	//if(gCharacterManager.IsMasterLevel(Hero->Class) == true)
	//if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
	//{
	//	wMaxShield = max(1, Master_Level_Data.wMaxShield);
	//	wShield = min(wMaxShield, CharacterAttribute->PrintPlayer.ViewCurSD);
	//}
	//else
	{
		wMaxShield = max(1, CharacterAttribute->PrintPlayer.ViewMaxSD);
		wShield = min(wMaxShield, CharacterAttribute->PrintPlayer.ViewCurSD);
	}

	float fShield = 0.0f;

	if (wMaxShield > 0)
	{
		fShield = (wMaxShield - wShield) / (float)wMaxShield;
	}

	width = 14;
	height = 36;
	x = 73.0f + DisplayWinExt;
	y = DisplayHeight - (480 - 435);
	fY = y + (fShield * height);
	fH = height - (fShield * height);
	fV = fShield;

	RenderBitmap(IMAGE_GAUGE_SD, x, fY, width, fH, 0.f, fV * height / 64.f, width / 32.f, (1.0f - fV) * height / 64.f);
	SEASON3B::RenderNumber(x + 15, (480 + DisplayHeightExt) - 18, (int)wShield);

	height = 39.f;
	y = (480.f + DisplayHeightExt) - 10.f - 39.f;
	if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		char strTipText[256];

		sprintf(strTipText, GlobalText[2037], wShield, wMaxShield);
		RenderTipText((int)x - 20, (int)(418 + DisplayHeightExt), strTipText);
	}
	//===Cord X Y
	SEASON3B::RenderNumber(DisplayWinExt+22, (480 + DisplayHeightExt) - 18, (int)Hero->PositionX);
	SEASON3B::RenderNumber(DisplayWinExt+22 + 24, (480 + DisplayHeightExt) - 18, (int)Hero->PositionY);


}
void SEASON3B::CNewUIMainFrameWindow::RenderExperienceSS2()
{

	__int64 wLevel;
	__int64 dwNexExperience;
	__int64 dwExperience;
	double x, y, width, height;



	{
		wLevel = CharacterAttribute->Level;
		dwNexExperience = CharacterAttribute->NextExperince;
		dwExperience = CharacterAttribute->Experience;
	}


	{
		x = 0 + DisplayWinExt;
		y = 470 + DisplayHeightExt;
		width = 6; height = 4;

		WORD wPriorLevel = wLevel - 1;
		DWORD dwPriorExperience = 0;

		if (wPriorLevel > 0)
		{
			dwPriorExperience = (9 + wPriorLevel) * wPriorLevel * wPriorLevel * 10;

			if (wPriorLevel > 255)
			{
				int iLevelOverN = wPriorLevel - 255;
				dwPriorExperience += (9 + iLevelOverN) * iLevelOverN * iLevelOverN * 1000;
			}
		}

		float fNeedExp = dwNexExperience - dwPriorExperience;
		float fExp = dwExperience - dwPriorExperience;

		if (dwExperience < dwPriorExperience)
		{
			fExp = 0.f;
		}

		float fExpBarNum = 0.f;
		if (fExp > 0.f && fNeedExp > 0)
		{
			fExpBarNum = (fExp / fNeedExp) * 10.f;
		}

		float fProgress = fExpBarNum;
		fProgress = fProgress - (int)fProgress;

		float FixX = 220.f, FixY = 439.f, FixW = 194;
		if (m_bExpEffect == true)
		{
			float fPreProgress = 0.f;
			fExp = m_dwPreExp - dwPriorExperience;
			if (m_dwPreExp < dwPriorExperience)
			{
				x = FixX + DisplayWinExt;
				y = FixY + DisplayHeightExt;
				width = fProgress * FixW;
				height = 4.f;
				RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
				glColor4f(1.f, 1.f, 1.f, 0.4f);
				RenderColor(x, y, width, height);
				EndRenderColor();
			}
			else
			{
				int iPreExpBarNum = 0;
				int iExpBarNum = 0;
				if (fExp > 0.f && fNeedExp > 0.f)
				{
					fPreProgress = (fExp / fNeedExp) * 10.f;
					iPreExpBarNum = (int)fPreProgress;
					fPreProgress = fPreProgress - (int)fPreProgress;
				}

				iExpBarNum = (int)fExpBarNum;

				if (iExpBarNum > iPreExpBarNum)
				{
					x = FixX + DisplayWinExt;
					y = FixY + DisplayHeightExt;
					width = fProgress * FixW;
					height = 4.f;
					RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
					glColor4f(1.f, 1.f, 1.f, 0.4f);
					RenderColor(x, y, width, height);
					EndRenderColor();
				}
				else
				{
					float fGapProgress = 0.f;
					fGapProgress = fProgress - fPreProgress;
					x = FixX + DisplayWinExt;
					y = FixY + DisplayHeightExt;
					width = fPreProgress * FixW;
					height = 4.f;
					RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
					x += width;
					width = fGapProgress * FixW;
					RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
					glColor4f(1.f, 1.f, 1.f, 0.4f);
					RenderColor(x, y, width, height);
					EndRenderColor();
				}
			}
		}
		else
		{
			x = FixX + DisplayWinExt;
			y = FixY + DisplayHeightExt;
			width = fProgress * FixW;
			height = 4.f;
			RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
		}
		//===Number Ex[
		int iExp = (int)fExpBarNum;
		x = 425 + DisplayWinExt;
		y = 433.f + DisplayHeightExt;
		SEASON3B::RenderNumber(x, y, iExp, 1.0);
		//=== Tool Tip
		x = FixX + DisplayWinExt;
		y = FixY + DisplayHeightExt;
		width = FixW + 40;
		height = 4.f;
		if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			char strTipText[256];

			sprintf(strTipText, GlobalText[1748], dwExperience, dwNexExperience);
			RenderTipText(280 + DisplayWinExt, 418 + DisplayHeightExt, strTipText);
		}
	}
}

void SEASON3B::CNewUIMainFrameWindow::RenderButtonsSS2()
{
#if defined(__ANDROID__)
	if (TakumiAndroid_ShouldHideLegacyMenuChrome())
	{
		return;
	}
#endif

	char strTipText[256];
	double x, y, width, height;
	x = ButtonSS2.x + 1;
	y = ButtonSS2.y - 1;

	width = ButtonSS2W; height = ButtonSS2H;
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER))
	{
		//RenderBitmap(IMAGE_MENUS2_BUTTON, x + 259 + 62, y, width, height, (width / 129) * 3, 0.f, width / 130, height / 33); //Char Info
		SEASON3B::RenderImage(IMAGE_iNewCharacter, x + 259 + 31, y, 25, 25);
	}
	else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY))
	{
		//RenderBitmap(IMAGE_MENUS2_BUTTON, x + 259 + 30, y, width, height, (width / 129) * 2, 0.f, width / 130, height / 33); //thung do
		SEASON3B::RenderImage(IMAGE_iNewInventory, x + 259 + 62, y, 25, 25);
	}
	else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PARTY))
	{
		SEASON3B::RenderImage(IMAGE_iNewParty, x + 259, y, 25, 25);
		//RenderBitmap(IMAGE_MENUS2_BUTTON, x + 259, y, width, height, (width / 130) * 1, 0.f, width / 130, height / 33); //Guild
	}
	else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP))
	{
		SEASON3B::RenderImage(IMAGE_iNewCashShop, x + 259 + (31 * 3), y, 25, 25);
		//RenderBitmap(IMAGE_MENUS2_BUTTON, x + 259, y, width, height, (width / 130) * 1, 0.f, width / 130, height / 33); //Guild
	}
	else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_FRIEND))
	{
		SEASON3B::RenderImage(IMAGE_iNewWinpush, x+492, y-20, 52, 20);
	}
	else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GUILDINFO))
	{
		SEASON3B::RenderImage(IMAGE_iNewGuild, x + 492, y+5, 52, 20);
	}
	if (SEASON3B::CheckMouseIn(x + 492, y + 5, 52, 20) == true)
	{
		//sprintf(strTipText, "Guild");
		RenderTipText(x + 492, y + 5 - 35, "Guild"); //Nhóm
		SEASON3B::RenderImage(IMAGE_iNewGuild, x + 492, y + 5, 52, 20);
		//RenderBitmap(IMAGE_MENUS2_BUTTON, x + 259, y, width, height, (width / 130) * 1, 0.f, width / 130, height / 33); //Guild
	}
	//nhom
	if (SEASON3B::CheckMouseIn(x + 259, y, width, height) == true)
	{
		//sprintf(strTipText, "Guild");
		RenderTipText(x + 259, y - 35, GlobalText[361]); //Nhóm
		SEASON3B::RenderImage(IMAGE_iNewParty, x + 259, y, 25, 25);
		//RenderBitmap(IMAGE_MENUS2_BUTTON, x + 259, y, width, height, (width / 130) * 1, 0.f, width / 130, height / 33); //Guild
	}
	//==Char
	if (SEASON3B::CheckMouseIn(x + 259 + 31, y, width, height) == true)
	{

		RenderTipText(x + 259 + 30, y - 35, GlobalText[363]);
		SEASON3B::RenderImage(IMAGE_iNewCharacter, x + 259 + 31, y, 25, 25);
	}
	//==Inv
	if (SEASON3B::CheckMouseIn(x + 259 + 62, y, width, height) == true)
	{
		RenderTipText(x + 259 + 62, y - 35, GlobalText[362]);
		SEASON3B::RenderImage(IMAGE_iNewInventory, x + 259 + 62, y, 25, 25);
	}
	//cash shop
	if (SEASON3B::CheckMouseIn(x + 259 + (31 * 3), y, width, height) == true)
	{
		RenderTipText(x + 259 + (31 * 3), y - 35, GlobalText[2277]);
		SEASON3B::RenderImage(IMAGE_iNewCashShop, x + 259 + (31 * 3), y, 25, 25);
	}
	//Feind
	if (SEASON3B::CheckMouseIn(x + 492, y - 20, 52, 18) == true)
	{
		RenderTipText(x + 492, y - 20-35, GlobalText[1043]);
		SEASON3B::RenderImage(IMAGE_iNewWinpush, x + 492, y - 20, 52, 20);
	}
}
bool SEASON3B::CNewUIMainFrameWindow::RenderSS2()
{
#if defined(__ANDROID__) || defined(MU_IOS)
	if (MU_MobileIsModernMobileHudEnabled())
	{
		return true;
	}
#endif
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	RenderFrame();

	m_pNewUI3DRenderMng->RenderUI2DEffect(ITEMHOTKEYNUMBER_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);

	g_pSkillList->RenderCurrentSkillAndHotSkillList();
	//
	EnableAlphaTest();
	RenderLifeManaSS2();
	RenderGuageSDSS2();
	RenderGuageAGSS2();
	RenderButtonsSS2();
	RenderExperienceSS2();
	DisableAlphaBlend();
	return true;
}
bool SEASON3B::CNewUIMainFrameWindow::Render()
{
#if defined(__ANDROID__) || defined(MU_IOS)
	if (MU_MobileIsModernMobileHudEnabled())
	{
		return true;
	}
#endif
	if (gProtect.m_MainInfo.IsVersion == 1) //Skin Ss2
	{
		return RenderSS2();
	}
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	RenderFrame();

	m_pNewUI3DRenderMng->RenderUI2DEffect(ITEMHOTKEYNUMBER_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);

	g_pSkillList->RenderCurrentSkillAndHotSkillList();

	EnableAlphaTest();
	RenderLifeMana();
	RenderGuageSD();
	RenderGuageAG();
	RenderButtons();
	RenderExperience();
	DisableAlphaBlend();

	

	return true;
}

void SEASON3B::CNewUIMainFrameWindow::Render3D()
{
#if defined(__ANDROID__) && TAKUMI_ANDROID_UI_SKIP_HOTKEY_ITEM_3D
	return;
#endif
#if defined(__ANDROID__) || defined(MU_IOS)
	if (MU_MobileIsModernMobileHudEnabled())
	{
		return;
	}
#endif
	m_ItemHotKey.RenderItems();
}

void SEASON3B::CNewUIMainFrameWindow::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
	g_pMainFrame->RenderHotKeyItemCount();

}

bool SEASON3B::CNewUIMainFrameWindow::IsVisible() const
{
	return CNewUIObj::IsVisible();
}

void SEASON3B::CNewUIMainFrameWindow::RenderFrame()
{
#if defined(__ANDROID__) || defined(MU_IOS)
	if (MU_MobileIsModernMobileHudEnabled())
	{
		return;
	}
#endif
	float width, height;
	float x, y;
	//if (gProtect.m_MainInfo.IsVersion == 1) //Skin Ss2
	//{

	//	width = 640.f; height = 61.f;
	//	x = DisplayWinExt;
	//	y = (DisplayHeight)-height;
	//	SEASON3B::RenderImage(IMAGE_MENU_1, x, y, width, height);

	//}
	//else
	{


		width = 256.f; height = 51.f;
		x = DisplayWinExt;
		y = (DisplayHeight)-height;
#if(WIDE_SCREEN)
		RenderBitmap(IMAGE_DECOR_WIDE, x - 70.28048780487805, y, 70.28048780487805, height, 0.0, 0.0, 0.8785000443, 0.6409999728, 1, 1, 0.0);
#endif
		SEASON3B::RenderImage(IMAGE_MENU_1, x, y, width, height);


		width = 128.f;
		x = 256.f + DisplayWinExt;
		SEASON3B::RenderImage(IMAGE_MENU_2, x, y, width, height);


		width = 256.f;
		x = 256.f + 128.f + DisplayWinExt;

		SEASON3B::RenderImage(IMAGE_MENU_3, x, y, width, height);
#if(WIDE_SCREEN)
		RenderBitmap(IMAGE_DECOR_WIDE, x + width, y, 70.28048780487805, height, 0.8785000443, 0.0, -0.8785000443, 0.6409999728, 1, 1, 0.0);
#endif

		if (g_pSkillList->IsSkillListUp() == true)
		{
			width = 160.f; height = 40.f;
			x = 222.f + DisplayWinExt;
			if (gProtect.m_MainInfo.IsVersion != 1)
			SEASON3B::RenderImage(IMAGE_MENU_2_1, x, y, width, height);
		}


	}
}

void SEASON3B::CNewUIMainFrameWindow::RenderLifeMana()
{
	DWORD wLifeMax, wLife, wManaMax, wMana;


	//if(gCharacterManager.IsMasterLevel( Hero->Class ) == true )
	//{
	//	wLifeMax = Master_Level_Data.wMaxLife;
	//	wLife = min(max(0, CharacterAttribute->PrintPlayer.ViewCurHP), wLifeMax);
	//	wManaMax = Master_Level_Data.wMaxMana;
	//	wMana = min(max(0, CharacterAttribute->PrintPlayer.ViewCurMP), wManaMax);
	//}
	//else
	{
		TakumiResolveMainFrameVitals(wLifeMax, wLife, wManaMax, wMana);
	}

	if (wLifeMax > 0)
	{
		if (wLife > 0 && (wLife / (float)wLifeMax) < 0.2f)
		{
			PlayBuffer(SOUND_HEART);
		}
	}

	float fLife = 0.f;
	float fMana = 0.f;

	if (wLifeMax > 0)
	{
		fLife = (wLifeMax - wLife) / (float)wLifeMax;
	}
	if (wManaMax > 0)
	{
		fMana = (wManaMax - wMana) / (float)wManaMax;
	}

	float width, height;
	float x, y;
	float fY, fH, fV;

	// life
	width = 45.f;
	x = 158 + DisplayWinExt;
	height = 39.f;
	y = DisplayHeight - 48.f;

	fY = y + (fLife * height);
	fH = height - (fLife * height);
	fV = fLife;
	if (g_isCharacterBuff((&Hero->Object), eDeBuff_Poison))
	{
		RenderBitmap(IMAGE_GAUGE_GREEN, x, fY, width, fH, 0.f, fV * height / 64.f, width / 64.f, (1.0f - fV) * height / 64.f);
	}
	else
	{
		RenderBitmap(IMAGE_GAUGE_RED, x, fY, width, fH, 0.f, fV * height / 64.f, width / 64.f, (1.0f - fV) * height / 64.f);
	}

	SEASON3B::RenderNumber(x + 25, (480 + DisplayHeightExt) - 38, wLife);

	char strTipText[256];
	if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		sprintf(strTipText, GlobalText[358], wLife, wLifeMax);
		RenderTipText((int)x, (int)(418 + DisplayHeightExt), strTipText);
	}

	// mana
	width = 45.f;
	x = 256.f + 128.f + 53.f + DisplayWinExt;
	height = 39.f;
	y = DisplayHeight - 48.f;

	fY = y + (fMana * height);
	fH = height - (fMana * height);
	fV = fMana;
	RenderBitmap(IMAGE_GAUGE_BLUE, x, fY, width, fH, 0.f, fV * height / 64.f, width / 64.f, (1.0f - fV) * height / 64.f);

	SEASON3B::RenderNumber(x + 30, (480 + DisplayHeightExt) - 38, wMana);

	// mana
	if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		sprintf(strTipText, GlobalText[359], wMana, wManaMax);
		RenderTipText((int)x, (int)(418 + DisplayHeightExt), strTipText);
	}
}

void SEASON3B::CNewUIMainFrameWindow::RenderGuageAG()
{
	float x, y, width, height;
	float fY, fH, fV;

	DWORD dwMaxSkillMana, dwSkillMana;

	//if(gCharacterManager.IsMasterLevel(Hero->Class) == true)
	//if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
	//{
	//
	//	dwMaxSkillMana = max(1, Master_Level_Data.wMaxBP);
	//	dwSkillMana = min(dwMaxSkillMana, CharacterAttribute->PrintPlayer.ViewCurBP);
	//}
	//else
	{
		dwMaxSkillMana = max(1, CharacterAttribute->PrintPlayer.ViewMaxBP);
		dwSkillMana = min(dwMaxSkillMana, CharacterAttribute->PrintPlayer.ViewCurBP);
	}

	float fSkillMana = 0.0f;

	if (dwMaxSkillMana > 0)
	{
		fSkillMana = (dwMaxSkillMana - dwSkillMana) / (float)dwMaxSkillMana;
	}

	width = 16.f, height = 39.f;
	x = 256 + 128 + 36 + DisplayWinExt; y = DisplayHeight - 49.f;
	fY = y + (fSkillMana * height);
	fH = height - (fSkillMana * height);
	fV = fSkillMana;

	RenderBitmap(IMAGE_GAUGE_AG, x, fY, width, fH, 0.f, fV * height / 64.f, width / 16.f, (1.0f - fV) * height / 64.f);
	SEASON3B::RenderNumber(x + 10, (480 + DisplayHeightExt) - 18, (int)dwSkillMana);

	if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		char strTipText[256];

		sprintf(strTipText, GlobalText[214], dwSkillMana, dwMaxSkillMana);
		RenderTipText((int)x - 20, (int)(418 + DisplayHeightExt), strTipText);
	}
}

void SEASON3B::CNewUIMainFrameWindow::RenderGuageSD()
{
	float x, y, width, height;
	float fY, fH, fV;
	DWORD wMaxShield, wShield;

	////Master_Level_Data.wMaxShield
	//if(gCharacterManager.IsMasterLevel(Hero->Class) == true)
	//if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
	//{
	//	wMaxShield = max(1, Master_Level_Data.wMaxShield);
	//	wShield = min(wMaxShield, CharacterAttribute->PrintPlayer.ViewCurSD);
	//}
	//else
	{
		wMaxShield = max(1, CharacterAttribute->PrintPlayer.ViewMaxSD);
		wShield = min(wMaxShield, CharacterAttribute->PrintPlayer.ViewCurSD);
	}

	float fShield = 0.0f;

	if (wMaxShield > 0)
	{
		fShield = (wMaxShield - wShield) / (float)wMaxShield;
	}

	width = 16.f, height = 39.f;
	x = 204 + DisplayWinExt; y = DisplayHeight - 49.f;
	fY = y + (fShield * height);
	fH = height - (fShield * height);
	fV = fShield;

	RenderBitmap(IMAGE_GAUGE_SD, x, fY, width, fH, 0.f, fV * height / 64.f, width / 16.f, (1.0f - fV) * height / 64.f);
	SEASON3B::RenderNumber(x + 15, (480 + DisplayHeightExt) - 18, (int)wShield);

	height = 39.f;
	y = (480.f + DisplayHeightExt) - 10.f - 39.f;
	if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		char strTipText[256];

		sprintf(strTipText, GlobalText[2037], wShield, wMaxShield);
		RenderTipText((int)x - 20, (int)(418 + DisplayHeightExt), strTipText);
	}
}

void SEASON3B::CNewUIMainFrameWindow::RenderExperience()
{

	__int64 wLevel;
	__int64 dwNexExperience;
	__int64 dwExperience;
	double x, y, width, height;


	//if(gCharacterManager.IsMasterLevel(CharacterAttribute->Class) == true)
	//{
	//	wLevel = (__int64)Master_Level_Data.nMLevel;
	//	dwNexExperience = (__int64)Master_Level_Data.lNext_MasterLevel_Experince;
	//	dwExperience = (__int64)Master_Level_Data.lMasterLevel_Experince;
	//}
	//else
	{
		wLevel = CharacterAttribute->Level;
		dwNexExperience = CharacterAttribute->NextExperince;
		dwExperience = CharacterAttribute->Experience;
	}

	if (gCharacterManager.IsMasterLevel(CharacterAttribute->Class) == true)
	{
		x = DisplayWinExt; y = 470 + DisplayHeightExt; width = 6; height = 4;

		__int64 iTotalLevel = wLevel + 400;
		__int64 iTOverLevel = iTotalLevel - 255;
		__int64 iBaseExperience = 0;

		__int64 iData_Master =	// A
			(
			(
				(__int64)9 + (__int64)iTotalLevel
				)
				* (__int64)iTotalLevel
				* (__int64)iTotalLevel
				* (__int64)10
				)
			+
			(
			(
				(__int64)9 + (__int64)iTOverLevel
				)
				* (__int64)iTOverLevel
				* (__int64)iTOverLevel
				* (__int64)1000
				);
		iBaseExperience = (iData_Master - (__int64)3892250000) / (__int64)2;	// B

		double fNeedExp = (double)dwNexExperience - (double)iBaseExperience;
		double fExp = (double)dwExperience - (double)iBaseExperience;

		if (dwExperience < iBaseExperience)
		{
			fExp = 0.f;
		}

		double fExpBarNum = 0.f;
		if (fExp > 0.f && fNeedExp > 0)
		{
			fExpBarNum = ((double)fExp / (double)fNeedExp) * (double)10.f;
		}

		double fProgress = fExpBarNum - __int64(fExpBarNum);

		if (m_bExpEffect == true)
		{
			double fPreProgress = 0.f;
			double fPreExp = (double)m_loPreExp - (double)iBaseExperience;
			if (m_loPreExp < iBaseExperience)
			{
				x = 2.f + DisplayWinExt; y = 473.f + DisplayHeightExt; width = fProgress * 629.f; height = 4.f;
				RenderBitmap(IMAGE_MASTER_GAUGE_BAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
				glColor4f(1.f, 1.f, 1.f, 0.6f);
				RenderColor(x, y, width, height);
				EndRenderColor();
			}
			else
			{
				int iPreExpBarNum = 0;
				int iExpBarNum = 0;
				if (fPreExp > 0.f && fNeedExp > 0.f)
				{
					fPreProgress = ((double)fPreExp / (double)fNeedExp) * (double)10.f;
					iPreExpBarNum = (int)fPreProgress;
					fPreProgress = (double)fPreProgress - __int64(fPreProgress);
				}
				iExpBarNum = (int)fExpBarNum;

				if (iExpBarNum > iPreExpBarNum)
				{
					x = 2.f + DisplayWinExt; y = 473.f + DisplayHeightExt; width = fProgress * 629.f; height = 4.f;
					RenderBitmap(IMAGE_MASTER_GAUGE_BAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
					glColor4f(1.f, 1.f, 1.f, 0.6f);
					RenderColor(x, y, width, height);
					EndRenderColor();
				}
				else
				{
					double fGapProgress = 0.f;
					fGapProgress = (double)fProgress - (double)fPreProgress;
					x = 2.f + DisplayWinExt; y = 473.f + DisplayHeightExt; width = (double)fPreProgress * (double)629.f; height = 4.f;
					RenderBitmap(IMAGE_MASTER_GAUGE_BAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);

					x += width; width = (double)fGapProgress * (double)629.f;
					RenderBitmap(IMAGE_MASTER_GAUGE_BAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
					glColor4f(1.f, 1.f, 1.f, 0.6f);
					RenderColor(x, y, width, height);
					EndRenderColor();
				}
			}
		}
		else
		{
			x = 2.f + DisplayWinExt; y = 473.f + DisplayHeightExt; width = fProgress * 629.f; height = 4.f;
			RenderBitmap(IMAGE_MASTER_GAUGE_BAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
		}

		int iExp = (int)fExpBarNum;
		x = 635.f + DisplayWinExt; y = 469.f + DisplayHeightExt;
		SEASON3B::RenderNumber(x, y, iExp);

		x = 2.f + DisplayWinExt; y = 473.f + DisplayHeightExt; width = 629.f; height = 4.f;
		if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			char strTipText[256];

			sprintf(strTipText, GlobalText[1748], dwExperience, dwNexExperience);
			RenderTipText(280 + DisplayWinExt, 418 + DisplayHeightExt, strTipText); //Text Exp
		}
	}
	else
	{
		x = 0 + DisplayWinExt; y = 470 + DisplayHeightExt; width = 6; height = 4;

		WORD wPriorLevel = wLevel - 1;
		DWORD dwPriorExperience = 0;

		if (wPriorLevel > 0)
		{
			dwPriorExperience = (9 + wPriorLevel) * wPriorLevel * wPriorLevel * 10;

			if (wPriorLevel > 255)
			{
				int iLevelOverN = wPriorLevel - 255;
				dwPriorExperience += (9 + iLevelOverN) * iLevelOverN * iLevelOverN * 1000;
			}
		}

		float fNeedExp = dwNexExperience - dwPriorExperience;
		float fExp = dwExperience - dwPriorExperience;

		if (dwExperience < dwPriorExperience)
		{
			fExp = 0.f;
		}

		float fExpBarNum = 0.f;
		if (fExp > 0.f && fNeedExp > 0)
		{
			fExpBarNum = (fExp / fNeedExp) * 10.f;
		}

		float fProgress = fExpBarNum;
		fProgress = fProgress - (int)fProgress;

		if (m_bExpEffect == true)
		{
			float fPreProgress = 0.f;
			fExp = m_dwPreExp - dwPriorExperience;
			if (m_dwPreExp < dwPriorExperience)
			{
				x = 2.f + DisplayWinExt; y = 473.f + DisplayHeightExt; width = fProgress * 629.f; height = 4.f;
				RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
				glColor4f(1.f, 1.f, 1.f, 0.4f);
				RenderColor(x, y, width, height);
				EndRenderColor();
			}
			else
			{
				int iPreExpBarNum = 0;
				int iExpBarNum = 0;
				if (fExp > 0.f && fNeedExp > 0.f)
				{
					fPreProgress = (fExp / fNeedExp) * 10.f;
					iPreExpBarNum = (int)fPreProgress;
					fPreProgress = fPreProgress - (int)fPreProgress;
				}

				iExpBarNum = (int)fExpBarNum;

				if (iExpBarNum > iPreExpBarNum)
				{
					x = 2.f + DisplayWinExt; y = 473.f + DisplayHeightExt; width = fProgress * 629.f; height = 4.f;
					RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
					glColor4f(1.f, 1.f, 1.f, 0.4f);
					RenderColor(x, y, width, height);
					EndRenderColor();
				}
				else
				{
					float fGapProgress = 0.f;
					fGapProgress = fProgress - fPreProgress;
					x = 2.f + DisplayWinExt; y = 473.f + DisplayHeightExt; width = fPreProgress * 629.f; height = 4.f;
					RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
					x += width; width = fGapProgress * 629.f;
					RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
					glColor4f(1.f, 1.f, 1.f, 0.4f);
					RenderColor(x, y, width, height);
					EndRenderColor();
				}
			}
		}
		else
		{
			x = 2.f + DisplayWinExt; y = 473.f + DisplayHeightExt; width = fProgress * 629.f; height = 4.f;
			RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f / 8.f, 4.f / 4.f);
		}

		int iExp = (int)fExpBarNum;
		x = 635.f + DisplayWinExt; y = 469.f + DisplayHeightExt;
		SEASON3B::RenderNumber(x, y, iExp);

		x = 2.f + DisplayWinExt; y = 473.f + DisplayHeightExt; width = 629.f; height = 4.f;
		if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			char strTipText[256];

			sprintf(strTipText, GlobalText[1748], dwExperience, dwNexExperience);
			RenderTipText(280 + DisplayWinExt, 418 + DisplayHeightExt, strTipText);
		}
	}
}

void SEASON3B::CNewUIMainFrameWindow::RenderHotKeyItemCount()
{
#if defined(__ANDROID__) || defined(MU_IOS)
	if (MU_MobileIsModernMobileHudEnabled())
	{
		return;
	}
#endif
	m_ItemHotKey.RenderItemCount();
}

void SEASON3B::CNewUIMainFrameWindow::RenderButtons()
{
#if defined(__ANDROID__)
	if (TakumiAndroid_ShouldHideLegacyMenuChrome())
	{
		return;
	}
#endif

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
	m_BtnCShop.Render();
#endif //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME

	RenderCharInfoButton(); // Thong Tin NV
	m_BtnMyInven.Render(); //Tui DO

	RenderFriendButton(); //Ban Be

	m_BtnWindow.Render(); //Menu mini
}

void SEASON3B::CNewUIMainFrameWindow::RenderCharInfoButton()
{
	m_BtnChaInfo.Render();

	if (g_QuestMng.IsQuestIndexByEtcListEmpty())
		return;

	if (g_Time.GetTimeCheck(5, 500))
		m_bButtonBlink = !m_bButtonBlink;

	if (m_bButtonBlink)
	{
		if (!(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER)))
		{
			RenderImage(IMAGE_MENU_BTN_CHAINFO, 489 + 30 + DisplayWinExt, (480 + DisplayHeightExt) - 51, 30, 41, 0.0f, 41.f);
		}

	}
}

void SEASON3B::CNewUIMainFrameWindow::RenderFriendButton()
{
	m_BtnFriend.Render();

	int iBlinkTemp = g_pFriendMenu->GetBlinkTemp();
	BOOL bIsAlertTime = (iBlinkTemp % 24 < 12);

	if (g_pFriendMenu->IsNewChatAlert() && bIsAlertTime)
	{
		RenderFriendButtonState();
	}
	if (g_pFriendMenu->IsNewMailAlert())
	{
		if (bIsAlertTime)
		{
			RenderFriendButtonState();

			if (iBlinkTemp % 24 == 11)
			{
				g_pFriendMenu->IncreaseLetterBlink();
			}
		}
	}
	else if (g_pLetterList->CheckNoReadLetter())
	{
		RenderFriendButtonState();
	}

	g_pFriendMenu->IncreaseBlinkTemp();
}

void SEASON3B::CNewUIMainFrameWindow::RenderFriendButtonState()
{
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_FRIEND) == true)
	{
		RenderImage(IMAGE_MENU_BTN_FRIEND, 489 + DisplayWinExt + (30 * 3), (480 + DisplayHeightExt) - 51, 30, 41, 0.0f, 123.f);
	}
	else
	{
		RenderImage(IMAGE_MENU_BTN_FRIEND, 489 + DisplayWinExt + (30 * 3), (480 + DisplayHeightExt) - 51, 30, 41, 0.0f, 41.f);
	}
#else //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_FRIEND) == true)
	{
		RenderImage(IMAGE_MENU_BTN_FRIEND, 488 + 76, 480 - 51, 38, 42, 0.0f, 126.f);
	}
	else
	{
		RenderImage(IMAGE_MENU_BTN_FRIEND, 488 + 76, 480 - 51, 38, 42, 0.0f, 42.f);
	}
#endif//defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
}

bool SEASON3B::CNewUIMainFrameWindow::UpdateMouseEvent()
{
	if (g_pNewUIHotKey->IsStateGameOver() == true)
	{
		return true;
	}

	if (BtnProcess() == true)
	{
		return false;
	}

	return true;
}

bool SEASON3B::CNewUIMainFrameWindow::BtnProcess()
{
#if defined(__ANDROID__) || defined(MU_IOS)
	if (MU_MobileIsModernMobileHudEnabled())
	{
		return false;
	}
#endif
	if (gProtect.m_MainInfo.IsVersion == 1) //Skin Ss2 Button Click
	{
		//if (SEASON3B::CheckMouseIn((int)ButtonSS2.x, (int)ButtonSS2.y, (int)ButtonSS2W, (int)ButtonSS2H))
		//{
		//	if (SEASON3B::IsPress(VK_LBUTTON))
		//	{

		//		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_WINDOW_MENU);
		//		PlayBuffer(SOUND_CLICK01);
		//		return true;
		//	}
		//}
		//===Nhom
		if (SEASON3B::CheckMouseIn((int)ButtonSS2.x + 259, (int)ButtonSS2.y, (int)ButtonSS2W, (int)ButtonSS2H))
		{
			if (SEASON3B::IsPress(VK_LBUTTON))
			{

				//g_pNewUISystem->Toggle(SEASON3B::INTERFACE_GUILDINFO);
				g_pNewUISystem->Toggle(SEASON3B::INTERFACE_PARTY_INFO_WINDOW);
				PlayBuffer(SOUND_CLICK01);
				return true;
			}
		}
		//=== Charinfo
		else if (SEASON3B::CheckMouseIn((int)ButtonSS2.x + 259 + 31, (int)ButtonSS2.y, (int)ButtonSS2W, (int)ButtonSS2H))
		{
			if (SEASON3B::IsPress(VK_LBUTTON))
			{

				g_pNewUISystem->Toggle(SEASON3B::INTERFACE_CHARACTER);
				PlayBuffer(SOUND_CLICK01);
				return true;
			}
		}
		//=== Inv
		else if (SEASON3B::CheckMouseIn((int)ButtonSS2.x + 259 + 62, (int)ButtonSS2.y, (int)ButtonSS2W, (int)ButtonSS2H))
		{
			if (SEASON3B::IsPress(VK_LBUTTON))
			{

				g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY);
				PlayBuffer(SOUND_CLICK01);
				return true;
			}
		}
		//=== Friend
		else if (SEASON3B::CheckMouseIn((int)ButtonSS2.x + 492, (int)ButtonSS2.y-20, (int)52, (int)20))
		{
			if (SEASON3B::IsPress(VK_LBUTTON))
			{

				g_pNewUISystem->Toggle(SEASON3B::INTERFACE_FRIEND);
				PlayBuffer(SOUND_CLICK01);
				return true;
			}
		}
		//=== Guild
		else if (SEASON3B::CheckMouseIn((int)ButtonSS2.x + 492, (int)ButtonSS2.y +5, (int)52, (int)20))
		{
			if (SEASON3B::IsPress(VK_LBUTTON))
			{

				g_pNewUISystem->Toggle(SEASON3B::INTERFACE_GUILDINFO);
				PlayBuffer(SOUND_CLICK01);
				return true;
			}
		}
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
		//=== cash
		else if (SEASON3B::CheckMouseIn((int)ButtonSS2.x + 259 + (31 * 3), (int)ButtonSS2.y, (int)ButtonSS2W, (int)ButtonSS2H))
		{
			if (SEASON3B::IsPress(VK_LBUTTON))
			{

				if (g_pInGameShop->IsInGameShopOpen() == false)
					return false;

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
				if (g_InGameShopSystem->IsScriptDownload() == true)
				{
					if (g_InGameShopSystem->ScriptDownload() == false)
						return false;
				}
				if (g_InGameShopSystem->IsBannerDownload() == true)
				{
					if (g_InGameShopSystem->BannerDownload() == true)
					{
						g_pInGameShop->InitBanner(g_InGameShopSystem->GetBannerFileName(), g_InGameShopSystem->GetBannerURL());
					}
				}
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

				if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP) == false)
				{
					if (g_InGameShopSystem->GetIsRequestShopOpenning() == false)
					{
						SendRequestIGS_CashShopOpen(0);
						g_InGameShopSystem->SetIsRequestShopOpenning(true);
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
						g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
					}
				}
				else
				{
					SendRequestIGS_CashShopOpen(1);
					g_pNewUISystem->Hide(SEASON3B::INTERFACE_INGAMESHOP);
				}
				PlayBuffer(SOUND_CLICK01);
				return true;
			}
		}
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
	}

	if (g_pNewUIHotKey->CanUpdateKeyEventRelatedMyInventory() == true)
	{
		if (m_BtnMyInven.UpdateMouseEvent() == true)
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY);

		}
	}
	else if (g_pNewUIHotKey->CanUpdateKeyEvent() == true)
	{
		if (m_BtnMyInven.UpdateMouseEvent() == true)
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY);
			PlayBuffer(SOUND_CLICK01);
			return true;
		}
		else if (m_BtnChaInfo.UpdateMouseEvent() == true)
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_CHARACTER);

			PlayBuffer(SOUND_CLICK01);

			if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER))
				g_QuestMng.SendQuestIndexByEtcSelection();

			return true;
		}
		else if (m_BtnFriend.UpdateMouseEvent() == true)
		{
			if (gMapManager.InChaosCastle() == true)
			{
				PlayBuffer(SOUND_CLICK01);
				return true;
			}

			int iLevel = CharacterAttribute->Level;

			if (iLevel < 6)
			{
				if (g_pChatListBox->CheckChatRedundancy(GlobalText[1067]) == FALSE)
				{
					g_pChatListBox->AddText("", GlobalText[1067], SEASON3B::TYPE_SYSTEM_MESSAGE);
				}
			}
			else
			{
				g_pNewUISystem->Toggle(SEASON3B::INTERFACE_FRIEND);
			}
			PlayBuffer(SOUND_CLICK01);
			return true;
		}
		else if (m_BtnWindow.UpdateMouseEvent() == true)
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_WINDOW_MENU);
			PlayBuffer(SOUND_CLICK01);
			return true;
		}

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
		else if (m_BtnCShop.UpdateMouseEvent() == true)
		{
			if (g_pInGameShop->IsInGameShopOpen() == false)
				return false;

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
			if (g_InGameShopSystem->IsScriptDownload() == true)
			{
				if (g_InGameShopSystem->ScriptDownload() == false)
					return false;
			}

			if (g_InGameShopSystem->IsBannerDownload() == true)
			{
				g_InGameShopSystem->BannerDownload();
			}
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

			if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP) == false)
			{
				if (g_InGameShopSystem->GetIsRequestShopOpenning() == false)
				{
					SendRequestIGS_CashShopOpen(0);
					g_InGameShopSystem->SetIsRequestShopOpenning(true);

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
					g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

				}
			}
			else
			{
				SendRequestIGS_CashShopOpen(1);
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_INGAMESHOP);
			}

			return true;
		}
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
	}

	return false;
}

bool SEASON3B::CNewUIMainFrameWindow::UpdateKeyEvent()
{
	if (m_ItemHotKey.UpdateKeyEvent() == false)
	{
		return false;
	}
	return true;
}

bool SEASON3B::CNewUIMainFrameWindow::Update()
{
	if (m_bExpEffect == true)
	{
		if (timeGetTime() - m_dwExpEffectTime > 2000)
		{
			m_bExpEffect = false;
			m_dwExpEffectTime = 0;
			m_dwGetExp = 0;
		}
	}

	return true;
}

float SEASON3B::CNewUIMainFrameWindow::GetLayerDepth()
{
	return 0.0;
}

float SEASON3B::CNewUIMainFrameWindow::GetKeyEventOrder()
{
	return 2.9f;
}

void SEASON3B::CNewUIMainFrameWindow::SetItemHotKey(int iHotKey, int iItemType, int iItemLevel)
{
	m_ItemHotKey.SetHotKey(iHotKey, iItemType, iItemLevel);
}

int SEASON3B::CNewUIMainFrameWindow::GetItemHotKey(int iHotKey)
{
	return m_ItemHotKey.GetHotKey(iHotKey);
}

int SEASON3B::CNewUIMainFrameWindow::GetItemHotKeyLevel(int iHotKey)
{
	return m_ItemHotKey.GetHotKeyLevel(iHotKey);
}

void SEASON3B::CNewUIMainFrameWindow::UseHotKeyItemRButton()
{
	m_ItemHotKey.UseItemRButton();
}

void SEASON3B::CNewUIMainFrameWindow::UpdateItemHotKey()
{
	m_ItemHotKey.UpdateKeyEvent();
}

void SEASON3B::CNewUIMainFrameWindow::ResetSkillHotKey()
{
	g_pSkillList->Reset();
}

void SEASON3B::CNewUIMainFrameWindow::SetSkillHotKey(int iHotKey, int iSkillType)
{
	g_pSkillList->SetHotKey(iHotKey, iSkillType);
}

int SEASON3B::CNewUIMainFrameWindow::GetSkillHotKey(int iHotKey)
{
	return g_pSkillList->GetHotKey(iHotKey);
}

int SEASON3B::CNewUIMainFrameWindow::GetSkillHotKeyIndex(int iSkillType)
{
	return g_pSkillList->GetSkillIndex(iSkillType);
}

SEASON3B::CNewUIItemHotKey::CNewUIItemHotKey()
{
	for (int i = 0; i < HOTKEY_COUNT; ++i)
	{
		m_iHotKeyItemType[i] = -1;
		m_iHotKeyItemLevel[i] = 0;
	}
}

SEASON3B::CNewUIItemHotKey::~CNewUIItemHotKey()
{

}

bool SEASON3B::CNewUIItemHotKey::UpdateKeyEvent()
{
	int iIndex = -1;

	if (SEASON3B::IsPress('Q') == true)
	{
		iIndex = GetHotKeyItemIndex(HOTKEY_Q);
	}
	else if (SEASON3B::IsPress('W') == true)
	{
		iIndex = GetHotKeyItemIndex(HOTKEY_W);
	}
	else if (SEASON3B::IsPress('E') == true)
	{
		iIndex = GetHotKeyItemIndex(HOTKEY_E);
	}
	else if (SEASON3B::IsPress('R') == true)
	{
		iIndex = GetHotKeyItemIndex(HOTKEY_R);
	}

	if (iIndex != -1)
	{
		ITEM* pItem = NULL;
		pItem = g_pMyInventory->FindItem(iIndex);
		if ((pItem->Type >= ITEM_POTION + 78 && pItem->Type <= ITEM_POTION + 82))
		{
			std::list<eBuffState> secretPotionbufflist;
			secretPotionbufflist.push_back(eBuff_SecretPotion1);
			secretPotionbufflist.push_back(eBuff_SecretPotion2);
			secretPotionbufflist.push_back(eBuff_SecretPotion3);
			secretPotionbufflist.push_back(eBuff_SecretPotion4);
			secretPotionbufflist.push_back(eBuff_SecretPotion5);

			if (g_isCharacterBufflist((&Hero->Object), secretPotionbufflist) != eBuffNone) {
				SEASON3B::CreateOkMessageBox(GlobalText[2530], RGBA(255, 30, 0, 255));
			}
			else {
				SendRequestUse(iIndex, 0);
			}
		}
		else

		{
			SendRequestUse(iIndex, 0);
		}
		return false;
	}

	return true;
}

int SEASON3B::CNewUIItemHotKey::GetHotKeyItemIndex(int iType, bool bItemCount)
{
	int iStartItemType = 0, iEndItemType = 0;
	int i, j;

	switch (iType)
	{
	case HOTKEY_Q:
		if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
		{
			if (m_iHotKeyItemType[iType] >= ITEM_POTION + 4 && m_iHotKeyItemType[iType] <= ITEM_POTION + 6)
			{
				iStartItemType = ITEM_POTION + 6; iEndItemType = ITEM_POTION + 4;
			}
			else
			{
				iStartItemType = ITEM_POTION + 3; iEndItemType = ITEM_POTION + 0;
			}
		}
		break;
	case HOTKEY_W:
		if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
		{
			if (m_iHotKeyItemType[iType] >= ITEM_POTION + 0 && m_iHotKeyItemType[iType] <= ITEM_POTION + 3)
			{
				iStartItemType = ITEM_POTION + 3; iEndItemType = ITEM_POTION + 0;
			}
			else
			{
				iStartItemType = ITEM_POTION + 6; iEndItemType = ITEM_POTION + 4;
			}
		}
		break;
	case HOTKEY_E:
		if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
		{
			if (m_iHotKeyItemType[iType] >= ITEM_POTION + 0 && m_iHotKeyItemType[iType] <= ITEM_POTION + 3)
			{
				iStartItemType = ITEM_POTION + 3; iEndItemType = ITEM_POTION + 0;
			}
			else if (m_iHotKeyItemType[iType] >= ITEM_POTION + 4 && m_iHotKeyItemType[iType] <= ITEM_POTION + 6)
			{
				iStartItemType = ITEM_POTION + 6; iEndItemType = ITEM_POTION + 4;
			}
			else
			{
				iStartItemType = ITEM_POTION + 8; iEndItemType = ITEM_POTION + 8;
			}
		}
		break;
	case HOTKEY_R:
		if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
		{
			if (m_iHotKeyItemType[iType] >= ITEM_POTION + 0 && m_iHotKeyItemType[iType] <= ITEM_POTION + 3)
			{
				iStartItemType = ITEM_POTION + 3; iEndItemType = ITEM_POTION + 0;
			}
			else if (m_iHotKeyItemType[iType] >= ITEM_POTION + 4 && m_iHotKeyItemType[iType] <= ITEM_POTION + 6)
			{
				iStartItemType = ITEM_POTION + 6; iEndItemType = ITEM_POTION + 4;
			}
			else
			{
				iStartItemType = ITEM_POTION + 37; iEndItemType = ITEM_POTION + 35;
			}
		}
		break;
	}

	int iItemCount = 0;
	ITEM* pItem = NULL;

	int iNumberofItems = g_pMyInventory->GetInventoryCtrl()->GetNumberOfItems();
	
	for (i = iStartItemType; i >= iEndItemType; --i)
	{
		if (bItemCount)
		{
			for (j = 0; j < iNumberofItems; ++j)
			{
				pItem = g_pMyInventory->GetInventoryCtrl()->GetItem(j);
				if (pItem == NULL)
				{
					continue;
				}

				if (
					(pItem->Type == i && ((pItem->Level >> 3) & 15) == m_iHotKeyItemLevel[iType])
					|| (pItem->Type == i && (pItem->Type >= ITEM_POTION + 0 && pItem->Type <= ITEM_POTION + 3))
					)
				{
					if (pItem->Type == ITEM_POTION + 9
						|| pItem->Type == ITEM_POTION + 10
						|| pItem->Type == ITEM_POTION + 20
						)
					{
						iItemCount++;
					}
					else
					{
						iItemCount += pItem->Durability;
					}
				}
			}
		}
		else
		{
			int iIndex = -1;
			if (i >= ITEM_POTION + 0 && i <= ITEM_POTION + 3)
			{
				iIndex = g_pMyInventory->FindItemReverseIndex(i);
			}
			else
			{
				iIndex = g_pMyInventory->FindItemReverseIndex(i, m_iHotKeyItemLevel[iType]);
			}

			if (-1 != iIndex)
			{
				pItem = g_pMyInventory->FindItem(iIndex);
				if ((pItem->Type != ITEM_POTION + 7
					&& pItem->Type != ITEM_POTION + 10
					&& pItem->Type != ITEM_POTION + 20)
					|| ((pItem->Level >> 3) & 15) == m_iHotKeyItemLevel[iType]
					)
				{
					return iIndex;
				}
			}
		}
	}

	//==Inv Ext
	for (int iInv = 0; iInv < CharacterAttribute->InventoryExtensions; iInv++)
	{
		int iNumberofItemsExt = g_pMyInventoryExt->GetInventoryCtrl(iInv)->GetNumberOfItems();
		if (iNumberofItemsExt == 0)
		{
			continue;
		}
		for (i = iStartItemType; i >= iEndItemType; --i)
		{
			if (bItemCount)
			{
				for (j = 0; j < iNumberofItemsExt; ++j)
				{
					pItem = g_pMyInventoryExt->GetInventoryCtrl(iInv)->GetItem(j);
					if (pItem == NULL)
					{
						continue;
					}

					if (
						(pItem->Type == i && ((pItem->Level >> 3) & 15) == m_iHotKeyItemLevel[iType])
						|| (pItem->Type == i && (pItem->Type >= ITEM_POTION + 0 && pItem->Type <= ITEM_POTION + 3))
						)
					{
						if (pItem->Type == ITEM_POTION + 9
							|| pItem->Type == ITEM_POTION + 10
							|| pItem->Type == ITEM_POTION + 20
							)
						{
							iItemCount++;
						}
						else
						{
							iItemCount += pItem->Durability;
						}
					}
				}
			}
			else
			{
				int iIndex = -1;
				if (i >= ITEM_POTION + 0 && i <= ITEM_POTION + 3)
				{
					iIndex = g_pMyInventoryExt->GetInventoryCtrl(iInv)->FindItemIndex(i,-1);
				}
				else
				{
					iIndex = g_pMyInventoryExt->GetInventoryCtrl(iInv)->FindItemIndex(i, m_iHotKeyItemLevel[iType]);
				}

				if (-1 != iIndex)
				{
					pItem = g_pMyInventoryExt->GetInventoryCtrl(iInv)->FindItem(iIndex);
					if ((pItem->Type != ITEM_POTION + 7
						&& pItem->Type != ITEM_POTION + 10
						&& pItem->Type != ITEM_POTION + 20)
						|| ((pItem->Level >> 3) & 15) == m_iHotKeyItemLevel[iType]
						)
					{
						return iIndex;
					}
				}
			}
		}
	}
	if (bItemCount == true)
	{
		return iItemCount;
	}

	return -1;
}

bool SEASON3B::CNewUIItemHotKey::GetHotKeyCommonItem(IN int iHotKey, OUT int& iStart, OUT int& iEnd)
{
	switch (m_iHotKeyItemType[iHotKey])
	{
	case ITEM_POTION + 7:
	case ITEM_POTION + 8:
	case ITEM_POTION + 9:
	case ITEM_POTION + 10:
	case ITEM_POTION + 20:
	case ITEM_POTION + 46:
	case ITEM_POTION + 47:
	case ITEM_POTION + 48:
	case ITEM_POTION + 49:
	case ITEM_POTION + 50:
	case ITEM_POTION + 70:
	case ITEM_POTION + 71:
	case ITEM_POTION + 78:
	case ITEM_POTION + 79:
	case ITEM_POTION + 80:
	case ITEM_POTION + 81:
	case ITEM_POTION + 82:
	case ITEM_POTION + 94:
	case ITEM_POTION + 85:
	case ITEM_POTION + 86:
	case ITEM_POTION + 87:
	case ITEM_POTION + 133:
		if (m_iHotKeyItemType[iHotKey] != ITEM_POTION + 20 || m_iHotKeyItemLevel[iHotKey] == 0)
		{
			iStart = iEnd = m_iHotKeyItemType[iHotKey];
			return true;
		}
		break;
	default:
		if (m_iHotKeyItemType[iHotKey] >= ITEM_POTION + 35 && m_iHotKeyItemType[iHotKey] <= ITEM_POTION + 37)
		{
			iStart = ITEM_POTION + 37; iEnd = ITEM_POTION + 35;
			return true;
		}
		else if (m_iHotKeyItemType[iHotKey] >= ITEM_POTION + 38 && m_iHotKeyItemType[iHotKey] <= ITEM_POTION + 40)
		{
			iStart = ITEM_POTION + 40; iEnd = ITEM_POTION + 38;
			return true;
		}
		break;
	}
	return false;
}

int SEASON3B::CNewUIItemHotKey::GetHotKeyItemCount(int iType)
{
	return 0;
}

void SEASON3B::CNewUIItemHotKey::SetHotKey(int iHotKey, int iItemType, int iItemLevel)
{
	if (iHotKey != -1 && CNewUIMyInventory::CanRegisterItemHotKey(iItemType) == true
		)
	{
		m_iHotKeyItemType[iHotKey] = iItemType;
		m_iHotKeyItemLevel[iHotKey] = iItemLevel;
	}
	else
	{
		m_iHotKeyItemType[iHotKey] = -1;
		m_iHotKeyItemLevel[iHotKey] = 0;
	}
}

int SEASON3B::CNewUIItemHotKey::GetHotKey(int iHotKey)
{
	if (iHotKey != -1)
	{
		return m_iHotKeyItemType[iHotKey];
	}

	return -1;
}

int SEASON3B::CNewUIItemHotKey::GetHotKeyLevel(int iHotKey)
{
	if (iHotKey != -1)
	{
		return m_iHotKeyItemLevel[iHotKey];
	}

	return 0;
}

void SEASON3B::CNewUIItemHotKey::RenderItems()
{
	float x, y, width, height;

	for (int i = 0; i < HOTKEY_COUNT; ++i)
	{
		int iIndex = GetHotKeyItemIndex(i);
		//gInterface.DrawMessage(1, " %d Index %d", i, iIndex);
		if (iIndex != -1)
		{
			ITEM* pItem = g_pMyInventory->FindItem(iIndex);
			if (pItem)
			{
				if (gProtect.m_MainInfo.IsVersion == 1) //Skin Ss2
				{
					x = 210.0f + DisplayWinExt + (i * 30);
					y = 453.0f + DisplayHeightExt;
					if (i == 3) break;
				}
				else
				{
					x = 10 + DisplayWinExt + (i * 38);
					y = 443 + DisplayHeightExt;
				}
				width = 20; height = 20;
				RenderItem3D(x, y, width, height, pItem->Type, pItem->Level, 0, 0);
			}
		}
	}
}

void SEASON3B::CNewUIItemHotKey::RenderItemCount()
{
	float x, y, width, height;

	glColor4f(1.f, 1.f, 1.f, 1.f);

	for (int i = 0; i < HOTKEY_COUNT; ++i)
	{
		int iCount = GetHotKeyItemIndex(i, true);
		if (iCount > 0)
		{
			if (gProtect.m_MainInfo.IsVersion == 1) //Skin Ss2
			{
				x = 230 + DisplayWinExt + (i * 30); y = 445.0f + DisplayHeightExt; width = 8; height = 9;
				if (i == 3) break;
			}
			else
			{
				x = 30 + DisplayWinExt + (i * 38); y = 457 + DisplayHeightExt; width = 8; height = 9;
			}

			SEASON3B::RenderNumber(x, y, iCount);
		}
	}
}

void SEASON3B::CNewUIItemHotKey::UseItemRButton()
{
	int x, y, width, height;

	for (int i = 0; i < HOTKEY_COUNT; ++i)
	{
		if (gProtect.m_MainInfo.IsVersion == 1) //Skin Ss2
		{
			x = 210 + DisplayWinExt + (i * 30); y = 445.0f + DisplayHeightExt; width = 20; height = 20;
			if (i == 3) break;
		}
		else
		{
			x = 10 + DisplayWinExt + (i * 38); y = 445 + DisplayHeightExt; width = 20; height = 20;
		}
		if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			if (MouseRButtonPush)
			{
				MouseRButtonPush = false;
				int iIndex = GetHotKeyItemIndex(i);
				if (iIndex != -1)
				{
					SendRequestUse(iIndex, 0);
					break;
				}
			}
		}
	}
}

SEASON3B::CNewUISkillList::CNewUISkillList()
{
	m_pNewUIMng = NULL;
	Reset();
}

SEASON3B::CNewUISkillList::~CNewUISkillList()
{
	Release();
}

bool SEASON3B::CNewUISkillList::Create(CNewUIManager * pNewUIMng, CNewUI3DRenderMng * pNewUI3DRenderMng)
{
	if (NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_SKILL_LIST, this);

	m_pNewUI3DRenderMng = pNewUI3DRenderMng;

	LoadImages();

	Show(true);

	return true;
}

void SEASON3B::CNewUISkillList::Release()
{
	if (m_pNewUI3DRenderMng)
	{
		m_pNewUI3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);
	}

	UnloadImages();

	if (m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

void SEASON3B::CNewUISkillList::Reset()
{
	m_bSkillList = false;
	m_bHotKeySkillListUp = false;
#if TAKUMI_ANDROID_UI_SKILL_PICKER_CACHE
	m_skillPickerLayoutDirty = true;
	m_skillPickerLayout.clear();
	m_skillPickerLayoutSig = 0;
#endif

	m_bRenderSkillInfo = false;
	m_iRenderSkillInfoType = 0;
	m_iRenderSkillInfoPosX = 0;
	m_iRenderSkillInfoPosY = 0;
	m_iAndroidTouchAssignSkillIndex = -1;

#if defined(__ANDROID__) || defined(MU_IOS)
	m_legacySkillPickerOffsetX = 0.f;
	m_virtualSkillPickerOffsetY = 0.f;
	m_virtualPickerAnchorCx = 0.f;
	m_virtualPickerAnchorCy = 0.f;
	m_legacyPickerTargetHotKey = -1;
	m_virtualPickerTargetVisualSlot = -1;
#endif

	for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
	{
		m_iHotKeySkillType[i] = -1;
	}

	m_EventState = EVENT_NONE;
}

void SEASON3B::CNewUISkillList::LoadImages()
{
	LoadBitmap("Interface\\newui_skill.jpg", IMAGE_SKILL1, GL_LINEAR);
	LoadBitmap("Interface\\newui_skill2.jpg", IMAGE_SKILL2, GL_LINEAR);
	LoadBitmap("Interface\\newui_command.jpg", IMAGE_COMMAND, GL_LINEAR);
	LoadBitmap("Interface\\newui_skillbox.jpg", IMAGE_SKILLBOX, GL_LINEAR);
	LoadBitmap("Interface\\newui_skillbox2.jpg", IMAGE_SKILLBOX_USE, GL_LINEAR);
	LoadBitmap("Interface\\newui_non_skill.jpg", IMAGE_NON_SKILL1, GL_LINEAR);
	LoadBitmap("Interface\\newui_non_skill2.jpg", IMAGE_NON_SKILL2, GL_LINEAR);
	LoadBitmap("Interface\\newui_non_command.jpg", IMAGE_NON_COMMAND, GL_LINEAR);
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	LoadBitmap("Interface\\newui_skill3.jpg", IMAGE_SKILL3, GL_LINEAR);
	LoadBitmap("Interface\\newui_non_skill3.jpg", IMAGE_NON_SKILL3, GL_LINEAR);
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
}

void SEASON3B::CNewUISkillList::UnloadImages()
{
	DeleteBitmap(IMAGE_SKILL1);
	DeleteBitmap(IMAGE_SKILL2);
	DeleteBitmap(IMAGE_COMMAND);
	DeleteBitmap(IMAGE_SKILLBOX);
	DeleteBitmap(IMAGE_SKILLBOX_USE);
	DeleteBitmap(IMAGE_NON_SKILL1);
	DeleteBitmap(IMAGE_NON_SKILL2);
	DeleteBitmap(IMAGE_NON_COMMAND);
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	DeleteBitmap(IMAGE_SKILL3);
	DeleteBitmap(IMAGE_NON_SKILL3);
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
}

bool SEASON3B::CNewUISkillList::UpdateMouseEvent()
{
#ifdef MOD_SKILLLIST_UPDATEMOUSE_BLOCK
	if (GFxProcess::GetInstancePtr()->GetUISelect() == 1)
	{
		return true;
	}
#endif //MOD_SKILLLIST_UPDATEMOUSE_BLOCK

	if (g_isCharacterBuff((&Hero->Object), eBuff_DuelWatch))
	{
		SetSkillPickerOpen(false);
		return true;
	}

	BYTE bySkillNumber = CharacterAttribute->SkillNumber;

	float x, y, width, height, FixX;

	m_bRenderSkillInfo = false;

	if (bySkillNumber <= 0)
	{
		return true;
	}
	if (gProtect.m_MainInfo.IsVersion == 1)
	{
		FixX = 75;
	}
	else
	{
		FixX = 0;
	}
	GetLegacyCurrentSkillSlotRect(x, y, width, height);

#if defined(__ANDROID__) || defined(MU_IOS)
	if (MouseLButtonPush && SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		SetSkillPickerOpen(!m_bSkillList);
		PlayBuffer(SOUND_CLICK01);
		m_EventState = EVENT_NONE;
		return false;
	}
#else
	if (m_EventState == EVENT_NONE && MouseLButtonPush == false
		&& SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		m_EventState = EVENT_BTN_HOVER_CURRENTSKILL;
		return true;
	}
	if (m_EventState == EVENT_BTN_HOVER_CURRENTSKILL && MouseLButtonPush == false
		&& SEASON3B::CheckMouseIn(x, y, width, height) == false)
	{
		m_EventState = EVENT_NONE;
		return true;
	}
	if (m_EventState == EVENT_BTN_HOVER_CURRENTSKILL && (MouseLButtonPush == true || MouseLButtonDBClick == true)
		&& SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		m_EventState = EVENT_BTN_DOWN_CURRENTSKILL;
		return false;
	}
	if (m_EventState == EVENT_BTN_DOWN_CURRENTSKILL)
	{
		if (MouseLButtonPush == false && MouseLButtonDBClick == false)
		{
			if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
			{
				SetSkillPickerOpen(!m_bSkillList);
				PlayBuffer(SOUND_CLICK01);
				m_EventState = EVENT_NONE;
				return false;
			}
			m_EventState = EVENT_NONE;
			return true;
		}

	}

	if (m_EventState == EVENT_BTN_HOVER_CURRENTSKILL)
	{
		m_bRenderSkillInfo = true;
		m_iRenderSkillInfoType = Hero->CurrentSkill;
		m_iRenderSkillInfoPosX = x - 5;
		m_iRenderSkillInfoPosY = y - 15;

		return false;
	}
	else if (m_EventState == EVENT_BTN_DOWN_CURRENTSKILL)
	{
		return false;
	}
#endif
	if (gProtect.m_MainInfo.IsVersion != 1) //ss2
	{
		x = 222.f - FixX + DisplayWinExt; y = 431.f + DisplayHeightExt; width = 32.f * 5.f; height = 38.f;
		if (m_EventState == EVENT_NONE && MouseLButtonPush == false
			&& SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			m_EventState = EVENT_BTN_HOVER_SKILLHOTKEY;
			return true;
		}
		if (m_EventState == EVENT_BTN_HOVER_SKILLHOTKEY && MouseLButtonPush == false
			&& SEASON3B::CheckMouseIn(x, y, width, height) == false)
		{
			m_EventState = EVENT_NONE;
			return true;
		}
		if (m_EventState == EVENT_BTN_HOVER_SKILLHOTKEY && MouseLButtonPush == true
			&& SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			m_EventState = EVENT_BTN_DOWN_SKILLHOTKEY;
			return false;
		}
	}
	x = 190.f - FixX + DisplayWinExt; y = 431.f + DisplayHeightExt; width = 32.f; height = 38.f;
	int iStartIndex = (m_bHotKeySkillListUp == true) ? 6 : 1;
	for (int i = 0, iIndex = iStartIndex; i < 5; ++i, iIndex++)
	{
		x += width;

		if (iIndex == 10)
		{
			iIndex = 0;
		}
		if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			const bool hasAssignedHotKey = (m_iHotKeySkillType[iIndex] != -1);
			const bool isPetCommandHotKey =
				hasAssignedHotKey
				&& m_iHotKeySkillType[iIndex] >= AT_PET_COMMAND_DEFAULT
				&& m_iHotKeySkillType[iIndex] < AT_PET_COMMAND_END;

			if (m_EventState == EVENT_BTN_DOWN_SKILLHOTKEY && MouseLButtonPush == false)
			{
#if defined(__ANDROID__) || defined(MU_IOS)
				if (m_iAndroidTouchAssignSkillIndex >= 0)
				{
					if (iIndex == 0)
					{
						ApplySelectedSkillIndex(m_iAndroidTouchAssignSkillIndex);
					}
					else
					{
						SetHotKey(iIndex, m_iAndroidTouchAssignSkillIndex);
					}
					SetSkillPickerOpen(false);
					m_iAndroidTouchAssignSkillIndex = -1;
					m_EventState = EVENT_NONE;
					PlayBuffer(SOUND_CLICK01);
					return false;
				}
#endif
			}

			if (!hasAssignedHotKey)
			{
				if (m_EventState == EVENT_BTN_HOVER_SKILLHOTKEY)
				{
					m_bRenderSkillInfo = false;
					m_iRenderSkillInfoType = -1;
				}
				if (m_EventState == EVENT_BTN_DOWN_SKILLHOTKEY && MouseLButtonPush == false)
				{
					m_EventState = EVENT_NONE;
				}
				continue;
			}

			WORD bySkillType = isPetCommandHotKey
				? static_cast<WORD>(m_iHotKeySkillType[iIndex])
				: CharacterAttribute->Skill[m_iHotKeySkillType[iIndex]];

			if (!isPetCommandHotKey && (bySkillType == 0 || (bySkillType >= AT_SKILL_STUN && bySkillType <= AT_SKILL_REMOVAL_BUFF)))
				continue;

			BYTE bySkillUseType = isPetCommandHotKey ? 0 : SkillAttribute[bySkillType].SkillUseType;

			if (!isPetCommandHotKey && bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
			{
				continue;
			}

			if (m_EventState == EVENT_BTN_HOVER_SKILLHOTKEY)
			{
				m_bRenderSkillInfo = true;
				m_iRenderSkillInfoType = m_iHotKeySkillType[iIndex];
				m_iRenderSkillInfoPosX = x - 5;
				m_iRenderSkillInfoPosY = y - 15;
				return true;
			}
			if (m_EventState == EVENT_BTN_DOWN_SKILLHOTKEY)
			{
				if (MouseLButtonPush == false)
				{
#if defined(__ANDROID__) || defined(MU_IOS)
					m_EventState = EVENT_NONE;
					if (m_iAndroidTouchAssignSkillIndex < 0)
					{
						AndroidTriggerHotKeySkillTap(m_iHotKeySkillType[iIndex]);
					}
					SetSkillPickerOpen(false);
					PlayBuffer(SOUND_CLICK01);
					return false;
#else
					if (m_iRenderSkillInfoType == m_iHotKeySkillType[iIndex])
					{
						m_EventState = EVENT_NONE;
						m_wHeroPriorSkill = GetCurrentSkillTypeForPrior();
						Hero->CurrentSkill = m_iHotKeySkillType[iIndex];
						PlayBuffer(SOUND_CLICK01);
						return false;
					}
					else
					{
						m_EventState = EVENT_NONE;
					}
#endif
				}
			}
		}
	}

	x = 222.f - FixX + DisplayWinExt; y = 431.f + DisplayHeightExt; width = 32.f * 5.f; height = 38.f;
	if (m_EventState == EVENT_BTN_DOWN_SKILLHOTKEY)
	{
		if (MouseLButtonPush == false && SEASON3B::CheckMouseIn(x, y, width, height) == false)
		{
			m_EventState = EVENT_NONE;
			return true;
		}
		return false;
	}

	if (m_bSkillList == false)
		return true;

	int iSkillCount = 0;
	bool bMouseOnSkillList = false;

	EVENT_STATE PrevEventState = m_EventState;

	for (int i = 0; i < MAX_MAGIC; ++i)
	{
		if (!LegacyPickerIncludeMagicArrayIndex(i))
		{
			continue;
		}

		GetLegacySkillPickerCellRect(iSkillCount, x, y, width, height);
#if defined(__ANDROID__) || defined(MU_IOS)
		x += m_legacySkillPickerOffsetX;
		y += m_virtualSkillPickerOffsetY;
#endif
		iSkillCount++;

		if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			bMouseOnSkillList = true;
			if (m_EventState == EVENT_NONE && MouseLButtonPush == false)
			{
				m_EventState = EVENT_BTN_HOVER_SKILLLIST;
				m_bRenderSkillInfo = true;
				m_iRenderSkillInfoType = i;
#if defined(__ANDROID__)
				TakumiLogSkillPickerHoverCell(i);
#endif
				m_iRenderSkillInfoPosX = x;
				m_iRenderSkillInfoPosY = y - 15;
				break;
			}
		}

		if (m_EventState == EVENT_BTN_HOVER_SKILLLIST && MouseLButtonPush == true
			&& SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			m_EventState = EVENT_BTN_DOWN_SKILLLIST;
			break;
		}

		if (m_EventState == EVENT_BTN_HOVER_SKILLLIST && MouseLButtonPush == false
			&& SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			m_bRenderSkillInfo = true;
			m_iRenderSkillInfoType = i;
#if defined(__ANDROID__)
			TakumiLogSkillPickerHoverCell(i);
#endif
			m_iRenderSkillInfoPosX = x;
			m_iRenderSkillInfoPosY = y - 15;
		}

		if (m_EventState == EVENT_BTN_DOWN_SKILLLIST && MouseLButtonPush == false
			&& m_iRenderSkillInfoType == i && SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			m_EventState = EVENT_NONE;
#if defined(__ANDROID__) || defined(MU_IOS)
			FinalizeMagicSlotSelectionFromLegacyPicker(i);
#else
			ApplySelectedSkillIndex(i);
			SetSkillPickerOpen(false);
			PlayBuffer(SOUND_CLICK01);
#endif
			return false;
		}
	}

	if (PrevEventState != m_EventState)
	{
		if (m_EventState == EVENT_NONE || m_EventState == EVENT_BTN_HOVER_SKILLLIST)
			return true;
		return false;
	}

	if (Hero != NULL && Hero->m_pPet != NULL)
	{
		int petCmd = AT_PET_COMMAND_DEFAULT;
		if (HitLegacyPickerPetCommandRow(static_cast<float>(MouseX), static_cast<float>(MouseY), &petCmd))
		{
			float px = 0.f;
			float py = 0.f;
			float pw = 0.f;
			float ph = 0.f;
			if (GetLegacyPickerPetCommandCellRect(petCmd, px, py, pw, ph))
			{
				bMouseOnSkillList = true;

				if (m_EventState == EVENT_NONE && MouseLButtonPush == false)
				{
					m_EventState = EVENT_BTN_HOVER_SKILLLIST;
					m_bRenderSkillInfo = true;
					m_iRenderSkillInfoType = petCmd;
#if defined(__ANDROID__)
					TakumiLogSkillPickerHoverCell(petCmd);
#endif
					m_iRenderSkillInfoPosX = px;
					m_iRenderSkillInfoPosY = py - 15;
					return true;
				}
				if (m_EventState == EVENT_BTN_HOVER_SKILLLIST && MouseLButtonPush == true)
				{
					m_EventState = EVENT_BTN_DOWN_SKILLLIST;
					return false;
				}

				if (m_EventState == EVENT_BTN_HOVER_SKILLLIST)
				{
					m_bRenderSkillInfo = true;
					m_iRenderSkillInfoType = petCmd;
#if defined(__ANDROID__)
					TakumiLogSkillPickerHoverCell(petCmd);
#endif
					m_iRenderSkillInfoPosX = px;
					m_iRenderSkillInfoPosY = py - 15;
				}
				if (m_EventState == EVENT_BTN_DOWN_SKILLLIST && MouseLButtonPush == false
					&& m_iRenderSkillInfoType == petCmd)
				{
					m_EventState = EVENT_NONE;
#if defined(__ANDROID__) || defined(MU_IOS)
					FinalizeMagicSlotSelectionFromLegacyPicker(petCmd);
#else
					ApplySelectedSkillIndex(petCmd);
					SetSkillPickerOpen(false);
					PlayBuffer(SOUND_CLICK01);
#endif
					return false;
				}
			}
		}
	}

	if (bMouseOnSkillList == false && m_EventState == EVENT_BTN_HOVER_SKILLLIST)
	{
		m_EventState = EVENT_NONE;
		return true;
	}
	if (bMouseOnSkillList == false && MouseLButtonPush == false
		&& m_EventState == EVENT_BTN_DOWN_SKILLLIST)
	{
		m_EventState = EVENT_NONE;
		return false;
	}
	if (m_EventState == EVENT_BTN_DOWN_SKILLLIST)
	{
		if (MouseLButtonPush == false)
		{
			m_EventState = EVENT_NONE;
			return true;
		}
		return false;
	}

	return true;
}

bool SEASON3B::CNewUISkillList::UpdateKeyEvent()
{
#if defined(__ANDROID__) || defined(MU_IOS)
	if (m_bSkillList == true && m_iAndroidTouchAssignSkillIndex >= 0)
	{
		for (int i = 0; i < 9; ++i)
		{
			if (SEASON3B::IsPress('1' + i))
			{
				SetHotKey(i + 1, m_iAndroidTouchAssignSkillIndex);
				ApplySelectedSkillIndex(m_iAndroidTouchAssignSkillIndex);
				SetSkillPickerOpen(false);
				m_iAndroidTouchAssignSkillIndex = -1;
				PlayBuffer(SOUND_CLICK01);
				return false;
			}
		}

		if (SEASON3B::IsPress('0'))
		{
			SetHotKey(0, m_iAndroidTouchAssignSkillIndex);
			ApplySelectedSkillIndex(m_iAndroidTouchAssignSkillIndex);
			SetSkillPickerOpen(false);
			m_iAndroidTouchAssignSkillIndex = -1;
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}
#endif

	for (int i = 0; i < 9; ++i)
	{
		if (SEASON3B::IsPress('1' + i))
		{
			UseHotKey(i + 1);
		}
	}

	if (SEASON3B::IsPress('0'))
	{
		UseHotKey(0);
	}

	if (m_EventState == EVENT_BTN_HOVER_SKILLLIST)
	{
		if (SEASON3B::IsRepeat(VK_CONTROL))
		{
			for (int i = 0; i < 9; ++i)
			{
				if (SEASON3B::IsPress('1' + i))
				{
					SetHotKey(i + 1, m_iRenderSkillInfoType);
					return false;
				}
			}

			if (SEASON3B::IsPress('0'))
			{
				SetHotKey(0, m_iRenderSkillInfoType);
				ApplySelectedSkillIndex(m_iRenderSkillInfoType);
				return false;
			}
		}
	}

	if (SEASON3B::IsRepeat(VK_SHIFT))
	{
		for (int i = 0; i < 4; ++i)
		{
			if (SEASON3B::IsPress('1' + i))
			{
				Hero->CurrentSkill = AT_PET_COMMAND_DEFAULT + i;
				return false;
			}
		}
	}

	return true;
}

bool SEASON3B::CNewUISkillList::IsArrayUp(BYTE bySkill)
{
	for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
	{
		if (m_iHotKeySkillType[i] == bySkill)
		{
			if (i == 0 || i > 5)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	return false;
}

bool SEASON3B::CNewUISkillList::IsArrayIn(BYTE bySkill)
{
	for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
	{
		if (m_iHotKeySkillType[i] == bySkill)
		{
			return true;
		}
	}

	return false;
}

void SEASON3B::CNewUISkillList::SetHotKey(int iHotKey, int iSkillType)
{
	for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
	{
		if (m_iHotKeySkillType[i] == iSkillType)
		{
			m_iHotKeySkillType[i] = -1;
			break;
		}
	}

	const int previousHotKey = m_iHotKeySkillType[iHotKey];
	m_iHotKeySkillType[iHotKey] = iSkillType;

	if (iHotKey == 0 && iSkillType >= 0 && Hero != NULL)
	{
		Hero->CurrentSkill = static_cast<BYTE>(iSkillType);
	}

	if (previousHotKey != iSkillType
		&& !TakumiIsApplyingServerSkillOptions()
		&& Hero != NULL
		&& CharacterAttribute != NULL)
	{
		SaveOptions();
	}
}

int SEASON3B::CNewUISkillList::GetHotKey(int iHotKey)
{
	return m_iHotKeySkillType[iHotKey];
}

int SEASON3B::CNewUISkillList::GetSkillIndex(int iSkillType)
{
	int iReturn = -1;
	for (int i = 0; i < MAX_MAGIC; ++i)
	{
		if (CharacterAttribute->Skill[i] == iSkillType)
		{
			iReturn = i;
			break;
		}
	}

	return iReturn;
}

void SEASON3B::CNewUISkillList::RenderSkillIconPreview(int iSkillIndex, float x, float y, float width, float height)
{
	if (iSkillIndex < 0)
	{
		return;
	}

	if (iSkillIndex >= AT_PET_COMMAND_DEFAULT && iSkillIndex < AT_PET_COMMAND_END)
	{
		RenderSkillIcon(iSkillIndex, x, y, width, height);
		return;
	}

	if (iSkillIndex >= MAX_MAGIC)
	{
		return;
	}

	if (CharacterAttribute == NULL || CharacterAttribute->Skill[iSkillIndex] <= 0)
	{
		return;
	}

	const int oldDelay = CharacterAttribute->SkillDelay[iSkillIndex];
	CharacterAttribute->SkillDelay[iSkillIndex] = 0;
	RenderSkillIcon(iSkillIndex, x, y, width, height);
	CharacterAttribute->SkillDelay[iSkillIndex] = oldDelay;
}

void SEASON3B::CNewUISkillList::RenderSkillIconCircularPreview(
	int iSkillIndex,
	float centerX,
	float centerY,
	float radiusUi)
{
	if (iSkillIndex < 0)
	{
		return;
	}

	if (iSkillIndex >= AT_PET_COMMAND_DEFAULT && iSkillIndex < AT_PET_COMMAND_END)
	{
		const float diameter = radiusUi * 2.f;
		RenderSkillIcon(iSkillIndex, centerX - radiusUi, centerY - radiusUi, diameter, diameter);
		return;
	}

	if (iSkillIndex >= MAX_MAGIC)
	{
		return;
	}

	if (CharacterAttribute == NULL || CharacterAttribute->Skill[iSkillIndex] <= 0)
	{
		return;
	}

	const int oldDelay = CharacterAttribute->SkillDelay[iSkillIndex];
	CharacterAttribute->SkillDelay[iSkillIndex] = 0;
	RenderSkillIcon(
		iSkillIndex,
		centerX,
		centerY,
		20.f,
		28.f,
		0,
		-1,
		true,
		radiusUi);
	CharacterAttribute->SkillDelay[iSkillIndex] = oldDelay;
}

bool SEASON3B::CNewUISkillList::IsSkillPickerOpen() const
{
	return m_bSkillList;
}

void SEASON3B::CNewUISkillList::SetSkillPickerOpen(bool open)
{
	m_bSkillList = open;
	m_EventState = EVENT_NONE;
	if (open == true)
	{
		m_iAndroidTouchAssignSkillIndex = -1;
	}
	else
	{
#if defined(__ANDROID__) || defined(MU_IOS)
		m_legacyPickerTargetHotKey = -1;
		m_legacySkillPickerOffsetX = 0.f;
		m_virtualSkillPickerOffsetY = 0.f;
		m_virtualPickerAnchorCx = 0.f;
		m_virtualPickerAnchorCy = 0.f;
		m_virtualPickerTargetVisualSlot = -1;
		ClearAndroidTouchSkillTooltip();
#endif
	}
#if TAKUMI_ANDROID_UI_SKILL_PICKER_CACHE
	InvalidateSkillPickerLayout();
#endif
}

#if TAKUMI_ANDROID_UI_SKILL_PICKER_CACHE
void SEASON3B::CNewUISkillList::OnMagicListUpdated()
{
	InvalidateSkillPickerLayout();
}

void SEASON3B::CNewUISkillList::InvalidateSkillPickerLayout()
{
	m_skillPickerLayoutDirty = true;
}

uint64_t SEASON3B::CNewUISkillList::ComputeSkillPickerLayoutSig() const
{
	if (CharacterAttribute == NULL)
	{
		return 0;
	}

	uint64_t sig = 14695981039346656037ull;
	auto mix = [&sig](uint64_t v)
	{
		sig = (sig ^ v) * 1099511628211ull;
	};

	mix((uint64_t)(int32_t)(DisplayWinExt * 1000.0f));
	mix((uint64_t)(int32_t)(DisplayHeightExt * 1000.0f));
#if defined(__ANDROID__) || defined(MU_IOS)
	mix((uint64_t)(int32_t)(m_legacySkillPickerOffsetX * 1000.0f));
	mix((uint64_t)(int32_t)(m_virtualSkillPickerOffsetY * 1000.0f));
	mix((uint64_t)(int32_t)(m_virtualPickerAnchorCx * 1000.0f));
	mix((uint64_t)(int32_t)(m_virtualPickerAnchorCy * 1000.0f));
	mix((uint64_t)m_virtualPickerTargetVisualSlot);
#endif

	for (int i = 0; i < MAX_MAGIC; ++i)
	{
		if (!LegacyPickerIncludeMagicArrayIndex(i))
		{
			continue;
		}
		mix((uint64_t)(uint32_t)i);
		mix((uint64_t)(uint32_t)CharacterAttribute->Skill[i]);
	}

	return sig;
}

void SEASON3B::CNewUISkillList::RebuildSkillPickerLayout()
{
	m_skillPickerLayout.clear();

	if (CharacterAttribute == NULL || CharacterAttribute->SkillNumber <= 0)
	{
		m_skillPickerLayoutDirty = false;
		return;
	}

	const float width = 32.f;
	const float height = 38.f;

#if defined(__ANDROID__) || defined(MU_IOS)
	// Modern HUD: compact grid above the tapped secondary skill button (legacy fan grid is off-screen).
	if (m_virtualPickerTargetVisualSlot >= 0)
	{
		const int columns = 4;
		const float gridW = columns * width;
		float startX = m_virtualPickerAnchorCx - gridW * 0.5f;
		float startY = m_virtualPickerAnchorCy - height * 4.f - 28.f;
		const float maxStartX = 640.f - gridW - 4.f;
		if (startX < 4.f)
		{
			startX = 4.f;
		}
		else if (startX > maxStartX)
		{
			startX = maxStartX;
		}
		if (startY < 40.f)
		{
			startY = 40.f;
		}

		int col = 0;
		int row = 0;
		for (int i = 0; i < MAX_MAGIC; ++i)
		{
			if (!LegacyPickerIncludeMagicArrayIndex(i))
			{
				continue;
			}

			SkillPickerLayoutEntry entry;
			entry.slotIndex = i;
			entry.x = startX + col * width;
			entry.y = startY + row * height;
			m_skillPickerLayout.push_back(entry);

			++col;
			if (col >= columns)
			{
				col = 0;
				++row;
			}
		}

		m_skillPickerLayoutDirty = false;
		return;
	}
#endif

	float FixX = (gProtect.m_MainInfo.IsVersion == 1) ? 75.f : 0.f;
	const float fOrigX = 385.f - FixX + DisplayWinExt;
	const float yBase = 390.f + DisplayHeightExt;
	int iSkillCount = 0;

	for (int i = 0; i < MAX_MAGIC; ++i)
	{
		if (!LegacyPickerIncludeMagicArrayIndex(i))
		{
			continue;
		}

		float x = fOrigX;
		float y = yBase;
		if (iSkillCount >= 18)
		{
			y -= height;
		}

		if (iSkillCount < 14)
		{
			const int iRemainder = iSkillCount % 2;
			const int iQuotient = iSkillCount / 2;

			if (iRemainder == 0)
			{
				x = fOrigX + iQuotient * width;
			}
			else
			{
				x = fOrigX - (iQuotient + 1) * width;
			}
		}
		else if (iSkillCount >= 14 && iSkillCount < 18)
		{
			x = fOrigX - (8 * width) - ((iSkillCount - 14) * width);
		}
		else
		{
			x = fOrigX - (12 * width) + ((iSkillCount - 17) * width);
		}

		SkillPickerLayoutEntry entry;
		entry.slotIndex = i;
#if defined(__ANDROID__) || defined(MU_IOS)
		entry.x = x + m_legacySkillPickerOffsetX;
		entry.y = y + m_virtualSkillPickerOffsetY;
#else
		entry.x = x;
		entry.y = y;
#endif
		m_skillPickerLayout.push_back(entry);
		++iSkillCount;
	}

	m_skillPickerLayoutDirty = false;
}
#endif

void SEASON3B::CNewUISkillList::ToggleSkillPicker()
{
	SetSkillPickerOpen(!m_bSkillList);
}

int SEASON3B::CNewUISkillList::GetHoveredSkillIndex() const
{
	return m_bRenderSkillInfo ? m_iRenderSkillInfoType : -1;
}

int SEASON3B::CNewUISkillList::GetAndroidTouchAssignSkillIndex() const
{
	return m_iAndroidTouchAssignSkillIndex;
}

void SEASON3B::CNewUISkillList::SetAndroidTouchAssignSkillIndex(int skillIndex)
{
	m_iAndroidTouchAssignSkillIndex = skillIndex;
}

#if defined(__ANDROID__) || defined(MU_IOS)
int SEASON3B::CNewUISkillList::HitTestLegacyMobileSkillHotKey(float uiX, float uiY) const
{
	if (CharacterAttribute == NULL || CharacterAttribute->SkillNumber <= 0)
	{
		return -1;
	}

	float x = 0.0f;
	float y = 0.0f;
	float width = 32.0f;
	float height = 38.0f;
	if (gProtect.m_MainInfo.IsVersion == 1)
	{
		x = 310.0f + DisplayWinExt;
		y = 447.0f + DisplayHeightExt;
		width = 20.0f;
		height = 28.0f;
	}
	else
	{
		x = 190.0f + DisplayWinExt;
		y = 431.0f + DisplayHeightExt;
	}

	int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
	for (int i = 0; i < 5; ++i)
	{
		x += width;
		int iIndex = iStartSkillIndex + i;
		if (iIndex == 10)
		{
			iIndex = 0;
		}

		if (uiX >= x && uiX <= (x + width) && uiY >= y && uiY <= (y + height))
		{
			return iIndex;
		}
	}

	return -1;
}

void SEASON3B::CNewUISkillList::RenderAndroidLegacySkillHotBar()
{
	if (CharacterAttribute == NULL || CharacterAttribute->SkillNumber <= 0)
	{
		return;
	}

	float x = 0.0f;
	float y = 0.0f;
	float width = 32.0f;
	float height = 38.0f;
	if (gProtect.m_MainInfo.IsVersion == 1)
	{
		x = 310.0f + DisplayWinExt;
		y = 447.0f + DisplayHeightExt;
		width = 20.0f;
		height = 28.0f;
	}
	else
	{
		x = 190.0f + DisplayWinExt;
		y = 431.0f + DisplayHeightExt;
	}

	int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
	for (int i = 0; i < 5; ++i)
	{
		x += width;
		int iIndex = iStartSkillIndex + i;
		if (iIndex == 10)
		{
			iIndex = 0;
		}

		if (m_iHotKeySkillType[iIndex] == -1)
		{
			continue;
		}

		if (m_iHotKeySkillType[iIndex] >= AT_PET_COMMAND_DEFAULT
			&& m_iHotKeySkillType[iIndex] < AT_PET_COMMAND_END)
		{
			if (Hero == NULL || Hero->m_pPet == NULL)
			{
				continue;
			}
		}

		if (Hero != NULL && Hero->CurrentSkill == m_iHotKeySkillType[iIndex])
		{
			SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
		}
		else
		{
			SEASON3B::RenderImage(IMAGE_SKILLBOX, x, y, width, height);
		}

		RenderSkillIcon(m_iHotKeySkillType[iIndex], x + 6.0f, y + 6.0f, 20.0f, 28.0f, 0, iIndex);
	}
}

bool SEASON3B::CNewUISkillList::GetLegacyMobileSkillHotBarSlotRect(
	int hotKeyUiIndex,
	float& outX,
	float& outY,
	float& outW,
	float& outH) const
{
	if (CharacterAttribute == NULL || CharacterAttribute->SkillNumber <= 0)
	{
		return false;
	}

	float x = 0.0f;
	float y = 0.0f;
	float width = 32.0f;
	float height = 38.0f;
	if (gProtect.m_MainInfo.IsVersion == 1)
	{
		x = 310.0f + DisplayWinExt;
		y = 447.0f + DisplayHeightExt;
		width = 20.0f;
		height = 28.0f;
	}
	else
	{
		x = 190.0f + DisplayWinExt;
		y = 431.0f + DisplayHeightExt;
	}

	const int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
	for (int i = 0; i < 5; ++i)
	{
		x += width;
		int idx = iStartSkillIndex + i;
		if (idx == 10)
		{
			idx = 0;
		}

		if (idx == hotKeyUiIndex)
		{
			outX = x;
			outY = y;
			outW = width;
			outH = height;
			return true;
		}
	}

	return false;
}

bool SEASON3B::CNewUISkillList::TryOpenLegacyMobileSkillAssignPickerForHotKey(int hotKeyUiIndex)
{
	if (CharacterAttribute == NULL || CharacterAttribute->SkillNumber <= 0)
	{
		return false;
	}

	if (hotKeyUiIndex < 0 || hotKeyUiIndex >= SKILLHOTKEY_COUNT)
	{
		return false;
	}

	float sx = 0.f;
	float sy = 0.f;
	float sw = 0.f;
	float sh = 0.f;
	if (!GetLegacyMobileSkillHotBarSlotRect(hotKeyUiIndex, sx, sy, sw, sh))
	{
		return false;
	}

	float fixX = (gProtect.m_MainInfo.IsVersion == 1) ? 75.f : 0.f;
	const float fOrigX = 385.f - fixX + DisplayWinExt;
	const float skillBoxHalf = 16.f;
	m_legacySkillPickerOffsetX = (sx + sw * 0.5f) - (fOrigX + skillBoxHalf);
	m_legacyPickerTargetHotKey = hotKeyUiIndex;

	SetSkillPickerOpen(true);
	PlayBuffer(SOUND_CLICK01);
	return true;
}

bool SEASON3B::CNewUISkillList::TryOpenVirtualMobileSkillAssignPicker(int visualSlot, float anchorCx, float anchorCy)
{
	if (CharacterAttribute == NULL || CharacterAttribute->SkillNumber <= 0)
	{
		return false;
	}

	if (visualSlot < 0 || visualSlot >= 3)
	{
		return false;
	}

	m_legacyPickerTargetHotKey = -1;
	m_virtualPickerTargetVisualSlot = visualSlot;
	m_virtualPickerAnchorCx = anchorCx;
	m_virtualPickerAnchorCy = anchorCy;
	m_legacySkillPickerOffsetX = 0.f;
	m_virtualSkillPickerOffsetY = 0.f;

	SetAndroidTouchAssignSkillIndex(-1);
	SetSkillPickerOpen(true);
	PlayBuffer(SOUND_CLICK01);
	return true;
}

int SEASON3B::CNewUISkillList::GetVirtualPickerTargetVisualSlot() const
{
	return m_virtualPickerTargetVisualSlot;
}

void SEASON3B::CNewUISkillList::ClearVirtualPickerTargetVisualSlot()
{
	m_virtualPickerTargetVisualSlot = -1;
	m_virtualPickerAnchorCx = 0.f;
	m_virtualPickerAnchorCy = 0.f;
}

bool SEASON3B::CNewUISkillList::HitTestAndroidSkillPickerPanel(float uiX, float uiY) const
{
	if (!m_bSkillList || CharacterAttribute == NULL)
	{
		return false;
	}

#if TAKUMI_ANDROID_UI_SKILL_PICKER_CACHE
	const uint64_t layoutSig = const_cast<CNewUISkillList*>(this)->ComputeSkillPickerLayoutSig();
	if (layoutSig != m_skillPickerLayoutSig)
	{
		const_cast<CNewUISkillList*>(this)->m_skillPickerLayoutSig = layoutSig;
		const_cast<CNewUISkillList*>(this)->m_skillPickerLayoutDirty = true;
	}
	if (const_cast<CNewUISkillList*>(this)->m_skillPickerLayoutDirty)
	{
		const_cast<CNewUISkillList*>(this)->RebuildSkillPickerLayout();
	}

	const float width = 32.f;
	const float height = 38.f;
	for (size_t layoutIdx = 0; layoutIdx < m_skillPickerLayout.size(); ++layoutIdx)
	{
		const SkillPickerLayoutEntry& entry = m_skillPickerLayout[layoutIdx];
		if (uiX >= entry.x && uiX <= (entry.x + width)
			&& uiY >= entry.y && uiY <= (entry.y + height))
		{
			return true;
		}
	}
#else
	int iSkillCount = 0;
	float x = 0.f;
	float y = 0.f;
	float width = 0.f;
	float height = 0.f;
	for (int i = 0; i < MAX_MAGIC; ++i)
	{
		if (!LegacyPickerIncludeMagicArrayIndex(i))
		{
			continue;
		}

		GetLegacySkillPickerCellRect(iSkillCount, x, y, width, height);
		x += m_legacySkillPickerOffsetX;
		y += m_virtualSkillPickerOffsetY;
		++iSkillCount;
		if (uiX >= x && uiX <= (x + width) && uiY >= y && uiY <= (y + height))
		{
			return true;
		}
	}
#endif

	if (Hero != NULL && Hero->m_pPet != NULL)
	{
		int petCmd = AT_PET_COMMAND_DEFAULT;
		if (HitLegacyPickerPetCommandRow(uiX, uiY, &petCmd))
		{
			return true;
		}
	}

	return false;
}

bool SEASON3B::CNewUISkillList::TryGetAndroidSkillPickerCellPos(int magicSlot, float& outX, float& outY) const
{
	if (magicSlot < 0)
	{
		return false;
	}

#if TAKUMI_ANDROID_UI_SKILL_PICKER_CACHE
	for (size_t layoutIdx = 0; layoutIdx < m_skillPickerLayout.size(); ++layoutIdx)
	{
		const SkillPickerLayoutEntry& entry = m_skillPickerLayout[layoutIdx];
		if (entry.slotIndex == magicSlot)
		{
			outX = entry.x + 16.f;
			outY = entry.y + 19.f;
			return true;
		}
	}
#endif

	int iSkillCount = 0;
	float x = 0.f;
	float y = 0.f;
	float width = 0.f;
	float height = 0.f;
	for (int i = 0; i < MAX_MAGIC; ++i)
	{
		if (!LegacyPickerIncludeMagicArrayIndex(i))
		{
			continue;
		}

		GetLegacySkillPickerCellRect(iSkillCount, x, y, width, height);
#if defined(__ANDROID__) || defined(MU_IOS)
		x += m_legacySkillPickerOffsetX;
		y += m_virtualSkillPickerOffsetY;
#endif
		++iSkillCount;
		if (i == magicSlot)
		{
			outX = x + width * 0.5f;
			outY = y + height * 0.5f;
			return true;
		}
	}

	if (magicSlot >= AT_PET_COMMAND_DEFAULT && magicSlot < AT_PET_COMMAND_END)
	{
		float px = 0.f;
		float py = 0.f;
		float pw = 0.f;
		float ph = 0.f;
		if (GetLegacyPickerPetCommandCellRect(magicSlot, px, py, pw, ph))
		{
			outX = px + pw * 0.5f;
			outY = py + ph * 0.5f;
			return true;
		}
	}

	return false;
}

void SEASON3B::CNewUISkillList::UpdateAndroidTouchSkillTooltip(float uiX, float uiY)
{
	if (!m_bSkillList || CharacterAttribute == NULL)
	{
		ClearAndroidTouchSkillTooltip();
		return;
	}

	int hit = HitTestAndroidTouchSkillPicker(uiX, uiY);
	if (hit < 0 && Hero != NULL && Hero->m_pPet != NULL)
	{
		int petCmd = AT_PET_COMMAND_DEFAULT;
		if (HitLegacyPickerPetCommandRow(uiX, uiY, &petCmd))
		{
			hit = petCmd;
		}
	}

	if (!IsRenderableSkillSlotIndex(hit))
	{
		ClearAndroidTouchSkillTooltip();
		return;
	}

	float anchorX = uiX;
	float anchorY = uiY;
	if (!TryGetAndroidSkillPickerCellPos(hit, anchorX, anchorY))
	{
		anchorX = uiX;
		anchorY = uiY;
	}

	m_bRenderSkillInfo = true;
	m_iRenderSkillInfoType = hit;
	m_iRenderSkillInfoPosX = anchorX;
	m_iRenderSkillInfoPosY = anchorY - 15.f;
#if defined(__ANDROID__)
	TakumiLogSkillPickerHoverCell(hit);
#endif
}

void SEASON3B::CNewUISkillList::UpdateAndroidVirtualSlotSkillTooltip(
	int magicSlotIndex,
	float anchorCx,
	float anchorCy)
{
	if (!IsRenderableSkillSlotIndex(magicSlotIndex))
	{
		ClearAndroidTouchSkillTooltip();
		return;
	}

	m_bRenderSkillInfo = true;
	m_iRenderSkillInfoType = magicSlotIndex;
	m_iRenderSkillInfoPosX = anchorCx;
	m_iRenderSkillInfoPosY = anchorCy - 15.f;
}

void SEASON3B::CNewUISkillList::ClearAndroidTouchSkillTooltip()
{
	m_bRenderSkillInfo = false;
	m_iRenderSkillInfoType = -1;
}

void SEASON3B::CNewUISkillList::AssignVirtualMobileSkillFromPicker(int pickedMagicSlot)
{
	FinalizeMagicSlotSelectionFromLegacyPicker(pickedMagicSlot);
}

void SEASON3B::CNewUISkillList::RenderAndroidSkillPickerBackdrop() const
{
	if (!m_bSkillList || !MU_MobileIsModernMobileHudEnabled() || MU_MobileIsLegacyMainHudEnabled())
	{
		return;
	}

	// Use standard alpha (not EnableAlphaBlend's GL_ONE,GL_ONE additive) or the world washes out white.
	EnableAlphaTest();
	RenderColor(0.f, 0.f, 640.f, 480.f, 0.55f, 1);
	EndRenderColor();
	glColor4f(1.f, 1.f, 1.f, 1.f);
}

bool SEASON3B::CNewUISkillList::TryOpenLegacyMobileSkillAssignPicker(float uiX, float uiY)
{
	if (CharacterAttribute == NULL || CharacterAttribute->SkillNumber <= 0)
	{
		return false;
	}

	const int hk = HitTestLegacyMobileSkillHotKey(uiX, uiY);
	if (hk < 0)
	{
		return false;
	}

	return TryOpenLegacyMobileSkillAssignPickerForHotKey(hk);
}

void SEASON3B::CNewUISkillList::FinalizeMagicSlotSelectionFromLegacyPicker(int pickedSlotIdx)
{
	if (!IsRenderableSkillSlotIndex(pickedSlotIdx))
	{
		return;
	}

	if (m_virtualPickerTargetVisualSlot >= 0 && m_virtualPickerTargetVisualSlot < 3)
	{
		TakumiAndroidAssignVirtualSkillSlot(m_virtualPickerTargetVisualSlot + 1, pickedSlotIdx);
		m_virtualPickerTargetVisualSlot = -1;
		m_legacySkillPickerOffsetX = 0.f;
		m_virtualSkillPickerOffsetY = 0.f;
		SetSkillPickerOpen(false);
		m_iAndroidTouchAssignSkillIndex = -1;
		PlayBuffer(SOUND_CLICK01);
		return;
	}

#if defined(TAKUMI_SKILL_PICKER_VERBOSE)
	{
		const int hkSnap = (m_legacyPickerTargetHotKey >= 0 && m_legacyPickerTargetHotKey < SKILLHOTKEY_COUNT)
			? m_legacyPickerTargetHotKey : -1;
		if (pickedSlotIdx >= AT_PET_COMMAND_DEFAULT && pickedSlotIdx < AT_PET_COMMAND_END)
		{
			fprintf(stderr, "[TakumiSkillPicker] finalize petCmd=%d targetHotKey=%d\n", pickedSlotIdx, hkSnap);
		}
		else if (CharacterAttribute != NULL && pickedSlotIdx >= 0 && pickedSlotIdx < MAX_MAGIC)
		{
			fprintf(
				stderr,
				"[TakumiSkillPicker] finalize magicSlot=%d skillType=%u targetHotKey=%d\n",
				pickedSlotIdx,
				static_cast<unsigned>(CharacterAttribute->Skill[pickedSlotIdx]),
				hkSnap);
		}
	}
#endif // TAKUMI_SKILL_PICKER_VERBOSE

	const bool hasTargetHotKey =
		m_legacyPickerTargetHotKey >= 0 && m_legacyPickerTargetHotKey < SKILLHOTKEY_COUNT;
	if (hasTargetHotKey)
	{
		SetHeroPriorSkill(GetCurrentSkillTypeForPrior());
		SetHotKey(m_legacyPickerTargetHotKey, pickedSlotIdx);
		if (Hero != NULL)
		{
			Hero->CurrentSkill = static_cast<BYTE>(pickedSlotIdx);
		}
		m_legacyPickerTargetHotKey = -1;
		m_legacySkillPickerOffsetX = 0.f;
	}
	else
	{
		ApplySelectedSkillIndex(pickedSlotIdx);
		m_iAndroidTouchAssignSkillIndex = pickedSlotIdx;
	}

	SetSkillPickerOpen(false);
	PlayBuffer(SOUND_CLICK01);
}
#endif

bool SEASON3B::CNewUISkillList::TryToggleSkillPickerAtTouch(float uiX, float uiY)
{
#if !defined(__ANDROID__) && !defined(MU_IOS)
	return false;
#else
	if (CharacterAttribute == NULL || CharacterAttribute->SkillNumber <= 0)
	{
		return false;
	}

	float x = 0.f;
	float y = 0.f;
	float width = 0.f;
	float height = 0.f;
	GetLegacyCurrentSkillSlotRect(x, y, width, height);

	if (uiX < x || uiX >(x + width) || uiY < y || uiY >(y + height))
	{
		return false;
	}

	if (!m_bSkillList)
	{
		m_legacyPickerTargetHotKey = -1;
		m_legacySkillPickerOffsetX = 0.f;
	}

	SetSkillPickerOpen(!m_bSkillList);
	PlayBuffer(SOUND_CLICK01);
	return true;
#endif
}

int SEASON3B::CNewUISkillList::HitTestAndroidTouchSkillPicker(float uiX, float uiY)
{
#if !defined(__ANDROID__) && !defined(MU_IOS)
	return -1;
#else
	if (m_bSkillList == false || CharacterAttribute == NULL)
	{
		return -1;
	}

#if TAKUMI_ANDROID_UI_SKILL_PICKER_CACHE
	const uint64_t layoutSig = ComputeSkillPickerLayoutSig();
	if (layoutSig != m_skillPickerLayoutSig)
	{
		m_skillPickerLayoutSig = layoutSig;
		m_skillPickerLayoutDirty = true;
	}
	if (m_skillPickerLayoutDirty)
	{
		RebuildSkillPickerLayout();
	}
	for (size_t layoutIdx = 0; layoutIdx < m_skillPickerLayout.size(); ++layoutIdx)
	{
		const SkillPickerLayoutEntry& entry = m_skillPickerLayout[layoutIdx];
		const float x = entry.x;
		const float y = entry.y;
		const float width = 32.f;
		const float height = 38.f;
		if (uiX >= x && uiX <= (x + width) && uiY >= y && uiY <= (y + height))
		{
			return entry.slotIndex;
		}
	}
#else
	int iSkillCount = 0;
	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;

	for (int i = 0; i < MAX_MAGIC; ++i)
	{
		if (!LegacyPickerIncludeMagicArrayIndex(i))
		{
			continue;
		}

		GetLegacySkillPickerCellRect(iSkillCount, x, y, width, height);
		x += m_legacySkillPickerOffsetX;
		y += m_virtualSkillPickerOffsetY;
		iSkillCount++;

		if (uiX >= x && uiX <= (x + width)
			&& uiY >= y && uiY <= (y + height))
		{
			return i;
		}
	}
#endif

	int petCmd = 0;
	if (HitLegacyPickerPetCommandRow(uiX, uiY, &petCmd))
	{
		return petCmd;
	}

	return -1;
#endif
}

void SEASON3B::CNewUISkillList::UseHotKey(int iHotKey)
{
	if (m_iHotKeySkillType[iHotKey] != -1)
	{
		const int iHotKeySkillIndex = m_iHotKeySkillType[iHotKey];

		if (iHotKeySkillIndex >= AT_PET_COMMAND_DEFAULT && iHotKeySkillIndex < AT_PET_COMMAND_END)
		{
			if (Hero->m_pPet == NULL)
			{
				return;
			}

			m_wHeroPriorSkill = GetCurrentSkillTypeForPrior();
			Hero->CurrentSkill = iHotKeySkillIndex;
			return;
		}

		if (CharacterAttribute == NULL || iHotKeySkillIndex < 0 || iHotKeySkillIndex >= MAX_MAGIC)
		{
			return;
		}

		WORD wHotKeySkill = CharacterAttribute->Skill[iHotKeySkillIndex];

		if (wHotKeySkill == 0)
		{
			return;
		}

		m_wHeroPriorSkill = GetCurrentSkillTypeForPrior();

		Hero->CurrentSkill = iHotKeySkillIndex;

		WORD bySkill = CharacterAttribute->Skill[Hero->CurrentSkill];

		if (
			g_pOption->IsAutoAttack() == true
			&& gMapManager.WorldActive != WD_6STADIUM
			&& gMapManager.InChaosCastle() == false
			&& (bySkill == AT_SKILL_TELEPORT || bySkill == AT_SKILL_TELEPORT_B))
		{
			SelectedCharacter = -1;
			Attacking = -1;
		}
	}
}

bool SEASON3B::CNewUISkillList::Update()
{
	if (IsArrayIn(Hero->CurrentSkill) == true)
	{
		if (IsArrayUp(Hero->CurrentSkill) == true)
		{
			m_bHotKeySkillListUp = true;
		}
		else
		{
			m_bHotKeySkillListUp = false;
		}
	}

	if (Hero->m_pPet == NULL)
	{
		if (Hero->CurrentSkill >= AT_PET_COMMAND_DEFAULT && Hero->CurrentSkill < AT_PET_COMMAND_END)
		{
			Hero->CurrentSkill = 0;
		}
	}

	return true;
}

void SEASON3B::CNewUISkillList::RenderCurrentSkillAndHotSkillList()
{
	int i;
	float x, y, width, height;

	BYTE bySkillNumber = CharacterAttribute->SkillNumber;

	if (bySkillNumber > 0)
	{
		int iStartSkillIndex = 1;
		if (m_bHotKeySkillListUp)
		{
			iStartSkillIndex = 6;
		}
		if (gProtect.m_MainInfo.IsVersion == 1) //Skin Ss2
		{
			x = 310.0f + DisplayWinExt;
			y = 447.0f + DisplayHeightExt;
			width = 20;
			height = 28;
		}
		else
		{
			x = 190 + DisplayWinExt;
			y = 431 + DisplayHeightExt;
			width = 32; height = 38;
			for (i = 0; i < 5; ++i)
			{
				x += width;

				int iIndex = iStartSkillIndex + i;
				if (iIndex == 10)
				{
					iIndex = 0;
				}

				if (m_iHotKeySkillType[iIndex] == -1)
				{
					continue;
				}

				if (m_iHotKeySkillType[iIndex] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iIndex] < AT_PET_COMMAND_END)
				{
					if (Hero->m_pPet == NULL)
					{
						continue;
					}
				}

				if (Hero->CurrentSkill == m_iHotKeySkillType[iIndex])
				{
					SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
				}
				RenderSkillIcon(m_iHotKeySkillType[iIndex], x + 6, y + 6, 20, 28, 0, iIndex);
			}
		}

		GetLegacyCurrentSkillSlotRect(x, y, width, height);
		const bool highlightCurrentSkill = ShouldHighlightCurrentSkillSlot()
			|| m_EventState == EVENT_BTN_DOWN_CURRENTSKILL
			|| m_EventState == EVENT_BTN_HOVER_CURRENTSKILL;
		SEASON3B::RenderImage(
			highlightCurrentSkill ? IMAGE_SKILLBOX_USE : IMAGE_SKILLBOX,
			x,
			y,
			width,
			height);
		if (ShouldDrawCurrentSkillIcon())
		{
			const int drawSkillIndex = GetPrimarySkillSlotIndex();
			RenderSkillIcon(drawSkillIndex, x + 6.f, y + 6.f, 20.f, 28.f, 0, 0);
		}
	}
}
//=== Type = 1
bool SEASON3B::CNewUISkillList::RenderMuHelper(float X, float Y, int SkillSelect, int SkillBuff)
{
	int i;
	float x, y, width, height;
	bool hoverIconSkill = false;
	BYTE bySkillNumber = CharacterAttribute->SkillNumber;

	if (bySkillNumber > 0)
	{
		//if (m_bSkillList == true)
		{
			x = X;
			y = Y;
			width = 32;
			height = 38;
			float fOrigX = X - 35;
			int iSkillType = 0;
			int iSkillCount = 0;

			for (i = 0; i < MAX_MAGIC; ++i)
			{
				iSkillType = CharacterAttribute->Skill[i];
				if (iSkillType >= AT_PET_COMMAND_DEFAULT && iSkillType < AT_PET_COMMAND_END)
				{
					continue;
				}



				if (iSkillType != 0 && (iSkillType < AT_SKILL_STUN || iSkillType > AT_SKILL_REMOVAL_BUFF))
				{
					BYTE bySkillUseType = SkillAttribute[iSkillType].SkillUseType;

					if (bySkillUseType == SKILL_USE_TYPE_MASTER || bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
					{
						continue;
					}

					if (SkillBuff) //Show Buff
					{
						if (SkillAttribute[iSkillType].Magic_Icon == 0 || SkillAttribute[iSkillType].MasteryType != 255) continue;

						if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Buff[0] == iSkillType
							|| g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Buff[1] == iSkillType
							|| g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Buff[2] == iSkillType)
						{
							continue;
						}
					}
					else
					{
						if (SkillAttribute[iSkillType].Magic_Icon > 0 && SkillAttribute[iSkillType].MasteryType == 255) continue;

						if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[0] == iSkillType
							|| g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[1] == iSkillType
							|| g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[2] == iSkillType)
						{
							continue;
						}
					}
					if (iSkillCount % 8 == 0 && iSkillCount > 0)
					{
						x -= width;
						y = Y;
					}
					y += height;

					iSkillCount++;

					if (i == Hero->CurrentSkill)
					{
						SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
					}
					else
					{
						SEASON3B::RenderImage(IMAGE_SKILLBOX, x, y, width, height);
					}

					RenderSkillIcon(iSkillType, x + 6, y + 6, 20, 28, TRUE);
					if (SEASON3B::CheckMouseIn(x + 6, y + 6, 20, 28) == true)
					{
						g_pNewUISystem->GetUI_NewBCustomMenuInfo()->SetBlockCur();
						m_bRenderSkillInfo = true;
						m_iRenderSkillInfoType = i;
						m_iRenderSkillInfoPosX = x - 5;
						m_iRenderSkillInfoPosY = y - 15;
						//g_ConsoleDebug->Write(1, "Skill %d (%d - %d)", iSkillType, SkillAttribute[iSkillType].Magic_Icon, SkillAttribute[iSkillType].MasteryType);
						if (SEASON3B::IsRelease(VK_LBUTTON))
						{
							if (SkillBuff == 0)
							{
								g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[SkillSelect] = iSkillType;
							}
							else if (SkillBuff == 1)
							{
								g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Buff[SkillSelect] = iSkillType;
							}
							return true;
						}
					}

				}
			}
			//RenderPetSkill();
		}
	}

	if (m_bRenderSkillInfo == true && m_pNewUI3DRenderMng)
	{
		m_pNewUI3DRenderMng->RenderUI2DEffect(ITEMHOTKEYNUMBER_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);
		m_bRenderSkillInfo = false;

	}

	return false;
}

bool SEASON3B::CNewUISkillList::Render()
{
	int i;
	float x, y, width, height, FixX;
	if (gProtect.m_MainInfo.IsVersion == 1)
	{
		FixX = 75;
	}
	else
	{
		FixX = 0;
	}
	BYTE bySkillNumber = CharacterAttribute->SkillNumber;

	gInterface.HidenCustom = m_bSkillList;

#if defined(__ANDROID__) || defined(MU_IOS)
	if (m_bSkillList)
	{
		RenderAndroidSkillPickerBackdrop();
	}
#endif

	if (bySkillNumber > 0)
	{
		if (m_bSkillList == true)
		{
			width = 32; height = 38;

#if TAKUMI_ANDROID_UI_SKILL_PICKER_CACHE
			const uint64_t layoutSig = ComputeSkillPickerLayoutSig();
			if (layoutSig != m_skillPickerLayoutSig)
			{
				m_skillPickerLayoutSig = layoutSig;
				m_skillPickerLayoutDirty = true;
			}
			if (m_skillPickerLayoutDirty)
			{
				RebuildSkillPickerLayout();
			}

			for (size_t layoutIdx = 0; layoutIdx < m_skillPickerLayout.size(); ++layoutIdx)
			{
				const SkillPickerLayoutEntry& entry = m_skillPickerLayout[layoutIdx];
				x = entry.x;
				y = entry.y;
				const int i = entry.slotIndex;

				if (i == Hero->CurrentSkill)
				{
					SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
				}
				else
				{
					SEASON3B::RenderImage(IMAGE_SKILLBOX, x, y, width, height);
				}

				RenderSkillIcon(i, x + 6, y + 6, 20, 28);
#if defined(__ANDROID__) || defined(MU_IOS)
				if (SEASON3B::CheckMouseIn(x + 6.f, y + 6.f, 20.f, 28.f))
				{
					m_bRenderSkillInfo = true;
					m_iRenderSkillInfoType = i;
					m_iRenderSkillInfoPosX = x + 16.f - 5.f;
					m_iRenderSkillInfoPosY = y + 19.f - 15.f;
				}
#endif
			}
#else
			x = 385 - FixX + DisplayWinExt; y = 390 + DisplayHeightExt; width = 32; height = 38;
			float fOrigX = 385.f - FixX + DisplayWinExt;
			int iSkillCount = 0;

			for (i = 0; i < MAX_MAGIC; ++i)
			{
				if (!LegacyPickerIncludeMagicArrayIndex(i))
				{
					continue;
				}

				if (iSkillCount == 18)
				{
					y -= height;
				}

				if (iSkillCount < 14)
				{
					int iRemainder = iSkillCount % 2;
					int iQuotient = iSkillCount / 2;

					if (iRemainder == 0)
					{
						x = fOrigX + iQuotient * width;
					}
					else
					{
						x = fOrigX - (iQuotient + 1) * width;
					}
				}
				else if (iSkillCount >= 14 && iSkillCount < 18)
				{
					x = fOrigX - (8 * width) - ((iSkillCount - 14) * width);
				}
				else
				{
					x = fOrigX - (12 * width) + ((iSkillCount - 17) * width);
				}

				iSkillCount++;

#if defined(__ANDROID__) || defined(MU_IOS)
				const float cellDrawX = x + m_legacySkillPickerOffsetX;
				const float cellDrawY = y + m_virtualSkillPickerOffsetY;
#else
				const float cellDrawX = x;
				const float cellDrawY = y;
#endif

				if (i == Hero->CurrentSkill)
				{
					SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, cellDrawX, cellDrawY, width, height);
				}
				else
				{
					SEASON3B::RenderImage(IMAGE_SKILLBOX, cellDrawX, cellDrawY, width, height);
				}

				RenderSkillIcon(i, cellDrawX + 6.f, cellDrawY + 6.f, 20.f, 28.f);
			}
#endif
			RenderPetSkill();
		}
	}

	if (m_bRenderSkillInfo == true && m_pNewUI3DRenderMng)
	{
		m_pNewUI3DRenderMng->RenderUI2DEffect(ITEMHOTKEYNUMBER_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);
		m_bRenderSkillInfo = false;

	}

	return true;
}

void SEASON3B::CNewUISkillList::RenderSkillInfo()
{
	::RenderSkillInfo(m_iRenderSkillInfoPosX + 15, m_iRenderSkillInfoPosY - 10, m_iRenderSkillInfoType);
}

float SEASON3B::CNewUISkillList::GetLayerDepth()
{
	return 5.2f;
}

WORD SEASON3B::CNewUISkillList::GetHeroPriorSkill()
{
	return m_wHeroPriorSkill;
}

void SEASON3B::CNewUISkillList::SetHeroPriorSkill(BYTE bySkill)
{
	m_wHeroPriorSkill = bySkill;
}

void SEASON3B::CNewUISkillList::RenderPetSkill()
{
	if (Hero->m_pPet == NULL)
	{
		return;
	}

	float x, y, width, height, FixX;
	if (gProtect.m_MainInfo.IsVersion == 1)
	{
		FixX = 75;
	}
	else
	{
		FixX = 0;
	}
	x = 353.f - FixX + DisplayWinExt; y = 352 + DisplayHeightExt; width = 32; height = 38;
	for (int i = AT_PET_COMMAND_DEFAULT; i < AT_PET_COMMAND_END; ++i)
	{
		if (i == Hero->CurrentSkill)
		{
			SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
		}
		else
		{
			SEASON3B::RenderImage(IMAGE_SKILLBOX, x, y, width, height);
		}

		RenderSkillIcon(i, x + 6, y + 6, 20, 28);
		x += width;
	}
}

void SEASON3B::CNewUISkillList::RenderSkillIcon(
	int iIndex,
	float x,
	float y,
	float width,
	float height,
	int TypeMuHelper,
	int hotKeyLabelOverride,
	bool useCircularDraw,
	float circularRadiusUi)
{
	if (TypeMuHelper != 1 && !IsRenderableSkillSlotIndex(iIndex))
	{
		return;
	}

	WORD bySkillType = CharacterAttribute->Skill[iIndex];
	if (TypeMuHelper == 1)
	{
		bySkillType = iIndex;
		if ((bySkillType == 0 || !gSkillManager.FindHeroSkill((ActionSkillType)bySkillType)))
		{
			return;
		}

	}
	if (iIndex >= AT_PET_COMMAND_DEFAULT)
	{
		bySkillType = iIndex;
	}

	bool bCantSkill = false;

	BYTE bySkillUseType = SkillAttribute[bySkillType].SkillUseType;
	int Skill_Icon = SkillAttribute[bySkillType].Magic_Icon;

	if (!gSkillManager.DemendConditionCheckSkill(bySkillType))
	{
		bCantSkill = true;
	}

	if (IsCanBCSkill(bySkillType) == false)
	{
		bCantSkill = true;
	}
	if (g_isCharacterBuff((&Hero->Object), eBuff_AddSkill) && bySkillUseType == SKILL_USE_TYPE_BRAND)
	{
		bCantSkill = true;
	}

#if(CB_ATTACK_HIDEN_PET==0)

	if (bySkillType == AT_SKILL_SPEAR && (Hero->Helper.Type<MODEL_HELPER + 2 || Hero->Helper.Type>MODEL_HELPER + 3) && Hero->Helper.Type != MODEL_HELPER + 37)
	{
		bCantSkill = true;
	}

	if (bySkillType == AT_SKILL_SPEAR && (Hero->Helper.Type == MODEL_HELPER + 2 || Hero->Helper.Type == MODEL_HELPER + 3 || Hero->Helper.Type == MODEL_HELPER + 37))
	{
		int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
		int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
		if ((iTypeL < ITEM_SPEAR || iTypeL >= ITEM_BOW) && (iTypeR < ITEM_SPEAR || iTypeR >= ITEM_BOW))
		{
			bCantSkill = true;
		}
	}

	if (
		bySkillType == MASTER_SKILL_ADD_CYCLONE_IMPROVED1 ||
		bySkillType == MASTER_SKILL_ADD_CYCLONE_IMPROVED2 ||
		bySkillType == MASTER_SKILL_ADD_SLASH_IMPROVED ||
		bySkillType == MASTER_SKILL_ADD_LUNGE_IMPROVED ||
		bySkillType >= AT_SKILL_BLOCKING && bySkillType <= AT_SKILL_SWORD5 && (Hero->Helper.Type == MODEL_HELPER + 2 || Hero->Helper.Type == MODEL_HELPER + 3 || Hero->Helper.Type == MODEL_HELPER + 37))
	{
		bCantSkill = true;
	}

	if ((bySkillType == AT_SKILL_ICE_BLADE ||
		bySkillType == MASTER_SKILL_ADD_POWER_SLASH_IMPROVED ||
		(AT_SKILL_POWER_SLASH_UP <= bySkillType && AT_SKILL_POWER_SLASH_UP + 4 >= bySkillType)) && (Hero->Helper.Type == MODEL_HELPER + 2 || Hero->Helper.Type == MODEL_HELPER + 3 || Hero->Helper.Type == MODEL_HELPER + 37))
	{
		bCantSkill = true;
	}
#endif

	int iEnergy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;

	if (g_csItemOption.IsDisableSkill(bySkillType, iEnergy))
	{
		bCantSkill = true;
	}

	if (bySkillType == AT_SKILL_PARTY_TELEPORT && PartyNumber <= 0)
	{
		bCantSkill = true;
	}

	if (bySkillType == AT_SKILL_PARTY_TELEPORT && (IsDoppelGanger1() || IsDoppelGanger2() || IsDoppelGanger3() || IsDoppelGanger4()))
	{
		bCantSkill = true;
	}

	if (bySkillType == AT_SKILL_DARK_HORSE ||
		bySkillType == MASTER_SKILL_ADD_EARTHQUAKE_IMPROVED ||
		bySkillType == MASTER_SKILL_ADD_EARTHQUAKE_ENHANCED
		|| (AT_SKILL_ASHAKE_UP <= bySkillType && bySkillType <= AT_SKILL_ASHAKE_UP + 4))
	{
		BYTE byDarkHorseLife = 0;
		byDarkHorseLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
		if (byDarkHorseLife == 0 || Hero->Helper.Type != MODEL_HELPER + 4)
		{
			bCantSkill = true;

		}
	}
#ifdef PJH_FIX_SPRIT
	/*¹ÚÁ¾ÈÆ*/
	if (bySkillType >= AT_PET_COMMAND_DEFAULT && bySkillType < AT_PET_COMMAND_END)
	{
		int iCharisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
		PET_INFO PetInfo;
		giPetManager::GetPetInfo(PetInfo, 421 - PET_TYPE_DARK_SPIRIT);
		int RequireCharisma = (185 + (PetInfo.m_wLevel * 15));
		if (RequireCharisma > iCharisma)
		{
			bCantSkill = true;
		}
	}
#endif //PJH_FIX_SPRIT
	if (
		(bySkillType == AT_SKILL_INFINITY_ARROW) ||
		(bySkillType == MASTER_SKILL_ADD_INFINITY_ARROW_IMPROVED) ||
		(bySkillType == AT_SKILL_SWELL_OF_MAGICPOWER) || (bySkillType == MASTER_SKILL_ADD_MAGIC_CIRCLE_ENHANCED) || bySkillType == MASTER_SKILL_ADD_MAGIC_CIRCLE_IMPROVED)
	{
		if (g_csItemOption.IsDisableSkill(bySkillType, iEnergy))
		{
			bCantSkill = true;
		}
		if (
			(g_isCharacterBuff((&Hero->Object), eBuff_InfinityArrow)) ||
			(g_isCharacterBuff((&Hero->Object), EFFECT_INFINITY_ARROW_IMPROVED)) //Mui ten vo tan Master
			|| (g_isCharacterBuff((&Hero->Object), eBuff_SwellOfMagicPower))
			|| (g_isCharacterBuff((&Hero->Object), EFFECT_MAGIC_CIRCLE_IMPROVED))
			|| (g_isCharacterBuff((&Hero->Object), EFFECT_MAGIC_CIRCLE_ENHANCED))
			)
		{
			bCantSkill = true;
		}
	}

	if (bySkillType == AT_SKILL_REDUCEDEFENSE
		|| bySkillType == MASTER_SKILL_ADD_FIRE_SLASH_IMPROVED
		|| bySkillType == MASTER_SKILL_ADD_FIRE_SLASH_ENHANCED

		|| (AT_SKILL_BLOOD_ATT_UP <= bySkillType && bySkillType <= AT_SKILL_BLOOD_ATT_UP + 4))
	{
		WORD Strength;
		const WORD wRequireStrength = 596;
		Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
		if (Strength < wRequireStrength)
		{
			bCantSkill = true;
		}
		int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
		int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

		if (!(iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) && (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX)))
		{
			bCantSkill = true;
		}
	}

	switch (bySkillType)
	{
		//case AT_SKILL_PIERCING:
	case AT_SKILL_PARALYZE:
	case MASTER_SKILL_ADD_ICE_ARROW_IMPROVED:
	{
		WORD  Dexterity;
		const WORD wRequireDexterity = 646;
		Dexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
		if (Dexterity < wRequireDexterity)
		{
			bCantSkill = true;
		}
	}break;
	}

	if (bySkillType == AT_SKILL_WHEEL
		|| bySkillType == MASTER_SKILL_ADD_TWISTING_SLASH_ENHANCED
		|| bySkillType == MASTER_SKILL_ADD_TWISTING_SLASH_IMPROVED1
		|| bySkillType == MASTER_SKILL_ADD_TWISTING_SLASH_IMPROVED2
		|| (AT_SKILL_TORNADO_SWORDA_UP <= bySkillType && bySkillType <= AT_SKILL_TORNADO_SWORDA_UP + 4) || (AT_SKILL_TORNADO_SWORDB_UP <= bySkillType && bySkillType <= AT_SKILL_TORNADO_SWORDB_UP + 4)
		)
	{
		int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
		int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

		if (!(iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) && (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX)))
		{
			bCantSkill = true;
		}
	}

	if (gMapManager.InChaosCastle() == true)
	{
		if (bySkillType == AT_SKILL_DARK_HORSE ||
			bySkillType == MASTER_SKILL_ADD_EARTHQUAKE_IMPROVED ||
			bySkillType == MASTER_SKILL_ADD_EARTHQUAKE_ENHANCED ||
			bySkillType == AT_SKILL_RIDER || (bySkillType >= AT_PET_COMMAND_DEFAULT && bySkillType <= AT_PET_COMMAND_TARGET) || (AT_SKILL_ASHAKE_UP <= bySkillType && bySkillType <= AT_SKILL_ASHAKE_UP + 4))
		{
			bCantSkill = true;
		}
	}
	else
	{
		if (bySkillType == AT_SKILL_DARK_HORSE ||
			bySkillType == MASTER_SKILL_ADD_EARTHQUAKE_IMPROVED ||
			bySkillType == MASTER_SKILL_ADD_EARTHQUAKE_ENHANCED ||
			(AT_SKILL_ASHAKE_UP <= bySkillType && bySkillType <= AT_SKILL_ASHAKE_UP + 4))
		{
			BYTE byDarkHorseLife = 0;
			byDarkHorseLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
			if (byDarkHorseLife == 0)
			{
				bCantSkill = true;
			}
		}
	}

	int iCharisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;

	if (g_csItemOption.IsDisableSkill(bySkillType, iEnergy, iCharisma))
	{
		bCantSkill = true;
	}

#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	if (!g_CMonkSystem.IsSwordformGlovesUseSkill(bySkillType))
	{
		bCantSkill = true;
	}
	if (g_CMonkSystem.IsRideNotUseSkill(bySkillType, Hero->Helper.Type))
	{
		bCantSkill = true;
	}

	ITEM* pLeftRing = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
	ITEM* pRightRing = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];

	if (g_CMonkSystem.IsChangeringNotUseSkill(pLeftRing->Type, pRightRing->Type, pLeftRing->Level, pRightRing->Level)
		&& (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER))
	{
		bCantSkill = true;
	}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL

	float fU, fV;
	int iKindofSkill = 0;
	// UV layout in newui_skill*.jpg is for 20x28 cells — independent of on-screen draw size.
	const float atlasW = 20.f;
	const float atlasH = 28.f;

	if (g_csItemOption.Special_Option_Check() == false && (
		bySkillType == AT_SKILL_ICE_BLADE
		|| bySkillType == MASTER_SKILL_ADD_POWER_SLASH_IMPROVED
		|| (AT_SKILL_POWER_SLASH_UP <= bySkillType && AT_SKILL_POWER_SLASH_UP + 4 >= bySkillType)))
	{
		bCantSkill = true;
	}

	if (g_csItemOption.Special_Option_Check(1) == false && (
		bySkillType == AT_SKILL_CROSSBOW ||
		bySkillType == MASTER_SKILL_ADD_TRIPLE_SHOT_IMPROVED ||
		bySkillType == MASTER_SKILL_ADD_TRIPLE_SHOT_ENHANCED ||
		(AT_SKILL_MANY_ARROW_UP <= bySkillType && AT_SKILL_MANY_ARROW_UP + 4 >= bySkillType)))
		bCantSkill = true;

	if (bySkillType >= AT_PET_COMMAND_DEFAULT && bySkillType <= AT_PET_COMMAND_END)
	{
		fU = ((bySkillType - AT_PET_COMMAND_DEFAULT) % 8) * atlasW / 256.f;
		fV = ((bySkillType - AT_PET_COMMAND_DEFAULT) / 8) * atlasH / 256.f;
		iKindofSkill = KOS_COMMAND;
	}
	else if (bySkillType == AT_SKILL_PLASMA_STORM_FENRIR)
	{
		fU = 4 * atlasW / 256.f;
		fV = 0.f;
		iKindofSkill = KOS_COMMAND;
	}
	else if ((bySkillType >= AT_SKILL_ALICE_DRAINLIFE && bySkillType <= AT_SKILL_ALICE_THORNS))
	{
		fU = ((bySkillType - AT_SKILL_ALICE_DRAINLIFE) % 8) * atlasW / 256.f;
		fV = 3 * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if (bySkillType >= AT_SKILL_ALICE_SLEEP && bySkillType <= AT_SKILL_ALICE_BLIND)
	{
		fU = ((bySkillType - AT_SKILL_ALICE_SLEEP + 4) % 8) * atlasW / 256.f;
		fV = 3 * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if (bySkillType == AT_SKILL_ALICE_BERSERKER)
	{
		fU = 10 * atlasW / 256.f;
		fV = 3 * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if (bySkillType >= AT_SKILL_ALICE_WEAKNESS && bySkillType <= AT_SKILL_ALICE_ENERVATION)
	{
		fU = (bySkillType - AT_SKILL_ALICE_WEAKNESS + 8) * atlasW / 256.f;
		fV = 3 * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if (bySkillType >= AT_SKILL_SUMMON_EXPLOSION && bySkillType <= AT_SKILL_SUMMON_REQUIEM)
	{
		fU = ((bySkillType - AT_SKILL_SUMMON_EXPLOSION + 6) % 8) * atlasW / 256.f;
		fV = 3 * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if (bySkillType == AT_SKILL_SUMMON_POLLUTION)
	{
		fU = 11 * atlasW / 256.f;
		fV = 3 * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if (bySkillType == AT_SKILL_BLOW_OF_DESTRUCTION)
	{
		fU = 7 * atlasW / 256.f;
		fV = 2 * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if (bySkillType == AT_SKILL_GAOTIC)
	{
		fU = 3 * atlasW / 256.f;
		fV = 8 * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if (bySkillType == AT_SKILL_RECOVER)
	{
		fU = 9 * atlasW / 256.f;
		fV = 2 * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if (bySkillType == AT_SKILL_MULTI_SHOT)
	{
		if (gCharacterManager.GetEquipedBowType_Skill() == BOWTYPE_NONE)
		{
			bCantSkill = true;
		}

		fU = 0 * atlasW / 256.f;
		fV = 8 * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if (bySkillType == AT_SKILL_FLAME_STRIKE)
	{
		int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
		int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

		if (!(iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) && (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX)))
		{
			bCantSkill = true;
		}

		fU = 1 * atlasW / 256.f;
		fV = 8 * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if (bySkillType == AT_SKILL_GIGANTIC_STORM)
	{
		fU = 2 * atlasW / 256.f;
		fV = 8 * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if (bySkillType == AT_SKILL_LIGHTNING_SHOCK)
	{
		fU = 2 * atlasW / 256.f;
		fV = 3 * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if (AT_SKILL_LIGHTNING_SHOCK_UP <= bySkillType && bySkillType <= AT_SKILL_LIGHTNING_SHOCK_UP + 4)
	{
		fU = 6 * atlasW / 256.f;
		fV = 8 * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if (bySkillType == AT_SKILL_SWELL_OF_MAGICPOWER)
	{
		fU = 8 * atlasW / 256.f;
		fV = 2 * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if (bySkillUseType == 4)
	{
		fU = (atlasW / 256.f) * (Skill_Icon % 12);
		fV = (atlasH / 256.f) * ((Skill_Icon / 12) + 4);
		iKindofSkill = KOS_SKILL2;
	}
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	else if (bySkillType >= AT_SKILL_THRUST)
	{
		fU = ((bySkillType - 260) % 12) * atlasW / 256.f;
		fV = ((bySkillType - 260) / 12) * atlasH / 256.f;
		iKindofSkill = KOS_SKILL3;
	}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	else if (bySkillType >= 57 && Skill_Icon != 0)
	{
		// Skill.bmd Magic_Icon on SKILL2 (12-wide, row +4) matches master/season UI for skill IDs >= 57.
		// Lower IDs keep the legacy SKILL1 / explicit branches so icon matches hotbar + tooltip.
		fU = (atlasW / 256.f) * (Skill_Icon % 12);
		fV = (atlasH / 256.f) * ((Skill_Icon / 12) + 4);
		iKindofSkill = KOS_SKILL2;
	}
	else if (bySkillType >= 57)
	{
		fU = ((bySkillType - 57) % 8) * atlasW / 256.f;
		fV = ((bySkillType - 57) / 8) * atlasH / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else
	{
		fU = ((bySkillType - 1) % 8) * atlasW / 256.f;
		fV = ((bySkillType - 1) / 8) * atlasH / 256.f;
		iKindofSkill = KOS_SKILL1;
	}
	int iSkillIndex = 0;
	switch (iKindofSkill)
	{
	case KOS_COMMAND:
	{
		iSkillIndex = IMAGE_COMMAND;
	}break;
	case KOS_SKILL1:
	{
		iSkillIndex = IMAGE_SKILL1;
	}break;
	case KOS_SKILL2:
	{
		iSkillIndex = IMAGE_SKILL2;
	}break;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	case KOS_SKILL3:
	{
		iSkillIndex = IMAGE_SKILL3;
	}break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	}

	if (bCantSkill == true)
	{
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		iSkillIndex += 6;
#else //PBG_ADD_NEWCHAR_MONK_SKILL
		iSkillIndex += 5;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	}

	if (iSkillIndex != 0)
	{
		if (bySkillUseType == 4) //Skill Master
		{
			JCCoord GetXY;

			if (g_pMasterSkillTreeInterface->GetXYImgMaster(&GetXY, bySkillType))
			{
				if (bCantSkill == true)
				{
					SEASON3B::RenderImage(BITMAP_INTERFACE_MASTER_BEGIN + 3, x, y, 20.0, 28.0, GetXY.CalcX, GetXY.CalcY, 0.0390625, 0.053710938); //Non
				}
				else
				{
					SEASON3B::RenderImage(BITMAP_INTERFACE_MASTER_BEGIN + 2, x, y, 20.0, 28.0, GetXY.CalcX, GetXY.CalcY, 0.0390625, 0.053710938);
				}
			}

		}

		else if (useCircularDraw && circularRadiusUi > 0.5f)
		{
			const float uW = atlasW / 256.f;
			const float uH = atlasH / 256.f;
			RenderBitmapCircle(
				iSkillIndex,
				x - circularRadiusUi,
				y - circularRadiusUi,
				circularRadiusUi,
				fU + uW * 0.5f,
				fV + uH * 0.5f,
				uW * 0.5f,
				uH * 0.5f,
				true,
				true,
				0.f);
		}
		else
		{
			RenderBitmap(iSkillIndex, x, y, width, height, fU, fV, atlasW / 256.f, atlasH / 256.f);
		}
	}

	int iHotKey = hotKeyLabelOverride;
	if (hotKeyLabelOverride < 0)
	{
		iHotKey = -1;
		for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
		{
			if (m_iHotKeySkillType[i] == iIndex)
			{
				iHotKey = i;
				break;
			}
		}
	}

	if (TypeMuHelper != 1 && iHotKey >= 0 && iHotKey < SKILLHOTKEY_COUNT)
	{
		glColor3f(1.f, 0.9f, 0.8f);
		SEASON3B::RenderNumber(x + 20, y + 20, iHotKey);
	}

#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	if ((bySkillType == AT_SKILL_GIANTSWING
		|| bySkillType == MASTER_SKILL_ADD_CHAIN_DRIVER_IMPROVED
		|| bySkillType == MASTER_SKILL_ADD_CHAIN_DRIVER_ENHANCED
		|| bySkillType == AT_SKILL_DRAGON_KICK
		|| bySkillType == MASTER_SKILL_ADD_DRAGON_SLAYER_IMPROVED
		|| bySkillType == MASTER_SKILL_ADD_DRAGON_SLAYER_ENHANCED
		|| bySkillType == AT_SKILL_MUITENPHUONGHOANG
		|| bySkillType == MASTER_SKILL_ADD_DRAGON_LORE_IMPROVED
		|| bySkillType == MASTER_SKILL_ADD_DRAGON_LORE_ENHANCED
		|| bySkillType == AT_SKILL_DRAGON_LOWER) && (bCantSkill))
		return;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL

	if (
		(bySkillType != AT_SKILL_INFINITY_ARROW) &&
		(bySkillType != MASTER_SKILL_ADD_INFINITY_ARROW_IMPROVED)
		&& (bySkillType != AT_SKILL_SWELL_OF_MAGICPOWER)
		&& (bySkillType != MASTER_SKILL_ADD_MAGIC_CIRCLE_IMPROVED) && (bySkillType != MASTER_SKILL_ADD_MAGIC_CIRCLE_ENHANCED))

	{
		RenderSkillDelay(iIndex, x, y, width, height);
	}
}

void SEASON3B::CNewUISkillList::RenderSkillDelay(int iIndex, float x, float y, float width, float height)
{
	int iSkillDelay = CharacterAttribute->SkillDelay[iIndex];
	if (iSkillDelay > 0)
	{
		int iSkillType = CharacterAttribute->Skill[iIndex];

		if (iSkillType == AT_SKILL_PLASMA_STORM_FENRIR)
		{
			if (!CheckAttack())
			{
				return;
			}
		}

		int iSkillMaxDelay = SkillAttribute[iSkillType].Delay;

		float fPersent = (float)(iSkillDelay / (float)iSkillMaxDelay);

		EnableAlphaTest();
		glColor4f(1.f, 0.5f, 0.5f, 0.5f);
		float fdeltaH = height * fPersent;
		RenderColor(x, y + height - fdeltaH, width, fdeltaH);
		EndRenderColor();
	}
}

bool SEASON3B::CNewUISkillList::IsSkillListUp()
{
	return m_bHotKeySkillListUp;
}

void SEASON3B::CNewUISkillList::ResetMouseLButton()
{
	MouseLButton = false;
	MouseLButtonPop = false;
	MouseLButtonPush = false;
}

void SEASON3B::CNewUISkillList::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
	if (pClass)
	{
		CNewUISkillList* pSkillList = (CNewUISkillList*)(pClass);
		pSkillList->RenderSkillInfo();
	}
}

void SEASON3B::CNewUIMainFrameWindow::SetPreExp_Wide(__int64 dwPreExp)
{
	m_loPreExp = dwPreExp;
}

void SEASON3B::CNewUIMainFrameWindow::SetGetExp_Wide(__int64 dwGetExp)
{
	m_loGetExp = dwGetExp;

	if (m_loGetExp > 0)
	{
		m_bExpEffect = true;
		m_dwExpEffectTime = timeGetTime();
	}
}

bool SEASON3B::CNewUIMainFrameWindow::GetHudExperienceBarFill(
	float& currentFill01, float& priorFill01, bool& highlightGain, bool& highlightFullBar) const
{
	currentFill01 = 0.f;
	priorFill01 = 0.f;
	highlightGain = false;
	highlightFullBar = false;

	if (CharacterAttribute == nullptr)
	{
		return false;
	}

	const WORD wLevel = (WORD)max(1, (int)CharacterAttribute->Level);
	const DWORD dwNexExperience = (DWORD)CharacterAttribute->NextExperince;
	const DWORD dwExperience = (DWORD)CharacterAttribute->Experience;

	if (gCharacterManager.IsMasterLevel(CharacterAttribute->Class) == true)
	{
		const __int64 iTotalLevel = (__int64)wLevel + 400;
		const __int64 iTOverLevel = iTotalLevel - 255;
		const __int64 iData_Master =
			(((__int64)9 + iTotalLevel) * iTotalLevel * iTotalLevel * (__int64)10)
			+ (((__int64)9 + iTOverLevel) * iTOverLevel * iTOverLevel * (__int64)1000);
		const __int64 iBaseExperience = (iData_Master - (__int64)3892250000) / (__int64)2;

		const double fNeedExp = (double)dwNexExperience - (double)iBaseExperience;
		double fExp = (double)dwExperience - (double)iBaseExperience;
		if (dwExperience < iBaseExperience || fNeedExp <= 0.0)
		{
			fExp = 0.0;
		}
		else if (fExp < 0.0)
		{
			fExp = 0.0;
		}

		if (fExp > 0.0 && fNeedExp > 0.0)
		{
			currentFill01 = (float)(fExp / fNeedExp);
		}

		priorFill01 = currentFill01;
		if (!m_bExpEffect)
		{
			return true;
		}

		highlightGain = true;
		if (m_loPreExp < iBaseExperience)
		{
			highlightFullBar = true;
			return true;
		}

		double fPreExp = (double)m_loPreExp - (double)iBaseExperience;
		if (fPreExp > 0.0 && fNeedExp > 0.0)
		{
			priorFill01 = (float)(fPreExp / fNeedExp);
		}
		else
		{
			priorFill01 = 0.f;
		}

		const int iExpBarNum = (int)(currentFill01 * 10.f);
		const int iPreExpBarNum = (int)(priorFill01 * 10.f);
		if (iExpBarNum > iPreExpBarNum)
		{
			highlightFullBar = true;
		}

		return true;
	}

	const int level = max(1, (int)wLevel);
	currentFill01 = TakumiComputeExperienceFill01(dwExperience, dwNexExperience, level);
	priorFill01 = currentFill01;
	if (!m_bExpEffect)
	{
		return true;
	}

	highlightGain = true;
	priorFill01 = TakumiComputeExperienceFill01((DWORD)m_dwPreExp, dwNexExperience, level);

	const int iExpBarNum = (int)(currentFill01 * 10.f);
	const int iPreExpBarNum = (int)(priorFill01 * 10.f);
	if (iExpBarNum > iPreExpBarNum)
	{
		highlightFullBar = true;
	}

	return true;
}

void SEASON3B::CNewUIMainFrameWindow::SetPreExp(DWORD dwPreExp)
{
	m_dwPreExp = dwPreExp;
}

void SEASON3B::CNewUIMainFrameWindow::SetGetExp(DWORD dwGetExp)
{
	m_dwGetExp = dwGetExp;

	if (m_dwGetExp > 0)
	{
		m_bExpEffect = true;
		m_dwExpEffectTime = timeGetTime();
	}
}

void SEASON3B::CNewUIMainFrameWindow::SetBtnState(int iBtnType, bool bStateDown)
{
	switch (iBtnType)
	{
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
	case MAINFRAME_BTN_PARTCHARGE:
	{
		if (bStateDown)
		{
			m_BtnCShop.UnRegisterButtonState();
			m_BtnCShop.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_CSHOP, 2);
			m_BtnCShop.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_CSHOP, 3);
			m_BtnCShop.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_CSHOP, 2);
			m_BtnCShop.ChangeImgIndex(IMAGE_MENU_BTN_CSHOP, 2);
		}
		else
		{
			m_BtnCShop.UnRegisterButtonState();
			m_BtnCShop.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_CSHOP, 0);
			m_BtnCShop.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_CSHOP, 1);
			m_BtnCShop.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_CSHOP, 2);
			m_BtnCShop.ChangeImgIndex(IMAGE_MENU_BTN_CSHOP, 0);
		}
	}
	break;
#endif //defined defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
	case MAINFRAME_BTN_CHAINFO:
	{
		if (bStateDown)
		{
			m_BtnChaInfo.UnRegisterButtonState();
			m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_CHAINFO, 2);
			m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_CHAINFO, 3);
			m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_CHAINFO, 2);
			m_BtnChaInfo.ChangeImgIndex(IMAGE_MENU_BTN_CHAINFO, 2);

		}
		else
		{
			m_BtnChaInfo.UnRegisterButtonState();
			m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_CHAINFO, 0);
			m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_CHAINFO, 1);
			m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_CHAINFO, 2);
			m_BtnChaInfo.ChangeImgIndex(IMAGE_MENU_BTN_CHAINFO, 0);
		}
	}
	break;
	case MAINFRAME_BTN_MYINVEN:
	{
		if (bStateDown)
		{
			m_BtnMyInven.UnRegisterButtonState();
			m_BtnMyInven.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_MYINVEN, 2);
			m_BtnMyInven.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_MYINVEN, 3);
			m_BtnMyInven.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_MYINVEN, 2);
			m_BtnMyInven.ChangeImgIndex(IMAGE_MENU_BTN_MYINVEN, 2);
		}
		else
		{
			m_BtnMyInven.UnRegisterButtonState();
			m_BtnMyInven.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_MYINVEN, 0);
			m_BtnMyInven.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_MYINVEN, 1);
			m_BtnMyInven.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_MYINVEN, 2);
			m_BtnMyInven.ChangeImgIndex(IMAGE_MENU_BTN_MYINVEN, 0);
		}
	}
	break;
	case MAINFRAME_BTN_FRIEND:
	{
		if (bStateDown)
		{
			m_BtnFriend.UnRegisterButtonState();
			m_BtnFriend.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_FRIEND, 2);
			m_BtnFriend.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_FRIEND, 3);
			m_BtnFriend.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_FRIEND, 2);
			m_BtnFriend.ChangeImgIndex(IMAGE_MENU_BTN_FRIEND, 2);
		}
		else
		{
			m_BtnFriend.UnRegisterButtonState();
			m_BtnFriend.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_FRIEND, 0);
			m_BtnFriend.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_FRIEND, 1);
			m_BtnFriend.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_FRIEND, 2);
			m_BtnFriend.ChangeImgIndex(IMAGE_MENU_BTN_FRIEND, 0);
		}
	}
	break;
	case MAINFRAME_BTN_WINDOW:
	{
		if (bStateDown)
		{
			m_BtnWindow.UnRegisterButtonState();
			m_BtnWindow.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_WINDOW, 2);
			m_BtnWindow.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_WINDOW, 3);
			m_BtnWindow.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_WINDOW, 2);
			m_BtnWindow.ChangeImgIndex(IMAGE_MENU_BTN_WINDOW, 2);
		}
		else
		{
			m_BtnWindow.UnRegisterButtonState();
			m_BtnWindow.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_WINDOW, 0);
			m_BtnWindow.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_WINDOW, 1);
			m_BtnWindow.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_WINDOW, 2);
			m_BtnWindow.ChangeImgIndex(IMAGE_MENU_BTN_WINDOW, 0);
		}
	}
	break;
	}
}
