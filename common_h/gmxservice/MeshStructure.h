#ifndef _____MESH_STRUCTURE_____
#define _____MESH_STRUCTURE_____

#define INVALID_STRING_TABLE_ENTRY (0x0)
#define INVALID_OFFSET (0xFFFFFFFF)

class IBaseTexture;
#include "..\render\ShaderID.h"



#define MESH_HEADER (0x4D4F4547)

//изменять версию при любой модификации этого файла !!!!
#define MESH_VERSION (80)

#define HASH_TABLE_SIZE (128)

#define GMX_MESH_MAX_TEXTURES_COUNT (4)

//#define INVALID_GMXHANDLE (0)
//typedef unsigned long GMXHANDLE;

struct GMXHANDLE
{
	dword index;

	GMXHANDLE()
	{
		index = 0;
	}

	//Проверить на правильность (cпециально dword а не bool, что бы компилятору на X360 было проще)
	__forceinline dword isValid()
	{
		return index;
	}

	//Сбросить...
	__forceinline void reset()
	{
		index = 0;
	}

	//Сравнение...
	__forceinline bool operator == (const GMXHANDLE & other) const
	{
		return (index == other.index);
	}


};


typedef unsigned long TTABLEITEM;
#define INVALID_BONE_IDX (0xFF)


#pragma pack(push, 1)

struct MeshAnimVertex
{
	Vector p;					// 3 float = 12 bytes
	dword packed_normal;				// 1 dword = 4 bytes
	dword packed_tangent;				// 1 dword  = 4 bytes
	dword packed_binormal;				// 1 dword  = 4 bytes

	short packed_u;					// 1 short = 2 bytes
	short packed_v;					// 1 short = 2 bytes

	short packed_blend_1;				// 1 short = 2 bytes
	short packed_blend_2;				// 1 short = 2 bytes

	// total:32
};

struct MeshStaticVertex
{
	Vector p;					// 3 float = 12 bytes
	dword packed_normal;				// 1 dword = 4 bytes
	dword packed_tangent;				// 1 dword  = 4 bytes
	dword packed_binormal;				// 1 dword  = 4 bytes

	short packed_u1;				// 1 short = 2 bytes
	short packed_v1;				// 1 short = 2 bytes
	short packed_u2;				// 1 short = 2 bytes
	short packed_v2;				// 1 short = 2 bytes  // total:32
};

struct MeshAnimBoneID
{
	unsigned long bones_id;				// 1 dword = 4 bytes  // total:4
};


#pragma pack(pop)

struct stringHead
{
	dword dwHashNoCase;
	dword dwLen;
};



struct GMXBoundBox
{
	Vector vMin;
	dword padding0;

	Vector vMax;
	dword padding1;
};

struct GMXBoundSphere
{
	Vector vCenter;
	float fRadius;
};



#ifdef _XBOX

//На X360 ничего делать не нужно...
__forceinline void MeshSwizzleFloat(float & v)
{
}

__forceinline void MeshSwizzleDWord(dword & v)
{
}

__forceinline void MeshSwizzleTableItem(TTABLEITEM & v)
{
}


__forceinline void MeshSwizzleULong(unsigned long & v)
{
}

__forceinline void MeshSwizzleWord(word & v)
{
}

__forceinline void MeshSwizzleShort(short & v)
{
}

__forceinline void MeshSwizzleLong(long & v)
{
}

__forceinline void MeshSwizzleInt(int & v)
{
}

__forceinline void MeshSwizzleMatrix(Matrix & m)
{
}


__forceinline void MeshSwizzleQuaternion(Quaternion & v)
{
}


__forceinline void MeshSwizzleVector3(Vector & v)
{
}

__forceinline void MeshSwizzleVector4(Vector4 & v)
{
}

__forceinline void MeshSwizzleAnimVertex(MeshAnimVertex & v)
{
}

__forceinline void MeshSwizzleAnimBoneID(MeshAnimBoneID & v)
{
}

__forceinline void MeshSwizzleStaticVertex(MeshStaticVertex & v)
{
}

#else

