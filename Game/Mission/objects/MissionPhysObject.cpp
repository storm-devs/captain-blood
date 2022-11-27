

#include "MissionPhysObject.h"

#define FADEOUT_TIME	1.0f


MissionPhysObject::MissionPhysObject() : solid(_FL_),
										 broken(_FL_)
//										 ,lines(_FL_)
{


	HP = 0.0f;
	flags = f_needCalcBox | f_isShowTips;
	hideTime = 3.0f;
	chacheIndex = -1;
	visAbb.min = visAbb.max = 0.0f;
	colAbb.min = colAbb.max = 0.0f;	
	aiColider = null;
}

MissionPhysObject::~MissionPhysObject()
{
	Release();
	if(EditMode_IsOn() && pattern.Validate())
	{
		pattern.Ptr()->regObjects.Del(this);
	}
	if(aiColider)
	{
		aiColider->Release();
	}
}

//Инициализировать объект
bool MissionPhysObject::Create(MOPReader & reader)
{
	DamageReceiver::Create(reader);
	if(!aiColider)
	{
		aiColider = QTCreateObject(MG_AI_COLLISION, _FL_);
	}
	aiColider->Activate(false);
	ResetFlag(flags, f_isFadeProcess | f_isShock);
	ReadParameters(reader);
	brokeEvent.Init(reader);
	if(reader.Bool())
	{
		SetFlag(flags, f_drawDebugBoxes);
	}else{
		ResetFlag(flags, f_drawDebugBoxes);
	}
	if(!pattern.Validate()) return false;
	UpdatePatternData();
	FindVisibleABB(solid, visAbb);
	Show(IsShow());
	return true;
}

//Пересоздать объект
void MissionPhysObject::Restart()
{
	Activate(false);
	SetShow(false);
	DelUpdate(&MissionPhysObject::HideTimer);
	DelUpdate(&MissionPhysObject::Fader);	
	Release();
	if(pattern.Validate())
	{
		pattern.Ptr()->regObjects.Del(this);
	}
	HP = 0.0f;
	flags = f_needCalcBox;
	hideTime = 3.0f;
	pattern.Reset();
	chacheIndex = -1;
	visAbb.min = visAbb.max = 0.0f;
	colAbb.min = colAbb.max = 0.0f;
	waterLevel.Reset();
	ReCreate();
}

//Инициализировать объект
bool MissionPhysObject::EditMode_Create(MOPReader & reader)
{
	SetUpdate(&MissionPhysObject::EditMode_Draw, ML_GEOMETRY2);
	return EditMode_Update(reader);
}

//Обновить параметры
bool MissionPhysObject::EditMode_Update(MOPReader & reader)
{
	//Удаляем всё
	Release();
	pattern.Reset();
	//Читаем и заводим заново
	ReadParameters(reader);
	UpdatePatternPointer();
	UpdatePatternData();
	return true;
}

//Получить размеры описывающего ящика
void MissionPhysObject::EditMode_GetSelectBox(Vector & min, Vector & max)
{
	ABB abb; abb.min = abb.max = 0.0f;
	bool isAdd = false;
	for(long i = 0; i < solid; i++)
	{
		Part & part = solid[i];
		if(part.model)
		{
			const GMXBoundBox & bbx = part.model->GetBound();
			if(isAdd)
			{
				min.Min(min, bbx.vMin);
				max.Min(max, bbx.vMax);
			}else{
				min = bbx.vMin;
				max = bbx.vMax;
				isAdd = true;
			}
			/*
			MissionPhysObjPattern::PatternObject & po = pattern.Ptr()->objects[part.descIndex];
			const GMXBoundBox & bbx = part.model->GetLocalBound();
			Box::FindABBforOBB(po.mtx, bbx.vMin, bbx.vMax, abb.min, abb.max, isAdd);
			isAdd = true;
			*/			
		}
	}
	/*
	min = abb.min;
	max = abb.max;
	*/
}

//Получить бокс, описывающий объект в локальных координатах
void MissionPhysObject::GetBox(Vector & min, Vector & max)
{
	min = visAbb.min;
	max = visAbb.max;
}

//Показать/скрыть объект
void MissionPhysObject::Show(bool isShow)
{	
	if(EditMode_IsOn())
	{
		MissionObject::Show(isShow);
		return;
	}
	MissionObject::Show(isShow);
	SetShow(isShow);
	LogicDebug(isShow ? "Show" : "Hide");
	Activate(IsActive());	
	ShowTips();
}

//Устоновить состояние подписки
void MissionPhysObject::SetShow(bool isShow)
{
	if(!pattern.Validate()) return;
	if(isShow)
	{
		DelUpdate(&MissionPhysObject::Draw);
		SetUpdate(&MissionPhysObject::Draw, pattern.Ptr()->level);
		if(CheckFlag(flags, f_isFadeProcess | f_isCheckLod))
		{
			SetUpdate(&MissionPhysObject::Draw, pattern.Ptr()->level + (ML_ALPHA1 + 1) - ML_GEOMETRY1);
		}		
		if(pattern.Ptr()->shadowCast)
		{
			Registry(MG_SHADOWCAST, (MOF_EVENT)&MissionPhysObject::ShadowInfo, pattern.Ptr()->level);
		}else{
			Unregistry(MG_SHADOWCAST);
		}
		if(pattern.Ptr()->shadowReceive)
		{
			Unregistry(MG_SHADOWDONTRECEIVE);
			Registry(MG_SHADOWRECEIVE, (MOF_EVENT)&MissionPhysObject::ShadowDraw, pattern.Ptr()->level);
		}else{
			Registry(MG_SHADOWDONTRECEIVE, (MOF_EVENT)&MissionPhysObject::ShadowDraw, pattern.Ptr()->level);
			Unregistry(MG_SHADOWRECEIVE);
		}
		if(pattern.Ptr()->seaReflection)
		{
			Registry(MG_SEAREFLECTION, (MOF_EVENT)&MissionPhysObject::SeaReflection, pattern.Ptr()->level);
		}else{
			Unregistry(MG_SEAREFLECTION);
		}
	}else{
		DelUpdate(&MissionPhysObject::Draw);
		Unregistry(MG_SHADOWCAST);
		Unregistry(MG_SHADOWRECEIVE);
		Unregistry(MG_SEAREFLECTION);
		Unregistry(MG_SHADOWDONTRECEIVE);
	}
}

//Обновить состояние типсов
void MissionPhysObject::ShowTips()
{
	bool currentShow = IsShow() && CheckFlag(flags, f_isShowTips);
	SetTipsState(solid, currentShow && !CheckFlag(flags, f_isBroken));
	SetTipsState(broken, currentShow && CheckFlag(flags, f_isBroken));
}

//Установить состояние подсказки
void MissionPhysObject::SetTipsState(array<Part> & obj, bool isActive)
{
	for(long i = 0; i < obj; i++)
	{
		Part & part = obj[i];
		if(part.tip)
		{
			if(isActive)
			{
				part.tip->Activate(true);
			}else{
				part.tip->Activate(false);
			}
			
		}
	}
}

