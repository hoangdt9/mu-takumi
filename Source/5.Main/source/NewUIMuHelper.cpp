#include "stdafx.h"
#include "NewUISystem.h"
#include "NewUIMuHelper.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "CBInterface.h"
#if(CB_OFFMUHELPER)
#include "CB_OffMuHelper.h"
#endif
using namespace SEASON3B;
extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayHeight;
extern int DisplayWinMid;
extern int DisplayWinExt;
extern int DisplayWinReal;
bool ShowSelectSkillMuHelper = false;
int bShowSelectSkill = -1;
int bShowType = -1;

CUITextInputBox* m_TimeBuffNhom = nullptr;
CUITextInputBox* m_InputTime[3] = { nullptr };
CUITextInputBox* m_ItemName = { nullptr };
CNewUIScrollBar* m_pScrollBarItem = nullptr;

int CountItemList = 0;
int SelectitemList = -1;
SEASON3B::CNewUIMuHelper::CNewUIMuHelper()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_ButtonList.clear();
    m_iNumCurOpenTab = 0;
	memset(&this->DataAutoMu, 0, sizeof(DataAutoMu));
	memset(&this->Helper, 0, sizeof(Helper));
	this->DataAutoMu.Clear();
	this->Helper.Clear();
	this->CacheConfig = false;
#if(CB_OFFMUHELPER)
	gCB_OffMuHelper = new CB_OffMuHelper;
#endif
}

SEASON3B::CNewUIMuHelper::~CNewUIMuHelper()
{
    Release();
	this->CacheConfig = false;
#if(CB_OFFMUHELPER)
	SAFE_DELETE(gCB_OffMuHelper);
#endif
}

bool SEASON3B::CNewUIMuHelper::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_MuHelper, this);

    SetPos(x, y);

    LoadImages();

    InitButtons();

    InitCheckBox();

    InitImage();

    InitText();

    Show(false);


    return true;
}

void SEASON3B::CNewUIMuHelper::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIMuHelper::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

void SEASON3B::CNewUIMuHelper::InitButtons()
{
    std::list<unicode::t_string> ltext;
    ltext.push_back(GlobalText[3500]);
    ltext.push_back(GlobalText[3501]);
    ltext.push_back(GlobalText[3590]);

    m_TabBtn.CreateRadioGroup(2, IMAGE_WINDOW_TAB_BTN, TRUE);
    m_TabBtn.ChangeRadioText(ltext);
    m_TabBtn.ChangeRadioButtonInfo(true, m_Pos.x + 10.f, m_Pos.y + 48.f, 56, 22);
    m_TabBtn.ChangeFrame(m_iNumCurOpenTab);

    InsertButton(IMAGE_CHAINFO_BTN_STAT, m_Pos.x + 56, m_Pos.y + 78, 16, 15, 0, 0, 0, 0, "", "", 0, 0);
    InsertButton(IMAGE_MACROUI_HELPER_RAGEMINUS, m_Pos.x + 56, m_Pos.y + 97, 16, 15, 0, 0, 0, 0, "", "", 1, 0);//- Range
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 191, 38, 24, 1, 0, 1, 1, GlobalText[3502], "", 2, 0); //-- skill 2
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 243, 38, 24, 1, 0, 1, 1, GlobalText[3502], "", 3, 0); //-- skill 3
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 84, 38, 24, 1, 0, 1, 1, GlobalText[3502], "", 4, 0); //-- Buff
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 79, 38, 24, 1, 0, 1, 1, GlobalText[3502], "", 5, 0); //-- potion
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 84, 38, 24, 1, 0, 1, 1, GlobalText[3502], "", 6, 0); //-- potion
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 17, m_Pos.y + 234, 38, 24, 1, 0, 1, 1, GlobalText[3502], "", 7, 0); //-- potion
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 17, m_Pos.y + 234, 38, 24, 1, 0, 1, 1, GlobalText[3502], "", 8, 0); //-- potion
    InsertButton(IMAGE_CHAINFO_BTN_STAT, m_Pos.x + 56, m_Pos.y + 78, 16, 15, 0, 0, 0, 0, "", "", 9, 1);
    InsertButton(IMAGE_MACROUI_HELPER_RAGEMINUS, m_Pos.x + 56, m_Pos.y + 97, 16, 15, 0, 0, 0, 0, "", "", 10, 1); //+ Range
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 208, 38, 24, 1, 0, 1, 1, GlobalText[3505], "", 11, 1); //-- Buff
    InsertButton(IMAGE_CLEARNESS_BTN, m_Pos.x + 132, m_Pos.y + 309, 38, 24, 1, 0, 1, 1, GlobalText[3506], "", 12, 1); //-- Buff
    //--
    InsertButton(IMAGE_IGS_BUTTON, m_Pos.x + 120, m_Pos.y + 388, 52, 26, 1, 0, 1, 1, GlobalText[3503], "", 13, -1);
    InsertButton(IMAGE_IGS_BUTTON, m_Pos.x + 65, m_Pos.y + 388, 52, 26, 1, 0, 1, 1, GlobalText[3504], "", 14, -1);
    InsertButton(IMAGE_BASE_WINDOW_BTN_EXIT, m_Pos.x + 20, m_Pos.y + 388, 36, 29, 0, 0, 0, 0, "", GlobalText[388], 15, -1);

    RegisterBtnCharacter(0xFF, 0);
    RegisterBtnCharacter(0xFF, 1);
    RegisterBtnCharacter(0xFF, 2);
    RegisterBtnCharacter(0xFF, 9);
    RegisterBtnCharacter(0xFF, 10);
    RegisterBtnCharacter(0xFF, 11);
    RegisterBtnCharacter(0xFF, 12);
    RegisterBtnCharacter(0xFF, 13);
    RegisterBtnCharacter(0xFF, 15);
    RegisterBtnCharacter(0xFF, 14);

    RegisterBtnCharacter(Dark_Knight, 3);
    RegisterBtnCharacter(Dark_Knight, 6);

    RegisterBtnCharacter(Dark_Wizard, 3);
    RegisterBtnCharacter(Dark_Wizard, 6);
    RegisterBtnCharacter(Dark_Wizard, 7);

    RegisterBtnCharacter(Magic_Gladiator, 3);
    RegisterBtnCharacter(Magic_Gladiator, 6);
    RegisterBtnCharacter(Dark_Lord, 6);

    RegisterBtnCharacter(Rage_Fighter, 3);
    RegisterBtnCharacter(Rage_Fighter, 6);

    RegisterBtnCharacter(Fairy_Elf, 3);
    RegisterBtnCharacter(Fairy_Elf, 4);
    RegisterBtnCharacter(Fairy_Elf, 8);

    RegisterBtnCharacter(Summoner, 3);
    RegisterBtnCharacter(Summoner, 5);
}

void SEASON3B::CNewUIMuHelper::InitCheckBox()
{
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 79, m_Pos.y + 80, 15, 15, 0, GlobalText[3507], 0, 0); //3507 "Bình thuốc"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 122, 15, 15, 0, GlobalText[3508], 1, 0);//3508 "Khoảng cách tấn công rộng"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 137, 15, 15, 0, GlobalText[3509], 2, 0);//3509 "Tấn công xung quanh"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 94, m_Pos.y + 174, 15, 15, 0, GlobalText[3510], 3, 0);//3510 "Delay"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 94, m_Pos.y + 191, 15, 15, 0, GlobalText[3511], 4, 0);//3511 "Con"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 94, m_Pos.y + 226, 15, 15, 0, GlobalText[3510], 5, 0);//3510 "Delay"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 94, m_Pos.y + 243, 15, 15, 0, GlobalText[3511], 6, 0);//3511 "Con"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 226, 15, 15, 0, GlobalText[3512], 7, 0);//3512 "Combo"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 25, m_Pos.y + 276, 15, 15, 0, GlobalText[3513], 8, 0);//3513 "Thời gian Buff "

    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 60, m_Pos.y + 218, 15, 15, 0, GlobalText[3514], 9, 0); //"Sử dụng Quạ Tinh"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 15, m_Pos.y + 218, 15, 15, 0, GlobalText[3515], 10, 0);//"Tạo Nhóm"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 79, m_Pos.y + 97, 15, 15, 0, GlobalText[3516], 11, 0);//"Điều trị"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 79, m_Pos.y + 97, 15, 15, 0, GlobalText[3517], 12, 0);//"Rút Sinh mệnh"

    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 79, m_Pos.y + 80, 15, 15, 0, GlobalText[3518], 13, 1);//"Sửa chữa vật phẩm"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 17, m_Pos.y + 125, 15, 15, 0, GlobalText[3519], 14, 1);//"Chọn tất cả vật phẩm"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 17, m_Pos.y + 152, 15, 15, 0, GlobalText[3520], 15, 1);//"Lựa chọn vật phẩm"

    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 22, m_Pos.y + 170, 15, 15, 0, GlobalText[3521], 16, 1);//"Jewel/Gem"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 85, m_Pos.y + 170, 15, 15, 0, GlobalText[3522], 17, 1);//"Set Item"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 22, m_Pos.y + 185, 15, 15, 0, GlobalText[3523], 18, 1);//"Zen"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 85, m_Pos.y + 185, 15, 15, 0, GlobalText[3524], 19, 1);//"Món đồ Hoàn Hảo"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 22, m_Pos.y + 200, 15, 15, 0, GlobalText[3525], 20, 1);//"Thêm item bổ sung"
    //--
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 80, 15, 15, 0, GlobalText[3591], 21, 2);//"Auto accept - Friend"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 97, 15, 15, 0, GlobalText[3592], 23, 2);//"Auto accept - Guild Member"
    InsertCheckBox(BITMAP_OPTION_BEGIN + 5, m_Pos.x + 18, m_Pos.y + 125, 15, 15, 0, GlobalText[3594], 22, 2);//"PVP Counterattack"

    RegisterBoxCharacter(0xFF, 0);
    RegisterBoxCharacter(0xFF, 1);
    RegisterBoxCharacter(0xFF, 2);
    RegisterBoxCharacter(0xFF, 3);
    RegisterBoxCharacter(0xFF, 4);
    RegisterBoxCharacter(0xFF, 8);
    RegisterBoxCharacter(0xFF, 13);
    RegisterBoxCharacter(0xFF, 14);
    RegisterBoxCharacter(0xFF, 15);
    RegisterBoxCharacter(0xFF, 16);
    RegisterBoxCharacter(0xFF, 17);
    RegisterBoxCharacter(0xFF, 19);
    RegisterBoxCharacter(0xFF, 18);
    RegisterBoxCharacter(0xFF, 20);
    RegisterBoxCharacter(0xFF, 21);
    RegisterBoxCharacter(0xFF, 23);
    RegisterBoxCharacter(0xFF, 22);

    RegisterBoxCharacter(Dark_Knight, 5);
    RegisterBoxCharacter(Dark_Knight, 6);
    RegisterBoxCharacter(Dark_Knight, 7); //combo

    RegisterBoxCharacter(Dark_Wizard, 5);
    RegisterBoxCharacter(Dark_Wizard, 6);
    RegisterBoxCharacter(Dark_Wizard, 10);  //Nhom

    RegisterBoxCharacter(Magic_Gladiator, 5);
    RegisterBoxCharacter(Magic_Gladiator, 6);

    RegisterBoxCharacter(Dark_Lord, 9); //su dung qua tinh

    RegisterBoxCharacter(Fairy_Elf, 11);  //dieu tri
    RegisterBoxCharacter(Fairy_Elf, 5);
    RegisterBoxCharacter(Fairy_Elf, 6);
    RegisterBoxCharacter(Fairy_Elf, 10); //Nhom

    RegisterBoxCharacter(Summoner, 5);
    RegisterBoxCharacter(Summoner, 6);
    RegisterBoxCharacter(Summoner, 12); //rut sinh menh

    RegisterBoxCharacter(Rage_Fighter, 5);
    RegisterBoxCharacter(Rage_Fighter, 6);
}

