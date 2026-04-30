#include "StdAfx.h"
#include "CustomMonterPoint.h"
#include "user.h"
#include "Message.h"
#include "DSProtocol.h"
#include "Notice.h"
#include "ItemManager.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "Util.h"
#include "Log.h"
#include "ItemLevel.h"
#include "ItemStack.h"
#include "BCustomChoTroi.h"
#include "Map.h"
#include "Viewport.h"
#include "MapManager.h"
CustomMonterPoint gCustomMonterPoint;
CustomMonterPoint::CustomMonterPoint()
{
}


CustomMonterPoint::~CustomMonterPoint()
{
}

void CustomMonterPoint::Load(char* FilePath)
{

	
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}
	pugi::xml_node oCustomMonterPoint = file.child("CustomMonterPoint");
	this->m_Enable = oCustomMonterPoint.attribute("Enable").as_int();
	this->m_Notice = oCustomMonterPoint.attribute("Notice").as_int();


	this->m_Text.clear();
	this->m_Text = oCustomMonterPoint.attribute("Text").as_string();
	this->m_DataMonterPoint.clear();
	for (pugi::xml_node rMonter = oCustomMonterPoint.child("Monter"); rMonter; rMonter = rMonter.next_sibling())
	{
		DATA_MONTER_POINT info = { 0 };
		info.ClassMin= rMonter.attribute("ClassMin").as_int();
		info.ClassMax= rMonter.attribute("ClassMax").as_int();
		info.PointAdd= rMonter.attribute("PointAdd").as_int();
		this->m_DataMonterPoint.push_back(info);
	}

	LogAdd(LOG_BLUE, "[CustomMonterPoint] Data Size %d", this->m_DataMonterPoint.size());
}
void CustomMonterPoint::GDAddPointSaveSend(int aIndex, int AddPoint) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	SDHP_MONTERPOINT_SAVE_SEND pMsg;

	pMsg.header.set(0x41, sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account, lpObj->Account, sizeof(pMsg.account));

	memcpy(pMsg.name, lpObj->Name, sizeof(pMsg.name));

	pMsg.PointAdd = AddPoint;

	gDataServerConnection.DataSend((BYTE*)& pMsg, pMsg.header.size);
}
void CustomMonterPoint::MonsterDieProc(LPOBJ lpObj, LPOBJ lpTarget) // OK
{
	if (!this->m_Enable) return;
	for (std::vector<DATA_MONTER_POINT>::iterator it = this->m_DataMonterPoint.begin(); it != this->m_DataMonterPoint.end(); it++)
	{
		if (it == this->m_DataMonterPoint.end())
		{
			break;
		}

		if (lpObj->Class < it->ClassMin || lpObj->Class > it->ClassMax) continue;

		//===Add diem
		if (it->PointAdd > 0)
		{
			this->GDAddPointSaveSend(lpTarget->Index, it->PointAdd);
		}
		if (this->m_Notice)
		{
			gNotice.GCNoticeSend(lpTarget->Index, 1, 0, 0, 0, 0, 0, strdup(this->m_Text.c_str()), it->PointAdd);
		}
	}
}