//Универсальный перестановщик на PC
template<class T>
__forceinline void meshSwizzleTemplate(T & value)
{
	union Swizzler
	{
		T value;
		__int8 bytes[sizeof(T)];
	};

	T & tmp = value;
	Swizzler & swizzler = (Swizzler &)tmp;
	for(long i = 0; i < sizeof(T) / 2; i++)
	{
		__int8 & b1 = swizzler.bytes[i];
		__int8 & b2 = swizzler.bytes[sizeof(T) - i - 1];
		__int8 tmp = b1;
		b1 = b2;
		b2 = tmp;
	}
}


__forceinline void MeshSwizzleFloat(float & v)
{
	meshSwizzleTemplate(v);
}

__forceinline void MeshSwizzleDWord(dword & v)
{
	meshSwizzleTemplate(v);
}

__forceinline void MeshSwizzleTableItem(TTABLEITEM & v)
{
	meshSwizzleTemplate(v);
}




__forceinline void MeshSwizzleULong(unsigned long & v)
{
	meshSwizzleTemplate(v);
}


__forceinline void MeshSwizzleWord(word & v)
{
	meshSwizzleTemplate(v);
}

__forceinline void MeshSwizzleShort(short & v)
{
	meshSwizzleTemplate(v);
}

__forceinline void MeshSwizzleLong(long & v)
{
	meshSwizzleTemplate(v);
}

__forceinline void MeshSwizzleInt(int & v)
{
	meshSwizzleTemplate(v);
}



__forceinline void MeshSwizzleMatrix(Matrix & m)
{
	MeshSwizzleFloat(m.matrix[0]);
	MeshSwizzleFloat(m.matrix[1]);
	MeshSwizzleFloat(m.matrix[2]);
	MeshSwizzleFloat(m.matrix[3]);

	MeshSwizzleFloat(m.matrix[4]);
	MeshSwizzleFloat(m.matrix[5]);
	MeshSwizzleFloat(m.matrix[6]);
	MeshSwizzleFloat(m.matrix[7]);

	MeshSwizzleFloat(m.matrix[8]);
	MeshSwizzleFloat(m.matrix[9]);
	MeshSwizzleFloat(m.matrix[10]);
	MeshSwizzleFloat(m.matrix[11]);

	MeshSwizzleFloat(m.matrix[12]);
	MeshSwizzleFloat(m.matrix[13]);
	MeshSwizzleFloat(m.matrix[14]);
	MeshSwizzleFloat(m.matrix[15]);
}


__forceinline void MeshSwizzleQuaternion(Quaternion & v)
{
	MeshSwizzleFloat(v.x);
	MeshSwizzleFloat(v.y);
	MeshSwizzleFloat(v.z);
	MeshSwizzleFloat(v.w);
}


__forceinline void MeshSwizzleVector3(Vector & v)
{
	MeshSwizzleFloat(v.x);
	MeshSwizzleFloat(v.y);
	MeshSwizzleFloat(v.z);
}

__forceinline void MeshSwizzleVector4(Vector4 & v)
{
	MeshSwizzleFloat(v.x);
	MeshSwizzleFloat(v.y);
	MeshSwizzleFloat(v.z);
	MeshSwizzleFloat(v.w);
}


__forceinline void MeshSwizzleAnimVertex(MeshAnimVertex & v)
{
	MeshSwizzleVector3(v.p);

	MeshSwizzleDWord(v.packed_normal);
	MeshSwizzleDWord(v.packed_tangent);
	MeshSwizzleDWord(v.packed_binormal);

	MeshSwizzleShort(v.packed_u);
	MeshSwizzleShort(v.packed_v);

	MeshSwizzleShort(v.packed_blend_1);
	MeshSwizzleShort(v.packed_blend_2);
}

__forceinline void MeshSwizzleAnimBoneID(MeshAnimBoneID & v)
{
	MeshSwizzleULong(v.bones_id);
}

