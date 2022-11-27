/********************************************************************
created:	2009 dec
author:		Sergey Makeev
purpose:	mesh static data class
*********************************************************************/
#ifndef _MESH_DATA_CONTAINER_CLASS_
#define _MESH_DATA_CONTAINER_CLASS_


#include "..\..\common_h\render.h"
#include "..\..\common_h\templates.h"
#include "..\..\common_h\gmxservice\MeshStructure.h"
#include "mesh_data.h"


//Создавать ресурсы X360 GPU (vb & ib) инплейс методом за одну физическую аллокацию
//если не установлено, то использовать обычные Create Vertex/Index Buffer,
#ifdef _XBOX
#define USE_XBOX_INPLACE_GPU_RESOURCES
#endif

class IDataFile;
class IFileService;


class MeshContainer
{

protected:

	int refCount;

	IRender* pRS;

	header msh;

	byte * const_data;
	byte * const_data_raw;

	static MESH_INLINE void ApplyMaterialToSubsetStatic(material * __restrict mat, subsetStatic * __restrict sub)
	{
		sub->tex[0] = mat->tex[0];
		sub->tex[1] = mat->tex[1];
		sub->tex[2] = mat->tex[2];
		sub->tex[3] = mat->tex[3];
		sub->tech = mat->tech;

		sub->isNoAlpha = mat->isNoAlpha;
		sub->dwBonesInSkin = mat->dwBonesInSkin;
	}

	static MESH_INLINE void ApplyMaterialToSubsetAnimated(material * __restrict mat, subsetAnim * __restrict sub)
	{
		sub->tex[0] = mat->tex[0];
		sub->tex[1] = mat->tex[1];
		sub->tex[2] = mat->tex[2];
		sub->tex[3] = mat->tex[3];
		sub->tech = mat->tech;

		sub->isNoAlpha = mat->isNoAlpha;
		sub->dwBonesInSkin = mat->dwBonesInSkin;
	}






/*

	MESH_INLINE const char * getString(dword dwOffset) const
	{
		if (dwOffset == INVALID_STRING_TABLE_ENTRY)
		{
			return NULL;
		}

		return (const char*)(const_data + dwOffset);
	}

	MESH_INLINE dword getStringHashByPtr(const char * strPtr) const
	{
		if (strPtr == NULL)
		{
			return 0xFFFFFFFF;
		}

		//На два dword от начала имени лежит хеш
		dword * pNameNoCaseHash = (dword *)(strPtr - sizeof(dword) - sizeof(dword));

		return *pNameNoCaseHash;
	}

	MESH_INLINE dword getStringLenByPtr(const char * strPtr) const
	{
		if (strPtr == NULL)
		{
			return 0xFFFFFFFF;
		}

		//На один dword от начала имени лежит длинна
		dword * pStringSize = (dword *)(strPtr - sizeof(dword));

		return *pStringSize;
	}



	MESH_INLINE dword getStringHash(dword dwOffset) const
	{
		return getStringHashByPtr(getString(dwOffset));
	}


	MESH_INLINE dword getStringLen(dword dwOffset) const
	{
		return getStringLenByPtr(getString(dwOffset));
	}



	
	MESH_INLINE const char * getMaterialShaderName(material * __restrict l) const
	{
		dword dwOffset = l->dwTechniqueID;
		return getString(dwOffset);
	}

	MESH_INLINE const char * getMaterialTextureName(material * __restrict l, dword dwIdx) const
	{
		Assert(dwIdx >= 0 && dwIdx <= 3);
		dword dwOffset = l->textures[dwIdx];
		return getString(dwOffset);
	}


	MESH_INLINE const char * getMeshName(mesh * __restrict m) const
	{
		dword dwOffset = m->dwNameID;
		return getString(dwOffset);
	}


	MESH_INLINE const char * getLocatorName(locator * __restrict l) const
	{
		dword dwOffset = l->dwNameID;
		return getString(dwOffset);
	}

	MESH_INLINE const char * getLocatorBoneName(locator * __restrict l) const
	{
		dword dwOffset = l->dwBoneNameID;
		return getString(dwOffset);
	}

	MESH_INLINE const char * getLocatorParticlesName(locator * __restrict l) const
	{
		dword dwOffset = l->dwParticlesNameID;
		return getString(dwOffset);
	}

	MESH_INLINE const Matrix & getLocatorMatrix (locator * __restrict l) const
	{
		return l->pTransform.ptr->mtxTransform;
	}
*/



	void LoadAsStatic(IDataFile* file);
	void LoadAsAnimated(IDataFile* file);
	void CreateAndLinkMaterials();

	void CreateCollision();


	void FixupPointers();
	void AllocConstantData(dword dwBytesCount);

#ifndef _XBOX
	void SwizzleStaticData();
#endif


public:


	MeshContainer(IRender* renderService);
	virtual ~MeshContainer();


	virtual bool Load (IDataFile * file, IFileService * pFS, const char * nameForDebugMessages);


public:

	virtual void ForceRelease ();
	virtual bool Release ();
	virtual void AddRef ();


public:

	const header & getHeader() const
	{
		return msh;
	}

	dword getBonesCount() const
	{
		return msh.dwBonesCount;
	}

	dword getDynamicTransformsCount() const
	{
		return msh.dwDynamicTransformationsCount;
	}

	dword getLocatorsWithParticlesCount() const
	{
		return msh.dwLocatorsWithParticlesCount;
	}

	//Сколько морф-таргетов в сцене
	dword getBlendShapesMorphTargetsCount() const
	{
		return msh.dwBlendShapeTargetsCount;
	}

	//Сколько костей используется в blendshape
	dword getBlendShapesBonesCount() const
	{
		return msh.dwBlendShapeBonesCount;
	}

	


};

#endif 