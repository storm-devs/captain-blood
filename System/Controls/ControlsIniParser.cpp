#include "ControlsIniParser.h"
#include "..\..\common_h\fileservice.h"
#include "pc.h"

ControlsIniParser::ControlsIniParser() :
m_mappingTables(__FILE__, __LINE__)
{

}

// выбирает из строки ini-файла элементы разделенные запятой или знаком плюс.
string ControlsIniParser::GetToken(const char* str, unsigned int startPosition, const char* separators, char * separator)
{
	string result;
	if (separator)
		*separator = 0;

	unsigned int inputStrLen = strlen(str);
	unsigned int sepStrLen = strlen(separators);
	for (unsigned int i = startPosition; i < inputStrLen; ++i)
	{
		for (unsigned int j = 0; j < sepStrLen; ++j)
		{
			if (str[i] == separators[j])
			{
				if (separator)
					*separator = separators[j];

				return result;
			}
		}
		result += str[i];
	}
	return result;
}

void ControlsIniParser::ParseToken(const char* token, Token& parsedToken)
{
	// формат
	// [-|!][(scale)]name
	// примеры
	// kb_down
	// -kb_up
	// -(0.75)kb_up
	// !gp_A0
	// (10)gp_b0
	// -!(0.75)kb_up

	parsedToken.scale = 1.0f;
	parsedToken.isInverted = false;
	parsedToken.isReverse = false;
	parsedToken.name.Empty();

	bool scaleSection = false;
	string scale;
	unsigned int tokenLen = strlen(token);
	for (unsigned int i = 0; i < tokenLen; ++i)
	{
		if (scaleSection)
		{
			if (token[i] == ')')
			{
				scaleSection = false;
				parsedToken.scale = (float)atof(&scale[0]);
			}
			else
				scale += token[i];
		}
		else
			switch (token[i])
			{
			case '-':
				parsedToken.isInverted = true;
				break;
			case '(':
				scaleSection = true;
				break;
			case '!':
				parsedToken.isReverse = true;
				break;
			case ' ':
			case '\t':
				break;
			default:
				parsedToken.name += token[i];
			}
	}
}

void ControlsIniParser::ParseAliases(Controls& ctrls, const array<string>& values)
{
	char sep;
	string seps(",+"), token;

	TokenOperation temp;
	array<TokenOperation> tokens(__FILE__, __LINE__);

	for (unsigned int i = 0; i < values.Size(); ++i)
	{
		tokens.DelAll();
		unsigned int pos = 0;
		while (	true )
		{
			token = GetToken(values[i], pos, seps, &sep);
			pos += token.Len() + 1;

			token.Trim();
			if (!token.Len())
				break;

			temp.token = token;
			temp.addNext = sep == '+';
			tokens.Add(temp);
		}

		if (tokens.Size() < 2) // алиас как минимум 1:1
		{
			api->Trace("Invalid alias format: \"%s\"\n", values[i].GetBuffer());
			continue;
		}

		Token parsedToken;
		array<Token> parsedTokens(__FILE__, __LINE__);
		for (unsigned int i = 0; i < tokens.Size(); ++i)
		{
			ParseToken(tokens[i].token, parsedToken);
			parsedToken.addNext = tokens[i].addNext;
			parsedTokens.Add(parsedToken);
		}
		ctrls.AddAlias(parsedTokens);
	}
}

void ControlsIniParser::ParseControls(Controls& ctrls, const array<string>& values)
{
	long curGroupIndex = -1;
	char sep;
	string seps(",+"), token;

	TokenOperation temp;
	array<TokenOperation> tokens(__FILE__, __LINE__);

	for (unsigned int i = 0; i < values.Size(); ++i)
	{
		tokens.DelAll();
		unsigned int pos = 0;
		while (	true )
		{
			token = GetToken(values[i], pos, seps, &sep);
			pos += token.Len() + 1;

			token.Trim();
			if (!token.Len())
				break;

			temp.token = token;
			temp.addNext = sep == '+';
			tokens.Add(temp);
		}

		if (tokens.Size() < 2) // алиас как минимум 1:1
		{
			api->Trace("Invalid control format: \"%s\"\n", values[i].GetBuffer());
			continue;
		}

		if (tokens[0].token != "group")
		{
			Token parsedToken;
			array<Token> parsedTokens(__FILE__, __LINE__);
			for (unsigned int i = 0; i < tokens.Size(); ++i)
			{
				ParseToken(tokens[i].token, parsedToken);
				parsedToken.addNext = tokens[i].addNext;
				parsedTokens.Add(parsedToken);
			}
			ctrls.AddControl(curGroupIndex, parsedTokens);
		}
		else
			curGroupIndex = ctrls.AddGroup(tokens[1].token);
	}
}

