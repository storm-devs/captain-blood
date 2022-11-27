
#include "PhysicsScene.h"
#include "PhysicsService.h"
#include "..\Common_h\Render.h"
#include "..\Common_h\FileService.h"
#include "PhysCharacter.h"
#include "PhysRigidBody.h"
#include "PhysCloth.h"
#include "PhysTriangleMesh.h"
#include "PhysRagdoll.h"
#include "PhysMaterial.h"
#include "NxCooking.h"

#include "ClothProxy.h"
#include "CombinedProxy.h"
#include "PlaneProxy.h"
#include "BoxProxy.h"
#include "MaterialProxy.h"
#include "SphereProxy.h"
#include "CapsuleProxy.h"
#include "MeshProxy.h"
#include "CharacterProxy.h"
#include "RagdollProxy.h"
#include "ConnectorProxy.h"

#include "time.h"

//============================================================================================

#define CONTACT_FORCE_THRESHOLD		0.5f


PhysicsScene::PhysicsScene(PhysicsService * srv) :
	m_objects(_FL_, 32),
	m_materials(_FL_),
	m_proxies(_FL_, 32),
	m_meshes(_FL_, 32),
	m_conproxies(_FL_),
	m_clothproxies(_FL_),
	m_hSimulateDoneEvent(NULL),
	m_lastDeltaTime(0.0f),
	m_normalizeObjectIndex(0),
	m_debugDraw(false),
	m_OverlapShapes(_FL_, 4),
	m_spheres(_FL_, 1024)
{
	m_contactReport.m_scene = this;
	m_service = srv;
	Assert(m_service);
	
	InitScene();

	if (IsMultiThreading())
	{
		m_hSimulateDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);		Assert(m_hSimulateDoneEvent);
	}

	m_sceneChangedIndex = 0;
}

PhysicsScene::~PhysicsScene()
{
	
}

