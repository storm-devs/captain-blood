#include "common.h"
#include "../common_h/FileService.h"
#include "PhysicsService.h"
#include "PhysicsScene.h"
#include "NxCooking.h"
#include "ClothMeshBuilder.h"
#include "CookedBinaries/CookBinManager.h"
#include <cmath>


// внутренности ClothMeshBuilder
struct ClothMeshBuilder::Data
{
	Data() :	buildBuffer_(__FILE__, __LINE__),
				vbCombined_(__FILE__, __LINE__),
				ibCombined_(__FILE__, __LINE__),
				mesh_(NULL), sdk_(NULL),
				uScale(1.0f), vScale(1.0f), uOffset(0.0f), vOffset(0.0f) {}
	~Data()	{ if (mesh_ && sdk_) sdk_->releaseClothMesh(*mesh_); }

	// кусок ткани
	struct Piece
	{
		Piece() :	vb(__FILE__, __LINE__, 16),
					ib(__FILE__, __LINE__, 64) {}
		array<Vertex>			vb;
		array<unsigned short>	ib;
	};

	array<Piece>				buildBuffer_;	// буфер для хранения кусков ткани
	NxClothMesh*				mesh_;			// скуканый меш
	NxPhysicsSDK*				sdk_;			// 
	bool						tearable_;		// флаг разрывности
	array<Vertex>				vbCombined_;
	array<unsigned short>		ibCombined_;

	float uScale;								// скейл U-координаты
	float vScale;								// скейл V-координаты
	float uOffset;								// смещение U-координаты
	float vOffset;								// смещение V-координаты

	void BuildCombinedBuffers()
	{
		vbCombined_.DelAll();
		ibCombined_.DelAll();

		for (unsigned int i = 0; i < buildBuffer_.Size(); ++i)
		{
			unsigned int vtxCount = buildBuffer_[i].vb.Size();
			unsigned int indCount = buildBuffer_[i].ib.Size();
			unsigned int ibOffset = vbCombined_.Size();

			vbCombined_.SetAddElements(vtxCount);
			for (unsigned int j = 0; j < vtxCount; ++j)
				vbCombined_.Add(buildBuffer_[i].vb[j]);

			ibCombined_.SetAddElements(indCount);
			for (unsigned int j = 0; j < indCount; ++j)
				ibCombined_.Add(buildBuffer_[i].ib[j] + ibOffset);
		}
	}
};



ClothMeshBuilder::ClothMeshBuilder(NxPhysicsSDK* sdk) :
internal_(NEW Data())
{
	Assert(internal_);
	internal_->sdk_ = sdk;
	iRefCount = 1;
}

ClothMeshBuilder::~ClothMeshBuilder()
{
	delete internal_;
}

void ClothMeshBuilder::AddRef()
{
	iRefCount++;
}

bool ClothMeshBuilder::Release() 
{ 
	iRefCount--;
	if (iRefCount > 0) return false;
	
	ForceRelease();
	return true;
}

void ClothMeshBuilder::ForceRelease()
{
	delete this; 
}

// установить scale для генерации текстурных координат
void ClothMeshBuilder::SetTexCoordScale(float uScale, float vScale)
{
	internal_->uScale = uScale;
	internal_->vScale = vScale;
}

// установить offset для генерации текстурных координат
void ClothMeshBuilder::SetTexCoordOffset(float uOffset, float vOffset)
{
	internal_->uOffset = uOffset;
	internal_->vOffset = vOffset;
}

// создана ли ткань разрываемой
bool ClothMeshBuilder::IsTearable() const
{
	Assert(internal_->mesh_);
	return internal_->tearable_;
}

// готова ли сетка для PhysX
bool ClothMeshBuilder::IsReady() const
{
	return internal_->mesh_ != NULL;
}

// отдать сетку
NxClothMesh& ClothMeshBuilder::GetMesh() const
{
	Assert(internal_->mesh_);
	return *(internal_->mesh_);
}

inline float lerp(float a, float b, float t)
{
	return a + (b-a)*t;
}

float smoothSquare(float a)
{
	if (a > 1.0f) a = 1.0f;
	if (a < 0.0f) a = 0.0f;
	return -(2.0f*a-1.0f)*(2.0f*a-1.0f)+1.0f;
}
inline float smoothSin(float a) { return sinf(PI*a); }

