#pragma once

struct REFERENCE_INFO
{
	DWORD count;
	DWORD start;
	DWORD end;
};

struct REFERENCE_BASE
{
	DWORD address;
};

struct REFERENCE_DATA
{
	DWORD address;
	DWORD value;
};

void SetByte(DWORD offset,BYTE value);
void SetWord(DWORD offset,WORD value);
void SetDword(DWORD offset,DWORD value);
void SetFloat(DWORD offset,float value);
void SetDouble(DWORD offset,double value);
void SetCompleteHook(BYTE head,DWORD offset,...);
void MemoryCpy(DWORD offset,void* value,DWORD size);
void MemorySet(DWORD offset,DWORD value,DWORD size);
void VirtualizeOffset(DWORD offset,DWORD size);
void PacketArgumentEncrypt(char* out_buff,char* in_buff,int size);
char* ConvertModuleFileName(char* name);
void LoadReferenceAddressTable(HMODULE mod,char* name,DWORD address);
int DrawMessage(int Mode, LPCSTR Text, ...);
//-> Add MiniMap + Auto Move
void ErrorMessageBox(char* message, ...);
bool FileExist(LPCSTR path);
bool CheckItemIsJewel(int ItemType);
char* CharToLower(char* text);
char* BGetItemName(int ItemType, int Level);
extern DWORD FrameValue;
extern DWORD MainTickCount;
extern BYTE NewAddressData1[240];
extern BYTE NewAddressData2[240];
extern BYTE NewAddressData3[6000];
int GetPageMax(int totalLines, int linesPerPage);
char* CharacterCode(int a);
bool CheckChuoiKyTuDacBiet(const std::string& str);

int getNumberOfExcOptions(int checksum);

void RenderMixEffect(int type, int x, int y, int iWidth = 2, int iHeight = 4);