//Активировать/деактивировать объект
void MissionPhysObject::Activate(bool isActive)
{
	if(EditMode_IsOn())
	{
		MissionObject::Activate(isActive);
		return;
	}
	MissionObject::Activate(isActive);
	if(!IsShow())
	{
		isActive = false;
	}
	finder->Activate(isActive);
	if(isActive && !CheckFlag(flags, f_isBroken) && (solid > 0) && pattern.Validate() && !EditMode_IsOn())
	{
		MissionPhysObjPattern::PatternObject & po = pattern.Ptr()->objects[solid[0].descIndex];
		aiColider->SetBox(po.min, po.max);
		aiColider->SetMatrix(solid[0].mtx);
		aiColider->Activate(true);
	}else{
		aiColider->Activate(false);
	}
	
	if(isActive)
	{
		SetUpdate(&MissionPhysObject::Update, ML_EXECUTE1);
		ApplyDamage(0.0f, dt_unknown);
		if(!CheckFlag(flags, f_isBroken))
		{
			MakePhysics(solid);
		}else{
			MakePhysics(broken);
			if(!CheckFlag(flags, f_isFadeProcess))
			{
				SetUpdate(&MissionPhysObject::HideTimer, ML_EXECUTE1);
			}else{
				if(!CheckFlag(flags, f_isDead))
				{
					SetUpdate(&MissionPhysObject::Fader, ML_EXECUTE1);
				}else{
					hideTime = 1.0f;
					Fader(0.0f, 0);
				}
			}
		}
		LogicDebug("Activate");
	}else{
		DelUpdate(&MissionPhysObject::Update);
		DelUpdate(&MissionPhysObject::HideTimer);
		DelUpdate(&MissionPhysObject::Fader);
		ReleasePhysics(solid);
		ReleasePhysics(broken);	
		LogicDebug("Deactivate");
	}
}

//Получить матрицу объекта
Matrix & MissionPhysObject::GetMatrix(Matrix & mtx)
{
	return mtx.SetIdentity();
}

//Обработчик команд для объекта
void MissionPhysObject::Command(const char * id, dword numParams, const char ** params)
{
	if(string::IsEqual(id, "broke"))
	{
		if(!CheckFlag(flags, f_isBroken))
		{
			Broke();
			if(numParams >= 2)
			{
				MOSafePointer mo;
				if(FindObject(ConstString(params[0]), mo))
				{
					Matrix mtx;
					mo.SPtr()->GetMatrix(mtx);
					char * endPtr = null;
					float forceMin = (float)strtod(params[1], &endPtr);
					float forceMax = forceMin*1.2f;
					if(numParams >= 3)
					{
						forceMax = (float)strtod(params[2], &endPtr);;
					}
					for(long i = 0; i < broken; i++)
					{
						Part & part = broken[i];
						Vector dir = part.mtx.pos - mtx.pos;
						dir.Normalize();
						Vector vforce(RRnd(forceMin, forceMax), RRnd(forceMin, forceMax), RRnd(forceMin, forceMax));
						part.impDir += dir*vforce;
						part.count++;
					}
					LogicDebug("Command: broke, take force [%f, %f] from object %s", forceMin, forceMax, params[0]);
				}else{
					LogicDebugError("Command: broke, object %s not found", params[0]);
				}
			}else{
				LogicDebug("Command: broke, without parameters");
			}
			ApplyImpulses(dt_unknown);
		}
	}else
	if(string::IsEqual(id, "shock"))
	{
		if(numParams >= 3)
		{
			char * endPtr = null;
			float time = (float)strtod(params[0], &endPtr);
			float force = (float)strtod(params[1], &endPtr);
			float period = (float)strtod(params[1], &endPtr);
			shockTime = Clampf(time, 0.0f, 10.0f);
			shockForce = Clampf(force, 0.0f, 100.0f);
			shockWaitTimeMid = Clampf(period, 0.0f, 10.0f);
			LogicDebug("Command: shock (set:%f, def:%f) (set:%f, def:%f) (set:%f, def:%f)", shockTime, time, shockForce, force, shockWaitTimeMid, period);
			if(shockTime > 1e-5f)
			{
				kShockTime = 1.0f/shockTime;
			}else{
				kShockTime = 0.0f;
			}
			shockTime = 0.0f;
			shockWaitTime = 0.0f;
			SetFlag(flags, f_isShock);
		}else{
			LogicDebugError("Command: shock, not enough parameters: shock 1:time 2:force");
		}
	}else
	if(string::IsEqual(id, "hackhideon"))
	{
		LogicDebug("Command: hackhideon !!!");
		SetFlag(flags, f_hackHide);
	}else
	if(string::IsEqual(id, "hackhideoff"))	
	{	
		LogicDebug("Command: hackhideoff");
		ResetFlag(flags, f_hackHide);
	}else{
		LogicDebugError("Unknown command: %s", id);
	}
}

//Обновление позиций
void _cdecl MissionPhysObject::Update(float dltTime, long level)
{
	array<Part> & parts = CheckFlag(flags, f_isBroken) ? broken : solid;
	if(CheckFlag(flags, f_isShock) && dltTime > 0.0f)
	{
		if(shockWaitTime <= 1e-10f)
		{
			float force = shockForce*RRnd(1.0f - shockTime, 1.0f);
			for(long i = 0; i < parts; i++)
			{
				Part & part = parts[i];
				if(part.physObject)
				{
					part.physObject->ApplyImpulse(Vector(0.0f, force, 0.0f), Vector(0.0f));
				}
			}
			shockWaitTime = shockWaitTimeMid*RRnd(0.8f, 1.2f);
		}else{
			shockWaitTime -= dltTime;
		}
		shockTime -= dltTime;
		if(shockTime <= 0.0f)
		{
			ResetFlag(flags, f_isShock);
		}
	}
	float wlevel = IWaterLevel::GetWaterLevel(Mission(), waterLevel);
	for(long i = 0; i < parts; i++)
	{
		Part & part = parts[i];
		if(part.sndMatMask)
		{
			part.sndCooldown -= dltTime;
			if(part.sndCooldown <= 0.0f)
			{
				part.sndMatMask = 0;
			}
		}
		if(part.physObject)
		{
			MissionPhysObjPattern::PatternObject & po = pattern.Ptr()->objects[part.descIndex];
			const Vector & com = po.centerOfMass;
			Vector curPos = part.mtx*com;
			part.physObject->GetTransform(part.mtx);
			if(part.model)
			{
				part.model->SetTransform(part.mtx);
				long mtlId = (long)part.physObject->GetContactReport();
				if(mtlId > pmtlid_air && mtlId <= pmtlid_fabrics)
				{
					//Попали в звучащий материал
					dword mask = 1 << mtlId;
					if((part.sndMatMask & mask) == 0)
					{
						part.sndCooldown = 0.8f;
						part.sndMatMask |= mask;
						Vector pos = part.physObject->GetContactPoint();
						float force = part.physObject->GetContactForce().GetLength2();
						pattern.Ptr()->PlayCollisionEffect(mtlId, pos, force);
					}
				}
			}
			if(part.tip)
			{
				part.tip->SetPos(part.mtx*po.tipPosition);
			}
			Vector newPos = part.mtx*com;
			if(newPos.y <= wlevel && curPos.y > wlevel)
			{
				WaterDrop(part, curPos, newPos, wlevel);
			}
			if(part.mtx.pos.y < -800.0f)
			{
				if(part.physObject)
				{
					part.physObject->Release();
					part.physObject = null;
				}
				if(part.model)
				{
					part.model->Release();
					part.model = null;
				}
				ResetFlag(part.flags, f_part_isCollision);
			}
		}
	}
	FindCollidersABB(parts, colAbb);
	finder->SetBox(colAbb.min, colAbb.max);	
	FindVisibleABB(parts, visAbb);
	if(!CheckFlag(flags, f_isBroken) && solid > 0)
	{
		aiColider->SetMatrix(solid[0].mtx);
	}
}


