// MemScript.h: interface for the CMemScript class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MEM_SCRIPT_ALLOC_ERROR "[%s] Could not alloc memory for MemScript\n"
#define MEM_SCRIPT_ERROR_CODE0 "[%s] Could not open file\n"
#define MEM_SCRIPT_ERROR_CODE1 "[%s] Could not alloc file buffer\n"
#define MEM_SCRIPT_ERROR_CODE2 "[%s] Could not read file\n"
#define MEM_SCRIPT_ERROR_CODE3 "[%s] Could not get file buffer\n"
#define MEM_SCRIPT_ERROR_CODE4 "[%s] The file were not configured correctly\n"
#define MEM_SCRIPT_ERROR_CODEX "[%s] Unknow error code: %d\n"

#define MAIN_LANGUAGE 0x00E8C5C4
#define MAIN_CONNECTION_STATUS 0x08793704
#define MAIN_SCREEN_STATE 0x00E609E8
#define MAIN_CHARACTER_STRUCT 0x08128AC8
#define MAIN_VIEWPORT_STRUCT 0x07BC4F04
#define MAIN_PACKET_SERIAL 0x08793700
#define MAIN_HELPER_STRUCT 0x00E8CB7C
#define MAIN_FONT_SIZE 0x081C0380
#define MAIN_RESOLUTION 0x00E8C240
#define MAIN_RESOLUTION_X 0x00E61E58
#define MAIN_RESOLUTION_Y 0x00E61E5C
#define MAIN_PARTY_MEMBER_COUNT 0x081F6B6C

#define ProtocolCore ((BOOL(*)(DWORD,BYTE*,DWORD,DWORD))0x00663B20)
#define DrawInterfaceText ((void(*)(char*,int,int,int,int,int,int,int))0x007D04D0)


enum eTokenResult
{
	TOKEN_NUMBER = 0,
	TOKEN_STRING = 1,
	TOKEN_END = 2,
	TOKEN_ERROR = 3,
};

class CMemScript
{
public:
	CMemScript();
	virtual ~CMemScript();
	bool SetBuffer(char* path);
	bool GetBuffer(char* buff,DWORD* size);
	char GetChar();
	void UnGetChar(char ch);
	char CheckComment(char ch);
	eTokenResult GetToken();
	eTokenResult GetTokenNumber(char ch);
	eTokenResult GetTokenString(char ch);
	eTokenResult GetTokenCommon(char ch);
	void SetLastError(int error);
	char* GetLastError();
	int GetNumber();
	int GetAsNumber();
	float GetFloatNumber();
	float GetAsFloatNumber();
	char* GetString();
	char* GetAsString();
private:
	char* m_buff;
	DWORD m_size;
	char m_path[256];
	DWORD m_count;
	float m_number;
	char m_string[256];
	DWORD m_tick;
	char m_LastError[256];
};
