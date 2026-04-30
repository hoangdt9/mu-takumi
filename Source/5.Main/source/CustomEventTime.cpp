#include "stdafx.h"
#include "CustomEventTime.h"
#include "CustomMessage.h"
#include "CBInterface.h"
#include "Util.h"
#include "Protect.h"
#include "Other.h"
#include "NewUISystem.h"
#include "UIControls.h"

#include "wsclientinline.h"

CCustomEventTime::CCustomEventTime()
{
}

CCustomEventTime gCustomEventTime;

void CCustomEventTime::Init()
{
	this->Click = false;
	this->mNewDataEventTime.clear();
	this->MaxListData = 0;
}


void CCustomEventTime::Load(CUSTOM_EVENT_INFO* info) // OK
{
	for (int n = 0; n < MAX_EVENTTIME; n++)
	{
		this->SetInfo(info[n]);
	}
}

void CCustomEventTime::SetInfo(CUSTOM_EVENT_INFO info) // OK
{
	if (info.Index < 0 || info.Index >= MAX_EVENTTIME)
	{
		return;
	}

	this->m_CustomEventInfo[info.Index] = info;
}

void CCustomEventTime::ClearCustomEventTime() // OK
{
	for (int n = 0; n < MAX_EVENTTIME; n++)
	{
		gCustomEventTime[n].time = -1;
	}
	this->count = 0;
	this->EventTimeEnable = 0;
}
void CCustomEventTime::OpenTestWindow(int Page)
{
	gInterface.Data[eWindowEventTime].OnShow = true; pSetCursorFocus = true;

	PMSG_CUSTOM_EVENTTIME_SEND pMsg;

	pMsg.header.set(0xF3, 0xE8, sizeof(pMsg));

	pMsg.Page = Page;

	DataSend((BYTE*)& pMsg, pMsg.header.size);
};
void CCustomEventTime::GCReqEventTime(PMSG_CUSTOM_EVENTTIME_RECV* lpMsg) // OK
{
	this->count = lpMsg->count;
	this->mNewDataEventTime.clear();
	this->MaxListData = lpMsg->MaxList;
	for (int n = 0; n < lpMsg->count; n++)
	{
		CUSTOM_EVENTTIME_DATA* lpInfo = (CUSTOM_EVENTTIME_DATA*)(((BYTE*)lpMsg) + sizeof(PMSG_CUSTOM_EVENTTIME_RECV) + (sizeof(CUSTOM_EVENTTIME_DATA) * n));

		//this->gCustomEventTime[n].index = lpInfo->index;
		//this->gCustomEventTime[n].time = lpInfo->time;
		//g_Console.AddMessage(1, "Debug %d (%d) %s Max %d", lpInfo->index, lpInfo->time, lpInfo->NameEvent, this->MaxListData);
		this->mNewDataEventTime.push_back(*lpInfo);
		//if (this->gCustomEventTime[n].index >= 28 && this->gCustomEventTime[n].time != -1) this->Arena = 1;
	}

	this->EventTimeEnable = 1;
}

