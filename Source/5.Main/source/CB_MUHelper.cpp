#include "stdafx.h"
#include "CB_MUHelper.h"
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
#include "SkillManager.h"
#include "ZzzInterface.h"
#include "wsclientinline.h"
#include "ZzzAI.h"

extern bool LogOut;
extern int TargetX, TargetY;
extern MovementSkill g_MovementSkill;
extern int SelectedCharacter;
SpinLock _targetsLock;
SpinLock _itemsLock;

CB_MUHelper* gCB_MUHelper = nullptr;
int CheckKQ = 0;
CB_MUHelper::CB_MUHelper()
{
	CB_MUHelper::ClearData();

	for (int i = 0; i < eMaxVaule; i++)
	{
		CacheAction[i] = 0;
	}
	m_ActionMoveRegen = 0;
	TimeDrawCircle = 0;
	RangeDraw = -1;
}

CB_MUHelper::~CB_MUHelper()
{

}
void CB_MUHelper::ClearData()
{
	m_setTargets.clear();
	m_setTargetsAttacking.clear();
	m_setItems.clear();
	m_iCurrentItem = -1;
	m_iCurrentTarget = -1;
	m_iCurrentItemMove = -1;
	ClearDateOn = false;
	m_bTimerActivatedBuffOngoing = true;


	m_SelectedCharacter = -1;
	m_DistanceSelectCharacter = -1;



}
void CB_MUHelper::AddTarget(int iTargetId, bool bIsAttacking)
{
	_targetsLock.lock();

	m_setTargets.insert(iTargetId);

	if (bIsAttacking)
	{
		m_setTargetsAttacking.insert(iTargetId);
	}

	_targetsLock.unlock();
}

void CB_MUHelper::DeleteTarget(int iTargetId)
{
	_targetsLock.lock();

	m_setTargets.erase(iTargetId);
	m_setTargetsAttacking.erase(iTargetId);

	_targetsLock.unlock();

	if (iTargetId == m_iCurrentTarget)
	{
		m_iCurrentTarget = -1;
	}
}


void CB_MUHelper::AddItem(int iItemId)
{

	_itemsLock.lock();
	m_setItems.insert(iItemId);
	_itemsLock.unlock();


}

void CB_MUHelper::DeleteItem(int iItemId)
{
	_itemsLock.lock();
	m_setItems.erase(iItemId);
	_itemsLock.unlock();

	if (iItemId == m_iCurrentItem)
	{
		m_iCurrentItem = MAX_ITEMS;
	}
}

double GetDistance(CHARACTER* a2)
{
	float v3; // [esp+4h] [ebp-18h]
	float v4; // [esp+14h] [ebp-8h]
	float v5; // [esp+18h] [ebp-4h]

	if (!a2)
		return 0.0;
	v4 = (float)(Hero->PositionX - a2->PositionX);
	v5 = (float)(Hero->PositionY - a2->PositionY);
	v3 = v4 * v4 + v5 * v5;
	//return sqrtf(v3);
	return sqrt(pow(((float)Hero->PositionX - (float)a2->PositionX), 2) + pow(((float)Hero->PositionY - (float)a2->PositionY), 2));
}
double GetDistanceXY(int X, int Y, int bX, int bY)
{
	float v3; // [esp+4h] [ebp-18h]
	float v4; // [esp+14h] [ebp-8h]
	float v5; // [esp+18h] [ebp-4h]

	v4 = (float)(X - bX);
	v5 = (float)(Y - bY);
	v3 = v4 * v4 + v5 * v5;
	//return sqrtf(v3);
	return sqrt(pow(((float)X - (float)bX), 2) + pow(((float)Y - (float)bY), 2));
}
int BuffTick = 0;
int GetSkillBuffParty(int TypeSkill)
{
	switch (TypeSkill)
	{
	case 1:
	{
		if (gCharacterManager.GetBaseClass(Hero->Class) == CNewUIBCustomMenuInfo::Fairy_Elf)
		{
			//=== khong co skill mns thi bo qua
			if (gSkillManager.FindHeroSkill((ActionSkillType)AT_SKILL_HEALING)) return AT_SKILL_HEALING;
			if (gSkillManager.FindHeroSkill((ActionSkillType)MASTER_SKILL_ADD_HEAL_IMPROVED)) return MASTER_SKILL_ADD_HEAL_IMPROVED;
		}
	}
	break;
	case 2:
	{
		if (gCharacterManager.GetBaseClass(Hero->Class) == CNewUIBCustomMenuInfo::Dark_Wizard)
		{
			//=== khong co skill mns thi bo qua
			if (gSkillManager.FindHeroSkill(AT_SKILL_WIZARDDEFENSE)) return AT_SKILL_WIZARDDEFENSE;
			if (gSkillManager.FindHeroSkill((ActionSkillType)MASTER_SKILL_ADD_MANA_SHIELD_IMPROVED)) return MASTER_SKILL_ADD_MANA_SHIELD_IMPROVED;
			if (gSkillManager.FindHeroSkill((ActionSkillType)MASTER_SKILL_ADD_MANA_SHIELD_ENHANCED)) return MASTER_SKILL_ADD_MANA_SHIELD_ENHANCED;
			if (gSkillManager.FindHeroSkill((ActionSkillType)MASTER_SKILL_ADD_MANA_SHIELD_MASTERED)) return MASTER_SKILL_ADD_MANA_SHIELD_MASTERED;
		}
	}
	break;
	case 3:
	{
		if (gCharacterManager.GetBaseClass(Hero->Class) == CNewUIBCustomMenuInfo::Fairy_Elf)
		{

			if (gSkillManager.FindHeroSkill((ActionSkillType)AT_SKILL_DEFENSE)) return AT_SKILL_DEFENSE;
			if (gSkillManager.FindHeroSkill((ActionSkillType)MASTER_SKILL_ADD_GREATER_DEFENSE_IMPROVED)) return MASTER_SKILL_ADD_GREATER_DEFENSE_IMPROVED;
			if (gSkillManager.FindHeroSkill((ActionSkillType)MASTER_SKILL_ADD_GREATER_DEFENSE_ENHANCED)) return MASTER_SKILL_ADD_GREATER_DEFENSE_ENHANCED;
		}
	}
	break;
	case 4:
	{
		if (gCharacterManager.GetBaseClass(Hero->Class) == CNewUIBCustomMenuInfo::Fairy_Elf)
		{

			if (gSkillManager.FindHeroSkill((ActionSkillType)AT_SKILL_ATTACK)) return AT_SKILL_ATTACK;
			if (gSkillManager.FindHeroSkill((ActionSkillType)MASTER_SKILL_ADD_GREATER_DAMAGE_IMPROVED)) return MASTER_SKILL_ADD_GREATER_DAMAGE_IMPROVED;
			if (gSkillManager.FindHeroSkill((ActionSkillType)MASTER_SKILL_ADD_GREATER_DAMAGE_ENHANCED)) return MASTER_SKILL_ADD_GREATER_DAMAGE_ENHANCED;
		}
	}
	break;
	case 5: //Get Skill hut mau
	{
		if (gCharacterManager.GetBaseClass(Hero->Class) == CNewUIBCustomMenuInfo::Summoner)
		{
			if (gSkillManager.FindHeroSkill((ActionSkillType)AT_SKILL_ALICE_DRAINLIFE)) return AT_SKILL_ALICE_DRAINLIFE;
			if (gSkillManager.FindHeroSkill((ActionSkillType)MASTER_SKILL_ADD_DRAIN_LIFE_IMPROVED)) return MASTER_SKILL_ADD_DRAIN_LIFE_IMPROVED;
			if (gSkillManager.FindHeroSkill((ActionSkillType)MASTER_SKILL_ADD_DRAIN_LIFE_ENHANCED)) return MASTER_SKILL_ADD_DRAIN_LIFE_ENHANCED;
		}
	}
	break;
	default:
		break;
	}
	return -1;
}

