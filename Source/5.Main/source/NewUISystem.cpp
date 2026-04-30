
#include "stdafx.h"
#include "NewUISystem.h"
#include "NewUIMessageBox.h"
#include "wsclientinline.h"
#include "PersonalShopTitleImp.h"
#include "w_CursedTemple.h"
#include "MapManager.h"
#include "./Utilities/Log/muConsoleDebug.h"

using namespace SEASON3B;
extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayHeight;
extern int DisplayWinMid;
extern int DisplayHeightExt;
extern int DisplayWinExt;
extern int DisplayWinReal;

#if defined(__ANDROID__) || defined(MU_IOS)
extern bool AndroidGetMoveMapWindowPosition(int panelWidth, int panelHeight, int* outX, int* outY);
#endif

SEASON3B::CNewUISystem::CNewUISystem()
{
	m_pNewUIMng = NULL;
	m_pNewUIHotKey = NULL;
	m_pNewChatLogWindow = NULL;
	m_pNewSlideWindow = NULL;
	m_pNewGuildMakeWindow = NULL;
	m_pNewFriendWindow = NULL;
	m_pNewMainFrameWindow = NULL;
	m_pNewSkillList = NULL;
	m_pNewChatInputBox = NULL;
	m_pNewItemMng = NULL;
	m_pNewMyInventory = NULL;
	m_pNewMyInventoryExt = nullptr;
	m_pNewNPCShop = NULL;
	m_pNewPetInfoWindow = NULL;
	m_pNewMixInventory = NULL;
	m_pNewCastleWindow = NULL;
	m_pNewGuardWindow = NULL;
	m_pNewGatemanWindow = NULL;
	m_pNewGateSwitchWindow = NULL;
	m_pNewStorageInventory = NULL;
	m_pNewStorageInventoryExt = NULL;
	m_pNewGuildInfoWindow = NULL;
	m_pNewMyShopInventory = NULL;
	m_pNewCharacterInfoWindow = NULL;
	m_pNewPartyInfoWindow = NULL;
	m_pNewPartyListWindow = NULL;
	m_pNewNPCQuest = NULL;
	m_pNewEnterBloodCastle = NULL;
	m_pNewEnterDevilSquare = NULL;
	m_pNewBloodCastle = NULL;
	m_pNewTrade = NULL;
	m_pNewKanturu2ndEnterNpc = NULL;
	m_pNewCatapultWindow = NULL;
	m_pNewChaosCastleTime = NULL;
	m_pNewBattleSoccerScore = NULL;
	m_pNewCommandWindow = NULL;
	m_pNewWindowMenu = NULL;
	m_pNewOptionWindow = NULL;
	m_pNewHeroPositionInfo = NULL;
	m_pNewBCustomMenuInfo = NULL;
	/*m_pNewBMacroInfo = NULL;*/
	m_pNewHelpWindow = NULL;
	m_pNewItemExplanationWindow = NULL;
	m_pNewSetItemExplanation = NULL;
	m_pNewQuickCommandWindow = NULL;
	m_pNewMoveCommandWindow = NULL;
	m_pNewSiegeWarfare = NULL;
	m_pNewItemEnduranceInfo = NULL;
	m_pNewBuffWindow = NULL;
	m_pNewCryWolfInterface = NULL;
	//m_pNewMaster_Level_Interface = NULL;

	m_pNewMaster_SkillTree_Interface = NULL;

	m_pNewCursedTempleResultWindow = NULL;
	m_pNewCursedTempleWindow = NULL;
	m_pNewCursedTempleEnterWindow = NULL;
	m_pNewGoldBowman = NULL;
	m_pNewGoldBowmanLena = NULL;
	m_pNewLuckyCoinRegistration = NULL;
	m_pNewExchangeLuckyCoinWindow = NULL;
	m_pNewDuelWatchWindow = NULL;
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
	m_pNewInGameShop = NULL;
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
	m_pNewDoppelGangerWindow = NULL;
	m_pNewDoppelGangerFrame = NULL;
	m_pNewNPCDialogue = NULL;
	m_pNewQuestProgress = NULL;
	m_pNewQuestProgressByEtc = NULL;
	m_pNewEmpireGuardianNPC = NULL;
	m_pNewEmpireGuardianTimer = NULL;
	m_pNewMiniMap = NULL;
#ifdef PBG_MOD_STAMINA_UI
	m_pNewUIStamina = NULL;
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
	m_pNewGensRanking = NULL;
#endif //PBG_ADD_GENSRANKING
	m_pNewUnitedMarketPlaceWindow = NULL;
	m_pNewUIMuHelper = NULL;
}

SEASON3B::CNewUISystem::~CNewUISystem() 
{ 
	Release(); 
}

bool SEASON3B::CNewUISystem::Create()
{
	m_pNewUIMng = new CNewUIManager;
	
	m_pNewUI3DRenderMng = new CNewUI3DRenderMng;
	if(false == m_pNewUI3DRenderMng->Create(m_pNewUIMng))
		return false;

	m_pNewChatLogWindow = new CNewUIChatLogWindow;
	if(false == m_pNewChatLogWindow->Create(m_pNewUIMng, 0, 480-50-47, 6))
		return false;

	m_pNewOptionWindow = new CNewUIOptionWindow;
	if(m_pNewOptionWindow->Create(m_pNewUIMng, DisplayWinMid-95, 35) == false)
	{
		return false;
	}

	m_pNewSlideWindow = new CNewUISlideWindow;
	if(m_pNewSlideWindow->Create(m_pNewUIMng) == false)
	{
		return false;
	}

	if(g_MessageBox->Create(m_pNewUIMng) == false)
		return false;

	return true;
}

void SEASON3B::CNewUISystem::Release()
{
	UnloadMainSceneInterface();

	SAFE_DELETE(m_pNewSlideWindow);
	SAFE_DELETE(m_pNewOptionWindow);
	SAFE_DELETE(m_pNewChatLogWindow);
	SAFE_DELETE(m_pNewUI3DRenderMng);
	
	m_pNewUIMng->RemoveAllUIObjs();

	SAFE_DELETE(m_pNewUIMng);
}

