#include "PhysRigidBody.h"
#include "PhysicsScene.h"
#include "PhysTriangleMesh.h"

#include "IProxy.h"

#ifndef STOP_DEBUG
	// определена в PhysicsService.cpp
	IProxy * FindProxyObject(NxActor * actor);
#endif

inline PhysRigidBodyActor::PhysRigidBodyActor(float density) :
actor(null),
group(phys_world),
m_density(density)
{
	bKinematic = false;
}

inline PhysRigidBodyActor::~PhysRigidBodyActor()
{
	Release();
}

inline void PhysRigidBodyActor::Release()
{
	if(actor)
	{
		actor->getScene().releaseActor(*actor);
	}
	actor = null;
}

inline bool PhysRigidBodyActorCheckDynamic(const NxActor & actor, const char * file, long line)
{
	return true;

	if (!actor.isDynamic())
	{
		api->Trace("Physics error: Bad call for static actor, %s, %d", file, line);
	}

	return true;
}

bool PhysRigidBodyActorCheckMtx(const Matrix & mtx, NxActor * actor, const char * filename, long fileline)
{
	bool isScaled = false;

#ifndef _XBOX
	const float eps = 1e-3f;
	const float ortoEps = 0.999f;
#else
	const float eps = 1e-2f;
	const float ortoEps = 0.999f;
#endif

	bool isNan = false;
	for (int i=0; i<16; i++)
		if (_isnan(mtx.matrix[i]))
		{
			isNan = true;
			break;
		}

	if(fabsf(~mtx.vx - 1.0f) > eps) isScaled = true;
	if(fabsf(~mtx.vy - 1.0f) > eps) isScaled = true;
	if(fabsf(~mtx.vz - 1.0f) > eps) isScaled = true;

	float orhto = (!(mtx.vx ^ mtx.vz) | !mtx.vy);
	bool isNonOrtho = (orhto >= -ortoEps);

	//if (api->DebugKeyState('8') && (rand()%10) == 5) isNonOrtho = true;

	if (isScaled || isNonOrtho || isNan)
	{
		if (isScaled)	api->Trace("Matrix have scale factor");
		if (isNonOrtho)	api->Trace("Matrix non-orthogonal, cos = %.6f, degree = %.2f", orhto, safeACos(orhto));
		if (isNan)		api->Trace("Matrix have NAN numbers");

		api->Trace("vx = (%f, %f, %f; %f); len = %f", mtx.vx.x, mtx.vx.y, mtx.vx.z, mtx.wx, mtx.vx.GetLength());
		api->Trace("vy = (%f, %f, %f; %f); len = %f", mtx.vy.x, mtx.vy.y, mtx.vy.z, mtx.wy, mtx.vy.GetLength());
		api->Trace("vz = (%f, %f, %f; %f); len = %f", mtx.vz.x, mtx.vz.y, mtx.vz.z, mtx.wz, mtx.vz.GetLength());
		api->Trace("pos = (%f, %f, %f; %f)", mtx.pos.x, mtx.pos.y, mtx.pos.z, mtx.w);
		api->Trace("file: %s, line: %d", filename, fileline);

#ifndef STOP_DEBUG
		if (!actor) return false;
		// трейсим несколько последних установок параметров в прокси объекте

		// ищем прокси объект
		IProxy * proxy = FindProxyObject(actor);
		if (proxy)
			proxy->OnSyncTrace();
#endif

		return false;
	}
	return true;
}


//array<Matrix> aMats(_FL_);

//Устоновить мировую позицию
inline void PhysRigidBodyActor::SetTransform(const Matrix & mtx)
{
	Assert(PhysRigidBodyActorCheckMtx(mtx, null, GetFileName(), GetFileLine()));

	NxMat34 m;
	GetNxActor().setGlobalPose(Nx(m, mtx));
}

//Получить позицию и ориентацию в мире

inline void PhysRigidBodyActor::GetTransform(Matrix & mtx) const
{
	Nx(mtx, GetNxActor().getGlobalPose());

	Assert(PhysRigidBodyActorCheckMtx(mtx, actor, GetFileName(), GetFileLine()));
}

// Нормализовать глобальную матрицу чтобы не искажалась
inline int PhysRigidBodyActor::Normalize()
{
	if (!actor->isDynamic()) 
		return 0;

	Matrix mtx;
	GetTransform(mtx);

	Vector & x = mtx.vx;
	Vector & y = mtx.vy;
	Vector & z = mtx.vz;

	x = !(y ^ z);
	y = !(z ^ x);
	z = !z;

	SetTransform(mtx);

	return 1;
}

//Установить массу
inline void PhysRigidBodyActor::SetMass(float mass)
{
	if(!GetNxActor().isDynamic()) return;
	GetNxActor().updateMassFromShapes(mass, 0.0f);
}

//Установить массу
float PhysRigidBodyActor::GetMass() const
{
	return GetNxActor().getMass();
}

