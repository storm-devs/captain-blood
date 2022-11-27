
#include "PitOfPain.h"
#include "..\..\Character\Components\CharacterLogic.h"
#include "..\..\Character\Components\CharacterPhysics.h"
#include "..\..\Character\Components\CharacterAnimationEvents.h"
#include "..\..\Character\Components\CharacterAnimation.h"



PitOfPain::PitOfPain():prisioners(_FL_,128)
{

}

//Инициализировать объект
bool PitOfPain::Create(MOPReader & reader)
{	
	arbiter = null;
	ptr_arbiter.Reset();

	InitParams(reader);
	
	Activate(reader.Bool());	

	if (EditMode_IsOn())	
	{
		SetUpdate(&PitOfPain::Draw, ML_DYNAMIC1);	
	}
	else
	{		
		DelUpdate(&PitOfPain::Draw);
	}

	
	return true;
}

//Обновить параметры
bool PitOfPain::EditMode_Update(MOPReader & reader)
{
	InitParams(reader);
	return true;
}

//Получить размеры описывающего ящика
void PitOfPain::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	min = zone.min + zone.mtx.pos;
	max = zone.max + zone.mtx.pos;
}

//Получить матрицу объекта
Matrix & PitOfPain::GetMatrix(Matrix & mtx)
{
	return (mtx = Matrix());
}

//Нарисовать модельку
void _cdecl PitOfPain::Draw(float dltTime, long level)
{
	if(!EditMode_IsVisible()) return;
	if (!Mission().EditMode_IsAdditionalDraw()) return;

	Render().DrawBox(zone.min, zone.max, zone.mtx, IsActive() ? 0xff00ff00 : 0xffff0000);
}

//Пересоздать объект
void PitOfPain::Restart()
{
	DelUpdate(&PitOfPain::Work);
	
	ReCreate();
}

void _cdecl PitOfPain::Work(float dltTime, long level)
{
	if (!ptr_arbiter.Validate())	
	{					
		static const ConstString objectId("CharactersArbiter");
		if (FindObject(objectId,ptr_arbiter))
		{
			arbiter = (CharactersArbiter*)ptr_arbiter.Ptr();
		}
		else
		{
			arbiter = null;
		}
	}

	if (arbiter)
	{
		for (int j=0;j<(int)prisioners.Size();j++)
		{
			if (!IsInside(prisioners[j]->physics->GetPos()))
			{
				prisioners.DelIndex(j);
				j--;
			}			
		}

		const array<Character*> & active_chars = arbiter->GetActiveCharacters();

		for (int i=0;i<(int)active_chars.Size();i++)
		{
			bool react = false;
			
			if (active_chars[i]->IsPlayer())
			{
				 if (bReactOnPlayer) react = true;
			}
			else
			{
				if (active_chars[i]->logic->GetSide() == CharacterLogic::s_ally && bReactOnAlly)
				{
					react = true;
				}
	
				if (active_chars[i]->logic->GetSide() == CharacterLogic::s_enemy && bReactOnEnemy)
				{
					react = true;
				}

				if (active_chars[i]->controller->IsStatist() == CharacterLogic::s_enemy && bReactOnStatist)
				{
					react = true;
				}
			}

			if (react)
			{
				for (int j=0;j<(int)prisioners.Size();j++)
				{
					if (active_chars[i] == prisioners[j])
					{
						react = false;
						break;
					}
				}
			}

			if (react)
			{							

				if (IsInside(active_chars[i]->physics->GetPos()))
				{
					active_chars[i]->logic->Hit(DamageReceiver::ds_trap_flame, fDamage,reaction,0.0f,fDamage,null,null,null);
					
					/*active_chars[i]->logic->SetHP(active_chars[i]->logic->GetHP() - fDamage);
					if (active_chars[i]->logic->GetHP()>0)
					{
						active_chars[i]->animation->ActivateLink(reaction,true);
					}*/

					prisioners.Add(active_chars[i]);
				}
			}
		}

	}
}

//Активировать/деактивировать объект
void PitOfPain::Activate(bool isActive)
{
	if (isActive)	
	{	
		SetUpdate(&PitOfPain::Work, ML_DYNAMIC1);		
	}
	else
	{
		DelUpdate(&PitOfPain::Work);
		prisioners.Empty();
	}

	MissionObject::Activate(isActive);	
}

//Проверить точку на попадание в зону
bool PitOfPain::IsInside(const Vector & pos)
{
	Vector p = zone.mtx.MulVertexByInverse(pos);
	
	if(zone.min <= p && p <= zone.max)
	{
		return true;
	}
	
	p = zone.mtx.MulVertexByInverse(pos + Vector(0.0f,1.0f,0.0f));

	if(zone.min <= p && p <= zone.max)
	{
		return true;
	}

	return false;
}


void PitOfPain::InitParams(MOPReader & reader)
{		
	zone.max.x = reader.Float()*0.5f;
	zone.max.y = reader.Float()*0.5f;
	zone.max.z = reader.Float()*0.5f;
	zone.min = -zone.max;
	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	zone.mtx.Build(ang, pos);

	fDamage = reader.Float();
	reaction = reader.String().c_str();

	bReactOnPlayer = reader.Bool();
	bReactOnAlly = reader.Bool();
	bReactOnEnemy = reader.Bool();
	bReactOnStatist = reader.Bool();	
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(PitOfPain, "Pit Of Pain", '1.00', 0, "Pit Of Pain", "Character objects")

	MOP_FLOATEX("Width", 3.0f, 0.01f, 1024.0f)
	MOP_FLOATEX("Height", 3.0f, 0.01f, 1024.0f)
	MOP_FLOATEX("Length", 0.5f, 0.01f, 1024.0f)
	MOP_POSITION("Position", Vector(0.0f))
	MOP_ANGLES("Angles", Vector(0.0f))

	MOP_FLOATEX("Damage", 10.0f, 0.0f, 1024.0f)
	MOP_STRING("Reaction", "Kick")

	MOP_BOOL("React on Player",true)
	MOP_BOOL("React on Ally",true)
	MOP_BOOL("React on Enemy",true)
	MOP_BOOL("React on Statist",true)
	
	MOP_BOOL("Active", true)	
MOP_ENDLIST(PitOfPain)