int CB_MUHelper::SimulateSkill(int iSkill, bool bTargetRequired, int iTarget)
{
	extern MovementSkill g_MovementSkill;
	extern int SelectedCharacter;
	extern int TargetX, TargetY;

	g_MovementSkill.m_iSkill = iSkill;
	g_MovementSkill.m_bMagic = true;

	float fSkillDistance = gSkillManager.GetSkillDistance(iSkill, Hero);

	if (bTargetRequired)
	{
		if (iTarget == -1)
		{
			CheckKQ = 1;
			return 0;
		}

		SelectedCharacter = FindCharacterIndex(iTarget);
		if (SelectedCharacter == MAX_CHARACTERS_CLIENT)
		{
			DeleteTarget(iTarget);
			CheckKQ = 2;
			return 0;
		}

		CHARACTER* pTarget = &CharactersClient[SelectedCharacter];
		if (pTarget->Dead)
		{
			DeleteTarget(iTarget);
			CheckKQ = 3;
			return 0;
		}


		//gInterface.DrawMessage(1, "Target %d (%d) (%d,%d) Skill %d %f / %d", iTarget, pTarget->Object.Kind , CheckTile(Hero, &Hero->Object, fSkillDistance), CheckWall(Hero->PositionX, Hero->PositionY, TargetX, TargetY), m_iCurrentSkill, gSkillManager.GetSkillDistance(iSkill, Hero), ComputeDistanceFromTarget(pTarget));
		//
		TargetX = (int)(pTarget->Object.Position[0] / TERRAIN_SCALE);
		TargetY = (int)(pTarget->Object.Position[1] / TERRAIN_SCALE);

		g_MovementSkill.m_iTarget = SelectedCharacter;
		float DistanceTarget = GetDistanceXY(Hero->PositionX, Hero->PositionY, pTarget->PositionX, pTarget->PositionY);
		if (DistanceTarget < (fSkillDistance - 1.0f))
		{
			//gInterface.DrawMessage(1, "AttackNext %d %d/%d", iTarget, ComputeDistanceFromTarget(pTarget) , (int)(fSkillDistance - 1.0f));
			TargetX = Hero->PositionX;
			TargetY = Hero->PositionY;
			goto AttackNext;
			return 1;
		}

		PATH_t tempPath;

		bool bHasPath = PathFinding2(Hero->PositionX, Hero->PositionY, TargetX, TargetY, &tempPath, fSkillDistance + 5);
		bool bTargetNear = CheckTile(Hero, &Hero->Object, fSkillDistance);
		bool bNoWall = CheckWall(Hero->PositionX, Hero->PositionY, TargetX, TargetY);

		// target not reachable, ignore it
		if (!bHasPath)
		{
			DeleteTarget(iTarget);
			CheckKQ = 4;
			return 0;
		}

		// target is not near or the path is obstructed by a wall, move closer
		if (!bTargetNear || !bNoWall)
		{
			//Hero->Path.Lock.lock();

			// Limit movement to 2 steps at a time
			int pathNum = min(tempPath.PathNum, 2);
			for (int i = 0; i < pathNum; i++)
			{
				Hero->Path.PathX[i] = tempPath.PathX[i];
				Hero->Path.PathY[i] = tempPath.PathY[i];
			}
			Hero->Path.PathNum = pathNum;
			Hero->Path.CurrentPath = 0;
			Hero->Path.CurrentPathFloat = 0;

			//Hero->Path.Lock.unlock();

			SendMove(Hero, &Hero->Object);
			CheckKQ = 5;
			return 0;
		}

	}
	else
	{
		SelectedCharacter = FindCharacterIndex(Hero->Key);
		g_MovementSkill.m_iTarget = SelectedCharacter;
		TargetX = Hero->PositionX;
		TargetY = Hero->PositionY;
	}
AttackNext:
	int iSkillResult = ExecuteSkill(Hero, iSkill, fSkillDistance);
	if (iSkillResult == -1)
	{
		DeleteTarget(iTarget);
	}
	CheckKQ = 6;
	return (int)(iSkillResult == 1);
}


