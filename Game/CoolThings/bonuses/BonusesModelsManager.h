

#ifndef _BonusesModelsManager_h_
#define _BonusesModelsManager_h_

#include "..\..\..\Common_h\mission.h"

class BonusesModelsManager
{
	struct Model
	{
		IGMXScene * scene;			//Моделька с инстансами
		IAnimationProcedural * ani;	//Пустая анимация для инстансных моделек
		dword use[8];				//Список флажков используемых костей
		long secondIndex;			//Индекс на следующую такуюже модельку
		dword nameIndex;			//Нормализованное имя модельки
	};

	struct Name
	{
		dword hash;
		dword len;
		char str[1];
	};

public:

	struct Index
	{
		Index();
		bool IsEmpty();

		long modelIndex;			//Индекс модели
		long boneIndex;				//Индекс кости
	};


public:
	BonusesModelsManager();
	~BonusesModelsManager();


	//Создать модельку
	bool CreateModel(const char * modelName, IAnimationScene * animationScene, IParticleManager* particlesManager, ISoundScene * soundScene, Index & model);
	//Удалить модель
	void DeleteModel(Index & model);
	//Установить матрицу бонуса
	void SetTransform(const Index & model, const Matrix & mtx, float alpha, float scale);
	//Нарисовать модели
	void Draw();

private:
	array<Model> loadedScenes;
	array<char> names;
	IFileService * fileService;
	IGMXService * geometryService;
	string fileName, strBuffer;
	IAnimationScene::Bone creationSkeleton[256];
};


#endif
