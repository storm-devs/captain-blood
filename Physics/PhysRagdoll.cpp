

#include "PhysRagdoll.h"
#include "..\Common_h\data_swizzle.h"

//===========================================================================================================
//SaveData
//===========================================================================================================

#define PhysRagdollSaveDataId	"Storm Ragdoll > "
#define PhysRagdollSaveDataVer	"1.00"

//===========================================================================================================
//PhysBone
//===========================================================================================================

void norm_vec(float _vec[3])
{
	Vector & v = *(Vector*)&_vec[0];
	Assert(v.GetLength() > 0.0f);
	v.Normalize();
}

PhysRagdoll::PhysBone::PhysBone(PhysRagdoll & r, PhysBone * p) : child(_FL_), ragdoll(r)
{
	parent = p;
	actor = null;
	sphereJoint = null;
	revoluteJoint = null;
}

PhysRagdoll::PhysBone::~PhysBone()
{
	for(long i = 0; i < child; i++)
	{
		delete child[i];
	}
	child.DelAll();
	ReleaseActor();
}

bool PhysRagdoll::PhysBone::CheckId(PhysRaycastId id)
{
	if(id == this)
	{
		return true;
	}
	for(long i = 0; i < child; i++)
	{
		if(child[i]->CheckId(id))
		{
			return true;
		}	
	}
	return false;
}

PhysRagdoll::PhysBone * PhysRagdoll::PhysBone::Find(const char * bname, dword bhash)
{
	if(bhash == hash && name == bname)
	{
		return this;
	}
	for(long i = 0; i < child; i++)
	{
		PhysBone * ptr = child[i]->Find(bname, bhash);
		if(ptr)
		{
			return ptr;
		}
	}
	return null;
}

void PhysRagdoll::PhysBone::Activate(bool isActive)
{
	if(isActive)
	{
		if(actor && actor->isDynamic())
		{
			actor->clearActorFlag(NX_AF_DISABLE_COLLISION);
			actor->clearActorFlag(NX_AF_DISABLE_RESPONSE);
			actor->clearBodyFlag(NX_BF_FROZEN);
			actor->raiseBodyFlag(NX_BF_VISUALIZATION);
		}
	}else{
		if(actor && actor->isDynamic())
		{
			actor->raiseActorFlag(NX_AF_DISABLE_COLLISION);
			actor->raiseActorFlag(NX_AF_DISABLE_RESPONSE);
			actor->raiseBodyFlag(NX_BF_FROZEN);
			actor->clearBodyFlag(NX_BF_VISUALIZATION);
		}
	}
	for(long i = 0; i < child; i++)
	{
		child[i]->Activate(isActive);
	}
}

//Представить кость ящиком
void PhysRagdoll::PhysBone::SetShape(const Matrix & worldTransform, const Matrix & localTransform, const Vector & size, float mass)
{
	ReleaseActor();
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions.set(size.x*0.5f, size.y*0.5f, size.z*0.5f);
	Nx(boxDesc.localPose, localTransform);
	boxDesc.userData = this;
	
	boxDesc.group = phys_ragdoll;
	//boxDesc.group = phys_pair;

	actorDesc.shapes.pushBack(&boxDesc);
	actorDesc.body = &bodyDesc;
	Nx(actorDesc.globalPose, worldTransform);
	actorDesc.userData = &ragdoll;
	
	//actorDesc.group = phys_ragdoll;
	actorDesc.group = phys_pair;
	
	actorDesc.density = 10.0f;
	actor = ragdoll.Scene().createActor(actorDesc);
	actor->setMaxAngularVelocity(100.0f);
	Assert(actor);
	ragdoll.SetBoneShape(this, worldTransform, localTransform, size, mass);
}

//Представить кость капсулой
void PhysRagdoll::PhysBone::SetShape(const Matrix & worldTransform, const Matrix & localTransform, float height, float radius, float mass)
{
	ReleaseActor();
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	NxCapsuleShapeDesc capsuleDesc;
	capsuleDesc.radius = radius;
	capsuleDesc.height = height;
	Nx(capsuleDesc.localPose, localTransform);
	capsuleDesc.userData = this;
		
	capsuleDesc.group = phys_ragdoll;
	//capsuleDesc.group = phys_pair;


	actorDesc.shapes.pushBack(&capsuleDesc);
	actorDesc.body = &bodyDesc;
	Nx(actorDesc.globalPose, worldTransform);
	actorDesc.userData = &ragdoll;	
	
	//actorDesc.group = phys_ragdoll;
	actorDesc.group = phys_pair;
	
	actorDesc.density = 10.0f;
	actor = ragdoll.Scene().createActor(actorDesc);
	actor->setMaxAngularVelocity(100.0f);
	Assert(actor);
	ragdoll.SetBoneShape(this, worldTransform, localTransform, height, radius, mass);
}

//Получить родителя
PhysRagdoll::IBone * PhysRagdoll::PhysBone::GetParent()
{
	return parent;
}

//Получить количество детей
long PhysRagdoll::PhysBone::ChildCount()
{
	return child;
}

//Получить ребёнка
PhysRagdoll::IBone & PhysRagdoll::PhysBone::GetChild(long index)
{
	return *child[index];
}

//Добавить ребёнка
PhysRagdoll::IBone & PhysRagdoll::PhysBone::AddChild()
{
	//Добавляем кость
	PhysBone * c = NEW PhysBone(ragdoll, this);
	ragdoll.AddBone(c);
	return *child[child.Add(c)];
}

//Установить имя кости
void PhysRagdoll::PhysBone::SetBoneName(const char * name)
{
	this->name = name;
	hash = string::HashNoCase(name);
}

