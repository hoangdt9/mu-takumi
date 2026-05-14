//////////////////////////////////////////////////////////////////////
// ServerListManager.cpp: implementation of the CServerListManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerListManager.h"
#include "./Utilities/Log/ErrorReport.h"

CServerListManager::CServerListManager()
{
	m_iTotalServer = 0;
	m_szSelectServerName[0] = '\0';
	m_iSelectServerIndex = -1;
}

CServerListManager::~CServerListManager()
{
	Release();
}

CServerListManager* CServerListManager::GetInstance()
{
	static CServerListManager s_ServerListManager;
	return &s_ServerListManager;
}

void CServerListManager::Release()
{	
	type_mapServerGroup::iterator iterServerGroup = m_mapServerGroup.begin();
	for( ; iterServerGroup!=m_mapServerGroup.end() ; iterServerGroup++ )
	{
		delete iterServerGroup->second;
	}

	m_mapServerGroup.clear();
	m_iTotalServer = 0;
	m_bTestServer = false;
}

void CServerListManager::BuxConvert(BYTE* pbyBuffer, int nSize)
{
	BYTE abyBuxCode[3] = { 0xfc, 0xcf, 0xab };
	for (int i = 0; i < nSize; ++i)
		pbyBuffer[i] ^= abyBuxCode[i%3];
}

void CServerListManager::LoadServerListScript()
{
	FILE* fp = ::fopen("Data\\Local\\ServerList.bmd", "rb");

	if (fp == NULL)
	{
		char szMessage[256];
		::sprintf(szMessage, "Data\\Local\\ServerList.bmd file not found.\r\n");
		g_ErrorReport.Write(szMessage);
		::MessageBox(g_hWnd, szMessage, NULL, MB_OK);
		::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
		return;
	}
	
#pragma pack(push, 1)
typedef struct _SERVER_GROUP_INFO
{
	WORD	m_wIndex;
	char	m_szName[SLM_MAX_SERVER_NAME_LENGTH];
	BYTE	m_byPos;
	BYTE	m_bySequence;
	BYTE	m_abyNonPVP[SLM_MAX_SERVER_COUNT];
	short	m_nDescriptLen;
} SERVER_GROUP_INFO;
#pragma pack(pop)
	
	int nSize = sizeof(SERVER_GROUP_INFO);
	SERVER_GROUP_INFO sServerGroupScript;
	char szDescript[1024];
	SServerGroupInfo sServerGroupInfo;
	int i;

	while (0 != ::fread(&sServerGroupScript, nSize, 1, fp))
	{
		BuxConvert((BYTE*)&sServerGroupScript, nSize);
		::fread(szDescript, sServerGroupScript.m_nDescriptLen, 1, fp);
		BuxConvert((BYTE*)szDescript, sServerGroupScript.m_nDescriptLen);
		::strncpy(sServerGroupInfo.m_szName, sServerGroupScript.m_szName,SLM_MAX_SERVER_NAME_LENGTH);
		sServerGroupInfo.m_byPos = sServerGroupScript.m_byPos;
		sServerGroupInfo.m_bySequence = sServerGroupScript.m_bySequence;
		for (i = 0; i < SLM_MAX_SERVER_COUNT; ++i)
			sServerGroupInfo.m_abyNonPVP[i] = sServerGroupScript.m_abyNonPVP[i];
		sServerGroupInfo.m_strDescript = szDescript;
		
		m_mapServerListScript.insert(std::make_pair(sServerGroupScript.m_wIndex, sServerGroupInfo));
	}
	
	::fclose(fp);
}

const SServerGroupInfo* CServerListManager::GetServerGroupInfoInScript(WORD wServerGroupIndex)
{
	ServerListScriptMap::const_iterator iter = m_mapServerListScript.find(wServerGroupIndex);
	if (iter == m_mapServerListScript.end())
		return NULL;

	return &(iter->second);
}

