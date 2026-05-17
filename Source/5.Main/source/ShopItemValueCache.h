#pragma once

struct tagITEM;

// Server GCItemValueSend (C2 F3 E9) overrides for NPC shop tooltips / buy cost display.
void ShopItemValueCache_Clear();
void ShopItemValueCache_ApplyPacket(const BYTE* receiveBuffer, int size);
bool ShopItemValueCache_TryGetBuy(const tagITEM* ip, int* outBuy);
bool ShopItemValueCache_TryGetBuyExact(const tagITEM* ip, int* outBuy);
bool ShopItemValueCache_TryGetSell(const tagITEM* ip, int* outSell);
// priceType: 0=zen, 1=WCoinC, 2=WCoinP, 3=GoblinPoint (parity ITEM_VALUE_DATA.type)
bool ShopItemValueCache_TryGetPrice(const tagITEM* ip, int* outPrice, int* outPriceType);
bool ShopItemValueCache_TryGetPriceFromWire(const BYTE* itemWire, int* outPrice, int* outPriceType);
bool ShopItemValueCache_TryGetBuyFromWire(const BYTE* itemWire, int* outBuy);
bool ShopItemValueCache_IsCoinPrice(const tagITEM* ip);