void SEASON3B::CNewUIMuHelper::InitImage()
{
    InsertTexture(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 17, m_Pos.y + 171, 32, 38, 0, 0);
    InsertTexture(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 61, m_Pos.y + 171, 32, 38, 1, 0);
    InsertTexture(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 61, m_Pos.y + 222, 32, 38, 2, 0);
    InsertTexture(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 21, m_Pos.y + 293, 32, 38, 3, 0);
    InsertTexture(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 55, m_Pos.y + 293, 32, 38, 4, 0);
    InsertTexture(BITMAP_INTERFACE_NEW_SKILLICON_BEGIN + 4, m_Pos.x + 89, m_Pos.y + 293, 32, 38, 5, 0);

    InsertTexture(IMAGE_MACROUI_HELPER_INPUTNUMBER, m_Pos.x + 135, m_Pos.y + 137, 28, 15, 6, 0);
    InsertTexture(IMAGE_MACROUI_HELPER_INPUTNUMBER, m_Pos.x + 135, m_Pos.y + 174, 28, 15, 7, 0);
    InsertTexture(IMAGE_MACROUI_HELPER_INPUTNUMBER, m_Pos.x + 135, m_Pos.y + 226, 28, 15, 8, 0);
    InsertTexture(IMAGE_MACROUI_HELPER_INPUTSTRING, m_Pos.x + 34, m_Pos.y + 216, 93, 15, 9, 1);

    RegisterTextureCharacter(0xFF, 0);
    RegisterTextureCharacter(0xFF, 1);
    RegisterTextureCharacter(0xFF, 3);
    RegisterTextureCharacter(0xFF, 4);
    RegisterTextureCharacter(0xFF, 5);
    RegisterTextureCharacter(0xFF, 6);
    RegisterTextureCharacter(0xFF, 7);
    RegisterTextureCharacter(0xFF, 9);

    RegisterTextureCharacter(Dark_Knight, 2);
    RegisterTextureCharacter(Dark_Knight, 8);
    RegisterTextureCharacter(Dark_Wizard, 2);
    RegisterTextureCharacter(Dark_Wizard, 8);
    RegisterTextureCharacter(Fairy_Elf, 2);
    RegisterTextureCharacter(Fairy_Elf, 8);
    RegisterTextureCharacter(Magic_Gladiator, 2);
    RegisterTextureCharacter(Magic_Gladiator, 8);
    RegisterTextureCharacter(Summoner, 2);
    RegisterTextureCharacter(Summoner, 8);
    RegisterTextureCharacter(Rage_Fighter, 2);
    RegisterTextureCharacter(Rage_Fighter, 8);
}

void SEASON3B::CNewUIMuHelper::InitText()
{
    InsertText(m_Pos.x + 18, m_Pos.y + 78, GlobalText[3526], 1, 0);
    InsertText(m_Pos.x + 18, m_Pos.y + 83, "________", 2, 0);
    InsertText(m_Pos.x + 113, m_Pos.y + 141, GlobalText[3527], 3, 0);
    InsertText(m_Pos.x + 162, m_Pos.y + 141, GlobalText[3528], 4, 0);
    InsertText(m_Pos.x + 18, m_Pos.y + 160, GlobalText[3529], 5, 0);
    InsertText(m_Pos.x + 59, m_Pos.y + 160, GlobalText[3530], 7, 0);
    InsertText(m_Pos.x + 162, m_Pos.y + 178, GlobalText[3528], 8, 0);
    InsertText(m_Pos.x + 59, m_Pos.y + 212, GlobalText[3531], 9, 0);

    InsertText(m_Pos.x + 162, m_Pos.y + 230, GlobalText[3528], 10, 0);
    InsertText(m_Pos.x + 18, m_Pos.y + 78, GlobalText[3532], 11, 1);
    InsertText(m_Pos.x + 18, m_Pos.y + 83, "________", 12, 1);

    RegisterTextCharacter(0xFF, 1);
    RegisterTextCharacter(0xFF, 2);
    RegisterTextCharacter(0xFF, 3);
    RegisterTextCharacter(0xFF, 4);
    RegisterTextCharacter(0xFF, 5);
    RegisterTextCharacter(0xFF, 7);
    RegisterTextCharacter(0xFF, 8);
    RegisterTextCharacter(0xFF, 11);
    RegisterTextCharacter(0xFF, 12);

    RegisterTextCharacter(Dark_Knight, 9);
    RegisterTextCharacter(Dark_Knight, 10);
    RegisterTextCharacter(Dark_Wizard, 9);
    RegisterTextCharacter(Dark_Wizard, 10);
    RegisterTextCharacter(Fairy_Elf, 9);
    RegisterTextCharacter(Fairy_Elf, 10);
    RegisterTextCharacter(Magic_Gladiator, 9);
    RegisterTextCharacter(Magic_Gladiator, 10);
    RegisterTextCharacter(Summoner, 9);
    RegisterTextCharacter(Summoner, 10);
    RegisterTextCharacter(Rage_Fighter, 9);
    RegisterTextCharacter(Rage_Fighter, 10);
}

bool SEASON3B::CNewUIMuHelper::Update()
{
    if (IsVisible())
    {
        int iNumCurOpenTab = m_TabBtn.UpdateMouseEvent();

        if (iNumCurOpenTab == RADIOGROUPEVENT_NONE)
            return true;

        m_iNumCurOpenTab = iNumCurOpenTab;
    }
    return true;
}

bool SEASON3B::CNewUIMuHelper::UpdateMouseEvent()
{
    /*switch (m_iNumCurOpenTab)
    {
    case TAB_GATE_MANAGING:
        UpdateGateManagingTab();
        break;
    case TAB_STATUE_MANAGING:
        UpdateStatueManagingTab();
        break;
    case TAB_TAX_MANAGING:
        UpdateTaxManagingTab();
        break;
    }

    if (true == BtnProcess())
        return false;*/

    POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };
    POINT ptExitBtn2 = { m_Pos.x + 13, m_Pos.y + 391 };

    if (SEASON3B::IsRelease(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
    {
		this->DataAutoMu = this->Helper;
        g_pNewUISystem->Hide(INTERFACE_MuHelper);
		return false;
    }

    int Identificador = UpdateMouseBtnList();

    ////g_ConsoleDebug->Write(MCD_RECEIVE, "MouseBtn Identificador: %d", Identificador);

    if (Identificador == 15)
    {
		this->DataAutoMu = this->Helper;
        g_pNewUISystem->Hide(INTERFACE_MuHelper);
        return false;
    }
    else 
	{

    }

	if(CheckMouseSelectSkill())  return false;

    UpdateMouseBoxList();


    if (CheckMouseIn(m_Pos.x, m_Pos.y, WINDOW_WIDTH, WINDOW_HEIGHT))
        return false;



    return true;
}

bool SEASON3B::CNewUIMuHelper::UpdateKeyEvent()
{
    if (IsVisible())
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(INTERFACE_MuHelper);
            //PlayBuffer(SOUND_CLICK01);

            return false;
        }
		if (g_pNewUISystem->IsVisible(INTERFACE_ExpandInventory) 
			|| g_pNewUISystem->IsVisible(INTERFACE_MOVEMAP)
			|| g_pNewUISystem->IsVisible(INTERFACE_PARTY)
			|| g_pNewUISystem->IsVisible(INTERFACE_TRADE)
			|| g_pNewUISystem->IsVisible(INTERFACE_INVENTORY)
			|| g_pNewUISystem->IsVisible(INTERFACE_CHARACTER)
			|| g_pNewUISystem->IsVisible(INTERFACE_CHARACTER)
			|| g_pNewUISystem->IsVisible(INTERFACE_NPC_DIALOGUE)
			|| g_pNewUISystem->IsVisible(INTERFACE_ExpandInventory)
			|| g_pNewUISystem->IsVisible(INTERFACE_WINDOW_MENU))
		{
			g_pNewUISystem->Hide(INTERFACE_MuHelper);
			return false;
		}
    }
    return true;
}

float SEASON3B::CNewUIMuHelper::GetLayerDepth()
{
    return 3.4;
}

float SEASON3B::CNewUIMuHelper::GetKeyEventOrder()
{
    return 3.4;
}
void SEASON3B::CNewUIMuHelper::OpenningProcess()
{

}

