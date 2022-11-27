
#include "WayTrack.h"

ShipWayTrack::ShipWayTrack() :
	m_aTrackData(_FL_)
{
	m_pAIParams = NULL;

	m_nCurWayNum = 0;
	m_pDefaultEnemy = NULL;
	m_fBehaviourTime = 0.f;

	m_wayPoint.pEnemy = NULL;
	m_wayPoint.bFollowToPoint = true;
	m_wayPoint.pos = 0.f;
	m_wayPoint.speedK = 0.f;
}

ShipWayTrack::~ShipWayTrack()
{
	m_pAIParams = NULL;
}

bool ShipWayTrack::InitWayTrack(ShipTrackAIParams* pAIParams)
{
	if( !pAIParams ) return false;

	m_pAIParams = pAIParams;

	// читаем трек
	m_aTrackData.DelAll();
	dword q = pAIParams->m_aWayTrack.Size();
	if( q > 0 )
	{
		m_aTrackData.AddElements( q );
		for( dword n=0; n<q; n++ )
		{
			m_aTrackData[n] = pAIParams->m_aWayTrack[n];
		}
	}
	// первое ускорение берем из первой точки
	if( m_aTrackData.Size()>0 )
		m_wayPoint.speedK = m_aTrackData[0].speedK;

	// враг по умолчанию
	MOSafePointer mo;
	if( pAIParams->FindObject( pAIParams->m_params.pcEnemyName, mo ) )
	{
		m_pDefaultEnemy = mo.Ptr();
		m_wayPoint.pEnemy = m_pDefaultEnemy;
	}

	return true;
}

void ShipWayTrack::ResetTrack()
{
	m_nCurWayNum = 0;
	m_fBehaviourTime = 0.f;
	m_wayPoint.pEnemy = m_pDefaultEnemy;
	FollowToWayPoint( 0 );
}

const ShipWayPoint& ShipWayTrack::GetWayPoint( float dltTime, const Matrix & mtx )
{
	// нет трека - возвращаем пустые данные
	if( m_aTrackData.Size() == 0 || !m_pAIParams )
	{
		m_wayPoint.bFollowToPoint = true;
		m_wayPoint.speedK = 0.f;
		m_wayPoint.pos = 0.f;
		return m_wayPoint;
	}

	if( m_nCurWayNum >= m_aTrackData )
		m_nCurWayNum = 0;

	//if( m_aTrackData[m_nCurWayNum].eBehaviourType == MOPShipWayPoint::Behaviour_follow )
	if( m_wayPoint.bFollowToPoint )
	{
		Matrix mTrackTransform(true);
		//m_wayPoint.speedK = 1.f;
		m_wayPoint.pos = m_pAIParams->GetMatrix(mTrackTransform).MulVertex(m_aTrackData[m_nCurWayNum].vPos);
		Vector vDist = mtx.pos - m_wayPoint.pos;
		float fDist2Pow = ~vDist;

		if( fDist2Pow < m_aTrackData[m_nCurWayNum].fDetectorRadius * m_aTrackData[m_nCurWayNum].fDetectorRadius )
		{
			// срабатываем тригер приплытия в точку
			if( m_pAIParams )
				m_aTrackData[m_nCurWayNum].triggerCome.Activate(m_pAIParams->Mission(), false);
			// смена врага
			if( string::IsEmpty(m_aTrackData[m_nCurWayNum].pcEnemyName) )
				m_wayPoint.pEnemy = m_pDefaultEnemy;
			m_wayPoint.speedK = m_aTrackData[m_nCurWayNum].speedK;
			if( !LaunchBehaviour(mtx) )
				FollowToWayPoint( m_aTrackData[m_nCurWayNum].nNextPoint );
		}
	}
	else
	{
		m_fBehaviourTime -= dltTime;
		if( m_fBehaviourTime <= 0.f )
		{
			// смена поведения на следование к другой точке
			FollowToWayPoint( m_aTrackData[m_nCurWayNum].nNextPoint );
		}
		else
		{
			if( m_aTrackData[m_nCurWayNum].eBehaviourType == MOPShipWayPoint::Behaviour_strafe )
			{
				ExecuteStrafe(dltTime, mtx);
			}
		}
	}

	return m_wayPoint;
}

