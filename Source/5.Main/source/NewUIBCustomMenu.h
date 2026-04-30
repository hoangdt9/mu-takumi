// NewUIBCustomMenu.h: interface for the CNewUIBCustomMenuInfo class.
//////////////////////////////////////////////////////////////////////
//g_pBCustomMenuInfo
#if !defined(AFX_NewUIBCustomMenu_H__5D452B62_B315_41EE_A862_5929286925F7__INCLUDED_)
#define AFX_NewUIBCustomMenu_H__5D452B62_B315_41EE_A862_5929286925F7__INCLUDED_

#pragma once
#include "NewUIBase.h"
#include "NewUIManager.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIButton.h"
#include "NewUIManager.h"
#include "UIControls.h"

struct GUIObj
{
	float X;
	float Y;
	float Default_X;
	float Default_Y;
	float curX;
	float curY;
	float Width;
	float Height;
	float Max_X;
	float Max_Y;
	bool  isFocus;
	bool onShow;
	bool dragging;
	bool allowDrag;
	bool scaleUp;
	BYTE state;
	DWORD lastActionTime;
	int FirstLoad;

	void Clear()
	{
		//X = Default_X;
		//Y = Default_Y;
		//curX = Default_X;
		//curY = Default_Y;
		dragging = false;
		onShow = 0;
		lastActionTime = GetTickCount();
		state = 0;
		isFocus = false;
		scaleUp = true;
		FirstLoad = 0;
	}


	GUIObj() {
		onShow = false;
		dragging = false;
		lastActionTime = GetTickCount();
		state = 0;
		isFocus = false;
		allowDrag = false;
		scaleUp = true;
		FirstLoad = 0;
	}
	bool GetFirstLoad()
	{
		if ((GetTickCount() - lastActionTime) > 300 && FirstLoad == 0)
		{
			return 1;
		}

		return 0;
	}

	void  OnOffWindow()
	{
		onShow ^= 1;
		lastActionTime = GetTickCount();
	}
	GUIObj(float aX, float aY, float aW, float aH, bool mallowDrag = false, bool mscaleUp = true)
	{
		onShow = false;
		dragging = false;
		lastActionTime = GetTickCount();
		state = 0;
		isFocus = false;
		allowDrag = false;
		scaleUp = true;

		X = aX;
		Y = aY;
		Default_X = aX;
		Default_Y = aY;
		Width = aW;
		Height = aH;
		Max_X = X + Width;
		Max_Y = Y + Height;
		allowDrag = mallowDrag;
		scaleUp = mscaleUp;
		FirstLoad = 0;
	}

};


namespace SEASON3B
{
	class CNewUIBCustomMenuInfo : public CNewUIObj
	{
	public:
		enum IMAGE_LIST
		{	
			IMAGE_BCUSTOM_WINDOW = BITMAP_BCUSTOM_MENU_BEGIN,		// newui_position02.tga			(70, 25)	
			IMAGE_BCUSTOM_WINDOW_31322,		
			IMAGE_BCUSTOM_WINDOW_31353,
			IMAGE_BCUSTOM_WINDOW_31357,
			IMAGE_BCUSTOM_WINDOW_31355,
			IMAGE_BCUSTOM_WINDOW_31356,
			IMAGE_BCUSTOM_WINDOW_32453,
			IMAGE_BCUSTOM_WINDOW_31326,
			IMAGE_OPTION_B_BACK,
			IMAGE_OPTION_B_COLOR,

			IMAGE_INFO_TOP_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,
			IMAGE_INFO_BOTTOM_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,
			IMAGE_INFO_LEFT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,
			IMAGE_INFO_RIGHT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,

			IMAGE_INFO_TOP_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,	//. newui_item_table01(L).tga (14,14)
			IMAGE_INFO_TOP_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,	//. newui_item_table01(R).tga (14,14)
			IMAGE_INFO_BOTTOM_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,	//. newui_item_table02(L).tga (14,14)
			IMAGE_INFO_BOTTOM_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)		

