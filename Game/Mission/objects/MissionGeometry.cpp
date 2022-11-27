//============================================================================================
// Spirenkov Maxim, 2004
//============================================================================================
// Mission objects
//============================================================================================
// MissionGeometry
//============================================================================================

#include "MissionGeometry.h"


#define GLUE_DISTANCE 3950.0f

//============================================================================================	

const MissionGeometry::PGroup MissionGeometry::pgroups[] =
{
	{ConstString("World"), phys_world},
	{ConstString("Character"), phys_character},
	{ConstString("Player"), phys_player},
	{ConstString("Particles"), phys_particles},
	{ConstString("Ships"), phys_ship},
	{ConstString("Blood"), phys_bloodpatch},
	{ConstString("Boss"), phys_boss}
};


MissionGeometry::MissionGeometry()
{
	bUseDistanceHack = false;
	rigidBody = null;
	model = null;
	events = null;
	shadow = null;
	fadeAlpha = 1.0f;
	alpha = 1.0f;	
	fade = fd_none;
	fadeTime = 0.0f;
	isCheckLod = false;
	isPrevLodAlpha = false;
	bNotUseFog = false;

	h_density = 0.0f;
	h_min = 0.0f;
	h_max = 0.0f;
	d_density = 0.0f;
	d_min = 0.0f;
	d_max = 0.0f;
	fog_color = Color(0.0f);

	
}


MissionGeometry::~MissionGeometry()
{
	if(events)
	{
		delete events;
		events = null;
	}
	if(rigidBody) rigidBody->Release();
	rigidBody = null;
	UnloadModel();
}

//============================================================================================

//Инициализировать объект
bool MissionGeometry::Create(MOPReader & reader)
{
	if(!GeometryObject::Create(reader)) return false;
	if(rigidBody) rigidBody->Release();
	rigidBody = null;
	if(!events)
	{
		delete events;
		events = null;
	}
	bool res = LoadModel(model, reader.String());
	res |= LoadModel(shadow, reader.String());
	if(!res) return false;
	//Назначаемая анимация
	const char * ani = reader.String().c_str();
	if(ani[0] && model)
	{
		events = NEW AnimationStdEvents();
		events->Init(&Sound(), &Particles(), &Mission());
		model->SetAnimationFile(ani);		
	}
	//Позиция
	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	modelMatrix.Build(ang, pos);
	//К кому присоединять
	connectToName = reader.String();
	connectToPtr.Reset();
	if(!reader.Bool())
	{
		connectToName.Empty();
	}
	if( EditMode_IsOn() && connectToName.NotEmpty())
		FindObject(connectToName, connectToPtr);
	//Использование рутовой кости
	bUseRootBone = reader.Bool();
	//Видимость, коллижен
	bool show = reader.Bool();
	bool act = reader.Bool();
	//Группа коллизии
	ConstString gid = reader.Enum();
	PhysicsCollisionGroup physGroup = phys_world;
	for(dword i = 0; i < ARRSIZE(pgroups); i++)
	{
		if(gid == pgroups[i].name)
		{
			physGroup = pgroups[i].group;
			break;
		}
	}
	Assert(i < ARRSIZE(MissionGeometry::pgroups));

	bNotUseFog = reader.Bool();

	//Уровень отрисовки
	level = reader.Long();
	//Цвет
	color = reader.Colors();

	if(model)
	{
		model->SetUserColor(color);
	}


	//Делаем "мягкую" альфу
	bool bSmoothAlpha = reader.Bool();
	if(bSmoothAlpha && model)
	{
		model->SetFloatAlphaReference(0.003921f);
	}


	enableRender = reader.Bool();

	//Уровень отрисовки	
	if(!reader.Bool())
	{
		level = (ML_GEOMETRY3 - ML_GEOMETRY1)*level/100 + ML_GEOMETRY1;
	}else{
		level = (ML_ALPHA3 - ML_ALPHA1)*level/100 + ML_ALPHA1;
	}
	//Освещение
	dynamicLighting = reader.Bool();
	if(model)
	{
		model->SetDynamicLightState(dynamicLighting);
	}
	//Теневые дела
	shadowCast = reader.Bool();
	shadowReceive = reader.Bool();
	//Отражение в море
	seaReflection = reader.Bool();
	//Преломление в море
	seaRefraction = reader.Bool();
	//Прятать при скрывании в редакторе
	hideInEditor = reader.Bool();
	//Качатся или нет
	noSwing = reader.Bool();
	//Дистанция лодирования
	float lodDistanceMin = reader.Float();
	float lodDistanceDelta = reader.Float();

	//Использовать distance hack или нет
	bUseDistanceHack = reader.Bool();

	lodDistanceDelta = Clampf(lodDistanceDelta, 0.001f, 1000.0f);
	isPrevLodAlpha = false;
	if(lodDistanceMin > 1e-5f)
	{
		float lodDistanceMax = lodDistanceMin + lodDistanceDelta;
		lodDistanceMin2 = lodDistanceMin*lodDistanceMin;		
		lodDistanceK2 = 1.0f/(lodDistanceMax*lodDistanceMax - lodDistanceMin*lodDistanceMin);
		isCheckLod = true;
	}else{
		lodDistanceMin2 = 0.0f;
		lodDistanceK2 = 0.0f;
		isCheckLod = false;
	}
	//Инициализация	
	if(!EditMode_IsOn() && events) events->SetScene(model, modelMatrix);	
	//Установим матрицу
	if(model)
	{
		model->SetTransform(modelMatrix);
		model->SetShadowReceiveState(shadowReceive);
		rigidBody = model->CreatePhysicsActor(Physics(), false);
		if(rigidBody)
		{
			rigidBody->SetGroup(physGroup);
			rigidBody->SetTransform(modelMatrix);
		}
	}
	//Применение флажков
	fade = fd_none;
	fadeAlpha = 1.0f;
	alpha = 1.0f;	
	Show(show);
	Activate(act);
	return true;
}

