#include "stdafx.h"
#include "WindowClass.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"

bool StatusCachePhoto = false;
CUIPhotoViewer m_PhotoChangeClass;
CNewUITextBox* m_pTextBoxChangeClass[7];
cWindowClass WindowClass;

cWindowClass::cWindowClass()
{
	this->Init();
	this->WCoinChangeClass = 0;

}

cWindowClass::~cWindowClass()
{
	for (int i = 0; i < 7; i++)
	{
		if (m_pTextBoxChangeClass[i])
			m_pTextBoxChangeClass[i]->Release();

		SAFE_DELETE(m_pTextBoxChangeClass[i]);
	}
}

void cWindowClass::Init()
{
	this->SetVisible(false);
	this->SetRect(120, 70, 400, 255);
	this->SetCurTab(0);
}

void cWindowClass::RecvData(PMSG_CHANGECLASS_DATA* Data)
{
	this->WCoinChangeClass = Data->m_WCoinC;
}

void cWindowClass::SendChangeClass(int Type)
{
	CG_CHANGECLASS_SEND pRequest;
	pRequest.Head.set(0xFB, 0x08, sizeof(pRequest));

	pRequest.Type = Type;

	DataSend((BYTE*)& pRequest, pRequest.Head.size);
}
void cWindowClass::SetRect(float a1, float a2, float a3, float a4)
{
	this->x = a1;
	this->y = a2;
	this->w = a3;
	this->h = a4;

}

