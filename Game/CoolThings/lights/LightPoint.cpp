//===========================================================================================================================
// Spirenkov Maxim, 2003
//===========================================================================================================================//	
// Mission objects
//===========================================================================================================================
// LightPoint	
//============================================================================================
			

#include "LightPoint.h"


#ifndef NO_CONSOLE
static bool bRuntimeLightDebug = false;
static bool bCommandIsRegistred = false;
#endif

//============================================================================================

LightPoint::LightPoint()
{
	light = null;
	texture = null;
	crnIntenc = 1.0f;
	crnFlicker = 0.0f;
	isFlicker = false;
	
	lenUpDownTimeSquared = 0.0f;
	flameUpDownTime = 0.0f;

	intensity = 0.0f;
	connectObjectPtr.Reset();	
	intensity_multiply = 1.0f;
}

LightPoint::~LightPoint()
{
	if(light)
	{
		Render().ReleaseLight(light);
	}
	light = null;
	if(texture) texture->Release();
	texture = null;
}


//============================================================================================

#ifndef NO_CONSOLE
void _cdecl LightPoint::Console_ShowRuntimeDebug(const ConsoleStack & params)
{
	bRuntimeLightDebug = !bRuntimeLightDebug;

	IConsole* console = (IConsole *)api->GetService("Console");
	if (bRuntimeLightDebug)
	{
		console->Trace(COL_ALL, "Runtime light debug is enabled");
	} else
	{
		console->Trace(COL_ALL, "Runtime light debug is disabled");
	}

}

#endif


//Инициализировать объект
bool LightPoint::Create(MOPReader & reader)
{
#ifndef NO_CONSOLE
	if (bCommandIsRegistred == false)
	{
		Console().Register_PureC_Command("sl", "Show/hide point lights debug", &Console_ShowRuntimeDebug);
	}
#endif

	basePosition = position = reader.Position();
	color = reader.Colors();
	color *= reader.Float();
	radius = reader.Float();
	directivity = reader.Float();
	if(!light)
	{
		light = Render().CreateLight(position, color, radius, directivity);
	}else{
		light->Set(position, color, radius, directivity);
	}

	if (light)
	{
		light->Enable(false);
		light->debugSetName(GetObjectID().c_str());
	}

	bool isActive = false;
	isFlicker = reader.Bool();
	for(long i = 0; i < ARRSIZE(osc); i++)
	{
		osc[i].step = reader.Float();
		if(osc[i].step > 0.0f) isActive = true; else osc[i].step = 0.0f;
		osc[i].amp = reader.Float();
		if(osc[i].amp > 0.0f) osc[i].kamp = osc[i].amp; else osc[i].kamp = 0.0f;
		osc[i].k = 0.0f;
		osc[i].oldv = RRnd(-osc[i].amp, osc[i].amp);
		osc[i].newv = RRnd(-osc[i].amp, osc[i].amp);
	}
	DelUpdate(&LightPoint::Flicker);
	Flicker(0.0f, 0);
	const char * textureName = reader.String().c_str();
	IBaseTexture * newTexture = string::NotEmpty(textureName) ? Render().CreateTexture(_FL_, textureName) : null;
	if(texture)
	{
		DelUpdate(&LightPoint::Corona);
		texture->Release();
	}
	texture = newTexture;
	crnRange = reader.Float();
	crnSize = reader.Float();
	crnFlicker = reader.Float();
	crnColor = reader.Colors();
	crnColor.Clamp();
	crnIntenc = 1.0f;
	crnIntencVis = 0.0f;
	connectToObject = reader.String();
	bShadowAffected = reader.Bool();
	Activate(reader.Bool());
	return true;
}

//Получить матрицу объекта
Matrix & LightPoint::GetMatrix(Matrix & mtx)
{
	mtx.SetIdentity();
	if(light)
	{
		mtx.pos = light->GetPos();
	}else{
		mtx.pos = position;
	}
	return mtx;
}

