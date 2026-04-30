#include "stdafx.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzTexture.h"
#include "ZzzOpenglUtil.h"
#include "zzzOpenData.h"
#include "zzzinfomation.h"
#include "wsclientinline.h"
#include "CSMapServer.h"

extern int  LogIn;
extern char LogInID[MAX_ID_SIZE+1];
extern int HeroKey;

static  CSMServer csMapServer;

CSMServer::CSMServer ()
{
    Init ();
}

void CSMServer::Init ( void )
{
    m_bFillServerInfo = false;
    memset ( &m_vServerInfo, 0, sizeof( MServerInfo ) );
}

void CSMServer::SetHeroID ( char* ID )
{
    m_strHeroID.clear();

    if (ID != nullptr)
    {
        m_strHeroID = ID;
    }

    g_ErrorReport.Write("[AndroidLogin] SetHeroID hero=%s\r\n", m_strHeroID.c_str());
}

void CSMServer::SetServerInfo ( MServerInfo sInfo )
{
    memcpy ( &m_vServerInfo, &sInfo, sizeof( MServerInfo) );

    m_bFillServerInfo = true;
}

void CSMServer::GetServerInfo ( MServerInfo& sInfo )
{
    if ( m_bFillServerInfo )
    {
        memcpy ( &sInfo, &m_vServerInfo, sizeof( MServerInfo ) );
    }
    else
    {
        memset ( &sInfo, 0, sizeof( MServerInfo ) );
    }
}

void CSMServer::GetServerAddress ( char* szAddress )
{
    if ( m_bFillServerInfo )
    {
        memcpy ( szAddress, m_vServerInfo.m_szMapSvrIpAddress, sizeof( char )*16 );
    }
    else
    {
        memset ( szAddress, 0, sizeof( char )*16 );
    }
}

void CSMServer::ConnectChangeMapServer(MServerInfo sInfo)
{
    SetServerInfo ( sInfo );

    if ( m_bFillServerInfo && LogIn!=0 )
    {
        g_ErrorReport.Write("[AndroidLogin] ConnectChangeMapServer ip=%s port=%u hero=%s logIn=%d\r\n",
            m_vServerInfo.m_szMapSvrIpAddress,
            (unsigned int)m_vServerInfo.m_wMapSvrPort,
            m_strHeroID.c_str(),
            LogIn);
		DeleteSocket ();
        SaveOptions();
		SaveMacro("Data\\Macro.txt");

		::Sleep ( 20 );

        if ( CreateSocket( m_vServerInfo.m_szMapSvrIpAddress, m_vServerInfo.m_wMapSvrPort ) )
        {
            g_bGameServerConnected = TRUE;
        }
    }
}

void CSMServer::SendChangeMapServer ( void )
{
    if ( m_bFillServerInfo==false || LogIn==0 ) return;

    if (m_strHeroID.empty() && CharacterAttribute != nullptr && CharacterAttribute->Name[0] != 0)
    {
        m_strHeroID = CharacterAttribute->Name;
        g_ErrorReport.Write("[AndroidLogin] SendChangeMapServer fallback hero=%s from CharacterAttribute\r\n", m_strHeroID.c_str());
    }

    char  CharID[MAX_ID_SIZE+1];

    strcpy ( CharID, m_strHeroID.c_str() );
//	memcpy ( CharID, m_strHeroID.c_str(), MAX_ID_SIZE );
	CharID[MAX_ID_SIZE] = NULL;

    g_ErrorReport.Write("[AndroidLogin] SendChangeMapServer hero=%s port=%u code=%u logIn=%d\r\n",
        CharID,
        (unsigned int)m_vServerInfo.m_wMapSvrPort,
        (unsigned int)m_vServerInfo.m_wMapSvrCode,
        LogIn);

    ClearCharacters ( -1 );
    InitGame ();
    SendChangeMServer(LogInID,CharID,m_vServerInfo.m_iJoinAuthCode1,m_vServerInfo.m_iJoinAuthCode2,m_vServerInfo.m_iJoinAuthCode3,m_vServerInfo.m_iJoinAuthCode4);
}

void CSMServer::SendJoinCurrentHero ( void )
{
    if ( LogIn == 0 || m_strHeroID.empty() )
    {
        g_ErrorReport.Write("[AndroidLogin] SendJoinCurrentHero skipped heroEmpty=%d logIn=%d\r\n",
            m_strHeroID.empty() ? 1 : 0,
            LogIn);
        return;
    }

    char CharID[MAX_ID_SIZE+1];

    strcpy(CharID, m_strHeroID.c_str());
    CharID[MAX_ID_SIZE] = 0;

    g_ErrorReport.Write("[AndroidLogin] SendJoinCurrentHero hero=%s\r\n", CharID);

    SendRequestJoinMapServer(CharID);
}