bool SEASON3B::CNewUISystem::LoadMainSceneInterface()
{
#ifdef __ANDROID__
#define ANDROID_UI_STAGE(msg) do { OutputDebugStringA("UI_LOAD: " msg); g_ErrorReport.Write("[UI_LOAD] " msg "\r\n"); } while(0)
#else
#define ANDROID_UI_STAGE(msg) ((void)0)
#endif
	ANDROID_UI_STAGE("begin");
	g_MessageBox->Show(true);
	m_pNewChatLogWindow->Show(true);
	m_pNewSlideWindow->Show(true);

	m_pNewItemMng = new CNewUIItemMng;

	m_pNewChatInputBox = new CNewUIChatInputBox;

	if(false == m_pNewChatInputBox->Create(m_pNewUIMng, m_pNewChatLogWindow, 0, 480-51-47))
	{
		return false;
	}
	SetFocus(g_hWnd);
	ANDROID_UI_STAGE("chat input ok");

	m_pNewUIHotKey = new CNewUIHotKey;
	if(false == m_pNewUIHotKey->Create(m_pNewUIMng))
		return false;
	ANDROID_UI_STAGE("hotkey ok");

	m_pNewMainFrameWindow = new CNewUIMainFrameWindow;
	if(m_pNewMainFrameWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng) == false)
		return false;
	ANDROID_UI_STAGE("main frame ok");

	m_pNewSkillList = new CNewUISkillList;
	if(m_pNewSkillList->Create(m_pNewUIMng, m_pNewUI3DRenderMng) == false)
		return false;
	ANDROID_UI_STAGE("skill list ok");

	m_pNewFriendWindow = new CNewUIFriendWindow;
	if(m_pNewFriendWindow->Create(m_pNewUIMng) == false)
		return false;

	m_pNewMyInventoryExt = new CNewUIInventoryExtension;
	if (false == m_pNewMyInventoryExt->Create(m_pNewUIMng, 640 - (190 * 2) + DisplayWinCDepthBox, 0))
		return false;

	m_pNewMyInventory = new CNewUIMyInventory;
	if(false == m_pNewMyInventory->Create(m_pNewUIMng, m_pNewUI3DRenderMng, 640-190, 0)) //
		return false;
	ANDROID_UI_STAGE("inventory group ok");



	m_pNewNPCShop = new CNewUINPCShop;
	if(false == m_pNewNPCShop->Create(m_pNewUIMng, CNewUINPCShop::NPCSHOP_POS_X + DisplayWinCDepthBox, CNewUINPCShop::NPCSHOP_POS_Y))
		return false;

	m_pNewPetInfoWindow = new CNewUIPetInfoWindow;
	if(false == m_pNewPetInfoWindow->Create(m_pNewUIMng, 640-(190*2) + DisplayWinCDepthBox, 0))
		return false;
	
	m_pNewMixInventory = new CNewUIMixInventory;
	if(m_pNewMixInventory->Create(m_pNewUIMng, 260+ DisplayWinCDepthBox, 0) == false) //Fix WIDE
		return false;

	m_pNewCastleWindow = new CNewUICastleWindow;
	if(m_pNewCastleWindow->Create(m_pNewUIMng, 640-190 + DisplayWinCDepthBox, 0) == false)
		return false;

	m_pNewGuardWindow = new CNewUIGuardWindow;
	if(m_pNewGuardWindow->Create(m_pNewUIMng, 640-190 + DisplayWinCDepthBox, 0) == false)
		return false;

	m_pNewGatemanWindow = new CNewUIGatemanWindow;
	if(m_pNewGatemanWindow->Create(m_pNewUIMng, 640-190 + DisplayWinCDepthBox, 0) == false)
		return false;

	m_pNewGateSwitchWindow = new CNewUIGateSwitchWindow;
	if(m_pNewGateSwitchWindow->Create(m_pNewUIMng, 640-190 + DisplayWinCDepthBox, 0) == false)
		return false;

	m_pNewStorageInventory = new CNewUIStorageInventory;
	if(m_pNewStorageInventory->Create(m_pNewUIMng, 260 + DisplayWinCDepthBox, 0) == false) //hom do
		return false;

	m_pNewStorageInventoryExt = new CNewUIStorageInventoryExt;
	if (m_pNewStorageInventoryExt->Create(m_pNewUIMng, (260 - 190)+DisplayWinCDepthBox, 0) == false)
		return false;
	ANDROID_UI_STAGE("storage group ok");


	m_pNewGuildInfoWindow = new CNewUIGuildInfoWindow;
	if(m_pNewGuildInfoWindow->Create(m_pNewUIMng, 640-190 + DisplayWinCDepthBox, 0) == false)
		return false;

	m_pNewGuildMakeWindow = new CNewUIGuildMakeWindow;
	if(m_pNewGuildMakeWindow->Create(m_pNewUIMng, 640-190 + DisplayWinCDepthBox, 0) == false)
		return false;

	CreatePersonalItemTable();

	m_pNewMyShopInventory = new CNewUIMyShopInventory;
	if(m_pNewMyShopInventory->Create(m_pNewUIMng, 640-190*2 + DisplayWinCDepthBox, 0) == false)
		return false;
	
	m_pNewPurchaseShopInventory = new CNewUIPurchaseShopInventory;
	if(m_pNewPurchaseShopInventory->Create(m_pNewUIMng, 640-190*2 + DisplayWinCDepthBox, 0) == false)
		return false;

	m_pNewCharacterInfoWindow = new CNewUICharacterInfoWindow;
	if(m_pNewCharacterInfoWindow->Create(m_pNewUIMng, 640-190 + DisplayWinCDepthBox, 0) == false)
		return false;

	m_pNewMyQuestInfoWindow = new CNewUIMyQuestInfoWindow;
	if(m_pNewMyQuestInfoWindow->Create(m_pNewUIMng, 640-190 + DisplayWinCDepthBox, 0) == false)
		return false;

	m_pNewPartyInfoWindow = new CNewUIPartyInfoWindow;
	if( m_pNewPartyInfoWindow->Create( m_pNewUIMng, 640-190 + DisplayWinCDepthBox, 0 ) == false )
		return false;

	m_pNewPartyListWindow = new CNewUIPartyListWindow;
	if( m_pNewPartyListWindow->Create( m_pNewUIMng, 640-79 + DisplayWinCDepthBox, 14 ) == false )
		return false;
	ANDROID_UI_STAGE("party group ok");

	m_pNewNPCQuest = new CNewUINPCQuest;
	if (m_pNewNPCQuest->Create(m_pNewUIMng, m_pNewUI3DRenderMng, (640-190), 0) == false)
		return false;

	m_pNewEnterBloodCastle = new CNewUIEnterBloodCastle;
	if( m_pNewEnterBloodCastle->Create(m_pNewUIMng,( 640-190) + DisplayWinCDepthBox, 0) == false )
		return false;

	m_pNewEnterDevilSquare = new CNewUIEnterDevilSquare;
	if( m_pNewEnterDevilSquare->Create(m_pNewUIMng, (640-190) + DisplayWinCDepthBox, 0) == false )
		return false;
	
	m_pNewBloodCastle = new CNewUIBloodCastle;
	if( m_pNewBloodCastle->Create(m_pNewUIMng, (640-127) + DisplayWinExt, DisplayHeight -132) == false )
		return false;

	m_pNewTrade = new CNewUITrade;
	if (m_pNewTrade->Create(m_pNewUIMng, 260 + DisplayWinCDepthBox, 0) == false)
		return false;
	ANDROID_UI_STAGE("trade group ok");

	m_pNewKanturu2ndEnterNpc = new CNewUIKanturu2ndEnterNpc;
	if(m_pNewKanturu2ndEnterNpc->Create(m_pNewUIMng, (640/2) - (230/2), 20) == false)	
	{
		return false;
	}

	m_pNewKanturuInfoWindow = new CNewUIKanturuInfoWindow;
	if(m_pNewKanturuInfoWindow->Create(m_pNewUIMng, 541 + DisplayWinExt, DisplayHeightExt +351) == false)
	{
		return false;
	}

	m_pNewChaosCastleTime = new CNewUIChaosCastleTime;
	if( m_pNewChaosCastleTime->Create(m_pNewUIMng, (640 - 127) + DisplayWinExt, DisplayHeight -132) == false )
		return false;

	m_pNewBattleSoccerScore = new CNewUIBattleSoccerScore;
	if (m_pNewBattleSoccerScore->Create(m_pNewUIMng, 509 + DisplayWinExt, DisplayHeightExt +359) == false)
		return false;
	
	m_pNewCommandWindow = new CNewUICommandWindow;
	if( m_pNewCommandWindow->Create(m_pNewUIMng, 640-190 + DisplayWinCDepthBox, 0) == false )
		return false;

	m_pNewCatapultWindow = new CNewUICatapultWindow;
	if(m_pNewCatapultWindow->Create(m_pNewUIMng, 640-190, 0) == false)
	{
		return false;
	}

	m_pNewWindowMenu = new CNewUIWindowMenu;
	if(m_pNewWindowMenu->Create(m_pNewUIMng, 640-112+ DisplayWinCDepthBox, 480-171) == false)
	{
		return false;
	}
	ANDROID_UI_STAGE("menu group ok");

	ANDROID_UI_STAGE("before custom menu new");
	m_pNewBCustomMenuInfo = new CNewUIBCustomMenuInfo;
	ANDROID_UI_STAGE("after custom menu new");
	if (m_pNewBCustomMenuInfo->Create(m_pNewUIMng, 0, 0) == false)
	{
		return false;
	}
	ANDROID_UI_STAGE("custom menu ok");

	m_pNewHeroPositionInfo = new CNewUIHeroPositionInfo;
	if(m_pNewHeroPositionInfo->Create(m_pNewUIMng, 0, 0) == false)
	{
		return false;
	}
	ANDROID_UI_STAGE("hero position ok");
	//m_pNewBMacroInfo = new CNewUIMacro;
	//if (m_pNewBMacroInfo->Create(m_pNewUIMng, 0, 0) == false)
	//{
	//	return false;
	//}
	//CNewUIMacro
	m_pNewHelpWindow = new CNewUIHelpWindow;
	if(m_pNewHelpWindow->Create(m_pNewUIMng, 0, 0) == false)
	{
		return false;
	}
	ANDROID_UI_STAGE("help window ok");

	m_pNewItemExplanationWindow = new CNewUIItemExplanationWindow;
	if(m_pNewItemExplanationWindow->Create(m_pNewUIMng, 0, 0) == false)
	{
		return false;
	}
	ANDROID_UI_STAGE("item explanation ok");

	m_pNewSetItemExplanation = new CNewUISetItemExplanation;
	if(m_pNewSetItemExplanation->Create(m_pNewUIMng, 0, 0) == false)
	{
		return false;
	}
	ANDROID_UI_STAGE("set item explanation ok");

	m_pNewQuickCommandWindow = new CNewUIQuickCommandWindow;
	if(m_pNewQuickCommandWindow->Create(m_pNewUIMng, 0, 0) == false)
	{
		return false;
	}
	ANDROID_UI_STAGE("quick command ok");

	m_pNewMoveCommandWindow = new CNewUIMoveCommandWindow;
	
	if(m_pNewMoveCommandWindow->Create(m_pNewUIMng, 1, 1) == false)
		return false;
	ANDROID_UI_STAGE("move command ok");

	m_pNewDuelWindow = new CNewUIDuelWindow;
	if(m_pNewDuelWindow->Create(m_pNewUIMng, 509 + DisplayWinExt, DisplayHeightExt +359) == false)
	{
		return false;
	}
	
	m_pNewNameWindow = new CNewUINameWindow;
	if(m_pNewNameWindow->Create(m_pNewUIMng, 0, 0) == false)
	{
		return false;
	}

	m_pNewSiegeWarfare = new CNewUISiegeWarfare;
	if(m_pNewSiegeWarfare->Create(m_pNewUIMng, 486 + DisplayWinExt, DisplayHeightExt +234) == false)
		return false;

	m_pNewItemEnduranceInfo = new CNewUIItemEnduranceInfo;
	if( m_pNewItemEnduranceInfo->Create( m_pNewUIMng, 2, 26 ) == false )
	{
		return false;
	}

	m_pNewBuffWindow = new CNewUIBuffWindow;
	if(m_pNewBuffWindow->Create(m_pNewUIMng, 220, 15) == false)
	{
		return false;
	}
	ANDROID_UI_STAGE("buff group ok");

	m_pNewCursedTempleEnterWindow = new CNewUICursedTempleEnter;
	if(m_pNewCursedTempleEnterWindow->Create(m_pNewUIMng, 640/2-230/2, 80) == false)
	{
		return false;
	}
	m_pNewCursedTempleWindow = new CNewUICursedTempleSystem;
	if(m_pNewCursedTempleWindow->Create(m_pNewUIMng, 0, 0) == false)
	{
		return false;
	}
	m_pNewCursedTempleResultWindow = new CNewUICursedTempleResult;
	if(m_pNewCursedTempleResultWindow->Create(m_pNewUIMng, 640/2-230/2, 120) == false)
	{
		return false;
	}

	m_pNewCryWolfInterface = new CNewUICryWolf;
	if(m_pNewCryWolfInterface->Create(m_pNewUIMng, 0, 0) == false)
		return false;

	//m_pNewMaster_Level_Interface = new CNewUIMasterLevel;
	//if(m_pNewMaster_Level_Interface->Create(m_pNewUIMng, 0, 0) == false)
	//	return false;

	m_pNewMaster_SkillTree_Interface = new CNewUIMasterSkillTree;
	if (m_pNewMaster_SkillTree_Interface->Create() == false)
		return false;
	ANDROID_UI_STAGE("master skill tree ok");

	m_pNewMiniMap = new CNewUIMiniMap;
	if(m_pNewMiniMap->Create(m_pNewUIMng, 0, 0) == false)
		return false;
	ANDROID_UI_STAGE("minimap ok");

	m_pNewGoldBowman = new CNewUIGoldBowmanWindow;
	if( m_pNewGoldBowman->Create(m_pNewUIMng, 640-190, 0) == false )
		return false;

	m_pNewGoldBowmanLena = new CNewUIGoldBowmanLena;
	if( m_pNewGoldBowmanLena->Create(m_pNewUIMng, 640-190, 0) == false )
		return false;

	m_pNewLuckyCoinRegistration = new CNewUIRegistrationLuckyCoin;
	if( m_pNewLuckyCoinRegistration->Create(m_pNewUIMng, 640 - (190 * 2), 0) == false )
		return false;	

	m_pNewExchangeLuckyCoinWindow = new CNewUIExchangeLuckyCoin;
	if( m_pNewExchangeLuckyCoinWindow->Create(m_pNewUIMng, 640-(190*2), 0) == false )
		return false;

	m_pNewDuelWatchWindow = new CNewUIDuelWatchWindow;
	if( m_pNewDuelWatchWindow->Create(m_pNewUIMng, 640-190, 0) == false )
		return false;

	m_pNewDuelWatchMainFrameWindow = new CNewUIDuelWatchMainFrameWindow;
	if(m_pNewDuelWatchMainFrameWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng) == false)
		return false;

	m_pNewDuelWatchUserListWindow = new CNewUIDuelWatchUserListWindow;
	if(m_pNewDuelWatchUserListWindow->Create(m_pNewUIMng, (640-57) + DisplayWinCDepthBox, DisplayHeightExt +(480-51)) == false)
		return false;
	ANDROID_UI_STAGE("duel watch ok");

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
	m_pNewInGameShop = new CNewUIInGameShop;
	ANDROID_UI_STAGE("ingameshop begin");
	if(m_pNewInGameShop->Create(m_pNewUIMng, DisplayWinExt, 0) == false)
		return false;
	ANDROID_UI_STAGE("ingameshop ok");
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME

	m_pNewDoppelGangerWindow = new CNewUIDoppelGangerWindow;
	if( m_pNewDoppelGangerWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng, (640-190)+ DisplayWinCDepthBox, 0) == false )
		return false;

	m_pNewDoppelGangerFrame = new CNewUIDoppelGangerFrame;
	if( m_pNewDoppelGangerFrame->Create(m_pNewUIMng, (640-227) + DisplayWinCDepthBox, DisplayHeightExt +(480-51-87)) == false )
		return false;

	m_pNewNPCDialogue = new CNewUINPCDialogue;
	if (m_pNewNPCDialogue->Create(m_pNewUIMng, 640-190, 0) == false)
		return false;

	m_pNewQuestProgress = new CNewUIQuestProgress;
	if (m_pNewQuestProgress->Create(m_pNewUIMng, (640-190) + DisplayWinCDepthBox, 0) == false)
		return false;

	m_pNewQuestProgressByEtc = new CNewUIQuestProgressByEtc;
	if (m_pNewQuestProgressByEtc->Create(m_pNewUIMng, 640-190 , 0) == false)
		return false;

	m_pNewEmpireGuardianNPC = new CNewUIEmpireGuardianNPC;
	if( m_pNewEmpireGuardianNPC->Create(m_pNewUIMng, m_pNewUI3DRenderMng, 450, 0) == false )
		return false;

	m_pNewEmpireGuardianTimer = new CNewUIEmpireGuardianTimer;
	if( m_pNewEmpireGuardianTimer->Create(m_pNewUIMng, 507 + DisplayWinCDepthBox, DisplayHeightExt +342) == false )
		return false;
	ANDROID_UI_STAGE("late ui group ok");
	
