#include "stdafx.h"
#include "Util.h"
#include "QueryManager.h"
#include "SocketManager.h"
#include "ChoTroi.h"
#include "DataServerProtocol.h"

#if(CUSTOM_BCHOTROI)
CChoTroi g_ChoTroi;

CChoTroi::CChoTroi()
{
}

CChoTroi::~CChoTroi()
{
}

void CChoTroi::Init()
{
}

void CChoTroi::Load()
{
}

bool CChoTroi::Connect()
{
	this->CreateTable();

	return true;
}

void CChoTroi::CreateTable()
{
	gQueryManager.ExecQuery("CREATE TABLE [dbo].[ItemMarketData]( [ID] [int] IDENTITY(1,1) NOT NULL) ON [PRIMARY]");
	gQueryManager.Fetch();
	gQueryManager.Close();

	gQueryManager.ExecQuery("ALTER TABLE [ItemMarketData] ADD [Account] [varchar](10) NULL");
	gQueryManager.Fetch();
	gQueryManager.Close();

#if(MARKET_NAME_DEV)

	gQueryManager.ExecQuery("ALTER TABLE [ItemMarketData] ADD [Name][varchar](10) NULL");
	gQueryManager.Fetch();
	gQueryManager.Close();

#endif

	gQueryManager.ExecQuery("ALTER TABLE [ItemMarketData] ADD [PriceType] INT not null default(0)");
	gQueryManager.Fetch();
	gQueryManager.Close();

	gQueryManager.ExecQuery("ALTER TABLE [ItemMarketData] ADD [PriceValue] INT not null default(0)");
	gQueryManager.Fetch();
	gQueryManager.Close();

	gQueryManager.ExecQuery("ALTER TABLE [ItemMarketData] ADD [Status] INT not null default(0)");
	gQueryManager.Fetch();
	gQueryManager.Close();

#if(MARKET_FILTER_DEV)

	gQueryManager.ExecQuery("ALTER TABLE [ItemMarketData] ADD [FilterType] INT not null default(0)");
	gQueryManager.Fetch();
	gQueryManager.Close();

	gQueryManager.ExecQuery("ALTER TABLE [ItemMarketData] ADD [FilterLevel] INT not null default(0)");
	gQueryManager.Fetch();
	gQueryManager.Close();

	gQueryManager.ExecQuery("ALTER TABLE [ItemMarketData] ADD [FilterLuck] INT not null default(0)");
	gQueryManager.Fetch();
	gQueryManager.Close();

	gQueryManager.ExecQuery("ALTER TABLE [ItemMarketData] ADD [FilterExl] INT not null default(0)");
	gQueryManager.Fetch();
	gQueryManager.Close();

	gQueryManager.ExecQuery("ALTER TABLE [ItemMarketData] ADD [FilterAnc] INT not null default(0)");
	gQueryManager.Fetch();
	gQueryManager.Close();

#endif

	gQueryManager.ExecQuery("ALTER TABLE [ItemMarketData] ADD [Date] [varchar](20) NULL");
	gQueryManager.Fetch();
	gQueryManager.Close();

	gQueryManager.ExecQuery("ALTER TABLE [ItemMarketData] ADD [Item] [varbinary](16) NULL");
	gQueryManager.Fetch();
	gQueryManager.Close();
}

void CChoTroi::Protocol(BYTE protoNum, BYTE* aRecv, int uIndex)
{
	switch (protoNum)
	{
	case 0x00:
		this->GDReqItemListPage((SDHP_REQ_MARKET_ITEM*)aRecv, uIndex);
		break;
	case 0x01:
		this->GDReqItemSell((SDHP_REQ_MARKET_SELL*)aRecv, uIndex);
		break;
	case 0x02:
		this->GDReqBuyItem((SDHP_REQ_MARKET_BUY*)aRecv, uIndex);
		break;
#if(MARKET_FILTER_DEV)
	case 0x03:
		this->GDReqItemStatus((SDHP_REQ_MARKET_STATUS*)aRecv, uIndex);
		break;
#endif
	}
}