//Установить центр масс
inline void PhysRigidBodyActor::SetCenterMass(const Vector & cm)
{
	if(!GetNxActor().isDynamic()) return;
	GetNxActor().setCMassOffsetLocalPosition(Nx(cm));
}

//Установить центр масс
inline Vector PhysRigidBodyActor::GetCenterMass() const
{
	if(!GetNxActor().isDynamic())
	{		
		return Nx(GetNxActor().getGlobalPosition());
	}
	return Nx(GetNxActor().getCMassLocalPosition());
}

//Установить группы
inline void PhysRigidBodyActor::SetGroup(PhysicsCollisionGroup group)
{
	this->group = group;	
	if(!GetNxActor().isDynamic())
	{		
		if(GetNxActor().getGroup() == phys_nocollision)
		{
			return;
		}
	}
	sys_SetGroup(group);
}

inline void PhysRigidBodyActor::sys_SetGroup(PhysicsCollisionGroup group)
{
	NxActor & act = GetNxActor();
	act.setGroup((NxActorGroup)group);
	NxU32 count = act.getNbShapes();
	NxShape * const * shapes = act.getShapes();
	for(NxU32 i = 0; i < count; i++)
	{
		shapes[i]->setGroup((NxCollisionGroup)group);
	}

	if (act.isDynamic())
		act.wakeUp();
}

//Получить группы
inline PhysicsCollisionGroup PhysRigidBodyActor::GetGroup() const
{
	return (PhysicsCollisionGroup)GetNxActor().getGroup();
}

//Приложить силу к кости в заданной локальной точке
inline void PhysRigidBodyActor::ApplyForce(const Vector & force, const Vector & localPosition)
{
#ifndef STOP_ASSERTS
	if (_isnan(force.x) || _isnan(force.y) || _isnan(force.z) || fabsf(force.x) > 100000.0f || fabsf(force.y) > 100000.0f || fabsf(force.z) > 100000.0f)
	{
		api->Trace("ApplyForce force: %.3f, %.3f, %.3f [%s, %d] ", force.x, force.y, force.z, GetFileName(), GetFileLine());
		Assert(false);
	}
	if (!(GetNxActor().isDynamic() && !bKinematic))
	{
		api->Trace("ApplyForce actor non dynamic or kinematic [%s, %d]", GetFileName(), GetFileLine());
		Assert(false);
	}
#endif

	GetNxActor().addForceAtLocalPos(Nx(force), Nx(localPosition), NX_FORCE);
}

//Приложить силу к кости в заданной локальной точке
inline void PhysRigidBodyActor::ApplyImpulse(const Vector & force, const Vector & localPosition)
{
#ifndef STOP_ASSERTS
	if (_isnan(force.x) || _isnan(force.y) || _isnan(force.z) || fabsf(force.x) > 100000.0f || fabsf(force.y) > 100000.0f || fabsf(force.z) > 100000.0f)
	{
		api->Trace("ApplyImpulse force: %.3f, %.3f, %.3f [%s, %d] ", force.x, force.y, force.z, GetFileName(), GetFileLine());
		Assert(false);
	}
	if (!(GetNxActor().isDynamic() && !bKinematic))
	{
		api->Trace("ApplyImpulse actor non dynamic or kinematic [%s, %d], GetFileName(), GetFileLine()");
		Assert(false);
	}
#endif

	float k = GetNxActor().getMass();
	k = Clampf(k, 0.01f, 4.0f);
	GetNxActor().addForceAtLocalPos(Nx(force*k), Nx(localPosition), NX_IMPULSE);
}

//приложить вращающий момент в СК актера
void PhysRigidBodyActor::ApplyLocalTorque(const Vector & torque)
{
#ifndef STOP_ASSERTS
	if (_isnan(torque.x) || _isnan(torque.y) || _isnan(torque.z))
	{
		api->Trace("ApplyLocalTorque torque: %.3f, %.3f, %.3f [%s, %d] ", torque.x, torque.y, torque.z, GetFileName(), GetFileLine());
		Assert(false);
	}
	if (!(GetNxActor().isDynamic() && !bKinematic))
	{
		api->Trace("ApplyLocalTorque actor non dynamic or kinematic [%s, %d], GetFileName(), GetFileLine()");
		Assert(false);
	}
#endif

	GetNxActor().addLocalTorque(Nx(torque));
}

// установить затухание движения
void PhysRigidBodyActor::SetMotionDamping ( float fDamping )
{
	GetNxActor().setLinearDamping ( fDamping );
	GetNxActor().setAngularDamping ( fDamping );
};

void PhysRigidBodyActor::SetMotionDamping( float fLinDamping, float fAngDamping )
{
	GetNxActor().setLinearDamping ( fLinDamping );
	GetNxActor().setAngularDamping ( fAngDamping );
}

