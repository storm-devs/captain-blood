#pragma once

class NxClothMesh;
class NxPhysicsSDK;
class IPhysicsScene;
class IPhysRigidBody;
struct PhysInternal;
class Vector;

// интерфейс получения информации от рендера ткани
class IClothRenderInfo
{
protected:
	virtual ~IClothRenderInfo() {}
	long ref;

public:
	IClothRenderInfo() { ref = 1; }

	void Release() { if (--ref==0) delete this; }
	void AddRef() { ref++; }

	// дать размер вершины в байтах
	virtual unsigned int GetSingleVertexSize() const = null;
	// дать смещение нормали внутри вершины
	virtual unsigned int GetNormalOffset() const = null;
	// дать смещение текстурных координат внутри вершины
	virtual unsigned int GetTexCoordOffset() const = null;
	// дать размер индекса в байтах
	virtual unsigned int GetSingleIndexSize() const = null;
	// сказать нужны ли нормали
	virtual bool NeedNormals() const = null;
	// сказать нужны ли текстурные координаты
	virtual bool NeedTexCoords() const = null;
};

// физическая ткань
class IPhysCloth : public IPhysBase
{
	friend class ClothProxy;
public:
	// данные симуляции ткани
	class SimulationData
	{
		virtual ~SimulationData() {DELETE(posBuffer); DELETE(ib); DELETE(parentIB);}
		int nref;

	public:
		SimulationData() :
			posBuffer(NULL),
			ib(NULL),
			parentIB(NULL),
			vertexCount(0),
			indexCount(0),
			parentIndexCount(0),
			maxVertexCount(0),
			maxIndexCount(0),
			posStride(0),
			indexStride(0) {nref=1;}
		void Release() {if(--nref==0) delete this;}
		void AddRef() {nref++;}

		enum DirtyFlags { VBDirty = 3, IBDirty = 4, PIBDirty = 8 };

		void* posBuffer;					// начало буфера позиций
		void* ib;							// начало буфера индексов
		unsigned int* parentIB;				// начало буфера ремапов (изначальных индексов дублированных вершин)
		
		unsigned int vertexCount;			// сюда PhysX пишет текущее кол-во вершин
		unsigned int indexCount;			// сюда PhysX пишет текущее кол-во индексов
		unsigned int parentIndexCount;		// кол-во ремап-индексов

		unsigned int maxVertexCount;		// максимальное кол-во вершин
		unsigned int maxIndexCount;			// максимальное кол-во индексов

		unsigned int posStride;				// шаг позиций (байты)
		unsigned int indexStride;			// шаг индексов

		unsigned int dirtyFlags;			// флаги наличия обновлений в буферах ткани с предыдущей симуляции
	};

protected:
	IPhysCloth ( IPhysicsScene * pPhysScene) : IPhysBase ( pPhysScene ){};

	virtual	~IPhysCloth (){};

	// ставит в очередь пересоздание всех внутреннестей cloth объекта
	virtual void RealRecreate() {};

public:
	// присоединить ткань ко всем объектам которые она пересекает
	virtual void Attach(bool twoWayInteraction = false, bool tearable = false) = null;
	// присоединить ткань ближайшим вертексом к мировой точке
	virtual unsigned int Attach(const Vector& pos) = null;
	// присоединить ткань заданным вертексом к мировой точке
	virtual void Attach(unsigned int vertexID, const Vector& pos) = null;
	// присоединить ткань к физ. телу
	virtual void Attach(IPhysRigidBody& body, bool twoWayInteraction = false, bool tearable = false) = null;
	// присоединить ткань к заданным элементам физ. тела
	virtual void Attach(IPhysRigidBody& body, const array<unsigned int>& shapes, bool twoWayInteraction = false, bool tearable = false) = null;
	// присоединить ткань к заданным элементам физ. тела
	virtual void Attach(IPhysRigidBody& body, int shapeIndex, const Vector & vPos, int vertexID, bool twoWayInteraction = false, bool tearable = false) = null;
	// отсоединить все закрепленные точки ткани
	virtual void Detach() = null;
	// отсоединить закрепленную точку ткани
	virtual void Detach(unsigned int vertexID) = null;
	// отсоединить все точки закрепленные на физическом теле
	virtual void Detach(IPhysRigidBody& body) = null;
	// отсоединить все точки закрепленные на элементах физ. тела
	virtual void Detach(IPhysRigidBody& body, const array<unsigned int>& shapes) = null;

	// получить ABB ткани
	virtual Box GetABB() const = null;
	// получить кол-во вершин в ткани
	virtual unsigned int GetVertexCount() const = null;
	// разорвать ткань в заданной точке
	virtual void Tear(unsigned int vertexID, bool tearWholeTriangle = false) = null;
	// разорвать ткань в заданной точке
	virtual void TearVertex(unsigned int vertexID, const Vector & normal) = null;
	// проверить пересечение ткани и луча
	virtual bool Raycast(const Vector& from, const Vector& to, Vector * hitPos = null, unsigned int * vertexID = null) const = null;

	// приложить силу/импульс в точку ткани
	virtual void AddForceAtPos(const Vector & pos, float magnitude, float radius, PhysForceMode forceMode) = null;

	// установить коэффициент разрыва ткани
	virtual void SetTearCoef(float coef) = null;
	// получить коэффициент разрыва ткани
	virtual float GetTearCoef() const = null;
	// установить коэффициент разрыва в точке крепления ткани
	virtual void SetAttachmentTearCoef(float coef) = null;
	// получить коэффициент разрыва в точке крепления ткани
	virtual float GetAttachmentTearCoef() const = null;
	// установить ускорение
	virtual void SetAcceleration(const Vector& accel ) = null;
	// получить ускорение
	virtual Vector GetAcceleration() const = null;
	// установить кол-во итераций солвера
	virtual void SetSolverIterations(unsigned int count) = null;

	// сохранить внутреннее состояние и прекратить симуляцию
	virtual void Sleep() = null;
	// восстановить внутреннее состояние и продолжить симуляцию
	virtual void WakeUp(const Matrix& mtx) = null;
	// применить трансформацию к каждой вершине ткани
	virtual void Transform(const Matrix& mtx) = null;
	// задать бокс где будет жить ткань, при выходе за этот бокс вершины ткани удаляются
	virtual void SetValidBox(const Vector& min, const Vector& max) = null;
	// включить/выключить использование ValidBox'а
	virtual void EnableValidBox(bool enable) = null;

	// получить внутренности - !только для использования внутри сервиса
	virtual void GetInternals(PhysInternal & internals) const = null;

	// ставит в очередь пересоздание всех внутреннестей cloth объекта
	virtual void Recreate() {};
};

