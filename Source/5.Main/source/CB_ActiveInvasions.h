#pragma once
#include "Protocol.h"
#if(CB_ActiveInvasionsD)
class CB_ActiveInvasions
{
public:
	struct PMSG_ACTIVE_INVASIONS_RECV
	{
		PSWMSG_HEAD header; // C2:F3:E2
		BYTE count;
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

	struct PMSG_INVASION_MONSTER_UPDATE_RECV
	{
		PSBMSG_HEAD header; // C2:F3:E2
		int monster_id;
		int count;
	};

	struct ACTIVE_INVASIONS
	{
		int monster_id;
		InvasionsData count;
		bool is_update;
		int blink_counter;
		bool blink_type;
	};

	struct INVASIONS_WINDOW_SAVE
	{
		float x;
		float y;
		int is_full;
	};


	CB_ActiveInvasions();
	~CB_ActiveInvasions();
	void list_recv(PMSG_ACTIVE_INVASIONS_RECV* lpMsg);
	void update_monster_recv(PMSG_INVASION_MONSTER_UPDATE_RECV* lpMsg);
	//
	void DrawWindow();
private:
	//std::map<int, int> m_data;
	std::vector <ACTIVE_INVASIONS> m_data;
	bool is_update;
};

extern CB_ActiveInvasions* gCB_ActiveInvasions;
#endif

