// NewUIHeroPositionInfo.cpp: implementation of the CNewUIHeroPositionInfo
// class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIHeroPositionInfo.h"
#include "CBInterface.h"
#include "MapManager.h"
#include "NewUISystem.h"
#include "Protocol.h"
#if defined(__ANDROID__) || defined(MU_IOS)
#include "Platform/MobileHud.h"
#endif
#include "ZzzAI.h"
#include "wsclientinline.h"
#if defined(__ANDROID__)
#include "Utilities/Log/TakumiAndroidUiPerf.h"
#endif

#include <algorithm>
#include <cmath>

using namespace SEASON3B;
char *ButtonText[6] = {"Menu",      "Show Name",   "Show HP Bar",
                       "Camera 3D", "Config Auto", "Active Auto"};
double ScaleMap = 0.125;
DWORD CacheTimeClick = 0;
bool BlockClickCustom = false;
bool ShowMiniMapColtrol = true;
extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayHeight;
extern int DisplayWinMid;
extern int DisplayHeightExt;
extern int DisplayWinExt;
extern int DisplayWinReal;
extern HWND g_hWnd;
extern CHARACTER_ATTRIBUTE *CharacterAttribute;
extern CHARACTER_MACHINE *CharacterMachine;
extern Interface gInterface;

#if defined(__ANDROID__) && TAKUMI_ANDROID_UI_DEBUG_TRACE
#define ANDROID_HERO_STAGE(msg) do { OutputDebugStringA("UI_HERO: " msg); g_ErrorReport.Write("[UI_HERO] " msg "\r\n"); } while(0)
#else
#define ANDROID_HERO_STAGE(msg) ((void)0)
#endif

namespace {
inline CNewUIMuHelper *GetSafeMuHelper() {
  if (g_pNewUISystem == NULL) {
    return NULL;
  }

  return g_pNewUISystem->Get_pNewUIMuHelper();
}

inline bool IsMuHelperStarted() {
  CNewUIMuHelper *muHelper = GetSafeMuHelper();
  return (muHelper != NULL && muHelper->DataAutoMu.Started);
}
} // namespace

CNewUIHeroPositionInfo::CNewUIHeroPositionInfo() {
  m_pNewUIMng = NULL;
  m_Pos.x = m_Pos.y = 0;
  m_CurHeroPosition.x = m_CurHeroPosition.y = 0;
  IndexIMGMap = -1;

  this->DataMap.ModelID = -1;
  this->DataMap.XSW_Minimap_Frame = 0; //-- Minimap_FrameX
  this->DataMap.YSW_Minimap_Frame = 0; //-- Minimap_FrameY
  this->DataMap.XSW_Minimap =
      15; //-- MiniMapX (Mover Minimapa Izquierda o Derecha)
  this->DataMap.YSW_Minimap =
      17; //-- MiniMapY (Minimapa Arriba) (Mover Minimapa Arriba o Abajo)
  this->DataMap.XSW_Time_Frame = 0; //-- Time_FrameX
  this->DataMap.YSW_Time_Frame = 0; //-- Time_FrameY
  this->DataMap.This_92 = 0;
  this->DataMap.This_96 = 0;
  this->DataMap.DropMoveUserX = 0; //-- DropMoveUserX
  this->DataMap.DropMoveUserY = 0; //-- DropMoveUserY
  this->DataMap.This_108 = 0;
  this->DataMap.This_112 = 0;
  this->DataMap.CurrentMoveX = 0; //-- CurrentMoveX
  this->DataMap.CurrentMoveY = 0; //-- CurrentMoveY
  this->DataMap.Scale = 1;        //-- Scale
  this->DataMap.TimerEventVK = 0; //-- Scale
  this->DataMap.Alpha = 1.0f;     //-- Alpha
}

CNewUIHeroPositionInfo::~CNewUIHeroPositionInfo() { Release(); }

//---------------------------------------------------------------------------------------------
// Create
bool CNewUIHeroPositionInfo::Create(CNewUIManager *pNewUIMng, int x, int y) {
  if (NULL == pNewUIMng)
    return false;

  ANDROID_HERO_STAGE("create begin");
  m_pNewUIMng = pNewUIMng;
  m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_HERO_POSITION_INFO, this);
  ANDROID_HERO_STAGE("add ui obj ok");

  WidenX = HERO_POSITION_INFO_BASEB_WINDOW_WIDTH;
  if (WindowWidth == 800) {
    WidenX = (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH +
              (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH * 0.4f));
  } else if (WindowWidth == 1024) {
    WidenX = (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH +
              (HERO_POSITION_INFO_BASEB_WINDOW_WIDTH * 0.2f));
  }

  SetPos(x, y);
  ANDROID_HERO_STAGE("set pos ok");
  LoadImages();
  ANDROID_HERO_STAGE("load images ok");

  unicode::t_string tooltiptext1 = GlobalText[3561];
  unicode::t_string btname1 = "";

  SetButtonInfo(&m_BtnConfig, IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 3,
                x + WidenX + 41, y, 18, 13, 1, 0, 1, 1u, btname1, tooltiptext1,
                0);
  ANDROID_HERO_STAGE("btn config ok");

  unicode::t_string tooltiptext2 = GlobalText[3562];
  unicode::t_string btname2 = "";

  SetButtonInfo(&m_BtnStart, IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 4,
                x + WidenX + 59, y, 18, 13, 1, 0, 1, 1u, btname2, tooltiptext2,
                0);
  ANDROID_HERO_STAGE("btn start ok");

  unicode::t_string tooltiptext3 = GlobalText[3563];
  unicode::t_string btname3 = "";

  SetButtonInfo(&m_BtnStop, IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 5,
                x + WidenX + 59, y, 18, 13, 1, 0, 1, 1u, btname3, tooltiptext3,
                0);
  ANDROID_HERO_STAGE("btn stop ok");

  MoveTextTipPos(&m_BtnConfig, -20, 9);
  MoveTextTipPos(&m_BtnStart, -20, 9);
  MoveTextTipPos(&m_BtnStop, -20, 9);
  ANDROID_HERO_STAGE("tooltip pos ok");

  Show(true);
  TimeClickMouseL = 0;
  ANDROID_HERO_STAGE("create done");
  return true;
}

