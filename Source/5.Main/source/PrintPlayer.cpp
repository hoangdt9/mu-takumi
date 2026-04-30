#include "stdafx.h"
#include "PrintPlayer.h"

void GetViewMagicDamage(DWORD* DamageMin, DWORD* DamageMax) // OK
{
	if (!CharacterAttribute) return;

	(*DamageMin) = CharacterAttribute->PrintPlayer.ViewMagicDamageMin;

	(*DamageMax) = CharacterAttribute->PrintPlayer.ViewMagicDamageMax;

	(*DamageMin) += CharacterAttribute->PrintPlayer.ViewSkillMagicDamage;

	(*DamageMax) += CharacterAttribute->PrintPlayer.ViewSkillMagicDamage + (CharacterAttribute->PrintPlayer.ViewSkillMagicDamage / 2);

	(*DamageMin) += ((*DamageMin) * CharacterAttribute->PrintPlayer.ViewMulMagicDamage) / 100;

	(*DamageMax) += ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewMulMagicDamage) / 100;

	(*DamageMin) -= ((*DamageMin) * CharacterAttribute->PrintPlayer.ViewDivMagicDamage) / 100;

	(*DamageMax) -= ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewDivMagicDamage) / 100;
}

void GetViewMagicDamage(DWORD* DamageMin, DWORD* DamageMax, DWORD* DamageRate) // OK
{
	if (!CharacterAttribute) return;

	(*DamageMin) = CharacterAttribute->PrintPlayer.ViewMagicDamageMin;

	(*DamageMax) = CharacterAttribute->PrintPlayer.ViewMagicDamageMax;

	(*DamageMin) += CharacterAttribute->PrintPlayer.ViewSkillMagicDamage;

	(*DamageMax) += CharacterAttribute->PrintPlayer.ViewSkillMagicDamage + (CharacterAttribute->PrintPlayer.ViewSkillMagicDamage / 2);

	(*DamageMin) += ((*DamageMin) * CharacterAttribute->PrintPlayer.ViewMulMagicDamage) / 100;

	(*DamageMax) += ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewMulMagicDamage) / 100;

	(*DamageMin) -= ((*DamageMin) * CharacterAttribute->PrintPlayer.ViewDivMagicDamage) / 100;

	(*DamageMax) -= ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewDivMagicDamage) / 100;

	(*DamageRate) = ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewMagicDamageRate) / 100;
}
void GetViewDarkSpiritDamage(DWORD* DamageMin, DWORD* DamageMax) // OK
{
	if (!CharacterAttribute) return;

	(*DamageMin) = CharacterAttribute->PrintPlayer.ViewDarkSpiritAttackDamageMin;

	(*DamageMax) = CharacterAttribute->PrintPlayer.ViewDarkSpiritAttackDamageMax;

}

void GetViewPhisyDamage(DWORD* DamageMin, DWORD* DamageMax) // OK
{
	if (!CharacterAttribute) return;

	(*DamageMin) = CharacterAttribute->PrintPlayer.ViewPhysiDamageMin;

	(*DamageMax) = CharacterAttribute->PrintPlayer.ViewPhysiDamageMax;

	(*DamageMin) += ((*DamageMin) * CharacterAttribute->PrintPlayer.ViewMulPhysiDamage) / 100;

	(*DamageMax) += ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewMulPhysiDamage) / 100;

	(*DamageMin) -= ((*DamageMin) * CharacterAttribute->PrintPlayer.ViewDivPhysiDamage) / 100;

	(*DamageMax) -= ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewDivPhysiDamage) / 100;
}