void cWindowClass::Draw()
{
	if (!this->GetVisible())
	{
		return;
	}

	if(!StatusCachePhoto) this->CachePhoto();
	glAlphaFunc(GL_GREATER, 0.0f);

	TextDraw((HFONT)g_hFontBold, x, y, 0x0, 0x373b45ff, w, h, 1, " ");


	int i = 0;


	g_pBCustomMenuInfo->gDrawWindowCustom(&x, &y, w, h, eWindowChangeClass, "ĐỔI GIỚI TÍNH NHÂN VẬT");

	for (int i = 0; i < 7; i++) // Hiển thị button
	{
		if (this->CurTab == 7)
		{
			const BYTE state[3] = { 1, 1, 2 };
			UIController.Button(CNewUIInGameShop::IMAGE_IGS_CATEGORY_BTN, x + 20, y + 36 + 30 * i, 73, 27, 73, 27, 3, state, WindowClass.SelectTabAction, i);
			TextDraw((HFONT)g_hFontBold, x + 20, y + 45 + 30 * i, 0xffffffff, 0x0, 73, 0, 3, gOther.Text_Chung[3 + i]);
		}
		else
		{
			const BYTE state[3] = { 0, 1, 2 };
			UIController.Button(CNewUIInGameShop::IMAGE_IGS_CATEGORY_BTN, x + 20, y + 36 + 30 * i, 73, 27, 73, 27, 3, state, WindowClass.SelectTabAction, i);
			TextDraw((HFONT)g_hFont, x + 20, y + 45 + 30 * i, 0xffffffbb, 0x0, 73, 0, 3, gOther.Text_Chung[3 + i]);
		}
	}

	glAlphaFunc(GL_GREATER, 0.25f);



	switch (this->CurTab)
	{
	case 0:

		this->TabPhuThuy();
		break;
	case 1:
		this->TabChienBinh();
		break;
	case 2:
		this->TabTienNu();
		break;
	case 3:
		this->TabDauSi();
		break;
	case 4:
		this->TabChuaTe();
		break;
	case 5:
		this->TabThuatSi();
		break;
	case 6:
		this->TabThietBinh();
		break;
	default:
		break;
	}
	if (gInterface.Data[eWindowChangeClass].OnClick)
	{
		this->SetRect(x, y, w, h);
		for (int i = 0; i < 7; i++)
		{
			if (m_pTextBoxChangeClass[i])
			{
				m_pTextBoxChangeClass[i]->SetPos(x + 102, y + 170, 290, 70);
			}

		}
		if (StatusCachePhoto)
		{
			m_PhotoChangeClass.SetPosition(x + 100, y + 25);
		}
	}

}
void cWindowClass::SetClassPhoto(int Class)
{
	m_PhotoChangeClass.SetAnimation(AT_STAND1);

	if (m_PhotoChangeClass.GetPhotoChar()->Class != Class)
	{
		m_PhotoChangeClass.CopyPlayer();
		m_PhotoChangeClass.SetClass(Class);
		for (int i = 0; i < MAX_BODYPART; ++i)
		{
			m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].m_pCloth[0] = NULL;
			m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].m_pCloth[1] = NULL;
			m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].m_byNumCloth = 0;
			m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Type = -1;
			m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Level = 0;
			m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Option1 = 0;
		}
		m_PhotoChangeClass.GetPhotoChar()->BodyPart[1].Type = MODEL_BODY_HELM + gCharacterManager.GetSkinModelIndex(Class);
		m_PhotoChangeClass.GetPhotoChar()->BodyPart[2].Type = MODEL_BODY_ARMOR + gCharacterManager.GetSkinModelIndex(Class);
		m_PhotoChangeClass.GetPhotoChar()->BodyPart[3].Type = MODEL_BODY_PANTS + gCharacterManager.GetSkinModelIndex(Class);
		m_PhotoChangeClass.GetPhotoChar()->BodyPart[4].Type = MODEL_BODY_GLOVES + gCharacterManager.GetSkinModelIndex(Class);
		m_PhotoChangeClass.GetPhotoChar()->BodyPart[5].Type = MODEL_BODY_BOOTS + gCharacterManager.GetSkinModelIndex(Class);

		m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Type = -1;
		m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Level = 0;
		m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Option1 = 0;

		m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Type = -1;
		m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Level = 0;
		m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Option1 = 0;

		m_PhotoChangeClass.GetPhotoChar()->Wing.Type = -1;
		m_PhotoChangeClass.GetPhotoChar()->Wing.Level = 0;
		m_PhotoChangeClass.GetPhotoChar()->Wing.Option1 = 0;
		m_PhotoChangeClass.GetPhotoChar()->Wing.m_pCloth[0] = NULL;
		m_PhotoChangeClass.GetPhotoChar()->Wing.m_pCloth[1] = NULL;
		m_PhotoChangeClass.GetPhotoChar()->Wing.m_byNumCloth = 0;
	}
	switch (Class)
	{
		case 0 :
		{
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Type = MODEL_ITEM + 5 * 512 + 10;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Option1 = 63;

			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Type = MODEL_ITEM + 6 * 512 + 15;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Option1 = 63;
			for (int i = 1; i < MAX_BODYPART; i++)
			{
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Type = MODEL_ITEM + (6 + i) * 512 + 18;
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Level = 15;
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Option1 = 63;
			}
			m_PhotoChangeClass.GetPhotoChar()->Wing.Type = MODEL_ITEM + 12 * 512 + 37;
			m_PhotoChangeClass.GetPhotoChar()->Wing.Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Wing.Option1 = 63;
		}
		break;
		case 1:
		{
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Type = MODEL_ITEM +19;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Option1 = 63;

			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Type = MODEL_ITEM +19;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Option1 = 63;
			for (int i = 1; i < MAX_BODYPART; i++)
			{
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Type = (MODEL_ITEM + ((6 + i) * 512)) + 1;
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Level = 15;
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Option1 = 63;
			}
			m_PhotoChangeClass.GetPhotoChar()->Wing.Type = MODEL_ITEM + 12 * 512 + 36;
			m_PhotoChangeClass.GetPhotoChar()->Wing.Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Wing.Option1 = 63;
		}
		break;
		case 2:
		{
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Type = MODEL_ITEM + 4 * 512 + 33;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Option1 = 63;

			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Type = -1;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Option1 = 63;
			for (int i = 1; i < MAX_BODYPART; i++)
			{
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Type = MODEL_ITEM + (6 + i) * 512 + 31;
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Level = 15;
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Option1 = 63;
			}
			m_PhotoChangeClass.GetPhotoChar()->Wing.Type = MODEL_ITEM + 12 * 512 + 38;
			m_PhotoChangeClass.GetPhotoChar()->Wing.Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Wing.Option1 = 63;
		}
		break;
		case 3:
		{
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Type = MODEL_ITEM + 19;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Option1 = 63;

			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Type = MODEL_ITEM + 19;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Option1 = 63;
			for (int i = 2; i < MAX_BODYPART; i++)
			{
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Type = MODEL_ITEM + (6 + i) * 512 + 15;
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Level = 15;
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Option1 = 63;
			}
			m_PhotoChangeClass.GetPhotoChar()->Wing.Type = MODEL_ITEM + 12 * 512 + 39;
			m_PhotoChangeClass.GetPhotoChar()->Wing.Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Wing.Option1 = 63;
		}
		break;
		case 4:
		{
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Type = MODEL_ITEM + 2 * 512 + 14;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Option1 = 63;

			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Type = MODEL_ITEM + 6 * 512 + 7;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Option1 = 63;
			for (int i = 1; i < MAX_BODYPART; i++)
			{
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Type = MODEL_ITEM + (6 + i) * 512 + 51;
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Level = 15;
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Option1 = 63;
			}
			m_PhotoChangeClass.GetPhotoChar()->Wing.Type = MODEL_ITEM + 12 * 512 + 40;
			m_PhotoChangeClass.GetPhotoChar()->Wing.Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Wing.Option1 = 63;
		}
		break;
		case 5:
		{
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Type = MODEL_ITEM + 5 * 512 + 32;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Option1 = 63;

			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Type = MODEL_ITEM + 5 * 512 + 29;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Option1 = 63;
			for (int i = 1; i < MAX_BODYPART; i++)
			{
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Type = MODEL_ITEM + (6 + i) * 512 + 44;
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Level = 15;
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Option1 = 63;
			}
			m_PhotoChangeClass.GetPhotoChar()->Wing.Type = MODEL_ITEM + 12 * 512 + 43;
			m_PhotoChangeClass.GetPhotoChar()->Wing.Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Wing.Option1 = 63;
		}
		break;
		case 6:
		{
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Type = MODEL_ITEM + 0 * 512 + 32;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[0].Option1 = 63;

			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Type = MODEL_ITEM + 0 * 512 + 32;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Weapon[1].Option1 = 63;
			for (int i = 1; i < MAX_BODYPART; i++)
			{
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Type = MODEL_ITEM + (6 + i) * 512 + 59;
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Level = 15;
				m_PhotoChangeClass.GetPhotoChar()->BodyPart[i].Option1 = 63;
			}
			m_PhotoChangeClass.GetPhotoChar()->Wing.Type = MODEL_ITEM + 12 * 512 + 50;
			m_PhotoChangeClass.GetPhotoChar()->Wing.Level = 15;
			m_PhotoChangeClass.GetPhotoChar()->Wing.Option1 = 63;
		}
		break;
	default:
		break;
	}


}
void cWindowClass::CachePhoto()
{
	m_PhotoChangeClass.Init(0);
	m_PhotoChangeClass.SetSize(150, 150);
	m_PhotoChangeClass.CopyPlayer();
	m_PhotoChangeClass.SetAutoupdatePlayer(TRUE);
	m_PhotoChangeClass.SetAnimation(AT_STAND1);
	m_PhotoChangeClass.SetAngle(120);
	m_PhotoChangeClass.SetZoom(0.8);
	m_PhotoChangeClass.SetPosition(x + 100, y + 25);
	for (int i = 0; i < 7; i++)
	{
		//===Text Box
		m_pTextBoxChangeClass[i] = new CNewUITextBox();
		m_pTextBoxChangeClass[i]->Create(x + 102, y + 170, 290, 70);
		if (m_pTextBoxChangeClass[i])
		{
			m_pTextBoxChangeClass[i]->ClearText();
			
		}
	}
	cWindowClass::CreateText();
	StatusCachePhoto = true;
}
void cWindowClass::TabHuongDan()
{

}


