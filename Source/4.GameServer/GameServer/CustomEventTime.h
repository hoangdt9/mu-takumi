
#include "Protocol.h"
#define MAX_EVENTTIME  42


struct CUSTOM_EVENTTIME_DATA
{
	int index;
	char NameEvent[30];
	char DesString[90];
	int NumberGate;
	int time;
};

//**********************************************//
//********** GameServer -> Cliente    **********//
//**********************************************//

struct PMSG_CUSTOM_EVENTTIME_SEND
{
	PSWMSG_HEAD header;
	int MaxList;
	int count;
};

//**********************************************//
//********** Cliente -> GameServer    **********//
//**********************************************//

struct PMSG_CUSTOM_EVENTTIME_RECV
{
	PSBMSG_HEAD header; // C1:BF:51
	BYTE Page;
};

struct BCUSTOM_EVENTTIME_DATA
{
	char NameEvent[60];
	int* TimeEvent;
	void Clear() // OK
	{
		memset(NameEvent, 0, sizeof(NameEvent));
		TimeEvent = 0;
	}
};
struct DATA_CUSTOMEVENTTIME
{
	int NumberEvent;
	char NameEvent[30];
	char DesString[90];
	int NumberGate;
};
// ---
class CCustomEventTime
{
public:
	CCustomEventTime();
	virtual ~CCustomEventTime();
	void GCReqEventTime(int Index, PMSG_CUSTOM_EVENTTIME_RECV* pMsg);
	// ---
	void CCustomEventTime::LoadData();
	//int CountTime;
	//BCUSTOM_EVENTTIME_DATA DataEventTime[255];
	bool LoadDataTime;
	std::vector <BCUSTOM_EVENTTIME_DATA> DataEventTime;
	void CCustomEventTime::AddDataEventsTime(char* NameEvent, int* Time);

	void LoadFileXML(char* FilePath);
	std::vector <DATA_CUSTOMEVENTTIME> m_SendClientDataEventTime;
	int m_Enable;
private:
	int m_count;
	// ---
	//CUSTOM_EVENTTIME_DATA r_Data[MAX_EVENTTIME];


};
extern CCustomEventTime gCustomEventTime;
// ---
