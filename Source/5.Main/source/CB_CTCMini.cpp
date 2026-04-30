#include "StdAfx.h"
#include "CB_CTCMini.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "./Time/CTimCheck.h"
#include "MapManager.h"
//===CTC Mini
BYTE CheckVaoLanhDia = 0;
BYTE m_ShowAnh = 0;
DWORD	 CTCMINI_TimeCTCMini = 0;
BYTE CTCMINI_Cong[3] = { 0,0,0 };
BYTE CTCMINI_Tru[3] = { 0,0,0 };
char CTCMINI_GNameTru1[9] = { 0 };
char CTCMINI_GNameTru2[9] = { 0 };
char CTCMINI_GNameTru3[9] = { 0 };
extern float fAlpha;
CBCTCMini gCBCTCMini;

CBCTCMini::CBCTCMini()
{

}

CBCTCMini::~CBCTCMini()
{

}
void CBCTCMini::OpenOnOff()
{
	if (GetTickCount() - gInterface.Data[eCTCMiniWindow].EventTick > 300)
	{
		gInterface.Data[eCTCMiniWindow].EventTick = GetTickCount();
		if (gInterface.Data[eCTCMiniWindow].OnShow)
		{
			gInterface.Data[eCTCMiniWindow].OnShow = 0;
			return;
		}
		//==Show hoac send packet open
		gInterface.Data[eCTCMiniWindow].OnShow = 1;


	}
}
void CGMoveCTCMini(int Type)
{
	if (Type > 0)
	{
		CBCTCMini::CTCMINI_CGPACKET pMsg;
		pMsg.header.set(0xF3, 0x39, sizeof(pMsg));
		pMsg.CongVao = Type - 1;
		DataSend((LPBYTE)& pMsg, pMsg.header.size);
	}
}
void CBCTCMini::LoadImg()
{

	int Number = 0;
	//pLoadImage("Custom\\CTCMini\\UiCtc.tga", 32584, 9729, 10496, 1, 0);
	//pLoadImage("Custom\\CTCMini\\CTCMiniMap.tga", 32583, 9729, 10496, 1, 0);
	//pLoadImage("Custom\\CTCMini\\StartCTC.tga", 32585, 9729, 10496, 1, 0);
	//pLoadImage("Custom\\CTCMini\\WinCTC.tga", 32586, 9729, 10496, 1, 0);
	//pLoadImage("Custom\\CTCMini\\LoseCTC.tga", 32589, 9729, 10496, 1, 0); //Trung voi ID effect 32587
	//pLoadImage("Custom\\CTCMini\\DialogCTC.tga", 32588, 9729, 10496, 1, 0);
	//pLoadImage("Custom\\CTCMini\\CharPoint.tga", 32506, 9729, 10496, 1, 0);
	//pLoadImage("Custom\\CTCMini\\OtherCharPoint.tga", 32507, 9729, 10496, 1, 0);

	g_pBCustomMenuInfo->BindObject(eUICTCImg+ Number++, "Custom\\CTCMini\\UiCtc.tga"); //0
	g_pBCustomMenuInfo->BindObject(eUICTCImg+ Number++, "Custom\\CTCMini\\CTCMiniMap.tga");//1
	g_pBCustomMenuInfo->BindObject(eUICTCImg+ Number++, "Custom\\CTCMini\\StartCTC.tga");//2
	g_pBCustomMenuInfo->BindObject(eUICTCImg+ Number++, "Custom\\CTCMini\\WinCTC.tga");//3
	g_pBCustomMenuInfo->BindObject(eUICTCImg+ Number++, "Custom\\CTCMini\\LoseCTC.tga");//4
	g_pBCustomMenuInfo->BindObject(eUICTCImg+ Number++, "Custom\\CTCMini\\DialogCTC.tga");//5
	g_pBCustomMenuInfo->BindObject(eUICTCImg+ Number++, "Custom\\CTCMini\\CharPoint.tga", 4, 4, -1, -1);//6
	g_pBCustomMenuInfo->BindObject(eUICTCImg+ Number++, "Custom\\CTCMini\\OtherCharPoint.tga", 4, 4, -1, -1);//7
}
void CBCTCMini::GCCTCMiniShowWindow(BYTE* Recv) // OK
{
	if (!Recv) return;
	INFOCTCMINI_SENDCLIENT* lpMsg = (INFOCTCMINI_SENDCLIENT*)Recv;
	if ((GetTickCount() - gInterface.Data[eCTCMiniWindow].EventTick) > 300)
	{
		gInterface.Data[eCTCMiniWindow].EventTick = GetTickCount();
		gInterface.Data[eCTCMiniWindow].OnShow ^= 1;
	}
	CheckVaoLanhDia = lpMsg->GuildWinOld;
	//lpMsg->OpenWindow;
}
void CBCTCMini::GCCTCMiniKill(BYTE* Recv) // OK
{
	if (!Recv) return;
	CTCMINIKILL_SENDCLIENT* lpMsg = (CTCMINIKILL_SENDCLIENT*)Recv;
	CTCMINI_TimeCTCMini = lpMsg->TimeCTCMini;
	for (int n = 0; n < 3; n++) //Them Cong
	{
		CTCMINI_Cong[n] = lpMsg->Cong[n];
		CTCMINI_Tru[n] = lpMsg->Tru[n];
	}
	m_ShowAnh = lpMsg->ShowAnh;

}
void CBCTCMini::DrawWindow()
{
	DrawCTCMiNiMap();
	if (gInterface.CheckWindow(Interface::Interface::MoveList) || gInterface.CheckWindow(Interface::Interface::CashShop) || gInterface.CheckWindow(Interface::Interface::SkillTree) || gInterface.CheckWindow(Interface::Interface::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)))
	{
		gInterface.Data[eCTCMiniWindow].OnShow = false;
		return;
	}

	if (!gInterface.Data[eCTCMiniWindow].OnShow)
	{
		return;
	}
	float CuaSoW = 250.0;
	float CuaSoH = 250.0;

	float StartX = (MAX_WIN_WIDTH / 2) - (CuaSoW / 2);
	float StartY = 45.0;

	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, CuaSoW, CuaSoH, eCTCMiniWindow, "CTC Mini");


	//RenderImage(71520, StartX + (CuaSoW / 2) - (200 / 2), StartY + 30, 200, 1); //-- Divisor
	//====================================
	StartY = StartY + 45;
	TextDraw((HFONT)g_hFont, StartX + (CuaSoW / 10), StartY, 0xFFDE26FF, 0x0, CuaSoW, 0, 1, gOther.Text_Event[1]); //
	StartY = StartY + 12;
	TextDraw((HFONT)g_hFont, StartX + (CuaSoW / 10), StartY, 0xFFDE26FF, 0x0, CuaSoW, 0, 1, gOther.Text_Event[2]); //
	StartY = StartY + 12;
	TextDraw((HFONT)g_hFont, StartX + (CuaSoW / 10), StartY, 0xFFDE26FF, 0x0, CuaSoW, 0, 1, gOther.Text_Event[3]); //
	StartY = StartY + 12;
	TextDraw((HFONT)g_hFont, StartX + (CuaSoW / 10), StartY, 0x00FFCCFF, 0x0, CuaSoW, 0, 1, gOther.Text_Event[4]); //
	StartY = StartY + 12;
	TextDraw((HFONT)g_hFont, StartX + (CuaSoW / 10), StartY, 0x00FF90FF, 0x0, CuaSoW, 0, 1, gOther.Text_Event[5]); //
	StartY = StartY + 25;
	EnableAlphaTest(true);
	glColor3f(1.0, 1.0, 1.0);
	RenderImage(gInterface.Data[eUICTCImg+1].ModelID, StartX, StartY, 128.0f, 128.0f); //Huong Dan CTC
	glColor3f(1.0f, 1.0f, 1.0f); //Xoa mau
	EnableAlphaTest(0);
	//============Button
	StartY = StartY + 10;
	float SizeButtonW = 95.0;
	float SizeButtonH = 19.0;
	StartX = StartX + (CuaSoW / 2);
	//=====================Button 1
	if (SEASON3B::CheckMouseIn(StartX, StartY, SizeButtonW, SizeButtonH) == 1)
	{
		glColor3f(1.0f, 0.6742f, 0.15f);
		if (GetTickCount() - gInterface.Data[eCTCMiniWindow].EventTick > 300) //Click
		{
			if (GetKeyState(1) & 0x8000)
			{

				glColor3f(0.72f, 0.438f, 0.0432f);
				CGMoveCTCMini(1);
				gInterface.Data[eCTCMiniWindow].OnShow = 0;
				gInterface.Data[eCTCMiniWindow].EventTick = GetTickCount();
				//pDrawMessage ( "Button 2", 0 );
			}
		}
	}
	else
	{
		glColor3f(1.0f, 1.0f, 1.0f);

	}
	RenderBitmap(CNewUITrade::IMAGE_TRADE_NICK_BACK, StartX, StartY, SizeButtonW, SizeButtonH, 0.0, 0.0, 0.64999998, 0.80000001, 1, 1, 0);
	TextDraw((HFONT)g_hFont, StartX, StartY + (SizeButtonH / 4), 0xE6FCF7FF, 0x0, SizeButtonW, 0, 3, gOther.Text_Event[6]); //Buoc Vao
	//=====================================
	StartY = StartY + 22;
	//=====================Button 2
	if (SEASON3B::CheckMouseIn(StartX, StartY, SizeButtonW, SizeButtonH) == 1)
	{
		glColor3f(1.0f, 0.6742f, 0.15f);
		if (GetTickCount() - gInterface.Data[eCTCMiniWindow].EventTick > 300) //Click
		{
			if (GetKeyState(1) & 0x8000)
			{

				glColor3f(0.72f, 0.438f, 0.0432f);


				CGMoveCTCMini(2);
				gInterface.Data[eCTCMiniWindow].OnShow = 0;
				gInterface.Data[eCTCMiniWindow].EventTick = GetTickCount();
				//pDrawMessage ( "Button 2", 0 );
			}
		}
	}
	else {
		glColor3f(1.0f, 1.0f, 1.0f);

	}
	RenderBitmap(CNewUITrade::IMAGE_TRADE_NICK_BACK, StartX, StartY, SizeButtonW, SizeButtonH, 0.0, 0.0, 0.64999998, 0.80000001, 1, 1, 0);
	TextDraw((HFONT)g_hFont, StartX, StartY + (SizeButtonH / 4), 0xE6FCF7FF, 0x0, SizeButtonW, 0, 3, gOther.Text_Event[7]); //Buoc Vao
	//=====================================
	StartY = StartY + 22;
	//=====================Button 3
	if (SEASON3B::CheckMouseIn(StartX, StartY, SizeButtonW, SizeButtonH) == 1)
	{
		glColor3f(1.0f, 0.6742f, 0.15f);
		if (GetTickCount() - gInterface.Data[eCTCMiniWindow].EventTick > 300) //Click
		{
			if (GetKeyState(1) & 0x8000)
			{

				glColor3f(0.72f, 0.438f, 0.0432f);


				CGMoveCTCMini(3);
				gInterface.Data[eCTCMiniWindow].OnShow = 0;
				gInterface.Data[eCTCMiniWindow].EventTick = GetTickCount();
				//pDrawMessage ( "Button 2", 0 );
			}
		}
	}
	else {
		glColor3f(1.0f, 1.0f, 1.0f);

	}
	RenderBitmap(CNewUITrade::IMAGE_TRADE_NICK_BACK, StartX, StartY, SizeButtonW, SizeButtonH, 0.0, 0.0, 0.64999998, 0.80000001, 1, 1, 0);
	TextDraw((HFONT)g_hFont, StartX, StartY + (SizeButtonH / 4), 0xE6FCF7FF, 0x0, SizeButtonW, 0, 3, gOther.Text_Event[8]); //Buoc Vao
	//=====================================
	StartY = StartY + 22;
	//=====================Button 4
	if (SEASON3B::CheckMouseIn(StartX, StartY, SizeButtonW, SizeButtonH) == 1)
	{
		glColor3f(1.0f, 0.6742f, 0.15f);
		if (GetTickCount() - gInterface.Data[eCTCMiniWindow].EventTick > 300) //Click
		{
			if (GetKeyState(1) & 0x8000)
			{

				glColor3f(0.72f, 0.438f, 0.0432f);


				CGMoveCTCMini(4);
				gInterface.Data[eCTCMiniWindow].OnShow = 0;
				gInterface.Data[eCTCMiniWindow].EventTick = GetTickCount();
				//pDrawMessage ( "Button 2", 0 );
			}
		}
	}
	else {
		glColor3f(1.0f, 1.0f, 1.0f);

	}
	RenderBitmap(CNewUITrade::IMAGE_TRADE_NICK_BACK, StartX, StartY, SizeButtonW, SizeButtonH, 0.0, 0.0, 0.64999998, 0.80000001, 1, 1, 0);
	TextDraw((HFONT)g_hFont, StartX, StartY + (SizeButtonH / 4), 0xE6FCF7FF, 0x0, SizeButtonW, 0, 3, gOther.Text_Event[9]); //Buoc Vao
		//=====================================
	StartY = StartY + 22;
	//=====================Button Lanh Dia
	if (SEASON3B::CheckMouseIn(StartX, StartY, SizeButtonW, SizeButtonH) == 1 && CheckVaoLanhDia)
	{
		glColor3f(1.0f, 0.6742f, 0.15f);
		if (GetTickCount() - gInterface.Data[eCTCMiniWindow].EventTick > 300) //Click
		{
			if (GetKeyState(1) & 0x8000)
			{

				glColor3f(0.72f, 0.438f, 0.0432f);


				CGMoveCTCMini(5);
				gInterface.Data[eCTCMiniWindow].OnShow = 0;
				gInterface.Data[eCTCMiniWindow].EventTick = GetTickCount();
				//pDrawMessage ( "Button 2", 0 );
			}
		}
	}
	else if (!CheckVaoLanhDia)
	{
		glColor3f(0.42f, 0.42f, 0.42f);
	}
	else
	{
		glColor3f(1.0f, 1.0f, 1.0f);

	}
	RenderBitmap(CNewUITrade::IMAGE_TRADE_NICK_BACK, StartX, StartY, SizeButtonW, SizeButtonH, 0.0, 0.0, 0.64999998, 0.80000001, 1, 1, 0);
	TextDraw((HFONT)g_hFont, StartX, StartY + (SizeButtonH / 4), 0xE6FCF7FF, 0x0, SizeButtonW, 0, 3, gOther.Text_Event[10]); //Buoc Vao
	//=====================================
	//=====================================
	glColor3f(1.0f, 1.0f, 1.0f); //Xoa mau
}

