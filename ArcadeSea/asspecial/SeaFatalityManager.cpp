#include "SeaFatalityManager.h"

SeaFatalityManager::SeaFatalityManager(void) :
m_playerCam(NULL),
m_isFatality(false),
m_missionTime(NULL),
m_ships(__FILE__, __LINE__),
m_mode(Normal),
m_cameraTime(0.0f),
m_curCam(0xFFFFFFFF)
{
}

SeaFatalityManager::~SeaFatalityManager(void)
{
}

bool SeaFatalityManager::Create( MOPReader& reader )
{
	Params::CamParams cam;

	m_params.nearCameraModeRadius = reader.Float();
	cam.camera = reader.String();
	m_params.missionTime = reader.String();

	cam.workTime = reader.Float();
	cam.slomoInTime = reader.Float();
	cam.slomoTime = reader.Float();
	cam.slomoOutTime = reader.Float();
	cam.slomoFactor = reader.Float();

	m_params.farProbability = reader.Float();

	m_params.cams.Add(cam);
	unsigned int count = reader.Array();
	for (unsigned int i = 0; i < count; ++i)
	{
		cam.weight = reader.Float();
		cam.camera = reader.String();

		cam.workTime = reader.Float();
		cam.slomoInTime = reader.Float();
		cam.slomoTime = reader.Float();
		cam.slomoOutTime = reader.Float();
		cam.slomoFactor = reader.Float();

		cam.clearRadius = reader.Float();

		m_params.cams.Add(cam);
	}

	Activate(reader.Bool() && !EditMode_IsOn());
	if (!EditMode_IsOn())
		SetUpdate(&SeaFatalityManager::InitHandler, ML_LAST);
	return true;
}

void _cdecl SeaFatalityManager::InitHandler( float dTime, long level )
{
	MOSafePointer mo;
	FindObject(m_params.cams[0].camera, mo);
	if (mo.Ptr())
	{
		m_playerCam = mo.Ptr();

		m_ships.DelAll();
		MGIterator& gi = Mission().GroupIterator(MG_SHIP, __FILE__, __LINE__);
		while (!gi.IsDone())
		{
			if (gi.Get() != Mission().Player())
				m_ships.Add(gi.Get());
			gi.Next();
		}
		gi.Release();
	}

	FindObject(m_params.missionTime, mo);
	if (mo.Ptr())
		m_missionTime = mo.Ptr();

	DelUpdate(&SeaFatalityManager::InitHandler);
}

void SeaFatalityManager::DoFatality( float dTime )
{
	m_fatalityTime += dTime;
	// выход из фаталити
	if (m_fatalityTime >	m_params.cams[m_curCam].slomoInTime +
							m_params.cams[m_curCam].slomoTime +
							m_params.cams[m_curCam].slomoOutTime &&	m_mode == SlomoOut)
	{
		m_isFatality = false;
		m_mode = Normal;
		//m_playerCam->Command("SeaFatality End", 0, NULL);
		MOSafePointer cam;
		FindObject(m_params.cams[m_curCam].camera, cam);
		if(cam.Ptr())
			cam.Ptr()->Command("SeaFatality End", 0, NULL);
	}
	else
	// выход из сломо
	if (m_fatalityTime >	m_params.cams[m_curCam].slomoInTime +
							m_params.cams[m_curCam].slomoTime && m_mode == Slomo)
	{
		char buf[20];
		const char* params[2];

		crt_snprintf(buf, sizeof(buf), "%0.2f", m_params.cams[m_curCam].slomoOutTime);
		params[0] = "1.0";
		params[1] = buf;
		m_missionTime->Command("set", 2, params);

		m_mode = SlomoOut;
	}
	else
	// сломо
	if (m_fatalityTime >	m_params.cams[m_curCam].slomoInTime && m_mode == SlomoIn)
	{
		m_mode = Slomo;
	}
	// вход в сломо
	else
	if (m_mode == Normal)
	{
		char buf1[20];
		char buf2[20];
		const char* params[2];

		crt_snprintf(buf1, sizeof(buf1), "%0.2f", m_params.cams[m_curCam].slomoFactor);
		crt_snprintf(buf2, sizeof(buf2), "%0.2f", m_params.cams[m_curCam].slomoInTime);
		params[0] = buf1;
		params[1] = buf2;
		m_missionTime->Command("set", 2, params);

		m_mode = SlomoIn;
	}
}

