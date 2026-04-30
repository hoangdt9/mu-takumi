/*******************************************************************************
*	작 성 자 : 진혜진
*	작 성 일 : 2009.06.10
*	내    용 : Include Header
*******************************************************************************/

#pragma once

#pragma warning(disable : 4995)

#include <iostream>
#if defined(__ANDROID__) || defined(MU_IOS)
#include "Platform/PlatformDefs.h"
#else
#include <Windows.h>
#include <Wininet.h>
#endif

#include <crtdbg.h>
#include <tchar.h>
#include <strsafe.h>

#include "GameShop/ShopListManager/interface/WZResult/WZResult.h"
#include "GameShop/ShopListManager/interface/DownloadInfo.h"
#include "GameShop/ShopListManager/interface/IDownloaderStateEvent.h"

#pragma comment(lib, "Wininet.lib")
