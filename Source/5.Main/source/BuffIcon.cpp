#include "stdafx.h"
#include "BuffIcon.h"
#include "NewUISystem.h"
BuffIcon gIconBuff;

BuffIcon::BuffIcon()
{
	this->LoadBuffCount = 0;
	this->UserBuffCount = 0;
	this->CUSTOM_BUFFICON = true;
	ZeroMemory(&RenderInfoBuff, sizeof(RenderInfoBuff));
	ZeroMemory(&UserBuff, sizeof(UserBuff));
	BuffEffectTRSData_ENG.clear();
	BuffEffectTRSData_VTM.clear();
	this->hThread = (HANDLE)_beginthreadex(NULL, 0, this->Timer, 0, 0, NULL);

}


BuffIcon::~BuffIcon()
{
	CloseHandle(this->hThread);
}

void BuffIcon::LoadEng(RenderTooltipBuff* info)
{
	for (int n = 0; n < 256; n++)
	{
		if (info[n].Index < 0 || info[n].Index > 256)
		{
			return;
		}

		this->BuffEffectTRSData_ENG.insert(std::pair<int, RenderTooltipBuff>(info[n].IndexBuff, info[n]));
	}
}

void BuffIcon::LoadVTM(RenderTooltipBuff* info)
{
	for (int n = 0; n < 256; n++)
	{
		if (info[n].Index < 0 || info[n].Index > 256)
		{
			return;
		}

		this->BuffEffectTRSData_VTM.insert(std::pair<int, RenderTooltipBuff>(info[n].IndexBuff, info[n]));
	}
}


unsigned int BuffIcon::Timer(void* Args)
{
	while (true)
	{
		for (int i = 0; i < MAX_EFFECT_LIST; i++)
		{
			if (gIconBuff.UserBuff[i].Index != NULL)
			{
				if (gIconBuff.UserBuff[i].Time > 0)
				{
					gIconBuff.UserBuff[i].Time--;
					if (gIconBuff.UserBuff[i].Time <= 0)
					{
						gIconBuff.UserBuff[i].Index = NULL;
						gIconBuff.UserBuff[i].Time = 0;

						gIconBuff.UserBuffCount--;
					}
				}
			}

			::Sleep(1);
		}
		//g_pNewUISystem->GetUI_NewBCustomMenuInfo()->RunningMuhelper();
		::Sleep(999);
	}

	return 0;
}

void BuffIcon::Recv(PMSG_SEND_BUFF_ICON* DataRecv)
{
	if (DataRecv->state == 0 && DataRecv->effect > 0)
	{
		for (int i = 0; i < MAX_EFFECT_LIST; i++)
		{
			if (this->UserBuff[i].Index == NULL || this->UserBuff[i].Index == DataRecv->effect)
			{
				if (this->UserBuff[i].Index == NULL)
				{
					this->UserBuffCount++;
				}
				this->UserBuff[i].Index = DataRecv->effect;
				this->UserBuff[i].Time = DataRecv->time;
				this->UserBuff[i].MaxTime = DataRecv->time;
				return;
			}
		}
	}
	else
	{
		for (int i = 0; i < MAX_EFFECT_LIST; i++)
		{
			if (this->UserBuff[i].Index == DataRecv->effect)
			{
				this->UserBuff[i].Index = NULL;
				this->UserBuff[i].Time = 0;
				this->UserBuff[i].MaxTime = 0;

				this->UserBuffCount--;
				return;
			}
		}
	}
}

void BuffIcon::DeleteAllBuff()
{
	this->UserBuffCount = 0;
	for (int i = 0; i < MAX_EFFECT_LIST; i++)
	{
		this->UserBuff[i].Index = NULL;
		this->UserBuff[i].Time = 0;
		this->UserBuff[i].MaxTime = 0;
	}
}