void CCustomEventTime::DrawEventTimePanelWindow()
{

	if (!gInterface.Data[eWindowEventTime].OnShow)
	{
		return;
	}


	float MainWidth = 420;
	float MainHeight = 390;
	float StartY = 10.0;
	float StartX = (MAX_WIN_WIDTH / 2) - (MainWidth / 2);
	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, MainWidth - 100, MainHeight, eWindowEventTime, gOther.Text_Chung[0]);
	int RowCol = (MainWidth / 4);
	float MainCenter = StartX + RowCol;
	float ButtonX = MainCenter - (float)(29.0 / 2);
	float StartBody = StartY;
	DWORD Color = 0xFFFFFFB8;

	TextDraw((HFONT)g_hFont, StartX- 50, StartY + (MainHeight - 28), 0x7DF4FFFF, 0x0, MainWidth, 0, 3, gOther.Text_Chung[21], this->Page+1, (this->MaxListData / 20)+1);

	if (this->Page > 0)
	{
		if (g_pBCustomMenuInfo->DrawButtonGUI(CNewUIInGameShop::IMAGE_IGS_STORAGE_PAGE_LEFT, StartX + 110, StartY + (MainHeight - 35), 20, 23, 3))
		{
			this->Page--;
			this->OpenTestWindow(this->Page);
		}
	}
	if ((this->MaxListData / 14) > this->Page)
	{
		if (g_pBCustomMenuInfo->DrawButtonGUI(CNewUIInGameShop::IMAGE_IGS_STORAGE_PAGE_RIGHT, StartX + (MainWidth - 225), StartY + (MainHeight - 35), 20, 23, 3))
		{
			this->Page++;
			this->OpenTestWindow(this->Page);
		}
	}
	if (this->EventTimeEnable == 1)
	{
		g_pBCustomMenuInfo->DrawInfoBox(StartX + 20, StartBody + 37, MainWidth - 140, 10, 0x00000096, 0); //1vs3
		TextDraw(g_hFontBold, StartX+10, StartBody + 39, 0xFFFFFFA8, 0x0, RowCol, 0, 3, gOther.Text_Chung[1]);
		TextDraw(g_hFontBold, StartX + 10 + (RowCol*1), StartBody + 39, 0xFFFFFFA8, 0x0, RowCol, 0, 3,"Vị Trí");
		TextDraw(g_hFontBold, StartX + 10 + (RowCol*2), StartBody + 39, 0xFFFFFFA8, 0x0, RowCol, 0, 3, gOther.Text_Chung[2]);

		if ((GetTickCount() - this->EventTimeTickCount) > 1000)
		{
			for (int i = 0; i < this->count; i++)
			{
				if (this->mNewDataEventTime[i].time > 0)
				{
					this->mNewDataEventTime[i].time = this->mNewDataEventTime[i].time - 1;
				}
			}
			this->EventTimeTickCount = GetTickCount();
		}

		char text1[20];
		char text2[30];
		int totalseconds;
		int hours;
		int minutes;
		int seconds;
		int days;

		int line = 0;

		for (int i = 0; i < this->mNewDataEventTime.size(); i++)
		{
			if (this->mNewDataEventTime[i].time <= -1)
			{
				wsprintf(text2, "Disabled");
				//continue;
			}
			else if (this->mNewDataEventTime[i].time == 0)
			{
				wsprintf(text2, "Online");
			}
			else
			{
				totalseconds = this->mNewDataEventTime[i].time;
				hours = totalseconds / 3600;
				minutes = (totalseconds / 60) % 60;
				seconds = totalseconds % 60;
				wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);

				if (hours > 23)
				{
					days = hours / 24;
					wsprintf(text2, "%d day(s)+", days);
				}
				else
				{
					wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
				}
			}

			if (this->mNewDataEventTime[i].time <= -1)
			{
				Color = 0xFFB145B8;
			}
			else if (this->mNewDataEventTime[i].time == 0)
			{
				Color = 0x45FF7AB8;
			}
			else if (this->mNewDataEventTime[i].time < 300)
			{
				Color = 0xA8FF45B8;
			}
			else
			{
				Color = 0xFFFFFFB8;
			}
			TextDraw(g_hFontBold, StartX + 10, StartBody + 58 + (line), 0xFFFF478A8, 0x0, RowCol, 0, 3, this->mNewDataEventTime[i].NameEvent);
			TextDraw(g_hFontBold, StartX + 10 + (RowCol*1), StartBody + 58 + (line), 0x61FFD0A8, 0x0, RowCol, 0, 3, this->mNewDataEventTime[i].DesString);
			TextDraw(g_hFontBold, StartX + 10 + (RowCol*2), StartBody + 58 + (line), Color, 0x0, RowCol, 0, 3, text2);

			if (this->mNewDataEventTime[i].NumberGate != -1)
			{
				//TextDraw(g_hFontBold, StartX + (RowCol * 3), StartBody + 58 + (line), Color, 0x0, RowCol, 0, 3, "%d", this->mNewDataEventTime[i].NumberGate);
				//if (g_pBCustomMenuInfo->DrawButtonGUI(gInterface.Data[eMenu].ModelID, StartX+30 + (RowCol * 3), StartBody + 58 + (line), gInterface.Data[eMenu].Width, gInterface.Data[eMenu].Height))
				//{
				//	//XULY_CGPACKET pMsg;
				//	//pMsg.header.set(0xD3, 0x01, sizeof(pMsg));
				//	//pMsg.ThaoTac = this->mNewDataEventTime[i].NumberGate;
				//	//DataSend((LPBYTE)& pMsg, pMsg.header.size);
				//}

				//if (SEASON3B::CheckMouseIn(StartX + 30 + (RowCol * 3), StartBody + 58 + (line), gInterface.Data[eMenu].Width, gInterface.Data[eMenu].Height) == 1)
				//{
				//	TextDraw((HFONT)g_hFont, StartX + 10, StartBody + 58 + (line), 0xFFFFFFFF, 0x00FBFF69, 335, 0, 3, " "); // Màu
				//
				//	//RenderTipText(StartX + 30 +30 + (RowCol * 3), StartBody + 58 + (line), "Di Chuyên Đến Event");
				//}


				// 
				//int BODY_X1 = GetPrivateProfileIntA("TEST", "BODY_X1", 0, "./config.ini");
				//int BODY_Y1 = GetPrivateProfileIntA("TEST", "BODY_Y1", 0, "./config.ini");
				//int BODY_W1 = GetPrivateProfileIntA("TEST", "BODY_W1", 0, "./config.ini");
				//int BODY_H1 = GetPrivateProfileIntA("TEST", "BODY_H1", 0, "./config.ini");
				if (SEASON3B::CheckMouseIn(StartX + 10, StartBody + 57 + (line), 300, 13))
				{
					TextDraw((HFONT)g_hFont, StartX + 10, StartBody + 57 + (line), 0xFFFFFFFF, 0x00FBFF69, 300, 13, 1, " "); // Màu

					if (SEASON3B::IsRelease(VK_LBUTTON))
					{
						XULY_CGPACKET pMsg;
						pMsg.header.set(0xD3, 0x01, sizeof(pMsg));
						pMsg.ThaoTac = this->mNewDataEventTime[i].NumberGate;
						DataSend((LPBYTE)&pMsg, pMsg.header.size);
						gInterface.Data[eWindowEventTime].OnShow = false;
					}
				}
			}
			line += 15;
		}
	}
	else
	{
		if (this->EventTimeLoad == 1)
		{
			gInterface.DrawFormat(eGold, MainCenter + 20, StartBody + 100, 52, 1, "Loading ..");
			this->EventTimeLoad = 2;
		}
		else if (this->EventTimeLoad == 2)
		{
			gInterface.DrawFormat(eGold, MainCenter + 20, StartBody + 100, 52, 1, "Loading ...");
			this->EventTimeLoad = 3;
		}
		else if (this->EventTimeLoad == 3)
		{
			gInterface.DrawFormat(eGold, MainCenter + 20, StartBody + 100, 52, 1, "Loading ....");
			this->EventTimeLoad = 4;
		}
		else if (this->EventTimeLoad == 4)
		{
			gInterface.DrawFormat(eGold, MainCenter + 20, StartBody + 100, 52, 1, "Loading .....");
			this->EventTimeLoad = 0;
		}
		else
		{
			gInterface.DrawFormat(eGold, MainCenter + 20, StartBody + 100, 52, 1, "Loading .");
			this->EventTimeLoad = 1;
		}
	}

}