//Отиграть эффект падения в воду
void MissionPhysObject::WaterDrop(Part & part, Vector & pos1, Vector & pos2, float level)
{
	const ConstString & prtName = pattern.Ptr()->objects[part.descIndex].waterParticle;
	const ConstString & sndName = pattern.Ptr()->objects[part.descIndex].waterSound;
	float k = (pos1.y - pos2.y);
	if(k > 0.0f)
	{
		k = (level - pos2.y)/k;
	}else{
		k = 0.0f;
	}
	Matrix mtx(0.0f, Rnd(2.0f*PI), 0.0f, pos2.x + (pos1.x - pos2.x)*k, level, pos2.z + (pos1.z - pos2.z)*k);
	if(prtName.NotEmpty())
	{
		Particles().CreateParticleSystemEx2(prtName.c_str(), mtx, true, _FL_);
	}
	if(sndName.NotEmpty())
	{
		Sound().Create3D(sndName, mtx.pos, _FL_);
	}
}

//Нарисовать модельку
void _cdecl MissionPhysObject::Draw(float dltTime, long level)
{
	Assert(pattern.Validate());
	array<Part> & parts = CheckFlag(flags, f_isBroken) ? broken : solid;
	bool haveSomeModels = false;
	Vector cam = Render().GetView().GetCamPos();
	Color userColor(0.0f, 0.0f, 0.0f, 1.0f);
	for(long i = 0; i < parts; i++)
	{
		Part & part = parts[i];
		if(part.model)
		{
			haveSomeModels = true;
			if(level == pattern.Ptr()->level)
			{
				if(CheckFlag(flags, f_isCheckLod))
				{
					const GMXBoundBox & gbb = part.model->GetBound();
					Vector center = (gbb.vMin + gbb.vMax)*0.5f;
					float k = (center - cam).GetAttenuation2(pattern.Ptr()->hideDistanceMin2, pattern.Ptr()->hideDistanceK2);
					if(k < 1.0f)
					{
						k *= part.alpha;
						if(k < 1e-5f)
						{
							continue;
						}
						userColor.a = k;
						part.model->SetUserColor(userColor);
					}else{
						userColor.a = part.alpha;
						part.model->SetUserColor(userColor);
					}
				}
				if(part.model->GetUserColor().a > 0.9999f)
				{
					if(!CheckFlag(flags, f_hackHide))
					{
						part.model->Draw();
					}					
					ResetFlag(part.flags, f_part_isDrawInAlphaLevel);
				}else{
					SetFlag(part.flags, f_part_isDrawInAlphaLevel);
				}
			}else{
				if(CheckFlag(part.flags, f_part_isDrawInAlphaLevel))
				{
					if(!CheckFlag(flags, f_hackHide))
					{
						part.model->Draw();
					}
				}
			}			
		}
	}
	if(level == pattern.Ptr()->level)
	{
		if(CheckFlag(flags, f_drawDebugBoxes))
		{
			for(long i = 0; i < parts; i++)
			{
				Part & part = parts[i];
				MissionPhysObjPattern::PatternObject & po = pattern.Ptr()->objects[part.descIndex];
				for(long j = 0; j < po.boxes; j++)
				{
					MissionPhysObjPattern::CollisionBox & box = po.boxes[j];
					Matrix mtx(box.mtx, part.mtx);
					Render().DrawBox(-box.size05, box.size05, mtx);
				}
				for(long j = 0; j < po.capsules; j++)
				{
					MissionPhysObjPattern::CollisionCapsule & cap = po.capsules[j];
					Matrix mtx(cap.mtx, part.mtx);
					Render().DrawBox(-cap.size05, cap.size05, mtx);
				}
			}
		}
		if(!haveSomeModels && !CheckFlag(flags, f_drawDebugBoxes))
		{
			SetShow(false);
		}
	}

/*
//	Render().DrawBox(visAbb.min, visAbb.max);
	Render().DrawBox(colAbb.min, colAbb.max);


	static Vector poly[4] = 
						{	
							Vector(-1.0f, 0.0f, 0.5f), 
							Vector(1.2f, 0.0f, 1.0f), 
							Vector(0.7f, 0.0f, -1.0f), 
							Vector(-1.0f, 0.0f, -1.0f)
						};

	static Vector p[4];

	static float cnt = 0.0f;

	cnt += dltTime;


//#define CAMERA

#ifndef CAMERA
	if(api->DebugKeyState(VK_SPACE)) cnt = 0.0f;
	if(cnt > 0.5f)
	{
		cnt = 0.0f;
		Matrix mtx(Vector().Rand(Vector(-PI), Vector(PI)), Vector().Rand(Vector(-4.0f), Vector(4.0f)));
#else
	if(api->DebugKeyState(VK_SPACE))
	{
		Matrix mtx = Render().GetView();
		mtx.Inverse();
#endif
		for(long i = 0; i < 4; i++)
		{
			p[i] = poly[i]*mtx;
		}
	}

	Vector size05(1.0f, 0.6f, 0.4f);

	
	bool bs = OverlapsBoxSphere(Matrix(), size05, p[0], 1.0f);
	bool bl = OverlapsBoxLine(Matrix(), size05, p[0], p[1]);
	bool bp = OverlapsBoxPoly(Matrix(), size05, p);
	
	//Render().DrawSphere(p[0], 1.0f, bs ? 0xffff0000 : 0xff0000ff);
	Render().DrawLine(p[0], bl ? 0xffff0000 : 0xff0000ff, p[1], bl ? 0xffff0000 : 0xff0000ff);
//	Render().DrawPolygon(p, 4, bp ? 0xffff0000 : 0xff0000ff);

	Render().DrawBox(-size05, size05);
	//*/

/*	Render().FlushBufferedLines();
	for(dword i = 0; i < lines.Size(); i++)
	{
		DLine & dl = lines[i];
		//Render().DrawBufferedLine(dl.from, dl.cf, dl.to, dl.ct);
		Render().DrawLine(dl.from, dl.cf, dl.to, dl.ct);
	}
	Render().FlushBufferedLines();
  */

//	Render().DrawBox(aiColider.GetBoxCenter() - aiColider.GetBoxSize05(), aiColider.GetBoxCenter() + aiColider.GetBoxSize05(), aiColider.GetMatrix());

}

//Нарисовать модельку
void _cdecl MissionPhysObject::EditMode_Draw(float dltTime, long level)
{
	/*
	Vector vmin, vmax;
	EditMode_GetSelectBox(vmin, vmax);
	Render().DrawBox(vmin, vmax, Matrix(), 0xffff0000);
	*/
	if(!EditMode_IsVisible())
	{
		return;
	}
	if(!IsShow())
	{
		return;
	}
	if(!pattern.Validate())
	{
		UpdatePatternPointer();
		if(!pattern.Validate())
		{
			return;
		}
		UpdatePatternData();
	}
	for(long i = 0; i < solid; i++)
	{
		if(solid[i].model)
		{
			solid[i].model->Draw();
		}
	}
}

//Нарисовать модельку для тени
void _cdecl MissionPhysObject::ShadowInfo(const char * group, MissionObject * sender)
{
	if(CheckFlag(flags, f_hackHide))
	{
		return;
	}
	((MissionShadowCaster *)sender)->AddObject(this, &MissionPhysObject::ShadowDraw, visAbb.min, visAbb.max);
}

//Нарисовать модельку для тени
void _cdecl MissionPhysObject::ShadowDraw(const char * group, MissionObject * sender)
{
	if(CheckFlag(flags, f_hackHide))
	{
		return;
	}
	Assert(pattern.Validate());
	array<Part> & parts = CheckFlag(flags, f_isBroken) ? broken : solid;
	for(long i = 0; i < parts; i++)
	{
		Part & part = parts[i];
		if(part.model)
		{
			part.model->Draw();
		}
	}
}