// включить-выключить коллизию
void PhysRigidBodyActor::EnableCollision ( bool bEnable )
{
	NxActor & rActor = GetNxActor();

	if (bEnable)
		rActor.clearActorFlag(NX_AF_DISABLE_COLLISION);
	else
		rActor.raiseActorFlag(NX_AF_DISABLE_COLLISION);

	/*NxU32 iShapesCount =  rActor.getNbShapes ();
	NxShape * const *  pShapes = rActor.getShapes ();

	for ( NxU32 i = 0; i < iShapesCount; ++ i )
	{
		Assert  ( pShapes[i] );
		pShapes[i]->setFlag ( NX_SF_DISABLE_COLLISION, !bEnable );
	};*/
};

// включить-выключить гравитацию
void PhysRigidBodyActor::EnableGravity ( bool bEnable )
{
	if (!bEnable)
		GetNxActor().raiseBodyFlag(NX_BF_DISABLE_GRAVITY);
	else
		GetNxActor().clearBodyFlag(NX_BF_DISABLE_GRAVITY);
}

// включить-выключить кинематическое поведение
void PhysRigidBodyActor::EnableKinematic(bool bEnable)
{
	bKinematic = bEnable;

	if (bEnable)
		GetNxActor().raiseBodyFlag(NX_BF_KINEMATIC);
	else
		GetNxActor().clearBodyFlag(NX_BF_KINEMATIC);
}

//Включить-выключить объект
inline void PhysRigidBodyActor::Activate(bool isActive)
{
	NxActor & a = GetNxActor();
	if(isActive)
	{
		if(a.isDynamic())
		{
			a.clearActorFlag(NX_AF_DISABLE_COLLISION);
			a.raiseBodyFlag(NX_BF_VISUALIZATION);
			a.clearActorFlag(NX_AF_DISABLE_RESPONSE);
			a.clearBodyFlag(NX_BF_FROZEN);
			a.wakeUp();
		}else{
			sys_SetGroup(group);
		}
	}else{
		if(a.isDynamic())
		{
			a.raiseActorFlag(NX_AF_DISABLE_COLLISION);
			a.clearBodyFlag(NX_BF_VISUALIZATION);
			a.raiseActorFlag(NX_AF_DISABLE_RESPONSE);
			a.raiseBodyFlag(NX_BF_FROZEN);
			a.wakeUp();
		}else{
			sys_SetGroup(phys_nocollision);
		}
	}
}

void PhysRigidBodyActor::SetFileLine(const char * filename, long fileline)
{
	m_fileName = filename;
	m_fileLine = fileline;
}

//получить владение
void PhysRigidBodyActor::SetNxActor(const char * filename, long fileline, NxActor* nxActor)
{
	SetFileLine(filename, fileline);

	Assert(nxActor);
	actor = nxActor;

	if (actor->isDynamic())
		actor->setContactReportThreshold(0.6f);
}

//Получить ссылку на актёра
inline NxActor & PhysRigidBodyActor::GetNxActor() const
{
	Assert(actor);
	return *actor;
}

// установить материал
void PhysRigidBodyActor::SetPhysMaterial(IPhysMaterial * material)
{
	this->material = material;
	for (unsigned int i = 0; i < actor->getNbShapes(); ++i)
		actor->getShapes()[i]->setMaterial(material->GetIndex());
}

// получить материал
IPhysMaterial * PhysRigidBodyActor::GetPhysMaterial() const
{
	return material;
}

// получить линейную составляющую скорости
Vector PhysRigidBodyActor::GetLinearVelocity() const
{
	return Nx(actor->getLinearVelocity());
}


PhysPlane::PhysPlane(const char * filename, long fileline, const Vector & n, float d, IPhysicsScene * _scene) :
	IPhysPlane(_scene),
	rbactor(0.0f)
{
	SetFileLine(filename, fileline);

	NxPlaneShapeDesc planeDesc;
	planeDesc.normal = Nx(n);
	planeDesc.d = (NxReal)d;
	planeDesc.userData = null;
	planeDesc.group = phys_world;
	NxActorDesc actorDesc;
	actorDesc.shapes.pushBack(&planeDesc);
	actorDesc.userData = this;
	actorDesc.group = phys_world;
	NxActor * nxActor = ((PhysicsScene *)scene)->Scene().createActor(actorDesc);
	Assert(nxActor);
	rbactor.SetNxActor(GetFileName(), GetFileLine(), nxActor);	
}

PhysPlane::~PhysPlane()
{
	scene->SceneChanged(); 
}

//Установить плоскость
void PhysPlane::SetPlane(Plane & p)
{
	Assert(rbactor.GetNxActor().getNbShapes() == 1);
	NxShape * shape = rbactor.GetNxActor().getShapes()[0];
	NxPlaneShape * plane = shape->isPlane();
	plane->setPlane(Nx(p.n), (NxReal)p.d);
}

//Получить плоскость
Plane PhysPlane::GetPlane()
{
	Assert(rbactor.GetNxActor().getNbShapes() == 1);
	NxShape * shape = rbactor.GetNxActor().getShapes()[0];
	NxPlaneShape * plane = shape->isPlane();
	NxPlane p = plane->getPlane();
	return Nx(p);
}


