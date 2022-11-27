
#include "RotatedCamera.h"

//============================================================================================
//MissionRotatedCamera
//============================================================================================

void MissionRotatedCamera::InitParams()
{
	aim_target.Reset();

	bOneObjectattach = false;

	aim_targetID.Empty();

	BaseCamera::InitParams();
}

//Прочитать параметры идущие после стандартных
bool MissionRotatedCamera::CameraPreCreate(MOPReader & reader)
{
	aim_target.Reset();
	fRotSpeedH = 0.0f;
	fRotSpeedV = 0.0f;		

	ang = reader.Angles();
	dir = Matrix(ang).vz;
	dist = reader.Float();
	dir = dir*dist;

	fMaxRotSpeedH = reader.Float();
	fRotAccelerateH = reader.Float();

	fMinRotAngleV = reader.Float() * PI/180.0f;
	fMaxRotAngleV = reader.Float() * PI/180.0f;

	int num_targets = reader.Array();

	LockTargets.DelAll();

	for (int i=0;i<num_targets;i++)
	{
		TLockTarget locktrg;

		locktrg.name=reader.String();

		LockTargets.Add(locktrg);
	}
	
	aim_targetID.Empty();

	autoturn_time = reader.Float();

	fMaxRotSpeedV = fMaxRotSpeedH;
	fRotAccelerateV = fRotAccelerateH;

	trg_dist = dist;
	trg_fMinRotAngleV = fMinRotAngleV;
	trg_fMaxRotAngleV = fMaxRotAngleV;

	return true;
}

//Получить позицию камеры
void MissionRotatedCamera::GetCameraPosition(Vector & position)
{
	if(!BaseCamera::GetCameraTarget(Vector(0.0f), position, Vector(0.0f, 1.0f, 0.0f)))
	{
		position = 0.0f;
		return;
	}
	position -= dir;
}


void MissionRotatedCamera::ChangeValue(float &value, float targetValue, float dltTime, float accel)
{
	if (targetValue>value)
	{
		value += dltTime * accel;

		if (targetValue<value)
		{
			value = targetValue;
		}
	}
	else
	{
		value -= dltTime * accel;

		if (targetValue>value)
		{
			value = targetValue;
		}
	}

}

void MissionRotatedCamera::ValidateMO(MOSafePointer & mo, const ConstString & name)
{
	if(name.NotEmpty())
	{
		if(!mo.Validate())
		{
			FindObject(name, mo);
		}
	}

}

bool MissionRotatedCamera::CheckMOCondition(MOSafePointer & mo)
{
	if (!mo.Validate()) return false;

	return (mo.Ptr()->IsActive() && mo.Ptr()->IsShow());
}

bool MissionRotatedCamera::CheckLockTargetList()
{
	for (int i = 0; i<(int)LockTargets.Size();i++)
	{
		ValidateMO(LockTargets[i].mo, LockTargets[i].name);
		if (CheckMOCondition(LockTargets[i].mo))
		{
			return true;

		}
	}

	return false;
}

void MissionRotatedCamera::FindFarLeft()
{
	for (int i = 0; i<(int)LockTargets.Size();i++)
	{
		if (LockTargets[i].ay>PI*0.5f && CheckMOCondition(LockTargets[i].mo))
		{
			locktrg_index=i;

			SetLockTarget(LockTargets[locktrg_index].name);

			return;
		}
	}

	locktrg_index=0;

	FindNextLockTarget(true);
}

void MissionRotatedCamera::FindFarRight()
{
	for (int i = (int)LockTargets.Size()-1; i>=0;i--)
	{
		if (LockTargets[i].ay<PI*3.0f/2.0f && CheckMOCondition(LockTargets[i].mo))
		{
			locktrg_index=i;

			SetLockTarget(LockTargets[locktrg_index].name);

			return;
		}
	}

	locktrg_index=LockTargets.Size()-1;

	FindNextLockTarget(false);
}

