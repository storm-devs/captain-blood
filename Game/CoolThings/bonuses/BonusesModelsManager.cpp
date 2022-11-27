

#include "BonusesModelsManager.h"


BonusesModelsManager::Index::Index()
{
	modelIndex = -1;
	boneIndex = -1;
}

bool BonusesModelsManager::Index::IsEmpty()
{
	return modelIndex < 0;
}


BonusesModelsManager::BonusesModelsManager() : loadedScenes(_FL_, 256),
												names(_FL_, 4096)
{
	fileService = (IFileService *)api->GetService("FileService");
	Assert(fileService);
	geometryService = (IGMXService *)api->GetService("GMXService");
	Assert(geometryService);
	memset(creationSkeleton, 0, sizeof(creationSkeleton));
}


BonusesModelsManager::~BonusesModelsManager()
{
	for(long i = 0; i < loadedScenes; i++)
	{
		Model & m = loadedScenes[i];
		m.scene->Release();
		if(m.ani)
		{
			m.ani->Release();			
		}
	}
}

//Создать модельку
bool BonusesModelsManager::CreateModel(const char * modelName, IAnimationScene * animationScene, IParticleManager* particlesManager, ISoundScene * soundScene, Index & model)
{
	if(string::IsEmpty(modelName)) return false;
	strBuffer = modelName;
	fileName.GetFileTitle(strBuffer);
	fileName.Lower();
	dword len = 0;
	long hash = string::Hash(fileName.c_str(), len);
	//Смотрим среди загруженных
	for(long i = 0, last = -1; i < loadedScenes; i++)
	{
		Model & m = loadedScenes[i];
		Name & name = (Name &)names[m.nameIndex];
		if(name.hash == hash && name.len == len &&  fileName == name.str)
		{
			//Перебираем по найденному имени, возможно есть доступная копия
			do
			{
				last = i;
				if(m.ani)
				{
					dword count = m.ani->GetNumBones();
					for(dword j = 0; j < count; j++)
					{
						dword index = j >> 5;
						dword mask = 1 << (j & 0x1f);
						if(!(m.use[index] & mask))
						{
							m.use[index] |= mask;
							model.modelIndex = i;
							model.boneIndex = j;
							return true;
						}
					}
				}else{
					if(!m.use[0])
					{
						m.use[0] = 1;						
						model.modelIndex = i;
						model.boneIndex = -1;
						return true;
					}
				}
				i = loadedScenes[i].secondIndex;
			}while(i >= 0);
			break;
		}
	}
	//Загруженных нет, нужна новая модель
	IGMXScene * scene = geometryService->CreateScene(modelName, animationScene, particlesManager, soundScene, _FL_);
	if(!scene)
	{
		return false;
	}	
	scene->SetDynamicLightState(true);
	scene->SetShadowReceiveState(false);
	scene->SetFloatAlphaReference(0.0f);
	model.modelIndex = loadedScenes.Add();
	model.boneIndex = -1;
	Model & m = loadedScenes[model.modelIndex];
	m.scene = scene;
	m.ani = null;
	m.use[0] = 1;
	for(dword i = 1; i < ARRSIZE(m.use); i++) m.use[i] = 0;
	m.secondIndex = -1;
	if(last >= 0)
	{
		m.nameIndex = loadedScenes[last].nameIndex;
		loadedScenes[last].secondIndex = model.modelIndex;
	}else{
		m.nameIndex = names.Size();
		names.AddElements(sizeof(Name) + fileName.Len());
		Name & name = (Name &)names[m.nameIndex];
		name.hash = hash;
		name.len = len;
		const char * str = fileName.c_str();
		for(dword i = 0; i <= len; i++) name.str[i] = str[i];
	}	
	//Заноcим в массив все рутовые кости в модели
	boneDataReadOnly * boneData = null;
	boneMtxInputReadOnly * boneTransformations = null;
	dword count = scene->GetBonesArray(&boneData, &boneTransformations);
	if(count > 0)
	{
		//Создаём для модельки анимацию
		if(count > 256) count = 256;		
		for(dword i = 0; i < count; i++)
		{
			creationSkeleton[i].name = boneData[i].name.c_str();
		}
		m.ani = animationScene->CreateProcedural(creationSkeleton, count, _FL_);
		Assert(m.ani);
		model.boneIndex = 0;
		m.scene->SetAnimation(m.ani);
	}
	return true;
}

//Удалить модель
void BonusesModelsManager::DeleteModel(Index & model)
{
	if(model.modelIndex < 0) return;
	Model & m = loadedScenes[model.modelIndex];
	if(m.ani)
	{
		dword index = model.boneIndex >> 5;
		dword mask = 1 << (model.boneIndex & 0x1f);
		m.use[index] &= ~mask;		
		m.ani->SetBoneMatrix(model.boneIndex, Matrix(true).SetZero());
	}else{
		m.use[0] = 0;
	}
	model.modelIndex = -1;
	model.boneIndex = -1;
}

//Установить матрицу бонуса
void BonusesModelsManager::SetTransform(const Index & model, const Matrix & mtx, float alpha, float scale)
{
	if(model.modelIndex < 0) return;
	Model & m = loadedScenes[model.modelIndex];
	if(m.ani)
	{
		m.ani->SetBoneMatrix(model.boneIndex, mtx);
		m.scene->SetBoneAlpha(model.boneIndex, Clampf(alpha));
		m.scene->SetBoneScale(model.boneIndex, Clampf(scale));
	}else{
		Matrix mt(mtx);
		mt.Scale3x3(scale);
		m.scene->SetTransform(mt);
		m.scene->SetUserColor(Color(0.0f, 0.0f, 0.0f, Clampf(alpha)));
	}	
}

//Нарисовать модели
void BonusesModelsManager::Draw()
{
	for(long i = 0; i < loadedScenes; i++)
	{
		Model & m = loadedScenes[i];
		for(dword i = 0; i < ARRSIZE(m.use); i++)
		{
			if(m.use[i])
			{
				m.scene->Draw();
				break;
			}
		}
	}
}







