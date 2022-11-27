

#include "AnimationProcedural.h"
#include "AnimationService.h"


dword AnimationProcedural::filesCounter[4] = {0, 0, 0, 0};


AnimationProcedural::AnimationProcedural(AnimationScene & s, const IAnimationScene::Bone * skeleton, dword count, const char * _cppFile, long _cppLine) : scene(s)
{
	Assert(skeleton);
	Assert(count > 0);
	Assert(count <= 256);
	refCounter = 1;
	bonesCount = count;
	//Создаём имя анимации
	string name = "*ProceduralFile";
	for(long i = 0, add = 1; i < ARRSIZE(filesCounter); i++)
	{
		name += '_';
		name += filesCounter[i];
		if(add)
		{
			add = (filesCounter[i] == 0xffffffff);
			filesCounter[i]++;
		}		
	}
	//Вычисляем требуемый размер массива
	dword size = count*(sizeof(Matrix) + sizeof(BoneDesc)) + name.Len() + 0x20;
	for(dword i = 0; i < count; i++)
	{
		size += string::Len(skeleton[i].name) + 1;
	}
	buffer = NEW byte[size];
	memcpy(buffer, name.c_str(), name.Len() + 1);
	matrices = (Matrix *)(((__int64)buffer + name.Len() + 16) & ~0xf);
	bones = (BoneDesc *)(matrices  + count);
	char * names = (char *)(bones + count);
	for(dword i = 0; i < count; i++)
	{
		matrices[i] = skeleton[i].mtx;
		BoneDesc & bone = bones[i];
		bone.parent = skeleton[i].parentIndex;
		bone.longName.name = names;
		bone.shortName.name = names;		
		for(const char * src = skeleton[i].name; *src; *names++ = *src++)
		{
			if(*src == '|')
			{
				bone.shortName.name = names + 1;
			}
		}
		*names++ = 0;
		bones[i].longName.hash = string::HashNoCase(bones[i].longName.name);
		bones[i].shortName.hash = string::HashNoCase(bones[i].shortName.name);
	}
	Assert((byte *)names - buffer < (long)size);
	cppFile = _cppFile;
	cppLine = _cppLine;
}

AnimationProcedural::~AnimationProcedural()
{
	AssertCoreThread
	delete buffer;
	matrices = null;
	bones = null;
	bonesCount = 0;
	buffer = null;	
	scene.Delete(this);
}

//Получить имя анимации
const char * AnimationProcedural::GetName()
{
	return (const char *)buffer;
}

//Копировать интерфейс
IAnimationTransform * AnimationProcedural::Clone()
{
	Assert(refCounter > 0);
	refCounter++;
	return this;
}

//Удалить интерфейс
void AnimationProcedural::Release()
{
	Assert(refCounter > 0);
	refCounter--;
	if(refCounter <= 0)
	{
		delete this;
	}
}

//Удалить интерфейс принудительно
void AnimationProcedural::ForceRelease()
{
	api->Trace("Animation -> Don't release procedural animation (cpp: %s, %u)", cppFile, cppLine);
	refCounter = 1;
	Release();
}


//Получить количество костей в анимации
long AnimationProcedural::GetNumBones()
{
	return bonesCount;
}

//Получить имя кости
const char * AnimationProcedural::GetBoneName(long index)
{
	Assert(index >= 0 && index < bonesCount);
	return bones[index].longName.name;
}

//Получить индекс родительской кости
long AnimationProcedural::GetBoneParent(long index)
{
	Assert(index >= 0 && index < bonesCount);
	return bones[index].parent;
}

//Найти по имени кость
long AnimationProcedural::FindBone(const char * boneName, bool shortName)
{
	if(!boneName || !boneName[0]) return -1;
	dword hash = string::HashNoCase(boneName);
	for(long i = 0; i < bonesCount; i++)
	{
		Name & name = shortName ? bones[i].shortName : bones[i].longName;
		if(name.hash == hash)
		{
			if(string::IsEqual(name.name, boneName))
			{
				return i;
			}
		}
	}
	return -1;
}

//Найти по имени кость (короткое имя)
long AnimationProcedural::FindBoneUseHash(const char * boneName, dword hash)
{
	for(long i = 0; i < bonesCount; i++)
	{
		if(bones[i].longName.hash == hash)
		{
			if(string::IsEqual(bones[i].longName.name, boneName))
			{
				return i;
			}
		}
	}
	return -1;
}

//Получить матрицу кости с учётом иерархии
const Matrix & AnimationProcedural::GetBoneMatrix(long index)
{
	Assert(index >= 0 && index < bonesCount);
	return matrices[index];
}

//Получить массив матриц с учётом иерархии
const Matrix * AnimationProcedural::GetBoneMatrices()
{
	return matrices;
}

//Установить пользовательскую матрицу кости
void AnimationProcedural::SetBoneMatrix(long index, const Matrix & mtx)
{
	Assert(index >= 0 && index < bonesCount);
	matrices[index] = mtx;
}

//Установить нулевую матрицу кости
void AnimationProcedural::CollapseBone(long index)
{
	Assert(index >= 0 && index < bonesCount);
	matrices[index].SetZero();
}