#include "stdafx.h"
#include "WinMain.h"
#include "wsclientinline.h"
#include "ZzzTexture.h"
#include "_TextureIndex.h"
#include "CSItemOption.h"
#include "UIControls.h"
#include "NewUISystem.h"
#include "NewUIMasterSkillTree.h"
#include "CharacterManager.h"
#include "NewUICustomMessageBox.h"

#if(NEW_MASTER_SKILL_TREE == 1)
extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayHeight;
extern int DisplayWinMid;
extern int DisplayHeightExt;
extern int DisplayWinExt;
extern int DisplayWinReal;
//MAX_MAGIC -> aumentar a 150
// MAX_SKILLS -> AUMENTAR A 650

_MASTER_SKILLTREE_DATA m_stMasterSkillTreeData[MAX_MASTER_SKILL_DATA];
_MASTER_SKILL_TOOLTIP m_stMasterSkillTooltip[MAX_MASTER_SKILL_DATA];

BOOL  InBounds(int a1, int  a2, int a3)
{
	return a1 >= (int)a2 && a1 <= (int)a3;
}

SEASON3B::CNewUIMasterSkillTree::CNewUIMasterSkillTree()
{
	this->pBitMap[0] = NULL;

	this->pBitMap[1] = NULL;

	this->ConsumePoint = 0;

	this->CurSkillID = 0;

	this->classCode = 0;

	this->CategoryTextIndex = 0;

	this->categoryPos[0] = { 11 + DisplayWinExt,55 };

	this->categoryPos[1] = { 221 + DisplayWinExt,55 };

	this->categoryPos[2] = { 431 + DisplayWinExt,55 };

	this->InitMasterSkillPoint();

	this->ClearSkillTreeInfo();

	this->ClearSkillTreeData();

	this->ClearSkillTooltipData();
}

SEASON3B::CNewUIMasterSkillTree::~CNewUIMasterSkillTree()
{
	this->Release();
}

void SEASON3B::CNewUIMasterSkillTree::ClearSkillTreeInfo()
{
	this->map_skilltreeinfo.clear();
}

BYTE SEASON3B::CNewUIMasterSkillTree::GetConsumePoint()
{
	return this->ConsumePoint;
}

int SEASON3B::CNewUIMasterSkillTree::GetCurSkillID()
{
	return this->CurSkillID;
}

bool SEASON3B::CNewUIMasterSkillTree::Create()
{
	g_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MASTER_LEVEL, this);

	this->SetPos();

	this->LoadImages();

	this->m_CloseBT.ChangeButtonImgState(1, IMAGE_MASTER_INTERFACE + 5, 0, 0, 0);
	this->m_CloseBT.ChangeButtonInfo(611+ DisplayWinExt, 9, 13, 14);
	this->m_CloseBT.ChangeToolTipText(GlobalText[1002]);

	
	//this->m_ClearBT.ChangeButtonImgState(1, CNewUICharacterInfoWindow::IMAGE_CHAINFO_BTN_STAT_TRU, 0, 0, 0);
	//this->m_ClearBT.ChangeButtonInfo(611 + DisplayWinExt, 24, 13, 14);
	//this->m_ClearBT.ChangeToolTipText("Clear master");


	for (int i = 0; i < MAX_MASTER_SKILL_CATEGORY; i++)
	{
		this->ButtonX[i] = 0;

		this->ButtonY[i] = 0;
	}

	return true;
}

void SEASON3B::CNewUIMasterSkillTree::Release()
{
	this->UnloadImages();

	g_pNewUIMng->RemoveUIObj(this);

	this->ClearSkillTreeInfo();

	this->ClearSkillTreeData();

	this->ClearSkillTooltipData();
}

void SEASON3B::CNewUIMasterSkillTree::SetPos()
{
	this->PosX = DisplayWinExt;

	this->PosY = 0;

	this->width = 640;

	this->height = 428;
}

void SEASON3B::CNewUIMasterSkillTree::OpenMasterSkillTreeData(char* path)
{
	memset(m_stMasterSkillTreeData, 0, sizeof(m_stMasterSkillTreeData));

	FILE* fp = fopen(path, "rb");

	char Text[256];

	if (fp == NULL)
	{
		sprintf(Text, "%s - File not exist.", path);
		g_ErrorReport.Write(Text);
		MessageBox(g_hWnd, Text, NULL, MB_OK);
		SendMessage(g_hWnd, WM_DESTROY, 0, 0);
		return;
	}

	int Size = sizeof(_MASTER_SKILLTREE_DATA);

	BYTE* Buffer = new BYTE[Size * MAX_MASTER_SKILL_DATA];

	fread(Buffer, Size * MAX_MASTER_SKILL_DATA, 1, fp);

	DWORD dwCheckSum;

	fread(&dwCheckSum, sizeof(DWORD), 1u, fp);

	fclose(fp);

	//if (dwCheckSum != GenerateCheckSum2(Buffer, 12288, 0x2BC1))
	//{
	//	sprintf(Text, "%s - File corrupted.", path);
	//	g_ErrorReport.Write(Text);
	//	MessageBox(g_hWnd, Text, NULL, MB_OK);
	//	SendMessage(g_hWnd, WM_DESTROY, 0, 0);
	//	return;
	//}

	BYTE* pSeek = Buffer;

	for (int i = 0; i < MAX_MASTER_SKILL_DATA; i++)
	{
		BuxConvert(pSeek, Size);

		memcpy(&m_stMasterSkillTreeData[i], pSeek, Size);
		pSeek += Size;

		if (pSeek == NULL)
		{
			break;
		}
	}

	delete[] Buffer;
}