void CServerListManager::InsertServerGroup(int iConnectIndex, int iServerPercent)
{
	CServerGroup* pServerGroup = NULL;

	type_mapServerGroup::iterator iterServerGroup = m_mapServerGroup.begin();

	bool bEqual = false;
	while( iterServerGroup != m_mapServerGroup.end() )
	{
		if( (iterServerGroup->second)->m_iServerIndex == iConnectIndex/20 )
		{
			bEqual = true;
			break;
		}

		iterServerGroup++;
	}

	if( bEqual == true )
	{
		pServerGroup = iterServerGroup->second;
	}
	else
	{
		pServerGroup = new CServerGroup;

		if (MakeServerGroup(iConnectIndex / 20, pServerGroup) == false)
		{
			delete pServerGroup;
			return;
		}

		m_mapServerGroup.insert(type_mapServerGroup::value_type(pServerGroup->m_iSequence, pServerGroup));
	}

	InsertServer(pServerGroup, iConnectIndex, iServerPercent);
	
	m_iterServerGroup = m_mapServerGroup.begin();
}

bool CServerListManager::MakeServerGroup(IN int iServerGroupIndex, OUT CServerGroup* pServerGroup)
{
	const SServerGroupInfo* pServerGroupInfo = GetServerGroupInfoInScript((WORD)iServerGroupIndex);
	if (NULL == pServerGroupInfo)
	{
		// Season20 (and other) ServerList.bmd often omit low group indices while LegacyLoginHost
		// sends the default F4 06 preset (groups 0,1,2 from connect ids 0..14, 20..34, 40..41).
		// Without a script row, InsertServerGroup dropped every line — empty server list on login UI.
		char szBuf[SLM_MAX_SERVER_NAME_LENGTH];
		snprintf(szBuf, sizeof(szBuf), "World %d", iServerGroupIndex);
		::strcpy(pServerGroup->m_szName, szBuf);
		pServerGroup->m_szDescription[0] = '\0';
		pServerGroup->m_iSequence = iServerGroupIndex;
		pServerGroup->m_iWidthPos = CServerGroup::SBP_LEFT;
		pServerGroup->m_iServerIndex = iServerGroupIndex;
		pServerGroup->m_bPvPServer = true;
		int j;
		for (j = 0; j < SLM_MAX_SERVER_COUNT; ++j)
		{
			pServerGroup->m_abyNonPvpServer[j] = 0;
		}
		for (; j < MAX_SERVER_LOW; ++j)
		{
			pServerGroup->m_abyNonPvpServer[j] = 0;
		}
		g_ErrorReport.Write(
			"[ServerList] ServerList.bmd missing group %d — synthesized group for F4 06 (LAN / default ids).\r\n",
			iServerGroupIndex);
		return true;
	}

	::strcpy(pServerGroup->m_szName, pServerGroupInfo->m_szName);
	::strcpy(pServerGroup->m_szDescription, pServerGroupInfo->m_strDescript.c_str());
	pServerGroup->m_iSequence = (int)pServerGroupInfo->m_bySequence;
	pServerGroup->m_iWidthPos = (int)pServerGroupInfo->m_byPos;
	pServerGroup->m_iServerIndex = iServerGroupIndex;
	pServerGroup->m_bPvPServer = true;
	int i;
	for (i = 0; i < SLM_MAX_SERVER_COUNT; ++i)
	{
		pServerGroup->m_abyNonPvpServer[i] = pServerGroupInfo->m_abyNonPVP[i];
		if (0x01 & pServerGroup->m_abyNonPvpServer[i])
			pServerGroup->m_bPvPServer = false;
	}
	for (; i < MAX_SERVER_LOW; ++i)
		pServerGroup->m_abyNonPvpServer[i] = 0;

	return true;
}

