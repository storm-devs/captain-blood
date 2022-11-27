#include "ActiveObject.h"

ActiveObject:: ActiveObject() :
	liveBoxes(_FL_),
	deadBoxes(_FL_)
{
	collider = null;

	model = null;
	anime = null;

	broke = null;
	brokeAnime = null;

	scene = null;

	time	= 0.0f;
	timeMax = 0.5f;

	tip = null;

	master.Reset();

	sfxLeast = null;
	sndLeast = null;

	m_active = false;
	m_show	 = false;
}

ActiveObject::~ActiveObject()
{
	DeleteColls();

	if( collider )
		collider->Release();

	if( model )
		model->Release();

	if( anime )
	{
		events.DelAnimation(anime);
		anime->Release();
	}

	if( broke )
		broke->Release();

	if( brokeAnime )
	{
		events.DelAnimation(brokeAnime);
		brokeAnime->Release();
	}

//	if( tip )
//		tip->Release();

	if( sfxLeast )
		sfxLeast->Release();
	if( sndLeast )
		sndLeast->Release();
}

bool ActiveObject::Create(MOPReader &reader)
{
	events.SetMission(&Mission());

	DamageReceiver::Create(reader);
	InitParams(reader);

	Assert(finder);

	return true;
}

bool ActiveObject::EditMode_Create(MOPReader &reader)
{
	return Create(reader);
}

bool ActiveObject::EditMode_Update(MOPReader &reader)
{
	return Create(reader);
}

