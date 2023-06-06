
class IRender;
class IClothMeshBuilder;
class IMission;

// плотность объекта по умолчанию
#define phys_density			10.0

//Физическая сцена
class IPhysicsScene : public Object
{
protected:
	virtual ~IPhysicsScene() {}

public:

	struct MeshInit
	{
		IPhysTriangleMesh * mesh;
		Matrix mtx;
	};

	typedef unsigned long MaterialID;

	struct RaycastResult
	{
		Vector position;			//Мировая позиция точки пересечения
		Vector normal;				//Нормаль к поверхности в мировой системе
		float distance;				//Дистанция до точки
		MaterialID mtl;				//Идентификатор материала
		PhysRaycastId id;			//Идентификатор для получения дополнительной информации
	};

public:
	//Удалить сцену
	virtual void Release() = null;

//-------------------------------------------------------------------------------------------------------------
//Объекты для представления в сцене
//-------------------------------------------------------------------------------------------------------------
public:
	//Создать плоскость
	virtual IPhysPlane * CreatePlane(const char * cppfile, long cppline, const Vector & n, float d) = null;
	//Создать ящик
	virtual IPhysBox * CreateBox(const char * cppfile, long cppline, const Vector & size, const Matrix & transform, bool isDynamic = true, float fDensity = phys_density) = null;
	//Создать шар
	virtual IPhysSphere * CreateSphere(const char * cppfile, long cppline, float radius, const Matrix & transform, bool isDynamic = true, float fDensity = phys_density) = null;
	//Создать капсулу
	virtual IPhysCapsule * CreateCapsule(const char * cppfile, long cppline, float radius, float height, const Matrix & transform, bool isDynamic = true, float fDensity = phys_density) = null;
	//Создать объект состоящий из множества фигур
	virtual IPhysCombined * CreateCombined(const char * cppfile, long cppline, const Matrix & transform, bool isDynamic = true, float fDensity = phys_density) = null;
	//Создать ткань
	virtual IPhysCloth * CreateCloth (const char * cppfile, long cppline, IClothRenderInfo& render, IClothMeshBuilder& builder, IPhysCloth::SimulationData& dataBuffer, float fDensity = phys_density) = null;
	//Создать цельный объект из сетки
	virtual IPhysRigidBody * CreateMesh(const char * cppfile, long cppline, const MeshInit * meshes, dword numMeshes, bool bDynamic = false, float fDensity = phys_density) = null;
	//Соеденить 2 цельных объекта
	virtual IPhysRigidBodyConnector * Connect(IPhysRigidBody * left, IPhysRigidBody * right, float brokeForce) = null;

	//Создать физическое представление персонажа (капсулой)
	virtual IPhysCharacter * CreateCharacter(const char * cppfile, long cppline, float radius, float height) = null;

	//Создать рэгдол
	virtual IPhysRagdoll * CreateRagdoll(const char * cppfile, long cppline, const void * data, dword dataSize) = null;
	//Создать редактируемый рэгдол
	virtual IPhysEditableRagdoll * CreateEditableRagdoll() = null;
	
	//Создать физический материал
	virtual IPhysMaterial * CreatePhysMaterial(const char * cppfile, long cppline, float statFriction = 0.5f, float dynFriction = 0.5f, float restitution = 0.5f) = null;
	virtual IPhysMaterial * GetPhysMaterial(PhysMaterialGroup group) const = null;

	//Тестит формы на пересечении со сферой, возвращает треугольники в массиве, возвращает true если есть хоть один треугольник
	virtual bool OverlapSphere(const Vector & pos, float radius, dword mask, bool staticShapes, bool dynamicShapes, array<Vector> & aTriangles, array<PhysTriangleMaterialID> * aTriangleMaterials = null) = null;
	//Тестит капсулу на пересечение с объектами, возвращает true если было пересечение
	virtual bool CheckOverlapCapsule(const Vector & pos, float height, float radius, dword mask, bool staticShapes, bool dynamicShapes) = null;

//-------------------------------------------------------------------------------------------------------------
//Дополнительные функции
//-------------------------------------------------------------------------------------------------------------
public:
	//Трейс луча через все объекты
	virtual IPhysBase * Raycast(const Vector & from, const Vector & to, dword mask = phys_mask(phys_world), RaycastResult * detail = null) = null;

	//Текущий режим работы
	virtual bool IsHardware() = null;

	//Пересчитать состояние
	inline void Update(float dltTime)
	{
		UpdateBeginFrame(dltTime);
		UpdateEndFrame(dltTime);
	}
	//Пересчитать состояние перед началом кадра
	virtual void UpdateBeginFrame(float dltTime) = null;
	//Пересчитать состояние после кадра
	virtual void UpdateEndFrame(float dltTime) = null;
	//Нарисовать отладочную информацию
	virtual void DebugDraw(IRender & render) {};

