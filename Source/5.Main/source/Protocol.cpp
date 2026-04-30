#include "stdafx.h"
#include "Protocol.h"
#include "WSclient.h"
#include "NewUISystem.h"
#include "BuffIcon.h"
#include "PetProtocol.h"
#include "Reconnect.h"
#include "wsclientinline.h"
#include "SkillManager.h"
#include "CharacterManager.h"
#include "CustomEventTime.h"
#include "Ranking.h"
#include "WindowClass.h"
#include "CBChoTroi.h"
#include "CB_AutoNapGame.h"
#include "CustomRankUser.h"
#include "CB_DanhHieu.h"
#if(JEWELBANKVER2)
#include "CB_NewJewelBank.h"
#else
#include "CB_JewelBank.h"
#endif
#include "CB_BossGuild.h"
#include "CB_CTCMini.h"
#include "APICB.h"

#include "CustomRanking.h"
#include "CB_DangKyInGame.h"

#if(CB_CUSTOMOFFTRADE)
#include "CB_OffTrade.h"
#endif
#include "CB_AutoResetInfo.h"
#include "CBInterfaceVIPChar.h"
#include "GOBoid.h"
#include "CSChaosCastle.h"
#if(CB_VIEWCHARITEM)
#include "CB_ViewCharItem.h"
#endif
#if(CB_OFFMUHELPER)
#include "CB_OffMuHelper.h"
#endif
#include "CB_LockItem.h"
#if(CB_CUSTOMMIXINFO)
#include "CB_InfoCustomMix.h"
#endif
#if(CB_NEWQUESTFF)
#include "CB_NewQuest.h"
#endif
#if(CB_HUYDONGEXC)
#include "CB_HuyDongExc.h"
#endif
#if(CB_ActiveInvasionsD)
#include "CB_ActiveInvasions.h"
#endif
#include "CB_CongHuong.h"
#include "MocDonate.h"

#if (CB_GETMIXRATE)
#include "CB_GetMixRate.h"
#endif
#include "CB_StatsAdvance.h"
#include "CB_DrawCustomRank.h"
#include "CB_BotTrader.h"
#include "CB_BXHTopDmg.h"
#include "VongQuay.h"