void PhysicsScene::InitScene()
{
	//Создаём сцену
	NxSceneDesc sceneDesc;
	sceneDesc.gravity = NxVec3(0.0f, -9.8f, 0.0f);
	//отключаем обработку физики в другом потоке средствами physx'a
	sceneDesc.flags &= ~NX_SF_SIMULATE_SEPARATE_THREAD;

	m_scene = m_service->physicsSDK->createScene(sceneDesc);
	Assert(m_scene);
	m_service->scenes.Add(this);
	
	//m_scene->setTiming(1.0f/60.0f, 8, NX_TIMESTEP_FIXED);
	//m_scene->setTiming(1.0f/60.0f, 8, NX_TIMESTEP_VARIABLE);

	m_currentTime = 0.0f;
	NxMaterial * defaultMaterial = m_scene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution((NxReal)0.0);
	defaultMaterial->setStaticFriction((NxReal)0.5);
	defaultMaterial->setDynamicFriction((NxReal)0.5);

	PhysicsCollisionGroup groups[] = {	phys_world,				// 1
										phys_player,			// 2
										phys_character,			// 3
										phys_ragdoll,			// 4
										phys_particles,			// 5
										phys_ship,				// 6
										phys_grp1,				// 7
										phys_grp2,				// 8
										phys_grp3,				// 9
										phys_grp4,				// 10
										phys_nocollision,		// 11
										phys_pair,				// 12
										phys_playerctrl,		// 13
										phys_physobjects,		// 14
										phys_charitems,			// 15
										phys_bloodpatch,		// 16
										phys_enemy,				// 17
										phys_ally,				// 18
										phys_boss,				// 19
										phys_grass};			// 20

	const unsigned int dim = sizeof(groups)/sizeof(groups[0]);
	unsigned char collisions[dim][dim] =
		//матрецо коллизий					1	2	3	4	5	6	7	8	9	10	11	12	13	14  15  16  17  18  19  20
										{{	1,	0,	0,	1,	0,	1,	0,	0,	0,	0,	0,	0,	1,	1,  0,  0,  0,  0,  0,  0},	 // 1
										{	0,	1,	1,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,  0,  0,  0,  0,  0,  0},	 // 2
										{	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,  0,  0,  0,  0,  0,  0},	 // 3
										{	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,  0,	0,  0,  0,  0,  0,  0,  0},	 // 4
										{	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,  0,  0,  0,  0,  0,  0},	 // 5
										{	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,  0,  0,  0,  0,  0,  0},	 // 6
										{	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,  0,  0,  0,  0,  0,  0},	 // 7
										{	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,  0,  0,  0,  0,  0,  0},	 // 8
										{	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,  0,  0,  0,  0,  0,  0},	 // 9
										{	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,  0,  0,  0,  0,  0,  0},	 // 10
										{	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,  0,  0,  0,  0,  0,  0},	 // 11
										{	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,  0,  0,  0,  0,  0,  0},	 // 12
										{	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	1,  0,  0,  0,  0,  0,  0},	 // 13
										{	1,	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	1,	1,  0,  0,  0,  0,  0,  0},	 // 14
										{	0,	0,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,  0,  0,  0,  0,  0,  0},	 // 15
										{	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,  0,  1,  0,  0,  0,  0},  // 16
										{	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,  0,  0,  1,  0,  0,  0},  // 17
										{	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,  0,  0,  0,  1,  0,  0},  // 18
										{	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,  0,  0,  0,  0,  1,  0},	 // 19
										{	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,  0,  0,  0,  0,  0,  1}}; // 20

	for(unsigned int i = 0; i < sizeof(groups)/sizeof(groups[0]); ++i)
		for(unsigned int j = 0; j < sizeof(groups)/sizeof(groups[0]); ++j)
		{
			m_scene->setGroupCollisionFlag(groups[i], groups[j], collisions[j][i] || collisions[i][j]);
		}

	m_scene->setActorGroupPairFlags(phys_world, phys_physobjects, NX_NOTIFY_FORCES | NX_NOTIFY_ON_START_TOUCH);
	m_scene->setActorGroupPairFlags(phys_world, phys_charitems, NX_NOTIFY_FORCES | NX_NOTIFY_ON_START_TOUCH);
	m_scene->setActorGroupPairFlags(phys_physobjects, phys_physobjects, NX_NOTIFY_FORCES | NX_NOTIFY_ON_START_TOUCH);
	m_scene->setUserContactReport(&m_contactReport);

	// рождаем материалы
	m_materials.Add(NEW PhysMaterial(_FL_, this, m_scene->getMaterialFromIndex(0)));
	for (unsigned int i = mtl_default+1; i < mtl_last; ++i)
		CreatePhysMaterial(_FL_);
}

void PhysicsScene::ReleaseScene()
{
	for (int i = m_objects.Size()-1; i >= 0; --i)
		m_objects[i]->ForceRelease();
	m_objects.DelAll();

	//for (int i = m_proxies.Size() - 1; i >= 0 ; --i) 
	//	m_proxies[i]->OnSyncRelease();
	m_proxies.DelAll();

	//for (int i = m_conproxies.Size() - 1; i >= 0 ; --i)		
	//	m_conproxies[i]->OnSyncRelease();
	m_conproxies.DelAll();

	//for (int i = m_clothproxies.Size() - 1; i >= 0 ; --i)	
	//	m_clothproxies[i]->OnSyncRelease();
	m_clothproxies.DelAll();

	for (unsigned int i = 0; i < m_materials.Size(); ++i)
		delete m_materials[i];
	m_materials.DelAll();

	m_service->scenes.Del(this);
	if(m_scene) m_service->physicsSDK->releaseScene(*m_scene);
	m_scene = null;
}

//Удалить сцену
void PhysicsScene::Release()
{
	if (IsMultiThreading())
	{
		CloseHandle(m_hSimulateDoneEvent);
		m_hSimulateDoneEvent = null;
	}

	ReleaseScene();

	delete this;
}