//Активировать/деактивировать объект
void LightPoint::Activate(bool isActive)
{
	Light::Activate(isActive);
	intensity = 1.0f;
	DelUpdate(&LightPoint::FlameUp);
	DelUpdate(&LightPoint::FlameDown);
	if(isActive)
	{
#ifndef NO_CONSOLE
		SetUpdate(&LightPoint::RuntimeDebug, ML_ALPHA5+100);
#endif

		LogicDebug("Activate");

		/*
#ifndef _XBOX
		Console().Trace(COL_ALL, "Point Light '%s' enabled", GetObjectID().c_str());
#endif
		*/

		if(texture)
		{
			SetUpdate(&LightPoint::Corona, ML_WEATHER5);
		}
		if(isFlicker)
		{
			SetUpdate(&LightPoint::Flicker, ML_EXECUTE1);
		}
		
		SetColorLight(color);

		UpdateConnectPointer();

		Registry(ACTIVATE_EVENT_GROUP, &LightPoint::ActivateEvent, 0);
		Registry(DEACTIVATE_EVENT_GROUP, &LightPoint::DeactivateEvent, 0);
		SetUpdate(&LightPoint::SetLight, ML_LIGHTS_ON);
		SetUpdate(&LightPoint::RemoveLight, ML_LIGHTS_OFF);

		if(light)
		{
			//		light->SetOn(isActive);
			SetColorLight(color*intensity);
		}

	}
	else
	{	

#ifndef NO_CONSOLE
		DelUpdate(&LightPoint::RuntimeDebug);
#endif

		LogicDebug("Deactivate");

		/*
#ifndef _XBOX
		Console().Trace(COL_ALL, "Point Light '%s' disabled", GetObjectID().c_str());
#endif
		*/

		DelUpdate(&LightPoint::Corona);
		DelUpdate(&LightPoint::Flicker);
		DelUpdate(&LightPoint::UpdateConnection);
		DelUpdate(&LightPoint::SetLight);		
		Unregistry(ACTIVATE_EVENT_GROUP);
		Unregistry(DEACTIVATE_EVENT_GROUP);		
	}
}

//Перевести объект в спящий режим
void LightPoint::EditMode_Sleep(bool isSleep)
{
#ifndef MIS_STOP_EDIT_FUNCS
	MissionObject::EditMode_Sleep(isSleep);
#endif
	if(isSleep)
	{
		light->Enable(false);
	}else{
		Activate(IsActive());
	}
}

//============================================================================================

//Инициализировать объект
bool LightPoint::EditMode_Create(MOPReader & reader)
{
	Create(reader);
	if(!model) CreateModel("editor\\PntLight.gmx");
	Light::EditMode_Create(reader);


	SetUpdate(&LightPoint::PointLightDraw, ML_GEOMETRY1);
	return true;
}


#ifndef NO_CONSOLE
void _cdecl LightPoint::RuntimeDebug(float dltTime, long level)
{
	if (!bRuntimeLightDebug) return;

	Color normalizedColor = color;

	if (normalizedColor.r > 1.0f || normalizedColor.g > 1.0f || normalizedColor.b > 1.0f)
	{
		normalizedColor.v4.v.Normalize();
	}

	Matrix mtx;
	GetMatrix(mtx);
	Render().DrawSphereGizmo(mtx.pos, radius, normalizedColor.GetDword(), normalizedColor.GetDword());
	Render().DrawSphere(mtx.pos, 0.1f, normalizedColor.GetDword());

	Vector camPos = Render().GetView().GetCamPos();

	float distToCam = (mtx.pos - camPos).GetLength();


	Render().Print(mtx.pos, 1000.0f, -3.0f, 0xFFFFFFFFL, "%s", GetObjectID().c_str());
	Render().Print(mtx.pos, 1000.0f, -2.0f, 0xFFFFFFFFL, "c: %3.2f, %3.2f, %3.2f", color.r, color.g, color.b);
	Render().Print(mtx.pos, 1000.0f, -1.0f, 0xFFFFFFFFL, "r: %3.2f d: %3.2f", radius, distToCam);
}
#endif

