// NewUIBCustomMenu.cpp: implementation of the CNewUIMacro class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIBMacro.h"
#include "NewUISystem.h"
#include "wsclientinline.h"
#include "MapManager.h"

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayHeight;
extern int DisplayWinMid;
extern int DisplayHeightExt;
extern int DisplayWinExt;
extern int DisplayWinReal;
using namespace SEASON3B;



CNewUIMacro::CNewUIMacro()
{
	m_pNewUIMng = NULL;

	m_Pos.x = m_Pos.y = 0;



}

CNewUIMacro::~CNewUIMacro()
{
	Release();
}

//---------------------------------------------------------------------------------------------
// Create
bool CNewUIMacro::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_MuHelper, this );


	SetPos(x, y);
	LoadImages();

	Show(true); //Set true neu xai GUIObj
	this->WindowAuto = new GUIObj(DisplayWin-190, 50, 210, 420, true);
	

	return true;
}

void CNewUIMacro::Release()
{
	UnloadImages();

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

void CNewUIMacro::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool CNewUIMacro::BtnProcess()
{

	return false;
}

bool CNewUIMacro::UpdateMouseEvent()
{
	if( true == BtnProcess() )
		return false;

	
	return true;
}

bool CNewUIMacro::UpdateKeyEvent()
{
	return true;
}

bool CNewUIMacro::Update()
{
	return true;
}
void  CNewUIMacro::DrawWindowAutoMuHelper()
{
	if (!this->WindowAuto->onShow)
	{
		return ;
	}

	bool res = g_pBCustomMenuInfo->DrawWindowBG(this->WindowAuto, "Auto In Game");
	float StartX = this->WindowAuto->X;
	float StartY = this->WindowAuto->Y;
	float WindowW = this->WindowAuto->Width;
	float WindowH = this->WindowAuto->Height;

	//== Khung Pham Vi
	POINT PosPhamVi = { 0 };
	PosPhamVi.x = StartX + 10;
	PosPhamVi.y = StartY + 65;
	g_pBCustomMenuInfo->DrawInfoBox(PosPhamVi.x, PosPhamVi.y, 70, 40, 0x000000A5);
	TextDraw(g_hFont, PosPhamVi.x + 5, PosPhamVi.y + 5, 0xFFFFFFFF, 0x0, 0, 0, 1, GlobalText[3526]); //Pham Vi
	//============
	glColor3f(1.f, 0.9f, 0.7f);
	SEASON3B::RenderNumber(PosPhamVi.x + 25, PosPhamVi.y + 23, 10, 1.5);
	glColor3f(1.f, 1.f, 1.f);
	DisableAlphaBlend();
	//============
}

bool CNewUIMacro::Render()
{

	CNewUIMacro::DrawWindowAutoMuHelper();
	return true;
}

float CNewUIMacro::GetLayerDepth()
{
	return 1.3f;
}

void CNewUIMacro::OpenningProcess()
{
	
}

void CNewUIMacro::ClosingProcess()
{
	
}


void CNewUIMacro::LoadImages()
{
	//MAx IMAGE_HERO_POSITION_INFO_BASE_WINDOW +5;
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMGA_AUTOINFO, GL_LINEAR);
	LoadBitmap("Custom\\Maps\\CRadar.tga", IMGA_AUTOINFO + 2, GL_LINEAR);
	LoadBitmap("Custom\\Maps\\map_radar.tga", IMGA_AUTOINFO + 4, GL_LINEAR);
	LoadBitmap("Custom\\Maps\\background-tcp.tga", IMGA_AUTOINFO + 5, GL_LINEAR);


}

void CNewUIMacro::UnloadImages()
{

	DeleteBitmap(IMGA_AUTOINFO);
	DeleteBitmap(IMGA_AUTOINFO+2);
	DeleteBitmap(IMGA_AUTOINFO +4);
	DeleteBitmap(IMGA_AUTOINFO +5);
}