void cWindowClass::TabPhuThuy()
{
	//character.DrawHero2(x + 90, y + 30, 150, 150);
	if (StatusCachePhoto)
	{
		if (m_PhotoChangeClass.GetPhotoChar()->Class != 0)
		{
			this->SetClassPhoto(0);
		}
		m_PhotoChangeClass.Render();
	}


	TextDraw((HFONT)g_hFont, x + 250, y + 38, 0x0, 0x87CEFA, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 1, 0x0, 0x00F5FF, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 2, 0x0, 0x00FA9A, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 3, 0x0, 0xCAFF70, 140, 15, 0, " ");

	TextDraw((HFONT)g_hFont, x + 240, y + 43, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[10], gOther.Text_Chung[3]);
	//TextDraw((HFONT)g_hFont, x + 240 , y + 43 + 25 * 1, 0xffffffff, 0x0, 150, 0, 3, "CHI PHÍ : %d WcoinC",gChangeClass.m_WCoinC);
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 1, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[11], gInterface.NumberFormat(this->WCoinChangeClass));
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 2, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[12]);
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 3, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[13]);

	UIController.Button(32331, x + 270, y + 38 + 25 * 4, 73, 27, 128.0, 128.0, this->ChangeClassDW);
	//UIController.Button(32331, x + 20, y + 36 + 30 * i, 73, 27, 73, 27, 3, state, WindowClass.SelectTabAction, i);
	TextDraw((HFONT)g_hFontBold, x + 270, y + 45 + 25 * 4, 0xffffffff, 0x0, 73, 0, 3, gOther.Text_Chung[14]);

	if (m_pTextBoxChangeClass[0])
	{
		m_pTextBoxChangeClass[0]->RenderKemScroll();
	}
	
	//zText1.Draw(x + 102, y + 170, 290, 5, 15, true);
}
void cWindowClass::TabChienBinh()
{

	//character.DrawHero1(x + 90, y + 30, 150, 150);
	if (StatusCachePhoto)
	{
		if (m_PhotoChangeClass.GetPhotoChar()->Class != 1)
		{
			this->SetClassPhoto(1);
		}
		m_PhotoChangeClass.Render();
	}
	TextDraw((HFONT)g_hFont, x + 250, y + 38, 0x0, 0x87CEFA, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 1, 0x0, 0x00F5FF, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 2, 0x0, 0x00FA9A, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 3, 0x0, 0xCAFF70, 140, 15, 0, " ");

	TextDraw((HFONT)g_hFont, x + 240, y + 43, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[10], gOther.Text_Chung[4]);
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 1, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[11], gInterface.NumberFormat(this->WCoinChangeClass));
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 2, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[12]);
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 3, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[13]);

	UIController.Button(32331, x + 270, y + 38 + 25 * 4, 73, 27, 128.0, 128.0, this->ChangeClassDK);
	TextDraw((HFONT)g_hFontBold, x + 270, y + 45 + 25 * 4, 0xffffffff, 0x0, 73, 0, 3, gOther.Text_Chung[14]);

	if (m_pTextBoxChangeClass[1])
	{
		m_pTextBoxChangeClass[1]->RenderKemScroll();
	}
}
void cWindowClass::TabTienNu()
{
	//character.DrawHero3(x + 90, y + 30, 150, 150);
	if (StatusCachePhoto)
	{
		if (m_PhotoChangeClass.GetPhotoChar()->Class != 2)
		{
			this->SetClassPhoto(2);
		}
		m_PhotoChangeClass.Render();
	}
	TextDraw((HFONT)g_hFont, x + 250, y + 38, 0x0, 0x87CEFA, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 1, 0x0, 0x00F5FF, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 2, 0x0, 0x00FA9A, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 3, 0x0, 0xCAFF70, 140, 15, 0, " ");

	TextDraw((HFONT)g_hFont, x + 240, y + 43, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[10], gOther.Text_Chung[5]);
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 1, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[11], gInterface.NumberFormat(this->WCoinChangeClass));
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 2, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[12]);
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 3, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[13]);

	UIController.Button(32331, x + 270, y + 38 + 25 * 4, 73, 27, 128.0, 128.0, this->ChangeClassEFL);
	TextDraw((HFONT)g_hFontBold, x + 270, y + 45 + 25 * 4, 0xffffffff, 0x0, 73, 0, 3, gOther.Text_Chung[14]);
	if (m_pTextBoxChangeClass[2])
	{
		m_pTextBoxChangeClass[2]->RenderKemScroll();
	}
}
void cWindowClass::TabDauSi()
{

	//character.DrawHero4(x + 90, y + 30, 150, 150);
	if (StatusCachePhoto)
	{
		if (m_PhotoChangeClass.GetPhotoChar()->Class != 3)
		{
			this->SetClassPhoto(3);
		}
		m_PhotoChangeClass.Render();
	}
	TextDraw((HFONT)g_hFont, x + 250, y + 38, 0x0, 0x87CEFA, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 1, 0x0, 0x00F5FF, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 2, 0x0, 0x00FA9A, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 3, 0x0, 0xCAFF70, 140, 15, 0, " ");

	TextDraw((HFONT)g_hFont, x + 240, y + 43, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[10], gOther.Text_Chung[6]);
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 1, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[11], gInterface.NumberFormat(this->WCoinChangeClass));
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 2, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[12]);
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 3, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[13]);

	UIController.Button(32331, x + 270, y + 38 + 25 * 4, 73, 27, 128.0, 128.0, this->ChangeClassMG);
	TextDraw((HFONT)g_hFontBold, x + 270, y + 45 + 25 * 4, 0xffffffff, 0x0, 73, 0, 3, gOther.Text_Chung[14]);

	if (m_pTextBoxChangeClass[3])
	{
		m_pTextBoxChangeClass[3]->RenderKemScroll();
	}
}
void cWindowClass::TabChuaTe()
{
	//character.DrawHero5(x + 90, y + 30, 150, 150);
	if (StatusCachePhoto)
	{
		if (m_PhotoChangeClass.GetPhotoChar()->Class != 4)
		{
			this->SetClassPhoto(4);
		}
		m_PhotoChangeClass.Render();
	}
	TextDraw((HFONT)g_hFont, x + 250, y + 38, 0x0, 0x87CEFA, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 1, 0x0, 0x00F5FF, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 2, 0x0, 0x00FA9A, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 3, 0x0, 0xCAFF70, 140, 15, 0, " ");

	TextDraw((HFONT)g_hFont, x + 240, y + 43, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[10], gOther.Text_Chung[7]);
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 1, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[11], gInterface.NumberFormat(this->WCoinChangeClass));
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 2, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[12]);
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 3, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[13]);

	UIController.Button(32331, x + 270, y + 38 + 25 * 4, 73, 27, 128.0, 128.0, this->ChangeClassDL);
	TextDraw((HFONT)g_hFontBold, x + 270, y + 45 + 25 * 4, 0xffffffff, 0x0, 73, 0, 3, gOther.Text_Chung[14]);

	if (m_pTextBoxChangeClass[4])
	{
		m_pTextBoxChangeClass[4]->RenderKemScroll();
	}
}

