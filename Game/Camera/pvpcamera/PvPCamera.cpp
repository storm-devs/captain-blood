
#include "PvPCamera.h"

__forceinline Vector MissionPvPCamera::CalculatePos(const Vector& lt, const Vector& rt, const Vector &lb, const Vector &rb, float u, float v)
{
	Vector t = lt + (rt-lt) * u;
	Vector b = lb + (rb-lb) * u;
	return t + (b-t) * v;
}

void MissionPvPCamera::DebugInfo::Reset()
{
	bApproach = bAway = false;
}

void MissionPvPCamera::Reset()
{
/*	m_fCamTargetSpeed = 0.f;
	targetcenter = 0.f;
	if( UpdateTargets() == 0 )
	{
		targetcenter = (m_nearLine[0] + m_nearLine[1]) * 0.5f;
	}
	realcenter = targetcenter;
	GetOffsetsFromPosition(m_vStartPos);
	m_vCurCamFrom = CalculatePos(m_farLine[0], m_farLine[1], m_nearLine[0], m_nearLine[1], m_fOffsetFactor, m_fApproachFactor);
	m_vRealCamFrom = m_vCurCamFrom;
	m_fCamCos = cosf(fov*0.45f);
	UpdateZoneNormal();

	m_distanceStep.fCurValue = 0.f;
	m_offsetStep.fCurValue = 0.f;
	m_vSpeedCamFrom = 0.f;
	m_fApproachPause = m_fApproachPauseTime;*/

	BaseCamera::Reset();
}

//Прочитать параметры идущие перед стандартными
bool MissionPvPCamera::CameraPreCreate(MOPReader & reader)
{
	m_vStartPos = reader.Position();
	m_farLine[0] = reader.Position();
	m_farLine[1] = reader.Position();
	m_nearLine[0] = reader.Position();
	m_nearLine[1] = reader.Position();
	for(long i = 0; i < max_players; i++)
	{
		targets[i].target.Reset();
		targets[i].targetID.Empty();
		targets[i].targetPos = 0.0f;
		targets[i].includeIntoCamera = false;
	}

	m_fNearestDistance = reader.Float();
	m_fMaxAngleCos = cosf( reader.Float() );

	float fAwayDistanceStep = reader.Float();
	float fApproachDistanceStep = reader.Float();
	float fOffsetStep = reader.Float();
	m_distanceStep.Init(-fAwayDistanceStep,fApproachDistanceStep);
	m_offsetStep.Init(-fOffsetStep,fOffsetStep);

	m_fCurDeltaTime = 0.f;
	m_vSpeedCamFrom = 0.f;
	m_fMaxCamSpeed = reader.Float();
	m_fCamTargetSpeedMax = reader.Float();

	targetsCount = reader.Array();
	for(long i = 0; i < targetsCount - 1; i++)
	{
		targets[i].targetID = reader.String();
		targets[i].targetPos = reader.Position();
	}

	m_fApproachPauseTime = 1.f;
	m_fApproachPause = m_fApproachPauseTime;

	m_fCam2ZoneLimit = 0.6f;

	return true;
}

//Прочитать параметры идущие после стандартных
bool MissionPvPCamera::CameraPostCreate(MOPReader & reader)
{
	targets[targetsCount - 1].targetID = targetID;
	targets[targetsCount - 1].targetPos = targetPos;

	m_fCamTargetSpeed = 0.f;
	//m_fCamTargetSpeedMax = 2.f;
	targetcenter = 0.f;
	if( UpdateTargets() == 0 )
	{
		targetcenter = (m_nearLine[0] + m_nearLine[1]) * 0.5f;
	}
	realcenter = targetcenter;

	//m_fApproachFactor = 0.f;
	//m_fOffsetFactor = 0.5f;
	//m_vCurCamFrom = m_farLine[0]+m_fOffsetFactor*(m_farLine[1]-m_farLine[0]);
	GetOffsetsFromPosition(m_vStartPos);
	m_vCurCamFrom = CalculatePos(m_farLine[0], m_farLine[1], m_nearLine[0], m_nearLine[1], m_fOffsetFactor, m_fApproachFactor);
	m_vRealCamFrom = m_vCurCamFrom;
	m_fCamCos = cosf(fov*0.45f);
	UpdateZoneNormal();

	return true;
}