void MissionRotatedCamera::SortLockTargetList()
{
	Vector pos = 0.0f;	

	BaseCamera::GetCameraTarget(Vector(0.0f), pos, Vector(0.0f, 1.0f, 0.0f));	

	Matrix mat = GetMatrix(Matrix());

	for (int i = 0; i<(int)LockTargets.Size();i++)
	{
		ValidateMO(LockTargets[i].mo, LockTargets[i].name);

		if (LockTargets[i].mo.Validate())
		{
			Matrix mat_mo;
			LockTargets[i].mo.Ptr()->GetMatrix(mat_mo);
			
			Vector mo_pos = mat_mo.pos - pos;
			
			mo_pos.y = 0.0f;

			Vector up =-mat.vz;

			LockTargets[i].ay = up.GetAngleXZ(mo_pos); 

			if (LockTargets[i].ay<0) LockTargets[i].ay=2*PI + LockTargets[i].ay;

			LockTargets[i].dist = (pos-mo_pos).GetLength(); 
		}				
	}

	for (int i = 0; i<(int)LockTargets.Size()-1;i++)
	{
		for (int j = i+1; j<(int)LockTargets.Size();j++)
		{
			if (LockTargets[i].ay>LockTargets[j].ay)
			{
				TLockTarget tmp;
				tmp = LockTargets[j];
				LockTargets[j] = LockTargets[i];
				LockTargets[i] = tmp;

				if (i==locktrg_index)
				{
					locktrg_index = j;
				}
				else
				if (j==locktrg_index)
				{
					locktrg_index = i;
				}
			}
		}
	}

}

void MissionRotatedCamera::NextLockIndex(bool forward)
{
	if (forward)
	{
		locktrg_index++;

		if (locktrg_index>=(int)LockTargets.Size())
		{
			locktrg_index=0;
		}
	}
	else
	{
		locktrg_index--;

		if (locktrg_index<0)
		{
			locktrg_index=(int)LockTargets.Size()-1;
		}
	}
}

void MissionRotatedCamera::FindNextLockTarget(bool forward)
{
	NextLockIndex(forward);	

	while (!CheckMOCondition(LockTargets[locktrg_index].mo))
	{
		NextLockIndex(forward);
	}				

	SetLockTarget(LockTargets[locktrg_index].name);	

	CalcAutoTurnSpeed();
}

