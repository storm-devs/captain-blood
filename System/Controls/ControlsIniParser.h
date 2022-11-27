#pragma once

#include "..\..\common_h\core.h"
#include "..\..\common_h\templates\string.h"

class Controls;
struct Token;

// разбор ini-файлов контролов
class ControlsIniParser
{
public:

	struct Table
	{
		Table() : items(__FILE__, __LINE__) {}
		Table(const string & name) : items(__FILE__, __LINE__), tableName(name) {}
		struct Item
		{
			string logName;
			string hwName;
		};
		string tableName;
		array<Item> items;
	};

private:

	struct TokenOperation
	{
		string	token;		// токен
		bool	addNext;	// операция со следующим токеном ( "," = "ИЛИ" ; "+" = "И")
	};

	// таблицы маппинга
	array<Table>		m_mappingTables;

	void MakeAlias(Controls& ctrls, const array<string>& tokens);
	void MakeMapping(const array<string>& strings, Table& mapping);
	void ParseToken(const char* token, Token& parsedToken);
//	void ParseAliases(Controls& ctrls, const array<string>& values);
	void ParseControls(Controls& ctrls, const array<string>& values);
//	static string GetToken(const char* str, unsigned int startPosition, const char* separators, char * separator = NULL);
	
public:

	void ParseAliases(Controls& ctrls, const array<string>& values);
	static string GetToken(const char* str, unsigned int startPosition, const char* separators, char * separator = NULL);

	ControlsIniParser();
	void Parse(Controls& ctrls);

	void AddJoystickMappingTable(const char* name ) { m_mappingTables.Add(Table(name)); }
	void ParseJoystickMappings();
	const Table& GetMappingTable(const char* name) const;
	void ClearMappings();

};