//Установить имя кости
const char * PhysRagdoll::PhysBone::GetBoneName()
{
	return name;
}

//Создать сферический сустав между текущей костью и ребёнком ребёнком
void PhysRagdoll::PhysBone::CreateJoint(IBone & child, const SphericalJointParams & params)
{
	PhysBone & chl = (PhysBone &)child;
	Assert(actor);
	Assert(chl.actor);
	chl.ReleaseJoint();
	NxSphericalJointDesc sphericalDesc;
	sphericalDesc.actor[0] = actor;
	sphericalDesc.actor[1] = chl.actor;
	sphericalDesc.setGlobalAnchor(Nx(params.worldJointPosition));
	Matrix mtx;
	//Nx(mtx, actor->getGlobalPose());	
	//sphericalDesc.setGlobalAxis(Nx(/*mtx* */params.swingAxisInParentSystem));	
	
	Nx(mtx, actor->getGlobalPose());
	sphericalDesc.localAxis[0] = Nx(mtx.MulNormalByInverse(params.swingAxisInParentSystem));
	sphericalDesc.localNormal[0] = Nx(mtx.MulNormalByInverse(params.twistAxisInParentSystem));
	Nx(mtx, chl.actor->getGlobalPose());	
	sphericalDesc.localAxis[1] = Nx(mtx.MulNormalByInverse(params.swingAxisInParentSystem));
	sphericalDesc.localNormal[1] = Nx(mtx.MulNormalByInverse(params.twistAxisInParentSystem));

	//sphericalDesc.swingAxis = Nx(mtx*params.swingAxisInParentSystem);

	//Параметры отклонения
	sphericalDesc.flags |= NX_SJF_SWING_LIMIT_ENABLED | NX_SJF_SWING_SPRING_ENABLED;
	Assert(params.swingLimit >= 0.0f);
	Assert(params.swingLimit <= PI);
	sphericalDesc.swingLimit.value = (NxReal)params.swingLimit;
	sphericalDesc.swingSpring.spring = (NxReal)params.swingSpring;
	sphericalDesc.swingSpring.damper = (NxReal)params.swingDamper;
	sphericalDesc.swingSpring.targetValue = 0;
	//Прараметры скручивания
	sphericalDesc.flags |= NX_SJF_TWIST_LIMIT_ENABLED | NX_SJF_TWIST_SPRING_ENABLED;	
	Assert(params.twistMin >= -PI);
	Assert(params.twistMin <= PI);
	Assert(params.twistMax >= -PI);
	Assert(params.twistMax <= PI);
	Assert(params.twistMin < params.twistMax);
	sphericalDesc.twistLimit.low.value = (NxReal)params.twistMin;
	sphericalDesc.twistLimit.high.value = (NxReal)params.twistMax;
	sphericalDesc.twistSpring.spring = (NxReal)params.twistSpring;
	sphericalDesc.twistSpring.damper = (NxReal)params.twistDamper;
	sphericalDesc.twistSpring.targetValue = 0;
	//Создаём кость
	chl.sphereJoint = (NxSphericalJoint *)ragdoll.Scene().createJoint(sphericalDesc);
	Assert(chl.sphereJoint);
	ragdoll.SetBoneJoint(&chl, params);
}

//Создать шарнирный сустав между текущей костью и ребёнком ребёнком
void PhysRagdoll::PhysBone::CreateJoint(IBone & child, const RevoluteJointParams & params)
{
	PhysBone & chl = (PhysBone &)child;
	Assert(actor);
	Assert(chl.actor);
	chl.ReleaseJoint();
	NxRevoluteJointDesc revDesc;
	revDesc.actor[0] = actor;
	revDesc.actor[1] = chl.actor;
	revDesc.setGlobalAnchor(Nx(params.worldJointPosition));
	
	Matrix mtx;
	//Nx(mtx, actor->getGlobalPose());
		
	//revDesc.setGlobalAxis(Nx( /*mtx* */ params.axisInParentSystem));

	//Пока так, пофиксят - переделаю
	//Vector up = params.axisInParentSystem ^ params.normalInParentSystem;
	//Vector normal = up ^ params.axisInParentSystem;
	//revDesc.localNormal[0] = revDesc.localNormal[1] = Nx(normal);

	//mtx.Inverse();

	Nx(mtx, actor->getGlobalPose());
	revDesc.localAxis[0]   = Nx(mtx.MulNormalByInverse(params.axisInParentSystem));
	revDesc.localNormal[0] = Nx(mtx.MulNormalByInverse(params.normalInParentSystem));
	Nx(mtx, chl.actor->getGlobalPose());
	revDesc.localAxis[1]   = Nx(mtx.MulNormalByInverse(params.axisInParentSystem));
	revDesc.localNormal[1] = Nx(mtx.MulNormalByInverse(params.normalInParentSystem));
		

	//Лимиты
	Assert(params.minAngle >= -PI);
	Assert(params.minAngle <= PI);
	Assert(params.maxAngle >= -PI);
	Assert(params.maxAngle <= PI);
	Assert(params.minAngle < params.maxAngle);
	revDesc.flags |= NX_RJF_LIMIT_ENABLED;
	revDesc.limit.low.value = (NxReal)params.minAngle;
	revDesc.limit.high.value = (NxReal)params.maxAngle;
	//Силы
	/*revDesc.flags |= NX_RJF_SPRING_ENABLED;
	revDesc.spring.spring = (NxReal)params.spring;
	revDesc.spring.damper = (NxReal)params.damper;
	revDesc.spring.targetValue = 0;*/
	//Создаём кость
	chl.revoluteJoint = (NxRevoluteJoint *)ragdoll.Scene().createJoint(revDesc);
	Assert(chl.revoluteJoint);
	ragdoll.SetBoneJoint(&chl, params);
}

