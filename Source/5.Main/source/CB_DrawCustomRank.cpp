#include "StdAfx.h"
#include "CB_DrawCustomRank.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"
#include "MapManager.h"

CB_DrawCustomRank* gCB_DrawCustomRank;

CB_DrawCustomRank::CB_DrawCustomRank()
{
	this->mDataInfoUserKill.clear();
	this->mDataBXHKiller.clear();
	this->CacheMap = -1;
}

CB_DrawCustomRank::~CB_DrawCustomRank()
{

}

void CB_DrawCustomRank::RecvPacketGS(BYTE* Data)
{
	if (!Data) return;

	PMSG_COUNTLIST* mRecv = (PMSG_COUNTLIST*)Data;
	
	switch (mRecv->TypeSend)
	{
	case eSendBXHKill: 
	{
		this->mDataBXHKiller.clear();
		for (int i = 0; i < mRecv->Count; i++)
		{
			DATA_BXH_KILL lpInfo = *(DATA_BXH_KILL*)(((BYTE*)Data) + sizeof(PMSG_COUNTLIST) + (sizeof(DATA_BXH_KILL) * i));
			this->mDataBXHKiller.push_back(lpInfo);
		}
		if (!this->mDataBXHKiller.empty()) this->CacheMap = gMapManager.WorldActive;
	}
	break;
	case eSendInfoUser:
	{
		this->mDataInfoUserKill.clear();
		for (int i = 0; i < mRecv->Count; i++)
		{
			DATA_INFOUSER_SEND lpInfo = *(DATA_INFOUSER_SEND*)(((BYTE*)Data) + sizeof(PMSG_COUNTLIST) + (sizeof(DATA_INFOUSER_SEND) * i));
			//gInterface.DrawMessage(1, "RecvPacketGS  %d %d %d", mRecv->TypeSend, lpInfo.Kills, lpInfo.Deaths);
			this->mDataInfoUserKill.push_back(lpInfo);
		}
		if (!this->mDataInfoUserKill.empty()) this->CacheMap = gMapManager.WorldActive;
	}
	break;
	default:
		break;
	}

}


void CB_DrawCustomRank::Draw()
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

	if (gMapManager.WorldActive != this->CacheMap)
	{
		this->CacheMap = -1;
		return;
	}

	float InfoW = 100;
	float InfoH = 40;
	float InfoX = MAX_WIN_WIDTH - (InfoW + 20) - 45;
	float InfoY = MAX_WIN_HEIGHT - (InfoH + 60);

	if (g_pBCustomMenuInfo->DrawButton(InfoX - 3, InfoY - 28, 113, 12, "BXH Sự Kiện") && (GetTickCount() - gInterface.Data[eWindowDrawCustomRank].EventTick) > 300)
	{
		////===BXH
		gInterface.Data[eWindowDrawCustomRank].OnShow ^= 1;
	}

	CB_DrawCustomRank::DrawBXH();
	CB_DrawCustomRank::DrawPoint();
}
void CB_DrawCustomRank::DrawBXH()
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
	if (!gInterface.Data[eWindowDrawCustomRank].OnShow)
	{
		return;
	}
	float CuaSoW = 250.0;
	float CuaSoH = 200.0;

	float StartX = (MAX_WIN_WIDTH / 2) - (CuaSoW / 2);
	float StartY = 45.0;

	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, CuaSoW, CuaSoH, eWindowDrawCustomRank, "BXH Event");
	//====================================
	TextDraw(g_hFontBold, StartX, StartY + 40, 0xFABB00FF, 0x0, CuaSoW / 3, 0, 3, "Top"); //
	TextDraw(g_hFontBold, StartX + CuaSoW / 3, StartY + 40, 0xFABB00FF, 0x0, CuaSoW / 3, 0, 3, "Name"); //
	TextDraw(g_hFontBold, StartX + ((CuaSoW / 3) * 2), StartY + 40, 0xFABB00FF, 0x0, CuaSoW / 3, 0, 3, "Kills"); //
	DWORD ColorTextRank = 0xE0DFDCFF;

	for (int i = 0; i < this->mDataBXHKiller.size(); i++)
	{
		if (i % 2)
		{
			ColorTextRank = 0xA3A3A3FF;
		}
		else {
			ColorTextRank = 0xE0DFDCFF;
		}
		if (i < 3) { ColorTextRank = 0xFFE240FF; }
		TextDraw(g_hFontBold, StartX, StartY + 55 + (i * 12), ColorTextRank, (i < 3) ? 0xFFE2400F : 0x0, CuaSoW / 3, 0, 3, "%d", i + 1); //
		TextDraw(g_hFontBold, StartX + CuaSoW / 3, StartY + 55 + (i * 12), ColorTextRank, (i < 3) ? 0xFFE2400F : 0x0, CuaSoW / 3, 0, 3, "%s", this->mDataBXHKiller[i].Name); //
		TextDraw(g_hFontBold, StartX + ((CuaSoW / 3) * 2), StartY + 55 + (i * 12), ColorTextRank, (i < 3) ? 0xFFE2400F : 0x0, CuaSoW / 3, 0, 3, "%d", this->mDataBXHKiller[i].Kills); //

	}
}


void CB_DrawCustomRank::DrawPoint()
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
	if (this->mDataInfoUserKill.empty())
	{
		return;
	}


	float InfoW = 100;
	float InfoH = 40;
	float InfoX = MAX_WIN_WIDTH - (InfoW + 20) - 45;
	float InfoY = MAX_WIN_HEIGHT - (InfoH + 60);


	g_pBCustomMenuInfo->DrawInfoBox(InfoX, InfoY, InfoW, InfoH, 0x00000096, 0, 0);

	//
	if (this->mDataInfoUserKill.empty())
	{
		TextDraw(g_hFontBold, InfoX, InfoY + 13, 0xE6FCF7FF, 0x00FFE564, InfoW + 7, 0, 3, "---"); //score
		TextDraw(g_hFontBold, InfoX, (InfoY + InfoH) - 17, 0xE6FCF7FF, 0xFFBB0064, InfoW + 7, 0, 3, "---");//Death
	}
	else
	{
		TextDraw(g_hFontBold, InfoX, InfoY + 13, 0xE6FCF7FF, 0x00FFE564, InfoW + 7, 0, 3, "Rank: %d", this->mDataInfoUserKill[0].Rank); //Kills
		TextDraw(g_hFontBold, InfoX, (InfoY + InfoH) - 17, 0xE6FCF7FF, 0xFFBB0064, InfoW + 7, 0, 3, "Kills: %d Death: %d", this->mDataInfoUserKill[0].Kills, this->mDataInfoUserKill[0].Deaths);//Death
	}
	return;
}