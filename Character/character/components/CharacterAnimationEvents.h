//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// Character
//============================================================================================
// CharacterAnimationEvents
//============================================================================================

#include "..\..\..\common_h\AnimationStdEvents.h"

#ifndef _CharacterAnimationEvents_h_
#define _CharacterAnimationEvents_h_


class Character;

class CharacterAnimationEvents : public AnimationStdEvents
{
	struct UpdateEvents
	{
		IAnimation * a;
		IAnimation::EventID showTrail;
		IAnimation::EventID hideTrail;
		IAnimation::EventID begAttack;
		IAnimation::EventID endAttack;
	};

	enum TimeEventUsers
	{
		teu_attack = 1,
		teu_trail = 2,
	};

public:
	CharacterAnimationEvents(Character * c);
	~CharacterAnimationEvents();

	//Прошёл заданный временной промежуток в анимации
	virtual void TimeEvent(IAnimation * a);

	//Установить обработчики анимации на заданную анимацию
	virtual void SetAnimation(IAnimation * anx);

	
	void _cdecl MissionCommand(IAnimation * ani, const char * name, const char ** params, dword numParams);

	//Отметить попадание
	void _cdecl ShootDamage(IAnimation * ani, const char * name, const char ** params, dword numParams);

	//Отметить попадание
	void _cdecl ShootTargets(IAnimation * ani, const char * name, const char ** params, dword numParams);
		
	//Отметить попадание
	void _cdecl AttachChar(IAnimation * ani, const char * name, const char ** params, dword numParams);

	//Отметить попадание
	void _cdecl DeattachChar(IAnimation * ani, const char * name, const char ** params, dword numParams);

	//Замедлить на время анимацию персонажа и его нападающего
	void _cdecl AnimSlowDown(IAnimation * ani, const char * name, const char ** params, dword numParams);

	//Переместить логический локатор
	void _cdecl MoveLogicLocator(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Завести партикловую систему в локаторе итема
	void _cdecl PlayItemParticles(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Завести партикловую систему привязанную к локатору итема
	void _cdecl PlayItemLinkedParticles(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Показать след от оружия
	void _cdecl ShowTrail(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Скрыть след от оружия
	void _cdecl HideTrail(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Подготовить анимацию к аттаке, подняв внутреннии фпс
	void _cdecl PrepareForAttack(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Начать атаку
	void _cdecl BeginAttack(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Закончить атаку
	void _cdecl EndAttack(IAnimation * ani, const char * name, const char ** params, dword numParams);	
	//Отдать персонажу команду
	void _cdecl CharacterCommand(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Активировать миссионный объект
	void _cdecl ActivateObject(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Момент броска бомбы
	void _cdecl DropBomb(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Перейти в рэгдол
	void _cdecl AniToRagdoll(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Перейти в анимацию
	void _cdecl RagdollToAni(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Перейти в рэгдол
	void _cdecl ArmRagdoll(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Отлетание головы
	void _cdecl FlyBodyPart(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Бросание предметов
	void _cdecl DropItems(IAnimation * ani, const char * name, const char ** params, dword numParams);	
	//Бросанить предмет
	void _cdecl DropItem(IAnimation * ani, const char * name, const char ** params, dword numParams);	
		
	void _cdecl BeginMiniGameParams(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Выбираем линк анимации	
	void _cdecl EndMiniGame(IAnimation * ani, const char * name, const char ** params, dword numParams);	

	//Повернуть в сторону нападающего
	void _cdecl OrientOnAttacker(IAnimation * ani, const char * name, const char ** params, dword numParams);	
	//Довестись до ближайшего противника
	void _cdecl AutoAim(IAnimation * ani, const char * name, const char ** params, dword numParams);
	//Толкнуть перса
	void _cdecl Kick(IAnimation * ani, const char * name, const char ** params, dword numParams);	
	//Кровь на камере
	void _cdecl BloodPuff(IAnimation * ani, const char * name, const char ** params, dword numParams);		
	//Голос персонажа
	void _cdecl SndVoice(IAnimation * ani, const char * name, const char ** params, dword numParams);

	//Начать вибрировыание :)
	void _cdecl StartVibration(IAnimation * ani, const char * name, const char ** params, dword numParams);
	
	//Бросить бонус
	void _cdecl DropBonus(IAnimation * ani, const char * name, const char ** params, dword numParams);

	//Включить режим SloMO
	void _cdecl BeginSloMo(IAnimation * ani, const char * name, const char ** params, dword numParams);

	// Попытка включить slowmo или сделать отложенное включение, входные параметры:
	void _cdecl Slowmo(IAnimation * ani, const char * name, const char ** params, dword numParams);
	
	//Отметить маркер SloMO
	void _cdecl SloMoMarker(IAnimation * ani, const char * name, const char ** params, dword numParams);
	
	//Отметить маркер SloMO
	void _cdecl FootStep(IAnimation * ani, const char * name, const char ** params, dword numParams);

	void _cdecl PlayEffect(IAnimation * ani, const char * name, const char ** params, dword numParams);

	void _cdecl PlayLocEffect(IAnimation * ani, const char * name, const char ** params, dword numParams);

	void _cdecl ShowWeapon(IAnimation * ani, const char * name, const char ** params, dword numParams);

	void _cdecl ProhibitionRotation(IAnimation * ani, const char * name, const char ** params, dword numParams);

	void _cdecl AllowRotation(IAnimation * ani, const char * name, const char ** params, dword numParams);

	void _cdecl BloodStain(IAnimation * ani, const char * name, const char ** params, dword numParams);

	void _cdecl BeginSplashdamage(IAnimation * ani, const char * name, const char ** params, dword numParams);
	void _cdecl EndSplashdamage(IAnimation * ani, const char * name, const char ** params, dword numParams);

	//void _cdecl PatchExplosion(IAnimation * ani, const char * name, const char ** params, dword numParams);

	void _cdecl ZoomCamera(IAnimation * ani, const char * name, const char ** params, dword numParams);

	void _cdecl SplashDamage(IAnimation * ani, const char * name, const char ** params, dword numParams);

    void _cdecl SwordEffect(IAnimation * ani, const char * name, const char ** params, dword numParams);
	void _cdecl SwordStopEffect(IAnimation * ani, const char * name, const char ** params, dword numParams);

	void _cdecl AddRage(IAnimation * ani, const char * name, const char ** params, dword numParams);

	void _cdecl PairFailed(IAnimation * ani, const char * name, const char ** params, dword numParams);	

	void CreateParticles(const char * name, IGMXScene * scene, GMXHANDLE loc, bool isAttach,const Matrix& mat, float scale);
	void SetParticlesPlaySpeed(float speed);

	void SetTimeEvent(bool enable);

	bool IsEnableTimeEvent() { return (isEnableTimeEvent>0); };
protected:
	virtual void AddHandlers(IAnimation * anx);
	void UpdateTimeEventState(IAnimation * ani);

public:

	void _AddHandlers(IAnimation *anx) { AddHandlers(anx); }
	void _DelHandlers(IAnimation *anx);

protected:
	Character & chr;
	array<UpdateEvents> updateEvents;	
	dword isEnableTimeEvent;
};


#endif