//Устоновить кости позицию в мире
void PhysRagdoll::PhysBone::SetWorldTransform(const Matrix & mtx)
{
	if(actor)
	{
		actor->setGlobalPose(Nx(NxMat34(), mtx));
	}
}

//Получить мировые координаты кости
void PhysRagdoll::PhysBone::GetWorldTransform(Matrix & mtx)
{
	if(actor)
	{
		Nx(mtx, actor->getGlobalPose());		
	}else{
		mtx.SetIdentity();
	}
}

//Приложить силу к кости в заданной локальной точке
void PhysRagdoll::PhysBone::ApplyForce(const Vector & force, const Vector & localPosition)
{
#ifndef STOP_ASSERTS
	if (_isnan(force.x) || _isnan(force.y) || _isnan(force.z))
	{
		api->Trace("PhysBone::ApplyForce force: %.3f, %.3f, %.3f [%s, %d] ", force.x, force.y, force.z, ragdoll.GetFileName(), ragdoll.GetFileLine());
		Assert(false);
	}
#endif

	if(actor)
	{
		actor->addForceAtLocalPos(Nx(force), Nx(localPosition), NX_FORCE);
	}
}

//Приложить импульс к кости в заданной локальной точке
void PhysRagdoll::PhysBone::ApplyImpulse(const Vector & imp, const Vector & localPosition)
{
#ifndef STOP_ASSERTS
	if (_isnan(imp.x) || _isnan(imp.y) || _isnan(imp.z) || fabsf(imp.x) > 100000.0f || fabsf(imp.y) > 100000.0f || fabsf(imp.z) > 100000.0f)
	{
		api->Trace("PhysBone::ApplyImpulse force: %.3f, %.3f, %.3f [%s, %d] ", imp.x, imp.y, imp.z, ragdoll.GetFileName(), ragdoll.GetFileLine());
		Assert(false);
	}
#endif

	if(actor)
	{
		actor->addForceAtLocalPos(Nx(imp), Nx(localPosition), NX_IMPULSE);
	}
}

//Приложить вращающий момент к рутовой кости в глобальном системе
inline void PhysRagdoll::PhysBone::ApplyTorque(const Vector & imp)
{
#ifndef STOP_ASSERTS
	if (_isnan(imp.x) || _isnan(imp.y) || _isnan(imp.z))
	{
		api->Trace("PhysBone::ApplyTorque force: %.3f, %.3f, %.3f [%s, %d] ", imp.x, imp.y, imp.z, ragdoll.GetFileName(), ragdoll.GetFileLine());
		Assert(false);
	}
#endif

	if (actor) 
	{
		actor->addTorque(Nx(imp), NX_IMPULSE);
	}
}

//Приложить вращающий момент к рутовой кости в локальной системе
inline void PhysRagdoll::PhysBone::ApplyLocalTorque(const Vector & imp)
{
#ifndef STOP_ASSERTS
	if (_isnan(imp.x) || _isnan(imp.y) || _isnan(imp.z))
	{
		api->Trace("PhysBone::ApplyLocalTorque force: %.3f, %.3f, %.3f [%s, %d] ", imp.x, imp.y, imp.z, ragdoll.GetFileName(), ragdoll.GetFileLine());
		Assert(false);
	}
#endif

	if (actor) 
	{
		actor->addLocalTorque(Nx(imp), NX_IMPULSE);
	}
}

//Установить линейную скорость
void PhysRagdoll::PhysBone::SetVelocity(const Vector & v, bool isRecursive)
{
	if(actor)
	{
		actor->setLinearVelocity(Nx(v));
	}
	if(isRecursive)
	{
		for(long i = 0; i < child; i++)
		{
			child[i]->SetVelocity(v, true);
		}
	}
}

//Установить скорость вращения		
void PhysRagdoll::PhysBone::SetRotation(float ay)
{
	if(actor)
	{
		actor->setAngularVelocity(NxVec3(0.0f, ay, 0.0f));
	}
}

//Изменить групппу
void PhysRagdoll::PhysBone::SetGroup(PhysicsCollisionGroup group)
{
	if(actor)
	{
		actor->setGroup(group);

		int NumShapes = actor->getNbShapes (); 

		if (NumShapes>0)
		{		
			NxShape *const* shapes = actor->getShapes();

			for(int i = 0; i < NumShapes; i++)
			{
				shapes[i]->setGroup(group);
			}
		}
	}


	for(long i = 0; i < child; i++)
	{
		child[i]->SetGroup(group);
	}
}

//Удалить коллижен шейп
void PhysRagdoll::PhysBone::ReleaseActor()
{
	ReleaseJoint();
	for(long i = 0; i < child; i++)
	{
		child[i]->ReleaseJoint();
	}
	if(actor)
	{
		ragdoll.Scene().releaseActor(*actor);
	}	
}


//Убить суставы у себя
void PhysRagdoll::PhysBone::ReleaseJoint()
{
	if(sphereJoint)
	{
		ragdoll.Scene().releaseJoint(*sphereJoint);
		sphereJoint = null;
	}
	if(revoluteJoint)
	{
		ragdoll.Scene().releaseJoint(*revoluteJoint);
		revoluteJoint = null;
	}
}


//===========================================================================================================
//BlendStage
//===========================================================================================================


