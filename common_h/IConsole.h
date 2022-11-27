#ifndef DEBUG_CONSOLE_INTERFACE
#define DEBUG_CONSOLE_INTERFACE


#include "core.h"
#include "math3D.h"
#include "templates.h"


enum ConsoleOutputLevel
{
	COL_ALL = 0,
	COL_CMD_OUTPUT,
	COL_DESIGNERS,
	COL_PROGRAMMERS,

	COL_FORCE_DWORD = 0x7fffffff
};

class ConsoleStack
{
	array<string> Params;
public:

	ConsoleStack() : Params(_FL_)
	{
	};
	~ConsoleStack() {};

	dword GetSize() const
	{
		return Params.Size();
	}

	const char* GetParam(dword dwIndex) const
	{
		if (dwIndex >= GetSize()) return NULL;
		return Params[dwIndex].c_str();
	}

	void Clear ()
	{
		Params.DelAll();
	}

	void Add(const char* szParam)
	{
		Params.Add(szParam);
	}

};

typedef void (_cdecl Object::*CONSOLE_COMMAND)(const ConsoleStack& stack);
typedef void (_cdecl *PURE_C_CONSOLE_COMMAND)(const ConsoleStack& stack);



class IConsole : public Service
{

public:

	IConsole() {};
	virtual ~IConsole() {};

	virtual void _cdecl Trace(ConsoleOutputLevel Level, const char* FormatString,...)= 0;

	virtual void ExecuteCommand(const char* szCommand) = 0;

	virtual void RegisterCommand(const char* CommandName, const char* HelpDesc, Object* _class, CONSOLE_COMMAND _method) = 0;
	virtual void Register_PureC_Command(const char* CommandName, const char* HelpDesc, PURE_C_CONSOLE_COMMAND _method) = 0;
	virtual void UnregisterCommand(const char* CommandName) = 0;


	virtual dword GetTextColor () = 0;
	virtual dword SetTextColor (dword dwNewColor) = 0;

	virtual bool IsHided() = 0;

};


#endif