extern int g_iLimitAttackTimeSet;
bool StatusAutoReset = false;
BOOL ProtocolCoreEx(BYTE head, BYTE* lpMsg, int size, int key) // OK
{
	switch (head)
	{
#if(CB_GETMIXRATE)
		case 0x88:
			if (gCB_GetMixRate) gCB_GetMixRate->GCRecvMixInfo(lpMsg, size);
			break;
#endif
		case 0x11:
			GCDamageRecv((PMSG_DAMAGE_RECV*)lpMsg);
			break;
		case 0x16:
			GCMonsterDieRecv((PMSG_MONSTER_DIE_RECV*)lpMsg);
			break;
		case 0x17:
			GCUserDieRecv((PMSG_USER_DIE_RECV*)lpMsg);
			break;
		case 0x26:
			GCLifeRecv((PMSG_LIFE_RECV*)lpMsg);
			break;

		case 0x27:
			GCManaRecv((PMSG_MANA_RECV*)lpMsg);
			break;
		case 0x2C:
			GCFruitResultRecv((PMSG_FRUIT_RESULT_RECV*)lpMsg);
			break;
		case 0x3F:
			switch (((lpMsg[0] == 0xC1) ? lpMsg[3] : lpMsg[4]))
			{
			case 0x10:
				//GCPShopTextChangeSend((PMSG_PSHOP_TEXT_CHANGE_RECV*)lpMsg);
				break;
			}
			break;
		//==Custom Packet
		case 0x4E: ///MuunSystem
			switch (((lpMsg[0] == 0xC1) ? lpMsg[3] : lpMsg[4]))
			{
#if(HAISLOTRING)
			case 0x0A:
				BReceiveInventory(lpMsg);
				return 1;
			case 0x0B:
				BReceiveDurability(lpMsg);
				return 1;
			case 0x0C:
				BReceiveDeleteInventory(lpMsg);
				return 1;
			case 0x0D:
				BReceiveItemChange(lpMsg);
				return 1;
#endif
			}
			break;

		//--post item
		case 0x78:
			RecvPostItem(lpMsg);
			break;
#if(RANKING_NEW == 1)
		case 0xFE:
			gRanking.RecvInfo((DGCharTop*)lpMsg);
			break;
#endif
		//-- Buff Icon
		case 0x2D:
			gIconBuff.Recv((PMSG_SEND_BUFF_ICON*)lpMsg);
			break;
		case 0x9C:
			//GCRewardExperienceRecv((PMSG_REWARD_EXPERIENCE_RECV*)lpMsg);
			break;
		case 0xA3:
			//GCQuestRewardRecv((PMSG_QUEST_REWARD_RECV*)lpMsg);
			break;
		case 0xB1:
			switch (((lpMsg[0] == 0xC1) ? lpMsg[3] : lpMsg[4]))
			{
			case 0x00:
				gIconBuff.DeleteAllBuff();
				//GCMapServerMoveRecv((PMSG_MAP_SERVER_MOVE_RECV*)lpMsg);
				break;
			case 0x01:
				gIconBuff.DeleteAllBuff();
				//GCMapServerMoveAuthRecv((PMSG_MAP_SERVER_MOVE_AUTH_RECV*)lpMsg);
				break;
			}
			break;
		case 0xF1:
			switch (((lpMsg[0] == 0xC1) ? lpMsg[3] : lpMsg[4]))
			{
			case 0x00:
				GCConnectClientRecv((PMSG_CONNECT_CLIENT_RECV*)lpMsg);
				break;
			}
			break;
		//===MUHelper
		case 0xAE:
			CGLoadDataMuHelper(lpMsg);
			break;
		case 0xBF:
			switch (((lpMsg[0] == 0xC1) ? lpMsg[3] : lpMsg[4]))
			{
				case 0x51:
					RecvStartMuHelper(lpMsg);
					break;
			}
			break;
		//====
		case 0xCC:
			switch (((lpMsg[0] == 0xC1) ? lpMsg[3] : lpMsg[4]))
			{
#if(BOSS_GUILD == 1)
			case 0x09:
				if(gCBBossGuild) gCBBossGuild->BossGuildRecv(lpMsg);
				break;
#endif
			}
			break;
		case 0xF3:
			switch (((lpMsg[0] == 0xC1) ? lpMsg[3] : lpMsg[4]))
			{
#if(BOSS_GUILD == 1)
			case 0x35: //Show Window Boss Guild
				if (gCBBossGuild) gCBBossGuild->RecvProtocol(lpMsg); // ok
				break;
				//case 0x36: //Show Window Boss Guild
				//	GCBossGuildTime((BOSSGUILDTIME_SENDCLIENT*)lpMsg); // ok
				//	break;
#endif
#if CTCMINI
			case 0x32: //Show Window CTC Mini
				gCBCTCMini.GCCTCMiniShowWindow(lpMsg);
				break;
			case 0x33: //Show Kill CTC Mini
				gCBCTCMini.GCCTCMiniKill(lpMsg);
				break;
#endif
			case 0x00:
				gIconBuff.DeleteAllBuff();
				ReceiveCharacterList(lpMsg);
				return 1;
				break;
				case 0x03:
					GCCharacterInfoRecv(lpMsg);
					break;
				case 0x04:
					GCCharacterRegenRecv((PMSG_CHARACTER_REGEN_RECV*)lpMsg);
					break;
				case 0x05:
					GCLevelUpRecv((PMSG_LEVEL_UP_RECV*)lpMsg);
					break;
				case 0x06:
					GCLevelUpPointRecv((PMSG_LEVEL_UP_POINT_RECV*)lpMsg);
					break;
				case 0x07:
					GCMonsterDamageRecv((PMSG_MONSTER_DAMAGE_RECV*)lpMsg);
					break;
				case 0x50:
					GCMasterInfoRecv((PMSG_MASTER_INFO_RECV*)lpMsg);
					break;
				case 0x51:
					GCMasterLevelUpRecv((PMSG_MASTER_LEVEL_UP_RECV*)lpMsg);
					break;
				case 0xE0:
				{
					GCNewCharacterInfoRecv(lpMsg);
					return 1;
				}
				break;

				case 0xE1:
				{
					GCNewCharacterCalcRecv(lpMsg);
					return 1;
				}
				break;
				//Get HP BAR
				case 0xE2:
					ReceiveNewHealthBar(lpMsg);
					break;
				case 0xEE:
					SetChaosBoxState((PMSG_SET_CHAOSBOX_STATE*)lpMsg);
					return 1;
				case 0xF1:
					GCPetCharSetRecv((PMSG_NEW_PET_CHARSET_RECV*)lpMsg);
					break;
				case 0xF2:
					GCPetCharSetSelectCharacterRecv((PMSG_NEW_SELECT_CHARACTER_CHARSET_RECV*)lpMsg);
					break;
				// ADVANCE STATS
				case 0xF3:
					if (gCB_StatsAdvance) gCB_StatsAdvance->Recv(lpMsg);
					return 1;
				// MasterSkill
				case 0xF4:
					//gCRenderEx700.RecvMasterSkill((PMSG_MASTER_SKILL_LIST_SEND*)lpMsg);
					return 1;
				case 0xE5:
					gCustomRankUser.GCReqRankLevelUser((SEND_COUNTLIST*) lpMsg);
					return 1;
				case 0xE6:
					if(gCustomRanking) gCustomRanking->GCReqRanking((CustomRanking::PMSG_CUSTOM_RANKING_RECV*)lpMsg);
					return 1;
				case 0xE7:
					if (gCustomRanking) gCustomRanking->GCReqRankingCount((CustomRanking::PMSG_CUSTOM_RANKING_COUNT_RECV*)lpMsg);
					return 1;
				case 0xE8:
					gCustomEventTime.GCReqEventTime((PMSG_CUSTOM_EVENTTIME_RECV*)lpMsg);
					return 1;
				case 0xE9:
					//gItemPrice.GCItemValueRecv((PMSG_ITEM_VALUE_RECV*)lpMsg);
					return 1;
				case 0xEA:
					GCRecvCoin((PMSG_COIN_RECV*)lpMsg);
					return 1;
#if(CB_CUSTOMOFFTRADE)
				case 0xEB:
					if(gCB_OffTrade) gCB_OffTrade->RecvPShop((CB_OffTrade::PMSG_OFFTRADE_RECV*)lpMsg);
					return 1;
				case 0xEC:
					if (gCB_OffTrade) gCB_OffTrade->PShopActiveRecv((CB_OffTrade::PMSG_SHOPACTIVE_RECV*)lpMsg);
					return 1;
#endif
				case 0xED:
					//GCBuyConfirmRecv((PMSG_ITEM_BUY_RECV*)lpMsg);
					break;
				case 0x13:
					gIconBuff.GC_BuffInfo((PMSG_SEND_BUFF_ICON_EX*)lpMsg);
					break;
				case 0xF9:
					//gMiniMap.RecvPartyInfo((MINIMAP_PARTY_INFO_RECV*)lpMsg);
					break;
			}
			break;
		case 0xFA:
			switch (((lpMsg[0] == 0xC1) ? lpMsg[3] : lpMsg[4]))
			{
#if (CongHuongV2)
				//Cong Huong
			case 0x15: //
			{
				//pDrawMessage("0x15", 1);
				if(gCB_CongHuong) gCB_CongHuong->CH_RecvInfoGSSend((CB_CongHuong::PMSG_GSSendInfoClient*)lpMsg);
				//gInterface.BEventSinhTon_Data((BEventSinhTon_UserData*)lpMsg);
			}
			break;
			case 0x14: // OPen NPC
			{
				XULY_CGPACKET* mRecv = (XULY_CGPACKET*)lpMsg;
				if (!mRecv->ThaoTac) return 1;
				if (GetTickCount() > gInterface.Data[eWindow_CongHuong].EventTick + 500)
				{
					gInterface.Data[eWindow_CongHuong].OnShow ^= 1;
					gInterface.Data[eWindow_CongHuong].EventTick = GetTickCount();
				}
			}
			break;
#endif
#if(CB_HUYDONGEXC)
			case 0x16:
				if(gCBHuyDongExc) gCBHuyDongExc->RecvProtocol(lpMsg);
				return 1;
				break;
#endif
#if(JEWELBANKVER2)
			case 0x19: //
			{
				//pDrawMessage("0x18", 1);
				if (!gCB_NewJewelBank) return 0;
				PMSG_NEW_HEALTH_BAR_RECV* pMsg = (PMSG_NEW_HEALTH_BAR_RECV*)lpMsg;
				gCB_NewJewelBank->mCListItemBank.clear();
				for (int n = 0; n < pMsg->count; n++)
				{
					USER_BANK_DATA* lpInfo = (USER_BANK_DATA*)(((BYTE*)lpMsg) + sizeof(PMSG_NEW_HEALTH_BAR_RECV) + (sizeof(USER_BANK_DATA) * n));
					USER_BANK_DATA info;
					info.ItemIndex = lpInfo->ItemIndex;
					info.ItemLevel = lpInfo->ItemLevel;
					info.ItemCount = lpInfo->ItemCount;
					info.AutoPick = lpInfo->AutoPick;
					gCB_NewJewelBank->mCListItemBank.push_back(info);
					//gInterface.DrawMessage(1, "Set InfoBank (%d) %d , %d  Count = %d (%d)", n, lpInfo->ItemIndex, lpInfo->ItemLevel, lpInfo->ItemCount, gCB_NewJewelBank.mCListItemBank.size());
				}
			}
			break;
#endif
			case 0x11:
				if(gCustomRanking) gCustomRanking->RankNameRev((CustomRanking::PMSG_RANK_INFO*)lpMsg);
				return 1;
				break;
			}
			break;
		case 0xFB:
			switch (((lpMsg[0] == 0xC1) ? lpMsg[3] : lpMsg[4]))
			{
			case 0x08:
				WindowClass.RecvData((PMSG_CHANGECLASS_DATA*)lpMsg);
				break;
			case 0x16:
				//UpdateLuckyWheel((ITEM_WIN_SEND*)lpMsg);
				break;
			case 0x17:
				//GCLuckyWheelActive((PMSG_LUCKYWHEELACTIVE_RECV*)lpMsg);
				break;
#if(!JEWELBANKVER2)
			case 0xE5: //-> Bank old
				gCBJewelBank.RecvProtocol(lpMsg);
				break;
#endif
			case 0xD4:
				//GCSetSetItemViewZone((PMSG_ITEMVIEW_RECV*)lpMsg);
				break;
			}
			break;

			//====Custom Dev
		case 0x0D:
		{
			PMSG_NOTICE_SEND* RecvlpMsg = (PMSG_NOTICE_SEND*)lpMsg;
			if (RecvlpMsg->type == 0xFF) //CUstomNotice Popup
			{
				gInterface.OpenMessageBox("Warning", RecvlpMsg->message);
				return 1;
			}
			if (RecvlpMsg->type == (0xD3 + 1))
			{
				Interface::DATA_NOTICEPK kill = { 0 };

				kill.Delay = 0;
				kill.Time = GetTickCount() + (RecvlpMsg->delay * 1000);
				kill.Color = RecvlpMsg->color;
				memcpy(kill.Mess, RecvlpMsg->message, sizeof(kill.Mess));

				if (gInterface.m_DataNoticePK.size() >= 8)
				{
					gInterface.m_DataNoticePK.erase(gInterface.m_DataNoticePK.begin());
				}
				gInterface.m_DataNoticePK.push_back(kill);

				return 1;
			}
		}
		break;
		case 0xD3:
			switch (((lpMsg[0] == 0xC1) ? lpMsg[3] : lpMsg[4]))
			{
			case 0x8A:
			{
				gVongQuay.GetListVQ(lpMsg);
			}
			break;
			case 0x8B:
			{
				gVongQuay.RecvListItemVQ(lpMsg);
			}
			break;
			case 0x8C:
			{
				gVongQuay.GetInfoVQ(lpMsg);
			}
			break;
#if(CB_BXHDMG)
			case 0x2D: //Recv List
			{
				if (gCB_BXHTopDmg) gCB_BXHTopDmg->RecvProtocol(lpMsg);
				return 1;
			}
			break;
#endif
#if(CB_BOTTRADER)
			case 0x2E: //Recv List
			{
				if (gCB_BotTrader) gCB_BotTrader->RecvProtocol(lpMsg);
				return 1;
			}
			break;
#endif
			case 0x1E: //Recv List
			{
				if (gCB_DrawCustomRank) gCB_DrawCustomRank->RecvPacketGS(lpMsg);
				return 1;
			}
			break;
#if(SAUDOIITEM)
			case 0x6A:
			{
				XULY_CGPACKET* mRecv = (XULY_CGPACKET*)lpMsg;
				if (mRecv->ThaoTac != 111) return 0;
				if (GetTickCount() > gInterface.Data[eWindowDoiItem].EventTick)
				{
					gInterface.Data[eWindowDoiItem].EventTick = GetTickCount() + 300;
					gInterface.Data[eWindowDoiItem].OnShow ^= 1;
				}
				gInterface.SetStateDoiItem = true;
			}
			break;
			case 0x6B:
			{
				INFOITEM_DOIITEM_CLIENT* mRecv = (INFOITEM_DOIITEM_CLIENT*)lpMsg;
				memcpy(gInterface.SAU_ItemChinh, mRecv->ItemChinh, sizeof(gInterface.SAU_ItemChinh));
				memcpy(gInterface.SAU_ItemPhu, mRecv->ItemPhu, sizeof(gInterface.SAU_ItemPhu));
				for (int n = 0; n < 6; n++)
				{
					memcpy(gInterface.SAU_ItemKetQua[n], mRecv->ItemKetQua[n], sizeof(gInterface.SAU_ItemKetQua[n]));
				}
				gInterface.RateSAU = mRecv->Rate;
				gInterface.SAU_ActiveMix = mRecv->ActiveMix;
				gInterface.SAU_EffectsMix = false;
			}
			break;


#endif
#if(CB_ATTACK_HIDEN_PET)
			case 0x06:
			{
				XULY_CGPACKET* mRecv = (XULY_CGPACKET*)lpMsg;
				if (!mRecv->ThaoTac) return 0;
				//gInterface.DrawMessage(1, "Recv 0x06 %d", mRecv->ThaoTac);
				int Index = FindCharacterIndex(mRecv->ThaoTac);

				if (Index != MAX_CHARACTERS_CLIENT)
				{

					CHARACTER* c = &CharactersClient[Index];
					if (c)
					{
						//gInterface.DrawMessage(1, "CheckHidenPetiSClass Class %d", c->Class);
						if (CheckHidenPetiSClass(gCharacterManager.GetBaseClass(c->Class)))
						{
								c->CacheTimeAttack = GetTickCount() + gProtect.m_MainInfo.HidenPetDelay;
								c->Object.CacheTimeAttack = GetTickCount() + gProtect.m_MainInfo.HidenPetDelay;
								
						}
					}

				}

			}
			break;
#endif
#if(CB_ATTACK_HIDEN_WING)
			case 0x07:
			{
				XULY_CGPACKET* mRecv = (XULY_CGPACKET*)lpMsg;
				if (!mRecv->ThaoTac) return 0;
				//gInterface.DrawMessage(1, "Recv 0x07 %d", mRecv->ThaoTac);
				int Index = FindCharacterIndex(mRecv->ThaoTac);

				if (Index != MAX_CHARACTERS_CLIENT)
				{

					CHARACTER* c = &CharactersClient[Index];
					if (c)
					{
						if (c->WingCache == 0 || c->WingCache == -1)
						{
							c->WingCache = c->Wing.Type;
							c->Wing.Type = -1;
						}
						c->CacheTimeAttack_WING = GetTickCount() + 5000;
						//gInterface.DrawMessage(1, "Recv 0x07 %s", c->ID);
					}

				}

			}
			break;
#endif
				case 0x9A:
				{
					gMocDonate.GetListDonate(lpMsg);
				}
				break;
				case 0x9B:
				{
					gMocDonate.RecvListPhanThuong(lpMsg);
				}
				break;

#if(CB_ActiveInvasionsD)
			case 0x98:
				if (gCB_ActiveInvasions) gCB_ActiveInvasions->list_recv(reinterpret_cast<CB_ActiveInvasions::PMSG_ACTIVE_INVASIONS_RECV*>(lpMsg));
				break;
			case 0x99:
				if (gCB_ActiveInvasions) gCB_ActiveInvasions->update_monster_recv((CB_ActiveInvasions::PMSG_INVASION_MONSTER_UPDATE_RECV*)lpMsg);
				break;
#endif
#if(CB_NEWQUESTFF)
			case 0x75:
				if (gCB_NewQuest) gCB_NewQuest->RecvInfo(lpMsg);
				break;
			case 0x76:
				if (gCB_NewQuest) gCB_NewQuest->RecvRewardList(lpMsg);
				break;
			case 0x77:
				if (gCB_NewQuest) gCB_NewQuest->RecvInfoMonterKill(lpMsg);
				break;
#endif
#if(CB_CUSTOMMIXINFO)
			case 0x71:
				if (gCB_InfoCustomMix) gCB_InfoCustomMix->RecvInfo((CB_InfoCustomMix::PMSG_BCUSTOM_MIX_INFO*)lpMsg);
				break;
#endif
#if(CUSTOM_WINDOWLOCKITEM)
			case 0x1A:
				if(gCB_LockItem) gCB_LockItem->RecvGSSatusLockWindow((CB_LockItem::CGPACKET_LOCKWINDOW*)lpMsg);
				break;
#endif
#if(CB_VIEWCHARITEM)
			case 0x02:
				if(gCB_ViewCharItem) gCB_ViewCharItem->RecvProtocol(lpMsg);
				break;
#endif
#if(CB_VIP_CHAR)
				//==Recv Info VIPChar
			case 0x22:
				gBInterfaceVIPChar->GCSetInfoVIPChar((CBInterfaceVIPChar::BPMSG_CUSTOM_VIPCHAR_SEND*)lpMsg);
				break;
				//==Recv Info VIPChar
			case 0x24:
				gBInterfaceVIPChar->GCSetInfoVIPCharList((CBInterfaceVIPChar::CSEV_COUNTLIST*)lpMsg);
				break;
#endif
#if(CB_DANGKYINGAME)
			case 0x05:
				gCB_DangKyInGame->RecvKQRegInGame((XULY_CGPACKET*)lpMsg);
				break;
#endif

#if(CUSTOM_BRANKINGNEW)
			case 0x40: //
			{
				if(gCustomRanking)gCustomRanking->GCReqInfoCharTop((CustomRanking::DATA_VIEWTOPRANKING*)lpMsg);
				return 1;
			}
			break;
#endif
#if(CB_AUTORESETINFO)
			case 0x36: //
			{
				gCBAutoResetInfo->RecvProtocol(lpMsg);
				return 1;
			}
			break;
			case 0x37: //
			{
				gCBAutoResetInfo->GCCharacterAutoResetRecv(lpMsg);
				return 1;
			}
			break;
#endif
#if(CB_ANTIHACKGGNEW)
			case 0xAC: //
			{
				gAPICB.Recv(lpMsg);
				return 1;
			}
			break;
#endif
#if(DANH_HIEU_NEW == 1)
			case 0x03:
				gCBDanhHieu.RecvProtocol(lpMsg);
				return 1;
				break;
#endif

				case 0xFF: //
				{
					gInterface.RecvInfoTime(lpMsg);
				}
				break;
#if(CB_AutoBanking)
				case 0x7B: //
				{
					gCBAutoNapGame.RecvDataListThanhToan(lpMsg);
					return 1;
				}
				break;
#endif
#if(CUSTOM_BCHOTROI)
				case 0x01:
					gCBChotroi.SetShowItemCache(lpMsg);
					break;
					//Recv List Cho Troi
				case 0x20:
					gCBChotroi.GCSetListChoTroi(lpMsg);
					break;
#endif
				default:
					return 1;
			}
			break;

	}
	return false;
}
void GCConnectClientRecv(PMSG_CONNECT_CLIENT_RECV* lpMsg) // OK
{
	CharacterAttribute->PrintPlayer.ViewIndex = MAKE_NUMBERW(lpMsg->index[0], lpMsg->index[1]);
}

