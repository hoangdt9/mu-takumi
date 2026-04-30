#include "stdafx.h"
#include "CustomRanking.h"
#include "Protocol.h"
#include "CustomMessage.h"
#include "CBInterface.h"
#include "Util.h"
#include "Protect.h"
#include "Other.h"
#include "NewUISystem.h"
#include "UIControls.h"
#include "CBChoTroi.h"
#include "wsclientinline.h"
#include "WindowClass.h"
#include "CharacterManager.h"

CUIPhotoViewer m_PhotoViewRanking;
int NextRanking = 0;
int PrevRanking = 0;

CustomRanking* gCustomRanking;

CustomRanking::CustomRanking() // OK
{
	this->ClearCustomRanking();
	this->PageBank = 0;
	this->MaxPerPageBank = 10;
}

CustomRanking::~CustomRanking() // OK
{

}
void CustomRanking::ClearCustomRanking() // OK
{
	for (int n = 0; n < MAX_RANK; n++)
	{
		memset(gCustomRanking[n].szName, 0, sizeof(gCustomRanking[n].szName));
		memset(gCustomRanking[n].value2, 0, sizeof(gCustomRanking[n].value2));
		memset(gCustomRanking[n].value3, 0, sizeof(gCustomRanking[n].value3));
#if(CUSTOM_BRANKINGNEW)
		gCustomRanking[n].CheckOnline = 0;
#endif
	}
#if(CUSTOM_BRANKINGNEW)
	ZeroMemory(&this->CacheDataUserTop, sizeof(&this->CacheDataUserTop));
	this->CacheUserRank = false;
	this->SetItemCharTop = false;
#endif
	this->RankEnable = 0;

}
void CustomRanking::GCReqRanking(CustomRanking::PMSG_CUSTOM_RANKING_RECV* lpMsg) // OK
{
	memcpy(this->rankname, lpMsg->rankname, sizeof(this->rankname));
	memcpy(this->col1, lpMsg->col1, sizeof(this->col1));
	memcpy(this->col2, lpMsg->col2, sizeof(this->col2));
	memcpy(this->col3, lpMsg->col3, sizeof(this->col3));
	this->RankEnable = 1;


	mListRanking.clear();

	this->count = lpMsg->count;

	for (int n = 0; n < lpMsg->count; n++)
	{
		CUSTOM_RANKING_DATA* lpInfo = (CUSTOM_RANKING_DATA*)(((BYTE*)lpMsg) + sizeof(PMSG_CUSTOM_RANKING_RECV) + (sizeof(CUSTOM_RANKING_DATA) * n));
		mListRanking.push_back(lpInfo->szName);
		memcpy(gCustomRanking[n].szName, lpInfo->szName, sizeof(gCustomRanking[n].szName));
		memcpy(gCustomRanking[n].value2, lpInfo->value2, sizeof(gCustomRanking[n].value2));
#if(CUSTOM_BRANKINGNEW)
		gCustomRanking[n].CheckOnline = lpInfo->CheckOnline;
#endif
		memcpy(gCustomRanking[n].value3, lpInfo->value3, sizeof(gCustomRanking[n].value3));
	}
}