// аптейт данных камеры на кадре
void MissionPvPCamera::WorkUpdate(float dltTime)
{
	m_fCurDeltaTime = dltTime;

	// отсчет паузы на приближение камеры
	if( m_fApproachPause < m_fApproachPauseTime )
		m_fApproachPause += dltTime;

	// обновить цели
	UpdateTargets();

	// подгоняем точку куда смотрит камера к центру целей
	if( m_fCamTargetSpeedMax > 0.f )
	{
		Vector vDir = targetcenter - realcenter;
		float fDist = vDir.Normalize();
		if( fDist > 0.001f )
		{
			// расстояние на которое можно подогнать
			float d = dltTime * m_fCamTargetSpeed;
			if( d<fDist )
			{
				realcenter += d * vDir;
				// увеличим скорость подгона (плавность движения)
				if( m_fCamTargetSpeed < m_fCamTargetSpeedMax )
				{
					m_fCamTargetSpeed += dltTime*m_fCamTargetSpeedMax;
					if( m_fCamTargetSpeed > m_fCamTargetSpeedMax )
						m_fCamTargetSpeed = m_fCamTargetSpeedMax;
				}
			}
			else
			{
				realcenter = targetcenter;
				// уменьшим скорость подгона (плавность движения)
				if( m_fCamTargetSpeed > 0.f )
				{
					m_fCamTargetSpeed -= dltTime*1.f;
					if( m_fCamTargetSpeed < 0.f )
						m_fCamTargetSpeed = 0.f;
				}
			}
		}
	}
	else
		realcenter = targetcenter;

	// подгон позиции камеры к оптимальной
	Vector vDiff = m_vCurCamFrom - m_vRealCamFrom;			// смещение позиции
	float fD = vDiff.Normalize();							// дистанция смещения
	float fMaxDist = m_fMaxCamSpeed * dltTime;				// максимальное смещение на этом кадре
	if( fD > fMaxDist ) fD = fMaxDist;						// дистанция смещения для этого кадра (ограниченная максимальным значением)
	float fK =  Clamp( dltTime * 30.f );					// коэффициент изменения скорости перемещения (0.f- не меняем скорость, 1.f- используем новое значение, в промежутке- используется бленд между старой и новой скоростью)
	m_vSpeedCamFrom += (vDiff*fD - m_vSpeedCamFrom) * fK;	// вектор скорости смещения
	m_vRealCamFrom += m_vSpeedCamFrom*dltTime;				// подгон позиции

	// уменьшаем шаг смещения сглаженных коэффициентов
	m_offsetStep.Decrease();
	m_distanceStep.Decrease();
}

// отрисовать инфу для редактора
void MissionPvPCamera::SelectedDraw(float dltTime)
{
	BaseCamera::SelectedDraw( dltTime );

	// рисуем зону камеры
	Vector v[4];
	v[0] = m_farLine[1];
	v[1] = m_farLine[0];
	v[2] = m_nearLine[0];
	v[3] = m_nearLine[1];
	Render().DrawPolygon( v, 4, 0x80808080 );

	if( !EditMode_IsOn() )
		DebugShowVerifyData();
}

// обновить таргеты камеры
long MissionPvPCamera::UpdateTargets()
{
	long count = 0;
	Vector vcentr = 0.f;
	for(long i = 0; i < targetsCount; i++)
	{
		Target & trg = targets[i];
		if(trg.targetID.NotEmpty())
		{
			trg.includeIntoCamera = false;
			// обновить(проверить) указатель на объект
			if(!trg.target.Validate())
			{
				FindObject(trg.targetID, trg.target);
			}
			// объект живой - значит используем его в списке активных целей
			if(trg.target.Validate())
			{
				if(trg.target.Ptr()->IsActive() && !trg.target.Ptr()->IsDie() )
				{
					trg.includeIntoCamera = true;
					// запомнить позицию этой цели
					Matrix mtx(true);
					trg.target.Ptr()->GetMatrix(mtx);
					trg.pos = mtx*trg.targetPos;
					// считать суммарную позицию всех используемых целей
					vcentr += trg.pos;
					count++;
				}
			}
		}
	}
	// геометрический центр позиций целей, является точкой куда надо смотреть камерой
	if( count > 0 )
		targetcenter = (1.f/(float)count) * vcentr;
	return count;
}

// обновить нормаль зоны (направление движения в зоне от дальней линии к ближней)
void MissionPvPCamera::UpdateZoneNormal()
{
	m_vZoneNorm = !((m_nearLine[0] - m_farLine[0])*(1.f-m_fOffsetFactor) + (m_nearLine[1] - m_farLine[1])*m_fOffsetFactor);
}