void GCDamageRecv(PMSG_DAMAGE_RECV* lpMsg) // OK
{
	int aIndex = MAKE_NUMBERW(lpMsg->index[0], lpMsg->index[1]) & 0x7FFF;

	if (CharacterAttribute->PrintPlayer.ViewIndex == aIndex)
	{
		CharacterAttribute->PrintPlayer.ViewCurHP = lpMsg->ViewCurHP;
		CharacterAttribute->PrintPlayer.ViewCurSD = lpMsg->ViewCurSD;
	}

	CharacterAttribute->PrintPlayer.ViewDamageHP = lpMsg->ViewDamageHP;
	CharacterAttribute->PrintPlayer.ViewDamageSD = lpMsg->ViewDamageSD;

	if ((lpMsg->type & 0x10) != 0)
	{
		if (CharacterAttribute->PrintPlayer.ViewDamageCount < 3)
		{
			CharacterAttribute->PrintPlayer.ViewDamageTable[CharacterAttribute->PrintPlayer.ViewDamageCount++] = lpMsg->ViewDamageHP;
			CharacterAttribute->PrintPlayer.ViewDamageValue = CharacterAttribute->PrintPlayer.ViewDamageCount;
		}
	}

	if ((lpMsg->type & 0x20) != 0)
	{
		if (CharacterAttribute->PrintPlayer.ViewDamageCount < 4)
		{
			CharacterAttribute->PrintPlayer.ViewDamageTable[CharacterAttribute->PrintPlayer.ViewDamageCount++] = lpMsg->ViewDamageHP;
			CharacterAttribute->PrintPlayer.ViewDamageValue = CharacterAttribute->PrintPlayer.ViewDamageCount;
		}
	}
}

