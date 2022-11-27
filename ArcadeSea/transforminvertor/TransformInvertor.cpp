#include "TransformInvertor.h"

TransformInvertor::TransformInvertor()
{
}

TransformInvertor::~TransformInvertor()
{
}

bool TransformInvertor::EditMode_Update(MOPReader & reader)
{
	pcObjName = reader.String();
	moSafePointer.Reset();
	return true;
}

void TransformInvertor::PostCreate()
{
	FindObject( pcObjName, moSafePointer );
}

//Получить матрицу объекта
Matrix & TransformInvertor::GetMatrix(Matrix & mtx)
{
	if( moSafePointer.Validate() )
	{
		moSafePointer.Ptr()->GetMatrix(mtx).Inverse();
		return mtx;
	}
	else
	{
		if( pcObjName.NotEmpty() && EditMode_IsOn() )
			FindObject( pcObjName, moSafePointer );
	}
	return mtx.SetIdentity();
}

MOP_BEGINLISTCG(TransformInvertor, "TransformInvertor", '1.00', 100, "Transform invertor - get inverse transform from object", "default");
MOP_STRING("Mission object", "")
MOP_ENDLIST(TransformInvertor)