void SEASON3B::CNewUIMasterSkillTree::OpenMasterSkillTooltip(char* path)
{
	memset(m_stMasterSkillTooltip, 0, sizeof(m_stMasterSkillTooltip));

	FILE* fp = fopen(path, "rb");

	char Text[256];

	if (fp == NULL)
	{
		sprintf(Text, "%s - File not exist.", path);
		g_ErrorReport.Write(Text);
		MessageBox(g_hWnd, Text, NULL, MB_OK);
		SendMessage(g_hWnd, WM_DESTROY, 0, 0);
		return;
	}

	int Size = sizeof(_MASTER_SKILL_TOOLTIP);

	BYTE* Buffer = new BYTE[Size * MAX_MASTER_SKILL_DATA];

	fread(Buffer, Size * MAX_MASTER_SKILL_DATA, 1, fp);

	DWORD dwCheckSum;

	fread(&dwCheckSum, sizeof(DWORD), 1u, fp);

	fclose(fp);

	//if (dwCheckSum != GenerateCheckSum2(Buffer, 12288, 0x2BC1))
	//{
	//	sprintf(Text, "%s - File corrupted.", path);
	//	g_ErrorReport.Write(Text);
	//	MessageBox(g_hWnd, Text, NULL, MB_OK);
	//	SendMessage(g_hWnd, WM_DESTROY, 0, 0);
	//	return;
	//}

	BYTE* pSeek = Buffer;

	for (int i = 0; i < MAX_MASTER_SKILL_DATA; i++)
	{
		BuxConvert(pSeek, Size);

		memcpy(&m_stMasterSkillTooltip[i], pSeek, Size);

		pSeek += Size;

		if (pSeek == NULL)
		{
			break;
		}
	}

	delete[] Buffer;
}

void SEASON3B::CNewUIMasterSkillTree::InitMasterSkillPoint()
{
	for (int i = 0; i < 3; i++)
	{
		this->CategoryPoint[i] = 0;

		for (int k = 0; k < 10; k++)
		{
			this->skillPoint[i][k] = 0;
		}
	}
}

void SEASON3B::CNewUIMasterSkillTree::SetMasterType(BYTE Class)
{
	switch (Class)
	{
	case CLASS_WIZARD:
	case CLASS_SOULMASTER:
	case CLASS_GRANDMASTER:
		this->classCode = 2;
		break;
	case CLASS_KNIGHT:
	case CLASS_BLADEKNIGHT:
	case CLASS_BLADEMASTER:
		this->classCode = 1;
		break;
	case CLASS_ELF:
	case CLASS_MUSEELF:
	case CLASS_HIGHELF:
		this->classCode = 4;
		break;
	case CLASS_DARK:
	case CLASS_DUELMASTER:
		this->classCode = 16;
		break;
	case CLASS_DARK_LORD:
	case CLASS_LORDEMPEROR:
		this->classCode = 32;
		break;
	case CLASS_SUMMONER:
	case CLASS_BLOODYSUMMONER:
	case CLASS_DIMENSIONMASTER:
		this->classCode = 8;
		break;
	case CLASS_RAGEFIGHTER:
	case CLASS_TEMPLENIGHT:
		this->classCode = 64;
		break;
	default:
		break;
	}

	this->SetMasterSkillTreeData();

	this->SetMasterSkillToolTipData();

	switch (Class)
	{
	case CLASS_WIZARD:
	case CLASS_SOULMASTER:
	case CLASS_GRANDMASTER:
		this->CategoryTextIndex = 1751;
		this->ClassNameTextIndex = 1669;
		break;
	case CLASS_KNIGHT:
	case CLASS_BLADEKNIGHT:
	case CLASS_BLADEMASTER:
		this->CategoryTextIndex = 1755;
		this->ClassNameTextIndex = 1668;
		break;
	case CLASS_ELF:
	case CLASS_MUSEELF:
	case CLASS_HIGHELF:
		this->CategoryTextIndex = 1759;
		this->ClassNameTextIndex = 1670;
		break;
	case CLASS_DARK:
	case CLASS_DUELMASTER:
		this->CategoryTextIndex = 1763;
		this->ClassNameTextIndex = 1671;
		break;
	case CLASS_DARK_LORD:
	case CLASS_LORDEMPEROR:
		this->CategoryTextIndex = 1767;
		this->ClassNameTextIndex = 1672;
		break;
	case CLASS_SUMMONER:
	case CLASS_BLOODYSUMMONER:
	case CLASS_DIMENSIONMASTER:
		this->CategoryTextIndex = 3136;
		this->ClassNameTextIndex = 1689;
		break;
	case CLASS_RAGEFIGHTER:
	case CLASS_TEMPLENIGHT:
		this->CategoryTextIndex = 3330;
		this->ClassNameTextIndex = 3151;
		break;
	default:
		return;
	}
}

void SEASON3B::CNewUIMasterSkillTree::SetMasterSkillTreeData()
{
	this->ClearSkillTreeData();

	for (int i = 0; i < MAX_MASTER_SKILL_DATA; i++)
	{
		if (m_stMasterSkillTreeData[i].Index == 0)
		{
			break;
		}

		if ((this->classCode & m_stMasterSkillTreeData[i].ClassCode) == 0)
		{
			continue;
		}

		if (this->map_masterData.insert(std::pair<BYTE, _MASTER_SKILLTREE_DATA>(m_stMasterSkillTreeData[i].Index, m_stMasterSkillTreeData[i])).second == false)
		{
			break;
		}
	}
}

void SEASON3B::CNewUIMasterSkillTree::SetMasterSkillToolTipData()
{
	this->ClearSkillTooltipData();

	for (int i = 0; i < MAX_MASTER_SKILL_DATA; i++)
	{
		if (m_stMasterSkillTooltip[i].SkillIndex == 0)
		{
			break;
		}

		if ((this->classCode & m_stMasterSkillTooltip[i].ClassCode) == 0)
		{
			continue;
		}

		if (this->map_masterSkillToolTip.insert(std::pair<DWORD, _MASTER_SKILL_TOOLTIP>(m_stMasterSkillTooltip[i].SkillIndex, m_stMasterSkillTooltip[i])).second == false)
		{
			break;
		}
	}
}