void ActiveObject::InitParams(MOPReader &reader)
{
	if( !collider )
		 collider = QTCreateObject(MG_AI_COLLISION,_FL_);

	collider->Activate(false);

	Vector pos = reader.Position();
	Vector ang = reader.Angles();

	matrix.Build(ang,pos);

	bool newModel = false;
	bool newAnime = false;

	const char *t = reader.String().c_str();

	if( !model || modelName != t )
	{
		if( model )
			model->Release();

		model = Geometry().CreateScene(t,&Animation(),&Particles(),&Sound(),_FL_);

		modelName = t; newModel = true;
	}

	t = reader.String().c_str();

	if( !anime || !string::IsEqual(anime->GetName(),t))
	{
		if( anime )
		{
			events.DelAnimation(anime);
			anime->Release();
		}

		anime = Animation().Create(t,_FL_);

		newAnime = true;
	}

	if( newModel || newAnime )
	{
		if( model )
			model->SetAnimation(anime);
	}

	nodeOnHit = reader.String().c_str(); onHitBlend = reader.Float();
	nodeBroke = reader.String().c_str(); brokeBlend = reader.Float();

	if( nodeBroke && !nodeBroke[0] )
		nodeBroke = null;

	events.Init(&Sound(),&Particles(),&Mission());

	if( !EditMode_IsOn())
	{
		events.AddAnimation(anime);
		events.SetScene(model,matrix);
	}

	healthMax = reader.Float();
	health = healthMax;

	masterName = reader.String();

	if( masterName.NotEmpty() )
		FindObject(masterName,master);

	Matrix m; GetMatrix(m);

	DeleteColls();

	InitBoxes(reader,m,liveBoxes);

	effOnHit.Init(reader);
	effBroke.Init(reader);

	if( sfxLeast )
		sfxLeast->Release(); sfxLeast = null;
	if( sndLeast )
		sndLeast->Release(); sndLeast = null;

	effLeast.Init(reader);

	finder->Activate(false);

	color = reader.Colors();

	bonusTable = reader.String();

	ConstString tName	= reader.String();
	pos = reader.Position();

	tipPos = pos;

	if( tip )
		tip->Release();

	tip = null;

	ITipsManager *tipsManager = ITipsManager::GetManager(&Mission());

	if( tipsManager && tName.NotEmpty() )
	{
		tip = tipsManager->CreateTip(tName,m.MulVertex(tipPos),this);

		if( tip )
		//	tip->Activate(false);
			tip->SetAlpha(0.0f);
	}

	newModel = false;
	newAnime = false;

	t = reader.String().c_str();

	if( !broke || brokeName != t )
	{
		if( broke )
			broke->Release();

		broke = Geometry().CreateScene(t,&Animation(),&Particles(),&Sound(),_FL_);

		brokeName = t; newModel = true;
	}

	t = reader.String().c_str();

	if( !brokeAnime || !string::IsEqual(brokeAnime->GetName(),t))
	{
		if( brokeAnime )
		{
			events.DelAnimation(brokeAnime);
			brokeAnime->Release();
		}

		brokeAnime = Animation().Create(t,_FL_);

		newAnime = true;
	}

	if( newModel || newAnime )
	{
		if( broke )
			broke->SetAnimation(brokeAnime);
	}

	if( !EditMode_IsOn())
	{
		events.AddAnimation(brokeAnime);
	//	events.SetScene(broken,matrix);
	}

	InitBoxes(reader,m,deadBoxes);

	EnableDeadColls(false);

	debugDraw = reader.Bool();
	debugDrawBroke = reader.Bool();

	if( anime )
	{
		if( EditMode_IsOn() && debugDrawBroke && nodeBroke )
		{
			if( !string::IsEqual(anime->CurrentNode(),nodeBroke))
				anime->Goto(nodeBroke,0.0f);
		}
		else
		{
			if( !string::IsEqual(anime->CurrentNode(),"start"))
				anime->Goto("start",0.0f);
		}
	}

	//// Render params ////

	skipFog = reader.Bool();

	drawLevel = reader.Long();
	drawColor = reader.Colors();

	bool smoothAlpha = reader.Bool();

	enableRender = reader.Bool();

	bool transparent = reader.Bool();

	//// уровень отрисовки

	long from = transparent ? ML_ALPHA1 : ML_GEOMETRY1;
	long to	  = transparent ? ML_ALPHA3 : ML_GEOMETRY3;

	drawLevel = from + (to - from)*drawLevel/100;

	////

	bool dynamicLighting = reader.Bool();

	shadowCast	  = reader.Bool();
	shadowReceive = reader.Bool();

	seaReflection = reader.Bool();
	seaRefraction = reader.Bool();

	hideInEditor = reader.Bool();

	noSwing = reader.Bool();

	float distanceMin = reader.Float();
	float distanceRad = reader.Float();

	if( distanceMin > 0.0f )
	{
		float distanceMax = distanceMin + distanceRad;

		distanceMin2 = distanceMin*distanceMin;
		distanceK2 = 1.0f/(distanceMax*distanceMax - distanceMin2);

		checkDistance = true;
	}
	else
	{
		distanceMin2 = 0.0f;
		distanceK2	 = 0.0f;

		checkDistance = false;
	}

	///////////////////////

	if( model ) model->SetUserColor(drawColor);
	if( broke ) broke->SetUserColor(drawColor);

	float alphaRef = smoothAlpha ? 0.003921f : 0.5f;

	if( model ) model->SetFloatAlphaReference(alphaRef);
	if( broke ) broke->SetFloatAlphaReference(alphaRef);

	if( model ) model->SetDynamicLightState(dynamicLighting);
	if( broke ) broke->SetDynamicLightState(dynamicLighting);

	///////////////////////

	Activate(m_active = reader.Bool());
	Show	(m_show	  = reader.Bool());

	scene = null;
	fadeAlpha = 0.0f;

	prevAlpha = false;
}

void ActiveObject::InitBoxes(MOPReader &reader, const Matrix &m, array<CollisionBox> &boxes)
{
	Vector pos;
	Vector ang;

	boxes.DelAll();
	boxes.AddElements(reader.Array());

	for( int i = 0 ; i < boxes ; i++ )
	{
		CollisionBox &box = boxes[i];

		pos = reader.Position();
		ang = reader.Angles();

		box.mtx.Build(ang,pos);

		box.size05.x = reader.Float()*0.5f;
		box.size05.y = reader.Float()*0.5f;
		box.size05.z = reader.Float()*0.5f;

		box.box = Physics().CreateBox(_FL_,box.size05*2.0f,Matrix(box.mtx,m),/*true*/false);
		box.box->SetGroup(/*phys_player*/phys_physobjects);
	}
}

void ActiveObject::PostCreate()
{
	if( masterName.NotEmpty() )
		FindObject(masterName,master);
}

Matrix &ActiveObject::GetMatrix(Matrix &m)
{
	Matrix mmat;

	if( master.Ptr())
		master.Ptr()->GetMatrix(mmat);

	return m = matrix*mmat;
}

