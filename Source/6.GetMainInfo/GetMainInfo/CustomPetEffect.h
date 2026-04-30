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
	void Load(char* path);
	void SetInfo(PET_EFFECT_CUSTOM info);

	void LoadGlow(char* path);
	void SetInfoGlow(RENDER_MESH info);
public:
	PET_EFFECT_CUSTOM m_PetCEffect[MAX_CUSTOPETEFFECT];
	RENDER_MESH m_RenderMeshPet[MAX_RENDER_MESH];

}; extern cCEffectPet gCustomCEffectPet;