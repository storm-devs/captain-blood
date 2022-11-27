#include <time.h>
#include "Console.h"
#include "..\..\..\Common_h\corecmds.h"

#include "..\..\..\Common_h\Mission.h"

#ifndef STOP_DEBUG

CREATE_SERVICE(Console, 110)

static char TempBuffer[16384];

struct ConsoleVertex
{
	Vector p;
	unsigned long color;
};

Console::Translate Console::translateTbl[] =
{
	{'Ё', '~'}, {'ё', '`'},
	{'Й', 'Q'}, {'й', 'q'},
	{'Ц', 'W'}, {'ц', 'w'},
	{'У', 'E'}, {'у', 'e'},
	{'К', 'R'}, {'к', 'r'},
	{'Е', 'T'}, {'е', 't'},
	{'Н', 'Y'}, {'н', 'y'},
	{'Г', 'U'}, {'г', 'u'},
	{'Ш', 'I'}, {'ш', 'i'},
	{'Щ', 'O'}, {'щ', 'o'},
	{'З', 'P'}, {'з', 'p'},
	{'Х', '{'}, {'х', '['},
	{'Ъ', '}'}, {'ъ', ']'},
	{'Ф', 'A'}, {'ф', 'a'},
	{'Ы', 'S'}, {'ы', 's'},
	{'В', 'D'}, {'в', 'd'},
	{'А', 'F'}, {'а', 'f'},
	{'П', 'G'}, {'п', 'g'},
	{'Р', 'H'}, {'р', 'h'},
	{'О', 'J'}, {'о', 'j'},
	{'Л', 'K'}, {'л', 'k'},
	{'Д', 'L'}, {'д', 'l'},
	{'Ж', ':'}, {'ж', ';'},
	{'Э', '"'}, {'э', '\''},
	{'Я', 'Z'}, {'я', 'z'},
	{'Ч', 'X'}, {'ч', 'x'},
	{'С', 'C'}, {'с', 'c'},
	{'М', 'V'}, {'м', 'v'},
	{'И', 'B'}, {'и', 'b'},
	{'Т', 'N'}, {'т', 'n'},
	{'Ь', 'M'}, {'ь', 'm'},
	{'Б', '<'}, {'б', ','},
	{'Ю', '>'}, {'ю', '.'},	
};


void Console::_ScreenToD3D (float sX, float sY, float &d3dX, float &d3dY)
{
	IRender* rs = GetRender();

	float fScrX = float(rs->GetScreenInfo2D().dwWidth) / 2.0f;
	float fScrY = float(rs->GetScreenInfo2D().dwHeight) / 2.0f;

	d3dX = (float)sX / fScrX - 1.0f;
	d3dY = -((float)sY / fScrY - 1.0f);
}

void Console::ScreenToD3D (float sX, float sY, float &d3dX, float &d3dY)
{
	_ScreenToD3D (sX, sY, d3dX, d3dY);
}


IRender * Console::GetRender()
{
	if(!rs)
	{
		rs = (IRender*)api->GetService("DX9Render");
		rs->GetShaderId("GUIBase", shaderGUIBase);
	}

	return rs;
}


void Console::Draw2DLine (float pX, float pY, float tX, float tY, unsigned long color)
{
	IRender* rs = GetRender();

	ConsoleVertex vrx[2];

	Vector From = Vector (0.0f);
	Vector To = Vector (0.0f);

	ScreenToD3D (pX, pY, From.x, From.y);
	ScreenToD3D (tX, tY, To.x, To.y);


	vrx[0].p = From;
	vrx[1].p = To;

	for (int r = 0; r < 2; r++) vrx[r].color = color;

	for (r = 0; r < 2; r++) vrx[r].p.z = 0.0f;

	rs->DrawPrimitiveUP (shaderGUIBase, PT_LINELIST, 1, vrx, sizeof (ConsoleVertex));

}


Console::Console() : ConsoleText (_FL_),
										 History(_FL_),
										 RegCommands(_FL_)