//Нарисовать отражёную модельку
void _cdecl MissionPhysObject::SeaReflection(const char * group, MissionObject * sender)
{
	ShadowDraw(null, null);
}

//Отсчёт исчезновения
void _cdecl MissionPhysObject::HideTimer(float dltTime, long level)
{
	brokeEvent.Activate(Mission());
	hideTime -= dltTime;
	if(hideTime > 0.0f) return;
	hideTime = 0.0f;
	DelUpdate(&MissionPhysObject::HideTimer);
	SetUpdate(&MissionPhysObject::Fader, ML_EXECUTE1);
	SetFlag(flags, f_isFadeProcess);
	LogicDebug("Start fade process");
	SetShow(IsShow());
}

//Процесс исчезновения, или другого окончания объекта
void _cdecl MissionPhysObject::Fader(float dltTime, long level)
{
	hideTime += dltTime*(1.0f/FADEOUT_TIME);
	if(hideTime > 1.0f)
	{
		SetFlag(flags, f_isDead);
		hideTime = 1.0f;
		DelUpdate(&MissionPhysObject::Fader);
	}
	bool isSomeVisible = false;
	Color userColor(0.0f, 0.0f, 0.0f, 1.0f - hideTime);
	for(long i = 0; i < broken; i++)
	{
		Part & part = broken[i];
		MissionPhysObjPattern::PatternObject & po = pattern.Ptr()->objects[part.descIndex];
		if(po.fe == MissionPhysObjPattern::fe_hide)
		{
			if(!CheckFlag(flags, f_isDead))
			{
				if(part.model)
				{
					part.model->SetUserColor(userColor);
					part.alpha = userColor.a;
					isSomeVisible = true;
				}					
			}else{
				LogicDebug("Object's part %i is faded", i);
				if(part.model)
				{
					part.model->Release();
					part.model = null;
				}
				if(part.physObject)
				{
					part.physObject->Release();
					part.physObject = null;
				}
				ResetFlag(part.flags, f_part_isCollision);
			}			
		}else
		if(po.fe == MissionPhysObjPattern::fe_static)
		{
			isSomeVisible = true;
			if(CheckFlag(flags, f_isDead))
			{
				LogicDebug("Object's part %i is modify to static", i);
				if(part.physObject)
				{
					part.physObject->Release();
					part.physObject = null;
				}
			}
		}else{
			isSomeVisible = true;
		}
	}
	if(!isSomeVisible)
	{
		LogicDebug("No more visible parts, object off");
		Show(false);
	}
}

//Воздействовать на объект сферой
bool MissionPhysObject::Attack(MissionObject * obj, dword source, float hp, const Vector & center, float radius)
{
	if(!pattern.Validate()) return false;
	if(!IsActive()) return false;
	DamageType dtype = DetectType(source);
	if(dtype == dt_pickup)
	{
		if(!pattern.Ptr()->isPickup || CheckFlag(flags, f_isBroken) || solid > 1)
		{
			return false;
		}
	}
	hp = ModifyHp(dtype, hp);
	array<Part> & parts = CheckFlag(flags, f_isBroken) ? broken : solid;
	bool isHit = false;
	float dmg = 0.0f;
	for(long i = 0; i < parts; i++)
	{
		Part & part = parts[i];
		part.impPos = 0.0f;
		part.impDir = 0.0f;
		part.count = 0;
		part.effectsPos = 0.0f;
		part.effectsCount = 0;
		if(!CheckFlag(part.flags, f_part_isCollision))
		{
			continue;
		}
		MissionPhysObjPattern::PatternObject & po = pattern.Ptr()->objects[part.descIndex];
		bool isApplyDmg = false;
		for(long j = 0; j < po.boxes; j++)
		{
			MissionPhysObjPattern::CollisionBox & box = po.boxes[j];
			Matrix mtx(box.mtx, part.mtx);
			if(Box::OverlapsBoxSphere(mtx, box.size05, center, radius))
			{
				isApplyDmg = true;
				ApplyImpulse(part, mtx, center, radius, hp, dtype);
			}
		}
		for(long j = 0; j < po.capsules; j++)
		{
			MissionPhysObjPattern::CollisionCapsule & cap = po.capsules[j];
			Matrix mtx(cap.mtx, part.mtx);
			if(Box::OverlapsBoxSphere(mtx, cap.size05, center, radius))
			{
				isApplyDmg = true;
				ApplyImpulse(part, mtx, center, radius, hp, dtype);
			}
		}
		if(isApplyDmg)
		{
			dmg += hp;
			isHit = true;
		}
	}
	ApplyDamage(dmg, dtype);
	ApplyImpulses(dtype);
	return isHit;
}

__forceinline void MissionPhysObject::ApplyImpulse(Part & p, const Matrix & boxTransform, const Vector & spherePos, float sphereRadius, float hp, DamageType dtype)
{
	//Вычисляем направление импульса
	Vector dir = boxTransform.pos - spherePos;
	float dist = dir.Normalize();
	if(dist < 0.01f)
	{
		dir.Rand();
	}
	//Слегка подбросим вверх
	dir.y += RRnd(0.1f, 0.2f);
	dir.Normalize();
	//Считаем коэфициент затухания в зависимости от удалённости от центра
	float k;
	if(sphereRadius > 1e-10f)
	{
		k = 1.0f - Clampf(dist/sphereRadius);
		k = powf(k, 0.3f);
	}else{
		k = 0.0f;
	}
	//Прикладываем импульс к части
	ApplyImpulseResult(p, boxTransform.MulVertexByInverse(spherePos), dir, k, hp, dtype, spherePos);
}

//Воздействовать на объект линией
bool MissionPhysObject::Attack(MissionObject * obj, dword source, float hp, const Vector & from, const Vector & to)
{
//	lines.Add(DLine(from, 0xff00ffff, to, 0xff0000ff));


	if(!pattern.Validate()) return false;
	if(!IsActive()) return false;
	DamageType dtype = DetectType(source);
	hp = ModifyHp(dtype, hp);
	array<Part> & parts = CheckFlag(flags, f_isBroken) ? broken : solid;
	bool isHit = false;
	float dmg = 0.0f;
	for(long i = 0; i < parts; i++)
	{
		Part & part = parts[i];
		part.impPos = 0.0f;
		part.impDir = 0.0f;
		part.count = 0;
		part.effectsPos = 0.0f;
		part.effectsCount = 0;
		if(!CheckFlag(part.flags, f_part_isCollision))
		{
			continue;
		}
		MissionPhysObjPattern::PatternObject & po = pattern.Ptr()->objects[part.descIndex];
		bool isApplyDmg = false;
		for(long j = 0; j < po.boxes; j++)
		{
			MissionPhysObjPattern::CollisionBox & box = po.boxes[j];
			Matrix mtx(box.mtx, part.mtx);
			if(Box::OverlapsBoxLine(mtx, box.size05, from, to))
			{
				isApplyDmg = true;
				ApplyImpulse(part, mtx, from, to , hp, dtype);
			}
		}
		for(long j = 0; j < po.capsules; j++)
		{
			MissionPhysObjPattern::CollisionCapsule & cap = po.capsules[j];
			Matrix mtx(cap.mtx, part.mtx);
			if(Box::OverlapsBoxLine(mtx, cap.size05, from, to))
			{
				isApplyDmg = true;
				ApplyImpulse(part, mtx, from, to , hp, dtype);
			}
		}
		if(isApplyDmg)
		{
			dmg += hp;
			isHit = true;
		}
	}
	ApplyDamage(dmg, dtype);
	ApplyImpulses(dtype);
	return isHit;
}

