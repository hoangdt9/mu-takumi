// NewUIBuffWindow.cpp: implementation of the CNewUIBuffWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIBuffWindow.h"
#include "ZzzBMD.h"
#include "ZzzCharacter.h"
#include "ZzzTexture.h"
#include "ZzzInventory.h"
#include "UIControls.h"
#include "NewUICommonMessageBox.h"
#include "CBInterface.h"


using namespace SEASON3B;

namespace
{
	const float BUFF_IMG_WIDTH = 15.0f;
	const float BUFF_IMG_HEIGHT = 19.0f;
	const int BUFF_MAX_LINE_COUNT = 8;
	const int BUFF_IMG_SPACE = 5;
};

SEASON3B::CNewUIBuffWindow::CNewUIBuffWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
}

SEASON3B::CNewUIBuffWindow::~CNewUIBuffWindow()
{
	Release();
}

bool SEASON3B::CNewUIBuffWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_BUFF_WINDOW, this );
	
	SetPos(x, y);

	LoadImages();
	
	Show( true );

	return true;
}

void SEASON3B::CNewUIBuffWindow::Release()
{
	UnloadImages();
	
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

void SEASON3B::CNewUIBuffWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

void SEASON3B::CNewUIBuffWindow::SetPos(int iScreenWidth)
{
	float FixX = 60;
	float FixY = - 4; // Tọa Độ
	if(iScreenWidth	== 640) {
		SetPos(220+ FixX, 15- FixY);
	}
	else if(iScreenWidth == 450) {
		SetPos(125+ FixX, 15- FixY);
	}
	else if(iScreenWidth == 373) {
		SetPos(86+ FixX, 15- FixY);
	}
	else if(iScreenWidth == 260) {
		SetPos(30+ FixX, 15- FixY);
	}
	else {
		SetPos(220+ FixX, 15- FixY);
	}
}

void SEASON3B::CNewUIBuffWindow::BuffSort(std::list<eBuffState>& buffstate )
{
	OBJECT* pHeroObject = &Hero->Object;

	int iBuffSize = g_CharacterBuffSize( pHeroObject );
	
	for(int i=0; i<iBuffSize; ++i) 
	{
		eBuffState eBuffType = g_CharacterBuff(pHeroObject, i);

		if(SetDisableRenderBuff(eBuffType))	continue;

		if(eBuffType != eBuffNone) {
			eBuffClass eBuffClassType = g_IsBuffClass(eBuffType);
			
			if(eBuffClassType == eBuffClass_Buff) {
				buffstate.push_front( eBuffType );
			}
			else if(eBuffClassType == eBuffClass_DeBuff) {
				buffstate.push_back( eBuffType );
			}
			else {
				assert(!"SetDisableRenderBuff");
			}
		}
	}	
}

bool SEASON3B::CNewUIBuffWindow::SetDisableRenderBuff(const eBuffState& _BuffState)
{
	switch(_BuffState)
	{
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
	case eDeBuff_MoveCommandWin:
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
	case eDeBuff_FlameStrikeDamage:
	case eDeBuff_GiganticStormDamage:
	case eDeBuff_LightningShockDamage:
	case eDeBuff_Discharge_Stamina:
		return true;
	default:
		return false;
	}
	return false;
}

bool SEASON3B::CNewUIBuffWindow::UpdateMouseEvent()
{
	float x = 0.0f, y = 0.0f;
	int buffwidthcount = 0, buffheightcount = 0;

	std::list<eBuffState> buffstate;
	BuffSort( buffstate );

	std::list<eBuffState>::iterator iter;
	for( iter = buffstate.begin(); iter != buffstate.end(); )
	{
		std::list<eBuffState>::iterator tempiter = iter;
		++iter;
		eBuffState buff = (*tempiter);

		x = m_Pos.x + (buffwidthcount * (BUFF_IMG_WIDTH+BUFF_IMG_SPACE));
		y = m_Pos.y + (buffheightcount * (BUFF_IMG_HEIGHT+BUFF_IMG_SPACE)) ;

		if( SEASON3B::CheckMouseIn(x, y, BUFF_IMG_WIDTH, BUFF_IMG_HEIGHT) )
		{
			if(
				buff == eBuff_InfinityArrow ||
				buff == EFFECT_INFINITY_ARROW_IMPROVED
				)
			{
				if(SEASON3B::IsRelease(VK_RBUTTON))
				{
					SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CInfinityArrowCancelMsgBoxLayout));
				}
			}
			else if(
				buff == eBuff_SwellOfMagicPower||
				buff == EFFECT_MAGIC_CIRCLE_IMPROVED ||
				buff == EFFECT_MAGIC_CIRCLE_ENHANCED
				)
			{
				if(SEASON3B::IsRelease(VK_RBUTTON))
				{
					SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CBuffSwellOfMPCancelMsgBoxLayOut));
				}
			}
			
			return false;
		}

		if( ++buffwidthcount >= BUFF_MAX_LINE_COUNT ) {
			buffwidthcount = 0;
			++buffheightcount;
		}
	}
	return true;
}

