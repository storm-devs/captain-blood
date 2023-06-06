#pragma once
#include "common.h"

// утилитный класс построения сеток для ткани
class ClothMeshBuilder : public IClothMeshBuilder
{
	struct Data;
	Data*	internal_;
	long	iRefCount;

public:

	ClothMeshBuilder(PxPhysics * sdk);
	virtual ~ClothMeshBuilder();


	virtual void AddRef();
	virtual bool Release();
	virtual void ForceRelease();
	// установить scale для генерации текстурных координат
	virtual void SetTexCoordScale(float uScale, float vScale);
	// установить offset для генерации текстурных координат
	virtual void SetTexCoordOffset(float uOffset, float vOffset);


	// добавить 4-хугольный парус
	virtual unsigned int AddSail(const Vector vtx[4], unsigned int wDensity, unsigned int hDensity,
		float windCurvature = 0.0f, float bottomCurvature = 0.9f);
	virtual unsigned int AddRope(const Vector& begin, const Vector& end, float thickness);

	// добавить выпуклый 4-хугольник
	virtual unsigned int AddConvexQuadrangle(const Vector vtx[4], unsigned int wDensity, unsigned int hDensity);
	// добавить 3-угольник
	virtual unsigned int AddTriangle(const Vector vtx[3], unsigned int density);
	// добавить прямоугольник (в плоскости XY, трансформированный матрицей mtx)
	virtual unsigned int AddRect(float width, float height, unsigned int wDensity, unsigned int hDensity, const Matrix& mtx);
	// добавить окружность
	virtual unsigned int AddCircle(const Matrix& mtx, float radius, unsigned int rDensity, unsigned int aDensity);
	// добавить цилиндр (вдоль оси Y, трансформированный матрицей mtx)
	virtual unsigned int AddCylinder(float height, float radius, unsigned int wDensity, unsigned int hDensity, const Matrix& mtx);
	// добавить произвольную сетку
	virtual unsigned int AddMesh(const array<Vector>& vb, const array<unsigned short>& ib, const Matrix& mtx);

	// создать соединение между кусками, вернуть индекс нового куска
	virtual unsigned int Connect(unsigned int index1, unsigned int index2, const Vector& pos);
	// построить сетку для PhysX
	virtual bool Build(IPhysicsScene& scene, bool isTearable = true, IMission* mission = null, const char* fileName = null );
	// построить сетку для PhysX из бинарного файла
	virtual bool Load(IPhysicsScene& scene, IMission& mission, const char* fileName);
	// вернуть скомбинированные буферы индексов и вершин
	virtual void GetCombinedBuffers(const Vertex*& vb, const unsigned short*& ib, unsigned int& vCount, unsigned int& iCount) const;
	// почистить временные данные (оставить только скуканый меш)
	virtual void FreeBuildData();
	// полностью очистить внутреннее состояние
	virtual void Reset();

	// создана ли ткань разрываемой
	virtual bool IsTearable() const;
	// готова ли сетка для PhysX
	virtual bool IsReady() const;
	// вернуть сетку
	// FIX_PX3 ClothMesh
	virtual PxCloth& GetMesh() const;
};