void CustomRanking::OpenWindow()
{
	if ((GetTickCount() - gInterface.Data[eRankPANEL_MAIN].EventTick) < 1500)
	{
		//gInterface.Data[eRankPANEL_MAIN].OnShow = false;
		return;
	}

	gInterface.Data[eRankPANEL_MAIN].EventTick = GetTickCount();

	if (gInterface.Data[eRankPANEL_MAIN].OnShow == true)
	{
		gInterface.Data[eRankPANEL_MAIN].OnShow = false;

		return;
	}

	this->ClearCustomRanking();

	this->RankingCount = 0;

	this->SelectBarRankingNumber = 0;

	PMSG_CUSTOM_RANKING_COUNT_SEND pMsg1;

	pMsg1.header.set(0xF3, 0xE7, sizeof(pMsg1));

	DataSend((BYTE*)& pMsg1, pMsg1.header.size);

	PMSG_CUSTOM_RANKING_SEND pMsg;

	pMsg.header.set(0xF3, 0xE6, sizeof(pMsg));

	this->RankingType = 0;

	pMsg.type = 0;

	DataSend((BYTE*)& pMsg, pMsg.header.size);

	//gInterface.Data[eRankPANEL_MAIN].EventTick = GetTickCount();
	gInterface.Data[eRankPANEL_MAIN].OnShow = true;

}
void CustomRanking::RankNameRev(PMSG_RANK_INFO * lpMsg)
{
	memset(this->m_RankInfo, 0, sizeof(this->m_RankInfo));
	for (int i = 0; i < MAX_RANK; i++)
	{
		memcpy(this->m_RankInfo[i].NameRank, lpMsg->Info[i].NameRank, 32);
	}
}

void CustomRanking::GCReqRankingCount(PMSG_CUSTOM_RANKING_COUNT_RECV * lpMsg) // OK
{
	this->RankingCount = (lpMsg->count > 0) ? lpMsg->count : 0;
}

#if(CUSTOM_BRANKINGNEW)
void GetInfoCharTop(char* Name)
{
	if (GetTickCount() < gInterface.Data[eRankPANEL_MAIN].EventTick + 300) return;
	gInterface.Data[eRankPANEL_MAIN].EventTick = GetTickCount();

	CustomRanking::REQUESTINFO_CHARTOP pMsg;
	pMsg.header.set(0xD3, 0x40, sizeof(pMsg));
	pMsg.aIndex = 0;
	memcpy(pMsg.NameChar, Name, sizeof(pMsg.NameChar) - 1);
	DataSend((LPBYTE)& pMsg, pMsg.header.size);
}