void MissionRotatedCamera::WorkUpdate(float dltTime)
{
	ChangeValue(dist, trg_dist, dltTime, 22.0f);
	ChangeValue(fMinRotAngleV, trg_fMinRotAngleV, dltTime, 45.0f * PI/180.0f);
	ChangeValue(fMaxRotAngleV, trg_fMaxRotAngleV, dltTime, 45.0f * PI/180.0f);

	if (!LockTargets.IsEmpty() && CheckLockTargetList() && !bOneObjectattach)
	{
		if (aim_targetID.NotEmpty())
		{
			if (Controls().GetControlStateFloat("MissionRotatedCamera_UnLockTarget")>0.7f)
			{
				UnlockTarget();			
			}
		}
			
		if (Controls().GetControlStateType("MissionRotatedCamera_PrevTarget") == CST_ACTIVATED)
		{
			if (aim_targetID.IsEmpty())
			{
				FindFarLeft();
			}
			else
			{
				FindNextLockTarget(false);
			}
		}

		if (Controls().GetControlStateType("MissionRotatedCamera_NextTarget") == CST_ACTIVATED)
		{
			if (aim_targetID.IsEmpty())
			{
				FindFarRight();
			}
			else
			{
				FindNextLockTarget(true);
			}
		}
	}
			
	ValidateMO(aim_target, aim_targetID);

	float fCurRotSpeed;


	if (!CheckMOCondition(aim_target) ||
		fabs(Controls().GetControlStateFloat("MissionRotatedCamera_Turn_H"))>0.25f ||
		fabs(Controls().GetControlStateFloat("MissionRotatedCamera_Turn_V"))>0.25f)
	{
		UnlockTarget();
	}


	if (aim_target.Validate())
	{
		Matrix mat;
		aim_target.Ptr()->GetMatrix(mat);

		Vector pos = 0.0f;

		BaseCamera::GetCameraTarget(Vector(0.0f), pos, Vector(0.0f, 1.0f, 0.0f));			
			
		new_ay = (mat.pos-pos).GetAY();
						
		Vector new_ang = ang;

		new_ang.y = new_ay;

		Matrix mat1 = Matrix(ang);
		Quaternion q1(mat1);
			
		Matrix mat2 = Matrix(new_ang);
		Quaternion q2(mat2);

		float k = 0.0f;

		Vector upvector(cos(ang.y),0.0f,sin(ang.y));
		Vector cur_upvector(cos(new_ay),0.0f,sin(new_ay));
			
		k = fabs(upvector.GetAngleXZ(cur_upvector));

		if (k>0.01f)
		{
			k = Clampf(autoturn_spd/k);
		}			

		q1.SLerp(q1,q2,k * dltTime);			

		q1.GetMatrix(mat2);

		mat2.GetAngles(ang);
	}
	else
	{
		fCurRotSpeed = Controls().GetControlStateFloat("MissionRotatedCamera_Turn_H") * fMaxRotSpeedV;

		ChangeValue(fRotSpeedH, fCurRotSpeed, dltTime, fRotAccelerateH);

		ang.y += dltTime * fRotSpeedH;

		bOneObjectattach = false;
	}	

	fCurRotSpeed = Controls().GetControlStateFloat("MissionRotatedCamera_Turn_V") * fMaxRotSpeedV;

	ChangeValue(fRotSpeedV, fCurRotSpeed, dltTime, fRotAccelerateV);

	ang.x -= dltTime * fRotSpeedV;		

	if (fMinRotAngleV>ang.x)
	{
		ang.x = fMinRotAngleV;
		fRotSpeedV = 0.0f;
	}

	if (fMaxRotAngleV<ang.x)
	{
		ang.x = fMaxRotAngleV;
		fRotSpeedV = 0.0f;
	}

	dir = Matrix(ang).vz;		
	dir = dir*dist;

	SortLockTargetList();

	/*for (int i = 0; i<(int)LockTargets.Size();i++)
	{
		if (LockTargets[i].mo)
		{
			Matrix mat_mo;
			LockTargets[i].mo->GetMatrix(mat_mo);

			Render().Print(mat_mo.pos,50,1,0xff00ffff,"%4.3f %i",LockTargets[i].ay/(PI/180.0f),i);
		}
	}

	Render().Print(10,10,0xff00ffff,"%i %s",locktrg_index, aim_targetID.c_str());*/
}

//Обработчик команд для объекта
void MissionRotatedCamera::Command(const char * id, dword numParams, const char ** params)
{
	if(!id) return;

	if(string::IsEqual(id, "ChangeDist"))
	{
		if(numParams >= 1)
		{
			char * tmp = null;
			float new_dist = (float)strtod(params[0], &tmp);			
			
			LogicDebug("ChangeDist: newdistance = %f", new_dist);			

			trg_dist = new_dist;
		}
		else
		{
			LogicDebugError("ChangeDist: invalidate params");
		}
	}
	else
	if(string::IsEqual(id, "ChangeMinVAngle"))
	{
		if(numParams >= 1)
		{
			char * tmp = null;
			float angle = (float)strtod(params[0], &tmp);			

			LogicDebug("ChangeMinVAngle: newMinVAngle = %f", angle);

			trg_fMinRotAngleV = angle * PI/180.0f;
		}
		else
		{
			LogicDebugError("ChangeMinVAngle: invalidate params");
		}
	}
	else
	if(string::IsEqual(id, "ChangeMaxVAngle"))
	{
		if(numParams >= 1)
		{
			char * tmp = null;
			float angle = (float)strtod(params[0], &tmp);			

			LogicDebug("ChangeMaxVAngle: newMaxVAngle = %f", angle);

			trg_fMaxRotAngleV = angle * PI/180.0f;
		}
		else
		{
			LogicDebugError("ChangeMaxVAngle: invalidate params");
		}
	}
	else
	if(string::IsEqual(id, "LockTarget") || string::IsEqual(id, "SeaFatality Start"))
	{
		if(numParams > 0)
		{
			SetLockTarget(ConstString(params[0]));
			
			LogicDebug("LockTarget: new target is \"%s\"", params[0]);

			bOneObjectattach = true;

			CalcAutoTurnSpeed();
		}
		else
		{
			UnlockTarget();

			LogicDebug("LockTarget: new target is \"\"");
		}
	}
	else
	if(string::IsEqual(id, "UnLockTarget") || string::IsEqual(id, "SeaFatality End"))
	{
		UnlockTarget();

		LogicDebug("Camera is UnLocked");
	}

	BaseCamera::Command(id, numParams, params);
}