bool SEASON3B::CNewUIMasterSkillTree::SetMasterSkillTreeInfo(int index, char SkillPoint, float value, float nextvalue)
{
	//g_ConsoleDebug->Write(MCD_ERROR, "SetMasterSkillTreeInfo Find %d", index);
	std::map<BYTE, _MASTER_SKILLTREE_DATA>::iterator it = this->map_masterData.find(index);

	if (it == this->map_masterData.end())
	{
		//g_ConsoleDebug->Write(MCD_ERROR, "SetMasterSkillTreeInfo END");
		return true;
	}

	CSkillTreeInfo skillInfo;
	

	skillInfo.SetSkillTreeInfo(SkillPoint, value, nextvalue);

	if (this->map_skilltreeinfo.insert(std::pair<DWORD, CSkillTreeInfo>(it->second.Skill, skillInfo)).second == false)
	{
		return false;
	}

	switch (it->second.Group)
	{
	case 0:
		this->CategoryPoint[0] += SkillPoint;
		break;
	case 1:
		this->CategoryPoint[1] += SkillPoint;
		break;
	case 2:
		this->CategoryPoint[2] += SkillPoint;
		break;
	}

	return true;
}

int SEASON3B::CNewUIMasterSkillTree::SetDivideString(char* text, int isItemTollTip, int TextNum, int iTextColor, int iTextBold, bool isPercent)
{
	if (text == NULL)
	{
		return TextNum;
	}

	char alpszDst[10][256];

	memset(alpszDst, 0, sizeof(alpszDst));

	int  nLine = 0;

	if (isItemTollTip == 0)
	{
		nLine = DivideStringByPixel((LPTSTR)alpszDst, 10, 256, text, 150, 1, 35);
	}
	else if (isItemTollTip == 1)
	{
		nLine = DivideStringByPixel((LPTSTR)alpszDst, 10, 256, text, 200, 1, 35);
	}

	for (int i = 0; i < nLine; i++)
	{
		TextListColor[TextNum] = iTextColor;

		TextBold[TextNum] = iTextBold;

		std::string cText = alpszDst[i];

		if (isPercent == true)
		{
			for (int j = cText.find("%", 0); j != -1; j = cText.find("%", j + 2))
			{
				cText.insert(j, "%");
			}
		}

		sprintf(TextList[TextNum], cText.c_str());

		TextNum++;
	}


	return TextNum;
}

bool SEASON3B::CNewUIMasterSkillTree::Render()
{
	EnableAlphaTest();

	glColor4f(1.0, 1.0, 1.0, 1.0);

	SEASON3B::RenderImage(IMAGE_MASTER_INTERFACE, this->PosX, this->PosY, pBitMap[0]->Width, 428); //Fix Den Khung Master Skill

	SEASON3B::RenderImage(IMAGE_MASTER_INTERFACE+1, this->PosX + pBitMap[0]->Width, this->PosY, pBitMap[1]->Width, 428);

	this->RenderIcon();

	this->m_CloseBT.Render();
	//this->m_ClearBT.Render();

	DisableAlphaBlend();

	this->RenderText();

	return true;
}

bool SEASON3B::CNewUIMasterSkillTree::Update()
{
	return 1;
}

bool SEASON3B::CNewUIMasterSkillTree::UpdateMouseEvent()
{
	const POINT& closePos = this->m_CloseBT.GetPos();
	const POINT& closeSize = this->m_CloseBT.GetSize();

#if defined(__ANDROID__)
	const int closePadX = 12;
	const int closePadY = 10;

	if (SEASON3B::IsRelease(VK_LBUTTON) == true
		&& SEASON3B::CheckMouseIn((closePos.x - closePadX), (closePos.y - closePadY), (closeSize.x + (closePadX * 2)), (closeSize.y + (closePadY * 2))) == true)
	{
		PlayBuffer(SOUND_CLICK01);
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_MASTER_LEVEL);

		return true;
	}
#endif

	if (this->m_CloseBT.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_MASTER_LEVEL);

		return true;
	}


	//if (this->m_ClearBT.UpdateMouseEvent() == true)
	//{
	//	char szCmd[32];
	//	sprintf(szCmd, "/taymaster");
	//	SendChat(szCmd);
	//	return true;
	//}

	bool result = true;

	if (SEASON3B::IsPress(VK_LBUTTON) == true)
	{
		result = this->CheckMouse(MouseX, MouseY);

		if (result == false)
		{
			PlayBuffer(SOUND_CLICK01);
		}
	}
	
	for (int i = 0; i < MAX_MASTER_SKILL_CATEGORY; i++)
	{
		if (this->ButtonX[i] == 1 && SEASON3B::IsPress(VK_LBUTTON) == true)
		{
			this->ButtonX[i] = 0;

			return true;
		}
	}
	
	this->CheckBtn();
	
	if (SEASON3B::CheckMouseIn(this->PosX, this->PosY, this->width, this->height) == true)
	{
		return false;
	}

	return result;
}

bool SEASON3B::CNewUIMasterSkillTree::UpdateKeyEvent()
{
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MASTER_LEVEL) == false || SEASON3B::IsPress(VK_ESCAPE) == false && SEASON3B::IsPress('A') == false)
	{
		return true;
	}

	g_pNewUISystem->Hide(SEASON3B::INTERFACE_MASTER_LEVEL);

	PlayBuffer(SOUND_CLICK01);

	return false;
}

float SEASON3B::CNewUIMasterSkillTree::GetLayerDepth()
{
	return 8.1000004;
}

