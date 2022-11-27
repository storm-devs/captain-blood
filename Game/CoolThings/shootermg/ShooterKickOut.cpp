#include "ShooterKickOut.h"

//Конструктор
ShooterKickOut::ShooterKickOut()
{
	m_pActiveShooter = NULL;
	m_fKickOutProbability = 1.f;
	m_fKickOutProbabilityForVisible = 1.f;
	m_fKickOutCooldown = 0.f;
	m_fTime = 0.f;
	m_bPrepairToKickOut = false;
}

//Деструктор
ShooterKickOut::~ShooterKickOut()
{
}


//Создание объекта
bool ShooterKickOut::Create(MOPReader & reader)
{
	return EditMode_Update(reader);
}

void ShooterKickOut::PostCreate()
{
	SetUpdate(&ShooterKickOut::Work, ML_TRIGGERS+10);
}

//Обновление параметров
bool ShooterKickOut::EditMode_Update(MOPReader & reader)
{
	m_fKickOutProbability = reader.Float() * 0.01f;
	m_fKickOutProbabilityForVisible = reader.Float() * 0.01f;
	m_fKickOutCooldown = reader.Float();
	m_fTime = 0.f;
	m_bPrepairToKickOut = false;
	m_trigerKick.Init(reader);
	return true;
}

void ShooterKickOut::Activate(bool isActive)
{
	MissionObject::Activate(isActive);
}

void _cdecl ShooterKickOut::Work(float fDeltaTime, long level)
{
	if( m_fTime < m_fKickOutCooldown )
		m_fTime += fDeltaTime;
}

void ShooterKickOut::InShooter(IShooter* pShooter)
{
	m_pActiveShooter = pShooter;
	m_fTime = 0.f;
}

void ShooterKickOut::OutShooter(IShooter* pShooter)
{
	if( m_pActiveShooter == pShooter )
	{
		m_bPrepairToKickOut = false;
		m_pActiveShooter = NULL;
	}
}

bool ShooterKickOut::KickOut(const Vector& srcPos, const Vector& pos)
{
	if( !m_pActiveShooter ) return true;

	// проверим на дальность
	Matrix mtx(true);
	m_pActiveShooter->GetMatrix(mtx);
	if( ~(pos - mtx.pos) > 50.f )
		return false;

	if( m_bPrepairToKickOut || m_fTime >= m_fKickOutCooldown )
	{
		bool bKick = true;
		if( !m_bPrepairToKickOut )
		{
			float fProbability = m_pActiveShooter->CheckDirection(srcPos) ? m_fKickOutProbabilityForVisible : m_fKickOutProbability;
			if( fProbability < 1.f )
				bKick = FRAND(1.f) < fProbability;
		}

		if( bKick )
		{
			m_pActiveShooter->Activate(false);
			m_trigerKick.Activate(Mission(), false);
		}

		m_bPrepairToKickOut = false;
	}
	return true;
}

void ShooterKickOut::KickOutPrepair()
{
	if( !m_pActiveShooter )
		m_bPrepairToKickOut = true;
}


MOP_BEGINLIST(ShooterKickOut, "ShooterKickOut", '1.00', 100)
	MOP_FLOATEXC("Probability", 100.0f, 0, 100.f, "Вероятность вышибания в процентах");
	MOP_FLOATEXC("ProbabilityVisible", 100.0f, 0, 100.f, "Вероятность вышибания в процентах для стрельбы с видимого корабля");
	MOP_FLOATC("Cooldown", 0.0f, "Время в течении которого не происходит вышибание");
	MOP_MISSIONTRIGGER("Kick event trigger")
MOP_ENDLIST(ShooterKickOut)
