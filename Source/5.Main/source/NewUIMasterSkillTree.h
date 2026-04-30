#ifndef __NEW_UI_MASTER_SKILL_TREE_H__
#define __NEW_UI_MASTER_SKILL_TREE_H__

#pragma once

#include "NewUIBase.h"
#include "ZzzTexture.h"
#include "NewUIButton.h"

#define MAX_MASTER_SKILL_DATA 512

#define MAX_MASTER_SKILL_CATEGORY 3

#if(NEW_MASTER_SKILL_TREE == 1)

class CSkillTreeInfo
{
public:
	CSkillTreeInfo()
	{
		this->SkillPoint = 0;
		this->SkillValue = 0.0f;
		this->SkillNextValue = 0.0f;
	};

	~CSkillTreeInfo()
	{
	};

	void SetSkillTreeInfo(BYTE point, float value, float nextvalue)
	{
		this->SkillPoint = point;
		this->SkillValue = value;
		this->SkillNextValue = nextvalue;
	}

	BYTE GetSkillPoint() { return this->SkillPoint; }
	float GetSkillValue() { return this->SkillValue; }
	float GetSkillNextValue() { return this->SkillNextValue; }
private:
	BYTE SkillPoint;
	float SkillValue;
	float SkillNextValue;
};

struct _MASTER_SKILLTREE_DATA
{
	WORD Index;
	WORD ClassCode;
	BYTE Group;
	BYTE RequiredPoints;
	BYTE MaxLevel;
	BYTE ArrowDirection;
	int RequireSkill[2];
	int Skill;
	float DefValue;
};

struct _MASTER_SKILL_TOOLTIP
{
	_MASTER_SKILL_TOOLTIP()
	{
		sizeof(_MASTER_SKILL_TOOLTIP);
	}
	int SkillIndex;
	WORD ClassCode;
	char Info1[64];
	char Info2[256];
	char Info3[32];
	char Info4[64];
	char Info5[64];
	char Info6[64];
	char Info7[64];
};

namespace SEASON3B
{
	//size = 404
	class CNewUIMasterSkillTree : public CNewUIObj
	{
	public:
		enum IMAGE_LIST
		{
			/*IMAGE_MASTER_SCROLLBAR_ON = CNewUIChatLogWindow::IMAGE_SCROLLBAR_ON,
			IMAGE_MASTER_SCROLLBAR_OFF = CNewUIChatLogWindow::IMAGE_SCROLLBAR_OFF,
			IMAGE_MASTER_SKILL_ICON = CNewUISkillList::IMAGE_SKILL2,	// newui_skill2.jpg
			IMAGE_MASTER_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,		//. newui_exit_00.tga*/
			IMAGE_MASTER_INTERFACE = BITMAP_INTERFACE_MASTER_BEGIN,
		};

		CNewUIMasterSkillTree();
		~CNewUIMasterSkillTree();
		void ClearSkillTreeInfo();
		BYTE GetConsumePoint();
		int GetCurSkillID();
		bool Create();
		void Release();
		void SetPos();
		void OpenMasterSkillTreeData(char* path);
		void OpenMasterSkillTooltip(char* path);
		void InitMasterSkillPoint();
		void SetMasterType(BYTE Class);
		void SetMasterSkillTreeData();
		void SetMasterSkillToolTipData();
		bool SetMasterSkillTreeInfo(int index, char SkillPoint, float value, float nextvalue);
		int SetDivideString(char* text, int isItemTollTip, int TextNum, int iTextColor, int iTextBold, bool isPercent);
		bool Render();
		bool Update();
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		float GetLayerDepth();
		void LoadImages();
		void UnloadImages();
		void RenderText();
		void RenderIcon();
		void RenderToolTip();
		bool CheckMouse(int posx, int posy);
		bool CheckBtn();
		bool CheckAttributeArea(int group, int index, int reqPoint, unsigned int ReqSkill1, unsigned int ReqSkill2, unsigned int Skill, int defValue);
		bool CheckSkillPoint(WORD mLevelUpPoint, int index, int reqPoint, int ReqSkill1, int ReqSkill2, int Skill, int defValue, WORD skillPoint);
		bool CheckParentSkill(DWORD reqSkill1, DWORD reqSkill2);
		bool CheckRankPoint(BYTE group, BYTE rank, BYTE skillPoint);
		bool CheckBeforeSkill(int skill, BYTE skillPoint);
		int GetBeforeSkillID(int index);
		int GetBaseSkillID(int index);
		void SkillUpgrade(int a2, char a3, float a4, float a5);
		bool IsLearnSkill(int index);
		float GetSkillValue(int index);
		float GetSkillCumulativeValue(int index, BYTE damage);
		BYTE GetSkillLevel(int index);
		void RegisterSkillInCharacterAttribute(int index);
		void SetConsumePoint(BYTE ConsumePoint);
		void SetCurSkillID(int index);
		void ClearSkillTreeData();
		void ClearSkillTooltipData();
		bool GetXYImgMaster(JCCoord* GetXY,int Skill);
	private:
		BITMAP_t* pBitMap[2];
		int PosX;
		int PosY;
		int width;
		int height;
		DWORD ButtonX[MAX_MASTER_SKILL_CATEGORY];
		DWORD ButtonY[MAX_MASTER_SKILL_CATEGORY];
		CNewUIButton m_CloseBT;
		//CNewUIButton m_ClearBT;
		BYTE CategoryPoint[MAX_MASTER_SKILL_CATEGORY];
		BYTE skillPoint[MAX_MASTER_SKILL_CATEGORY][10];
		BYTE ConsumePoint;
		int CurSkillID;
		DWORD classCode;
		POINT categoryPos[MAX_MASTER_SKILL_CATEGORY];
		DWORD dword_118;
		DWORD dword_11C;
		DWORD dword_120;
		DWORD dword_124;
		DWORD dword_128;
		DWORD CategoryTextIndex;
		DWORD ClassNameTextIndex;
		std::map<DWORD, CSkillTreeInfo> map_skilltreeinfo;
		std::map<BYTE, _MASTER_SKILLTREE_DATA> map_masterData;
		std::map<DWORD, _MASTER_SKILL_TOOLTIP> map_masterSkillToolTip;
	};
}

#endif

#endif