#ifdef PBG_MOD_STAMINA_UI
	m_pNewUIStamina = new CNewUIStamina;
	if( m_pNewUIStamina->Create(m_pNewUIMng, 640, 480) == false )
		return false;
#endif //PBG_MOD_STAMINA_UI

	m_pNewGensRanking = new CNewUIGensRanking;
	if(m_pNewGensRanking->Create(m_pNewUIMng, 640, 480) == false)
		return false;

	m_pNewUnitedMarketPlaceWindow = new CNewUIUnitedMarketPlaceWindow;
	if( m_pNewUnitedMarketPlaceWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng, (640-190) + DisplayWinCDepthBox, 0) == false )
		return false;
	ANDROID_UI_STAGE("marketplace ok");

#ifdef LEM_ADD_LUCKYITEM
	m_pNewUILuckyItemWnd = new CNewUILuckyItemWnd;
	if(m_pNewUILuckyItemWnd->Create(m_pNewUIMng, 260, 0) == false)
		return false;
#endif // LEM_ADD_LUCKYITEM
	m_pNewUIMuHelper = new CNewUIMuHelper;
	if (m_pNewUIMuHelper->Create(m_pNewUIMng, (640 - 190) + DisplayWinCDepthBox, 0) == false)
		return false;
	ANDROID_UI_STAGE("helper ok");
#undef ANDROID_UI_STAGE
	return true;
}

void SEASON3B::CNewUISystem::UnloadMainSceneInterface()
{
	if(g_pNewUIMng)
	{
		g_pNewUIMng->ShowAllInterfaces(false);
	}

	SAFE_DELETE(m_pNewHelpWindow);
	SAFE_DELETE(m_pNewItemExplanationWindow);
	SAFE_DELETE(m_pNewSetItemExplanation);
	SAFE_DELETE(m_pNewQuickCommandWindow);
	SAFE_DELETE(m_pNewWindowMenu);
	SAFE_DELETE(m_pNewBattleSoccerScore);
	SAFE_DELETE(m_pNewCatapultWindow);
	SAFE_DELETE(m_pNewKanturu2ndEnterNpc);
	SAFE_DELETE(m_pNewKanturuInfoWindow);
	SAFE_DELETE(m_pNewTrade);
	SAFE_DELETE(m_pNewNPCQuest);
	SAFE_DELETE(m_pNewMyQuestInfoWindow);
	SAFE_DELETE(m_pNewCharacterInfoWindow);
	SAFE_DELETE(m_pNewPurchaseShopInventory);
	SAFE_DELETE(m_pNewMyShopInventory);
	SAFE_DELETE(m_pNewGuildMakeWindow);
	SAFE_DELETE(m_pNewGuildInfoWindow);
	SAFE_DELETE(m_pNewStorageInventory);
	SAFE_DELETE(m_pNewStorageInventoryExt);
	SAFE_DELETE(m_pNewMixInventory);
	SAFE_DELETE(m_pNewCastleWindow);
	SAFE_DELETE(m_pNewGuardWindow);
	SAFE_DELETE(m_pNewGatemanWindow);
	SAFE_DELETE(m_pNewGateSwitchWindow);
	SAFE_DELETE(m_pNewNPCShop);
	SAFE_DELETE(m_pNewPetInfoWindow);
	SAFE_DELETE(m_pNewMyInventory);
	SAFE_DELETE(m_pNewMyInventoryExt);
	SAFE_DELETE(m_pNewFriendWindow);
	SAFE_DELETE(m_pNewChatInputBox);
	SAFE_DELETE(m_pNewNameWindow);
	SAFE_DELETE(m_pNewSkillList);
	SAFE_DELETE(m_pNewMainFrameWindow);
	SAFE_DELETE(m_pNewPartyInfoWindow );
	SAFE_DELETE(m_pNewPartyListWindow );
	SAFE_DELETE(m_pNewEnterBloodCastle );
	SAFE_DELETE(m_pNewEnterDevilSquare );
	SAFE_DELETE(m_pNewBloodCastle );
	SAFE_DELETE(m_pNewChaosCastleTime );
	SAFE_DELETE(m_pNewCommandWindow );
	SAFE_DELETE(m_pNewHeroPositionInfo );
	SAFE_DELETE(m_pNewBCustomMenuInfo);
	//SAFE_DELETE(m_pNewBMacroInfo);
	SAFE_DELETE(m_pNewMoveCommandWindow );
	SAFE_DELETE(m_pNewUIHotKey);
	SAFE_DELETE(m_pNewSiegeWarfare);
	SAFE_DELETE(m_pNewItemEnduranceInfo );
	SAFE_DELETE(m_pNewBuffWindow);
	SAFE_DELETE(m_pNewCursedTempleResultWindow);
	SAFE_DELETE(m_pNewCursedTempleWindow);
	SAFE_DELETE(m_pNewCursedTempleEnterWindow);
	SAFE_DELETE(m_pNewCryWolfInterface);
	//SAFE_DELETE(m_pNewMaster_Level_Interface);

	SAFE_DELETE(m_pNewMaster_SkillTree_Interface);

	SAFE_DELETE(m_pNewGoldBowman);
	SAFE_DELETE(m_pNewGoldBowmanLena);
	SAFE_DELETE(m_pNewLuckyCoinRegistration);
	SAFE_DELETE(m_pNewExchangeLuckyCoinWindow);
	SAFE_DELETE(m_pNewDuelWatchWindow);
	SAFE_DELETE(m_pNewDuelWindow);
	SAFE_DELETE(m_pNewDuelWatchMainFrameWindow);
	SAFE_DELETE(m_pNewDuelWatchUserListWindow);
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
	SAFE_DELETE(m_pNewInGameShop);
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
	SAFE_DELETE(m_pNewDoppelGangerWindow);
	SAFE_DELETE(m_pNewDoppelGangerFrame);
	SAFE_DELETE(m_pNewNPCDialogue);
	SAFE_DELETE(m_pNewQuestProgress);
	SAFE_DELETE(m_pNewQuestProgressByEtc);
	SAFE_DELETE(m_pNewEmpireGuardianNPC);
	SAFE_DELETE(m_pNewEmpireGuardianTimer);
	SAFE_DELETE(m_pNewMiniMap);
	SAFE_DELETE(m_pNewItemMng);
#ifdef PBG_MOD_STAMINA_UI
	SAFE_DELETE(m_pNewUIStamina);
#endif //PBG_MOD_STAMINA_UI
	SAFE_DELETE(m_pNewGensRanking);
	SAFE_DELETE(m_pNewUnitedMarketPlaceWindow);
	SAFE_DELETE(m_pNewUIMuHelper);
#ifdef LEM_FIX_LUCKYITEM_UICLASS_SAFEDELETE
	SAFE_DELETE( m_pNewUILuckyItemWnd );
#endif // LEM_FIX_LUCKYITEM_UICLASS_SAFEDELETE
	
	ReleasePersonalItemTable();
}

