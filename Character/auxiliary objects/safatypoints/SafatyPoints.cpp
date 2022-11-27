
#include "SafatyPoints.h"

SaftyPoints::SaftyPoints(): Points(_FL_)
{	
	bShow = true;
}

SaftyPoints::~SaftyPoints()
{
}

bool SaftyPoints::Create(MOPReader & reader)
{
	Activate(true);
	EditMode_Update (reader);

	Registry(SAFTY_POINTS_GROUP);

	return true;
}

bool SaftyPoints::EditMode_Update(MOPReader & reader)
{
	Points.DelAll();

	long PointsCount = reader.Array();

	for (long i=0;i<PointsCount;i++)
	{
		SaftyPoint SPoint;

		SPoint.position = reader.Position();		
		SPoint.radius = reader.Float();

		Points.Add(SPoint);
	}

	if (EditMode_IsOn()) SetUpdate((MOF_UPDATE)&SaftyPoints::Realize, ML_ALPHA5);

	bShow = reader.Bool();

	Activate(reader.Bool());	

	return true;
}

void _cdecl SaftyPoints::Realize(float fDeltaTime, long level)
{
	if (!Mission().EditMode_IsAdditionalDraw()) return;
	
	if (!IsActive()) return;

	if (!EditMode_IsOn() || !bShow ) return;

	for (dword i=0;i<Points.Size();i++)
	{
		Render().DrawSphere(Points[i].position,Points[i].radius,0xff00aaaa);
		//Render().Print(10,10+i*15,0xff00ffff,"%i %i",Points[i].NumAllyAttackers,Points[i].NumEnemyAttackers);
	}
}

static const char * comment;
const char * SaftyPoints::comment = 
"Safaty Points";

MOP_BEGINLISTG(SaftyPoints, "Safty Points", '1.00', 100, "Character objects")
	MOP_ARRAYBEG("Points", 0, 200)
		
		MOP_POSITION("Position", Vector(0.0f))		
		MOP_FLOATEX("Radius", 4, 1,50)		

	MOP_ARRAYEND	
	MOP_BOOL("Show", true)
	MOP_BOOL("Active", true)
MOP_ENDLIST(SaftyPoints)