void SEASON3B::CNewUIMuHelper::ClosingProcess()
{
	gInterface.Data[eMuHelperConfig].Close();
	m_TimeBuffNhom = nullptr;
	m_InputTime[0] = nullptr;
	m_InputTime[1] = nullptr;
	m_InputTime[2] = nullptr;
	m_ItemName = nullptr;
	m_pScrollBarItem->Release();
	SAFE_DELETE(m_pScrollBarItem);
	SAFE_DELETE(m_TimeBuffNhom);
	SAFE_DELETE(m_ItemName);
	SAFE_DELETE(m_InputTime[0]);
	SAFE_DELETE(m_InputTime[1]);
	SAFE_DELETE(m_InputTime[2]);

	SelectitemList = -1;
}
char zbtemp[30] = { 0 };
void SEASON3B::CNewUIMuHelper::RenderConfigExtra()
{

	if (!gInterface.Data[eMuHelperConfig].OnShow) return;

	gInterface.Data[eMuHelperConfig].AllowMove = false;
	float MainWidth = 190;
	float MainHeight = 429;
	float StartY = 0;
	float StartX = m_Pos.x - MainWidth;

	//--
	switch (gInterface.Data[eMuHelperConfig].Type)
	{
	//===Thiet Lap Hoi Mau, Hut Mau, Nhan Vat
	case 4: //ELf
	{
		g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, MainWidth, MainHeight, eMuHelperConfig, GlobalText[3553]); //Config hoi mau
		float PosX = StartX + 10;
		float PosY = StartY + 45;
		g_pBCustomMenuInfo->RenderGroupBox(PosX, PosY, 170, 70, 70, 20);
		TextDraw(g_hFontBold, PosX + 5, PosY + 5, 0xffffffff, 0x0, 0, 0, 1, GlobalText[3545]);
		PosY += 30;
		int TestVaule = (this->DataAutoMu.PotPercent / 10);
		g_pBCustomMenuInfo->RederBarOptionW(PosX + 25, PosY, &TestVaule);
		this->DataAutoMu.PotPercent = (TestVaule * 10);
		TextDraw(g_hFont, PosX, PosY + 17, 0xffffffff, 0x0, 170, 0, 3, "%s (%d%%)", GlobalText[3547], this->DataAutoMu.PotPercent);

		//===Dung Skill Hoi Mau
		PosY += 45;
		g_pBCustomMenuInfo->RenderGroupBox(PosX, PosY, 170, 70, 70, 20);
		TextDraw(g_hFontBold, PosX + 5, PosY + 5, 0xffffffff, 0x0, 0, 0, 1, GlobalText[3546]);
		PosY += 30;
		int HVaule = (this->DataAutoMu.HealPercent / 10);
		g_pBCustomMenuInfo->RederBarOptionW(PosX + 25, PosY, &HVaule);
		this->DataAutoMu.HealPercent = (HVaule * 10);
		TextDraw(g_hFont, PosX, PosY + 17, 0xffffffff, 0x0, 170, 0, 3, "%s (%d%%)", GlobalText[3547], this->DataAutoMu.HealPercent);
	}
	break;
	case 5: //SUM
	{
		g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, MainWidth, MainHeight, eMuHelperConfig, GlobalText[3553]); //Config hoi mau
		float PosX = StartX + 10;
		float PosY = StartY + 45;
		g_pBCustomMenuInfo->RenderGroupBox(PosX, PosY, 170, 70, 70, 20);
		TextDraw(g_hFontBold, PosX + 5, PosY + 5, 0xffffffff, 0x0, 0, 0, 1, GlobalText[3545]);
		PosY += 30;
		int TestVaule = (this->DataAutoMu.PotPercent / 10);
		g_pBCustomMenuInfo->RederBarOptionW(PosX + 25, PosY, &TestVaule);
		this->DataAutoMu.PotPercent = (TestVaule * 10);
		TextDraw(g_hFont, PosX, PosY + 17, 0xffffffff, 0x0, 170, 0, 3, "%s (%d%%)", GlobalText[3547], this->DataAutoMu.PotPercent);

		//==Dung Skill Hut Mau
		 PosY += 45;
		g_pBCustomMenuInfo->RenderGroupBox(PosX, PosY, 170, 70, 70, 20);
		TextDraw(g_hFontBold, PosX + 5, PosY + 5, 0xffffffff, 0x0, 0, 0, 1, GlobalText[3517]);
		PosY += 30;
		int HVaule = (this->DataAutoMu.HealPercent / 10);
		g_pBCustomMenuInfo->RederBarOptionW(PosX + 25, PosY, &HVaule);
		this->DataAutoMu.HealPercent = (HVaule * 10);
		TextDraw(g_hFont, PosX, PosY + 17, 0xffffffff, 0x0, 170, 0, 3, "%s (%d%%)", GlobalText[3547], this->DataAutoMu.HealPercent);
	}
	break;
	case 6:
	{
		g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, MainWidth, MainHeight, eMuHelperConfig, GlobalText[3553]); //Config hoi mau
		float PosX = StartX + 10;
		float PosY = StartY + 45;
		g_pBCustomMenuInfo->RenderGroupBox(PosX, PosY, 170, 70, 70, 20);
		TextDraw(g_hFontBold, PosX + 5, PosY + 5, 0xffffffff, 0x0, 0, 0, 1, GlobalText[3545]);
		PosY += 30;
		int TestVaule = (this->DataAutoMu.PotPercent/10);
		g_pBCustomMenuInfo->RederBarOptionW(PosX+25, PosY, &TestVaule);
		this->DataAutoMu.PotPercent = (TestVaule * 10);
		TextDraw(g_hFont, PosX, PosY + 17, 0xffffffff, 0x0, 170, 0, 3, "%s (%d%%)",GlobalText[3547], this->DataAutoMu.PotPercent);
	}
	break;
	//====================================================
	//=== Thiet Lap Buff Party
	case 7: //DW
	{
		g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, MainWidth, MainHeight, eMuHelperConfig, GlobalText[3554]); //Config Buff Nhom
		float PosX = StartX + 10;
		float PosY = StartY + 45;
		//==Ho tro buff khac cho nhom
		g_pBCustomMenuInfo->RenderGroupBox(PosX, PosY, 170, 70, 70, 20);
		TextDraw(g_hFontBold, PosX + 5, PosY + 5, 0xffffffff, 0x0, 0, 0, 1, GlobalText[3549]);
		PosY += 25;
		if (g_pBCustomMenuInfo->RenderCheckBox(PosX + 15, PosY, 0xFFFFFFFF, this->DataAutoMu.PartyAutoBuff == 1 ? TRUE : FALSE, GlobalText[3540]))
		{
			this->DataAutoMu.PartyAutoBuff ^= 1;
		}
		TextDraw(g_hFont, PosX + 15, PosY + 20, 0xffffffff, 0x0, 0, 0, 1, GlobalText[3551]);
		TextDraw(g_hFont, PosX + 105, PosY + 20, 0xffffffff, 0x0, 0, 0, 1, GlobalText[3528]);

		RenderImage(IMAGE_MACROUI_HELPER_INPUTNUMBER, PosX + 78, PosY + 16, 28, 15);
		char ztemp[30] = { 0 };
		sprintf_s(ztemp, "%d", this->DataAutoMu.PartyBuffTime);
		if (g_pBCustomMenuInfo->RenderInputBox(PosX + 83, PosY + 19, 50, 14, ztemp, m_TimeBuffNhom, UIOPTION_NUMBERONLY, 2))
		{
			char m_GetText[15];
			memset(&m_GetText, 0, sizeof(char) * 15);
			m_TimeBuffNhom->GetText(m_GetText);
			if (atoi(m_GetText) > 1)
			{
				this->DataAutoMu.PartyBuffTime = atoi(m_GetText);
			}
			else
			{
				this->DataAutoMu.PartyBuffTime = 1;
			}
			//TextDraw(g_hFont, PosX + 105, PosY + 20, 0xffffffff, 0x0, 0, 0, 1, m_GetText);

			m_TimeBuffNhom->SetTextColor(255, 0, 0, 0);
			m_TimeBuffNhom->SetBackColor(255, 255, 255, 255);

		}
	}
	break;
	case 8: //Elf
	{
		g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, MainWidth, MainHeight, eMuHelperConfig, GlobalText[3554]); //Config Buff Nhom
		float PosX = StartX + 10;
		float PosY = StartY + 45;

		g_pBCustomMenuInfo->RenderGroupBox(PosX, PosY, 170, 85, 70, 20);
		TextDraw(g_hFontBold, PosX + 5, PosY + 5, 0xffffffff, 0x0, 0, 0, 1, GlobalText[3548]); //Ho tro dieu tri
		//==ho tro buff mau nhom
		PosY += 25;
		if (g_pBCustomMenuInfo->RenderCheckBox(PosX + 15, PosY, 0xFFFFFFFF, this->DataAutoMu.PartyAutoHeal == 1 ? TRUE : FALSE, GlobalText[3539]))
		{
			this->DataAutoMu.PartyAutoHeal ^= 1;
		}
		PosY += 20;
		int TestVaule = (this->DataAutoMu.PartyHealPercent / 10);
		g_pBCustomMenuInfo->RederBarOptionW(PosX + 25, PosY, &TestVaule);
		this->DataAutoMu.PartyHealPercent = (TestVaule * 10);
		TextDraw(g_hFont, PosX, PosY + 17, 0xffffffff, 0x0, 170, 0, 3, "%s (%d%%)", GlobalText[3550], this->DataAutoMu.PartyHealPercent);


		//==Ho tro buff khac cho nhom
		PosY += 45;
		g_pBCustomMenuInfo->RenderGroupBox(PosX, PosY, 170, 70, 70, 20);
		TextDraw(g_hFontBold, PosX + 5, PosY + 5, 0xffffffff, 0x0, 0, 0, 1, GlobalText[3549]);
		PosY += 25;
		if (g_pBCustomMenuInfo->RenderCheckBox(PosX + 15, PosY, 0xFFFFFFFF, this->DataAutoMu.PartyAutoBuff == 1 ? TRUE : FALSE, GlobalText[3540]))
		{
			this->DataAutoMu.PartyAutoBuff  ^= 1;
		}
		TextDraw(g_hFont, PosX + 15, PosY + 20, 0xffffffff, 0x0, 0, 0, 1, GlobalText[3551]);
		TextDraw(g_hFont, PosX + 105, PosY + 20, 0xffffffff, 0x0, 0, 0, 1, GlobalText[3528]);
		
		RenderImage(IMAGE_MACROUI_HELPER_INPUTNUMBER, PosX + 78, PosY + 16, 45, 15);
		char ztemp[30] = { 0 };
		sprintf_s(ztemp, "%d", this->DataAutoMu.PartyBuffTime);
		if (g_pBCustomMenuInfo->RenderInputBox(PosX + 83, PosY + 19, 50, 14, ztemp, m_TimeBuffNhom, UIOPTION_NUMBERONLY, 2))
		{
			char m_GetText[15];
			memset(&m_GetText, 0, sizeof(char) * 15);
			m_TimeBuffNhom->GetText(m_GetText);
			if (atoi(m_GetText) > 1)
			{
				this->DataAutoMu.PartyBuffTime = atoi(m_GetText);
			}
			else
			{
				this->DataAutoMu.PartyBuffTime = 1;
			}
			//TextDraw(g_hFont, PosX + 105, PosY + 20, 0xffffffff, 0x0, 0, 0, 1, m_GetText);

			m_TimeBuffNhom->SetTextColor(255, 0, 0, 0);
			m_TimeBuffNhom->SetBackColor(255, 255, 255, 255);
			
		}
	}
	break;
	//=== Thiet Lap so luong Quai ra skill
	case 2:
	case 3:
	{
		int SelectCat = gInterface.Data[eMuHelperConfig].Type - 2;
		g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, MainWidth, MainHeight, eMuHelperConfig, "%s (%d)",GlobalText[3552], gInterface.Data[eMuHelperConfig].Type); //Config sl quai skill 2
		float PosX = StartX + 10;
		float PosY = StartY + 45;
	
		g_pBCustomMenuInfo->RenderGroupBox(PosX, PosY, 170, 70, 70, 20);
		TextDraw(g_hFontBold, PosX + 5, PosY + 5, 0xffffffff, 0x0, 0, 0, 1, GlobalText[3543]);
		PosY += 25;
		if (g_pBCustomMenuInfo->RenderCheckOption(PosX + 10, PosY, 0xFFFFFFFF, this->DataAutoMu.SkillPreCon[SelectCat] == 0 ? TRUE : FALSE, GlobalText[3555]))
		{
			this->DataAutoMu.SkillPreCon[SelectCat] = 0;
		}
		PosY += 20;
		if (g_pBCustomMenuInfo->RenderCheckOption(PosX + 10, PosY, 0xFFFFFFFF, this->DataAutoMu.SkillPreCon[SelectCat] == 1 ? TRUE : FALSE, GlobalText[3556]))
		{
			this->DataAutoMu.SkillPreCon[SelectCat] = 1;
		}
		PosY += 45;
		g_pBCustomMenuInfo->RenderGroupBox(PosX, PosY, 170, 70, 70, 20);
		TextDraw(g_hFontBold, PosX + 5, PosY + 5, 0xffffffff, 0x0, 0, 0, 1, GlobalText[3544]);
		PosY += 25;
		if (g_pBCustomMenuInfo->RenderCheckOption(PosX + 10, PosY, 0xFFFFFFFF, this->DataAutoMu.SkillSubCon[SelectCat] == 0 ? TRUE : FALSE, GlobalText[3557]))
		{
			this->DataAutoMu.SkillSubCon[SelectCat] = 0;
		}
		if (g_pBCustomMenuInfo->RenderCheckOption(PosX + 85, PosY, 0xFFFFFFFF, this->DataAutoMu.SkillSubCon[SelectCat] == 1 ? TRUE : FALSE, GlobalText[3558]))
		{
			this->DataAutoMu.SkillSubCon[SelectCat] = 1;
		}
		PosY += 20;
		if (g_pBCustomMenuInfo->RenderCheckOption(PosX + 10, PosY, 0xFFFFFFFF, this->DataAutoMu.SkillSubCon[SelectCat] == 2 ? TRUE : FALSE, GlobalText[3559]))
		{
			this->DataAutoMu.SkillSubCon[SelectCat] = 2;
		}
		if (g_pBCustomMenuInfo->RenderCheckOption(PosX + 85, PosY, 0xFFFFFFFF, this->DataAutoMu.SkillSubCon[SelectCat] == 3 ? TRUE : FALSE, GlobalText[3560]))
		{
			this->DataAutoMu.SkillSubCon[SelectCat] = 3;
		}
	}
	break;

	default:
		break;
	}
	
}
bool SEASON3B::CNewUIMuHelper::Render()
{
	if (!CacheConfig) return false;
	if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Started)
	{
			// todo: start mu helper when it's implemented :)
			StartMuHelper(1);
			return true;
	}

    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    DWORD TextColor = g_pRenderText->GetTextColor();

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(0xFFFFFFFF);

    g_pRenderText->SetBgColor(0);

    RenderImage(IMAGE_BASE_WINDOW_BACK, m_Pos.x, m_Pos.y, float(WINDOW_WIDTH), float(WINDOW_HEIGHT));
    RenderImage(IMAGE_BASE_WINDOW_TOP, m_Pos.x, m_Pos.y, float(WINDOW_WIDTH), 64.f);
    RenderImage(IMAGE_BASE_WINDOW_LEFT, m_Pos.x, m_Pos.y + 64.f, 21.f, float(WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_BASE_WINDOW_RIGHT, m_Pos.x + float(WINDOW_WIDTH) - 21.f, m_Pos.y + 64.f, 21.f, float(WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_BASE_WINDOW_BOTTOM, m_Pos.x, m_Pos.y + float(WINDOW_HEIGHT) - 45.f, float(WINDOW_WIDTH), 45.f);

    g_pRenderText->SetFont(g_hFontBold);

    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 13, GlobalText[3536], 190, 0, RT3_SORT_CENTER);


	//==Nhet Button Ủy Thác Offline vào đây

    RenderBack(m_Pos.x + 12, m_Pos.y + 340, 165, 46);
#if(CB_OFFMUHELPER)
	if(gCB_OffMuHelper) gCB_OffMuHelper->Render(m_Pos.x+12, m_Pos.y + 340);
#else
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + 347, GlobalText[3537], 0, 0, RT3_SORT_CENTER);

    g_pRenderText->SetTextColor(0xFF00B4FF);
    g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + 365, GlobalText[3538], 0, 0, RT3_SORT_CENTER);
	g_pRenderText->SetTextColor(TextColor);