void MissionPhysObject::ApplyImpulse(Part & p, const Matrix & boxTransform, const Vector & from, const Vector & to, float hp, DamageType dtype)
{
	if(dtype == dt_sword)
	{
		Vector middle = (from + to)*0.5f;
		Vector dir = boxTransform.pos - middle;
		float dist = dir.Normalize();
		float k;
		if(dist < 1e-10f)
		{
			dir.Rand();
			k = 1.0f;
		}else{
			k = 1.0f/(1.0f + dist);
		}
		ApplyImpulseResult(p, boxTransform.MulVertexByInverse(middle), dir, k, hp, dtype, middle);
		return;
	}
	Vector dir = to - from;
	float distDir = dir.Normalize();
	ApplyImpulseResult(p, boxTransform.MulVertexByInverse(from), dir, 1.0f, hp, dtype, from);
}

//Воздействовать на объект выпуклым чехырёхугольником
bool MissionPhysObject::Attack(MissionObject * obj, dword source, float hp, const Vector vrt[4])
{
	if(!pattern.Validate()) return false;
	if(!IsActive()) return false;
	DamageType dtype = DetectType(source);	
	array<Part> & parts = CheckFlag(flags, f_isBroken) ? broken : solid;
	bool isHit = false;
	float dmg = 0.0f;
	for(long i = 0; i < parts; i++)
	{
		Part & part = parts[i];
		part.impPos = 0.0f;
		part.impDir = 0.0f;
		part.count = 0;
		part.effectsPos = 0.0f;
		part.effectsCount = 0;
		if(!CheckFlag(part.flags, f_part_isCollision))
		{
			continue;
		}
		MissionPhysObjPattern::PatternObject & po = pattern.Ptr()->objects[part.descIndex];
		bool isApplyDmg = false;
		for(long j = 0; j < po.boxes; j++)
		{
			MissionPhysObjPattern::CollisionBox & box = po.boxes[j];
			Matrix mtx(box.mtx, part.mtx);
			if(Box::OverlapsBoxPoly(mtx, box.size05, vrt))
			{
				isApplyDmg = true;
				ApplyImpulse(part, mtx, vrt, hp, dtype);
			}
		}
		for(long j = 0; j < po.capsules; j++)
		{
			MissionPhysObjPattern::CollisionCapsule & cap = po.capsules[j];
			Matrix mtx(cap.mtx, part.mtx);
			if(Box::OverlapsBoxPoly(mtx, cap.size05, vrt))
			{
				isApplyDmg = true;
				ApplyImpulse(part, mtx, vrt, hp, dtype);
			}
		}
		if(isApplyDmg)
		{
			dmg += ModifyHp(dtype, hp);
			isHit = true;
		}
	}
	ApplyDamage(dmg, dtype);
	ApplyImpulses(dtype);
	return isHit;
}

__forceinline void MissionPhysObject::ApplyImpulse(Part & p, const Matrix & boxTransform, const Vector vrt[4], float hp, DamageType dtype)
{	
	//Направление силы в направлении тьраектории движения	
	Vector dir = vrt[1] - vrt[2] + vrt[0] - vrt[3];	
	dir.Normalize();
	//Позиция определяется серединой оружия на передней кромке
	Vector pos = (vrt[0] + vrt[1])*0.5f;
	//На силу влияет только наносимые повреждения
	ApplyImpulseResult(p, boxTransform.MulVertexByInverse(pos), dir, 1.0f, hp, dtype, pos);
}

//Применить импульс к части зная параметры импульса
__forceinline void MissionPhysObject::ApplyImpulseResult(Part & p, const Vector & impulsePos, const Vector & impulseDir, float kAttenuation, float hp, DamageType dtype, const Vector & effectsPos)
{
	//Добавляем эффект
	p.effectsPos += effectsPos;
	p.effectsCount++;
	//Модификатор импульса в зависимости от источника демеджа
	/*
	if(hp <= 1e-10f || kAttenuation <= 1e-10f)
	{
		return;
	}*/
	float force = 1.0f;
	switch(dtype)
	{
		case dt_sword:
			force = 1.0f;
			break;
		case dt_bomb:
			//force = 6.0f;			
			force = 0.1f*Clampf(fabsf(hp)*RRnd(0.9f, 1.1f), 1.0f, 500.0f);
			break;
		case dt_bullet:
			force = 5.0f;
			break;
		case dt_cannon:
			//force = 6.0f;
			force = 0.1f*Clampf(fabsf(hp)*RRnd(0.9f, 1.1f), 1.0f, 500.0f);
			break;
		case dt_flame:
			force = 1.0f;
			break;
		case dt_shooter:
			force = 6.0f;
			break;
		case dt_unknown:
			force = 1.0f;
			break;
		case dt_pickup:
			force = 0.0f;
			break;
	default:
		force = 0.0f;
	}

/*	//Вычислим силу в зависимости от наносимого повреждения
	force = Clampf(fabsf(hp)*RRnd(0.9f, 1.1f), 1.0f, 500.0f);
	//Добавляем импульс к объекту
	p.impPos += impulsePos;
	p.impDir += impulseDir*(force*modifier*kAttenuation); //Уберём ослабление
*/

	

	//Вычислим силу в зависимости от наносимого повреждения
	p.impPos += impulsePos;

	if (dtype == dt_bullet)
	{
		Vector dir;
		dir.Rand(0.0f,1.0f);
		dir.y = fabs(dir.y);

		p.impDir += dir*(force*RRnd(0.9f, 1.1f));
	}
	else
	{
		p.impDir += impulseDir*(force*RRnd(0.9f, 1.1f));
	}

	p.count++;

//	lines.Add(DLine(p.effectsPos, 0xff00ffff, p.effectsPos + impulseDir, 0xffff00ff));
}

//Применить импульсы
__forceinline void MissionPhysObject::ApplyImpulses(DamageType type)
{
	if(type == dt_check)
	{
		return;
	}
	array<Part> & parts = CheckFlag(flags, f_isBroken) ? broken : solid;
	for(long i = 0; i < parts; i++)
	{
		Part & part = parts[i];
		if(part.count && part.physObject)
		{
			float kNorm = 1.0f/float(part.count);
			//Усредняем позицию и импульс по количеству приложенных
			part.impPos *= kNorm;
			part.impDir *= kNorm;
			//Ограничиваем приложенный импульс предельным значением
			float impulseScalar = part.impDir.Normalize();
			static const float velocityMax = 100.0f;	// m/s			
			const float impulseMax = velocityMax;
			if(impulseScalar > impulseMax)
			{
				impulseScalar = impulseMax;
			}
			if(type != dt_unknown)
			{
				float mass = part.physObject->GetMass();
				part.impDir *= impulseScalar/Clampf(mass, 0.5f, 10.0f);
			}else{
				part.impDir *= impulseScalar;
			}
			//Прикладываем полученный сумарный импульс к физическому объекту
			part.physObject->ApplyImpulse(part.impDir, part.impPos);
		}
		//Проигрываем спецэффект в точки приложения импульса
		if(part.effectsCount)
		{
			ConstString particlesName;
			ConstString soundName;
			switch(type)
			{
			case dt_sword:
				particlesName = pattern.Ptr()->hitSword.particles;
				soundName = pattern.Ptr()->hitSword.sound;
				break;
			case dt_bomb:
				particlesName = pattern.Ptr()->hitBomb.particles;
				soundName = pattern.Ptr()->hitBomb.sound;
				break;
			case dt_bullet:
				particlesName = pattern.Ptr()->hitBullet.particles;
				soundName = pattern.Ptr()->hitBullet.sound;
				break;
			case dt_cannon:
				particlesName = pattern.Ptr()->hitCannon.particles;
				soundName = pattern.Ptr()->hitCannon.sound;
				break;
			case dt_flame:
				particlesName = pattern.Ptr()->hitFlame.particles;
				soundName = pattern.Ptr()->hitFlame.sound;
				break;
			case dt_shooter:
				particlesName = pattern.Ptr()->hitShooter.particles;
				soundName = pattern.Ptr()->hitShooter.sound;
				break;
			}
			part.effectsPos *= 1.0f/part.effectsCount;

//			lines.Add(DLine(part.effectsPos, 0xffffffff, part.effectsPos - part.impDir, 0xffff0000));

			if(particlesName.NotEmpty())
			{
				Matrix mtx;
				if(!mtx.BuildOrient(-part.impDir, Vector(0.0f, 1.0f, 0.0f)))
				{
					mtx.SetIdentity();
				}
				mtx.pos = part.effectsPos;
				Particles().CreateParticleSystemEx2(particlesName.c_str(), mtx, true, _FL_);
			}
			if(soundName.NotEmpty())
			{				
				Sound().Create3D(soundName, part.effectsPos, _FL_);
			}
		}
		part.impPos = 0.0f;
		part.impDir = 0.0f;
		part.effectsPos = 0.0f;
		part.count = 0;
	}
}

