// NewUIOptionWindow.cpp: implementation of the CNewUIOptionWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIOptionWindow.h"
#include "NewUISystem.h"
#include "ZzzTexture.h"
#include "DSPlaySound.h"
#include "ZzzInterface.h"
#include "CBInterface.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIOptionWindow::CNewUIOptionWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = 0;
	m_Pos.y = 0;

	m_bAutoAttack = true;
	m_bWhisperSound = false;
	m_bSlideHelp = true;
	m_iVolumeLevel = 0;
	m_iRenderLevel = 4;
	for (int i = 0; i < eEndOnOffGrap; i++) this->OnOffGrap[i] = true;

	//==Graphics
	this->OnOffGrap[eGlowEffect] = GetPrivateProfileInt("Graphics", "GlowEffect", 1, ".\\config.ini");
	this->OnOffGrap[eEffectDynamic] = GetPrivateProfileInt("Graphics", "EffectDynamic", 1, ".\\config.ini");
	this->OnOffGrap[eEffectStatic] = GetPrivateProfileInt("Graphics", "EffectStatic", 1, ".\\config.ini");
	this->OnOffGrap[eBMDPlayer] = GetPrivateProfileInt("Graphics", "BMDPlayer",1, ".\\config.ini");
	this->OnOffGrap[eBMDWings] = GetPrivateProfileInt("Graphics", "BMDWings", 1, ".\\config.ini");
	this->OnOffGrap[eBMDWeapons] = GetPrivateProfileInt("Graphics", "BMDWeapons", 1, ".\\config.ini");
	this->OnOffGrap[eBMDImg] = GetPrivateProfileInt("Graphics", "BMDImg", 1 ,".\\config.ini");
	this->OnOffGrap[eBMDMonter] = GetPrivateProfileInt("Graphics", "BMDMonter", 1, ".\\config.ini");
	this->OnOffGrap[eRenderObjects] = GetPrivateProfileInt("Graphics", "RenderObjects", 1, ".\\config.ini");
	this->OnOffGrap[eRenderTerrain] = GetPrivateProfileInt("Graphics", "RenderTerrain", 1, ".\\config.ini");
	this->OnOffGrap[eExcellentEffect] = GetPrivateProfileInt("Graphics", "ExcellentEffect", 1, ".\\config.ini");
	this->OnOffGrap[eBMDZen] = GetPrivateProfileInt("Graphics", "BMDZen", 1, ".\\config.ini");
}

SEASON3B::CNewUIOptionWindow::~CNewUIOptionWindow()
{
	Release();
}

bool SEASON3B::CNewUIOptionWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_OPTION, this);
	SetPos(x, y);
	LoadImages();
	SetButtonInfo();
	Show(false);
	return true;
}

void SEASON3B::CNewUIOptionWindow::SetButtonInfo()
{
	m_BtnClose.ChangeTextBackColor(RGBA(255,255,255,0));
	m_BtnClose.ChangeButtonImgState( true, IMAGE_OPTION_BTN_CLOSE, true );
	m_BtnClose.ChangeButtonInfo(m_Pos.x+68, m_Pos.y+209, 54, 30);
	m_BtnClose.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnClose.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
}

void SEASON3B::CNewUIOptionWindow::Release()
{
	UnloadImages();
	
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}