#endif



    int sTapIndex = m_iNumCurOpenTab;

    m_TabBtn.Render();

	

    if (sTapIndex)
    {
        if (sTapIndex == 1)
        {
            RenderBack(m_Pos.x + 12, m_Pos.y + 73, 68, 50);
            RenderBack(m_Pos.x + 75, m_Pos.y + 73, 102, 50);
            RenderBack(m_Pos.x + 12, m_Pos.y + 120, 165, 30);
            RenderBack(m_Pos.x + 12, m_Pos.y + 147, 165, 195);
            RenderBack(m_Pos.x + 16, m_Pos.y + 235, 158, 74);

            RenderImage(BITMAP_INTERFACE_CRYWOLF_BEGIN + 33 + this->DataAutoMu.Range[1], m_Pos.x + 29, m_Pos.y + 92, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f);
        }
        else
        {
            RenderBack(m_Pos.x + 12, m_Pos.y + 73, 165, 50);
            RenderBack(m_Pos.x + 12, m_Pos.y + 120, 165, 222);
        }
    }
    else
    {
        RenderBack(m_Pos.x + 12, m_Pos.y + 73, 68, 50);
        RenderBack(m_Pos.x + 75, m_Pos.y + 73, 102, 50);
        RenderBack(m_Pos.x + 12, m_Pos.y + 120, 165, 39);
        RenderBack(m_Pos.x + 12, m_Pos.y + 156, 165, 120);
        RenderBack(m_Pos.x + 12, m_Pos.y + 273, 165, 69);

        RenderImage(BITMAP_INTERFACE_CRYWOLF_BEGIN + 33 + this->DataAutoMu.Range[0], m_Pos.x + 29, m_Pos.y + 92, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f);

    }


    RenderBoxList();
    RenderTextureList();
    RenderTextList();
    RenderBtnList();
	if (sTapIndex==0)
	{
		//int SkillSelect = -1;
		//==Render icon skill
		g_pSkillList->RenderSkillIcon(this->DataAutoMu.Skill[0], m_Pos.x + 17+5, m_Pos.y + 171+5, 20, 28, TRUE);

		g_pSkillList->RenderSkillIcon(this->DataAutoMu.Skill[1], m_Pos.x + 61 +5, m_Pos.y + 171+5, 20, 28, TRUE);

		g_pSkillList->RenderSkillIcon(this->DataAutoMu.Skill[2], m_Pos.x + 61 +5, m_Pos.y + 222 +5, 20, 28, TRUE);

		g_pSkillList->RenderSkillIcon(this->DataAutoMu.Buff[0], m_Pos.x + 21 +5, m_Pos.y + 293 +5, 20, 28, TRUE);

		g_pSkillList->RenderSkillIcon(this->DataAutoMu.Buff[1], m_Pos.x + 55 +5, m_Pos.y + 293 +5, 20, 28, TRUE);

		g_pSkillList->RenderSkillIcon(this->DataAutoMu.Buff[2], m_Pos.x + 89 +5, m_Pos.y + 293 +5, 20, 28, TRUE);


		if (ShowSelectSkillMuHelper)
		{
			if (g_pSkillList->RenderMuHelper(m_Pos.x - 33, m_Pos.y + 40, bShowSelectSkill, bShowType))
			{
				ShowSelectSkillMuHelper = 0;
				bShowSelectSkill = -1;
				bShowType = -1;

			}
		}
		
		//==Render Input
		float InputX = m_Pos.x + 142;
		float InputY = m_Pos.y + 140;

		sprintf_s(zbtemp, "%d", (this->DataAutoMu.MoveTime > 1 ? this->DataAutoMu.MoveTime : 1));
		if (g_pBCustomMenuInfo->RenderInputBox(InputX, InputY, 50, 14, zbtemp, m_InputTime[0], UIOPTION_NUMBERONLY, 2))
		{
			char m_GetText[15];
			memset(&m_GetText, 0, sizeof(char) * 15);
			m_InputTime[0]->GetText(m_GetText);
			if (atoi(m_GetText) > 1)
			{
				this->DataAutoMu.MoveTime = atoi(m_GetText);
			}
			else
			{
				this->DataAutoMu.MoveTime = 1;
			}

			m_InputTime[0]->SetTextColor(255, 0, 0, 0);
			m_InputTime[0]->SetBackColor(255, 255, 255, 255);

		}
		//=== Skill delay
		InputY += 38;
		sprintf_s(zbtemp, "%d", (this->DataAutoMu.Delay[0] > 1 ? this->DataAutoMu.Delay[0] : 1));
		if (g_pBCustomMenuInfo->RenderInputBox(InputX, InputY, 50, 14, zbtemp, m_InputTime[1], UIOPTION_NUMBERONLY, 2))
		{
			char m_GetText[15];
			memset(&m_GetText, 0, sizeof(char) * 15);
			m_InputTime[1]->GetText(m_GetText);
			if (atoi(m_GetText) > 1)
			{
				this->DataAutoMu.Delay[0] = atoi(m_GetText);
			}
			else
			{
				this->DataAutoMu.Delay[0] = 1;
			}

			m_InputTime[1]->SetTextColor(255, 0, 0, 0);
			m_InputTime[1]->SetBackColor(255, 255, 255, 255);

		}
		if (gCharacterManager.GetBaseClass(Hero->Class) != Dark_Lord)
		{
			InputY += 52;
			sprintf_s(zbtemp, "%d", (this->DataAutoMu.Delay[1] > 1 ? this->DataAutoMu.Delay[1] : 1));
			if (g_pBCustomMenuInfo->RenderInputBox(InputX, InputY, 50, 14, zbtemp, m_InputTime[2], UIOPTION_NUMBERONLY, 2))
			{
				char m_GetText[15];
				memset(&m_GetText, 0, sizeof(char) * 15);
				m_InputTime[2]->GetText(m_GetText);
				if (atoi(m_GetText) > 1)
				{
					this->DataAutoMu.Delay[1] = atoi(m_GetText);
				}
				else
				{
					this->DataAutoMu.Delay[1] = 1;
				}

				m_InputTime[2]->SetTextColor(255, 0, 0, 0);
				m_InputTime[2]->SetBackColor(255, 255, 255, 255);

			}
		}
	}
	if (sTapIndex == 1)
	{
		//==Render Input
		float InputX = m_Pos.x + 38;
		float InputY = m_Pos.y + 219;


		if (g_pBCustomMenuInfo->RenderInputBox(InputX, InputY, 80, 14, " ", m_ItemName, UIOPTION_NULL, 15))
		{
			//char m_GetText[15];
			//memset(&m_GetText, 0, sizeof(char) * 15);
			//m_ItemName->GetText(m_GetText);
			//if (atoi(m_GetText) > 1)
			//{
			//	this->DataAutoMu.MoveTime = atoi(m_GetText);
			//}
			//else
			//{
			//	this->DataAutoMu.MoveTime = 1;
			//}

			m_ItemName->SetTextColor(255, 0, 0, 0);
			m_ItemName->SetBackColor(255, 255, 255, 255);

		}
		//==Draw Item List


		InputY += 22;
		DWORD ColorBG = 0x0;
		int Line = 0;
		//===Scroll Bar
		if (m_pScrollBarItem == NULL)
		{
			m_pScrollBarItem = new CNewUIScrollBar();
			m_pScrollBarItem->Create(InputX +120, InputY, 55);
			m_pScrollBarItem->SetMaxPos(MAX_HELPER_ITEM/5);
			m_pScrollBarItem->SetPos(InputX + 120, InputY);
		}
		else
		{
			m_pScrollBarItem->MouseWheelWindow = SEASON3B::CheckMouseIn(InputX -10, InputY, 140, 55);

			m_pScrollBarItem->Render();
			m_pScrollBarItem->UpdateMouseEvent();
			m_pScrollBarItem->Update();
		}
		//======
		int mPage = m_pScrollBarItem->GetCurPos();
		for (int n = (mPage*5); n < MAX_HELPER_ITEM; n++)
		{
			if (strlen(this->DataAutoMu.ItemList[n]) < 1) continue;
			if (n == SelectitemList && SelectitemList != -1)
			{
				ColorBG = 0xFFD042A5;
			}
			else
			{
				ColorBG = 0x0;
			}
			g_pRenderText->SetBgColor(ColorBG);
			g_pRenderText->RenderText(InputX - 10, InputY + (Line * 12), this->DataAutoMu.ItemList[n], 125);
			//TextDraw(g_hFont, InputX-10, InputY+(Line *12), 0xffffffff, ColorBG, 125, 0, 3, this->DataAutoMu.ItemList[n]);
			if (SEASON3B::CheckMouseIn(InputX - 5, InputY + (Line * 12), 125, 13) == 1)
			{
				if (GetKeyState(VK_LBUTTON) & 0x8000 && (GetTickCount() - gInterface.Data[eTIME].EventTick) > 300)
				{
					gInterface.Data[eTIME].EventTick = GetTickCount();
					SelectitemList = n;
				}
			}
			Line++;
			if (Line >= 5) break;
		}

			
	}
	CNewUIMuHelper::RenderConfigExtra();
    DisableAlphaBlend();

    return true;
}
bool SEASON3B::CNewUIMuHelper::CheckMouseSelectSkill()
{
	if (m_iNumCurOpenTab == 0) //table select skill
	{
		//==Click chon skill
		if (SEASON3B::CheckMouseIn(m_Pos.x + 17 + 5, m_Pos.y + 171 + 5, 20, 28))
		{
			if (SEASON3B::IsRelease(VK_LBUTTON))
			{
				ShowSelectSkillMuHelper ^= 1;
				bShowSelectSkill = 0;
				bShowType = 0;
				return true;
			}
			else if (SEASON3B::IsRelease(VK_RBUTTON))
			{
				g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[0] = -1;
				return true;
			}
		}

		if (SEASON3B::CheckMouseIn(m_Pos.x + 61 + 5, m_Pos.y + 171 + 5, 20, 28))
		{
			if (SEASON3B::IsRelease(VK_LBUTTON))
			{
				ShowSelectSkillMuHelper ^= 1;
				bShowSelectSkill = 1;
				bShowType = 0;
				return true;
			}
			else if (SEASON3B::IsRelease(VK_RBUTTON))
			{
				g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[1] = -1;
				return true;
			}
		}

		if (SEASON3B::CheckMouseIn(m_Pos.x + 61 + 5, m_Pos.y + 222 + 5, 20, 28) && gCharacterManager.GetBaseClass(Hero->Class) != Dark_Lord)
		{
			if (SEASON3B::IsRelease(VK_LBUTTON))
			{
				ShowSelectSkillMuHelper ^= 1;
				bShowSelectSkill = 2;
				bShowType = 0;
				return true;
			}
			else if (SEASON3B::IsRelease(VK_RBUTTON))
			{
				g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[2] = -1;
				return true;
			}
		}
		//-==Buff
		if (SEASON3B::CheckMouseIn(m_Pos.x + 21 + 5, m_Pos.y + 293 + 5, 20, 28))
		{
			if (SEASON3B::IsRelease(VK_LBUTTON))
			{
				ShowSelectSkillMuHelper ^= 1;
				bShowSelectSkill = 0;
				bShowType = 1;
				return true;
			}
			else if (SEASON3B::IsRelease(VK_RBUTTON))
			{
				g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Buff[0] = -1;
				return true;
			}
		}
		if (SEASON3B::CheckMouseIn(m_Pos.x + 55 + 5, m_Pos.y + 293 + 5, 20, 28))
		{
			if (SEASON3B::IsRelease(VK_LBUTTON))
			{
				ShowSelectSkillMuHelper ^= 1;
				bShowSelectSkill = 1;
				bShowType = 1;
				return true;
			}
			else if (SEASON3B::IsRelease(VK_RBUTTON))
			{
				g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Buff[1] = -1;
				return true;
			}
		}

		if (SEASON3B::CheckMouseIn(m_Pos.x + 89 + 5, m_Pos.y + 293 + 5, 20, 28))
		{
			if (SEASON3B::IsRelease(VK_LBUTTON))
			{
				ShowSelectSkillMuHelper ^= 1;
				bShowSelectSkill = 2;
				bShowType = 1;
				return true;
			}
			else if (SEASON3B::IsRelease(VK_RBUTTON))
			{
				g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Buff[2] = -1;
				return true;
			}
		}
	}
	//==end
	return false;
}
void SEASON3B::CNewUIMuHelper::RenderBack(int x, int y, int width, int height)
{
    EnableAlphaTest();
    glColor4f(0.0, 0.0, 0.0, 0.4f);
    RenderColor(x + 3.f, y + 2.f, width - 7.f, height - 7, 0.0, 0);
    EndRenderColor();

    RenderImage(IMAGE_TABLE_TOP_LEFT, x, y, 14.0, 14.0);
    RenderImage(IMAGE_TABLE_TOP_RIGHT, (x + width) - 14.f, y, 14.0, 14.0);
    RenderImage(IMAGE_TABLE_BOTTOM_LEFT, x, (y + height) - 14.f, 14.0, 14.0);
    RenderImage(IMAGE_TABLE_BOTTOM_RIGHT, (x + width) - 14.f, (y + height) - 14.f, 14.0, 14.0);
    RenderImage(IMAGE_TABLE_TOP_PIXEL, x + 6.f, y, (width - 12.f), 14.0);
    RenderImage(IMAGE_TABLE_RIGHT_PIXEL, (x + width) - 14.f, y + 6.f, 14.0, (height - 14.f));
    RenderImage(IMAGE_TABLE_BOTTOM_PIXEL, x + 6.f, (y + height) - 14.f, (width - 12.f), 14.0);
    RenderImage(IMAGE_TABLE_LEFT_PIXEL, x, (y + 6.f), 14.0, (height - 14.f));
}