void GCMonsterDieRecv(PMSG_MONSTER_DIE_RECV* lpMsg) // OK
{
	CharacterAttribute->PrintPlayer.ViewDamageHP = lpMsg->ViewDamageHP;
}

void GCUserDieRecv(PMSG_USER_DIE_RECV* lpMsg) // OK
{
	int aIndex = MAKE_NUMBERW(lpMsg->index[0], lpMsg->index[1]) & 0x7FFF;

	if (CharacterAttribute->PrintPlayer.ViewIndex == aIndex)
	{
		CharacterAttribute->PrintPlayer.ViewCurHP = 0;
	}
}

void GCLifeRecv(PMSG_LIFE_RECV* lpMsg) // OK
{
	if (lpMsg->type == 0xFE)
	{
		CharacterAttribute->PrintPlayer.ViewMaxHP = lpMsg->ViewHP;
		CharacterAttribute->PrintPlayer.ViewMaxSD = lpMsg->ViewSD;
	}

	if (lpMsg->type == 0xFF)
	{
		CharacterAttribute->PrintPlayer.ViewCurHP = ((CharacterAttribute->PrintPlayer.ViewCurHP == 0) ? CharacterAttribute->PrintPlayer.ViewCurHP : lpMsg->ViewHP);
		CharacterAttribute->PrintPlayer.ViewCurSD = lpMsg->ViewSD;
	}
}

void GCManaRecv(PMSG_MANA_RECV * lpMsg) // OK
{
	if (lpMsg->type == 0xFE)
	{
		CharacterAttribute->PrintPlayer.ViewMaxMP = lpMsg->ViewMP;
		CharacterAttribute->PrintPlayer.ViewMaxBP = lpMsg->ViewBP;
	}

	if (lpMsg->type == 0xFF)
	{
		CharacterAttribute->PrintPlayer.ViewCurMP = lpMsg->ViewMP;
		CharacterAttribute->PrintPlayer.ViewCurBP = lpMsg->ViewBP;
	}
}

//void GCPShopTextChangeSend(PMSG_PSHOP_TEXT_CHANGE_RECV * lpMsg)
//{
//	memcpy(StoreName, &lpMsg->text, sizeof(StoreName));
//}

void GCFruitResultRecv(PMSG_FRUIT_RESULT_RECV * lpMsg) // OK
{
	if (lpMsg->result == 0 || lpMsg->result == 3 || lpMsg->result == 6 || lpMsg->result == 17)
	{
		CharacterAttribute->PrintPlayer.ViewValue = lpMsg->ViewValue;
		CharacterAttribute->PrintPlayer.ViewPoint = lpMsg->ViewPoint;
		CharacterAttribute->PrintPlayer.ViewStrength = lpMsg->ViewStrength;
		CharacterAttribute->PrintPlayer.ViewDexterity = lpMsg->ViewDexterity;
		CharacterAttribute->PrintPlayer.ViewVitality = lpMsg->ViewVitality;
		CharacterAttribute->PrintPlayer.ViewEnergy = lpMsg->ViewEnergy;
		CharacterAttribute->PrintPlayer.ViewLeadership = lpMsg->ViewLeadership;
	}
}

//=== POst Item
void RecvPostItem(BYTE* ReceiveBuffer)
{


	PMSG_POSTITEM* lpMsg = (PMSG_POSTITEM*)ReceiveBuffer;

	ITEM* pNewItem = new ITEM;
	memset(pNewItem, 0, sizeof(ITEM));
	memcpy_s(pNewItem, sizeof(ITEM), lpMsg->item_data, sizeof(ITEM));
	//g_pChatListBox->AddItemPost(pNewItem);

	g_ConsoleDebug->Write(MCD_RECEIVE, "[RecvPostItem] [%s] %s %d" , lpMsg->chatid, lpMsg->chatmsg, g_pChatListBox->GetCurrentRenderEndLine());
	char Name[11] = { '\0' };
	char Messase[61] = { '\0' };
	char Messase_API[70] = { '\0' };
	memcpy_s(&Name[0], 10, &lpMsg->chatid[0], 10);
	memcpy_s(&Messase[0], 60, &lpMsg->chatmsg[0], 60);

	sprintf_s(Messase_API, gCustomMessage.GetMessageA(87), Messase);
	//g_pChatListBox->AddText(Name, Messase_API, SEASON3B::TYPE_POST_ITEM);
	g_pChatListBox->AddText(Name, Messase_API, MESSAGE_TYPE::TYPE_WHISPER_MESSAGE, MESSAGE_TYPE::TYPE_ALL_MESSAGE, pNewItem);

}


