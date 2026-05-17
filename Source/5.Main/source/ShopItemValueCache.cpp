#include "stdafx.h"
#include "ShopItemValueCache.h"
#include "_struct.h"

#include <unordered_map>
#include <cstdint>

namespace
{
	struct Key
	{
		int index;
		int level;
		int newopt;

		bool operator==(const Key& o) const
		{
			return index == o.index && level == o.level && newopt == o.newopt;
		}
	};

	struct KeyHash
	{
		size_t operator()(const Key& k) const
		{
			return (size_t)k.index ^ ((size_t)k.level << 10) ^ ((size_t)k.newopt << 16);
		}
	};

	struct Value
	{
		int buy;
		int sell;
	};

	std::unordered_map<Key, Value, KeyHash> g_cache;
}

void ShopItemValueCache_Clear()
{
	g_cache.clear();
}

void ShopItemValueCache_ApplyPacket(const BYTE* receiveBuffer, int size)
{
	if (receiveBuffer == nullptr || size < 9 || receiveBuffer[0] != 0xC2)
	{
		return;
	}

	const int count = *(const int*)(receiveBuffer + 5);
	const int payload = size - 9;
	if (count <= 0 || payload < count * 28)
	{
		return;
	}

	g_cache.clear();
	int offset = 9;
	for (int i = 0; i < count; ++i)
	{
		const int index = *(const int*)(receiveBuffer + offset);
		const int level = *(const int*)(receiveBuffer + offset + 4);
		const int newopt = *(const int*)(receiveBuffer + offset + 8);
		const int value = *(const int*)(receiveBuffer + offset + 16);
		const int sellvalue = *(const int*)(receiveBuffer + offset + 24);
		g_cache[{ index, level, newopt }] = { value, sellvalue };
		offset += 28;
	}
}

static Key MakeKey(const ITEM* ip)
{
	const int index = ip->Type;
	const int level = (ip->Level >> 3) & 15;
	const int newopt = ip->ExtOption & 0x3F;
	return { index, level, newopt };
}

bool ShopItemValueCache_TryGetBuy(const ITEM* ip, int* outBuy)
{
	if (ip == nullptr || outBuy == nullptr)
	{
		return false;
	}

	const auto it = g_cache.find(MakeKey(ip));
	if (it == g_cache.end())
	{
		return false;
	}

	*outBuy = it->second.buy;
	return true;
}

bool ShopItemValueCache_TryGetSell(const ITEM* ip, int* outSell)
{
	if (ip == nullptr || outSell == nullptr)
	{
		return false;
	}

	const auto it = g_cache.find(MakeKey(ip));
	if (it == g_cache.end())
	{
		return false;
	}

	*outSell = it->second.sell;
	return true;
}