//Вызываеться, когда все объекты созданны но ещё не началось исполнение миссии
void MissionGeometry::PostCreate()
{
	if(connectToName.NotEmpty())
	{
		FindObject(connectToName, connectToPtr);
	}
}

/*
//Пересоздать объект
void MissionGeometry::Restart()
{
	ReCreate();
}
*/

//Обработчик команд для объекта
void MissionGeometry::Command(const char * id, dword numParams, const char ** params)
{
	if(string::IsEmpty(id)) return;
	const ConstString cmd(id);
	//Список команд
	static const ConstString cmd_postdraw("postdraw");
	static const ConstString cmd_teleport("teleport");
	static const ConstString cmd_startnode("startnode");
	static const ConstString cmd_actlink("actlink");
	static const ConstString cmd_fadeout("fadeout");
	static const ConstString cmd_fadein("fadein");
	static const ConstString cmd_fade("fade");
	static const ConstString cmd_shadow_cast("shadow_cast");
	static const ConstString cmd_shadow_receive("shadow_receive");
	static const ConstString cmd_collapse("collapse");
	static const ConstString cmd_reset("reset");
	static const ConstString cmd_pause("pause");
	static const ConstString cmd_unpause("unpause");
	static const ConstString cmd_RedirectSoundEvent("RedirectSoundEvent");
	//Исполняем команду
	if(cmd == cmd_postdraw)
	{
		Draw(0.0f, 0);
	}else
	if(cmd == cmd_teleport)
	{
		if(numParams < 1) return;
		MOSafePointer obj;
		if(FindObject(ConstString(params[0]), obj))
		{
			obj.Ptr()->GetMatrix(modelMatrix);
			if(model) model->SetTransform(modelMatrix);
			if(rigidBody)
			{
				rigidBody->SetTransform(modelMatrix);
			}
			LogicDebug("Teleport geometry \"%s\" to mission object \"%s\"", GetObjectID().c_str(), obj.Ptr()->GetObjectID().c_str());
		}else{
			LogicDebugError("Can't teleport geometry \"%s\" to mission object \"%s\", object not found...", GetObjectID().c_str(), params[0]);
		}
	}else
	if(cmd == cmd_startnode)
	{
		if(numParams < 1 || !model) return;
		float blendTime = 0.0f;
		if(numParams >= 2)
		{
			char * pos = null;
			blendTime = (float)strtod(params[1], &pos);
			blendTime = Clampf(blendTime, 0.0f, 100.0f);
		}
		IAnimation * a = model->GetAnimation();
		if(a)
		{
			if(!a->Goto(params[0], blendTime))
			{
				LogicDebugError("Can't start animation node \"%s\" in geometry \"%s\".", params[0], GetObjectID().c_str());
			}
			a->Release();
		}else{
			LogicDebugError("Can't start animation node \"%s\" in geometry \"%s\", geometry not animated.", params[0], GetObjectID().c_str());
		}
	}else
	if(cmd == cmd_actlink)
	{
		if(numParams < 1 || !model) return;
		IAnimation * a = model->GetAnimation();
		if(a)
		{
			if(!ActivateAnimationLink(params[0]))
			{
				LogicDebugError("Can't start animation link \"%s\" in geometry \"%s\".", params[0], GetObjectID().c_str());
			}
			a->Release();
		}else{
			LogicDebugError("Can't start animation node \"%s\" in geometry \"%s\", geometry not animated.", params[0], GetObjectID().c_str());
		}
	}else
	if(cmd == cmd_fadeout)
	{
		if(fade != fd_hide)
		{
			LogicDebug("Fadeout");
			fade = fd_fadeout;
			fadeTime = 0.0f;
		}else{
			LogicDebugError("Fadeout ignore, course work \"Fade\"");
		}
	}else
	if(cmd == cmd_fadein)
	{		
		if(fade != fd_hide)
		{
			LogicDebug("Fadein");
			fade = fd_fadein;
			fadeTime = 0.0f;
		}else{
			LogicDebugError("Fadein ignore, course work \"Fade\"");
		}
	}else
	if(cmd == cmd_fade)
	{
		if(fade != fd_hide)
		{
			LogicDebug("Fade");
			fade = fd_hide;
			fadeTime = 0.0f;
		}else{
			LogicDebugError("Ignore fade already work \"Fade\"");
		}
	}else
	if(cmd == cmd_shadow_cast)
	{
		if(numParams >= 1 && params[0][0])
		{
			shadowCast = params[0][0] != '0';
			Show(IsShow());
			LogicDebug("%s state shadow_cast in geometry \"%s\"", shadowCast ? "Enable" : "Disable", GetObjectID().c_str());
		}else{
			LogicDebugError("Can't change shadow_cast state in geometry \"%s\", because no params", GetObjectID().c_str());
		}
	}else
	if(cmd == cmd_shadow_receive)
	{		
		if(numParams >= 1 && params[0][0])
		{
			shadowReceive = params[0][0] != '0';
			Show(IsShow());
			LogicDebug("%s state shadow_receive in geometry \"%s\"", shadowReceive ? "Enable" : "Disable", GetObjectID().c_str());
		}else{
			LogicDebugError("Can't change shadow_receive state in geometry \"%s\", because no params", GetObjectID().c_str());
		}
	}else
	if(cmd == cmd_collapse)
	{		
		if(numParams >= 1 && params[0][0])
		{
			if(model)
			{
				IAnimation * ani = model->GetAnimation();
				if(ani)
				{
					const char * prms[2];
					static const char * collapse = "Collapse";
					static const char * restore = "Restore";
					if(numParams >= 2)
					{
						prms[0] = (params[1][0] == 't' || params[1][0] == 'T') ? collapse : restore;
					}else{
						prms[0] = collapse;
					}					
					prms[1] = params[0];
					LogicDebug("%s bone: %s", prms[0], params[0]);
					if(events)
					{
						events->BoneCollapse(ani, "BoneCollapse", prms, 2);
					}else{
						LogicDebugError("Can't collapse bone in geometry \"%s\", because events not created", GetObjectID().c_str());
					}
					ani->Release();
				}else{
					LogicDebugError("Can't collapse bone in geometry \"%s\", because animation isn't set", GetObjectID().c_str());
				}
			}else{
				LogicDebugError("Can't collapse bone in geometry \"%s\", because model not loaded", GetObjectID().c_str());
			}
		}else{
			LogicDebugError("Can't collapse bone in geometry \"%s\", because no params", GetObjectID().c_str());
		}
	}else
	if(cmd == cmd_reset)
	{
		LogicDebug("Reset events");
		if(events)
		{
			events->ResetParticles();
			events->ResetSounds();
			events->ResetCollapser();
		}
		fadeAlpha = 1.0f;
		alpha = 1.0f;
		fade = fd_none;
		IAnimation * ani = model->GetAnimation();
		if(ani)
		{
			ani->Start();
			ani->Release();
		}else{
			LogicDebugError("Can't reset animation, because it's not set");
		}
		Show(IsShow());
	}else
	if(cmd == cmd_pause)
	{
		IAnimation * ani = model->GetAnimation();
		if(ani)
		{
			LogicDebug("Pause");
			ani->Pause(true);
			ani->Release();
		}
	}else
	if(cmd == cmd_unpause)
	{
		IAnimation * ani = model->GetAnimation();
		if(ani)
		{
			LogicDebug("Unpause");
			ani->Pause(false);
			ani->Release();
		}
	}else
	if(cmd == cmd_RedirectSoundEvent)
	{
		LogicDebug("RedirectSoundEvent");
		IAnimation * ani = model->GetAnimation();
		if(ani && events)
		{
			events->PlaySound(ani, "Snd", params, numParams);
			ani->Release();
		}else{
			LogicDebugError("Animation not set for this geometry");	
		}
	}else{
		LogicDebugError("Unknown command: %s", id);
	}
}