void CNewUIHeroPositionInfo::Release() {
  UnloadImages();
  IndexIMGMap = -1;
  if (m_pNewUIMng) {
    m_pNewUIMng->RemoveUIObj(this);
    m_pNewUIMng = NULL;
  }
}

void CNewUIHeroPositionInfo::SetPos(int x, int y) {
  m_Pos.x = x;
  m_Pos.y = y;
}

bool CNewUIHeroPositionInfo::BtnProcess() {
  if (m_BtnConfig.UpdateMouseEvent()) {
    g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MuHelper);
    PlayBuffer(SOUND_CLICK01);
    return true;
  }
  if (!IsMuHelperStarted()) {
    if (m_BtnStart.UpdateMouseEvent()) {
      // todo: start mu helper when it's implemented :)
      StartMuHelper();
      PlayBuffer(SOUND_CLICK01);
      return true;
    }
  } else {
    if (m_BtnStop.UpdateMouseEvent()) {
      // todo: start mu helper when it's implemented :)
      StartMuHelper(1);
      PlayBuffer(SOUND_CLICK01);
      return true;
    }
  }
  return false;
}

bool CNewUIHeroPositionInfo::UpdateMouseEvent() {
  if (true == BtnProcess() || BlockClickCustom)
    return false;

  int Width = HERO_POSITION_INFO_BASEA_WINDOW_WIDTH + WidenX +
              HERO_POSITION_INFO_BASEC_WINDOW_WIDTH;
  if (CheckMouseIn(m_Pos.x, m_Pos.y, Width + 80,
                   HERO_POSITION_INFO_BASE_WINDOW_HEIGHT))
    return false;

  return true;
}

bool CNewUIHeroPositionInfo::UpdateKeyEvent() { return true; }

bool CNewUIHeroPositionInfo::Update() {
  if ((IsVisible() == true) && (Hero != NULL)) {
    m_CurHeroPosition.x = (Hero->PositionX);
    m_CurHeroPosition.y = (Hero->PositionY);
  }

  return true;
}
bool MiniMapFileCheck1(int Map) // OK
{
  if (Map - 1 == 30) {
    return 0;
  }

  char Path[64];

  wsprintf(Path, "Data\\Custom\\Maps\\World%d.ozt", Map);
  FILE *fp = fopen(Path, "rb");

  if (fp != NULL) {
    fclose(fp);
    return 1;
  } else {
    return 0;
  }
}
int CacheMapNumber = -1;
void CNewUIHeroPositionInfo::MiniMapLoad() // OK
{
  if (gMapManager.WorldActive != CacheMapNumber) {
    if (gMapManager.WorldActive != 30) {
      char buff[32];

      wsprintf(buff, "Custom\\Maps\\World%d.tga",
               (gMapManager.WorldActive + 1));

      if (MiniMapFileCheck1(gMapManager.WorldActive + 1) != 0) {
        IndexIMGMap = IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 6;
        LoadBitmap(buff, IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 6, GL_LINEAR);
      } else {
        IndexIMGMap = -1;
      }
    } else {
      IndexIMGMap = -1;
    }
    CacheMapNumber = gMapManager.WorldActive;
  }
}

void CNewUIHeroPositionInfo::DataViewPortMapLoad(float x, float y, float mapCenterAbsX,
                                                 float mapCenterAbsY, float mapRadiusPx) {
  if (Hero == NULL)
    return;

  float v53;         // ST34_4@16
  float v48;         // ST34_4@16
  float X;           // ST34_4@16
  float Y;           // ST38_4@16
  float lpObj[2];    // [sp+78h] [bp-3Ch]@1
  float lpTarget[2]; // [sp+78h] [bp-3Ch]@1
  EnableAlphaTest(1);
  float BoxPrintW;   // ST20_4@12
  float BoxCurrentX; // ST70_4@12
  float BoxCurrentY; // ST6C_4@12

  lpObj[0] = (double)(Hero->PositionX) + 2.5;
  lpObj[1] = (double)(Hero->PositionY);

  BoxPrintW = (ScaleMap * 2);

  v53 = (double)(lpObj[0] + 2.5) / BoxPrintW;
  v48 = (double)(253.5 - lpObj[1]) / BoxPrintW;
  for (int i = 0; i < 400; ++i) {
    CHARACTER *c = &CharactersClient[i]; // SelectedCharacter
    OBJECT *o = &c->Object;
    if (!c || c == Hero || !o->Live)
      continue;

    lpTarget[0] = (double)(c->PositionX) + 2.5;
    lpTarget[1] = (double)(c->PositionY);

    if (ScaleMap == 0.125) {
      BoxCurrentX = 56.0 + ((lpObj[0] - lpTarget[0]) *
                            ((lpObj[0] < lpTarget[0]) ? 1.1 : 1.7));
      BoxCurrentY = 46.0 - ((lpObj[1] - lpTarget[1]) * 1.4);
    } else if (ScaleMap == 0.25) {
      BoxCurrentX = 56.0 + ((lpObj[0] - lpTarget[0]) * 0.7);
      BoxCurrentY = 46.0 - ((lpObj[1] - lpTarget[1]) * 0.5);
    } else {
      BoxCurrentX = 51.0 + ((lpObj[0] - lpTarget[0]) * 0.4);
      BoxCurrentY = 46.0 - ((lpObj[1] - lpTarget[1]) * 0.4);
    }
    X = x + BoxCurrentX + (lpTarget[0] / BoxPrintW) - v53;
    Y = y + BoxCurrentY + ((253.5 - lpTarget[1]) / BoxPrintW) - v48;

    if (o->Kind == KIND_NPC) {
      glColor3f(206 / 255.f, 209 / 255.f, 4 / 255.f); // NPC
      // RenderColor((float)(XNPC), (float)(YNPC), 3.f, 3.f);
    } else if (o->Kind == KIND_PLAYER) {
      glColor3f(0 / 255.f, 255 / 255.f, 72 / 255.f); // Player
      // RenderColor((float)(XNPC), (float)(YNPC), 3.f, 3.f);
    } else if (o->Kind == KIND_MONSTER) {
      glColor3f(209 / 255.f, 144 / 255.f, 4 / 255.f); // Monter
      // RenderColor((float)(XNPC), (float)(YNPC), 3.f, 3.f);
    } else {
      glColor3f(200 / 255.f, 200 / 255.f, 200 / 255.f);
    }
    if (c->GuildMarkIndex == Hero->GuildMarkIndex &&
        Hero->GuildMarkIndex >= 0) {
      glColor3f(0 / 255.f, 255 / 255.f, 234 / 255.f); // Guild
      // RenderColor((float)(XNPC), (float)(YNPC), 3.f, 3.f);
    }

#if defined(__ANDROID__) || defined(MU_IOS)
    if (MU_MobileIsModernMobileHudEnabled()) {
      // Nhỏ hơn classic 3×3; giữ trong vòng minimap (clip theo bán kính cố định).
      constexpr float kDot = 2.0f;
      const float half = kDot * 0.5f;
      float px = X;
      float py = Y;
      if (mapRadiusPx > 4.0f) {
        float mcx = px + half;
        float mcy = py + half;
        float dx = mcx - mapCenterAbsX;
        float dy = mcy - mapCenterAbsY;
        // (std::max) — extra parens: PlatformDefs.h defines `max` macro and breaks std::max.
        const float maxR = (std::max)(1.0f, mapRadiusPx - 1.5f);
        const float distSq = dx * dx + dy * dy;
        const float maxRSq = maxR * maxR;
        if (distSq > maxRSq) {
          const float dist = std::sqrt(distSq);
          if (dist > 1.0e-4f) {
            const float s = maxR / dist;
            mcx = mapCenterAbsX + dx * s;
            mcy = mapCenterAbsY + dy * s;
            px = mcx - half;
            py = mcy - half;
          }
        }
      }
      RenderColor(px, py, kDot, kDot);
      continue;
    }
#endif

    if (x < 94 && y < 94)
      RenderColor((float)(X), (float)(Y), 3.f, 3.f); // 11
  }
  EndRenderColor();
}
void SetBlockCur(bool Set) {
  if (g_pBCustomMenuInfo != NULL) {
    g_pBCustomMenuInfo->SetBlockCur(Set);
  }
}
void JShowButtonSlash(float X, float Y, float W, float DropCurrentY) {
  if (SEASON3B::CheckMouseIn((int)X, (int)Y, (int)W, (int)W) &&
      !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MOVEMAP)) {
    SetBlockCur(true);
    RenderTipText(X + 30, Y + 3, ButtonText[0]);
    if (SEASON3B::IsPress(VK_LBUTTON) &&
        (GetTickCount() - CacheTimeClick) > 300) {
      CacheTimeClick = GetTickCount();
      PlayBuffer(SOUND_CLICK01);
      gInterface.Data[eMenu_MAIN].OnShow ^= 1;
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.2588,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    } else {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.1341000199,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    }
  } else {
    if (gInterface.Data[eMenu_MAIN].OnShow) {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.1341000199,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    } else {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.008999999613,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    }
  }
}

