#include "PhysCloth.h"
#include "PhysicsScene.h"

PhysCloth::PhysCloth(	const char * filename, long fileline, 
						IPhysicsScene& physScene,
						IClothRenderInfo& render,
						IClothMeshBuilder& builder,
						SimulationData& externalSimData,
						float density) :
	IPhysCloth(&physScene),
	m_physCloth(NULL),
	m_isSleeping(false),
	m_builder(builder)
{
	SetFileLine(filename, fileline);

	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::PhysCloth");
	/*
	m_internalSimData.maxVertices = externalSimData.maxVertexCount;
	m_internalSimData.maxIndices = externalSimData.maxIndexCount;
	m_internalSimData.maxParentIndices = 0;
	if (externalSimData.parentIB)
		m_internalSimData.maxParentIndices = m_internalSimData.maxVertices;
	
	m_internalSimData.verticesPosBegin = externalSimData.posBuffer;
	m_internalSimData.verticesNormalBegin = NULL;
	if (render.NeedNormals()) // указываем на нормали, если надо
		m_internalSimData.verticesNormalBegin = ((char*)m_internalSimData.verticesPosBegin) + render.GetNormalOffset();

	m_internalSimData.indicesBegin = externalSimData.ib;
	m_internalSimData.parentIndicesBegin = externalSimData.parentIB;

	m_internalSimData.indicesByteStride			= externalSimData.indexStride;
	m_internalSimData.verticesPosByteStride		= externalSimData.posStride;
	m_internalSimData.verticesNormalByteStride	= externalSimData.posStride;
	m_internalSimData.parentIndicesByteStride	= 0;
	if (externalSimData.parentIB)
		m_internalSimData.parentIndicesByteStride = sizeof(unsigned int);

	m_internalSimData.numIndicesPtr			= &externalSimData.indexCount;
	m_internalSimData.numVerticesPtr		= &externalSimData.vertexCount;
	m_internalSimData.numParentIndicesPtr	= &externalSimData.parentIndexCount;
	m_internalSimData.dirtyBufferFlagsPtr	= &externalSimData.dirtyFlags;

	m_internalSimData.flags = externalSimData.indexStride==2 ? NX_MDF_16_BIT_INDICES : 0;

	NxClothDesc clothDesc;
	clothDesc.clothMesh = &(m_builder.GetMesh());
	clothDesc.meshData = m_internalSimData;
	clothDesc.stretchingStiffness = 1.0f;
	clothDesc.bendingStiffness = 1.0f;
	clothDesc.dampingCoefficient = 1.0f;
	clothDesc.friction = 1.0f;
	clothDesc.density = density;				// 1.0f - default
	clothDesc.thickness = 0.10f;				// 0.01f - default
	clothDesc.solverIterations = 5;				// 5 - default

	//clothDesc.flags |= NX_CLF_BENDING | NX_CLF_BENDING_ORTHO;
//	clothDesc.flags |= NX_CLF_DAMPING | NX_CLF_COMDAMPING;

	if (m_builder.IsTearable())
		clothDesc.flags |= NX_CLF_TEARABLE;
	if (scene->IsHardware())
		clothDesc.flags |= NX_CLF_HARDWARE;
	//clothDesc.collisionGroup = phys_player;
	//clothDesc.groupsMask.bits0 = 1;
	//clothDesc.groupsMask.bits1 = 0;
	//clothDesc.groupsMask.bits2 = 0;
	//clothDesc.groupsMask.bits3 = 0;

	// FIX-ME Временно добавил отключение коллижена
	clothDesc.flags |= NX_CLF_DISABLE_COLLISION;

	m_physCloth = ((PhysicsScene*)scene)->Scene().createCloth(clothDesc);
	Assert(m_physCloth);
	m_builder.AddRef();
	*/
}

PhysCloth::~PhysCloth()
{
	if (m_physCloth)
	{
		api->Trace("FIX_PX3 rewrite Cloth PhysCloth::~PhysCloth");
		//((PhysicsScene*)scene)->Scene().releaseCloth(*m_physCloth);
		m_physCloth = NULL;
	}
	m_builder.Release();
}

// присоединить ткань ко всем объектам которые она пересекает
void PhysCloth::Attach(bool twoWayInteraction, bool tearable)
{
	unsigned int flags = 0;
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::Attach1");
	/*
	if (twoWayInteraction)
		flags |= NX_CLOTH_ATTACHMENT_TWOWAY;

	if (tearable)
		flags |= NX_CLOTH_ATTACHMENT_TEARABLE;
	 
	m_physCloth->attachToCollidingShapes(flags);
	*/
}

// присоединить ткань ближайшим вертексом к мировой точке
unsigned int PhysCloth::Attach(const Vector& pos)
{
	unsigned int vID = 0;
	float length = 0.0f;
	float min = FLT_MAX;
	array<PxVec3> vPosBuf(__FILE__, __LINE__);
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::Attach2");
	/*
	vPosBuf.AddElements(m_physCloth->getNumberOfParticles());
	m_physCloth->getPositions(&vPosBuf[0]);

	for (dword i = 0; i < vPosBuf.Size(); ++i)
		if ( (length=(Nx(vPosBuf[i])-pos).GetLength()) < min )
		{
			vID = i;
			min = length;
		}
	*/

	Attach(vID, pos);
	return vID;
}