//Инициализировать объект
bool MissionGeometry::EditMode_Create(MOPReader & reader)
{
	Create(reader);
	return true;
}

//Обновить параметры
bool MissionGeometry::EditMode_Update(MOPReader & reader)
{
	DelUpdate();
	Unregistry(MG_SHADOWCAST);
	Unregistry(MG_SHADOWRECEIVE);
//	Unregistry(MG_DOF);
	Unregistry(MG_SHADOWDONTRECEIVE);
	Create(reader);
	return true;
}

//Получить размеры описывающего ящика
void MissionGeometry::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	if(model || shadow)
	{
		GetBox(min, max);
		return;
	}
	min = -0.5f;
	max = 0.5f;
}

//Получить бокс, описывающий объект в локальных координатах
void MissionGeometry::GetBox(Vector & min, Vector & max)
{
	if(model)
	{
		min = model->GetLocalBound().vMin;
		max = model->GetLocalBound().vMax;
		if(bUseRootBone)
		{
			IAnimation * ani = model->GetAnimation();
			if(ani)
			{
				max = (max - min)*0.5f;
				min = -max;
				ani->Release();
			}
		}
		return;
	}else
	if(shadow)
	{
		min = shadow->GetLocalBound().vMin;
		max = shadow->GetLocalBound().vMax;
		return;
	}
	min = 0.0f;
	max = 0.0f;
}

