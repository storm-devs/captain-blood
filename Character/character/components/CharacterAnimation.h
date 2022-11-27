
#ifndef _CharacterAnimation_h_
#define _CharacterAnimation_h_


#include "..\Character.h"
#include "AnimationsBlender.h"
#include "CharacterLogic.h"

class CharacterAnimation
{	
public:
	
	CharacterAnimation(Character * character);
	~CharacterAnimation();
	

	IAnimation* AddAnimation(const char* id, const char* anim_name);
	IAnimation* GetAnimation(const char* id);
	IAnimation* GetCurAnimation();
	IAnimation* GetLastAnimation();
	void SetAnimation(IAnimation* anim, float blend_time = 0.2f);
	void SetAnimation(const char* id, float blend_time = 0.2f);
	void SetLastAnimation(float blend_time = 0.2f);

	void EnableGraphBlend(const char* id);
	void DisableGraphBlend();
	void SetGraphBlend(float blend);

public: 

	//Активировать линк анимации
	

	//Активировать линк анимации
	bool ActivateLink(const char * link, bool isAlways = false);
	//Проверить возможность линк анимации
	bool TestActivateLink(const char * link, bool useLastAnimation = false);
	
	
	//Запутсить граф с начлаьного нода
	bool Start(bool isInstant = false);
	//Перейти на нод анимации
	bool Goto(const char * node, float blend_time);	
	//Приостановить воспроизведение анимации
	void Pause(bool pause);	
	//Задать скорость воспроизведения анимациии
	void SetPlaySpeed(float speed);
	//получить смещение из анимации
	void GetMovement(Vector &dltPos, float &dltAng);
	//Доступиться до константы типа string
	const char* GetConstString(const char * constName,const char* node_name = null);
	//Доступиться до константы типа float
	float GetConstFloat(const char * constName,const char* node_name = null);
	//Доступиться до константы типа blend
	float GetConstBlend(const char * constName);


	//Получить имя текущего нода
	const char* CurrentNode();
	//Получить имя анимации
	const char* AnimationName();

	//Установить функцию обработчика на исполнение для основной анимации
	void SetEventHandler(IAnimationListener * listener, AniEvent func, const char * eventName);
	//Удалить функцию обработчика для основной анимации
	void DelEventHandler(IAnimationListener * listener, AniEvent func, const char * eventName);
	//разрешено ли ориентентация перса в текущем проигроваемом клипе	

	void Reset();

	struct AnimData
	{
		char id[64];
		IAnimation* animation;		
	};

	array<AnimData> animData;

private:
	
	Character & chr;   //Персонаж которому принадлежим	
	IAnimation* animation;
	IAnimation* last_animation;

	int anim_blended_index;
	AnimationsBlender* anim_blender;	
};

//Активировать линк анимации
inline bool CharacterAnimation::ActivateLink(const char * link, bool isAlways)
{	
	if (chr.logic)
		chr.logic->NeedStateUpdate();

	if (chr.logic->slave)
	{
		chr.logic->slave->animation->ActivateLink(link, isAlways);
	}

	if (anim_blended_index != -1)
	{
		if (animData[anim_blended_index].animation)
			animData[anim_blended_index].animation->ActivateLink(link, isAlways);
	}

	if(animation)
	{
		return animation->ActivateLink(link, isAlways);
	}	

	return false;
}

//Проверить возможность линк анимации
inline bool CharacterAnimation::TestActivateLink(const char * link, bool useLastAnimation)
{
	if (useLastAnimation)
	{
		if (last_animation)
			return last_animation->IsCanActivateLink(link);

		return false;
	}

	if (animation)
		return animation->IsCanActivateLink(link);

	return false;
}

//Запутсить граф с начлаьного нода
inline bool CharacterAnimation::Start(bool isInstant)
{
	if (chr.logic)
		chr.logic->NeedStateUpdate();

	if (anim_blended_index != -1)
	{
		if (animData[anim_blended_index].animation)
			animData[anim_blended_index].animation->Start(null, isInstant);
	}

	if (animation)
	{		
		return animation->Start(null, isInstant);
	}

	return false;
}

inline void CharacterAnimation::Pause(bool pause)
{
	if (animation)
	{
		return animation->Pause(pause);
	}
}

//Перейти на нод анимации
inline bool CharacterAnimation::Goto(const char * node, float blend_time)
{
	if (chr.logic)
		chr.logic->NeedStateUpdate();

	if (anim_blended_index != -1)
	{
		if (animData[anim_blended_index].animation)
			animData[anim_blended_index].animation->Goto(node,blend_time);
	}

	if(animation)
	{		
		return animation->Goto(node,blend_time);
	}

	return false;
}

//Задать скорость воспроизведения анимациии
inline void CharacterAnimation::SetPlaySpeed(float speed)
{
	if (anim_blended_index != -1)
	{
		if (animData[anim_blended_index].animation)
			animData[anim_blended_index].animation->SetPlaySpeed(speed);
	}

	if (animation)
	{
		animation->SetPlaySpeed(speed);
	}
}

inline void CharacterAnimation::GetMovement(Vector &dltPos, float &dltAng)
{
	if (animation)
	{
		animation->GetMovement(dltPos);
		dltAng = 0.0f;
	}else{
		dltPos = 0.0f;
		dltAng = 0.0f;
	}
}

//Доступиться до константы типа string
inline const char* CharacterAnimation::GetConstString(const char * constName,const char* node_name)
{
	if(animation)
	{
		return animation->GetConstString(constName,node_name);
	}	

	return null;
}

//Доступиться до константы типа float
inline float CharacterAnimation::GetConstFloat(const char * constName,const char* node_name)
{
	if(animation)
	{
		return animation->GetConstFloat(constName,node_name);
	}

	return 0.0f;
}

//Доступиться до константы типа blend
inline float CharacterAnimation::GetConstBlend(const char * constName)
{
	if(animation)
	{
		return animation->GetConstBlend(constName);
	}
	return 0.0f;
}

//Получить имя текущего нода
inline const char * CharacterAnimation::CurrentNode()
{
	if(animation)
	{
		return animation->CurrentNode();
	}
	return "";
}

//Получить имя анимации
inline const char * CharacterAnimation::AnimationName()
{
	if(animation)
	{
		const char * name = animation->GetName();
		return name ? name : "";
	}
	return "Animation not loaded";
}

inline void CharacterAnimation::SetEventHandler(IAnimationListener * listener, AniEvent func, const char * eventName)
{
	if(animation)
	{
		animation->SetEventHandler(listener, func, eventName);
	}
}

inline void CharacterAnimation::DelEventHandler(IAnimationListener * listener, AniEvent func, const char * eventName)
{
	if(animation)
	{
		animation->DelEventHandler(listener, func, eventName);
	}
}

#endif