unsigned int ClothMeshBuilder::AddSail(const Vector vtx[4], unsigned int wDensity, unsigned int hDensity,
									   float windCurvature, float bottomCurvature)
{
	Vector pos, t1,t2;
	float cw, ch;
	Vertex v;

	float width = 0.0f;
	float height = 0.0f;
	float minY = 1e38f;
	float minX = 1e38f;

	for (unsigned int i = 0; i < 4; ++i)
	{
		if (minY > vtx[i].y)
			minY = vtx[i].y;

		if (minX > vtx[i].x)
			minX = vtx[i].x;

		if (abs(vtx[i].y - vtx[0].y) > height)
			height = abs(vtx[i].y - vtx[0].y);

		if (abs(vtx[i].x - vtx[0].x) > width)
			width = abs(vtx[i].x - vtx[0].x);
	}

	internal_->buildBuffer_.Add();
	array<Vertex>& vb = internal_->buildBuffer_[internal_->buildBuffer_.Last()].vb;
	array<unsigned short>& ib = internal_->buildBuffer_[internal_->buildBuffer_.Last()].ib;

	Vector vNormal = (vtx[1]-vtx[0]) ^ (vtx[2]-vtx[0]);
	vNormal.Normalize();

	for (unsigned int i = 0; i < hDensity; ++i)
		for (unsigned int j = 0; j < wDensity; ++j)
		{
			cw = j/(wDensity-1.0f);
			ch = i/(hDensity-1.0f);

			t1 = (vtx[3] - vtx[0])*ch + vtx[0];
			t2 = (vtx[2] - vtx[1])*ch + vtx[1];

			Vector vtmp = (t2 - t1)*cw + t1;

			//MOD: sail cooking tweak
			////////////////////////////
			
			vtmp.y -= (minY + height);
			vtmp.y *= lerp(1.0f, bottomCurvature, smoothSquare((0.5f*width-abs(vtmp.x))/width));
			vtmp.y += (minY + height);

			//vtmp += vNormal * windCurvature*smoothSquare(abs((v.pos.y-minY))/height)*smoothSquare(abs((v.pos.x-minX))/width);
			vtmp += vNormal * windCurvature*smoothSquare(abs((vtmp.y-minY))/height)*smoothSquare(cw);
			//v.pos.z += windCurvature*smoothSquare(abs((v.pos.y-minY))/height)*smoothSquare(abs((v.pos.x-minX))/width);
			//v.pos.z += windCurvature*(smoothSin(abs((v.pos.y-minY))/height)*smoothSin(abs((v.pos.x-minX))/width));
			////////////////////////////

			v.pos = vtmp;

			v.tu = internal_->uOffset + cw*internal_->uScale;
			v.tv = internal_->vOffset + ch*internal_->vScale;

			vb.Add(v);
		}

	for (unsigned int i = 0; i < hDensity-1; ++i)
		for (unsigned int j = 0; j < wDensity-1; ++j)
		{
			ib.Add(i*wDensity + j);
			ib.Add(i*wDensity + j + wDensity);
			ib.Add(i*wDensity + j + wDensity + 1);

			ib.Add(i*wDensity + j );
			ib.Add(i*wDensity + j + wDensity + 1);
			ib.Add(i*wDensity + j + 1);
		}

	return internal_->buildBuffer_.Last();
}