void ActiveObject::Reset()
{
	if( anime )
		anime->Goto("start",0);

	if( sfxLeast )
		sfxLeast->Release(); sfxLeast = null;
	if( sndLeast )
		sndLeast->Release(); sndLeast = null;

	health = healthMax;

	Activate(IsActive());

	events.ResetCollapser();

	if( !EditMode_IsOn())
	{
		events.SetScene(model,matrix);
	}

	EnableDeadColls(false);
	EnableLiveColls(IsActive() && IsShow());
}

void ActiveObject::Restart()
{
	Activate(m_active);
	Show	(m_show);

	Reset();
}

void ActiveObject::Activate(bool isActive)
{
	DamageReceiver::Activate(isActive);

	if( anime )
		anime->Pause(!isActive);

	ActivateColliders(isActive && IsShow());

	if( tip )
	//	tip->Activate(isActive && IsShow());
		tip->SetAlpha(isActive && IsShow() ? 1.0f : 0.0f);

	LogicDebug(isActive ? "Activate" : "Deactivate");
}

void ActiveObject::Show(bool isShow)
{
	DamageReceiver::Show(isShow);

	if( EditMode_IsOn())
		DelUpdate(&ActiveObject::Draw);

	prevAlpha = false;

	if( isShow )
	{
		long level = drawLevel;

		//

		if( enableRender )
		{
			SetUpdate(&ActiveObject::Draw,level);
		}

		if( shadowCast )
			Registry  (MG_SHADOWCAST,&ActiveObject::ShadowInfo,level);
		else
			Unregistry(MG_SHADOWCAST);

		if( shadowReceive )
		{
			Unregistry(MG_SHADOWDONTRECEIVE);
			Registry  (MG_SHADOWRECEIVE,&ActiveObject::ShadowReceive,level);
		}
		else
		{
			Unregistry(MG_SHADOWRECEIVE);

			if( level < ML_ALPHA1 && enableRender )
				Registry(MG_SHADOWDONTRECEIVE,&ActiveObject::ShadowReceive,level);
		}

		if( seaReflection )
			Registry  (MG_SEAREFLECTION,&ActiveObject::SeaReflection,level);
		else
			Unregistry(MG_SEAREFLECTION);

		if( seaRefraction )
			Registry  (MG_SEAREFRACTION,&ActiveObject::SeaReflection,level);
		else
			Unregistry(MG_SEAREFRACTION);
	}
	else
	{
		DelUpdate(&ActiveObject::Draw);

		events.ResetParticles();
		events.ResetSounds();

		Unregistry(MG_SHADOWCAST);
		Unregistry(MG_SHADOWRECEIVE);

		Unregistry(MG_SEAREFLECTION);
		Unregistry(MG_SEAREFRACTION);

		Unregistry(MG_SHADOWDONTRECEIVE);
	}

	ActivateColliders(isShow && IsActive());

	if( tip )
	//	tip->Activate(isShow && IsActive());
		tip->SetAlpha(isShow && IsActive() ? 1.0f : 0.0f);

	LogicDebug(isShow ? "Show" : "Hide");
}

void ActiveObject::ActivateColliders(bool en)
{
	if( health > 0.0f )
	{
		if( en )
		{
			if( liveBoxes )
			{
				const CollisionBox &box = liveBoxes[0];

				Matrix m; GetMatrix(m); m = box.mtx*m;

				finder->SetMatrix(m);
				finder->SetBox(-box.size05,box.size05);

				collider->SetMatrix(m);
				collider->SetBox(-box.size05,box.size05);
			}
		}

		  finder->Activate(en);
		collider->Activate(en);

		EnableLiveColls(en);
	}
	else
	{
		if( en )
		{
			if( deadBoxes )
			{
				const CollisionBox &box = deadBoxes[0];

				Matrix m; GetMatrix(m); m = box.mtx*m;

			//	finder->SetMatrix(m);
			//	finder->SetBox(-box.size05,box.size05);

				collider->SetMatrix(m);
				collider->SetBox(-box.size05,box.size05);
			}
		}

	//	  finder->Activate(en);
		collider->Activate(en);

		EnableDeadColls(en);
	}
}

