//============================================================================================
// Spirenkov Maxim, 2006
//============================================================================================
// Mission objects
//============================================================================================
// StateAccessors
//============================================================================================

#include "StateAccessors.h"


//============================================================================================

StateAccessor::StateAccessor()
{
	name = null;
}

//Получить текущие имя стейта
const char * StateAccessor::GetName(const char * name, bool useAccessFilter)
{
	if(string::IsEmpty(name))
	{
		LogicDebugError("Can't operate because name is empty");
		return null;
	}
	//Для начала проверим имя на наличее звёзд и посчитаем длинну
	bool isNeedReplace = false;
	for(long i = 0; name[i]; i++)
	{
		isNeedReplace |= (name[i] == '*');
	}
	if(isNeedReplace)
	{
		bool isError = false;
		//Разбираем строку
		tempName.Empty();
		tempName.Reserve(i + 32);
		for(long i = 0; name[i]; i++)
		{
			if(name[i] == '*')
			{				
				const char * insert = "";
				//Начинаеться имя объекта
				dword index = tempName.Len();
				for(i++; name[i] != '*' && name[i]; i++)
				{
					tempName += name[i];
				}
				if(index < tempName.Len())
				{
					const char * objectName = tempName.c_str() + index;
					MOSafePointer mo;
					if(FindObject(ConstString(objectName), mo))
					{
						MO_IS_IF(stVarId, "StateVariable", mo.Ptr())
						{
							insert = ((StateVariable *)mo.Ptr())->GetParameter();
						}else{
							LogicDebugError("Mission object \"%s\" is not type \"State parameter\"", objectName);
							isError = true;
						}
					}else{
						LogicDebugError("Mission object \"%s\" not found", objectName);
						isError = true;
					}
				}
				tempName.Delete(index, tempName.Len() - index);
				tempName += insert;
			}else{
				tempName += name[i];
			}
		}
		if(isError)
		{
			return null;
		}
		name = tempName.c_str();		
	}
	if(!useAccessFilter)
	{
		return name;
	}
	//Проверяем на возможность доступа по префиксам
	const char * enableAccess[] = {"profile.", "profile1.", "options.", "game.", "runtime."};
	for(long i = 0; i < ARRSIZE(enableAccess); i++)
	{		
		const char * pefexName = enableAccess[i];
		for(long j = 0; pefexName[j]; j++)
		{
			char c = name[j];
			if(c <= 'Z' && c >= 'A') c += 'a' - 'A';
			if(pefexName[j] != c) break;
		}
		if(!pefexName[j])
		{
			return name;
		}
	}
	LogicDebugError("Can't operate because path \"%s\" not accessible", name);
	return null;
}


//============================================================================================

StateVariable::StateVariable() : table(_FL_, 1)
{
	mode = tm_force;
	param = null;
}

//Инициализировать объект
bool StateVariable::Create(MOPReader & reader)
{
	ConstString smode = reader.Enum();
	mode = tm_force;
	for(long m = 0; m < tm_count; m++)
	{
		if(smode == tms[m].name)
		{
			mode = tms[m].mode;
			break;
		}
	}
	param = reader.String().c_str();
	if(mode == tm_case_string || mode == tm_case_id)
	{
		//Надо прочитать дальше
		long count = reader.Array();
		if(mode == tm_case_id)
		{
			table.AddElements(count);
			for(long i = 0; i < count; i++)	
			{
				for(long i = 0; i < count; i++)	
				{
					table[i] = reader.LocString();
				}
			}
		}else{
			mode = tm_case_id;
			for(long i = 0; i < count; i++)	
			{
				reader.LocString();
			}
			count = reader.Array();
			table.AddElements(count);
			for(long i = 0; i < count; i++)	
			{
				table[i] = reader.String().c_str();
			}
		}
	}
	return true;
}