bool MissionRotatedCamera::CheckCommand(const char * id)
{
	if(!id) return false;

	if(string::IsEqual(id, "ChangeDist"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "ChangeMinVAngle"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "ChangeMaxVAngle"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "LockTarget"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "UnLockTarget"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "SeaFatality Start"))
	{
		return true;
	}
	else
	if(string::IsEqual(id, "SeaFatality End"))
	{
		return true;
	}

	return BaseCamera::CheckCommand(id);
}

void MissionRotatedCamera::CalcAutoTurnSpeed()
{
	if (!aim_target.Validate())
	{
		autoturn_spd = 1.0f;
		return;
	}

	Vector pos = 0.0f;
	BaseCamera::GetCameraTarget(Vector(0.0f), pos, Vector(0.0f, 1.0f, 0.0f));				

	float new_ay = (aim_target.Ptr()->GetMatrix(Matrix()).pos-pos).GetAY();

	Vector upvector(cos(ang.y),0.0f,sin(ang.y));
	Vector cur_upvector(cos(new_ay),0.0f,sin(new_ay));

	float delta_ang = upvector.GetAngleXZ(cur_upvector);
	autoturn_spd = fabs(delta_ang) / autoturn_time;
}

inline void MissionRotatedCamera::SetLockTarget(const ConstString & target)
{
	ShowAim(false);

	aim_targetID = target;
	aim_target.Reset();

	ValidateMO(aim_target, aim_targetID);

	ShowAim(true);
}

inline void MissionRotatedCamera::UnlockTarget()
{
	ShowAim(false);

	aim_targetID.Empty();
	aim_target.Reset();
}

inline void MissionRotatedCamera::ShowAim(bool show)
{
	if (aim_target.Validate())
	{
		if (show)
		{
			aim_target.Ptr()->Command("show aim",0,NULL);
		}
		else
		{
			aim_target.Ptr()->Command("hide aim",0,NULL);
		}
	}
}

MOP_BEGINLISTCG(MissionRotatedCamera, "Rotated camera", '1.00', 0, CAMERA_COMMENT("Rotated camera\n\n"), "Cameras")
	MOP_ANGLESC("Angles", Vector(0.6f, 0.0f, 0.0f), "Rotated camera's start orientation angles")
	MOP_FLOATEXC("Distance", 3.0f, 0.5f, 195.0f, "Distance to target point in meters")
	MOP_FLOATEXC("MaxRotSpeed", 2.5f, 0.5f, 10.0f, "Maximum camera's rotation speed")
	MOP_FLOATEXC("RotAccelerate", 4.0f, 0.5f, 10.0f, "Camera's rotation acceleration")
	MOP_FLOATEXC("MinVertAngle", 10.0f, -89.0f, 89.0f, "Minimal camera's vertical angle")
	MOP_FLOATEXC("MaxVertAngle", 45.0f, -89.0f, 89.0f, "Maximum camera's vertical angle")	
	
	MOP_ARRAYBEG("Lock Target List", 0, 200)
		MOP_STRING("LockTarget", "")
	MOP_ARRAYEND

	MOP_FLOATEXC("AutoTurnSpeed", 1.5f, 0.1f, 20.0f, "AutoTurnSpeed")
	MOP_CAM_STD
MOP_ENDLIST(MissionRotatedCamera)