void SEASON3B::CNewUIOptionWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool SEASON3B::CNewUIOptionWindow::UpdateMouseEvent()
{
	if(m_BtnClose.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_OPTION);
		return false;
	}

	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x+150, m_Pos.y+43, 15, 15))
	{
		m_bAutoAttack = !m_bAutoAttack;
	}
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x+150, m_Pos.y+65, 15, 15))
	{
		m_bWhisperSound = !m_bWhisperSound;
	}
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x+150, m_Pos.y+127, 15, 15))
	{
		m_bSlideHelp = !m_bSlideHelp;
	}
	
	if(CheckMouseIn(m_Pos.x+33-8, m_Pos.y+104, 124+8, 16))
	{
		int iOldValue = m_iVolumeLevel;
		if(MouseWheel > 0)
		{
			MouseWheel = 0;
			m_iVolumeLevel++;
			if(m_iVolumeLevel > 10)
			{
				m_iVolumeLevel = 10;
			}
		}
		else if(MouseWheel < 0)
		{
			MouseWheel = 0;
			m_iVolumeLevel--;
			if(m_iVolumeLevel < 0)
			{
				m_iVolumeLevel = 0;
			}
		}
		if(SEASON3B::IsRepeat(VK_LBUTTON))
		{
			int x = MouseX - (m_Pos.x + 33);
			if(x < 0)
			{
				m_iVolumeLevel = 0;
			}
			else
			{
				float fValue = (10.f * x) / 124.f;
				m_iVolumeLevel = (int)fValue + 1;
			}
		}

		if(iOldValue != m_iVolumeLevel)
		{
			SetEffectVolumeLevel(m_iVolumeLevel);
		}
	}
	if(CheckMouseIn(m_Pos.x+25, m_Pos.y+168, 141, 29))
	{
		if(SEASON3B::IsRepeat(VK_LBUTTON))
		{
			int x = MouseX - (m_Pos.x + 25);
			float fValue = (5.f * x) / 141.f;
			m_iRenderLevel = (int)fValue;
		}
	}

	if(CheckMouseIn(m_Pos.x, m_Pos.y, 190, 249) == true)
	{
		return false;
	}

	return true;
}

bool SEASON3B::CNewUIOptionWindow::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_OPTION) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_OPTION);
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}

	return true;
}

bool SEASON3B::CNewUIOptionWindow::Update()
{
	return true;
}

bool SEASON3B::CNewUIOptionWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	RenderFrame();
	RenderContents();
	RenderButtons();
	RenderCustomFrame();
	DisableAlphaBlend();
	return true;
}

float SEASON3B::CNewUIOptionWindow::GetLayerDepth()	//. 10.5f
{
	return 10.5f;
}

float SEASON3B::CNewUIOptionWindow::GetKeyEventOrder()	// 10.f;
{
	return 10.0f;
}

void SEASON3B::CNewUIOptionWindow::OpenningProcess()
{
	gInterface.Data[eMenu_OPTION].Open();
}

void SEASON3B::CNewUIOptionWindow::ClosingProcess()
{
	gInterface.Data[eMenu_OPTION].Close();
}

void SEASON3B::CNewUIOptionWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_button_close.tga", IMAGE_OPTION_BTN_CLOSE, GL_LINEAR);
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_OPTION_FRAME_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_OPTION_FRAME_DOWN, GL_LINEAR);	
	LoadBitmap("Interface\\newui_option_top.tga", IMAGE_OPTION_FRAME_UP, GL_LINEAR);	
	LoadBitmap("Interface\\newui_option_back06(L).tga", IMAGE_OPTION_FRAME_LEFT, GL_LINEAR);	
	LoadBitmap("Interface\\newui_option_back06(R).tga", IMAGE_OPTION_FRAME_RIGHT, GL_LINEAR);	
	LoadBitmap("Interface\\newui_option_line.jpg", IMAGE_OPTION_LINE, GL_LINEAR);	
	LoadBitmap("Interface\\newui_option_point.tga", IMAGE_OPTION_POINT, GL_LINEAR);	
	LoadBitmap("Interface\\newui_option_check.tga", IMAGE_OPTION_BTN_CHECK, GL_LINEAR);	
	LoadBitmap("Interface\\newui_option_effect03.tga", IMAGE_OPTION_EFFECT_BACK, GL_LINEAR);	
	LoadBitmap("Interface\\newui_option_effect04.tga", IMAGE_OPTION_EFFECT_COLOR, GL_LINEAR);	
	LoadBitmap("Interface\\newui_option_volume01.tga", IMAGE_OPTION_VOLUME_BACK, GL_LINEAR);	
	LoadBitmap("Interface\\newui_option_volume02.tga", IMAGE_OPTION_VOLUME_COLOR, GL_LINEAR);	
}