bool MissionPvPCamera::VerifyCamera()
{
	return (m_fCam2ZoneCos>=m_fMaxAngleCos && m_fNearDist>=m_fNearestDistance && m_fNearPlane>0.f && m_fCamCos<=m_fLeftCos && m_fCamCos<=m_fRightCos);
}

bool MissionPvPCamera::VerifyCamera09()
{
	return (m_fCam2ZoneCos>m_fMaxAngleCos+0.03f && m_fNearDist>=m_fNearestDistance*0.9f && m_fNearPlane>0.1f && m_fCamCos*1.1f<=m_fLeftCos && m_fCamCos*1.1f<=m_fRightCos);
}

void MissionPvPCamera::DebugShowVerifyData()
{
	Render().Print( 310.f, 35.f, 0xFFFFFFFF, "Verify camera - %s, Verify09 - %s", VerifyCamera()?"true":"false", VerifyCamera09()?"true":"false" );

	Render().Print( 310.f, 50.f, 0xFFFFFFFF, "CameraZoneCos >= x: %.3f", m_fCam2ZoneCos );
	Render().Print( 600.f, 50.f, m_fCam2ZoneCos>=m_fMaxAngleCos ? 0xFF00FF00 : 0xFFFF0000, "%.3f", m_fMaxAngleCos );
	Render().Print( 700.f, 50.f, m_fCam2ZoneCos>m_fMaxAngleCos+0.03f ? 0xFF00FF00 : 0xFFFF0000, "%.3f", m_fMaxAngleCos+0.03f );

	Render().Print( 310.f, 65.f, 0xFFFFFFFF, "NearDistance >= x: %.3f", m_fNearDist );
	Render().Print( 600.f, 65.f, m_fNearDist>=m_fNearestDistance ? 0xFF00FF00 : 0xFFFF0000, "%.3f", m_fNearestDistance );
	Render().Print( 700.f, 65.f, m_fNearDist>=m_fNearestDistance*0.9f ? 0xFF00FF00 : 0xFFFF0000, "%.3f", m_fNearestDistance*0.9f );

	Render().Print( 310.f, 80.f, 0xFFFFFFFF, "NearPlane > x: %.3f", m_fNearPlane );
	Render().Print( 600.f, 80.f, m_fNearPlane>0.f ? 0xFF00FF00 : 0xFFFF0000, "%.3f", 0.f );
	Render().Print( 700.f, 80.f, m_fNearPlane>0.1f ? 0xFF00FF00 : 0xFFFF0000, "%.3f", 0.1f );

	Render().Print( 310.f, 95.f, 0xFFFFFFFF, "LeftCos >= x: %.3f", m_fLeftCos );
	Render().Print( 600.f, 95.f, m_fCamCos<=m_fLeftCos ? 0xFF00FF00 : 0xFFFF0000, "%.3f", m_fCamCos );
	Render().Print( 700.f, 95.f, m_fCamCos*1.1f<=m_fLeftCos ? 0xFF00FF00 : 0xFFFF0000, "%.3f", m_fCamCos*1.1f );

	Render().Print( 310.f, 110.f, 0xFFFFFFFF, "RightCos >= x: %.3f", m_fRightCos );
	Render().Print( 600.f, 110.f, m_fCamCos<=m_fRightCos ? 0xFF00FF00 : 0xFFFF0000, "%.3f", m_fCamCos );
	Render().Print( 700.f, 110.f, m_fCamCos*1.1f<=m_fRightCos ? 0xFF00FF00 : 0xFFFF0000, "%.3f", m_fCamCos*1.1f );

	Render().Print(310.f, 130.f, 0xFFFFFFFF, "Pause time (current/max): %.3f / %3f", m_fApproachPause, m_fApproachPauseTime );

	Render().Print(310.f, 150.f, 0xFFFFFFFF, "Target position real/need: (%.2f,%.2f,%.2f) / (%.2f,%2.f,%.2f)",
		realcenter.x, realcenter.y, realcenter.z,	targetcenter.x, targetcenter.y, targetcenter.z );

	Render().Print(310.f, 170.f, 0xFFFFFFFF, "Camera position real/need: (%.2f,%.2f,%.2f) / (%.2f,%2.f,%.2f)",
		m_vRealCamFrom.x, m_vRealCamFrom.y, m_vRealCamFrom.z,	m_vCurCamFrom.x, m_vCurCamFrom.y, m_vCurCamFrom.z );

	Render().Print(310.f, 190.f, 0xFFFFFFFF, "Approach - %s, Away - %s", dbginf.bApproach?"true":"false", dbginf.bAway?"true":"false" );

	dbginf.Reset();
}