//Создать плоскость
IPhysPlane * PhysicsScene::CreatePlane(const char * cppfile, long cppline, const Vector & n, float d)
{
	PlaneProxy * p = NEW PlaneProxy(n, d, *this);
	p->SetFileLine(cppfile, cppline);
	m_objects.Add((IPhysBase *)p);
	m_proxies.Add(p);
	return p;
}

//Создать ящик
IPhysBox * PhysicsScene::CreateBox(const char * cppfile, long cppline, const Vector & size, const Matrix & transform, bool isDynamic, float density)
{
	BoxProxy * p = NEW BoxProxy(size, transform, isDynamic, density, *this);
	p->SetFileLine(cppfile, cppline);
	m_objects.Add(p);
	m_proxies.Add(p);
	return p;
}

//Создать шар
IPhysSphere * PhysicsScene::CreateSphere(const char * cppfile, long cppline, float radius, const Matrix & transform, bool isDynamic, float density)
{
	SphereProxy * p = NEW SphereProxy(radius, transform, isDynamic, density, *this);
	p->SetFileLine(cppfile, cppline);
	m_objects.Add(p);
	m_proxies.Add(p);
	return p;
}

//Создать капсулу
IPhysCapsule * PhysicsScene::CreateCapsule(const char * cppfile, long cppline, float radius, float height, const Matrix & transform, bool isDynamic, float density)
{
	CapsuleProxy * p = NEW CapsuleProxy(radius, height, transform, isDynamic, density, *this);
	p->SetFileLine(cppfile, cppline);
	m_objects.Add(p);
	m_proxies.Add(p);
	return p;
}

//Создать объект состоящий из множества фигур
IPhysCombined * PhysicsScene::CreateCombined(const char * cppfile, long cppline, const Matrix & transform, bool isDynamic, float density)
{
	CombinedProxy * p = NEW CombinedProxy(cppfile, cppline, transform, isDynamic, density, *this);
	// setfileline делается внутри combinedproxy
	//p->SetFileLine(cppfile, cppline);
	m_objects.Add(p);
	m_proxies.Add(p);
	return p;
}

// Создать ткань
IPhysCloth * PhysicsScene::CreateCloth (const char * cppfile, long cppline, IClothRenderInfo& render, IClothMeshBuilder& builder, IPhysCloth::SimulationData& dataBuffer, float density)
{
	ClothProxy * pNewCloth = NEW ClothProxy(*this, render, builder, dataBuffer, density);
	pNewCloth->SetFileLine(cppfile, cppline);
	m_objects.Add(pNewCloth);
	m_clothproxies.Add(pNewCloth);
	return pNewCloth;
}

//Создать цельный объект из сетки
IPhysRigidBody * PhysicsScene::CreateMesh(const char * cppfile, long cppline, const MeshInit * meshes, dword numMeshes, bool bDynamic, float density)
{
	MeshProxy * p = NEW MeshProxy(meshes, numMeshes, bDynamic, density, *this);
	p->SetFileLine(cppfile, cppline);
	m_objects.Add(p);
	m_proxies.Add(p);
	return p;
}

//Соединить 2 цельных объекта
IPhysRigidBodyConnector * PhysicsScene::Connect(IPhysRigidBody * left, IPhysRigidBody * right, float brokeForce)
{
	ConnectorProxy * p = NEW ConnectorProxy(*this, left, right, brokeForce);
	m_objects.Add(p);
	m_conproxies.Add(p);
	return p;
}

//Создать физическое представление персонажа (капсулой)
IPhysCharacter * PhysicsScene::CreateCharacter(const char * cppfile, long cppline, float radius, float height)
{
	CharacterProxy * p = NEW CharacterProxy(radius, height, *this);
	p->SetFileLine(cppfile, cppline);
	m_objects.Add(p);
	m_proxies.Add(p);
	return p;
}

void PhysicsScene::Error(const char * error, const char * cppfile, long cppline)
{
	if (error)
		api->Trace(error);

	api->Trace("file: %s, line: %d", cppfile, cppline);
}

