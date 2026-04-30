// Protect.cpp: implementation of the CProtect class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Protect.h"
#include "Utilities/CCRC32.H"
#include "Util.h"

CProtect gProtect;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProtect::CProtect() // OK
{
	ZeroMemory(&this->m_MainInfo, sizeof(this->m_MainInfo));
	ZeroMemory(&this->m_TextInfo, sizeof(this->m_TextInfo));
}

CProtect::~CProtect() // OK
{

}

BYTE EncDecKey1;
BYTE EncDecKey2;
bool CProtect::CheckSocketPort(SOCKET s) // OK1
{
	SOCKADDR_IN addr;
	int addr_len = sizeof(addr);

	if (getpeername(s, (SOCKADDR*)& addr, &addr_len) == SOCKET_ERROR)
	{
		return 0;
	}

	int port = ntohs(addr.sin_port);
	if ((port < gProtect.m_MainInfo.GSPortMin || port > gProtect.m_MainInfo.GSPortMax))
	{
		return 0;
	}

	return 1;
}
void CProtect::DecryptData(BYTE* lpMsg, int size) // OK
{
	for (int n = 0; n < size; n++)
	{
		lpMsg[n] ^= EncDecKey1;
		lpMsg[n] -= EncDecKey2;
	}
}

void CProtect::EncryptData(BYTE* lpMsg, int size) // OK
{
	for (int n = 0; n < size; n++)
	{
		lpMsg[n] += EncDecKey2;
		lpMsg[n] ^= EncDecKey1;
	}
}
void CProtect::LoadEncDec()
{
#if(ENCRYPT_STATE==1)
	//==Init EncDec
	WORD EncDecKey = 0;

	for (int n = 0; n < sizeof(gProtect.m_MainInfo.CustomerName); n++)
	{
		EncDecKey += (BYTE)(gProtect.m_MainInfo.CustomerName[n] ^ gProtect.m_MainInfo.ClientSerial[(n % sizeof(gProtect.m_MainInfo.ClientSerial))]);
	}
	////==VietPlus
	//EncDecKey1 = (BYTE)0x01;
	//EncDecKey2 = (BYTE)0x02; //default 76
	//===DO
	EncDecKey1 = BEncDecKey1;
	EncDecKey2 = BEncDecKey2; //default 76

	EncDecKey1 += LOBYTE(EncDecKey);
	EncDecKey2 += HIBYTE(EncDecKey);
#endif
}
bool CProtect::ReadMainFile(char* name) // OK
{
	CCRC32 CRC32;

	if (CRC32.FileCRC(name, reinterpret_cast<unsigned long*>(&this->m_ClientFileCRC), 1024) == 0)
	{
		return 0;
	}

	HANDLE file = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, 0);

	if (file == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	if (GetFileSize(file, 0) != sizeof(MAIN_FILE_INFO))
	{
		CloseHandle(file);
		return 0;
	}

	DWORD OutSize = 0;

	if (ReadFile(file, &this->m_MainInfo, sizeof(MAIN_FILE_INFO), &OutSize, 0) == 0)
	{
		CloseHandle(file);
		return 0;
	}

	for (int n = 0; n < sizeof(MAIN_FILE_INFO); n++)
	{
		((BYTE*)& this->m_MainInfo)[n] -= (BYTE)(0x95 ^ HIBYTE(n));
		((BYTE*)& this->m_MainInfo)[n] ^= (BYTE)(0xCA ^ LOBYTE(n));
	}

	CloseHandle(file);


	return 1;
}

