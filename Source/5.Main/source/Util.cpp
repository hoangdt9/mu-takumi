#include "stdafx.h"
#include "Util.h"
#include "NewUISystem.h"
BYTE NewAddressData1[240];
BYTE NewAddressData2[240];
BYTE NewAddressData3[6000];

void SetByte(DWORD offset,BYTE value) // OK
{
	DWORD OldProtect;

	VirtualProtect((void*)offset,1,PAGE_EXECUTE_READWRITE,&OldProtect);

	*(BYTE*)(offset) = value;

	VirtualProtect((void*)offset,1,OldProtect,&OldProtect);
}

void SetWord(DWORD offset,WORD value) // OK
{
	DWORD OldProtect;

	VirtualProtect((void*)offset,2,PAGE_EXECUTE_READWRITE,&OldProtect);

	*(WORD*)(offset) = value;

	VirtualProtect((void*)offset,2,OldProtect,&OldProtect);
}

void SetDword(DWORD offset,DWORD value) // OK
{
	DWORD OldProtect;

	VirtualProtect((void*)offset,4,PAGE_EXECUTE_READWRITE,&OldProtect);

	*(DWORD*)(offset) = value;

	VirtualProtect((void*)offset,4,OldProtect,&OldProtect);
}

void SetFloat(DWORD offset,float value) // OK
{
	DWORD OldProtect;

	VirtualProtect((void*)offset,4,PAGE_EXECUTE_READWRITE,&OldProtect);

	*(float*)(offset) = value;

	VirtualProtect((void*)offset,4,OldProtect,&OldProtect);
}

void SetDouble(DWORD offset,double value) // OK
{
	DWORD OldProtect;

	VirtualProtect((void*)offset,8,PAGE_EXECUTE_READWRITE,&OldProtect);

	*(double*)(offset) = value;

	VirtualProtect((void*)offset,8,OldProtect,&OldProtect);
}

void SetCompleteHook(BYTE head,DWORD offset,...) // OK
{
	DWORD OldProtect;

	VirtualProtect((void*)offset,5,PAGE_EXECUTE_READWRITE,&OldProtect);

	if(head != 0xFF)
	{
		*(BYTE*)(offset) = head;
	}

	DWORD* function = &offset+1;

	*(DWORD*)(offset+1) = (*function)-(offset+5);

	VirtualProtect((void*)offset,5,OldProtect,&OldProtect);
}

void MemoryCpy(DWORD offset,void* value,DWORD size) // OK
{
	DWORD OldProtect;

	VirtualProtect((void*)offset,size,PAGE_EXECUTE_READWRITE,&OldProtect);

	memcpy((void*)offset,value,size);

	VirtualProtect((void*)offset,size,OldProtect,&OldProtect);
}

void MemorySet(DWORD offset,DWORD value,DWORD size) // OK
{
	DWORD OldProtect;

	VirtualProtect((void*)offset,size,PAGE_EXECUTE_READWRITE,&OldProtect);

	memset((void*)offset,value,size);

	VirtualProtect((void*)offset,size,OldProtect,&OldProtect);
}

void VirtualizeOffset(DWORD offset,DWORD size) // OK
{
	DWORD OldProtect;

	VirtualProtect((void*)offset,size,PAGE_EXECUTE_READWRITE,&OldProtect);

	DWORD HookAddr = (DWORD)(uintptr_t)malloc(size+5);

	memcpy((void*)HookAddr,(void*)offset,size);

	*(BYTE*)(HookAddr+size) = 0xE9;

	*(DWORD*)(HookAddr+size+1) = (offset+size)-((HookAddr+size)+5);

	*(BYTE*)(offset) = 0xE9;

	*(DWORD*)(offset+1) = HookAddr-(offset+5);

	memset((void*)(offset+5),0x90,(size-5));

	VirtualProtect((void*)offset,size,OldProtect,&OldProtect);
}

void PacketArgumentEncrypt(char* out_buff,char* in_buff,int size) // OK
{
	BYTE XorTable[3] = {0xFC,0xCF,0xAB};

	for(int n=0;n < size;n++)
	{
		out_buff[n] = in_buff[n]^XorTable[n%3];
	}
}

char* ConvertModuleFileName(char* name) // OK
{
	static char buff[MAX_PATH] = {0};

	for(int n=strlen(name);n > 0;n--)
	{
		if(name[n] == '\\')
		{
			strcpy_s(buff,sizeof(buff),&name[(n+1)]);
			break;
		}
	}

	return buff;
}

