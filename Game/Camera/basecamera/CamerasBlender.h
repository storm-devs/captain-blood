//============================================================================================
// Spirenkov Maxim
//============================================================================================
// Camera's blender
//============================================================================================

#ifndef _MissionCamerasBlender_h_
#define _MissionCamerasBlender_h_

#include "..\..\..\Common_h\Mission\Mission.h"

class MissionCamerasBlender
{
public:

	enum TShockType
	{
		SH_ShockRND = 0,
		SH_ShockRND_H,
		SH_ShockRND_V,
		SH_ShockPolar,
		SH_ShockPolar_H,
		SH_ShockPolar_V
	};
	
	struct TShockParams
	{
		float shockTime;			//Время тряски
		float shockKTime;			//Масштаб течения времени
		float shockAmp;				//Амплитуда тряски
		float shockMoveTime;		//Время жизни текущей позиции смещения
		float shockIntencity;		//Интетсивность тряски
		Vector shockMoveStart;		//Относительное текущее смещение
		Vector shockMoveEnd;		//Относительное желаемое смещение	

		TShockType ShockType;       //Тип встряски
	};

public:
	MissionCamerasBlender();
	~MissionCamerasBlender();
	//Инициализировать
	void __fastcall Init(float blendTime);

public:
	//Активация
	void __fastcall Activate(const Matrix & view, const Matrix & prj, float fWidth, float fHeight, float fWidthAspectMultipler);
	
	//Встряхнуть камеру
	void __fastcall Shock(TShockType ShType,float amp, float time, float intencity);

	//Vano: добавил ресет для сумашедшего качания, потому что оно залипало когда камеры переключались в процессе ActiveCrazy(true)
	void __fastcall ResetCrazy()
	{
		bCrazyOn = false;
		waveAmp = 0.0f;
	}

	void __fastcall ResetShock()
	{
		for (int i=0;i<3;i++)
		{
			ShockParams[i].shockTime = 1.0f;
		}
	};

	//Обновить состояние
	void __fastcall Update(float dltTime);
	//Получить новую матрицу из текущей
	Matrix & __fastcall GetMatrix(Matrix & mtx,float fov,bool ignore_blender_fov = false);
	//Получить угол зрения
	float __fastcall GetFov(float fov);	

	void __fastcall ActiveCrazy(bool _CrazyOn) { bCrazyOn = _CrazyOn; };

	void __fastcall SetCrazyParams(float Amp, float Speed) { waveMAXAmp=Amp; waveSpeed=Speed; };

private:
	float kBlendTime;					//Время подтягивания камеры
	float currentTime;					//Текущее время
	Quaternion actOriention;			//Ориентация камеры в момент активации
	Vector actPosition;					//Позиция камеры в момент активации	
	float actFOV;						//Угол зрения в момент активации	

	TShockParams ShockParams[3];
	int          LastShockIndex;
		 
	bool   bCrazyOn;
	float  waveAmp;
	float  fTime;

	float waveMAXAmp;
	float waveSpeed;
};

__forceinline MissionCamerasBlender::MissionCamerasBlender()
{
	kBlendTime = 0.0f;
	currentTime = 1.0f;

	waveAmp = 0;
	fTime = 0;

	bCrazyOn=false;

	LastShockIndex = 0;
	
	ShockParams[0].shockTime = 1.0f;
	ShockParams[0].shockKTime = 1.0f;

	ShockParams[1].shockTime = 1.0f;
	ShockParams[1].shockKTime = 1.0f;

	ShockParams[2].shockTime = 1.0f;
	ShockParams[2].shockKTime = 1.0f;

	waveMAXAmp=0.04f;
	waveSpeed=2.0f;	
}

__forceinline MissionCamerasBlender::~MissionCamerasBlender()
{
}

//Инициализировать
__forceinline void __fastcall MissionCamerasBlender::Init(float blendTime)
{
	if(blendTime > 0.0f)
	{
		kBlendTime = 1.0f/blendTime;
	}else{
		kBlendTime = 0.0f;
	}	
	currentTime = 1.0f;
}

//Активация
__forceinline void __fastcall MissionCamerasBlender::Activate(const Matrix & view, const Matrix & prj, float fWidth, float fHeight, float fWidthAspectMultipler)
{
	if(kBlendTime > 0.0f)
	{
		Matrix mtx(true);
		mtx.Inverse(view);
		actPosition = mtx.pos;
		actOriention = Quaternion(mtx);

		float fovFromProjMatrix = float(fabs(atan(1.0f/prj.m[0][0])*2.0));	

 		actFOV = invRecalculateFov(fWidth, fHeight, fWidthAspectMultipler, fovFromProjMatrix);

		currentTime = 0.0f;
	}else{
		currentTime = 1.0f;
	}	
}

