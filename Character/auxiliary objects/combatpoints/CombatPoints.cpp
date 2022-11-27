
#include "CombatPoints.h"

CombatPoints::CombatPoints(): Points(_FL_)
{
	bDebugShow = false;
	bShow = true;
}

CombatPoints::~CombatPoints()
{
}

bool CombatPoints::Create(MOPReader & reader)
{
	EditMode_Update (reader);

	Registry(COMBAT_POINTS_GROUP);

	return true;
}

bool CombatPoints::EditMode_Update(MOPReader & reader)
{
	Points.DelAll();

	long PointsCount = reader.Array();

	for (long i=0;i<PointsCount;i++)
	{
		CombatPoint CPoint;

		CPoint.position = reader.Position();
		CPoint.NumAllyAttackers = 0;
		CPoint.NumEnemyAttackers = 0;
		CPoint.MaxAttackers = reader.Long();
		CPoint.radius = reader.Float();

		//CPoint.pEnemyChar = NULL;
		//CPoint.pAllyChar = NULL;

		CPoint.Chars.Empty();

		Points.Add(CPoint);
	}

	bShow = reader.Bool();

	bDebugShow = reader.Bool();;

	Activate(reader.Bool());	

	if (EditMode_IsOn() || bDebugShow)
	{
		SetUpdate((MOF_UPDATE)&CombatPoints::Realize, ML_ALPHA5);
	}
	else
	{
		DelUpdate((MOF_UPDATE)&CombatPoints::Realize);
	}	

	return true;
}

void CombatPoints::ShowDebug(bool show)
{
	bDebugShow = show;

	if (bDebugShow)
	{
		SetUpdate((MOF_UPDATE)&CombatPoints::Realize, ML_ALPHA5);
	}
	else
	{
		DelUpdate((MOF_UPDATE)&CombatPoints::Realize);
	}
}

void _cdecl CombatPoints::Realize(float fDeltaTime, long level)
{
	if(!EditMode_IsVisible()) return;
	if (!Mission().EditMode_IsAdditionalDraw()) return;	

	if ( (!EditMode_IsOn() && !bDebugShow) || !bShow ) return;

	for (dword i=0;i<Points.Size();i++)
	{
		dword color = 0xff00aaaa;

		bool AllActive = true;

		for (int j=0;j<(int)Points[i].Chars.Size();j++)
		{
			if (!Points[i].Chars[j]->IsActive()||!Points[i].Chars[j]->IsShow())
			{
				AllActive = false;

				break;
			}
		}

		if (!AllActive)
		{
			color = 0xffff0000;
		}
		else
		if (Points[i].NumAllyAttackers>0 && Points[i].NumEnemyAttackers>0)
		{
			color = 0xffffffff;
		}
		else
		if (Points[i].NumAllyAttackers>0)
		{
			color = 0xffffff00;
		}
		else
		if (Points[i].NumEnemyAttackers>0)
		{
			color = 0xffff00ff;
		}	
		else
		if (!IsActive())
		{
			color = 0xff0000ff;
		}		
			
		Render().DrawSphere(Points[i].position,Points[i].radius,color);
		//Render().Print(10,10+i*15,0xff00ffff,"%i %i %i",Points[i].NumAllyAttackers,Points[i].NumEnemyAttackers,Points[i].MaxAttackers);
		Render().Print(Points[i].position,20,1,0xffffffff,"%s - %i %i %i",GetObjectID().c_str(),Points[i].NumAllyAttackers,Points[i].NumEnemyAttackers,Points[i].MaxAttackers);

		for (int j=0;j<(int)Points[i].Chars.Size();j++)
		{
			Matrix mat;

			Points[i].Chars[j]->GetMatrix(mat);

			Render().DrawLine(mat.pos,0xff00ffff,Points[i].position,0xff00ffff);
		}
	}
}

void CombatPoints::Command(const char * id, dword numParams, const char ** params)
{
}

static const char * comment;
const char * CombatPoints::comment = 
"Combat Points";

MOP_BEGINLISTG(CombatPoints, "Combat Points", '1.00', 100, "Character objects")
	MOP_ARRAYBEG("Points", 0, 200)
		
		MOP_POSITION("Position", Vector(0.0f))		
		MOP_LONGEX("MaxAttacers", 1, 1,5)
		MOP_FLOATEX("Radius", 4, 1,50)		

	MOP_ARRAYEND	
	MOP_BOOL("Show", true)
	MOP_BOOL("DebugShow", false)
	MOP_BOOL("Active", true)
MOP_ENDLIST(CombatPoints)