int CB_MUHelper::BuffTarget(CHARACTER * pTargetChar, int iBuffSkill)
{
	// TODO: List other buffs here
	if ((iBuffSkill == AT_SKILL_ATTACK
		|| iBuffSkill == AT_SKILL_ATT_POWER_UP
		|| iBuffSkill == MASTER_SKILL_ADD_GREATER_DAMAGE_IMPROVED
		|| iBuffSkill == MASTER_SKILL_ADD_GREATER_DAMAGE_ENHANCED
		|| iBuffSkill == AT_SKILL_ATT_POWER_UP + 1
		|| iBuffSkill == AT_SKILL_ATT_POWER_UP + 2
		|| iBuffSkill == AT_SKILL_ATT_POWER_UP + 3
		|| iBuffSkill == AT_SKILL_ATT_POWER_UP + 4)
		&& ((!g_isCharacterBuff((&pTargetChar->Object), eBuff_Attack))
			))
	{
		return 0;
	}

	if ((iBuffSkill == AT_SKILL_DEFENSE
		|| iBuffSkill == MASTER_SKILL_ADD_GREATER_DEFENSE_IMPROVED
		|| iBuffSkill == MASTER_SKILL_ADD_GREATER_DEFENSE_ENHANCED
		|| iBuffSkill == AT_SKILL_DEF_POWER_UP
		|| iBuffSkill == AT_SKILL_DEF_POWER_UP + 1
		|| iBuffSkill == AT_SKILL_DEF_POWER_UP + 2
		|| iBuffSkill == AT_SKILL_DEF_POWER_UP + 3
		|| iBuffSkill == AT_SKILL_DEF_POWER_UP + 4)
		&& ((!g_isCharacterBuff((&pTargetChar->Object), eBuff_Defense))
			))
	{
		return 0;
	}



	if ((iBuffSkill == AT_SKILL_WIZARDDEFENSE
		|| iBuffSkill == MASTER_SKILL_ADD_MANA_SHIELD_IMPROVED
		|| iBuffSkill == MASTER_SKILL_ADD_MANA_SHIELD_ENHANCED
		|| iBuffSkill == MASTER_SKILL_ADD_MANA_SHIELD_MASTERED
		|| iBuffSkill == AT_SKILL_SOUL_UP
		|| iBuffSkill == AT_SKILL_SOUL_UP + 1
		|| iBuffSkill == AT_SKILL_SOUL_UP + 2
		|| iBuffSkill == AT_SKILL_SOUL_UP + 3
		|| iBuffSkill == AT_SKILL_SOUL_UP + 4)
		&& ((!g_isCharacterBuff((&pTargetChar->Object), eBuff_PhysDefense)
			&& !g_isCharacterBuff((&pTargetChar->Object), EFFECT_MANA_SHIELD_MASTERED))))
	{
		return 0;
	}

	if ((iBuffSkill == AT_SKILL_VITALITY
		|| iBuffSkill == MASTER_SKILL_ADD_GREATER_LIFE_IMPROVED
		|| iBuffSkill == MASTER_SKILL_ADD_GREATER_LIFE_ENHANCED
		|| iBuffSkill == MASTER_SKILL_ADD_GREATER_LIFE_MASTERED
		|| iBuffSkill == AT_SKILL_LIFE_UP
		|| iBuffSkill == AT_SKILL_LIFE_UP + 1
		|| iBuffSkill == AT_SKILL_LIFE_UP + 2
		|| iBuffSkill == AT_SKILL_LIFE_UP + 3
		|| iBuffSkill == AT_SKILL_LIFE_UP + 4)
		&& ((!g_isCharacterBuff((&pTargetChar->Object), eBuff_HpRecovery) && !g_isCharacterBuff((&pTargetChar->Object), EFFECT_GREATER_LIFE_ENHANCED) && !g_isCharacterBuff((&pTargetChar->Object), EFFECT_GREATER_LIFE_MASTERED))))
	{
		return 0;
	}



	if ((iBuffSkill == AT_SKILL_ADD_CRITICAL
		|| iBuffSkill == MASTER_SKILL_ADD_GREATER_CRITICAL_DAMAGE_EXTENDED
		|| iBuffSkill == MASTER_SKILL_ADD_GREATER_CRITICAL_DAMAGE_ENHANCED
		|| iBuffSkill == MASTER_SKILL_ADD_GREATER_CRITICAL_DAMAGE_MASTERED
		|| iBuffSkill == MASTER_SKILL_ADD_GREATER_CRITICAL_DAMAGE_IMPROVED
		)
		&& (!g_isCharacterBuff((&pTargetChar->Object), eBuff_AddCriticalDamage)
			&& !g_isCharacterBuff((&pTargetChar->Object), EFFECT_GREATER_CRITICAL_DAMAGE_MASTERED)
			&& !g_isCharacterBuff((&pTargetChar->Object), EFFECT_GREATER_CRITICAL_DAMAGE_EXTENDED)
			))
	{
		return 0;
	}

	if ((iBuffSkill == AT_SKILL_ALICE_BERSERKER
		|| iBuffSkill == MASTER_SKILL_ADD_SWORD_POWER_IMPROVED
		|| iBuffSkill == MASTER_SKILL_ADD_SWORD_POWER_ENHANCED
		|| iBuffSkill == MASTER_SKILL_ADD_SWORD_POWER_MASTERED
		)
		&& (!g_isCharacterBuff((&pTargetChar->Object), eBuff_Berserker)
			&& !g_isCharacterBuff((&pTargetChar->Object), EFFECT_SWORD_POWER_IMPROVED)
			&& !g_isCharacterBuff((&pTargetChar->Object), EFFECT_SWORD_POWER_ENHANCED)
			&& !g_isCharacterBuff((&pTargetChar->Object), EFFECT_SWORD_POWER_MASTERED)
			))
	{
		return 0;
	}
	if ((iBuffSkill == AT_SKILL_ALICE_THORNS)
		&& (!g_isCharacterBuff((&pTargetChar->Object), eBuff_Thorns)))
	{
		return 0;
	}
	//==RF
	if ((iBuffSkill == AT_SKILL_ATT_UP_OURFORCES)
		&& (!g_isCharacterBuff((&pTargetChar->Object), eBuff_Att_up_Ourforces)))
	{
		return 0;
	}
	if ((iBuffSkill == AT_SKILL_HP_UP_OURFORCES
		|| iBuffSkill == MASTER_SKILL_ADD_FITNESS_IMPROVED)
		&& (!g_isCharacterBuff((&pTargetChar->Object), eBuff_Hp_up_Ourforces) && !g_isCharacterBuff((&pTargetChar->Object), EFFECT_FITNESS_IMPROVED)))
	{
		return 0;
	}
	if ((iBuffSkill == AT_SKILL_DEF_UP_OURFORCES
		|| iBuffSkill == MASTER_SKILL_ADD_GREATER_DEFENSE_SUCCESS_RATE_IMPROVED
		|| iBuffSkill == MASTER_SKILL_ADD_GREATER_DEFENSE_SUCCESS_RATE_ENHANCED
		)
		&& (!g_isCharacterBuff((&pTargetChar->Object), eBuff_Def_up_Ourforces)
			&& !g_isCharacterBuff((&pTargetChar->Object), EFFECT_GREATER_DEFENSE_SUCCESS_RATE_IMPROVED)
			&& !g_isCharacterBuff((&pTargetChar->Object), EFFECT_GREATER_DEFENSE_SUCCESS_RATE_ENHANCED)
			))
	{
		return 0;
	}


	if (pTargetChar->Key == Hero->Key)
	{
		//=== Vo dinh tien
		if ((iBuffSkill == AT_SKILL_INFINITY_ARROW
			|| iBuffSkill == MASTER_SKILL_ADD_INFINITY_ARROW_IMPROVED)
			&&
			(!g_isCharacterBuff((&pTargetChar->Object), eBuff_InfinityArrow) && !g_isCharacterBuff((&pTargetChar->Object), EFFECT_INFINITY_ARROW_IMPROVED))
			)
		{
			return 0;
		}
		//=== Tang ma luc
		if ((iBuffSkill == AT_SKILL_SWELL_OF_MAGICPOWER
			|| iBuffSkill == MASTER_SKILL_ADD_MAGIC_CIRCLE_IMPROVED
			|| iBuffSkill == MASTER_SKILL_ADD_MAGIC_CIRCLE_ENHANCED
			)
			&& (!g_isCharacterBuff((&pTargetChar->Object), eBuff_SwellOfMagicPower)
				&& !g_isCharacterBuff((&pTargetChar->Object), EFFECT_MAGIC_CIRCLE_IMPROVED)
				&& !g_isCharacterBuff((&pTargetChar->Object), EFFECT_MAGIC_CIRCLE_ENHANCED)
				))
		{
			return 0;
		}
	}

	return 1;
}
int GetSkillBuffHero = 0;
bool MuHelperActiveBuff()
{

	if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.AutoBuff && gCB_MUHelper->CacheAction[CB_MUHelper::eBuff] < 6)
	{

		if (GetSkillBuffHero > 2) GetSkillBuffHero = 0;
		if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Buff[GetSkillBuffHero] != 0)
		{
			if (gSkillManager.FindHeroSkill((ActionSkillType)g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Buff[GetSkillBuffHero]) == 1 && g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Buff[GetSkillBuffHero] > 0)
			{
				int iSkillType = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Buff[GetSkillBuffHero];
				//DrawMessage(1, "Get Buff %d <> %d %d (%d)", gSkillManager.FindHeroCurrentSkill((ActionSkillType)iSkillType), iSkillType, gCB_MUHelper->BuffTarget(Hero, iSkillType), GetSkillBuffHero);

				if (!gCB_MUHelper->BuffTarget(Hero, iSkillType) && !Hero->SafeZone)
				{
					//DrawMessage(1, "MuHelperActiveBuff Hero %d (%d) (%d)", iSkillType, gCB_MUHelper->BuffTarget(Hero, iSkillType), GetSkillBuffHero);
					Hero->CurrentSkill = gSkillManager.FindHeroCurrentSkill((ActionSkillType)iSkillType);
					gCB_MUHelper->SimulateSkill(iSkillType, true, Hero->Key);
					gCB_MUHelper->CacheAction[CB_MUHelper::eBuff]++;
					return 0;
				}

			}
		}
		GetSkillBuffHero++;

	}
	if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PartyAutoBuff && gCB_MUHelper->CacheAction[CB_MUHelper::eBuffParty] < PartyNumber * 5) //Neu tick vao party buff
	{
		//if (GetTickCount() >= g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.TimerBuffParty)
		{
			//===Buff Party
			for (int i = 0; i < PartyNumber; i++)
			{
				if (Party[i].index >= 0 && Party[i].index < MAX_CHARACTERS_CLIENT)
				{

					CHARACTER* pChar = &CharactersClient[Party[i].index];
					OBJECT* pObj = &pChar->Object;
					int iSkillType = -1;
					if (pChar->SafeZone) continue;
					if (gCharacterManager.GetBaseClass(Hero->Class) == CNewUIBCustomMenuInfo::Dark_Wizard)
					{
						iSkillType = GetSkillBuffParty(2);
					}
					if (gCharacterManager.GetBaseClass(Hero->Class) == CNewUIBCustomMenuInfo::Fairy_Elf)
					{
						iSkillType = GetSkillBuffParty(3);
						if (iSkillType == -1 || gCB_MUHelper->BuffTarget(pChar, iSkillType))
						{
							iSkillType = GetSkillBuffParty(4);
						}
					}
					if (iSkillType == -1) break;



					if (!gCB_MUHelper->BuffTarget(pChar, iSkillType))
					{
						//DrawMessage(1, "1Get BuffPT %d <> %d %d", gSkillManager.FindHeroCurrentSkill((ActionSkillType)iSkillType), iSkillType, gCB_MUHelper->BuffTarget(pChar, iSkillType));
						Hero->CurrentSkill = gSkillManager.FindHeroCurrentSkill((ActionSkillType)iSkillType);
						gCB_MUHelper->SimulateSkill(iSkillType, true, pChar->Key);
						//DrawMessage(1, "MuHelperActiveBuff Char %s Party %d (%d) (%d)", Hero->ID, iSkillType, gCB_MUHelper->BuffTarget(Hero, iSkillType));
						gCB_MUHelper->CacheAction[CB_MUHelper::eBuffParty]++;
						return 0;
					}
				}
			}

			if (GetTickCount() >= g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.TimerBuffParty)
			{
				gCB_MUHelper->m_bTimerActivatedBuffOngoing = true;
				g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.TimerBuffParty = GetTickCount() * ((g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PartyBuffTime) * 1000);
			}
			else
			{
				gCB_MUHelper->m_bTimerActivatedBuffOngoing = false;
			}
		}
	}
	if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PartyAutoHeal && gCB_MUHelper->CacheAction[CB_MUHelper::eBuffPartyHeal] < PartyNumber * 5) //hoi mau trong nhom
	{
		//===Buff Party
		for (int i = 0; i < PartyNumber; i++)
		{
			if (Party[i].index >= 0 && Party[i].index < MAX_CHARACTERS_CLIENT)
			{
				CHARACTER* pChar = &CharactersClient[Party[i].index];
				OBJECT* pObj = &pChar->Object;
				if ((Party[i].stepHP * 10) < g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PartyHealPercent && !pChar->SafeZone)
				{
					int iSkillType = GetSkillBuffParty(1);
					if (iSkillType == -1) break;
					Hero->CurrentSkill = gSkillManager.FindHeroCurrentSkill((ActionSkillType)iSkillType);
					SelectedCharacter = Party[i].index;
					gCB_MUHelper->SimulateSkill(iSkillType, true, pChar->Key);
					gCB_MUHelper->CacheAction[CB_MUHelper::eBuffPartyHeal]++;
					return 0;

				}
			}
		}

	}
	if (gCB_MUHelper->CacheAction[CB_MUHelper::eBuff] > 0) gCB_MUHelper->CacheAction[CB_MUHelper::eBuff]--;
	if (gCB_MUHelper->CacheAction[CB_MUHelper::eBuffParty] > 0) gCB_MUHelper->CacheAction[CB_MUHelper::eBuffParty]--;
	if (gCB_MUHelper->CacheAction[CB_MUHelper::eBuffPartyHeal] > 0) gCB_MUHelper->CacheAction[CB_MUHelper::eBuffPartyHeal]--;
	return 1;
}

