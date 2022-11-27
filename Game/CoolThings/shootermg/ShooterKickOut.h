#ifndef _ShooterKickOut_
#define _ShooterKickOut_

#include "..\..\..\Common_h\IShooter.h"

class ShooterKickOut : public IShooterKickOut
{
public:
	//Конструктор - деструктор
	ShooterKickOut();
	virtual ~ShooterKickOut();

	//Создание объекта
	virtual bool Create(MOPReader & reader);
	virtual void PostCreate();

	//Обновление параметров
	virtual bool EditMode_Update(MOPReader & reader);	

	// активировать объект (попытка викинуть из шутера)
	virtual void Activate(bool isActive);

	// исполнение на кадре
	void _cdecl Work(float fDeltaTime, long level);

	// IShooterKickOut
	virtual void InShooter(IShooter* pShooter);
	virtual void OutShooter(IShooter* pShooter);
	virtual bool KickOut(const Vector& srcPos, const Vector& pos);
	virtual void KickOutPrepair();

protected:
	float m_fKickOutProbability;
	float m_fKickOutProbabilityForVisible;
	float m_fKickOutCooldown;
	float m_fTime;
	bool m_bPrepairToKickOut;
	MissionTrigger m_trigerKick;	// евент на то, что выкинуло из шутера

	IShooter* m_pActiveShooter;
};

#endif