{
	rs = NULL;
	shaderGUIBase = 0;

	bKeyboardLockStatus = false;
	ConsoleLine = 0;
	HistoryCurrent = -1;
	Filter = COL_ALL;

	dwConsoleColor = 0xFFFFFF00;
	dwCommandLineColor = 0xFFFFFFFF;


	CursorPosition = 0;
	fBlinkTime = 0.8f;
	fCursorTime = 0.0f;
	fConsoleSpeed = 2.0f;
	fHeight = 1.0f;
	bAnimDown = false;
	bAnimUp = false;
	bHide = true;
	pRS = NULL;
	pCtrl = NULL;

	CurrentCommand = "";
}

Console::~Console()
{
	if (pBackgroundTexture) pBackgroundTexture->Release();
	pBackgroundTexture = NULL;
}



void Console::EndFrame(float dltTime)
{
	long lockCount = 0;

	if( pCtrl )
	{
		lockCount = pCtrl->LockCount();

		for( int i = 0 ; i < lockCount ; i++ )
		{
			pCtrl->Unlock();
		}
	}

	long KeyBufLen = 0;
	const KeyDescr* KeyBuf = NULL;

	if (pCtrl)
	{
		KeyBufLen = pCtrl->GetKeyBufferLength();
		if (bHide) KeyBufLen = 0;
		KeyBuf = pCtrl->GetKeyBuffer();
	}

	for (long i = 0; i < KeyBufLen; i++)
	{
		byte key = KeyBuf[i].ucVKey;
		if (key == VK_LEFT) CursorPosition--;
		if (key == VK_RIGHT) CursorPosition++;

		if (key == VK_HOME) CursorPosition = 0;
		if (key == VK_END) CursorPosition = CurrentCommand.Len();

		if (key == VK_UP && HistoryCurrent >= 0)
		{
			CurrentCommand = History[HistoryCurrent];
			HistoryCurrent--;
			if (HistoryCurrent < 0) HistoryCurrent = 0;
			CursorPosition = CurrentCommand.Len();
		}
		if (key == VK_DOWN && HistoryCurrent < (int)History.Size())
		{
			if(HistoryCurrent >= 0)
			{			
				CurrentCommand = History[HistoryCurrent];
				HistoryCurrent++;
				if (HistoryCurrent >= (int)History.Size()) HistoryCurrent = History.Size()-1;
				
				CursorPosition = CurrentCommand.Len();
			}
		}





		if (KeyBuf[i].bSystem)
		{
			if (key == VK_NEXT)
			{
				ConsoleLine--;
				if (ConsoleLine < 0) ConsoleLine = 0;
			}
			if (key == VK_PRIOR)
			{
				int TotalLines = (int)ConsoleText.Size();
				ConsoleLine++;
				if (ConsoleLine >= TotalLines) ConsoleLine = TotalLines-1;
			}


			if (key == 13)
			{
				ExecuteCommand (CurrentCommand.c_str());
				CurrentCommand = "";
				CursorPosition = 0;
				ConsoleLine = 0;
			}

			if (key == 46)
			{
				CurrentCommand.Delete(CursorPosition, 1);
			}
			if (key == 8)
			{
				CurrentCommand.Delete(CursorPosition-1, 1);
				CursorPosition--;
			}
		}


		if ((key >= 32) && (!KeyBuf[i].bSystem) && (key != 96))
		{
			char szText[2];			
			szText[0] = (char)KeyBuf[i].ucVKey;
			szText[1] = 0;

			for(dword i = 0; i < ARRSIZE(translateTbl); i++)
			{
				if(((byte)szText[0]) == translateTbl[i].from)
				{
					szText[0] = translateTbl[i].to;
					break;
				}
			}

			if(szText[0] != '`' && szText[0] != '~')
			{
				CurrentCommand.Insert(CursorPosition, szText);
				CursorPosition++;
			}
		}

		
	}


	fCursorTime += api->GetDeltaTime();

	if ((bAnimDown == false) && (bAnimUp == false))
	{
		bool bTildaPressed = false;

		if (pCtrl)
		{
			KeyBufLen = pCtrl->GetKeyBufferLength();
			KeyBuf = pCtrl->GetKeyBuffer();

			for (long k = 0; k < KeyBufLen; k++)
			{
				if (KeyBuf[k].ucVKey == 192)
				{
					bTildaPressed = true;
				}
			}
		}



		if (bTildaPressed)
		{
			if (bHide)
			{
				bAnimDown = true;
			} else
			{
				bAnimUp = true;
			}
		}
	}

	
	for( int i = 0 ; i < lockCount ; i++ )
	{
		pCtrl->Lock();
	}


	if (!bAnimUp && !bAnimDown)
	{
		if (bHide) return;
	}

	if (bAnimUp)
	{
		fHeight += api->GetDeltaTime() * fConsoleSpeed;
		if (fHeight >= 1.0f)
		{
			fHeight = 1.0f;
			bAnimUp = false;
			bHide = true;
			//pCtrl->LockDebugKeys(bKeyboardLockStatus);
			api->ExecuteCoreCommand(CoreCommand_LockDebugKeys(bKeyboardLockStatus));
		//	IControls *ctrl = (IControls *)api->GetService("Controls");
		//	Assert(ctrl)
			if( pCtrl /*&& bKeyboardLockStatus == false*/ && pCtrl->LockCount() > 0 )
				pCtrl->Unlock();
		}

	}

	if (bAnimDown)
	{
		fHeight -= api->GetDeltaTime() * fConsoleSpeed;
		if (fHeight <= 0.0f)
		{
			fHeight = 0.0f;
			bAnimDown = false;
			bHide = false;

		//	bKeyboardLockStatus = false;
			CoreCommand_StateOp ss(ccid_getstate,corestate_debugkeys,0);
			api->ExecuteCoreCommand(ss);
			bKeyboardLockStatus = !ss.value;

			//pCtrl->LockDebugKeys(true);
			api->ExecuteCoreCommand(CoreCommand_LockDebugKeys(true));
		//	IControls *ctrl = (IControls *)api->GetService("Controls");
		//	Assert(ctrl)
			if( pCtrl )
				pCtrl->Lock();
		}
	}

	RS_SPRITE spr[4];
	spr[0].vPos = Vector (-1.0f, 0.0f+fHeight, 0.0f);
	spr[0].tu = 0.0f;
	spr[0].tv = 1.0f;
	spr[0].dwColor = 0xFFFFFFFF;

	spr[1].vPos = Vector (-1.0f, 1.0f+fHeight, 0.0f);
	spr[1].tu = 0.0f;
	spr[1].tv = 0.0f;
	spr[1].dwColor = 0xFFFFFFFF;

	spr[2].vPos = Vector (1.0f, 1.0f+fHeight, 0.0f);
	spr[2].tu = 1.0f;
	spr[2].tv = 0.0f;
	spr[2].dwColor = 0xFFFFFFFF;

	spr[3].vPos = Vector (1.0f, 0.0f+fHeight, 0.0f);
	spr[3].tu = 1.0f;
	spr[3].tv = 1.0f;
	spr[3].dwColor = 0xFFFFFFFF;

	pRS->DrawSprites(pBackgroundTexture,spr, 1);




	float fHalfScreenSize = (float)pRS->GetScreenInfo2D().dwHeight * 0.5f;
	
	float fFontHeight = pRS->GetSystemFont()->GetHeight();
	float TextHeight = fHalfScreenSize - (fHeight*fHalfScreenSize);
	TextHeight -= 40.0f; //нижняя граница...

	Draw2DLine(0.0f, TextHeight+4.0f, (float)pRS->GetScreenInfo2D().dwWidth, TextHeight+4.0f, 0xFFFFFFFF);
	Draw2DLine(0.0f, TextHeight+5.0f, (float)pRS->GetScreenInfo2D().dwWidth, TextHeight+5.0f, 0xFF000000);


	float CommandHeight = TextHeight + 8.0f;

	bool bCursorInLastPos = false;
	if (CursorPosition >= (int)CurrentCommand.Len())
	{
		bCursorInLastPos = true;
		CursorPosition = CurrentCommand.Len();
	}

	if (CursorPosition < 0) CursorPosition = 0;

	string CommandPart1 = CurrentCommand;
	CommandPart1.Delete(CursorPosition, CommandPart1.Last() - CursorPosition);
	string CommandPart2;
	
	if (!bCursorInLastPos)
	{
		CommandPart2 = CurrentCommand.c_str() + CursorPosition;
	}

	//dwCursorPosition
	float fXPosition = 0.0f;
	if (!CommandPart1.IsEmpty())
	{
		pRS->Print(fXPosition, CommandHeight, dwCommandLineColor, "%s", CommandPart1.c_str());
	}
	fXPosition += pRS->GetSystemFont()->GetLength("%s", CommandPart1.c_str());

	if (fCursorTime < (fBlinkTime*0.5f))
	{
		//draw cursor
		//pRS->Print(fXPosition, CommandHeight, dwCommandLineColor, "_");
		Draw2DLine(fXPosition, CommandHeight, fXPosition, CommandHeight+fFontHeight, dwCommandLineColor);
	} else
	{
		if (fCursorTime > fBlinkTime)
		{
			fCursorTime = 0.0f;
		}
	}

	fXPosition+=2.0f;
	//fXPosition += pRS->GetSystemFont()->GetLength("_");
	if (!CommandPart2.IsEmpty())
	{
		pRS->Print(fXPosition, CommandHeight, dwCommandLineColor, "%s", CommandPart2.c_str());
	}


	


	int DrawFromLine = ConsoleText.Last();
	DrawFromLine -= ConsoleLine;
	for (int i = DrawFromLine; i >= 0; i--)
	{
		if (!NeedPrint(ConsoleText[i].Level)) continue;
		TextHeight -= fFontHeight;
		pRS->Print(0, TextHeight, ConsoleText[i].dwColor, "%s", ConsoleText[i].Text.c_str());

		if (TextHeight < fFontHeight) break;
	}
}