//Получить параметер
const char * StateVariable::GetParameter()
{
	bool isOk = false;
	const char * name;
	if(mode == tm_param)
	{
		parameter = param;
	}else
	if(mode == tm_string)
	{
		name = GetName(param, false);
		if(name)
		{
			parameter = api->Storage().GetString(name, "", &isOk);
			if(!isOk)
			{
				LogicDebugError("Can't access to string state \"%s\" (taking from \"%s\")", name, param);
				return null;
			}
		}
	}else{
		name = GetName(param, false);
		if(name)
		{
			float value = api->Storage().GetFloat(name, 0.0f, &isOk);
			if(!isOk)
			{
				LogicDebugError("Can't access to number state \"%s\" (taking from \"%s\")", name, param);
				return null;
			}
			if(mode == tm_case_id)
			{
				if(!table.Size())
				{
					LogicDebugError("Can't get table value because table is empty");
					return null;
				}
				int index = (int)value;
				if(index < 0 || index >= table)
				{
					LogicDebugError("Number state \"%s\" (taking from \"%s\") value %i out of range [0..%i]", name, param, index, table.Size() - 1);
					if(index < 0)
					{
						index = 0;
					}else{
						index = table - 1;
					}					
				}
				parameter = table[index];
			}else{
				const char * formatString = "%f";
				switch(mode)
				{
				case tm_int:
					formatString = "%.0f";
					break;
				case tm_float_X_1:
					formatString = "%.1f";
					break;
				case tm_float_X_2:
					formatString = "%.2f";
					break;
				case tm_float_X_3:
					formatString = "%.3f";
					break;
				case tm_float_X_4:
					formatString = "%.4f";
					break;
				default:
					Assert(false);
				}
				char buf[32];
				crt_snprintf(buf, sizeof(buf) - 1, formatString, value);
				buf[sizeof(buf) - 1] = 0;
				parameter = buf;
			}
		}
	}
	LogicDebug("Return value = %s", parameter.c_str());
	return parameter.c_str();
}


const char * StateVariable::comment = 
"This object make dynamic state name.\n"
"This sample show how to access to gold:\n"
"    In object with type \"State number set\" write:  \"Profile.*Difficulty*.Player.gold\"\n"
"    Also we can make object with name Difficulty and type of \"State variable\"\n"
"    This object will be access to \"Profile.Global.Difficulty\" as string state\n"
"    When we activate object \"State number set\" they insert to *Difficulty* part\n"
"    value from string state \"Profile.Global.Difficulty\".\n"
"    You can use any count of replaces in state name."
" ";

const StateVariable::EnumParams StateVariable::tms[tm_count] = {{tm_param, ConstString("Constant")},
																	{tm_string, ConstString("String state")},
																	{tm_int, ConstString("Number state as integer")},
																	{tm_float_X_1, ConstString("Number state as float X.X")},
																	{tm_float_X_2, ConstString("Number state as float X.XX")},
																	{tm_float_X_3, ConstString("Number state as float X.XXX")},
																	{tm_float_X_4, ConstString("Number state as float X.XXXX")},
																	{tm_case_id, ConstString("Number state as index in loctable")},
																	{tm_case_string, ConstString("Number state as index in strtable")}};


MOP_BEGINLISTCG(StateVariable, "State variable", '1.00', 0, StateVariable::comment, "Managment")
	MOP_ENUMBEG("Md")
		for(long i = 0; i < ARRSIZE(StateVariable::tms); i++)
		{
			MOP_ENUMELEMENT(StateVariable::tms[i].name.c_str())
		}
	MOP_ENUMEND
	MOP_ENUMC("Md", "Mode", "How to use this parameter")
	MOP_STRINGC("Parameter", "", "String parameter use la-la-la")
	MOP_ARRAYBEGC("Loctable", 0, 100, "Table of localization strings")
		MOP_LOCSTRINGC("String", "This string use only for set UI elements")
	MOP_ARRAYEND
	MOP_ARRAYBEGC("Strtable", 0, 100, "Table of system strings")
		MOP_STRINGC("Value", "", "This string use only in system purpose")
	MOP_ARRAYEND
MOP_ENDLIST(StateVariable)



//============================================================================================