bool SEASON3B::CNewUIBuffWindow::UpdateKeyEvent()
{
	return true;
}

bool SEASON3B::CNewUIBuffWindow::Update()
{
	return true;
}

bool SEASON3B::CNewUIBuffWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);

	RenderBuffStatus( BUFF_RENDER_ICON );

	RenderBuffStatus( BUFF_RENDER_TOOLTIP );
		
	DisableAlphaBlend();

	return true;
}
//== Render Icon Buff
#include "BuffIcon.h"
void SEASON3B::CNewUIBuffWindow::RenderBuffStatus( BUFF_RENDER renderstate ) //DAT
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
	OBJECT* pHeroObject = &Hero->Object;

	float x = 0.0f, y = 0.0f;
	int buffwidthcount = 0, buffheightcount = 0;
	
	std::list<eBuffState> buffstate;
	BuffSort( buffstate );

	std::list<eBuffState>::iterator iter;
	int IndexTime = 0;
	for( iter = buffstate.begin(); iter != buffstate.end(); )
	{
		std::list<eBuffState>::iterator tempiter = iter;
		++iter;
		eBuffState buff = (*tempiter);
		
		x = m_Pos.x + (buffwidthcount * (BUFF_IMG_WIDTH+BUFF_IMG_SPACE));
		y = m_Pos.y + (buffheightcount * (BUFF_IMG_HEIGHT+BUFF_IMG_SPACE)) ;
		
		if( renderstate == BUFF_RENDER_ICON ) 
		{
			RenderBuffIcon(buff, x, y, BUFF_IMG_WIDTH, BUFF_IMG_HEIGHT);
//#ifdef DEBUG
//			int iBuffReferenceCount = g_CharacterBuffCount( pHeroObject, buff );
//			RenderNumber(x+5, y+5, buff, 1.f);
//#endif //_DEBUG
			//===Render Bar Time
			if (gIconBuff.UserBuff[IndexTime].Index != NULL)
			{
				if (gIconBuff.UserBuff[IndexTime].Index != 28)
				{
					int pBuffHp = 100;

					if (gIconBuff.UserBuff[IndexTime].MaxTime > 0 && gIconBuff.UserBuff[IndexTime].Time > 0)
					{
						pBuffHp = (gIconBuff.UserBuff[IndexTime].Time * 100) / gIconBuff.UserBuff[IndexTime].MaxTime;
					}
					EnableAlphaTest();
					glColor4f(0.0, 1.0, 0.0, 1.0);
					RenderColor(x, (y + BUFF_IMG_HEIGHT), (BUFF_IMG_WIDTH * pBuffHp / 100.0f), 2.0f, 0.0f, 0);
					::EnableAlphaTest();
					::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				}
			}
		}
		else if( renderstate == BUFF_RENDER_TOOLTIP )
		{
			// ¹öÇÁ ÅøÆÁ ·»´õ¸µ
			if(SEASON3B::CheckMouseIn(x, y, BUFF_IMG_WIDTH, BUFF_IMG_HEIGHT)) 
			{
				float fTooltip_x = x + (BUFF_IMG_WIDTH / 2);
				float fTooltip_y = y + BUFF_IMG_WIDTH;
				eBuffClass buffclass = g_IsBuffClass(buff);
				RenderBuffTooltip( buffclass, buff, fTooltip_x, fTooltip_y , IndexTime);
			}
		}
		IndexTime++;
		if( ++buffwidthcount >= BUFF_MAX_LINE_COUNT ) {
			buffwidthcount = 0;
			++buffheightcount;
		}
	}
}	