void CChoTroi::GDReqItemList(SDHP_REQ_MARKET_ITEM* lpMsg, int uIndex)
{
#if(MARKET_PRICE_DEV)

	this->GDReqItemListPage(lpMsg, uIndex);

#else

	char szQuery[256];

	int iReturnCode = 0;

	int iCount = 0;

	SDHP_ANS_MARKET_ITEM pMsg;

	memset(&pMsg, 0, sizeof(pMsg));

	pMsg.h.set((LPBYTE)& pMsg, 0xFE, 0x00, sizeof(pMsg));

	pMsg.Result = 1;

	pMsg.aIndex = lpMsg->aIndex;

	// --

	char szPriceType[256];

	memset(szPriceType, 0, sizeof(szPriceType));

	if (lpMsg->PriceType == eMarketPriceTypeZen)
	{
		sprintf(szPriceType, "AND PriceType = %d ", lpMsg->PriceType);
	}

	if (lpMsg->PriceType == eMarketPriceTypeWcoin)
	{
		sprintf(szPriceType, "AND PriceType = %d ", lpMsg->PriceType);
	}

	if (lpMsg->PriceType == eMarketPriceTypeCredit)
	{
		sprintf(szPriceType, "AND PriceType = %d ", lpMsg->PriceType);
	}

	// --

#if(MARKET_FILTER_DEV)

	char szFilter[256];

	memset(szFilter, 0, sizeof(szFilter));

	if (lpMsg->Filter.FilterType < 16)
	{
		sprintf(szFilter, "AND FilterType = %d ", lpMsg->Filter.FilterType);
	}

	if (lpMsg->Filter.FilterLevel > 0 && lpMsg->Filter.FilterLevel <= 15)	//=(
	{
		char szLevel[256];

		memset(szLevel, 0, sizeof(szLevel));

		sprintf(szLevel, "AND FilterLevel >= %d AND FilterLevel <= 15 ", lpMsg->Filter.FilterLevel);

		strcat(szFilter, szLevel);
	}

	if (lpMsg->Filter.FilterLuck)
	{
		strcat(szFilter, "AND FilterLuck = 1 ");
	}

	if (lpMsg->Filter.FilterExl)
	{
		strcat(szFilter, "AND FilterExl = 1 ");
	}

	if (lpMsg->Filter.FilterAnc)
	{
		strcat(szFilter, "AND FilterAnc = 1 ");
	}

	sprintf(szQuery, "SELECT TOP %d ID, Account, Name, PriceType, PriceValue FROM ItemMarketData WHERE Status = 0 %s %s ORDER BY ID DESC", MARKET_ITEM_MAX, szFilter, szPriceType);

#else

	sprintf(szQuery, "SELECT TOP %d ID, Account, PriceType, PriceValue FROM ItemMarketData WHERE Status = 0 ORDER BY ID DESC", MARKET_ITEM_MAX); // <<== Arg

#endif

	if (!gQueryManager.Exec(szQuery))
	{
		gQueryManager.Close();

		pMsg.Result = 0;
	}
	else
	{
		short sqlReturn = gQueryManager.Fetch();

		while (sqlReturn != SQL_NO_DATA && sqlReturn != SQL_NULL_DATA)
		{
			iReturnCode = gQueryManager.GetInt(1);

			if (iReturnCode < 0)
			{
				break;
			}

			char szSeller[MARKET_NAME_LEN];
			gQueryManager.GetStr("Name", szSeller);

			pMsg.Data[iCount].Name[MARKET_NAME_LEN - 1] = 0;

			memcpy(pMsg.Data[iCount].Name, szSeller, MARKET_NAME_LEN - 1);

			pMsg.Data[iCount].PriceType = gQueryManager.GetInt("PriceType");

			pMsg.Data[iCount].Price = gQueryManager.GetInt("PriceValue");

			// --

			int id = gQueryManager.GetInt("ID");

			pMsg.Data[iCount].ID = id;

			sprintf(szQuery, "SELECT Item FROM ItemMarketData WHERE ID = %d", id);

			int	nRet = this->m_DBItem.ReadBlob(szQuery, pMsg.Data[iCount].Item);

			if (nRet < 0)
			{
				memset(pMsg.Data[iCount].Item, -1, sizeof(pMsg.Data[iCount].Item));
			}

			// --

			iCount++;

			if (iCount >= MARKET_ITEM_MAX)
			{
				break;
			}

			sqlReturn = gQueryManager.Fetch();
		}

		gQueryManager.Close();
	}

	wsjServer.DataSend(uIndex, (char*)& pMsg, sizeof(pMsg));

#endif
}