//Инициализировать объект
bool StateEmpty::Create(MOPReader & reader)
{
	name = reader.String().c_str();
	return true;
}

//Активировать/деактивировать объект
void StateEmpty::Activate(bool isActive)
{
	if(isActive)
	{
		api->Storage().Delete(name);
		LogicDebug("Activate: now state \"%s\" is empty", name);
	}else{
		LogicDebug("Deactivate: ignore");
	}
}

MOP_BEGINLISTG(StateEmpty, "State folder cleaner", '1.00', 10, "Managment")
	MOP_STRING("State folder name", "")
MOP_ENDLIST(StateEmpty)


//============================================================================================

StateCopy::StateCopy()
{
	name2 = null;
}

//Инициализировать объект
bool StateCopy::Create(MOPReader & reader)
{
	name = reader.String().c_str();
	name2 = reader.String().c_str();
	return true;
}

//Активировать/деактивировать объект
void StateCopy::Activate(bool isActive)
{
	if(isActive)
	{
		array<byte> buffer(_FL_, 1024);
		const char * n = GetName(name, false);
		if(n)
		{
			if(api->Storage().Save(n, buffer))
			{
				LogicDebug("Copy fields from %s to buffer", n);
				n = GetName(name2, true);
				if(n)
				{
					dword readPointer = 0;
					if(api->Storage().Load(n, buffer.GetBuffer(), buffer.Size(), readPointer))
					{
						LogicDebug("Past buffer to %s.", n);
					}else{
						LogicDebugError("Can't past buffer to %s. Difference types? Try before copy delete destination folder.", n);
					}
				}
			}else{
				LogicDebugError("Can't copy fields from %s to buffer", n);
			}
		}
	}else{
		LogicDebug("Deactivate: ignore");
	}
}

MOP_BEGINLISTG(StateCopy, "State copy", '1.00', 10, "Managment")
	MOP_STRINGC("From", "", "Folder or filed from get copy")
	MOP_STRINGC("To", "", "Folder when put copy")
MOP_ENDLIST(StateCopy)

//============================================================================================


//Инициализировать объект
bool StateStringSet::Create(MOPReader & reader)
{
	name = reader.String().c_str();
	initState = reader.String().c_str();
	Activate(reader.Bool());
	return true;
}

//Активировать/деактивировать объект
void StateStringSet::Activate(bool isActive)
{
	if(isActive)
	{
		const char * n = GetName(name, true);
		if(n)
		{
			if(api->Storage().SetString(n, initState))
			{
				LogicDebug("Activate, set state \"%s\" to \"%s\"", n, initState);
			}else{
				LogicDebugError("Cant set state \"%s\", invalidate type", n);
			}
		}
	}else{
		LogicDebug("Deactivate: ignore");
	}
}

MOP_BEGINLISTG(StateStringSet, "State string set", '1.00', 10, "Managment")
	MOP_STRING("State name", "")
	MOP_STRING("State", "")
	MOP_BOOL("Active", false)
MOP_ENDLIST(StateStringSet)


//Инициализировать объект
bool StateStringCheck::Create(MOPReader & reader)
{
	inActiveProcess = false;
	name = reader.String().c_str();
	value = reader.String().c_str();
	triggerEq.Init(reader);
	triggerNEq.Init(reader);	
	return true;
}

//Активировать/деактивировать объект
void StateStringCheck::Activate(bool isActive)
{
	if(inActiveProcess)
	{
		LogicDebugError("Recursive call detected!");
		return;
	}
	inActiveProcess = true;
	if(isActive)
	{
		const char * n = GetName(name, false);
		if(n)
		{
			const char * sv = api->Storage().GetString(n, null);
			if(sv)
			{
				bool res = string::IsEqual(sv, value);
				LogicDebug("Compare state \"%s\"(\"%s\") with \"%s\", strings is ", n, sv, value, res ? "equal" : "not equal");
				if(res)
				{
					triggerEq.Activate(Mission(), false);
				}else{
					triggerNEq.Activate(Mission(), false);
				}
			}else{
				LogicDebugError("Cant check state \"%s\", invalidate type", name);
			}
		}
	}else{
		LogicDebug("Deactivate: ignore");
	}
	inActiveProcess = false;
}