void LoadReferenceAddressTable(HMODULE mod,char* name,DWORD address) // OK
{
#if defined(__ANDROID__)
	(void)mod; (void)name; (void)address;
	return;
#else
	if(IS_INTRESOURCE(name) == 0)
	{
		return;
	}

	HRSRC resource = FindResource(mod,name,"BIN");

	if(resource == 0)
	{
		return;
	}

	if(SizeofResource(mod,resource) < sizeof(REFERENCE_INFO))
	{
		return;
	}

	HGLOBAL data = LoadResource(mod,resource);

	if(data == 0)
	{
		return;
	}

	REFERENCE_INFO* ReferenceInfo = (REFERENCE_INFO*)LockResource(data);

	if(ReferenceInfo == 0)
	{
		FreeResource(data);
		return;
	}

	if(ReferenceInfo->start == ReferenceInfo->end)
	{
		if(SizeofResource(mod,resource) < (sizeof(REFERENCE_INFO)+(ReferenceInfo->count*sizeof(REFERENCE_BASE))))
		{
			return;
		}

		REFERENCE_BASE* ReferenceBase = (REFERENCE_BASE*)((uintptr_t)ReferenceInfo+sizeof(REFERENCE_INFO));

		for(DWORD n=0;n < ReferenceInfo->count;n++)
		{
			SetDword(ReferenceBase[n].address,address);
		}
	}
	else
	{
		if(SizeofResource(mod,resource) < (sizeof(REFERENCE_INFO)+(ReferenceInfo->count*sizeof(REFERENCE_DATA))))
		{
			return;
		}

		REFERENCE_DATA* ReferenceData = (REFERENCE_DATA*)((uintptr_t)ReferenceInfo+sizeof(REFERENCE_INFO));

		for(DWORD n=0;n < ReferenceInfo->count;n++)
		{
			SetDword(ReferenceData[n].address,(address+ReferenceData[n].value));
		}
	}

	FreeResource(data);
#endif
}
//--------------------------------------------------------------------------------------------------------------------------
//-> Add MiniMap + Auto Move
void ErrorMessageBox(char* message, ...) // OK
{
	char buff[256];

	memset(buff, 0, sizeof(buff));

	va_list arg;
	va_start(arg, message);
	vsnprintf(buff, sizeof(buff), message, arg);
	va_end(arg);

	MessageBox(0, buff, "Error", MB_OK | MB_ICONERROR);

	ExitProcess(0);
}

bool FileExist(LPCSTR path)
{
	HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, 0);

	if (file == INVALID_HANDLE_VALUE)
	{
		CloseHandle(file);
		return 0;
	}

	CloseHandle(file);
	return true;
}

char* CharToLower(char* text) // OK
{
	static char buff[256];

	memset(buff, 0, sizeof(buff));

	int size = strlen(text);

	memcpy(buff, text, size);

	buff[size] = 0;

	for (int n = 0; n < size; n++)
	{
		buff[n] = tolower(buff[n]);
	}

	return buff;
}

int DrawMessage(int Mode, LPCSTR Text, ...)
{
	char Buff[2048];
	int BuffLen = sizeof(Buff);
	ZeroMemory(Buff, BuffLen);
	// ----
	va_list args;
	va_start(args, Text);
	int Len = vsprintf_s(Buff, BuffLen, Text, args);
	va_end(args);
	// ----
	g_pChatListBox->AddText("", Buff,(SEASON3B::MESSAGE_TYPE) Mode);
	return 1;
}

bool CheckItemIsJewel(int ItemType)
{
	if ((ItemType == ITEM_POTION + 13)
		|| (ItemType == ITEM_POTION + 14)
		|| (ItemType == ITEM_POTION + 16)
		|| (ItemType == ITEM_POTION + 22)
		|| (ItemType == ITEM_POTION + 31)
		|| (ItemType == ITEM_POTION + 41)
		|| (ItemType == ITEM_POTION + 42)
		|| (ItemType == ITEM_WING + 15)
		|| (ItemType == ITEM_WING + 30)
		|| (ItemType == ITEM_WING + 31)
		|| (gCustomJewel.CheckCustomJewelByItem(ItemType)) //jewel custom
		|| (ItemType >= ITEM_WING + 136 && ItemType <= ITEM_WING + 143)) //Ngoc cum
	{
		return 1;
	}
	return 0;
}