void SeaFatalityManager::FindFatalityTarget()
{
	Matrix mtx1, mtx2;
	long nearCandidat = -1; // кандидат на показ ближнего фаталити
	long farCandidat = -1; // кандидат на показ дальнего фаталити
	float nearCandidatDist = 1e38f;
	float farCandidatDist = 1e38f;
	for (unsigned int i = 0; i < m_ships.Size(); ++i)
	{
		float dist = (m_ships[i].ship->GetMatrix(mtx1).pos - Mission().Player()->GetMatrix(mtx2).pos).GetLength();

		if ( m_ships[i].ship->IsShow() && m_ships[i].ship->IsDead() && !m_ships[i].isProcessed)
		{
			if ( dist < nearCandidatDist && dist <= m_params.nearCameraModeRadius )
			{
				nearCandidat = i;
				nearCandidatDist = dist;
			}

			if ( dist < farCandidatDist && dist > m_params.nearCameraModeRadius)
			{
				farCandidat = i;
				farCandidatDist = dist;
			}
		}
		if (!m_ships[i].ship->IsDead())
			m_ships[i].isProcessed = false;
	}

	if (nearCandidat != -1)
	{
		m_curCam = 0;
		m_isFatality = true;
		m_ships[nearCandidat].isProcessed = true;
		m_fatalityTime = 0.0f;
		m_cameraTime = m_params.cams[0].workTime;

		const char* param[1];
		param[0] = m_ships[nearCandidat].ship->GetObjectID().c_str();
		m_playerCam->Command("SeaFatality Start", 1, param);
		m_playerCam->Activate(true);
	}
	else
	if (farCandidat != -1 && 100.0f*rand()/(float)RAND_MAX < m_params.farProbability)
	{
		m_ships[farCandidat].isProcessed = true;

		float selector = rand()/(float)RAND_MAX;
		float curValue = 0.0f;
		for (unsigned int i = 1; i < m_params.cams.Size(); ++i)
		{
			if (selector >= curValue && selector < curValue + m_params.cams[i].weight)
			{
				MOSafePointer cam;
				FindObject(m_params.cams[i].camera, cam);

				// ищем - есть ли корабли в "чистой" зоне от цели
				Vector radius(m_params.cams[i].clearRadius);
				Vector shipPos = m_ships[farCandidat].ship->GetMatrix(mtx1).pos;
				dword count = 0;//FindObjects(MG_SHIP, shipPos-radius, shipPos+radius);
				bool zoneClear = true;
				/*for (unsigned int k = 0; k < count; ++k)
					if (GetFindObject(k)->GetMissionObject()->IsShow() &&
						!GetFindObject(k)->GetMissionObject()->IsDead())
					{
						zoneClear = false;
						break;
					}*/

				// установим выбранную камеру
				if (cam.Ptr() && zoneClear)
				{
					const char* param[1];
					param[0] = m_ships[farCandidat].ship->GetObjectID().c_str();
					cam.Ptr()->Command("SeaFatality Start", 1, param);

					m_playerCam->Activate(false);
					cam.Ptr()->Activate(true);

					m_curCam = i;
					m_isFatality = true;
					m_fatalityTime = 0.0f;
					m_cameraTime = m_params.cams[i].workTime;

					return;
				}
			}
			curValue += m_params.cams[i].weight;
		}
	}
}

void _cdecl SeaFatalityManager::Work( float dTime, long level )
{
	if (!m_playerCam || !Mission().Player() || !m_missionTime) return;

	m_cameraTime -= dTime;
	if (m_isFatality)
		DoFatality(dTime);
	else // если нет активных фаталити
	if (m_cameraTime <= 0.0f)
	{
		if ( 0xFFFFFFFF != m_curCam)
		{
			MOSafePointer mo;
			FindObject(m_params.cams[m_curCam].camera, mo);
			if (mo.Ptr())	mo.Ptr()->Activate(false);
			m_playerCam->Activate(true);
			m_curCam = 0xFFFFFFFF;
		}

		FindFatalityTarget();
	}
}

void SeaFatalityManager::Activate( bool isActive )
{
	MissionObject::Activate(isActive);
	DelUpdate(&SeaFatalityManager::Work);
	if (isActive)
		SetUpdate(&SeaFatalityManager::Work, ML_CAMERAMOVE_BASE);
}

const char Description[] = "Object for controlling fatality cameras.";
MOP_BEGINLISTCG(SeaFatalityManager, "Sea Fatality Manager", '1.00', 101, Description, "Arcade Sea");
	MOP_FLOAT("Near camera mode radius", 50.0f)
	MOP_STRING("Player camera", "")
	MOP_STRING("Mission time object", "")

	MOP_FLOAT("Near camera work time", 4.0f)
	MOP_FLOAT("Near camera slomo-in time", 1.0f)
	MOP_FLOAT("Near camera slomo time", 2.0f)
	MOP_FLOAT("Near camera slomo-out time", 1.0f)
	MOP_FLOAT("Near camera slomo factor", 0.5f)

	MOP_FLOATEX("Far camera probability", 50.0f, 0.0f, 100.f)
	MOP_ARRAYBEG("Far cameras", 0, 10)
		MOP_FLOATEX("Weight", 0.1f, 0.0f, 1.0f)
		MOP_STRING("Camera", "")

		MOP_FLOAT("Work time", 5.0f);
		MOP_FLOAT("Slomo-in time", 1.0f)
		MOP_FLOAT("Slomo time", 2.0f)
		MOP_FLOAT("Slomo-out time", 1.0f)
		MOP_FLOAT("Slomo factor", 0.5f)

		MOP_FLOAT("Clear radius", 100.0f)
	MOP_ARRAYEND

MOP_BOOL("Active", true)
MOP_ENDLIST(SeaFatalityManager)
