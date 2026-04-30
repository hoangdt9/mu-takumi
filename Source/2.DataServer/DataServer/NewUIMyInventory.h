#pragma once
#if(HAISLOTRING)
#include "DataServerProtocol.h"

#define PET_INVENTORY_SIZE 16

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_PET_INVENTORY_RECV
{
	PSBMSG_HEAD header; // C1:27:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_PET_INVENTORY_SAVE_RECV
{
	PSWMSG_HEAD header; // C2:27:30
	WORD index;
	char account[11];
	char name[11];
	BYTE PetInventory[PET_INVENTORY_SIZE][16];
};

struct SDHP_PET_INVENTORY_SEND
{
	PSWMSG_HEAD header; // C2:27:00
	WORD index;
	char account[11];
	char name[11];
	BYTE PetInventory[PET_INVENTORY_SIZE][16];
};

class NewUIMyInventory
{
public:
	NewUIMyInventory(void);
	virtual ~NewUIMyInventory(void);
	void GDPetInventoryRecv(SDHP_PET_INVENTORY_RECV* lpMsg, int index);
	void GDPetInventorySaveRecv(SDHP_PET_INVENTORY_SAVE_RECV* lpMsg);
};

extern NewUIMyInventory g_pNewMyInventory;
#endif