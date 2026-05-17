#pragma once

struct tagITEM;

// Server GCItemValueSend (C2 F3 E9) overrides for NPC shop tooltips / buy cost display.
void ShopItemValueCache_Clear();
void ShopItemValueCache_ApplyPacket(const BYTE* receiveBuffer, int size);
bool ShopItemValueCache_TryGetBuy(const tagITEM* ip, int* outBuy);
bool ShopItemValueCache_TryGetSell(const tagITEM* ip, int* outSell);