void _cdecl LightPoint::PointLightDraw(float dltTime, long level)
{
	if (!Mission().EditMode_IsAdditionalDraw()) return;
	//Render().DrawSphere(GetMatrix(mtx).pos, radius, color.GetDword());

//	DrawRotateGizmo(radius, color.GetDword());

	Matrix mtx;
	GetMatrix(mtx);
	Render().DrawSphereGizmo(mtx.pos, radius, color.GetDword(), color.GetDword());

}

//Мерцание
void _cdecl LightPoint::Flicker(float dltTime, long level)
{
	if (!isFlicker) return;

	float intenc = 0.0f;
	float kAmp = 1.0f;
	for(long i = 0; i < ARRSIZE(osc); i++)
	{
		Oscilator & o = osc[i];
		o.k += o.step*dltTime;
		if(o.k >= 1.0f)
		{
			if(o.k < 2.0f)
			{
				o.k -= 1.0f;
				o.oldv = o.newv;
				o.newv = RRnd(-o.amp, o.amp);
			}else{
				o.k = 0.0f;
				o.oldv = RRnd(-o.amp, o.amp);
				o.newv = RRnd(-o.amp, o.amp);
			}
		}
		float ins = o.oldv + (o.newv - o.oldv)*o.k;
		intenc += ins*kAmp;
		kAmp -= ins*o.kamp;
		if(kAmp < 0.0f) break;
	}
	intenc = 1.0f + Clampf(intenc, -1.0f, 1.0f);
	
	if(light)
	{
		SetColorLight(color*intenc);		
  		light->SetPos(position + Clampf((intenc - 1.0f)*0.8f, -0.1f, 0.1f));
		light->SetRadius(radius * (intenc*0.1f + 1.0f));
	}
	crnIntenc = 1.0f + (intenc - 1.0f)*crnFlicker;
}

//Корона
void _cdecl LightPoint::Corona(float dltTime, long level)
{
	static const float minDist = 0.3f;
	bool isVisible = true;
	Vector src = Render().GetView().GetCamPos();
	float dist = (src - position).GetLength();
	if(dist < crnRange)
	{
		if(dist > minDist)
		{
//			float res = Mission().GroupTrace(MG_COLLISION, src, position);
//			isVisible = !(res < 1.0f && res < (1.0f - minDist/dist));
		}
	}else isVisible = false;
	//Интенсивность короны в зависимости от дистанции
	dist = 1.0f - dist/crnRange;
	//Интенсивность короны в зависимости от видимости
	if(isVisible)
	{
		crnIntencVis += dltTime*7.0f;
		if(crnIntencVis > 1.0f) crnIntencVis = 1.0f;
	}else{
		crnIntencVis -= dltTime*7.0f;
		if(crnIntencVis < 0.0f) crnIntencVis = 0.0f;
	}
	//Рисуем
	RS_RECT r;
	r.vPos = position;
	r.fSizeX = crnSize*crnIntenc;
	r.fSizeY = crnSize*crnIntenc;
	r.fAngle = 0.0f;
	r.dwColor = crnColor*(dist*dist*crnIntencVis*crnIntenc);
	r.dwSubTexture = 0;	
	Render().DrawRects(texture,&r, 1, "PointLightCorona");	
}

//Плавное разгорание источника
void _cdecl LightPoint::FlameUp(float dltTime, long level)
{
	flameUpDownTime += dltTime;
	float fIntensity = ((flameUpDownTime*flameUpDownTime) * lenUpDownTimeSquared);

	if(fIntensity >= 0.999999f)
	{
		DelUpdate(&LightPoint::FlameUp);
		return;
	}

	SetColorLight(color * fIntensity);	
}

//Плавное погосание источника
void _cdecl LightPoint::FlameDown(float dltTime, long level)
{
	flameUpDownTime += dltTime;

	float fIntensity = ((flameUpDownTime*flameUpDownTime) * lenUpDownTimeSquared);
	fIntensity = 1.0f - fIntensity;

	if(fIntensity <= 0.0001f)
	{
		LogicDebug("Flamedown command is done, deactivate light");
		Activate(false);
		SetColorLight(color * 0.0f);	
		return;
	}
	
	SetColorLight(color * fIntensity);	
}