PhysRagdoll::BlendStage::BlendStage(PhysRagdoll & r, IAnimation * ani) : 
	ragdoll(r), 
	bones(_FL_, 1)
{
	Assert(ani);
	SetAnimation(ani);
	level = 100;
	currentBlend = 0.0f;
	dltBlend = 0.0f;
	long numBones = ani->GetNumBones();
	bones.AddElements(numBones);
	for(long i = 0; i < numBones; i++)
	{
		BSBone & b = bones[i];
		const char * boneName = ani->GetBoneName(i);
		dword hash = string::HashNoCase(boneName);
		b.physBone = ragdoll.root->Find(boneName, hash);
		if(b.physBone)
		{
			b.position = ani->GetBonePosition(i);
			b.rotation = ani->GetBoneRotate(i);
		}
	}
	isActive = false;
//	isSetFreezed = false;
}


PhysRagdoll::BlendStage::~BlendStage()
{
	UnregistryBlendStage();
}


//Обновить текущий уровень
void PhysRagdoll::BlendStage::UpdateLevel(dword lvl)
{
	if(level == lvl)
	{
		return;
	}
	level = lvl;
	if(isActive)
	{
		UnregistryBlendStage();
		RegistryBlendStage(level);
	}
}

//Получить коэфициент блендинга для интересующей кости
float PhysRagdoll::BlendStage::GetBoneBlend(long boneIndex)
{
	if(isActive)
	{
		if(bones[boneIndex].physBone)
		{
			return currentBlend;
		}
	}
	return 0.0f;
}

void PhysRagdoll::BlendStage::Freeze()
{
	/*
	Quaternion	rot, old_rot;
	Vector		pos, old_pos;
	Vector		scale, old_scale;

	isSetFreezed = true;
	// Получаем последние матрицы для всех костей
	for (long i=0; i<bones.Len(); i++)
	{
		GetBoneTransform(i, rot, pos, scale, old_rot, old_pos, old_scale);
	}
	isSetFreezed = false;
	*/
}

//Получить трансформацию кости
void PhysRagdoll::BlendStage::GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & pfrevScale)
{
	//Кость
	BSBone & bone = bones[boneIndex];
	if(!bone.physBone)
	{
		return;
	}

	if (!ragdoll.IsFreezed())
	{
		//Вычисляем матрицу кости
		Matrix mtx;
		IBone * parent = bone.physBone->GetParent();
		if(parent)
		{
			//Получаем матрицу в системе родителя регдола
			Matrix cmtx, tmp;
			parent->GetWorldTransform(tmp);
			bone.physBone->GetWorldTransform(cmtx);
			mtx.EqMultiply(cmtx, tmp.Inverse());
			//Компенсируем смещение анимационной иерархией при несовпадении
			long parentIndex = Ani()->GetBoneParent(boneIndex);
			if(parentIndex >= 0 && !bones[parentIndex].physBone)
			{
				Matrix correct;
				while(true)
				{
					//Дополняем трансформацию родительской
					const Quaternion & rot = Ani()->GetBoneRotate(parentIndex);
					rot.GetMatrix(tmp);
					tmp.pos = Ani()->GetBonePosition(parentIndex);				
					correct = correct*tmp;
					//Перемещаемся на следующего родителя
					parentIndex = Ani()->GetBoneParent(parentIndex);
					if(parentIndex < 0) break;
					if(bones[parentIndex].physBone) break;
				}
				//Вычетаем смещение
				if(parentIndex >= 0)
				{
					correct.Inverse();
					mtx = mtx*correct;
				}
			}
		}else{
			bone.physBone->GetWorldTransform(mtx);
			mtx = mtx*iworld;
		}
		//Получаем параметры
		rotation.Set(mtx);
		position = mtx.pos;
		scale = 1.0f;
		bone.position = position;
		bone.rotation = rotation;
	}
	else
	{
		//Если заморожен рэкдол, то вернём то что есть
		rotation = bone.rotation;
		position = bone.position;
		scale = 1.0f;
		return;
	}
}

//Обновить
void PhysRagdoll::BlendStage::Update(float dltTime)
{
	currentBlend += dltBlend*dltTime;
	if(currentBlend <= 0.0f)
	{
		if(isActive)
		{
			UnregistryBlendStage();
			isActive = false;
			ragdoll.UpdateActiveState();
		}
		currentBlend = 0.0f;
	}else{
		if(currentBlend > 1.0f)
		{
			currentBlend = 1.0f;
		}
	}
}

//Установить знак и время блендинга
void PhysRagdoll::BlendStage::SetBlendTime(float time, const Matrix & transform)
{
	dltBlend = 1.0f/time;
	if(!isActive)
	{
		if(dltBlend > 0.0f)
		{
			RegistryBlendStage(level);
			isActive = true;
			//Инициализируем позиции костей
			iworld = transform;
			iworld.Inverse();
			const Matrix * m = Ani()->GetBoneMatrices();
			for(long i = 0; i < bones; i++)
			{
				PhysBone * bone = bones[i].physBone;
				if(!bone) continue;
				Matrix world(m[i], transform);
				bone->SetWorldTransform(world);
			}
			ragdoll.UpdateActiveState();
		}
	}
}

//Текущие состояние стадии
bool PhysRagdoll::BlendStage::IsActive()
{
	return isActive;
}

//===========================================================================================================
//SaveDataInfo
//===========================================================================================================

void PhysRagdoll::SaveDataInfo::Init()
{
	bone = null;
	boxData = null;
	capsuleData = null;
	sphericalJoint = null;
	revoluteJoint = null;
}

void PhysRagdoll::SaveDataInfo::Release()
{
	bone = null;
	if(boxData) delete boxData; boxData = null;
	if(capsuleData) delete capsuleData; capsuleData = null;
	if(sphericalJoint) delete sphericalJoint; sphericalJoint = null;
	if(revoluteJoint) delete revoluteJoint; revoluteJoint = null;
}


//===========================================================================================================
//PhysRagdoll
//===========================================================================================================