//Создать рэгдол
IPhysRagdoll * PhysicsScene::CreateRagdoll(const char * cppfile, long cppline, const void * data, dword dataSize)
{
	if(!data)
	{
		Error("Physics: Can't create ragdoll, invalidate data pointer (null)", cppfile, cppline);
		return null;
	}
	if(!dataSize)
	{
		Error("Physics: Can't create ragdoll, invalidate data size (0)", cppfile, cppline);
		return null;
	}

	RagdollProxy * ragdoll = NEW RagdollProxy(data, dataSize, *this);
	ragdoll->SetFileLine(cppfile, cppline);
	m_objects.Add(ragdoll);
	m_proxies.Add(ragdoll);
	return ragdoll;
}

//Создать редактируемый рэгдол
IPhysEditableRagdoll * PhysicsScene::CreateEditableRagdoll()
{
	RagdollProxy * p = NEW RagdollProxy(*this);
	m_objects.Add(p);
	m_proxies.Add(p);
	return p;
}

IPhysEditableRagdoll * PhysicsScene::CreateRealEditableRagdoll()
{
	PhysRagdoll * p = NEW PhysRagdoll(null, _FL_, this);
	return p;
}

//Создать физический материал
IPhysMaterial * PhysicsScene::CreatePhysMaterial(const char * cppfile, long cppline, float statFriction, float dynFriction, float restitution)
{
	MaterialProxy * mat = NEW MaterialProxy(*this, statFriction, dynFriction, restitution);
	mat->SetFileLine(cppfile, cppline);
	m_materials.Add(mat);
	m_proxies.Add(mat);
	return mat;
}

IPhysMaterial * PhysicsScene::GetPhysMaterial(PhysMaterialGroup group) const
{
	return m_materials[group];
}

class PhysSceneMeshesReport : public NxUserEntityReport<NxShape*> 
{
private:
	// ссылка на массив куда добавляются все прошедшие тест меши
	array<NxTriangleMeshShape*> & m_overlapShapes;

public:
	PhysSceneMeshesReport(array<NxTriangleMeshShape*> & overlapShapes) :
		m_overlapShapes(overlapShapes)
	{
		m_overlapShapes.Empty();
	}

	virtual bool onEvent(NxU32 nbEntities, NxShape * * entities)
	{
		for (NxU32 i=0; i<nbEntities; i++)
		{
			NxTriangleMeshShape * shape = entities[i]->isTriangleMesh();
			if (shape) 
				m_overlapShapes.Add(shape);
		}
		return true;
	}
};

class PhysSceneTrianglesReport : public NxUserEntityReport<NxU32> 
{
private:
	// ссылка на массив куда сохраняются треугольники(просто друг за другом)
	array<Vector> & m_triangles;
	// ссылка на массив куда сохраняются материалы треугольников(просто друг за другом)
	array<PhysTriangleMaterialID> * m_trianglesMaterials;
	// Меш с которого берутся и тестятся эти треугольники
	NxTriangleMeshShape & m_meshShape;

public:
	PhysSceneTrianglesReport(array<Vector> & triangles, array<PhysTriangleMaterialID> * trianglesMaterials, NxTriangleMeshShape & meshShape) :
		m_triangles(triangles),
		m_trianglesMaterials(trianglesMaterials),
		m_meshShape(meshShape)
	{
	}

	virtual bool onEvent(NxU32 nbEntities, NxU32 * entities)
	{
		NxMaterialIndex shape_mtl = m_meshShape.getMaterial();
		NxTriangleMesh & mesh = m_meshShape.getTriangleMesh();

		for (NxU32 i=0; i<nbEntities; i++)
		{
			NxTriangle trg;
			m_meshShape.getTriangle(trg, null, null, entities[i], true, true);

			for (long j=0; j<3; j++)
				m_triangles.Add(Nx(trg.verts[j]));

			// если есть массив с материалами
			if (m_trianglesMaterials)
			{
				NxMaterialIndex material = mesh.getTriangleMaterial(entities[i]);
				if (material != 0xFFFF)
					m_trianglesMaterials->Add((PhysTriangleMaterialID)material);
				else
					m_trianglesMaterials->Add((PhysTriangleMaterialID)shape_mtl);
			}
		}
		return true;
	}
};