int rankmodel = 0;
void DrawPlayerRank(int X, int Y, int W, int H)
{
	X = X + 40;
	Y = Y + 40;
	H = H - 100;

	if (!gCustomRanking->CacheUserRank)
	{
		gInterface.DrawFormat(eGold, X + (W / 2), Y + (H / 2), 52, 1, "Loading ..");
	}
	else
	{
		g_pBCustomMenuInfo->DrawInfoBox(X, Y, W, H, 0x00000096, 0, 0);
		//===Show 3D
		m_PhotoViewRanking.SetPosition(X, Y);
		m_PhotoViewRanking.Render();
		m_PhotoViewRanking.DoMouseAction();
		//==Name Char
		TextDraw(g_hFontBold, X + 1, Y, 0xFFFFFFFF, 0x00FBFF69, W + 6, 0, 3, gCustomRanking->CacheDataUserTop.NameChar);//
		////==View Item 
		//g_pBCustomMenuInfo->DrawButtonGUI(31230, (X + W) - 15, Y + 15, 15, 15, 1);//?
		//if (SEASON3B::CheckMouseIn((X + W) - 15, Y + 15, 15, 15) == 1)
		//{
		//	if (SEASON3B::IsPress(VK_LBUTTON) && GetTickCount() > gInterface.Data[eRankPANEL_MAIN].EventTick + 300)
		//	{

		//		//ViewCharItems.DangXem ^= 1;
		//		ViewCharItems.ActionTime = GetTickCount();
		//		gInterface.Data[eRankPANEL_MAIN].EventTick = GetTickCount();
		//	}

		//	//pDrawToolTip(pCursorX - 60, pCursorY - 10, gOther.Text_GAME[124]);
		//}
		if (gCustomRanking->CacheDataUserTop.GuildName[0] != 0xFFFFFFFF && strlen(gCustomRanking->CacheDataUserTop.GuildName) > 1)
		{
			//=Guild
			TextDraw(g_hFontBold, X + (W / 2) - 15, Y + 145, 0xFFFFFFFF, 0x0, 0, 0, 1, "Hội:");//
			TextDraw(g_hFontBold, X + (W / 2), Y + 145, 0x00FBFFFF, 0x0, 0, 0, 1, gCustomRanking->CacheDataUserTop.GuildName);//
			//==Logo Guild
			g_pBCustomMenuInfo->DrawInfoBox((X + W) - 30, Y + 125, 27, 27, 0x00000096, 0, 0);
			//gInterface.DrawBarForm((X + W) - 27, Y + 128, 30, 30, 1.0, 0.0, 0.0, 1.0);//Test
			BCreateGuildMark(gCustomRanking->CacheDataUserTop.GuildMark, 1);
			RenderBitmap(BITMAP_GUILD, (X + W) - 27, Y + 128, 30, 30, 0.0, 0.0, 1.0, 1.0, 1, 1, 0.0);
		}

		float TextX = X + 10;
		//==Tong Point
		g_pBCustomMenuInfo->DrawInfoBox(TextX - 10, (Y + H) + 13, W, 42, 0x00000096, 0, 0);
		TextDraw(g_hFont, TextX, (Y + H) + 15, 0xEBA000FF, 0x0, (W / 4), 0, 3, "Tổng Điểm:");//
		TextDraw(g_hFontBold, TextX + (W / 4), (Y + H) + 15, 0x43F72FFF, 0x0, (W / 4), 0, 3, "%s", gInterface.NumberFormat(gCustomRanking->CacheDataUserTop.TongPoint));//
		//Tong Reset
		TextDraw(g_hFont, TextX + ((W / 4) * 2), (Y + H) + 15, 0xEBA000FF, 0x0, (W / 4), 0, 3, "Reset:");//
		TextDraw(g_hFontBold, TextX + ((W / 4) * 3), (Y + H) + 15, 0x43F72FFF, 0x0, (W / 4), 0, 3, "%s", gInterface.NumberFormat(gCustomRanking->CacheDataUserTop.Reset));//
		//Cấp Độ
		TextDraw(g_hFont, TextX, (Y + H) + 25, 0xEBA000FF, 0x0, (W / 4), 0, 3, "Cấp:");//
		TextDraw(g_hFontBold, TextX + (W / 4), (Y + H) + 25, 0x43F72FFF, 0x0, (W / 4), 0, 3, "%s", gInterface.NumberFormat(gCustomRanking->CacheDataUserTop.Level));//
		//Cấp Master
		TextDraw(g_hFont, TextX + ((W / 4) * 2), (Y + H) + 25, 0xEBA000FF, 0x0, (W / 4), 0, 3, "Master:");//
		TextDraw(g_hFontBold, TextX + ((W / 4) * 3), (Y + H) + 25, 0x43F72FFF, 0x0, (W / 4), 0, 3, "%s", gInterface.NumberFormat(gCustomRanking->CacheDataUserTop.MasterLevel));//
		//Class
		TextDraw(g_hFont, TextX, (Y + H) + 35, 0xEBA000FF, 0x0, (W / 4), 0, 3, "Class:");//
		TextDraw(g_hFontBold, (TextX + (W / 2)) - 40, (Y + H) + 35, 0x43F72FFF, 0x0, 0, 0, 1, "%s", CharacterCode(gCustomRanking->CacheDataUserTop.Class));//
		HFONT CHFontMini = CreateFontA(22, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3u, 0, "Arial");
		DeleteObject(CHFontMini);
		//==Set Item Char

		if (!gCustomRanking->SetItemCharTop)
		{
			m_PhotoViewRanking.ClearItem();
			for (int n = 0; n < 12; n++)
			{
				if (gCustomRanking->CacheDataUserTop.Item[n][0] != 0xFF && gCustomRanking->CacheDataUserTop.Item[n][0] != 0)
				{
					ITEM* CTItem = g_pNewItemMng->CreateItem(gCustomRanking->CacheDataUserTop.Item[n]);
					const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[CTItem->Type];
					if (CTItem) m_PhotoViewRanking.UpdateItemChar(pItemAttr->m_byItemSlot, CTItem->Type + 1171, CTItem->Level, CTItem->Option1, CTItem->ExtOption);

					//int GetInfoLoadItem = ((int(__thiscall*)(int a1, void* lp)) 0x7E1B10)(*(DWORD*)(GetInstance() + 36), gCustomRanking->CacheDataUserTop.Item[n]);
					//if (GetInfoLoadItem)
					//{
					//	character.UpdateItemChar(oHero9 + rankmodel, n, *(WORD*)GetInfoLoadItem + 1171, ((*(DWORD*)(GetInfoLoadItem + 2) >> 3) & 15), *(BYTE*)(GetInfoLoadItem + 23), *(BYTE*)(GetInfoLoadItem + 24));
					//	memcpy(&ViewCharItems.charItem.Item[n], (BYTE*)GetInfoLoadItem, sizeof(ViewCharItems.charItem.Item[n]));

					//}
				}
			}
			gCustomRanking->SetItemCharTop = true;
		}
	}
}