void CServerListManager::InsertServer(CServerGroup* pServerGroup, int iConnectIndex, int iServerPercent)
{
	CServerInfo* pServerInfo = new CServerInfo;
	pServerInfo->m_iSequence = pServerGroup->GetServerSize();

	int mod = iConnectIndex % 20;
	if (mod < 0)
	{
		mod += 20;
	}

	pServerInfo->m_iIndex = mod + 1;
	if (pServerInfo->m_iIndex < 1)
	{
		pServerInfo->m_iIndex = 1;
	}
	else if (pServerInfo->m_iIndex > MAX_SERVER_LOW)
	{
		pServerInfo->m_iIndex = MAX_SERVER_LOW;
	}

	int idx = pServerInfo->m_iIndex - 1;
	if (idx < 0)
	{
		idx = 0;
	}
	else if (idx >= MAX_SERVER_LOW)
	{
		idx = MAX_SERVER_LOW - 1;
	}

	pServerInfo->m_iConnectIndex = iConnectIndex;
	pServerInfo->m_iPercent = iServerPercent;
	pServerInfo->m_byNonPvP = pServerGroup->m_abyNonPvpServer[idx];

	int iTextIndex;
	if (iServerPercent >= 128)
	{
		iTextIndex = 560;
	}
	else if (iServerPercent >= 100)
	{
		iTextIndex = 561;
	}
	else
	{
		iTextIndex = 562;
	}

	const char* loadText = GlobalText[iTextIndex];
	if (loadText == NULL)
	{
		loadText = "";
	}

	const size_t nameCap = sizeof(pServerInfo->m_bName);

	switch (pServerInfo->m_byNonPvP)
	{
	case 0:
		snprintf(
			pServerInfo->m_bName,
			nameCap,
			"%s-%d %s",
			pServerGroup->m_szName,
			pServerInfo->m_iIndex,
			loadText);
		break;

	case 1:
		snprintf(
			pServerInfo->m_bName,
			nameCap,
			"%s-%d(Non-PVP) %s",
			pServerGroup->m_szName,
			pServerInfo->m_iIndex,
			loadText);
		break;

	case 2:
		snprintf(
			pServerInfo->m_bName,
			nameCap,
			"%s-%d(Gold PVP) %s",
			pServerGroup->m_szName,
			pServerInfo->m_iIndex,
			loadText);
		break;

	case 3:
		snprintf(
			pServerInfo->m_bName,
			nameCap,
			"%s-%d(Gold) %s",
			pServerGroup->m_szName,
			pServerInfo->m_iIndex,
			loadText);
		break;

	default:
		snprintf(
			pServerInfo->m_bName,
			nameCap,
			"%s-%d %s",
			pServerGroup->m_szName,
			pServerInfo->m_iIndex,
			loadText);
		break;
	}

	pServerGroup->InsertServerInfo(pServerInfo);
}

int CServerListManager::GetServerGroupSize()
{
	return m_mapServerGroup.size();
}

void CServerListManager::SetFirst()
{
	m_iterServerGroup = m_mapServerGroup.begin();
}

bool CServerListManager::GetNext(OUT CServerGroup* &pServerGroup)
{
	if(m_iterServerGroup == m_mapServerGroup.end())
	{
		pServerGroup = NULL;

		return false;
	}

	pServerGroup = m_iterServerGroup->second;
	
	m_iterServerGroup++;

	return true;
}

CServerGroup* CServerListManager::GetServerGroupByBtnPos(int iBtnPos)
{
	type_mapServerGroup::iterator iterServerGroup = m_mapServerGroup.begin();
	
	while(iterServerGroup != m_mapServerGroup.end())
	{
		if( iterServerGroup->second->m_iBtnPos == iBtnPos )
			return iterServerGroup->second;

		iterServerGroup++;
	}
	
	return NULL;
}

void CServerListManager::SetSelectServerInfo(unicode::t_char* pszName, int iIndex, int iCensorshipIndex, BYTE byNonPvP, bool bTestServer)
{
	strcpy(m_szSelectServerName, pszName);
	m_iSelectServerIndex = iIndex;
	m_iCensorshipIndex = iCensorshipIndex;
	m_byNonPvP = byNonPvP;
	m_bTestServer = bTestServer;
}

unicode::t_char* CServerListManager::GetSelectServerName()
{
	return m_szSelectServerName;
}

int CServerListManager::GetSelectServerIndex()
{
	return m_iSelectServerIndex;
}

int CServerListManager::GetCensorshipIndex()
{
	return m_iCensorshipIndex;
}

BYTE CServerListManager::GetNonPVPInfo()
{
	return m_byNonPvP;
}

bool CServerListManager::IsNonPvP()
{
	return bool(0x01 & GetNonPVPInfo());
}

bool CServerListManager::IsTestServer()
{
	return m_bTestServer;
}

void CServerListManager::SetTotalServer(int iTotalServer)
{
	m_iTotalServer = iTotalServer;
}

int CServerListManager::GetTotalServer()
{
	return m_iTotalServer;
}
