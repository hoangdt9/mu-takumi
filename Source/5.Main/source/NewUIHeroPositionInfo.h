// NewUIHeroPositionInfo.h: interface for the CNewUIHeroPositionInfo class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIHEROPOSITIONINFO_H__5D452B62_B315_41EE_A862_5929286925F7__INCLUDED_)
#define AFX_NEWUIHEROPOSITIONINFO_H__5D452B62_B315_41EE_A862_5929286925F7__INCLUDED_

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIButton.h"

namespace SEASON3B
{
	class CNewUIHeroPositionInfo : public CNewUIObj
	{
	public:
		enum IMAGE_LIST
		{	
			IMAGE_HERO_POSITION_INFO_BASE_WINDOW = BITMAP_HERO_POSITION_INFO_BEGIN,		// newui_position02.tga			(70, 25)	
		};
	private:
		enum HERO_POSITION_INFO_BASE_WINDOW_SIZE
		{
			HERO_POSITION_INFO_BASEA_WINDOW_WIDTH = 22,
			HERO_POSITION_INFO_BASE_WINDOW_HEIGHT = 25,
			HERO_POSITION_INFO_BASEB_WINDOW_WIDTH = 80,
			HERO_POSITION_INFO_BASEC_WINDOW_WIDTH = 22,
		};
		struct LoadTypeMap
		{
			/*+0*/   DWORD ModelID;
			/*+4*/   DWORD XSW_Minimap_Frame;
			/*+8*/   DWORD YSW_Minimap_Frame;
			/*+12*/  DWORD XSW_Minimap;
			/*+16*/  DWORD YSW_Minimap;
			/*+20*/  DWORD XSW_Time_Frame;
			/*+24*/  DWORD YSW_Time_Frame;
			/*+92*/  DWORD This_92;
			/*+96*/  DWORD This_96;
			/*+100*/ DWORD DropMoveUserX;
			/*+104*/ DWORD DropMoveUserY;
			/*+108*/ DWORD This_108;
			/*+112*/ DWORD This_112;
			/*+116*/ float CurrentMoveX;
			/*+120*/ float CurrentMoveY;
			/*+124*/ DWORD Scale;
			/*+126*/ DWORD TimerEventVK;
			/*+128*/ float Alpha;
		};

	private:
		int							WidenX;
		CNewUIManager*				m_pNewUIMng;
		POINT						m_Pos;
		POINT						m_CurHeroPosition;
		CNewUIButton				m_BtnConfig;
		CNewUIButton				m_BtnStart;
		CNewUIButton				m_BtnStop;
	public:
		CNewUIHeroPositionInfo();
		virtual ~CNewUIHeroPositionInfo();
		
		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();

		bool BtnProcess();
		
		float GetLayerDepth();	//. 4.3f

		void OpenningProcess();
		void ClosingProcess();

		void SetCurHeroPosition( int x, int y );
		void MoveTextTipPos(CNewUIButton* m_Btn, int iX, int iY);
		void SetButtonInfo(CNewUIButton* m_Btn, int imgindex, int x, int y, int sx, int sy, bool overflg, bool isimgwidth, bool bClickEffect, bool MoveTxt, unicode::t_string btname, unicode::t_string tooltiptext, bool istoppos);

		void MiniMapLoad();
		void DrawMiniMap();
		LoadTypeMap DataMap;
		void DataMapLoad();
		void DataViewPortMapLoad(float x, float y);
		int IndexIMGMap;
	private:
		void LoadImages();
		void UnloadImages();	
		
		DWORD TimeClickMouseL;
		
	};
}

#endif // !defined(AFX_NEWUIHEROPOSITIONINFO_H__5D452B62_B315_41EE_A862_5929286925F7__INCLUDED_)
