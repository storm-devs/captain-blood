
#include "AutoZoom.h"

AutoZoom::AutoZoom()
{
	fLastFov = -1.0f;
	camcontroller = null;
	lerp_speed = 6.0;
	ang_speed = 1.0;
}

bool AutoZoom::Create(MOPReader & reader)
{
//	SetUpdate(&AutoZoom::Work, ML_DYNAMIC1);

	return true;
}

void AutoZoom::Activate(bool isActive)
{	
	if (isActive == IsActive()) return;

	if (isActive)	
	{			
		if (!camcontroller)
		{
			MOSafePointerTypeEx<ICameraController> controller;
			static const ConstString objectId("Camera Controller");
			static const ConstString typeId("CameraController");
			controller.FindObject(&Mission(),objectId,typeId);

			camcontroller = controller.Ptr();
		}

		const Matrix & prj =  Render().GetProjection();
		//lst_fov = fabs(atanf(1.0f/prj.m[0][0])*2.0f);	
		//lst_aspect = fabs(atanf(1.0f / prj.m[1][1]) * 2.0f) / lst_fov;
		//lst_fov /= Render().GetWideScreenAspectFovMultipler();	

		float fovFromProjMatrix = float(fabs(atan(1.0f/prj.m[0][0])*2.0));	
		float fWidth = (float)Render().GetViewport().Width;
		float fHeight = (float)Render().GetViewport().Height;
		lst_fov = invRecalculateFov(fWidth, fHeight, Render().GetWideScreenAspectWidthMultipler(), fovFromProjMatrix);
		lst_aspect = 1.0f;


		MissionObject* player = Mission().Player();
		if (player)
		{
			Matrix pl_mat;
			player->GetMatrix(pl_mat).pos;	
			last_pl_pos = pl_mat.pos;	
		}

		SetUpdate(&AutoZoom::Work, ML_CAMERAMOVE_ANI - 1);		
	}
	else
	{
		lst_fov = fLastFov;	
	}

	MissionObject::Activate(isActive);
}

void _cdecl AutoZoom::Work(float dltTime, long level)
{
	if (!camcontroller) return;

	if (!camcontroller->AllowZooming()) return;

	float fNeededDist=2.5f;
	float fMinAngle=10*PI/180;
	float fMaxAngle=25*PI/180;

	float fMinSpeedAngle=80*PI/180;
	float fMaxSpeedAngle=120*PI/180;


	const Matrix & view = Render().GetView();
	const Matrix & prj =  Render().GetProjection();

	//float actFOV = fabs(atanf(1.0f/prj.m[0][0])*2.0f);	
	float fovFromProjMatrix = float(fabs(atan(1.0f/prj.m[0][0])*2.0));	
	float fWidth = (float)Render().GetViewport().Width;
	float fHeight = (float)Render().GetViewport().Height;
	float actFOV = invRecalculateFov(fWidth, fHeight, Render().GetWideScreenAspectWidthMultipler(), fovFromProjMatrix);

	
	//actFOV /= Render().GetWideScreenAspectFovMultipler();			
				
	float fNeededFOV = actFOV;
		
	float fCurFOV = actFOV;

	MissionObject* player = Mission().Player();
	Matrix pl_mat;
	if (player) player->GetMatrix(pl_mat);

	last_pl_pos.Lerp(last_pl_pos, pl_mat.pos, Clampf(dltTime*lerp_speed));

	if (!player->IsShow() || !player->IsActive()) return;

	if (IsActive())
	{	
		if (fLastFov>-0.9) fCurFOV = fLastFov;
				
		if (player)
		{							
			fNeededFOV = fNeededDist / (view.GetCamPos()-last_pl_pos).GetLength() * 2;

			//if (fNeededFOV>PI * 0.55f) fNeededFOV = PI * 0.55f;

			if (fNeededFOV>fCurFOV)
			{
				fNeededFOV=fCurFOV;				
			}			
		}	
	}
	else
	{
		fNeededFOV = actFOV;

		if (fLastFov<-0.95f)
		{
			fCurFOV = actFOV;
		}
		else
		{
			fCurFOV = fLastFov;
		}		
	}	

	float tmp = (fNeededFOV - fCurFOV);

	float delta = tmp;
	float k = SIGN(delta);

	delta = fabs(delta);

	if (delta< fMinAngle) delta = fMinAngle;
	if (delta> fMaxAngle) delta = fMaxAngle;

	float fAngleSpeed = (delta - fMinAngle)/(fMaxAngle - fMinAngle);

	fAngleSpeed = (fMinSpeedAngle + (fMaxSpeedAngle - fMinSpeedAngle) * fAngleSpeed);

	if (dltTime * fAngleSpeed * ang_speed > tmp * k)
	{
		fCurFOV = fNeededFOV;

		if (!IsActive())
		{
			DelUpdate(&AutoZoom::Work);
		}
	}
	else
	{
		fCurFOV += dltTime * fAngleSpeed * ang_speed * k;
	}

	fLastFov = fCurFOV;


	Matrix proj(prj);



	//float fWidth = (float)Render().GetViewport().Width;
	//float fHeight = (float)Render().GetViewport().Height;
	float fPerspective = recalculateFov(fWidth, fHeight, Render().GetWideScreenAspectWidthMultipler(), fCurFOV);

	Matrix __mProjection;
	__mProjection.BuildProjection(fPerspective, fWidth, fHeight, 0.1f, 4000.0f);
	proj.m[0][0] = __mProjection.m[0][0];
	proj.m[1][1] = __mProjection.m[1][1];



/*
	proj.m[0][0] = float(1.0 / tan(fCurFOV * 0.5));
	proj.m[1][1] = float(1.0 / tan(lst_aspect * fCurFOV * 0.5f));	
*/



	//proj.m[0][0] = float(1.0/tan(fCurFOV*0.5*Render().GetWideScreenAspectFovMultipler()));
	//proj.m[1][1] = float(1.0/tan((600.0f/800.0f)*fCurFOV*0.5f));	

	Render().SetProjection(proj);

	
	if (player)
	{
		float blend = 0.0f;
			
		if (fabs(fNeededFOV - lst_fov)>0.0001)
		{
			blend = (fCurFOV - lst_fov)/(fNeededFOV - lst_fov);

			if (!IsActive())
			{
				blend = 1.0f - blend;
			}			
		}

		Matrix mView = view;	
		mView.Inverse();

		Quaternion dir = Quaternion(mView.vz.x,mView.vz.y,mView.vz.z,0.0f);		

		

		Vector to_player = last_pl_pos - mView.pos;

		if (camcontroller)
		{
			Vector target_pos;
			camcontroller->GetTargetPoint(target_pos);
			to_player += Vector(target_pos.x, target_pos.y-0.5f, target_pos.z);
		}
				
		to_player.Normalize();

		dir.SLerp(dir,Quaternion(to_player.x,to_player.y,to_player.z,0.0f),blend);

		mView.BuildView(mView.pos,mView.pos+Vector(dir.x,dir.y,dir.z),mView.vy);

		if (camcontroller)
		{
			Matrix shock_mat;
			camcontroller->GetBlenderMatrix(shock_mat,fCurFOV);
			mView.Inverse();

			mView = shock_mat * mView;

			mView.Inverse();
		}		

		Render().SetView(mView);
	}	

	//fCurFOV *= Render().GetWideScreenAspectFovMultipler();			
}

MOP_BEGINLIST(AutoZoom, "", '1.00', 0)
MOP_ENDLIST(AutoZoom)

