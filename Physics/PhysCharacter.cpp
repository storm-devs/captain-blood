
#include "PhysCharacter.h"
#include "PhysicsScene.h"



class UserControllerHitReport : public NxUserControllerHitReport
{
public:



	virtual NxControllerAction  onShapeHit(const NxControllerShapeHit& hit)
	{
		
		//return NX_ACTION_NONE;


		if(hit.shape)
		{
			NxActor& actor = hit.shape->getActor();
			if(actor.isDynamic())
			{
				NxCollisionGroup group = hit.shape->getGroup();
				if(group == phys_ragdoll)
				{
					if(hit.dir.y < -0.8f)
					{
						NxVec3 vec = hit.dir*Clampf(hit.length, -0.01f, 0.01f);
						vec.x = 0.0f;
						vec.z = 0.0f;
						//actor.addForceAtLocalPos(vec, NxVec3(0,0,0), NX_IMPULSE);
						//actor.addForceAtLocalPos(vec, NxVec3(0,0,0), NX_FORCE);
					}
				}

				NxActor* ControllerActor = hit.controller->getActor();
				PhysCharacter* pChar = (PhysCharacter*)ControllerActor->userData;

				bool bKinematic = actor.readBodyFlag(NX_BF_KINEMATIC);

				if(group != phys_ragdoll && pChar->isApplyForce && !bKinematic)
				{
					NxVec3 vec = hit.dir* pChar->force;
					vec.y = 0.0f;

					NxActor& actor = hit.shape->getActor();
					actor.addForceAtLocalPos(vec, NxVec3(0,0,0), NX_FORCE );
				}
			}

		}

		return NX_ACTION_NONE;
	};

	virtual NxControllerAction  onControllerHit(const NxControllersHit& hit)
	{
//		NxVec3 p = hit.controller->getPosition();
//		NxVec3 e = hit.other->getPosition();



		return NX_ACTION_NONE;
	};


	static UserControllerHitReport report;
};

UserControllerHitReport UserControllerHitReport::report;



PhysCharacter::PhysCharacter(const char * filename, long fileline, float r, float h, IPhysicsScene * _scene) : 
	IPhysCharacter(_scene)
{
	SetFileLine(filename, fileline);

	controller = null;
	desc.radius = r;
	desc.height = 1.0f;
//#ifndef _XBOX
//	//desc.stepOffset = 0.015f;
//	//desc.skinWidth = 0.01f;
//	desc.stepOffset = 0.1f;
//	desc.skinWidth = 0.05f;
//#else
	desc.stepOffset = 0.23f;
	desc.skinWidth = 0.1f;
//#endif
	desc.upDirection = NX_Y;
	static float slopeLimit = cosf(NxMath::degToRad(65.0f));
	desc.slopeLimit = slopeLimit;
	desc.userData = this;
	desc.position = NxExtendedVec3(0.0f, 0.0f, 0.0f);
	desc.callback = &UserControllerHitReport::report;
	isApplyForce = false;
	force = 0.0f;
	move_collisionflags = 0;
	SetHeight(h);
	Activate(true);

	sceneChangedIndex = 0xFFFFFFFF;
}

PhysCharacter::~PhysCharacter()
{
	if(controller)
	{
		((PhysicsScene *)scene)->CtrManager().releaseController(*controller);
		controller = null;
	}
}

//Установить радиус персонажу
void PhysCharacter::SetRadius(float v)
{
	desc.radius = v;
	if(controller)
	{
		controller->setRadius(v);
	}	
}

//Получить радиус персонажа
float PhysCharacter::GetRadius()
{	
	return desc.radius;
}

//Установить высоту персонажу
void PhysCharacter::SetHeight(float v)
{
	float r = GetRadius();
	v -= r*2.0f;
	if(v < 0.0001f) v = 0.0001f;
	desc.height = v;
	if(controller)
	{
		controller->setHeight(v);
	}	
}

