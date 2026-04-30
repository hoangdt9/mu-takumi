//////////////////////////////////////////////////////////////////////
// NewUIMoveCommandWindow.cpp: implementation of the CNewUIMoveCommandWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIMoveCommandWindow.h"
#include "NewUISystem.h"
#include "ZzzInterface.h"
#include "wsclientinline.h"
#include "ChangeRingManager.h"
#include "KeyGenerater.h"
#include "Local.h"
#include "ServerListManager.h"
#include "ZzzOpenData.h"
#include "MapManager.h"
#include "CharacterManager.h"
extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayHeight;
extern int DisplayWinMid;
extern int DisplayHeightExt;
extern int DisplayWinExt;
extern int DisplayWinReal;
#if defined(__ANDROID__) || defined(MU_IOS)
extern bool AndroidGetMoveMapWindowPosition(int panelWidth, int panelHeight, int* outX, int* outY);
#endif
using namespace SEASON3B;

namespace 
{
	const int MapNameCount = 6;

	const std::string MapName[MapNameCount] = 
	{
		"Lorencia",
		"Noria",
		"Elbeland",
		"Dungeon",
		"Devias",
		"LostTower",
	};

	const bool IsLuckySeal( const std::string& name )
	{
		if( name.size() != 0 ) {
			for( int i = 0; i < MapNameCount; ++i)  {
				if( name == MapName[i]) 
				{
					return true;
				}
			}
		}
		return false;
	}

#if defined(__ANDROID__) || defined(MU_IOS)
	const int kAndroidMoveMapColumns = 4;
	const int kAndroidMoveMapRows = 4;
	const int kAndroidMoveMapTileWidth = 114;
	const int kAndroidMoveMapTileHeight = 50;
	const int kAndroidMoveMapTileGapX = 8;
	const int kAndroidMoveMapTileGapY = 8;
	const int kAndroidMoveMapPanelPaddingX = 14;
	const int kAndroidMoveMapPanelPaddingY = 12;
	const int kAndroidMoveMapHeaderHeight = 34;
	const int kAndroidMoveMapFooterHeight = 32;
	const int kAndroidMoveMapCloseButtonSize = 22;
	const int kAndroidMoveMapPageButtonWidth = 44;
	const int kAndroidMoveMapPageButtonHeight = 22;

	int GetAndroidMoveMapItemsPerPage()
	{
		return kAndroidMoveMapColumns * kAndroidMoveMapRows;
	}

	int GetAndroidMoveMapPageCount(int itemCount)
	{
		const int itemsPerPage = GetAndroidMoveMapItemsPerPage();
		if (itemCount <= 0 || itemsPerPage <= 0)
		{
			return 1;
		}

		int pages = itemCount / itemsPerPage;
		if ((itemCount % itemsPerPage) != 0)
		{
			pages++;
		}
		return (pages > 0) ? pages : 1;
	}

	void GetAndroidMoveMapPageRange(int itemCount, int page, int* outStartIndex, int* outEndIndex)
	{
		const int itemsPerPage = GetAndroidMoveMapItemsPerPage();
		const int clampedPage = (page > 1) ? page : 1;
		const int startIndex = (clampedPage - 1) * itemsPerPage;
		const int endIndex = ((startIndex + itemsPerPage) < itemCount) ? (startIndex + itemsPerPage) : itemCount;

		if (outStartIndex != NULL)
		{
			*outStartIndex = startIndex;
		}
		if (outEndIndex != NULL)
		{
			*outEndIndex = endIndex;
		}
	}

	void GetAndroidMoveMapTileRect(int panelX, int panelY, int pageOrder, int* outX, int* outY, int* outW, int* outH)
	{
		const int column = pageOrder % kAndroidMoveMapColumns;
		const int row = pageOrder / kAndroidMoveMapColumns;
		const int tileStepX = kAndroidMoveMapTileWidth + kAndroidMoveMapTileGapX;
		const int tileStepY = kAndroidMoveMapTileHeight + kAndroidMoveMapTileGapY;

		if (outX != NULL)
		{
			*outX = panelX + kAndroidMoveMapPanelPaddingX + (column * tileStepX);
		}
		if (outY != NULL)
		{
			*outY = panelY + kAndroidMoveMapHeaderHeight + (row * tileStepY);
		}
		if (outW != NULL)
		{
			*outW = kAndroidMoveMapTileWidth;
		}
		if (outH != NULL)
		{
			*outH = kAndroidMoveMapTileHeight;
		}
	}

	void GetAndroidMoveMapCloseButtonRect(int panelX, int panelY, int panelWidth, int* outX, int* outY, int* outW, int* outH)
	{
		if (outX != NULL)
		{
			*outX = panelX + panelWidth - kAndroidMoveMapPanelPaddingX - kAndroidMoveMapCloseButtonSize;
		}
		if (outY != NULL)
		{
			*outY = panelY + ((kAndroidMoveMapHeaderHeight - kAndroidMoveMapCloseButtonSize) / 2);
		}
		if (outW != NULL)
		{
			*outW = kAndroidMoveMapCloseButtonSize;
		}
		if (outH != NULL)
		{
			*outH = kAndroidMoveMapCloseButtonSize;
		}
	}

	void GetAndroidMoveMapPageButtonRect(int panelX, int panelY, int panelWidth, int panelHeight, bool nextButton, int* outX, int* outY, int* outW, int* outH)
	{
		const int y = panelY + panelHeight - kAndroidMoveMapFooterHeight + ((kAndroidMoveMapFooterHeight - kAndroidMoveMapPageButtonHeight) / 2);
		const int x = nextButton
			? (panelX + panelWidth - kAndroidMoveMapPanelPaddingX - kAndroidMoveMapPageButtonWidth)
			: (panelX + kAndroidMoveMapPanelPaddingX);

		if (outX != NULL)
		{
			*outX = x;
		}
		if (outY != NULL)
		{
			*outY = y;
		}
		if (outW != NULL)
		{
			*outW = kAndroidMoveMapPageButtonWidth;
		}
		if (outH != NULL)
		{
			*outH = kAndroidMoveMapPageButtonHeight;
		}
	}

	bool IsAndroidMoveMapGridMode()
	{
		return true;
	}
#else
	bool IsAndroidMoveMapGridMode()
	{
		return false;
	}
#endif
};

CNewUIMoveCommandWindow::CNewUIMoveCommandWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;

	memset( &m_StartUISubjectName, 0, sizeof(POINT) );
	memset( &m_StartMapNamePos, 0, sizeof(POINT) );
	memset( &m_MapNameUISize, 0, sizeof(POINT) );
	memset(&m_StrifePos, 0, sizeof(POINT));
	memset( &m_MapNamePos, 0, sizeof(POINT) );
	memset( &m_ReqLevelPos, 0, sizeof(POINT) );
	memset( &m_ReqZenPos, 0, sizeof(POINT) );
	m_iSelectedMapName = -1;

	memset( &m_ScrollBarPos, 0, sizeof(POINT) );
	memset( &m_ScrollBtnStartPos, 0, sizeof(POINT) );
	memset( &m_ScrollBtnPos, 0, sizeof(POINT) );
	m_iScrollBarHeightPixel = 0;
	m_iRenderStartTextIndex = 0;
	m_iSelectedTextIndex = -1;
	m_iScrollBtnInterval = 0;
	m_iScrollBarMiddleNum = 0;
	m_iScrollBarMiddleRemainderPixel = 0;
	m_iNumPage = 0;
	m_iCurPage = 0;
	m_iMousePosY = 0;
	m_bScrollBtnActive = false;
	m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;
}

CNewUIMoveCommandWindow::~CNewUIMoveCommandWindow()
{
	Release();
}

bool SEASON3B::CNewUIMoveCommandWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_MOVEMAP, this );

	SetPos(x, y);
	
	LoadImages();
	
	Show( false );
	
	return true;
}