// Тестит формы на пересечении со сферой, возвращает объект в котором куча треугольников, или null если ничего нету
bool PhysicsScene::OverlapSphere(const Vector & pos, float radius, dword mask, bool staticShapes, bool dynamicShapes, array<Vector> & aTriangles, array<PhysTriangleMaterialID> * aTriangleMaterials)
{
	NxSphere		nxSphere(Nx(pos), radius);
	
	int nxShapeType = 0;
	if (staticShapes)	nxShapeType |= NX_STATIC_SHAPES;
	if (dynamicShapes)	nxShapeType |= NX_DYNAMIC_SHAPES;
	
	if (!nxShapeType)
		return false;

	// находим меши которые пересекаются со сферой
	PhysSceneMeshesReport reportMeshes(m_OverlapShapes);
	m_scene->overlapSphereShapes(nxSphere, NxShapesType(nxShapeType), 0, null, &reportMeshes, mask, null, false);

	// тестим выбранные меши на треугольники
	aTriangles.Empty();
	if (aTriangleMaterials)
		aTriangleMaterials->Empty();
	NxBounds3 bounds;
	bounds.set(Nx(pos - radius), Nx(pos + radius));
	for (long i=0; i<m_OverlapShapes; i++)
	{
		PhysSceneTrianglesReport reportTriangles(aTriangles, aTriangleMaterials, *m_OverlapShapes[i]);
		m_OverlapShapes[i]->overlapAABBTriangles(bounds, NX_QUERY_WORLD_SPACE, &reportTriangles);
	}

	return !aTriangles.IsEmpty();
}

//Тестит капсулу на пересечение с объектами, возвращает true если было пересечение
bool PhysicsScene::CheckOverlapCapsule(const Vector & pos, float height, float radius, dword mask, bool staticShapes, bool dynamicShapes)
{
	Vector h = Vector(0.0f, height * 0.5f, 0.0f);
	NxSegment seg(Nx(pos - h), Nx(pos + h));
	NxCapsule capsule(seg, radius);

	int	nxShapeType = 0;
	if (staticShapes)	nxShapeType |= NX_STATIC_SHAPES;
	if (dynamicShapes)	nxShapeType |= NX_DYNAMIC_SHAPES;

	if (!nxShapeType)
		return false;

	bool result = m_scene->checkOverlapCapsule(capsule, NxShapesType(nxShapeType), mask);

	return result;
}

//-------------------------------------------------------------------------------------------------------------
//Дополнительные функции
//-------------------------------------------------------------------------------------------------------------

//Трейс луча через все объекты
IPhysBase * PhysicsScene::Raycast(const Vector & from, const Vector & to, dword mask, RaycastResult * detail)
{
	//Направление
	Vector dir = to - from;
	float dist = dir.Normalize();
	//Луч
	NxRay ray;
	ray.orig = Nx(from);
	ray.dir = Nx(dir);
	//Трейсим луч
	NxRaycastHit hit;
	NxShape * shape = m_scene->raycastClosestShape(ray, NX_ALL_SHAPES, hit, mask, dist + 0.000001f);
	if(!shape) return null;
	if(detail)
	{
		detail->position = Nx(hit.worldImpact);
		detail->normal = Nx(hit.worldNormal);
		detail->distance = (float)hit.distance;
		detail->mtl = (MaterialID)hit.materialIndex;
		detail->id = shape->userData;
	}

	IPhysBase * physObj = (IPhysBase *)(shape->getActor().userData);
	if (physObj)
	{
		IPhysBase * proxyObj = physObj->GetProxyObject();
		if (proxyObj)
			return proxyObj;
	}

	return physObj;
}