void LightPoint::SetPosition(const Vector& vec)
{
	position = vec;
	if(light && !isFlicker)
	{
		light->SetPos(position);
	}
}

//Обработчик команд для объекта
void LightPoint::Command(const char * id, dword numParams, const char ** params)
{
	if(!id || !id[0]) return;
	if(string::IsEqual(id, "moveto"))
	{
		if(numParams >= 3)
		{			
			float x = (float)atof(params[0]);
			float y = (float)atof(params[1]);
			float z = (float)atof(params[2]);
			SetPosition (Vector(x,y,z));
			//LogicDebug("Command: moveto, new light position is (x:%f, y:%f, z:%f)", x, y, z);
		}else{
			LogicDebugError("Command: moveto, Ignore because light already active");
		}
		return;
	}else
	if(string::IsEqual(id, "SetMultiply"))
	{
		if(numParams >= 1)
		{			
			intensity_multiply = Clampf((float)atof(params[0]));
			SetColorLight(color*intensity);
		}
	}
	else
	if(string::IsEqual(id, "Flameup"))
	{
		// Vano: убрал этот if, потому что если был включен flamedown и он еще не закончился, то
		//       flameup не начинается, из-за этого источники света не включаются
		/*if(IsActive())
		{
			LogicDebugError("Command: Flameup, Ignore because light already active");
			return;
		}*/
		if(numParams < 1)
		{
			LogicDebugError("Command: Flameup, invalidate param count, time not set");
			return;
		}
		flameUpDownTime = 0.0f;
		float maxFlameUpDownTime = (float)atof(params[0]);
		LogicDebug("Command: Flameup %f", maxFlameUpDownTime);

		maxFlameUpDownTime = maxFlameUpDownTime * maxFlameUpDownTime;
		lenUpDownTimeSquared = 1.0f / maxFlameUpDownTime;

		Activate(true);
		if(maxFlameUpDownTime > 1e-5f)
		{			
			DelUpdate(&LightPoint::FlameDown);
			SetUpdate(&LightPoint::FlameUp, ML_EXECUTE2);
			FlameUp(0.0f, 0);
		}
		return;
	}else
	if(string::IsEqual(id, "Flamedown"))
	{
		if(!IsActive())
		{
			LogicDebugError("Command: Flamedown, Ignore because light is not active");
			return;
		}
		if(numParams < 1)
		{
			LogicDebugError("Command: Flamedown, invalidate param count, time not set");
			return;
		}
		flameUpDownTime = 0.0f;
		float maxFlameUpDownTime = (float)atof(params[0]);
		LogicDebug("Command: Flamedowm, time %f", maxFlameUpDownTime);

		maxFlameUpDownTime = maxFlameUpDownTime * maxFlameUpDownTime;
		lenUpDownTimeSquared = 1.0f / maxFlameUpDownTime;

		if(maxFlameUpDownTime > 1e-5f)
		{
			DelUpdate(&LightPoint::FlameUp);
			SetUpdate(&LightPoint::FlameDown, ML_EXECUTE2);
		}else{
			Activate(false);
		}
	}else{
		LogicDebug("Unknown command: %s", id);
	}
}

