#pragma once

struct tagITEM;

void ShopPriceTraceLog(const char* fmt, ...);
void ShopPriceTraceLogSellTooltip(const tagITEM* ip, int source, int rawZen, int afterDurZen);
void ShopPriceTraceLogBuyTooltip(const tagITEM* ip, int source, int buyZen);
void ShopPriceTraceLogZenSync(const char* eventTag, int backupGold, int newGold, int pendingSpend);
void ShopPriceTraceLogF3E9(int rowCount);

// source: 1=cache-sell 2=cache-buy-fallback 3=ItemValue-sell 4=ItemValue-buy 5=cache-buy-exact