PhysBox::PhysBox(const char * filename, long fileline, const Vector & size, const Matrix & transform, bool isDynamic, float density, IPhysicsScene * _scene) : 
	IPhysBox(_scene),
	rbactor(density)
{
	SetFileLine(filename, fileline);

	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions.set(size.x*0.5f, size.y*0.5f, size.z*0.5f);
	boxDesc.userData = null;
	boxDesc.group = phys_world;
	actorDesc.shapes.pushBack(&boxDesc);
	actorDesc.body = isDynamic ? &bodyDesc : null;
	actorDesc.density = density;
	Nx(actorDesc.globalPose, transform);
	actorDesc.userData = this;
	actorDesc.group = phys_world;

	//if ( _scene->IsHardware () )
	//	actorDesc.managedHwSceneIndex = 1;


	NxActor * nxActor = ((PhysicsScene *)scene)->Scene().createActor(actorDesc);
	if (nxActor == NULL)
	{
		api->Trace("Error creating physical BOX !!!");
		api->Trace("Size: %f, %f, %f", size.x, size.y, size.z);
		api->Trace("Dynamic: %d", isDynamic);

		api->Trace("%f, %f, %f, %f", transform.m[0][0], transform.m[0][1], transform.m[0][2], transform.m[0][3]);
		api->Trace("%f, %f, %f, %f", transform.m[1][0], transform.m[1][1], transform.m[1][2], transform.m[1][3]);
		api->Trace("%f, %f, %f, %f", transform.m[2][0], transform.m[2][1], transform.m[2][2], transform.m[2][3]);
		api->Trace("%f, %f, %f, %f", transform.m[3][0], transform.m[3][1], transform.m[3][2], transform.m[3][3]);

	}
	Assert(nxActor);
	rbactor.SetNxActor(GetFileName(), GetFileLine(), nxActor);
}

PhysBox::~PhysBox()
{
	scene->SceneChanged(); 
}

//Установить размер ящика
void PhysBox::SetSize(const Vector & size)
{
	Assert(rbactor.GetNxActor().getNbShapes() == 1);
	NxShape * shape = rbactor.GetNxActor().getShapes()[0];
	NxBoxShape * box = shape->isBox();
	box->setDimensions(Nx(size*0.5f));
}

//Получить размер ящика
Vector PhysBox::GetSize()
{
	Assert(rbactor.GetNxActor().getNbShapes() == 1);
	NxShape * shape = rbactor.GetNxActor().getShapes()[0];
	NxBoxShape * box = shape->isBox();
	Vector size = Nx(box->getDimensions())*2.0f;
	return size;
}

//Сфера
PhysSphere::PhysSphere(const char * filename, long fileline, float radius, const Matrix & transform, bool isDynamic, float density, IPhysicsScene * _scene) : 
	IPhysSphere(_scene),
	rbactor(density)
{
	SetFileLine(filename, fileline);

	Assert(PhysRigidBodyActorCheckMtx(transform, null, GetFileName(), GetFileLine()));
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	NxSphereShapeDesc sphereDesc;
	sphereDesc.radius = radius;
	sphereDesc.userData = null;
	sphereDesc.group = phys_world;
	actorDesc.shapes.pushBack(&sphereDesc);
	actorDesc.body = isDynamic ? &bodyDesc : null;
	actorDesc.density = density;
	Nx(actorDesc.globalPose, transform);
	actorDesc.userData = this;
	actorDesc.group = phys_world;
	NxActor * nxActor = ((PhysicsScene *)scene)->Scene().createActor(actorDesc);
	Assert(nxActor);
	rbactor.SetNxActor(GetFileName(), GetFileLine(), nxActor);
}

PhysSphere::~PhysSphere()
{
	scene->SceneChanged(); 
}

//Установить радиус
void PhysSphere::SetRadius(float radius)
{
	Assert(rbactor.GetNxActor().getNbShapes() == 1);
	NxShape * shape = rbactor.GetNxActor().getShapes()[0];
	NxSphereShape * sphere = shape->isSphere();
	sphere->setRadius(radius);
}

//Получить радиус
float PhysSphere::GetRadius()
{
	Assert(rbactor.GetNxActor().getNbShapes() == 1);
	NxShape * shape = rbactor.GetNxActor().getShapes()[0];
	NxSphereShape * sphere = shape->isSphere();
	return (float)sphere->getRadius();
}

//Капсула
PhysCapsule::PhysCapsule(const char * filename, long fileline, float radius, float height, const Matrix & transform, bool isDynamic, float density, IPhysicsScene * _scene) : 
	IPhysCapsule(_scene),
	rbactor(density)
{
	SetFileLine(filename, fileline);

	Assert(PhysRigidBodyActorCheckMtx(transform, null, GetFileName(), GetFileLine()));
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	NxCapsuleShapeDesc capsuleDesc;
	capsuleDesc.radius = radius;
	capsuleDesc.height = height;
	capsuleDesc.userData = null;
	capsuleDesc.group = phys_world;
	actorDesc.shapes.pushBack(&capsuleDesc);
	actorDesc.body = isDynamic ? &bodyDesc : null;
	actorDesc.density = density;
	Nx(actorDesc.globalPose, transform);
	actorDesc.userData = this;
	actorDesc.group = phys_world;
	NxActor * nxActor = ((PhysicsScene *)scene)->Scene().createActor(actorDesc);
	Assert(nxActor);
	rbactor.SetNxActor(GetFileName(), GetFileLine(), nxActor);
}

