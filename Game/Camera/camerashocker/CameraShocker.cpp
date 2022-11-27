#include "CameraShocker.h"
#include "..\..\..\common_h\ICameraController.h"

CameraShocker::ShockTypes CameraShocker::shockTypes[] =	
							{	{ ConstString("shockRND"), CameraShocker::shockRND },
								{ ConstString("shockRND1"), CameraShocker::shockRND1 },
								{ ConstString("shockRND2"), CameraShocker::shockRND2 },
								{ ConstString("shockRND3"), CameraShocker::shockRND3 },
								{ ConstString("shockRND4"), CameraShocker::shockRND4 },
								{ ConstString("shockRND5"), CameraShocker::shockRND5 },
								{ ConstString("shockRND_H"), CameraShocker::shockRND_H },
								{ ConstString("shockRND_V"), CameraShocker::shockRND_V },
								{ ConstString("shockPolar1"), CameraShocker::shockPolar1 },
								{ ConstString("shockPolar2"), CameraShocker::shockPolar2 },
								{ ConstString("shockPolar3"), CameraShocker::shockPolar3 },
								{ ConstString("shockPolar4"), CameraShocker::shockPolar4 },
								{ ConstString("shockPolar5"), CameraShocker::shockPolar5 },
								{ ConstString("shockPolar"), CameraShocker::shockPolar },
								{ ConstString("shockPolar_V"), CameraShocker::shockPolar_V },
								{ ConstString("shockPolar_H"), CameraShocker::shockPolar_H },
								{ ConstString("resetShock"), CameraShocker::shockReset } };

CameraShocker::CameraShocker()
{
	strAmplitude[0] = 0;
	strTime[0] = 0;
	strIntensity[0] = 0;

	shockPosition = 0.0f;
}

CameraShocker::~CameraShocker()
{
}

bool CameraShocker::Create(MOPReader & reader)
{
	return EditMode_Update(reader);
}

bool CameraShocker::EditMode_Update(MOPReader & reader)
{
	ConstString type = reader.Enum();

	shockType = shockLast;
	for (int i=0; i<ARRSIZE(shockTypes); i++)
		if (shockTypes[i].name == type)
		{
			shockType = shockTypes[i].type;
			break;
		}

	ConstString fromEnum = reader.Enum();

	if (fromEnum.c_str()[0] == 'C' || fromEnum.c_str()[0] == 'c')
		fadingFrom = fromCamera;
	else if (fromEnum.c_str()[0] == 'M' || fromEnum.c_str()[0] == 'm')
		fadingFrom = fromMissionObject;

	moName = reader.String();

	useFading = reader.Bool();

	min_distance = reader.Float();
	min_amplitude = reader.Float();
	min_time = reader.Float();
	min_intensity = reader.Float();

	max_distance = reader.Float();
	max_amplitude = reader.Float();
	max_time = reader.Float();
	max_intensity = reader.Float();

	distanceDelta = 0.0f;
	if (max_distance - min_distance > 1e-4f)
		distanceDelta  = 1.0f / (max_distance - min_distance);

	if (!useFading)
		PrepareParams();

	shockParams[0] = strAmplitude;
	shockParams[1] = strTime;
	shockParams[2] = strIntensity;

	return true;
}

void CameraShocker::PostCreate()
{
	if (!FindObject(moName, moObject))
	{
		LogicDebugError("Can't find object \"%s\"", moName.c_str());
	}
}

void CameraShocker::SetMatrix(const Matrix & mtx)
{
	shockPosition = mtx.pos;
}

void CameraShocker::PrepareParams()
{
	float k = 0.0f;
	
	if (useFading)
	{
		Matrix mtx;
		Vector fromPos = 0.0f;
		switch (fadingFrom)
		{
			case fromCamera:
				fromPos = Render().GetView().GetCamPos();
			break;
			case fromMissionObject:
				if (moObject.Validate())
					fromPos = moObject.Ptr()->GetMatrix(mtx).pos;
				else
				{
				}
			break;
		}

		float distance = (shockPosition - fromPos).GetLength();
		distance = MinMax(min_distance, max_distance, distance);
		k = (distance - min_distance) * distanceDelta;
	}

	crt_snprintf(strAmplitude, sizeof(strAmplitude), "%5.3f", Lerp(min_amplitude, max_amplitude, k));
	crt_snprintf(strTime, sizeof(strTime), "%5.3f", Lerp(min_time, max_time, k));
	crt_snprintf(strIntensity, sizeof(strIntensity), "%5.3f", Lerp(min_intensity, max_intensity, k));
}

void CameraShocker::Command(const char * id, dword numParams, const char ** params)
{
}

void CameraShocker::Activate(bool isActive)
{
	if (!isActive)
		return;

	MOSafePointerType<ICameraController> CamController;
	static const ConstString objectId("Camera Controller");
	FindObject(objectId, CamController.GetSPObject());
	
	if (CamController.Ptr()) switch (shockType)
	{
		case shockRND1:
		case shockRND2:
		case shockRND3:
		case shockRND4:
		case shockRND5:
		case shockPolar1:
		case shockPolar2:
		case shockPolar3:
		case shockPolar4:
		case shockPolar5:
		case shockReset:
			CamController.Ptr()->Command(shockTypes[shockType].name.c_str(), 0, null);
		break;
		case shockRND:
		case shockRND_H:
		case shockRND_V:
		case shockPolar:
		case shockPolar_V:
		case shockPolar_H:
			if (useFading)
				PrepareParams();	
			CamController.Ptr()->Command(shockTypes[shockType].name.c_str(), 3, (const char **)&shockParams[0]);
		break;
	}
}

MOP_BEGINLISTG(CameraShocker, "Camera shocker", '1.00', 1, "Cameras")
	MOP_ENUMBEG("FadingDistanceFrom")
		MOP_ENUMELEMENT("Camera position")
		MOP_ENUMELEMENT("Mission object")
	MOP_ENUMEND

	MOP_ENUMBEG("ShockTypes")
		for (int i=0; i<ARRSIZE(CameraShocker::shockTypes); i++)
		{
			MOP_ENUMELEMENT(CameraShocker::shockTypes[i].name.c_str())
		}
	MOP_ENUMEND
	MOP_ENUM("ShockTypes", "Shock type")
	MOP_ENUM("FadingDistanceFrom", "Fading from")
	MOP_STRING("Mission object", "")
	MOP_BOOLC("Use fading", false, "true - enable fading by distance")
	MOP_GROUPBEG("Minimum distance")
		MOP_FLOAT("Distance |min|", 0.0f)	
		MOP_FLOAT("Amplitude |min|", 0.1f)	
		MOP_FLOAT("Time |min|", 0.2f)
		MOP_FLOAT("Intesity |min|", 40.0f)
	MOP_GROUPEND()
	MOP_GROUPBEG("Maximum distance")
		MOP_FLOAT("Distance |max|", 100.0f)	
		MOP_FLOAT("Amplitude |max|", 0.02f)
		MOP_FLOAT("Time |max|", 0.2f)
		MOP_FLOAT("Intesity |max|", 40.0f)
	MOP_GROUPEND()
MOP_ENDLIST(CameraShocker)
