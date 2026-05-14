

#ifndef _LAUNCHERHELPER_H_
#define _LAUNCHERHELPER_H_

#pragma once

#pragma warning(disable : 4786)
#include <string>

#pragma pack(push, 1)
typedef struct __LAUNCHINFO {
	std::string ip;			//. ip address to connect
	unsigned long port;		//. port number to connect
} WZLAUNCHINFO, *LPWZLAUNCHINFO;
#pragma pack(pop)

bool wzRegisterConnectionKey();		//. Register connection key.
void wzUnregisterConnectionKey();	//. Unregister connection key
unsigned long wzGetConnectionKey();
/* Connection Key�� ���ٸ� �����Ѵ�.(return 0xFFFFFFFF) */

bool wzPushLaunchInfo(const WZLAUNCHINFO& LaunchInfo);
/* ����Ű�� ���ų� ����Ű�� ��ϵ��� 5�� �̳��� �Լ��� ȣ����� ��������� �����Ѵ�. */
/* �Լ��� ȣ��� �ڿ��� ����Ű�� ��������ȴ� */

bool wzPopLaunchInfo(WZLAUNCHINFO& LaunchInfo);				
/* LaunchInfo�� ���ٸ� �����Ѵ�. */
/* �Լ��� ȣ��� �ڿ��� ���������� �����ȴ� */

/*
	// example

	//. Mu online launcher side
	WZLAUNCHINFO LaunchInfo;
	LaunchInfo.ip = "YOUR_LAN_HOST";
	LaunchInfo.port = 44605;
	if(wzPushLaunchInfo(LaunchInfo)) {		//. encryption management
		//. success
		//. launching Mu update application
	}

	//. Mu online application side
	WZLAUNCHINFO LaunchInfo;
	if(wzPopLaunchInfo(LaunchInfo)) {
		//. success
		//. connect to LaunchInfo.ip:LaunchInfo.port
	}
	else {
		//. failed.
		//. exit process
	}

*/

#endif // _LAUNCHERHELPER_H_