//Встряхнуть камеру
__forceinline void __fastcall MissionCamerasBlender::Shock(TShockType ShType, float amp, float time, float intencity)
{
	LastShockIndex++;
	
	if (LastShockIndex>2)
	{
		LastShockIndex=0;
	}

	ShockParams[LastShockIndex].ShockType = ShType;

	if(time <= 1e-10f)
	{
		ShockParams[LastShockIndex].shockTime = 1.0f;
		return;
	}

	ShockParams[LastShockIndex].shockTime = 0.0f;
	ShockParams[LastShockIndex].shockKTime = 1.0f/time;
	ShockParams[LastShockIndex].shockAmp = amp;
	ShockParams[LastShockIndex].shockMoveTime = 1.0f;

	if (intencity<0.001f)
	{
		intencity = 0.001f;
	}

	if (ShockParams[LastShockIndex].ShockType==SH_ShockRND_H ||
		ShockParams[LastShockIndex].ShockType==SH_ShockRND_V ||
		ShockParams[LastShockIndex].ShockType==SH_ShockRND)
	{	
		intencity = 1/intencity;
	}	

	ShockParams[LastShockIndex].shockIntencity = intencity;
	
	if(ShockParams[LastShockIndex].shockIntencity < 0.000001f)
	{
		ShockParams[LastShockIndex].shockIntencity = 0.000001f;
	}

	ShockParams[LastShockIndex].shockMoveStart = 0.0f;
	ShockParams[LastShockIndex].shockMoveEnd = 0.0f;
}


//Обновить состояние
__forceinline void __fastcall MissionCamerasBlender::Update(float dltTime)
{
	//Попытка убрать фоновую тряску на паузе
	if(dltTime < 1e-8f)
	{
		return;
	}

	if(currentTime < 1.0f)
	{
		currentTime += dltTime*kBlendTime;
	}
	
	for (int i=0;i<3;i++)
	{	
		ShockParams[i].shockTime += dltTime*ShockParams[i].shockKTime;

		if (ShockParams[i].shockTime>1.0f)
		{
			ShockParams[i].shockTime=1.0f;
		}

		if(ShockParams[i].shockTime < 1.0f)
		{				
			if (ShockParams[i].ShockType==SH_ShockRND)
			{			
				float k = 1.0f - powf(ShockParams[i].shockTime, 0.4f);
		
				if(ShockParams[i].shockMoveTime >= 1.0f)
				{
					ShockParams[i].shockMoveTime = 0.0f;
					float amp = k*ShockParams[i].shockAmp;
					
					ShockParams[i].shockMoveStart = ShockParams[i].shockMoveEnd;
					
					ShockParams[i].shockMoveEnd.RandXZ() *= amp;
			
					ShockParams[i].shockMoveEnd.z = fabs(ShockParams[i].shockMoveEnd.z);

					ShockParams[i].shockMoveEnd.y = ShockParams[i].shockMoveEnd.z;
					ShockParams[i].shockMoveEnd.z = 0.0f;
				}
				else
				{
					k = ShockParams[i].shockIntencity*(0.1f + k*0.9f);
					ShockParams[i].shockMoveTime += dltTime*(1.0f/k);
				}
			}
			else
			if (ShockParams[i].ShockType==SH_ShockRND_H||
				ShockParams[i].ShockType==SH_ShockRND_V)
			{			
				float k = 1.0f - powf(ShockParams[i].shockTime, 0.4f);

				if(ShockParams[i].shockMoveTime >= 1.0f)
				{
					ShockParams[i].shockMoveTime = 0.0f;
					float amp = k*ShockParams[i].shockAmp;

					ShockParams[i].shockMoveStart = ShockParams[i].shockMoveEnd;

					float sign=-SIGN(ShockParams[i].shockMoveStart.y);

					ShockParams[i].shockMoveEnd.RandXZ() *= amp;

					ShockParams[i].shockMoveEnd.z = fabs(ShockParams[i].shockMoveEnd.z) * sign;

					ShockParams[i].shockMoveEnd.y = ShockParams[i].shockMoveEnd.z;
					ShockParams[i].shockMoveEnd.z = 0.0f;
				}
				else
				{
					k = ShockParams[i].shockIntencity*(0.1f + k*0.9f);
					ShockParams[i].shockMoveTime += dltTime*(1.0f/k);
				}
			}
		}
	}
	
	if (bCrazyOn)
	{	
		waveAmp += dltTime * 0.05f;
		if (waveAmp > waveMAXAmp) waveAmp = 0.04f;
	}
	else
	{
		waveAmp -= dltTime * 0.05f;
		if (waveAmp < 0.00f) waveAmp = 0.00f;
	}	

	fTime += dltTime * waveSpeed;
}