static void BPagePrevAction(LPVOID pClass)
{
	if (GetTickCount() < gInterface.Data[eRankPANEL_MAIN].EventTick + 300) return;
	gInterface.Data[eRankPANEL_MAIN].EventTick = GetTickCount();

	if (gCustomRanking->PageBank > 0)
	{
		gCustomRanking->PageBank--;
	}
	//pDrawMessage("-",1);
}
static void BPageNextAction(LPVOID pClass)
{
	if (GetTickCount() < gInterface.Data[eRankPANEL_MAIN].EventTick + 300) return;
	gInterface.Data[eRankPANEL_MAIN].EventTick = GetTickCount();

	if (gCustomRanking->PageBank < (gCustomRanking->count / gCustomRanking->MaxPerPageBank) - 1)
	{
		gCustomRanking->PageBank++;
	}
	//pDrawMessage("+", 1);
}
void CustomRanking::GCReqInfoCharTop(DATA_VIEWTOPRANKING * lpMsg)
{
	//pDrawMessage("+", 1);
	if (!this->CacheUserRank)
	{
		m_PhotoViewRanking.Init(0);
		m_PhotoViewRanking.SetSize(150, 150);
		//m_PhotoViewRanking.CopyPlayer();
		m_PhotoViewRanking.SetAutoupdatePlayer(TRUE);
		m_PhotoViewRanking.SetAnimation(AT_STAND1);
		m_PhotoViewRanking.SetAngle(120);
		m_PhotoViewRanking.SetZoom(0.8);
		m_PhotoViewRanking.SetPosition(0, 0);
		m_PhotoViewRanking.SetOption(UIPHOTOVIEWER_CANCONTROL);
	}

	ZeroMemory(&this->CacheDataUserTop, sizeof(&this->CacheDataUserTop));
	memcpy(&this->CacheDataUserTop, lpMsg, sizeof(this->CacheDataUserTop));
	this->CacheDataUserTop.Class = lpMsg->Class;
	//ViewCharItems.ClearSession();
	//rankmodel = rand() % 100;

	//memcpy(&ViewCharItems.charItem.Guild[0], &this->CacheDataUserTop.GuildName[0], sizeof(ViewCharItems.charItem.Guild));
	//memcpy(&ViewCharItems.charItem.Name[0], &this->CacheDataUserTop.NameChar[0], 10);
	//m_PhotoViewRanking.SetClass(gCharacterManager.ChangeServerClassTypeToClientClassType(this->CacheDataUserTop.Class));
	m_PhotoViewRanking.SetClass(gCharacterManager.GetCharacterClassCode(this->CacheDataUserTop.Class));
	//gInterface.DrawMessage(1, "Class (%d)-> %d %d", this->CacheDataUserTop.Class, gCharacterManager.GetCharacterClassCode(this->CacheDataUserTop.Class), Hero->Class);
	this->CacheUserRank = true;
	this->SetItemCharTop = false;
}
DWORD TimeGetTop1 = 0;
bool SelectBarRanking = false;

#endif

