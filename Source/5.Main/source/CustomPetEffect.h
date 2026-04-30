#pragma once

struct PET_EFFECT_CUSTOM
{
	int Index;
	int ItemIndex;
	int Join;
	int TypeEffect;
	int EffectCode;
	float EffectSize;
	int LargeRand;
	float ColorR;
	float ColorG;
	float ColorB;
};
struct RENDER_MESH
{
	int Index;
	int ItemIndex;
	int EffectIndex;
	int EffectType;
	float ColorR;
	float ColorG;
	float ColorB;
};

class cCEffectPet
{
public:
	cCEffectPet();
	virtual ~cCEffectPet();
	void Init();
	void Load(PET_EFFECT_CUSTOM* info);
	void LoadGlow(RENDER_MESH* info);
public:
	std::vector<PET_EFFECT_CUSTOM> m_PetCEffect;
	std::vector<RENDER_MESH> m_RenderMeshPet;

}; extern cCEffectPet gCustomCEffectPet;