bool Console::Init()
{
	api->SetEndFrameLevel(this, 0);

	pRS = (IRender *)api->GetService("DX9Render");
	pCtrl = (IControlsService *)api->GetService("ControlsService");

	pBackgroundTexture = pRS->CreateTexture(_FL_, "console");




	RegisterCommand("Help", "console help command", this, (CONSOLE_COMMAND)&Console::HelpConsoleCommand);
	RegisterCommand("TextColor", "Change console text color\nParams: Color in HEX, ARGB\nExample: TextColor 0xFFFF0000", this, (CONSOLE_COMMAND)&Console::SetTextColor);
	RegisterCommand("CommandColor", "Change command line text color\nParams: Color in HEX, ARGB\nExample: CommandColor 0xFFFF0000", this, (CONSOLE_COMMAND)&Console::SetCommandColor);
	RegisterCommand("Filter", "Change mesages filter\nParams: FilterName\nValid filters are:\nALL - all messages\nCMDOUT - commands output\nDESIGNERS - designers messages\nPROGRAMMERS - all messages :)\nExample: Filter CMDOUT", this, (CONSOLE_COMMAND)&Console::FilterCommand);
	RegisterCommand("ChangeTexture", "Change background console texture\nParams: TextureName\nExample: ChangeTexture MyBackground", this, (CONSOLE_COMMAND)&Console::ChangeTexture);
	RegisterCommand("Quit", "Quit engine", this, (CONSOLE_COMMAND)&Console::QuitCommand);
	RegisterCommand("Cls", "Clear console", this, (CONSOLE_COMMAND)&Console::CLSCommand);
	RegisterCommand("Clear", "Clear console", this, (CONSOLE_COMMAND)&Console::CLSCommand);
	RegisterCommand("MemStat", "Output to log memory statistic:\nMemStat [sort:type] [min:minsize] [max:maxsize] [name:postfixfilename]\nSort types: size, blocks, freq, size20, blocks20, freq20, file\nExample: MemStat sort:blocks20 max:1024 name:common_h\\myfile.cpp", this, (CONSOLE_COMMAND)&Console::MemStatCommand);

	///
	RegisterCommand("UnlockLevels", "Unlock all levels in game", this, (CONSOLE_COMMAND)&Console::UnlockLevels);		

	return true;
}