//Текущий режим работы
bool PhysicsScene::IsHardware()
{
	return m_service->IsHardware();
}

bool PhysicsScene::IsMultiThreading()
{
	return m_service->IsMultiThreading();
}

#ifndef STOP_DEBUG
//Нарисовать отладочную информацию
void PhysicsScene::DebugDraw(IRender & render)
{
	m_debugDraw = true;
}

void PhysicsScene::RealDebugDraw()
{
	if (!m_debugDraw) return;

	IRender & render = *(IRender *)api->GetService("DX9Render");

	const NxDebugRenderable * renderable = m_scene->getDebugRenderable();
	if(!renderable) return;
	const NxDebugRenderable & data = *renderable;
	render.SetWorld(Matrix());
	//Рисуем треугольники
	NxU32 count = data.getNbTriangles();
	const NxDebugTriangle * triangles = data.getTriangles();
	struct DrawTriVertex
	{
		Vector pos;
		dword color;
	} tbuffer[256*3];
	for(NxU32 i = 0, tc = 0; i < count; i++)
	{
		const NxDebugTriangle & trg = triangles[i];
		tbuffer[tc + 0].pos = Nx(trg.p0);
		tbuffer[tc + 0].color = trg.color;
		//tbuffer[tc + 0].color = 0xff00ff00;
		tbuffer[tc + 1].pos = Nx(trg.p1);
		tbuffer[tc + 1].color = trg.color;
		//tbuffer[tc + 1].color = 0xff00ff00;
		tbuffer[tc + 2].pos = Nx(trg.p2);
		tbuffer[tc + 2].color = trg.color;
		//tbuffer[tc + 2].color = 0xff00ff00;
		tc++;
		if(tc >= ARRSIZE(tbuffer)/3)
		{
			ShaderId id;
			render.GetShaderId("stdPolygon", id);
			render.DrawPrimitiveUP(id, PT_TRIANGLELIST, tc, tbuffer, sizeof(tbuffer[0]));
			tc = 0;
		}
	}
	if(tc > 0)
	{
		ShaderId id;
		render.GetShaderId("stdPolygon", id);
		render.DrawPrimitiveUP(id, PT_TRIANGLELIST, tc, tbuffer, sizeof(tbuffer[0]));
	}
	//Рисуем линии
	count = data.getNbLines();
	const NxDebugLine * lines = data.getLines();
	render.FlushBufferedLines();
	for(NxU32 i = 0; i < count; i++)
	{
		const NxDebugLine & line = lines[i];
		
		//render.DrawBufferedLine(Nx(line.p0), line.color, Nx(line.p1), line.color);

		render.DrawBufferedLine(Nx(line.p0), 0xff0000ff, Nx(line.p1), 0xff0000ff);
	}
	render.FlushBufferedLines();
	//Рисуем точки в виде пактиклов
	count = data.getNbPoints();
	const NxDebugPoint * points = data.getPoints();
	for(NxU32 i = 0; i < count; i++)
	{
	}

	// выходим из секции
	if (IsMultiThreading())
	{
		//m_SyncSection.Leave();
	}
}
#endif

//Получить физическую сцену
NxScene & PhysicsScene::Scene()
{
	return *m_scene;
}

//Получить мэнеджер контролеров для персонажей
NxControllerManager & PhysicsScene::CtrManager()
{
	return m_service->CtrManager();
}

//Пересчитать состояние перед началом кадра
void PhysicsScene::UpdateBeginFrame(float dltTime)
{
	m_lastDeltaTime = dltTime;

	if (m_service->isStop) return;

	if (IsMultiThreading() && m_lastDeltaTime > 0.0f)
	{
		// стартуем выполнения физики в отдельном треде
		m_service->AddScene2Execute(this);
		//SetEvent(m_hSimulateStartEvent);
	}
}

void PhysicsScene::SimulationDone()
{
	if (IsMultiThreading())
		SetEvent(m_hSimulateDoneEvent);
}