void ControlsIniParser::Parse(Controls& ctrls)
{
	IFileService* fileService = (IFileService*)api->GetService("FileService");
	Assert(fileService);

	array<IIniFile*> iniFiles(__FILE__, __LINE__);

	IIniFile* iniFile = fileService->OpenIniFile("resource\\ini\\controls.ini", _FL_);
	if (iniFile) iniFiles.Add(iniFile);

	IFinder* finder = fileService->CreateFinder("resource\\ini\\controls\\", "*.ini",
		find_all_files_no_mirrors | find_no_recursive, _FL_);

	for(dword i = 0; i < finder->Count(); i++ )
	{
		iniFile = fileService->OpenIniFile(finder->FilePath(i), _FL_);
		if (iniFile) iniFiles.Add(iniFile);

	//	api->Trace("    LOADING: %s",finder->FilePath(i));
	}

	finder->Release();

/*	finder = fileService->CreateFinder("resource\\ini\\controls\\profiles\\", "*.ini", find_all_files_no_mirrors, _FL_);
	for(dword i = 0; i < finder->Count(); i++ )
	{
		iniFile = fileService->OpenIniFile(finder->FilePath(i), _FL_);
		if (iniFile) iniFiles.Add(iniFile);
	}
	finder->Release();*/

/*	IIniFile* profiles = fileService->OpenIniFile("resource\\ini\\controls\\profiles\\profiles.ini",_FL_);

	if( profiles )
	{
		char key[16];

		for( int i = 0 ; i < 10 ; i++ )
		{
			sprintf_s(key,16,"active%d",i);

		//	const char *s = profiles->GetString("profiles","active");
			const char *s = profiles->GetString("profiles",key);

			if( s[0] )
			{
				string name = "resource\\ini\\controls\\profiles\\";

				name += s;
				name += ".ini";

				iniFile = fileService->OpenIniFile(name,_FL_);

				if( iniFile )
				{
					iniFiles.Add(iniFile); ctrls.SetCurrentProfile(s,i);

				//	api->Trace("CONTROLS: Profile '%s' was loaded.",name);
				}
			}
			else
				break;
		}

		profiles->Release();
	}*/

	//
	for (unsigned int i = 0; i < iniFiles.Size(); ++i)
	{
		array<string> controls(__FILE__, __LINE__);
		array<string> aliases(__FILE__, __LINE__);
		
		iniFiles[i]->GetStrings("aliases", "assign", aliases);
		iniFiles[i]->GetStrings("controls", "assign", controls);

		ParseAliases(ctrls, aliases);
		ParseControls(ctrls, controls);
	}

	for(unsigned int i = 0; i < iniFiles.Size(); ++i)
		iniFiles[i]->Release();
}

const ControlsIniParser::Table& ControlsIniParser::GetMappingTable( const char* mappingTable) const
{
	for (unsigned int i = 0; i < m_mappingTables.Size(); ++i)
		if ( m_mappingTables[i].tableName == mappingTable )
			return m_mappingTables[i];

	if (mappingTable != "Default")
		return GetMappingTable("Default");

	Assert(!"Bad joystick mapping ini-file! (no default mapping)");
	return *(const Table*)NULL;
}

void ControlsIniParser::ClearMappings()
{
	m_mappingTables.DelAll();
}

void ControlsIniParser::MakeMapping(const array<string>& strings, Table& mappings)
{
	string seps(",");

	for (unsigned int i = 0; i < strings.Size(); ++i)
	{
		unsigned int pos = 0;
		Table::Item item;

		item.logName = GetToken(strings[i], pos, seps, NULL);
		pos += item.logName.Len() + 1;
		item.hwName = GetToken(strings[i], pos, seps, NULL);

		item.logName.Trim();
		item.hwName.Trim();

		mappings.items.Add(item);
	}
}

void ControlsIniParser::ParseJoystickMappings()
{
	IFileService* fileService = (IFileService*)api->GetService("FileService");
	Assert(fileService);

	IIniFile* iniFile = fileService->OpenIniFile("resource\\ini\\controls.ini", _FL_);

	if(!iniFile) return;
	
	for (unsigned int k = 0; k < m_mappingTables.Size(); ++k)
	{
		array<string> mappingStrings(__FILE__, __LINE__);

		iniFile->GetStrings(m_mappingTables[k].tableName, "assign", mappingStrings);
		MakeMapping(mappingStrings, m_mappingTables[k]);
	}
	iniFile->Release();
}