PhysRagdoll::PhysRagdoll(IPhysBase * proxy, const char * filename, long fileline, IPhysicsScene * _scene) : IPhysEditableRagdoll(_scene), stages(_FL_)
{
	SetFileLine(filename, fileline);
	SetProxyObject(proxy);

	isFreezed = false;
	isActive = true;
	root = NEW PhysBone(*this, null);
	bones = NEW array<SaveDataInfo>(_FL_);
	AddBone(root);
}

PhysRagdoll::PhysRagdoll(IPhysBase * proxy, const char * filename, long fileline, const void * data, dword size, IPhysicsScene * _scene) : IPhysEditableRagdoll(_scene), stages(_FL_)
{
	SetFileLine(filename, fileline);
	SetProxyObject(proxy);

	isFreezed = false;
	isActive = true;
	root = NEW PhysBone(*this, null);
	bones = null;
	array<IBone *> bonesList(_FL_, 16);
	try
	{
		dword current = 0;
		//Заголовок
		const SaveDataHeader & hdr = GetSaveData<SaveDataHeader>(current, data, size);
		const char * id = PhysRagdollSaveDataId;
		const char * ver = PhysRagdollSaveDataVer;
		for(long i = 0; i < sizeof(hdr.id); i++)
		{
			if(hdr.id[i] != id[i]) throw "Invalidate data Id";
		}
		for(long i = 0; i < sizeof(hdr.ver); i++)
		{
			if(hdr.ver[i] != ver[i]) throw "Invalidate data version";
		}			
		dword bonesCount = SwizzleDWord(hdr.bonesCount);
		//Читаем кости
		for(dword i = 0; i < bonesCount; i++)
		{
			//Описание кости
			const SaveDataBone & boneData = GetSaveData<SaveDataBone>(current, data, size);
			//Воздаём кость
			long parentIndex = SwizzleShort(boneData.parent);
			IBone * bone = null;
			if(parentIndex >= 0)
			{
				if(parentIndex >= bonesList)
				{
					throw "Invalidate bone parent index";
				}
				bone = &bonesList[parentIndex]->AddChild();
			}else{
				if(bonesList != 0)
				{
					throw "Invalidate bone parent index";
				}
				bone = root;
			}
			bonesList.Add(bone);
			//Устанавливаем фигуру кости
			if(boneData.shapeType == SaveDataBone::st_box)
			{
				CreateBoxFromSaveData(bone, GetSaveData<SaveDataBox>(current, data, size));
			}else
			if(boneData.shapeType == SaveDataBone::st_capsule)
			{
				CreateCapsuleFromSaveData(bone, GetSaveData<SaveDataCapsule>(current, data, size));
			}else{
				throw "Invalidate bone shape type";
			}
			if(parentIndex >= 0)
			{
				//Устанавливаем соединение
				if(boneData.jointType == SaveDataBone::jt_spherical)
				{
					CreateSphericalJointFromSaveData(bone, GetSaveData<SaveDataSphericalJoint>(current, data, size));
				}else
				if(boneData.jointType == SaveDataBone::jt_revolute)
				{
					CreateRevoluteJointFromSaveData(bone, GetSaveData<SaveDataRevoluteJoint>(current, data, size));
				}else{
					throw "Invalidate bone joint type";
				}
			}
			//Зачитываем имя кости
			dword nameLen = SwizzleDWord(boneData.namelength);
			const char * name = (const char *)GetSaveData(current, data, size, nameLen);
			if(name[nameLen - 1] != 0)
			{
				throw "Invalidate bone name data";
			}
			bone->SetBoneName(name);
		}
		Activate(false);
	}catch(const char * err){
		api->Trace("Ragdoll error: can't load data -> %s", err);
		delete root;
		root = null;
	}
}

PhysRagdoll::~PhysRagdoll()
{
	for(long i = 0; i < stages; i++)
	{
		delete stages[i];
	}
	stages.Empty();
	delete root;
	root = null;
	if(bones)
	{
		for(long i = 0; i < *bones; i++)
		{
			(*bones)[i].Release();
		}
		delete bones;
		bones = null;
	}
}

void PhysRagdoll::Freeze()
{
	if (IsFreezed()) return;

	for(long i = 0; i < stages; i++)
		stages[i]->Freeze();

	isFreezed = true;
	
	delete root;
	root = null;
}

bool PhysRagdoll::IsFreezed()
{
	return isFreezed;
}

//Очистить для создания нового
void PhysRagdoll::Clear()
{
	isFreezed = false;
	for(long i = 0; i < stages; i++)
	{
		delete stages[i];
	}
	stages.Empty();
	if (root) delete root;
	root = NEW PhysBone(*this, null);
	AddBone(root);
	if(bones)
	{
		for(long i = 0; i < *bones; i++)
		{
			(*bones)[i].Release();
		}
		bones->Empty();
		AddBone(root);
	}
}

//Получить рутовую кость
PhysRagdoll::IBone & PhysRagdoll::GetRootBone()
{
	return *root;
}

//Получить кость по идентификатору рэйкаста
PhysRagdoll::IBone * PhysRagdoll::GetBone(PhysRaycastId id)
{
	Assert(root->CheckId(id));
	return (IBone *)id;
}

//Получить сцену, которой принадлежим
NxScene & PhysRagdoll::Scene()
{
	return ((PhysicsScene *)scene)->Scene();	
}

//Установить блендер для анимацию
void PhysRagdoll::SetBlendStage(IAnimation * ani, dword level)
{
	for(long i = 0; i < stages; i++)
	{
		if(stages[i]->Ani() == ani)
		{
			stages[i]->UpdateLevel(level);
			return;
		}
	}
	BlendStage * bs = NEW BlendStage(*this, ani);
	bs->UpdateLevel(level);
	stages.Add(bs);
}