//Получить матрицу объекта
Matrix & MissionGeometry::GetMatrix(Matrix & mtx)
{
	if(bUseRootBone && model)
	{
		IAnimation * a = model->GetAnimation();
		if(a)
		{
			mtx = a->GetBoneMatrix(0)*modelMatrix;
			a->Release();
			return mtx;
		}
	}else{
		mtx = modelMatrix;
	}
	if(connectToPtr.Validate())
	{
		mtx = mtx*connectToPtr.Ptr()->GetMatrix(Matrix());
	}

	return mtx;
};

//Показать/скрыть объект
void MissionGeometry::Show(bool isShow)
{
	GeometryObject::Show(isShow);
	if(IsShow())
	{
		LogicDebug("Show");
		PauseAnimation(false);
		long curLevel = level;
		bool isTransp = false;
		if(alpha < 1.0f || fade != fd_none)
		{
			curLevel = c_fadelevel;
			isTransp = true;
		}
		DelUpdate(&MissionGeometry::Draw);
		if(enableRender)
		{
			SetUpdate(&MissionGeometry::Draw, curLevel);
		}
		if(EditMode_IsOn())
		{
			DelUpdate(&MissionGeometry::DrawTransparency);
		}
		if(shadowCast)
		{
			Registry(MG_SHADOWCAST, (MOF_EVENT)&MissionGeometry::ShadowInfo, curLevel);
		}else{
			Unregistry(MG_SHADOWCAST);
		}
		if(shadowReceive)
		{
			Unregistry(MG_SHADOWDONTRECEIVE);
			if(!isTransp)
			{
				Registry(MG_SHADOWRECEIVE, (MOF_EVENT)&MissionGeometry::ShadowRecive, curLevel);
			}else{
				Unregistry(MG_SHADOWRECEIVE);
			}
		}else{
			Unregistry(MG_SHADOWRECEIVE);
			if(curLevel < ML_ALPHA1 && enableRender) Registry(MG_SHADOWDONTRECEIVE, (MOF_EVENT)&MissionGeometry::ShadowRecive, curLevel);
		}
		if(seaReflection)
		{
			Registry(MG_SEAREFLECTION, (MOF_EVENT)&MissionGeometry::SeaReflection, curLevel);
		}else{
			Unregistry(MG_SEAREFLECTION);
		}
		
		if(seaRefraction)
		{
			Registry(MG_SEAREFRACTION, (MOF_EVENT)&MissionGeometry::SeaRefraction, curLevel);
		}else{
			Unregistry(MG_SEAREFRACTION);
		}
//		Registry(MG_DOF, (MOF_EVENT)&MissionGeometry::SeaReflection, curLevel);
	}else{
		LogicDebug("Hide");
		PauseAnimation(true);
		if(events)
		{
			events->ResetParticles();
		}
		DelUpdate(&MissionGeometry::Draw);
		if(EditMode_IsOn() && !hideInEditor)
		{
			SetUpdate(&MissionGeometry::DrawTransparency, ML_ALPHA4);
		}
//		Unregistry(MG_DOF);
		Unregistry(MG_SHADOWCAST);
		Unregistry(MG_SHADOWRECEIVE);
		Unregistry(MG_SEAREFLECTION);
		Unregistry(MG_SEAREFRACTION);
		Unregistry(MG_SHADOWDONTRECEIVE);
	}
}

//Активировать/деактивировать объект
void MissionGeometry::Activate(bool isActive)
{
	GeometryObject::Activate(isActive);
	if(EditMode_IsOn())
	{
		return;
	}
	if(rigidBody)
	{
		rigidBody->Activate(IsActive());
		if(IsActive())
		{
			LogicDebug("Activate");
		}else{
			LogicDebug("Deactivate");
		}
	}else{
		if(isActive)
		{
			LogicDebugError("Can't activate collision -> no present collision data in file \"%s\"", (model ? model->GetFileName() : ""));
		}else{
			LogicDebug("Deactivate");
		}
	}
}