//=== Load Data MuHelper
void CGLoadDataMuHelper(BYTE* ReceiveBuffer)
{
	PMSG_HELPER_DATA_SEND* lpMsg = (PMSG_HELPER_DATA_SEND*)ReceiveBuffer;
	//g_ConsoleDebug->Write(MCD_ERROR, "CGLoadDataMuHelper %d", sizeof(lpMsg->data));
	BYTE BData[256] = { 0 };
	if (!lpMsg->result)
	{
		memcpy_s(BData, sizeof(BData), lpMsg->data, sizeof(BData));
		g_pNewUISystem->GetInstance()->Get_pNewUIMuHelper()->LoadConfig(BData);
		//g_pBMacroInfo->LoadConfig(BData);
	}
	else
	{
		g_pNewUISystem->GetInstance()->Get_pNewUIMuHelper()->ResetConfig();
		//g_pBMacroInfo->ResetConfig();
	}
	g_pNewUISystem->GetInstance()->Get_pNewUIMuHelper()->CacheConfig = true;
}

void CGSaveDataMuHelper(BYTE* ReceiveBuffer)
{
	PMSG_HELPER_DATA_SEND pMsg;

	pMsg.header.set(0xAE, sizeof(pMsg));

	pMsg.result = 1;

	//memset(pMsg.data, 0, sizeof(pMsg.data));
	memcpy(pMsg.data, ReceiveBuffer, sizeof(pMsg.data));

	BDataSend((BYTE*)& pMsg, sizeof(pMsg));
	//CreateNotice("Save CGSaveDataMuHelper", 1);
}

void StartMuHelper(int Type)
{
	if (Type == 0)
	{
		//g_ConsoleDebug->Write(1, "%d", g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[0]);
		if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MuHelper))
		{
			SEASON3B::CreateOkMessageBox(GlobalText[3576]);
			return;
		}
		if (Hero->SafeZone)
		{
			SEASON3B::CreateOkMessageBox(GlobalText[3574]);
			return;
		}

		if (gSkillManager.FindHeroSkill((ActionSkillType)g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[0]) == 0)
		{
			SEASON3B::CreateOkMessageBox(GlobalText[3577]);
			return;
		}
#if(CB_OFFMUHELPER)
		if (gCB_OffMuHelper) gCB_OffMuHelper->RunningOffHelper();
#endif	

	}
	PMSG_HELPER_START_SEND pMsg;

	pMsg.header.set(0xBF, 0x51, sizeof(pMsg));

	pMsg.type = Type; //0 la Start / 1 la Off

	BDataSend((BYTE*)& pMsg, pMsg.header.size);
	
}
void RecvStartMuHelper(BYTE* ReceiveBuffer)
{
	//g_pChatListBox->AddText("", "RecvStartMuHelper", SEASON3B::TYPE_ERROR_MESSAGE);
	PMSG_HELPER_START_RECV* lpMsg = (PMSG_HELPER_START_RECV*)ReceiveBuffer;
	g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.TimerMoney = lpMsg->time;
	if (!lpMsg->result)
	{
		if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Started == 0)
		{
			g_pChatListBox->AddText("", GlobalText[3578], SEASON3B::TYPE_ERROR_MESSAGE);
			g_pChatListBox->AddText("", GlobalText[3582], SEASON3B::TYPE_ERROR_MESSAGE);
			//CharacterAttribute->PrintPlayer.StartAutoMuHelper = 1;
			g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Started = 1;
			g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.StartX = Hero->PositionX;
			g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.StartY = Hero->PositionY;
			g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.TimerMove = GetTickCount();

			if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.DarkSpirit)
			{
				PMSG_PET_ITEM_COMMAND_SEND pMsg;

				pMsg.header.set(0xA7, sizeof(pMsg));
				pMsg.type = 0;
				pMsg.command = 2;
				pMsg.index[0] = SET_NUMBERHB(-1);
				pMsg.index[1] = SET_NUMBERLB(-1);

				BDataSend((BYTE*)& pMsg, pMsg.header.size);
			}
		}
		else
		{
			char zbtemp[128] = { 0 };
			sprintf_s(zbtemp, GlobalText[3586], lpMsg->money);
			g_pChatListBox->AddText("", zbtemp, SEASON3B::TYPE_ERROR_MESSAGE);
		}
	}
	else
	{
		g_pChatListBox->AddText("", GlobalText[3579], SEASON3B::TYPE_ERROR_MESSAGE);
		g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Clear();
	}
	
}
//===HP Bar
void ReceiveNewHealthBar(BYTE* ReceiveBuffer)
{
	//g_ConsoleDebug->Write(MCD_RECEIVE, "0xE2 [ReceiveNewHealthBar]");
	PMSG_NEW_HEALTH_BAR_RECV* lpMsg = (PMSG_NEW_HEALTH_BAR_RECV*)ReceiveBuffer;

	for (int n = 0; n < lpMsg->count; n++)
	{
		PMSG_NEW_HEALTH_RECV* lpInfo = (PMSG_NEW_HEALTH_RECV*)(((BYTE*)lpMsg) + sizeof(PMSG_NEW_HEALTH_BAR_RECV) + (sizeof(PMSG_NEW_HEALTH_RECV) * n));

		//InsertNewHealthBar(lpInfo->index, lpInfo->type, lpInfo->rate, lpInfo->rate2);
		int Index = FindCharacterIndex(lpInfo->index);
		if (Index != MAX_CHARACTERS_CLIENT)
		{

			CHARACTER* c = &CharactersClient[Index];
			if (!c) continue;
			c->InfoHealBar.Clear();
			c->InfoHealBar.Type = lpInfo->type;
			c->InfoHealBar.Life = lpInfo->rate;
			c->InfoHealBar.SD = lpInfo->rate2;
			c->InfoHealBar.Level = lpInfo->Level;
			//c->InfoHealBar.Reset = lpInfo->Reset;
			//c->InfoHealBar.MReset = lpInfo->MReset;
			//g_ConsoleDebug->Write(MCD_RECEIVE, "0xE2 [ReceiveNewHealthBar] %d", lpInfo->rate);
		}


	}
}