//Удалить блендер для анимации
void PhysRagdoll::RemoveBlendStage(IAnimation * ani)
{
	for(long i = 0; i < stages; i++)
	{
		if(stages[i]->Ani() == ani)
		{
			delete stages[i];
			stages.DelIndex(i);
			return;
		}
	}
}

//Включить блэндер
void PhysRagdoll::Activate(float blendTime, const Matrix & transform)
{
	if(blendTime < 0.001f) blendTime = 0.001f;
	for(long i = 0; i < stages; i++)
	{
		stages[i]->SetBlendTime(blendTime, transform);
	}
}

//Выключить блэндер
void PhysRagdoll::Deactivate(float blendTime, Matrix & transform)
{
	if(blendTime < 0.001f) blendTime = 0.001f;
	blendTime = -blendTime;
	for(long i = 0; i < stages; i++)
	{
		stages[i]->SetBlendTime(blendTime, transform);
	}
}

//Установить линейную скорость
void PhysRagdoll::SetVelocity(const Vector & vel, bool isRecursive)
{
	if(root) root->SetVelocity(vel, isRecursive);
}

//Установить угловую скорость
void PhysRagdoll::SetRotation(float ay)
{
	if(root) root->SetRotation(ay);
}

//Применить силу к рутовой кости
void PhysRagdoll::ApplyForce(const Vector & force)
{
	if(root) root->ApplyForce(force, Vector(0.0f));
}

//Применить импульс к рутовой кости
void PhysRagdoll::ApplyImpulse(const Vector & imp)
{
	if(root) root->ApplyImpulse(imp, Vector(0.0f));
}

//Приложить вращающий момент к рутовой кости в глобальном системе
inline void PhysRagdoll::ApplyTorque(const Vector & imp)
{
	if (root) root->ApplyTorque(imp);
}

//Приложить вращающий момент к рутовой кости в локальной системе
inline void PhysRagdoll::ApplyLocalTorque(const Vector & imp)
{
	if (root) root->ApplyLocalTorque(imp);
}

//Изменить групппу
void PhysRagdoll::SetGroup(PhysicsCollisionGroup group)
{
	if(root) root->SetGroup(group);
}

//Сохранить параметры рэгдола в массив
bool PhysRagdoll::BuildSaveData(array<byte> & buffer)
{
	buffer.Empty();
	if(!bones) return false;
	//Заголовок
	SaveDataHeader header;
	memcpy(header.id, PhysRagdollSaveDataId, sizeof(header.id));
	memcpy(header.ver, PhysRagdollSaveDataVer, sizeof(header.ver));
	header.bonesCount = bones->Size();
	AddSaveData(buffer, &header, sizeof(header));
	//Кости
	for(long i = 0; i < *bones; i++)
	{
		//Кость
		SaveDataInfo & di = (*bones)[i];
		IBone * parent = di.bone->GetParent();
		long parentIndex = -1;
		if(parent)
		{
			for(parentIndex = 0; parentIndex < i; parentIndex++)
			{
				if((*bones)[parentIndex].bone == parent)
				{
					break;
				}
			}
			Assert(parentIndex < i);
		}
		const char * boneName = ((PhysBone *)di.bone)->GetBoneName();
		//Заголовок кости
		SaveDataBone bone;
		bone.parent = SwizzleShort(short(parentIndex));		
		bone.shapeType = di.boxData ? SaveDataBone::st_box : SaveDataBone::st_capsule;
		if(di.bone->GetParent())
		{
			bone.jointType = di.sphericalJoint ? SaveDataBone::jt_spherical : SaveDataBone::jt_revolute;
		}else{
			bone.jointType = SaveDataBone::jt_unkown;
		}		
		bone.namelength = SwizzleDWord((dword)strlen(boneName) + 1);
		AddSaveData(buffer, &bone, sizeof(bone));
		//Сохраняем данные кости		
		if(di.boxData)
		{
			AddSaveData(buffer, di.boxData, sizeof(SaveDataBox));
		}else
		if(di.capsuleData)
		{
			AddSaveData(buffer, di.capsuleData, sizeof(SaveDataCapsule));
		}else{
			buffer.Empty();
			api->Trace("Ragdoll error: can't write data -> not all bones have shapes");
			return false;
		}
		if(di.sphericalJoint)
		{
			norm_vec(di.sphericalJoint->swingAxisInParentSystem);
			norm_vec(di.sphericalJoint->twistAxisInParentSystem);

			AddSaveData(buffer, di.sphericalJoint, sizeof(SaveDataSphericalJoint));
		}else
		if(di.revoluteJoint)
		{
			norm_vec(di.revoluteJoint->axisInParentSystem);
			norm_vec(di.revoluteJoint->normalInParentSystem);

			AddSaveData(buffer, di.revoluteJoint, sizeof(SaveDataRevoluteJoint));
		}else
		if(di.bone->GetParent())
		{
			buffer.Empty();
			api->Trace("Ragdoll error: can't write data -> not all bones have joines");
			return false;
		}
		AddSaveData(buffer, boneName, bone.namelength);
	}
	return true;
}

//Загрузился ли регдол
bool PhysRagdoll::IsLoaded()
{
	return root != null;
}

//Добавить кость
void PhysRagdoll::AddBone(IBone * bone)
{
	if(!bones) return;
	for(long i = 0; i < *bones; i++)
	{
		if((*bones)[i].bone == bone) return;
	}
	dword index = bones->Add();
	(*bones)[index].Init();
	(*bones)[index].bone = bone;
}