//Нарисовать модельку
void _cdecl MissionGeometry::Draw(float dltTime, long level)
{
	if(!EditMode_IsVisible()) return;	
	if(!model) return;
	if(fade != fd_none)
	{
		if(fadeTime <= 1e-10f)
		{
			if(level == c_fadelevel)
			{
				static const float fadeSpeed = 2.0f;
				static const float hideSpeed = 0.7f;
				switch(fade)
				{
				case fd_fadeout:
					fadeAlpha -= dltTime*fadeSpeed;
					if(fadeAlpha < 0.3f)
					{
						fadeAlpha = 0.3f;
						fade = fd_none;
					}
					break;
				case fd_fadein:
					fadeAlpha += dltTime*fadeSpeed;
					if(fadeAlpha > 1.0f)
					{						
						fadeAlpha = 1.0f;
						fade = fd_none;						
						if(!isCheckLod)
						{							
							alpha = 1.0f;
							Show(false);
							Show(true);
						}else{
							if(!isPrevLodAlpha)
							{
								alpha = 1.0f;
								Show(false);
								Show(true);
							}
						}
					}
					break;
				case fd_hide:
					fadeAlpha -= dltTime*hideSpeed;
					if(fadeAlpha < 0.0f)
					{
						fadeAlpha = 0.0f;
						Show(false);
						return;
					}
					break;
				}
			}else{
				Show(false);
				Show(true);
				return;
			}
		}else{
			fadeTime -= dltTime;
		}
	}
	Matrix view(false);
	if(isCheckLod)
	{
		const GMXBoundBox & gbb = model->GetLocalBound();
		Vector center = (gbb.vMin + gbb.vMax)*0.5f;
		center = GetMatrix(view)*center;
		float k = (Render().GetView().GetCamPos() - center).GetAttenuation2(lodDistanceMin2, lodDistanceK2);
		if(k < 1.0f)
		{
			alpha = k*fadeAlpha;
			if(!isPrevLodAlpha)
			{
				isPrevLodAlpha = true;
				Show(IsShow());
			}			
		}else{
			alpha = fadeAlpha;
			if(isPrevLodAlpha)
			{
				isPrevLodAlpha = false;
				Show(IsShow());
			}
		}
	}else{
		alpha = fadeAlpha;
	}
	float a = color.a*alpha;
	if(a < 1e-5f)
	{
		return;
	}	
	if(noSwing)
	{
		view = Render().GetView();
		Render().SetView((Mission().GetInverseSwingMatrix()*Matrix(view).Inverse()).Inverse());
	}
	model->SetUserColor(Color(color.r, color.g, color.b, a));

	Matrix drawMtx;
	BuildViewMatrix(drawMtx);

	if(bUseRootBone)
	{
		Matrix mtx;
		GetMatrix(mtx);
		if(events)
		{
			events->Update(mtx, dltTime);
		}
		model->SetTransform(drawMtx);

		FogParamsSave();
		model->Draw();
		FogParamsRestore();

	}else{
		if(events)
		{
			events->Update(drawMtx, dltTime);
		}

		if (bUseDistanceHack)
		{
			Matrix mModelMatrix;
			CalculateGluedMatrix(drawMtx, mModelMatrix);
			model->SetTransform(mModelMatrix);
		} else
		{
			model->SetTransform(drawMtx);
		}

		
		FogParamsSave();
		model->Draw();
		FogParamsRestore();
	}
	if(noSwing)
	{
		Render().SetView(view);	
	}
};




//Нарисовать прозрачную модельку
void _cdecl MissionGeometry::DrawTransparency(float dltTime, long level)
{
	if(!EditMode_IsVisible()) return;
	if(!Mission().EditMode_IsAdditionalDraw()) return;
	if(model)
	{
		Matrix drawMtx;
		BuildViewMatrix(drawMtx);


		if (bUseDistanceHack)
		{
			const Matrix & mProj = Render().GetProjection();
			const GMXBoundBox & bbox = model->GetLocalBound();
			const GMXBoundSphere& bsphere = model->GetLocalBoundSphere();
			Vector vCamPos = Render().GetView().GetCamPos();
			Matrix mModelMatrix;
			CalculateGluedMatrix(drawMtx, mModelMatrix);
			model->SetTransform(mModelMatrix);
		} else
		{
			model->SetTransform(drawMtx);
		}

		if(bUseRootBone && events)
		{
			Matrix mtx;
			GetMatrix(mtx);
			events->Update(mtx, dltTime);			
		}
		model->SetUserColor(Color(color.r, color.g, color.b, color.a*0.1f));

		Matrix view(false);
		if(noSwing)
		{
			view = Render().GetView();
			Render().SetView((Mission().GetInverseSwingMatrix()*Matrix(view).Inverse()).Inverse());
		}

		FogParamsSave();
		model->Draw();
		FogParamsRestore();

		if(noSwing)
		{
			Render().SetView(view);	
		}

		model->SetUserColor(color);
	}
};


