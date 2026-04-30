#include "StdAfx.h"
#include "CB_ActiveInvasions.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"

#if(CB_ActiveInvasionsD)
CB_ActiveInvasions* gCB_ActiveInvasions;

CB_ActiveInvasions::CB_ActiveInvasions() :  is_update(false)
{
	this->m_data.clear();
}

CB_ActiveInvasions::~CB_ActiveInvasions()
{
}
void CB_ActiveInvasions::list_recv(PMSG_ACTIVE_INVASIONS_RECV* lpMsg)
{

	this->m_data.clear();

	for (int n = 0; n < lpMsg->count; n++)
	{
		auto info = reinterpret_cast<PMSG_ACTIVE_INVASIONS*>(reinterpret_cast<BYTE*>(lpMsg) + sizeof(PMSG_ACTIVE_INVASIONS_RECV) + (sizeof(
			PMSG_ACTIVE_INVASIONS) * n));

		ACTIVE_INVASIONS data;
		data.monster_id = info->monster_id;
		data.count = info->count;
		data.is_update = false;
		data.blink_counter = 0;
		data.blink_type = false;

		this->m_data.push_back(data);
	}

	if (lpMsg->count)
		this->is_update = true;
}
void CB_ActiveInvasions::update_monster_recv(PMSG_INVASION_MONSTER_UPDATE_RECV* lpMsg)
{
	for (auto it = this->m_data.begin(); it != this->m_data.end();)
	{
		if (it->monster_id == lpMsg->monster_id)
		{
			this->is_update = true;
			it->is_update = true;
			it->count._count = lpMsg->count;

			if (it->count._count == 0)
				it = this->m_data.erase(it);

			break;
		}

		++it;
	}
}
extern float NoticeInverse;
int startValue = 0;
void CB_ActiveInvasions::DrawWindow()
{
	if (gInterface.CheckWindow(Interface::MoveList) || gInterface.CheckWindow(Interface::ObjWindow::CashShop) || gInterface.CheckWindow(Interface::ObjWindow::SkillTree) || gInterface.CheckWindow(Interface::ObjWindow::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)))
	{
		gInterface.Data[eActiveInvasionsMain].OnShow = false;
		return;
	}

	float width = 150;
	float height = 20;
	float X = (MAX_WIN_WIDTH/2)-(width/2);
	float Y = 0;
	gInterface.DrawBarForm(X + 30, Y, width - 60, height-4, 0.0, 0.0, 0.0, 0.7);//Background
	if ((int)NoticeInverse % 20 < 5)
	{
		TextDraw(g_hFontBold, X, Y + 3, 0xFFD51CFF, 0x0, width, height, 3, "[Thông Tin Boss]"); //
	}
	else
	{
		TextDraw(g_hFont, X, Y + 3, 0xFFD51CFF, 0x0, width, height, 3, "[Thông Tin Boss]"); //
	}

	if (SEASON3B::CheckMouseIn(X, Y, width, height) == true)
	{
		if (SEASON3B::IsPress(VK_LBUTTON))
		{
			gInterface.Data[eActiveInvasionsMain].OnShow ^= 1;
		}
	}

	if (!gInterface.Data[eActiveInvasionsMain].OnShow)
	{
		return;
	}
	float WindowW = 190;
	float WindowH = 250;
	float StartX = (MAX_WIN_WIDTH - WindowW) - (10);
	float StartY = 0;


	g_pBCustomMenuInfo->DrawWindowCustomMini(&StartX, &StartY, WindowW, WindowH, eActiveInvasionsMain, "Thông Tin BOSS");
	int InfoX = StartX + 5;
	int InfoY = StartY + 10;
	int HText = 12;
	int CountText = 0;
	//TextDraw((HFONT)g_hFont, InfoX, InfoY + (HText * CountText++), 0xFFC421FF, 0x0, WindowW - 30, 0, 1, "TTTTTTTTTTTT/BVBBBBBB");

	//int startValue = ((this->m_data.size() - 15.0) / 100.0 * static_cast<double>(this->scroll_value + 1));
	

	for (int i = startValue, counter = 0; counter < 12; i++)
	{
		if (i >= this->m_data.size())
		{
			break;
		}

		std::string Text;

		Text = getMonsterName(this->m_data[i].monster_id);

		std::string count = std::to_string(static_cast<long long>(this->m_data[i].count._count)) + "/" + std::to_string(static_cast<long long>(this->m_data[i].count._max_count));

		DWORD Color = 0xFFD51CFF;

		if (this->m_data[i].is_update)
		{
			if (this->m_data[i].blink_type)
				Color = 0xBD7AFFFF;

			//if (this->draw_count >= 20)
			//{
			//	this->m_data[i].blink_type = !this->draw_type;
			//}
		}

		TextDraw(g_hFontBold, InfoX + 10, InfoY + 18 + (counter * 15), Color, 0x0, 0, 0, 1, "%s", Text.c_str());
		TextDraw(g_hFontBold, InfoX + WindowW - 15, InfoY + 18 + (counter * 15), Color, 0x0, 0, 0, 7, "%s", count.c_str());
		//gInterface.DrawBarForm(InfoX + 5, InfoY + 29 + (counter * 15), WindowW - 7, 2, 0.0, 0.0, 0.0, 1.0);

		if (SEASON3B::CheckMouseIn(InfoX + 10, InfoY + 18 + (counter * 15), InfoX + 10 + 80, InfoY + 18 + (counter * 15) + 15))
		{
			this->m_data[i].is_update = false;
		}

		counter++;
	}

	if (startValue > 0)
	{
		if (g_pBCustomMenuInfo->DrawButtonGUI(CNewUIGuildInfoWindow::IMAGE_GUILDINFO_TAB_BUTTON, StartX + 40, StartY + (WindowH - 40), 20, 23, 3))
		{
			startValue--;
		}
	}
	if ((this->m_data.size() / 12) > startValue)
	{
		if (g_pBCustomMenuInfo->DrawButtonGUI(CNewUIGuildInfoWindow::IMAGE_GUILDINFO_TAB_BUTTON, StartX + (WindowW - 65), StartY + (WindowH - 40), 20, 23, 3))
		{
			startValue++;
	
		}
	}
}
#endif