void SEASON3B::CNewUIMoveCommandWindow::Release()
{
	UnloadImages();
	
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

void SEASON3B::CNewUIMoveCommandWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;

	m_listMoveInfoData = CMoveCommandData::GetInstance()->GetMoveCommandDatalist();
	m_iRealFontHeight = FontHeight * 640 / WindowWidth + 2;
	//m_iRealFontHeight = FontHeight * DisplayHeight / WindowWidth + 2; //Fix Test Wide

#if defined(__ANDROID__) || defined(MU_IOS)
	if (IsAndroidMoveMapGridMode())
	{
		const int itemCount = static_cast<int>(m_listMoveInfoData.size());
		const int gridWidth = (kAndroidMoveMapColumns * kAndroidMoveMapTileWidth) + ((kAndroidMoveMapColumns - 1) * kAndroidMoveMapTileGapX);
		const int gridHeight = (kAndroidMoveMapRows * kAndroidMoveMapTileHeight) + ((kAndroidMoveMapRows - 1) * kAndroidMoveMapTileGapY);

		m_MapNameUISize.x = (kAndroidMoveMapPanelPaddingX * 2) + gridWidth;
		m_MapNameUISize.y = kAndroidMoveMapPanelPaddingY + kAndroidMoveMapHeaderHeight + gridHeight + kAndroidMoveMapFooterHeight + kAndroidMoveMapPanelPaddingY;

		m_StartUISubjectName.x = m_Pos.x + (m_MapNameUISize.x / 2);
		m_StartUISubjectName.y = m_Pos.y + 7;

		m_StartMapNamePos.x = m_Pos.x + kAndroidMoveMapPanelPaddingX;
		m_StartMapNamePos.y = m_Pos.y + kAndroidMoveMapHeaderHeight;

		m_StrifePos = m_StartMapNamePos;
		m_MapNamePos = m_StartMapNamePos;
		m_ReqLevelPos = m_StartMapNamePos;
		m_ReqZenPos = m_StartMapNamePos;

		m_ScrollBarPos = m_Pos;
		m_ScrollBtnStartPos = m_Pos;
		m_ScrollBtnPos = m_Pos;
		m_iScrollBarHeightPixel = 0;
		m_iScrollBarMiddleNum = 0;
		m_iScrollBarMiddleRemainderPixel = 0;
		m_iNumPage = GetAndroidMoveMapPageCount(itemCount);
		if (m_iCurPage < 1 || m_iCurPage > m_iNumPage)
		{
			m_iCurPage = 1;
		}
		m_iTotalMoveScrBtnperStep = 0;
		m_iRemainMoveScrBtnperStep = 0;
		m_iTotalMoveScrBtnPixel = 0;
		m_iRemainMoveScrBtnPixel = 0;
		m_iMinMoveScrBtnPixelperStep = 0;
		m_iMaxMoveScrBtnPixelperStep = 0;
		m_iTotalNumMaxMoveScrBtnperStep = 0;
		m_iTotalNumMinMoveScrBtnperStep = 0;
		m_icurMoveScrBtnPixelperStep = 0;
		m_iAcumMoveMouseScrollPixel = 0;
		m_bScrollBtnActive = false;
		GetAndroidMoveMapPageRange(itemCount, m_iCurPage, &m_iRenderStartTextIndex, &m_iRenderEndTextIndex);
		return;
	}
#endif

	m_StrifePos.x = m_Pos.x + 20;
	switch( WindowWidth )
	{
	case 800:
		m_MapNameUISize.x = 200; m_MapNamePos.x = m_Pos.x + 69; m_ReqLevelPos.x = m_Pos.x + 129; m_ReqZenPos.x = m_Pos.x + 174;
		break;
	case 1024:
		m_MapNameUISize.x = 180; m_MapNamePos.x = m_Pos.x + 64; m_ReqLevelPos.x = m_Pos.x + 119; m_ReqZenPos.x = m_Pos.x + 159;
		break;
	case 1280:
	default:
		m_MapNameUISize.x = 160; m_MapNamePos.x = m_Pos.x + 59; m_ReqLevelPos.x = m_Pos.x + 104; m_ReqZenPos.x = m_Pos.x + 139;
		break;
	}

	m_MapNameUISize.x += 10;

	m_MapNameUISize.y = 60 + (m_iRealFontHeight * MOVECOMMAND_MAX_RENDER_TEXTLINE);
	
	m_StartUISubjectName.x = m_Pos.x + m_MapNameUISize.x / 2;
	m_StartUISubjectName.y = m_Pos.y + 4;

	m_StartMapNamePos.x = m_Pos.x + 2;
	m_StartMapNamePos.y = m_Pos.y + 38;
	
	m_StrifePos.y = m_StartMapNamePos.y;

	m_MapNamePos.y = m_StartMapNamePos.y;
	m_ReqLevelPos.y = m_StartMapNamePos.y;
	m_ReqZenPos.y = m_StartMapNamePos.y;
	m_ScrollBarPos.x = m_Pos.x + m_MapNameUISize.x - 14;
	m_ScrollBarPos.y = m_Pos.y + m_StartMapNamePos.y - MOVECOMMAND_SCROLLBAR_TOP_HEIGHT;
	
	m_ScrollBtnStartPos.x = m_ScrollBarPos.x - (MOVECOMMAND_SCROLLBTN_WIDTH/2 - MOVECOMMAND_SCROLLBAR_TOP_WIDTH/2);
	m_ScrollBtnStartPos.y = m_ScrollBarPos.y;
	m_ScrollBtnPos.x = m_ScrollBtnStartPos.x;
	m_ScrollBtnPos.y = m_ScrollBtnStartPos.y;

	m_iScrollBarHeightPixel = MOVECOMMAND_MAX_RENDER_TEXTLINE * m_iRealFontHeight;
	
	m_iScrollBarMiddleNum = (m_iScrollBarHeightPixel-(MOVECOMMAND_SCROLLBAR_TOP_HEIGHT*2))/MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT;
	m_iScrollBarMiddleRemainderPixel = (m_iScrollBarHeightPixel-(MOVECOMMAND_SCROLLBAR_TOP_HEIGHT*2))%MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT;

	m_iNumPage = 1+(m_listMoveInfoData.size()/MOVECOMMAND_MAX_RENDER_TEXTLINE);
	
	m_iCurPage = 1;
	
	m_iTotalMoveScrBtnperStep = static_cast<int>(m_listMoveInfoData.size()) - MOVECOMMAND_MAX_RENDER_TEXTLINE;
	m_iRemainMoveScrBtnperStep		= m_iTotalMoveScrBtnperStep;
	m_iTotalMoveScrBtnPixel			= m_iScrollBarHeightPixel-MOVECOMMAND_SCROLLBTN_HEIGHT;	
	m_iRemainMoveScrBtnPixel		= m_iTotalMoveScrBtnPixel;
	if (m_iTotalMoveScrBtnperStep > 0)
	{
		m_iMinMoveScrBtnPixelperStep	= m_iTotalMoveScrBtnPixel/m_iTotalMoveScrBtnperStep;
		m_iMaxMoveScrBtnPixelperStep	= m_iMinMoveScrBtnPixelperStep+1;
		m_iTotalNumMaxMoveScrBtnperStep = m_iTotalMoveScrBtnPixel-(m_iTotalMoveScrBtnperStep*m_iMinMoveScrBtnPixelperStep);
		m_iTotalNumMinMoveScrBtnperStep = m_iTotalMoveScrBtnperStep - m_iTotalNumMaxMoveScrBtnperStep;
		m_icurMoveScrBtnPixelperStep = m_iMaxMoveScrBtnPixelperStep;
	}
	else
	{
		m_iRemainMoveScrBtnperStep = 0;
		m_iTotalMoveScrBtnPixel = 0;
		m_iRemainMoveScrBtnPixel = 0;
		m_iMinMoveScrBtnPixelperStep = 0;
		m_iMaxMoveScrBtnPixelperStep = 0;
		m_iTotalNumMaxMoveScrBtnperStep = 0;
		m_iTotalNumMinMoveScrBtnperStep = 0;
		m_icurMoveScrBtnPixelperStep = 0;
	}
	m_iAcumMoveMouseScrollPixel = 0;

	if( m_iNumPage > 1 && m_iTotalMoveScrBtnperStep > 0 )
	{
		m_bScrollBtnActive = true;
	}

	m_iRenderStartTextIndex = 0;
	m_iRenderEndTextIndex = m_iRenderStartTextIndex+MOVECOMMAND_MAX_RENDER_TEXTLINE;

	if( m_iRenderEndTextIndex > (int)m_listMoveInfoData.size() )
	{
		m_iRenderEndTextIndex -= (m_iRenderEndTextIndex-m_listMoveInfoData.size());
	}
}