//Нарисовать модельку для тени
void _cdecl MissionGeometry::ShadowInfo(const char * group, MissionObject * sender)
{
	if(!EditMode_IsVisible() || !IsShow()) return;
	if(shadow)
	{
		Matrix drawMtx;
		BuildViewMatrix(drawMtx);
		shadow->SetTransform(drawMtx);
		const Vector & vMin = shadow->GetBound().vMin;
		const Vector & vMax = shadow->GetBound().vMax;
		((MissionShadowCaster *)sender)->AddObject(this, &MissionGeometry::ShadowCast, vMin, vMax);
	}else{
		if(model)
		{
			Matrix drawMtx;
			BuildViewMatrix(drawMtx);
			model->SetTransform(drawMtx);
			const Vector & vMin = model->GetBound().vMin;
			const Vector & vMax = model->GetBound().vMax;
			((MissionShadowCaster *)sender)->AddObject(this, &MissionGeometry::ShadowCast, vMin, vMax);
		}
	}
}

//Нарисовать модельку для тени
void _cdecl MissionGeometry::ShadowCast(const char * group, MissionObject * sender)
{	
	if(shadow)
	{
		Matrix drawMtx;
		BuildViewMatrix(drawMtx);
		shadow->SetTransform(drawMtx);
		shadow->Draw();
	}else{
		if(model)
		{
			Matrix drawMtx;
			BuildViewMatrix(drawMtx);
			model->SetTransform(drawMtx);
			model->Draw();
		}
	}
}

//Нарисовать модельку для тени
void _cdecl MissionGeometry::ShadowRecive(const char * group, MissionObject * sender)
{
	if(!EditMode_IsVisible() || !IsShow()) return;
	if(model)
	{
		Matrix drawMtx;
		BuildViewMatrix(drawMtx);
		model->SetTransform(drawMtx);		
		model->Draw();
	}
}

void _cdecl MissionGeometry::SeaReflection(const char * group, MissionObject * sender)
{
	if(!EditMode_IsVisible() || !IsShow()) return;
	if(model)
	{
		Matrix view(false);
		if(noSwing)
		{
			view = Render().GetView();
			Render().SetView((Mission().GetInverseSwingMatrix()*Matrix(view).Inverse()).Inverse());
		}
		Matrix drawMtx;
		BuildViewMatrix(drawMtx);

		if (bUseDistanceHack)
		{
			const Matrix & mProj = Render().GetProjection();
			const GMXBoundBox & bbox = model->GetLocalBound();
			const GMXBoundSphere& bsphere = model->GetLocalBoundSphere();
			Vector vCamPos = Render().GetView().GetCamPos();
			Matrix mModelMatrix;
			CalculateGluedMatrix(drawMtx, mModelMatrix);
			model->SetTransform(mModelMatrix);
		} else
		{
			model->SetTransform(drawMtx);
		}

		
		FogParamsSave();
		model->Draw();
		FogParamsRestore();
		if(noSwing)
		{
			Render().SetView(view);	
		}
	}
}

void _cdecl MissionGeometry::SeaRefraction(const char * group, MissionObject * sender)
{
	if(!EditMode_IsVisible() || !IsShow()) return;
	if(model)
	{
		Matrix view(false);
		if(noSwing)
		{
			view = Render().GetView();
			Render().SetView((Mission().GetInverseSwingMatrix()*Matrix(view).Inverse()).Inverse());
		}	
		Matrix drawMtx;
		BuildViewMatrix(drawMtx);

		if (bUseDistanceHack)
		{
			const Matrix & mProj = Render().GetProjection();
			const GMXBoundBox & bbox = model->GetLocalBound();
			const GMXBoundSphere& bsphere = model->GetLocalBoundSphere();
			Vector vCamPos = Render().GetView().GetCamPos();
			Matrix mModelMatrix;
			CalculateGluedMatrix(drawMtx, mModelMatrix);
			model->SetTransform(mModelMatrix);
		} else
		{
			model->SetTransform(drawMtx);
		}

		
		FogParamsSave();
		model->Draw();
		FogParamsRestore();
		if(noSwing)
		{
			Render().SetView(view);	
		}
	}
}

//Сгенерировать матрицу отрисовки
inline void MissionGeometry::BuildViewMatrix(Matrix & m)
{
	if(!connectToPtr.Validate())
	{
		m = modelMatrix;		
	}else{
		m = modelMatrix*connectToPtr.Ptr()->GetMatrix(Matrix());
	}
}

//Загрузить модельку
bool MissionGeometry::LoadModel(IGMXScene * & m, const ConstString & fileName)
{
	//Создаём новую модельку
	IGMXScene * mdl = Geometry().CreateGMX(fileName.c_str(), &Animation(), &Particles(), &Sound());
	//Удаляем модельку
	if(m) m->Release();
	m = mdl;
	return (m != null);
};