bool SEASON3B::CNewUISystem::IsVisible(DWORD dwKey)
{
	if(m_pNewUIMng)
	{
		return m_pNewUIMng->IsInterfaceVisible(dwKey);
	}

	return false;
}
//=== Show ALl Window && Set Pos WIDE VK_OPEN
void SEASON3B::CNewUISystem::Show(DWORD dwKey)
{
#ifdef PBG_ADD_INGAMESHOP_UI_ITEMSHOP
	if(g_pInGameShop->IsInGameShop())
		return;
#endif //PBG_ADD_INGAMESHOP_UI_ITEMSHOP
	if(m_pNewUIMng) 
	{
		if(dwKey == SEASON3B::INTERFACE_FRIEND)
		{
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_FRIEND, true);

			m_pNewFriendWindow->OpenMainWnd(640-250, 432-173);
		}
		else if (dwKey == SEASON3B::INTERFACE_ExpandInventory)
		{

		}
		else if(dwKey == SEASON3B::INTERFACE_INVENTORY)
		{
			HideGroupBeforeOpenInterface();
			
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);

			if(IsVisible(SEASON3B::INTERFACE_CHARACTER))
			{
				g_pMyInventory->SetPos((640-(190*2)), 0);
				g_pMyInventoryExt->SetPos(g_pMyInventory->GetPos().x - 190, 0);
			}
			if(IsVisible(SEASON3B::INTERFACE_MYQUEST))
			{
				Hide(SEASON3B::INTERFACE_MYQUEST);
			}
			g_pMyInventory->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CHARACTER)
		{
			HideGroupBeforeOpenInterface();

			g_pMainFrame->SetBtnState(MAINFRAME_BTN_CHAINFO, true);

			if(IsVisible(SEASON3B::INTERFACE_INVENTORY))
			{
				g_pMyInventory->SetPos(640-(190*2), 0);
				g_pMyInventoryExt->SetPos(g_pMyInventory->GetPos().x - 190, 0);
			}

			else if(IsVisible(SEASON3B::INTERFACE_MYQUEST))
			{
				g_pMyQuestInfoWindow->SetPos(640-190*2, 0);
			}
			g_pCharacterInfoWindow->OpenningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_PET )
		{
			if(IsVisible(SEASON3B::INTERFACE_INVENTORY))
			{
				Hide(SEASON3B::INTERFACE_INVENTORY);
			}
			if(IsVisible(SEASON3B::INTERFACE_MYQUEST))
			{
				Hide(SEASON3B::INTERFACE_MYQUEST);
			}

			HideGroupBeforeOpenInterface();

			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_CHARACTER);
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_CHAINFO, true);
			m_pNewPetInfoWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_MYQUEST)
		{
			HideGroupBeforeOpenInterface();

			if(IsVisible(SEASON3B::INTERFACE_CHARACTER))
			{
				g_pMyQuestInfoWindow->SetPos(640-190*2, 0);
			}
			if(IsVisible(SEASON3B::INTERFACE_INVENTORY))
			{
				Hide(SEASON3B::INTERFACE_INVENTORY);
			}
			if(IsVisible(SEASON3B::INTERFACE_PET))
			{
				Hide(SEASON3B::INTERFACE_PET);
			}
			g_pMyQuestInfoWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_MIXINVENTORY)
		{
			HideAllGroupA();
			g_pMixInventory->OpeningProcess();
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY);
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
		}
		else if(dwKey == SEASON3B::INTERFACE_NPCSHOP)
		{
			HideAllGroupA();
			g_pNPCShop->OpenningProcess();
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY);
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
		}
		else if(dwKey == SEASON3B::INTERFACE_STORAGE)
		{
			HideAllGroupA();
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY);
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
		}
		else if (dwKey == INTERFACE_ExpandWarehouse)
		{
			if (IsVisible(INTERFACE_ExpandWarehouse))
			{
				m_pNewUIMng->ShowInterface(INTERFACE_ExpandWarehouse, false);
			}

			//Hide(INTERFACE_HERO_POSITION_INFO);
			//g_pStorageInventory->SetPos(640 - 190 * 2, 0);
			g_pStorageInventoryExt->SetPos((260 - 190) + DisplayWinCDepthBox, 0);

			m_pNewUIMng->ShowInterface(INTERFACE_ExpandWarehouse);
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
		}
		else if(dwKey == SEASON3B::INTERFACE_MYSHOP_INVENTORY)
		{
			HideAllGroupA();
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY);
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
			if(gMapManager.IsCursedTemple() == true)
			{
				g_pMyShopInventory->OpenButtonLock();
			}
			else
			{
				g_pMyShopInventory->OpenButtonUnLock();
			}
		}
		else if(dwKey == SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY)
		{
			HideAllGroupA();
		}
		else if( dwKey == SEASON3B::INTERFACE_PARTY)
		{
			HideAllGroupA();
			m_pNewPartyInfoWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_NPCQUEST)
		{
			HideAllGroupA();
			g_pNPCQuest->ProcessOpening();
		}
		else if(dwKey == SEASON3B::INTERFACE_TRADE)
		{
			HideAllGroupA();
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY);
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
		}
		else if( dwKey == SEASON3B::INTERFACE_BLOODCASTLE )
		{
			HideAllGroupA();
			g_pEnterBloodCastle->OpenningProcess( );
		} 
		else if( dwKey == SEASON3B::INTERFACE_DEVILSQUARE )
		{
			HideAllGroupA();
			g_pEnterDevilSquare->OpenningProcess();
		} 
		else if(dwKey == SEASON3B::INTERFACE_CATAPULT)
		{
			HideAllGroupA();
			g_pCatapultWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_COMMAND)
		{
			HideAllGroupA();
			m_pNewCommandWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_GUILDINFO)
		{
			HideAllGroupA();
			g_pGuildInfoWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_WINDOW_MENU)
		{
			g_pWindowMenu->OpenningProcess();
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_WINDOW, true);
		}
		else if(dwKey == SEASON3B::INTERFACE_SENATUS)
		{
			HideAllGroupA();
			g_pCastleWindow->OpeningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_GUARDSMAN)
		{
			HideAllGroupA();
			g_pGuardWindow->OpeningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_GATEKEEPER)
		{
			HideAllGroupA();
			g_pGatemanWindow->OpeningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_GATESWITCH)
		{
			HideAllGroupA();
			g_pGateSwitchWindow->OpeningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_NPCGUILDMASTER)
		{
			HideAllGroupA();
		}

		else if(dwKey == SEASON3B::INTERFACE_MASTER_LEVEL)
		{
			HideAllGroupA();
		}

		else if(dwKey == SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC)
		{
			HideAllGroupB();
		}
		else if(dwKey == SEASON3B::INTERFACE_HERO_POSITION_INFO)
		{
			m_pNewHeroPositionInfo->OpenningProcess();

		}
		else if (dwKey == SEASON3B::INTERFACE_BCUSTOMMENU)
		{
			m_pNewBCustomMenuInfo->OpenningProcess();
		}
		else if (dwKey == SEASON3B::INTERFACE_MuHelper)
		{
			m_pNewUIMuHelper->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CHAOSCASTLE_TIME)
		{
			if(IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX))
			{
				Hide(SEASON3B::INTERFACE_CHATINPUTBOX);
			}
			m_pNewChaosCastleTime->OpenningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_BLOODCASTLE_TIME )
		{
			g_pBloodCastle->OpenningProcess( );
		} 
		else if(dwKey == SEASON3B::INTERFACE_OPTION)
		{
			g_pOption->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_HELP)
		{
			Hide(SEASON3B::INTERFACE_MOVEMAP);
			Hide(SEASON3B::INTERFACE_ITEM_EXPLANATION);
			Hide(SEASON3B::INTERFACE_SETITEM_EXPLANATION);
			g_pHelp->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_ITEM_EXPLANATION)
		{
			Hide(SEASON3B::INTERFACE_MOVEMAP);
			Hide(SEASON3B::INTERFACE_HELP);
			Hide(SEASON3B::INTERFACE_SETITEM_EXPLANATION);
			g_pItemExplanation->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_SETITEM_EXPLANATION)
		{
			Hide(SEASON3B::INTERFACE_MOVEMAP);
			Hide(SEASON3B::INTERFACE_HELP);
			Hide(SEASON3B::INTERFACE_ITEM_EXPLANATION);
			g_pSetItemExplanation->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_QUICK_COMMAND)
		{
			g_pQuickCommand->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_MOVEMAP)
		{
			Hide(SEASON3B::INTERFACE_HELP);
			Hide(SEASON3B::INTERFACE_ITEM_EXPLANATION);
			Hide(SEASON3B::INTERFACE_SETITEM_EXPLANATION);
#if defined(__ANDROID__) || defined(MU_IOS)
			int moveMapX = 0;
			int moveMapY = 0;
			if (AndroidGetMoveMapWindowPosition(
					m_pNewMoveCommandWindow->GetWindowWidth(),
					m_pNewMoveCommandWindow->GetWindowHeight(),
					&moveMapX,
					&moveMapY))
			{
				m_pNewMoveCommandWindow->SetPos(moveMapX, moveMapY);
			}
#endif
			m_pNewMoveCommandWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CHATINPUTBOX)
		{
			m_pNewChatInputBox->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_SIEGEWARFARE)
		{
			m_pNewSiegeWarfare->OpenningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_ITEM_ENDURANCE_INFO )
		{
			m_pNewItemEnduranceInfo->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_BUFF_WINDOW)
		{
			m_pNewBuffWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CRYWOLF)
		{
			m_pNewCryWolfInterface->OpenningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_GOLD_BOWMAN )
		{
			m_pNewGoldBowman->OpeningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_GOLD_BOWMAN_LENA )
		{
			m_pNewGoldBowmanLena->OpeningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION)
		{
			HideAllGroupA();
			g_pLuckyCoinRegistration->OpeningProcess();
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY);
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
		}
		else if(dwKey == SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN)
		{
			HideAllGroupA();
			g_pExchangeLuckyCoinWindow->OpenningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_DUELWATCH )
		{
			m_pNewDuelWatchWindow->OpeningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_DUELWATCH_MAINFRAME )
		{
			m_pNewDuelWatchMainFrameWindow->OpeningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_DUELWATCH_USERLIST )
		{
			m_pNewDuelWatchUserListWindow->OpeningProcess();
		}
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
		else if(dwKey == SEASON3B::INTERFACE_INGAMESHOP)
		{
			g_ConsoleDebug->Write(MCD_NORMAL,"InGameShopStatue.Txt CallStack - CNewUISystem.Show()\r\n");
			HideAll();
			g_pInGameShop->OpeningProcess();
#ifndef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
		}
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
		else if( dwKey == SEASON3B::INTERFACE_DOPPELGANGER_NPC )
		{
			m_pNewDoppelGangerWindow->OpeningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_DOPPELGANGER_FRAME )
		{
			m_pNewDoppelGangerFrame->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_NPC_DIALOGUE)
		{
			HideAllGroupA();
			g_pNPCDialogue->ProcessOpening();
		}
		else if(dwKey == SEASON3B::INTERFACE_QUEST_PROGRESS)
		{
			HideAllGroupA();
			g_pQuestProgress->ProcessOpening();
		}
		else if(dwKey == SEASON3B::INTERFACE_QUEST_PROGRESS_ETC)
		{
			if (IsVisible(SEASON3B::INTERFACE_INVENTORY))
				Hide(SEASON3B::INTERFACE_INVENTORY);
			if (IsVisible(SEASON3B::INTERFACE_MYQUEST))
			{
				Hide(SEASON3B::INTERFACE_MYQUEST);
				g_pQuestProgressByEtc->SetPos(640-190, 0);
			}
			if (IsVisible(SEASON3B::INTERFACE_CHARACTER))
				g_pQuestProgressByEtc->SetPos(640-190*2, 0);
			g_pQuestProgressByEtc->ProcessOpening();
		}
		else if( dwKey == SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC )
		{
			m_pNewEmpireGuardianNPC->OpenningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER)
		{
			m_pNewEmpireGuardianTimer->OpenningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_MINI_MAP)
		{
			m_pNewMiniMap->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_GENSRANKING)
		{
			HideAllGroupA();
			g_pNewUIGensRanking->OpenningProcess();
			g_pNewUIGensRanking->SetPos(640-190, 0);
		}
		else if( dwKey == SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA )
		{
			m_pNewUnitedMarketPlaceWindow->OpeningProcess();
		}
#ifdef LEM_ADD_LUCKYITEM
		else if( dwKey == SEASON3B::INTERFACE_LUCKYITEMWND )
		{
			HideAllGroupA();
			g_pLuckyItemWnd->OpeningProcess();
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY);
		}
#endif // LEM_ADD_LUCKYITEM

		m_pNewUIMng->ShowInterface(dwKey);
		
		int iScreenWidth = BGetScreenWidth();
		m_pNewItemEnduranceInfo->SetPos( iScreenWidth );
		m_pNewBuffWindow->SetPos(iScreenWidth);
		m_pNewPartyListWindow->SetPos( iScreenWidth );

		//===Fix Ware
		if (IsVisible(SEASON3B::INTERFACE_STORAGE) && IsVisible(SEASON3B::INTERFACE_ExpandInventory))
		{
			g_pStorageInventory->SetPos(g_pMyInventoryExt->GetPos().x - 190, 0);
			//g_pStorageInventoryExt->SetPos(g_pMyInventoryExt->GetPos().x - (190 * 2), 0);

		}
		else
		{
			g_pStorageInventory->SetPos(g_pMyInventory->GetPos().x - 190, 0);
			//g_pStorageInventoryExt->SetPos(g_pMyInventory->GetPos().x - (190 * 2), 0);
		}

		//===Fix Shop
		if (IsVisible(SEASON3B::INTERFACE_NPCSHOP) && IsVisible(SEASON3B::INTERFACE_ExpandInventory))
		{
			g_pNPCShop->SetPos(g_pMyInventoryExt->GetPos().x - 190, 0);

		}
		else
		{
			g_pNPCShop->SetPos(g_pMyInventory->GetPos().x - 190, 0);
		}
		//==Fix Trade
		if (IsVisible(SEASON3B::INTERFACE_TRADE) && IsVisible(SEASON3B::INTERFACE_ExpandInventory))
		{
			g_pTrade->SetPos(g_pMyInventoryExt->GetPos().x - 190, 0);

		}
		else
		{
			g_pTrade->SetPos(g_pMyInventory->GetPos().x - 190, 0);
		}
	}

	
}

void SEASON3B::CNewUISystem::Hide(DWORD dwKey)
{
	if(m_pNewUIMng) 
	{
		if(dwKey == SEASON3B::INTERFACE_FRIEND)
		{
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_FRIEND, false);
			m_pNewFriendWindow->HideAllWindow(TRUE, TRUE);
		}
		else if(dwKey == SEASON3B::INTERFACE_CHARACTER)
		{
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_CHAINFO, false);
			if(IsVisible(SEASON3B::INTERFACE_MYQUEST))
			{
				g_pMyQuestInfoWindow->SetPos(640-190, 0);
			}
			if(IsVisible((SEASON3B::INTERFACE_INVENTORY)))
			{
				g_pMyInventory->SetPos(640-190, 0);
				g_pMyInventoryExt->SetPos(g_pMyInventory->GetPos().x - 190, 0);
			}
			if(IsVisible((SEASON3B::INTERFACE_PET)))
			{
				Hide(SEASON3B::INTERFACE_PET);
			}
			if(IsVisible((SEASON3B::INTERFACE_QUEST_PROGRESS_ETC)))
			{
				g_pQuestProgressByEtc->SetPos(640-190, 0);
			}
		}
		else if (dwKey == SEASON3B::INTERFACE_ExpandInventory)
		{
			//g_pMyInventoryExt->Process();

			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_ExpandInventory, false);
		}
		else if(dwKey == SEASON3B::INTERFACE_INVENTORY)
		{
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_ExpandWarehouse, false);
			if (IsVisible(SEASON3B::INTERFACE_ExpandInventory))
			{
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_ExpandInventory, false);
			}
			if(IsVisible(SEASON3B::INTERFACE_MIXINVENTORY))
			{
				if(g_pMixInventory->ClosingProcess() == false)
					return;
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_MIXINVENTORY, false);
			}