// присоединить ткань заданным вертексом к мировой точке
void PhysCloth::Attach(unsigned int vertexID, const Vector& pos)
{
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::Attach3");
	/*
	Assert(vertexID < m_physCloth->getNumberOfParticles());

	m_physCloth->attachVertexToGlobalPosition(vertexID, Nx(pos));
	*/
}

// присоединить ткань к физ. телу
void PhysCloth::Attach(IPhysRigidBody& body, bool twoWayInteraction, bool tearable)
{
	unsigned int flags = 0;
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::Attach4");
	/*
	if (twoWayInteraction)
		flags |= NX_CLOTH_ATTACHMENT_TWOWAY;

	if (tearable)
		flags |= NX_CLOTH_ATTACHMENT_TEARABLE;

	PhysInternal NxActor;
	body.GetInternals(NxActor);
	dword shapesCount = NxActor.actor->getNbShapes();
	for (dword i = 0; i < shapesCount; ++i)
		m_physCloth->attachToShape(NxActor.actor->getShapes()[i], flags);
	*/
}

// присоединить ткань к заданным элементам физ. тела
void PhysCloth::Attach(IPhysRigidBody& body, const array<unsigned int>& shapes, bool twoWayInteraction, bool tearable)
{
	unsigned int flags = 0;
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::Attach5");
	/*
	if (twoWayInteraction)
		flags |= NX_CLOTH_ATTACHMENT_TWOWAY;

	if (tearable)
		flags |= NX_CLOTH_ATTACHMENT_TEARABLE;

	PhysInternal NxActor;
	body.GetInternals(NxActor);
		
	dword shapesCount = shapes.Size();
	for (dword i = 0; i < shapesCount; ++i)
		m_physCloth->attachToShape(NxActor.actor->getShapes()[shapes[i]], flags);
	*/
}

// присоединить ткань к заданным элементам физ. тела
void PhysCloth::Attach(IPhysRigidBody& body, int shapeIndex, const Vector & vPos, int vertexID, bool twoWayInteraction, bool tearable)
{
	unsigned int flags = 0;
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::Attach6");
	/*
	if (twoWayInteraction)
		flags |= NX_CLOTH_ATTACHMENT_TWOWAY;

	if (tearable) 
		flags |= NX_CLOTH_ATTACHMENT_TEARABLE;
	
	PhysInternal phiActor;
	body.GetInternals( phiActor );

	NxShape * pShape = phiActor.actor->getShapes()[shapeIndex];
	m_physCloth->attachVertexToShape(vertexID, pShape, *(NxVec3*)&vPos, flags);
	*/
}

// отсоединить все закрепленные точки ткани
void PhysCloth::Detach()
{
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::Detach1");
	/*
	dword vertCount = m_physCloth->getNumberOfParticles();
	for (dword i = 0; i < vertCount; ++i)
		m_physCloth->freeVertex(i);
	*/
}

// отсоединить закрепленную точку ткани
void PhysCloth::Detach(unsigned int vertexID)
{
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::Detach2");
	/*
	Assert(vertexID < m_physCloth->getNumberOfParticles());

	m_physCloth->freeVertex(vertexID);
	*/
}

// отсоединить все точки закрепленные на физическом теле
void PhysCloth::Detach(IPhysRigidBody& body)
{
	PhysInternal NxActor;
	body.GetInternals(NxActor);

	dword shapesCount = NxActor.actor->getNbShapes();
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::Detach3");
	/*
	for (dword i = 0; i < shapesCount; ++i)
		m_physCloth->detachFromShape(NxActor.actor->getShapes()[i]);
	*/
}

// отсоединить все точки закрепленные на элементах физ. тела
void PhysCloth::Detach(IPhysRigidBody& body, const array<unsigned int>& shapes)
{
	PhysInternal NxActor;
	body.GetInternals(NxActor);

	dword shapesCount = shapes.Size();
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::Detach4");
	/*
	for (dword i = 0; i < shapesCount; ++i)
		m_physCloth->detachFromShape(NxActor.actor->getShapes()[shapes[i]]);
	*/
}

// получить ABB ткани
Box PhysCloth::GetABB() const
{
	PxBounds3 bounds = m_physCloth->getWorldBounds();
	PxVec3 dims = bounds.getDimensions();
	PxVec3 center = bounds.getCenter();

	Box box;
	box.center = Nx(center);
	box.size = Nx(dims);
	return box;
}

// получить кол-во вершин в ткани
unsigned int PhysCloth::GetVertexCount() const
{
	return m_physCloth->getNbParticles();
}

// приложить силу/импульс в точку ткани
void PhysCloth::AddForceAtPos(const Vector & pos, float magnitude, float radius, PhysForceMode forceMode)
{
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::AddForceAtPos");
	//m_physCloth->addForceAtPos(Nx(pos), magnitude, radius, Nx(forceMode));
}