bool CProtect::ReadTextFile(char* name) // OK
{
	HANDLE file = CreateFile(name,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,0);

	if(file == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	if(GetFileSize(file,0) != sizeof(TEXT_FILE_INFO))
	{
		CloseHandle(file);
		MessageBox(0, "ReadTextFile 1", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	DWORD OutSize = 0;

	if(ReadFile(file,&this->m_TextInfo,sizeof(TEXT_FILE_INFO),&OutSize,0) == 0)
	{
		CloseHandle(file);
		MessageBox(0, "ReadTextFile 2", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	for(int n=0;n < sizeof(TEXT_FILE_INFO);n++)
	{
		((BYTE*)&this->m_TextInfo)[n] -= (BYTE)(0x95^HIBYTE(n));
		((BYTE*)&this->m_TextInfo)[n] ^= (BYTE)(0xCA^LOBYTE(n));
	}

	CloseHandle(file);


	return 1;
}

void CProtect::CheckLauncher() // OK
{
	if ((this->m_MainInfo.LauncherType & 1) == 0)
	{
		return;
	}

	if (FindWindow(0, gProtect.m_MainInfo.LauncherName) == 0)
	{
		//int result = system(gProtect.m_MainInfo.FileLauncherName);
		WinExec(gProtect.m_MainInfo.LauncherFile, 0);
		ExitProcess(0);
	}


}

void CProtect::CheckInstance() // OK
{
	if ((this->m_MainInfo.LauncherType & 2) == 0)
	{
		return;
	}

	HANDLE hMutex = CreateMutex(NULL, FALSE, gProtect.m_MainInfo.LauncherName);
	//
	if (GetLastError() != ERROR_ALREADY_EXISTS)
	{
		WinExec(gProtect.m_MainInfo.LauncherFile, 0);	//ten launcher
		ExitProcess(0);
	}
	return;
}

void CProtect::CheckClientFile() // OK
{
	if(this->m_MainInfo.ClientCRC32 == 0)
	{
		return;
	}

	char name[MAX_PATH] = {0};

	if(GetModuleFileName(0,name,sizeof(name)) == 0)
	{
		ExitProcess(0);
	}

	if(_stricmp(ConvertModuleFileName(name),this->m_MainInfo.ClientName) != 0)
	{
		ExitProcess(0);
	}

	CCRC32 CRC32;

	DWORD ClientCRC32;

	if(CRC32.FileCRC(this->m_MainInfo.ClientName,reinterpret_cast<unsigned long*>(&ClientCRC32),1024) == 0)
	{
		ExitProcess(0);
	}

	if(this->m_MainInfo.ClientCRC32 != ClientCRC32)
	{
		ExitProcess(0);
	}
}

void CProtect::CheckPluginFile() // OK
{
	if(this->m_MainInfo.PluginCRC32 == 0)
	{
		return;
	}

	CCRC32 CRC32;

	DWORD PluginCRC32;

	if(CRC32.FileCRC(this->m_MainInfo.PluginName,reinterpret_cast<unsigned long*>(&PluginCRC32),1024) == 0)
	{
		ExitProcess(0);
	}

	if(this->m_MainInfo.PluginCRC32 != PluginCRC32)
	{
		ExitProcess(0);
	}

	HMODULE module = LoadLibrary(this->m_MainInfo.PluginName);

	if(module == 0)
	{
		ExitProcess(0);
	}

	void (*EntryProc)() = (void(*)())GetProcAddress(module,"EntryProc");

	if(EntryProc != 0)
	{
		EntryProc();
	}
}

void CProtect::CheckCameraFile() // OK
{
	if(this->m_MainInfo.CameraCRC32 == 0)
	{
		return;
	}

	CCRC32 CRC32;

	DWORD CameraCRC32;

	if(CRC32.FileCRC(this->m_MainInfo.CameraName,reinterpret_cast<unsigned long*>(&CameraCRC32),1024) == 0)
	{
		ExitProcess(0);
	}

	if(this->m_MainInfo.CameraCRC32 != CameraCRC32)
	{
		ExitProcess(0);
	}

	HMODULE module = LoadLibrary(this->m_MainInfo.CameraName);

	if(module == 0)
	{
		ExitProcess(0);
	}

	void (*EntryProc)() = (void(*)())GetProcAddress(module,"EntryProc");

	if(EntryProc != 0)
	{
		EntryProc();
	}
}