PhysCapsule::~PhysCapsule()
{
	scene->SceneChanged(); 
}

//Установить радиус
void PhysCapsule::SetRadius(float radius)
{
	Assert(rbactor.GetNxActor().getNbShapes() == 1);
	NxShape * shape = rbactor.GetNxActor().getShapes()[0];
	NxCapsuleShape * capsule = shape->isCapsule();
	capsule->setRadius(radius);
}

//Получить радиус
float PhysCapsule::GetRadius()
{
	Assert(rbactor.GetNxActor().getNbShapes() == 1);
	NxShape * shape = rbactor.GetNxActor().getShapes()[0];
	NxCapsuleShape * capsule = shape->isCapsule();
	return (float)capsule->getRadius();
}

//Установить высоту
void PhysCapsule::SetHeight(float height)
{
	Assert(rbactor.GetNxActor().getNbShapes() == 1);
	NxShape * shape = rbactor.GetNxActor().getShapes()[0];
	NxCapsuleShape * capsule = shape->isCapsule();
	capsule->setHeight(height);
}

//Получить высоту
float PhysCapsule::GetHeight()
{
	Assert(rbactor.GetNxActor().getNbShapes() == 1);
	NxShape * shape = rbactor.GetNxActor().getShapes()[0];
	NxCapsuleShape * capsule = shape->isCapsule();
	return (float)capsule->getHeight();
}

//Объект состоящий из сетки треугольников
PhysMesh::PhysMesh(const char * filename, long fileline, const IPhysicsScene::MeshInit * meshes, dword numMeshes, bool isDynamic, float density, IPhysicsScene * _scene) : 
	IPhysMesh(_scene),
	rbactor(density),
	refMeshes(_FL_)
{
	SetFileLine(filename, fileline);

	AssertCoreThread
	NxActorDesc actorDesc;
	actorDesc.shapes.reserve(numMeshes);
	static NxTriangleMeshShapeDesc static_shapeDesc[16];
	NxTriangleMeshShapeDesc * descs = static_shapeDesc;
	array<NxTriangleMeshShapeDesc> dyn_shapeDesc(_FL_);
	if(numMeshes > ARRSIZE(static_shapeDesc))
	{
		dyn_shapeDesc.AddElements(numMeshes);
		descs = dyn_shapeDesc.GetBuffer();
	}
	for(long i = 0; i < (long)numMeshes; i++)
	{
		PhysTriangleMesh * physMesh = (PhysTriangleMesh *)meshes[i].mesh;
		physMesh->AddRef();
		refMeshes.Add(physMesh);

		NxTriangleMeshShapeDesc & shapeDesc = descs[i];
		Assert(meshes[i].mesh);
		shapeDesc.shapeFlags |= NX_SF_FEATURE_INDICES;
		shapeDesc.meshData = physMesh->TriangleMesh();
		Nx(shapeDesc.localPose, meshes[i].mtx);
		shapeDesc.userData = null;
		actorDesc.group = phys_world;
		actorDesc.shapes.pushBack(&shapeDesc);
	}

	NxBodyDesc bodyDesc;
	actorDesc.body = isDynamic ? &bodyDesc : null;	
	actorDesc.density = density;
	actorDesc.userData = this;
	actorDesc.group = phys_world;
	rbactor.SetNxActor(GetFileName(), GetFileLine(), ((PhysicsScene *)scene)->Scene().createActor(actorDesc));

	if (isDynamic)
		EnableKinematic(true);
}

void PhysMesh::EnableCollision(bool bEnable)
{ 
	//Assert(!(bEnable && rbactor.GetNxActor().isDynamic()));
	rbactor.EnableCollision(bEnable); 
}


PhysMesh::~PhysMesh()
{
	rbactor.Release();

	for (int i=0; i<refMeshes.Len(); i++)
		refMeshes[i]->Release();

	refMeshes.DelAll();
	
	scene->SceneChanged(); 
}

void PhysMesh::EnableKinematic(bool bEnable) 
{
	if (!bEnable) 
		api->Trace("PhysMesh Error: Someone tryed to disable kinematic flag for PhysMesh!!");

	rbactor.EnableKinematic(true); 
}


PhysCombined::ObjectDesc::ObjectDesc() : descs(_FL_)
{
}

PhysCombined::ObjectDesc::~ObjectDesc()
{
	for(long i = 0; i < descs; i++)
	{
		delete descs[i];
	}
}