			IMAGE_B_TABLE_TOP_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,			//. newui_item_table01(L).tga (14,14)
			IMAGE_B_TABLE_TOP_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,			//. newui_item_table01(R).tga (14,14)
			IMAGE_B_TABLE_BOTTOM_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,		//. newui_item_table02(L).tga (14,14)
			IMAGE_B_TABLE_BOTTOM_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)
			IMAGE_B_TABLE_TOP_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,			//. newui_item_table03(up).tga (1, 14)
			IMAGE_B_TABLE_BOTTOM_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,	//. newui_item_table03(dw).tga (1,14)
			IMAGE_B_TABLE_LEFT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,		//. newui_item_table03(L).tga (14,1)
			IMAGE_B_TABLE_RIGHT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,		//. newui_item_table03(R).tga (14,1)
		};
		enum CLASS_LIST_
		{
			Dark_Wizard = 0,
			Dark_Knight,
			Fairy_Elf,
			Magic_Gladiator,
			Dark_Lord,
			Summoner,
			Rage_Fighter,
		};

	private:
	
		CNewUIManager*				m_pNewUIMng;
		POINT								m_Pos;
		
	public:
		CNewUIBCustomMenuInfo();
		virtual ~CNewUIBCustomMenuInfo();
		
		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();

		bool BtnProcess();
		void SetBlockCur(bool Delay =  true);
		
		float GetLayerDepth();	//. 4.3f

		void OpenningProcess();

		void ClosingProcess();

		void BindObject(short ObjectID, char* Patch, float Width = 0, float Height = 0, float X =0, float Y = 0,int Type = GL_CLAMP); //GL_REPEAT
		void DrawGUI(short ObjectID, float PosX, float PosY, bool SetPos = false);
		bool		DrawWindowBG(GUIObj*, LPCSTR, ...);
		bool		DrawButton(float PosX, float PosY, float SizeButton, int FontSize, char* ButtonText, float mSizeButtonW = 0.0f, bool IsEnter = false);
		bool		DrawButtonGUI(int IDGUID, float PosX, float PosY, float SizeW, float SizeH, int TypeButton = 0);
		void		DrawInfoBox(float PosX, float PosY, float Width, float Height, DWORD bkcolor = 0x0, int Arg6=0, int Arg7=0);
		void		gItemBoxInv(int X, int Y, int Doc, int Ngang, int Use);
		//GUIObj*		WindowMenuCustom;
		//GUIObj*		WindowMUHelperConfig;
		bool gDrawWindowCustom(float* StartX, float* StartY, float CuaSoW, float CuaSoH, int eNumWindow, LPCSTR Text, ...);
		bool DrawWindowCustomMini(float* StartX, float* StartY, float CuaSoW, float CuaSoH, int eNumWindow, LPCSTR Text, ...);
		
		void RenderGroupBox(int iPosX, int iPosY, int iWidth, int iHeight, int iTitleWidth = 60, int iTitleHeight = 20);
		bool RenderCheckBoxMini(float PosX, float PosY, DWORD TextColor, bool Checkis, LPCSTR Text, ...);
		bool RenderCheckBox(float PosX, float PosY, DWORD TextColor, bool Checkis, LPCSTR Text, ...);
		bool RenderCheckOption(float PosX, float PosY, DWORD TextColor, bool Checkis, LPCSTR Text, ...);
		void RederBarOptionW(int iPosX, int iPosY, int* Vaule);
		bool RenderInputBox(float PosX, float PosY, float W, float H, char* TextSet, CUITextInputBox*& Input, UIOPTIONS UiOption, int MaxChar = 50, bool isPass = 0);
		bool UseHotKey();
		bool BLockButtonMoveWindow;
		bool BLockButtonHover;
		bool BLockMessageBox;

		bool AutoCtrlPK;
		bool AutoHP;
	private:
		void LoadImages();
		void UnloadImages();	

		
	};
}

#endif // !defined(AFX_NewUIBCustomMenu_H__5D452B62_B315_41EE_A862_5929286925F7__INCLUDED_)
