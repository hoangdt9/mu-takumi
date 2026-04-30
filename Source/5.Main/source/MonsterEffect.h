#pragma once


class jCEffectMonster
{
public:
	struct xMonsterEffectBBBBB
	{
		int IndexMonster;
		int TypeEffect;
		int IDEffect;
		int EfectValue;
		float Join[3];
		float Color[3];
		float Scale;
	};
	jCEffectMonster();
	virtual ~jCEffectMonster();
	void Load(xMonsterEffectBBBBB* info);
public:
	std::vector<xMonsterEffectBBBBB> m_MonsterEffect;
};

extern jCEffectMonster gggJCEffectMonster;