bool SEASON3B::CNewUIMoveCommandWindow::IsLuckySealBuff()
{
	if( g_isCharacterBuff((&Hero->Object), eBuff_Seal1)  
		|| g_isCharacterBuff((&Hero->Object), eBuff_Seal2)   
		|| g_isCharacterBuff((&Hero->Object), eBuff_Seal3)
		|| g_isCharacterBuff((&Hero->Object), eBuff_Seal4) 
		|| g_isCharacterBuff((&Hero->Object), eBuff_Seal_HpRecovery)
		|| g_isCharacterBuff((&Hero->Object), eBuff_Seal_MpRecovery)
		|| g_isCharacterBuff((&Hero->Object), eBuff_AscensionSealMaster)
		|| g_isCharacterBuff((&Hero->Object), eBuff_WealthSealMaster)
		|| g_isCharacterBuff((&Hero->Object), eBuff_NewWealthSeal)
		|| g_isCharacterBuff((&Hero->Object), eBuff_PartyExpBonus)
		)
	{
		return true;
	}
	return false;
}

bool SEASON3B::CNewUIMoveCommandWindow::IsMapMove( const std::string& src )
{
	if( Hero->Object.Kind == KIND_PLAYER 
		&& Hero->Object.Type == MODEL_PLAYER 
		&& Hero->Object.SubType == MODEL_GM_CHARACTER )
	{
		return true;
	}

	if( g_isCharacterBuff((&Hero->Object), eBuff_GMEffect) )
	{
		return true;
	}

	if( IsLuckySealBuff() == false ) {
		char lpszStr1[1024]; char* lpszStr2 = NULL;
		if(src.find( GlobalText[260] ) != std::string::npos) {
			std::string temp = GlobalText[260];
			temp += ' ';
			wsprintf( lpszStr1, src.c_str() );	
			lpszStr2 = strtok( lpszStr1, temp.c_str() );
			if( lpszStr2 == NULL ) return false;

			SettingCanMoveMap();
			std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
			for( int i=0; i<m_iRenderEndTextIndex ; i++, li++ ) {
				if(!strcmp( lpszStr2, (*li)->_ReqInfo.szMainMapName )) {
					if((*li)->_bCanMove == true ){
						return IsLuckySeal( (*li)->_ReqInfo.szSubMapName );
					}
				}
			}
			return false;
		}
		else if(src.find( "/move" ) != std::string::npos) {
			std::string temp = "/move";
			temp += ' ';
			wsprintf( lpszStr1, src.c_str() );	
			lpszStr2 = strtok( lpszStr1, temp.c_str() );
			if( lpszStr2 == NULL ) return false;
			
			SettingCanMoveMap();
			std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
			for( int i=0; i<m_iRenderEndTextIndex ; i++, li++ ) {
				if(!stricmp( lpszStr2, (*li)->_ReqInfo.szMainMapName )) {
					if((*li)->_bCanMove == true ){
						return IsLuckySeal( (*li)->_ReqInfo.szSubMapName );
					}
				}
			}
			return false;
		}
		else {
			return IsLuckySeal( src );
		}
	}
	return true;
}

void SEASON3B::CNewUIMoveCommandWindow::SetMoveCommandKey(DWORD dwKey)
{
	m_dwMoveCommandKey = dwKey;
}

DWORD SEASON3B::CNewUIMoveCommandWindow::GetMoveCommandKey()
{
	m_dwMoveCommandKey = g_KeyGenerater.GenerateKeyValue(m_dwMoveCommandKey);
	
	return m_dwMoveCommandKey;
}

void SEASON3B::CNewUIMoveCommandWindow::SetStrifeMap()
{
	std::list<CMoveCommandData::MOVEINFODATA*>::iterator li;

	if (!g_ServerListManager->IsNonPvP())
	{
		int anStrifeIndex[1] = { 42 };
		int i;
		for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); advance(li, 1))
		{
			(*li)->_bStrife = false;
			for (i = 0; i < 1; ++i)
			{
				if ((*li)->_ReqInfo.index == anStrifeIndex[i])
				{
					(*li)->_bStrife = true;
					break;
				}
			}
		}
	}
	else
	{
		for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); advance(li, 1))
			(*li)->_bStrife = false;
	}
}

void SEASON3B::CNewUIMoveCommandWindow::SettingCanMoveMap()
{
	int a = gMapManager.WorldActive;

	DWORD iZen;
	int iLevel, iReqLevel, iReqZen;

	std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
	for( int i=0 ; i<m_iRenderEndTextIndex ; i++, li++ )
	{
		if( li == m_listMoveInfoData.end() )
		{
			break;
		}
		if( i<m_iRenderStartTextIndex )
		{
			continue;
		}

		(*li)->_bCanMove = false;
		(*li)->_bSelected = false;

	//	if( i < m_iRenderEndTextIndex-1 )	continue;

		iLevel = CharacterAttribute->Level;
		iZen = CharacterMachine->Gold;
		iReqLevel = (*li)->_ReqInfo.iReqLevel;
		iReqZen = (*li)->_ReqInfo.iReqZen;

		if( ( gCharacterManager.GetBaseClass(CharacterAttribute->Class)==CLASS_DARK || gCharacterManager.GetBaseClass(CharacterAttribute->Class)==CLASS_DARK_LORD 
#ifdef PBG_ADD_NEWCHAR_MONK
				|| gCharacterManager.GetBaseClass(CharacterAttribute->Class)==CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
			)
			&& (iReqLevel != 400) )
		{
			iReqLevel = int(float(iReqLevel)*2.f/3.f);
		}
		
		if( iLevel >= iReqLevel && (int)iZen >= iReqZen && (int)Hero->PK<PVP_MURDERER1)
		{
			ITEM* pEquipedRightRing = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
			ITEM* pEquipedLeftRing = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
			ITEM* pEquipedHelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
			ITEM* pEquipedWing = &CharacterMachine->Equipment[EQUIPMENT_WING];
			
			if(strcmp((*li)->_ReqInfo.szMainMapName,GlobalText[55])==0)
			{
				if( 
					(
					pEquipedHelper->Type == ITEM_HELPER+37		
					|| gCustomPet2.GetInfoPetType(pEquipedHelper->Type) == 5 //Custom pet
					|| gCustomPet2.GetInfoPetType(pEquipedHelper->Type) == 6 //Custom pet
					|| pEquipedHelper->Type == ITEM_HELPER+3
					|| pEquipedHelper->Type == ITEM_HELPER+4
					|| pEquipedWing->Type == ITEM_HELPER+30
					|| (pEquipedWing->Type >= ITEM_WING+36 && pEquipedWing->Type <= ITEM_WING+43)			
					|| (pEquipedWing->Type >= ITEM_WING && pEquipedWing->Type <= ITEM_WING+6) 
					|| ( ITEM_WING+130 <= pEquipedWing->Type && pEquipedWing->Type <= ITEM_WING+134 )
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
					|| (pEquipedWing->Type >= ITEM_WING+49 && pEquipedWing->Type <= ITEM_WING+50)
					|| (pEquipedWing->Type == ITEM_WING+135)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
					)
					&& !( pEquipedHelper->Type == ITEM_HELPER+2 )
					&& ( g_ChangeRingMgr->CheckBanMoveIcarusMap(pEquipedRightRing->Type, pEquipedLeftRing->Type) == false )
					)
				{
					(*li)->_bCanMove = true;
				}
				//=== Check Custom Wing 
				else if (gCustomWing.CheckCustomWingByItem(pEquipedWing->Type))
				{
					(*li)->_bCanMove = true;
				}
				else
				{
					(*li)->_bCanMove = false;
				}
			}
			else if(strncmp((*li)->_ReqInfo.szMainMapName, GlobalText[37], 8)==0)
			{
				if(pEquipedHelper->Type == ITEM_HELPER+2 || pEquipedHelper->Type == ITEM_HELPER+3)
				{
					(*li)->_bCanMove = false;
				}
				else
				{
					(*li)->_bCanMove = true;
				}
			}
			//else if( ( g_ServerListManager->IsNonPvP() == true ) && (strcmp((*li)->_ReqInfo.szMainMapName, GlobalText[2686]) == 0) )
			//{
			//	(*li)->_bCanMove = false;
			//}
			else
			{
				(*li)->_bCanMove = true;
			}	
		}

		//if ((*li)->_bCanMove && (*li)->_bStrife && 0 == Hero->m_byGensInfluence)
		//	(*li)->_bCanMove = false;

	}
}