int ActiveCheckMonterCount(int SkillNum)
{
	if (gSkillManager.FindHeroSkill((ActionSkillType)g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[SkillNum]) == 0)
	{
		return -1;
	}

	if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SkillDelay[SkillNum - 1] == 1)
	{
		if ((GetTickCount() - g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.TimerDelay[SkillNum]) >= (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Delay[SkillNum - 1]) * 1000) //s
		{
			return 1;
		}
	}
	else if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SkillCondition[SkillNum - 1] == 1)
	{
		float Distance = 0;
		int HostileEnemyNum = 0;
		for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
		{
			CHARACTER* c = &CharactersClient[i];
			OBJECT* o = &c->Object;
			if (KIND_MONSTER == o->Kind && o->Live && o->Visible && o->Alpha > 0.f && o->CurrentAction != 6)
			{
				Distance = GetDistance(c);
				//==Khoang cach rong
				if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.LongDistance)
				{
					if (Distance < g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Range[0] + 1)
					{
						HostileEnemyNum++;
					}
				}
				else
				{
					float SkillDistance = gSkillManager.GetSkillDistance(gSkillManager.FindHeroCurrentSkill((ActionSkillType)g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[SkillNum]), Hero);
					//if (Distance < g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Range[0]+1)
					if (Distance < SkillDistance)
					{
						HostileEnemyNum++;
					}

				}
			}
		}
		//char zbtemp[128] = { 0 };
		//sprintf_s(zbtemp, "ActiveCheckMonterCount %d (%d)", HostileEnemyNum, g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SkillSubCon[SkillNum - 1]);
		//g_pChatListBox->AddText("", zbtemp, SEASON3B::TYPE_WHISPER_MESSAGE);
		//DrawMessage(1, "%d Skill Get %d (%d)  ", SkillNum - 1,HostileEnemyNum, g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SkillSubCon[SkillNum - 1]);
		switch (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SkillSubCon[SkillNum - 1])
		{
		case 3:
			if (HostileEnemyNum >= 8) return 3;
			break;
		case 2:
			if (HostileEnemyNum >= 6) return 2;
			break;
		case 1:
			if (HostileEnemyNum >= 4) return 1;
			break;
		case 0:
			if (HostileEnemyNum >= 2) return 0;
			break;
		default:
			return -1;
		}

		return -1;
	}
	return -1;
}
void CB_MUHelper::GetTarget()
{
	float Distance = 0;
	float DistanceTarget = 0;
	int Target = -1;
	float CachDis = 0;
	for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
	{
		CHARACTER* c = &CharactersClient[i];
		OBJECT* o = &c->Object;
		//if (KIND_MONSTER == o->Kind && o->Live && o->Visible && o->Alpha > 0.f && o->CurrentAction != 6)
		if (KIND_MONSTER == o->Kind && o->Live)
		{
			Distance = GetDistance(c);
			if (!g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.LongDistance)
			{
				DistanceTarget = GetDistanceXY(c->PositionX, c->PositionY, g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.StartX, g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.StartY);
			}
			else
			{
				DistanceTarget = GetDistanceXY(c->PositionX, c->PositionY, Hero->PositionX, Hero->PositionY);
			}

			if (DistanceTarget > g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Range[0] + 1)
			{
				continue;
			}
			this->AddTarget(c->Key, false);
		}
	}
}
int CB_MUHelper::GetMonter()
{
	float Distance = 0;
	float DistanceTarget = 0;
	int Target = -1;
	float CachDis = 0;

	gCB_MUHelper->CheckLiveTarget();

	if (m_SelectedCharacter != -1)
	{
		return m_SelectedCharacter;
	}

	GetTarget();

	if (!m_setTargets.empty())
	{
		CleanupTargets();
		int iMinDistance = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Range[0] + 1;

		std::set<int> setTargets;
		{
			_targetsLock.lock();
			setTargets = m_setTargets;
			_targetsLock.unlock();
		}

		for (const int& iMonsterId : setTargets)
		{
			int iIndex = FindCharacterIndex(iMonsterId);
			CHARACTER* pTarget = &CharactersClient[iIndex];

			int iDistance = GetDistance(pTarget);
			if (iDistance < iMinDistance)
			{
				iMinDistance = iDistance;
				Target = iIndex;
				//===
				m_SelectedCharacter = Target;
				m_DistanceSelectCharacter = iMinDistance;
			}
		}
	}



	return Target;
}

