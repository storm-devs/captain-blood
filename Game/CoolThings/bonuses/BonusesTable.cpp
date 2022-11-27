

#include "BonusesTable.h"


BonusesTable::BonusesTable() : descs(_FL_)
{
}


//Инициализировать объект
bool BonusesTable::Create(MOPReader & reader)
{
	long count = reader.Array();
	descs.Reserve(count);
	for(long i = 0; i < count; i++)
	{
		ConstString name = reader.String();
		Desc desc;
		desc.bonus.Reset();
		desc.hiP = Clampf(reader.Float()*0.01f);
		desc.hiLevel = Clampf(reader.Float()*0.01f);
		desc.loP = Clampf(reader.Float()*0.01f);
		desc.loLevel = Clampf(reader.Float()*0.01f);
		if(desc.hiLevel < desc.loLevel)
		{
			LogicDebugError("Lo threshold is greater then hi threshold for item %i. Fix it!", i);
			desc.hiLevel = desc.loLevel = 1.0f;
		}
		desc.select = false;
		desc.isUnique = reader.Bool();
		MOSafePointer mo;
		FindObject(name, mo);
		if(!mo.Ptr())
		{
			LogicDebugError("Bonus \"%s\" not found", name.c_str());
			continue;
		}
		MO_IS_IF_NOT(tid, "BonusBase", mo.Ptr())
		{
			LogicDebugError("Object \"%s\" isn't bonus", name.c_str());
			continue;
		}
		desc.bonus.GetSPObject() = mo;
		descs.Add(desc);
	}
	if(!descs.Size())
	{
		LogicDebugError("Bonuses table is empty...");
	}
	return true;
}

//Обработчик команд для объекта
void BonusesTable::Command(const char * id, dword numParams, const char ** params)
{
	if(string::IsEqual(id, "reset"))
	{
		BonusesManager::ResetAllBonuses(Mission());
	}
}

//Выбрать бонус из таблицы
bool BonusesTable::SelectBonus(array<BonusBase *> & bonuses)
{
	bonuses.Empty();
	if(!descs.Size())
	{
		return false;
	}
	//Разделяем вероятности для разных видов дропа
	long uniqueCount = 0;
	bool isSelected = false;
	for(long i = 0; i < descs; i++)
	{
		Desc & desc = descs[i];
		desc.select = false;
		if(!desc.bonus.Validate())
		{
			desc.bonus.Reset();
			continue;
		}
		float pnt = desc.bonus.Ptr()->GetPoints();
		if(desc.hiLevel - desc.loLevel > 1e-5f)
		{
			pnt = Clampf((pnt - desc.loLevel)/(desc.hiLevel - desc.loLevel));
		}else{
			pnt = 1.0f;
		}
		desc.p = desc.loP + (desc.hiP - desc.loP)*pnt;
		float v = Rnd();
		desc.select = (v <= desc.p);
		if(desc.p <= 1e-30f)
		{
			desc.select = false;
		}
		isSelected |= desc.select;
		if(desc.select && desc.isUnique)
		{
			uniqueCount++;
		}
	}
	if(uniqueCount > 0)
	{
		//Выбрано несколько уникальных, выбираем среди них единственный
		long sel = (long)Rnd(uniqueCount - 1.0f);
		for(long i = 0, c = 0; i < descs; i++)
		{
			Desc & desc = descs[i];
			if(desc.select)
			{
				if(sel == c)
				{					
					bonuses.Add(desc.bonus.Ptr());
					return true;
				}
				c++;
			}
		}
		//Сюда не должно попадать
		Assert(false);
	}else
	if(isSelected)
	{
		//Добавляем не уникальные бонусы
		for(long i = 0, c = 0; i < descs; i++)
		{
			Desc & desc = descs[i];
			if(desc.select)
			{
				bonuses.Add(desc.bonus.Ptr());
			}			
		}
		return true;
	}
	return false;
}


MOP_BEGINLISTCG(BonusesTable, "Bonuses table", '1.00', 10, "Bonuses drop rate table\nCommands:\n    reset - reset all bonuses", "Bonuses")
	MOP_ARRAYBEG("Bonuses", 0, 1000)
		MOP_STRINGC("Bonus", "", "Name of bonus object")
		MOP_FLOATEXC("Hi probability", 10.0f, 0.0f, 100.0f, "Drop probability in percents for high state points")
		MOP_FLOATEXC("Hi threshold", 100.0f, 0.0f, 100.0f, "High threshold in percents")
		MOP_FLOATEXC("Lo probability", 50.0f, 0.0f, 100.0f, "Drop probability in percents for low state points")
		MOP_FLOATEXC("Low threshold", 0.0f, 0.0f, 100.0f, "Low threshold in percents")
		MOP_BOOLC("Unique", false, "Single or multiple drop mode for this bonus")
	MOP_ARRAYEND
MOP_ENDLIST(BonusesTable)