void SEASON3B::CNewUIMuHelper::LoadImages()
{
    LoadBitmap("Interface\\MacroUI\\MacroUI_RangeMinus.tga", IMAGE_MACROUI_HELPER_RAGEMINUS, GL_LINEAR, GL_CLAMP, 1, 0);
    LoadBitmap("Interface\\MacroUI\\MacroUI_OptionButton.tga", IMAGE_MACROUI_HELPER_OPTIONBUTTON, GL_LINEAR, GL_CLAMP, 1, 0);
    LoadBitmap("Interface\\MacroUI\\MacroUI_InputNumber.tga", IMAGE_MACROUI_HELPER_INPUTNUMBER, GL_LINEAR, GL_CLAMP, 1, 0);
    LoadBitmap("Interface\\MacroUI\\MacroUI_InputString.tga", IMAGE_MACROUI_HELPER_INPUTSTRING, GL_LINEAR, GL_CLAMP, 1, 0);
    //--
    LoadBitmap("Interface\\InGameShop\\Ingame_Bt03.tga", IMAGE_IGS_BUTTON, GL_LINEAR, GL_CLAMP, 1, 0);
}

void SEASON3B::CNewUIMuHelper::UnloadImages()
{
    DeleteBitmap(IMAGE_MACROUI_HELPER_RAGEMINUS);
    DeleteBitmap(IMAGE_MACROUI_HELPER_OPTIONBUTTON);
    DeleteBitmap(IMAGE_MACROUI_HELPER_INPUTNUMBER);
    DeleteBitmap(IMAGE_MACROUI_HELPER_INPUTSTRING);
    //--
    DeleteBitmap(IMAGE_IGS_BUTTON);
}