unsigned int ClothMeshBuilder::AddRope(const Vector& begin, const Vector& end, float thickness)
{
	internal_->buildBuffer_.Add();
	array<Vertex>& vb = internal_->buildBuffer_[internal_->buildBuffer_.Last()].vb;
	array<unsigned short>& ib = internal_->buildBuffer_[internal_->buildBuffer_.Last()].ib;

	Matrix mtx;
	Vertex v;
	float ch,cw;
	Vector dir = (end-begin);
	float length = dir.Normalize();

	mtx.BuildOriented(begin, end, Vector(0,1,0));

	const unsigned int wDensity = 4;
	const unsigned int hDensity = 8;
	for (unsigned int i = 0; i < hDensity; ++i)
		for (unsigned int j = 0; j < wDensity; ++j)
		{
			cw = j/(float)(wDensity);
			ch = i/(float)(hDensity-1.0f);

			v.pos = Vector( 0.5f*thickness*cosf(2*PI*cw), 0.5f*thickness*sinf(2*PI*cw), length*ch);
			v.pos = mtx.MulVertex(v.pos);

			v.tu = internal_->uOffset + cw*internal_->uScale;
			v.tv = internal_->vOffset + ch*internal_->vScale;

			vb.Add(v);
		}

	
	for (unsigned int i = 0; i < hDensity-1; ++i)
	{
		for (unsigned int j = 0; j < wDensity-1; ++j)
		{
			ib.Add(i*wDensity + j);
			ib.Add(i*wDensity + j + wDensity);
			ib.Add(i*wDensity + j + wDensity + 1);

			ib.Add(i*wDensity + j );
			ib.Add(i*wDensity + j + wDensity + 1);
			ib.Add(i*wDensity + j + 1);
		}

		ib.Add(i*wDensity + wDensity-1 );
		ib.Add((i+1)*wDensity + wDensity-1);
		ib.Add((i+1)*wDensity);

		ib.Add(i*wDensity + wDensity-1 );
		ib.Add((i+1)*wDensity);
		ib.Add(i*wDensity);
	}

	// дополнительная наружная прошивка

	for (unsigned int i = 0; i < hDensity-1; ++i)
	{
		for (unsigned int j = 0; j < wDensity-1; ++j)
		{
			ib.Add(i*wDensity + j);
			ib.Add(i*wDensity + j + 1);
			ib.Add(i*wDensity + j + wDensity);

			ib.Add(i*wDensity + j + wDensity);
			ib.Add(i*wDensity + j + 1);
			ib.Add(i*wDensity + j + wDensity + 1);
		}

		ib.Add(i*wDensity + wDensity-1 );
		ib.Add(i*wDensity);
		ib.Add((i+1)*wDensity + wDensity-1);
		
		ib.Add(i*wDensity + wDensity-1 );
		ib.Add(i*wDensity);
		ib.Add((i+1)*wDensity);
	}

	return internal_->buildBuffer_.Last();
}

// добавить выпуклый 4-хугольник
unsigned int ClothMeshBuilder::AddConvexQuadrangle(const Vector vtx[4], unsigned int wDensity, unsigned int hDensity)
{
	Vector pos, t1,t2;
	float cw, ch;
	Vertex v;

	internal_->buildBuffer_.Add();
	array<Vertex>& vb = internal_->buildBuffer_[internal_->buildBuffer_.Last()].vb;
	array<unsigned short>& ib = internal_->buildBuffer_[internal_->buildBuffer_.Last()].ib;

	for (unsigned int i = 0; i < hDensity; ++i)
		for (unsigned int j = 0; j < wDensity; ++j)
		{
			cw = j/(wDensity-1.0f);
			ch = i/(hDensity-1.0f);

			t1 = (vtx[3] - vtx[0])*ch + vtx[0];
			t2 = (vtx[2] - vtx[1])*ch + vtx[1];

			v.pos =	(t2 - t1)*cw + t1;

			v.tu = internal_->uOffset + cw*internal_->uScale;
			v.tv = internal_->vOffset + ch*internal_->vScale;

			vb.Add(v);
		}

	for (unsigned int i = 0; i < hDensity-1; ++i)
		for (unsigned int j = 0; j < wDensity-1; ++j)
		{
			ib.Add(i*wDensity + j);
			ib.Add(i*wDensity + j + wDensity);
			ib.Add(i*wDensity + j + wDensity + 1);

			ib.Add(i*wDensity + j );
			ib.Add(i*wDensity + j + wDensity + 1);
			ib.Add(i*wDensity + j + 1);
		}

	return internal_->buildBuffer_.Last();
}