//Получить позицию камеры
void MissionPvPCamera::GetCameraPosition(Vector & position)
{
	// подсчитать косинусы между текущим направлением камеры и направлением до целей
	CalcTargsCos();

	// если есть выход за пределы камеры, то работаем по отъезжанию камеры
	if( !VerifyCamera() )
		WorkMoveAway();
	// если нет выхода за пределы камеры, то работаем по приближению
	else if( m_fApproachPause>=m_fApproachPauseTime && VerifyCamera09() )
		WorkApproach();

	// возвращаем позицию камеры
	position = m_vRealCamFrom;
}

// расчет параметров попадания целей в камеру
void MissionPvPCamera::CalcTargsCos()
{
	// вектор направления камеры
	Vector vCamDir = !(realcenter-m_vCurCamFrom);

	m_fLeftCos = m_fRightCos = 1.f;
	m_fNearDist = 1000.f;
	m_fNearPlane = 1000.f;
	for(long i = 0; i < targetsCount; i++)
	{
		Target & trg = targets[i];
		if(trg.includeIntoCamera && trg.target.Validate())
		{
			// направоение на цель
			Vector dirT = trg.pos - m_vCurCamFrom;
			// дистанция до цели
			float fDist = dirT | vCamDir;
			if( fDist < m_fNearDist )
				m_fNearDist = fDist;
			// дистанция до плоскости сечения зоны (плоскость проходящая через позицию камеры, с нормалью по направлению движения зоны от far к near линии)
			float fPlane = dirT | m_vZoneNorm;
			if( fPlane < m_fNearPlane )
				m_fNearPlane = fPlane;
			// косинус цели
			dirT.Normalize();
			float fCosT = vCamDir | dirT;
			// сторона поворта (по часовой / против часовой)
			if( dirT.z*vCamDir.x - dirT.x*vCamDir.z < 0 )
			{ // против часовой от направления камеры
				if( fCosT<m_fLeftCos )
					m_fLeftCos = fCosT;
			}
			else
			{ // по часовой от направления камеры
				if( fCosT<m_fRightCos )
					m_fRightCos = fCosT;
			}
		}
	}

	m_fCam2ZoneCos = vCamDir | m_vZoneNorm;

	// направление внутри зоны (x=sin,z=cos)
	//m_vGreedDir.x = vCamDir ^ m_vZoneNorm;
	//m_vGreedDir.z = m_fCam2ZoneCos;
}

// двигаем камеру вперед (положительное значение - вперед, отрицательное - назад)
bool MissionPvPCamera::MoveCamForward(float dist)
{
	// проверяем новую позицию на попадание в зону камеры
	float newApproach = m_fApproachFactor + dist;
	if( newApproach > 1.f ) newApproach = 1.f;
	if( newApproach < 0.f ) newApproach = 0.f;
	if( newApproach == m_fApproachFactor ) return false;

	// двигаем камеру
	m_fApproachFactor = newApproach;
	Vector vFar = m_farLine[0] + (m_farLine[1]-m_farLine[0])*m_fOffsetFactor;
	Vector vNear = m_nearLine[0] + (m_nearLine[1]-m_nearLine[0])*m_fOffsetFactor;
	m_vCurCamFrom = vFar + (vNear-vFar)*m_fApproachFactor;

	return true;
}

// двигаем камеру в сторону (положительное значение - вправо, отрицательное - влево)
bool MissionPvPCamera::MoveCamRight(float dist)
{
	// проверяем новую позицию на попадание в зону камеры
	float newOffset = m_fOffsetFactor + dist;
	if( newOffset > 1.f ) newOffset = 1.f;
	if( newOffset < 0.f ) newOffset = 0.f;
	if( newOffset == m_fOffsetFactor ) return false;

	// двигаем камеру
	m_fOffsetFactor = newOffset;
	Vector vFar = m_farLine[0] + (m_farLine[1]-m_farLine[0])*m_fOffsetFactor;
	Vector vNear = m_nearLine[0] + (m_nearLine[1]-m_nearLine[0])*m_fOffsetFactor;
	m_vCurCamFrom = vFar + (vNear-vFar)*m_fApproachFactor;

	UpdateZoneNormal();
	return true;
}