void ActiveObject::UpdateColliders()
{
	if( health > 0.0f )
	{
		if( liveBoxes )
		{
			Matrix m; GetMatrix(m);

			for( int i = 0 ; i < liveBoxes ; i++ )
			{
				const CollisionBox &box = liveBoxes[i];

				box.box->SetTransform(box.mtx*m);
				box.box->SetSize(box.size05*2.0f);
			}

			const CollisionBox &box = liveBoxes[0];

			box.box->GetTransform(m);

			if( IsActive())
			{
				finder->SetMatrix(m);
				finder->SetBox(-box.size05,box.size05);
			}

			collider->SetMatrix(m);
			collider->SetBox(-box.size05,box.size05);
		}
	}
	else
	{
		if( deadBoxes )
		{
			Matrix m; GetMatrix(m);

			for( int i = 0 ; i < deadBoxes ; i++ )
			{
				const CollisionBox &box = deadBoxes[i];

				box.box->SetTransform(box.mtx*m);
				box.box->SetSize(box.size05*2.0f);
			}

			const CollisionBox &box = deadBoxes[0];

			box.box->GetTransform(m);

			collider->SetMatrix(m);
			collider->SetBox(-box.size05,box.size05);
		}
	}
}

void _cdecl ActiveObject::Draw(float dltTime, long level)
{
	if( EditMode_IsOn())
	{
		if( masterName.NotEmpty() )
			if( !master.Validate())
				 FindObject(masterName,master);
	}

	Matrix m; GetMatrix(m);

	if( tip )
		tip->SetPos(m.MulVertex(tipPos));

	events.Update(m, dltTime);

	UpdateColliders();

	if( sfxLeast )
		sfxLeast->SetTransform(Matrix(effLeast.sfx.m,m));
	if( sndLeast )
		sndLeast->SetPosition(m.MulVertex(effLeast.snd.p));

	if( time > 0.0f )
	{
		time -= dltTime;

		if( time < 0.0f )
			time = 0.0f;
	}

	Color sceneColor(drawColor);

	if( health > 0.0f && !(EditMode_IsOn() && debugDrawBroke))
	{
			scene = model;

		if( time > 0.0f )
		{
			sceneColor.LerpA(drawColor,color,time/timeMax);
		}
	}
	else
	{
		if( anime && nodeBroke )
		{
			scene = model;
		}
		else
		{
			scene = broke;
		}
	}

	if( scene )
	{
		if( checkDistance )
		{
			const GMXBoundBox &bb = scene->GetLocalBound();
			
			Vector center = (bb.vMin + bb.vMax)*0.5f;

			m.MulVertex(center);

			fadeAlpha = (Render().GetView().GetCamPos() - center).GetAttenuation2(distanceMin2,distanceK2);
		}
		else
		{
			fadeAlpha = 1.0f;
		}

		sceneColor.a *= fadeAlpha;

		scene->SetUserColor(sceneColor);
	}

	if( fadeAlpha < 0.99f )
	{
		if( prevAlpha == false )
		{
			ResetUpdate(true);  prevAlpha = true;
		}
	}
	else
	{
		if( prevAlpha == true )
		{
			ResetUpdate(false);	prevAlpha = false;
		}
	}

	DrawScene();

	if( EditMode_IsOn() && EditMode_IsSelect() || !EditMode_IsOn() && debugDraw )
	{
		if( health > 0.0f && !(EditMode_IsOn() && debugDrawBroke))
		{
			for( int i = 0 ; i < liveBoxes ; i++ )
			{
				const CollisionBox &box = liveBoxes[i];

				Render().DrawBox(-box.size05,box.size05,Matrix(box.mtx,m),/*0xff0000ff*/-1);
			}
		}
		else
		{
			for( int i = 0 ; i < deadBoxes ; i++ )
			{
				const CollisionBox &box = deadBoxes[i];

				Render().DrawBox(-box.size05,box.size05,Matrix(box.mtx,m),/*0xff0000ff*/-1);
			}
		}
	}
}

void ActiveObject::DrawScene()
{
	if( fadeAlpha < 0.001f )
		return;

	if( scene )
	{
		Matrix view(true);
		if( noSwing )
		{
			view = Render().GetView();
			Render().SetView((Mission().GetInverseSwingMatrix()*Matrix(view).Inverse()).Inverse());
		}

		Matrix m; GetMatrix(m);

		FogParamsSave();

		scene->SetTransform(m);
		scene->Draw();

		FogParamsRestore();

		if( noSwing )
		{
			Render().SetView(view);
		}
	}
}