// добавить 3-угольник
unsigned int ClothMeshBuilder::AddTriangle(const Vector vtx[3], unsigned int density)
{
	Vector pos, t1,t2;
	float cw, ch;
	Vertex v;

	internal_->buildBuffer_.Add();
	array<Vertex>& vb = internal_->buildBuffer_[internal_->buildBuffer_.Last()].vb;
	array<unsigned short>& ib = internal_->buildBuffer_[internal_->buildBuffer_.Last()].ib;

	for (unsigned int i = 0; i < density; ++i)
		for (unsigned int j = 0; j < i+1; ++j)
		{
			cw = 0.0f;
			if (i)
				cw = j/(float)i;
			ch = i/(density-1.0f);

			t1 = (vtx[2] - vtx[0])*ch + vtx[0];
			t2 = (vtx[1] - vtx[0])*ch + vtx[0];

			v.pos =	(t2 - t1)*cw + t1;

			v.tu = internal_->uOffset + j/(density-1.0f)*internal_->uScale;
			v.tv = internal_->vOffset + ch*internal_->vScale;

			vb.Add(v);
		}

	unsigned int startIndex = 0;
	for (unsigned int i = 0; i < density-1; ++i)
	{
		for (unsigned int j = 0; j < 2*i+1; ++j)
		{
			if (j & 0x01)
			{
				ib.Add(startIndex + j/2);
				ib.Add(startIndex + j/2 + i + 2);
				ib.Add(startIndex + j/2 + 1);
			}
			else
			{
				ib.Add(startIndex + j/2);
				ib.Add(startIndex + j/2 + i + 1);
				ib.Add(startIndex + j/2 + i + 2);
			}
		}
		startIndex += i+1;
	}


	return internal_->buildBuffer_.Last();
}

// добавить прямоугольник
unsigned int ClothMeshBuilder::AddRect(float width, float height, unsigned int wDensity, unsigned int hDensity, const Matrix& mtx)
{
	Vector vtx[4] = {	Vector(-width/2, -height/2, 0.0f),
						Vector(width/2, -height/2, 0.0f),
						Vector(width/2, height/2, 0.0f),
						Vector(-width/2, height/2, 0.0f) };

	for (unsigned int i = 0; i < sizeof(vtx)/sizeof(vtx[0]); ++i)
		vtx[i] = mtx.MulVertex(vtx[i]);

	return AddConvexQuadrangle(vtx, wDensity, hDensity);
}

// добавить окружность
unsigned int ClothMeshBuilder::AddCircle(const Matrix& mtx, float radius, unsigned int rDensity, unsigned int aDensity)
{
	Assert(!"Not implemented");
	return 0xFFFFFFFF;
}

// добавить цилиндр
unsigned int ClothMeshBuilder::AddCylinder(float height, float radius, unsigned int wDensity, unsigned int hDensity, const Matrix& mtx)
{
	Vector pos;
	float cw, ch;
	Vertex v;

	internal_->buildBuffer_.Add();
	array<Vertex>& vb = internal_->buildBuffer_[internal_->buildBuffer_.Last()].vb;
	array<unsigned short>& ib = internal_->buildBuffer_[internal_->buildBuffer_.Last()].ib;

	for (unsigned int i = 0; i < hDensity; ++i)
		for (unsigned int j = 0; j < wDensity; ++j)
		{
			cw = j/(float)(wDensity);
			ch = i/(float)(hDensity-1.0f);

			v.pos = Vector( radius*cosf(2*PI*cw), height*ch, radius*sinf(2*PI*cw));
			v.pos = mtx.MulVertex(v.pos);

			v.tu = internal_->uOffset + cw*internal_->uScale;
			v.tv = internal_->vOffset + ch*internal_->vScale;

			vb.Add(v);
		}

	for (unsigned int i = 0; i < hDensity-1; ++i)
	{
		for (unsigned int j = 0; j < wDensity-1; ++j)
		{
			ib.Add(i*wDensity + j);
			ib.Add(i*wDensity + j + wDensity);
			ib.Add(i*wDensity + j + wDensity + 1);

			ib.Add(i*wDensity + j );
			ib.Add(i*wDensity + j + wDensity + 1);
			ib.Add(i*wDensity + j + 1);
		}

		ib.Add(i*wDensity + wDensity-1 );
		ib.Add((i+1)*wDensity + wDensity-1);
		ib.Add((i+1)*wDensity);

		ib.Add(i*wDensity + wDensity-1 );
		ib.Add((i+1)*wDensity);
		ib.Add(i*wDensity);
	}

	return internal_->buildBuffer_.Last();
}

// добавить произвольную сетку
unsigned int ClothMeshBuilder::AddMesh(const array<Vector>& vb, const array<unsigned short>& ib, const Matrix& mtx)
{
	Assert(!"Not implemented");
	return 0xFFFFFFFF;
}