void SEASON3B::CNewUIMasterSkillTree::LoadImages()
{
	LoadBitmap("Interface\\new_Master_back01.jpg", IMAGE_MASTER_INTERFACE, GL_LINEAR);	//31534

	LoadBitmap("Interface\\new_Master_back02.jpg", IMAGE_MASTER_INTERFACE + 1, GL_LINEAR);	//31535

	this->pBitMap[0] = &Bitmaps[IMAGE_MASTER_INTERFACE];

	this->pBitMap[1] = &Bitmaps[IMAGE_MASTER_INTERFACE + 1];

	LoadBitmap("Interface\\new_Master_Icon.jpg", IMAGE_MASTER_INTERFACE + 2, GL_LINEAR);	//31536

	LoadBitmap("Interface\\new_Master_Non_Icon.jpg", IMAGE_MASTER_INTERFACE + 3, GL_LINEAR);	//31537

	LoadBitmap("Interface\\new_Master_box.tga", IMAGE_MASTER_INTERFACE + 4, GL_LINEAR);	//31538

	LoadBitmap("Interface\\new_Master_exit.jpg", IMAGE_MASTER_INTERFACE + 5, GL_LINEAR);	//31539

	LoadBitmap("Interface\\new_Master_arrow01.tga", IMAGE_MASTER_INTERFACE + 6, GL_LINEAR);	//31540

	LoadBitmap("Interface\\new_Master_arrow02.tga", IMAGE_MASTER_INTERFACE + 7, GL_LINEAR);	//31541

	LoadBitmap("Interface\\new_Master_arrow03.tga", IMAGE_MASTER_INTERFACE + 8, GL_LINEAR);	//31542

	LoadBitmap("Interface\\new_Master_arrow04.tga", IMAGE_MASTER_INTERFACE + 9, GL_LINEAR);	//31543

	LoadBitmap("Interface\\new_Master_arrow05.tga", IMAGE_MASTER_INTERFACE + 10, GL_LINEAR);	//31544

	LoadBitmap("Interface\\new_Master_arrow06.tga", IMAGE_MASTER_INTERFACE + 11, GL_LINEAR);	//31545

	LoadBitmap("Interface\\new_Master_arrow07.tga", IMAGE_MASTER_INTERFACE + 12, GL_LINEAR);	//31546

	LoadBitmap("Interface\\new_Master_arrow08.tga", IMAGE_MASTER_INTERFACE + 13, GL_LINEAR);	//31547
}

void SEASON3B::CNewUIMasterSkillTree::UnloadImages()
{
	for (int i = 0; i < 14; i++)	//estaba en 4, el viejo master tenia 4 asique se deben haber olvidado actualizarlo
	{
		DeleteBitmap(i + IMAGE_MASTER_INTERFACE, 0);
	}
}

void SEASON3B::CNewUIMasterSkillTree::RenderText()
{
	g_pRenderText->SetFont(g_hFont);

	if (SEASON3B::IsPress(VK_LBUTTON) == false && SEASON3B::CheckMouseIn(458, 11, 81, 10) == true)
	{
		TextList[0][0] = 0;
		TextBold[0] = 0;
		TextListColor[0] = 0;
		sprintf(TextList[0], "%I64d / %I64d", Master_Level_Data.lMasterLevel_Experince, Master_Level_Data.lNext_MasterLevel_Experince);
		RenderTipTextList(466 + DisplayWinExt, 26, 1, 0, 3, 0, 1);
	}

	g_pRenderText->SetTextColor(255, 255, 255, 0xFFu);

	g_pRenderText->SetBgColor(0, 0, 0, 1u);

	char Buffer[256];

	memset(Buffer, 0, sizeof(Buffer));

	sprintf(Buffer, GlobalText[1746], Master_Level_Data.nMLevel);

	g_pRenderText->RenderText(275 + DisplayWinExt, 11, Buffer, 0, 0, 1, 0);

	sprintf(Buffer, GlobalText[1747], Master_Level_Data.nMLevelUpMPoint);

	g_pRenderText->RenderText(372 + DisplayWinExt, 11, Buffer, 0, 0, 1, 0);

	if (Master_Level_Data.lNext_MasterLevel_Experince != 0)
	{
		__int64 iTotalLevel = Master_Level_Data.nMLevel + 400;				// 종합레벨 - 400렙이 만렙이기 때문에 더해준다.
		__int64 iTOverLevel = iTotalLevel - 255;		// 255레벨 이상 기준 레벨
		__int64 iBaseExperience = 0;					// 레벨 초기 경험치

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

		// 레벨업 경험치
		double fNeedExp = (double)Master_Level_Data.lNext_MasterLevel_Experince - (double)iBaseExperience;

		// 현재 획득한 경험치
		double fExp = (double)Master_Level_Data.lMasterLevel_Experince - (double)iBaseExperience;

		sprintf(Buffer, GlobalText[3335], fExp / fNeedExp * 100.0);

		g_pRenderText->RenderText(466 + DisplayWinExt, 11, Buffer, 0, 0, 1, 0);
	}

	g_pRenderText->RenderText(154 + DisplayWinExt, 11, GlobalText[this->ClassNameTextIndex], 0, 0, 1, 0);

	g_pRenderText->SetTextColor(255, 155, 0, 0xFFu);

	sprintf(Buffer, GlobalText[this->CategoryTextIndex], this->CategoryPoint[0]);

	g_pRenderText->RenderText(92 + DisplayWinExt, 40, Buffer, 0, 0, 1, 0);

	sprintf(Buffer, GlobalText[this->CategoryTextIndex + 1], this->CategoryPoint[1]);

	g_pRenderText->RenderText(302 + DisplayWinExt, 40, Buffer, 0, 0, 1, 0);

	sprintf(Buffer, GlobalText[this->CategoryTextIndex + 2], this->CategoryPoint[2]);

	g_pRenderText->RenderText(513 + DisplayWinExt, 40, Buffer, 0, 0, 1, 0);
}

bool SEASON3B::CNewUIMasterSkillTree::GetXYImgMaster(JCCoord* GetXY, int Skill)
{

	JCCoord BGetXY;
	SKILL_ATTRIBUTE* p = &SkillAttribute[Skill];

	if (!p) { return false; }

	BGetXY.CalcY = (double)(p->SkillGroup / 25) * 0.0546875;
	BGetXY.CalcX = (double)(p->SkillGroup % 25) * 0.0390625;
	*GetXY = BGetXY;
	//g_ConsoleDebug->Write(MCD_RECEIVE, "Skill %d %f ~ %f ", Skill, BGetXY.CalcX, BGetXY.CalcY);
	return true;

}

