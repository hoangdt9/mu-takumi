#pragma once
#include "Protocol.h"
#if(CB_ActiveInvasionsD)
struct PMSG_ACTIVE_INVASIONS_SEND
{
	PSWMSG_HEAD header; // C2:F3:E2
	BYTE count;
};

struct PMSG_INVASION_MONSTER_UPDATE_SEND
{
	PSBMSG_HEAD header; // C2:F3:E2
	int monster_id;
	int count;
};

struct InvasionsData
{
	InvasionsData(int count, int max_count) : _count(count), _max_count(max_count) {	}
	InvasionsData() : _count(0), _max_count(0) {}
	int _count;
	int _max_count;
};

struct PMSG_ACTIVE_INVASIONS
{
	int monster_id;
	InvasionsData count;
};

class CB_ActiveInvasions
{
public:
	CB_ActiveInvasions();
	void monster_add(int monster_id);
	void monster_del(int monster_id, bool need_send = false);
	void send_list_to_client();
	void update_by_monster_id(int monster_id);
private:
	std::map<int, InvasionsData> m_data;
}; extern CB_ActiveInvasions gCB_ActiveInvasions;

#endif