bool Console::NeedPrint (ConsoleOutputLevel MessageLevel)
{
	if (Filter == COL_ALL) return true;

	if (Filter == COL_CMD_OUTPUT)
	{
		if (MessageLevel != COL_CMD_OUTPUT) return false;
		return true;
	}

	if (Filter == COL_PROGRAMMERS)
	{
		return true;
	}

	if (Filter == COL_DESIGNERS)
	{
		if (MessageLevel < COL_PROGRAMMERS) return true;
		return false;
	}

	return true;
}

void _cdecl Console::Trace(ConsoleOutputLevel Level, const char* FormatString, ...)
{
	va_list args;
	va_start(args, FormatString);
	crt_vsnprintf(TempBuffer, sizeof(TempBuffer) - 4, FormatString, args);
	va_end(args);

	dword i = strlen(TempBuffer);

	DWORD dwBeginFrom = 0;
	for (dword j = 0; j < i; j++)
	{
		if (TempBuffer[j] == '\n')
		{
			TempBuffer[j] = 0;

			TextEntry t;
			t.Text = (TempBuffer + dwBeginFrom);
			t.Level = Level;
			t.dwColor = dwConsoleColor;
			ConsoleText.Add(t);

			dwBeginFrom = j + 1;
		}
	}

	TextEntry t;
	t.Text = (TempBuffer + dwBeginFrom);
	t.Level = Level;
	t.dwColor = dwConsoleColor;
	ConsoleText.Add(t);

}

