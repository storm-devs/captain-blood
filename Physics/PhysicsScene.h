#pragma once

#include "..\Common_h\Physics.h"
#include "NxDebugRenderable.h"
#include "NxStream.h"
#include "ControllerManager.h"

#include "PhysRigidBody.h"

class PhysicsService;
class IProxy;

//Физическая сцена
class PhysicsScene : public IPhysicsScene
{
//-------------------------------------------------------------------------------------------------------------
//Объекты для представления в сцене
//-------------------------------------------------------------------------------------------------------------
public:
	PhysicsScene(PhysicsService * srv);
	virtual ~PhysicsScene();
	//Удалить сцену
	virtual void Release();
	//Создать плоскость
	virtual IPhysPlane * CreatePlane(const char * cppfile, long cppline, const Vector & n, float d);
	//Создать ящик
	virtual IPhysBox * CreateBox(const char * cppfile, long cppline, const Vector & size, const Matrix & transform, bool isDynamic = true, float density = phys_density);
	//Создать шар
	virtual IPhysSphere * CreateSphere(const char * cppfile, long cppline, float radius, const Matrix & transform, bool isDynamic = true, float density = phys_density);
	//Создать капсулу
	virtual IPhysCapsule * CreateCapsule(const char * cppfile, long cppline, float radius, float height, const Matrix & transform, bool isDynamic = true, float density = phys_density);
	//Создать прокси объект состоящий из множества фигур
	virtual IPhysCombined * CreateCombined(const char * cppfile, long cppline, const Matrix & transform, bool isDynamic = true, float density = phys_density);
	//Создать прокси ткань
	virtual IPhysCloth * CreateCloth (const char * cppfile, long cppline, IClothRenderInfo& render, IClothMeshBuilder& builder, IPhysCloth::SimulationData& dataBuffer, float density = phys_density);
	//Создать цельный объект из сетки
	virtual IPhysRigidBody * CreateMesh(const char * cppfile, long cppline, const MeshInit * meshes, dword numMeshes, bool bDynamic, float density = phys_density);
	//Соеденить 2 цельных объекта
	virtual IPhysRigidBodyConnector * Connect(IPhysRigidBody * left, IPhysRigidBody * right, float brokeForce);

	//Создать физическое представление персонажа (капсулой)
	virtual IPhysCharacter * CreateCharacter(const char * cppfile, long cppline, float radius, float height);

	//Создать рэгдол
	virtual IPhysRagdoll * CreateRagdoll(const char * cppfile, long cppline, const void * data, dword dataSize);
	//Создать редактируемый рэгдол
	virtual IPhysEditableRagdoll * CreateEditableRagdoll();
	IPhysEditableRagdoll * CreateRealEditableRagdoll();

	//Создать физический материал
	virtual IPhysMaterial * CreatePhysMaterial(const char * cppfile, long cppline, float statFriction = 0.5f, float dynFriction = 0.5f, float restitution = 0.5f);
	virtual IPhysMaterial * GetPhysMaterial(PhysMaterialGroup group) const;

	//Тестит формы на пересечении со сферой, возвращает треугольники в массиве, возвращает true если есть хоть один треугольник
	virtual bool OverlapSphere(const Vector & pos, float radius, dword mask, bool staticShapes, bool dynamicShapes, array<Vector> & aTriangles, array<PhysTriangleMaterialID> * aTriangleMaterials = null);
	//Тестит капсулу на пересечение с объектами, возвращает true если было пересечение
	virtual bool CheckOverlapCapsule(const Vector & pos, float height, float radius, dword mask, bool staticShapes, bool dynamicShapes);

	//Говорит сцене о том что она (сцена) изменилась(появились/исчезли объекты, сменились группы и т.п.)
	virtual void SceneChanged();
	//Возвращает последнее индекс когда сцена была изменена(каждое вкл/выкл объектов, смена групп и т.п. увеличивает счетчик) 
	virtual dword GetSceneChangedIndex();

//-------------------------------------------------------------------------------------------------------------
//Дополнительные функции
//-------------------------------------------------------------------------------------------------------------
public:
	// Трейс луча через все объекты
	virtual IPhysBase * Raycast(const Vector & from, const Vector & to, dword mask, RaycastResult * detail);
	// Текущий режим работы
	virtual bool IsHardware();
	// Использование мультипоточности
	virtual bool IsMultiThreading();
	// 
	virtual void Error(const char * error, const char * cppfile, long cppline);

	// Вызывается когда закончилась работа в потоке физике
	virtual void SimulationDone();
	// Возвращает delta time текущего кадра
	virtual float GetLastDeltaTime() { return m_lastDeltaTime; }

#ifndef STOP_DEBUG
	//Нарисовать отладочную информацию
	virtual void DebugDraw(IRender & render);
	virtual void RealDebugDraw();
#endif
	//Получить физическую сцену
	virtual NxScene & Scene();
	//Получить мэнеджер контролеров для персонажей
	NxControllerManager & CtrManager();

	//Пересчитать состояние перед началом кадра
	virtual void UpdateBeginFrame(float dltTime);
	//Пересчитать состояние после кадра
	virtual void UpdateEndFrame(float dltTime);

	//Удалить персонажа
	void DeleteCharacterCtl(NxController * ctr);

	//Ищем прокси объект с данным актером
	IProxy * FindProxyObject(NxActor * actor);

	//Блокировка physx в мультитред режиме, если он что-то внутри делает, что сломает результаты не threadsafe функций
	void lockQueries();
	void unlockQueries();

	virtual void UnregistryProxyObject(IProxy * obj);
//-------------------------------------------------------------------------------------------------------------
private:
	virtual void UnregistryPhysObject(IPhysBase * obj);
	void InitScene();
	void ReleaseScene();
	void ExecuteProxies(array<IProxy*> & proxies);

#ifndef STOP_DEBUG
	void Debug_DrawMaterialsPatches();
	void DebugFrame();
#endif

private:
	PhysicsService *	m_service;
	NxScene *			m_scene;
	float				m_currentTime;
	float				m_lastDeltaTime;
	dword				m_normalizeObjectIndex;
	dword				m_sceneChangedIndex;

	array<IPhysBase*>		m_objects;
	array<IPhysMaterial*>	m_materials;
	array<IProxy*>			m_proxies, m_conproxies, m_clothproxies;
	array<IPhysBase*>		m_meshes;

	array<NxTriangleMeshShape*>		m_OverlapShapes;

	HANDLE				m_hSimulateDoneEvent;

	bool				m_debugDraw;

private:
	class ContactReport : public NxUserContactReport
	{
	public:
		PhysicsScene * m_scene;

		virtual void onContactNotify(NxContactPair & pair, NxU32 events);
	};
	
	ContactReport		m_contactReport;
public:
	array<Vector> m_spheres;
};

