#pragma once
// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
#include "zzzinfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"


struct CUSTOMCLOAK_DATA
{
	int		Index;
	int		ItemIndex;
	int		Mode;
	int		IndexCloak;
	char	CloakName[100];
	int		IndexStrip;
	char	StripName[100];
};
// -------------------------------------------------------------------------------
struct CEFFECT_DATA
{
	int		Index;
	int		ItemIndex;
	int		EffectIndex;
};
class cCustomCloak
{
public:
	cCustomCloak();
	virtual ~cCustomCloak();
	// ----
	void	Init();
	void	Load(CUSTOMCLOAK_DATA* info);
	void	SetInfo(CUSTOMCLOAK_DATA info);
	CUSTOMCLOAK_DATA* GetInfo(int index);
	CUSTOMCLOAK_DATA* GetInfoByItem(int ItemIndex);
	void	LoadTexture();
	bool	isCloak(int ItemIndex);
	bool	isCloakStrip(int ItemIndex);
	int		code;
	DWORD		getCodeCape(int ItemIndex); // OK

	void	RenderCEffect(int ItemIndex, OBJECT* o, BMD* b);
	bool isCEffect(int ItemIndex);
	void	LoadCEffect(CEFFECT_DATA* info);
	void	SetInfoCEffect(CEFFECT_DATA info);
	CEFFECT_DATA* GetInfoCEffect(int index);
	CEFFECT_DATA* GetInfoCEffectByItem(int ItemIndex);
	// ----
	// ----
public:
	CUSTOMCLOAK_DATA m_Cloak[MAX_CUSTOMCLOAK];
	CEFFECT_DATA m_CEffect[MAX_CUSTOMCEFFECT];


}; extern cCustomCloak gCloak;
extern void RenderBrightEffect(BMD* b, int Bitmap, int Link, float Scale, vec3_t Light, OBJECT* o);
// -------------------------------------------------------------------------------