void GCCharacterInfoRecv(BYTE* ReceiveBuffer) // OK
{
	PRECEIVE_JOIN_MAP_SERVER* lpMsg = (PRECEIVE_JOIN_MAP_SERVER*)ReceiveBuffer;
#if(UseReconnect)
	g_pReconnect->ReconnectOnCharacterInfo();  //Add
#endif
	//CharacterAttribute->PrintPlayer.Clear();
	CharacterAttribute->PrintPlayer.ViewReset = lpMsg->ViewReset;
	CharacterAttribute->PrintPlayer.ViewMasterReset = lpMsg->ViewMasterReset;
	CharacterAttribute->PrintPlayer.ViewPoint = lpMsg->ViewPoint;
	CharacterAttribute->PrintPlayer.ViewCurHP = lpMsg->ViewCurHP;
	CharacterAttribute->PrintPlayer.ViewMaxHP = lpMsg->ViewMaxHP;
	CharacterAttribute->PrintPlayer.ViewCurMP = lpMsg->ViewCurMP;
	CharacterAttribute->PrintPlayer.ViewMaxMP = lpMsg->ViewMaxMP;
	CharacterAttribute->PrintPlayer.ViewCurBP = lpMsg->ViewCurBP;
	CharacterAttribute->PrintPlayer.ViewMaxBP = lpMsg->ViewMaxBP;
	CharacterAttribute->PrintPlayer.ViewCurSD = lpMsg->ViewCurSD;
	CharacterAttribute->PrintPlayer.ViewMaxSD = lpMsg->ViewMaxSD;
	CharacterAttribute->PrintPlayer.ViewStrength = lpMsg->ViewStrength;
	CharacterAttribute->PrintPlayer.ViewDexterity = lpMsg->ViewDexterity;
	CharacterAttribute->PrintPlayer.ViewVitality = lpMsg->ViewVitality;
	CharacterAttribute->PrintPlayer.ViewEnergy = lpMsg->ViewEnergy;
	CharacterAttribute->PrintPlayer.ViewLeadership = lpMsg->ViewLeadership;
	
	//*(WORD*)(*(DWORD*)(MAIN_VIEWPORT_STRUCT)+0x07E) = 0; //aIndex

	//*(BYTE*)(*(DWORD*)(MAIN_VIEWPORT_STRUCT)+0x30C) = 0;

	switch (gCharacterManager.GetBaseClass(Hero->Class))
	{
	case 0:
		g_iLimitAttackTimeSet = ((gProtect.m_MainInfo.DWMaxAttackSpeed >= 0xFFFF) ? 0x02 : 0x0F);
		break;
	case 1:
		g_iLimitAttackTimeSet =  ((gProtect.m_MainInfo.DKMaxAttackSpeed >= 0xFFFF) ? 0x06 : 0x0F);
		g_iLimitAttackTimeSet =  ((gProtect.m_MainInfo.SUMaxAttackSpeed >= 0xFFFF) ? 0x02 : 0x0F);
		break;
	case 2:
		g_iLimitAttackTimeSet = ((gProtect.m_MainInfo.FEMaxAttackSpeed >= 0xFFFF) ? 0x02 : 0x0F);
		break;
	case 3:
		g_iLimitAttackTimeSet = ((gProtect.m_MainInfo.MGMaxAttackSpeed >= 0xFFFF) ? 0x02 : 0x0F);
		break;
	case 4:
		g_iLimitAttackTimeSet = ((gProtect.m_MainInfo.DLMaxAttackSpeed >= 0xFFFF) ? 0x02 : 0x0F);
		break;
	case 5:
		g_iLimitAttackTimeSet = ((gProtect.m_MainInfo.SUMaxAttackSpeed >= 0xFFFF) ? 0x02 : 0x0F);
		break;
	case 6:
		g_iLimitAttackTimeSet = ((gProtect.m_MainInfo.RFMaxAttackSpeed >= 0xFFFF) ? 0x0F : 0x0F);
		break;
	}
}

void GCNewCharacterInfoRecv(BYTE* ReceiveBuffer)
{
	PMSG_NEW_CHARACTER_INFO_RECV* lpMsg = (PMSG_NEW_CHARACTER_INFO_RECV*)ReceiveBuffer;

	CharacterAttribute->Level = lpMsg->Level;
	CharacterAttribute->LevelUpPoint = lpMsg->LevelUpPoint;
	CharacterAttribute->Strength = lpMsg->Strength;
	CharacterAttribute->Dexterity = lpMsg->Dexterity;
	CharacterAttribute->Vitality = lpMsg->Vitality;
	CharacterAttribute->Energy = lpMsg->Energy;
	CharacterAttribute->Charisma = lpMsg->Leadership;
	CharacterAttribute->Life = lpMsg->Life;
	CharacterAttribute->LifeMax = lpMsg->MaxLife;
	CharacterAttribute->Mana = lpMsg->Mana;
	CharacterAttribute->ManaMax = lpMsg->MaxMana;
	CharacterAttribute->SkillMana = lpMsg->BP;
	CharacterAttribute->SkillManaMax = lpMsg->MaxBP;
	CharacterAttribute->Shield = lpMsg->Shield;
	CharacterAttribute->ShieldMax = lpMsg->MaxShield;
	CharacterAttribute->AddPoint = lpMsg->FruitAddPoint;
	CharacterAttribute->MaxAddPoint = lpMsg->MaxFruitAddPoint;
	CharacterAttribute->wMinusPoint = lpMsg->FruitSubPoint;
	CharacterAttribute->wMaxMinusPoint = lpMsg->MaxFruitSubPoint;
	CharacterAttribute->Experience = lpMsg->Experience;
	CharacterAttribute->NextExperince = lpMsg->NextExperience;
	//EXTRA
	CharacterAttribute->PrintPlayer.ViewReset = lpMsg->ViewReset;
	CharacterAttribute->PrintPlayer.ViewMasterReset = lpMsg->ViewMasterReset;
	CharacterAttribute->PrintPlayer.ViewPoint = lpMsg->ViewPoint;
	CharacterAttribute->PrintPlayer.ViewCurHP = lpMsg->ViewCurHP;
	CharacterAttribute->PrintPlayer.ViewMaxHP = lpMsg->ViewMaxHP;
	CharacterAttribute->PrintPlayer.ViewCurMP = lpMsg->ViewCurMP;
	CharacterAttribute->PrintPlayer.ViewMaxMP = lpMsg->ViewMaxMP;
	CharacterAttribute->PrintPlayer.ViewCurBP = lpMsg->ViewCurBP;
	CharacterAttribute->PrintPlayer.ViewMaxBP = lpMsg->ViewMaxBP;
	CharacterAttribute->PrintPlayer.ViewCurSD = lpMsg->ViewCurSD;
	CharacterAttribute->PrintPlayer.ViewMaxSD = lpMsg->ViewMaxSD;
	CharacterAttribute->PrintPlayer.ViewStrength = lpMsg->ViewStrength;
	CharacterAttribute->PrintPlayer.ViewDexterity = lpMsg->ViewDexterity;
	CharacterAttribute->PrintPlayer.ViewVitality = lpMsg->ViewVitality;
	CharacterAttribute->PrintPlayer.ViewEnergy = lpMsg->ViewEnergy;
	CharacterAttribute->PrintPlayer.ViewLeadership = lpMsg->ViewLeadership;
}
void GCNewCharacterCalcRecv(BYTE* ReceiveBuffer)
{
	PMSG_NEW_CHARACTER_CALC_RECV* lpMsg = (PMSG_NEW_CHARACTER_CALC_RECV*)ReceiveBuffer;
	CharacterAttribute->PrintPlayer.ViewCurHP = lpMsg->ViewCurHP;
	CharacterAttribute->PrintPlayer.ViewMaxHP = lpMsg->ViewMaxHP;
	CharacterAttribute->PrintPlayer.ViewCurMP = lpMsg->ViewCurMP;
	CharacterAttribute->PrintPlayer.ViewMaxMP = lpMsg->ViewMaxMP;
	CharacterAttribute->PrintPlayer.ViewCurBP = lpMsg->ViewCurBP;
	CharacterAttribute->PrintPlayer.ViewMaxBP = lpMsg->ViewMaxBP;
	CharacterAttribute->PrintPlayer.ViewCurSD = lpMsg->ViewCurSD;
	CharacterAttribute->PrintPlayer.ViewMaxSD = lpMsg->ViewMaxSD;
	CharacterAttribute->PrintPlayer.ViewAddStrength = lpMsg->ViewAddStrength;
	CharacterAttribute->PrintPlayer.ViewAddDexterity = lpMsg->ViewAddDexterity;
	CharacterAttribute->PrintPlayer.ViewAddVitality = lpMsg->ViewAddVitality;
	CharacterAttribute->PrintPlayer.ViewAddEnergy = lpMsg->ViewAddEnergy;
	CharacterAttribute->PrintPlayer.ViewAddLeadership = lpMsg->ViewAddLeadership;
	CharacterAttribute->PrintPlayer.ViewPhysiDamageMin = lpMsg->ViewPhysiDamageMin;
	CharacterAttribute->PrintPlayer.ViewPhysiDamageMax = lpMsg->ViewPhysiDamageMax;
	CharacterAttribute->PrintPlayer.ViewMagicDamageMin = lpMsg->ViewMagicDamageMin;
	CharacterAttribute->PrintPlayer.ViewMagicDamageMax = lpMsg->ViewMagicDamageMax;
	CharacterAttribute->PrintPlayer.ViewCurseDamageMin = lpMsg->ViewCurseDamageMin;
	CharacterAttribute->PrintPlayer.ViewCurseDamageMax = lpMsg->ViewCurseDamageMax;
	CharacterAttribute->PrintPlayer.ViewMulPhysiDamage = lpMsg->ViewMulPhysiDamage;
	CharacterAttribute->PrintPlayer.ViewDivPhysiDamage = lpMsg->ViewDivPhysiDamage;
	CharacterAttribute->PrintPlayer.ViewMulMagicDamage = lpMsg->ViewMulMagicDamage;
	CharacterAttribute->PrintPlayer.ViewDivMagicDamage = lpMsg->ViewDivMagicDamage;
	CharacterAttribute->PrintPlayer.ViewMulCurseDamage = lpMsg->ViewMulCurseDamage;
	CharacterAttribute->PrintPlayer.ViewDivCurseDamage = lpMsg->ViewDivCurseDamage;
	CharacterAttribute->PrintPlayer.ViewMagicDamageRate = lpMsg->ViewMagicDamageRate;
	CharacterAttribute->PrintPlayer.ViewCurseDamageRate = lpMsg->ViewCurseDamageRate;
	CharacterAttribute->PrintPlayer.ViewPhysiSpeed = lpMsg->ViewPhysiSpeed;
	CharacterAttribute->PrintPlayer.ViewMagicSpeed = lpMsg->ViewMagicSpeed;
	CharacterAttribute->PrintPlayer.ViewAttackSuccessRate = lpMsg->ViewAttackSuccessRate;
	CharacterAttribute->PrintPlayer.ViewAttackSuccessRatePvP = lpMsg->ViewAttackSuccessRatePvP;
	CharacterAttribute->PrintPlayer.ViewDefense = lpMsg->ViewDefense;
	CharacterAttribute->PrintPlayer.ViewDefenseSuccessRate = lpMsg->ViewDefenseSuccessRate;
	CharacterAttribute->PrintPlayer.ViewDefenseSuccessRatePvP = lpMsg->ViewDefenseSuccessRatePvP;
	CharacterAttribute->PrintPlayer.ViewDamageMultiplier = lpMsg->ViewDamageMultiplier;
	CharacterAttribute->PrintPlayer.ViewRFDamageMultiplierA = lpMsg->ViewRFDamageMultiplierA;
	CharacterAttribute->PrintPlayer.ViewRFDamageMultiplierB = lpMsg->ViewRFDamageMultiplierB;
	CharacterAttribute->PrintPlayer.ViewRFDamageMultiplierC = lpMsg->ViewRFDamageMultiplierC;
	CharacterAttribute->PrintPlayer.ViewDarkSpiritAttackDamageMin = lpMsg->ViewDarkSpiritAttackDamageMin;
	CharacterAttribute->PrintPlayer.ViewDarkSpiritAttackDamageMax = lpMsg->ViewDarkSpiritAttackDamageMax;
	CharacterAttribute->PrintPlayer.ViewDarkSpiritAttackSpeed = lpMsg->ViewDarkSpiritAttackSpeed;
	CharacterAttribute->PrintPlayer.ViewDarkSpiritAttackSuccessRate = lpMsg->ViewDarkSpiritAttackSuccessRate;
}