__forceinline void MeshSwizzleStaticVertex(MeshStaticVertex & v)
{
	MeshSwizzleVector3(v.p);

	MeshSwizzleDWord(v.packed_normal);
	MeshSwizzleDWord(v.packed_tangent);
	MeshSwizzleDWord(v.packed_binormal);

	MeshSwizzleShort(v.packed_u1);
	MeshSwizzleShort(v.packed_v1);
	MeshSwizzleShort(v.packed_u2);
	MeshSwizzleShort(v.packed_v2);
}

#endif









//typedef dword strTableOffset;





struct IDirect3DVertexBuffer9;
struct IDirect3DIndexBuffer9;

class IVBuffer;
class IIBuffer;
class IPhysTriangleMesh;
struct renderOrder;
struct material;
struct blendShapeBoneDescription;
struct blendShapeBoneTransform;
struct blendShapeTarget;
struct subSubsetAnim;
struct subsetStatic;
struct subsetAnim;
struct locator;
struct mesh;
struct header;
struct nodeTransform;

struct boneDataReadOnly;
struct boneMtxInputReadOnly;


#define NODE_TYPE_UNDEFINED (0)
#define NODE_TYPE_LOCATOR (1)
#define NODE_TYPE_LOCATORPARTICLES (3)
#define NODE_TYPE_MESH (2)


// The PPC back-end of the C compiler by default defines bitfields to be
// ordered from the MSB to the LSB, which is opposite the convention on
// the X86 platform.  Use the 'bitfield_order' pragma to switch the
// ordering.  Note that this does not affect endianness in any way.
#if defined(_M_PPCBE)
#pragma bitfield_order(push)
#pragma bitfield_order(lsb_to_msb)
#endif


struct nodeTypeDesc
{
	union
	{
		struct
		{
			dword type                 : 2; //0 - undefined, 1 - locator, 2 - mesh, 3 - particles on locator 
			dword dynamicNode          : 1; //0 - static transform, 1 - dynamic transform
			dword dynamicParent        : 1; //0 - static parent node or without parent, 1 - dynamic parent
			dword padding              : 28;
		};
		dword dw;
	};


	nodeTypeDesc()
	{
		dw = 0;
	}
};

#if defined(_M_PPCBE)
#pragma bitfield_order(pop)
#endif



struct meshStaticString
{
	union
	{
		dword offset;
		const char * strPtr;
	};

	dword hashNoCase() const
	{
		if (strPtr == NULL)
		{
			return 0xFFFFFFFF;
		}

		stringHead * pHead = (stringHead *)(strPtr - sizeof(stringHead));
		return pHead->dwHashNoCase;
	}

	dword length() const
	{
		if (strPtr == NULL)
		{
			return 0;
		}

		stringHead * pHead = (stringHead *)(strPtr - sizeof(stringHead));
		return pHead->dwLen;
	}

	const char * c_str() const
	{
		return strPtr;
	}

	void clear ()
	{
		strPtr = NULL;
	}

	void fixup(byte * stringTableStart)
	{
		MeshSwizzleDWord(offset);
		if (offset == INVALID_STRING_TABLE_ENTRY)
		{
			strPtr = NULL;
			return;
		}

		strPtr = (const char *)(stringTableStart + offset);
	}

};



template< class T >
struct fixablePointer
{
	union
	{
		dword index;
		dword offsetInBytes;
		T * ptr;
	};

	/*
	void fixupAsStringTable(byte * startOffset)
	{
	DWORD dwOffset = offsetInBytes;
	if (dwOffset == INVALID_STRING_TABLE_ENTRY)
	{
	ptr = NULL;
	return;
	}

	ptr = (T *)(startOffset + dwOffset);
	}
	*/

	void fixupAsOffset(byte * startOffset)
	{
		dword dwOffset = offsetInBytes;
		MeshSwizzleDWord(dwOffset);
		if (dwOffset == INVALID_OFFSET)
		{
			ptr = NULL;
			return;
		}

		ptr = (T *)(startOffset + dwOffset);
	}


	void fixupAsIndex(T * arrayPtr)
	{
		dword dwIndex = index;
		MeshSwizzleDWord(dwIndex);
		if (dwIndex == INVALID_OFFSET)
		{
			ptr = NULL;
			return;
		}
		ptr = (arrayPtr + dwIndex);
	}

