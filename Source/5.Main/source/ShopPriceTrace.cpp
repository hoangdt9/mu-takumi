#include "stdafx.h"
#include "ShopPriceTrace.h"
#include "ShopItemValueCache.h"
#include "_struct.h"
#include "ZzzInfomation.h"
#include "Utilities/Log/ErrorReport.h"
#include "Utilities/Log/TakumiAndroidDiag.h"

#include <cstdarg>
#include <cstdio>

#if defined(__ANDROID__)
#include <android/log.h>
#endif

void ShopPriceTraceLog(const char* fmt, ...)
{
#if TAKUMI_ANDROID_DEBUG_WEAR_INVENTORY
	char buf[768];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	g_ErrorReport.Write("[ShopPrice] %s\r\n", buf);
#if defined(__ANDROID__)
	__android_log_print(ANDROID_LOG_INFO, "TakumiErrorReport", "[ShopPrice] %s", buf);
#endif
#else
	(void)fmt;
#endif
}

static int ExcellentKeyBitsForLog(const tagITEM* ip)
{
	int bits = ip->Option1 & 0x3F;
	if (bits == 0 && (ip->ExtOption & 0x3F) != 0)
	{
		bits = ip->ExtOption & 0x3F;
	}

	return bits;
}

void ShopPriceTraceLogSellTooltip(const tagITEM* ip, int source, int rawZen, int afterDurZen)
{
	if (ip == nullptr)
	{
		return;
	}

	const int level = (ip->Level >> 3) & 15;
	const int exc = ExcellentKeyBitsForLog(ip);
	ShopPriceTraceLog(
		"tooltip-sell type=%d lvl=%d exc=%d dur=%d opt1=0x%02X ext=0x%02X src=%d raw=%d afterDur=%d itemValue1=%d",
		ip->Type,
		level,
		exc,
		ip->Durability,
		ip->Option1,
		ip->ExtOption,
		source,
		rawZen,
		afterDurZen,
		ItemValue(const_cast<tagITEM*>(ip), 1));
}

void ShopPriceTraceLogBuyTooltip(const tagITEM* ip, int source, int buyZen)
{
	if (ip == nullptr)
	{
		return;
	}

	const int level = (ip->Level >> 3) & 15;
	const int exc = ExcellentKeyBitsForLog(ip);
	ShopPriceTraceLog(
		"tooltip-buy type=%d lvl=%d exc=%d src=%d buy=%d itemValue0=%d",
		ip->Type,
		level,
		exc,
		source,
		buyZen,
		ItemValue(const_cast<tagITEM*>(ip), 0));
}

void ShopPriceTraceLogZenSync(const char* eventTag, int backupGold, int newGold, int pendingSpend)
{
	const int delta = newGold - backupGold;
	ShopPriceTraceLog(
		"zen-sync event=%s backup=%d new=%d delta=%d pendingSpend=%d",
		eventTag != nullptr ? eventTag : "?",
		backupGold,
		newGold,
		delta,
		pendingSpend);
}

void ShopPriceTraceLogF3E9(int rowCount)
{
	ShopPriceTraceLog("F3-E9-applied rows=%d", rowCount);
}