void SEASON3B::CNewUIOptionWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_OPTION_BTN_CLOSE);
	DeleteBitmap(IMAGE_OPTION_FRAME_BACK);
	DeleteBitmap(IMAGE_OPTION_FRAME_DOWN);
	DeleteBitmap(IMAGE_OPTION_FRAME_UP);
	DeleteBitmap(IMAGE_OPTION_FRAME_LEFT);
	DeleteBitmap(IMAGE_OPTION_FRAME_RIGHT);
	DeleteBitmap(IMAGE_OPTION_LINE);
	DeleteBitmap(IMAGE_OPTION_POINT);
	DeleteBitmap(IMAGE_OPTION_BTN_CHECK);
	DeleteBitmap(IMAGE_OPTION_EFFECT_BACK);
	DeleteBitmap(IMAGE_OPTION_EFFECT_COLOR);
	DeleteBitmap(IMAGE_OPTION_VOLUME_BACK);
	DeleteBitmap(IMAGE_OPTION_VOLUME_COLOR);
}

void SEASON3B::CNewUIOptionWindow::RenderFrame()
{
	//float x, y;
	//x = m_Pos.x;
	//y = m_Pos.y;
	//RenderImage(IMAGE_OPTION_FRAME_BACK, x, y, 190.f, 285);
	//RenderImage(IMAGE_OPTION_FRAME_UP, x, y, 190.f, 64.f);
	//y += 64.f;
	//for(int i=0; i<18; ++i)
	//{
	//	RenderImage(IMAGE_OPTION_FRAME_LEFT, x, y, 21.f, 10.f);
	//	RenderImage(IMAGE_OPTION_FRAME_RIGHT, x+190-21, y, 21.f, 10.f);
	//	y += 10.f;
	//}
	//RenderImage(IMAGE_OPTION_FRAME_DOWN, x, y, 190.f, 45.f);

	//y = m_Pos.y + 60.f;
	//RenderImage(IMAGE_OPTION_LINE, x+18, y, 154.f, 2.f);
	//y += 22.f;
	//RenderImage(IMAGE_OPTION_LINE, x+18, y, 154.f, 2.f);
	//y += 40.f;
	//RenderImage(IMAGE_OPTION_LINE, x+18, y, 154.f, 2.f);
	//y += 22.f;
	//RenderImage(IMAGE_OPTION_LINE, x + 18, y, 154.f, 2.f);
	//y += 60.f;
	//RenderImage(IMAGE_OPTION_LINE, x+18, y, 154.f, 2.f);
	if (!gInterface.Data[eMenu_OPTION].OnShow)
	{
		g_pNewUISystem->Hide(INTERFACE_OPTION);
		return;
	}
	float MainWidth = 360;
	float MainHeight = 275;
	float StartX = (MAX_WIN_WIDTH / 2) - (MainWidth / 2);
	float StartY = m_Pos.y;
	//--
	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, MainWidth, MainHeight, eMenu_OPTION, "Config System"); //
	SetPos(StartX, StartY);
	m_BtnClose.SetPos(m_Pos.x + (MainWidth / 2) - 30, m_Pos.y + (MainHeight-50));

	float x, y;
	x = m_Pos.x;
	y = m_Pos.y;


	y = m_Pos.y + 60.f;
	RenderImage(IMAGE_OPTION_LINE, x + 18, y, 154.f, 2.f);
	y += 22.f;
	RenderImage(IMAGE_OPTION_LINE, x + 18, y, 154.f, 2.f);
	y += 40.f;
	RenderImage(IMAGE_OPTION_LINE, x + 18, y, 154.f, 2.f);
	y += 22.f;
	RenderImage(IMAGE_OPTION_LINE, x + 18, y, 154.f, 2.f);
	y += 60.f;
	RenderImage(IMAGE_OPTION_LINE, x + 18, y, 154.f, 2.f);
}
//==Config Custom
void SEASON3B::CNewUIOptionWindow::RenderCustomFrame()
{
	float x, y;
	x = m_Pos.x +175;
	y = m_Pos.y + 46.f;
	//===OPtion Custom
	RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);
	g_pRenderText->RenderText(x + 20, y, "On/Off Graphics");

	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15, y + 15, 0xFFCC00C8, this->OnOffGrap[eGlowEffect] == 1 ? TRUE : FALSE, "Glow Efflect"))
	{
		this->OnOffGrap[eGlowEffect] ^= 1;
	}
	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15 + 80, y + 15, 0xFFCC00C8, this->OnOffGrap[eEffectDynamic] == 1 ? TRUE : FALSE, "Effect Dynamic"))
	{
		this->OnOffGrap[eEffectDynamic] ^= 1;
	}
	y = y + 17;
	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15, y + 15, 0xFFCC00C8, this->OnOffGrap[eEffectStatic] == 1 ? TRUE : FALSE, "Effect Static"))
	{
		this->OnOffGrap[eEffectStatic] ^= 1;
	}
	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15 + 80, y + 15, 0xFFCC00C8, this->OnOffGrap[eBMDPlayer] == 1 ? TRUE : FALSE, "BMD Player"))
	{
		this->OnOffGrap[eBMDPlayer] ^= 1;
	}
	y = y + 17;
	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15, y + 15, 0xFFCC00C8, this->OnOffGrap[eBMDWings] == 1 ? TRUE : FALSE, "BMD Wings"))
	{
		this->OnOffGrap[eBMDWings] ^= 1;
	}
	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15 + 80, y + 15, 0xFFCC00C8, this->OnOffGrap[eBMDWeapons] == 1 ? TRUE : FALSE, "BMD Weapons"))
	{
		this->OnOffGrap[eBMDWeapons] ^= 1;
	}
	y = y + 17;
	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15 , y + 15, 0xFFCC00C8, this->OnOffGrap[eBMDImg] == 1 ? TRUE : FALSE, "BMD Img"))
	{
		this->OnOffGrap[eBMDImg] ^= 1;
	}
	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15 + 80, y + 15, 0xFFCC00C8, this->OnOffGrap[eBMDMonter] == 1 ? TRUE : FALSE, "BMD Monter"))
	{
		this->OnOffGrap[eBMDMonter] ^= 1;
	}
	y = y + 17;
	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15 , y + 15, 0xFFCC00C8, this->OnOffGrap[eRenderObjects] == 1 ? TRUE : FALSE, "Objects"))
	{
		this->OnOffGrap[eRenderObjects] ^= 1;
	}
	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15 + 80, y + 15, 0xFFCC00C8, this->OnOffGrap[eRenderTerrain] == 1 ? TRUE : FALSE, "Terrain"))
	{
		this->OnOffGrap[eRenderTerrain] ^= 1;
	}
	y = y + 17;
	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15, y + 15, 0xFFCC00C8, this->OnOffGrap[eExcellentEffect] == 1 ? TRUE : FALSE, "Excellent Effect"))
	{
		this->OnOffGrap[eExcellentEffect] ^= 1;
	}
	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15 + 80, y + 15, 0xFFCC00C8, this->OnOffGrap[eBMDZen] == 1 ? TRUE : FALSE, "BMD Zen Drop"))
	{
		this->OnOffGrap[eBMDZen] ^= 1;
	}
	//==================
	y = y + 40;
	RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);
	g_pRenderText->RenderText(x+20, y, "On/Off Custom");

	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15, y+15, 0xFFCC00C8, mShowHPBar == 1 ? TRUE : FALSE, "Show HP Bar"))
	{
		mShowHPBar ^= 1;
	}
	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15 + 80,y + 15, 0xFFCC00C8, mShowName == 1 ? TRUE : FALSE, "Show Name"))
	{
		mShowName ^= 1;
		g_bGMObservation = mShowName;
	}
	y = y + 17;
	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15, y + 15, 0xFFCC00C8, mShowMiniMap == 1 ? TRUE : FALSE, "Show Minimap"))
	{
		mShowMiniMap ^= 1;
	}
	if (g_pBCustomMenuInfo->RenderCheckBox(x + 15+80, y + 15, 0xFFCC00C8, mShowDanhHieu == 1 ? TRUE : FALSE, "Show Danh Hiệu"))
	{
		mShowDanhHieu ^= 1;
	}
}
void SEASON3B::CNewUIOptionWindow::RenderContents()
{
	float x, y;
	x = m_Pos.x + 20.f;
	y = m_Pos.y + 46.f;
	RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);
	y += 22.f;
	RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);
	y += 22.f;
	RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);
	y += 40.f;
	RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);
	y += 22.f;
	RenderImage(IMAGE_OPTION_POINT, x, y, 10.f, 10.f);
	
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->RenderText(m_Pos.x+40, m_Pos.y+48, GlobalText[386]);
	g_pRenderText->RenderText(m_Pos.x+40, m_Pos.y+70, GlobalText[387]);
	g_pRenderText->RenderText(m_Pos.x+40, m_Pos.y+92, GlobalText[389]);
	g_pRenderText->RenderText(m_Pos.x+40, m_Pos.y+132, GlobalText[919]);
	g_pRenderText->RenderText(m_Pos.x+40, m_Pos.y+154, GlobalText[1840]);


}