#ifdef LEM_ADD_LUCKYITEM
			if(IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND))
			{
 				if(g_pLuckyItemWnd->ClosingProcess() == false)
 					return;
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_LUCKYITEMWND, false);
			}
#endif // LEM_ADD_LUCKYITEM
			if(IsVisible(SEASON3B::INTERFACE_NPCSHOP))
			{
				g_pNPCShop->ClosingProcess();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_NPCSHOP, false);
			}
			if(IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY))
			{
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_MYSHOP_INVENTORY, false);
			}
			if(IsVisible(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY))
			{
				g_pPurchaseShopInventory->ClosingProcess();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY, false);
			}
			if(IsVisible(SEASON3B::INTERFACE_STORAGE))
			{
				if (!g_pStorageInventory->ProcessClosing())
					return;
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_STORAGE, false);
			}
			if(IsVisible(SEASON3B::INTERFACE_TRADE))
			{
				g_pTrade->ProcessCloseBtn();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_TRADE, false);
			}

			if(IsVisible(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION))
			{
				m_pNewLuckyCoinRegistration->ClosingProcess();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION, false);
			}
			if(IsVisible(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN))
			{	
				m_pNewExchangeLuckyCoinWindow->ClosingProcess();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN, false);
			}

#ifdef LEM_ADD_LUCKYITEM
			if(IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND))
			{	
				m_pNewUILuckyItemWnd->ClosingProcess();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_LUCKYITEMWND, false);
			}
#endif // LEM_ADD_LUCKYITEM

			g_pMyInventory->SetPos(640-190, 0);
			g_pMyInventoryExt->SetPos(g_pMyInventory->GetPos().x - 190, 0);
			g_pMyInventory->ClosingProcess();
		}
		else if (dwKey == SEASON3B::INTERFACE_MIXINVENTORY)
		{
			if (g_pMixInventory->ClosingProcess() == false)
			{
				return;
			}
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY, false);
		}
		else if(dwKey == SEASON3B::INTERFACE_NPCSHOP)
		{
			g_pNPCShop->ClosingProcess();
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY, false);
		}
		else if(dwKey == SEASON3B::INTERFACE_MYSHOP_INVENTORY 
			 || dwKey == SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY )
		{
			if(dwKey == SEASON3B::INTERFACE_MYSHOP_INVENTORY)
			{
				g_pMyShopInventory->ClosingProcess();
			}
			else if(dwKey == SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY)
			{
				g_pPurchaseShopInventory->ClosingProcess();
			}
			g_pMyInventory->SetPos(640-190, 0);
			g_pMyInventoryExt->SetPos(g_pMyInventory->GetPos().x - 190, 0);
		}
		else if (dwKey == SEASON3B::INTERFACE_STORAGE)
		{
		
 			if (!g_pStorageInventory->ProcessClosing())
				return;
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY, false);
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_ExpandWarehouse, false);
		}
		else if( dwKey == SEASON3B::INTERFACE_PET )
		{
			m_pNewPetInfoWindow->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_PARTY )
		{
			m_pNewPartyInfoWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_MYQUEST)
		{
			m_pNewMyQuestInfoWindow->ClosingProcess();

			m_pNewMyQuestInfoWindow->SetPos(640-190, 0);
		}
		else if( dwKey == SEASON3B::INTERFACE_SENATUS )
		{
			m_pNewCastleWindow->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_GUARDSMAN )
		{
			m_pNewGuardWindow->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_GATEKEEPER )
		{
			m_pNewGatemanWindow->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_GATESWITCH )
		{
			m_pNewGateSwitchWindow->ClosingProcess();
		}
		else if (dwKey == SEASON3B::INTERFACE_NPCQUEST)
		{
			m_pNewNPCQuest->ProcessClosing();
		}
		else if( dwKey == SEASON3B::INTERFACE_BLOODCASTLE )
		{
			g_pEnterBloodCastle->ClosingProcess( );
		} 
		else if( dwKey == SEASON3B::INTERFACE_DEVILSQUARE )
		{
			g_pEnterDevilSquare->ClosingProcess( );
		} 
		else if( dwKey == SEASON3B::INTERFACE_BLOODCASTLE_TIME )
		{
			g_pBloodCastle->ClosingProcess( );
		} 
		else if (dwKey == SEASON3B::INTERFACE_TRADE)
		{
 			g_pTrade->ProcessClosing();
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY, false);
		}
		else if(dwKey == SEASON3B::INTERFACE_CATAPULT)
		{
			g_pCatapultWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CHAOSCASTLE_TIME)
		{
			m_pNewChaosCastleTime->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_COMMAND )
		{
			m_pNewCommandWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_WINDOW_MENU)
		{
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_WINDOW, false);
		}
		else if(dwKey == SEASON3B::INTERFACE_OPTION)
		{
			g_pOption->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_HERO_POSITION_INFO )
		{
			m_pNewHeroPositionInfo->ClosingProcess();
		}
		else if (dwKey == SEASON3B::INTERFACE_BCUSTOMMENU)
		{
			m_pNewBCustomMenuInfo->ClosingProcess();
		}
		else if (dwKey == SEASON3B::INTERFACE_MuHelper)
		{
			m_pNewUIMuHelper->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_HELP)
		{
			g_pHelp->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_ITEM_EXPLANATION)
		{
			g_pItemExplanation->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_SETITEM_EXPLANATION)
		{
			g_pSetItemExplanation->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_QUICK_COMMAND)
		{
			g_pQuickCommand->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_MOVEMAP)
		{
			m_pNewCommandWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CHATINPUTBOX)
		{
			m_pNewChatInputBox->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_GUILDINFO)
		{
			m_pNewGuildInfoWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_NPCGUILDMASTER)
		{
			m_pNewGuildMakeWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_SIEGEWARFARE)
		{
			m_pNewSiegeWarfare->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_ITEM_ENDURANCE_INFO )
		{
			m_pNewItemEnduranceInfo->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_BUFF_WINDOW)
		{
			m_pNewBuffWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CURSEDTEMPLE_RESULT)
		{
			m_pNewCursedTempleResultWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CRYWOLF)
		{
			m_pNewCryWolfInterface->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_GOLD_BOWMAN )
		{
			m_pNewGoldBowman->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_GOLD_BOWMAN_LENA )
		{
			m_pNewGoldBowmanLena->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION)
		{
			m_pNewLuckyCoinRegistration->ClosingProcess();
			
			if(IsVisible(SEASON3B::INTERFACE_INVENTORY))
			{
				m_pNewMyInventory->ClosingProcess();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY, false);
			}
		}
		else if(dwKey == SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN)
		{
			m_pNewExchangeLuckyCoinWindow->ClosingProcess();
			
			if(IsVisible(SEASON3B::INTERFACE_INVENTORY))
			{
				m_pNewMyInventory->ClosingProcess();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY, false);
			}
		}
		else if( dwKey == SEASON3B::INTERFACE_DUELWATCH )
		{
			m_pNewDuelWatchWindow->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_DUELWATCH_MAINFRAME )
		{
			m_pNewDuelWatchMainFrameWindow->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_DUELWATCH_USERLIST )
		{
			m_pNewDuelWatchUserListWindow->ClosingProcess();
		}
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
		else if(dwKey == SEASON3B::INTERFACE_INGAMESHOP)
		{
			g_pInGameShop->ClosingProcess();
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, false);
		}
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
		else if( dwKey == SEASON3B::INTERFACE_DOPPELGANGER_NPC )
		{
			m_pNewDoppelGangerWindow->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_DOPPELGANGER_FRAME )
		{
			m_pNewDoppelGangerFrame->ClosingProcess();
		}
		else if (dwKey == SEASON3B::INTERFACE_NPC_DIALOGUE)
		{
			m_pNewNPCDialogue->ProcessClosing();
		}
		else if (dwKey == SEASON3B::INTERFACE_QUEST_PROGRESS)
		{
			m_pNewQuestProgress->ProcessClosing();
		}
		else if (dwKey == SEASON3B::INTERFACE_QUEST_PROGRESS_ETC)
		{
			m_pNewQuestProgressByEtc->ProcessClosing();
		}
		else if( dwKey == SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC )
		{
			m_pNewEmpireGuardianNPC->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER)
		{
			m_pNewEmpireGuardianTimer->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_MINI_MAP)
		{
			m_pNewMiniMap->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_GENSRANKING)
		{
			g_pNewUIGensRanking->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA )
		{
			m_pNewUnitedMarketPlaceWindow->ClosingProcess();
		}
#ifdef LEM_ADD_LUCKYITEM
		else if( dwKey == SEASON3B::INTERFACE_LUCKYITEMWND )
		{
			if( g_pLuckyItemWnd->ClosingProcess() == false )
				return;
			if(IsVisible(SEASON3B::INTERFACE_INVENTORY))
			{
				m_pNewMyInventory->ClosingProcess();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY, false);
			}
		}
#endif // LEM_ADD_LUCKYITEM

		m_pNewUIMng->ShowInterface(dwKey, false);

		int iScreenWidth = BGetScreenWidth();
		m_pNewItemEnduranceInfo->SetPos( iScreenWidth );
		m_pNewBuffWindow->SetPos(iScreenWidth);
		m_pNewPartyListWindow->SetPos( iScreenWidth );

		//===Fix Ware
		if (IsVisible(SEASON3B::INTERFACE_STORAGE) && IsVisible(SEASON3B::INTERFACE_ExpandInventory))
		{
			g_pStorageInventory->SetPos(g_pMyInventoryExt->GetPos().x - 190, 0);
			//g_pStorageInventoryExt->SetPos(g_pMyInventoryExt->GetPos().x - (190*2), 0);

		}
		else
		{
			g_pStorageInventory->SetPos(g_pMyInventory->GetPos().x - 190, 0);
			//g_pStorageInventoryExt->SetPos(g_pMyInventory->GetPos().x - (190 * 2), 0);
		}
		//===Fix Shop
		if (IsVisible(SEASON3B::INTERFACE_NPCSHOP) && IsVisible(SEASON3B::INTERFACE_ExpandInventory))
		{
			g_pNPCShop->SetPos(g_pMyInventoryExt->GetPos().x - 190, 0);

		}
		else
		{
			g_pNPCShop->SetPos(g_pMyInventory->GetPos().x - 190, 0);
		}
		//==Fix Trade
		if (IsVisible(SEASON3B::INTERFACE_TRADE) && IsVisible(SEASON3B::INTERFACE_ExpandInventory))
		{
			g_pTrade->SetPos(g_pMyInventoryExt->GetPos().x - 190, 0);

		}
		else
		{
			g_pTrade->SetPos(g_pMyInventory->GetPos().x - 190, 0);
		}
		if (IsVisible(SEASON3B::INTERFACE_ExpandInventory) && !IsVisible(SEASON3B::INTERFACE_INVENTORY))
		{
			g_pMyInventoryExt->Show(false);		
		}
	}
}

void SEASON3B::CNewUISystem::Toggle(DWORD dwKey) 
{ 
	IsVisible(dwKey) ? Hide(dwKey) : Show(dwKey); 
}

void SEASON3B::CNewUISystem::HideAll()
{
	if(m_pNewUIMng)
	{
		for(int i=SEASON3B::INTERFACE_BEGIN+1; i<INTERFACE_END; i++)
		{
			if(IsImpossibleHideInterface(i) == false)
			{
				if(IsVisible(i) == true)
				{
					Hide(i);
				}
			}
		}
	}
}