MOP_BEGINLISTCG(StateStringCheck, "State string check", '1.00', 0, "Compare string state with defined value", "Managment")
	MOP_STRING("State name", "")
	MOP_STRING("Value", "")
	MOP_MISSIONTRIGGERG("Equal", "eq.")
	MOP_MISSIONTRIGGERG("Not equal", "nq.")
	MOP_BOOL("Active", false)
MOP_ENDLIST(StateStringCheck)


//Инициализировать объект
bool StateNumberSet::Create(MOPReader & reader)
{
	name = reader.String().c_str();
	initState = reader.Float();
	Activate(reader.Bool());
	return true;
}

//Активировать/деактивировать объект
void StateNumberSet::Activate(bool isActive)
{
	if(isActive)
	{
		const char * n = GetName(name, true);
		if(n)
		{
			if(api->Storage().SetFloat(n, initState))
			{
				LogicDebug("Activate, set state \"%s\" to %f", n, initState);
			}else{
				LogicDebugError("Cant set state \"%s\", invalidate type", n);
			}
		}
	}else{
		LogicDebug("Deactivate: ignore");
	}
}

MOP_BEGINLISTG(StateNumberSet, "State number set", '1.00', 10, "Managment")
	MOP_STRING("State name", "")
	MOP_FLOAT("State", 0.0f)
	MOP_BOOL("Active", false)
MOP_ENDLIST(StateNumberSet)


//Инициализировать объект
bool StateNumberCheck::Create(MOPReader & reader)
{
	inActiveProcess = false;
	name = reader.String().c_str();
	value = reader.Float();
	valueName = reader.String().c_str();
	if(string::IsEmpty(valueName)) valueName = null;
	ConstString scnd = reader.Enum();
	for(long i = 0; i < ARRSIZE(cnds); i++)
	{
		if(scnd == cnds[i].name)
		{
			cnd = cnds[i].cnd;
			break;
		}
	}
	Assert(i < ARRSIZE(cnds));
	trigger.Init(reader);
	Activate(reader.Bool());
	return true;
}

//Активировать/деактивировать объект
void StateNumberCheck::Activate(bool isActive)
{
	if(inActiveProcess)
	{
		LogicDebugError("Recursive call detected!");
		return;
	}
	inActiveProcess = true;
	if(isActive)
	{
		const char * n = GetName(name, false);
		if(!n)
		{
			LogicDebugError("Can't build state path for State name: \"%s\"", name);
			inActiveProcess = false;
			return;
		}
		bool isOk = false;
		float arg1 = api->Storage().GetFloat(n, 0.0f, &isOk);
		if(!isOk)
		{
			LogicDebugError("Can't get number for State name: \"%s\"", n);
			inActiveProcess = false;
			return;
		}
		float arg2 = value;
		if(valueName)
		{
			n = GetName(valueName, false);
			if(!n)
			{
				LogicDebugError("Can't build state path for Value name: \"%s\"", valueName);
				inActiveProcess = false;
				return;
			}
			isOk = false;
			arg2 = api->Storage().GetFloat(n, 0.0f, &isOk);
			if(!isOk)
			{
				LogicDebugError("Can't get number for Value name: \"%s\"", n);
				inActiveProcess = false;
				return;
			}
		}
		bool isCnd = false;
		const char * scnd = "";
		switch(cnd)
		{
		case c_less:
			isCnd = (arg1 < arg2);
			scnd = "<";
			break;
		case c_less_eq:
			isCnd = (arg1 <= arg2);
			scnd = "<=";
			break;
		case c_eq:
			isCnd = fabsf(arg1 - arg2) < 1e-10f;
			scnd = "==";
			break;
		case c_above_eq:
			isCnd = (arg1 >= arg2);
			scnd = ">=";
			break;
		case c_above:
			isCnd = (arg1 > arg2);
			scnd = ">";
			break;
		default:
			Assert(false);
		}
		LogicDebug("Activate");
		LogicDebug("Arg1 is \"%s\" = %f", name, arg1);
		if(valueName)
		{
			LogicDebug("Arg2 is \"%s\" = %f", valueName, arg2);
		}else{
			LogicDebug("Arg2 is constant: %f", value);
		}
		LogicDebug("Activate, condition is %f %s %f", arg1, arg2);
		if(isCnd)
		{
			LogicDebug("Start trigger...");
			trigger.Activate(Mission(), false);
		}else{
			LogicDebug("Skip trigger, because condition is false...");
		}
	}else{
		LogicDebug("Deactivate: ignore");
	}
	inActiveProcess = false;
}