void GCPetCharSetRecv(PMSG_NEW_PET_CHARSET_RECV* lpMsg)
{
	ClearNewPetCharSet();

	for (int n = 0; n < lpMsg->count; n++)
	{
		NEW_PET_STRUCT* lpInfo = (NEW_PET_STRUCT*)(((BYTE*)lpMsg) + sizeof(PMSG_NEW_PET_CHARSET_RECV) + (sizeof(NEW_PET_STRUCT) * n));

		InsertNewPetCharSet(lpInfo->index, lpInfo->PetCharSet);
	}
}


void GCPetCharSetSelectCharacterRecv(PMSG_NEW_SELECT_CHARACTER_CHARSET_RECV * lpMsg)
{
	ClearNewPetCharSetSelectCharacter();

	for (int n = 0; n < lpMsg->count; n++)
	{
		PMSG_NEW_CHARACTER_CHARSET_RECV* lpInfo = (PMSG_NEW_CHARACTER_CHARSET_RECV*)(((BYTE*)lpMsg) + sizeof(PMSG_NEW_SELECT_CHARACTER_CHARSET_RECV) + (sizeof(PMSG_NEW_CHARACTER_CHARSET_RECV) * n));

		InsertNewPetCharSetSelectCharacter(lpInfo->Name, lpInfo->PetCharSet);
	}
}

void GCCharacterRegenRecv(PMSG_CHARACTER_REGEN_RECV* lpMsg) // OK
{
	CharacterAttribute->PrintPlayer.ViewCurHP = lpMsg->ViewCurHP;
	CharacterAttribute->PrintPlayer.ViewCurMP = lpMsg->ViewCurMP;
	CharacterAttribute->PrintPlayer.ViewCurBP = lpMsg->ViewCurBP;
	CharacterAttribute->PrintPlayer.ViewCurSD = lpMsg->ViewCurSD;
}

void GCLevelUpRecv(PMSG_LEVEL_UP_RECV* lpMsg) // OK
{
	CharacterAttribute->PrintPlayer.ViewPoint = lpMsg->ViewPoint;
	CharacterAttribute->PrintPlayer.ViewMaxHP = lpMsg->ViewMaxHP;
	CharacterAttribute->PrintPlayer.ViewMaxMP = lpMsg->ViewMaxMP;
	CharacterAttribute->PrintPlayer.ViewMaxBP = lpMsg->ViewMaxBP;
	CharacterAttribute->PrintPlayer.ViewMaxSD = lpMsg->ViewMaxSD;
	CharacterAttribute->PrintPlayer.ViewExperience = lpMsg->ViewExperience;
	CharacterAttribute->PrintPlayer.ViewNextExperience = lpMsg->ViewNextExperience;
}

void GCLevelUpPointRecv(PMSG_LEVEL_UP_POINT_RECV* lpMsg) // OK
{
	if (lpMsg->result >= 16 && lpMsg->result <= 20)
	{
		CharacterAttribute->PrintPlayer.ViewPoint = lpMsg->ViewPoint;
		CharacterAttribute->PrintPlayer.ViewMaxHP = lpMsg->ViewMaxHP;
		CharacterAttribute->PrintPlayer.ViewMaxMP = lpMsg->ViewMaxMP;
		CharacterAttribute->PrintPlayer.ViewMaxBP = lpMsg->ViewMaxBP;
		CharacterAttribute->PrintPlayer.ViewMaxSD = lpMsg->ViewMaxSD;
		CharacterAttribute->PrintPlayer.ViewStrength = lpMsg->ViewStrength;
		CharacterAttribute->PrintPlayer.ViewDexterity = lpMsg->ViewDexterity;
		CharacterAttribute->PrintPlayer.ViewVitality = lpMsg->ViewVitality;
		CharacterAttribute->PrintPlayer.ViewEnergy = lpMsg->ViewEnergy;
		CharacterAttribute->PrintPlayer.ViewLeadership = lpMsg->ViewLeadership;
	}
}

void GCMonsterDamageRecv(PMSG_MONSTER_DAMAGE_RECV* lpMsg) // OK
{
	CharacterAttribute->PrintPlayer.ViewCurHP = lpMsg->ViewCurHP;
	CharacterAttribute->PrintPlayer.ViewCurSD = lpMsg->ViewCurSD;
	CharacterAttribute->PrintPlayer.ViewDamageHP = lpMsg->ViewDamageHP;
	CharacterAttribute->PrintPlayer.ViewDamageSD = lpMsg->ViewDamageSD;
}

