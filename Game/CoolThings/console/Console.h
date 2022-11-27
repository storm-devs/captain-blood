#ifndef DEBUG_CONSOLE_IMPLEMENTATION
#define DEBUG_CONSOLE_IMPLEMENTATION


#include "..\..\..\Common_h\iconsole.h"
#include "..\..\..\Common_h\render.h"
#include "..\..\..\Common_h\templates.h"
#include "..\..\..\Common_h\core.h"
#include "..\..\..\Common_h\controls.h"
#include "simpleparser.h"




class Console : public IConsole
{
	struct RegistredCommand
	{
		string Command;
		string Help;
		Object* Class;
		CONSOLE_COMMAND Method;
		PURE_C_CONSOLE_COMMAND pureC_func;
	};

	struct Translate
	{
		byte from;
		byte to;
	};

	bool bKeyboardLockStatus;
	array<RegistredCommand> RegCommands;



	IRender * rs;
	ShaderId shaderGUIBase;

	static Translate translateTbl[];

	void _ScreenToD3D (float sX, float sY, float &d3dX, float &d3dY);
	void ScreenToD3D (float sX, float sY, float &d3dX, float &d3dY);
	IRender * GetRender();
	void Draw2DLine (float pX, float pY, float tX, float tY, unsigned long color);


	RegistredCommand* FindCommand(const char* szCommand);
	
	
	ConsoleParser parser;
	ConsoleStack stack;


	dword dwConsoleColor;
	dword dwCommandLineColor;

	int CursorPosition;

	bool bAnimDown;
	bool bAnimUp;
	bool bHide;

	IBaseTexture *pBackgroundTexture;
	IRender *pRS;

	IControlsService *pCtrl;

	float fConsoleSpeed;
	float fHeight;

	struct TextEntry
	{
		dword dwColor;
		ConsoleOutputLevel Level;
		string Text;
	};

	int ConsoleLine;
	int HistoryCurrent;
	array<TextEntry> ConsoleText;
	array<string> History;

	string CurrentCommand;

	float fBlinkTime;
	float fCursorTime;

	ConsoleOutputLevel Filter;


	bool NeedPrint (ConsoleOutputLevel MessageLevel);

public:

	Console();
	virtual ~Console();


	virtual void EndFrame(float dltTime);

	virtual bool Init();

	virtual void _cdecl Trace(ConsoleOutputLevel Level, const char* FormatString,...);

	virtual void ExecuteCommand(const char* szCommand);

	virtual void RegisterCommand(const char* CommandName, const char* HelpDesc, Object* _class, CONSOLE_COMMAND _method);
	virtual void Register_PureC_Command(const char* CommandName, const char* HelpDesc, PURE_C_CONSOLE_COMMAND _method);

	virtual void UnregisterCommand(const char* CommandName);

	virtual dword GetTextColor ();
	virtual dword SetTextColor (dword dwNewColor);

	virtual bool IsHided();



	void _cdecl HelpConsoleCommand(const ConsoleStack& stack);
	void _cdecl SetTextColor(const ConsoleStack& stack);
	void _cdecl SetCommandColor(const ConsoleStack& stack);
	void _cdecl ChangeTexture(const ConsoleStack& stack);
	void _cdecl FilterCommand(const ConsoleStack& stack);
	void _cdecl QuitCommand(const ConsoleStack& stack);
	void _cdecl CLSCommand(const ConsoleStack& stack);
	void _cdecl MemStatCommand(const ConsoleStack& stack);
	

	///
	void _cdecl UnlockLevels(const ConsoleStack & params);
};


#endif