bool CB_MUHelper::MuHelperActiveAttack()
{
	int SelectSkillNum = 0;
	int GetCount1 = ActiveCheckMonterCount(1);
	int GetCount2 = ActiveCheckMonterCount(2);

	//===Combo
	if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Combo == 1)
	{
		SelectSkillNum = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SkillCache;
	}
	else
	{
		if (GetCount1 != -1 || GetCount2 != -1)
		{
			SelectSkillNum = (GetCount2 > GetCount1 ? 2 : 1);
			goto SetAttackSkill;
		}
	}
SetAttackSkill:
	//DrawMessage(1, "%d ~ %d (%d) Check %d", GetCount1, GetCount2, SelectSkillNum, gSkillManager.FindHeroSkill((ActionSkillType)g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[SelectSkillNum]));
	if (SelectSkillNum == -1 || gSkillManager.FindHeroSkill((ActionSkillType)g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[SelectSkillNum]) == 0) SelectSkillNum = 0;
	if (gSkillManager.FindHeroSkill((ActionSkillType)g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[SelectSkillNum]) == 1)
	{
		SelectedCharacter = GetMonter();

		int iSkillType = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[SelectSkillNum];
		//Hero->CurrentSkill = gSkillManager.FindHeroCurrentSkill((ActionSkillType)iSkillType);
		//g_MovementSkill.m_iSkill = gSkillManager.FindHeroCurrentSkill((ActionSkillType)iSkillType);
		//g_MovementSkill.m_bMagic = true;
		if (SelectedCharacter != -1)
		{
			//Attacking = 2;
			gCB_MUHelper->SimulateSkill(iSkillType, true, CharactersClient[SelectedCharacter].Key);
			//MouseRButtonPush = 1;
			g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Stage++;

			g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.TimerDelay[SelectSkillNum] = GetTickCount();
			g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SkillCache = (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SkillCache >= 2 ? 0 : g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SkillCache + 1);
			//DrawMessage(1, "Attack Skill %d -> Monter %d", g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[SelectSkillNum], SelectedCharacter);
		}

	}
	return 1;
}
bool CB_MUHelper::MuHelperActiveHeal()
{
	BYTE RateHP = (BYTE)((CharacterAttribute->PrintPlayer.ViewCurHP * 100) / (CharacterAttribute->PrintPlayer.ViewMaxHP));
	//==SKill Heal  , hut mau
	if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.AutoHeal && gCB_MUHelper->CacheAction[CB_MUHelper::eHeal] < 25)
	{
		//DrawMessage(1, "Heal HP %d / %d", RateHP, g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.HealPercent);
		if (RateHP <= g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.HealPercent)
		{
			if (gCharacterManager.GetBaseClass(Hero->Class) == CNewUIBCustomMenuInfo::Fairy_Elf)
			{
				int iSkillType = GetSkillBuffParty(1); //lay skill hoi mau
				if (iSkillType != -1)
				{
					g_MovementSkill.m_iSkill = gSkillManager.FindHeroCurrentSkill((ActionSkillType)iSkillType);
					g_MovementSkill.m_bMagic = true;
					Hero->CurrentSkill = gSkillManager.FindHeroCurrentSkill((ActionSkillType)iSkillType);
					SelectedCharacter = FindCharacterIndex(Hero->Key);
					g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Stage++;
					gCB_MUHelper->CacheAction[CB_MUHelper::eHeal]++;
					return 0;
				}
			}
			else 	if (gCharacterManager.GetBaseClass(Hero->Class) == CNewUIBCustomMenuInfo::Summoner)
			{
				int iSkillType = GetSkillBuffParty(5); //lay skill hut maiu
				if (iSkillType != -1)
				{
					g_MovementSkill.m_iSkill = gSkillManager.FindHeroCurrentSkill((ActionSkillType)iSkillType);
					g_MovementSkill.m_bMagic = true;
					Hero->CurrentSkill = gSkillManager.FindHeroCurrentSkill((ActionSkillType)iSkillType);
					SelectedCharacter = GetMonter();
					g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Stage++;
					gCB_MUHelper->CacheAction[CB_MUHelper::eHeal]++;
					return 0;
				}
			}
		}
	}
	//==Dung thuoc hoi mau
	if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.AutoPotion)
	{
		if (RateHP <= g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PotPercent)
		{
			int Index = g_pMyInventory->FindHPItemIndex();

			if (Index != -1)
			{
				SendRequestUse(Index, 0);
			}
		}
	}
	if (gCB_MUHelper->CacheAction[CB_MUHelper::eHeal] > 0) gCB_MUHelper->CacheAction[CB_MUHelper::eHeal]--;
	return 1;
}
bool CompareStringsIgnoringSpaces(const std::string & strA, const std::string & strB)
{

	size_t lenA = strA.size();
	size_t lenB = strB.size();

	if (lenA < lenB)
	{
		//	std::cout << "CompareStringsIgnoringSpaces 1" << std::endl;
		return false;
	}
	int j = 0;
	for (int i = 0; i < lenB; ++i)
	{
		if (tolower(strA[i]) == ' ' || tolower(strB[i]) == ' ')
		{
			continue;
		}
		if (tolower(strA[i]) != tolower(strB[i]))
		{
			std::string charAsStringA(1, strA[i]);
			std::string charAsStringB(1, strB[i]);
			//DrawMessage(4, "Khac (%d) %s ~ %s", i, charAsStringA.c_str(), charAsStringB.c_str());
			//std::cout << "CompareStringsIgnoringSpaces 2 " << charAsStringA << " ~ " << charAsStringB << std::endl;
			return false;
		}

	}

	return true;
}
bool FindItemName(const std::string & strA, const std::string & strB)
{
	for (int i = 0; i < strA.size(); ++i)
	{
		if (tolower(strA[i]) == tolower(strB[0]))
		{

			std::string remainingString = strA.substr(i);

			if (remainingString.size() >= strB.size())
			{
				if (CompareStringsIgnoringSpaces(remainingString, strB))
				{
					return true;
				}
			}
		}
	}
	return false;
}

