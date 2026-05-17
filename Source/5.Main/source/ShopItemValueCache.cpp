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
		int priceType;
	};

	std::unordered_map<Key, Value, KeyHash> g_cache;

	int DecodeWireItemIndex(const BYTE* wire)
	{
		// Parity ItemWire602.DecodeItemIndex / CNewUIItemMng::ExtractItemType.
		return wire[0] + ((wire[3] & 128) << 1) + ((wire[5] & 240) << 5);
	}

	Key KeyFromWire(const BYTE* wire)
	{
		return {
			DecodeWireItemIndex(wire),
			(wire[1] >> 3) & 15,
			wire[3] & 0x3F,
		};
	}

	bool TryLookup(const Key& key, int* outPrice, int* outPriceType, int* outSell)
	{
		const auto it = g_cache.find(key);
		if (it == g_cache.end())
		{
			return false;
		}

		if (outPrice != nullptr)
		{
			*outPrice = it->second.buy;
		}

		if (outPriceType != nullptr)
		{
			*outPriceType = it->second.priceType;
		}

		if (outSell != nullptr)
		{
			*outSell = it->second.sell;
		}

		return true;
	}

	bool TryLookupWithFallbacks(const Key& key, int* outPrice, int* outPriceType, int* outSell)
	{
		if (TryLookup(key, outPrice, outPriceType, outSell))
		{
			return true;
		}

		// F3 E9 may key exc differently than post-ItemConvert tooltip item; try index+level.
		// If several rows match (should not happen after server sends shop-only E9), pick lowest zen buy.
		int bestBuy = 0;
		int bestSell = 0;
		bool found = false;
		for (const auto& entry : g_cache)
		{
			if (entry.first.index != key.index
				|| entry.first.level != key.level
				|| entry.second.priceType != 0)
			{
				continue;
			}

			if (!found || entry.second.buy < bestBuy)
			{
				bestBuy = entry.second.buy;
				bestSell = entry.second.sell;
				found = true;
			}
		}

		if (!found)
		{
			return false;
		}

		if (outPrice != nullptr)
		{
			*outPrice = bestBuy;
		}

		if (outPriceType != nullptr)
		{
			*outPriceType = 0;
		}

		if (outSell != nullptr)
		{
			*outSell = bestSell;
		}

		return true;
	}
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
		const int priceType = *(const int*)(receiveBuffer + offset + 12);
		const int value = *(const int*)(receiveBuffer + offset + 16);
		const int sellvalue = *(const int*)(receiveBuffer + offset + 24);
		g_cache[{ index, level, newopt }] = { value, sellvalue, priceType };
		offset += 28;
	}
}

static int ExcellentKeyBits(const tagITEM* ip)
{
	// Wire byte 3 is stored in Option1 before ItemConvert; wings may also use ExtOption.
	int bits = ip->Option1 & 0x3F;
	if (bits == 0 && (ip->ExtOption & 0x3F) != 0)
	{
		bits = ip->ExtOption & 0x3F;
	}

	return bits;
}

static Key MakeKey(const tagITEM* ip)
{
	const int index = ip->Type;
	const int level = (ip->Level >> 3) & 15;
	const int newopt = ExcellentKeyBits(ip);
	return { index, level, newopt };
}

bool ShopItemValueCache_TryGetPriceFromWire(const BYTE* itemWire, int* outPrice, int* outPriceType)
{
	if (itemWire == nullptr || outPrice == nullptr || outPriceType == nullptr)
	{
		return false;
	}

	return TryLookupWithFallbacks(KeyFromWire(itemWire), outPrice, outPriceType, nullptr);
}

bool ShopItemValueCache_TryGetPrice(const tagITEM* ip, int* outPrice, int* outPriceType)
{
	if (ip == nullptr || outPrice == nullptr || outPriceType == nullptr)
	{
		return false;
	}

	return TryLookupWithFallbacks(MakeKey(ip), outPrice, outPriceType, nullptr);
}

bool ShopItemValueCache_IsCoinPrice(const tagITEM* ip)
{
	int price = 0;
	int priceType = 0;
	return ShopItemValueCache_TryGetPrice(ip, &price, &priceType) && priceType > 0;
}

bool ShopItemValueCache_TryGetBuy(const tagITEM* ip, int* outBuy)
{
	if (ip == nullptr || outBuy == nullptr)
	{
		return false;
	}

	int priceType = 0;
	if (!ShopItemValueCache_TryGetPrice(ip, outBuy, &priceType))
	{
		return false;
	}

	return priceType == 0;
}

bool ShopItemValueCache_TryGetSell(const tagITEM* ip, int* outSell)
{
	if (ip == nullptr || outSell == nullptr)
	{
		return false;
	}

	return TryLookupWithFallbacks(MakeKey(ip), nullptr, nullptr, outSell);
}

bool ShopItemValueCache_TryGetBuyFromWire(const BYTE* itemWire, int* outBuy)
{
	if (itemWire == nullptr || outBuy == nullptr)
	{
		return false;
	}

	int priceType = 0;
	if (!ShopItemValueCache_TryGetPriceFromWire(itemWire, outBuy, &priceType))
	{
		return false;
	}

	return priceType == 0;
}