void ShipWayTrack::ShowDebugInfo(IRender & render)
{
	if( m_aTrackData.Size() == 0 || !m_pAIParams ) return;
	Matrix mtx(true);
	if( m_pAIParams )
		m_pAIParams->GetMatrix(mtx);
	else
		mtx.SetIdentity();
	if( m_fBehaviourTime > 0.f && m_aTrackData[m_nCurWayNum].eBehaviourType == MOPShipWayPoint::Behaviour_strafe )
	{
		Vector pos = mtx.MulVertex(m_aTrackData[m_nCurWayNum].vPos + Vector(m_targ.x,0.f,m_targ.z));
		render.DrawSphere(pos,5.f,0xFF000000);
		render.DrawLine(pos,0xFFFF0000, pos+Vector(0.f,50.f,0.f), 0xFF990000);
	}
}

void ShipWayTrack::FollowToWayPoint(long n)
{
	if( n<0 )
		n = m_nCurWayNum + 1;

	// ищем индекс следующей точки на треке
	if( n >= m_aTrackData )
		if( m_pAIParams && m_pAIParams->m_bLoopTrack )
			n = 0;
		else
			n = m_aTrackData.Size() - 1;
	if( n < 0 ) n = 0;
	m_nCurWayNum = n;
	m_wayPoint.bFollowToPoint = true;
	// ставим точку к которой надо плыть
	Matrix mtx(true);
	if( m_aTrackData.Size() == 0 || !m_pAIParams )
		m_wayPoint.pos = 0.f;
	else
		m_wayPoint.pos = m_pAIParams->GetMatrix(mtx).MulVertex(m_aTrackData[n].vPos);
}

bool ShipWayTrack::LaunchBehaviour(const Matrix & mtx)
{
	// если поведение это следовать к точке, то выходим - ибо уже исполнено
	if( m_aTrackData[m_nCurWayNum].eBehaviourType == MOPShipWayPoint::Behaviour_follow )
		return false;

	// если время для специального режима не положительное, то выходим
	m_fBehaviourTime = m_aTrackData[m_nCurWayNum].fBehaviourTime;
	if( m_fBehaviourTime <= 0.f )
		return false;

	// ставим режим стрейфа
	if( m_aTrackData[m_nCurWayNum].eBehaviourType == MOPShipWayPoint::Behaviour_strafe )
	{
		// отключим следование к точке
		m_wayPoint.bFollowToPoint = false;
		// установим скоростные режимы для перемещения внутри зоны
		m_targ.rotSpeed = m_aTrackData[m_nCurWayNum].fStrafeRotSpeed;
		m_targ.maxAngle = m_aTrackData[m_nCurWayNum].fStrafeMaxAngle;
		m_targ.maxSpeed = m_aTrackData[m_nCurWayNum].fStrafeMaxSpeed;
		m_targ.basesin = sinf(m_aTrackData[m_nCurWayNum].fStrafeBaseAngle);
		m_targ.basecos = cosf(m_aTrackData[m_nCurWayNum].fStrafeBaseAngle);
		// выберем точку и угол в зоне, до которых будем стремиться
		SetStrafePoint(mtx);
		SetStrafeAng(mtx);
		return true;
	}

	return false;
}

