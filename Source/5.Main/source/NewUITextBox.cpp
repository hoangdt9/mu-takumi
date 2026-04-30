//=============================================================================
//	NewUITextBox.cpp
//=============================================================================
#pragma once
#include "StdAfx.h"
#include "NewUITextBox.h"
#include "UIControls.h"
#include "wsclientinline.h"

using namespace SEASON3B;

const int iMAX_TEXT_LINE = 512;
const int iLINE_INTERVAL = 2;

CNewUITextBox::CNewUITextBox()
{
	m_iWidth = 0;
	m_iHeight = 0;
	memset( &m_ptPos, 0, sizeof(POINT) );
	
	m_iTextHeight = 0;
	m_iTextLineHeight = 1;
	m_iLimitLine = 0;
		
	m_iMaxLine = 0;
	m_iCurLine = 0;


	
}

CNewUITextBox::~CNewUITextBox()
{
	Release();
	if (m_pScrollBar)
		m_pScrollBar->Release();

	SAFE_DELETE(m_pScrollBar);
}

bool CNewUITextBox::Create( int iX, int iY, int iWidth ,int iHeight )
{
	m_pScrollBar = new CNewUIScrollBar();
	m_pScrollBar->Create(iX, iY, iHeight - 10);
	SetPos( iX, iY, iWidth, iHeight );
	Show( true );
	return true;
}

void CNewUITextBox::SetPos( int iX, int iY, int iWidth ,int iHeight )
{
	m_ptPos.x = iX;
	m_ptPos.y = iY;
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	SIZE Fontsize;
	g_pRenderText->SetFont(g_hFont);

	unicode::t_string strTemp = _T("A");

		g_pMultiLanguage->_GetTextExtentPoint32( g_pRenderText->GetFontDC(), strTemp.c_str(), strTemp.size(), &Fontsize );

	m_iTextHeight = Fontsize.cy;
	m_iTextLineHeight = m_iTextHeight + iLINE_INTERVAL;

	m_iLimitLine = m_iHeight / m_iTextLineHeight;

	m_iMaxLine = 0;
	m_iCurLine = 0;
	if (m_pScrollBar) m_pScrollBar->SetPos(m_ptPos.x + (m_iWidth - 15), m_ptPos.y);
}

void CNewUITextBox::Release()
{

}

float CNewUITextBox::GetLayerDepth()
{
	return 4.4f;
}

bool CNewUITextBox::UpdateMouseEvent()
{
	if (m_pScrollBar)
		m_pScrollBar->UpdateMouseEvent();

	return true;
}

bool CNewUITextBox::UpdateKeyEvent()
{
	return true;
}

bool CNewUITextBox::Update()
{
	if (this->GetMoveableLine() > 0)
	{
		if (m_pScrollBar)
		{
			m_pScrollBar->Show(true);

			int iMaxPos = this->GetMoveableLine();
			m_pScrollBar->SetMaxPos(iMaxPos);
			m_pScrollBar->Update();
			this->SetCurLine(m_pScrollBar->GetCurPos());
		}
	}
	else
	{
		if (m_pScrollBar)
		{
			m_pScrollBar->Show(false);
		}
	}
 	return true;
}
bool CNewUITextBox::RenderKemScroll()
{
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	CNewUITextBox::Update();
	CNewUITextBox::UpdateMouseEvent();
	for (int iIndex = 0; iIndex < m_iLimitLine; iIndex++)
	{
		int iLineIndex = m_iCurLine + iIndex;

		if (GetLineText(iLineIndex).empty() == false)
		{
			g_pRenderText->SetFont(g_hFont);
			g_pRenderText->SetBgColor(0);
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			g_pRenderText->RenderText(m_ptPos.x, m_ptPos.y + iIndex * m_iTextLineHeight, GetLineText(iLineIndex).c_str(),
				m_iWidth, 0, RT3_SORT_LEFT);
		}
	}
	if (m_pScrollBar)
		m_pScrollBar->MouseWheelWindow = SEASON3B::CheckMouseIn(m_ptPos.x, m_ptPos.y, m_iWidth, m_iHeight);
		m_pScrollBar->Render();
	return true;
}
bool CNewUITextBox::Render()
{
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	for( int iIndex = 0; iIndex < m_iLimitLine; iIndex++ )
	{
		int iLineIndex = m_iCurLine + iIndex;
		
		if( GetLineText( iLineIndex ).empty() == false  )
		{
			g_pRenderText->SetFont( g_hFont );
			//g_pRenderText->SetBgColor( 0, 0, 0, 0 );
			g_pRenderText->SetTextColor( 255, 255, 255, 255 );
			g_pRenderText->RenderText(	m_ptPos.x,	m_ptPos.y + iIndex * m_iTextLineHeight,	GetLineText( iLineIndex ).c_str(),
										m_iWidth,	0,		RT3_SORT_LEFT );
		}
	}

	return true;
}

void CNewUITextBox::AddText( unicode::t_char* strText )
{
	unicode::t_char strTemp[ iMAX_TEXT_LINE ][ iMAX_TEXT_LINE ];
	::memset( strTemp[0], 0, sizeof( char ) * iMAX_TEXT_LINE * iMAX_TEXT_LINE );

	int iTextLine = ::DivideStringByPixel( &strTemp[0][0], iMAX_TEXT_LINE, iMAX_TEXT_LINE, strText, m_iWidth, true, '#' );
	
	for( int iIndex=0;iIndex<iTextLine;iIndex++ )
	{
		m_vecText.push_back( strTemp[ iIndex ] );
	}

	m_pScrollBar->SetMaxPos(GetMoveableLine());
}	

void CNewUITextBox::AddText( const unicode::t_char* strText )
{
	unicode::t_char strTempText[iMAX_TEXT_LINE] = {0,};
	sprintf( strTempText, strText );

	AddText( strTempText );
	m_pScrollBar->SetMaxPos(GetMoveableLine());
}

unicode::t_string CNewUITextBox::GetFullText()
{
	unicode::t_string strTemp;
	
	type_vector_textbase::iterator vi = m_vecText.begin();
	for(; vi != m_vecText.end(); vi++)
	{
		strTemp += (*vi);
	}

	return strTemp;
}

unicode::t_string CNewUITextBox::GetLineText( int iLineIndex )
{
	if( 0 > iLineIndex || (int)m_vecText.size() <= iLineIndex )
		return "";

	return m_vecText[ iLineIndex ];
}

int CNewUITextBox::GetMoveableLine()		
{
	int iMoveableLine = m_vecText.size() - m_iLimitLine;
	if( iMoveableLine <= 0 )
		iMoveableLine = 0;

	return iMoveableLine;
}
