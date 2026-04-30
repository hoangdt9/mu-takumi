// -------------------------------------------------------------------------------



struct CUSTOM_COMMAND_INFO
{
	int Index;
	int Color;
	char Command[40];
	char infoCommand[140];
};

class CCustomCommandInfo
{
public:
	CCustomCommandInfo();
	~CCustomCommandInfo();

	void Init();
	void Load(CUSTOM_COMMAND_INFO* info);
	void SetInfo(CUSTOM_COMMAND_INFO info);
	CUSTOM_COMMAND_INFO* GetInfo(int index);
	// ----
public:
	CUSTOM_COMMAND_INFO m_CustomCommandInfo[MAX_CUSTOM_COMMAND];
	int Page;
	int Count;
	// ----
}; extern CCustomCommandInfo gCustomCommandInfo;
// -------------------------------------------------------------------------------