const char * StateNumberCheck::comment = "Check number state with default value";

const StateNumberCheck::EnumParams StateNumberCheck::cnds[c_size] = {{c_less, ConstString("Less")},
																	{c_less_eq, ConstString("Less-Equal")},
																	{c_eq, ConstString("Equal")},
																	{c_above_eq, ConstString("Above-Equal")},
																	{c_above, ConstString("Above")}};



MOP_BEGINLISTCG(StateNumberCheck, "State number check", '1.00', 0, StateNumberCheck::comment, "Managment")
	MOP_ENUMBEG("Cnd")
		for(long i = 0; i < ARRSIZE(StateNumberCheck::cnds); i++)
		{
			MOP_ENUMELEMENT(StateNumberCheck::cnds[i].name.c_str())
		}
	MOP_ENUMEND

	MOP_STRING("State name", "")
	MOP_FLOAT("Value", 0.0f)
	MOP_STRINGC("Value name", "", "Set there state name if you want use game state instead of value constant")
	MOP_ENUM("Cnd", "Condition")
	MOP_MISSIONTRIGGERG("Event", "")
	MOP_BOOL("Active", false)
MOP_ENDLIST(StateNumberCheck)



//Инициализировать объект
bool StateNumberOperation::Create(MOPReader & reader)
{
	name = reader.String().c_str();
	name2 = reader.String().c_str();
	value = reader.Float();
	valueName = reader.String().c_str();
	if(string::IsEmpty(valueName)) valueName = null;
	useLimits = reader.Bool();
	minResult = reader.Float();
	maxResult = reader.Float();
	ConstString sop = reader.Enum();
	for(long i = 0; i < ARRSIZE(ops); i++)
	{
		if(sop == ops[i].name)
		{
			op = ops[i].op;
		}		
	}
	Activate(reader.Bool());
	return true;
}