// создать соединение между кусками
unsigned int ClothMeshBuilder::Connect(unsigned int index1, unsigned int index2, const Vector& pos)
{
	Assert(index1 < internal_->buildBuffer_.Size());
	Assert(index2 < internal_->buildBuffer_.Size());

	// 1) ищем по 2 ближайшие к pos вершины в каждой сетке
	float minDist = FLT_MAX;
	float dist;
	unsigned int minIndex11 = 0, minIndex12 = 0; // индексы 2-х ближайших вершин 1-й сетки
	for (unsigned int i = 0; i < internal_->buildBuffer_[index1].vb.Size(); ++i)
	{
		dist = (pos-internal_->buildBuffer_[index1].vb[i].pos).GetLength();
		if ( minDist > dist )
		{
			minDist = dist;
			minIndex11 = i;
		}
	}
	minDist = FLT_MAX;
	for (unsigned int i = 0; i < internal_->buildBuffer_[index1].vb.Size(); ++i)
	{
		dist = (pos-internal_->buildBuffer_[index1].vb[i].pos).GetLength();
		if ( minDist > dist && i != minIndex11)
		{
			minDist = dist;
			minIndex12 = i;
		}
	}

	minDist = FLT_MAX;
	unsigned int minIndex21 = 0, minIndex22 = 0; // индексы 2-х ближайших вершин 2-й сетки
	for (unsigned int i = 0; i < internal_->buildBuffer_[index2].vb.Size(); ++i)
	{
		dist = (pos-internal_->buildBuffer_[index2].vb[i].pos).GetLength();
		if ( minDist > dist )
		{
			minDist = dist;
			minIndex21 = i;
		}
	}
	minDist = FLT_MAX;
	for (unsigned int i = 0; i < internal_->buildBuffer_[index2].vb.Size(); ++i)
	{
		dist = (pos-internal_->buildBuffer_[index2].vb[i].pos).GetLength();
		if ( minDist > dist && i != minIndex21)
		{
			minDist = dist;
			minIndex22 = i;
		}
	}

	// 2) сливаем сетки в одну
	internal_->buildBuffer_.Add();
	array<Vertex>& vb = internal_->buildBuffer_[internal_->buildBuffer_.Last()].vb;
	array<unsigned short>& ib = internal_->buildBuffer_[internal_->buildBuffer_.Last()].ib;

	vb = internal_->buildBuffer_[index1].vb;
	ib = internal_->buildBuffer_[index1].ib;
	unsigned int offset = vb.Size();

	for (unsigned int i = 0; i < internal_->buildBuffer_[index2].vb.Size(); ++i)
		vb.Add(internal_->buildBuffer_[index2].vb[i]);

	for (unsigned int i = 0; i < internal_->buildBuffer_[index2].ib.Size(); ++i)
		ib.Add(internal_->buildBuffer_[index2].ib[i] + offset);

	// 3) добавляем в новую сетку квад, соединяющий части
	ib.Add(minIndex11);
	ib.Add(minIndex12);
	ib.Add(minIndex21 + offset);

	ib.Add(minIndex12);
	ib.Add(minIndex21 + offset);
	ib.Add(minIndex22 + offset);
	
	// 4) чистим старые сетки
	internal_->buildBuffer_[index1].vb.DelAll();
	internal_->buildBuffer_[index1].ib.DelAll();

	internal_->buildBuffer_[index2].vb.DelAll();
	internal_->buildBuffer_[index2].ib.DelAll();

	// 5) даем индекс новой сетки
	return internal_->buildBuffer_.Last();
}

bool Cook(NxClothMeshDesc& desc, NxPlatform target, PhysicsService::MemoryWriteStream& wb)
{
	NxCookingParams params;
	params.targetPlatform = target;
	NxSetCookingParams(params);

	if (!NxCookClothMesh(desc, wb)) 
		return false;

	return true;
}