void _cdecl ActiveObject::ShadowInfo(const char *group, MissionObject *sender)
{
	if(!EditMode_IsVisible()||!IsShow()) return;

	if( scene )
	{
		Matrix m(true); GetMatrix(m);

		scene->SetTransform(m);

		const Vector &vMin = scene->GetBound().vMin;
		const Vector &vMax = scene->GetBound().vMax;

		((MissionShadowCaster *)sender)->AddObject(this,&ActiveObject::ShadowCast,vMin,vMax);
	}
}

bool ActiveObject::Attack(MissionObject *obj, dword src, float hp, const Vector &center, float rad)
{
	if(!IsActive())
		return false;

	if( src != ds_sword )
		return false;

	bool isHit = false;

	Matrix m; GetMatrix(m);

	for( long j = 0 ; j < liveBoxes ; j++ )
	{
		CollisionBox &box = liveBoxes[j];

		Matrix mtx(box.mtx,m);

		if( Box::OverlapsBoxSphere(mtx,box.size05,center,rad))
		{
			isHit = true; break;
		}
	}

	if( isHit )
		ApplyDamage(hp);

	return isHit;
}

bool ActiveObject::Attack(MissionObject *obj, dword src, float hp, const Vector &from, const Vector &to)
{
	if(!IsActive())
		return false;

	if( src != ds_sword )
		return false;

	bool isHit = false;

	Matrix m; GetMatrix(m);

	for( long j = 0 ; j < liveBoxes ; j++ )
	{
		CollisionBox &box = liveBoxes[j];

		Matrix mtx(box.mtx,m);

		if( Box::OverlapsBoxLine(mtx,box.size05,from,to))
		{
			isHit = true; break;
		}
	}

	if( isHit && src != ds_check )
		ApplyDamage(hp);

	return isHit;
}

bool ActiveObject::Attack(MissionObject *obj, dword src, float hp, const Vector vert[4])
{
	if(!IsActive())
		return false;

	if( src != ds_sword )
		return false;

	bool isHit = false;

	Matrix m; GetMatrix(m);

	for( long j = 0 ; j < liveBoxes ; j++ )
	{
		CollisionBox &box = liveBoxes[j];

		Matrix mtx(box.mtx,m);

		if( Box::OverlapsBoxPoly(mtx,box.size05,vert))
		{
			isHit = true; break;
		}
	}

	if( isHit )
		ApplyDamage(hp);

	return isHit;
}

void ActiveObject::ApplyDamage(float hp)
{
	health -= hp;

	Matrix m; GetMatrix(m);

	if( health <= 0.0f )
	{
		if( model )
			model->SetUserColor(drawColor);

		if( anime && nodeBroke )
		{
			anime->Goto(nodeBroke,brokeBlend);
		}

		Particles().CreateParticleSystemEx2(
			effBroke.sfx.name,
			Matrix(effBroke.sfx.m,m),true,_FL_);

		Sound().Create3D(effBroke.snd.name,m.MulVertex(effBroke.snd.p),_FL_);

		time = 0.0f;

		  finder->Activate(false);
	//	collider->Activate(false);

	//	EnableColls(false);

	//	if( tip )
	//		tip->Release();

	//	tip = null;
		if( tip )
		//	tip->Activate(false);
			tip->SetAlpha(0.0f);

		health = 0.0f;

		sfxLeast = Particles().CreateParticleSystemEx2(
			effLeast.sfx.name,
			Matrix(effLeast.sfx.m,m),false,_FL_);

		sndLeast = Sound().Create3D(effLeast.snd.name,m.MulVertex(effLeast.snd.p),_FL_,true,false);

		if( bonusTable.NotEmpty() )
		{
			BonusesManager::CreateBonus(Mission(),m.pos,bonusTable);
		}

		if( !EditMode_IsOn())
		{
			if( broke )
				events.SetScene(broke,matrix);
		}

		EnableLiveColls(false);
		EnableDeadColls(IsActive() && IsShow());
	}
	else
	{
		if( anime && nodeOnHit )
		{
			anime->Goto(nodeOnHit,onHitBlend);
		}

		Particles().CreateParticleSystemEx2(
			effOnHit.sfx.name,
			Matrix(effOnHit.sfx.m,m),true,_FL_);

		Sound().Create3D(effOnHit.snd.name,m.MulVertex(effOnHit.snd.p),_FL_);

		time = timeMax;
	}
}