//===============================================================================================================
//===============================================================================================================

void SEASON3B::CNewUIMuHelper::RegisterButton(int Identificador, CButtonTap button)
{
    m_ButtonList.insert(std::pair<int, CButtonTap>(Identificador, button));
}

void SEASON3B::CNewUIMuHelper::RegisterBtnCharacter(BYTE class_character, int Identificador)
{
    auto li = m_ButtonList.find(Identificador);

    if (li != m_ButtonList.end())
    {
        CButtonTap* cBTN = &li->second;
        if (class_character >= 0 && class_character < MAX_CLASS)
        {
            cBTN->class_character[class_character] = TRUE;
        }
        else
        {
            memset(cBTN->class_character, 1, sizeof(cBTN->class_character));
        }
    }
}

void SEASON3B::CNewUIMuHelper::InsertButton(int imgindex, int x, int y, int sx, int sy, bool overflg, bool isimgwidth, bool bClickEffect, bool MoveTxt, unicode::t_string btname, unicode::t_string tooltiptext, int Identificador, int iNumTab)
{
    CButtonTap cBTN;
    auto* button = new CNewUIButton();

    button->ChangeButtonImgState(1, imgindex, overflg, isimgwidth, bClickEffect);
    button->ChangeButtonInfo(x, y, sx, sy);

    button->ChangeText(btname);
    button->ChangeToolTipText(tooltiptext, TRUE);

    if (MoveTxt)
    {
        button->MoveTextPos(0, -1);
    }

    cBTN.btn = button;
    cBTN.iNumTab = iNumTab;
    memset(cBTN.class_character, 0, sizeof(cBTN.class_character));

    RegisterButton(Identificador, cBTN);
}

void SEASON3B::CNewUIMuHelper::RenderBtnList()
{
    auto li = m_ButtonList.begin();

    for (; li != m_ButtonList.end(); li++)
    {
        CButtonTap* cBTN = &li->second;

        if ((cBTN->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBTN->iNumTab == m_iNumCurOpenTab || cBTN->iNumTab == -1))
        {
            cBTN->btn->Render();
        }
    }
}
void SEASON3B::CNewUIMuHelper::UpdateBtnData(int NumBtn)
{
	//g_ConsoleDebug->Write(MCD_ERROR, "UpdateBtnData %d", NumBtn);
	//g_pBCustomMenuInfo->WindowMenuCustom->onShow ^= 1;

	switch (NumBtn)
	{
	case 0:
	{
		if (NumBtn == 0 && this->DataAutoMu.Range[0] < 6) this->DataAutoMu.Range[0]++;
	}
	break;
	//===COnfig hoi mau
	case 4: //Elf
	case 5: //Sum
	case 6:
	//===BUff Nhom
	case 7:
	case 8:
	//===Config Kich hoat ky nang
	case 2:
	case 3:
	{
		if (gInterface.Data[eMuHelperConfig].OnShow)
		{
			if (gInterface.Data[eMuHelperConfig].Type != NumBtn)
			{
				gInterface.Data[eMuHelperConfig].Type = NumBtn;
			}
			else
			{
				gInterface.Data[eMuHelperConfig].OpenClose(NumBtn);
			}
			
		}
		else
		{
			gInterface.Data[eMuHelperConfig].OpenClose(NumBtn);
		}
		
	}
	break;
	case 1:
	{
		if (NumBtn == 1 && this->DataAutoMu.Range[0] > 1) this->DataAutoMu.Range[0]--;
	}
	break;
	case 9:
	{
		if (this->DataAutoMu.Range[1] < 8) this->DataAutoMu.Range[1]++;
	}
	break;

	case 10:
	{
		if (this->DataAutoMu.Range[1] > 1) this->DataAutoMu.Range[1]--;
	}
	break;
	case 11: //input name item 
	{
		WCHAR oText[16];
		::memset(&oText, 0, sizeof(char) * 16);
		m_ItemName->GetText(oText);
		std::string szText = "";
		g_pMultiLanguage->ConvertWideCharToStr(szText, oText, CP_UTF8);
		for (int n = 0; n < MAX_HELPER_ITEM; n++)
		{
			if(strlen(this->DataAutoMu.ItemList[n]) < 1)
			{
				::memcpy(this->DataAutoMu.ItemList[n], strdup(szText.c_str()), 16);
				break;
			}
		}	
		m_ItemName->SetText("");
	}
	break;
	case 12: //clear
	{
		::memset(&this->DataAutoMu.ItemList[SelectitemList], 0, sizeof(this->DataAutoMu.ItemList[SelectitemList]));
	}
	break;
	
	case 14:
	{
		this->ResetConfig();
	}
	break;
	case 13:
	{
		this->SaveConfig();
		g_pNewUISystem->Hide(INTERFACE_MuHelper);
	}
	break;
	default:
		break;
	}
}
int SEASON3B::CNewUIMuHelper::UpdateMouseBtnList()
{
    int Identificador = -1;

    auto li = m_ButtonList.begin();

    for (; li != m_ButtonList.end(); li++)
    {
        CButtonTap* cBTN = &li->second;

        if ((cBTN->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBTN->iNumTab == m_iNumCurOpenTab || cBTN->iNumTab == -1))
        {
            if (cBTN->btn->UpdateMouseEvent())
            {
				
				UpdateBtnData(li->first);
                return li->first;
            }
        }
    }
    return Identificador;
}

//===============================================================================================================
//===============================================================================================================

void SEASON3B::CNewUIMuHelper::RegisterBoxCharacter(BYTE class_character, int Identificador)
{
    auto li = m_CheckBoxList.find(Identificador);

    if (li != m_CheckBoxList.end())
    {
        CheckBoxTap* cBOX = &li->second;

        if (class_character >= 0 && class_character < MAX_CLASS)
        {
            cBOX->class_character[class_character] = TRUE;
        }
        else
        {
            memset(cBOX->class_character, 1, sizeof(cBOX->class_character));
        }
    }
}

void SEASON3B::CNewUIMuHelper::RegisterCheckBox(int Identificador, CheckBoxTap button)
{
    m_CheckBoxList.insert(std::pair<int, CheckBoxTap>(Identificador, button));
}

void SEASON3B::CNewUIMuHelper::InsertCheckBox(int imgindex, int x, int y, int sx, int sy, bool overflg, unicode::t_string btname, int Identificador, int iNumTab)
{
    CheckBoxTap cBOX;

    auto* cbox = new CNewUICheckBox;

    cbox->CheckBoxImgState(imgindex);
    cbox->CheckBoxInfo(x, y, sx, sy);

    cbox->ChangeText(btname);
    cbox->RegisterBoxState(overflg);

    cBOX.box = cbox;
    cBOX.iNumTab = iNumTab;
    memset(cBOX.class_character, 0, sizeof(cBOX.class_character));

    RegisterCheckBox(Identificador, cBOX);
}

void SEASON3B::CNewUIMuHelper::RenderBoxList()
{
    auto li = m_CheckBoxList.begin();

    for (; li != m_CheckBoxList.end(); li++)
    {
        CheckBoxTap* cBOX = &li->second;

        if ((cBOX->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBOX->iNumTab == m_iNumCurOpenTab || cBOX->iNumTab == -1))
        {
            cBOX->box->Render();
        }
    }
}

void SEASON3B::CNewUIMuHelper::SetCheckBox(int Number, bool State)
{
	std::map<int, CheckBoxTap>::iterator it = this->m_CheckBoxList.find(Number);

	if (it != this->m_CheckBoxList.end())
	{
		it->second.box->RegisterBoxState(State);
	}
}
void SEASON3B::CNewUIMuHelper::UpdateCheckBoxData(int NumBtn,int State)
{
	//g_ConsoleDebug->Write(MCD_ERROR, "UpdateCheckBoxData %d State (%d)", NumBtn, State);
	
	switch (NumBtn)
	{
	case 0:
	{
		this->DataAutoMu.AutoPotion = State;
	}
	break;
	case 1:
	{
		this->DataAutoMu.LongDistance = State;
	}
	break;
	case 2:
	{
		this->DataAutoMu.OriginalPosition = State;
	}
	break;
	case 3:
	{
		////g_ConsoleDebug->Write(MCD_ERROR, "SkillSecond1ID %d State (%d)", this->DataAutoMu.Skill[1], State);
		if (this->DataAutoMu.Skill[1] > 0)
		{
			this->DataAutoMu.SkillDelay[0] = State;
			this->DataAutoMu.SkillCondition[0] = 0;
			this->SetCheckBox(4, 0);
		}
		else
		{
			this->DataAutoMu.SkillDelay[0] = 0;
			this->DataAutoMu.SkillCondition[0] = 0;
			this->SetCheckBox(3, 0);
			this->SetCheckBox(4, 0);
		}	
	}
	break;
	case 4:
	{
		if (this->DataAutoMu.Skill[1] > 0)
		{
			this->DataAutoMu.SkillDelay[0] = 0;
			this->DataAutoMu.SkillCondition[0] = State;
			this->SetCheckBox(3, 0);
		}
		else
		{
			this->DataAutoMu.SkillDelay[0] = 0;
			this->DataAutoMu.SkillCondition[0] = 0;
			this->SetCheckBox(3, 0);
			this->SetCheckBox(4, 0);
		}
	}
	break;
	case 5:
	{
		if (this->DataAutoMu.Skill[2] > 0)
		{
			this->DataAutoMu.SkillDelay[1] = State;
			this->DataAutoMu.SkillCondition[1] = 0;
			this->SetCheckBox(6, 0);
		}
		else
		{
			this->DataAutoMu.SkillDelay[1] = 0;
			this->DataAutoMu.SkillCondition[1] = 0;
			this->SetCheckBox(5, 0);
			this->SetCheckBox(6, 0);
		}
	}
	break;
	case 6:
	{
		if (this->DataAutoMu.Skill[2] > 0)
		{
			this->DataAutoMu.SkillDelay[1] = 0;
			this->DataAutoMu.SkillCondition[1] = State;
			this->SetCheckBox(5, 0);
		}
		else
		{
			this->DataAutoMu.SkillDelay[1] = 0;
			this->DataAutoMu.SkillCondition[1] = 0;
			this->SetCheckBox(5, 0);
			this->SetCheckBox(6, 0);
		}
	}
	break;
	case 7:
	{
		this->DataAutoMu.Combo = State;
	}
	break;
	case 8:
	{
		this->DataAutoMu.AutoBuff = State;
	}
	break;
	case 9:
	{
		this->DataAutoMu.DarkSpirit = State;
	}
	break;
	case 10:
	{
		this->DataAutoMu.Party = State;
	}
	break;
	//==Heal
	case 11:
	{
		this->DataAutoMu.AutoHeal = State;
	}
	break;
	//==Rut mau Sum
	case 12:
	{
		this->DataAutoMu.AutoHeal = State;
	}
	break;
	case 13:
	{
		this->DataAutoMu.RepairItem = State;
	}
	break;
	case 14:
	{
		this->DataAutoMu.PickAllItem = State;
		if (this->DataAutoMu.PickAllItem > 0)
		{
			this->DataAutoMu.PickSelected = 0;
			this->SetCheckBox(15, 0);
		}
	}
	break;
	case 15:
	{
		this->DataAutoMu.PickSelected = State;
		if (this->DataAutoMu.PickSelected > 0)
		{
			this->DataAutoMu.PickAllItem = 0;
			this->SetCheckBox(14, 0);
		}
	}
	break;
	case 16:
	{
		this->DataAutoMu.PickJewel = State;
	}
	break;
	case 17:
	{
		this->DataAutoMu.PickSet = State;
	}
	break;
	case 18:
	{
		this->DataAutoMu.PickMoney = State;
	}
	break;
	case 19:
	{
		this->DataAutoMu.PickExc = State;
	}
	break;
	case 20:
	{
		this->DataAutoMu.PickExtra = State;
	}
	break;
	default:
		break;
	}
}
int SEASON3B::CNewUIMuHelper::UpdateMouseBoxList()
{
    int Identificador = -1;

    auto li = m_CheckBoxList.begin();

    for (; li != m_CheckBoxList.end(); li++)
    {
        CheckBoxTap* cBOX = &li->second;

        if ((cBOX->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cBOX->iNumTab == m_iNumCurOpenTab || cBOX->iNumTab == -1))
        {
            if (cBOX->box->UpdateMouseEvent())
            {
				
				UpdateCheckBoxData(li->first, cBOX->box->GetBoxState());
                return li->first;
            }
        }
    }
    return Identificador;
}