//Комбинированный объект
PhysCombined::PhysCombined(const char * filename, long fileline, const Matrix & transform, bool isDynamic, float density, IPhysicsScene * _scene) : 
	IPhysCombined(_scene),
	rbactor(density)
{
	SetFileLine(filename, fileline);

	Assert(PhysRigidBodyActorCheckMtx(transform, null, GetFileName(), GetFileLine()));
	desc = NEW ObjectDesc();
	Nx(desc->actorDesc.globalPose, transform);
	desc->actorDesc.body = isDynamic ? &desc->bodyDesc : null;
	desc->actorDesc.density = density;
	desc->actorDesc.userData = this;
	desc->actorDesc.group = phys_world;	
}

PhysCombined::~PhysCombined()
{
	if(desc)
	{
		delete desc;
		desc = null;
	}
	
	scene->SceneChanged(); 
}

//Добавить ящик
void PhysCombined::AddBox(const Vector & size, const Matrix & transform)
{
	Assert(PhysRigidBodyActorCheckMtx(transform, null, GetFileName(), GetFileLine()));
	Assert(desc);
	NxBoxShapeDesc * boxDesc = NEW NxBoxShapeDesc;
	boxDesc->dimensions.set(size.x*0.5f, size.y*0.5f, size.z*0.5f);
	Nx(boxDesc->localPose, transform);
	boxDesc->userData = null;
	boxDesc->group = phys_world;
	desc->actorDesc.shapes.pushBack(boxDesc);
	desc->descs.Add(boxDesc);
}

//Добавить шар
void PhysCombined::AddSphere(float radius, const Matrix & transform)
{
	Assert(PhysRigidBodyActorCheckMtx(transform, null, GetFileName(), GetFileLine()));
	Assert(desc);
	NxSphereShapeDesc * sphereDesc = NEW NxSphereShapeDesc;
	sphereDesc->radius = radius;
	Nx(sphereDesc->localPose, transform);
	sphereDesc->userData = null;
	sphereDesc->group = phys_world;
	desc->actorDesc.shapes.pushBack(sphereDesc);
	desc->descs.Add(sphereDesc);
}

//Добавить капсулу
void PhysCombined::AddCapsule(float radius, float height, const Matrix & transform)
{
	Assert(PhysRigidBodyActorCheckMtx(transform, null, GetFileName(), GetFileLine()));
	Assert(desc);
	NxCapsuleShapeDesc * capsuleDesc = NEW NxCapsuleShapeDesc;
	capsuleDesc->radius = radius;
	capsuleDesc->height = height;
	Nx(capsuleDesc->localPose, transform);
	capsuleDesc->userData = null;
	capsuleDesc->group = phys_world;
	desc->actorDesc.shapes.pushBack(capsuleDesc);
	desc->descs.Add(capsuleDesc);
}

//Установить массу элемента
bool PhysCombined::SetMass(unsigned int index, float mass)
{
	Assert(desc);
	if ( index >= desc->descs.Size() ) return false;
	desc->descs[index]->mass = mass;
	return true;
}

//Получить текущее число фигур для билда
unsigned int PhysCombined::GetCountForBuild()
{
	Assert(desc);
	return desc->descs.Size();
}

//Сконструировать объект
void PhysCombined::Build()
{
	Assert(desc);
	Assert(desc->descs > 0);
	NxActor * nxActor = ((PhysicsScene *)scene)->Scene().createActor(desc->actorDesc);
	Assert(nxActor);
	rbactor.SetNxActor(GetFileName(), GetFileLine(), nxActor);
	delete desc;
	desc = null;
}

//Получить количество фигур
long PhysCombined::GetCount()
{
	if(desc) return 0;
	return (long)rbactor.GetNxActor().getNbShapes();
}

//Получить локальную позицию
bool PhysCombined::SetLocalTransform(long index, const Matrix & transform)
{
	if(desc) return false;
	if(index < 0 || index >= (long)rbactor.GetNxActor().getNbShapes()) return false;
	NxMat34 mtx;
	rbactor.GetNxActor().getShapes()[index]->setLocalPose(Nx(mtx, transform));
	return true;
}

//Установить локальную позицию
bool PhysCombined::GetLocalTransform(long index, Matrix & transform)
{
	if(desc) return false;
	if(index < 0 || index >= (long)rbactor.GetNxActor().getNbShapes()) return false;
	Nx(transform, rbactor.GetNxActor().getShapes()[index]->getLocalPose());
	return true;
}

//Получить тип
IPhysCombined::Type PhysCombined::GetType(long index)
{
	if(desc) return t_error;
	if(index < 0 || index >= (long)rbactor.GetNxActor().getNbShapes()) return t_error;
	NxShape * shp = rbactor.GetNxActor().getShapes()[index];
	if(!shp) return t_error;
	switch(shp->getType())
	{
	case NX_SHAPE_SPHERE:
		return t_sphere;
	case NX_SHAPE_BOX:
		return t_box;
	case NX_SHAPE_CAPSULE:
		return t_capsule;
	}
	return t_error;
}