	void reset()
	{
		ptr = NULL;
	}
};

//144 bytes (64 + 64 + 16)
struct nodeTransform
{
	Matrix mtxTransform;
	Matrix mtxLocalTransform;

	int parentIdx;
	dword padding;

	//(4) Ссылка strTableOffset заменяющаяся на указатель
	meshStaticString boneName;

	//(4) Ссылка strTableOffset заменяющаяся на указатель
	meshStaticString debugNodeName;
};

struct renderOrder
{
	dword dwMeshOrder;

	//Указатель/индекс на сабсет в меше
	fixablePointer<void> pSubset;

	//указатель/индекс на геометрию
	fixablePointer<mesh> pMesh;

	renderOrder()
	{
		dwMeshOrder = INVALID_OFFSET;
		pSubset.index = INVALID_OFFSET;
		pMesh.index = INVALID_OFFSET;
	}
};

struct material
{
	meshStaticString shaderName;
	dword dwBonesInSkin;
	dword dwTexCount;

	//заполнять надо в runtime к сожалению (это от формата текстур зависит)
	dword isNoAlpha; 
	dword dwLevel;
	dword dwDiffuseSlotIdx;

	meshStaticString textureNames[GMX_MESH_MAX_TEXTURES_COUNT];

	ShaderLightingId tech;
	IBaseTexture* tex[GMX_MESH_MAX_TEXTURES_COUNT];

};


//Это табличка где хранится информация о каждой кости в блендшейпе... всего их dwBlendShapeBonesCount
struct blendShapeBoneDescription
{
	meshStaticString boneName;

	//Индекс в анимации - он нужен только в инстансе...
	//long AnimIndex;

};

//Это позиция блендшейп кости, на них ссылаются из blendShapeTarget (всего их должно быть dwBlendShapeBonesCount * dwBlendShapeTargetsCount)
struct blendShapeBoneTransform
{
	//Позиция и вращение
	Vector p;

	//кости которые на этом кадре не затронуты у них будет 0.0
	float weightScale;

	Quaternion q;
};


//Это кадры блендшейпа (фонемы) их по количеству кадров в майке
struct blendShapeTarget
{
	//указатель на начало кадра blendshape target'a, внутри будет dwBlendShapeBonesCount кадров
	fixablePointer<blendShapeBoneTransform> frameBones;

	//вес нужнен тоже только в инстансе...
	//float Weight;
};


//32 байта
struct locator
{
	fixablePointer<nodeTransform> pTransform;
	fixablePointer<nodeTransform> pParentTransform;
	nodeTypeDesc desc;
	meshStaticString name;
	meshStaticString boneName;
	meshStaticString particleName;

	//просто индекс в массиве трансформаций
	GMXHANDLE handle;

	//фактически pTransform, только индекс (внутри msh.pDynamicTransformations) а не фиксап и для только динамик трансформаций, остальные 0xFFFFFFFF
	dword dwDynamicTransformCacheIndex;

	//индекс в массиве указателей на партиклы в инстансе, если нет партикловый локатор, то 0xFFFFFFFF
	dword dwParticleCacheIndex;

	fixablePointer<locator> pHashTableNext;
};






struct subSubsetAnim
{
	dword offsetInStream1InBytes;
	dword numVertexes;
	dword offsetInIndexBufferInInds;
	dword triCount;


	dword dwTranslateTableSize;

	//можно быстро узнать по локальному индексу -> индекс мастерскелета
	fixablePointer<TTABLEITEM> pTTableLocalToGlobal;
};


struct subsetStatic
{
	//индекс/указатель на material
	fixablePointer<material> pMaterial;
	dword offsetInVertexBufferInBytes;
	dword numVertexes;
	dword offsetInIndexBufferInInds;
	dword triCount;
	ShaderLightingId tech;
	dword isNoAlpha;
	dword dwBonesInSkin;
	IBaseTexture* tex[GMX_MESH_MAX_TEXTURES_COUNT];

	Vector min;
	Vector max;
	Vector sphereC;
	float sphereR;




	// для коллижена
	//------------------------------------------
	dword dwNovodexCollisionSizePC;
	dword dwNovodexPMapSizePC;