void BuffIcon::GC_BuffInfo(PMSG_SEND_BUFF_ICON_EX* aRecv)
{
	int BuffIndex = aRecv->btEffectIndex;

	bool continuar = false;

	std::map<int, RenderTooltipBuff>::iterator it;

	if (strcmp((char*)g_strSelectedML.c_str(), "Eng") == 0)
	{
		it = this->BuffEffectTRSData_ENG.find(BuffIndex);

		if (it != this->BuffEffectTRSData_ENG.end())
		{
			continuar = true;
		}
	}

	if (strcmp((char*)g_strSelectedML.c_str(), "Vn") == 0)
	{
		it = this->BuffEffectTRSData_VTM.find(BuffIndex);

		if (it != this->BuffEffectTRSData_VTM.end())
		{
			continuar = true;
		}
	}

	if (continuar == true)
	{
		switch (BuffIndex)
		{
		case EFFECT_GREATER_DAMAGE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[0], aRecv->iValue[0]);
		}
		break;
		case EFFECT_GREATER_DEFENSE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_ELF_BUFFER:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff,
				aRecv->iValue[0], aRecv->iValue[0], aRecv->iValue[0], aRecv->iValue[1]);
		}
		break;
		case EFFECT_MANA_SHIELD:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_GREATER_CRITICAL_DAMAGE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_INFINITY_ARROW:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_BP_RECOVERY:
			break;
		case EFFECT_GREATER_LIFE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_GREATER_MANA:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_BLESS_POTION:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_SOUL_POTION:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff
				, aRecv->iValue[0], aRecv->iValue[0], aRecv->iValue[1]);

		}
		break;
		case EFFECT_DISABLE_MAGIC:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_CASTLE_GATE_STATE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_GUILD_STATE1:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_GUILD_STATE2:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_GUILD_STATE3:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_GUILD_STATE4:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_INVISIBILITY:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_GUILD_STATE5:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_CASTLE_CROWN_STATE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_CRYWOLF_STATE1:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_CRYWOLF_STATE2:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_CRYWOLF_STATE3:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_CRYWOLF_STATE4:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_CRYWOLF_STATE5:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_CRYWOLF_STATE6:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_CRYWOLF_STATE7:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_GAME_MASTER:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_SEAL_OF_ASCENSION1:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_SEAL_OF_WEALTH1:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1]);
		}
		break;
		case EFFECT_SEAL_OF_SUSTENANCE1:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_ORDER_OF_SPEED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_ORDER_OF_SUBLIMATION:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_ORDER_OF_PROTECTION:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_HALLOWEEN1:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[0]);
		}
		break;
		case EFFECT_HALLOWEEN2:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[0]);
		}
		break;
		case EFFECT_HALLOWEEN3:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_HALLOWEEN4:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_HALLOWEEN5:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_SEAL_OF_ASCENSION2:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_SEAL_OF_WEALTH2:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1]);
		}
		break;
		case EFFECT_SEAL_OF_SUSTENANCE2:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_SEAL_OF_MOVEMENT:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_SCROLL_OF_QUICK:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[0]);
		}
		break;
		case EFFECT_SCROLL_OF_DEFENSE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_SCROLL_OF_DAMAGE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_SCROLL_OF_MAGIC_DAMAGE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_SCROLL_OF_LIFE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_SCROLL_OF_MANA:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_ELIXIR_OF_STRENGTH:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_ELIXIR_OF_DEXTERITY:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_ELIXIR_OF_VITALITY:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_ELIXIR_OF_ENERGY:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_ELIXIR_OF_LEADERSHIP:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_POISON:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_ICE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_ICE_ARROW:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_FIRE_SLASH:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_PHYSI_DAMAGE_IMMUNITY:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_MAGIC_DAMAGE_IMMUNITY:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_STERN:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_MAGIC_DEFENSE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_ORDER_OF_RESTRAINT:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_CRYWOLF_STATE8:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_CRYWOLF_STATE9:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_CRYWOLF_STATE10:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_CRYWOLF_STATE11:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_CRYWOLF_STATE12:
		{
			memcpy(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_DAMAGE_REFLECT:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_SLEEP:
		{
			memcpy(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_BLIND:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_NEIL:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_SAHAMUTT:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_LESSER_DAMAGE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[0], aRecv->iValue[0]);
		}
		break;
		case EFFECT_LESSER_DEFENSE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_CHERRY_BLOSSOM1:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_CHERRY_BLOSSOM2:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_CHERRY_BLOSSOM3:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[0]);
		}
		break;
		case EFFECT_SWORD_POWER:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff
				, aRecv->iValue[0], aRecv->iValue[1], aRecv->iValue[2], aRecv->iValue[3]);
		}
		break;
		case EFFECT_MAGIC_CIRCLE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[0]);
		}
		break;
		case EFFECT_SWORD_SLASH:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_LIGHTNING_STORM:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_RED_STORM:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_FROZEN_STAB:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_SEAL_OF_LIFE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1]);
		}
		break;
		case EFFECT_SEAL_OF_MANA:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1]);
		}
		break;
		case EFFECT_SCROLL_OF_BATTLE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_SCROLL_OF_STRENGTH:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_CHRISTMAS1:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff
				, aRecv->iValue[0], aRecv->iValue[0], aRecv->iValue[0], aRecv->iValue[1]);
		}
		break;
		case EFFECT_CHRISTMAS2:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_CHRISTMAS3:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_CHRISTMAS4:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff
				, aRecv->iValue[0], aRecv->iValue[0], aRecv->iValue[0]);
		}
		break;
		case EFFECT_CHRISTMAS5:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_CHRISTMAS6:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[0]);
		}
		break;
		case EFFECT_CHRISTMAS7:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_DUEL_ARENA_WATCH:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_TALISMAN_OF_GUARDIAN:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_TALISMAN_OF_PROTECTION:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_MASTER_SEAL_OF_ASCENSION:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_MASTER_SEAL_OF_WEALTH:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1]);
		}
		break;
		case EFFECT_GLADIATORS_GLORY:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_PARTY_EXPERIENCE_BONUS:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1], aRecv->iValue[2]);
		}
		break;
		case EFFECT_MAX_AG_BOOST_AURA:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_MAX_SD_BOOST_AURA:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_SCROLL_OF_HEALING:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddHPRecoveryRate += lpEffect->m_value[0];
		//lpObj->EffectOption.AddMaxHP += lpEffect->m_value[1];
		break;
		case EFFECT_HAWK_FIGURINE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddCriticalDamageRate += lpEffect->m_value[0];
		break;
		case EFFECT_GOAT_FIGURINE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddExcellentDamageRate += lpEffect->m_value[0];
		break;
		case EFFECT_OAK_CHARM:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddMaxHP += lpEffect->m_value[0];
		break;
		case EFFECT_MAPLE_CHARM:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddMaxMP += lpEffect->m_value[0];
		break;
		case EFFECT_GOLDEN_OAK_CHARM:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddMaxHP += lpEffect->m_value[0];
		//lpObj->EffectOption.AddMaxSD += lpEffect->m_value[1];
		break;
		case EFFECT_GOLDEN_MAPLE_CHARM:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddMaxMP += lpEffect->m_value[0];
		//lpObj->EffectOption.AddMaxBP += lpEffect->m_value[1];
		break;
		case EFFECT_WORN_HORSESHOE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddItemDropRate += lpEffect->m_value[0];
		break;
		case EFFECT_GREATER_IGNORE_DEFENSE_RATE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		//lpObj->EffectOption.AddIgnoreDefenseRate += lpEffect->m_value[0];
		break;
		case EFFECT_FITNESS:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		//lpObj->EffectOption.AddVitality += lpEffect->m_value[0];
		break;
		case EFFECT_GREATER_DEFENSE_SUCCESS_RATE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		//lpObj->EffectOption.AddDefenseSuccessRate += lpEffect->m_value[0];
		break;
		case EFFECT_IRON_DEFENSE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1]);
		}
		//lpObj->EffectOption.AddMaxHP += lpEffect->m_value[0];
		//lpObj->EffectOption.AddDefense += lpEffect->m_value[1];
		break;
		case EFFECT_GREATER_LIFE_ENHANCED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1]);
		}
		//lpObj->EffectOption.MulMaxHP += lpEffect->m_value[0];
		//lpObj->EffectOption.MulMaxMP += lpEffect->m_value[1];
		break;
		case EFFECT_GREATER_LIFE_MASTERED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1], aRecv->iValue[2]);
		}
		//lpObj->EffectOption.MulMaxHP += lpEffect->m_value[0];
		//lpObj->EffectOption.MulMaxMP += lpEffect->m_value[1];
		//lpObj->EffectOption.MulMaxBP += lpEffect->m_value[2];
		break;
		case EFFECT_DEATH_STAB_ENHANCED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_MAGIC_CIRCLE_IMPROVED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1]);
		}
		break;
		case EFFECT_MAGIC_CIRCLE_ENHANCED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1], aRecv->iValue[2]);
		}
		break;
		case EFFECT_MANA_SHIELD_MASTERED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1]);
		}
		break;
		case EFFECT_FROZEN_STAB_MASTERED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_BLESS:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[0], aRecv->iValue[0], aRecv->iValue[0]);
		}
		break;
		case EFFECT_INFINITY_ARROW_IMPROVED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_BLIND_IMPROVED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_DRAIN_LIFE_ENHANCED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_ICE_STORM_ENHANCED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_EARTH_PRISON:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_GREATER_CRITICAL_DAMAGE_MASTERED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1]);
		}
		break;
		case EFFECT_GREATER_CRITICAL_DAMAGE_EXTENDED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1], aRecv->iValue[2]);
		}
		break;
		case EFFECT_SWORD_POWER_IMPROVED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1], aRecv->iValue[2], aRecv->iValue[3]);
		}
		break;
		case EFFECT_SWORD_POWER_ENHANCED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1], aRecv->iValue[2], aRecv->iValue[3]);
		}
		break;
		case EFFECT_SWORD_POWER_MASTERED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1], aRecv->iValue[2], aRecv->iValue[3]);
		}
		break;
		case EFFECT_GREATER_DEFENSE_SUCCESS_RATE_IMPROVED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_GREATER_DEFENSE_SUCCESS_RATE_ENHANCED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[1]);
		}
		break;
		case EFFECT_FITNESS_IMPROVED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_DRAGON_ROAR_ENHANCED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_CHAIN_DRIVER_ENHANCED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_POISON_ARROW:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_POISON_ARROW_IMPROVED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_BLESS_IMPROVED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0], aRecv->iValue[0], aRecv->iValue[0], aRecv->iValue[0]);
		}
		break;
		case EFFECT_LESSER_DAMAGE_IMPROVED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_LESSER_DEFENSE_IMPROVED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_FIRE_SLASH_ENHANCED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, aRecv->iValue[0]);
		}
		break;
		case EFFECT_IRON_DEFENSE_IMPROVED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_BLOOD_HOWLING:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_BLOOD_HOWLING_IMPROVED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_PENTAGRAM_JEWEL_HALF_SD:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.DivMaxSD += lpEffect->m_value[0];
		break;
		case EFFECT_PENTAGRAM_JEWEL_HALF_MP:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.DivMaxMP += lpEffect->m_value[0];
		break;
		case EFFECT_PENTAGRAM_JEWEL_HALF_SPEED:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.DivPhysiSpeed += lpEffect->m_value[0];
		//lpObj->EffectOption.DivMagicSpeed += lpEffect->m_value[0];
		break;
		case EFFECT_PENTAGRAM_JEWEL_HALF_HP:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.DivMaxHP += lpEffect->m_value[0];
		break;
		case EFFECT_PENTAGRAM_JEWEL_STUN:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_PENTAGRAM_JEWEL_SLOW:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		break;
		case EFFECT_TALISMAN_OF_ASCENSION1:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddExperienceRate += lpEffect->m_value[0];
		//lpObj->EffectOption.AddMasterExperienceRate += lpEffect->m_value[0];
		break;
		case EFFECT_TALISMAN_OF_ASCENSION2:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddExperienceRate += lpEffect->m_value[0];
		//lpObj->EffectOption.AddMasterExperienceRate += lpEffect->m_value[0];
		break;
		case EFFECT_TALISMAN_OF_ASCENSION3:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddExperienceRate += lpEffect->m_value[0];
		//lpObj->EffectOption.AddMasterExperienceRate += lpEffect->m_value[0];
		break;
		case EFFECT_SEAL_OF_ASCENSION3:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddExperienceRate += lpEffect->m_value[0];
		break;
		case EFFECT_MASTER_SEAL_OF_ASCENSION2:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddMasterExperienceRate += lpEffect->m_value[0];
		break;
		case EFFECT_BLESSING_OF_LIGHT:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddExperienceRate += lpEffect->m_value[0];
		//lpObj->EffectOption.AddMasterExperienceRate += lpEffect->m_value[0];
		break;
		case EFFECT_MASTER_SCROLL_OF_DEFENSE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddDefense += lpEffect->m_value[0];
		break;
		case EFFECT_MASTER_SCROLL_OF_MAGIC_DAMAGE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddMagicDamage += lpEffect->m_value[0];
		break;
		case EFFECT_MASTER_SCROLL_OF_LIFE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddMaxHP += lpEffect->m_value[0];
		break;
		case EFFECT_MASTER_SCROLL_OF_MANA:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddMaxMP += lpEffect->m_value[0];
		break;
		case EFFECT_MASTER_SCROLL_OF_DAMAGE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddPhysiDamage += lpEffect->m_value[0];
		break;
		case EFFECT_MASTER_SCROLL_OF_HEALING:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddHPRecoveryRate += lpEffect->m_value[0];
		//lpObj->EffectOption.AddMaxHP += lpEffect->m_value[1];
		break;
		case EFFECT_MASTER_SCROLL_OF_BATTLE:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddCriticalDamageRate += lpEffect->m_value[0];
		break;
		case EFFECT_MASTER_SCROLL_OF_STRENGTH:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddExcellentDamageRate += lpEffect->m_value[0];
		break;
		case EFFECT_MASTER_SCROLL_OF_QUICK:
		{
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
		}
		//lpObj->EffectOption.AddPhysiSpeed += lpEffect->m_value[0];
		//lpObj->EffectOption.AddMagicSpeed += lpEffect->m_value[0];
		break;
		default:
			wsprintf(RenderInfoBuff[BuffIndex], it->second.InfoBuff, 100);
			break;
		}
	}
}