void cWindowClass::TabThuatSi()
{
	//character.DrawHero6(x + 90, y + 30, 150, 150);
	if (StatusCachePhoto)
	{
		if (m_PhotoChangeClass.GetPhotoChar()->Class != 5)
		{
			this->SetClassPhoto(5);
		}
		m_PhotoChangeClass.Render();
	}
	TextDraw((HFONT)g_hFont, x + 250, y + 38, 0x0, 0x87CEFA, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 1, 0x0, 0x00F5FF, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 2, 0x0, 0x00FA9A, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 3, 0x0, 0xCAFF70, 140, 15, 0, " ");

	TextDraw((HFONT)g_hFont, x + 240, y + 43, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[10], gOther.Text_Chung[8]);
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 1, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[11], gInterface.NumberFormat(this->WCoinChangeClass));
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 2, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[12]);
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 3, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[13]);

	UIController.Button(32331, x + 270, y + 38 + 25 * 4, 73, 27, 128.0, 128.0, this->ChangeClassSUM);
	TextDraw((HFONT)g_hFontBold, x + 270, y + 45 + 25 * 4, 0xffffffff, 0x0, 73, 0, 3, gOther.Text_Chung[14]);

	if (m_pTextBoxChangeClass[5])
	{
		m_pTextBoxChangeClass[5]->RenderKemScroll();
	}
}

