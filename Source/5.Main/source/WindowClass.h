#pragma once
#include "Protocol.h"
typedef void(*PUSHEVENT_CALLBACK_LPVOID)(LPVOID);


struct PMSG_CHANGECLASS_DATA
{
	PSBMSG_HEAD	Head;
	int m_WCoinC;
};
struct CG_CHANGECLASS_SEND
{
	PSBMSG_HEAD Head;
	int Type;
};
class cWindowClass
{
public:
	cWindowClass();
	~cWindowClass();
	void Init();
	void Hook();
	void ReadRegeditConfig();
	void SetRect(float a1, float a2, float a3, float a4);
	void Draw();
	void CreateText();
	void TabHuongDan();
	void TabPhuThuy();
	void TabChienBinh();
	void TabTienNu();
	void TabDauSi();
	void TabChuaTe();
	void TabThuatSi();
	void TabThietBinh();
	bool BlockMouse();
	bool GetVisible();
	void SetVisible(bool a1);
	void SetCurTab(int a1);
	int  GetSelectedTab();
	static void SelectTabAction(LPVOID pClass);
	static void ChangeClassDW(LPVOID pClass);
	static void ChangeClassDK(LPVOID pClass);
	static void ChangeClassEFL(LPVOID pClass);
	static void ChangeClassMG(LPVOID pClass);
	static void ChangeClassDL(LPVOID pClass);
	static void ChangeClassSUM(LPVOID pClass);
	static void ChangeClassRF(LPVOID pClass);
	void DrawButton(int IMGcode, float x, float y, float w, float h, float a6, float a7);
	int WCoinChangeClass;
	void RecvData(PMSG_CHANGECLASS_DATA* Data);
	void SendChangeClass(int Type);
	void CachePhoto();
	void SetClassPhoto(int Class);
private:
	float x;
	float y;
	float w;
	float h;
	bool Show;
	int	  CurTab;
	int   Text;
	bool WindowMode;
	int  CurLanguage;
	bool Tab03_Option[27];
	BYTE Tab03_ASM_Backup[27][4];
	int Page;
	
	//cScrollText zText1;
	//cScrollText zText2;
	//cScrollText zText3;
	//cScrollText zText4;
	//cScrollText zText5;
	//cScrollText zText6;
	//cScrollText zText7;


};

extern cWindowClass WindowClass;