void Console::ExecuteCommand(const char* szCommand)
{
	if (!szCommand || !szCommand[0]) return;

	History.Add(szCommand);
	HistoryCurrent = History.Size()-1;

	parser.Tokenize(szCommand);

	bool bExecuted = false;

	if (parser.GetTokensCount() > 0)
	{

		string cmd = parser.GetTokenByIndex(0);

		Console::RegistredCommand* pCommand = FindCommand(cmd.c_str());
		if (pCommand)
		{
			bExecuted = true;

			stack.Clear();
			for (dword i = 1; i < parser.GetTokensCount(); i++)
			{
				stack.Add(parser.GetTokenByIndex(i));
			}

			if (pCommand->pureC_func)
			{
				pCommand->pureC_func(stack);
			} else
			{
				if (pCommand->Class && pCommand->Method)
				{
					(pCommand->Class->*pCommand->Method)(stack);
				}
			}


		}



	}

	if (bExecuted)
	{
		//Trace(COL_CMD_OUTPUT, "%s", szCommand);
	} else
	{
		dword dwOldColor = SetTextColor(0xFFFF0000);
		Trace(COL_CMD_OUTPUT, "Error, command not found : %s", szCommand);
		SetTextColor(dwOldColor);
	}


}

Console::RegistredCommand* Console::FindCommand(const char* szCommand)
{
	for (dword i = 0; i < RegCommands.Size(); i++)
	{
		if (RegCommands[i].Command == szCommand) return &RegCommands[i];
	}

	return NULL;
}

void Console::RegisterCommand(const char* CommandName, const char* HelpDesc, Object* _class, CONSOLE_COMMAND _method)
{
	RegistredCommand * _cmd = FindCommand(CommandName);
	if (_cmd != NULL)
	{
		if (_cmd->Class != _class || _cmd->Method != _method)
		{
			Trace (COL_PROGRAMMERS, "Command %s already registred", CommandName);
		}
		return;
	}

	RegistredCommand cmd;
	cmd.Class = _class;
	cmd.pureC_func = NULL;
	cmd.Method = _method;
	cmd.Command = CommandName;
	cmd.Help = HelpDesc;
	RegCommands.Add(cmd);

}