void SEASON3B::CNewUISystem::HideAllGroupA()
{
	Hide(SEASON3B::INTERFACE_INVENTORY);
	Hide(SEASON3B::INTERFACE_CHARACTER);

	DWORD dwGroupA[] = 
	{ 
		//SEASON3B::INTERFACE_INVENTORY,
		//SEASON3B::INTERFACE_CHARACTER,			
		//SEASON3B::INTERFACE_WINDOW_MENU,

		SEASON3B::INTERFACE_MIXINVENTORY,
		SEASON3B::INTERFACE_STORAGE,
		SEASON3B::INTERFACE_NPCSHOP,
		SEASON3B::INTERFACE_MYSHOP_INVENTORY,
		SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY,
		SEASON3B::INTERFACE_PET,
		SEASON3B::INTERFACE_MYQUEST,
		SEASON3B::INTERFACE_NPCQUEST,
		SEASON3B::INTERFACE_PARTY,
		SEASON3B::INTERFACE_SENATUS,
		SEASON3B::INTERFACE_GUARDSMAN,
		SEASON3B::INTERFACE_COMMAND,
		SEASON3B::INTERFACE_GUILDINFO,
		SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC,
		SEASON3B::INTERFACE_DUELWATCH,
		SEASON3B::INTERFACE_DOPPELGANGER_NPC,
		//SEASON3B::INTERFACE_HELP,
		//SEASON3B::INTERFACE_ITEM_EXPLANATION,
		//SEASON3B::INTERFACE_SETITEM_EXPLANATION,
		SEASON3B::INTERFACE_GOLD_BOWMAN,
		SEASON3B::INTERFACE_GOLD_BOWMAN_LENA,
		SEASON3B::INTERFACE_NPC_DIALOGUE,
		SEASON3B::INTERFACE_QUEST_PROGRESS,
		SEASON3B::INTERFACE_QUEST_PROGRESS_ETC,
		SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC,
		SEASON3B::INTERFACE_MuHelper,
		SEASON3B::INTERFACE_ExpandInventory,
		SEASON3B::INTERFACE_ExpandWarehouse,
#ifdef PBG_MOD_STAMINA_UI
		SEASON3B::INTERFACE_STAMINA_GAUGE,
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
		SEASON3B::INTERFACE_GENSRANKING,
#endif //PBG_ADD_GENSRANKING
		SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA,

#ifdef LEM_ADD_LUCKYITEM
		SEASON3B::INTERFACE_LUCKYITEMWND,
#endif // LEM_ADD_LUCKYITEM

		0,
	};

	if(m_pNewUIMng)
	{
		for(int i=0; dwGroupA[i] != 0; i++)
		{
			m_pNewUIMng->ShowInterface(dwGroupA[i], false);
		}
	}
}

void SEASON3B::CNewUISystem::HideAllGroupB()
{
	Hide(SEASON3B::INTERFACE_FRIEND);
	Hide(SEASON3B::INTERFACE_INVENTORY);
	Hide(SEASON3B::INTERFACE_CHARACTER);

	DWORD dwGroupB[] = 
	{ 
		//SEASON3B::INTERFACE_FRIEND,
		//SEASON3B::INTERFACE_INVENTORY,
		//SEASON3B::INTERFACE_CHARACTER,
		//SEASON3B::INTERFACE_WINDOW_MENU,

		SEASON3B::INTERFACE_MIXINVENTORY,
		SEASON3B::INTERFACE_STORAGE,
		SEASON3B::INTERFACE_NPCSHOP,
		SEASON3B::INTERFACE_MYSHOP_INVENTORY,
		SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY,
		SEASON3B::INTERFACE_PET,
		SEASON3B::INTERFACE_MYQUEST,
		SEASON3B::INTERFACE_NPCQUEST,
		SEASON3B::INTERFACE_PARTY,
		SEASON3B::INTERFACE_SENATUS,
		SEASON3B::INTERFACE_GUARDSMAN,
		SEASON3B::INTERFACE_COMMAND,
		SEASON3B::INTERFACE_GUILDINFO,
		SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC,
		SEASON3B::INTERFACE_CURSEDTEMPLE_NPC,
		SEASON3B::INTERFACE_DUELWATCH,
		SEASON3B::INTERFACE_DOPPELGANGER_NPC,
		//SEASON3B::INTERFACE_HELP,
		//SEASON3B::INTERFACE_ITEM_EXPLANATION,
		//SEASON3B::INTERFACE_SETITEM_EXPLANATION,	
		SEASON3B::INTERFACE_GOLD_BOWMAN,
		SEASON3B::INTERFACE_GOLD_BOWMAN_LENA,
		SEASON3B::INTERFACE_NPC_DIALOGUE,
		SEASON3B::INTERFACE_QUEST_PROGRESS,
		SEASON3B::INTERFACE_QUEST_PROGRESS_ETC,
		SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC,
		SEASON3B::INTERFACE_MuHelper,
		SEASON3B::INTERFACE_ExpandInventory,
		SEASON3B::INTERFACE_ExpandWarehouse,
#ifdef PBG_MOD_STAMINA_UI
		SEASON3B::INTERFACE_STAMINA_GAUGE,
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
		SEASON3B::INTERFACE_GENSRANKING,
#endif //PBG_ADD_GENSRANKING
		SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA,
#ifdef LEM_ADD_LUCKYITEM
		SEASON3B::INTERFACE_LUCKYITEMWND,
#endif // LEM_ADD_LUCKYITEM

		0, 
	};
	
	if(m_pNewUIMng)
	{
		for(int i=0; dwGroupB[i] != 0; i++)
		{
			m_pNewUIMng->ShowInterface(dwGroupB[i], false);
		}
	}
}
void SEASON3B::CNewUISystem::HideGroupBeforeOpenInterface()
{
	DWORD dwGroupC[] = 
	{ 
		SEASON3B::INTERFACE_PARTY,
		SEASON3B::INTERFACE_COMMAND,
		SEASON3B::INTERFACE_GUILDINFO,
		SEASON3B::INTERFACE_GOLD_BOWMAN,
		SEASON3B::INTERFACE_GOLD_BOWMAN_LENA,
		SEASON3B::INTERFACE_GENSRANKING,
		0, 
	};
	
	if(m_pNewUIMng)
	{
		for(int i=0; dwGroupC[i] != 0; i++)
		{
			m_pNewUIMng->ShowInterface(dwGroupC[i], false);
		}
	}
}

void SEASON3B::CNewUISystem::Enable(DWORD dwKey)
{
	if(m_pNewUIMng)
	{
		m_pNewUIMng->EnableInterface(dwKey);
	}
}

void SEASON3B::CNewUISystem::Disable(DWORD dwKey)
{
	if(m_pNewUIMng)
	{
		m_pNewUIMng->EnableInterface(dwKey, false);
	}
}

bool SEASON3B::CNewUISystem::CheckMouseUse()
{
	if(m_pNewUIMng)
	{
		if(m_pNewUIMng->GetActiveMouseUIObj())
			return true;
	}
	return false;
}

bool SEASON3B::CNewUISystem::CheckKeyUse()
{
	if(m_pNewUIMng)
	{
		if(m_pNewUIMng->GetActiveKeyUIObj())
			return true;
	}
	return false;
}

bool SEASON3B::CNewUISystem::Update()
{
	if(m_pNewItemMng)
	{
		m_pNewItemMng->Update();
	}

	if(m_pNewUIMng)
	{
		m_pNewUIMng->UpdateMouseEvent();
		m_pNewUIMng->UpdateKeyEvent();
		return m_pNewUIMng->Update();
	}
	return false;
}

bool SEASON3B::CNewUISystem::Render()
{
	bool bResult = false;

	BeginBitmap();
	if(m_pNewUIMng)
		bResult = m_pNewUIMng->Render();
	EndBitmap();
	
	return bResult;
}

CNewUIManager* SEASON3B::CNewUISystem::GetNewUIManager() const
{ 
	return m_pNewUIMng; 
}

CNewUI3DRenderMng* SEASON3B::CNewUISystem::GetNewUI3DRenderMng() const
{ 
	return m_pNewUI3DRenderMng; 
}


CNewUIHotKey* SEASON3B::CNewUISystem::GetNewUIHotKey() const
{
	return m_pNewUIHotKey;
}

bool SEASON3B::CNewUISystem::IsImpossibleSendMoveInterface()
{
	if(IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)
		|| IsVisible(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC)
#ifdef LEM_ADD_LUCKYITEM
		|| IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
#endif // LEM_ADD_LUCKYITEM
		)
	{
		return true;
	}

	return false;
}

bool SEASON3B::CNewUISystem::IsImpossibleTradeInterface()
{
	if(IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)
		|| IsVisible(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC)
		|| IsVisible(SEASON3B::INTERFACE_STORAGE)
		|| IsVisible(SEASON3B::INTERFACE_INGAMESHOP)
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
		|| IsVisible(SEASON3B::INTERFACE_INGAMESHOP)
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#ifdef LEM_ADD_LUCKYITEM
		|| IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
#endif // LEM_ADD_LUCKYITEM
	  )
	{
		return true;
	}

	return false;
}

bool SEASON3B::CNewUISystem::IsImpossibleDuelInterface()
{
	if(IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)
		|| IsVisible(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC)
		|| IsVisible(SEASON3B::INTERFACE_STORAGE)
		|| IsVisible(SEASON3B::INTERFACE_INGAMESHOP)
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
		|| IsVisible(SEASON3B::INTERFACE_INGAMESHOP)
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#ifdef LEM_ADD_LUCKYITEM
		|| IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)				
#endif // LEM_ADD_LUCKYITEM
	  )
	{
		return true;
	}

	return false;
}

bool SEASON3B::CNewUISystem::IsImpossibleHideInterface(DWORD dwKey)
{
	if(dwKey == SEASON3B::INTERFACE_MAINFRAME
		|| dwKey == SEASON3B::INTERFACE_SKILL_LIST
		|| dwKey == SEASON3B::INTERFACE_SLIDEWINDOW
		|| dwKey == SEASON3B::INTERFACE_MESSAGEBOX
		|| dwKey == SEASON3B::INTERFACE_CHATLOGWINDOW
		|| dwKey == SEASON3B::INTERFACE_PARTY_INFO_WINDOW
		|| dwKey == SEASON3B::INTERFACE_KANTURU_INFO
		|| dwKey == SEASON3B::INTERFACE_BLOODCASTLE_TIME
		|| dwKey == SEASON3B::INTERFACE_CHAOSCASTLE_TIME
		|| dwKey == SEASON3B::INTERFACE_BATTLE_SOCCER_SCORE
		|| dwKey == SEASON3B::INTERFACE_DUEL_WINDOW
		|| dwKey == SEASON3B::INTERFACE_CRYWOLF
		|| dwKey == SEASON3B::INTERFACE_HERO_POSITION_INFO
		|| dwKey == SEASON3B::INTERFACE_BCUSTOMMENU
		|| dwKey == SEASON3B::INTERFACE_MuHelper
		|| dwKey == SEASON3B::INTERFACE_NAME_WINDOW
		|| dwKey == SEASON3B::INTERFACE_SIEGEWARFARE
		|| dwKey == SEASON3B::INTERFACE_ITEM_TOOLTIP
		|| dwKey == SEASON3B::INTERFACE_HOTKEY
		|| dwKey == SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM
		|| dwKey == SEASON3B::INTERFACE_ITEM_ENDURANCE_INFO
		|| dwKey == SEASON3B::INTERFACE_BUFF_WINDOW
		|| (dwKey >= SEASON3B::INTERFACE_3DRENDERING_CAMERA_BEGIN && dwKey <= SEASON3B::INTERFACE_3DRENDERING_CAMERA_END)
		|| dwKey == SEASON3B::INTERFACE_DUELWATCH_MAINFRAME
		|| dwKey == SEASON3B::INTERFACE_DUELWATCH_USERLIST
		|| dwKey == SEASON3B::INTERFACE_DOPPELGANGER_FRAME
		|| dwKey == SEASON3B::INTERFACE_GOLD_BOWMAN
		|| dwKey == SEASON3B::INTERFACE_GOLD_BOWMAN_LENA
		|| dwKey == SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER
		)
	{
		return true;
	}
	
	return false;
}

