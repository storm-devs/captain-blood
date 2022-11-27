#pragma once

#include "..\FlyManager\FlyManager.h"

class FlyersCloud : public MissionObject
{
public:

	 FlyersCloud();
	~FlyersCloud();

public:

	bool Create			(MOPReader &reader);
	bool EditMode_Update(MOPReader &reader);

	void Restart()
	{
		ReCreate();
	}

	void Show(bool isShow);

private:

	void InitParams		(MOPReader &reader);

private:

	Vector pos;
	float  rad;

	int	  count;
	float alpha;

	IFlysCloud *cloud;

};