//Умирает
bool MissionPhysObject::IsDie()
{
	return HP <= 0.0f;
}

//Мёртв
bool MissionPhysObject::IsDead()
{
	return CheckFlag(flags, f_isDead);
}

//Проверить на пересечение отрезка и ящиков описывающих объекты
bool MissionPhysObject::OverlapLine(const Vector & v1, const Vector & v2, float skin)
{
	array<Part> & parts = !CheckFlag(flags, f_isBroken) ? solid : broken;
	for(long i = 0; i < parts; i++)
	{
		Part & part = parts[i];
		if(!part.physObject || !CheckFlag(part.flags, f_part_isCollision))
		{
			continue;
		}
		Vector abbMin, abbMax;
		bool isAdd = false;
		MissionPhysObjPattern::PatternObject & po = pattern.Ptr()->objects[part.descIndex];		
		for(long j = 0; j < po.boxes; j++)
		{
			MissionPhysObjPattern::CollisionBox & box = po.boxes[j];
			Box::FindABBforOBB(box.mtx, -box.size05, box.size05, abbMin, abbMax, isAdd);
			isAdd = true;
		}
		for(long j = 0; j < po.capsules; j++)
		{
			MissionPhysObjPattern::CollisionCapsule & cap = po.capsules[j];
			Box::FindABBforOBB(cap.mtx, -cap.size05, cap.size05, abbMin, abbMax, isAdd);
			isAdd = true;
		}
		if(isAdd)
		{
			Vector size05 = (abbMax - abbMin)*0.5f;
			Vector center = (abbMin + abbMax)*0.5f;
			Matrix mtx(part.mtx);
			mtx.pos = mtx*center;
			if(Box::OverlapsBoxLine(mtx, size05 + Vector(skin), v1, v2))
			{
				return true;
			}
		}
	}
	return false;
};

//Патерн удаляется
void MissionPhysObject::DeletePattern()
{
	Release();
	pattern.Reset();
}

//Прочитать параметры
void MissionPhysObject::ReadParameters(MOPReader & reader)
{
	Vector pos = reader.Position();
	Vector ang = reader.Angles();
	initMtx.Build(ang, pos);
	patternName = reader.String();
	pattern.Reset();
	MissionObject::Show(reader.Bool());
	MissionObject::Activate(reader.Bool());
	UpdatePatternPointer();
	if(reader.Bool())
	{
		reader.Float();
		if(pattern.Validate())
		{
			HP = pattern.Ptr()->hp;
		}else{
			HP = 1.0f;
		}
	}else{
		HP = reader.Float();
	}
	if(pattern.Validate())
	{
		hideTime = pattern.Ptr()->activeTime;
	}	
}

//Получить поинтер на патерн
void MissionPhysObject::UpdatePatternPointer()
{
	if(patternName.IsEmpty())
	{
		pattern.Reset();
		return;
	}
	FindObject(patternName, pattern.GetSPObject());
	if(pattern.Validate())
	{
		MO_IS_IF_NOT(id_MissionPhysObjPattern, "MissionPhysObjPattern", pattern.Ptr())
		{
			pattern.Reset();
			if(!EditMode_IsOn())
			{
				LogicDebug("Incorrect physic object pattern type (object: \"%s\", type: \"%s\")", patternName.c_str(), pattern.Ptr()->GetObjectType());
			}			
		}
	}else{
		if(!EditMode_IsOn())
		{
			LogicDebug("Physic object pattern \"%s\" not found", patternName.c_str());
		}
	}
	if(pattern.Validate())
	{
		if(!EditMode_IsOn())
		{
			pattern.Ptr()->CacheModels();
		}else{
			for(long i = 0; i < pattern.Ptr()->regObjects; i++)
			{
				if(pattern.Ptr()->regObjects[i] == this)
				{
					return;
				}
			}
			pattern.Ptr()->regObjects.Add(this);
		}
	}
}

//Перестроить данные с паттерна
void MissionPhysObject::UpdatePatternData()
{
	Release();
	if(!pattern.Validate()) return;
	if(pattern.Ptr()->hideDistanceMin2 > 1e-10f)
	{
		SetFlag(flags, f_isCheckLod);
	}else{
		ResetFlag(flags, f_isCheckLod);
	}
	bool isDynamicLighting = pattern.Ptr()->dynamicLighting;
	bool isShadowReceive = pattern.Ptr()->shadowReceive;
	ITipsManager * tmanager = ITipsManager::GetManager(&Mission());
	solid.AddElements(pattern.Ptr()->soldObjectsCount);
	for(long i = 0; i < solid; i++)
	{
		Part & psolid = solid[i];
		psolid.mtx.EqMultiplyFast(pattern.Ptr()->objects[i].mtx, initMtx);
		psolid.model = Geometry().CreateScene(pattern.Ptr()->objects[i].modelName.c_str(), &Animation(), &Particles(), &Sound(), _FL_);		
		if(psolid.model)
		{
			psolid.model->SetTransform(psolid.mtx);
			psolid.model->SetDynamicLightState(isDynamicLighting);
			psolid.model->SetShadowReceiveState(isShadowReceive);
		}
		psolid.physObject = null;
		psolid.impPos = 0.0f;
		psolid.impDir = 0.0f;
		psolid.count = 0;
		psolid.effectsPos = 0.0f;
		psolid.effectsCount = 0;
		psolid.alpha = 1.0f;
		psolid.curAlpha = 1.0f;
		psolid.descIndex = i;
		psolid.tip = null;
		psolid.flags = f_part_isCollision;
		psolid.sndMatMask = -1;
		psolid.sndCooldown = 3.0f;
		if(!EditMode_IsOn())
		{
			if(tmanager && pattern.Ptr()->objects[i].tipId.NotEmpty())
			{
				psolid.tip = tmanager->CreateTip(pattern.Ptr()->objects[i].tipId, psolid.mtx.pos, this);
				if(psolid.tip)
				{
					psolid.tip->Activate(false);
				}
			}
		}
	}
	broken.AddElements(pattern.Ptr()->objects - pattern.Ptr()->soldObjectsCount);
	for(long i = 0; i < broken; i++)
	{
		Part & pbroken = broken[i];
		long index = pattern.Ptr()->soldObjectsCount + i;
		pbroken.mtx.EqMultiplyFast(pattern.Ptr()->objects[index].mtx, initMtx);
		pbroken.model = Geometry().CreateScene(pattern.Ptr()->objects[index].modelName.c_str(), &Animation(), &Particles(), &Sound(), _FL_);
		if(pbroken.model)
		{
			pbroken.model->SetTransform(pbroken.mtx);
			pbroken.model->SetDynamicLightState(isDynamicLighting);
			pbroken.model->SetShadowReceiveState(isShadowReceive);
		}
		pbroken.physObject = null;
		pbroken.impPos = 0.0f;
		pbroken.impDir = 0.0f;
		pbroken.count = 0;
		pbroken.effectsPos = 0.0f;
		pbroken.effectsCount = 0;
		pbroken.alpha = 1.0f;
		pbroken.curAlpha = 1.0f;
		pbroken.descIndex = index;
		pbroken.tip = null;
		pbroken.flags = 0;
		pbroken.sndMatMask = -1;
		pbroken.sndCooldown = 3.0f;
		if(!EditMode_IsOn())
		{
			if(tmanager && pattern.Ptr()->objects[index].tipId.NotEmpty())
			{
				pbroken.tip = tmanager->CreateTip(pattern.Ptr()->objects[index].tipId, pbroken.mtx.pos, this);
				if(pbroken.tip)
				{
					pbroken.tip->Activate(false);
				}
			}
		}
	}
}