// построить сетку для PhysX
bool ClothMeshBuilder::Build(IPhysicsScene& scene, bool isTearable, IMission* mission, const char* fileName)
{
	internal_->tearable_ = isTearable;

	if ( internal_->mesh_ )
	{
		internal_->sdk_->releaseClothMesh(*(internal_->mesh_));
		internal_->mesh_ = NULL;
	}

	NxClothMeshDesc desc;
	desc.flags = 0;
	if (isTearable)
		desc.flags |= NX_CLOTH_MESH_TEARABLE;

	internal_->BuildCombinedBuffers();

	desc.numTriangles = internal_->ibCombined_.Size()/3;
	desc.numVertices = internal_->vbCombined_.Size();
	desc.points = &internal_->vbCombined_[0];
	desc.pointStrideBytes = sizeof(internal_->vbCombined_[0]);
	desc.triangles = &internal_->ibCombined_[0];
	desc.triangleStrideBytes = sizeof(internal_->ibCombined_[0])*3;
	desc.flags |= NX_MF_16_BIT_INDICES;
	desc.vertexFlags = 0;
	desc.vertexMasses = 0;
	desc.vertexMassStrideBytes = 0;

	Assert(sizeof(internal_->ibCombined_[0]) == 2);


	PhysicsService::MemoryWriteStream wbPC;
	PhysicsService::MemoryWriteStream wbXBOX;
	MOSafePointerType<CookBinManager> binMgr;
	//CookBinManager* binMgr = NULL;
	static const ConstString objName("CookBinManager");
	if (mission)
		(CookBinManager*)mission->CreateObject(binMgr.GetSPObject(), "CookBinManager", objName);

#ifndef _XBOX
	bool bSuccessCook = true;
	if( Cook(desc, PLATFORM_PC, wbPC) )
	{
		if (binMgr.Validate())
			binMgr.Ptr()->SaveCookedMesh(fileName, wbPC.Pointer(), wbPC.Size());
	}
	else
	{
		bSuccessCook = false;
		api->Trace("ClothMeshBuilder::Build() Error! Cook for PC failed!");
	}

	if( Cook(desc, PLATFORM_XENON, wbXBOX) )
	{
		if (binMgr.Validate())
			binMgr.Ptr()->SaveCookedMesh(string("x360_")+fileName, wbXBOX.Pointer(), wbXBOX.Size());
	}
	else
	{
		bSuccessCook = false;
		api->Trace("ClothMeshBuilder::Build() Error! Cook for XBox failed!");
	}

	if( bSuccessCook )
	{
		PhysicsService::MemoryReadStream rb(wbPC.Pointer(), wbPC.Size());
		internal_->mesh_ = internal_->sdk_->createClothMesh(rb);
	}
/*#else // на XBox не бум ничего сами строить - должны быть скукенные уже.
	Cook(desc, PLATFORM_XENON, wbXBOX);

	PhysicsService::MemoryReadStream rb(wbXBOX.Pointer(), wbXBOX.Size());
	internal_->mesh_ = internal_->sdk_->createClothMesh(rb);*/
#endif
	
	
	return internal_->mesh_ != NULL;
}

// построить сетку для PhysX из бинарного файла
bool ClothMeshBuilder::Load(IPhysicsScene& scene, IMission& mission, const char* fileName)
{
	internal_->BuildCombinedBuffers();

	IFileService * fileSrv = (IFileService *)api->GetService("FileService");
	dword fileSize = 0;

	MOSafePointerType<CookBinManager> binMgr;

	static const ConstString objName("CookBinManager");
	mission.CreateObject(binMgr.GetSPObject(), "CookBinManager", objName);
	
	const void * pData = NULL;
#ifdef _XBOX
	if (binMgr.Validate())
		binMgr.Ptr()->LoadCookedMesh(string("x360_") + fileName, pData, fileSize);
#else
	if (binMgr.Validate())
		binMgr.Ptr()->LoadCookedMesh(fileName, pData, fileSize);
#endif
	
	if(!pData) return false;

	PhysicsService::MemoryReadStream rb(pData, fileSize);
	internal_->mesh_ = internal_->sdk_->createClothMesh(rb);

	return internal_->mesh_ != NULL;
}

// вернуть скомбинированные буферы индексов и вершин
void ClothMeshBuilder::GetCombinedBuffers(const Vertex*& vb, const unsigned short*& ib, unsigned int& vCount, unsigned int& iCount) const
{
	vb = &internal_->vbCombined_[0];
	ib = &internal_->ibCombined_[0];

	vCount = internal_->vbCombined_.Size();
	iCount = internal_->ibCombined_.Size();
}

// почистить временные данные (оставить только скуканый меш)
void ClothMeshBuilder::FreeBuildData()
{
	internal_->buildBuffer_.DelAll();
	internal_->vbCombined_.DelAll();
	internal_->ibCombined_.DelAll();
}

// очистить внутреннее состояние
void ClothMeshBuilder::Reset()
{
	if (internal_)
		delete internal_, internal_ = NULL;
	internal_ = NEW Data();
}