void CChoTroi::GDReqItemSell(SDHP_REQ_MARKET_SELL* lpMsg, int uIndex)
{
	char szQuery[512];

	char	szAccountID[11] = { 0 };
	strncpy(szAccountID, lpMsg->Account, 10);

	char szName[11];
	memset(szName, 0, sizeof(szName));
	memcpy(szName, lpMsg->Name, sizeof(szName) - 1);

	int TimeHSD = (int)(time(0) + ((lpMsg->ItemDay * 86400))); //HSD

	//LogAdd(LOG_RED, "GDReqItemSell Day %d Time %d Pass %d", lpMsg->ItemDay, TimeHSD,lpMsg->Pass);
	sprintf(szQuery,
		"INSERT INTO ItemMarketData (Account, PriceType, PriceValue, Date, TypeItem, Name, Time,Pass) VALUES ('%s', %d, %d, '%s', %d, '%s', %d, %d)",
		szAccountID,
		lpMsg->PriceType,
		lpMsg->Price,
		"19.12.2020",
		lpMsg->TypeItem,
		szName,
		TimeHSD,
		lpMsg->Pass
	);


	//pMsg.Result = gQueryManager.ExecQuery(szQuery);
	gQueryManager.ExecQuery(szQuery);

	gQueryManager.Fetch();

	gQueryManager.Close();

	// --

	sprintf(szQuery, "SELECT TOP 1 ID FROM ItemMarketData ORDER BY ID DESC");	//need recode

	if (!gQueryManager.ExecQuery(szQuery) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		//pMsg.Result = 0;
		LogAdd(LOG_RED, "[ChoTroi] Loi Sell Item len cho !!");
		gQueryManager.Close();
		return;
	}

	int id = gQueryManager.GetAsInteger("ID");

	gQueryManager.Close();

	// --

	sprintf(szQuery, "UPDATE ItemMarketData SET Item = ? WHERE ID = %d", id);

	gQueryManager.BindParameterAsBinary(1, lpMsg->ItemData, sizeof(lpMsg->ItemData));

	gQueryManager.ExecQuery(szQuery, id);

	gQueryManager.Close();


	// --

	//gSocketManager.DataSend(uIndex, (BYTE*)&pMsg, sizeof(pMsg));
}

void CChoTroi::GDReqBuyItem(SDHP_REQ_MARKET_BUY* lpMsg, int uIndex)
{
	char szQuery[256];

	int iResult = 0;


	sprintf(szQuery, "Select Status from ItemMarketData WHERE ID = %d And Status = 0", lpMsg->ID);

	if (!gQueryManager.ExecQuery(szQuery) || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		iResult = 0;
		gQueryManager.Close();
		goto Exit;
	}

	gQueryManager.Close();

	if (lpMsg->Result == 1)
	{
		sprintf(szQuery, "UPDATE ItemMarketData SET Status = 1 WHERE ID = %d", lpMsg->ID);

		iResult = gQueryManager.ExecQuery(szQuery);

		gQueryManager.Close();
	}
	if (lpMsg->Result == 2)
	{
		sprintf(szQuery, "Delete FROM ItemMarketData WHERE ID = %d", lpMsg->ID);

		iResult = gQueryManager.ExecQuery(szQuery);

		gQueryManager.Close();
	}
Exit:
	//LogAdd(LOG_RED, "GDReqBuyItem %d", iResult);
	SDHP_ANS_MARKET_BUY pMsg;

	memset(&pMsg, 0, sizeof(pMsg));

	pMsg.h.set((LPBYTE)& pMsg, 0xFE, 0x02, sizeof(pMsg));

	pMsg.Result = iResult == 0 ? 3 : lpMsg->Result;

	pMsg.aIndex = lpMsg->aIndex;

	pMsg.ID = lpMsg->ID;

	gSocketManager.DataSend(uIndex, (BYTE*)& pMsg, sizeof(pMsg));
}