	//Говорит сцене о том что она (сцена) изменилась(появились/исчезли объекты, сменились группы и т.п.)
	virtual void SceneChanged() = null;
	//Возвращает последнее индекс когда сцена была изменена(каждое вкл/выкл объектов, смена групп и т.п. увеличивает счетчик) 
	virtual dword GetSceneChangedIndex() = null;

	//Получить физическую сцену
//	virtual NxScene & Scene() = null;

//-------------------------------------------------------------------------------------------------------------
protected:
	friend class IPhysBase;
	virtual void UnregistryPhysObject(IPhysBase * obj) = null;
};

//Физический сервис
class IPhysics : public Service
{
public:
	//Создать сцену
	virtual IPhysicsScene * CreateScene() = null;
	//Текущий режим работы
	virtual bool IsHardware() = null;
	//Создать сетку, основываясь на бинарных данных
	virtual IPhysTriangleMesh * CreateTriangleMesh(const void * meshData, dword meshDataSize, const void * pMapData = null, dword pMapDataSize = 0) = null;	
	//Создать построитель сеток ткани
	virtual IClothMeshBuilder* CreateClothMeshBuilder() = null;
protected:
	friend class IPhysTriangleMesh;
	virtual void UnregistryPhysTriangleMesh(IPhysTriangleMesh * obj) = null;
	friend class IClothMeshBuilder;
	virtual void UnregistryClothMeshBuilder(IClothMeshBuilder * obj) = null;
};

inline void IPhysBase::UnregistryPhysObject()
{
	//Assert(scene);
	if (scene)
		scene->UnregistryPhysObject(this);
}

inline void IPhysTriangleMesh::UnregistryPhysTriangleMesh()
{
	Assert(physics);
	physics->UnregistryPhysTriangleMesh(this);
}



// утилитный класс построения сеток для ткани
class IClothMeshBuilder
{
public:
	struct Vertex
	{
		Vector	pos;
		float	tu,tv;
	};
protected:
	virtual ~IClothMeshBuilder() = 0
	{
		IPhysics * srv = (IPhysics *)api->GetService("PhysicsService");
		srv->UnregistryClothMeshBuilder(this);
	}
public:

	virtual void AddRef() = 0;
	virtual bool Release() = 0;
	virtual void ForceRelease() = 0;
	// установить scale для генерации текстурных координат
	virtual void SetTexCoordScale(float uScale, float vScale) = 0;
	// установить offset для генерации текстурных координат
	virtual void SetTexCoordOffset(float uOffset, float vOffset) = 0;


	// добавить 4-хугольный парус
	virtual unsigned int AddSail(	const Vector vtx[4], unsigned int wDensity, unsigned int hDensity,
		float windCurvature = 0.0f, float bottomCurvature = 0.9f) = 0;
	virtual unsigned int AddRope(const Vector& begin, const Vector& end, float thickness) = 0;

	// добавить выпуклый 4-хугольник
	virtual unsigned int AddConvexQuadrangle(const Vector vtx[4], unsigned int wDensity, unsigned int hDensity) = 0;
	// добавить 3-угольник
	virtual unsigned int AddTriangle(const Vector vtx[3], unsigned int density) = 0;
	// добавить прямоугольник (в плоскости XY, трансформированный матрицей mtx)
	virtual unsigned int AddRect(float width, float height, unsigned int wDensity, unsigned int hDensity, const Matrix& mtx) = 0;
	// добавить окружность
	virtual unsigned int AddCircle(const Matrix& mtx, float radius, unsigned int rDensity, unsigned int aDensity) = 0;
	// добавить цилиндр (вдоль оси Y, трансформированный матрицей mtx)
	virtual unsigned int AddCylinder(float height, float radius, unsigned int wDensity, unsigned int hDensity, const Matrix& mtx) = 0;
	// добавить произвольную сетку
	virtual unsigned int AddMesh(const array<Vector>& vb, const array<unsigned short>& ib, const Matrix& mtx) = 0;

	// создать соединение между кусками, вернуть индекс нового куска
	virtual unsigned int Connect(unsigned int index1, unsigned int index2, const Vector& pos) = 0;
	// построить сетку для PhysX
	virtual bool Build(IPhysicsScene& scene, bool isTearable = true, IMission* mission = null, const char* fileName = null ) = 0;
	// построить сетку для PhysX из бинарного файла
	virtual bool Load(IPhysicsScene& scene, IMission& mission, const char* fileName) = 0;
	// вернуть скомбинированные буферы индексов и вершин
	virtual void GetCombinedBuffers(const Vertex*& vb, const unsigned short*& ib, unsigned int& vCount, unsigned int& iCount) const = 0;
	// почистить временные данные (оставить только скуканый меш)
	virtual void FreeBuildData() = 0;
	// полностью очистить внутреннее состояние
	virtual void Reset() = 0;

	// создана ли ткань разрываемой
	virtual bool IsTearable() const = 0;
	// готова ли сетка для PhysX
	virtual bool IsReady() const = 0;
	// вернуть сетку
	// FIX_PX3 NxClothMesh
	//virtual NxClothMesh& GetMesh() const = 0;
};