//Создать физические объекты
void MissionPhysObject::MakePhysics(array<Part> & obj)
{
	if(EditMode_IsOn()) return;
	if(!pattern.Validate()) return;
	for(long i = 0; i < obj; i++)
	{
		Part & part = obj[i];
		if(part.physObject) continue;
		MissionPhysObjPattern::PatternObject & po = pattern.Ptr()->objects[part.descIndex];
		if((po.boxes || po.capsules) && !po.isStatic)
		{
			IPhysCombined * cb  = Physics().CreateCombined(_FL_, part.mtx, true);
			part.physObject = cb;
			for(long j = 0; j < po.boxes; j++)
			{
				cb->AddBox(po.boxes[j].size05*2.0f, po.boxes[j].mtx);
			}
			for(long j = 0; j < po.capsules; j++)
			{
				cb->AddCapsule(po.capsules[j].radius, po.capsules[j].height - po.capsules[j].radius*2.0f, po.capsules[j].mtx);
			}			
			cb->Build();
			cb->SetGroup(phys_physobjects);
			cb->SetMaterial(pattern.Ptr()->materialId);
		}
	}
}

//Удалить физические объекты
void MissionPhysObject::ReleasePhysics(array<Part> & obj)
{
	for(long i = 0; i < obj; i++)
	{
		Part & part = obj[i];
		if(part.physObject)
		{
			part.physObject->Release();
			part.physObject = null;
		}
	}
}

//Удалить модельки
void MissionPhysObject::ReleaseModels(array<Part> & obj)
{
	for(long i = 0; i < obj; i++)
	{
		Part & part = obj[i];
		if(part.model)
		{
			part.model->Release();
			part.model = null;
		}
		if(part.tip)
		{
			part.tip->Release();
			part.tip = null;
		}
	}
}

//Удалить данные
void MissionPhysObject::Release()
{
	ReleasePhysics(solid);
	ReleasePhysics(broken);
	ReleaseModels(solid);
	ReleaseModels(broken);
	solid.DelAll();
	broken.DelAll();
}

//Сломать
void MissionPhysObject::Broke()
{
	LogicDebug("Broke");
	//Обновляем состояние подписки
	SetShow(IsShow());
	//Ломаем
	SetFlag(flags, f_isBroken);
	//Больше нет препятствий для АИ
	aiColider->Activate(false);
	//Проигрываем звуки и партиклы
	for(long i = 0; i < solid; i++)
	{
		Part & sld = solid[i];
		ResetFlag(sld.flags, f_part_isCollision);
		MissionPhysObjPattern::PatternObject & po = pattern.Ptr()->objects[sld.descIndex];
		//Запускаем эффекты
		for(long j = 0; j < po.particles; j++)
		{
			IParticleSystem * ps = Particles().CreateParticleSystemEx(po.particles[j].name.c_str(), _FL_);
			if(ps)
			{
				ps->SetTransform(sld.mtx*po.particles[j].mtx);
				ps->AutoDelete(true);
			}
		}
		for(long j = 0; j < po.sounds; j++)
		{
			Sound().Create3D(po.sounds[j].name, sld.mtx*po.sounds[j].mtx.pos, _FL_);			
		}
		//Взрываем если есть что
		if(po.receiveBoom.NotEmpty())
		{
			MOSafePointer mo;
			if(FindObject(po.receiveBoom, mo))
			{
				Vector p = sld.mtx*po.centerOfMass;
				const char * params[3];
				char pos[3][32];
				for(long i = 0; i < 3; i++)
				{
					crt_snprintf(pos[i], sizeof(pos[i]), "%f", p.v[i]);
					params[i] = pos[i];
				}
				mo.Ptr()->Command("boom", 3, params);
			}
		}
		//Бросаем бонусы
		if(po.bonusesTable.NotEmpty())
		{
			BonusesManager::CreateBonus(Mission(), sld.mtx.pos, po.bonusesTable);
		}
	}
	//Удаляем физические объекты
	ReleasePhysics(solid);
	for(long i = 0; i < broken; i++)
	{
		//Проставляем матрицу для будующего сломаного объекта
		MissionPhysObjPattern::PatternObject & po = pattern.Ptr()->objects[broken[i].descIndex];
		long j = 0;
		Part & brn = broken[i];
		SetFlag(brn.flags, f_part_isCollision);
		if(j < solid)
		{
			brn.mtx = solid[j].mtx;
			brn.impPos = solid[j].impPos;
			brn.impDir = solid[j].impDir;
			brn.count = solid[j].count;
			brn.effectsPos = 0.0f;
			brn.effectsCount = 0;
		}else{
			brn.impPos = 0.0f;
			brn.impDir = 0.0f;
			brn.count = 1;
			brn.effectsPos = 0.0f;
			brn.effectsCount = 0;
		}
		if(po.isAddImpulse)
		{
			brn.impPos += brn.mtx*po.impulsePosition;
			brn.impDir += brn.mtx.MulNormal(po.impulse)*10.0f;
			brn.count += 1;
		}
	}
	if(broken > 0)
	{
		broken[0].effectsPos = solid[0].effectsPos;
		broken[0].effectsCount = solid[0].effectsCount;
	}
	//Создаём физические объекты
	MakePhysics(broken);
	//Установим таймер на исчезновение
	SetUpdate(&MissionPhysObject::HideTimer, ML_EXECUTE1);
	//Обновляем состояние типсов
	ShowTips();
	//
	Update(0.0f, 0);
}

//Модифицировать наносимое повреждение с учётом коэфициентов демеджа
float MissionPhysObject::ModifyHp(DamageType type, float hp)
{
	float modifier = pattern.Ptr()->damageModifier.Multiplier(type, 0.0f);
	return hp*modifier;
}

//Нанести повреждение
bool MissionPhysObject::ApplyDamage(float damage, DamageType type)
{
	if(!pattern.Validate()) return false;
	if(type == dt_check) return false;
	if(CheckFlag(flags, f_isBroken)) return false;
	HP -= damage;
	if(HP > 0.0f) return false;
	Broke();
	return true;
}