//Получить параметры ящика
bool PhysCombined::GetBox(long index, Vector & size)
{
	if(desc) return false;
	if(index < 0 || index >= (long)rbactor.GetNxActor().getNbShapes()) return false;
	NxBoxShape * shp = rbactor.GetNxActor().getShapes()[index]->isBox();
	if(!shp) return false;
	size = Nx(shp->getDimensions());
	return true;
}

//Получить параметры шара
bool PhysCombined::GetSphere(long index, float & radius)
{
	if(desc) return false;
	if(index < 0 || index >= (long)rbactor.GetNxActor().getNbShapes()) return false;
	NxSphereShape * shp = rbactor.GetNxActor().getShapes()[index]->isSphere();
	if(!shp) return false;
	radius = float(shp->getRadius());
	return true;
}

//Получить параметры капсулы
bool PhysCombined::GetCapsule(long index, float & radius, float & height)
{
	if(desc) return false;
	if(index < 0 || index >= (long)rbactor.GetNxActor().getNbShapes()) return false;
	NxCapsuleShape * shp = rbactor.GetNxActor().getShapes()[index]->isCapsule();
	if(!shp) return false;
	radius = float(shp->getRadius());
	height = float(shp->getHeight());
	return true;
}

//Установить глобальную позицию
bool PhysCombined::SetGlobalTransform(unsigned int index, const Matrix & transform)
{
	if (desc) return false;
	if (index >= rbactor.GetNxActor().getNbShapes()) return false;
	NxMat34 mtx;
	rbactor.GetNxActor().getShapes()[index]->setGlobalPose(Nx(mtx, transform));
	return true;
}

//Получить глобальную позицию
bool PhysCombined::GetGlobalTransform(unsigned int index, Matrix & transform)
{
	if (desc) return false;
	if (index >= rbactor.GetNxActor().getNbShapes()) return false;
	Nx(transform, rbactor.GetNxActor().getShapes()[index]->getGlobalPose());
	return true;
}

//Включить/выключить коллизии
void PhysCombined::EnableCollision(unsigned int index, bool enable)
{
	if (desc)
	{
		if (index >= desc->descs.Size())
			return;

		if (enable)
			desc->descs[index]->shapeFlags &= ~NX_SF_DISABLE_COLLISION;
		else
			desc->descs[index]->shapeFlags |= NX_SF_DISABLE_COLLISION;
	}
	else
	{
		if (index >= rbactor.GetNxActor().getNbShapes()) return;
		rbactor.GetNxActor().getShapes()[index]->setFlag(NX_SF_DISABLE_COLLISION, !enable);
	}
}

//Включить/выключить коллизию для актера
void PhysCombined::EnableCollision(bool enable)
{
	if (desc)
		for (unsigned int i = 0; i < GetCountForBuild(); ++i)
			EnableCollision(i, enable);
	else
	{
		if (enable)
			rbactor.GetNxActor().clearActorFlag(NX_AF_DISABLE_COLLISION);
		else
			rbactor.GetNxActor().raiseActorFlag(NX_AF_DISABLE_COLLISION);
			
	}
}

//Включить/выключить реакцию на коллизии
void PhysCombined::EnableResponse(unsigned int index, bool enable)
{
	if (desc)
	{
		if (index >= desc->descs.Size())
			return;

		if (enable)
			desc->descs[index]->shapeFlags &= ~NX_SF_DISABLE_RESPONSE;
		else
			desc->descs[index]->shapeFlags |= NX_SF_DISABLE_RESPONSE;
	}
	else
	{
		if (index >= rbactor.GetNxActor().getNbShapes()) return;
		rbactor.GetNxActor().getShapes()[index]->setFlag(NX_SF_DISABLE_RESPONSE, !enable);
	}
}

//Включить/выключить рэйкаст
void PhysCombined::EnableRaycast(unsigned int index, bool enable)
{
	if (desc)
	{
		if (index >= desc->descs.Size())
			return;

		if (enable)
			desc->descs[index]->shapeFlags &= ~NX_SF_DISABLE_RAYCASTING;
		else
			desc->descs[index]->shapeFlags |= NX_SF_DISABLE_RAYCASTING;
	}
	else
	{
		if (index >= rbactor.GetNxActor().getNbShapes()) return;
		rbactor.GetNxActor().getShapes()[index]->setFlag(NX_SF_DISABLE_RAYCASTING, !enable);
	}
}

//Включить/выключить визуализацию
void PhysCombined::EnableVisualization(unsigned int index, bool enable)
{
	if (desc)
	{
		if (index >= desc->descs.Size())
			return;

		if (enable)
			desc->descs[index]->shapeFlags |= NX_SF_VISUALIZATION;
		else
			desc->descs[index]->shapeFlags &= ~NX_SF_VISUALIZATION;
	}
	else
	{
		if (index >= rbactor.GetNxActor().getNbShapes()) return;
		rbactor.GetNxActor().getShapes()[index]->setFlag(NX_SF_VISUALIZATION, enable);
	}
}

