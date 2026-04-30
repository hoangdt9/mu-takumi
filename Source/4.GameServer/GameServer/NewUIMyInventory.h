#pragma once
#pragma once
#if(HAISLOTRING)
#define EQUIP_INVENTORY_SIZE 16
#define CTRL_INVENTORY_WEAR_SIZE 4
#define EQUIP_INVENTORY_RANGE(x) (((x)<0)?0:((x)>=EQUIP_INVENTORY_SIZE)?0:1)
#define CTRL_INVENTORY_BASE_RANGE(x) (((x)<CTRL_INVENTORY_WEAR_SIZE)?0:((x)>=EQUIP_INVENTORY_SIZE)?0:1)

#define EQUIPMENT_WEAPON_RIGHT	0
#define EQUIPMENT_WEAPON_LEFT	1
#define EQUIPMENT_HELM			2
#define EQUIPMENT_ARMOR			3
#define EQUIPMENT_PANTS			4
#define EQUIPMENT_GLOVES		5
#define EQUIPMENT_BOOTS			6
#define EQUIPMENT_WING			7
#define EQUIPMENT_HELPER		8
#define EQUIPMENT_AMULET		9
#define EQUIPMENT_RING_RIGHT	10
#define EQUIPMENT_RING_LEFT		11
#define MAX_EQUIPMENT_INDEX		12
#define EQUIPMENT_PET_1			12
#define EQUIPMENT_ERRING_RIGHT	13
#define EQUIPMENT_ERRING_LEFT	14
#define EQUIPMENT_PENTA			15

struct SDHP_PET_INVENTORY_SEND
{
	PSBMSG_HEAD header; // C1:27:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_PET_INVENTORY_SAVE_SEND
{
	PSWMSG_HEAD header; // C2:27:30
	WORD index;
	char account[11];
	char name[11];
	BYTE PetInventory[EQUIP_INVENTORY_SIZE][16];
};

struct SDHP_PET_INVENTORY_RECV
{
	PSWMSG_HEAD header; // C2:27:00
	WORD index;
	char account[11];
	char name[11];
	BYTE PetInventory[EQUIP_INVENTORY_SIZE][16];
};

struct PMSG_PET_ITEM_LIST_SEND
{
	PSWMSG_HEAD header; // C4:4E:02
	BYTE count;
};

struct PMSG_PET_ITEM_LIST
{
	BYTE slot;
	BYTE ItemInfo[MAX_ITEM_INFO];
};

struct PMSG_PET_ITEM_DUR_SEND
{
	PSBMSG_HEAD header; // C1:4E:03
	BYTE slot;
	BYTE dur;
};

struct PMSG_PET_ITEM_DELETE_SEND
{
	PSBMSG_HEAD header; // C1:4E:04
	BYTE slot;
	BYTE flag;
};

struct PMSG_PET_ITEM_CHANGE_SEND
{
	PSBMSG_HEAD header; // C1:4E:06
	BYTE index[2];
	BYTE ItemInfo[MAX_ITEM_INFO];
};

struct PMSG_EQUIP_ITEM_SELL_RECV
{
	PSBMSG_HEAD header; // C3:4E:09
	BYTE slot;
};

class CNewUIMyInventory
{
public:
	CNewUIMyInventory(void);
	virtual ~CNewUIMyInventory(void);
	void MainProc();
	void EquipSprtite(LPOBJ lpObj, int damage);
	void PetInventoryDelItem(LPOBJ lpObj, int slot); // OK
	void CalcItemCommonOption(LPOBJ lpObj, bool flag);
	void PetInventoryItemSet(int aIndex, int slot, BYTE type);
	BYTE PetInventoryAddItem(int aIndex, CItem item, int slot);
	//--
	void GCPetItemListSend(int aIndex);
	void GCPetItemChangeSend(int aIndex, BYTE slot); // OK
	void GCPetItemDurSend(int aIndex, BYTE slot, BYTE dur); // OK
	void GCPetItemDeleteSend(int aIndex, BYTE slot, BYTE flag); // OK
	//--
	void GDPetInventorySend(int aIndex);
	void GDPetInventorySaveSend(int aIndex);
	void DGPetInventoryRecv(SDHP_PET_INVENTORY_RECV* lpMsg);

	void CGItemSellRecv(PMSG_EQUIP_ITEM_SELL_RECV* lpMsg, int aIndex);
};

extern CNewUIMyInventory g_pNewMyInventory;
#endif