//Получить высоту персонажа
float PhysCharacter::GetHeight()
{
	return desc.height + desc.radius*2.0f;
}

//Установить позицию
void PhysCharacter::SetPosition(const Vector & pos)
{
	desc.position = Nxe(pos);
	desc.position.y += desc.skinWidth * 1.1f;

	if(controller)
	{
		controller->setPosition(desc.position);
	}	
}

//Получить позицию
Vector PhysCharacter::GetPosition()
{
	if(controller)
	{
		desc.position = controller->getFilteredPosition();
		desc.position.y -= desc.skinWidth;
	}
	return Nxe(desc.position);
}

//Передвинуть персонажа
dword PhysCharacter::Move(const Vector & move, dword collisionGroups)
{
	if(!controller) return 0;
	NxU32 flags;
	NxVec3 m = Nx(move);
	if(fabs(m.y) < 1e-35f)
	{
		if(m.y <= 0.0f)
		{
			m.y = -1e-20f;
		}else{	
			m.y = 1e-20f;
		}
	}

	dword newSceneChangedIndex = scene->GetSceneChangedIndex();
	if (newSceneChangedIndex != sceneChangedIndex)
	{
		sceneChangedIndex = newSceneChangedIndex;
		controller->reportSceneChanged(); 
	}

	controller->move(m, collisionGroups, 0.001f, flags, 1.0f);
	move_collisionflags = flags;

	/*IPhysicsScene::RaycastResult detail;
	IPhysBase* ragdoll = scene->Raycast(GetPosition(),GetPosition()+Vector(0,-desc.height-0.2f,0),phys_mask(phys_ragdoll),&detail);

	if (ragdoll)
	{		
		Vector velocity = move;		
		velocity *= 60.0f * 10.0f * 0.4f;
		velocity.y = 5.0f;

		//VANO изменил, тут баг, может быть IPhysMesh*, IPhysPlane* и так далее
		//((IPhysRagdoll*)ragdoll)->ApplyForce(velocity);

		if (ragdoll->UserNumber() == 666)
		{
			//((IPhysRagdoll*)ragdoll)->ApplyForce(velocity);
		}
	}*/

	return (dword)flags;
}

//Активировать/деактивировать персонажа
void PhysCharacter::Activate(bool isActive)
{	
	if(isActive)
	{
		if (!controller)
		{
			controller = (NxCapsuleController *)((PhysicsScene *)scene)->CtrManager().createController(&((PhysicsScene *)scene)->Scene(), desc);

			NxActor * controllerActor = controller->getActor();
			// VANO:: TEMP CHANGE	{
			if (controllerActor)
			{
			// VANO:: TEMP CHANGE	}
				controllerActor->userData = this;

				controllerActor->setGroup((NxActorGroup)phys_player);
				NxU32 count = controllerActor->getNbShapes();
				NxShape * const * shapes = controllerActor->getShapes();
				for(NxU32 i = 0; i < count; i++)
				{
					shapes[i]->setGroup((NxCollisionGroup)phys_playerctrl);
				}

				//desc.position.y += desc.skinWidth;
				controller->setPosition(desc.position);

				// коллизим контроллеры строго по маске
				controller->setInteraction(NXIF_INTERACTION_USE_FILTER);
			// VANO:: TEMP CHANGE	{
			}
			else
			{
				((PhysicsScene *)scene)->CtrManager().releaseController(*controller);
				controller = null;
			}
			// VANO:: TEMP CHANGE	}
		}
	}
	else
	{
		if(controller)
		{
			((PhysicsScene *)scene)->CtrManager().releaseController(*controller);
			controller = null;
		}
	}
}

//Если не 0.0f, то на все объекты при столкновении будет приложена сила v
void PhysCharacter::ApplyForceToObjects(float v)
{
	force = v;
	isApplyForce = (fabs(force) > 1e-30f);
}

//Возвращает коллижн флаги с последнего выполненого Move
dword PhysCharacter::GetMoveCollisionFlags()
{
	return dword(move_collisionflags);
}




