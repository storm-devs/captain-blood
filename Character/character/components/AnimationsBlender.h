

#ifndef _AnimationsBlender_h_
#define _AnimationsBlender_h_

#include "..\..\..\Common_h\mission.h"

class AnimationsBlender : public IAniBlendStage
{
	struct PoseElement
	{
		Quaternion q;
		Vector p;
		long sourseIndex;
	};	

public:
	AnimationsBlender(IAnimation * from, IAnimation * to, float time, bool autoBlend);
	virtual ~AnimationsBlender();

	//Прочитать данные
	void ReadData();	
	//Перезапустить блендер
	AnimationsBlender* Restart();
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
	//Составить таблицу соостветствия по скилетам
	void Assign(IAnimation * from, IAnimation * to);

protected:
	array<PoseElement> pose;	//Поза из которой блендимся
	float k, dlt;				//Текущий коэфициент блендинга и скорость убывания
	IAnimation * from;			//Анимация из которой берём позу
	bool isAutoBlend;			//Автаматический бленд графов
};



#endif

