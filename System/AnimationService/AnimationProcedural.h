
#ifndef _AnimationProcedural_h_
#define _AnimationProcedural_h_


#include "..\..\common_h\core.h"
#include "..\..\common_h\Animation.h"
#include "..\..\common_h\templates\array.h"
#include "..\..\common_h\templates\string.h"

class AnimationScene;

class AnimationProcedural : public IAnimationProcedural
{
	struct Name
	{
		const char * name;
		long hash;
	};

	struct BoneDesc
	{
		long parent;
		Name longName;
		Name shortName;
	};

public:
	AnimationProcedural(AnimationScene & s, const IAnimationScene::Bone * skeleton, dword count, const char * _cppFile, long _cppLine);
	~AnimationProcedural();

public:
	//Получить имя анимации
	virtual const char * GetName();
	
public:
	//Копировать интерфейс
	virtual IAnimationTransform * Clone();
	//Удалить интерфейс
	virtual void Release();
	//Удалить интерфейс принудительно с сообщением об ошибке
	void ForceRelease();

public:
	//Получить количество костей в анимации
	virtual long GetNumBones();
	//Получить имя кости
	virtual const char * GetBoneName(long index);
	//Получить индекс родительской кости
	virtual long GetBoneParent(long index);
	//Найти по имени кость
	virtual long FindBone(const char * boneName, bool shortName);
	//Найти по имени кость (короткое имя)
	virtual long FindBoneUseHash(const char * boneName, dword hash);

	//Получить матрицу кости с учётом иерархии
	virtual const Matrix & GetBoneMatrix(long index);
	//Получить массив матриц с учётом иерархии
	virtual const Matrix * GetBoneMatrices();
	//Установить пользовательскую матрицу кости
	virtual void SetBoneMatrix(long index, const Matrix & mtx);
	//Установить нулевую матрицу кости
	virtual void CollapseBone(long index);

private:
	Matrix * matrices;				//Массив матриц
	BoneDesc * bones;				//Массив описаний костей
	long bonesCount;				//Количество костей
	long refCounter;				//Счётчик ссылок	
	byte * buffer;					//Буфер с данными
	AnimationScene & scene;			//Сцена, которой принадлежит анимация
	const char * cppFile;			//Исходный Файл в котором был создан объект
	long cppLine;					//Строка от куда был создан объект
	static dword filesCounter[4];	//Счётчик файлов для генерации уникального имени
};

#endif