//Включить/выключить визуализацию для актера
void PhysCombined::EnableVisualization(bool enable)
{
	if (desc)
		for (unsigned int i = 0; i < GetCountForBuild(); ++i)
			EnableCollision(i, enable);
	else
	{
		if (enable)
			rbactor.GetNxActor().raiseBodyFlag(NX_BF_VISUALIZATION);
		else
			rbactor.GetNxActor().clearBodyFlag(NX_BF_VISUALIZATION);

	}
}

// включить-выключить гравитацию
void PhysCombined::EnableGravity( bool bEnable )
{
	if (desc)
	{
		Assert(!"can't change gravity state - no actor created");
	}
	else
	{
		if (bEnable)
			rbactor.GetNxActor().clearBodyFlag(NX_BF_DISABLE_GRAVITY);
		else
			rbactor.GetNxActor().raiseBodyFlag(NX_BF_DISABLE_GRAVITY);
	}
}

// включить-выключить кинематическое поведение
void PhysCombined::EnableKinematic(bool bEnable)
{
	rbactor.EnableKinematic(bEnable);
}

// установить кол-во итерация солвера для актера
void PhysCombined::SetSolverIterations(unsigned int count)
{
	rbactor.GetNxActor().setSolverIterationCount(count);
}

//Протестировать на пересечение с лучом
bool PhysCombined::Raycast(unsigned int index, const Vector& from, const Vector& to, RaycastResult * details)
{
	if (desc) return false;
	if (index >= rbactor.GetNxActor().getNbShapes()) return false;
	
	NxRay worldRay;
	NxReal maxDist;
	NxU32 hintFlags = NX_RAYCAST_IMPACT | NX_RAYCAST_NORMAL | NX_RAYCAST_DISTANCE | NX_RAYCAST_MATERIAL;
	NxRaycastHit hit;
	bool firstHit = details == null;

	worldRay.orig = Nx(from);
	worldRay.dir = Nx(to-from);
	maxDist = worldRay.dir.normalize();
	
	bool result = rbactor.GetNxActor().getShapes()[index]->raycast(worldRay, maxDist, hintFlags, hit, firstHit);

	if (details && result)
	{
		details->distance = hit.distance;
		details->mtl = hit.materialIndex;
		details->position = Nx(hit.worldImpact);
		details->normal = Nx(hit.worldNormal);
	}

	return result;
}

//Протестировать элемент на пересечение с боксом
bool PhysCombined::OverlapBox(unsigned int index, const Vector& size, const Matrix& transform )
{
	if (desc) return false;
	if (index >= rbactor.GetNxActor().getNbShapes()) return false;

	NxMat33 rot;
	NxBox box(Nx(transform.pos), Nx(size*0.5f), Nx(rot, transform));
	return rbactor.GetNxActor().getShapes()[index]->checkOverlapOBB(box);
}

//Протестировать элемент на пересечение со сферой
bool PhysCombined::OverlapSphere(unsigned int index, const Vector& center, float radius)
{
	if (desc) return false;
	if (index >= rbactor.GetNxActor().getNbShapes()) return false;

	NxSphere sph(Nx(center), radius);
	return rbactor.GetNxActor().getShapes()[index]->checkOverlapSphere(sph);
}

PhysRigidBodyConnector::PhysRigidBodyConnector(const char * filename, long fileline, IPhysicsScene * _scene, IPhysRigidBody * _left, IPhysRigidBody * _right, float brokeForce) :
IPhysRigidBodyConnector(_scene)
{
	SetFileLine(filename, fileline);

	left = _left;
	right = _right;
	Assert(left);
	Assert(right);
    NxFixedJointDesc fixedDesc;
	PhysInternal nxFirst;
	PhysInternal nxSecond;
	_left->GetInternals(nxFirst);
	_right->GetInternals(nxSecond);
	fixedDesc.actor[0] = nxFirst.actor;
	fixedDesc.actor[1] = nxSecond.actor;
	joint = (NxFixedJoint *)((PhysicsScene *)scene)->Scene().createJoint(fixedDesc);
	Assert(joint);
	joint->setBreakable((NxReal)brokeForce, (NxReal)brokeForce*0.105f);
}

PhysRigidBodyConnector::~PhysRigidBodyConnector()
{
	((PhysicsScene *)scene)->Scene().releaseJoint(*joint);
}

//Сломано соединение или нет
bool PhysRigidBodyConnector::IsBroke()
{	
	return joint->getState() == NX_JS_BROKEN;
}

//Поставить обработчик
void PhysRigidBodyConnector::SetEventHandler(BrokeEvent * event)
{
	Assert(false);
}

//Получить первый объект к которому присоеденены
IPhysRigidBody * PhysRigidBodyConnector::GetLeft()
{
	return left;
}

//Получить второй объект к которому присоеденены
IPhysRigidBody * PhysRigidBodyConnector::GetRight()
{
	return right;
}