//Пересчитать состояние после кадра
void PhysicsScene::UpdateEndFrame(float dltTime)
{	
	if (m_service->isStop) return;
	//if(dltTime > 0.1f) dltTime = 0.1f;

	// очищаем все контакты, потому что на fetchResult придут новые
	// обнуляем флажок контактов у всех объектов
	for (int i=0; i<m_objects.Len(); i++)
		m_objects[i]->SetContactReport(-1);

	if (!IsMultiThreading())
	{
		if (dltTime > 0.0f) 
		{
			m_scene->simulate((NxReal)dltTime);
			m_scene->flushStream();

			NxU32 uErrorCode = 0;
			m_scene->fetchResults(NX_RIGID_BODY_FINISHED, true, &uErrorCode );
		}

		ExecuteProxies(m_proxies);
		ExecuteProxies(m_clothproxies);
		ExecuteProxies(m_conproxies);

		CtrManager().updateControllers();
	}
	else
	{
		// ждем эвента на старте выполнения треда
		if (m_lastDeltaTime > 0.0f) 
		{
			// ждем окончания выполнения Simulate
			WaitForSingleObject(m_hSimulateDoneEvent, INFINITE);

			m_scene->fetchResults(NX_RIGID_BODY_FINISHED, true);
		}

		ExecuteProxies(m_proxies);
		ExecuteProxies(m_clothproxies);
		ExecuteProxies(m_conproxies);

		CtrManager().updateControllers();
	}

	// нормализуем 1 объект в кадр
	int normalizeCount = (m_objects.Size() + 15) / 16;
	int normalized = 0;
	while (normalizeCount && normalized < 5)
	{
		if (m_normalizeObjectIndex >= m_objects.Size())
			m_normalizeObjectIndex = 0;

		IPhysBase * realObject = m_objects[m_normalizeObjectIndex]->GetRealObject();
		if (realObject)
			normalized += realObject->Normalize();
		m_normalizeObjectIndex++;
		normalizeCount--;
	}

#ifndef STOP_DEBUG
	DebugFrame();
	if (m_debugDraw)
		RealDebugDraw();
#endif
}

void PhysicsScene::ExecuteProxies(array<IProxy*> & proxies)
{
	for (unsigned int i = 0; i < proxies.Size(); i++)
		proxies[i]->OnSyncCreate();

	for (unsigned int i = 0; i < proxies.Size(); i++)
		proxies[i]->OnSyncCalls();

	for (unsigned int i = 0; i < proxies.Size(); i++)
	{
		if (proxies[i]->OnSyncRelease())
		{
			i--;
		}
	}
}

void PhysicsScene::UnregistryPhysObject(IPhysBase * obj)
{
	// FIX-ME: может быть стоит использовать ExtractNoShift?
	dword idx = m_objects.Find(obj);
	if (idx != INVALID_ARRAY_INDEX)
	{
		if (m_normalizeObjectIndex >= idx) 
			m_normalizeObjectIndex = Min(m_normalizeObjectIndex - 1, m_objects.Size());

		m_objects.DelIndex(idx);
	}

	// удаляем из мешей
	m_meshes.Del(obj);
}

void PhysicsScene::UnregistryProxyObject(IProxy * obj)
{
	// FIX-ME: может быть стоит использовать ExtractNoShift?
	//if (IsMultiThreading())
	{
		m_proxies.Del(obj);
		m_conproxies.Del(obj);
		m_clothproxies.Del(obj);
	}
}

#ifndef STOP_DEBUG
//Ищем прокси объект с данным актером
IProxy * PhysicsScene::FindProxyObject(NxActor * actor)
{
#ifndef _XBOX
	for (long i=0; i<m_proxies; i++)
	{
		try 
		{
			PhysInternal internals;
			IPhysRigidBody * rigid_body = dynamic_cast<IPhysRigidBody*>(m_proxies[i]);
			if (rigid_body)
			{
				rigid_body->GetInternals(internals);
				if (internals.actor == actor)
					return m_proxies[i];
			}
		} 
		catch(...)
		{
		}
	}
#endif
	return null;
}
#endif