void SEASON3B::CNewUIMasterSkillTree::RenderIcon()
{
	float v19; // [esp+38h] [ebp-178h]
	float v20; // [esp+40h] [ebp-170h]
	float v21; // [esp+38h] [ebp-178h]
	float v22; // [esp+40h] [ebp-170h]
	float v35; // [esp+84h] [ebp-12Ch]
	float v32; // [esp+78h] [ebp-138h]
	char Buffer[256];
	for (std::map<BYTE, _MASTER_SKILLTREE_DATA>::iterator it = this->map_masterData.begin(); it != this->map_masterData.end(); it++)
	{
		BYTE Group = it->second.Group;

		int Skill = it->second.Skill;

		SKILL_ATTRIBUTE* p = &SkillAttribute[Skill];

		int skillPoint = 0;

		int index = (it->second.Index - 1) % 4;

		BYTE rank = p->SkillRank;

		std::map<DWORD, CSkillTreeInfo>::iterator sit = this->map_skilltreeinfo.find(Skill);

		if (sit != this->map_skilltreeinfo.end())
		{
			skillPoint = sit->second.GetSkillPoint();
		}
		
		DWORD textColor = RGBA(255, 255, 255, 255);
		::EnableAlphaTest();
		::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		g_pRenderText->SetBgColor(0);
		g_pRenderText->SetTextColor(textColor);

		if (this->CheckParentSkill(it->second.RequireSkill[0], it->second.RequireSkill[1]) == false || this->CheckRankPoint(Group, rank, skillPoint) == false || this->CheckBeforeSkill(Skill, skillPoint) == false || this->CheckBeforeSkill(Skill, skillPoint) == false)
		{
			textColor = RGBA(120, 120, 120, 255);

			g_pRenderText->SetTextColor(textColor);
		}
		int CalcX = (int)(index * 49.0f + this->categoryPos[Group].x);
		int CalcY = (int)(this->categoryPos[Group].y + (p->SkillRank - 1) * 41.0f);
		SEASON3B::RenderImage(IMAGE_MASTER_INTERFACE + 4, CalcX, CalcY, 50.0, 38.0, 0.0, 0.0, textColor);
		v22 = (double)(p->SkillGroup / 25) * 0.0546875;
		v21 = (double)(p->SkillGroup % 25) * 0.0390625;
		v20 = CalcY + 5.0;
		v19 = CalcX + 8.5;
		SEASON3B::RenderImage(IMAGE_MASTER_INTERFACE + 2, v19, v20, 20.0, 28.0, v21, v22, 0.0390625, 0.053710938, textColor);
		//===Draw Skill Point

		memset(Buffer, 0, sizeof(Buffer));
		//sprintf(Buffer, "%d", skillPoint);
		float v12 = (int)(CalcY + 25.0);
		float  v11 = (int)(CalcX + 38.0);

		g_pRenderText->RenderText( v11, v12, Buffer, 0, 0, 1, 0);
		RenderNumber(v11, v12, skillPoint, 0.75f);
		int v40 = it->second.ArrowDirection;
		int v34 = 0.0;
		int v39 = 0.0;
		int v33 = 0.0;
		int v37 = 0.0;
		if (InBounds(v40, 1u, 8u))
		{
			switch (v40)
			{
			case 1u:
				v34 = 30.0;
				v39 = 15.0;
				v33 = 28.0;
				v37 = 7.0;
				break;
			case 2u:
				v34 = -21.0;
				v39 = 15.0;
				v33 = 28.0;
				v37 = 7.0;
				break;
			case 3u:
				v34 = 16.0;
				v39 = 35.0;
				v33 = 7.0;
				v37 = 12.0;
				break;
			case 4u:
				v34 = 16.0;
				v39 = 35.0;
				v33 = 7.0;
				v37 = 52.0;
				break;
			case 5u:
				v34 = 35.0;
				v39 = 15.0;
				v33 = 42.0;
				v37 = 31.0;
				break;
			case 6u:
				v34 = -35.0;
				v39 = 15.0;
				v33 = 42.0;
				v37 = 31.0;
				break;
			case 7u:
				v34 = 19.0;
				v39 = 35.0;
				v33 = 40.0;
				v37 = 28.0;
				break;
			case 8u:
				v34 = -21.0;
				v39 = 35.0;
				v33 = 40.0;
				v37 = 28.0;
				break;
			default:
				break;
			}
			int v18 = CalcY + v39;
			int v17 = CalcX + v34;
			SEASON3B::RenderImage(v40 + (IMAGE_MASTER_INTERFACE + 5), v17, v18, v33, v37, 0.0, 0.0, textColor);
		}
	}
	DisableAlphaBlend();
	this->RenderToolTip();
}

