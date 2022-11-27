
#include "ChestKeeper.h"
#include "Chest.h"

ChestKeeper::ChestKeeper()
{
	iter = null;
}

ChestKeeper::~ChestKeeper()
{
	if (iter) iter->Release();
}

bool ChestKeeper::Create(MOPReader & reader)
{
	iter = &Mission().GroupIterator(CHEST_OBJECT_GROUP, _FL_);

	return true;
}


bool ChestKeeper::EditMode_Update(MOPReader & reader)
{	
	return true;
}

void ChestKeeper::Activate(bool isActive)
{
	if (!iter) return;

	iter->Reset();

	while(!iter->IsDone())
	{
		Chest* chest = (Chest*)iter->Get();

		chest->SaveState();

		iter->Next();
	}		
}

void ChestKeeper::Restart()
{
	if (!iter) return;

	iter->Reset();

	while(!iter->IsDone())
	{
		Chest* chest = (Chest*)iter->Get();

		if (chest->AllowRestart())
		{
			chest->Restart();
		}

		iter->Next();
	}	
}

const char * ChestKeeper::comment = "Chest Keeper - сохраняет состояние сундуков при аткивации.\nДля рестарта сундуков, нужно отрестартть чест кипер,\nчест кипер сам отрестартит те сундуки которые были закрыты до рестарта.";

MOP_BEGINLISTCG(ChestKeeper, "ChestKeeper Object", '1.00', 150, ChestKeeper::comment,"Quick Events")
			

MOP_ENDLIST(ChestKeeper)