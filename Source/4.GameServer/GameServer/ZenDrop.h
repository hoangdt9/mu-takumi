#pragma once

struct ZEN_DROP
{
	BYTE MapNumber;
	DWORD MinMoney;
	DWORD MaxMoney;
};

class cZenDrop
{
public:
	void Load(char* path);
	bool IsZenDropActive(BYTE Map);
	DWORD GetZenAmount(BYTE MapNumber, WORD MonsterLevel);
	bool m_ZenDropEnable;
	std::map<int, ZEN_DROP> m_ZenDrop;
}; extern cZenDrop gZenDrop;