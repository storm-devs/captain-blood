#pragma once
#include "..\..\common_h\mission.h"

class TransformInvertor : public MissionObject
{
public:
	TransformInvertor();
	~TransformInvertor();

	virtual bool Create(MOPReader & reader) {return EditMode_Update(reader);}
	virtual bool EditMode_Update(MOPReader & reader);
	virtual void PostCreate();

	MO_IS_FUNCTION(TransformInvertor, MissionObject);

	//Получить матрицу объекта
	virtual Matrix & GetMatrix(Matrix & mtx);

private:
	ConstString pcObjName;
	MOSafePointer moSafePointer;
};