static std::vector<std::string> accentChars =
{
	"á", "à", "ả", "ã", "ạ", "ă", "ắ", "ằ", "ẳ", "ẵ", "ặ", "â", "ấ", "ầ", "ẩ", "ẫ", "ậ",
	"é", "è", "ẻ", "ẽ", "ẹ", "ê", "ế", "ề", "ể", "ễ", "ệ",
	"í", "ì", "ỉ", "ĩ", "ị",
	"ó", "ò", "ỏ", "õ", "ọ", "ô", "ố", "ồ", "ổ", "ỗ", "ộ", "ơ", "ớ", "ờ", "ở", "ỡ", "ợ",
	"ú", "ù", "ủ", "ũ", "ụ", "ư", "ứ", "ừ", "ử", "ữ", "ự",
	"ý", "ỳ", "ỷ", "ỹ", "ỵ",
	"Á", "À", "Ả", "Ã", "Ạ", "Â", "Ấ", "Ầ", "Ẩ", "Ẫ", "Ậ",
	"Ă", "Ắ", "Ằ", "Ẳ", "Ẵ", "Ặ",
	"É", "È", "Ẻ", "Ẽ", "Ẹ", "Ê", "Ế", "Ề", "Ể", "Ễ", "Ệ",
	"Í", "Ì", "Ỉ", "Ĩ", "Ị",
	"Ó", "Ò", "Ỏ", "Õ", "Ọ", "Ô", "Ố", "Ồ", "Ổ", "Ỗ", "Ộ",
	"Ơ", "Ớ", "Ờ", "Ở", "Ỡ", "Ợ",
	"Ú", "Ù", "Ủ", "Ũ", "Ụ", "Ư", "Ứ", "Ừ", "Ử", "Ữ", "Ự",
	"Ý", "Ỳ", "Ỷ", "Ỹ", "Ỵ"
};
void CheckSyntax(char* text) // OK
{
	for (int n = 0; n < accentChars.size(); n++)
	{
		char* temp = text;

		while (true)
		{
			temp = strstr(temp, accentChars[n].c_str());

			if (temp == 0)
			{
				break;
			}

			int len = strlen(accentChars[n].c_str());

			memset(temp, 0x20, len);

			temp += len;
		}
	}
}

bool PickUpItemName(ITEM * pItem)
{
	unicode::t_char szItemName[128];
	GetItemName(pItem->Type, (pItem->Level >> 3) & 15, szItemName);
	CheckSyntax(szItemName);
	std::string szItemNameString = szItemName;
	szItemNameString.erase(std::remove(szItemNameString.begin(), szItemNameString.end(), ' '), szItemNameString.end());

	//DrawMessage(4, "Item Drop %s (%d %d", szItemNameString.c_str(), pItem->Type, pItem->Level);
	for (int n = 0; n < MAX_HELPER_ITEM; n++)
	{
		if (strlen(g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.ItemList[n]) > 0)
		{
			unicode::t_char sztemList[128];
			::wsprintf(sztemList, "%s", g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.ItemList[n]);
			CheckSyntax(sztemList);
			std::string ItemListString = sztemList;

			ItemListString.erase(std::remove(ItemListString.begin(), ItemListString.end(), ' '), ItemListString.end());

			//bool CheckText = FindText(szItemNameString.c_str(), ItemListString.c_str());

			//DrawMessage(4, "%s(%d) ~ %s(%d) (%d)", szItemNameString.c_str(),strlen(szItemNameString.c_str()), ItemListString.c_str(),strlen(ItemListString.c_str()), CheckText);
			//== tim 2 chuoi trung nhau
			//if (FindItemName(szItemName, g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.ItemList[n]))
			if (FindItemName(szItemNameString.c_str(), ItemListString.c_str()))
			{
				return 1;
				break;
			}

		}
	}

	return 0;
}

int BComputeDistanceBetween(POINT posA, POINT posB)
{
	int iDx = posA.x - posB.x;
	int iDy = posA.y - posB.y;

	return static_cast<int>(std::ceil(std::sqrt(iDx * iDx + iDy * iDy)));
}
int BomputeDistanceByRange(int iRange)
{
	return BComputeDistanceBetween({ 0, 0 }, { iRange, iRange });
}
struct ItemDistance
{
	int index;     // Chỉ số của item
	float distance;  // Khoảng cách tới Hero
};
void GetListItemPickUp()
{
	for (int i = 0; i < MAX_ITEMS; i++)
	{
		OBJECT* o = &Items[i].Object;
		if (o->Live && o->Visible)
		{
			int dx = (int)(o->Position[0] / TERRAIN_SCALE);
			int dy = (int)(o->Position[1] / TERRAIN_SCALE);
			float iDistance = 15;

			if (!g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.LongDistance)
			{
				iDistance = GetDistanceXY(g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.StartX, g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.StartY, dx, dy);
			}
			else
			{
				iDistance = GetDistanceXY(Hero->PositionX, Hero->PositionY, dx, dy);
			}


			if ((int)iDistance > g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Range[1] + 1) continue;
			//gInterface.DrawMessage(1, "add item %d", Items[i].Item.Type);
			gCB_MUHelper->AddItem(i);
		}
	}
}
bool CB_MUHelper::ShouldObtainItem(int iItemId)
{
	ITEM_t* pDrop = &Items[iItemId];
	ITEM* pItem = &pDrop->Item;

	if ((pItem->Type == ITEM_POTION + 15 && g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PickMoney)
		|| ((CheckItemIsJewel(pItem->Type)) && g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PickJewel)
		|| (pItem->ExtOption && g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PickSet)
		|| (pItem->Option1 && g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PickExc))
	{
		return true;
	}

	if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PickExtra && PickUpItemName(pItem))
	{
		return true;
	}

	return g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PickAllItem;
}
int CB_MUHelper::PickUpItemMuHelper()
{
	int iClosestItemId = -1;
	int iMinDistance = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Range[1] + 1;

	std::set<int> setItems;
	{
		_itemsLock.lock();
		setItems = m_setItems;
		_itemsLock.unlock();
	}

	for (const int& iItemId : setItems)
	{
		if (!ShouldObtainItem(iItemId) /*|| !gItemManager.CheckItemIsData(Items[iItemId].Item.Type)*/)
		{
			continue;
		}

		int iItemX = (int)(Items[iItemId].Object.Position[0] / TERRAIN_SCALE);
		int iItemY = (int)(Items[iItemId].Object.Position[1] / TERRAIN_SCALE);
		float iDistance = 15;
		if (!g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.LongDistance)
		{
			iDistance = GetDistanceXY(g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.StartX, g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.StartY, iItemX, iItemY);
		}
		else
		{
			iDistance = GetDistanceXY(Hero->PositionX, Hero->PositionY, iItemX, iItemY);
		}

		if (iDistance < iMinDistance)
		{
			iMinDistance = iDistance;
			iClosestItemId = iItemId;
		}
	}

	return iClosestItemId;
}
bool MuHelperRepairItem()
{
	//==sua do
	if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.RepairItem && GetTickCount() > g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.TimerRepair)
	{
		for (int i = 0; i < 12; i++)
		{
			ITEM* pItem = &CharacterMachine->Equipment[i];
			if (pItem == nullptr || pItem->Type == -1)
			{
				continue;
			}
			ITEM_ATTRIBUTE* p = &ItemAttribute[pItem->Type];
			int maxDurability = calcMaxDurability(pItem, p, pItem->Level);
			//DrawMessage(1, "Item %d Dur (%d/%d)", pItem->Type, pItem->Durability, maxDurability);
			if (pItem->Durability < maxDurability) SendRequestRepair(i, 1);
		}
		g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.TimerRepair = GetTickCount() + (2 * 60 * 1000);
	}
	return 1;
}
int CB_MUHelper::SimulateMove(POINT posMove)
{
	extern int TargetX, TargetY;

	Hero->MovementType = MOVEMENT_MOVE;
	TargetX = (int)posMove.x;
	TargetY = (int)posMove.y;

	if (!CheckTile(Hero, &Hero->Object, 1.5f))
	{
		if (PathFinding2((Hero->PositionX), (Hero->PositionY), TargetX, TargetY, &Hero->Path))
		{
			SendMove(Hero, &Hero->Object);
		}
		return 0;
	}

	return 1;
}
void CB_MUHelper::CheckLiveTarget()
{
	if (gCB_MUHelper->m_SelectedCharacter != -1 && gCB_MUHelper->m_SelectedCharacter <= 400)
	{
		CHARACTER* pTarget = &CharactersClient[gCB_MUHelper->m_SelectedCharacter];
		float fDistanceTarget = GetDistance(pTarget);
		if (pTarget->Dead || !pTarget->Object.Live || !pTarget->Object.Visible || fDistanceTarget > gCB_MUHelper->m_DistanceSelectCharacter || pTarget->Object.AnimationFrame == 6)
		{
			gCB_MUHelper->m_SelectedCharacter = -1;
			gCB_MUHelper->m_DistanceSelectCharacter = 1.f;
			SelectedCharacter = -1;
		}
	}
}