void GetViewDarkHorseDamage(DWORD* DamageMin, DWORD* DamageMax) // OK
{
	if (!CharacterAttribute) return;

	(*DamageMin) = CharacterAttribute->PrintPlayer.ViewPhysiDamageMin;

	(*DamageMax) = CharacterAttribute->PrintPlayer.ViewPhysiDamageMax;

	(*DamageMin) += CharacterAttribute->PrintPlayer.ViewSkillPhysiDamage;

	(*DamageMax) += CharacterAttribute->PrintPlayer.ViewSkillPhysiDamage + (CharacterAttribute->PrintPlayer.ViewSkillPhysiDamage / 2);

	(*DamageMin) += ((*DamageMin) * CharacterAttribute->PrintPlayer.ViewMulPhysiDamage) / 100;

	(*DamageMax) += ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewMulPhysiDamage) / 100;

	(*DamageMin) -= ((*DamageMin) * CharacterAttribute->PrintPlayer.ViewDivPhysiDamage) / 100;

	(*DamageMax) -= ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewDivPhysiDamage) / 100;
}


void GetViewSkillDamage(DWORD* DamageMin, DWORD* DamageMax) // OK
{
	if (!CharacterAttribute) return;

	(*DamageMin) = CharacterAttribute->PrintPlayer.ViewPhysiDamageMin;

	(*DamageMax) = CharacterAttribute->PrintPlayer.ViewPhysiDamageMax;

	(*DamageMin) += CharacterAttribute->PrintPlayer.ViewSkillPhysiDamage;

	(*DamageMax) += CharacterAttribute->PrintPlayer.ViewSkillPhysiDamage + (CharacterAttribute->PrintPlayer.ViewSkillPhysiDamage / 2);

	(*DamageMin) += ((*DamageMin) * CharacterAttribute->PrintPlayer.ViewMulPhysiDamage) / 100;

	(*DamageMax) += ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewMulPhysiDamage) / 100;

	(*DamageMin) -= ((*DamageMin) * CharacterAttribute->PrintPlayer.ViewDivPhysiDamage) / 100;

	(*DamageMax) -= ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewDivPhysiDamage) / 100;
}

void GetViewCurseDamage(DWORD* DamageMin, DWORD* DamageMax) // OK
{
	if (!CharacterAttribute) return;

	(*DamageMin) = CharacterAttribute->PrintPlayer.ViewCurseDamageMin;

	(*DamageMax) = CharacterAttribute->PrintPlayer.ViewCurseDamageMax;

	(*DamageMin) += CharacterAttribute->PrintPlayer.ViewSkillMagicDamage;

	(*DamageMax) += CharacterAttribute->PrintPlayer.ViewSkillMagicDamage + (CharacterAttribute->PrintPlayer.ViewSkillMagicDamage / 2);

	(*DamageMin) += ((*DamageMin) * CharacterAttribute->PrintPlayer.ViewMulCurseDamage) / 100;

	(*DamageMax) += ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewMulCurseDamage) / 100;

	(*DamageMin) -= ((*DamageMin) * CharacterAttribute->PrintPlayer.ViewDivCurseDamage) / 100;

	(*DamageMax) -= ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewDivCurseDamage) / 100;
}

void GetViewCurseDamage(DWORD* DamageMin, DWORD* DamageMax, DWORD* DamageRate) // OK
{
	if (!CharacterAttribute) return;

	(*DamageMin) = CharacterAttribute->PrintPlayer.ViewCurseDamageMin;

	(*DamageMax) = CharacterAttribute->PrintPlayer.ViewCurseDamageMax;

	(*DamageMin) += CharacterAttribute->PrintPlayer.ViewSkillMagicDamage;

	(*DamageMax) += CharacterAttribute->PrintPlayer.ViewSkillMagicDamage + (CharacterAttribute->PrintPlayer.ViewSkillMagicDamage / 2);

	(*DamageMin) += ((*DamageMin) * CharacterAttribute->PrintPlayer.ViewMulCurseDamage) / 100;

	(*DamageMax) += ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewMulCurseDamage) / 100;

	(*DamageMin) -= ((*DamageMin) * CharacterAttribute->PrintPlayer.ViewDivCurseDamage) / 100;

	(*DamageMax) -= ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewDivCurseDamage) / 100;

	(*DamageRate) = ((*DamageMax) * CharacterAttribute->PrintPlayer.ViewCurseDamageRate) / 100;
}