void JGetEventZoom(float X, float Y, float W) {
  if (SEASON3B::CheckMouseIn((int)X, (int)Y, (int)W, (int)W) &&
      !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MOVEMAP)) {
    SetBlockCur(true);
    if (SEASON3B::IsPress(VK_LBUTTON) && ScaleMap != 0.125 &&
        GetTickCount() >= CacheTimeClick + 300) {
      PlayBuffer(25, 0, 0);
      ScaleMap = (ScaleMap == 0.375) ? 0.25 : 0.125;
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.2588,
                   0.3775999844, 0.1209000051, 0.05900005624, 1, 1, 0.0);
      CacheTimeClick = GetTickCount();
    } else {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W,
                   (ScaleMap == 0.125) ? 0.3834999801 : 0.1341000199,
                   0.3775999844, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    }
  } else {
    RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W,
                 (ScaleMap == 0.125) ? 0.3834999801 : 0.008999999613,
                 0.3775999844, 0.1209000051, 0.05900005624, 1, 1, 0.0);
  }
}

void JGetEventinZoom(float X, float Y, float W) {
  if (SEASON3B::CheckMouseIn((int)X, (int)Y, (int)W, (int)W) &&
      !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MOVEMAP)) {
    SetBlockCur(true);
    if (SEASON3B::IsPress(VK_LBUTTON) && ScaleMap != 0.375 &&
        GetTickCount() >= CacheTimeClick + 300) {
      PlayBuffer(25, 0, 0);

      ScaleMap = (ScaleMap == 0.125) ? 0.25 : 0.375;

      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.2588,
                   0.4396000206, 0.1209000051, 0.05900005624, 1, 1, 0.0);
      CacheTimeClick = GetTickCount();
    } else {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W,
                   (ScaleMap == 0.375) ? 0.3834999801 : 0.1341000199,
                   0.4396000206, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    }
  } else {
    RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W,
                 (ScaleMap == 0.375) ? 0.3834999801 : 0.008999999613,
                 0.4396000206, 0.1209000051, 0.05900005624, 1, 1, 0.0);
  }
}
void JShowButton3DCamera(float X, float Y, float W, float DropCurrentY) {
  if (SEASON3B::CheckMouseIn((int)X, (int)Y, (int)W, (int)W) &&
      !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MOVEMAP)) {
    SetBlockCur(true);
    RenderTipText(X + 30, Y + 3, ButtonText[3]);
    if (SEASON3B::IsPress(VK_LBUTTON) &&
        (GetTickCount() - CacheTimeClick) > 300) {
      CacheTimeClick = GetTickCount();
      PlayBuffer(SOUND_CLICK01);
      Camera3DEnable ^= 1;
      CameraAngle[2] = -45.f;
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.2588,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
      if (Camera3DEnable) {
        CreateNotice("On Camera 3D", 1);
      } else {
        CreateNotice("Off Camera 3D", 1);
      }
    } else {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.1341000199,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    }
  } else {
    if (Camera3DEnable) {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.1341000199,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    } else {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.008999999613,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    }
  }
}
void JShowButtonHPBar(float X, float Y, float W, float DropCurrentY) {
  if (SEASON3B::CheckMouseIn((int)X, (int)Y, (int)W, (int)W) &&
      !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MOVEMAP)) {
    RenderTipText(X + 30, Y + 3, ButtonText[2]);
    SetBlockCur(true);
    if (SEASON3B::IsPress(VK_LBUTTON) &&
        (GetTickCount() - CacheTimeClick) > 300) {
      CacheTimeClick = GetTickCount();
      PlayBuffer(SOUND_CLICK01);
      mShowHPBar ^= 1;
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.2588,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    } else {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.1341000199,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    }
  } else {
    if (mShowHPBar) {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.1341000199,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    } else {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.008999999613,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    }
  }
}
void JShowButtonShowName(float X, float Y, float W, float DropCurrentY) {
  if (SEASON3B::CheckMouseIn((int)X, (int)Y, (int)W, (int)W) &&
      !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MOVEMAP)) {
    RenderTipText(X + 30, Y + 3, ButtonText[1]);
    SetBlockCur(true);
    if (SEASON3B::IsPress(VK_LBUTTON) &&
        (GetTickCount() - CacheTimeClick) > 300) {
      CacheTimeClick = GetTickCount();
      PlayBuffer(SOUND_CLICK01);
      if (gProtect.m_MainInfo.PlayerShowName) {
        mShowName ^= 1;
        g_bGMObservation = mShowName;
      }

      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.2588,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    } else {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.1341000199,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    }
  } else {
    if (mShowName) {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.1341000199,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    } else {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.008999999613,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    }
  }
}