	fixablePointer<byte> pNovodexCollisionPC;
	fixablePointer<byte> pNovodexPMapPC;

	dword dwNovodexCollisionSizeXBOX360;
	dword dwNovodexPMapSizeXBOX360;

	fixablePointer<byte> pNovodexCollisionXBOX360;
	fixablePointer<byte> pNovodexPMapXBOX360;
	//------------------------------------------


};

struct subsetAnim
{
	//индекс/указатель на material
	fixablePointer<material> pMaterial;

	dword offsetInStream0InBytes;
	dword numVertexes;
	ShaderLightingId tech;
	dword isNoAlpha;
	dword dwBonesInSkin;
	IBaseTexture* tex[GMX_MESH_MAX_TEXTURES_COUNT];


	//dword dwBonesCount;
	dword dwPassesCount;



	//индекс/указатель на subSubsetAnim
	fixablePointer<subSubsetAnim> pSubSubsets;
};


struct mesh
{
	nodeTypeDesc desc;

	meshStaticString name;
	fixablePointer<nodeTransform> pTransform;
	fixablePointer<nodeTransform> pParentTransform;

	Vector min;
	Vector max;

	dword dwRenderOrder;

	dword bCollision;
	dword bVisible;

	dword dwSubsetCount;

	fixablePointer<subsetStatic> pStaticSubsets;
	fixablePointer<subsetAnim> pAnimSubsets;



};


struct header
{
	dword dwHeader;
	dword dwVersion;

	dword bAnimated;

	fixablePointer<IDirect3DIndexBuffer9> inplaceIB;
	IIBuffer * indexBuffer;

	fixablePointer<renderOrder> pRenderOrders;
	dword dwOrdersCount;

	fixablePointer<IDirect3DVertexBuffer9> inplaceVBStream0;
	fixablePointer<IDirect3DVertexBuffer9> inplaceVBStream1;

	IVBuffer * stream0;
	IVBuffer * stream1;


	GMXBoundBox staticBoundBox;
	GMXBoundSphere staticBoundSphere;


	dword bHaveBlendShapes;


	dword dwBonesCount;
	fixablePointer<boneDataReadOnly> pBonesInfo;
	fixablePointer<boneMtxInputReadOnly> pBonesMatrices;

	dword dwStringTableSize;
	dword dwStaticDataSize;

	dword dwMaterialsCount;
	fixablePointer<material> pMaterials;

	dword dwLocatorsCount;
	fixablePointer<locator> pLocators;


	dword dwMeshesCount;
	fixablePointer<mesh> pMeshes;

	dword dwSubsetsCount;
	fixablePointer<subsetStatic> pStaticSubsets;
	fixablePointer<subsetAnim> pAnimSubsets;

	dword dwTranslateTableCount;
	fixablePointer<TTABLEITEM> pTranslateTables;


	dword dwSubSubsetAnimCount;
	fixablePointer<subSubsetAnim> pSubSubsets;



	dword dwBlendShapeBonesCount;
	fixablePointer<blendShapeBoneDescription> pBlendShapeBones;

	dword dwBlendShapeTargetsCount;
	fixablePointer<blendShapeTarget> pBlendShapeTargets;

	dword dwBlendShapeBonesTransformCount;
	fixablePointer<blendShapeBoneTransform> pBlendShapeBonesTransform;


	//Всего матриц трансформаций статических+динамических
	dword dwTransformationsCount;
	dword dwStaticTransformationsCount;

	//указатель на статически и динамические трансформации, сначала лежат все статические, потом все динамические
	//обновлять в движке нужно только матрицы динамических каждый кадр
	fixablePointer<nodeTransform> pAllTransformations;


	dword dwDynamicTransformationsCount;
	//"Динамические" это те узлы которые сами привязаны к костям или дети узлов привязанных к костям
	//их матрицы меняются каждый кадр - если есть анимация
	fixablePointer<nodeTransform> pDynamicTransformations;


	dword dwCreatedPhysicMeshesCount;
	//на самом деле class IPhysicsScene::MeshInit;
	fixablePointer<void> pCreatedPhysicMeshes;