//Удалить модельку
void MissionGeometry::UnloadModel()
{
	if(model) model->Release();
	model = null;
	if(shadow) shadow->Release();
	shadow = null;
};

//Остановить - продолжить проигрывание анимации
void MissionGeometry::PauseAnimation(bool isPause)
{
	if(!model) return;
	IAnimation * ani = model->GetAnimation();
	if(ani)
	{
		ani->Pause(isPause);
		ani->Release();
	}
}

//Получить сцену
IGMXScene * MissionGeometry::GetScene()
{
	return model;
}

void MissionGeometry::CalculateGluedMatrix(const Matrix & matrixIn, Matrix & matrixOut)
{
	float fGlueDistance = GLUE_DISTANCE;
	const Matrix & mProjection = Render().GetProjection();
	const GMXBoundBox & boundBox = model->GetLocalBound();
	const GMXBoundSphere& localBoundSphere = model->GetLocalBoundSphere();
	Vector vCamPos = Render().GetView().GetCamPos();
	Vector vCenter = matrixIn * localBoundSphere.vCenter;
	Vector vDir = vCenter - vCamPos;
	float fDist = vDir.GetLength();
	vDir.y = 0.0f;
	vDir.Normalize();
	float fDistToCompleteVisible = (fGlueDistance-localBoundSphere.fRadius);
	if(fDist < fDistToCompleteVisible)
	{
		matrixOut = matrixIn;
		return;
	}
	Vector4 down = Vector4 (0.0f, boundBox.vMin.y, fDist, 1.0f);
	Vector4 up = Vector4 (0.0f, boundBox.vMax.y, fDist, 1.0f);
	Vector4 down_proj = mProjection.MulVertex(down);
	Vector4 up_proj = mProjection.MulVertex(up);
	down_proj.y = down_proj.y / down_proj.w;
	up_proj.y = up_proj.y / up_proj.w;
	float sizeMustBe = up_proj.y - down_proj.y;
	//ставим нужную дистанцию
	fDist = fDistToCompleteVisible;
	down = Vector4 (0.0f, boundBox.vMin.y, fDist, 1.0f);
	up = Vector4 (0.0f, boundBox.vMax.y, fDist, 1.0f);
	down_proj = mProjection.MulVertex(down);
	up_proj = mProjection.MulVertex(up);
	down_proj.y = down_proj.y / down_proj.w;
	up_proj.y = up_proj.y / up_proj.w;
	float sizeCurrent = up_proj.y - down_proj.y;
	float fScaleK = 0.00000000001f;
	if (sizeCurrent > 0.00001f)
	{
		fScaleK = Clampf (sizeMustBe / sizeCurrent);
	}
	Vector vNewCenter = vCamPos + (vDir * fDistToCompleteVisible);
	vNewCenter.y = vCenter.y;
	//	pRS->DrawSphere(vNewCenter, 100.0f, 0xFFFF0000);
	Vector vDeltaCenter = vCenter - vNewCenter;
	Matrix matTransform = matrixIn;
	matTransform.pos -= vDeltaCenter;
	//Надо матрицу которая центрирует по боксу, скейлит и потом обратно сдвигает...
	Matrix mScale;
	mScale.BuildScale(fScaleK, fScaleK, fScaleK);
	Matrix mWorldToBound;
	mWorldToBound.pos = -localBoundSphere.vCenter;
	Matrix mBoundToWorld;
	mBoundToWorld = mWorldToBound;
	mBoundToWorld.Inverse();
	Matrix mScaleAroundPivot = (mWorldToBound * mScale) * mBoundToWorld;
	matrixOut = mScaleAroundPivot * matTransform;
	//т.к. скейлим относительно центра а не относительно нижней плоскости bound Box пододвигаем...
	matrixOut.pos.y = (matrixIn.pos.y * fScaleK);
}

inline void MissionGeometry::FogParamsSave()
{
	if(bNotUseFog == false) return;
	Render().getFogParams(h_density, h_min, h_max, d_density, d_min, d_max, fog_color);
	Render().setFogParams(0.0f, h_min, h_max, 0.0f, d_min, d_max, fog_color);
}

inline void MissionGeometry::FogParamsRestore()
{
	if (bNotUseFog == false) return;
	Render().setFogParams(h_density, h_min, h_max, d_density, d_min, d_max, fog_color);
}


//============================================================================================
//Параметры инициализации
//============================================================================================