void DeleteItemCoin(int ID)
{
	gQueryManager.ExecQuery("Delete FROM ItemMarketData WHERE ID = %d", ID);
	gQueryManager.Fetch();
	gQueryManager.Close();
}
void CChoTroi::GDReqItemStatus(SDHP_REQ_MARKET_STATUS* lpMsg, int uIndex) //Get va Update List Item Da Ban
{

	BYTE send[4096];

	CBCUSTOM_LOAD_COUNT pMsg;

	pMsg.header.set(0xFE, 0x03, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	pMsg.aIndex = lpMsg->aIndex;

	char	szAccountID[11] = { 0 };
	strncpy(szAccountID, lpMsg->Account, 10);

	SDHP_ANS_MARKET_STATUS info;

	if (gQueryManager.ExecQuery("SELECT * FROM ItemMarketData WHERE Account = '%s' AND Status = 1", szAccountID) != 0)
	{
		while (gQueryManager.Fetch() != SQL_NO_DATA)
		{

			info.TypeItem = gQueryManager.GetAsInteger("TypeItem");

			info.PriceType = gQueryManager.GetAsInteger("PriceType");

			info.PriceValue = gQueryManager.GetAsInteger("PriceValue");

			gQueryManager.GetAsString("Account", info.Account, sizeof(info.Account));

			memset(info.ItemData, 0xFF, sizeof(info.ItemData));
			gQueryManager.GetAsBinary("Item", info.ItemData, sizeof(info.ItemData));

			memcpy(&send[size], &info, sizeof(info));
			size += sizeof(info);

			pMsg.count++;

		}
		gQueryManager.Close();
		gQueryManager.ExecQuery("Delete FROM ItemMarketData WHERE Account = '%s' AND Status = 1", szAccountID);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		return;
	}



	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(uIndex, send, size);

}




void CChoTroi::GDReqItemListPage(SDHP_REQ_MARKET_ITEM* lpMsg, int uIndex)
{
	char szQuery[256];

	int iPageNumber = lpMsg->PageNumber;
	// --

	char szPriceType[256];
	int DayNow = (int)time(0);
	//LogAdd(LOG_RED, "GDReqItemListPage TypeHSD %d ", lpMsg->TypeHSD);
	if (lpMsg->TypeHSD == 1) //Delete item het hajn
	{

		gQueryManager.ExecQuery("Delete ItemMarketData WHERE Status = 0 And Time < %d", DayNow);
		gQueryManager.Close();
	}
	else if (lpMsg->TypeHSD == 0)
	{
		DayNow = 0;
	}
	memset(szPriceType, 0, sizeof(szPriceType));

	if (lpMsg->PriceType == eMarketPriceWC)
	{
		sprintf(szPriceType, "AND PriceType = '%d' ", lpMsg->PriceType);
	}

	if (lpMsg->PriceType == eMarketPriceWP)
	{
		sprintf(szPriceType, "AND PriceType = '%d' ", lpMsg->PriceType);
	}

	if (lpMsg->PriceType == eMarketPriceGP)
	{
		sprintf(szPriceType, "AND PriceType = '%d' ", lpMsg->PriceType);
	}

	if (lpMsg->PriceType == eMarketPriceB)
	{
		sprintf(szPriceType, "AND PriceType = '%d' ", lpMsg->PriceType);
	}

	if (lpMsg->PriceType == eMarketPriceS)
	{
		sprintf(szPriceType, "AND PriceType = '%d' ", lpMsg->PriceType);
	}

	if (lpMsg->PriceType == eMarketPriceC)
	{
		sprintf(szPriceType, "AND PriceType = '%d' ", lpMsg->PriceType);
	}

	// --


	char szFilter[256];
	char	szAccountID[11] = { 0 };
	strncpy(szAccountID, lpMsg->Account, 10);
	memset(szFilter, 0, sizeof(szFilter));

	// TypeItem = 1 Vũ Khí, Khiên
	// 	TypeItem = 2 Mũ
	// 	TypeItem = 3 Áo
	// 	TypeItem = 4 Quần
	// 	TypeItem = 5 Tay
	// 	TypeItem = 6 Chân
	// 	TypeItem = 7 Cánh
	// 	TypeItem = 8 Pet, sói, thú cưỡi
	// 	TypeItem = 9 Dây Chuyền
	// 	TypeItem = 10 Nhẫn
	if (lpMsg->GetTypeItem > 0)
	{
		if (lpMsg->GetTypeItem != 12)
		{
			int TypeItem = lpMsg->GetTypeItem;
			if (lpMsg->GetTypeItem == 11) TypeItem = 255;
			sprintf(szFilter, "AND TypeItem = '%d' ", TypeItem);
		}
		else
		{
			sprintf(szFilter, "AND Account = '%s' ", szAccountID);
		}
	}

	if (lpMsg->GetTypeItem == 12)
	{
		sprintf(szQuery, "SELECT TOP 100 ID, Account, Name, PriceType, PriceValue, Item,TypeItem,Time,Pass FROM ItemMarketData WHERE Status = 0  %s %s ORDER BY ID DESC", szFilter, szPriceType);
	}
	else
	{
		sprintf(szQuery, "SELECT TOP 100 ID, Account, Name, PriceType, PriceValue, Item,TypeItem,Time,Pass FROM ItemMarketData WHERE Status = 0 And Time > %d  %s %s ORDER BY ID DESC", DayNow, szFilter, szPriceType);
	}

	BYTE send[8192];

	CBCUSTOM_LOAD_COUNT pMsg;

	pMsg.header.set(0xFE, 0x00, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	pMsg.aIndex = lpMsg->aIndex;


	if (gQueryManager.ExecQuery(szQuery) != 0)
	{
		short i = gQueryManager.Fetch();

		while (i != SQL_NO_DATA && i != SQL_NULL_DATA)
		{
			int iReturnCode = gQueryManager.GetResult(0);

			if (iReturnCode < 0)
			{
				break;
			}
			MARKET_DATA info = { 0 };
			info.PriceType = gQueryManager.GetAsInteger("PriceType");
			info.Price = gQueryManager.GetAsInteger("PriceValue");
			info.ID = gQueryManager.GetAsInteger("ID");
			info.TypeItem = gQueryManager.GetAsInteger("TypeItem");
			info.TimeItemRaoBan = gQueryManager.GetAsInteger("Time") - (int)time(0);
			info.Pass = gQueryManager.GetAsInteger("Pass");

			memset(info.Item, 0xFF, sizeof(info.Item));
			gQueryManager.GetAsBinary("Item", info.Item, sizeof(info.Item));
			gQueryManager.GetAsString("Name", info.Name, sizeof(info.Name));

			//			LogAdd(LOG_RED, "GDReqItemListPage %d  %s (%d) %d", info.ID, info.Name, info.TimeItemRaoBan,info.Pass);

			if ((size + sizeof(info)) >= sizeof(send))
			{
				break;
			}
			// --
			memcpy(&send[size], &info, sizeof(info));
			size += sizeof(info);
			pMsg.count++;

			i = gQueryManager.Fetch();
		}
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		return;
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(uIndex, send, size);
	//LogAdd(LOG_RED, "GDReqItemListPage %d", pMsg.count);
}
#endif