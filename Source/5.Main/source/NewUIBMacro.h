// NewUIBCustomMenu.h: interface for the CNewUIMacro class.
//////////////////////////////////////////////////////////////////////


#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIButton.h"
#include "NewUIBCustomMenu.h"

namespace SEASON3B
{
	class CNewUIMacro : public CNewUIObj
	{
	public:
		enum IMAGE_LIST
		{	
			IMGA_AUTOINFO = BITMAP_INTERFACE_CUSTOM,

		};

	private:
	
		CNewUIManager*				m_pNewUIMng;
		POINT								m_Pos;
		
		
	public:
		CNewUIMacro();
		virtual ~CNewUIMacro();
		
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
		GUIObj*		WindowAuto;
		void DrawWindowAutoMuHelper();

		
	private:
		void LoadImages();
		void UnloadImages();	

		
	};
}