bool SEASON3B::CNewUIMoveCommandWindow::BtnProcess()
{
	int iX, iY;

#if defined(__ANDROID__) || defined(MU_IOS)
	if (IsAndroidMoveMapGridMode())
	{
		if (CheckMouseIn(m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y) && IsPress(VK_LBUTTON))
		{
			SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
		}

		SettingCanMoveMap();

		if (CheckMouseIn(m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y))
		{
			BlockMouseWheel = GetTickCount() + 500;

			int closeX = 0;
			int closeY = 0;
			int closeW = 0;
			int closeH = 0;
			GetAndroidMoveMapCloseButtonRect(m_Pos.x, m_Pos.y, m_MapNameUISize.x, &closeX, &closeY, &closeW, &closeH);
			if (SEASON3B::IsRelease(VK_LBUTTON) && CheckMouseIn(closeX, closeY, closeW, closeH))
			{
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_MOVEMAP);
				return true;
			}

			int prevX = 0;
			int prevY = 0;
			int prevW = 0;
			int prevH = 0;
			GetAndroidMoveMapPageButtonRect(m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y, false, &prevX, &prevY, &prevW, &prevH);
			if (m_iCurPage > 1 && SEASON3B::IsRelease(VK_LBUTTON) && CheckMouseIn(prevX, prevY, prevW, prevH))
			{
				--m_iCurPage;
				UpdateScrolling();
				SettingCanMoveMap();
				return true;
			}

			int nextX = 0;
			int nextY = 0;
			int nextW = 0;
			int nextH = 0;
			GetAndroidMoveMapPageButtonRect(m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y, true, &nextX, &nextY, &nextW, &nextH);
			if (m_iCurPage < m_iNumPage && SEASON3B::IsRelease(VK_LBUTTON) && CheckMouseIn(nextX, nextY, nextW, nextH))
			{
				++m_iCurPage;
				UpdateScrolling();
				SettingCanMoveMap();
				return true;
			}

			int order = 0;
			std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
			for (; li != m_listMoveInfoData.end(); ++li, ++order)
			{
				(*li)->_bSelected = false;
				if (order < m_iRenderStartTextIndex || order >= m_iRenderEndTextIndex)
				{
					continue;
				}

				int tileX = 0;
				int tileY = 0;
				int tileW = 0;
				int tileH = 0;
				GetAndroidMoveMapTileRect(
					m_Pos.x,
					m_Pos.y,
					order - m_iRenderStartTextIndex,
					&tileX,
					&tileY,
					&tileW,
					&tileH);

				if (CheckMouseIn(tileX, tileY, tileW, tileH))
				{
					if ((*li)->_bCanMove == true)
					{
						(*li)->_bSelected = true;

						if (SEASON3B::IsRelease(VK_LBUTTON))
						{
							if (IsTheMapInDifferentServer(gMapManager.WorldActive, (*li)->_ReqInfo.index))
							{
								SaveOptions();
							}

							SendRequestMoveMap(g_pMoveCommandWindow->GetMoveCommandKey(), (*li)->_ReqInfo.index);
							g_pNewUISystem->Hide(SEASON3B::INTERFACE_MOVEMAP);
							return true;
						}
					}
				}
			}
		}

		return false;
	}
#endif

	if( CheckMouseIn( m_ScrollBtnPos.x, m_ScrollBtnPos.y, MOVECOMMAND_SCROLLBTN_WIDTH, MOVECOMMAND_SCROLLBTN_HEIGHT ))
	{	
		if(IsPress(VK_LBUTTON))
		{
			m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_CLICKED;
			m_iMousePosY = MouseY;
			m_iAcumMoveMouseScrollPixel = 0;
		}	
	}

	if(IsRelease(VK_LBUTTON))
	{
 		m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;
		m_iAcumMoveMouseScrollPixel = 0;
	}
	
//	if( CheckMouseIn( 0, m_ScrollBarPos.y, 640, m_iTotalMoveScrBtnPixel) )
//	{
		if( m_iScrollBtnMouseEvent == MOVECOMMAND_MOUSEBTN_CLICKED && m_icurMoveScrBtnPixelperStep > 0 )
		{
			//MoveScrollBtn(MouseY-m_iMousePosY);
			int iMoveValue = MouseY-m_iMousePosY;
				
			if( iMoveValue < 0 )
			{
				if( MouseY <= m_ScrollBtnPos.y+(MOVECOMMAND_SCROLLBTN_HEIGHT/2))
				{
					if( -(iMoveValue) > (m_iTotalMoveScrBtnPixel-m_iRemainMoveScrBtnPixel) )
					{
						iMoveValue = -(m_iTotalMoveScrBtnPixel-m_iRemainMoveScrBtnPixel);
					}
					ScrollUp(-iMoveValue);
				}
			}
			else if( iMoveValue > 0 )
			{
				if( MouseY >= m_ScrollBtnPos.y+(MOVECOMMAND_SCROLLBTN_HEIGHT/2))
				{
					if( iMoveValue > m_iRemainMoveScrBtnPixel )
					{
						iMoveValue = m_iRemainMoveScrBtnPixel;
					}
					ScrollDown(iMoveValue);
				}
			}
			m_iMousePosY = MouseY;	
			//UpdateScrolling();
		}