//===============================================================================================================
//===============================================================================================================

void SEASON3B::CNewUIMuHelper::RenderTextureList()
{
    auto li = m_TextureList.begin();

    for (; li != m_TextureList.end(); li++)
    {
        cTexture* cImage = &li->second;

        if ((cImage->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cImage->iNumTab == m_iNumCurOpenTab || cImage->iNumTab == -1))
        {
            RenderImage(cImage->s_ImgIndex, cImage->m_Pos.x, cImage->m_Pos.y, cImage->m_Size.x, cImage->m_Size.y);
        }
    }
}

int SEASON3B::CNewUIMuHelper::UpdateTextureList()
{
    int Identificador = -1;

    auto li = m_TextureList.begin();

    for (; li != m_TextureList.end(); li++)
    {
        cTexture* cImage = &li->second;

        if ((cImage->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cImage->iNumTab == m_iNumCurOpenTab || cImage->iNumTab == -1))
        {
            if (CheckMouseIn(cImage->m_Pos.x, cImage->m_Pos.y, cImage->m_Size.x, cImage->m_Size.y))
            {
                return li->first;
            }
        }
    }

    return Identificador;
}

void SEASON3B::CNewUIMuHelper::RegisterTextureCharacter(BYTE class_character, int Identificador)
{
    auto li = m_TextureList.find(Identificador);

    if (li != m_TextureList.end())
    {
        cTexture* cImage = &li->second;

        if (class_character >= 0 && class_character < MAX_CLASS)
        {
            cImage->class_character[class_character] = TRUE;
        }
        else
        {
            memset(cImage->class_character, 1, sizeof(cImage->class_character));
        }
    }
}

void SEASON3B::CNewUIMuHelper::RegisterTexture(int Identificador, cTexture button)
{
    m_TextureList.insert(std::pair<int, cTexture>(Identificador, button));
}

void SEASON3B::CNewUIMuHelper::InsertTexture(int imgindex, int x, int y, int sx, int sy, int Identificador, int iNumTab)
{
    cTexture cImage;

    cImage.s_ImgIndex = imgindex;
    cImage.m_Pos.x = x;
    cImage.m_Pos.y = y;
    cImage.m_Size.x = sx;
    cImage.m_Size.y = sy;
    cImage.iNumTab = iNumTab;

    memset(cImage.class_character, 0, sizeof(cImage.class_character));

    RegisterTexture(Identificador, cImage);
}

//===============================================================================================================
//===============================================================================================================

void SEASON3B::CNewUIMuHelper::RenderTextList()
{
    auto li = m_TextNameList.begin();

    for (; li != m_TextNameList.end(); li++)
    {
        cTextName* cImage = &li->second;

        if ((cImage->class_character[gCharacterManager.GetBaseClass(Hero->Class)]) && (cImage->iNumTab == m_iNumCurOpenTab || cImage->iNumTab == -1))
        {
            g_pRenderText->RenderText(cImage->m_Pos.x, cImage->m_Pos.y, cImage->m_Name.c_str());
        }
    }
}

void SEASON3B::CNewUIMuHelper::RegisterTextCharacter(BYTE class_character, int Identificador)
{
    auto li = m_TextNameList.find(Identificador);

    if (li != m_TextNameList.end())
    {
        cTextName* cImage = &li->second;

        if (class_character >= 0 && class_character < MAX_CLASS)
        {
            cImage->class_character[class_character] = TRUE;
        }
        else
        {
            memset(cImage->class_character, 1, sizeof(cImage->class_character));
        }
    }
}

void SEASON3B::CNewUIMuHelper::RegisterTextur(int Identificador, cTextName button)
{
    m_TextNameList.insert(std::pair<int, cTextName>(Identificador, button));
}