void SEASON3B::CNewUISystem::RenderItem3DFree(float sx, float sy, float Width, float Height, int Type, int Level, int Option1, int ExtOption, bool PickUp, float Scale, bool FixY)
{
	float x = sx;
	float y = sy;
	if (FixY == true)
	{
		if (Type >= BConverITEM(0, 0) && Type < BConverITEM(1, 0))
		{
			x = sx - 5;
			if (Type == BConverITEM(0, 16) || Type == BConverITEM(0, 19)) { y = sy + 30; }
			else if ((Type >= BConverITEM(0, 32) && Type <= BConverITEM(0, 35)) /*|| gCustomBattleGloves.CheckGloves(Type) == true*/) { y = sy + 15; }
			else if (Type >= BConverITEM(0, 15) && Type < BConverITEM(0, 32)) { y = sy + 25; }
			else if (Type > BConverITEM(0, 35)) { y = sy + 20; }
			else if (Type == BConverITEM(0, 0)) { y = sy + 30; }
			else { y = sy + 20; }
		}
		else if (Type >= BConverITEM(3, 0) && Type < BConverITEM(4, 0))
		{
			y = sy + 30;
		}
		else if (Type >= BConverITEM(7, 0) && Type < BConverITEM(8, 0)) { y = sy + 25; }
		else if (Type >= BConverITEM(8, 0) && Type < BConverITEM(9, 0))
		{
			if ((Type >= BConverITEM(8, 2) && Type <= BConverITEM(8, 4))
				|| Type == BConverITEM(8, 7) || Type == BConverITEM(8, 8)
				|| Type == BConverITEM(8, 15))
			{
				y = sy + 20;
			}
			else if (Type == BConverITEM(8, 34) || Type == BConverITEM(8, 35)) { y = sy + 25; }
			else if (Type <= BConverITEM(8, 73)) { y = sy + 25; }
			else if (Type > BConverITEM(8, 73)) { y = sy + 35; }
		}
		else if (Type >= BConverITEM(9, 0) && Type < BConverITEM(12, 0)) { y = sy + 25; }
		else if (Type >= 2048)
		{
			y = sy + 25;

			if (Type == BConverITEM(12, 40)) { y = sy + 10; }
		}
		else { y = sy + 20; }
	}
	//EndBitmap();
	EnableAlphaTest();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glViewport2(0, 0, WindowWidth, WindowHeight);
	//DAT Fix Size Item Wide ITEM NPC
	float SizeItem = Scale;
	//if (m_Resolution >= 3) { SizeItem += 0.70f; }
	//if (m_Resolution >= 3) { SizeItem += 0.20f; }
	switch ((int)m_Resolution)
	{
	case 3: SizeItem += 0.30f; break;	//1280x1024
	case 4: SizeItem += 0.40f; break;	//1366x768
	case 5: SizeItem += 0.50f; break;	//1440x900
	case 6: SizeItem += 0.60f; break;	//1600x900
	case 7: SizeItem += 0.70f; break;	//1680x1050
	case 8: SizeItem += 0.80f; break;	//1920x1080
	}
	gluPerspective2(SizeItem, (float)(WindowWidth) / (float)(WindowHeight), RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	GetOpenGLMatrix(CameraMatrix);
	EnableDepthTest();
	EnableDepthMask();

	glClear(GL_DEPTH_BUFFER_BIT);

	//	int Type = ITEM_POTION + 21;
		//int Level = 0;
		//float x = 640.f - 120.f;
		//float y = 200.f;
	float BWidth = (float)ItemAttribute[Type].Width * INVENTORY_SCALE;
	float BHeight = (float)ItemAttribute[Type].Height * INVENTORY_SCALE;
	RenderItem3D(x, y, Width, Height, Type, Level, Option1, ExtOption, PickUp);

	UpdateMousePositionn();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	DisableDepthTest();
	DisableAlphaBlend();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	//BeginBitmap();
}
/*
	//===Render Khung Custom
	EnableAlphaTest();

	int iPos_x = 50;
	int iPos_y = 50;
	float iWidth = 100;
	float iHeight = 50;
	int iTabNum = 3;
	int iSelectNum = 12;

	SetLineColor(5, 0.5f);
	RenderColor((float)iPos_x + 4, (float)iPos_y + 4, (float)iWidth - 8, (float)iHeight - 8); //Nen

	SetLineColor(0);
	RenderColor((float)iPos_x, (float)iPos_y, (float)iWidth, (float)1);
	SetLineColor(1);
	RenderColor((float)iPos_x, (float)iPos_y + 1, (float)iWidth, (float)3);
	SetLineColor(2);
	RenderColor((float)iPos_x, (float)iPos_y + 4, (float)iWidth, (float)1);

	SetLineColor(2);
	RenderColor((float)iPos_x, (float)iPos_y + iHeight - 5, (float)iWidth, (float)1);
	SetLineColor(1);
	RenderColor((float)iPos_x, (float)iPos_y + iHeight - 4, (float)iWidth, (float)3);
	SetLineColor(0);
	RenderColor((float)iPos_x, (float)iPos_y + iHeight - 1, (float)iWidth, (float)1);

	SetLineColor(0);
	RenderColor((float)iPos_x, (float)iPos_y, (float)1, (float)iHeight);
	SetLineColor(1);
	RenderColor((float)iPos_x + 1, (float)iPos_y + 5, (float)3, (float)iHeight - 10);
	SetLineColor(2);
	RenderColor((float)iPos_x + 4, (float)iPos_y + 1, (float)1, (float)iHeight - 2);

	SetLineColor(2);
	RenderColor((float)iPos_x + iWidth - 5, (float)iPos_y + 1, (float)1, (float)iHeight - 2);
	SetLineColor(1);
	RenderColor((float)iPos_x + iWidth - 4, (float)iPos_y + 5, (float)3, (float)iHeight - 10);
	SetLineColor(0);
	RenderColor((float)iPos_x + iWidth - 1, (float)iPos_y, (float)1, (float)iHeight);

	SetLineColor(4);
	RenderColor((float)iPos_x + 2, (float)iPos_y + 2, (float)1, (float)1);
	RenderColor((float)iPos_x + iWidth - 3, (float)iPos_y + 2, (float)1, (float)1);
	RenderColor((float)iPos_x + 2, (float)iPos_y + iHeight - 3, (float)1, (float)1);

	EndRenderColor();
	//============
*/
//---------------------------------------------------------------------------------------------

void SEASON3B::CNewUISystem::UpdateSendMoveInterface()
{
	if (IsVisible(SEASON3B::INTERFACE_TRADE))
	{
		SendRequestTradeExit();
		Hide(SEASON3B::INTERFACE_TRADE);
	}
	if (IsVisible(SEASON3B::INTERFACE_STORAGE))
	{
		Hide(SEASON3B::INTERFACE_STORAGE);
	}
	if (IsVisible(SEASON3B::INTERFACE_NPCGUILDMASTER))
	{
		Hide(SEASON3B::INTERFACE_NPCGUILDMASTER);
	}
	if (IsVisible(SEASON3B::INTERFACE_MYQUEST))
	{
		Hide(SEASON3B::INTERFACE_MYQUEST);
	}
	if (IsVisible(SEASON3B::INTERFACE_NPCQUEST))
	{
		Hide(SEASON3B::INTERFACE_NPCQUEST);
	}
	if (IsVisible(SEASON3B::INTERFACE_NPCSHOP))
	{
		Hide(SEASON3B::INTERFACE_NPCSHOP);
	}
	if (IsVisible(SEASON3B::INTERFACE_GUARDSMAN))
	{
		Hide(SEASON3B::INTERFACE_GUARDSMAN);
	}
	if (IsVisible(SEASON3B::INTERFACE_GUARDSMAN))
	{
		Hide(SEASON3B::INTERFACE_GUARDSMAN);
	}
	if (IsVisible(SEASON3B::INTERFACE_DEVILSQUARE))
	{
		Hide(SEASON3B::INTERFACE_DEVILSQUARE);
	}
	if (IsVisible(SEASON3B::INTERFACE_BLOODCASTLE))
	{
		Hide(SEASON3B::INTERFACE_BLOODCASTLE);
	}
	if (IsVisible(SEASON3B::INTERFACE_CURSEDTEMPLE_NPC))
	{
		Hide(SEASON3B::INTERFACE_CURSEDTEMPLE_NPC);
	}
	if (IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY))
	{
		Hide(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
	}
	if (IsVisible(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY))
	{
		Hide(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY);
	}
	if (IsVisible(SEASON3B::INTERFACE_DUELWATCH))
	{
		Hide(SEASON3B::INTERFACE_DUELWATCH);
	}
	if (IsVisible(SEASON3B::INTERFACE_DOPPELGANGER_NPC))
	{
		Hide(SEASON3B::INTERFACE_DOPPELGANGER_NPC);
	}
	if (IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE))
	{
		Hide(SEASON3B::INTERFACE_NPC_DIALOGUE);
	}
	if (IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS))
	{
		Hide(SEASON3B::INTERFACE_QUEST_PROGRESS);
	}
	if (IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
	{
		Hide(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC);
	}
	if(IsVisible(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC))
	{
		Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC);
	}
	if(IsVisible(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION))
	{
		Hide(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION);
	}
	if(IsVisible(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN))
	{
		Hide(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN);
	}
	if(IsVisible(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA))
	{
		Hide(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA);
	}
}

// GetInstance()
CNewUISystem* SEASON3B::CNewUISystem::GetInstance()
{
	static CNewUISystem s_NewUISystem;
	return &s_NewUISystem;
}

CNewUIChatLogWindow* SEASON3B::CNewUISystem::GetUI_NewChatLogWindow() const
{ 
	return m_pNewChatLogWindow; 
}

CNewUISlideWindow* SEASON3B::CNewUISystem::GetUI_NewSlideWindow() const
{ 
	return m_pNewSlideWindow; 
}

CNewUIFriendWindow* SEASON3B::CNewUISystem::GetUI_NewFriendWindow() const
{ 
	return m_pNewFriendWindow; 
}

CNewUIMainFrameWindow* SEASON3B::CNewUISystem::GetUI_NewMainFrameWindow() const
{
	return m_pNewMainFrameWindow;
}

CNewUISkillList* SEASON3B::CNewUISystem::GetUI_NewSkillList() const
{
	return m_pNewSkillList;
}

CNewUIChatInputBox* SEASON3B::CNewUISystem::GetUI_NewChatInputBox() const
{
	return m_pNewChatInputBox;
}

CNewUIItemMng* SEASON3B::CNewUISystem::GetUI_NewItemMng() const
{
	return m_pNewItemMng;
}

CNewUIMyInventory* SEASON3B::CNewUISystem::GetUI_NewMyInventory() const
{ 
	return m_pNewMyInventory; 
}
CNewUIInventoryExtension* CNewUISystem::GetUI_NewMyInventoryExt() const
{
	return m_pNewMyInventoryExt;
}
CNewUINPCShop* SEASON3B::CNewUISystem::GetUI_NewNpcShop() const
{
	return m_pNewNPCShop;
}

CNewUIPetInfoWindow* SEASON3B::CNewUISystem::GetUI_NewPetInfoWindow() const
{
	return m_pNewPetInfoWindow;
}

CNewUIMixInventory* SEASON3B::CNewUISystem::GetUI_NewMixInventory() const
{ 
	return m_pNewMixInventory; 
}