void SEASON3B::CNewUIMasterSkillTree::RenderToolTip()
{
	for (std::map<BYTE, _MASTER_SKILLTREE_DATA>::iterator it = this->map_masterData.begin(); it != this->map_masterData.end(); it++)
	{
		BYTE group = it->second.Group;

		int Skill = it->second.Skill;

		SKILL_ATTRIBUTE* p = &SkillAttribute[Skill];

		if (p == NULL)
		{
			break;
		}

		int index = (it->second.Index - 1) % 4;

		int CalcX = (int)(index * 49.0f + this->categoryPos[group].x);

		int CalcY = (int)(this->categoryPos[group].y + (p->SkillRank - 1) * 41.0f);

		if (SEASON3B::IsPress(VK_LBUTTON) == true || SEASON3B::CheckMouseIn(CalcX + 8, CalcY + 5, 20, 28) == false)
		{
			continue;
		}

		std::map<DWORD, _MASTER_SKILL_TOOLTIP>::iterator mtit = this->map_masterSkillToolTip.find(Skill);

		if (mtit == this->map_masterSkillToolTip.end())
		{
			return;
		}

		int skillPoint = 0;

		float skillValue = 0.0f;

		float skillNextValue = 0.0f;

		std::map<DWORD, CSkillTreeInfo>::iterator sit = this->map_skilltreeinfo.find(Skill);

		if (sit != this->map_skilltreeinfo.end())
		{
			skillPoint = sit->second.GetSkillPoint();

			skillValue = sit->second.GetSkillValue();

			skillNextValue = sit->second.GetSkillNextValue();
		}

		for (int i = 0; i < 30; i++)
		{
			TextList[i][0] = 0;
		}

		memset(TextBold, 0, sizeof(TextBold));

		for (int i = 0; i < 30; i++)
		{
			TextListColor[i] = i == 0 ? TEXT_COLOR_YELLOW : TEXT_COLOR_WHITE;
		}

		int lineCount = 0;

		sprintf(TextList[lineCount], "%s", p->Name);

		TextBold[lineCount] = true;

		lineCount++;

		sprintf(TextList[lineCount], mtit->second.Info1, p->SkillRank, skillPoint, it->second.MaxLevel);

		lineCount++;

		char buffer[512];

		memset(buffer, 0, sizeof(buffer));

		if (it->second.DefValue == -1.0f)
		{
			sprintf(buffer, mtit->second.Info2);
		}
		else
		{
			sprintf(buffer, mtit->second.Info2, skillPoint != 0 ? skillValue : it->second.DefValue);
		}

		lineCount = this->SetDivideString(buffer, 0, lineCount, 0, 0, true);

		if (skillPoint != 0 && skillPoint < it->second.MaxLevel)
		{
			sprintf(buffer, GlobalText[3328]);

			lineCount = this->SetDivideString(buffer, 0, lineCount, 4, 0, true);

			TextBold[lineCount] = 1;

			sprintf(buffer, mtit->second.Info2, skillNextValue);

			lineCount = this->SetDivideString(buffer, 0, lineCount, 0, 0, true);
		}

		if (skillPoint < it->second.MaxLevel)
		{
			sprintf(buffer, GlobalText[3329]);

			lineCount = this->SetDivideString(buffer, 0, lineCount, 1, 0, true);

			TextBold[lineCount] = 1;

			sprintf(buffer, mtit->second.Info3, it->second.RequiredPoints);

			if (it->second.RequiredPoints <= Master_Level_Data.nMLevelUpMPoint)
			{
				lineCount = this->SetDivideString(buffer, 0, lineCount, 0, 0, true);
			}
			else
			{
				lineCount = this->SetDivideString(buffer, 0, lineCount, 2, 0, true);
			}
		}

		int iTextColor = this->CheckBeforeSkill(Skill, skillPoint) == true ? 0 : 2;

		sprintf(buffer, mtit->second.Info4);

		lineCount = this->SetDivideString(buffer, 0, lineCount, iTextColor, 0, true);

		if (skillPoint < it->second.MaxLevel && p->SkillRank != 1)
		{
			iTextColor = this->CheckRankPoint(group, p->SkillRank, skillPoint) == true ? 0 : 2;

			sprintf(buffer, mtit->second.Info5);

			lineCount = this->SetDivideString(buffer, 0, lineCount, iTextColor, 0, true);

			int RequireSkill1 = it->second.RequireSkill[0];

			int RequireSkill2 = it->second.RequireSkill[1];

			if (RequireSkill1 != 0)
			{
				std::map<DWORD, CSkillTreeInfo>::iterator rq1it = this->map_skilltreeinfo.find(RequireSkill1);

				if (rq1it != this->map_skilltreeinfo.end())
				{
					iTextColor = rq1it->second.GetSkillValue() < 10 ? 2 : 0;
				}
				else
				{
					iTextColor = 2;
				}

				sprintf(buffer, mtit->second.Info6);

				lineCount = this->SetDivideString(buffer, 0, lineCount, iTextColor, 0, true);
			}

			if (RequireSkill2 != 0)
			{
				std::map<DWORD, CSkillTreeInfo>::iterator rq1it = this->map_skilltreeinfo.find(RequireSkill2);

				if (rq1it != this->map_skilltreeinfo.end())
				{
					iTextColor = rq1it->second.GetSkillValue() < 10 ? 2 : 0;
				}
				else
				{
					iTextColor = 2;
				}

				sprintf(buffer, mtit->second.Info7);

				lineCount = this->SetDivideString(buffer, 0, lineCount, iTextColor, 0, true);
			}
		}

		RenderTipTextList(CalcX + 8, CalcY + 33, lineCount, 0, 3, 0, 1);
	}
}

bool SEASON3B::CNewUIMasterSkillTree::CheckMouse(int posx, int posy)
{
	POINT position[3] = { {185,65},{385,65},{585,65} };

	for (int i = 0; i < MAX_MASTER_SKILL_CATEGORY; i++)
	{
		if (SEASON3B::CheckMouseIn(position[i].x + this->PosX, this->ButtonY[i] + position[i].y + this->PosY, 15, 30) == true && this->ButtonX[i] == 0)
		{
			this->ButtonY[i] = 1;

			return 0;
		}
	}

	return 1;
}