void SEASON3B::CNewUIOptionWindow::RenderButtons()
{
	m_BtnClose.Render();

	if(m_bAutoAttack)
	{
		RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x+150, m_Pos.y+43, 15, 15, 0, 0);
	}
	else 
	{
		RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x+150, m_Pos.y+43, 15, 15, 0, 15.f);
	}

	if(m_bWhisperSound)
	{
		RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x+150, m_Pos.y+65, 15, 15, 0, 0);
	}
	else
	{
		RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x+150, m_Pos.y+65, 15, 15, 0, 15.f);
	}

	if(m_bSlideHelp)
	{
		RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x+150, m_Pos.y+127, 15, 15, 0, 0);
	}
	else
	{
		RenderImage(IMAGE_OPTION_BTN_CHECK, m_Pos.x+150, m_Pos.y+127, 15, 15, 0, 15.f);
	}

	RenderImage(IMAGE_OPTION_VOLUME_BACK, m_Pos.x+33, m_Pos.y+104, 124.f, 16.f);
	if(m_iVolumeLevel > 0)
	{
		RenderImage(IMAGE_OPTION_VOLUME_COLOR, m_Pos.x+33, m_Pos.y+104, 124.f * 0.1f * 	(m_iVolumeLevel), 16.f);
	}

	RenderImage(IMAGE_OPTION_EFFECT_BACK, m_Pos.x+25, m_Pos.y+168, 141.f, 29.f);
	if(m_iRenderLevel >= 0)
	{
		RenderImage(IMAGE_OPTION_EFFECT_COLOR, m_Pos.x+25, m_Pos.y+168, 141.f * 0.2f * 	(m_iRenderLevel+1), 29.f);
	}

}