//Установить ящик для кости
void PhysRagdoll::SetBoneShape(IBone * bone, const Matrix & worldTransform, const Matrix & localTransform, const Vector & size, float mass)
{
	if(!bones) return;
	SaveDataInfo & di = GetSaveDataInfo(bone);
	if(di.boxData) delete di.boxData;
	if(di.capsuleData) delete di.capsuleData;
	di.boxData = NEW SaveDataBox;
	di.capsuleData = null;	
	ConvertMatrix(di.boxData->worldTransform, worldTransform);
	ConvertMatrix(di.boxData->localTransform, localTransform);
	ConvertVector(di.boxData->size, size);
	di.boxData->mass = SwizzleFloat(mass);
}

//Установить капсулу для кости
void PhysRagdoll::SetBoneShape(IBone * bone, const Matrix & worldTransform, const Matrix & localTransform, float height, float radius, float mass)
{
	if(!bones) return;
	SaveDataInfo & di = GetSaveDataInfo(bone);
	if(di.boxData) delete di.boxData;
	if(di.capsuleData) delete di.capsuleData;
	di.boxData = null;
	di.capsuleData = NEW SaveDataCapsule;
	ConvertMatrix(di.capsuleData->worldTransform, worldTransform);
	ConvertMatrix(di.capsuleData->localTransform, localTransform);
	di.capsuleData->radius = SwizzleFloat(radius);
	di.capsuleData->height = SwizzleFloat(height);
	di.capsuleData->mass = SwizzleFloat(mass);
}

//Установить сферический сустав для ребёнка и его родителя
void PhysRagdoll::SetBoneJoint(IBone * bone, const SphericalJointParams & params)
{
	if(!bones) return;
	SaveDataInfo & di = GetSaveDataInfo(bone);
	if(di.sphericalJoint) delete di.sphericalJoint;
	if(di.revoluteJoint) delete di.revoluteJoint;
	di.sphericalJoint = NEW SaveDataSphericalJoint;
	di.revoluteJoint = null;
	ConvertVector(di.sphericalJoint->worldJointPosition, params.worldJointPosition);
	ConvertVector(di.sphericalJoint->swingAxisInParentSystem, params.swingAxisInParentSystem);
	di.sphericalJoint->swingLimit = SwizzleFloat(params.swingLimit);
	di.sphericalJoint->swingSpring = SwizzleFloat(params.swingSpring);
	di.sphericalJoint->swingDamper = SwizzleFloat(params.swingDamper);
	ConvertVector(di.sphericalJoint->twistAxisInParentSystem, params.twistAxisInParentSystem);
	di.sphericalJoint->twistMin = SwizzleFloat(params.twistMin);
	di.sphericalJoint->twistMax = SwizzleFloat(params.twistMax);
	di.sphericalJoint->twistSpring = SwizzleFloat(params.twistSpring);
	di.sphericalJoint->twistDamper = SwizzleFloat(params.twistDamper);
}

//Установить шарнирный сустав для ребёнка и его родителя
void PhysRagdoll::SetBoneJoint(IBone * bone, const RevoluteJointParams & params)
{
	if(!bones) return;
	SaveDataInfo & di = GetSaveDataInfo(bone);
	if(di.sphericalJoint) delete di.sphericalJoint;
	if(di.revoluteJoint) delete di.revoluteJoint;
	di.sphericalJoint = null;
	di.revoluteJoint = NEW SaveDataRevoluteJoint;
	ConvertVector(di.revoluteJoint->worldJointPosition, params.worldJointPosition);
	ConvertVector(di.revoluteJoint->axisInParentSystem, params.axisInParentSystem);
	ConvertVector(di.revoluteJoint->normalInParentSystem, params.normalInParentSystem);
	di.revoluteJoint->minAngle = SwizzleFloat(params.minAngle);
	di.revoluteJoint->maxAngle = SwizzleFloat(params.maxAngle);
	di.revoluteJoint->spring = SwizzleFloat(params.spring);
	di.revoluteJoint->damper = SwizzleFloat(params.damper);
}

//Получить описания кости по указателю
PhysRagdoll::SaveDataInfo & PhysRagdoll::GetSaveDataInfo(IBone * bone)
{
	for(long i = 0; i < *bones; i++)
	{
		if((*bones)[i].bone == bone) break;
	}
	Assert(i < *bones);
	return (*bones)[i];
}

//Создать ящик опираясь на сохранённые данные
void PhysRagdoll::CreateBoxFromSaveData(IBone * bone, const SaveDataBox & data)
{
	//Получаем нормальные параметры
	Matrix mtxWorld(false), mtxLocal(false);
	ConvertMatrix(mtxWorld, data.worldTransform);
	ConvertMatrix(mtxLocal, data.localTransform);
	Vector size;
	ConvertVector(size, data.size);
	float mass = SwizzleFloat(data.mass);
	//Создаём
	bone->SetShape(mtxWorld, mtxLocal, size, mass);
}

//Создать капсулу опираясь на сохранённые данные
void PhysRagdoll::CreateCapsuleFromSaveData(IBone * bone, const SaveDataCapsule & data)
{
	//Получаем нормальные параметры
	Matrix mtxWorld(false), mtxLocal(false);
	ConvertMatrix(mtxWorld, data.worldTransform);
	ConvertMatrix(mtxLocal, data.localTransform);
	float height = SwizzleFloat(data.height);
	float radius = SwizzleFloat(data.radius);
	float mass = SwizzleFloat(data.mass);
	//Создаём
	bone->SetShape(mtxWorld, mtxLocal, height, radius, mass);
}