void SEASON3B::CNewUIMuHelper::InsertText(int x, int y, unicode::t_string Name, int Identificador, int iNumTab)
{
    cTextName cText;

    cText.m_Pos.x = x;
    cText.m_Pos.y = y;
    cText.m_Name = Name;
    cText.iNumTab = iNumTab;

    memset(cText.class_character, 0, sizeof(cText.class_character));
    RegisterTextur(Identificador, cText);
}
void SEASON3B::CNewUIMuHelper::SetConfigData()
{
	this->SetCheckBox(0, this->DataAutoMu.AutoPotion);
	this->SetCheckBox(1, this->DataAutoMu.LongDistance);
	this->SetCheckBox(2, this->DataAutoMu.OriginalPosition);
	this->SetCheckBox(3, this->DataAutoMu.SkillDelay[0]);
	this->SetCheckBox(4, this->DataAutoMu.SkillCondition[0]);
	this->SetCheckBox(5, this->DataAutoMu.SkillDelay[1]);
	this->SetCheckBox(6, this->DataAutoMu.SkillCondition[1]);
	this->SetCheckBox(7, this->DataAutoMu.Combo);
	this->SetCheckBox(8, this->DataAutoMu.AutoBuff);
	this->SetCheckBox(9, this->DataAutoMu.DarkSpirit);
	this->SetCheckBox(10, this->DataAutoMu.Party);
	this->SetCheckBox(11, this->DataAutoMu.AutoHeal);
	this->SetCheckBox(12, this->DataAutoMu.AutoHeal);
	this->SetCheckBox(13, this->DataAutoMu.RepairItem);
	this->SetCheckBox(14, this->DataAutoMu.PickAllItem);
	this->SetCheckBox(15, this->DataAutoMu.PickSelected);
	this->SetCheckBox(16, this->DataAutoMu.PickJewel);
	this->SetCheckBox(17, this->DataAutoMu.PickSet);
	this->SetCheckBox(18, this->DataAutoMu.PickMoney);
	this->SetCheckBox(19, this->DataAutoMu.PickExc);
	this->SetCheckBox(20, this->DataAutoMu.PickExtra);
}
void SEASON3B::CNewUIMuHelper::LoadConfig(BYTE* Data)
{
	//g_ConsoleDebug->Write(MCD_ERROR, "DEBUG CNewUIMacro::LoadConfig %d", sizeof(Data));
	::memset(&this->DataAutoMu, 0, sizeof(DataAutoMu));
	::memset(&this->Helper, 0, sizeof(Helper));
	this->DataAutoMu.Clear();
	this->Helper.Clear();
	//return;
	BYTE v6[256] = { 0 };
	memcpy_s(v6, sizeof(v6), Data, sizeof(v6));
	if (sizeof(Data) == 255)
	{
		this->ResetConfig();
	}
	else
	{
		
		HELPER_INFO* lpMsg = (HELPER_INFO*)Data;
		this->Helper.Range[0] = lpMsg->Range & 0x0F;
		this->Helper.MoveTime = lpMsg->MovementTime;
		this->Helper.Skill[0] = lpMsg->SkillAttack1;
		this->Helper.Skill[1] = lpMsg->SkillAttack2;
		this->Helper.Skill[2] = lpMsg->SkillAttack3;
		this->Helper.Delay[0] = lpMsg->SkillDelay1;
		this->Helper.Delay[1] = lpMsg->SkillDelay2;
		this->Helper.Buff[0] = lpMsg->SkillBuff[0];
		this->Helper.Buff[1] = lpMsg->SkillBuff[1];
		this->Helper.Buff[2] = lpMsg->SkillBuff[2];
		this->Helper.PotPercent = 10 * (lpMsg->PercentFlag & 0x0F);
		this->Helper.HealPercent = 10 * ((lpMsg->PercentFlag >> 4) & 0x0F);
		this->Helper.DrainPercent = 10 * ((lpMsg->PercentFlag >> 8) & 0x0F);
		this->Helper.PartyHealPercent = 10 * ((lpMsg->PercentFlag >> 12) & 0x0F);
		this->Helper.PartyBuffTime = lpMsg->BuffPartyTime;

		this->Helper.Range[1] = (lpMsg->Range >> 4) & 0x0F;
		this->Helper.PickJewel = (lpMsg->PickFlag >> 3) & 1;
		this->Helper.PickExc = (lpMsg->PickFlag >> 4) & 1;
		this->Helper.PickSet = (lpMsg->PickFlag >> 5) & 1;
		this->Helper.PickMoney = (lpMsg->PickFlag >> 6) & 1;
		this->Helper.PickExtra = (lpMsg->PickFlag >> 7) & 1;

		memset(this->Helper.ItemList, 0, sizeof(this->Helper.ItemList));
		CountItemList = 0;
		for (int n = 0; n < MAX_HELPER_ITEM; n++)
		{
			//memcpy(this->Helper.ItemList[n], CharToLower(lpMsg->ItemList[n]), sizeof(this->Helper.ItemList[n]));
			memcpy(this->Helper.ItemList[n], (lpMsg->ItemList[n]), sizeof(this->Helper.ItemList[n]));
			if (strlen(this->Helper.ItemList[n]) > 1) CountItemList++;
		}

		this->Helper.AutoPotion = lpMsg->OptionFlag & 1;
		this->Helper.AutoHeal = (lpMsg->OptionFlag >> 1) & 1;
		this->Helper.AutoDrainLife = (lpMsg->OptionFlag >> 2) & 1;
		this->Helper.LongDistance = (lpMsg->OptionFlag >> 3) & 1;
		this->Helper.OriginalPosition = (lpMsg->OptionFlag >> 4) & 1;
		this->Helper.Combo = (lpMsg->OptionFlag >> 5) & 1;
		this->Helper.Party = (lpMsg->OptionFlag >> 6) & 1;
		this->Helper.PartyAutoHeal = (lpMsg->OptionFlag >> 7) & 1;
		this->Helper.PartyAutoBuff = (lpMsg->OptionFlag >> 8) & 1;
		this->Helper.DarkSpirit = (lpMsg->OptionFlag >> 9) & 1;
		this->Helper.AutoBuff = (lpMsg->OptionFlag >> 10) & 1;
		this->Helper.SkillDelay[0] = (lpMsg->OptionFlag >> 11) & 1;
		this->Helper.SkillCondition[0] = (lpMsg->OptionFlag >> 12) & 1;
		this->Helper.SkillPreCon[0] = ((lpMsg->OptionFlag >> 13) & 1) % 2;
		this->Helper.SkillSubCon[0] = ((lpMsg->OptionFlag >> 8) >> 6) % 4;
		this->Helper.SkillDelay[1] = (lpMsg->OptionFlag >> 16) & 1;
		this->Helper.SkillCondition[1] = (lpMsg->OptionFlag >> 17) & 1;
		this->Helper.SkillPreCon[1] = ((lpMsg->OptionFlag >> 18) & 1) % 2;
		this->Helper.SkillSubCon[1] = ((lpMsg->OptionFlag >> 13) >> 6) % 4;
		this->Helper.RepairItem = (lpMsg->OptionFlag >> 21) & 1;
		this->Helper.PickAllItem = (lpMsg->OptionFlag >> 22) & 1;
		this->Helper.PickSelected = (lpMsg->OptionFlag >> 23) & 1;
		this->Helper.DarkSpiritAuto = (lpMsg->OptionFlag >> 24) & 1;
		this->Helper.DarkSpiritAttack = (lpMsg->OptionFlag >> 25) & 1;
		this->Helper.AutoAcceptFriend = (lpMsg->OptionFlag >> 26) & 1;
		this->Helper.AutoAcceptGuild = (lpMsg->OptionFlag >> 27) & 1;
		this->Helper.PotionElite = (lpMsg->OptionFlag >> 28) & 1;
		this->Helper.ShortDistance = (lpMsg->OptionFlag >> 29) & 1;
		this->Helper.RegularAttack = (lpMsg->OptionFlag >> 30) & 1;

		this->DataAutoMu = this->Helper;
	}
	this->SetConfigData();
}
HELPER_INFO ConvertToHelperInfo(const HELPER_STRUCT& helperStruct)
{
	HELPER_INFO result;

	result.PickFlag = 0;
	result.Range = (helperStruct.Range[0] & 0x0F) | ((helperStruct.Range[1] & 0x0F) << 4);
	result.MovementTime = static_cast<WORD>(helperStruct.MoveTime);
	result.SkillAttack1 = static_cast<WORD>(helperStruct.Skill[0]);
	result.SkillAttack2 = static_cast<WORD>(helperStruct.Skill[1]);
	result.SkillAttack3 = static_cast<WORD>(helperStruct.Skill[2]);
	result.SkillDelay1 = static_cast<WORD>(helperStruct.Delay[0]);
	result.SkillDelay2 = static_cast<WORD>(helperStruct.Delay[1]);
	result.SkillBuff[0] = static_cast<WORD>(helperStruct.Buff[0]);
	result.SkillBuff[1] = static_cast<WORD>(helperStruct.Buff[1]);
	result.SkillBuff[2] = static_cast<WORD>(helperStruct.Buff[2]);
	result.PercentFlag = (helperStruct.PotPercent / 10) | ((helperStruct.HealPercent / 10) << 4) |
		((helperStruct.DrainPercent / 10) << 8) | ((helperStruct.PartyHealPercent / 10) << 12);
	result.BuffPartyTime = static_cast<WORD>(helperStruct.PartyBuffTime);

	result.PickFlag |= (helperStruct.PickJewel & 1) << 3;
	result.PickFlag |= (helperStruct.PickExc & 1) << 4;
	result.PickFlag |= (helperStruct.PickSet & 1) << 5;
	result.PickFlag |= (helperStruct.PickMoney & 1) << 6;
	result.PickFlag |= (helperStruct.PickExtra & 1) << 7;

	for (int n = 0; n < MAX_HELPER_ITEM; n++)
	{
		strncpy(result.ItemList[n], helperStruct.ItemList[n], sizeof(result.ItemList[n]) - 1);
		result.ItemList[n][sizeof(result.ItemList[n]) - 1] = '\0';
	}

	result.OptionFlag = 0;
	result.OptionFlag |= helperStruct.AutoPotion & 1;
	result.OptionFlag |= (helperStruct.AutoHeal & 1) << 1;
	result.OptionFlag |= (helperStruct.AutoDrainLife & 1) << 2;
	result.OptionFlag |= (helperStruct.LongDistance & 1) << 3;
	result.OptionFlag |= (helperStruct.OriginalPosition & 1) << 4;
	result.OptionFlag |= (helperStruct.Combo & 1) << 5;
	result.OptionFlag |= (helperStruct.Party & 1) << 6;
	result.OptionFlag |= (helperStruct.PartyAutoHeal & 1) << 7;
	result.OptionFlag |= (helperStruct.PartyAutoBuff & 1) << 8;
	result.OptionFlag |= (helperStruct.DarkSpirit & 1) << 9;
	result.OptionFlag |= (helperStruct.AutoBuff & 1) << 10;
	result.OptionFlag |= (helperStruct.SkillDelay[0] & 1) << 11;
	result.OptionFlag |= (helperStruct.SkillCondition[0] & 1) << 12;
	result.OptionFlag |= ((helperStruct.SkillPreCon[0] % 2) & 1) << 13;
	result.OptionFlag |= ((helperStruct.SkillSubCon[0] % 4) << 6) << 8;
	result.OptionFlag |= (helperStruct.SkillDelay[1] & 1) << 16;
	result.OptionFlag |= (helperStruct.SkillCondition[1] & 1) << 17;
	result.OptionFlag |= ((helperStruct.SkillPreCon[1] % 2) & 1) << 18;
	result.OptionFlag |= ((helperStruct.SkillSubCon[1] % 4) << 6) << 13;
	result.OptionFlag |= (helperStruct.RepairItem & 1) << 21;
	result.OptionFlag |= (helperStruct.PickAllItem & 1) << 22;
	result.OptionFlag |= (helperStruct.PickSelected & 1) << 23;
	result.OptionFlag |= (helperStruct.DarkSpiritAuto & 1) << 24;
	result.OptionFlag |= (helperStruct.DarkSpiritAttack & 1) << 25;
	result.OptionFlag |= (helperStruct.AutoAcceptFriend & 1) << 26;
	result.OptionFlag |= (helperStruct.AutoAcceptGuild & 1) << 27;
	result.OptionFlag |= (helperStruct.PotionElite & 1) << 28;
	result.OptionFlag |= (helperStruct.ShortDistance & 1) << 29;
	result.OptionFlag |= (helperStruct.RegularAttack & 1) << 30;

	return result;
}

void SEASON3B::CNewUIMuHelper::SaveConfig()
{
	this->Helper = this->DataAutoMu;
	HELPER_INFO helperInfo = ConvertToHelperInfo(this->Helper);
	CGSaveDataMuHelper((BYTE*)&helperInfo);
	//memcpy(&this->Helper, &this->DataAutoMu, sizeof(this->Helper));
}
void SEASON3B::CNewUIMuHelper::ResetConfig()
{
	memset(&this->DataAutoMu, 0, sizeof(this->DataAutoMu));
	this->DataAutoMu.Clear();
	// *((_DWORD *)this + 43) = 1;
	this->DataAutoMu.Range[0] = 1;
	this->DataAutoMu.AutoPotion = 0;
	this->DataAutoMu.PotPercent = 50;
	this->DataAutoMu.AutoHeal = 0;
	this->DataAutoMu.HealPercent = 50;
	this->DataAutoMu.AutoDrainLife = 0;
	this->DataAutoMu.DrainPercent = 50;
	this->DataAutoMu.LongDistance = 0;
	this->DataAutoMu.OriginalPosition = 0;
	//*((_DWORD *)this + 46) = 0;
	//*((_DWORD *)this + 47) = 0;
	this->DataAutoMu.Combo = 0;
	this->DataAutoMu.Party = 0;
	this->DataAutoMu.PartyAutoHeal = 0;
	this->DataAutoMu.PartyHealPercent = 50;
	this->DataAutoMu.PartyAutoBuff = 0;
	//*((_DWORD *)this + 62) = 0;
	this->DataAutoMu.DarkSpiritAuto = 0;
	this->DataAutoMu.DarkSpiritAttack = 0;
	//*((_DWORD *)this + 72) = 1;
	this->DataAutoMu.Range[1] = 1; //Range pick up
	this->DataAutoMu.RepairItem = 0;
	this->DataAutoMu.PickAllItem = 0;
	this->DataAutoMu.PickSelected = 1;
	this->DataAutoMu.PickJewel = 0;
	this->DataAutoMu.PickSet = 0;
	this->DataAutoMu.PickMoney = 0;
	this->DataAutoMu.PickExc = 0;
	this->DataAutoMu.PickExtra = 0;
	this->Helper = this->DataAutoMu;
	this->SetConfigData();
}