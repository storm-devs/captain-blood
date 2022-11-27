#include "QuitObject.h"

QuitObject::QuitObject()
{
}

QuitObject::~QuitObject()
{
}

bool QuitObject::Create (MOPReader &reader)
{
	EditMode_Update(reader);
	return true;
}

bool QuitObject::EditMode_Update(MOPReader &reader)
{
	Activate(reader.Bool());

	return true;
}


void QuitObject::Activate(bool isActive)
{
	if (isActive)
	{
		api->Exit();
	}

}


MOP_BEGINLISTCG(QuitObject, "QuitObject", '1.00', 100, "Actibate this object and game is quit", "Managment")

	MOP_BOOL("Active", false)

MOP_ENDLIST(QuitObject)