void CustomRanking::DrawRankPanelWindow()
{
	if (!gInterface.Data[eRankPANEL_MAIN].OnShow)
	{
		return;
	}
	if (gInterface.CheckWindow(Interface::Inventory)
		|| gInterface.CheckWindow(Interface::Character) //-- 13 & 16
		|| gInterface.CheckWindow(Interface::Warehouse) //-- 8
		|| gInterface.CheckWindow(Interface::Shop) //-- 12
		|| gInterface.CheckWindow(Interface::ChaosBox) //-- 9
		|| gInterface.CheckWindow(Interface::Trade) //-- 7
		|| gInterface.CheckWindow(Interface::Store) //-- 14
		|| gInterface.CheckWindow(Interface::OtherStore) //-- 15
		|| gInterface.CheckWindow(Interface::LuckyCoin1) //-- 60
		|| gInterface.CheckWindow(Interface::NPC_ChaosMix)
		|| gInterface.CheckWindow(Interface::MoveList))
	{
		gInterface.Data[eRankPANEL_MAIN].OnShow = false;
		return;
	}
	float MainWidth = 310.0;
#if(CUSTOM_BRANKINGNEW)
	MainWidth += 160;
#endif

	//if (ViewCharItems.DangXem == 1)
	//{
	//	MainWidth += 160;
	//}


	float MainHeight = 255.0;
	float StartBody = ((MAX_WIN_HEIGHT - 51) / 2) - (MainHeight / 2);
	float StartY = ((MAX_WIN_HEIGHT - 51) / 2) - (MainHeight / 2);
	float StartX = (MAX_WIN_WIDTH / 2) - (MainWidth / 2);
	float MainCenter = StartX + (MainWidth / 3);
	float ButtonX = MainCenter - (float)(29.0 / 2);


	if (g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, MainWidth + 50, MainHeight + 10, eRankPANEL_MAIN, " "))
	{
		DWORD Color = eGray100;

		gInterface.DrawFormat(eExcellent, (int)StartX + MainWidth / 3 - 15, (int)StartY + 10, 210, 3, " Bảng Xếp Hạng %s", this->rankname);


		if (SEASON3B::IsPress(VK_LBUTTON) && SEASON3B::CheckMouseIn(StartX + 258.0, gInterface.Data[eRankPANEL_MAIN].Y + 99, 14, 14))
		{
			PlayBuffer(25, 0, 0);
			gInterface.Data[eRankPANEL_MAIN].Close();
		}
		float SizeButtonW = 80.0;

		if (this->RankEnable == 1)
		{

			gInterface.DrawFormat(eWhite, StartX + 30, StartY + 35, 52, 1, "Top:");

			gInterface.DrawFormat(eWhite, StartX + 80, StartY + 35, 52, 1, "%s", this->col1);

			gInterface.DrawFormat(eWhite, StartX + 145, StartY + 35, 52, 1, "%s", this->col2);

			gInterface.DrawFormat(eWhite, StartX + 245, StartY + 35, 52, 1, "%s", this->col3);


			char* name = (char*)(CharacterAttribute->Name);

			//for (int i = 0; i < this->count; i++)
#if(CUSTOM_BRANKINGNEW)
			for (int n = 0, i = (PageBank * MaxPerPageBank); n < MaxPerPageBank && i < this->count; i++)
#else
			for (int i = 0; i < this->count; i++)
#endif
			{
				Color = eGold;

				if (strcmp(name, this->gCustomRanking[i].szName) == 0)
				{
					Color = eExcellent;
				}
#if(CUSTOM_BRANKINGNEW)
				if (SEASON3B::CheckMouseIn(StartX + 20, StartY + 50 + (18 * n), 315, 15) == 1 /*&& !ViewCharItems.getState()*/)
				{
					if (SEASON3B::IsPress(VK_LBUTTON) && GetTickCount() > gInterface.Data[eRankPANEL_MAIN].EventTick + 300 && SelectBarRanking == 0)
					{
						//gInterface.DrawMessage(1,"%s", this->gCustomRanking[i].szName);
						GetInfoCharTop(this->gCustomRanking[i].szName);
						gInterface.Data[eRankPANEL_MAIN].EventTick = GetTickCount();
					}

					g_pBCustomMenuInfo->DrawInfoBox(StartX + 20, StartY + 51 + (18 * n), 305, 6, 0x26E0A596, 0, 0);
				}
				else
				{
					g_pBCustomMenuInfo->DrawInfoBox(StartX + 20, StartY + 51 + (18 * n), 305, 6, 0x00000096, 0, 0);
				}
#else
				gCItemSetOption.ItemTooltipS15(StartX + 20, StartY + 50 + (18 * n), 245, 15, 0.0, 0);
#endif

				gInterface.DrawFormat(Color, StartX + 30, StartY + 53 + (18 * n), 52, 1, "%dº", (i + 1));
#if(CUSTOM_BRANKINGNEW)

				if (this->gCustomRanking[i].CheckOnline)
				{
					gInterface.DrawBarForm(StartX + 70, StartY + 55 + (18 * n), 5.0, 5.0, 0.0, 1.0, 0.0, 0.75);
				}
				else
				{
					gInterface.DrawBarForm(StartX + 70, StartY + 55 + (18 * n), 5.0, 5.0, 1.0, 0.0, 0.0, 0.75);
				}

				//GetDrawCircle(550000, StartX + 70, StartBody + 55 + (18 * n), 5.0, 0, 0, 0, 1, 1, 0.0);
				//glColor4f(1.0, 1.0, 1.0, 1.0); //Color Raw Online
				if (!this->CacheUserRank)
				{
					GetInfoCharTop(this->gCustomRanking[0].szName);
				}
#endif
				gInterface.DrawFormat(Color, StartX + 80, StartY + 53 + (18 * n), 52, 1, "%s", this->gCustomRanking[i].szName);

				gInterface.DrawFormat(Color, StartX + 145, StartY + 53 + (18 * n), 100, 1, "%s", this->gCustomRanking[i].value2);

				gInterface.DrawFormat(Color, StartX + 235, StartY + 53 + (18 * n), 100, 1, "%s", this->gCustomRanking[i].value3);
#if(CUSTOM_BRANKINGNEW)
				n++;
#endif
			}

#if(CUSTOM_BRANKINGNEW)
			//Next Page
			float CenterX = StartX + (290 / 2);
			float CenterY = (StartY + 270);
			const BYTE state[3] = { 0, 1, 2 };
			RenderBitmap(32344, CenterX - (60 / 2), CenterY - 40, 60.f, 22.f, 0, 0, 80.f / 128.f, 30.f / 34.f, 1, 1, 0.0);

			TextDraw(g_hFont, CenterX - (60 / 2), CenterY - 40 + 5, 0xffffffff, 0x0, 60, 0, 3, "%d / %d", PageBank + 1, (this->count / MaxPerPageBank));

			if (g_pBCustomMenuInfo->DrawButtonGUI(32345, CenterX - (60 / 2) - 26, CenterY - 40, 20, 22, 3))
			{
				BPagePrevAction(NULL); 
			}
			if (g_pBCustomMenuInfo->DrawButtonGUI(32346, CenterX + (60 / 2) + 6, CenterY - 40, 20, 22, 3))
			{
				BPageNextAction(NULL);
			}

			DrawPlayerRank(StartX + 300, StartY, 150, MainHeight);

			//ViewCharItems.DrawCharInfoRanking(StartX + 240, StartY, 150, MainHeight);
			float ListItemX = StartX + 30;
			DWORD SelectBarHoverBG = 0x0;
			float SelectBarY = StartY - 30;
			int SelectBarHover = 0;
			RenderBitmap(31424, ListItemX - 2, SelectBarY + 37, 110, 20, 0.0, 0.0, 0.82, 0.71, 1, 1, 0.0);
			if (SEASON3B::CheckMouseIn(ListItemX - 2, SelectBarY + 37, 110, 20))
			{
				SelectBarHover = 1;
				SelectBarHoverBG = 0x66646450;
				if (GetTickCount() - gInterface.Data[eRankPANEL_MAIN].EventTick > 500) //Click
				{
					if (GetKeyState(VK_LBUTTON) & 0x8000)
					{
						SelectBarHover = 2;
						gInterface.Data[eRankPANEL_MAIN].EventTick = GetTickCount();
						PlayBuffer(25, 0, 0);
						SelectBarRanking ^= 1;
					}
				}
			}
			RenderBitmap(31422, ListItemX + 2, SelectBarY + 39.5, 13.5, 13.5, 0.0, 0.21 * SelectBarHover, 0.91, 0.21, 1, 1, 0.0);
			TextDraw(g_hFontBold, ListItemX + 15, SelectBarY + 41, 0xFFFFFFFF, SelectBarHoverBG, 85, 0, 3, "%s", this->m_RankInfo[this->SelectBarRankingNumber].NameRank);// 
			if (SelectBarRanking)
			{
				int CountSelectBar = 1;
				gInterface.DrawBarForm(ListItemX + 15, SelectBarY + 41 + (15), 85, 15 * 12, 0.0, 0.0, 0.0, 0.8);
				for (int i = 0; i < this->RankingCount; i++)
				{
					if (SelectBarRankingNumber == i) continue;
					DWORD SelectHover = 0x0;
					if (SEASON3B::CheckMouseIn(ListItemX + 15, SelectBarY + 41 + (15 * CountSelectBar), 130, 15))
					{
						SelectHover = 0x66646450;
						if (GetTickCount() - gInterface.Data[eRankPANEL_MAIN].EventTick > 500) //Click
						{
							if (GetKeyState(VK_LBUTTON) & 0x8000)
							{
								//gInterface.DrawMessage(1,"%d ~ %d", this->RankingType,this->RankingCount);
								SelectBarRankingNumber = i;
								gInterface.Data[eRankPANEL_MAIN].EventTick = GetTickCount();
								PlayBuffer(25, 0, 0);
								SelectBarRanking ^= 1;
								this->ClearCustomRanking();
								PageBank = 0;
								PMSG_CUSTOM_RANKING_SEND pMsg;
								pMsg.header.set(0xF3, 0xE6, sizeof(pMsg));
								pMsg.type = i;
								DataSend((BYTE*)& pMsg, pMsg.header.size);
							}
						}
					}
					TextDraw(g_hFontBold, ListItemX + 15, SelectBarY + 41 + (15 * CountSelectBar), 0xFFFFFFFF, SelectHover, 85, 0, 3, "%s", this->m_RankInfo[i].NameRank);// 
					CountSelectBar++;
				}
			}
#endif
		}
		else
		{
			if (this->RankLoad == 1)
			{
				gInterface.DrawFormat(eGold, MainCenter - (sizeof(this->rankname) / 2) + 30, StartY + 120, 52, 1, "Loading ..");
				this->RankLoad = 2;
			}
			else if (this->RankLoad == 2)
			{
				gInterface.DrawFormat(eGold, MainCenter - (sizeof(this->rankname) / 2) + 30, StartY + 120, 52, 1, "Loading ...");
				this->RankLoad = 3;
			}
			else if (this->RankLoad == 3)
			{
				gInterface.DrawFormat(eGold, MainCenter - (sizeof(this->rankname) / 2) + 30, StartY + 120, 52, 1, "Loading ....");
				this->RankLoad = 4;
			}
			else if (this->RankLoad == 4)
			{
				gInterface.DrawFormat(eGold, MainCenter - (sizeof(this->rankname) / 2) + 30, StartY + 120, 52, 1, "Loading .....");
				this->RankLoad = 0;
			}
			else
			{
				gInterface.DrawFormat(eGold, MainCenter - (sizeof(this->rankname) / 2) + 30, StartY + 120, 52, 1, "Loading .");
				this->RankLoad = 1;
			}
		}

	}
}