//	}
// 	else
// 	{
// 		m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;
// 		m_iAcumMoveMouseScrollPixel = 0;
// 	}
	if( CheckMouseIn( m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y ) && IsPress(VK_LBUTTON))
	{
		SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	}

	SettingCanMoveMap();



	if( CheckMouseIn( m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y ) )
	{
		BlockMouseWheel = GetTickCount() + 500;
		if( m_icurMoveScrBtnPixelperStep > 0 )
		{
			if( MouseWheel > 0 )
			{
				ScrollUp(m_icurMoveScrBtnPixelperStep);
				
			}
			else if( MouseWheel < 0 )
			{
				ScrollDown(m_icurMoveScrBtnPixelperStep);
				
			}
		}
		MouseWheel = 0;
		

		std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
		int iCurRenderTextIndex = 0;
		for( int i=0; i<m_iRenderEndTextIndex ; i++, li++ )
		{
			if( li == m_listMoveInfoData.end() )
			{
				break;
			}

			if( i<m_iRenderStartTextIndex )
			{
				continue;
			}
			iX = m_StartMapNamePos.x;
			iY = m_StartMapNamePos.y + ( m_iRealFontHeight * iCurRenderTextIndex );
			if( CheckMouseIn( iX, iY, m_MapNameUISize.x-22, m_iRealFontHeight ) )
			{
				if( (*li)->_bCanMove == true )
				{
					(*li)->_bSelected = true;
					
					if( SEASON3B::IsRelease(VK_LBUTTON) )
					{
						if (IsTheMapInDifferentServer(gMapManager.WorldActive, (*li)->_ReqInfo.index))
						{
							SaveOptions();
						}
						SendRequestMoveMap(g_pMoveCommandWindow->GetMoveCommandKey(), (*li)->_ReqInfo.index);

						g_pNewUISystem->Hide( SEASON3B::INTERFACE_MOVEMAP );
						return true;	
					}
				}	
			}

			iCurRenderTextIndex++;
			
			if( SEASON3B::IsRelease(VK_LBUTTON)  && CheckMouseIn( 3, m_MapNameUISize.y-m_iRealFontHeight-6, m_MapNameUISize.x-5, m_iRealFontHeight ))
			{
				g_pNewUISystem->Hide( SEASON3B::INTERFACE_MOVEMAP );
				return true;
			}
		}
	}
	return false;
}

bool SEASON3B::CNewUIMoveCommandWindow::UpdateMouseEvent()
{
	if( true == BtnProcess() )
		return false;

#if defined(__ANDROID__) || defined(MU_IOS)
	if (IsAndroidMoveMapGridMode())
	{
		if (CheckMouseIn(m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y))
			return false;

		return true;
	}
#endif

	if( m_iScrollBtnMouseEvent == MOVECOMMAND_MOUSEBTN_CLICKED )
		return false;
	
	if( CheckMouseIn( m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y ) )
		return false;

	return true;
}

bool SEASON3B::CNewUIMoveCommandWindow::UpdateKeyEvent()
{
	if( IsVisible() )
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			Show( false );
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}
	return true;
}

bool SEASON3B::CNewUIMoveCommandWindow::Update()
{
	if( !IsVisible() )
	{
		return true;
	}

	UpdateScrolling();

 	return true;
}

void SEASON3B::CNewUIMoveCommandWindow::ScrollUp(int iMoveValue)
{
	if( m_iRemainMoveScrBtnperStep < m_iTotalMoveScrBtnperStep )
	{
		int iMovePixel = 0;
		m_iAcumMoveMouseScrollPixel -= iMoveValue;
		if( (-m_iAcumMoveMouseScrollPixel) < m_icurMoveScrBtnPixelperStep )
		{
			return;
		}
		else
		{
			//m_iAcumMoveMouseScrollPixel = 0;
			RecursiveCalcScroll(m_iAcumMoveMouseScrollPixel, &iMovePixel, false);

			g_ConsoleDebug->Write(MCD_NORMAL, "m_ScrollBtnPos.y : (%d)", m_ScrollBtnPos.y);
			
			m_ScrollBtnPos.y += iMovePixel;
			m_iAcumMoveMouseScrollPixel -= iMovePixel;

			g_ConsoleDebug->Write(MCD_NORMAL, "m_ScrollBtnPos.y : (%d)", m_ScrollBtnPos.y);	
			g_ConsoleDebug->Write(MCD_NORMAL, "iMoveValue : (%d)", -iMoveValue);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iRemainMoveScrBtnPixel : (%d)", m_iRemainMoveScrBtnPixel);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_icurMoveScrBtnPixelperStep : (%d)", m_icurMoveScrBtnPixelperStep);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iRemainMoveScrBtnperStep : (%d)", m_iRemainMoveScrBtnperStep);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iAcumMoveMouseScrollPixel : (%d)", m_iAcumMoveMouseScrollPixel);		
		}
	}
}

void SEASON3B::CNewUIMoveCommandWindow::ScrollDown(int iMoveValue)
{
	if( m_iRemainMoveScrBtnperStep > 0 )
	{
		int iMovePixel = 0;
		m_iAcumMoveMouseScrollPixel += iMoveValue;
		if( m_iAcumMoveMouseScrollPixel < m_icurMoveScrBtnPixelperStep )
		{
			return;
		}
		else
		{	
			RecursiveCalcScroll(m_iAcumMoveMouseScrollPixel, &iMovePixel, true);

			g_ConsoleDebug->Write(MCD_NORMAL, "m_ScrollBtnPos.y : (%d)", m_ScrollBtnPos.y);	

			m_iAcumMoveMouseScrollPixel -= iMovePixel;
			m_ScrollBtnPos.y += iMovePixel;

			g_ConsoleDebug->Write(MCD_NORMAL, "m_ScrollBtnPos.y : (%d)", m_ScrollBtnPos.y);	
			g_ConsoleDebug->Write(MCD_NORMAL, "iMoveValue : (%d)", iMoveValue);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iRemainMoveScrBtnPixel : (%d)", m_iRemainMoveScrBtnPixel);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_icurMoveScrBtnPixelperStep : (%d)", m_icurMoveScrBtnPixelperStep);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iRemainMoveScrBtnperStep : (%d)", m_iRemainMoveScrBtnperStep);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iAcumMoveMouseScrollPixel : (%d)", m_iAcumMoveMouseScrollPixel);
			
		}
	}
}

void SEASON3B::CNewUIMoveCommandWindow::RecursiveCalcScroll(IN int piScrollValue, OUT int* piMovePixel, bool bSign /* = true */)
{
	if( bSign == true )
	{ // DownScroll
		if( m_iRemainMoveScrBtnperStep > 0)
		{	

			m_iRemainMoveScrBtnperStep--;
			m_iRemainMoveScrBtnPixel -= m_icurMoveScrBtnPixelperStep;
			piScrollValue -= m_icurMoveScrBtnPixelperStep;
			(*piMovePixel) += m_icurMoveScrBtnPixelperStep;

			if( m_iRemainMoveScrBtnperStep > m_iTotalNumMinMoveScrBtnperStep )
			{
				m_icurMoveScrBtnPixelperStep = m_iMaxMoveScrBtnPixelperStep;
			}
			else 
			{
				m_icurMoveScrBtnPixelperStep = m_iMinMoveScrBtnPixelperStep;
			}

			if( piScrollValue >= m_icurMoveScrBtnPixelperStep )
			{
				RecursiveCalcScroll(piScrollValue, piMovePixel, bSign);
			}
		}
		else
		{
			(*piMovePixel) = piScrollValue;
		}
	}
	else
	{ // UpScroll
		if( m_iRemainMoveScrBtnperStep < m_iTotalMoveScrBtnperStep )
		{		
			m_iRemainMoveScrBtnperStep++;
			m_iRemainMoveScrBtnPixel += m_icurMoveScrBtnPixelperStep;
			piScrollValue += m_icurMoveScrBtnPixelperStep;
			(*piMovePixel) -= m_icurMoveScrBtnPixelperStep;

			if( m_iRemainMoveScrBtnperStep >= m_iTotalNumMinMoveScrBtnperStep)
			{
				m_icurMoveScrBtnPixelperStep = m_iMaxMoveScrBtnPixelperStep;
			}
			else 
			{
				m_icurMoveScrBtnPixelperStep = m_iMinMoveScrBtnPixelperStep;
			}

			if( (-piScrollValue) >= m_icurMoveScrBtnPixelperStep )
			{
				RecursiveCalcScroll(piScrollValue, piMovePixel, bSign);
			}
		}
		else
		{
			(*piMovePixel) = piScrollValue;
		}
	}

	return;
}