bool MUHelperRegenPos()
{
	gCB_MUHelper->CheckLiveTarget();

	//==Di chuyen ve vi tri cu
	float DistanceTarget = GetDistanceXY(Hero->PositionX, Hero->PositionY, g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.StartX, g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.StartY);

	//gInterface.DrawMessage(1, "Move Regen %f Target %d %f", DistanceTarget, gCB_MUHelper->m_SelectedCharacter, gCB_MUHelper->m_DistanceSelectCharacter);

	if (gCB_MUHelper->m_SelectedCharacter != -1) return 1;

	if (gCB_MUHelper->CacheAction[CB_MUHelper::eMoveRegen] <= 5)
	{
		//if ((g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.MoveTime > 1 && GetTickCount() > g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.TimerMove
		//	&& g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.OriginalPosition == 1) || ((DistanceTarget >= g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Range[0]+2) && !g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.LongDistance))

		if (DistanceTarget > 3 && !g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.LongDistance ||
			(DistanceTarget > g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Range[0] && GetTickCount() > g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.TimerMove && g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.OriginalPosition == 1))
		{
			g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.TimerMove = GetTickCount() + ((g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.MoveTime) * 1000);
			//bool bTargetNear = CheckTile(Hero, &Hero->Object, 1.5f);
			//bool bNoWall = CheckWall(Hero->PositionX, Hero->PositionY, TargetX, TargetY);
			//if (!bTargetNear || !bNoWall)
			//{
			//	if (PathFinding2(Hero->PositionX, Hero->PositionY, g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.StartX, g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.StartY, &Hero->Path))
			//	{
			//		Hero->MovementType = MOVEMENT_MOVE;
			//		Hero->Movement = true;
			//		SendMove(Hero, &Hero->Object);
			//		gCB_MUHelper->CacheAction[CB_MUHelper::eMoveRegen]++;
			//		gCB_MUHelper->m_ActionMoveRegen = GetTickCount() + 1000;
			//		return 0;
			//	}
			//}
			POINT  m_posOriginal;
			m_posOriginal.x = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.StartX;
			m_posOriginal.y = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.StartY;
			if (!gCB_MUHelper->SimulateMove(m_posOriginal))
			{
				return 0;
			}

		}
	}
	else
	{
		g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Started = false;
		return 1;
	}
	if (gCB_MUHelper->CacheAction[CB_MUHelper::eMoveRegen] > 0) gCB_MUHelper->CacheAction[CB_MUHelper::eMoveRegen]--;
	return 1;
}
bool CB_MUHelper::MUHelperNhatItem()
{
	if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem != -1)
	{
		OBJECT* o = &Items[g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem].Object;
		if (!o->Live || !o->Visible)
		{
			g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem = -1;
		}
	}


	if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem == -1)
	{
		this->m_setItems.clear();
		GetListItemPickUp();
	}
	g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem = PickUpItemMuHelper();
	//gInterface.DrawMessage(1, "Pick Item %d %d %d size %d", gCB_MUHelper->m_iCurrentItem, PickUpItemMuHelper(), g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem, this->m_setItems.size());

	if (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem >= 0 && g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem <= 1000 && gCB_MUHelper->CacheAction[CB_MUHelper::ePickUp] < 5)
	{
		OBJECT* o = &Items[g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem].Object;
		if (o->Live && o->Visible)
		{
			int dx = (int)(o->Position[0] / TERRAIN_SCALE);
			int dy = (int)(o->Position[1] / TERRAIN_SCALE);
			float ItemiDistance = GetDistanceXY(Hero->PositionX, Hero->PositionY, dx, dy);
			if (ItemiDistance < 2.5f)
			{
				SendRequestGetItem(g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem);
				DeleteItem(g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem);
				gCB_MUHelper->CacheAction[CB_MUHelper::ePickUp]++;
				return 0;
			}
			else
			{
				CHARACTER* c = Hero;
				OBJECT* o = &c->Object;
				c->MovementType = MOVEMENT_GET;
				ItemKey = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem;
				TargetX = (int)(Items[g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem].Object.Position[0] / TERRAIN_SCALE);
				TargetY = (int)(Items[g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem].Object.Position[1] / TERRAIN_SCALE);
				OBJECT * obiItem = &Items[g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem].Object;

				bool bFinding = PathFinding2((Hero->PositionX), (Hero->PositionY), TargetX, TargetY, &Hero->Path);
				bool bTargetNear = CheckTile(Hero, &Hero->Object, 1.5f);
				bool bNoWall = CheckWall(Hero->PositionX, Hero->PositionY, TargetX, TargetY);
				//PATH_t tempPath;

				//bool bHasPath = PathFinding2(Hero->PositionX, Hero->PositionY, TargetX, TargetY, &tempPath, fSkillDistance + 5);
				//bool bTargetNear = CheckTile(Hero, &Hero->Object, fSkillDistance);
				//bool bNoWall = CheckWall(Hero->PositionX, Hero->PositionY, TargetX, TargetY);
				//gInterface.DrawMessage(1, "Pick Item %d %d %d size %d", gCB_MUHelper->m_iCurrentItem, PickUpItemMuHelper(), g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem, this->m_setItems.size());
				//gInterface.DrawMessage(1, "bTargetNear %d bNoWall %d bFinding %d", bTargetNear, bNoWall, bFinding);
				if (!bTargetNear || !bNoWall)
				{
					if (bFinding)
					{
						SendMove(Hero, &Hero->Object);
						m_iCurrentItemMove = ItemKey;

					}
					//else
					//{
					//	SendRequestGetItem(g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem);
					//	DeleteItem(g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem);
					//}
					gCB_MUHelper->CacheAction[CB_MUHelper::ePickUp]++;
					return 0;
				}
				//else
				//{
				//	SendRequestGetItem(g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem);
				//	DeleteItem(g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem);
				//	gCB_MUHelper->CacheAction[CB_MUHelper::ePickUp]++;
				//	return 0;
				//}
			}
		}
	}
	if (gCB_MUHelper->CacheAction[CB_MUHelper::ePickUp] > 0) gCB_MUHelper->CacheAction[CB_MUHelper::ePickUp]--;
	m_iCurrentItemMove = -1;
	return 1;
}
void CB_MUHelper::CleanupTargets()
{
	std::set<int> setTargets;
	{
		_targetsLock.lock();
		setTargets = m_setTargets;
		_targetsLock.unlock();
	}

	for (const int& iMonsterId : setTargets)
	{
		int iIndex = FindCharacterIndex(iMonsterId);
		if (iIndex == MAX_CHARACTERS_CLIENT)
		{
			DeleteTarget(iMonsterId);
		}

		CHARACTER* pTarget = &CharactersClient[iIndex];
		if (!pTarget || (pTarget && (pTarget->Dead || !pTarget->Object.Live)))
		{
			DeleteTarget(iMonsterId);
		}
	}
}
void CB_MUHelper::CleanupItems()
{
	std::set<int> setItems;
	{
		_itemsLock.lock();
		setItems = m_setItems;
		_itemsLock.unlock();
	}

	for (const int& iItemId : setItems)
	{
		ITEM_t* pDrop = &Items[iItemId];
		ITEM* pItem = &pDrop->Item;

		if (!pDrop->Object.Live || !pDrop->Object.Visible || !ShouldObtainItem(iItemId))
		{
			DeleteTarget(iItemId);
		}
	}
}
bool CB_MUHelper::MUHelperAttack()
{
	//if (m_setTargets.empty())
	//{
	//	GetTarget();
	//}
	//gInterface.DrawMessage(1, "MUHelperAttack %d Size %d", SelectedCharacter, m_setTargets.size());
	if (SelectedCharacter == -1 || SelectedCharacter == FindCharacterIndex(Hero->Key))
	{
		SelectedCharacter = GetMonter();
	}

	if (SelectedCharacter != -1)
	{
		int Skill = CharacterAttribute->Skill[Hero->CurrentSkill];
		float DistanceSkill = gSkillManager.GetSkillDistance(Skill, Hero) - 0.3f;
		float DistanceTarget = GetDistanceXY(Hero->PositionX, Hero->PositionY, CharactersClient[SelectedCharacter].PositionX, CharactersClient[SelectedCharacter].PositionY);

		//if (DistanceSkill == 0) DistanceSkill = 1.2f;
		CHARACTER* c = &CharactersClient[SelectedCharacter];
		OBJECT* o = &c->Object;
		//float Distance = GetDistance(c);
		//gInterface.DrawMessage(1, "Get %d Dis %f [%f] <> %f (%d)", CharactersClient[SelectedCharacter].Key, DistanceTarget, Distance, DistanceSkill, (CheckWall((Hero->PositionX), (Hero->PositionY), c->PositionX, c->PositionY)));

		//if (!g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.LongDistance && GetDistanceXY(Hero->PositionX, Hero->PositionY, CharactersClient[SelectedCharacter].PositionX, CharactersClient[SelectedCharacter].PositionY) > 3)
		if (CheckWall((Hero->PositionX), (Hero->PositionY), c->PositionX, c->PositionY) == 0 || DistanceTarget > DistanceSkill)
		{
			if (PathFinding2(Hero->PositionX, Hero->PositionY, CharactersClient[SelectedCharacter].PositionX, CharactersClient[SelectedCharacter].PositionY, &Hero->Path))
			{
				Hero->MovementType = MOVEMENT_SKILL;

				Hero->Movement = true;
				SendMove(Hero, &Hero->Object);
				return 0;
			}
		}

		//if (o->CurrentAction == 6 || !o->Live || !o->Visible || c->Dead /*|| Distance > (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Range[0] + 1)*/
		//	|| !CheckWall((Hero->PositionX), (Hero->PositionY), c->PositionX, c->PositionY)
		//	|| CheckTarget(Hero)
		//	)

		//if (!o->Live || !o->Visible)
		//{
		//	SelectedCharacter = GetMonter();
		//	//gInterface.DrawMessage(1, "Get lai Monmter %d (%f)", SelectedCharacter, DistanceSkill);
		//	return 0;
		//}

		//==Attack
		//gInterface.DrawMessage(1, "Attack Monmter %d Item %d (%d) DisSkill %f", SelectedCharacter, ItemKey, g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.SelectedItem, DistanceSkill);
		if (!MuHelperActiveAttack()) return 0;

	}
	return 1;
}