CNewUICastleWindow* SEASON3B::CNewUISystem::GetUI_NewCastleWindow() const
{ 
	return m_pNewCastleWindow; 
}

CNewUIGuardWindow* SEASON3B::CNewUISystem::GetUI_NewGuardWindow() const
{ 
	return m_pNewGuardWindow; 
}

CNewUIGatemanWindow* SEASON3B::CNewUISystem::GetUI_NewGatemanWindow() const
{ 
	return m_pNewGatemanWindow; 
}

CNewUIGateSwitchWindow* SEASON3B::CNewUISystem::GetUI_NewGateSwitchWindow() const
{ 
	return m_pNewGateSwitchWindow; 
}

CNewUIStorageInventory* SEASON3B::CNewUISystem::GetUI_NewStorageInventory() const
{ 
	return m_pNewStorageInventory; 
}
CNewUIStorageInventoryExt* CNewUISystem::GetUI_NewStorageInventoryExt() const
{
	return m_pNewStorageInventoryExt;
}
CNewUIGuildMakeWindow* SEASON3B::CNewUISystem::GetUI_NewGuildMakeWindow() const
{ 
	return m_pNewGuildMakeWindow; 
}

CNewUIGuildInfoWindow* SEASON3B::CNewUISystem::GetUI_NewGuildInfoWindow() const
{ 
	return m_pNewGuildInfoWindow; 
}

CNewUICryWolf* SEASON3B::CNewUISystem::GetUI_NewCryWolfInterface() const
{
	return m_pNewCryWolfInterface;
}

//CNewUIMasterLevel* SEASON3B::CNewUISystem::GetUI_NewMasterLevelInterface() const
//{
//	return m_pNewMaster_Level_Interface;
//}

CNewUIMasterSkillTree* SEASON3B::CNewUISystem::GetUI_NewMasterSkillTreeInterface() const
{
	return m_pNewMaster_SkillTree_Interface;
}

CNewUIMyShopInventory* SEASON3B::CNewUISystem::GetUI_NewMyShopInventory() const
{ 
	return m_pNewMyShopInventory;
}

CNewUIPurchaseShopInventory* SEASON3B::CNewUISystem::GetUI_NewPurchaseShopInventory() const
{ 
	return m_pNewPurchaseShopInventory;
}

CNewUICharacterInfoWindow* SEASON3B::CNewUISystem::GetUI_NewCharacterInfoWindow() const
{
	return m_pNewCharacterInfoWindow;
}

CNewUIMyQuestInfoWindow* SEASON3B::CNewUISystem::GetUI_NewMyQuestInfoWindow() const
{
	return m_pNewMyQuestInfoWindow;
}

CNewUIPartyInfoWindow* SEASON3B::CNewUISystem::GetUI_NewPartyInfoWindow() const
{
	return m_pNewPartyInfoWindow;
}

CNewUIPartyListWindow* SEASON3B::CNewUISystem::GetUI_NewPartyListWindow() const
{
	return m_pNewPartyListWindow;
}

CNewUINPCQuest* SEASON3B::CNewUISystem::GetUI_NewNPCQuest() const
{
	return m_pNewNPCQuest;
}

CNewUIEnterBloodCastle* SEASON3B::CNewUISystem::GetUI_NewEnterBloodCastle( ) const
{
	return m_pNewEnterBloodCastle;
}

CNewUIEnterDevilSquare* SEASON3B::CNewUISystem::GetUI_NewEnterDevilSquare( ) const
{
	return m_pNewEnterDevilSquare;
}

CNewUIBloodCastle* SEASON3B::CNewUISystem::GetUI_NewBloodCastle( ) const
{
	return m_pNewBloodCastle;
}

CNewUITrade* SEASON3B::CNewUISystem::GetUI_NewTrade() const
{
	return m_pNewTrade;
}

CNewUIKanturu2ndEnterNpc* SEASON3B::CNewUISystem::GetUI_NewKanturu2ndEnterNpc() const
{
	return m_pNewKanturu2ndEnterNpc;
}

CNewUIKanturuInfoWindow* SEASON3B::CNewUISystem::GetUI_NewKanturuInfoWindow() const
{
	return m_pNewKanturuInfoWindow;
}

CNewUICatapultWindow* SEASON3B::CNewUISystem::GetUI_NewCatapultWindow() const
{
	return m_pNewCatapultWindow;
}

CNewUIChaosCastleTime* SEASON3B::CNewUISystem::GetUI_NewChaosCastleTime() const
{
	return m_pNewChaosCastleTime;
}

CNewUICommandWindow* SEASON3B::CNewUISystem::GetUI_NewCommandWindow() const
{
	return m_pNewCommandWindow;
}

CNewUIWindowMenu* SEASON3B::CNewUISystem::GetUI_NewWindowMenu() const
{
	return m_pNewWindowMenu;
}

CNewUIOptionWindow* SEASON3B::CNewUISystem::GetUI_NewOptionWindow() const
{
	return m_pNewOptionWindow;
}

CNewUIHeroPositionInfo* SEASON3B::CNewUISystem::GetUI_NewHeroPositionInfo() const
{
	return m_pNewHeroPositionInfo;
}
//===B Custom menu
CNewUIBCustomMenuInfo* SEASON3B::CNewUISystem::GetUI_NewBCustomMenuInfo() const
{
	return m_pNewBCustomMenuInfo;
}

//CNewUIMacro* SEASON3B::CNewUISystem::GetUI_NewBMacroInfo() const
//{
//	return m_pNewBMacroInfo;
//}
CNewUIHelpWindow* SEASON3B::CNewUISystem::GetUI_NewHelpWindow() const
{
	return m_pNewHelpWindow;
}

CNewUIItemExplanationWindow* SEASON3B::CNewUISystem::GetUI_NewItemExplanationWindow() const
{
	return m_pNewItemExplanationWindow;
}

CNewUISetItemExplanation* SEASON3B::CNewUISystem::GetUI_NewSetItemExplanation() const
{
	return m_pNewSetItemExplanation;
}

CNewUIQuickCommandWindow* SEASON3B::CNewUISystem::GetUI_NewQuickCommandWindow() const
{
	return m_pNewQuickCommandWindow;
}

CNewUIMoveCommandWindow* SEASON3B::CNewUISystem::GetUI_NewMoveCommandWindow() const
{
	return m_pNewMoveCommandWindow;
}

CNewUIBattleSoccerScore* SEASON3B::CNewUISystem::GetUI_NewBattleSoccerScore() const
{
	return m_pNewBattleSoccerScore;
}

CNewUIDuelWindow* SEASON3B::CNewUISystem::GetUI_NewDuelWindow() const
{
	return m_pNewDuelWindow;
}

CNewUISiegeWarfare* SEASON3B::CNewUISystem::GetUI_NewSiegeWarfare() const
{
	return m_pNewSiegeWarfare;
}

CNewUIItemEnduranceInfo* SEASON3B::CNewUISystem::GetUI_NewItemEnduranceInfo() const
{
	return m_pNewItemEnduranceInfo;
}

CNewUIBuffWindow* SEASON3B::CNewUISystem::GetUI_NewBuffWindow() const
{
	return m_pNewBuffWindow;
}

CNewUICursedTempleEnter* SEASON3B::CNewUISystem::GetUI_NewCursedTempleEnterWindow() const
{
	return m_pNewCursedTempleEnterWindow;
}

CNewUICursedTempleSystem* SEASON3B::CNewUISystem::GetUI_NewCursedTempleWindow() const
{
	return m_pNewCursedTempleWindow;
}

CNewUICursedTempleResult* SEASON3B::CNewUISystem::GetUI_NewCursedTempleResultWindow() const
{
	return m_pNewCursedTempleResultWindow;
}

CNewUIGoldBowmanWindow*	SEASON3B::CNewUISystem::GetUI_pNewGoldBowman() const
{
	return m_pNewGoldBowman;
}

CNewUIGoldBowmanLena*	SEASON3B::CNewUISystem::GetUI_pNewGoldBowmanLena() const
{
	return m_pNewGoldBowmanLena;
}

CNewUIRegistrationLuckyCoin* SEASON3B::CNewUISystem::GetUI_pNewLuckyCoinRegistration() const
{
	return m_pNewLuckyCoinRegistration;
}

CNewUIExchangeLuckyCoin* SEASON3B::CNewUISystem::GetUI_pNewExchangeLuckyCoin() const
{
	return m_pNewExchangeLuckyCoinWindow;
}

CNewUIMiniMap* SEASON3B::CNewUISystem::GetUI_pNewUIMiniMap() const
{
	return m_pNewMiniMap;
}

CNewUIDuelWatchWindow* SEASON3B::CNewUISystem::GetUI_pNewDuelWatch() const
{
	return m_pNewDuelWatchWindow;
}

CNewUIDuelWatchMainFrameWindow* SEASON3B::CNewUISystem::GetUI_pNewDuelWatchMainFrame() const
{
	return m_pNewDuelWatchMainFrameWindow;
}

CNewUIDuelWatchUserListWindow* SEASON3B::CNewUISystem::GetUI_pNewDuelWatchUserList() const
{
	return m_pNewDuelWatchUserListWindow;
}

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
CNewUIInGameShop* SEASON3B::CNewUISystem::GetUI_pNewInGameShop() const
{
	return m_pNewInGameShop;
}
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME

CNewUIDoppelGangerWindow* SEASON3B::CNewUISystem::GetUI_pNewDoppelGangerWindow() const
{
	return m_pNewDoppelGangerWindow;
}

CNewUIDoppelGangerFrame* SEASON3B::CNewUISystem::GetUI_pNewDoppelGangerFrame() const
{
	return m_pNewDoppelGangerFrame;
}

CNewUINPCDialogue* SEASON3B::CNewUISystem::GetUI_NewNPCDialogue() const
{
	return m_pNewNPCDialogue;
}

CNewUIQuestProgress* SEASON3B::CNewUISystem::GetUI_NewQuestProgress() const
{
	return m_pNewQuestProgress;
}

CNewUIQuestProgressByEtc* SEASON3B::CNewUISystem::GetUI_NewQuestProgressByEtc() const
{
	return m_pNewQuestProgressByEtc;
}

CNewUIEmpireGuardianNPC* SEASON3B::CNewUISystem::GetUI_pNewEmpireGuardianNPC() const
{
	return m_pNewEmpireGuardianNPC;
}

CNewUIEmpireGuardianTimer* SEASON3B::CNewUISystem::GetUI_pNewEmpireGuardianTimer() const
{
	return m_pNewEmpireGuardianTimer;
}

#ifdef PBG_MOD_STAMINA_UI
CNewUIStamina* SEASON3B::CNewUISystem::GetUI_pNewUIStamina() const
{
	return m_pNewUIStamina;
}
#endif //PBG_MOD_STAMINA_UI

#ifdef PBG_ADD_GENSRANKING
CNewUIGensRanking* SEASON3B::CNewUISystem::GetUI_NewGensRanking() const
{
	return m_pNewGensRanking;
}
#endif //PBG_ADD_GENSRANKING

CNewUIUnitedMarketPlaceWindow* SEASON3B::CNewUISystem::GetUI_pNewUnitedMarketPlaceWindow() const
{
	return m_pNewUnitedMarketPlaceWindow;
}
CNewUIMuHelper* CNewUISystem::Get_pNewUIMuHelper() const
{
	return m_pNewUIMuHelper;
}
#ifdef LEM_ADD_LUCKYITEM
CNewUILuckyItemWnd* SEASON3B::CNewUISystem::Get_pNewUILuckyItemWnd() const
{
	return m_pNewUILuckyItemWnd;
}
#endif // LEM_ADD_LUCKYITEM
