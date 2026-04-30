#include "StdAfx.h"
#include "CustomPetEffect.h"


cCEffectPet gCustomCEffectPet;

cCEffectPet::cCEffectPet()
{
	this->m_PetCEffect.clear();
	this->m_RenderMeshPet.clear();
}
cCEffectPet::~cCEffectPet()
{
}

void cCEffectPet::Load(PET_EFFECT_CUSTOM* info)
{
	for (int n = 0; n < MAX_CUSTOPETEFFECT; n++)
	{
		if (info[n].Index < 0 || info[n].Index >= MAX_CUSTOPETEFFECT)
		{
			return;
		}
		this->m_PetCEffect.push_back(info[n]);
	}
}
void cCEffectPet::LoadGlow(RENDER_MESH* info)
{
	for (int n = 0; n < MAX_RENDER_MESH; n++)
	{
		if (info[n].Index < 0 || info[n].Index >= MAX_RENDER_MESH)
		{
			return;
		}
		this->m_RenderMeshPet.push_back(info[n]);
	}
}