//Активировать/деактивировать объект
void StateNumberOperation::Activate(bool isActive)
{
	if(isActive)
	{
		const char * n = GetName(name, false);
		if(!n)
		{
			LogicDebugError("Can't build state path for State1 name: \"%s\"", name);
			return;
		}
		bool isOk = false;
		float arg1 = api->Storage().GetFloat(n, 0.0f, &isOk);
		if(!isOk)
		{
			LogicDebugError("Can't get number for State1 name: \"%s\"", n);
			return;
		}
		float arg2 = value;
		if(valueName)
		{
			n = GetName(valueName, false);
			if(!n)
			{
				LogicDebugError("Can't build state path for Value name: \"%s\"", valueName);
				return;
			}
			isOk = false;
			arg2 = api->Storage().GetFloat(n, 0.0f, &isOk);
			if(!isOk)
			{
				LogicDebugError("Can't get number for Value name: \"%s\"", n);
				return;
			}
		}
		float res = 0.0f;
		switch(op)
		{	
		case o_add:
			res = arg1 + arg2;
			LogicDebug("Activate, state number operation: \"%s\"(%f) + %f = %f", name, arg1, arg2, res);
			break;
		case o_sub_s_v:
			res = arg1 - arg2;
			LogicDebug("Activate, state number operation: \"%s\"(%f) - %f = (%f)", name, arg1, arg2, res);
			break;
		case o_sub_v_s:
			res = arg2 - arg1;
			LogicDebug("Activate, state number operation:  %f - \"%s\"(%f) = (%f)", arg2, name, arg1, res);
			break;
		case o_mul:
			res = arg1*arg2;
			LogicDebug("Activate, state number operation:  \"%s\"(%f)*%f = (%f)", name, arg1, arg2, res);
			break;
		case o_div_s_v:
			if(fabsf(arg2) < 1e-10f)
			{
				LogicDebugError("Activate, can't divide state by value, because value is zero");
				return;
			}
			res = arg1/arg2;
			LogicDebug("Activate, state number operation:  \"%s\"(%f)/%f = (%f)", name, arg1, arg2, res);
			break;
		case o_div_v_s:
			if(fabsf(arg1) < 1e-10f)
			{
				LogicDebugError("Activate, can't divide value by state, because state \"%s\" is zero", name);
				return;
			}
			res = arg2/arg1;
			LogicDebug("Activate, state number operation:  %f/\"%s\"(%f) = (%f)", arg2, name, arg1, res);
			break;
		}
		if(useLimits)
		{
			if(res < minResult)
			{
				LogicDebug("Use limits, change %f to Min(%f)", res, minResult);
				res = minResult;
			}
			if(res > maxResult)
			{
				LogicDebug("Use limits, change %f to Max(%f)", res, maxResult);
				res = maxResult;
			}
		}
		n = GetName(name2, true);
		if(!n)
		{
			LogicDebugError("Can't build state path for State2 name: \"%s\"", name);
		}
		if(api->Storage().SetFloat(n, res))
		{
			LogicDebug("Set relult %f to State2 name \"%s\"", res, n);
		}else{
			LogicDebugError("Can't set result to State2 name \"%s\", because state have not float type", n);
		}		
	}else{
		LogicDebug("Deactivate: ignore");
	}
}

const char * StateNumberOperation::comment = "Mathimatic operations with state";

const StateNumberOperation::EnumParams StateNumberOperation::ops[o_size] = {{o_add, ConstString("State2 = State1 + Value")},
																			{o_sub_s_v, ConstString("State2 = State1 - Value")},
																			{o_sub_v_s, ConstString("State2 = Value - State1")},
																			{o_mul, ConstString("State2 = State1*Value")},
																			{o_div_s_v, ConstString("State2 = State1 / Value")},
																			{o_div_v_s, ConstString("State2 = Value / State1")}};


MOP_BEGINLISTCG(StateNumberOperation, "State number operation", '1.00', 0, StateNumberOperation::comment, "Managment")
	MOP_ENUMBEG("Op")
		for(long i = 0; i < ARRSIZE(StateNumberOperation::ops); i++)
		{
			MOP_ENUMELEMENT(StateNumberOperation::ops[i].name.c_str())
		}
	MOP_ENUMEND
	MOP_STRING("State1 name", "")
	MOP_STRING("State2 name", "")
	MOP_FLOAT("Value", 0.0f)
	MOP_STRINGC("Value name", "", "Set there state name if you want use game state instead of value constant")
	MOP_BOOL("Use limits", true)
	MOP_FLOAT("Min", 0.0f)
	MOP_FLOAT("Max", 1000000.0f)
	MOP_ENUM("Op", "Operation")
	MOP_BOOL("Active", false)
MOP_ENDLIST(StateNumberOperation)




StateNumberGet::StateNumberGet() : buffer(_FL_, 256)
{
}

//Инициализировать объект
bool StateNumberGet::Create(MOPReader & reader)
{
	objName = reader.String();
	mo.Reset();
	command = reader.String().c_str();
	//Параметры команды
	numParams = reader.Array();
	Assert(numParams <= ARRSIZE(params));
	for(long i = 0; i < numParams; i++)
	{
		params[i].str = reader.String().c_str();
		params[i].needTranslate = reader.Bool();
	}
	return true;
}

