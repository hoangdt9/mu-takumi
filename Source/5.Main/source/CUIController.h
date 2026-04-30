#pragma once

typedef void(*PUSHEVENT_CALLBACK_LPVOID)(LPVOID);

class cUIController
{
public:
	cUIController();
	~cUIController();
	float nearly(float a1);
	void Button(int IMGcode, float x, float y, float w, float h, float a6, float a7, PUSHEVENT_CALLBACK_LPVOID pCallbackFunc);
	void Button(int IMGcode, float x, float y, float w, float h, float a6, float a7, PUSHEVENT_CALLBACK_LPVOID pCallbackFunc, float uw, float uh);
	void Button(int IMGcode, float x, float y, float w, float h, float uw, float uh, float a8, float a9, PUSHEVENT_CALLBACK_LPVOID pCallbackFunc, bool a11 = false);
	int  Button(int IMGcode, float x, float y, float w, float h, float uw, float uh, int a8, PUSHEVENT_CALLBACK_LPVOID pCallbackFunc);
	void Button(int IMGcode, float x, float y, float w, float h, float uw, float uh, int a8, const BYTE* a9, PUSHEVENT_CALLBACK_LPVOID pCallbackFunc);
	void Button(int IMGcode, float x, float y, float w, float h, float uw, float uh, int a8, const BYTE* a9, PUSHEVENT_CALLBACK_LPVOID pCallbackFunc, int pCallbackValue);
	bool BButton(int IMGcode, float x, float y, float w, float h, float uw, float uh, int a8, const BYTE* a9);

	int GetCallBackValue();
private:
	int CallBackValue;
};

extern cUIController UIController;