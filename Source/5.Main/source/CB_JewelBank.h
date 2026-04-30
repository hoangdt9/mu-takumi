#include "CBInterface.h"
#include "Protocol.h"
class CBJewelBank
{
public:
	struct PMSG_JEWELSBANK
	{
		PSBMSG_HEAD h;
		int Chaos;
		int Bless;
		int Soul;
		int Life;
		int CreateonBank;
		int GemStoneBank;
		int GuardianBank;
		int HarmonyBank;
		int LowStoneBank;
		int HighStoneBank;
	};

	struct PMSG_JEWELBANK_SUB
	{
		PSBMSG_HEAD h;
		BYTE Type;
		BYTE Mode;
	};
	CBJewelBank();
	virtual ~CBJewelBank();
	void DrawWindow();
	void OpenOnOff();
	void RecvProtocol(BYTE* Recv);
	int Pagina;
	int ItemJewelry[15];
	int BankJewelry[15];
	bool SendItem(ITEM* ItemSell, int Slot, bool KeyClick);
};

extern CBJewelBank gCBJewelBank;