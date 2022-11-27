
#include "Barrel.h"
#include "..\..\character\character.h"
#include "..\..\character\components\characterlogic.h"
#include "..\..\character\components\characterphysics.h"
#include "..\..\character\components\characterAnimation.h"
#include "..\..\player\playerController.h"

void Barrel::InitData()
{
	model = NULL;
	iSelWay = 0;
	dir_changed = false;
	barrel_stage = 0;
	cur_way_pt = 0;

	fSpeed = 1.0f;
	distance = 0.0f;
	segment_lenght = 0.0f;

	angle = 0.0f;
}

bool Barrel::EditMode_Update(MOPReader & reader)
{	
	player = (Character*)Mission().Player();
	SetPlayerAnimEvents();

	ReadStdParams(reader);	
	
	IGMXScene* old_model = model;
	model = Geometry().CreateGMX( reader.String() , &Animation(), &Particles(), &Sound());	
	RELEASE(old_model);

	fSpeed = reader.Float();
	fAcceleration= reader.Float();

	if (model)
	{
		model->SetDynamicLightState(true);

		model->SetTransform(Matrix());
		
		const Vector & vMin = model->GetBound().vMin;
		const Vector & vMax = model->GetBound().vMax;
		
		Vector size = (vMax - vMin) * 0.5f;
		fRadius = size.y;
		center_matrix.pos = - vMin - size;
		center_matrix = center_matrix * Matrix().RotateY(PI*0.5f);		
	}

	num_ways = reader.Array();

	for (int i=0;i<num_ways;i++)
	{
		ways[i].num_points = reader.Array();

		for (int j=0;j<ways[i].num_points;j++)
		{
			ways[i].points[j] = reader.Position();
		}
	}
	
	return true;
}

void _cdecl Barrel::EditMode_Draw(float dltTime, long level)
{
	if (!Mission().EditMode_IsAdditionalDraw()) return;

	Render().DrawSphere(player_point,0.25f,0xffffaaff);
	Render().DrawSphere(position,radius,0xffaaccff);

	for (int i=0;i<num_ways;i++)
	{		
		Vector prev = position;

		for (int j=0;j<ways[i].num_points;j++)
		{
			Vector next = ways[i].points[j];

			Render().DrawLine(prev,0xff00ff00,next,0xff00ff00);

			prev = next;
		}
	}
}

void Barrel::BeginQuickEvent()
{
	cur_pl_offset = player->physics->GetPos();

	player->physics->Orient(ways[iSelWay].points[0]);

	if (player->animation) player->animation->Goto(startNode,0.0f);

	cur_way_pt = 0;
	iSelWay = 0;	
	distance = 0.0f;
	angle = 0.0f;

	player->logic->SetPairMode(true);

	QuickEvent::BeginQuickEvent();
}

void Barrel::QuickEventUpdate(float dltTime)
{
	cur_pl_offset.MoveByStep(player_point, dltTime*2.0f);	
	player->physics->SetPos(cur_pl_offset);	
	
	if (barrel_stage == 0)
	{
		player->physics->Orient(ways[iSelWay].points[0]);

		float dir = Controls().GetControlStateFloat("ChrLeftRight1");

		if (dir>0.7f && !dir_changed) 
		{
			iSelWay++;
	
			if (iSelWay>=num_ways)
			{
				iSelWay=0;
			}

			dir_changed = true;
		}
		else
		if (dir<-0.7f && !dir_changed) 
		{
			iSelWay--;

			if (iSelWay<0)
			{
				iSelWay=num_ways-1;
			}

			dir_changed = true;
		}
		else
		if (fabs(dir)<0.7f)
		{
			dir_changed = false;
		}

		if(Controls().GetControlStateType(buttonCode) == CST_ACTIVATED)
		{
			barrel_stage = 1;
			player->animation->ActivateLink("kick");
		}
	}
	else
	if (barrel_stage == 2)
	{
		if(Controls().GetControlStateType(buttonCode) == CST_ACTIVATED)
		{			
			Interupt(true);
			return;
		}

		fSpeed += fAcceleration * dltTime;

		if (fSpeed<0.25f)
		{
			fSpeed = 0.25f;
		}

		distance += fSpeed * dltTime;
		angle += fSpeed / fRadius * dltTime;

		if (distance>segment_lenght)
		{
			distance -= segment_lenght;

			cur_way_pt++;

			if (cur_way_pt>=ways[iSelWay].num_points)
			{				
				Interupt(true);
				return;
			}
			else
			{
				new_orient = (ways[iSelWay].points[cur_way_pt] - ways[iSelWay].points[cur_way_pt-1]);
				segment_lenght = new_orient.Normalize();				
			}
		}						

		orient.MoveByStep(new_orient,dltTime * 1.25f);

		
		transform.BuildOrient(orient,Vector(0,1.0f,0));
		
		if (cur_way_pt>0)
		{
			transform.pos = Vector().Lerp(ways[iSelWay].points[cur_way_pt-1], ways[iSelWay].points[cur_way_pt],distance/segment_lenght);			
		}
		else
		{
			transform.pos = Vector().Lerp(init_pos, ways[iSelWay].points[cur_way_pt],distance/segment_lenght);			
		}		

		transform = center_matrix * Matrix().RotateX(angle) * Matrix().RotateZ(sinf(angle * 2.75f) * 0.045f) * transform;
		transform.pos.y += fRadius;

		player->physics->Orient(transform.pos);

		player->arbiter->SplashDamage(player,transform.pos,1.0f,30,false);
	}
}

void Barrel::Interupt(bool win_game)
{
	if (!bQuickEventEnabled)
		return;

	//bQuickEventEnabled = false;
	barrel_stage = 0;
	
	player->logic->SetPairMode(false);

	if (win_game)
	{
		player->arbiter->Boom(player, htds_item, transform.pos, 3.5f, 25.0f, 1.4f);

		//Рождаем эффекты
		IParticleSystem * p = null;	
		
		player->Sound().Create3D("bmb_blast", transform.pos, _FL_);

		p = player->Particles().CreateParticleSystem("ExplosionBomb");
		if(p)
		{
			p->Teleport(Matrix(Vector(0.0f), transform.pos));
			p->AutoDelete(true);
		}

		player->animation->ActivateLink("shoot");
		Activate(false);
		Show(false);
	}
	else
	{
		LogicDebug("%s Interrupt Triger Activated",GetObjectType());
		triger_Interrupt.Activate(Mission(),false);
	}

	QuickEvent::Interupt(win_game);
}

void Barrel::AnimEvent(const char * param)
{	
	if (!bQuickEventEnabled) return;

	if (param[0] == 'k')
	{
		init_pos = transform.pos;
		barrel_stage = 2;
		orient = ways[iSelWay].points[cur_way_pt] - init_pos;
		segment_lenght = orient.Normalize();			
		new_orient = orient;
	}
}

static const char * comment;
const char * Barrel::comment = "Barrel Object. Set Player Start Node.";

MOP_BEGINLISTG(Barrel, "Barrel Object", '1.00', 150, "Quick Events")

	MOP_QE_STD

	MOP_STRING("Model", "")
	MOP_FLOATEX("Speed", 1.0,0.5f,64.0f)
	MOP_FLOATEX("Acceleration", 0.2f,-32.0f,32.0f)
	
	MOP_ARRAYBEG("Barrel Track", 1, 5);
		MOP_ARRAYBEG("Track", 1, 5);
			MOP_POSITION ("Position", Vector(0.0f));		
		MOP_ARRAYEND  
	MOP_ARRAYEND  

MOP_ENDLIST(Barrel)