void SEASON3B::CNewUIBuffWindow::RenderBuffIcon(eBuffState& eBuffType, float x, float y, float width, float height) //DAT
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

	int iWidthIndex, iHeightIndex;
	float u, v;

	if (eBuffType < 81) // eBuff_Berserker
	{
		iWidthIndex = (eBuffType - 1) % 10;
		iHeightIndex = (eBuffType - 1) / 10;
		
		u = iWidthIndex * 20 / 256.f;
		v = iHeightIndex * 28 / 256.f;
		
		RenderBitmap(IMAGE_BUFF_STATUS, x, y, width, height, u, v, 20/256.f, 28/256.f);
	}
	else
	{
		// eBuff_Santa
		iWidthIndex = (eBuffType - 81) % 10; // eBuff_Berserker
		iHeightIndex = (eBuffType - 81) / 10; // eBuff_Berserker
		
		u = iWidthIndex * 20 / 256.f;
		v = iHeightIndex * 28 / 256.f;
		
		RenderBitmap(IMAGE_BUFF_STATUS2, x , y, width, height, u, v, 20/256.f, 28/256.f);
	}
}

void SEASON3B::CNewUIBuffWindow::RenderBuffTooltip(eBuffClass& eBuffClassType, eBuffState& eBuffType, float x, float y, int IndexTime)
{

	int TextNum = 0;
	::memset(TextList[0], 0, sizeof(char) * 30 * 100);
	::memset(TextListColor, 0, sizeof(int) * 30);
	::memset(TextBold, 0, sizeof(int) * 30);
	

	//===Render Custom BUff ToolTip
	bool continuar = false;

	std::map<int, RenderTooltipBuff>::iterator it;

	if (strcmp((char*)g_strSelectedML.c_str(), "Eng") == 0)
	{
		it = gIconBuff.BuffEffectTRSData_ENG.find(eBuffType);

		if (it != gIconBuff.BuffEffectTRSData_ENG.end())
		{
			continuar = true;
		}
	}

	if (strcmp((char*)g_strSelectedML.c_str(), "Vn") == 0)
	{
		it = gIconBuff.BuffEffectTRSData_VTM.find(eBuffType);

		if (it != gIconBuff.BuffEffectTRSData_VTM.end())
		{
			continuar = true;
		}
	}

	//====

	if (continuar)
	{
		char Buff[256] = { 0 };
		char BuffDate[256] = { 0 };
		//--
		wsprintf(TextList[TextNum], "\n");
		TextListColor[TextNum] = 0;
		TextBold[TextNum++] = 0;
		//--
		wsprintf(TextList[TextNum], "%s", it->second.NameBuff);
		TextListColor[TextNum] = 1;
		TextBold[TextNum++] = 0;
		//--
		wsprintf(TextList[TextNum], "\n");
		TextListColor[TextNum] = 0;
		TextBold[TextNum++] = 0;

		strncpy(Buff, gIconBuff.RenderInfoBuff[eBuffType], 200);

		char* Line = strtok(Buff, "/");

		while (Line != NULL)
		{
			wsprintf(TextList[TextNum], "%s", Line);
			TextListColor[TextNum] = 0;
			TextBold[TextNum++] = 0;

			Line = strtok(NULL, "/");
		}
		if (IndexTime != -1)
		{
			if (gIconBuff.UserBuff[IndexTime].Time > 0)
			{
				time_t t = gIconBuff.UserBuff[IndexTime].Time;
				tm* m = gmtime(&t);
				char timerCount[254];

				if (m->tm_yday > 0)
				{
					wsprintf(timerCount, "%d %s %d %s %d %s %d %s", m->tm_yday, GlobalText[2298], m->tm_hour, GlobalText[2299], m->tm_min, GlobalText[2300], m->tm_sec, GlobalText[2301]);
				}
				else if (m->tm_hour > 0)
				{
					wsprintf(timerCount, "%d %s %d %s %d %s", m->tm_hour, GlobalText[2299], m->tm_min, GlobalText[2300], m->tm_sec, GlobalText[2301]);
				}
				else if (m->tm_min > 0)
				{
					wsprintf(timerCount, "%d %s %d %s", m->tm_min, GlobalText[2300], m->tm_sec, GlobalText[2301]);
				}
				else if (m->tm_sec > 0)
				{
					wsprintf(timerCount, "%d %s", m->tm_sec, GlobalText[2301]);
				}
				wsprintf(BuffDate, GlobalText[2533], timerCount);
			}
			if (gIconBuff.UserBuff[IndexTime].Time > 0 && gIconBuff.UserBuff[IndexTime].Index != 28)
			{
				//--
				wsprintf(TextList[TextNum], "\n");
				TextListColor[TextNum] = 0;
				TextBold[TextNum++] = 0;
				//-
				wsprintf(TextList[TextNum], "%s", BuffDate);
				TextListColor[TextNum] = 6;
				TextBold[TextNum++] = 0;
			}
		}

		wsprintf(TextList[TextNum], "\n");
		TextListColor[TextNum] = 0;
		TextBold[TextNum++] = 0;
	}
	else
	{
		std::list<std::string> tooltipinfo;
		g_BuffToolTipString(tooltipinfo, eBuffType);

		for (std::list<std::string>::iterator iter = tooltipinfo.begin(); iter != tooltipinfo.end(); ++iter)
		{
			std::string& temp = *iter;

			unicode::_sprintf(TextList[TextNum], temp.c_str());

			if (TextNum == 0)
			{
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = true;
			}
			else
			{
				TextListColor[TextNum] = TEXT_COLOR_WHITE;
				TextBold[TextNum] = false;
			}

			TextNum += 1;
		}

		std::string bufftime;
		g_BuffStringTime(eBuffType, bufftime);

		if (bufftime.size() != 0)
		{
			unicode::_sprintf(TextList[TextNum], GlobalText[2533], bufftime.c_str());
			TextListColor[TextNum] = TEXT_COLOR_PURPLE;
			TextBold[TextNum] = false;
			TextNum += 1;
		}
	}
	SIZE TextSize = {0, 0};	
	g_pMultiLanguage->_GetTextExtentPoint32( g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize );
	RenderTipTextList( x,y,TextNum, 0 );
}

float SEASON3B::CNewUIBuffWindow::GetLayerDepth()	//. 5.3f
{
	return 0.95f;
}

void SEASON3B::CNewUIBuffWindow::OpenningProcess()
{

}

void SEASON3B::CNewUIBuffWindow::ClosingProcess()
{

}

void SEASON3B::CNewUIBuffWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_statusicon.jpg", IMAGE_BUFF_STATUS, GL_LINEAR);
	LoadBitmap("Interface\\newui_statusicon2.jpg", IMAGE_BUFF_STATUS2, GL_LINEAR);
}

void SEASON3B::CNewUIBuffWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_BUFF_STATUS2);
	DeleteBitmap(IMAGE_BUFF_STATUS);	
}