void CTCImgSlow()
{
	float fPosX, fPosY, fWidth, fHeight, tu, tv;
	fWidth = 372.0f;
	fHeight = 99.0f;
	fPosX = (MAX_WIN_WIDTH - fWidth) / 2.0f;
	fPosY = (480.0f / 2) - 99.0f;
	tu = fWidth / 512.f;
	tv = fHeight / 128.f;

	if (fAlpha <= 1.0f)
		fAlpha += 0.01f;
	else if (fAlpha >= 0.99f && g_Time.GetTimeCheck(3, 2000))
	{
		fAlpha = 0.1f;
		m_ShowAnh = 0;
	}
	EnableAlphaTest(true);
	glColor3f(1.0, 1.0, 1.0);
	RenderBitmap(gInterface.Data[eUICTCImg + 2].ModelID, fPosX, fPosY, fWidth, fHeight, 0.f, 0.f, tu, tv, true, true, fAlpha);
	glColor3f(1.0f, 1.0f, 1.0f); //Xoa mau
	EnableAlphaTest(0);
}

void CBCTCMini::DrawCTCMiNiMap()
{

	if (gInterface.CheckWindow(Interface::CashShop)
		|| gInterface.CheckWindow(Interface::FriendList)
		|| gInterface.CheckWindow(Interface::MoveList)
		|| gInterface.CheckWindow(Interface::Party)
		|| gInterface.CheckWindow(Interface::Quest)
		|| gInterface.CheckWindow(Interface::NPC_Devin)
		|| gInterface.CheckWindow(Interface::Guild)
		|| gInterface.CheckWindow(Interface::Trade)
		|| gInterface.CheckWindow(Interface::Warehouse)
		|| gInterface.CheckWindow(Interface::ChaosBox)
		|| gInterface.CheckWindow(Interface::CommandWindow)
		|| gInterface.CheckWindow(Interface::PetInfo)
		|| gInterface.CheckWindow(Interface::Shop)
		|| gInterface.CheckWindow(Interface::Inventory)
		|| gInterface.CheckWindow(Interface::Store)
		|| gInterface.CheckWindow(Interface::OtherStore)
		|| gInterface.CheckWindow(Interface::Character)
		|| gInterface.CheckWindow(Interface::DevilSquare)
		|| gInterface.CheckWindow(Interface::BloodCastle)
		|| gInterface.CheckWindow(Interface::CreateGuild)
		|| gInterface.CheckWindow(Interface::GuardNPC)
		|| gInterface.CheckWindow(Interface::SeniorNPC)
		|| gInterface.CheckWindow(Interface::GuardNPC2)
		|| gInterface.CheckWindow(Interface::CastleGateSwitch)
		|| gInterface.CheckWindow(Interface::CatapultNPC)
		|| gInterface.CheckWindow(Interface::CrywolfGate)
		|| gInterface.CheckWindow(Interface::IllusionTemple)
		|| gInterface.CheckWindow(Interface::HeroList)
		|| gInterface.CheckWindow(Interface::ChatWindow)
		|| gInterface.CheckWindow(Interface::FastMenu)
		|| gInterface.CheckWindow(Interface::Options)
		|| gInterface.CheckWindow(Interface::Help)
		|| gInterface.CheckWindow(Interface::FastDial)
		|| gInterface.CheckWindow(Interface::SkillTree)
		|| gInterface.CheckWindow(Interface::GoldenArcher1)
		|| gInterface.CheckWindow(Interface::GoldenArcher2)
		|| gInterface.CheckWindow(Interface::LuckyCoin1)
		|| gInterface.CheckWindow(Interface::LuckyCoin2)
		|| gInterface.CheckWindow(Interface::NPC_Duel)
		|| gInterface.CheckWindow(Interface::NPC_Titus)
		|| gInterface.CheckWindow(Interface::CashShop)
		|| gInterface.CheckWindow(Interface::Lugard)
		|| gInterface.CheckWindow(Interface::QuestList1)
		|| gInterface.CheckWindow(Interface::QuestList2)
		|| gInterface.CheckWindow(Interface::Jerint)
		|| gInterface.CheckWindow(Interface::FullMap)
		|| gInterface.CheckWindow(Interface::NPC_Dialog)
		|| gInterface.CheckWindow(Interface::GensInfo)
		|| gInterface.CheckWindow(Interface::NPC_Julia)
		|| gInterface.CheckWindow(Interface::NPC_ChaosMix)
		|| gInterface.CheckWindow(Interface::ExpandInventory)
		|| gInterface.CheckWindow(Interface::ExpandWarehouse)
		|| gInterface.CheckWindow(Interface::MuHelper))
	{
		return;
	}
	if (gMapManager.WorldActive != 98)
	{
		return;
	}
	float MainWidth = 138.0;
	float MainHeight = 265.0;
	float StartY = 264.0;
	float StartX = 512.0; //512
	float Y = 294;
	EnableAlphaTest(true);
	glColor3f(1.0, 1.0, 1.0);
	RenderImage(gInterface.Data[eUICTCImg].ModelID, StartX, StartY, 128.0f, 157.0f); //UI
	RenderImage(gInterface.Data[eUICTCImg+1].ModelID, StartX, StartY + 28.0, 128.0f, 128.0f); //MAP
	if (CTCMINI_TimeCTCMini > 0)
	{
		gInterface.BRenderTimeCountDown(MAX_WIN_WIDTH - 98, 274, 70, CTCMINI_TimeCTCMini);
	}


	//== Cong Vao 1
	if (CTCMINI_Cong[0] == 0) {
		RenderBitmap(CNewUICursedTempleSystem::IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_PC, (float)(StartX - 3 + 96 / 2), (float)(Y - 5 + (255 - 130) / 2), 7.0, 7.0, 0.0, 0.0, 0.875, 0.875, 1, 1, 0.0);
	}
	//== Cong Vao 2
	if (CTCMINI_Cong[1] == 0) {
		RenderBitmap(CNewUICursedTempleSystem::IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_PC, (float)(StartX - 3 + 128 / 2), (float)(Y - 5 + (255 - 130) / 2), 7.0, 7.0, 0.0, 0.0, 0.875, 0.875, 1, 1, 0.0);
	}
	//== Cong Vao 3
	if (CTCMINI_Cong[2] == 0) {
		RenderBitmap(CNewUICursedTempleSystem::IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_PC, (float)(StartX - 2.5 + 161 / 2), (float)(Y - 5 + (255 - 130) / 2), 7.0, 7.0, 0.0, 0.0, 0.875, 0.875, 1, 1, 0.0);
	}

	//Tru Chua Cong 1 //CNewUICursedTempleSystem::IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_HOLYITEM Thuoc Ve Guild
	if (CTCMINI_Tru[0] == 0)
	{
		RenderBitmap(CNewUICursedTempleSystem::IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_HOLYITEM, (float)(StartX - 4.5 + 96 / 2), (float)(Y - 6 + (255 - 165) / 2), 9.0, 8.0, 0.0, 0.0, 0.5625, 1.0, 1, 1, 0.0);
	}
	else
	{
		RenderBitmap(CNewUICursedTempleSystem::IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_HOLYITEM, (float)(StartX - 4.5 + 96 / 2), (float)(Y - 6 + (255 - 165) / 2), 9.0, 8.0, 0.0, 0.0, 0.5625, 1.0, 1, 1, 0.0);
	}
	//Tru Chua Cong 2
	if (CTCMINI_Tru[1] == 0)
	{
		RenderBitmap(CNewUICursedTempleSystem::IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_HOLYITEM, (float)(StartX - 4.5 + 128 / 2), (float)(Y - 6 + (255 - 165) / 2), 9.0, 8.0, 0.0, 0.0, 0.5625, 1.0, 1, 1, 0.0);
	}
	else
	{
		RenderBitmap(CNewUICursedTempleSystem::IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_HOLYITEM, (float)(StartX - 4.5 + 128 / 2), (float)(Y - 6 + (255 - 165) / 2), 9.0, 8.0, 0.0, 0.0, 0.5625, 1.0, 1, 1, 0.0);
	}
	//Tru Chua Cong 3
	if (CTCMINI_Tru[2] == 0)
	{
		RenderBitmap(CNewUICursedTempleSystem::IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ILLUSION_HOLYITEM, (float)(StartX - 4.5 + 161 / 2), (float)(Y - 6 + (255 - 165) / 2), 9.0, 8.0, 0.0, 0.0, 0.5625, 1.0, 1, 1, 0.0);
	}
	else
	{
		RenderBitmap(CNewUICursedTempleSystem::IMAGE_CURSEDTEMPLESYSTEM_MINIMAPICON_ALLIED_HOLYITEM, (float)(StartX - 4.5 + 161 / 2), (float)(Y - 6 + (255 - 165) / 2), 9.0, 8.0, 0.0, 0.0, 0.5625, 1.0, 1, 1, 0.0);
	}
	//RenderImage(32585, 640/2-(210/2), 480/2-(94/2),210,94);
	if ((GetTickCount() - gInterface.Data[eCTCMiniTime].EventTick) > 1000)
	{
		if (CTCMINI_TimeCTCMini > 0)
		{
			CTCMINI_TimeCTCMini = CTCMINI_TimeCTCMini - 1;
		}
		gInterface.Data[eCTCMiniTime].EventTick = GetTickCount();
	}

	int hours = CTCMINI_TimeCTCMini / 3600;
	int minutes = (CTCMINI_TimeCTCMini / 60) % 60;
	int seconds = CTCMINI_TimeCTCMini % 60;

	if (m_ShowAnh == 2)
	{
		CTCImgSlow();
	}
	int Model;
	glColor3f(1.0f, 1.0f, 1.0f); //Xoa mau
	EnableAlphaTest(0);
	for (int i = 0; i < 400; i++)
	{
		CHARACTER* c = &CharactersClient[i]; //SelectedCharacter
		OBJECT* o = &c->Object;
		if (!c || !o->Live) continue;
		if (o->Kind == KIND_PLAYER)
		{
			if (c!= Hero)
			{
				//g_pBCustomMenuInfo->DrawGUI(eUICTCImg + 7,150, 150);
				g_pBCustomMenuInfo->DrawGUI(eUICTCImg + 7, (float)(StartX - 1 + c->PositionX / 2), (float)(Y - 1 + (255 - c->PositionY) / 2));
			}
			else
			{
				g_pBCustomMenuInfo->DrawGUI(eUICTCImg + 6, (float)(StartX - 1 + c->PositionX / 2), (float)(Y - 1 + (255 - c->PositionY) / 2));
			}

		}
	}

}
void CBCTCMini::RecvProtocol(BYTE* Recv)
{

}