void Console::Register_PureC_Command(const char* CommandName, const char* HelpDesc, PURE_C_CONSOLE_COMMAND _method)
{
	RegistredCommand * _cmd = FindCommand(CommandName);
	if (_cmd != NULL)
	{
		if(_cmd->pureC_func != _method)
		{
			Trace (COL_PROGRAMMERS, "Command %s already registred", CommandName);
		}
		return;
	}

	RegistredCommand cmd;
	cmd.Class = NULL;
	cmd.pureC_func = _method;
	cmd.Method = NULL;
	cmd.Command = CommandName;
	cmd.Help = HelpDesc;
	RegCommands.Add(cmd);


}

void Console::UnregisterCommand(const char* CommandName)
{
	for (dword i = 0; i < RegCommands.Size(); i++)
	{
		if (RegCommands[i].Command == CommandName)
		{
			RegCommands.ExtractNoShift(i);
			return;
		}
	}

}


void _cdecl Console::HelpConsoleCommand(const ConsoleStack& stack)
{
	dword SavedColor = GetTextColor();

	
	SetTextColor(0xFF00FF00);
	Trace(COL_ALL, "\n\nCommands format:\ncommand param param param ...\nif param include < > use \"param whith space\"\nif param include <\"> use <\"\"> instead <\">\n\n");

	SetTextColor(0xFFFF0000);

	Trace(COL_ALL, "Available commands list...\n\n");

	for (dword i = 0; i < RegCommands.Size(); i++)
	{
		SetTextColor(0xFF00FFFF);
		Trace(COL_ALL, "[%d] Command : %s", i, RegCommands[i].Command.c_str());
		SetTextColor(0xFF0000FF);
		Trace(COL_ALL, "-------------------------------------------");
		SetTextColor(SavedColor);
		Trace(COL_ALL, "%s\n\n", RegCommands[i].Help.c_str());
	}

	SetTextColor(SavedColor);
}

void _cdecl Console::SetTextColor(const ConsoleStack& stack)
{
	const char* szColor = stack.GetParam(0);
	if (szColor)
	{
		char *stopstring = NULL;

		dwConsoleColor = strtoul(szColor, &stopstring, 16);

		Trace(COL_CMD_OUTPUT, "Text color 0x%08X", dwConsoleColor);
	}

}

void _cdecl Console::SetCommandColor(const ConsoleStack& stack)
{
	const char* szColor = stack.GetParam(0);
	if (szColor)
	{
		char *stopstring = NULL;

		dwCommandLineColor = strtoul(szColor, &stopstring, 16);

		Trace(COL_CMD_OUTPUT, "Command color 0x%08X", dwCommandLineColor);
	}
}

void _cdecl Console::ChangeTexture(const ConsoleStack& stack)
{
	const char* szTexName = stack.GetParam(0);
	if (szTexName)
	{
		if (pBackgroundTexture) pBackgroundTexture->Release();
		pBackgroundTexture = NULL;

		pBackgroundTexture = pRS->CreateTexture(_FL_, szTexName);

		Trace(COL_CMD_OUTPUT, "Background texture : %s", szTexName);
	}

}


void _cdecl Console::FilterCommand(const ConsoleStack& stack)
{
	const char* szFilterDesc = stack.GetParam(0);
	if (szFilterDesc)
	{
		if (string::IsEqual(szFilterDesc, "ALL"))
		{
			Filter = COL_ALL;
			Trace(COL_CMD_OUTPUT, "Filter : ALL");
		}
		if (string::IsEqual(szFilterDesc, "CMDOUT"))
		{
			Filter = COL_CMD_OUTPUT;
			Trace(COL_CMD_OUTPUT, "Filter : CMDOUT");
		}
		if (string::IsEqual(szFilterDesc, "DESIGNERS"))
		{
			Filter = COL_DESIGNERS;
			Trace(COL_CMD_OUTPUT, "Filter : DESIGNERS");
		}
		if (string::IsEqual(szFilterDesc, "PROGRAMMERS"))
		{
			Filter = COL_PROGRAMMERS;
			Trace(COL_CMD_OUTPUT, "Filter : PROGRAMMERS");
		}
		
	}
}

