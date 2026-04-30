#pragma once
#include "zzzinfomation.h"

#include <atomic>

class SpinLock {
	std::atomic_flag locked = ATOMIC_FLAG_INIT;
public:
	void lock()
	{
		while (locked.test_and_set(std::memory_order_acquire))
		{
			Sleep(1);
		}
	}

	void unlock()
	{
		locked.clear(std::memory_order_release);
	}
};

extern int MenuStateCurrent;
extern int MenuStateNext;
extern int  SceneFlag;
extern int  MoveSceneFrame;
//extern bool EnableEdit;
extern int  ErrorMessage;
extern bool InitServerList;
extern bool InitLogIn;
extern bool InitLoading;
extern bool InitCharacterScene;
extern bool InitMainScene;
extern bool EnableMainRender;

extern int g_iLengthAuthorityCode;

inline SpinLock* g_render_lock = new SpinLock();
inline SpinLock* g_protocol_lock = new SpinLock();


extern void LogInScene(HDC hDC);
extern void LoadingScene(HDC hDC);
extern void Scene(HDC Hdc);
extern bool CheckName();
void    StartGame();
void SetTargetFps(float targetFps);
BOOL	ShowCheckBox( int num, int index, int message=MESSAGE_TRADE_CHECK );

int	SeparateTextIntoLines( const char* lpszText, char *lpszSeparated, int iMaxLine, int iLineSize);

bool	GetTimeCheck(int DelayTime);
void	SetEffectVolumeLevel(int level);
void    SetViewPortLevel ( int level );

bool IsEnterPressed();
void SetEnterPressed( bool enterpressed );
extern float   DeltaT;