void SEASON3B::CNewUIMoveCommandWindow::UpdateScrolling()
{
#if defined(__ANDROID__) || defined(MU_IOS)
	if (IsAndroidMoveMapGridMode())
	{
		const int itemCount = static_cast<int>(m_listMoveInfoData.size());
		m_iNumPage = GetAndroidMoveMapPageCount(itemCount);
		if (m_iCurPage < 1)
		{
			m_iCurPage = 1;
		}
		else if (m_iCurPage > m_iNumPage)
		{
			m_iCurPage = m_iNumPage;
		}

		GetAndroidMoveMapPageRange(itemCount, m_iCurPage, &m_iRenderStartTextIndex, &m_iRenderEndTextIndex);
		return;
	}
#endif

	m_iRenderStartTextIndex = m_iTotalMoveScrBtnperStep - m_iRemainMoveScrBtnperStep;

	m_iRenderEndTextIndex = m_iRenderStartTextIndex + MOVECOMMAND_MAX_RENDER_TEXTLINE;
	
	if( m_iRenderEndTextIndex > (int)m_listMoveInfoData.size() )
	{
		m_iRenderEndTextIndex -= (m_iRenderEndTextIndex-m_listMoveInfoData.size());
	}
}

void SEASON3B::CNewUIMoveCommandWindow::RenderFrame()
{
#if defined(__ANDROID__) || defined(MU_IOS)
	if (IsAndroidMoveMapGridMode())
	{
		EnableAlphaBlend3();
		glColor4f(0.01f, 0.01f, 0.01f, 1.00f);
		RenderColor((float)m_Pos.x, (float)m_Pos.y, (float)m_MapNameUISize.x, (float)m_MapNameUISize.y);

		glColor4f(0.06f, 0.06f, 0.06f, 1.00f);
		RenderColor((float)m_Pos.x + 2.0f, (float)m_Pos.y + 2.0f, (float)m_MapNameUISize.x - 4.0f, (float)m_MapNameUISize.y - 4.0f);

		glColor4f(0.08f, 0.08f, 0.08f, 1.00f);
		RenderColor(
			(float)(m_Pos.x + 2),
			(float)(m_Pos.y + 2),
			(float)(m_MapNameUISize.x - 4),
			(float)(kAndroidMoveMapHeaderHeight - 4));

		glColor4f(0.05f, 0.05f, 0.05f, 1.00f);
		RenderColor(
			(float)(m_Pos.x + 2),
			(float)(m_Pos.y + m_MapNameUISize.y - kAndroidMoveMapFooterHeight - 2),
			(float)(m_MapNameUISize.x - 4),
			(float)(kAndroidMoveMapFooterHeight));

		glColor4f(0.65f, 0.50f, 0.16f, 1.00f);
		RenderColor((float)m_Pos.x + 2.0f, (float)(m_Pos.y + kAndroidMoveMapHeaderHeight - 1), (float)m_MapNameUISize.x - 4.0f, 1.0f);
		RenderColor((float)m_Pos.x + 2.0f, (float)(m_Pos.y + m_MapNameUISize.y - kAndroidMoveMapFooterHeight - 2), (float)m_MapNameUISize.x - 4.0f, 1.0f);

		int closeX = 0;
		int closeY = 0;
		int closeW = 0;
		int closeH = 0;
		GetAndroidMoveMapCloseButtonRect(m_Pos.x, m_Pos.y, m_MapNameUISize.x, &closeX, &closeY, &closeW, &closeH);
		glColor4f(0.18f, 0.04f, 0.04f, 1.00f);
		RenderColor((float)closeX, (float)closeY, (float)closeW, (float)closeH);
		glColor4f(0.75f, 0.18f, 0.18f, 1.00f);
		RenderColor((float)closeX, (float)closeY, (float)closeW, 1.0f);
		RenderColor((float)closeX, (float)(closeY + closeH - 1), (float)closeW, 1.0f);
		RenderColor((float)closeX, (float)closeY, 1.0f, (float)closeH);
		RenderColor((float)(closeX + closeW - 1), (float)closeY, 1.0f, (float)closeH);

		int prevX = 0;
		int prevY = 0;
		int prevW = 0;
		int prevH = 0;
		GetAndroidMoveMapPageButtonRect(m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y, false, &prevX, &prevY, &prevW, &prevH);
		glColor4f(m_iCurPage > 1 ? 0.10f : 0.03f, m_iCurPage > 1 ? 0.10f : 0.03f, m_iCurPage > 1 ? 0.10f : 0.03f, 1.00f);
		RenderColor((float)prevX, (float)prevY, (float)prevW, (float)prevH);
		glColor4f(m_iCurPage > 1 ? 0.72f : 0.22f, m_iCurPage > 1 ? 0.72f : 0.22f, m_iCurPage > 1 ? 0.72f : 0.22f, 1.00f);
		RenderColor((float)prevX, (float)prevY, (float)prevW, 1.0f);
		RenderColor((float)prevX, (float)(prevY + prevH - 1), (float)prevW, 1.0f);
		RenderColor((float)prevX, (float)prevY, 1.0f, (float)prevH);
		RenderColor((float)(prevX + prevW - 1), (float)prevY, 1.0f, (float)prevH);
		int nextX = 0;
		int nextY = 0;
		int nextW = 0;
		int nextH = 0;
		GetAndroidMoveMapPageButtonRect(m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y, true, &nextX, &nextY, &nextW, &nextH);
		glColor4f(m_iCurPage < m_iNumPage ? 0.10f : 0.03f, m_iCurPage < m_iNumPage ? 0.10f : 0.03f, m_iCurPage < m_iNumPage ? 0.10f : 0.03f, 1.00f);
		RenderColor((float)nextX, (float)nextY, (float)nextW, (float)nextH);
		glColor4f(m_iCurPage < m_iNumPage ? 0.72f : 0.22f, m_iCurPage < m_iNumPage ? 0.72f : 0.22f, m_iCurPage < m_iNumPage ? 0.72f : 0.22f, 1.00f);
		RenderColor((float)nextX, (float)nextY, (float)nextW, 1.0f);
		RenderColor((float)nextX, (float)(nextY + nextH - 1), (float)nextW, 1.0f);
		RenderColor((float)nextX, (float)nextY, 1.0f, (float)nextH);
		RenderColor((float)(nextX + nextW - 1), (float)nextY, 1.0f, (float)nextH);

		EndRenderColor();
		EnableAlphaTest();
		glColor4f(1.f, 1.f, 1.f, 1.f);

		g_pRenderText->SetBgColor(0, 0, 0, 0);
		g_pRenderText->SetFont(g_hFontBold ? g_hFontBold : (g_hFixFont ? g_hFixFont : g_hFont));
		g_pRenderText->SetTextColor(255, 255, 255, 255);
		g_pRenderText->RenderText(m_StartUISubjectName.x, m_StartUISubjectName.y, GlobalText[933], 0, 0, RT3_WRITE_CENTER);
		g_pRenderText->RenderText(closeX + (closeW / 2), closeY + 4, "X", 0, 0, RT3_WRITE_CENTER);
		g_pRenderText->SetTextColor(m_iCurPage > 1 ? 255 : 110, m_iCurPage > 1 ? 255 : 110, m_iCurPage > 1 ? 255 : 110, 255);
		g_pRenderText->RenderText(prevX + (prevW / 2), prevY + 4, "<", 0, 0, RT3_WRITE_CENTER);
		g_pRenderText->SetTextColor(m_iCurPage < m_iNumPage ? 255 : 110, m_iCurPage < m_iNumPage ? 255 : 110, m_iCurPage < m_iNumPage ? 255 : 110, 255);
		g_pRenderText->RenderText(nextX + (nextW / 2), nextY + 4, ">", 0, 0, RT3_WRITE_CENTER);

		char pageText[32];
		wsprintf(pageText, "%d / %d", m_iCurPage, m_iNumPage);
		g_pRenderText->SetFont(g_hFixFont ? g_hFixFont : g_hFont);
		g_pRenderText->SetTextColor(255, 255, 255, 255);
		g_pRenderText->RenderText(
			m_Pos.x + (m_MapNameUISize.x / 2),
			m_Pos.y + m_MapNameUISize.y - kAndroidMoveMapFooterHeight + 5,
			pageText,
			0,
			0,
			RT3_WRITE_CENTER);
		return;
	}
#endif

	glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
	
	RenderColor((float)m_Pos.x, (float)m_Pos.y, (float)m_MapNameUISize.x, (float)m_MapNameUISize.y);

	glColor4f ( 0.6f, 0.f, 0.f, 1.f );

	RenderColor( m_StartMapNamePos.x, m_MapNameUISize.y-m_iRealFontHeight-6, m_MapNameUISize.x-5, m_iRealFontHeight+3 );

	glColor4f ( 1.f, 1.f, 1.f, 1.f );
	EnableAlphaTest();

	RenderImage(IMAGE_MOVECOMMAND_SCROLL_TOP, m_ScrollBarPos.x, m_ScrollBarPos.y, MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_TOP_HEIGHT );		// TOP
	
	int icntText = 0;
	for( int i=0 ; i<m_iScrollBarMiddleNum ; i++ )
	{
		icntText = i;
		RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, m_ScrollBarPos.x, 
			m_ScrollBarPos.y+MOVECOMMAND_SCROLLBAR_TOP_HEIGHT+(i*MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
			MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT );	// MIDDLE
	}
	if( m_iScrollBarMiddleRemainderPixel > 0 )
	{
		RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, m_ScrollBarPos.x, 
			m_ScrollBarPos.y+MOVECOMMAND_SCROLLBAR_TOP_HEIGHT+(icntText*MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
			MOVECOMMAND_SCROLLBAR_TOP_WIDTH, m_iScrollBarMiddleRemainderPixel );	// MIDDLE ³ª¸ÓÁö
	}

	RenderImage(IMAGE_MOVECOMMAND_SCROLL_BOTTOM, m_ScrollBarPos.x, m_ScrollBarPos.y+m_iScrollBarHeightPixel-MOVECOMMAND_SCROLLBAR_TOP_HEIGHT,
					MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_TOP_HEIGHT );		// BOTTOM

	if( m_bScrollBtnActive == true )
	{
		if (m_iScrollBtnMouseEvent == MOVECOMMAND_MOUSEBTN_CLICKED) 
		{
			glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
		}
		RenderImage(IMAGE_MOVECOMMAND_SCROLLBAR_ON, m_ScrollBtnPos.x, m_ScrollBtnPos.y, 
			MOVECOMMAND_SCROLLBTN_WIDTH, MOVECOMMAND_SCROLLBTN_HEIGHT);
	}
	else
	{
		RenderImage(IMAGE_MOVECOMMAND_SCROLLBAR_OFF, m_ScrollBtnPos.x, m_ScrollBtnPos.y, 
			MOVECOMMAND_SCROLLBTN_WIDTH, MOVECOMMAND_SCROLLBTN_HEIGHT);
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);


	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(255, 204, 26, 255);
	g_pRenderText->RenderText(m_StartUISubjectName.x, m_StartUISubjectName.y, GlobalText[933], 0 ,0, RT3_WRITE_CENTER);
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(127, 178, 255, 255);
	//g_pRenderText->RenderText(m_StrifePos.x, m_StartUISubjectName.y+20, GlobalText[2988], 0 ,0, RT3_WRITE_CENTER);
	g_pRenderText->RenderText(m_MapNamePos.x, m_StartUISubjectName.y+20, GlobalText[934], 0 ,0, RT3_WRITE_CENTER);
	g_pRenderText->RenderText(m_ReqLevelPos.x, m_StartUISubjectName.y+20, GlobalText[935], 0 ,0, RT3_WRITE_CENTER);
	g_pRenderText->RenderText(m_ReqZenPos.x, m_StartUISubjectName.y+20, GlobalText[936], 0 ,0, RT3_WRITE_CENTER);
}

