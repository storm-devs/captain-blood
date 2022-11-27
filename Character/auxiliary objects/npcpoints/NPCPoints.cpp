
#include "NPCPoints.h"

NPCPoints::NPCPoints(): Points(_FL_)
{
	bDebugShow = false;
	bShow = true;
}

NPCPoints::~NPCPoints()
{
}

bool NPCPoints::Create(MOPReader & reader)
{
	EditMode_Update (reader);

	Registry(NPC_POINTS_GROUP);

	return true;
}

bool NPCPoints::EditMode_Update(MOPReader & reader)
{
	Points.DelAll();

	long PointsCount = reader.Array();

	for (long i=0;i<PointsCount;i++)
	{
		NPCPoint FPoint;

		FPoint.position = reader.Position();
		FPoint.num_chars = 0;

		Points.Add(FPoint);
	}

	bShow = reader.Bool();

	bDebugShow = reader.Bool();

	Activate(reader.Bool());	

	if (EditMode_IsOn() || bDebugShow)
	{
		SetUpdate((MOF_UPDATE)&NPCPoints::Realize, ML_ALPHA5);
	}
	else
	{
		DelUpdate((MOF_UPDATE)&NPCPoints::Realize);
	}	

	return true;
}

void NPCPoints::ShowDebug(bool show)
{
	bDebugShow = show;

	if (bDebugShow)
	{
		SetUpdate((MOF_UPDATE)&NPCPoints::Realize, ML_ALPHA5);
	}
	else
	{
		DelUpdate((MOF_UPDATE)&NPCPoints::Realize);
	}
}

int NPCPoints::GivePoint(int num_chars)
{
	int num_free_pt = 0;

	for (long i=0;i<(int)Points.Size();i++)
	{
		if (num_chars == Points[i].num_chars)
		{
			num_free_pt++;
		}
	}

	if (num_free_pt == 0) return -1;

	int start_index = (int)(Points.Size() * Rnd(0.999f));

	while (num_chars != Points[start_index].num_chars)
	{
		start_index++;
		
		if (start_index>=(int)Points.Size())
		{
			start_index = 0;
		}
	}

	return start_index;
}

MissionObject* NPCPoints::GetFrined(int index, MissionObject* initiator)
{
	if (Points[index].num_chars == 2)
	{
		if (Points[index].chr1 == initiator)
		{
			return Points[index].chr2;
		}

		if (Points[index].chr2 == initiator)
		{
			return Points[index].chr1;
		}
	}

	return null;
}

void NPCPoints::TakePoint(int index, bool take, MissionObject* invader)
{
	if (index<0 || index>=(int)Points.Size())
	{
		return;
	}

	if (take)
	{
		Points[index].num_chars++;
		if (Points[index].num_chars == 1)
		{
			Points[index].chr1 = invader;

		}
		else
		if (Points[index].num_chars == 2)
		{
			Points[index].chr2 = invader;
		}
	}
	else
	{
		if (Points[index].num_chars == 1)
		{
			Points[index].chr1 = null;
		}
		else
		if (Points[index].num_chars == 2)
		{
			Points[index].chr2 = null;
		}

		Points[index].num_chars--;

		Assert(Points[index].num_chars>=0);
	}
}

void _cdecl NPCPoints::Realize(float fDeltaTime, long level)
{
	if(!EditMode_IsVisible()) return;
	if (!Mission().EditMode_IsAdditionalDraw()) return;	

	if ( (!EditMode_IsOn() && !bDebugShow) || !bShow ) return;

	for (dword i=0;i<Points.Size();i++)
	{
		dword color = 0xff00aaaa;
						
		Render().DrawSphere(Points[i].position,0.5f,color);		
		
		if (Points[i].num_chars > 0)
		{
			Render().Print(Points[i].position,20,0,0xffffffff,"num - %i",Points[i].num_chars);
		}			
		
		if (!EditMode_IsOn())
		{
			if (Points[i].chr1)
			{
				Matrix mt;
				Points[i].chr1->GetMatrix(mt);

				Render().DrawLine(Points[i].position, 0xff00ff00, mt.pos, 0xff00ff00);
			}

			if (Points[i].chr2)
			{
				Matrix mt;
				Points[i].chr2->GetMatrix(mt);

				Render().DrawLine(Points[i].position, 0xff00ff00, mt.pos, 0xff00ff00);
			}
		}
	}
}

bool NPCPoints::IsPointReached(int index, Vector pos)
{	
	if (index < 0) return false;

	return ((Points[index].position - pos).GetLength() < 2.0f);
}

static const char * comment;
const char * NPCPoints::comment = 
"NPC Points";

MOP_BEGINLISTG(NPCPoints, "NPC Points", '1.00', 50, "Character objects")
	MOP_ARRAYBEG("Points", 0, 200)
		
		MOP_POSITION("Position", Vector(0.0f))		

	MOP_ARRAYEND	
	MOP_BOOL("Show", true)
	MOP_BOOL("DebugShow", false)
	MOP_BOOL("Active", true)
MOP_ENDLIST(NPCPoints)