bool SEASON3B::CNewUIMasterSkillTree::CheckBtn()
{
	int posX = 220;

	for (int i = 0; i < MAX_MASTER_SKILL_CATEGORY; i++)
	{
		if (this->ButtonX[i] == 1 && SEASON3B::IsRelease(VK_LBUTTON))
		{
			this->ButtonX[i] = 0;
			return 0;
		}

		if (this->ButtonX[i] == 1)
		{
			this->ButtonY[i] = MouseY - 65;

			if (this->ButtonY[i] <= posX)
			{
				if (*((int*)this + i + 12) <= 0)
					this->ButtonY[i] = 0;
			}
			else
			{
				this->ButtonY[i] = posX;
			}

		}
	}
	
	for (std::map<BYTE, _MASTER_SKILLTREE_DATA>::iterator it = this->map_masterData.begin(); it != this->map_masterData.end(); it++)
	{
		BYTE Group = it->second.Group;

		switch (Group)
		{
		case 0:
			this->CheckAttributeArea(Group, it->second.Index, it->second.RequiredPoints, it->second.RequireSkill[0], it->second.RequireSkill[1], it->second.Skill, it->second.DefValue);
			break;
		case 1:
			this->CheckAttributeArea(Group, it->second.Index, it->second.RequiredPoints, it->second.RequireSkill[0], it->second.RequireSkill[1], it->second.Skill, it->second.DefValue);
			break;
		case 2:
			this->CheckAttributeArea(Group, it->second.Index, it->second.RequiredPoints, it->second.RequireSkill[0], it->second.RequireSkill[1], it->second.Skill, it->second.DefValue);
			break;
		}
	}

	return true;
}

bool SEASON3B::CNewUIMasterSkillTree::CheckAttributeArea(int group, int index, int reqPoint, unsigned int ReqSkill1, unsigned int ReqSkill2, unsigned int Skill, int defValue)
{
	if (!InBounds(group, 0, 2))
	{
		return 0;
	}

	SKILL_ATTRIBUTE* lpskill = &SkillAttribute[Skill];

	if (lpskill == NULL)
	{
		return 0;
	}

	int tindex = (index - 1) % 4;

	int posX = (int)((double)this->categoryPos[group].x + tindex * 49.0);

	int posY = (int)((double)this->categoryPos[group].y + (lpskill->SkillRank - 1) * 41.0);

	if (SEASON3B::IsPress(1) && SEASON3B::CheckMouseIn(posX + 8, posY + 5, 20, 28))
	{
		std::map<DWORD, CSkillTreeInfo>::iterator it = this->map_skilltreeinfo.find(Skill);
		std::map<BYTE, _MASTER_SKILLTREE_DATA>::iterator bp = this->map_masterData.find(index);

		int skillPoint = 0;
		int mMaxPoint = 0;
		if (it != this->map_skilltreeinfo.end())
		{
			skillPoint = it->second.GetSkillPoint();
		}
		if (bp != this->map_masterData.end())
		{
			mMaxPoint = bp->second.MaxLevel;
		}
		PlayBuffer(SOUND_CLICK01);


		if (this->CheckSkillPoint(Master_Level_Data.nMLevelUpMPoint, index, mMaxPoint, ReqSkill1, ReqSkill2, Skill, defValue, skillPoint) == false)
		{
			//g_ConsoleDebug->Write(MCD_ERROR, "CheckAttributeArea 1 ");
			return 1;
		}

		if (this->CheckParentSkill(ReqSkill1, ReqSkill2) == false || this->CheckRankPoint(group, lpskill->SkillRank, skillPoint) == false || this->CheckBeforeSkill(Skill, skillPoint) == false)
		{
			//g_ConsoleDebug->Write(MCD_ERROR, "%d %d %d", this->CheckParentSkill(ReqSkill1, ReqSkill2), this->CheckRankPoint(group, lpskill->SkillRank, skillPoint), this->CheckBeforeSkill(Skill, skillPoint));
			SEASON3B::CreateOkMessageBox(GlobalText[3327]);

			return 1;
		}

		if (this->CheckBeforeSkill(Skill, skillPoint) == false/*g_csItemOption.Special_Option_Check(Skill) == false*/)
		{
			//g_ConsoleDebug->Write(MCD_ERROR, "CheckAttributeArea 3");
			SEASON3B::CreateOkMessageBox(GlobalText[3336]);
			return 1;
		}
	//	g_ConsoleDebug->Write(MCD_RECEIVE, "index %d Skill %d reqPoint %d", index, Skill, reqPoint);

		this->SetConsumePoint(reqPoint);

		this->SetCurSkillID(Skill);

		//SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CMaster_Level_Interface));

		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CMasterSkillPointMsgBoxLayout));

		MouseLButton = 0;
		
		MouseLButtonPop = 0;
		
		MouseLButtonPush = 0;
		
		
	}

	return 1;
}
//#include "CBInterface.h"
bool SEASON3B::CNewUIMasterSkillTree::CheckSkillPoint(WORD mLevelUpPoint, int index, int reqPoint, int ReqSkill1, int ReqSkill2, int Skill, int defValue, WORD skillPoint)
{
	//g_ConsoleDebug->Write(MCD_RECEIVE, "CheckSkillPoint %d ~ %d ", mLevelUpPoint, reqPoint);
	//gInterface.DrawMessage(1, "CheckSkillPoint %d %d", mLevelUpPoint, reqPoint);
	if (skillPoint == reqPoint)
	{
		SEASON3B::CreateOkMessageBox(GlobalText[3326]);
		return 0;
	}

	if (mLevelUpPoint >= 1)
	{
		return 1;
	}

	char Buffer[358];

	memset(Buffer, 0, sizeof(Buffer));

	sprintf(Buffer, GlobalText[3326], reqPoint - mLevelUpPoint);

	SEASON3B::CreateOkMessageBox(Buffer);

	return 0;
}

bool SEASON3B::CNewUIMasterSkillTree::CheckParentSkill(DWORD reqSkill1, DWORD reqSkill2)
{
	if (reqSkill1 == 0)
	{
		return 1;
	}

	std::map<DWORD, CSkillTreeInfo>::iterator it = this->map_skilltreeinfo.find(reqSkill1);

	if (it == this->map_skilltreeinfo.end())
	{
		return 0;
	}

	if (it->second.GetSkillPoint() < 10)
	{
		return 0;
	}

	if (reqSkill2 == 0)
	{
		return 1;
	}

	it = this->map_skilltreeinfo.find(reqSkill2);

	if (it == this->map_skilltreeinfo.end())
	{
		return 0;
	}

	if (it->second.GetSkillPoint() < 10)
	{
		return 0;
	}

	return 1;
}

