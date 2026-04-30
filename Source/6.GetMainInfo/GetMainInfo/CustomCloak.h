#pragma once
// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------

#define ITEM(x, y)					((x * 512) + y)
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
	void Init();
	void Load(char* path);
	void LoadCEffect(char* path);
	void SetInfo(CUSTOMCLOAK_DATA info);
	void SetInfoCEffect(CEFFECT_DATA info);
public:
	CUSTOMCLOAK_DATA m_Cloak[MAX_CUSTOMCLOAK];
	CEFFECT_DATA m_CEffect[MAX_CUSTOMCEFFECT];
}; extern cCustomCloak gCloak;