#pragma once

#include "..\..\..\Common_h\mission.h"

class TaranBlender : public IAniBlendStage
{
	struct PoseElement
	{
		long lightIndex, strongIndex;
		// Quaternion for light animation, Quaternion for strong animation
		Quaternion ql, qs;
		// Position for light animation, position for strong animation
		Vector pl, ps;
	};	

public:
	TaranBlender(IAnimation * idle, IAnimation * light, IAnimation * strong);
	virtual ~TaranBlender();

	//Прочитать данные
	void ReadData();	
	//Перезапустить блендер
	void Restart();
	//Получить необходимость в данных предыдущих стадий
	virtual bool NeedPrevStageData(long boneIndex);
	//Получить коэфициент блендинга для интересующей кости
	virtual float GetBoneBlend(long boneIndex);
	//Получить трансформацию кости
	virtual void GetBoneTransform(long boneIndex, Quaternion & rotation, Vector & position, Vector & scale, const Quaternion & prevRotation, const Vector & prevPosition, const Vector & prevScale);
	//Обновить состояние
	virtual void Update(float dltTime);

	void SetBlend(float blend);

protected:
	//Составить таблицу соостветствия по скелетам
	void Assign();

protected:
	array<PoseElement> pose;	// Поза из которой блендимся
	float kBlend;				// Текущий коэфициент блендинга 
	IAnimation * idle;
	IAnimation * light;
	IAnimation * strong;
};