void _cdecl Console::QuitCommand(const ConsoleStack& stack)
{
	api->Exit();
}


void _cdecl Console::CLSCommand(const ConsoleStack& stack)
{
	ConsoleText.DelAll();
	ConsoleLine = 0;
}

void _cdecl Console::MemStatCommand(const ConsoleStack& stack)
{
	string type, param, fileName;
	CoreCommand_MemStat memStat(cmemstat_bysize);
	for(dword i = 0; i < stack.GetSize(); i++)
	{
		type = stack.GetParam(i);
		for(dword j = 0; j < type.Len(); j++)
		{
			if(type[j] == ':') break;
		}
		if(j >= type.Len())
		{
			Trace(COL_CMD_OUTPUT, "Unknown command param \"%s\"", type.GetBuffer());
			continue;
		}
		type.Delete(j, type.Len() - j);
		param = stack.GetParam(i);
		param.Delete(0, j + 1);
		if(!param.Len())
		{
			Trace(COL_CMD_OUTPUT, "Invalidate command param option \"%s\": \"\"", type.GetBuffer());
			continue;
		}
		if(type == "sort")
		{
			if(param == "size")
			{
				memStat.sortType = cmemstat_bysize;
			}else
			if(param == "blocks")
			{
				memStat.sortType = cmemstat_byblocks;
			}else
			if(param == "freq")
			{
				memStat.sortType = cmemstat_byfreq;
			}else
			if(param == "size20")
			{
				memStat.sortType = cmemstat_bysize20;
			}else
			if(param == "blocks20")
			{
				memStat.sortType = cmemstat_byblocks20;
			}else
			if(param == "freq20")
			{
				memStat.sortType = cmemstat_byfreq20;
			}else
			if(param == "file")
			{
				memStat.sortType = cmemstat_byfile;
			}else{
				Trace(COL_CMD_OUTPUT, "Unknown sort type: \"%s\"", param.GetBuffer());
			}
		}else
		if(type == "min")
		{
			memStat.minSize = atol(param);
		}else
		if(type == "max")
		{
			memStat.maxSize = atol(param);
		}else
		if(type == "name")
		{
			fileName = param;
			memStat.fileName = fileName;
		}else{
			Trace(COL_CMD_OUTPUT, "Unknown command param: \"%s\"", type.GetBuffer());
		}
	}
	api->ExecuteCoreCommand(memStat);
}

void _cdecl Console::UnlockLevels(const ConsoleStack & params)
{
	api->Storage().SetString("Profile.Easy.Missions.Unlocked", "m15_9");
	api->Storage().SetString("Profile.Normal.Missions.Unlocked", "m15_9");
	api->Storage().SetString("Profile.Hard.Missions.Unlocked", "m15_9");	

	array<Object*> missions(_FL_);
	api->FindObject("Mission", missions);


	for (int i = 0; i < (int)missions.Size(); i++)
	{
		IMission* ms = (IMission*)missions[i];

		MOSafePointer obj;
		ms->FindObject(ConstString("save_delay"),obj);
		
		if (obj.Ptr())
		{
			obj.Ptr()->Activate(true);
			break;
		}
	}
	
	
	Trace(COL_CMD_OUTPUT, "All levels were unlocked");
}

dword Console::GetTextColor ()
{
	return dwConsoleColor;
}

dword Console::SetTextColor (dword dwNewColor)
{
	dword r = dwConsoleColor;
	dwConsoleColor = dwNewColor;
	return r;
}

bool Console::IsHided()
{
	return bHide && !bAnimUp && !bAnimDown;
}


#endif