char* BGetItemName(int ItemType, int Level)
{
	char ItemNameGet[100];
	memset(ItemNameGet, 0, sizeof(ItemNameGet));
	GetItemName(ItemType, (GET_ITEMOPT_LEVEL(Level)), (char*)ItemNameGet);
	//pBGetItemName(ItemType, (const char*)(GET_ITEMOPT_LEVEL(Level)), (char*)ItemNameGet);

	std::string a(ItemNameGet);

	return strdup(a.c_str());

}
int GetPageMax(int totalLines, int linesPerPage)
{
	int pages = totalLines / linesPerPage;
	if (totalLines % linesPerPage != 0) {
		pages++;
	}
	return pages;
}

char* CharacterCode(int a)
{
	unicode::t_string tooltiptext1 = GlobalText[20];
	switch (a)
	{
	case 0:  return (char*)GlobalText[20];
	case 1:  return (char*)GlobalText[25];
	case 2:  return (char*)GlobalText[1669];
	case 3:  return (char*)GlobalText[1669];
	case 16: return  (char*)GlobalText[21];
	case 17: return  (char*)GlobalText[26];
	case 18: return  (char*)GlobalText[1668];
	case 19: return  (char*)GlobalText[1668];
	case 32: return  (char*)GlobalText[22];
	case 33: return  (char*)GlobalText[27];
	case 34: return  (char*)GlobalText[1670];
	case 35: return  (char*)GlobalText[1670];
	case 48: return  (char*)GlobalText[23];
	case 50: return  (char*)GlobalText[1671];
	case 66: return  (char*)GlobalText[24];
	case 67: return  (char*)GlobalText[24];
	case 64: return  (char*)GlobalText[24];
	case 65: return  (char*)GlobalText[24];
	case 68: return  (char*)GlobalText[1672];
	case 80: return  (char*)GlobalText[1687];
	case 81: return  (char*)GlobalText[1688];
	case 82: return  (char*)GlobalText[1689];
	case 83: return  (char*)GlobalText[1689];
	case 96: return  (char*)GlobalText[3150];
	case 97: return  (char*)GlobalText[3151];
	case 98: return  (char*)GlobalText[3151];
	}
	return "unknown";
}

bool CheckChuoiKyTuDacBiet(const std::string& str)
{
	for (char ch : str)
	{
		if (!isdigit(ch) && !(ch >= 'a' && ch <= 'z') && !(ch >= 'A' && ch <= 'Z'))
		{
			return false;
		}
	}
	return true;
}

int getNumberOfExcOptions(int checksum)
{
	int optionscount = 0;
	int ExcOrgArr[6];

	ExcOrgArr[0] = 1;
	ExcOrgArr[1] = 2;
	ExcOrgArr[2] = 4;
	ExcOrgArr[3] = 8;
	ExcOrgArr[4] = 16;
	ExcOrgArr[5] = 32;

	if (checksum > 0)
	{
		for (int i = 0; i < 6; i++)
		{
			int and_val = checksum & ExcOrgArr[i];
			if (and_val != 0)
				optionscount += 1;
		}
	}

	return optionscount;
}

void RenderMixEffect(int type, int x, int y, int iWidth, int iHeight)
{
	if (type <= 0)
	{
		return;
	}

	EnableAlphaBlend();
	for (int h = 0; h < iHeight; ++h)
	{
		for (int w = 0; w < iWidth; ++w)
		{
			glColor3f((float)(rand() % 6 + 6) * 0.1f, (float)(rand() % 4 + 4) * 0.1f, 0.2f);
			float Rotate = (float)((int)(WorldTime) % 100) * 20.f;
			float Scale = 5.f + (rand() % 10);
			RenderBitmapRotate(BITMAP_SHINY, x + ((rand() % iWidth)*20), y + ((rand() % iHeight) * 20), Scale, Scale, 0);
			RenderBitmapRotate(BITMAP_SHINY, x + ((rand() % iWidth) * 20), y + ((rand() % iHeight) * 20), Scale, Scale, Rotate);
			RenderBitmapRotate(BITMAP_SHINY + 1, x + ((rand() % iWidth) * 20), y + ((rand() % iHeight) * 20), Scale * 3.f, Scale * 3.f, Rotate);
			RenderBitmapRotate(BITMAP_LIGHT, x + ((rand() % iWidth) * 20), y + ((rand() % iHeight) * 20), Scale * 6.f, Scale * 6.f, 0);


		}
	}
	DisableAlphaBlend();
}