//Блокировка physx в мультитред режиме, если он что-то внутри делает, что сломает результаты не threadsafe функций
void PhysicsScene::lockQueries()
{
	if (!IsMultiThreading()) return;
	m_scene->lockQueries();
}

void PhysicsScene::unlockQueries()
{
	if (!IsMultiThreading()) return;
	m_scene->unlockQueries();
}

void PhysicsScene::SceneChanged()
{
	m_sceneChangedIndex++;
}

dword PhysicsScene::GetSceneChangedIndex()
{
	return m_sceneChangedIndex;
}

void PhysicsScene::ContactReport::onContactNotify(NxContactPair & pair, NxU32 events)
{
	Vector force = Nx(pair.sumNormalForce);

	if (pair.isDeletedActor[0] || pair.isDeletedActor[1])
		return;

	if (force.GetLength2() >= CONTACT_FORCE_THRESHOLD)
	{
		NxContactStreamIterator iter(pair.stream);
	
		while (iter.goNextPair())
		{
			//user can also call getShape() and getNumPatches() here
			NxShape * shape0 = iter.getShape(0);
			NxShape * shape1 = iter.getShape(1);

			IPhysBase * receiver = null;

			if (shape0)
			{
				NxActor & actor0 = shape0->getActor();
				receiver = (IPhysBase *)actor0.userData;
			}

			while (iter.goNextPatch())
			{
				while (iter.goNextPoint())
				{
					if (shape1 && receiver && receiver->GetContactReport() == -1)
					{
						NxTriangleMeshShape * shape = shape1->isTriangleMesh();

						// Достаем материал из меша
						if (shape)
						{
							NxTriangleMesh & mesh = shape->getTriangleMesh();
							NxInternalFormat fmt = mesh.getFormat(0, NX_ARRAY_TRIANGLES_REMAP);

							if (fmt == NX_FORMAT_INT && mesh.getSubmeshCount() == 1)
							{
								NxTriangleMesh & mesh = shape->getTriangleMesh();
								int * remap = (int *)mesh.getBase(0, NX_ARRAY_TRIANGLES_REMAP);
								int remapCounts = mesh.getCount(0, NX_ARRAY_TRIANGLES_REMAP);
								int trianglesCounts = mesh.getCount(0, NX_ARRAY_TRIANGLES);

								//if (remap && remapCounts < iter.getFeatureIndex1())
								{
									int trg_index = -1;
									int featureIndex1 = iter.getFeatureIndex1();
									if (remap)
									{
										if (featureIndex1 < remapCounts)
											trg_index = remap[featureIndex1];
									}
									else
									{
										if (featureIndex1 < trianglesCounts)
											trg_index = featureIndex1;
									}

									NxMaterialIndex mat = 0xFFFF;
									if (trg_index >= 0)
										mat = mesh.getTriangleMaterial(trg_index);

									if (mat == 0xFFFF)
										mat = shape->getMaterial();

									//NxActor & actor = shape0->getActor();

									if (mat != 0xFFFF)
									{
										receiver->SetContactReport(dword(mat));
										receiver->SetContactReportPoint(Nx(iter.getPoint()));
										receiver->SetContactReportForce(force);
										//m_scene->m_spheres.Add(Nx(iter.getPoint()));
									}
								}
							}
						}
						else
						{	// Достаем материал из актера второго шейпа
							//NxMaterialIndex mat = shape1->getMaterial();
							NxActor & actor1 = shape1->getActor();
							IPhysRigidBody * collider = (IPhysRigidBody *)actor1.userData;

							if (collider)
							{
								receiver->SetContactReport(collider->GetMaterial());
								receiver->SetContactReportPoint(Nx(iter.getPoint()));
								receiver->SetContactReportForce(Nx(pair.sumNormalForce));
								//m_scene->m_spheres.Add(Nx(iter.getPoint()));
							}
						}
					}
				}
			}
		}
	}
}