void ShipWayTrack::ExecuteStrafe(float dltTime, const Matrix& mtx)
{
	Vector targPos = Vector(m_targ.x,0.f,m_targ.z);
	float targCos = m_targ.acos;
	float targSin = m_targ.asin;

	// матрица трека (при глобальном перемещении задаваемым другим объектом)
	Matrix mtxTrack(true);
	if( m_pAIParams )
		m_pAIParams->GetMatrix(mtxTrack);
	else
		mtxTrack.SetIdentity();

	// коррекция позиции при перемещении зоны
	if( m_aTrackData[m_nCurWayNum].vStrafePosOffset.x != 0.f ||
		m_aTrackData[m_nCurWayNum].vStrafePosOffset.z != 0.f )
	{
		// зона стрейфа сдвигается на заданное смещение в течении всего времени нахождения в режиме стрейфа
		float k = (m_aTrackData[m_nCurWayNum].fBehaviourTime - m_fBehaviourTime) / m_aTrackData[m_nCurWayNum].fBehaviourTime;
		targPos += Clampf(k) * m_aTrackData[m_nCurWayNum].vStrafePosOffset;
	}
	// коррекция угла при перемещении зоны
	if( m_aTrackData[m_nCurWayNum].fStrafeAngOffset != 0.f )
	{
		// зона стрейфа сдвигается на заданный угол в течении всего времени нахождения в режиме стрейфа
		float k = (m_aTrackData[m_nCurWayNum].fBehaviourTime - m_fBehaviourTime) / m_aTrackData[m_nCurWayNum].fBehaviourTime;

		float fs = sinf(m_aTrackData[m_nCurWayNum].fStrafeAngOffset * k);
		float fc = cosf(m_aTrackData[m_nCurWayNum].fStrafeAngOffset * k);

		targSin = m_targ.asin * fc + m_targ.acos * fs;
		targCos = m_targ.acos * fc - m_targ.asin * fs;
	}

	// позиция (с учетом внешней заданной транчформацией)
	Vector dir = mtxTrack.MulVertex(m_aTrackData[m_nCurWayNum].vPos + targPos);
	dir = mtx.MulVertexByInverse( dir );
	dir.y = 0.f;
	float dist = dir.NormalizeXZ();
	if( dist < 5.f )
		SetStrafePoint(mtx);
	else
	{
		// максимальная скорость на больших дистанциях больше в 10 раз
		float fMaxSpeed = dist<10.f ? m_targ.maxSpeed : (
						dist<50.f ? m_targ.maxSpeed * 10.f :
						m_targ.maxSpeed * 50.f);

		m_wayPoint.velocity = dir.z * fMaxSpeed;
		m_wayPoint.offset = dir.x * fMaxSpeed;
	}

	// направление корабля
	dir = Vector(mtx.vz.x, 0.f, mtx.vz.z);
	// учтем внешнюю трансформацию
	dir = mtxTrack.MulNormalByInverse(dir);
	dir.NormalizeXZ();
	// угол доворота
	float fcos = -dir.x * targCos - dir.z * targSin;
	float fsin = dir.z * targCos - dir.x * targSin;
	// если угол практически совпадает с заданным, то выбираем новый угол для доворота
	if( fcos > 0.99f )
		SetStrafeAng(mtx);
	else
	// иначе, продолжаем доворачиваться до нужного нам угла
	{
		// угол разворота на больших углах увеличиваем в 10 раз на остальных коэффициент другой
		float fMaxRotSpeed = fcos > 0.8f ? m_targ.rotSpeed * 0.1f : (
					fcos > 0.5f ? m_targ.rotSpeed * 0.5f : (
					fcos > 0.3f ? m_targ.rotSpeed :
					m_targ.rotSpeed * 10.f ));

		if( fsin < 0.f )
			m_wayPoint.rotate = -fMaxRotSpeed * (1.5f-fcos);
		else
			m_wayPoint.rotate = fMaxRotSpeed * (1.5f-fcos);
	}
}

void ShipWayTrack::SetStrafePoint(const Matrix& mtx)
{
	float z = FRAND( m_aTrackData[m_nCurWayNum].strafeZoneHeight ) - 0.5f * m_aTrackData[m_nCurWayNum].strafeZoneHeight;
	float x = FRAND( m_aTrackData[m_nCurWayNum].strafeZoneWidth ) - 0.5f * m_aTrackData[m_nCurWayNum].strafeZoneWidth;
	m_targ.x = x*m_targ.basecos + z*m_targ.basesin;
	m_targ.z = z*m_targ.basecos - x*m_targ.basesin;

	m_wayPoint.velocity = 0.f;
	m_wayPoint.offset = 0.f;
}

void ShipWayTrack::SetStrafeAng(const Matrix& mtx)
{
	float ang = FRAND( 2.f * m_targ.maxAngle ) - m_targ.maxAngle;
	float sa = sinf(ang);
	float ca = cosf(ang);
	m_targ.asin = m_targ.basesin * ca - m_targ.basecos * sa;
	m_targ.acos = m_targ.basecos * ca + m_targ.basesin * sa;
	m_wayPoint.rotate = 0.f;
}
