#include "..\pch.h"
#include ".\rope.h"
#include "..\..\Common_h\Mission.h"
#include "Ship.h"

// конструктор по умолчанию
Rope::Rope()
{
	params_.model_ = "";
}

// конструктор копий
Rope::Rope(const Rope& other)
{
	params_ = other.params_;
}

// конструктор по параметрам
Rope::Rope(const Rope::Params& other)
{
	params_ = other;
}

// оператор присваивания
Rope& Rope::operator=(const Rope& other)
{
	params_ = other.params_;
	return *this;
}

// деструктор
Rope::~Rope(void)
{

}

void Rope::Make()
{
	if (GetMOOwner()->EditMode_IsOn())
		return;
	
}

void Rope::Attach()
{
//	ropeObject_.AttachToIntersections();
}

void Rope::Detach()
{
//	ropeObject_.Detach();
}

void Rope::DoWork(const Matrix& mtx)
{
//	ropeObject_.Work(api->GetDeltaTime());
}