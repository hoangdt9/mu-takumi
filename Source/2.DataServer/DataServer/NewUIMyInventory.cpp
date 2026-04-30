#include "stdafx.h"
#if(HAISLOTRING)
#include "NewUIMyInventory.h"
#include "QueryManager.h"
#include "SocketManager.h"

NewUIMyInventory g_pNewMyInventory;

NewUIMyInventory::NewUIMyInventory(void)
{
}

NewUIMyInventory::~NewUIMyInventory(void)
{
}

void NewUIMyInventory::GDPetInventoryRecv(SDHP_PET_INVENTORY_RECV* lpMsg, int index) // OK
{
#if(DATASERVER_UPDATE>=603)

	SDHP_PET_INVENTORY_SEND pMsg;

	pMsg.header.set(0x27, 0x01, sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account, lpMsg->account, sizeof(pMsg.account));

	memcpy(pMsg.name, lpMsg->name, sizeof(pMsg.name));

	if (gQueryManager.ExecQuery("SELECT Items FROM EquipInventory WHERE CharName='%s'", lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		memset(pMsg.PetInventory, 0xFF, sizeof(pMsg.PetInventory));
	}
	else
	{
		gQueryManager.GetAsBinary("Items", pMsg.PetInventory[0], sizeof(pMsg.PetInventory));

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index, (BYTE*)& pMsg, sizeof(pMsg));

#endif
}

void NewUIMyInventory::GDPetInventorySaveRecv(SDHP_PET_INVENTORY_SAVE_RECV* lpMsg) // OK
{
#if(DATASERVER_UPDATE>=603)

	if (gQueryManager.ExecQuery("SELECT CharName FROM EquipInventory WHERE CharName='%s'", lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.BindParameterAsBinary(1, lpMsg->PetInventory[0], sizeof(lpMsg->PetInventory));
		gQueryManager.ExecQuery("INSERT INTO EquipInventory (CharName, Items) VALUES ('%s',?)", lpMsg->name);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.BindParameterAsBinary(1, lpMsg->PetInventory[0], sizeof(lpMsg->PetInventory));
		gQueryManager.ExecQuery("UPDATE EquipInventory SET Items=? WHERE CharName='%s'", lpMsg->name);
		gQueryManager.Close();
	}

#endif
}
#endif