void CB_MUHelper::Work()
{
	try
	{
		if (!g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Started)
		{
			if (ClearDateOn) CB_MUHelper::ClearData();
			return;
		}

		//gInterface.DrawMessage(1, "CB_MUHelper::Start() [%d/%d] %d",Hero->PositionX,Hero->PositionY, Hero->MovementType);

		if (LogOut)
		{
			return;
		}
		if (Hero->Movement || Hero->Dead || g_isCharacterBuff((&Hero->Object), eDeBuff_Stun) /*||| Hero->Appear | g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Stage > 1*/)
		{
			return;
		}

		if (gInterface.Data[eStatus_KepChuot].OnShow)
		{
			g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Started = false;
			return;
		}
		//MouseOnWindow = true;

		//gInterface.DrawMessage(1, "End : %d ,%d ,%d ,%d, %d, %d SizeAttack(%d) [%d] %f",
		//	CacheAction[0],
		//	CacheAction[1],
		//	CacheAction[2],
		//	CacheAction[3],
		//	CacheAction[4],
		//	CacheAction[5],
		//	m_setTargets.size(),
		//	m_SelectedCharacter,
		//	m_DistanceSelectCharacter
		//);

		if (!MUHelperRegenPos())
		{
			//gInterface.DrawMessage(1, "MUHelperRegenPos()");
			CB_MUHelper::ClearData();
			return;
		}

		if (gCB_MUHelper->m_ActionMoveRegen > GetTickCount())
		{
			//gInterface.DrawMessage(1, "MUHelperRegenPos()");
			return;
		}

		//==BUff & Buff Nhom
		if (!MuHelperActiveBuff())
		{
			//gInterface.DrawMessage(1, "MuHelperActiveBuff()");
			return;
		}
		//==Hoi mau , hut mau
		if (!MuHelperActiveHeal())
		{
			//gInterface.DrawMessage(1, "MuHelperActiveHeal()");
			return;
		}

		if (!MUHelperNhatItem())
		{
			//gInterface.DrawMessage(1, "MUHelperNhatItem()");
			return;
		}

		if (m_iCurrentItemMove == -1)
		{
			//gInterface.DrawMessage(1, "Attack %d / %d ", m_iCurrentTarget, m_setTargets.size());
			MUHelperAttack();
			//gInterface.DrawMessage(1, "MUHelperAttack() %d", m_setTargets.size());
		}


		for (int i = 0; i < eMaxVaule; i++)
		{
			if (CacheAction[i] > 0) CacheAction[i]--;
		}
		//==sua do
		MuHelperRepairItem();
		ClearDateOn = true;
	}
	catch (...)
	{
		g_ConsoleDebug->Write(MCD_NORMAL, "[MU Helper] Exception occurred. Ignoring...");
	}
}
void CB_MUHelper::SetCircle(float Range)
{
	TimeDrawCircle = GetTickCount() + 1200;
	RangeDraw = Range;
}
void CB_MUHelper::RenderCircle(CHARACTER * c)
{

}