void ActiveObject::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEmpty(id))
		return;
	
	if( string::IsEqual(id,"reset"))
	{
		Reset();

		LogicDebug("Command <reset>. Reset complete.");
	}
	else
	if( string::IsEqual(id,"restart"))
	{
		Restart();

		LogicDebug("Command <restart>. Restart complete.");
	}
	else
	{
		LogicDebugError("Unknown command \"%s\".",id);
	}
}

MOP_BEGINLISTCG(ActiveObject, "Active object", '1.00', 1, "Active environment object", "Geometry")

	MOP_POSITION("Position", Vector(0.0f))
	MOP_ANGLES("Angles", Vector(0.0f))

	MOP_STRING("Model"	  , "")
	MOP_STRING("Animation", "")

	MOP_STRING("OnHit node", "")
	MOP_FLOATEX("OnHit blend time", 0.5f, 0.0f, 10.0f)
	MOP_STRING("Break node", "")
	MOP_FLOATEX("Break blend time", 0.5f, 0.0f, 10.0f)

	MOP_FLOATEX("HP", 0.0f, 0.0f, 100000.0f)

	MOP_STRING("Master","")

	MOP_ARRAYBEG("Box", 0, 4)
		MOP_POSITION("Position", Vector(0.0f))
		MOP_ANGLES("Angles", Vector(0.0f))
		MOP_FLOATEX("Size x", 0.1f, 0.001f, 1000.0f)
		MOP_FLOATEX("Size y", 0.1f, 0.001f, 1000.0f)
		MOP_FLOATEX("Size z", 0.1f, 0.001f, 1000.0f)
	MOP_ARRAYEND

	MOP_GROUPBEG("OnHit effect")

		MOP_GROUPBEG("Sound")

			MOP_STRING("hsName", "")
			MOP_POSITION("hsPosition", Vector(0.0f))

		MOP_GROUPEND()

		MOP_GROUPBEG("Particles")

		MOP_STRING("hpName", "")
		MOP_POSITION("hpPosition", Vector(0.0f))
		MOP_ANGLES("hpAngles", Vector(0.0f))

		MOP_GROUPEND()

	MOP_GROUPEND()

	MOP_GROUPBEG("Break effect")

		MOP_GROUPBEG("Sound")

			MOP_STRING("bsName", "")
			MOP_POSITION("bsPosition", Vector(0.0f))

		MOP_GROUPEND()

		MOP_GROUPBEG("Particles")

		MOP_STRING("bpName", "")
		MOP_POSITION("bpPosition", Vector(0.0f))
		MOP_ANGLES("bpAngles", Vector(0.0f))

		MOP_GROUPEND()

	MOP_GROUPEND()

	MOP_GROUPBEG("Least effect")

		MOP_GROUPBEG("Sound")

			MOP_STRING("lsName", "")
			MOP_POSITION("lsPosition", Vector(0.0f))

		MOP_GROUPEND()

		MOP_GROUPBEG("Particles")

			MOP_STRING("lpName", "")
			MOP_POSITION("lpPosition", Vector(0.0f))
			MOP_ANGLES("lpAngles", Vector(0.0f))

		MOP_GROUPEND()

	MOP_GROUPEND()

	MOP_COLOR("OnHit color", (dword)0xff333333);

	MOP_STRING("Bonus table", "")

	MOP_STRING("Tip id", "")
	MOP_POSITION("Tip position", Vector(0.0f))

	MOP_STRING("broken Model", "")
	MOP_STRING("broken Animation", "")

	MOP_ARRAYBEG("broken Box", 0, 4)
		MOP_POSITION("Position", Vector(0.0f))
		MOP_ANGLES("Angles", Vector(0.0f))
		MOP_FLOATEX("Size x", 0.1f, 0.001f, 1000.0f)
		MOP_FLOATEX("Size y", 0.1f, 0.001f, 1000.0f)
		MOP_FLOATEX("Size z", 0.1f, 0.001f, 1000.0f)
	MOP_ARRAYEND

	MOP_BOOL("Debug", false)
	MOP_BOOL("Debug draw broken", false)

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

	MOP_GROUPEND()

	MOP_BOOL("Active", true)
	MOP_BOOL("Show"	 , true)

MOP_ENDLIST(ActiveObject)