void SEASON3B::CNewUIOptionWindow::SetAutoAttack(bool bAuto)
{
	m_bAutoAttack = bAuto;
}

bool SEASON3B::CNewUIOptionWindow::IsAutoAttack()
{
	return m_bAutoAttack;
}

void SEASON3B::CNewUIOptionWindow::SetWhisperSound(bool bSound)
{
	m_bWhisperSound = bSound;
}

bool SEASON3B::CNewUIOptionWindow::IsWhisperSound()
{
	return m_bWhisperSound;
}

void SEASON3B::CNewUIOptionWindow::SetSlideHelp(bool bHelp)
{
	m_bSlideHelp = bHelp;
}

bool SEASON3B::CNewUIOptionWindow::IsSlideHelp()
{
	return m_bSlideHelp;
}

void SEASON3B::CNewUIOptionWindow::SetVolumeLevel(int iVolume)
{
	m_iVolumeLevel = iVolume;
}

int SEASON3B::CNewUIOptionWindow::GetVolumeLevel()
{
	return m_iVolumeLevel;
}

void SEASON3B::CNewUIOptionWindow::SetRenderLevel(int iRender)
{
	m_iRenderLevel = iRender;
}

int SEASON3B::CNewUIOptionWindow::GetRenderLevel()
{
	return m_iRenderLevel;
}
