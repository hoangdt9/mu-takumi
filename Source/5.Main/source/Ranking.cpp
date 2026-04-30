#include "stdafx.h"
#include "Ranking.h"
#include "ZzzInfomation.h"
#include "GlobalText.h"
#include "_types.h"
#include "CBInterface.h"
#include "NewUISystem.h"
#include "Other.h"
#include "Util.h"
#if(RANKING_NEW == 1)

cRanking gRanking;
int DelayOpenRankWindow = 0;
cRanking::cRanking()
{
	this->StartX = 0;
	this->StartY = 0;
	this->Page = 1;
	ZeroMemory(&this->RankingChar, sizeof(this->RankingChar));
}

cRanking::~cRanking()
{
}


void cRanking::RecvInfo(DGCharTop* lpMsg)
{
	ZeroMemory(&this->RankingChar, sizeof(this->RankingChar));
	for (int i = 0; i < MAXTOP; i++)
	{
		strncpy(this->RankingChar[i].Name, lpMsg->tp[i].Name, 11);
		this->RankingChar[i].Class = lpMsg->tp[i].Class;
		this->RankingChar[i].level = lpMsg->tp[i].level;
		this->RankingChar[i].Reset = lpMsg->tp[i].Reset;
		this->RankingChar[i].Relifes = lpMsg->tp[i].Relifes;
		this->RankingChar[i].Time = lpMsg->tp[i].Time;
		this->RankingChar[i].Map = lpMsg->tp[i].Map;
		strncpy(this->RankingChar[i].Guild, lpMsg->tp[i].Guild, 9);
	}
	if (DelayOpenRankWindow == 2)
	{
		gInterface.Data[eMAIN_RANKING].OnShow = 1;
		
	}
	DelayOpenRankWindow = 1;
}
DWORD TimeUpdateRanking = 0;
void cRanking::OpenOnOff()
{
	if (GetTickCount() < (gInterface.Data[eMAIN_RANKING].EventTick + 300))
	{
		return;
	}
	if (gInterface.Data[eMAIN_RANKING].OnShow == 0)
	{
		SEND_GS_REQ pRequest;
		pRequest.h.set(0xFD, 0x21, sizeof(pRequest));
		DataSend((BYTE*)& pRequest, pRequest.h.size);
		TimeUpdateRanking = GetTickCount();
		DelayOpenRankWindow = 2;
	}
	else
	{
		gInterface.Data[eMAIN_RANKING].OnShow = 0;
	}
	gInterface.Data[eMAIN_RANKING].EventTick = GetTickCount();
}
int cRanking::GetNumberTop(int KeyIndex)
{
	if (DelayOpenRankWindow == 0)
	{
		SEND_GS_REQ pRequest;
		pRequest.h.set(0xFD, 0x21, sizeof(pRequest));
		DataSend((BYTE*)& pRequest, pRequest.h.size);
		TimeUpdateRanking = GetTickCount();
		return -1;
	}

	int Index = FindCharacterIndex(KeyIndex);
	if (Index >= 0 && Index != MAX_CHARACTERS_CLIENT)
	{
		for (int i = 0; i < 18; i++)
		{
			if (strcmp(CharactersClient[Index].ID, gRanking.RankingChar[i].Name) == 0) //
			{
				return i;
			}
		}
	}
	return -1;
}
void cRanking::Draw()
{
	if (!gInterface.Data[eMAIN_RANKING].OnShow) return;
	float MainWidth = 310.0;
	float MainHeight = 364.0;
	float StartY = ((MAX_WIN_HEIGHT - 51) / 2) - (MainHeight / 2);
	float StartX = (MAX_WIN_WIDTH / 2) - (MainWidth / 2);

	//--

	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, MainWidth, MainHeight, eMAIN_RANKING, gOther.Text_Chung[15]);
	float MainCenter = StartX + (MainWidth / 3);
	float ButtonX = MainCenter - (float)(29.0 / 2);

	int TitleX = StartX + 20;
	int TitleY = StartY + 40;
	g_pBCustomMenuInfo->DrawInfoBox(TitleX - 10, TitleY - 3, MainWidth - 30, 10, 0x00000096, 0); //1vs3
	TextDraw((HFONT)g_hFontBold, TitleX, TitleY, 0xFFFFFFA8, 0x0, 100, 0, 1, gOther.Text_Chung[16]);
	//gInterface.DrawFormat(eYellow, TitleX, TitleY, 100, 1, "(N)");

	TitleX += 20;
	//gInterface.DrawFormat(eYellow, TitleX, TitleY, 100, 1, "Name");
	TextDraw((HFONT)g_hFontBold, TitleX, TitleY, 0xFFFFFFA8, 0x0, 100, 0, 1, gOther.Text_Chung[17]);

	TitleX += 80;
	//gInterface.DrawFormat(eYellow, TitleX, TitleY, 100, 1, "Class");
	TextDraw((HFONT)g_hFontBold, TitleX, TitleY, 0xFFFFFFA8, 0x0, 100, 0, 1, gOther.Text_Chung[18]);

	TitleX += 70;
	//gInterface.DrawFormat(eYellow, TitleX, TitleY, 100, 1, "Level");
	TextDraw((HFONT)g_hFontBold, TitleX, TitleY, 0xFFFFFFA8, 0x0, 100, 0, 1, gOther.Text_Chung[19]);

	TitleX += 40;
	//gInterface.DrawFormat(eYellow, TitleX, TitleY, 100, 1, "Reset/Relife");
	TextDraw((HFONT)g_hFontBold, TitleX, TitleY, 0xFFFFFFA8, 0x0, 100, 0, 1, gOther.Text_Chung[20]);

	//TitleX += 50;
	//gInterface.DrawFormat(eYellow, TitleX, TitleY, 100, 1, "Relifes");

	//TitleX += 55;
	//gInterface.DrawFormat(eYellow, TitleX, TitleY, 100, 1, "Guild");

	//TitleX += 70;
	//gInterface.DrawFormat(eYellow, TitleX, TitleY, 100, 1, "Thời gian Resets cuối");

	int InfoY = StartY + 60;

	int c = 0;


	time_t mTime;
	tm * lpTime = NULL;

	for (int i = 0; i < 20; i++)
	{
		int InfoX = StartX + 20;
		int up = i + ((this->Page - 1) * 20);

		if (this->RankingChar[up].Name[0] == NULL) continue;

		DWORD Color;

		if (c == 0)
		{
			Color = eWhite;
		}
		else
		{
			Color = eShinyGreen;
		}
		if (up == 0)
		{
			gInterface.DrawBarForm(InfoX - 10, InfoY - 3, MainWidth - 25, 14, 1.0, 0.8418, 0.27, 0.4);
		}
		else if (up == 1)
		{
			gInterface.DrawBarForm(InfoX - 10, InfoY - 3, MainWidth - 25, 14, 0.97, 0.5199, 0.0388, 0.4);
		}
		else if (up == 2)
		{
			gInterface.DrawBarForm(InfoX - 10, InfoY - 3, MainWidth - 25, 14, 0.0388, 0.97, 0.6286, 0.4);
		}
		else
		{
			if (up % 2)
			{
				gInterface.DrawBarForm(InfoX - 10, InfoY - 3, MainWidth - 25, 14, 0.46, 0.459, 0.4554, 0.4);
			}
			else
			{
				gInterface.DrawBarForm(InfoX - 10, InfoY - 3, MainWidth - 25, 14, 0.29, 0.2378, 0.0493, 0.4);
			}
		}
		TextDraw((HFONT)g_hFontBold, InfoX, InfoY, 0xFFB845A8, 0x0, 100, 0, 1, "%d", up + 1);
		//gInterface.DrawFormat(Color, InfoX, InfoY, 100, 1, "%d", up + 1);

		InfoX += 20;
		char NameID[11];
		memcpy(NameID, this->RankingChar[up].Name, sizeof(this->RankingChar[up].Name));
		//gInterface.DrawFormat(Color, InfoX, InfoY, 100, 1, NameID);
		TextDraw((HFONT)g_hFontBold, InfoX, InfoY, 0x45FFD7FF, 0x0, 100, 0, 1, NameID);

		InfoX += 80;
		//gInterface.DrawFormat(Color, InfoX, InfoY, 100, 1, CharacterCode(this->RankingChar[up].Class));
		TextDraw((HFONT)g_hFontBold, InfoX, InfoY, 0xFCFF45FF, 0x0, 100, 0, 1, CharacterCode(this->RankingChar[up].Class));

		InfoX += 70;
		//gInterface.DrawFormat(Color, InfoX, InfoY, 100, 1, "%d", this->RankingChar[up].level);
		TextDraw((HFONT)g_hFontBold, InfoX, InfoY, 0xFFD045FF, 0x0, 100, 0, 1, "%d", this->RankingChar[up].level);

		InfoX += 50;
		//gInterface.DrawFormat(Color, InfoX, InfoY, 100, 1, "%d/(%d)", this->RankingChar[up].Reset, this->RankingChar[up].Relifes);
		TextDraw((HFONT)g_hFontBold, InfoX, InfoY, 0x26F70AFF, 0x0, 100, 0, 1, "%d/(%d)", this->RankingChar[up].Reset, this->RankingChar[up].Relifes);
		//InfoX += 55;
		//char GuildID[9];
		//memcpy(GuildID, this->RankingChar[up].Guild, sizeof(this->RankingChar[up].Guild));
		//gInterface.DrawFormat(Color, InfoX, InfoY, 100, 1, GuildID);

		//int mYear = 0;
		//int mMonth = 0;
		//int mDays = 0;
		//int hours = 0;
		//int minutes = 0;
		//int seconds = 0;
		//mTime = this->RankingChar[up].Time;
		//lpTime = localtime(&mTime);
		//if (lpTime)
		//{
		//	mYear = 1952 + lpTime->tm_year;
		//	mMonth = 1 + lpTime->tm_mon;
		//	mDays = lpTime->tm_mday;
		//	hours = lpTime->tm_hour;
		//	minutes = lpTime->tm_min;
		//	seconds = lpTime->tm_sec;
		//}
		//InfoX += 65;
		//gInterface.DrawFormat(eSocket, InfoX, InfoY, 100, 1, "%02d:%02d:%02d (%02d/%02d/%d)", hours, minutes, seconds, mDays, mMonth, mYear);


		InfoY += 14; // 12
		c++;
		if (c == 2)c = 0;
	}

	DWORD Delay = (GetTickCount() - this->ClickTime);

	int ButX = 50;
	int ButY = 63;

	RenderImage(CNewUIQuestProgress::IMAGE_QP_BTN_L, StartX + 60 + ButX, StartY + 275 + ButY, 17, 17);

	if (SEASON3B::CheckMouseIn(StartX + 60 + ButX, StartY + 275 + ButY, 17, 17))
	{
		if ((GetKeyState(VK_LBUTTON) & 0x8000) && Delay > 300)
		{
			this->ClickTime = GetTickCount();
			if (this->Page > 1) this->Page--;
		}
		RenderImage(CNewUIQuestProgress::IMAGE_QP_BTN_L, StartX + 60 + ButX, StartY + 275 + ButY, 17, 17, 0, 0, RGBA(255, 204, 20, 130));
	}

	TextDraw((HFONT)g_hFontBold, StartX + 96 + ButX, StartY + 277 + ButY, 0xFFFFFFA8, 0x0, 100, 0, 1, gOther.Text_Chung[21], this->Page, 7);

	RenderImage(CNewUIQuestProgress::IMAGE_QP_BTN_R, StartX + 150 + ButX, StartY + 275 + ButY, 17, 17);

	if (SEASON3B::CheckMouseIn(StartX + 150 + ButX, StartY + 275 + ButY, 17, 17))
	{
		if ((GetKeyState(VK_LBUTTON) & 0x8000) && Delay > 300)
		{
			this->ClickTime = GetTickCount();
			if (this->Page < 7) this->Page++;
		}
		RenderImage(CNewUIQuestProgress::IMAGE_QP_BTN_R, StartX + 150 + ButX, StartY + 275 + ButY, 17, 17, 0, 0, RGBA(255, 204, 20, 130));
	}
}
#endif