void JShowButtonConfigAuto(float X, float Y, float W, float DropCurrentY) {
  if (SEASON3B::CheckMouseIn((int)X, (int)Y, (int)W, (int)W) &&
      !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MOVEMAP)) {
    RenderTipText(X + 30, Y + 3, ButtonText[4]);
    SetBlockCur(true);
    if (SEASON3B::IsPress(VK_LBUTTON) &&
        (GetTickCount() - CacheTimeClick) > 300) {
      CacheTimeClick = GetTickCount();
      PlayBuffer(SOUND_CLICK01);
      // g_pBMacroInfo->WindowAuto->onShow ^= 1;
      g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MuHelper);
      PlayBuffer(SOUND_CLICK01);
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.2588,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    } else {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.1341000199,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    }
  } else {
    // if (mShowName)
    //{
    //	RenderBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 4, X, Y, W, W,
    // 0.1341000199, DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    // }
    // else
    {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.008999999613,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    }
  }
}
void JShowButtonPlayAuto(float X, float Y, float W, float DropCurrentY) {
  if (SEASON3B::CheckMouseIn((int)X, (int)Y, (int)W, (int)W) &&
      !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MOVEMAP)) {
    RenderTipText(X + 30, Y + 3, ButtonText[5]);
    SetBlockCur(true);
    if (SEASON3B::IsPress(VK_LBUTTON) &&
        (GetTickCount() - CacheTimeClick) > 300) {
      CacheTimeClick = GetTickCount();
      if (!IsMuHelperStarted()) {
        // todo: start mu helper when it's implemented :)
        StartMuHelper();
      } else {
        StartMuHelper(1);
      }
      PlayBuffer(SOUND_CLICK01);

      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.2588,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    } else {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.1341000199,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    }
  } else {
    if (IsMuHelperStarted()) {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.1341000199,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    } else {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.008999999613,
                   DropCurrentY, 0.1209000051, 0.05900005624, 1, 1, 0.0);
    }
  }
}
void JGetButtonClosed(float X, float Y, float W) {
  if (SEASON3B::CheckMouseIn((int)X, (int)Y, (int)W, (int)W) &&
      !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MOVEMAP)) {
    SetBlockCur(true);
    if (SEASON3B::IsPress(VK_LBUTTON) &&
        (GetTickCount() - CacheTimeClick) > 300) {
      CacheTimeClick = GetTickCount();
      PlayBuffer(SOUND_CLICK01);
      PlayBuffer(25, 0, 0);
      ShowMiniMapColtrol ^= 1;
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.7661997676,
                   0.7609999776 + (!ShowMiniMapColtrol ? 0.06260011024 : 0),
                   0.08950000256, 0.04200005904, 1, 1, 0.0);
    } else {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.6410998702,
                   0.7609999776 + (!ShowMiniMapColtrol ? 0.06260011024 : 0),
                   0.08950000256, 0.04200005904, 1, 1, 0.0);
    }
  } else {
    RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.5159999728,
                 0.7609999776 + (!ShowMiniMapColtrol ? 0.06260011024 : 0),
                 0.08950000256, 0.04200005904, 1, 1, 0.0);
  }
}