//Создать сферический сустав опираясь на сохранённые данные
void PhysRagdoll::CreateSphericalJointFromSaveData(IBone * bone, const SaveDataSphericalJoint & data)
{
	//Получаем нормальные параметры
	SphericalJointParams params;
	ConvertVector(params.worldJointPosition, data.worldJointPosition);
	ConvertVector(params.swingAxisInParentSystem, data.swingAxisInParentSystem);
	params.swingAxisInParentSystem = !params.swingAxisInParentSystem;	// нормализуем
	params.swingLimit = SwizzleFloat(data.swingLimit);
	params.swingSpring = SwizzleFloat(data.swingSpring);
	params.swingDamper = SwizzleFloat(data.swingDamper);
	ConvertVector(params.twistAxisInParentSystem, data.twistAxisInParentSystem);
	params.twistAxisInParentSystem = !params.twistAxisInParentSystem;	// нормализуем
	params.twistMin = SwizzleFloat(data.twistMin);
	params.twistMax = SwizzleFloat(data.twistMax);
	params.twistSpring = SwizzleFloat(data.twistSpring);
	params.twistDamper = SwizzleFloat(data.twistDamper);
	//Создаём
	bone->GetParent()->CreateJoint(*bone, params);
}

//Создать шарнирный сустав опираясь на сохранённые данные
void PhysRagdoll::CreateRevoluteJointFromSaveData(IBone * bone, const SaveDataRevoluteJoint & data)
{
	//Получаем нормальные параметры
	RevoluteJointParams params;
	ConvertVector(params.worldJointPosition, data.worldJointPosition);
	ConvertVector(params.axisInParentSystem, data.axisInParentSystem);
	ConvertVector(params.normalInParentSystem, data.normalInParentSystem);
	params.minAngle = SwizzleFloat(data.minAngle);
	params.maxAngle = SwizzleFloat(data.maxAngle);
	params.spring = SwizzleFloat(data.spring);
	params.damper = SwizzleFloat(data.damper);
	//Создаём
	bone->GetParent()->CreateJoint(*bone, params);
}

//Сконвертировать вектор
void PhysRagdoll::ConvertVector(float v[3], const Vector & vct)
{
	v[0] = SwizzleFloat(vct.x);
	v[1] = SwizzleFloat(vct.y);
	v[2] = SwizzleFloat(vct.z);
}

//Сконвертировать вектор
void PhysRagdoll::ConvertVector(Vector & vct, const float v[3])
{
	vct.x = SwizzleFloat(v[0]);
	vct.y = SwizzleFloat(v[1]);
	vct.z = SwizzleFloat(v[2]);
}

//Сконвертировать матрицу
void PhysRagdoll::ConvertMatrix(float m[12], const Matrix & mtx)
{
	m[0] = SwizzleFloat(mtx.vx.x);
	m[1] = SwizzleFloat(mtx.vx.y);
	m[2] = SwizzleFloat(mtx.vx.z);
	m[3] = SwizzleFloat(mtx.vy.x);
	m[4] = SwizzleFloat(mtx.vy.y);
	m[5] = SwizzleFloat(mtx.vy.z);
	m[6] = SwizzleFloat(mtx.vz.x);
	m[7] = SwizzleFloat(mtx.vz.y);
	m[8] = SwizzleFloat(mtx.vz.z);
	m[9] = SwizzleFloat(mtx.pos.x);
	m[10] = SwizzleFloat(mtx.pos.y);
	m[11] = SwizzleFloat(mtx.pos.z);
}

//Сконвертировать матрицу
void PhysRagdoll::ConvertMatrix(Matrix & mtx, const float m[12])
{	
	mtx.vx.x = SwizzleFloat(m[0]);
	mtx.vx.y = SwizzleFloat(m[1]);
	mtx.vx.z = SwizzleFloat(m[2]);
	mtx.wx = 0.0f;
	mtx.vy.x = SwizzleFloat(m[3]);
	mtx.vy.y = SwizzleFloat(m[4]);
	mtx.vy.z = SwizzleFloat(m[5]);
	mtx.wy = 0.0f;
	mtx.vz.x = SwizzleFloat(m[6]);
	mtx.vz.y = SwizzleFloat(m[7]);
	mtx.vz.z = SwizzleFloat(m[8]);
	mtx.wz = 0.0f;
	mtx.pos.x = SwizzleFloat(m[9]);
	mtx.pos.y = SwizzleFloat(m[10]);
	mtx.pos.z = SwizzleFloat(m[11]);
	mtx.w = 1.0f;
}

//Добавить данные в сейв
void PhysRagdoll::AddSaveData(array<byte> & buffer, const void * ptr, dword size)
{
	const byte * p = (const byte *)ptr;
	for(; size > 0; size--)
	{
		buffer.Add(*p++);
	}
}

//Прочитать данные
template <class T> __forceinline const T & PhysRagdoll::GetSaveData(dword & ptr, const void * data, dword dataSize)
{
	return *(const T *)GetSaveData(ptr, data, dataSize, sizeof(T));
}

//Прочитать данные
__forceinline const void * PhysRagdoll::GetSaveData(dword & ptr, const void * data, dword dataSize, dword size)
{
	const void * p = (char *)data + ptr;
	ptr += size;
	if(ptr > dataSize)
	{
		throw "Incorrect data size";
	}
	return p;

}

// отписаться от всех активных анимационных блендеров
void PhysRagdoll::RemoveBlendStages()
{
	for(long i = 0; i < stages; i++)
	{
		delete stages[i];
	}
	stages.Empty();
}

//Обновить состояние рэгдола
void PhysRagdoll::UpdateActiveState()
{
	bool isAct = false;
	for(long i = 0; i < stages; i++)
	{
		if(stages[i]->IsActive())
		{
			isAct = true;
		}
	}
	Activate(isAct);
}

//Активировать/деактивировать рэгдол
void PhysRagdoll::Activate(bool isAct)
{
	if(isAct == isActive)
	{
		return;
	}
	isActive = isAct;
	
	if (root)
		root->Activate(isAct);
}