/*
bool LightPoint::NeedDrawRotateGizmoLine (const Vector &v1, const Vector &v2, const Vector& vCamDir)
{
	//return true;
	Matrix mTransform;
	GetMatrix(mTransform);

	Vector vCenter = mTransform.pos;

	Vector dir1 = v1 - vCenter;
	Vector dir2 = v1 - vCenter;

	if ((vCamDir | dir1) > 0) return false;
	if ((vCamDir | dir2) > 0) return false;

	return true;
}



void LightPoint::DrawRotateGizmo (float fRadius, DWORD dwCOLOR)
{
	if (!EditMode_IsOn()) return;
	Matrix mTransform;
	GetMatrix(mTransform);


	Matrix mCurView = Render().GetView();
	mCurView.Inverse();
	Vector vCamDir = mCurView.vz;


	Matrix wrld = Matrix();

	float fDelta = 0.2f;


	Vector vStart;
	Vector vEnd;

	Vector mFrom = mTransform.pos;


	Vector vStartPoint = Vector(sinf(0)*fRadius, cosf(0)*fRadius, 0.0f);
	for (float Angle = fDelta; Angle <= (PI*2); Angle += fDelta)
	{
		Vector vPoint = Vector(sinf(Angle)*fRadius, cosf(Angle)*fRadius, 0.0f);

		vStart = vStartPoint*wrld;
		vEnd = vPoint*wrld;

		vStart += mFrom;
		vEnd += mFrom;

		if (NeedDrawRotateGizmoLine(vStart, vEnd, vCamDir))
		{
			Render().DrawLine(vStart, dwCOLOR, vEnd, dwCOLOR, false, "EditorGridLine");
		}


		vStartPoint = vPoint;
	}

	vStart = vStartPoint*wrld;
	vEnd = Vector(sinf(0)*fRadius, cosf(0)*fRadius, 0.0f)*wrld;
	vStart += mFrom;
	vEnd += mFrom;
	if (NeedDrawRotateGizmoLine(vStart, vEnd, vCamDir))
	{
		Render().DrawLine(vStart, dwCOLOR, vEnd, dwCOLOR, false, "EditorGridLine");
	}




	vStartPoint = Vector(0.0f, cosf(0)*fRadius, sinf(0)*fRadius);
	for (float Angle = fDelta; Angle <= (PI*2); Angle += fDelta)
	{
		Vector vPoint = Vector(0.0f, cosf(Angle)*fRadius, sinf(Angle)*fRadius);
		vStart = vStartPoint*wrld;
		vEnd = vPoint*wrld;
		vStart += mFrom;
		vEnd += mFrom;

		if (NeedDrawRotateGizmoLine(vStart, vEnd, vCamDir))
		{
			Render().DrawLine(vStart, dwCOLOR, vEnd, dwCOLOR, false, "EditorGridLine");
		}

		vStartPoint = vPoint;
	}

	vStart = vStartPoint*wrld;
	vEnd = Vector(0.0f, cosf(0)*fRadius, sinf(0)*fRadius)*wrld;
	vStart += mFrom;
	vEnd += mFrom;
	if (NeedDrawRotateGizmoLine(vStart, vEnd, vCamDir))
	{
		Render().DrawLine(vStart, dwCOLOR, vEnd, dwCOLOR, false, "EditorGridLine");
	}




	vStartPoint = Vector(cosf(0)*fRadius, 0.0f, sinf(0)*fRadius);
	for (float Angle = fDelta; Angle <= (PI*2); Angle += fDelta)
	{
		Vector vPoint = Vector(cosf(Angle)*fRadius, 0.0f, sinf(Angle)*fRadius);

		vStart = vStartPoint*wrld;
		vEnd = vPoint*wrld;
		vStart += mFrom;
		vEnd += mFrom;
		if (NeedDrawRotateGizmoLine(vStart, vEnd, vCamDir))
		{
			Render().DrawLine(vStart, dwCOLOR, vEnd, dwCOLOR, false, "EditorGridLine");
		}

		vStartPoint = vPoint;
	}

	vStart = vStartPoint*wrld;
	vEnd = Vector(cosf(0)*fRadius, 0.0f, sinf(0)*fRadius)*wrld;
	vStart += mFrom;
	vEnd += mFrom;

	if (NeedDrawRotateGizmoLine(vStart, vEnd, vCamDir))
	{
		Render().DrawLine(vStart, dwCOLOR, vEnd, dwCOLOR, false, "EditorGridLine");
	}





	Matrix mView = Render().GetView();
	mView.pos = 0.0f;
	mView.Inverse();
	DWORD dwColor = 0xFFFFFFFF;
	vStartPoint = Vector(sinf(0)*fRadius, cosf(0)*fRadius, 0.0f);
	for (float Angle = fDelta; Angle <= (PI*2); Angle += fDelta)
	{
		Vector vPoint = Vector(sinf(Angle)*fRadius, cosf(Angle)*fRadius, 0.0f);

		vStart = vStartPoint*mView;
		vEnd = vPoint*mView;
		vStart += mFrom;
		vEnd += mFrom;

		Render().DrawLine(vStart, dwColor, vEnd, dwColor, false, "EditorGridLine");
		vStartPoint = vPoint;
	}

	vStart = vStartPoint*mView;
	vEnd = Vector(sinf(0)*fRadius, cosf(0)*fRadius, 0.0f)*mView;
	vStart += mFrom;
	vEnd += mFrom;
	Render().DrawLine(vStart, dwColor, vEnd, dwColor, false, "EditorGridLine");

}
*/