void GCMasterInfoRecv(PMSG_MASTER_INFO_RECV* lpMsg) // OK
{
	CharacterAttribute->PrintPlayer.ViewMaxHP = lpMsg->ViewMaxHP;
	CharacterAttribute->PrintPlayer.ViewMaxMP = lpMsg->ViewMaxMP;
	CharacterAttribute->PrintPlayer.ViewMaxBP = lpMsg->ViewMaxBP;
	CharacterAttribute->PrintPlayer.ViewMaxSD = lpMsg->ViewMaxSD;
}

void GCMasterLevelUpRecv(PMSG_MASTER_LEVEL_UP_RECV* lpMsg) // OK
{
	CharacterAttribute->PrintPlayer.ViewMaxHP = lpMsg->ViewMaxHP;
	CharacterAttribute->PrintPlayer.ViewMaxMP = lpMsg->ViewMaxMP;
	CharacterAttribute->PrintPlayer.ViewMaxBP = lpMsg->ViewMaxBP;
	CharacterAttribute->PrintPlayer.ViewMaxSD = lpMsg->ViewMaxSD;
	CharacterAttribute->PrintPlayer.ViewMasterExperience = lpMsg->ViewMasterExperience;
	CharacterAttribute->PrintPlayer.ViewMasterNextExperience = lpMsg->ViewMasterNextExperience;
}

void DataSend(BYTE* lpMsg, DWORD size) // OK
{
	BDataSend(lpMsg, size);
}


void GCRecvCoin(PMSG_COIN_RECV* lpMsg) // OK
{
	CharacterAttribute->PrintPlayer.Coin1 = lpMsg->Coin1;
	CharacterAttribute->PrintPlayer.Coin2 = lpMsg->Coin2;
	CharacterAttribute->PrintPlayer.Coin3 = lpMsg->Coin3;
}
//Right Click
void SetChaosBoxState(PMSG_SET_CHAOSBOX_STATE* Data)
{
	g_pNewUISystem->GetUI_NewMixInventory()->SetMixState(Data->state);
	return;
}

#if(HAISLOTRING)
void BReceiveInventory(BYTE* ReceiveBuffer)
{
	//g_pMyInventory->UnequipAllItems();

	for (int i = 0; i < 16; i++)
	{
		CharacterMachine->EquipmentMuun[i].Type = -1;
		CharacterMachine->EquipmentMuun[i].Level = 0;
		CharacterMachine->EquipmentMuun[i].Option1 = 0;
	}

	LPPHEADER_DEFAULT_SUBCODE_WORD DataRecv = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
	int Offset = sizeof(PHEADER_DEFAULT_SUBCODE_WORD);

	for (int i = 0; i < DataRecv->Value; i++)
	{
		LPPRECEIVE_INVENTORY Data = (LPPRECEIVE_INVENTORY)(ReceiveBuffer + Offset);
		//gInterface.DrawMessage(1,"%d~%d", DataRecv->Value, Data->Index);
		g_pMyInventory->EquipItem(Data->Index, Data->Item);
		Offset += sizeof(PRECEIVE_INVENTORY);
	}
	//this->LoadInv = true;
}
void BReceiveDurability(BYTE* ReceiveBuffer)
{
	LPPHEADER_DEFAULT_VALUE_DUR Data = (LPPHEADER_DEFAULT_VALUE_DUR)ReceiveBuffer;
	if (Data->Key != 0xFF)
	{
		ITEM* ip = &CharacterMachine->EquipmentMuun[Data->Key];
		if (ip->Type != 65535)
		{
			ip->Durability = Data->Value;
		}
	}
}
void BReceiveDeleteInventory(BYTE* ReceiveBuffer)
{
	LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;

	//gInterface.DrawMessage(1, "BReceiveDeleteInventory %d %d", Data->SubCode, SEASON3B::CNewUIInventoryCtrl::GetPickedItem());
	if (Data->SubCode != 0xff)
	{
		if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem())
		{
			SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();
			return;
		}
		g_pMyInventory->UnequipItem(Data->SubCode);
	}

	if (Data->Value)
	{
		//EnableUse = 0;
	}
}
void BReceiveItemChange(BYTE* ReceiveBuffer)
{
	LPPSCHANGE_CHARACTER Data = (LPPSCHANGE_CHARACTER)ReceiveBuffer;




	int Key = ((int)(Data->KeyH) << 8) + Data->KeyL;
	Key &= 0x7FFF;

	CHARACTER* c;
	int iIndex = FindCharacterIndex(Key);
	c = &CharactersClient[iIndex];

	if (c == NULL) { return; }

	int Type = ConvertItemType(Data->Item);
	//zITEM* TempIp = CreateItem(Data->Item);
	BYTE Level = Data->Item[1] & 0xF;
	BYTE Option = Data->Item[3] & 63;
	BYTE ExtOption = Data->Item[4];
	//ITEM_ATTRIBUTE* pItemAttr = ItemAttribute(Type);
	//gInterface.DrawMessage(1, "ReceiveItemChange %d %x Key %d", Data->Item[1] >> 4, Type, *(WORD*)(c+126));
	//gInterface.DrawMessage(1, "%d",this->deopet);
	if (c)
	{
		if (Data->Item[1] >> 4 == 12)
		{
			if (Type != 0x1FFF)
			{
				c->m_PetInfo[PET_TYPE_END].m_dwPetType = Type + 1171;
				CreateEffectEquipping(Type, c, &c->Object);
			}
			else
			{
				c->m_PetInfo[PET_TYPE_END].m_dwPetType = 0xFFFF;
				DeleteBug(&c->Object);
			}
		}
	}
	ChangeChaosCastleUnit(c);
	SetCharacterScale(c);
}
#endif

bool CheckHidenPetiSClass(BYTE Class)
{
	switch (Class)
	{
	case CLASS_WIZARD:
	case CLASS_SOULMASTER:
	case CLASS_GRANDMASTER:
		return gProtect.m_MainInfo.HidenPetIsClass[0];
		break;
	case CLASS_KNIGHT:
	case CLASS_BLADEKNIGHT:
	case CLASS_BLADEMASTER:
		return gProtect.m_MainInfo.HidenPetIsClass[1];
		break;
	case CLASS_ELF:
	case CLASS_MUSEELF:
	case CLASS_HIGHELF:
		return gProtect.m_MainInfo.HidenPetIsClass[2];
		break;
	case CLASS_DARK:
	case CLASS_DUELMASTER:
		return gProtect.m_MainInfo.HidenPetIsClass[3];
		break;
	case CLASS_DARK_LORD:
	case CLASS_LORDEMPEROR:
		return gProtect.m_MainInfo.HidenPetIsClass[4];
		break;
	case CLASS_SUMMONER:
	case CLASS_BLOODYSUMMONER:
	case CLASS_DIMENSIONMASTER:
		return gProtect.m_MainInfo.HidenPetIsClass[5];
		break;
	case CLASS_RAGEFIGHTER:
	case CLASS_TEMPLENIGHT:
		return gProtect.m_MainInfo.HidenPetIsClass[6];
		break;
	default:
		break;
	}

	return false;
}

struct AUTOMOVE_REQ
{
	PSBMSG_HEAD h;
	int	TYPE;
};
void CGAutoMove(int Type)
{
	AUTOMOVE_REQ pRequest;
	pRequest.h.set(0xF3, 0x37, sizeof(pRequest));
	pRequest.TYPE = Type;
	BDataSend((LPBYTE)& pRequest, pRequest.h.size);
}