bool SEASON3B::CNewUIMasterSkillTree::CheckRankPoint(BYTE group, BYTE rank, BYTE skillPoint)
{
	//return 1;
	if (this->skillPoint[group][rank] < skillPoint)
	{
		this->skillPoint[group][rank] = skillPoint;
	}

	if (rank == 1)
	{
		return 1;
	}

	return this->skillPoint[group][rank-1] >= 10;	//PUEDE SER +2 SI HAY ALGUN ERROR
}

bool SEASON3B::CNewUIMasterSkillTree::CheckBeforeSkill(int skill, BYTE skillPoint)
{

	int Index = this->GetBeforeSkillID(skill);

	if (Index == 0)
	{
		return 1;
	}

	SKILL_ATTRIBUTE* lpSkill = &SkillAttribute[Index];

	if (lpSkill == NULL)
	{
		return 0;
	}

	if (lpSkill->SkillUseType == 4)
	{
		return 1;
	}

	if (skillPoint != 0)
	{
		return 1;
	}

	for (int i = 0; i < MAX_MAGIC; i++)
	{
		if (CharacterAttribute->Skill[i] == Index)
		{
			return 1;
		}
	}

	return 0;
}

int SEASON3B::CNewUIMasterSkillTree::GetBeforeSkillID(int index)
{
	return SkillAttribute[index].SkillBrand;
}

int SEASON3B::CNewUIMasterSkillTree::GetBaseSkillID(int index)
{
	if (index >= MAX_SKILLS)
	{
		return 0;
	}

	while (SkillAttribute[index].SkillBrand != 0 && SkillAttribute[index].SkillBrand != 75)
	{
		index = SkillAttribute[index].SkillBrand;
	}

	return index;
}

void SEASON3B::CNewUIMasterSkillTree::SkillUpgrade(int index, char point, float value, float nextvalue)
{
	std::map<BYTE, _MASTER_SKILLTREE_DATA>::iterator it = this->map_masterData.find(index);

	int realSkill = 0;

	if (it != this->map_masterData.end())
	{
		realSkill = it->second.Skill;
	}

	std::map<DWORD, CSkillTreeInfo>::iterator sit = this->map_skilltreeinfo.find(realSkill);

	if (sit != this->map_skilltreeinfo.end())
	{
		this->map_skilltreeinfo.erase(sit);
	}

	CSkillTreeInfo skillinfo;

	skillinfo.SetSkillTreeInfo(point, value, nextvalue);

	if (this->map_skilltreeinfo.insert(std::pair< DWORD, CSkillTreeInfo>(realSkill, skillinfo)).second == 0)
	{
		return;
	}

	this->CategoryPoint[it->second.Group] += it->second.RequiredPoints;
}

bool SEASON3B::CNewUIMasterSkillTree::IsLearnSkill(int index)
{
	std::map<DWORD, CSkillTreeInfo>::iterator it = this->map_skilltreeinfo.find(index);

	if (it == this->map_skilltreeinfo.end())
	{
		return false;
	}

	return true;
}

float SEASON3B::CNewUIMasterSkillTree::GetSkillValue(int index)
{
	std::map<DWORD, CSkillTreeInfo>::iterator it = this->map_skilltreeinfo.find(index);

	if (it == this->map_skilltreeinfo.end())
	{
		return 0.0f;
	}

	return it->second.GetSkillValue();
}

float SEASON3B::CNewUIMasterSkillTree::GetSkillCumulativeValue(int index, BYTE damage)
{
	if (!index || index < 300 || index > 608 || damage == 0 || damage >= 5)
	{
		return 0.0;
	}

	float value = 0.0;

	SKILL_ATTRIBUTE* lpSkill;

	for (int i = index; i != 75; i = lpSkill->SkillBrand)
	{
		if (i == 0)
		{
			break;
		}

		lpSkill = &SkillAttribute[i];

		if (damage == lpSkill->IsDamage)
		{
			value += this->GetSkillValue(i);
		}
	}

	return value;
}

BYTE SEASON3B::CNewUIMasterSkillTree::GetSkillLevel(int index)
{
	std::map<DWORD, CSkillTreeInfo>::iterator it = this->map_skilltreeinfo.find(index);

	if (it == this->map_skilltreeinfo.end())
	{
		return 0;
	}

	return it->second.GetSkillPoint();
}

void SEASON3B::CNewUIMasterSkillTree::RegisterSkillInCharacterAttribute(int index)
{
	if (index < 300 || index > 608)
	{
		return;
	}

	SKILL_ATTRIBUTE* lpskill = &SkillAttribute[index];

	if (lpskill->SkillUseType == 4)
	{
		for (int i = 0; i < MAX_MAGIC; i++)
		{
			if (lpskill->SkillBrand != CharacterAttribute->Skill[i])
			{
				continue;
			}

			CharacterAttribute->Skill[i] = index;

			return;
		}

		return;
	}

	for (int i = 0; i < MAX_MAGIC; i++)
	{
		if (CharacterAttribute->Skill[i] != 0)
		{
			continue;
		}

		CharacterAttribute->Skill[i] = index;

		return;
	}
}

void SEASON3B::CNewUIMasterSkillTree::SetConsumePoint(BYTE ConsumePoint)
{
	this->ConsumePoint = ConsumePoint;
}

void SEASON3B::CNewUIMasterSkillTree::SetCurSkillID(int index)
{
	this->CurSkillID = index;
}

void SEASON3B::CNewUIMasterSkillTree::ClearSkillTreeData()
{
	this->map_masterData.clear();
}

void SEASON3B::CNewUIMasterSkillTree::ClearSkillTooltipData()
{
	this->map_masterSkillToolTip.clear();
}

#endif