void JGetButtonAutoReset(float X, float Y, float W) {
  EnableAlphaTest(1);
  glColor4f(1.0, 1.0, 1.0, 1.0);
  if (SEASON3B::CheckMouseIn((int)X, (int)Y, (int)W, (int)W) &&
      !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MOVEMAP)) {
    SetBlockCur(true);
    if (SEASON3B::IsPress(VK_LBUTTON) &&
        (GetTickCount() - CacheTimeClick) > 300) {
      CacheTimeClick = GetTickCount();
      PlayBuffer(SOUND_CLICK01);
      PlayBuffer(25, 0, 0);

      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.7661997676,
                   0.3775999844 + (0.06260011024 * 9), 0.1209000051,
                   0.05900005624, 1, 1, 0.0);
    } else {
      RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.6410998702,
                   0.3775999844 + (0.06260011024 * 9), 0.1209000051,
                   0.05900005624, 1, 1, 0.0);
    }
  } else {
    RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, W, 0.5159999728,
                 0.3775999844 + (0.06260011024 * 9), 0.1209000051,
                 0.05900005624, 1, 1, 0.0);
  }
  DisableAlphaBlend();
  EnableAlphaTest(0);
}
void JGetTimeBar(float X, float Y, float W, float H) {
  time_t TimeLocal;
  struct tm *LocalT;
  time(&TimeLocal);

  LocalT = localtime(&TimeLocal);
  RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, X, Y, W, H, 0.7919999957,
               0.4439999759, 0.1950000077, 0.05000000075, 1, 1, 0.0);

  char LocalTime[30];
  sprintf(LocalTime, "%2d:%02d", LocalT->tm_hour, LocalT->tm_min);

  g_pRenderText->SetFont(g_hFontBold);
  g_pRenderText->SetBgColor(0);
  g_pRenderText->SetTextColor(255, 255, 255, 255);
  g_pRenderText->RenderText((int)X, (int)Y + 4, LocalTime, (int)W, 0,
                            RT3_SORT_CENTER);
}
void CNewUIHeroPositionInfo::DrawMiniMap() {
  if (Hero == NULL)
    return;
  if (IndexIMGMap == -1 || !mShowMiniMap)
    return;

  float v67;           // [sp+94h] [bp-1094h]@172
  uint8_t v87; // [sp+10EEh] [bp-3Ah]@72
  int v92;             // [sp+10FCh] [bp-2Ch]@1
  char Cord[256];      // [sp+31h] [bp-FFh]@1
  float y;             // ST1C_4@1
  float x;             // ST18_4@1
  int JGetHelperBar;   // [sp+2Ch] [bp-104h]@1
  double RectangleRotate[2][8];

  float usercurrenty; // ST74_4@3
  float usercurrentx; // ST20_4@12

  //-- 225
  RectangleRotate[0][0] = 50.0;
  RectangleRotate[1][0] = 70.0;
  //-- 180.0
  RectangleRotate[0][1] = 51.5;
  RectangleRotate[1][1] = 69.5;
  //-- 135.0
  RectangleRotate[0][2] = 52.5;
  RectangleRotate[1][2] = 68.5;
  //-- 90.0
  RectangleRotate[0][3] = 51.5;
  RectangleRotate[1][3] = 67.0;
  //-- 45.0
  RectangleRotate[0][4] = 51.0;
  RectangleRotate[1][4] = 66.0;
  //-- 360.0
  RectangleRotate[0][5] = 49.0;
  RectangleRotate[1][5] = 67.0;

  RectangleRotate[0][6] = 48.5;
  RectangleRotate[1][6] = 68.0;

  RectangleRotate[0][7] = 49.0;
  RectangleRotate[1][7] = 69.5;

  x = 2;
  y = 2;
#if defined(__ANDROID__) || defined(MU_IOS)
  if (MU_MobileIsModernMobileHudEnabled()) {
    x = 640.0f - 108.0f;
    y = 10.0f;
  }
#endif
  EnableAlphaTest(1);
  glColor4f(1.0, 1.0, 1.0, 1.0);

  //===ButonClose
  JGetButtonClosed(x, y, 15.0);
  //=== End Button Close

  //-- Khung Map Name
  // RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, x + 16.0, y, 135.0, 15.0,
  // 0.326000005, 0.004999999888, 0.6700000167, 0.04500000179, 1, 1, 0.0);
  // g_pRenderText->SetFont(g_hFontBold);

  // g_pRenderText->SetBgColor(0);

  // g_pRenderText->SetShadowText(2);
  // g_pRenderText->SetTextColor(255, 255, 255, 255);
  // unicode::_sprintf(Cord, "%s (%d , %d)",
  // gMapManager.GetMapName(gMapManager.WorldActive), m_CurHeroPosition.x,
  // m_CurHeroPosition.y); g_pRenderText->RenderText(x + 20.0, y + 1.0, Cord,
  // 131, 13 - 4, RT3_SORT_CENTER); g_pRenderText->SetShadowText(0);
  if (ShowMiniMapColtrol) {
    //=== Vong Tron Map — kích thước cố định 84px (không đổi theo zoom). Zoom +/- chỉ đổi
    // ScaleMap trong GetDrawCircle + DataViewPortMapLoad (phạm vi / vị trí quái).
    float ViTriXMap = x;
    constexpr float kCircleD = 84.0f;
    constexpr float kCircleR = kCircleD * 0.5f;
    const float mapCenterAbsX = ViTriXMap + 9.0f + kCircleR;
    const float mapCenterAbsY = y + 20.0f + kCircleR;
    const float mapClipR = (std::max)(4.0f, kCircleR - 3.0f);

    GetDrawCircle(BITMAP_INTERFACE_CUSTOM + 5, ViTriXMap + 9.0, y + 20.0, kCircleD,
                  0.5, 0.5, 0.5, 1, 1, 0.0);
    usercurrentx = (double)(Hero->PositionX) / 256.0;
    usercurrenty = (double)(256 - Hero->PositionY) / 256.0;
    GetDrawCircle(IndexIMGMap, ViTriXMap + 9.0, y + 20.0, kCircleD, usercurrentx,
                  usercurrenty, ScaleMap, 1, 1, 0.0); // Map

    //===Show NPC monter
    DataViewPortMapLoad(x + 3, y + 19.0f, mapCenterAbsX, mapCenterAbsY, mapClipR);
    //===
    v87 = static_cast<uint8_t>((static_cast<int>(((Hero->Object.Angle[2]) + 22.5f) / 360.0f * 8.0f + 1.0f)) % 8);
    v67 = (float)(180 + (Hero->Object.Angle[2]));
    // RenderBitmapRotate
    RenderBitmapRotate(BITMAP_INTERFACE_CUSTOM + 4,
                       ViTriXMap + RectangleRotate[0][(int)v87],
                       y + RectangleRotate[1][(int)v87] - 5, 12.0, 12.0, v67,
                       0.0, 0.0, 0.1030000001, 0.05200000107);
    //-- carcaza
    RenderBitmap(BITMAP_INTERFACE_CUSTOM + 4, ViTriXMap + 5.0, y + 18.0, 92.0,
                 92.0, 0.3700000048, 0.06200000271, 0.5769999027, 0.2899999917,
                 1, 1, 0.0);
    //===================
    //-- Butoon menu
    float SacleY = 0.06260011024; // Di len thi + xuong thi -;
    float SacleX = 0.1239999682;  //+ thi di qua phai;

    JShowButtonSlash(x + 1, y + 23.0, 18.0, 0.8768191338); // Menu

    JGetEventZoom(x, y + 77.0, 18.0);          //-- zoom in
    JGetEventinZoom(x + 14.0, y + 93.0, 18.0); //-- walk away
    //===Config Auto
    if (!IsMuHelperStarted()) {
      JShowButtonPlayAuto(x + 75.0, y + 19, 18.0, 0.3775999844 + (SacleY * 2));
    } else {
      JShowButtonPlayAuto(x + 75.0, y + 19, 18.0, 0.3775999844 + (SacleY * 3));
    }
    JShowButtonConfigAuto(x + 88.0, y + 37.0, 18.0,
                          0.3775999844 + (SacleY * 5));

    JShowButton3DCamera(x + 90.0, y + 56.0, 18.0, 0.3775999844 + (SacleY * 4));
    JShowButtonHPBar(x + 85.0, y + 75.0, 18.0, 0.3775999844 + (SacleY * 9));
    JShowButtonShowName(x + 71.0, y + 93.0, 18.0, 0.3775999844 + (SacleY * 7));

    // JGetTimeBar(x + 36, y + 108, 30, 17);

    DisableAlphaBlend();
    EnableAlphaTest(0);
    //--
    EnableAlphaBlend();
    // 61520
    RenderBitmap(BITMAP_INTERFACE_CUSTOM + 2, ViTriXMap + 9.5, y + 22.5, 82.0,
                 82.0, 0.3729999959, 0.03899973631, 0.2620999813, 0.2600000203,
                 1, 1, 0.0);
  }
  DisableAlphaBlend();
  EnableAlphaTest(0);
}
// g_pNewUIMiniMap
int MoveAutoLoad = 0;
void AutoMove() {
  if (Hero == NULL)
    return;

  if (!g_pNewUIMiniMap->Movement) {
    return;
  }
  char MessText[255] = {0};
  //==Khung

  if (SEASON3B::IsPress(VK_ESCAPE)) {
    Hero->Movement = false;
    g_pNewUIMiniMap->Movement = false;
    SetPlayerStop(Hero);
    CGAutoMove(0);
    return;
  }
  RenderColor((int)(DisplayWin / 2) - (300 / 2), (int)20, 300, 40, 0.75f, 1);
  EndRenderColor();
  EnableAlphaTest();
  glColor4f(1.f, 1.f, 1.f, 1.f);

  g_pRenderText->SetBgColor(0);
  g_pRenderText->SetShadowText(1);
  g_pRenderText->SetTextColor(255, 179, 0, 255);

  unicode::_sprintf(MessText, gCustomMessage.GetMessageA(93),
                    g_pNewUIMiniMap->ViTriDiChuyen.x,
                    g_pNewUIMiniMap->ViTriDiChuyen.y);
  g_pRenderText->RenderText((int)(DisplayWin / 2) - (300 / 2), (int)30,
                            MessText, 300, 25, RT3_SORT_CENTER);

  if (MoveAutoLoad == 1) {
    g_pRenderText->RenderText((int)(DisplayWin / 2) - (50 / 2), (int)42,
                              "AUTO RUNNING ..", 0, 0, RT3_SORT_CENTER);
    MoveAutoLoad = 2;
  } else if (MoveAutoLoad == 2) {
    g_pRenderText->RenderText((int)(DisplayWin / 2) - (50 / 2), (int)42,
                              "AUTO RUNNING ...", 0, 0, RT3_SORT_CENTER);
    MoveAutoLoad = 3;
  } else if (MoveAutoLoad == 3) {
    g_pRenderText->RenderText((int)(DisplayWin / 2) - (50 / 2), (int)42,
                              "AUTO RUNNING ....", 0, 0, RT3_SORT_CENTER);
    MoveAutoLoad = 4;
  } else if (MoveAutoLoad == 4) {
    g_pRenderText->RenderText((int)(DisplayWin / 2) - (50 / 2), (int)42,
                              "AUTO RUNNING .....", 0, 0, RT3_SORT_CENTER);
    MoveAutoLoad = 0;
  } else {
    g_pRenderText->RenderText((int)(DisplayWin / 2) - (50 / 2), (int)42,
                              "AUTO RUNNING ......", 0, 0, RT3_SORT_CENTER);
    MoveAutoLoad = 1;
  }
  g_pRenderText->SetShadowText(0);
  if (Hero->Movement) {
    return;
  }

  if (Hero->Dead == 0 && Hero->Movement == 0) {
    if (Hero->Appear == 0) {
      if (PathFinding2(Hero->PositionX, Hero->PositionY,
                       g_pNewUIMiniMap->ViTriDiChuyen.x,
                       g_pNewUIMiniMap->ViTriDiChuyen.y, &Hero->Path, 0.0f)) {
        Hero->MovementType = MOVEMENT_MOVE;

        Hero->Movement = true;
        SendMove(Hero, &Hero->Object);

      } else {
        Hero->Movement = false;
        g_pNewUIMiniMap->Movement = false;
      }
    }
  } else {
    Hero->Movement = false;
    g_pNewUIMiniMap->Movement = false;
  }
}