//Обновить указатель на объект к которому прицепленны
void LightPoint::UpdateConnectPointer()
{
	if(!connectToObject.c_str())
	{
		return;
	}

	if(!connectToObject.c_str()[0])
	{
		connectToObject.Empty();
		connectObjectPtr.Reset();
		DelUpdate(&LightPoint::UpdateConnection);
		return;
	}

	LogicDebug("Light connected to %s", connectToObject.c_str());
	SetUpdate(&LightPoint::UpdateConnection);
}

//Обновить позицию источника
void _cdecl LightPoint::UpdateConnection(float dltTime, long level)
{
	if(connectObjectPtr.Validate())
	{
		SetPosition(connectObjectPtr.Ptr()->GetMatrix(Matrix())*basePosition);
	}
	else
	{
		SetPosition(basePosition);
		FindObject(connectToObject,connectObjectPtr);
	}	
}

void LightPoint::SetColorLight(Color color)
{
	if (light)
	{
		light->SetColor(color * intensity_multiply);

		light->setAffectedByShadow(bShadowAffected);
	}

}

//Установка источника в основном цикле
void _cdecl LightPoint::SetLight(float dltTime, long level)
{
	if(EditMode_IsOn() && EditMode_IsSleep())
	{
		return;
	}
	ActivateEvent(null, null);
}

//Установка источника в основном цикле
void _cdecl LightPoint::RemoveLight(float dltTime, long level)
{
	if(EditMode_IsOn() && EditMode_IsSleep())
	{
		return;
	}
	DeactivateEvent(null, null);
	if(!IsActive())
	{
		DelUpdate(&LightPoint::RemoveLight);
	}	
}

//Выполнить событие активации
void _cdecl LightPoint::ActivateEvent(const char * group, MissionObject * sender)
{
	light->Enable(IsActive());
}

//Выполнить событие деактивации
void _cdecl LightPoint::DeactivateEvent(const char * group, MissionObject * sender)
{
	light->Enable(false);
}


//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(LightPoint, "Point light", '1.00', 900, "Dynamic point light\nCommands: Flameup time; flamedown time; Moveto x, y, z", "Lighting")
	MOP_POSITION("Position", Vector(0.0f))
	MOP_COLOR("Color", Color(1.0f))
	MOP_FLOATEX("Intensity", 1.0f, -1000.0f, 1000.0f)
	MOP_FLOATEX("Radius", 10.0f, 0.01f, 1000.0f)	
	MOP_FLOATEX("Directivity", 0.8f, 0.0f, 1.0f)
	MOP_BOOL("Enable flicker", false);
	MOP_FLOATEX("Freq1", 0.4f, 0.0f, 100.0f)
	MOP_FLOATEX("Amp1", 0.3f, 0.0f, 1.0f)
	MOP_FLOATEX("Freq2", 10.0f, 0.0f, 100.0f)
	MOP_FLOATEX("Amp2", 0.08f, 0.0f, 1.0f)
	MOP_STRING("Corona's texture", "")
	MOP_FLOATEX("Corona's range", 5.0f, 0.0f, 1000.0f)
	MOP_FLOATEX("Corona's size", 0.2f, 0.01f, 1000.0f)
	MOP_FLOATEX("Corona's flicker", 0.4f, 0.0f, 1.0f)
	MOP_COLOREX("Corona's color", Color(1.0f), Color(0L), Color(0xffffffffL))
	MOP_STRING("Connect to object", "")
	MOP_BOOL("Affected by shadow", false)
	MOP_BOOL("Active", true)
MOP_ENDLIST(LightPoint)