	dword dwLocatorsWithParticlesCount;


	dword dwBonesPaddingSize;
	dword dwTransformsPaddingSize;


	dword dwIndexBufferDataSize;
	dword dwVertexBufferDataSize0;
	dword dwVertexBufferDataSize1;

	dword dwIndexBufferIndexCount;
	dword dwVertexBufferVertexCount0;
	dword dwVertexBufferVertexCount1;

	dword dwIndexBufferDataSize4KAligned;
	dword dwVertexBufferDataSize4KAligned0;
	dword dwVertexBufferDataSize4KAligned1;

	dword dwNovodexCollisionPC_size;
	dword dwNovodexCollisionXBOX360_size;

	dword sizeofD3DVertexBuffer;
	dword sizeofD3DIndexBuffer;

	dword dwPadding0;
	dword dwPadding1;
	dword dwPadding2;

	dword dwBlendShapeBonesTransformPaddingSize;


	byte * physicalMemory;
	byte * collisionDataPlatform;
	fixablePointer<locator> entryTable[HASH_TABLE_SIZE];


#ifdef _XBOX
	__forceinline void swizzle(bool bNeedSwizzlePointers)
	{
	}
#else
	__forceinline void swizzle(bool bNeedSwizzlePointers)
	{
		MeshSwizzleDWord(dwStringTableSize);

		MeshSwizzleDWord(bAnimated);
		MeshSwizzleDWord(bHaveBlendShapes);
		MeshSwizzleDWord(dwBlendShapeBonesTransformCount);
		MeshSwizzleDWord(dwBlendShapeBonesCount);
		MeshSwizzleDWord(dwBlendShapeTargetsCount);
		MeshSwizzleDWord(dwBonesCount);
		MeshSwizzleDWord(dwBonesPaddingSize);
		MeshSwizzleDWord(dwDynamicTransformationsCount);
		MeshSwizzleDWord(dwHeader);
		MeshSwizzleDWord(dwIndexBufferDataSize);
		MeshSwizzleDWord(dwLocatorsCount);
		MeshSwizzleDWord(dwMaterialsCount);
		MeshSwizzleDWord(dwMeshesCount);
		MeshSwizzleDWord(dwOrdersCount);
		MeshSwizzleDWord(dwStaticDataSize);
		MeshSwizzleDWord(dwStaticTransformationsCount);
		MeshSwizzleDWord(dwSubsetsCount);
		MeshSwizzleDWord(dwSubSubsetAnimCount);
		MeshSwizzleDWord(dwTransformationsCount);
		MeshSwizzleDWord(dwTransformsPaddingSize);
		MeshSwizzleDWord(dwTranslateTableCount);
		MeshSwizzleDWord(dwVersion);
		MeshSwizzleDWord(dwVertexBufferDataSize0);
		MeshSwizzleDWord(dwVertexBufferDataSize1);
		MeshSwizzleDWord(dwCreatedPhysicMeshesCount);
		MeshSwizzleDWord(dwLocatorsWithParticlesCount);
		MeshSwizzleDWord(dwBlendShapeBonesTransformPaddingSize);






		MeshSwizzleDWord(dwPadding0);
		MeshSwizzleDWord(dwPadding1);
		MeshSwizzleDWord(dwPadding2);


		MeshSwizzleDWord(dwVertexBufferVertexCount0);
		MeshSwizzleDWord(dwVertexBufferVertexCount1);

		MeshSwizzleDWord(dwVertexBufferDataSize4KAligned0);
		MeshSwizzleDWord(dwVertexBufferDataSize4KAligned1);

		MeshSwizzleDWord(dwIndexBufferDataSize4KAligned);
		MeshSwizzleDWord(dwIndexBufferIndexCount);

		MeshSwizzleDWord(sizeofD3DVertexBuffer);
		MeshSwizzleDWord(sizeofD3DIndexBuffer);


		MeshSwizzleVector3(staticBoundBox.vMax);
		MeshSwizzleVector3(staticBoundBox.vMin);

		MeshSwizzleVector3(staticBoundSphere.vCenter);
		MeshSwizzleFloat(staticBoundSphere.fRadius);


		MeshSwizzleDWord(dwNovodexCollisionPC_size);
		MeshSwizzleDWord(dwNovodexCollisionXBOX360_size);



		if (bNeedSwizzlePointers == false)
		{
			return;
		}


		MeshSwizzleDWord(pCreatedPhysicMeshes.offsetInBytes);

		MeshSwizzleDWord(inplaceVBStream0.offsetInBytes);
		MeshSwizzleDWord(inplaceVBStream1.offsetInBytes);
		MeshSwizzleDWord(inplaceIB.offsetInBytes);

		for (dword i = 0; i < HASH_TABLE_SIZE; i++)
		{
			MeshSwizzleDWord(entryTable[i].index);
		}


		MeshSwizzleDWord(pAllTransformations.offsetInBytes);
		MeshSwizzleDWord(pAnimSubsets.offsetInBytes);
		MeshSwizzleDWord(pBlendShapeBones.offsetInBytes);
		MeshSwizzleDWord(pBlendShapeBonesTransform.offsetInBytes);
		MeshSwizzleDWord(pBlendShapeTargets.offsetInBytes);
		MeshSwizzleDWord(pBonesInfo.offsetInBytes);
		MeshSwizzleDWord(pBonesMatrices.offsetInBytes);
		MeshSwizzleDWord(pDynamicTransformations.offsetInBytes);
		MeshSwizzleDWord(pLocators.offsetInBytes);
		MeshSwizzleDWord(pMaterials.offsetInBytes);
		MeshSwizzleDWord(pMeshes.offsetInBytes);
		MeshSwizzleDWord(pRenderOrders.offsetInBytes);
		MeshSwizzleDWord(pStaticSubsets.offsetInBytes);
		MeshSwizzleDWord(pSubSubsets.offsetInBytes);
		MeshSwizzleDWord(pTranslateTables.offsetInBytes);

		//MeshSwizzleDWord(stream0);
		//MeshSwizzleDWord(stream1);
		//MeshSwizzleDWord(indexBuffer);
		//MeshSwizzleDWord(physicalMemory);

	}
#endif



};