// удаляем камеру от целей (т.к. не можем все охватить)
void MissionPvPCamera::WorkMoveAway()
{
	dbginf.bAway = true;
	// цикл поиска нового положения камеры
	for(long n=0; n<1000; n++)
	{
		// смещаем камеру
		float fCosDelta = m_fLeftCos - m_fRightCos;
		if( fabs(fCosDelta) > 0.1f )
		{
			float fOffset = m_offsetStep.ChangeValue( fCosDelta>0.f ? m_offsetStep.fMinValue : m_offsetStep.fMaxValue );
			if( MoveCamRight(fOffset) )
			{
				CalcTargsCos();
				if( VerifyCamera() )
					break;
			}
		}
		// отодвигаем камеру
		if( MoveCamForward( m_distanceStep.ChangeValue(m_distanceStep.fMinValue) ) )
		{
			CalcTargsCos();
			if( VerifyCamera() )
				break;
		}
	}
	// запустить паузу на приближение камеры
	m_fApproachPause = 0.f;
}

// приближение камеры к целям (т.к. в запасе есть место для приближения)
void MissionPvPCamera::WorkApproach()
{
	dbginf.bApproach = true;
	Vector vOldCamFrom = m_vCurCamFrom;
	float fOldApproach = m_fApproachFactor;

	// пододвигаем камеру
	float fstep = m_fCurDeltaTime * (1.f/0.033f) * m_distanceStep.ChangeValue(m_distanceStep.fMaxValue);
	if( MoveCamForward( fstep ) )
	{
		CalcTargsCos();
		if( VerifyCamera09() )
		{
			UpdateZoneNormal();
			return;
		}

		// откат назад? слишком большое смещение
		m_vCurCamFrom = vOldCamFrom;
		m_fApproachFactor = fOldApproach;
		CalcTargsCos();
	}

	float fOldOffset = m_fOffsetFactor;

	// смещаем камеру
	float fOffset = 0.f;
	float fCosDelta = m_fLeftCos - m_fRightCos;
	if( fabs(fCosDelta) > 0.1f && m_fCam2ZoneCos > m_fCam2ZoneLimit )
	{
		fOffset = m_fCurDeltaTime * (1.f/0.033f) * m_offsetStep.ChangeValue( fCosDelta>0.f ? m_offsetStep.fMinValue : m_offsetStep.fMaxValue );
	}

	if( m_fCam2ZoneCos <= m_fCam2ZoneLimit )
	{
		Vector vCamDir = !(realcenter-m_vCurCamFrom);
		// сторона поворта (по часовой / против часовой)
		if( vCamDir.z*m_vZoneNorm.x - vCamDir.x*m_vZoneNorm.z < 0 )
		{ // против часовой от направления нормали
			fOffset = m_fCurDeltaTime * (1.f/0.033f) * m_offsetStep.ChangeValue( m_offsetStep.fMinValue );
		}
		else
		{ // по часовой от направления камеры
			fOffset = m_fCurDeltaTime * (1.f/0.033f) * m_offsetStep.ChangeValue( m_offsetStep.fMaxValue );
		}
	}
/*	if( m_fCam2ZoneCos <= m_fCam2ZoneLimit )
	{
		fOffset = if( m_fLeftCos
		m_fCam2ZoneCos +
		m_fCam2ZoneLimit
	}*/

	if( fOffset != 0.f && MoveCamRight(fOffset) )
	{
		CalcTargsCos();
		if( VerifyCamera09() )
		{
			UpdateZoneNormal();
			return;
		}

		// откат назад? слишком большое смещение
		m_vCurCamFrom = vOldCamFrom;
		m_fOffsetFactor = fOldOffset;
	}
}

//Получить точку наблюдения камеры
bool MissionPvPCamera::GetCameraTarget(const Vector & position, Vector & target_pos, Vector & up)
{
	target_pos = realcenter;
	return true;
}

//Инициализация параметров
void MissionPvPCamera::InitParams()
{
	for(long i = 0; i < max_players; i++)
	{
		targets[i].target.Reset();
	}	
}

//Найти косинус угола охватывающий цели
float MissionPvPCamera::CalcCosFoV(const Vector & from, const Vector & to)
{
	float curCos = 1.0f;
	Vector dirC = !(to - from);
	for(long i = 0; i < targetsCount; i++)
	{
		Target & trg = targets[i];
		if(trg.target.Validate())
		{
			Vector dirT = !(trg.pos - from);
			float cosCT = dirC | dirT;
			if(cosCT < curCos)
			{
				curCos = cosCT;
			}
		}
	}
	return curCos;
}