//Найти видимый ABB
void MissionPhysObject::FindVisibleABB(array<Part> & obj, ABB & abb)
{
	abb.min = abb.max = 0.0f;
	bool isAdd = false;
	for(long i = 0; i < obj; i++)
	{
		Part & part = obj[i];
		if(part.model)
		{
			const GMXBoundBox & bbx = part.model->GetLocalBound();
			Box::FindABBforOBB(part.model->GetTransform(), bbx.vMin, bbx.vMax, abb.min, abb.max, isAdd);
			isAdd = true;
		}
	}
}

//Найти для физики ABB
void MissionPhysObject::FindCollidersABB(array<Part> & obj, ABB & abb)
{
	abb.min = abb.max = 0.0f;
	bool isAdd = false;
	for(long i = 0; i < obj; i++)
	{
		Part & part = obj[i];
		MissionPhysObjPattern::PatternObject & po = pattern.Ptr()->objects[part.descIndex];
		for(long j = 0; j < po.boxes; j++)
		{
			MissionPhysObjPattern::CollisionBox & box = po.boxes[j];
			Box::FindABBforOBB(box.mtx*part.mtx, -box.size05, box.size05, abb.min, abb.max, isAdd);
			isAdd = true;
		}
		for(long j = 0; j < po.capsules; j++)
		{
			MissionPhysObjPattern::CollisionCapsule & cap = po.capsules[j];
			Box::FindABBforOBB(cap.mtx*part.mtx, -cap.size05, cap.size05, abb.min, abb.max, isAdd);
			isAdd = true;
		}
	}
}

__forceinline MissionPhysObject::DamageType MissionPhysObject::DetectType(dword source)
{
	switch(source)
	{
	case DamageReceiver::ds_sword:
		return dt_sword;
	case DamageReceiver::ds_bomb:
		return dt_bomb;
	case DamageReceiver::ds_bullet:
		return dt_bullet;
	case DamageReceiver::ds_cannon:
		return dt_cannon;
	case DamageReceiver::ds_flame:
		return dt_flame;
	case DamageReceiver::ds_shooter:
		return dt_shooter;
	case DamageReceiver::ds_unknown:
		return dt_unknown;
	case DamageReceiver::ds_check:
		return dt_check;
	case ds_pickup:
		return dt_pickup;
	}
	return dt_unknown;
}


//Переместить целую часть в новую позицию
bool MissionPhysObject::SetPartMatrix(long partIndex, const Matrix & mtx)
{
	if(IsActive() || CheckFlag(flags, f_isBroken)) return false;
	if(partIndex >= solid || partIndex < 0) return false;
	Part & part = solid[partIndex];
	part.mtx = mtx;
	if(part.model)
	{
		part.model->SetTransform(part.mtx);
	}
	if(part.tip)
	{
		part.tip->SetPos(part.mtx.pos);
	}
	float wlevel = IWaterLevel::GetWaterLevel(Mission(), waterLevel);
	if(part.mtx.pos.y < wlevel)
	{
		const Vector & com = pattern.Ptr()->objects[part.descIndex].centerOfMass;
		Vector newPos = part.mtx*com;
		Vector curPos = Vector(newPos.x, wlevel - 1.0f, newPos.z);
		WaterDrop(part, curPos, newPos, wlevel);
	}
	Update(0.0f, 0);
	return true;
}

//Получить текущую матрицу целой части объекта
bool MissionPhysObject::GetPartMatrix(long partIndex, Matrix & mtx)
{
	if(CheckFlag(flags, f_isBroken)) return false;
	if(partIndex >= solid || partIndex < 0) return false;
	Part & part = solid[partIndex];
	mtx = part.mtx;
	return true;
}

//Получить пивот целой части
bool MissionPhysObject::GetPartPivot(long partIndex, Matrix & mtx)
{
	if(CheckFlag(flags, f_isBroken)) return false;
	if(partIndex >= solid || partIndex < 0) return false;
	Part & part = solid[partIndex];
	MissionPhysObjPattern::PatternObject & po = pattern.Ptr()->objects[part.descIndex];
	mtx = po.mtx;
	return true;
}

//Применить импульс в мировых координатах
void MissionPhysObject::Impulse(const Vector & dir)
{
	array<Part> & part = CheckFlag(flags, f_isBroken) ? broken : solid;
	for(long i = 0; i < part; i++)
	{
		part[i].impDir = dir;
		part[i].impPos = 0.0f;
		part[i].count = 1;
	}
	ApplyImpulses(dt_unknown);
}

//Сломать объект
void MissionPhysObject::BrokeObject()
{
	if(!CheckFlag(flags, f_isBroken))
	{
		Broke();
	}	
}

//Показать-скрыть подсказки
void MissionPhysObject::ShowTips(bool isShow)
{
	if(CheckFlag(flags, f_isShowTips) != isShow)
	{
		if(isShow)
		{
			SetFlag(flags, f_isShowTips);
		}else{
			ResetFlag(flags, f_isShowTips);
		}
		ShowTips();
	}
}

//Найти индекс локатора по имени, -1 если нет такого
long MissionPhysObject::GetLocatorIndexByName(const ConstString & name)
{
	if(!pattern.Validate() || name.IsEmpty())
	{
		return -1;
	}
	MissionPhysObjPattern::Locator * locs = pattern.Ptr()->locators.GetBuffer();
	long size = pattern.Ptr()->locators;
	for(long i = 0; i < size; i++)
	{
		if(locs[i].name == name)
		{
			return i;
		}
	}
	return -1;
}

//Получить позицию локатора по индексу
bool MissionPhysObject::GetLocator(long index, Matrix & mtx)
{
	if(!pattern.Validate() || index < 0 || index >= pattern.Ptr()->locators)
	{
		return false;
	}
	MissionPhysObjPattern::Locator & loc = pattern.Ptr()->locators[index];
	Part & part = (loc.objectIndex < pattern.Ptr()->soldObjectsCount) ? solid[loc.objectIndex] : broken[loc.objectIndex - pattern.Ptr()->soldObjectsCount];	
	mtx.EqMultiply(loc.mtx, part.mtx);
	return true;
}


__forceinline void MissionPhysObject::ResetFlag(dword & f, dword value)
{
	f &= ~value;
}
__forceinline void MissionPhysObject::SetFlag(dword & f, dword value)
{
	f |= value;
}

__forceinline bool MissionPhysObject::CheckFlag(dword f, dword value)
{
	return (f & value) != 0;
}

//============================================================================================
//Параметры инициализации
//============================================================================================

MOP_BEGINLISTCG(MissionPhysObject, "Physic object", '1.00', 1, "Patterned physic object\n  Commands:\n    broke [objectId impulseForce [impulseForceMax]]\n    shock time[0..10] force[0..100] period[0..10]\n    hackhideon\n    hackhideoff", "Physics")
	MOP_POSITIONC("Position", Vector(0.0f), "Model position in world")
	MOP_ANGLESC("Angles", Vector(0.0f), "Model orientation in world")
	MOP_STRINGC("Pattern", "", "Name of physics object pattern")
	MOP_BOOLC("Show", true, "Show or hide geometry in start mission time")
	MOP_BOOLC("Active", true, "Show or hide geometry in start mission time")
	MOP_BOOLC("Use pattern HP", true, "Object HP")
	MOP_FLOATEXC("HP", 0.0f, 0.0f, 100000.0f, "Physic objects HP")
	MOP_MISSIONTRIGGERG("Broke trigger", "")
	MOP_BOOLC("Draw collision", false, "Draw collision primitives")
MOP_ENDLIST(MissionPhysObject)