void cWindowClass::TabThietBinh()
{
	//character.DrawHero7(x + 90, y + 30, 150, 150);
	if (StatusCachePhoto)
	{
		if (m_PhotoChangeClass.GetPhotoChar()->Class != 6)
		{
			this->SetClassPhoto(6);
		}
		m_PhotoChangeClass.Render();
	}
	TextDraw((HFONT)g_hFont, x + 250, y + 38, 0x0, 0x87CEFA, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 1, 0x0, 0x00F5FF, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 2, 0x0, 0x00FA9A, 140, 15, 0, " ");
	TextDraw((HFONT)g_hFont, x + 250, y + 38 + 25 * 3, 0x0, 0xCAFF70, 140, 15, 0, " ");

	TextDraw((HFONT)g_hFont, x + 240, y + 43, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[10], gOther.Text_Chung[9]);
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 1, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[11], gInterface.NumberFormat(this->WCoinChangeClass));
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 2, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[12]);
	TextDraw((HFONT)g_hFont, x + 240, y + 43 + 25 * 3, 0xffffffff, 0x0, 150, 0, 3, gOther.Text_Chung[13]);

	UIController.Button(32331, x + 270, y + 38 + 25 * 4, 73, 27, 128.0, 128.0, this->ChangeClassRF);
	TextDraw((HFONT)g_hFontBold, x + 270, y + 45 + 25 * 4, 0xffffffff, 0x0, 73, 0, 3, gOther.Text_Chung[14]);

	if (m_pTextBoxChangeClass[6])
	{
		m_pTextBoxChangeClass[6]->RenderKemScroll();
	}
}

bool cWindowClass::BlockMouse()
{
	if (!this->GetVisible())
	{
		return false;
	}

	return SEASON3B::CheckMouseIn(x - 20, y - 20, w + 70, h + 90);
}
bool cWindowClass::GetVisible()
{
	return gInterface.Data[eWindowChangeClass].OnShow;

}
void cWindowClass::SetVisible(bool a1)
{
	gInterface.Data[eWindowChangeClass].OnShow = a1;
	gInterface.Data[eWindowChangeClass].EventTick = GetTickCount();
}

void cWindowClass::SetCurTab(int a1)
{
	this->CurTab = a1;
	gInterface.Data[eWindowChangeClass].EventTick = GetTickCount();
}


void cWindowClass::SelectTabAction(LPVOID pClass)
{
	cUIController* This = (cUIController*)pClass;
	WindowClass.SetCurTab(This->GetCallBackValue());
}

void cWindowClass::ChangeClassDW(LPVOID pClass)
{
	if (GetTickCount() - gInterface.Data[eWindowChangeClass].EventTick < 300) return;
	cUIController* This = (cUIController*)pClass;
	WindowClass.SendChangeClass(0);
	WindowClass.SetVisible(false);
}

void cWindowClass::ChangeClassDK(LPVOID pClass)
{
	if (GetTickCount() - gInterface.Data[eWindowChangeClass].EventTick < 300) return;
	cUIController* This = (cUIController*)pClass;
	WindowClass.SendChangeClass(16);
	WindowClass.SetVisible(false);
}

void cWindowClass::ChangeClassEFL(LPVOID pClass)
{
	if (GetTickCount() - gInterface.Data[eWindowChangeClass].EventTick < 300) return;
	cUIController* This = (cUIController*)pClass;
	WindowClass.SendChangeClass(32);
	WindowClass.SetVisible(false);
}

void cWindowClass::ChangeClassMG(LPVOID pClass)
{
	if (GetTickCount() - gInterface.Data[eWindowChangeClass].EventTick < 300) return;
	cUIController* This = (cUIController*)pClass;
	WindowClass.SendChangeClass(48);
	WindowClass.SetVisible(false);
}

void cWindowClass::ChangeClassDL(LPVOID pClass)
{
	if (GetTickCount() - gInterface.Data[eWindowChangeClass].EventTick < 300) return;
	cUIController* This = (cUIController*)pClass;
	WindowClass.SendChangeClass(64);
	WindowClass.SetVisible(false);
}

void cWindowClass::ChangeClassSUM(LPVOID pClass)
{
	if (GetTickCount() - gInterface.Data[eWindowChangeClass].EventTick < 300) return;
	cUIController* This = (cUIController*)pClass;
	WindowClass.SendChangeClass(80);
	WindowClass.SetVisible(false);
}

void cWindowClass::ChangeClassRF(LPVOID pClass)
{
	if (GetTickCount() - gInterface.Data[eWindowChangeClass].EventTick < 300) return;
	cUIController* This = (cUIController*)pClass;
	WindowClass.SendChangeClass(96);
	WindowClass.SetVisible(false);
}


void cWindowClass::DrawButton(int IMGcode, float x, float y, float w, float h, float a6, float a7)
{
	if (SEASON3B::CheckMouseIn(x, y, w, h))
	{
		if (GetKeyState(VK_LBUTTON) & 0x8000)
		{
			RenderBitmap(IMGcode, x, y, w, h, 0.0, h / a7 * 2.0, w / a6, h / a7, 1, 1, 0.0);
		}
		else {
			RenderBitmap(IMGcode, x, y, w, h, 0.0, h / a7, w / a6, h / a7, 1, 1, 0.0);
		}
		return;
	}
	else
	{
		RenderBitmap(IMGcode, x, y, w, h, 0.0, 0.0, w / a6, h / a7, 1, 1, 0.0);
	}
	return;
}