// void ShowInfoTitleWindow()
// {
// 	//===Set WIndow INfo
// 	bool condition = true;
// 	float fpsValue = 30.5;
// 	std::string windowName = gProtect.m_MainInfo.WindowName;
// 	//if (Hero)
// 	//{
// 	//	windowName += " [Char: " + std::string(CharacterAttribute->Name)
// + "]"; //WINDOWS NAME
// 	//	windowName += " (LV:" +
// std::to_string(CharacterAttribute->Level);
// 	//	windowName += "/" +
// std::to_string(CharacterAttribute->PrintPlayer.ViewReset);
// 	//	windowName += "/" +
// std::to_string(CharacterAttribute->PrintPlayer.ViewMasterReset) + ")";
// 	//	//windowName += " || [WC " +
// std::string(gInterface.NumberFormat(CharacterAttribute->PrintPlayer.Coin1));
// 	//	//windowName += " || WP " +
// std::string(gInterface.NumberFormat(CharacterAttribute->PrintPlayer.Coin2));
// 	//	//windowName += " || GP " +
// std::string(gInterface.NumberFormat(CharacterAttribute->PrintPlayer.Coin3)) +
// "]";
// 	//}
// 	//if (gProtect.m_MainInfo.FpsMonitor)
// 	//{
// 	//	std::ostringstream ss;
// 	//	ss << std::fixed << std::setprecision(1) << " [FPS : " <<
// (int)FPS_AVG << "]";
// 	//	windowName += ss.str();
// 	//}