bool SEASON3B::CNewUIMoveCommandWindow::Render()
{
#if defined(__ANDROID__) || defined(MU_IOS)
	if (IsAndroidMoveMapGridMode())
	{
		EnableAlphaBlend3();
		glColor4f(1.f, 1.f, 1.f, 1.f);
		SettingCanMoveMap();
		RenderFrame();

		g_pRenderText->SetBgColor(0, 0, 0, 0);

		int order = 0;
		std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
		for (; li != m_listMoveInfoData.end(); ++li, ++order)
		{
			if (order < m_iRenderStartTextIndex || order >= m_iRenderEndTextIndex)
			{
				continue;
			}

			int tileX = 0;
			int tileY = 0;
			int tileW = 0;
			int tileH = 0;
			GetAndroidMoveMapTileRect(
				m_Pos.x,
				m_Pos.y,
				order - m_iRenderStartTextIndex,
				&tileX,
				&tileY,
				&tileW,
				&tileH);

			const bool canMove = ((*li)->_bCanMove == true);
			const bool selected = ((*li)->_bSelected == true);

			if (canMove)
			{
				glColor4f(selected ? 0.14f : 0.03f, selected ? 0.14f : 0.03f, selected ? 0.14f : 0.03f, 1.00f);
			}
			else
			{
				glColor4f(0.09f, 0.02f, 0.02f, 1.00f);
			}
			RenderColor((float)tileX, (float)tileY, (float)tileW, (float)tileH);

			glColor4f(selected ? 0.95f : (canMove ? 0.32f : 0.28f), selected ? 0.76f : (canMove ? 0.32f : 0.10f), selected ? 0.18f : (canMove ? 0.32f : 0.10f), 1.00f);
			RenderColor((float)tileX, (float)tileY, (float)tileW, 1.0f);
			RenderColor((float)tileX, (float)(tileY + tileH - 1), (float)tileW, 1.0f);
			RenderColor((float)tileX, (float)tileY, 1.0f, (float)tileH);
			RenderColor((float)(tileX + tileW - 1), (float)tileY, 1.0f, (float)tileH);

			EndRenderColor();
			EnableAlphaTest();
			glColor4f(1.f, 1.f, 1.f, 1.f);

			g_pRenderText->SetFont(g_hFontBig ? g_hFontBig : (g_hFontBold ? g_hFontBold : (g_hFixFont ? g_hFixFont : g_hFont)));
			if (canMove)
			{
				g_pRenderText->SetTextColor(255, 255, 255, 255);
			}
			else
			{
				g_pRenderText->SetTextColor(185, 185, 185, 255);
			}
			g_pRenderText->RenderText(tileX, tileY + 7, (*li)->_ReqInfo.szMainMapName, tileW, 0, RT3_SORT_CENTER);

			char szLevelText[32];
			wsprintf(szLevelText, "Lv %d", (*li)->_ReqInfo.iReqLevel);
			g_pRenderText->SetFont(g_hFixFont ? g_hFixFont : g_hFont);
			g_pRenderText->SetTextColor(canMove ? 235 : 140, canMove ? 215 : 110, canMove ? 120 : 110, 255);
			g_pRenderText->RenderText(tileX, tileY + tileH - 16, szLevelText, tileW, 0, RT3_SORT_CENTER);
		}

		DisableAlphaBlend();
		return true;
	}
#endif

	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetTextColor( 255, 255, 255, 255 );

	RenderFrame();	

	std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
	int iX, iY;

	int iLevel = CharacterAttribute->Level;
	DWORD iZen = CharacterMachine->Gold;
	int iReqLevel;
	char szText[24];

	int iCurRenderTextIndex = 0;
	for ( int i=0 ; i<m_iRenderEndTextIndex ; i++, li++ )
    {
		if( li == m_listMoveInfoData.end())
		{
			break;
		}

		if( i < m_iRenderStartTextIndex )
		{
			continue;
		}

		iX = m_StartMapNamePos.x;
		iY = m_StartMapNamePos.y + ( m_iRealFontHeight * iCurRenderTextIndex );

		iReqLevel = (*li)->_ReqInfo.iReqLevel;
		if ( (gCharacterManager.GetBaseClass(CharacterAttribute->Class)==CLASS_DARK || gCharacterManager.GetBaseClass(CharacterAttribute->Class)==CLASS_DARK_LORD
#ifdef PBG_ADD_NEWCHAR_MONK
				|| gCharacterManager.GetBaseClass(CharacterAttribute->Class)==CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
				) 
			&& (iReqLevel != 400) )
		{
			iReqLevel = int(float(iReqLevel)*2.f/3.f);
		}
		
		if( (*li)->_bCanMove == true )
		{
			g_pRenderText->SetTextColor(255, 255, 255, 255);

			//if ((*li)->_bStrife)
			//	g_pRenderText->RenderText(m_StrifePos.x, iY, GlobalText[2987], 0 ,0, RT3_WRITE_CENTER);
			g_pRenderText->RenderText(m_MapNamePos.x, iY, (*li)->_ReqInfo.szMainMapName, 0 ,0, RT3_WRITE_CENTER);
			itoa(iReqLevel, szText, 10);
			g_pRenderText->RenderText(m_ReqLevelPos.x, iY, szText, 0 ,0, RT3_WRITE_CENTER);
				itoa((*li)->_ReqInfo.iReqZen, szText, 10);
			g_pRenderText->RenderText(m_ReqZenPos.x, iY, szText, 0 ,0, RT3_WRITE_CENTER);
			
			if( (*li)->_bSelected == true )
			{
				glColor4f ( 0.8f, 0.8f, 0.1f, 0.6f );
				RenderColor( iX, iY, m_MapNameUISize.x-22, m_iRealFontHeight );
				glColor4f ( 1.f, 1.f, 1.f, 1.f );
				EnableAlphaTest();
			}
		}
		else
		{
			g_pRenderText->SetTextColor(164, 39, 17, 255);

			//if ((*li)->_bStrife)
			//	g_pRenderText->RenderText(m_StrifePos.x, iY, GlobalText[2987], 0 ,0, RT3_WRITE_CENTER);

			g_pRenderText->RenderText(m_MapNamePos.x, iY, (*li)->_ReqInfo.szMainMapName, 0 ,0, RT3_WRITE_CENTER);

			itoa(iReqLevel, szText, 10);
			if( iReqLevel > iLevel )
			{
				g_pRenderText->SetTextColor(255, 51, 26, 255);

			}
			else
			{
				g_pRenderText->SetTextColor(164, 39, 17, 255);
			}
			g_pRenderText->RenderText(m_ReqLevelPos.x, iY, szText, 0 ,0, RT3_WRITE_CENTER);

			itoa((*li)->_ReqInfo.iReqZen, szText, 10);
			if( (*li)->_ReqInfo.iReqZen > (int)iZen )
			{
				g_pRenderText->SetTextColor(255, 51, 26, 255);
			}
			else
			{
				g_pRenderText->SetTextColor(164, 39, 17, 255);
			}
			g_pRenderText->RenderText(m_ReqZenPos.x, iY, szText, 0 ,0, RT3_WRITE_CENTER);
		}

		iCurRenderTextIndex++;
	}

	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->RenderText(m_MapNameUISize.x/2, m_MapNameUISize.y-m_iRealFontHeight-5, GlobalText[1002], 0 ,0, RT3_WRITE_CENTER);
	DisableAlphaBlend();
	return true;
}

