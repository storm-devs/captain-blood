#pragma once
#include "common.h"
#include "PhysicsService.h"
#include "PhysInternal.h"

class PhysCloth : public IPhysCloth
{
	PxCloth*			m_physCloth;
	// FIX_PX3 NxMeshData
	//NxMeshData			m_internalSimData;
	IClothMeshBuilder&	m_builder;

	// FIX_PX3 Don't need NxStream
	//PhysicsService::MemoryWriteStream m_sleepStream;
	bool  m_isSleeping;

public:
	PhysCloth(	const char * filename, long fileline, 
				IPhysicsScene& physScene,
				IClothRenderInfo& render,
				IClothMeshBuilder& builder,
				SimulationData& externalSimData,
				float density);
	virtual ~PhysCloth();

public:
	// присоединить ткань ко всем объектам которые она пересекает
	virtual void Attach(bool twoWayInteraction = false, bool tearable = false);
	// присоединить ткань ближайшим вертексом к мировой точке
	virtual unsigned int Attach(const Vector& pos);
	// присоединить ткань заданным вертексом к мировой точке
	virtual void Attach(unsigned int vertexID, const Vector& pos);
	// присоединить ткань к физ. телу
	virtual void Attach(IPhysRigidBody& body, bool twoWayInteraction = false, bool tearable = false);
	// присоединить ткань к заданным элементам физ. тела
	virtual void Attach(IPhysRigidBody& body, const array<unsigned int>& shapes, bool twoWayInteraction = false, bool tearable = false);
	// присоединить ткань к заданным элементам физ. тела
	virtual void Attach(IPhysRigidBody& body, int shapeIndex, const Vector & vPos, int vertexID, bool twoWayInteraction = false, bool tearable = false);
	// отсоединить все закрепленные точки ткани
	virtual void Detach();
	// отсоединить закрепленную точку ткани
	virtual void Detach(unsigned int vertexID);
	// отсоединить все точки закрепленные на физическом теле
	virtual void Detach(IPhysRigidBody& body);
	// отсоединить все точки закрепленные на элементах физ. тела
	virtual void Detach(IPhysRigidBody& body, const array<unsigned int>& shapes);

	// приложить силу/импульс в точку ткани
	void AddForceAtPos(const Vector & pos, float magnitude, float radius, PhysForceMode forceMode);

	// получить ABB ткани
	virtual Box GetABB() const;
	// получить кол-во вершин в ткани
	virtual unsigned int GetVertexCount() const;
	// разорвать ткань в заданной точке
	virtual void Tear(unsigned int vertexID, bool tearWholeTriangle = false);
	// разорвать ткань в заданной точке
	virtual void TearVertex(unsigned int vertexID, const Vector & normal);
	// проверить пересечение ткани и луча
	virtual bool Raycast(const Vector& from, const Vector& to, Vector * hitPos = null, unsigned int * vertexID = null) const;

	// установить коэффициент разрыва ткани
	virtual void SetTearCoef(float coef);
	// получить коэффициент разрыва ткани
	virtual float GetTearCoef() const;
	// установить коэффициент разрыва в точке крепления ткани
	virtual void SetAttachmentTearCoef(float coef);
	// получить коэффициент разрыва в точке крепления ткани
	virtual float GetAttachmentTearCoef() const;
	// установить ускорение
	virtual void SetAcceleration(const Vector& accel );
	// получить ускорение
	virtual Vector GetAcceleration() const;
	// установить кол-во итераций солвера
	virtual void SetSolverIterations(unsigned int count);

	// сохранить внутреннее состояние и прекратить симуляцию
	virtual void Sleep();
	// восстановить внутреннее состояние и продолжить симуляцию
	virtual void WakeUp(const Matrix& mtx);
	// применить трансформацию к каждой вершине ткани
	virtual void Transform(const Matrix& mtx);
	// задать бокс где будет жить ткань, при выходе за этот бокс вершины ткани удаляются
	virtual void SetValidBox(const Vector& min, const Vector& max);
	// включить/выключить использование ValidBox'а
	virtual void EnableValidBox(bool enable);

	// получить внутренности - !только для использования внутри сервиса
	virtual void GetInternals(PhysInternal & internals) const { internals.cloth = m_physCloth; }
};