// 	SetWindowText(g_hWnd, windowName.c_str());
// }
void ShowInfoTitleWindow() {
  static DWORD s_LastTick = 0;
  static std::string s_LastWindowName = "";

  DWORD currentTick = GetTickCount();

  if (currentTick - s_LastTick < 100)
    return;

  s_LastTick = currentTick;

  std::string windowName = gProtect.m_MainInfo.WindowName;

  if (Hero && CharacterAttribute && CharacterMachine) {
    windowName += " || ";

    size_t nameLen =
        strnlen(CharacterAttribute->Name, sizeof(CharacterAttribute->Name));
    if (nameLen > 0)
      windowName.append(CharacterAttribute->Name, nameLen);
    else
      windowName += "Unknown";

    windowName += " || Lv: ";
    windowName += std::to_string(CharacterAttribute->Level);

    windowName += " || Rs: ";
    windowName += std::to_string(CharacterAttribute->PrintPlayer.ViewReset);

    //windowName += " || Zen: ";
    //windowName += std::string(gInterface.NumberFormat(CharacterMachine->Gold));

    windowName += " || WC: ";
    windowName += std::string(
        gInterface.NumberFormat(CharacterAttribute->PrintPlayer.Coin1));

    windowName += " || WP: ";
    windowName += std::string(
        gInterface.NumberFormat(CharacterAttribute->PrintPlayer.Coin2));

    windowName += " || GP: ";
    windowName += std::string(
        gInterface.NumberFormat(CharacterAttribute->PrintPlayer.Coin3));

    // Mouse position and Map ID - only for Admin
    if (nameLen >= 5 && strncmp(CharacterAttribute->Name, "Admin", 5) == 0) {
      char szExtra[64];
      sprintf(szExtra, " || Mouse: %d %d || Map: %d", MouseX,
                MouseY - DisplayHeightExt, gMapManager.WorldActive);
      windowName += szExtra;
    }
  } else {
    // Not in game - reset to default title if it was changed
    if (!s_LastWindowName.empty() && s_LastWindowName != windowName) {
      SetWindowText(g_hWnd, windowName.c_str());
      s_LastWindowName = "";
    }
    return;
  }

  if (windowName != s_LastWindowName) {
    SetWindowText(g_hWnd, windowName.c_str());
    s_LastWindowName = windowName;
  }
}

#if (UseNewMacroUI == 0)
bool CNewUIHeroPositionInfo::Render() {
  // gInterface.DrawMessage(1, "CNewUIHeroPositionInfo::Render() %d/%d",
  // m_Pos.x, m_Pos.y);
  ShowInfoTitleWindow();
  MiniMapLoad();
  AutoMove();
  unicode::t_char szText[255] = {
      NULL,
  };

  EnableAlphaTest();
  glColor4f(1.f, 1.f, 1.f, 1.f);

  g_pRenderText->SetFont(g_hFont);
  g_pRenderText->SetTextColor(255, 255, 255, 255);
  g_pRenderText->SetBgColor(0, 0, 0, 0);

  RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW, m_Pos.x, m_Pos.y,
              float(HERO_POSITION_INFO_BASEA_WINDOW_WIDTH),
              float(HERO_POSITION_INFO_BASE_WINDOW_HEIGHT));

  RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 1,
              m_Pos.x + HERO_POSITION_INFO_BASEA_WINDOW_WIDTH, m_Pos.y,
              float(WidenX), float(HERO_POSITION_INFO_BASE_WINDOW_HEIGHT), 0.1f,
              0.f, 22.4f / 32.f, 25.f / 32.f);

  RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 2,
              m_Pos.x + HERO_POSITION_INFO_BASEA_WINDOW_WIDTH + WidenX, m_Pos.y,
              73.f, 20.f);
  //--
  m_BtnConfig.Render();

  // if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MuHelper))
  //{
  //	m_BtnConfig.ChangeButtonState(BUTTON_STATE_DOWN, 2);
  // }
  if (!IsMuHelperStarted()) {
    m_BtnStart.Render();
  } else {
    m_BtnStop.Render();
  }

  //--
  unicode::_sprintf(szText, "%s (%d/%d)",
                    gMapManager.GetMapName(gMapManager.WorldActive),
                    m_CurHeroPosition.x, m_CurHeroPosition.y);

  g_pRenderText->RenderText(m_Pos.x + 10, m_Pos.y + 2, szText, WidenX + 20,
                            13 - 4, RT3_SORT_CENTER);

  //==MenuCustom
#if defined(__ANDROID__) || defined(MU_IOS)
  const int kMobileHeroMenuShiftX =
      (MU_MobileIsModernMobileHudEnabled() && !MU_MobileIsUtilityToolbarExpanded()) ? 176 : 0;
#else
  const int kMobileHeroMenuShiftX = 0;