void SEASON3B::CNewUIMoveCommandWindow::OpenningProcess()
{
	SetPos(m_Pos.x, m_Pos.y);
	SetStrifeMap();
	SettingCanMoveMap();

#if defined(__ANDROID__) || defined(MU_IOS)
	if (IsAndroidMoveMapGridMode())
	{
		m_iCurPage = 1;
		int moveMapX = 0;
		int moveMapY = 0;
		if (AndroidGetMoveMapWindowPosition(m_MapNameUISize.x, m_MapNameUISize.y, &moveMapX, &moveMapY))
		{
			SetPos(moveMapX, moveMapY);
		}

		m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;
		UpdateScrolling();
		return;
	}
#endif

	m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;
	m_ScrollBtnPos.y = m_ScrollBtnStartPos.y;
	m_iRenderStartTextIndex = 0;

	m_iRenderEndTextIndex = m_iRenderStartTextIndex + MOVECOMMAND_MAX_RENDER_TEXTLINE;

	if( m_iRenderEndTextIndex > (int)m_listMoveInfoData.size() )
	{
		m_iRenderEndTextIndex -= (m_iRenderEndTextIndex-m_listMoveInfoData.size());
	}
}

void SEASON3B::CNewUIMoveCommandWindow::ClosingProcess()
{

}

float SEASON3B::CNewUIMoveCommandWindow::GetLayerDepth()
{
	return 8.3f;
}

void SEASON3B::CNewUIMoveCommandWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_scrollbar_up.tga", IMAGE_MOVECOMMAND_SCROLL_TOP);
	LoadBitmap("Interface\\newui_scrollbar_m.tga", IMAGE_MOVECOMMAND_SCROLL_MIDDLE, GL_LINEAR);
	LoadBitmap("Interface\\newui_scrollbar_down.tga", IMAGE_MOVECOMMAND_SCROLL_BOTTOM);
	LoadBitmap("Interface\\newui_scroll_on.tga", IMAGE_MOVECOMMAND_SCROLLBAR_ON, GL_LINEAR);
	LoadBitmap("Interface\\newui_scroll_off.tga", IMAGE_MOVECOMMAND_SCROLLBAR_OFF, GL_LINEAR);
}

void CNewUIMoveCommandWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_MOVECOMMAND_SCROLL_TOP);
	DeleteBitmap(IMAGE_MOVECOMMAND_SCROLL_MIDDLE);
	DeleteBitmap(IMAGE_MOVECOMMAND_SCROLL_BOTTOM);
	DeleteBitmap(IMAGE_MOVECOMMAND_SCROLLBAR_ON);
	DeleteBitmap(IMAGE_MOVECOMMAND_SCROLLBAR_OFF);
}

BOOL CNewUIMoveCommandWindow::IsTheMapInDifferentServer(const int iFromMapIndex, const int iToMapIndex) const
{
	BOOL bInOtherServer = FALSE;
	
	switch(iFromMapIndex) 
	{
	case WD_30BATTLECASTLE:
	case WD_79UNITEDMARKETPLACE:
		bInOtherServer = TRUE;
		break;
	default:
		break;
	}
	
	switch(iToMapIndex)
	{
	case 24:
	case 44:
		bInOtherServer = TRUE;
		break;
	default:
		break;
	}
	
	return bInOtherServer;
}

int CNewUIMoveCommandWindow::GetMapIndexFromMovereq(const char *pszMapName)
{
	if (pszMapName == NULL)
		return -1;

	int iMapIndex = -1;
	std::list<CMoveCommandData::MOVEINFODATA*>::iterator li;
	for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); li++)
	{
		if (stricmp((*li)->_ReqInfo.szMainMapName, pszMapName) == 0 || stricmp((*li)->_ReqInfo.szSubMapName, pszMapName) == 0)
		{
			iMapIndex = (*li)->_ReqInfo.index;
			break;
		}
	}
	
	return iMapIndex;
}
