#pragma once
#include "..\..\Common_h\templates\array.h"

// бокс с ориентацией
class OrientBox : public Box
{
public:
	Vector angles;
};

// бокс с ориентацией
class ShipBox : public OrientBox
{
public:
	unsigned int	indexValue;
	Vector			angles;
	//bool			isHaveGeometry;

	void ReadMOPs(MOPReader& reader)
	{
		center = reader.Position();
		size = reader.Position();
		angles = reader.Angles();
		//isHaveGeometry = reader.Bool();
	}
};

// описывает форму сложного объекта
// примитивами физического движка
struct PhysForm
{
	PhysForm() :	boxes_(__FILE__, __LINE__) {}

	array<ShipBox>	boxes_;
};