#endif
  const int menuClusterX =
      m_Pos.x + HERO_POSITION_INFO_BASEA_WINDOW_WIDTH + WidenX + 57 - kMobileHeroMenuShiftX;

  if (g_pBCustomMenuInfo != NULL && gInterface.Data[eMenu].ModelID > 0 &&
      gInterface.Data[eMenu].Width > 0 &&
      gInterface.Data[eMenu].Height > 0) {
    g_pBCustomMenuInfo->DrawGUI(
        eMenuBG, menuClusterX,
        0);
    if (g_pBCustomMenuInfo->DrawButtonGUI(
            gInterface.Data[eMenu].ModelID,
            menuClusterX + 1,
            2, gInterface.Data[eMenu].Width, gInterface.Data[eMenu].Height)) {
      gInterface.Data[eMenu_MAIN].OnShow ^= 1;
    }
  }

  if (gInterface.Data[eMenu].Width > 0 && gInterface.Data[eMenu].Height > 0 &&
      SEASON3B::CheckMouseIn(
          menuClusterX + 1, 2,
          gInterface.Data[eMenu].Width, gInterface.Data[eMenu].Height) == 1) {
    RenderTipText(static_cast<float>(menuClusterX + 1 - 5),
                  2 + 27, "Menu");
  }
  DisableAlphaBlend();

  //===DrawMapo
  DrawMiniMap();
  return true;
}
#else
bool CNewUIHeroPositionInfo::Render() {
  ShowInfoTitleWindow();
  MiniMapLoad();
  unicode::t_char szText[256] = {
      NULL,
  };
  unicode::t_char szLevel[55] = {
      NULL,
  };
  unicode::t_char szReset[55] = {
      NULL,
  };
  unicode::t_char szAutoReset[55] = {
      NULL,
  };
  SetBlockCur(false);
  float TableX = (IndexIMGMap == -1) ? 20 : 110;
  float TableY = 25;
  EnableAlphaTest();
  glColor4f(1.f, 1.f, 1.f, 1.f);
  //=========
  AutoMove();

  //=========
  if (ShowMiniMapColtrol) {
    BDrawOutLine(TableX, TableY, 140, 30);
    DisableAlphaBlend();
    BRenderTabLine(TableX + 5, TableY + 17, 131.2 / 2, 18, 2, -1);
    EndRenderColor();
    //
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);
    g_pRenderText->SetFont(g_hFontBold);

    g_pRenderText->SetBgColor(0);

    g_pRenderText->SetShadowText(2);

    float CaclcX = 131.2 / 2;
    ////RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW, m_Pos.x, m_Pos.y,
    /// float(HERO_POSITION_INFO_BASEA_WINDOW_WIDTH),
    /// float(HERO_POSITION_INFO_BASE_WINDOW_HEIGHT));
    ////RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 1,
    /// m_Pos.x+HERO_POSITION_INFO_BASEA_WINDOW_WIDTH, m_Pos.y, float(WidenX),
    /// float(HERO_POSITION_INFO_BASE_WINDOW_HEIGHT),0.1f,0.f,22.4f/32.f,25.f/32.f);
    ////RenderImage(IMAGE_HERO_POSITION_INFO_BASE_WINDOW+2,
    /// m_Pos.x+HERO_POSITION_INFO_BASEA_WINDOW_WIDTH+WidenX, m_Pos.y,
    /// float(HERO_POSITION_INFO_BASEC_WINDOW_WIDTH),
    /// float(HERO_POSITION_INFO_BASE_WINDOW_HEIGHT));
    g_pRenderText->SetTextColor(255, 255, 255, 255);

    static time_t s_LastTimeServer = 0;
    static char s_TimeTextCache[64] = "";

    time_t TimeServer;
    time(&TimeServer);

    if (TimeServer != s_LastTimeServer) {
      s_LastTimeServer = TimeServer;
      struct tm *ServerT = gmtime(&TimeServer);
      int displayHour = ((ServerT->tm_hour + GMT) % 24 + 24) % 24;
      unicode::_sprintf(s_TimeTextCache, "Time : %02d:%02d:%02d", displayHour,
                        ServerT->tm_min, ServerT->tm_sec);
    }

    g_pRenderText->RenderText(TableX, TableY + 7, s_TimeTextCache, 140, 13 - 4,
                              RT3_SORT_CENTER);
    //==Level
    g_pRenderText->SetTextColor(255, 174, 0, 255);
    unicode::_sprintf(szLevel, "Lv %d", CharacterAttribute->Level);
    g_pRenderText->RenderText(TableX + 5, TableY + 21, szLevel, CaclcX, 13 - 4,
                              RT3_SORT_CENTER);
    //==szReset
    g_pRenderText->SetTextColor(0, 255, 238, 255);
    unicode::_sprintf(szReset, "Rs %d",
                      CharacterAttribute->PrintPlayer.ViewReset);
    g_pRenderText->RenderText(TableX + 5 + CaclcX, TableY + 21, szReset, CaclcX,
                              13 - 4, RT3_SORT_CENTER);
    //==sz Auto Reset Static
    //===ButonClose
    JGetButtonAutoReset(TableX + 5, TableY + 38, 18.0);
    g_pRenderText->SetTextColor(55, 55, 55, 255);
    unicode::_sprintf(szAutoReset, "[Auto RS Not Active]");
    g_pRenderText->RenderText(TableX + 5, TableY + 40, szAutoReset, 132, 13 - 4,
                              RT3_SORT_CENTER);

    g_pRenderText->SetShadowText(0);
    DisableAlphaBlend();
  }
  //===DrawMapo
  DrawMiniMap();
  return true;
}
#endif
float CNewUIHeroPositionInfo::GetLayerDepth() { return 0.1f; }

void CNewUIHeroPositionInfo::OpenningProcess() {}

void CNewUIHeroPositionInfo::ClosingProcess() {}

void CNewUIHeroPositionInfo::SetCurHeroPosition(int x, int y) {
  m_CurHeroPosition.x = x;
  m_CurHeroPosition.y = y;
}

void CNewUIHeroPositionInfo::LoadImages() {
#if (!UseNewMacroUI)
  // MAx IMAGE_HERO_POSITION_INFO_BASE_WINDOW +5;
  LoadBitmap("Interface\\Minimap_positionA.tga",
             IMAGE_HERO_POSITION_INFO_BASE_WINDOW, GL_LINEAR);
  LoadBitmap("Interface\\Minimap_positionB.tga",
             IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 1, GL_LINEAR);
  LoadBitmap("Interface\\MacroUI\\Minimap_positionC.tga",
             IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 2, GL_LINEAR);
  LoadBitmap("Interface\\MacroUI\\MacroUI_Setup.tga",
             IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 3, GL_LINEAR);
  LoadBitmap("Interface\\MacroUI\\MacroUI_Start.tga",
             IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 4, GL_LINEAR);
  LoadBitmap("Interface\\MacroUI\\MacroUI_Stop.tga",
             IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 5, GL_LINEAR);
#else
  LoadBitmap("Custom\\Maps\\CRadar.tga",
             IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 2, GL_LINEAR);
  LoadBitmap("Custom\\Maps\\map_radar.tga",
             IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 4, GL_LINEAR);
  LoadBitmap("Custom\\Maps\\background-tcp.tga",
             IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 5, GL_LINEAR);
#endif
}

void CNewUIHeroPositionInfo::UnloadImages() {
  DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW);
  DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 1);
  DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 2);
  DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 3);
  DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 4);
  DeleteBitmap(IMAGE_HERO_POSITION_INFO_BASE_WINDOW + 5);
}

void CNewUIHeroPositionInfo::SetButtonInfo(
    CNewUIButton *m_Btn, int imgindex, int x, int y, int sx, int sy,
    bool overflg, bool isimgwidth, bool bClickEffect, bool MoveTxt,
    unicode::t_string btname, unicode::t_string tooltiptext, bool istoppos) {
  m_Btn->ChangeButtonImgState(1, imgindex, overflg, isimgwidth, bClickEffect);
  m_Btn->ChangeButtonInfo(x, y, sx, sy);

  m_Btn->ChangeText(btname);
  m_Btn->ChangeToolTipText(tooltiptext, istoppos);

  if (MoveTxt) {
    m_Btn->MoveTextPos(0, -1);
  }
}

void CNewUIHeroPositionInfo::MoveTextTipPos(CNewUIButton *m_Btn, int iX,
                                            int iY) {
  m_Btn->MoveTextTipPos(iX, iY);
}
