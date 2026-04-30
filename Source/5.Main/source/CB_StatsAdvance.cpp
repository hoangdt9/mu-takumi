#include "stdafx.h"
#include "CB_StatsAdvance.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "Protocol.h"
#include "ZzzLodTerrain.h"
#include "CustomRankUser.h"
#include "MapManager.h"

CB_StatsAdvance* gCB_StatsAdvance;


CB_StatsAdvance::CB_StatsAdvance()
{
	this->m_Loaded = false;
	memset(&this->m_Data, 0, sizeof(this->m_Data));
	this->Page = 0;
}


CB_StatsAdvance::~CB_StatsAdvance()
{

}
void CB_StatsAdvance::DrawLine(float PosX, float PosY, int Width, LPCSTR Text, ...)
{
	float flResult = 0.0;

	char szBuff[256];
	int iBuffLen = sizeof(szBuff) - 1;
	ZeroMemory(szBuff, iBuffLen);

	va_list args;
	va_start(args, Text);
	int Len = vsprintf_s(szBuff, iBuffLen, Text, args);
	va_end(args);
	TextDraw(g_hFont, PosX, PosY, 0xFFFFFFFF, 0x0, Width, 0, 0, szBuff);
}
extern void CBRenderText(char* text, int x, int y, int sx, int sy, DWORD color, DWORD backcolor, int sort);
float CB_StatsAdvance::DrawLineR(float PosX, float PosY, int Width, DWORD dwColor, LPCSTR Text, ...)
{
	float flResult = 0.0;

	char szBuff[256];
	int iBuffLen = sizeof(szBuff) - 1;
	ZeroMemory(szBuff, iBuffLen);

	va_list args;
	va_start(args, Text);
	int Len = vsprintf_s(szBuff, iBuffLen, Text, args);
	va_end(args);

	//gInterface.DrawFormat(dwColor, PosX, PosY, Width, 4, szBuff);
	CBRenderText(szBuff, PosX, PosY, Width, 0, dwColor, 0, 4);

	flResult = PosY + 12;

	return flResult;
}

void CB_StatsAdvance::RenderInfo(int StartX, int StartY)
{
	if (!this->m_Loaded) return;

	g_pBCustomMenuInfo->DrawInfoBox(StartX + 10, StartY + 50, 165, 310, 0x00000096, 0, 0);

	int CountLine = 0;

	float flDrawX = StartX + 15;	// - 15;
	float flDrawY = StartY + 55;//this->m_CharY - 52;	//18;
	
	float flWidth = 160;
	int StartIndexText = 1;
	if (this->Page == 0)
	{
		StartIndexText = 2;
		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.TotalDamageReflect);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.FullDamageReflectRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.CriticalDamageRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eBlue, "%d", this->m_Data.CriticalDamagePVM);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eBlue, "%d", this->m_Data.CriticalDamagePVP);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.ExellentDamageRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eAncient, "%d", this->m_Data.ExellentDamagePVM);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eAncient, "%d", this->m_Data.ExellentDamagePVP);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.DoubleDamageRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.TripleDamageRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.DamageReductionRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.ShieldSkillDamageReductionRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.SDDamageReductionRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.SDDecreaseDamageRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.IgnoreDefenceRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.IgnoreSDRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d", this->m_Data.IncreaseDamagePvP);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d", this->m_Data.IncreaseDefencePvP);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.ResistDoubleDamageRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.ResistIgnoreDefenceRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.ResistIgnoreSDRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.ResistCriticalDamageRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.ResistExellentDamageRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%%", this->m_Data.ResistStumRate);
	}
	else
	{
		StartIndexText = 26;
		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d - %d%%", this->m_Data.IncreaseIce, this->m_Data.ResistIce);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d - %d%%", this->m_Data.IncreasePoison, this->m_Data.ResistPoison);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d - %d%%", this->m_Data.IncreaseLighting, this->m_Data.ResistLighting);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d - %d%%", this->m_Data.IncreaseFire, this->m_Data.ResistFire);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d - %d%%", this->m_Data.IncreaseEarth, this->m_Data.ResistEarth);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d - %d%%", this->m_Data.IncreaseWind, this->m_Data.ResistWind);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d - %d%%", this->m_Data.IncreaseWater, this->m_Data.ResistWater);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%% - %d%%", this->m_Data.FullHPRestoreRateDef, this->m_Data.FullHPRestoreRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%% - %d%%", this->m_Data.FullMPRestoreRateDef, this->m_Data.FullMPRestoreRate);

		this->DrawLine(flDrawX, flDrawY, flWidth, gOther.Text_ChacracterInfo[StartIndexText++]);
		flDrawY = this->DrawLineR(flDrawX, flDrawY, flWidth, eGold, "%d%% - %d%%", this->m_Data.FullSDRestoreRateDef, this->m_Data.FullSDRestoreRate);
	}

	//Next Page
	float MainWidth = 190;
	float MainHeight = 380;
	TextDraw((HFONT)g_hFont, StartX, StartY + (MainHeight - 35), 0x7DF4FFFF, 0x0, MainWidth, 0, 3, gOther.Text_Chung[21], this->Page + 1, 2);

	if (this->Page > 0)
	{
		if (g_pBCustomMenuInfo->DrawButtonGUI(CNewUIInGameShop::IMAGE_IGS_STORAGE_PAGE_LEFT, StartX + 40, StartY + (MainHeight - 40), 20, 23, 3))
		{ 
			this->Page--;
		}
	}
	if (this->Page < 1)
	{
		if (g_pBCustomMenuInfo->DrawButtonGUI(CNewUIInGameShop::IMAGE_IGS_STORAGE_PAGE_RIGHT, StartX + (MainWidth - 65), StartY + (MainHeight - 40), 20, 23, 3))
		{
			this->Page++;
		}
	}

}
void CB_StatsAdvance::Recv(BYTE* Recv)
{
	if (!Recv) return;

	PMSG_STATS_ADVANCE* lpMsg = (PMSG_STATS_ADVANCE*)Recv;

	if (!lpMsg->Result)
	{
		return;
	}

	memcpy(&this->m_Data, &lpMsg->StatInfo, sizeof(this->m_Data));

	this->m_Loaded = true;
}