// разорвать ткань в заданной точке
void PhysCloth::TearVertex(unsigned int vertexID, const Vector & normal)
{
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::TearVertex");
	/*
	Assert(vertexID < m_physCloth->getNumberOfParticles());

	m_physCloth->tearVertex(vertexID, Nx(normal));
	*/
}

// разорвать ткань в заданной точке
void PhysCloth::Tear(unsigned int vertexID, bool tearWholeTriangle)
{
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::Tear");
	/*
	Assert(vertexID < m_physCloth->getNumberOfParticles());

	m_physCloth->tearVertex(vertexID, NxVec3(1.0f, 1.0f, 0.0f));
	if (tearWholeTriangle)
	{
		m_physCloth->tearVertex(vertexID, NxVec3(-1.0f, 1.0f, 0.0f));
		m_physCloth->tearVertex(vertexID, NxVec3(0.0f, 1.0f, 0.0f));
		m_physCloth->tearVertex(vertexID, NxVec3(1.0f, 0.0f, 0.0f));
	}
	*/
}

// проверить пересечение ткани и луча
bool PhysCloth::Raycast(const Vector& from, const Vector& to, Vector * hitPos, unsigned int * vertexID) const
{
	NxRay ray;
	PxVec3 pos;
	unsigned int vID;

	ray.orig = Nx(from);
	ray.dir = Nx(to-from);
	float maxDist = ray.dir.normalize();
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::Raycast");
	/*
	bool result = m_physCloth->raycast(ray, pos, vID);

	if (!result || (Nx(pos)-from).GetLength() > maxDist)
		return false;
	*/

	if (hitPos)
		*hitPos = Nx(pos);
	if (vertexID)
		*vertexID = vID;
	
	return true;
}


// установить коэффициент разрыва ткани
void PhysCloth::SetTearCoef(float coef)
{
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::SetTearCoef");
	//m_physCloth->setTearFactor(coef);
}

// получить коэффициент разрыва ткани
float PhysCloth::GetTearCoef() const
{
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::GetTearCoef");
	//return m_physCloth->getTearFactor();
	return 0.0f;
}

// установить коэффициент разрыва в точке крепления ткани
void PhysCloth::SetAttachmentTearCoef(float coef)
{
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::SetAttachmentTearCoef");
	//m_physCloth->setAttachmentTearFactor(coef);
}

// получить коэффициент разрыва в точке крепления ткани
float PhysCloth::GetAttachmentTearCoef() const
{
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::GetAttachmentTearCoef");
	//return m_physCloth->getAttachmentTearFactor();
	return 0.0f;
}

// установить ускорение
void PhysCloth::SetAcceleration(const Vector& accel )
{
	m_physCloth->setExternalAcceleration(Nx(accel));
}

// получить ускорение
Vector PhysCloth::GetAcceleration() const
{
	return Nx(m_physCloth->getExternalAcceleration());
}

// установить кол-во итераций солвера
void PhysCloth::SetSolverIterations(unsigned int count)
{
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::SetSolverIterations");
	//m_physCloth->setSolverIterations(count);
}

// сохранить внутреннее состояние и прекратить симуляцию
void PhysCloth::Sleep()
{
	if ( m_isSleeping )
		return;

	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::Sleep");
	//m_physCloth->setFlags( m_physCloth->getFlags() | NX_CLF_STATIC );
	
	m_isSleeping = true;
}

// восстановить внутреннее состояние и продолжить симуляцию
void PhysCloth::WakeUp(const Matrix& mtx)
{
	if ( !m_isSleeping )
		return;

	Transform(mtx);
	
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::WakeUp");
	//m_physCloth->setFlags( m_physCloth->getFlags() & ~NX_CLF_STATIC );
	m_physCloth->wakeUp();
	m_isSleeping = false;
}

// применить трансформацию к каждой вершине ткани
void PhysCloth::Transform(const Matrix& mtx)
{
	array<PxVec3> vPosBuf(__FILE__, __LINE__);
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::Transform");
	/*
	vPosBuf.AddElements(m_physCloth->getNumberOfParticles());
	m_physCloth->getPositions(&vPosBuf[0]);

	for (dword i = 0; i < vPosBuf.Size(); ++i)
		vPosBuf[i] = Nx(mtx.MulVertex(Nx(vPosBuf[i])));

	m_physCloth->setPositions(&vPosBuf[0]);
	*/
}

// задать бокс где будет жить ткань, при выходе за этот бокс вершины ткани удаляются
void PhysCloth::SetValidBox(const Vector& min, const Vector& max)
{
	PxBounds3 bounds(Nx(min), Nx(max));
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::SetValidBox");
	//m_physCloth->setValidBounds(bounds);
}

// включить/выключить использование ValidBox'а
void PhysCloth::EnableValidBox(bool enable)
{
	api->Trace("FIX_PX3 rewrite Cloth PhysCloth::EnableValidBox");
	/*
	if (enable)
		m_physCloth->setFlags(m_physCloth->getFlags() | NX_CLF_VALIDBOUNDS);
	else
		m_physCloth->setFlags(m_physCloth->getFlags() & ~NX_CLF_VALIDBOUNDS);
	*/
}