//Рассчитаная матрица кости
//в каждой сцене своя - референсируется, тут посчитанные данные для GPU скининга
//(3 blocks by 16 bytes = 48 bytes)
struct boneGpuData
{
	/*
	float vx_x;
	float vy_x;
	float vz_x;
	float pos_x;

	float vx_y;
	float vy_y;
	float vz_y;
	float pos_y;

	float fAlpha;
	float fScale;
	float unsued_empty_padding;
	float pos_z;
	*/

	Vector4 data[3];
};

/*
//(1 blocks by 16 bytes = 16 bytes)
//Та часть кости скелета, которая референсируется в каждую модель - тут рассчитанные баунды и сфера для тестов
struct boneCachedData
{
//(16) Должен быть выравнен на 16 байт
Vector4 bbSphereCenterGlobalAndRadius;

//(16) Должен быть выравнен на 16 байт
//Vector4 bbMinGlobal;

//(16) Должен быть выравнен на 16 байт
//Vector4 bbMaxGlobal;
};
*/

//Бинд поза кости...
//В одном экземпляре на все загруженные модели, read-only
//(4 blocks by 16 bytes = 64 bytes)
struct boneMtxInputReadOnly
{
	Matrix mtxBindPose;
};


//(4 blocks by 16 bytes = 64 bytes)
//Глобальная часть кости скелета, которая одна на все загруженные модели, всегда в одном экземлпяре read-only
struct boneDataReadOnly
{
	//(12) Должен быть выравнен на 16 байт
	Vector bbMin;

	//(4) Ссылка strTableOffset заменяющаяся на указатель
	meshStaticString name;

	//(12) Должен быть выравнен на 16 байт
	Vector bbMax;

	dword dwPadding;

	//(16) Должен быть выравнен на 16 байт
	Vector4 bbSphereCenterAndRadius;

	//(16) Должен быть выравнен на 16 байт
	Vector4 bbSphereRadius;
};

#endif