//Получить новую матрицу из текущей
__forceinline Matrix & __fastcall MissionCamerasBlender::GetMatrix(Matrix & mtx,float fov,bool ignore_blender_fov)
{
	if(currentTime < 1.0f && !ignore_blender_fov)
	{	
		//Позиция
		Vector pos; pos.Lerp(actPosition, mtx.pos, currentTime);
		//Ориентация
		Quaternion q(mtx);
		q.SLerp(actOriention, q, currentTime);
		q.GetMatrix(mtx);
		mtx.pos = pos;
	}

	float fovk = 1.0f;
/*

	float fovk = GetFov(fov)/ (71.619f *PI/180.0f);

	if (ignore_blender_fov)
	{
		fovk = fov/ (71.619f *PI/180.0f);
	}
*/


	for (int i=0;i<3;i++)
	{	
		if(ShockParams[i].shockTime < 1.0f)
		{
			if (ShockParams[i].ShockType == SH_ShockRND)
			{	
				Vector locMove;				
				locMove.Lerp(Vector(ShockParams[i].shockMoveStart.x,ShockParams[i].shockMoveStart.y,ShockParams[i].shockMoveStart.z),
							 Vector(ShockParams[i].shockMoveStart.x,ShockParams[i].shockMoveEnd.y,ShockParams[i].shockMoveEnd.z),
							 ShockParams[i].shockMoveTime);					

				Matrix tmp;

				tmp.BuildView(Vector(0,0,0),Vector(locMove.x*fovk,locMove.y*fovk,1.0f),Vector(0,1,0));
				
				//mtx = mtx*tmp;

				mtx = tmp * mtx;
			}		
			else
			if (ShockParams[i].ShockType == SH_ShockRND_V)
			{	
				Vector locMove;				
				locMove.Lerp(Vector(0,ShockParams[i].shockMoveStart.y,ShockParams[i].shockMoveStart.z),
							 Vector(0,ShockParams[i].shockMoveEnd.y,ShockParams[i].shockMoveEnd.z),
							 ShockParams[i].shockMoveTime);					

				Matrix tmp;

				tmp.BuildView(Vector(0,0,0),Vector(0,locMove.y*fovk,1.0f),Vector(0,1,0));
				
				//mtx = mtx*tmp;

				mtx = tmp * mtx;
			}		
			else
			if (ShockParams[i].ShockType == SH_ShockRND_H)
			{	
				Vector locMove;				
				locMove.Lerp(Vector(0,ShockParams[i].shockMoveStart.y,ShockParams[i].shockMoveStart.z),
							 Vector(0,ShockParams[i].shockMoveEnd.y,ShockParams[i].shockMoveEnd.z),
							 ShockParams[i].shockMoveTime);				

				Matrix tmp;

				tmp.BuildView(Vector(0,0,0),Vector(locMove.y*fovk,0,1.0f),Vector(0,1,0));
				
				//mtx = mtx*tmp;

				mtx = tmp * mtx;
			}		
			else
			if (ShockParams[i].ShockType == SH_ShockPolar ||
				ShockParams[i].ShockType == SH_ShockPolar_V ||
				ShockParams[i].ShockType == SH_ShockPolar_H)
			{		
				float alpha = ShockParams[i].shockTime / ShockParams[i].shockKTime;
		
				float r = 1.0f;				
		
				if (ShockParams[i].shockTime<0.2f)
				{			
					r = ShockParams[i].shockTime/0.2f;
				}
				else
				if (ShockParams[i].shockTime>0.5f)
				{
					r = (1.0f - ShockParams[i].shockTime) * 2.0f;
				}				

				r *= ShockParams[i].shockAmp * 0.1f;
	
				alpha *= ShockParams[i].shockIntencity;

				float dx= r * cos (alpha) * fovk;				
				float dy= r * sin (alpha) * fovk;

				Matrix tmp;

				float kx = 1.0f;
				float ky = 1.0f;
				
				if (ShockParams[i].ShockType == SH_ShockPolar_V)
				{
					kx = 0.075f;
				}
				else
				if (ShockParams[i].ShockType == SH_ShockPolar_H)
				{
					ky = 0.075f;
				}

				tmp.BuildView(Vector(0,0,0),Vector(dx * kx,dy * ky,1.0f),Vector(0,1,0));				

				mtx = tmp * mtx;
			}			
		}		
	}

	{		
		float alpha = fTime;

		float r = waveAmp;
		
		float dx= r * (cos (alpha));
		float dy=1.0f;
		float dz= r * (sin (alpha));

		Matrix tmp;

		//tmp.BuildView(Vector(0,0,0),Vector(dx*0.05f,dz,dy),Vector(0,1,0));
		tmp.BuildView(Vector(0,0,0),Vector(0,0,1.0f),Vector(0+dz*2,1,0-dz*0));

		mtx = tmp * mtx;
	}	
	
	return mtx;
}

//Получить угол зрения
__forceinline float __fastcall MissionCamerasBlender::GetFov(float fov)
{
	if(currentTime >= 1.0f) return fov;
	return actFOV + (fov - actFOV)*currentTime;
}


#endif