void MissionPvPCamera::GetOffsetsFromPosition(const Vector & pos)
{
	float u = 0.5f;
	float v = 0.5f;
	float ftmp;

	for( long j=0; j<10000; j++ )
	{
		Vector c = CalculatePos(m_farLine[0], m_farLine[1], m_nearLine[0], m_nearLine[1], u, v);
		float fcurdist = ~(c - pos);
		if( fcurdist < 0.01f )
			break;

		// check left
		if( u>0.f )
		{
			float l = u - 0.025f;
			if( l < 0.f ) l = 0.f;
			c = CalculatePos(m_farLine[0], m_farLine[1], m_nearLine[0], m_nearLine[1], l, v);
			ftmp = ~(c-pos);
			if( ftmp < fcurdist ) {
				u = l;
				continue;
			}
		}

		// check right
		if( u<1.f )
		{
			float r = u + 0.025f;
			if( r > 1.f ) r = 1.f;
			c = CalculatePos(m_farLine[0], m_farLine[1], m_nearLine[0], m_nearLine[1], r, v);
			ftmp = ~(c-pos);
			if( ftmp < fcurdist ) {
				u = r;
				continue;
			}
		}

		// check top
		if( v>0.f )
		{
			float t = v-0.025f;
			if( t<0.f ) t=0.f;
			c = CalculatePos(m_farLine[0], m_farLine[1], m_nearLine[0], m_nearLine[1], u, t);
			ftmp = ~(c-pos);
			if( ftmp < fcurdist ) {
				v = t;
				continue;
			}
		}

		// check bottom
		if( v<1.f )
		{
			float b = v+0.025f;
			if( b>1.f) b=1.f;
			c = CalculatePos(m_farLine[0], m_farLine[1], m_nearLine[0], m_nearLine[1], u, b);
			ftmp = ~(c-pos);
			if( ftmp < fcurdist ) {
				v = b;
				continue;
			}
		}

		break;
	}

	m_fOffsetFactor = u;
	m_fApproachFactor = v;
}


MOP_BEGINLISTCG(MissionPvPCamera, "PvP camera", '1.00', 0, CAMERA_COMMENT("Camera for PvP game mode\n\n"), "Cameras")
	MOP_POSITIONC("Start position", Vector(0.0f, 0.0f, 0.0f), "Position in start mission moment")
	MOP_POSITIONC("Point far left", Vector(0.0f), "Position of camera zone which left and far from targets")
	MOP_POSITIONC("Point far right", Vector(0.0f), "Position of camera zone which right and far from targets")
	MOP_POSITIONC("Point near left", Vector(0.0f), "Position of camera zone which left and near to targets")
	MOP_POSITIONC("Point near right", Vector(0.0f), "Position of camera zone which right and near to targets")
	MOP_FLOATEXC("Nearest distance", 8.f, 3.f, 1000.f, "Minimal allowed distance to target")
	MOP_FLOATEXC("Maximal angle", 45.f, 0.f, 180.f, "Maximal allowed angle from camera to zone normal")
	MOP_FLOATEXC("Move away distance step", 0.01f, 0.001f, 1.f, "Fore|back step for camera move away (realative full camera zone)")
	MOP_FLOATEXC("Approach distance step", 0.01f, 0.001f, 1.f, "Fore|back step for camera approach (realative full camera zone)")
	MOP_FLOATEXC("Move offset step", 0.01f, 0.001f, 1.f, "Right|left step for camera move (realative full camera zone)")
	MOP_FLOATC("Max camera speed", 100.f, "Maximal camera movement speed (m/sec)")
	MOP_FLOATEXC("Max camera target speed", 2.f, 0.f, 100.f, "Maximal speed for target of camera (if 0.0 then no latence)")
	MOP_ARRAYBEG("Targets", 1, MissionPvPCamera::max_players)
		MOP_CAM_TARGET
	MOP_ARRAYEND
	MOP_CAM_FOV
	MOP_CAM_BLENDER
	MOP_CAM_LEVEL
	MOP_CAM_ACTIVATE
	MOP_CAM_PREVIEW
	MOP_CAM_DRAWCAMERA
MOP_ENDLIST(MissionPvPCamera)