const char * MissionGeometry::comment =
"Commands list:\n"
"----------------------------------------\n"
"  Teleport geometry to position of some\n"
"  mission object\n"
"----------------------------------------\n"
"    command: teleport\n"
"    parm: name of mission object\n"
" \n"
"----------------------------------------\n"
"  Start of any animation node\n"
"----------------------------------------\n"
"    command: startnode\n"
"    parm: name of animation node\n"
"    [parm: blend time in seconds]\n"
"    * if no blend time, then start node without blend\n"
" \n"
"----------------------------------------\n"
"  Move to animation link from\n"
"  current node\n"
"----------------------------------------\n"
"    command: actlink\n"
"    parm: name of animation link\n"
" \n"
"----------------------------------------\n"
"  Fade out model to transparence draw\n"
"----------------------------------------\n"
"    command: fadeout\n"
" \n"
"----------------------------------------\n"
"  Fade in model to solid draw\n"
"----------------------------------------\n"
"    command: fadein\n"
" \n"
"----------------------------------------\n"
"  Fade out model and hide after\n"
"----------------------------------------\n"
"    command: fade\n"
"    [parm: waiting time before fade]\n"
" \n"
"----------------------------------------\n"
"  Change shadow states\n"
"----------------------------------------\n"
"    command: shadow_cast\n"
"    parm: 0 = disable or 1 = enable\n"
" \n"
"    command: shadow_receive\n"
"    parm: 0 = disable or 1 = enable\n"
" \n"
"----------------------------------------\n"
"  Collapse bone\n"
"----------------------------------------\n"
"    command: collapse\n"
"    parm: bone name or postfix bone name\n"
"    [parm: true or false] default is true\n"
" \n"
"----------------------------------------\n"
"  Reset uncliiapse all bones,\n"
"  restart animation\n"
"----------------------------------------\n"
"    command: reset\n"
" \n"
"----------------------------------------\n"
"  Pause animation\n"
"----------------------------------------\n"
"    command: pause\n"
" \n"
"----------------------------------------\n"
"  UnPause animation\n"
"----------------------------------------\n"
"    command: unpause\n"
" \n"
" ";


MOP_BEGINLISTCG(MissionGeometry, "Geometry", '1.00', 0, MissionGeometry::comment, "Geometry")
	MOP_ENUMBEG("PhysGroup")
		for(dword i = 0; i < ARRSIZE(MissionGeometry::pgroups); i++)
		{
			MOP_ENUMELEMENT(MissionGeometry::pgroups[i].name.c_str())
		}
	MOP_ENUMEND
	MOP_STRINGC("Name", "", "Name of model file (.gmx)")
	MOP_STRINGC("Shadow", "", "Name of model file (.gmx) for shadow cast")
	MOP_STRINGC("Animation", "", "Name of animation file (.anx) for replace current animation")
	MOP_POSITIONC("Position", Vector(0.0f), "Model position in world")
	MOP_ANGLESC("Angles", Vector(0.0f), "Model orientation in world")
	MOP_STRINGC("Connect to", "", "Connect geometry to object (not for collision)")
	MOP_BOOLC("Make connection", false, "Make connection to object into Edit Mode")
	MOP_BOOLC("Use root bone", false, "Addition root bone transformation for current. (detector's and other using)")	
	MOP_BOOLC("Show", true, "Show or hide geometry in start mission time")
	MOP_BOOLC("Active (collision)", false, "Wiht active geometry can collision some objects")
	MOP_ENUM("PhysGroup", "Collision groups")
	MOP_GROUPBEG("Render params")
	  MOP_BOOLC("Disable fog", false, "Fog don't affect to this geometry")
		MOP_LONGEXC("Level", 0, 0, 100, "Order of geometry draw")	
		MOP_COLOR("Color", Color(0.0f, 0.0f, 0.0f, 1.0f))	
		MOP_BOOLC("Smooth alpha", false, "Disable alpha test (Make smooth alpha)")
		MOP_BOOLC("Render", true, "Enable primary render")
		MOP_BOOLC("Transparency", false, "Geometry draw as transparency (i.e. light rays)")
		MOP_BOOL("Dynamic lighting", false)
		MOP_BOOLC("Shadow cast", false, "Geometry can is shadow cast by some objects")
		MOP_BOOLC("Shadow receive", false, "Geometry can is shadow receive from casting objects")
		MOP_BOOLC("Sea reflection", false, "Geometry can reflect in sea")		
		MOP_BOOLC("Sea refraction", false, "Geometry can refract in sea")		
		MOP_BOOLC("Hide in editor", true, "Hide invisible geometry in editor")				
		MOP_BOOLC("No swing", false, "No swing geometry in swing machine")
		MOP_FLOATC("Hide distance", -1.0f, "If distance less 0, then ignore this feature")
		MOP_FLOATEXC("Fade distance", 5.0f, 0.001f, 1000.0f, "When distance for camera great then Hide + Fade, geometry is hidden")
		MOP_BOOLC("Distance hack", false, "Use distance hack to far-far-far-far-far objects")
MOP_GROUPEND()
	
MOP_ENDLIST(MissionGeometry)