void cWindowClass::CreateText()
{
	//zText1.SetFont(CreateFontA(13, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3, 0, "Tahoma"));
	//zText2.SetFont(CreateFontA(13, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3, 0, "Tahoma"));
	//zText3.SetFont(CreateFontA(13, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3, 0, "Tahoma"));
	//zText4.SetFont(CreateFontA(13, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3, 0, "Tahoma"));
	//zText5.SetFont(CreateFontA(13, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3, 0, "Tahoma"));
	//zText6.SetFont(CreateFontA(13, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3, 0, "Tahoma"));
	//zText7.SetFont(CreateFontA(13, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3, 0, "Tahoma"));
	////alpha B G R
	for (int st = 0; st < 50; st++)
	{
		if (strcmp(gOther.Text_InfoDW[st], "Null"))
		{
			//zText1.AddText(1, (st == 0) ? 3 : 1, (st == 0) ? 0xFF00FFFF : 0xFFffffff, 0x0, gOther.Text_InfoDW[st], 1);
			if(m_pTextBoxChangeClass[0])m_pTextBoxChangeClass[0]->AddText(gOther.Text_InfoDW[st]);
		}
		if (strcmp(gOther.Text_InfoDK[st], "Null"))
		{
			//zText2.AddText(1, (st == 0) ? 3 : 1, (st == 0) ? 0xFF00FFFF : 0xFFffffff, 0x0, gOther.Text_InfoDK[st], 1);
			if (m_pTextBoxChangeClass[1])m_pTextBoxChangeClass[1]->AddText(gOther.Text_InfoDK[st]);
		}
		if (strcmp(gOther.Text_InfoELF[st], "Null"))
		{
			//zText3.AddText(1, (st == 0) ? 3 : 1, (st == 0) ? 0xFF00FFFF : 0xFFffffff, 0x0, gOther.Text_InfoELF[st], 1);
			if (m_pTextBoxChangeClass[2])m_pTextBoxChangeClass[2]->AddText(gOther.Text_InfoELF[st]);
		}
		if (strcmp(gOther.Text_InfoMG[st], "Null"))
		{
			//zText4.AddText(1, (st == 0) ? 3 : 1, (st == 0) ? 0xFF00FFFF : 0xFFffffff, 0x0, gOther.Text_InfoMG[st], 1);
			if (m_pTextBoxChangeClass[3])m_pTextBoxChangeClass[3]->AddText(gOther.Text_InfoMG[st]);
		}
		if (strcmp(gOther.Text_InfoDL[st], "Null"))
		{
			//zText5.AddText(1, (st == 0) ? 3 : 1, (st == 0) ? 0xFF00FFFF : 0xFFffffff, 0x0, gOther.Text_InfoDL[st], 1);
			if (m_pTextBoxChangeClass[4])m_pTextBoxChangeClass[4]->AddText(gOther.Text_InfoDL[st]);
		}
		if (strcmp(gOther.Text_InfoSUM[st], "Null"))
		{
			//zText6.AddText(1, (st == 0) ? 3 : 1, (st == 0) ? 0xFF00FFFF : 0xFFffffff, 0x0, gOther.Text_InfoSUM[st], 1);
			if (m_pTextBoxChangeClass[5])m_pTextBoxChangeClass[5]->AddText(gOther.Text_InfoSUM[st]);
		}
		if (strcmp(gOther.Text_InfoRF[st], "Null"))
		{
			//zText7.AddText(1, (st == 0) ? 3 : 1, (st == 0) ? 0xFF00FFFF : 0xFFffffff, 0x0, gOther.Text_InfoRF[st], 1);
			if (m_pTextBoxChangeClass[6])m_pTextBoxChangeClass[6]->AddText(gOther.Text_InfoRF[st]);
		}
	}
	//zText1.AddText(1, 3, 0xFF00FFFF, 0x0, "Giới Thiệu Về Phù Thủy",1);

	//zText1.AddText(1, 1, 0xFFffffff, 0x0, "Một số người cố gắng để có những thân thể cường tráng", 1);
	//zText1.AddText(1, 1, 0xFFffffff, 0x0, "những quy luật của thiên nhiên để làm lợi cho chính họ.", 1);
	//zText1.AddText(1, 1, 0xFFffffff, 0x0, "Phù Thủy có thể dùng các phép thuật để tấn công kẻ thù.", 1);
	//zText1.AddText(1, 1, 0xFFffffff, 0x0, "Ngoài ra, anh còn ta có thể yêu cầu sự hỗ trợ từ những", 1);
	//zText1.AddText(1, 1, 0xFFffffff, 0x0, "linh hồn bí ẩn. Mọi người đều e ngại sức mạnh phép thuật", 1);
	//zText1.AddText(1, 1, 0xFFffffff, 0x0, "của Phù Thủy nên anh ta thường sống một mình. Tuy nhiên", 1);
	//zText1.AddText(1, 1, 0xFFffffff, 0x0, "vẻ bề ngoài yếu ớt có thể làm bạn có suy nghĩ sai lệch", 1);
	//zText1.AddText(1, 1, 0xFFffffff, 0x0, "về anh ta, đừng để  bị đánh lừa. Nhiều chiến binh", 1);
	//zText1.AddText(1, 1, 0xFFffffff, 0x0, "đã bất ngờ bị nhấn chìm trong lửa trước khi chạm đến Phù Thủy", 1);
	//zText1.AddText(1, 1, 0xFFffffff, 0x0, "Phù Thủy có một danh sách phép thuật tấn công riêng.", 1);
	//zText1.AddText(1, 1, 0xFFffffff, 0x0, "Dù có sức mạnh nhưng nhiều phép thuậtcủa họ yêu cầu", 1);
	//zText1.AddText(1, 1, 0xFFffffff, 0x0, "thời gian để tìm và tốn một lượng năng lượng khổng lồ để thi triển.", 1);

	//zText2.SetFont(CreateFontA(13, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3, 0, "Tahoma"));
	////alpha B G R
	//zText2.AddText(1, 3, 0xFF00FFFF, 0x0, "Giới Thiệu Về Chiến Binh", 1);
	//zText2.AddText(1, 1, 0xFFffffff, 0x0, "Chiến Binh là hiện thân của sức mạnh và quyền lực.", 1);
	//zText2.AddText(1, 1, 0xFFffffff, 0x0, "Trong những trận đánh càn, họ có thể giết chết ", 1);
	//zText2.AddText(1, 1, 0xFFffffff, 0x0, "những dòng nhân vật khác ngay khi đã bị thương rất nặng.", 1);
	//zText2.AddText(1, 1, 0xFFffffff, 0x0, "Nhờ khả năng sống còn tuyệt vời, Chiến Binh có khả năng mạo hiểm", 1);
	//zText2.AddText(1, 1, 0xFFffffff, 0x0, "một mình mà không hề gặp bất kỳ khó khăn nào.", 1);
	//zText2.AddText(1, 1, 0xFFffffff, 0x0, "Anh ta có thể mang trên người những loại giáp trụ khác nhau", 1);
	//zText2.AddText(1, 1, 0xFFffffff, 0x0, "sử dụng những loại vũ khí khác nhau như Gươm, Búa hay Giáo.", 1);
	//zText2.AddText(1, 1, 0xFFffffff, 0x0, "Kỹ năng của Chiến Binh không tốn quá nhiều năng lượng.", 1);
	//zText2.AddText(1, 1, 0xFFffffff, 0x0, "Vì thế Chiến Binh thường là sự lựa chọn của những người mới chơi.", 1);
	//zText2.AddText(1, 1, 0xFFffffff, 0x0, "Thêm vào đó,Chiến Binh sẽ có thêm kỹ năng đặc biệt", 1);
	//zText2.AddText(1, 1, 0xFFffffff, 0x0, "khi cưỡi trên Quái điểu.", 1);


	//zText3.SetFont(CreateFontA(13, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3, 0, "Tahoma"));
	////alpha B G R
	//zText3.AddText(1, 3, 0xFF00FFFF, 0x0, "Giới Thiệu Về Tiên Nữ", 1);
	//zText3.AddText(1, 1, 0xFFffffff, 0x0, "Tiên Nữ của vùng Noria là nhân vật thuộc những dòng tộc cổ xưa.", 1);
	//zText3.AddText(1, 1, 0xFFffffff, 0x0, "Họ cho rằng họ là dòng tộc đầu tiên của MU trước cả con người ", 1);
	//zText3.AddText(1, 1, 0xFFffffff, 0x0, ".Một Tiên Nữ ở Noria đã truyền lại rằng họ có một vẻ đẹp huyền ảo", 1);
	//zText3.AddText(1, 1, 0xFFffffff, 0x0, "Tuy họ có vẻ tránh né những trận chiến nhưng không thể nói rằng", 1);
	//zText3.AddText(1, 1, 0xFFffffff, 0x0, "họ nhút nhát. Các Tiên Nữ đã dạy cho con người những kỹ năng ", 1);
	//zText3.AddText(1, 1, 0xFFffffff, 0x0, "dùng cung điệu nghệ. Không ai có thể chiến thắng Tiên Nữ", 1);
	//zText3.AddText(1, 1, 0xFFffffff, 0x0, "trong một trận chiến bằng cung. Những Tiên Nữ của Noria", 1);
	//zText3.AddText(1, 1, 0xFFffffff, 0x0, "cũng có những giao ước với các dòng tộc khác", 1);
	//zText3.AddText(1, 1, 0xFFffffff, 0x0, "và những hiệp ước đó đã cuốn họ vào các cuộc chiến.", 1);


	//zText4.SetFont(CreateFontA(13, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3, 0, "Tahoma"));
	////alpha B G R
	//zText4.AddText(1, 3, 0xFF00FFFF, 0x0, "Giới Thiệu Về Đấu Sĩ", 1);
	//zText4.AddText(1, 1, 0xFFffffff, 0x0, "Đấu Sĩ có thể sử dụng một loạt các kỹ năng mà Chiến Binh,Phù Thủy", 1);
	//zText4.AddText(1, 1, 0xFFffffff, 0x0, "Mặc dù các kỹ năng chiến đấu và phép thuật không hoàn hảo như", 1);
	//zText4.AddText(1, 1, 0xFFffffff, 0x0, "Chiến Binh hay Phù Thủy nhưng sự kết hợp này làm anh ta là một", 1);
	//zText4.AddText(1, 1, 0xFFffffff, 0x0, "nhân vật đột phá nhất trong các dòng nhân vật.", 1);
	//zText4.AddText(1, 1, 0xFFffffff, 0x0, "Đấu Sĩ có khả năng thi triển các đòn đánh quyết định,", 1);
	//zText4.AddText(1, 1, 0xFFffffff, 0x0, "họ có thể sử dụng các món đồ mạnh hay đi vào những vùng đất ", 1);
	//zText4.AddText(1, 1, 0xFFffffff, 0x0, "khác nhau với yêu cầu cấp độ thấp hơn các dòng nhân vật khác.", 1);
	//zText4.AddText(1, 1, 0xFFffffff, 0x0, "cũng có những giao ước với các dòng tộc khác", 1);
	//zText4.AddText(1, 1, 0xFFffffff, 0x0, "Cuối cùng để có được Đấu Sĩ bạn phải đạt cấp độ 220", 1);


	//zText5.SetFont(CreateFontA(13, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3, 0, "Tahoma"));
	////alpha B G R
	//zText5.AddText(1, 3, 0xFF00FFFF, 0x0, "Giới Thiệu Về Chúa Tể", 1);
	//zText5.AddText(1, 1, 0xFFffffff, 0x0, "Chúa Tể vung gươm ra hiệu với uy lực của một vị lãnh tụ", 1);
	//zText5.AddText(1, 1, 0xFFffffff, 0x0, "trong các trận chiến. Họ có sức mạnh triệu hồi quạ tinh và chiến mã", 1);
	//zText5.AddText(1, 1, 0xFFffffff, 0x0, "giúp họ đàn áp kẻ thù của mình. Như Đấu Sĩ,họ có thể xâm nhập vào", 1);
	//zText5.AddText(1, 1, 0xFFffffff, 0x0, "những vùng đất và mang trên người những món đồ tốt với ", 1);
	//zText5.AddText(1, 1, 0xFFffffff, 0x0, "yêu cầu thấp hơn các dòng nhân vật khác. Bên cạnh đó,", 1);
	//zText5.AddText(1, 1, 0xFFffffff, 0x0, "họ có những phép thuật mới và kỹ thuật chưa ", 1);
	//zText5.AddText(1, 1, 0xFFffffff, 0x0, "từng thấy trên lục địa MU. Cuối cùng, để có được Chúa Tể ", 1);
	//zText5.AddText(1, 1, 0xFFffffff, 0x0, "bạn phải  đạt cấp độ 250.", 1);

	//zText6.SetFont(CreateFontA(13, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3, 0, "Tahoma"));
	////alpha B G R
	//zText6.AddText(1, 3, 0xFF00FFFF, 0x0, "Giới Thiệu Về Thuật Sĩ", 1);
	//zText6.AddText(1, 1, 0xFFffffff, 0x0, "Thuật Sĩ được ra đời từ Elbeland, ngôi làng của người ẩn dật", 1);
	//zText6.AddText(1, 1, 0xFFffffff, 0x0, "nằm ở phía Tây Nam của lục địa MU.", 1);
	//zText6.AddText(1, 1, 0xFFffffff, 0x0, "Theo truyền thuyết lâu đời, đây là nơi sinh sống của những người có", 1);
	//zText6.AddText(1, 1, 0xFFffffff, 0x0, "khả năng siêu nhiên, họ rất nhạy cảm và dễ dàng giao tiếp được ", 1);
	//zText6.AddText(1, 1, 0xFFffffff, 0x0, "với các linh hồn chưa siêu thoát. Do đặc điểm nguồn gốc đó,", 1);
	//zText6.AddText(1, 1, 0xFFffffff, 0x0, "Summoner được thừa hưởng tất cả những phép thuật của tổ tiên", 1);
	//zText6.AddText(1, 1, 0xFFffffff, 0x0, "Ngoài ra họ có kỹ năng hút máu và gọi linh hồn", 1);
	//zText6.AddText(1, 1, 0xFFffffff, 0x0, "để hỗ trợ cho bản thân. Đối thủ khi bị linh hồn đánh trúng", 1);
	//zText6.AddText(1, 1, 0xFFffffff, 0x0, "sẽ bị ếm bùa và mất một lượng máu khá lớn.", 1);

	//zText7.SetFont(CreateFontA(13, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3, 0, "Tahoma"));
	////alpha B G R
	//zText7.AddText(1, 3, 0xFF00FFFF, 0x0, "Giới Thiệu Về Thiết Binh", 1);
	//zText7.AddText(1, 1, 0xFFffffff, 0x0, "Thiết Binh (Rage Fighter) thuộc hậu duệ của quân đoàn Royal Knights", 1);
	//zText7.AddText(1, 1, 0xFFffffff, 0x0, "tại vương quốc Kalrutan, Thiết Binh đã thành công khi ", 1);
	//zText7.AddText(1, 1, 0xFFffffff, 0x0, "sử dụng chiến thuật dùng sức mạnh cơ thể làm điểm mạnh. ", 1);
	//zText7.AddText(1, 1, 0xFFffffff, 0x0, "Với lối di chuyển nhanh nhẹn và ra đòn bất ngờ,", 1);
	//zText7.AddText(1, 1, 0xFFffffff, 0x0, "Thiết Binh luôn tạo cho đối phương rơi vào thế bị động ", 1);
	//zText7.AddText(1, 1, 0xFFffffff, 0x0, "không kịp trở tay và hạ gục đối phương một cách nhanh chóng ", 1);
	//zText7.AddText(1, 1, 0xFFffffff, 0x0, "bằng thân hình đồ sộ của mình. Ngoài ra nhân vật này cũng ", 1);
	//zText7.AddText(1, 1, 0xFFffffff, 0x0, "có khả năng tiêu diệt kẻ địch từ xa với sức sát thương lớn ", 1);
	//zText7.AddText(1, 1, 0xFFffffff, 0x0, "từ năng lượng bản thân.", 1);
}