//Активировать/деактивировать объект
void StateNumberGet::Activate(bool isActive)
{
	if(!isActive)
	{
		LogicDebug("Deactivate: ignore");
		return;
	}
	if(!mo.Validate())
	{
		if(!FindObject(objName, mo))
		{
			LogicDebugError("Activate: Can't find mission object: \"%s\"", name);
			return;
		}
		Assert(mo.Validate());
	}
	buffer.Empty();
	const char * prms[ARRSIZE(params)];
	long trs[ARRSIZE(params)];
	for(long i = 0; i < numParams; i++)
	{
		prms[i] = null;
		trs[i] = -1;
		if(params[i].needTranslate)
		{			
			const char * str = GetName(params[i].str, false);
			if(str)
			{
				trs[i] = buffer;
				long len = string::Size(str) + 1;
				buffer.AddElements(len);
				memcpy(&buffer[trs[i]], str, len);
			}
		}else{
			prms[i] = params[i].str;
		}
	}
	for(long i = 0; i < numParams; i++)
	{
		if(!prms[i])
		{
			if(trs[i] >= 0)
			{
				prms[i] = buffer.GetBuffer() + trs[i];
			}else{
				LogicDebugError("Activate: No send command. Can't get all parameters.");
				return;
			}
		}
	}
	LogicDebug("Execute command \"%s\" for object \"%s\"", command, name);
	for(long n = 0; n < numParams; n++)
	{
		LogicDebug("|-param %i: \"%s\"", n, prms[n]);
	}
	LogicDebugLevel(true);
	mo.Ptr()->Command(command, numParams, prms);
	LogicDebugLevel(false);
}


const char * StateNumberGet::comment = "This object send command with morphing params based on number states";

MOP_BEGINLISTCG(StateNumberGet, "State number get", '1.00', 0, StateNumberGet::comment, "Managment")
	MOP_STRINGC("Object ID", "", "Mission object, what can receave command")
	MOP_STRINGC("Command", "", "Command name for object")
	MOP_ARRAYBEG("Command params", 0, 8)
		MOP_STRINGC("Prameter", "", "Parameter for command")
		MOP_BOOLC("Translate parameter", false, "Translate parameter like state name or leave as have")
	MOP_ARRAYEND
MOP_ENDLIST(StateNumberGet)


long MissionTestConfigurator::isCreated = 0;


MissionTestConfigurator::MissionTestConfigurator()
{
}

MissionTestConfigurator::~MissionTestConfigurator()
{
	isCreated--;
	if(isCreated < 0)
	{
		isCreated = 0;
	}
}

//Инициализировать объект
bool MissionTestConfigurator::EditMode_Create(MOPReader & reader)
{
	if(isCreated > 1)
	{
		return false;
	}
	isCreated = 1;
	EditMode_Update(reader);
	return true;
}

//Обновить параметры
bool MissionTestConfigurator::EditMode_Update(MOPReader & reader)
{
	if(reader.Bool())
	{
		return true;
	}
	long count = reader.Array();
	for(long i = 0; i < count; i++)
	{
		const char * type = reader.Enum().c_str();
		const char * name = reader.String().c_str();
		const char * svalue = reader.String().c_str();
		float nvalue = reader.Float();
		if(name[0])
		{
			if(type[0] == 's' || type[0] == 'S')
			{
				api->Storage().SetString(name, svalue);
			}else{
				api->Storage().SetFloat(name, nvalue);
			}
		}
	}
	return true;
}


MOP_BEGINLISTCG(MissionTestConfigurator, "Test configurator", '1.00', 0, "Write states in editor", "Managment")
	MOP_ENUMBEG("tp")
		MOP_ENUMELEMENT("string")
		MOP_ENUMELEMENT("number")
	MOP_ENUMEND
	MOP_BOOLC("Edit process", true, "Set this flag when edit state name or type")
	MOP_ARRAYBEG("Command params", 0, 10)
		MOP_ENUM("tp", "Type")
		MOP_STRING("State name", "")
		MOP_STRING("String value", "")
		MOP_FLOAT("Number value", 0.0f)
	MOP_ARRAYEND
MOP_ENDLIST(MissionTestConfigurator)







