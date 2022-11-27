#include "FlyersCloud.h"

FlyersCloud:: FlyersCloud()
{
	pos = 0.0f;
	rad = 0.0f;

	count = 0;
	alpha = 0;

	cloud = null;
}

FlyersCloud::~FlyersCloud()
{
	if( cloud )
		cloud->Release(false);

	cloud = null;
}

bool FlyersCloud::Create(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

bool FlyersCloud::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void FlyersCloud::Show(bool isShow)
{
//	if( cloud )
//		cloud->Show(isShow);

	if( isShow != IsShow())
	{
		if( isShow )
		{
			Assert(cloud == null)

			cloud = FlyManager::CreateFlys(Mission(),rad,count,false);

			if( cloud )
			{
				cloud->SetPosition(pos);
				cloud->SetAlpha(alpha);
			}
		}
		else
		{
			if( cloud )
			{
				cloud->Release(false);

				cloud = null;
			}
		}
	}

	MissionObject::Show(isShow);
}

void FlyersCloud::InitParams(MOPReader &reader)
{
	Vector pos = reader.Position();
	float  rad = reader.Float();

	int	  count = reader.Long();
	float alpha = reader.Float();

	bool show = reader.Bool();

	if( cloud )
	{
		if( show )
		{
			if( rad	  != this->rad ||
				count != this->count )
			{
				cloud->Release(false);

				cloud = FlyManager::CreateFlys(Mission(),rad,count,false);
			}
		}
		else
		{
			cloud->Release(false);

			cloud = null;
		}
	}
	else
	{
		if( show )
		{
			cloud = FlyManager::CreateFlys(Mission(),rad,count,false);
		}
	}

	if( cloud )
	{
		cloud->SetPosition(pos);
		cloud->SetAlpha(alpha);
	}

	this->pos = pos;
	this->rad = rad;

	this->count = count;
	this->alpha = alpha;

	MissionObject::Show(show);
}

MOP_BEGINLISTCG(FlyersCloud, "Flyers cloud", '1.00', 100, "", "Default")

	MOP_POSITION("Position", 0.0f)
	MOP_FLOAT("Radius", 0.7f)
	MOP_LONGEX("Count", 200, 1, 2000)
	MOP_FLOATEX("Alpha", 1.0f, 0.0f, 1.0f)
	MOP_BOOL("Show", true)

MOP_ENDLIST(FlyersCloud)
