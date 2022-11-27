#include "AnimatedModel.h"
#include "AnimatedModelsArbiter.h"

#define draw_bounds false

void AnimatedModel::CheckLinks()
{
	for( dword i = 0 ; i < links.Size() ; i++ )
	{
		Link &link = links[i];

		if( link.active && link.srcTrack == curTrack &&
						   link.srcItem	 == curNode )
		{
			dword t = link.dstTrack;
			dword n = link.dstItem;

			if( t < tracks.Size() &&
				n < tracks[t].nodes.Size())
			{
				lastLook = n&1 ? tracks[t].nodes[n].binormal : tracks[t].nodes[n].tangent;

				GotoNode(t,n); return;
			}
			else
				LogicDebugError("Invalid link destination: [%i][%i].",t,n);
		}
	}
}

void AnimatedModel::GotoNode(dword ti, dword ni, float estTime)
{
	if( IsActive() == false )
	{
		saveTrack = ti;
		saveNode  = ni; saveTime = estTime;

		saved = true;

		return;
	}
	else
	{
		saved = false;
	}

	curTrack = ti;
	curNode	 = ni;

	if( !tracks.Size())
		return;

	bool edit_mode = EditMode_IsOn();

	if( edit_mode )
	{
		if( trackTime >= 0.0f )
		{
			array<Node> &nodes = tracks[curTrack].nodes;

			const Node &last = nodes.LastE();

			if( trackTime >= last.timeTotal + last.time )
			{
				curNode = 0;

				curTime = 0.0f;
				curPos  = 0.0f;
			}
			else
			{
				for( int i = 0 ; i < nodes ; i++ )
				{
					const Node &node = nodes[i];

					if( i < nodes - 1 )
					{
						const Node &next = nodes[i + 1];

						if( next.timeTotal > trackTime )
						{
							curNode = i;

							curTime = trackTime - node.timeTotal;
							curPos  = curTime/node.time*node.len;

							break;
						}
					}
					else
					{
						curNode = i;

						curTime = trackTime - node.timeTotal;
						curPos  = curTime/node.time*node.len;
					}
				}
			}
		}
	}

	Node &node = tracks[curTrack].nodes[curNode];

	if( animation && node.node && !string::IsEqual(node.node,animation->CurrentNode()))
	{
		if( animation->Goto(node.node,node.blendTime))
		{
			animationNode = node.node;
		}
	}

	if( !edit_mode )
	{
		if( curTrack < tracks_native.Size() && curNode < tracks_native[curTrack].nodes.Size())
		{
			if( tracks_native[curTrack].nodes[curNode].event.IsEmpty() == false )
			{
				LogicDebug("Animated model event");
				tracks_native[curTrack].nodes[curNode].event.Activate(Mission(),false);
			}
		}
		else
		{
			if( node.event.IsEmpty() == false )
			{
				LogicDebug("Animated model event");
				node.event.Activate(Mission(),false);
			}
		}
	}

	if( edit_mode && trackTime >= 0.0f )
	{
		nodeStarted = false;
	}
	else
	{
		curPos	= 0;

	//	curTime = estTime;
		curTime = 0;

		nodeStarted = true;
	}

	Vector dPos; float dAng = 0.0f;

	if( animation )
		animation->GetMovement(dPos);

	delay = 0.0f;

	lastScale = modelScale*node.sc;

	restartDelay = 0.0f;
}

void AnimatedModel::Reset()
{
	if( animation )
		animation->Goto(null,0);

	events.ResetCollapser();

	 curVelocity = 0.0f;
	lastVelocity = 0.0f;

	prevVel = 0.0f;

	dword tn = (EditMode_IsOn() && editing)? editTrack : startTrack;

	GotoNode(tn,0);
}

AnimatedModel::AnimatedModel() :
	links		 (_FL_),
	tracks_native(_FL_),tracks(&tracks_native)
{
	model	  = null;
	animation = null;

	model_loadFailed = false;

	curTrack = 0;
	curNode	 = 0;

	startTrack = 0;

	editTrack = -1;
	editing = false;

	lastLook = Vector(0,0,1);
	lastScale = 1.0f;

	curPos  = 0;
	curTime = 0;

	trackTime = -1.0f;

	lastArrive = 0;

	started = true;

	restartDelay = 0.0f;

	 curVelocity = 0.0f;
	lastVelocity = 0.0f;

	prevVel = 0.0f;

	nodeStarted = true;

	master.Reset();

//	firstTime = true;

	fade = fade_none;

	fadeAlpha	 = 1.0f;
	fadeAlphaMin = 0.3f;

	fadeTime = 2.0f;

//	curModel = "";

	///////////////

	arbiter = null;

	debugDraw = false;

	saved = false;
}

AnimatedModel::~AnimatedModel()
{
	if( model )
		model->Release();

	if( animation )
	{
		events.DelAnimation(animation);
		animation->Release();
	}

	if( arbiter )
		arbiter->Activate(this,false);
}

bool AnimatedModel::Create(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

bool AnimatedModel::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void AnimatedModel::Activate(bool isActive)
{
	MissionObject::Activate(isActive);

	if( animation )
		animation->Pause(!isActive);

	if( isActive )
		LogicDebug("Activate");
	else
		LogicDebug("Deactivate");

	if( arbiter )
		arbiter->Activate(this,IsShow() && isActive);

	if( IsActive() && saved )
	{
		GotoNode(saveTrack,saveNode,saveTime);

		saved = false;
	}
}

void AnimatedModel::Show(bool isShow)
{
	MissionObject::Show(isShow);

	if( EditMode_IsOn())
		DelUpdate(&AnimatedModel::Draw);

	long level = drawLevel;

	if( isShow /*&& enableRender*/ )
	//	SetUpdate(&AnimatedModel::Draw,ML_ALPHA5);
		SetUpdate(&AnimatedModel::Draw,level);
	else
		DelUpdate(&AnimatedModel::Draw);

	if( isShow )
	{
		if( shadowCast )
			Registry  (MG_SHADOWCAST,&AnimatedModel::ShadowInfo,level);
		else
			Unregistry(MG_SHADOWCAST);

		if( shadowReceive )
		{
			Unregistry(MG_SHADOWDONTRECEIVE);
			Registry  (MG_SHADOWRECEIVE,&AnimatedModel::ShadowDraw,level);
		}
		else
		{
			Unregistry(MG_SHADOWRECEIVE);

			if( level < ML_ALPHA1 && enableRender )
				Registry(MG_SHADOWDONTRECEIVE,&AnimatedModel::ShadowDraw,level);
		}

		if( seaReflection )
			Registry  (MG_SEAREFLECTION,&AnimatedModel::SeaReflection,level);
		else
			Unregistry(MG_SEAREFLECTION);

		if( seaRefraction )
			Registry  (MG_SEAREFRACTION,&AnimatedModel::SeaReflection,level);
		else
			Unregistry(MG_SEAREFRACTION);
	}
	else
	{
		events.ResetParticles();

		Unregistry(MG_SHADOWCAST);
		Unregistry(MG_SHADOWRECEIVE);

		Unregistry(MG_SEAREFLECTION);
		Unregistry(MG_SEAREFRACTION);
	}

	if( isShow )
		LogicDebug("Show");
	else
		LogicDebug("Hide");

	if( arbiter )
		arbiter->Activate(this,IsActive() && isShow);
}

void AnimatedModel::GetBox(Vector &min, Vector &max)
{
	min = 0.0f;
	max = 0.0f;

	if( model )
	{
		const GMXBoundBox box = model->GetLocalBound();

		min = box.vMin;//*modelScale;
		max = box.vMax;//*modelScale;
	}
}

void AnimatedModel::Restart()
{
	started = true;
	Reset();

	ReCreate();
}

Matrix &AnimatedModel::GetMatrix(Matrix &m)
{
/*	Matrix zz;

	MissionObject *p = master.Ptr();

	if( p )
		zz = p->GetMatrix(zz);

	return m = matModel*zz;*/

	m = matModel;
	m.Normalize();

	MissionObject *p = master.Ptr();

	if( p )
	{
		Matrix zz(true);
		zz = p->GetMatrix(zz);

		m *= zz;
	}

	return m;
}

void _cdecl AnimatedModel::ShadowInfo(const char *group, MissionObject *sender)
{
	if( model )
	{
		model->SetTransform(matModel);

		const Vector &vMin = model->GetBound().vMin;
		const Vector &vMax = model->GetBound().vMax;

		((MissionShadowCaster *)sender)->AddObject(
			this,&AnimatedModel::ShadowDraw,vMin,vMax);
	}	
}

void _cdecl AnimatedModel::ShadowDraw(const char *group, MissionObject *sender)
{
	if( !EditMode_IsVisible() || !IsShow())
		return;

	if( model )
	{
		Matrix zz;

		MissionObject *p = master.Ptr();

		if( p )
			zz = p->GetMatrix(zz);

		model->SetTransform(matModel*zz);

	//	model->Draw();
		DrawModel(false);
	}
}

void _cdecl AnimatedModel::SeaReflection(const char *group, MissionObject *sender)
{
	if( model )
	{
	//	Draw(0.0f,0);

		Matrix zz;

		MissionObject *p = master.Ptr();

		if( p )
			zz = p->GetMatrix(zz);

		model->SetUserColor(color);
		model->SetTransform(matModel*zz);

	//	model->Draw();
		DrawModel(false);
	}
}

void AnimatedModel::DrawModel(bool primary)
{
	if( primary &&
		enableRender == false )
		return;

	if( draw_bounds )
	{
		Vector vMin;
		Vector vMax;

		GetBox(vMin,vMax);

		Render().DrawBox(vMin,vMax,matModel);
	}

	const GMXBoundSphere &sph = model->GetBoundSphere();
	float r = sph.fRadius*modelScale;

	if( draw_bounds )
		Render().DrawSphereGizmo(sph.vCenter,r,-1,-1);

	const Plane * planes = Render().GetFrustum();
	for(dword i = 0; i < 4; i++)
	{
		if(planes[i].Dist(sph.vCenter) < -r)
		{
			return;
		}
	}

	bool s = Geometry().GetGlobalFrustumCullState();
	Geometry().SetGlobalFrustumCullState(false);

	Color old = model->GetUserColor();
	Color c = old;

	c.a *= fadeAlpha;

	model->SetUserColor(c);

	Matrix view = Render().GetView();

	if( !noSwing )
	{
		Render().SetView((Mission().GetInverseSwingMatrix()*Matrix(view).Inverse()).Inverse());
	}

	model->Draw();

	if( !noSwing )
	{
		Render().SetView(view);
	}

	model->SetUserColor(old);

	Geometry().SetGlobalFrustumCullState(s);

}

void AnimatedModel::PostCreate()
{
	if( masterName.NotEmpty() && !master.Ptr())
		FindObject(masterName,master);
}

void _cdecl AnimatedModel::Draw(float dltTime, long)
{
	bool edit_mode = EditMode_IsOn();

	if( fade > fade_none )
	{
		static const float fadeSpeed = 2.0f;
		static const float hideSpeed = 0.7f;

	//	float da = dltTime/fadeTime;

		if( fade == fade_out )
		{
		//	fadeAlpha -= da;
			fadeAlpha -= dltTime*hideSpeed;

			if( fadeAlpha < fadeAlphaMin )
			{
				fadeAlpha = fadeAlphaMin;
				fade = fade_none;
			}
		}
		else
		{
		//	fadeAlpha += da;
			fadeAlpha += dltTime*fadeSpeed;

			if( fadeAlpha > 1.0f )
			{
				fadeAlpha = 1.0f;
				fade = fade_none;
			}
		}
	}

	if( masterName.NotEmpty() )
	{
		if( edit_mode )
		{
			if( !master.Validate())
				 FindObject(masterName,master);
		}
		else
		{
		/*	if( firstTime )
			{
				if( !master )
					 master = FindObject(masterName);
			}*/
		}
	}

	if(!EditMode_IsVisible())
		return;

	if( dltTime > 0.5f )
		dltTime = 0.5f;

	bool additionalDraw = Mission().EditMode_IsAdditionalDraw();

	if( started )
	{
		dword tn = (edit_mode && editing)? editTrack : startTrack;

		GotoNode(tn,0);

//		Animation().Update(dltTime);

		started = false;
	}

	bool selected = EditMode_IsSelect() || !edit_mode;

	if( !tracks.Size())
	{
		if( model )
		{
			if( !selected )
				model->SetUserColor(0x80000000);

			Matrix z; z.BuildScale(modelScale/trackScale);

			z = z*matWorld;

			Matrix zz;

			MissionObject *p = master.Ptr();

			if( p )
				zz = p->GetMatrix(zz);
			
			model->SetTransform(z*zz);

			DrawModel();

			matModel = z; events.Update(z*zz, dltTime);

			if( !selected )
				model->SetUserColor(color);
		}

		return;
	}

	bool preview = edit_mode && !editing;

	if( preview && (!selected || !additionalDraw) && model )
	{
		Track &track = tracks[0];
		Node  &node	 = track.nodes[0];

		Matrix m;

		track.GetNodeMatrix(m,node,matWorld,matAngle,modelScale);

	//	Matrix z; z.BuildScale(1.0f/trackScale);

	//	z = z*m;

	//	matModel = z;

		matModel = m;

		model->SetUserColor(0x80000000);

		Matrix zz;

		MissionObject *p = master.Ptr();

		if( p )
			zz = p->GetMatrix(zz);

		model->SetTransform(m*zz);

	//	model->Draw();
		DrawModel();

		model->SetUserColor(color);

		events.Update(m*zz, dltTime);

		return;
	}

	if( !IsActive() || preview )
		dltTime = 0;

	if( edit_mode && trackTime >= 0.0f )
		dltTime = 0;

	if( edit_mode && additionalDraw && selected )
	{
		dword tn = editing ? editTrack : -1;

		for( dword ti = 0 ; ti < tracks.Size() ; ti++ )
		{
			dword size = tracks[ti].nodes;

			if( ti != tn && size )
			{
				for( dword i = 0 ; i < size ; i++ )
				{
					if( i < size - 1 )
						tracks[ti].DrawPath(i,i + 1,Render(),matWorld,false,false);
					else
						if( tracks[ti].looped )
							tracks[ti].DrawPath(i,0,Render(),matWorld,false,false);
				}
			}
		}

		for( dword ti = 0 ; ti < tracks.Size() ; ti++ )
		{
			Track &track = tracks[ti];

			array<Node> &nodes = track.nodes;

			dword last = nodes.Size() - 1;

			for( dword i = 0 ; i <= last ; i++ )
			{
				Node &node = nodes[i];
				Node &next = i < last ? nodes[i + 1] : nodes[0];

				if( i < last )
					track.DrawPath(i,i + 1,Render(),matWorld,showRotation,showScale);
				else
					if( track.looped )
						track.DrawPath(i,0,Render(),matWorld,showRotation,showScale);

				Vector node_pos = node.pos*matWorld;

			//	if( editMode > edit_path )
				{
					if( !showThrough )
					{
						Matrix m;

						track.GetNodeMatrix(m,node,matWorld,matAngle,modelScale);

						Vector vz = m.vz; vz.Normalize();
						Vector vy = m.vy; vy.Normalize();
						Vector vx = m.vx; vx.Normalize();

						Vector look_to = node_pos + vz;
						Vector look_up = node_pos + vy;
						Vector look_rg = node_pos + vx;

						Render().DrawLine(node_pos,0xa00000ff,look_to,0xa00000ff);
						Render().DrawSphere(look_to,0.05f,0xa00000ff);

						Render().DrawLine(node_pos,0xa000ff00,look_up,0xa000ff00);
						Render().DrawSphere(look_up,0.05f,0xa000ff00);

						Render().DrawLine(node_pos,0xa0ff0000,look_rg,0xa0ff0000);
						Render().DrawSphere(look_rg,0.05f,0xa0ff0000);

						if( model )
						{
							model->SetUserColor(0x80000000);

						//	Matrix z; z.BuildScale(1.0f/trackScale);

						//	z = z*m;

							Matrix zz;

							MissionObject *p = master.Ptr();

							if( p )
								zz = p->GetMatrix(zz);

							model->SetTransform(m*zz);

						//	model->Draw();
							DrawModel();

						//	model->SetUserColor(0xff000000);
							model->SetUserColor(color);
						}
					}

					switch( editMode )
					{
						case edit_rotation_z:
							if( showRotation )
								DrawHandle(node.zp,node.ztg,0x900000ff);
							break;

						case edit_rotation_y:
							if( showRotation )
								DrawHandle(node.yp,node.ytg,0x90008f00);
							break;

						case edit_rotation_x:
							if( showRotation )
								DrawHandle(node.xp,node.xtg,0x900000ff);
							break;

						case edit_scale:
							if( showScale )
								DrawHandle(node.sp,node.stg,0x90808080);
							break;
					}
				}

				Vector t = node_pos + node.tangent *matAngle*0.5f;
				Vector u = node_pos + node.binormal*matAngle*0.5f;

				Node &prev = i ? nodes[i - 1] : nodes[nodes.Last()];

				if( editMode == edit_path && last != 0 )
					Render().DrawLine(t,0xa0ffff00,u,0xa0ffff00);

				switch( node.moveMode )
				{
					case move_arrive:
						Render().Print(node_pos,30.0f,2.0f,0xffff0000,
						//	"Arrive [%d] %u",i,node.bakes.Size());
							"Arrive [%d]",i);
						break;

					case move_wait:
						Render().Print(node_pos,30.0f,2.0f,0xffff0000,
							"Wait for [%d]\n%.1f sec.",i,node.time);
						break;

					case move_linear:
						Render().Print(node_pos,30.0f,2.0f,0xffff0000,
							"Linear by [%d]\n%.1f sec.\n%.1f m.",i,node.time,node.len);
						break;

				/*	case move_accel:
						Render().Print(node_pos,30.0f,2.0f,0xffff0000,
							"Accel by [%d]\n%.1f sec.",i,node.time);
						break;*/

					case move_step:
						Render().Print(node_pos,30.0f,2.0f,0xffff0000,
							"Step [%d]",i);
						break;
				}

				Render().DrawSphere(node_pos,0.09f,0xffff0000);

				if( editMode == edit_path && last != 0 )
				{
					Render().DrawSphere(t,0.04f,0xff00ff00);
					Render().DrawSphere(u,0.04f,0xff00ff00);
				}
			}

			if( /*editMode > edit_path &&*/ showThrough )
			{
				if( model )
					model->SetUserColor(0x80000000);

				Vector lookVec;	Vector upVec = Vector(0.0f,1.0f,0.0f);
				Vector pos;

				float za;
				float ya;
				float xa;
			
				float sc;

				Matrix m;

				float t = 0;
				float s = 0;

				bool end = false;

				Vector prev = nodes[0].pos;

				dword to = track.looped ? last : last - 1;

				for( dword i = 0 ; !end ; i++ )
				{
					if( i > to )
					{
						end = true;
						i = 0;
					}

					if( i >= nodes.Size())
						break;

					Node &node = nodes[i];

					while( t < node.len )
					{
						GetPosition(track,i,t,lookVec,upVec,za,ya,xa,sc,pos,track.lookMode);

						if( sc > 0.0f && sc <  0.1f )
							sc =  0.1f;

						if( sc < 0.0f && sc > -0.1f )
							sc = -0.1f;

						float s_ = fabsf(sc);

						if( s && s > s_ )
						{
							t -= 2.0f*s /capacity;
							t += 2.0f*s_/capacity;

							if( t < 0 )
							{
								i = i ? i - 1 : to;

								t = nodes[i].len + t;
							}
							else
								if( end )
									break;

							GetPosition(track,i,t,lookVec,upVec,za,ya,xa,sc,pos,track.lookMode);

							if( sc > 0.0f && sc <  0.1f )
								sc =  0.1f;

							if( sc < 0.0f && sc > -0.1f )
								sc = -0.1f;

							s_ = fabsf(sc);
						}
						else
							if( end )
								break;

						s = s_;

						float d = (pos - prev).GetLength();

						prev = pos;

						Vector look;
						
						if( track.lookMode != look_full )
							look = lookVec.GetXZ();
						else
							look = lookVec;

						m.BuildScale(modelScale*sc);

						Matrix n;

						if( track.lookMode >= look_follow )
							n.BuildOrient(look,upVec);

						lastLook = look;

						m.Rotate(Vector(  0,ya,  0));
						m.Rotate(Vector(-xa, 0,  0));
						m.Rotate(Vector(  0, 0,-za));

						m = m*n;

						if( track.lookMode == look_static )
						{
							m = m*matAngle;
						}

						m.Move(pos);

						if( model )
						{
						//	Matrix z; z.BuildScale(1.0f/trackScale);

						//	z = z*m;

							Matrix zz;

							MissionObject *p = master.Ptr();

							if( p )
								zz = p->GetMatrix(zz);

							model->SetTransform(m*zz);

						//	model->Draw();
							DrawModel();
						}

						t += 2.0f*s_/capacity;
					}

					t = t - node.len;
				}

				if( model )
					model->SetUserColor(color);
			}
		}
	}

	Track &track = tracks[curTrack];

	array<Node> &nodes = track.nodes;

	dword last = nodes.Size() - 1;

	bool canRun = (curNode != last || track.looped);

	if( model && !(edit_mode && hideInEditor))
	{
	//	Vector vmin,vmax; GetBox(vmin,vmax);

	//	Render().DrawBox(vmin,vmax);

		///////////////////////////

		if( curNode >= nodes.Size())
			GotoNode(curTrack,0);

		///////////////////////////

		Node &node = nodes[curNode];

		float velocity = 0.0f;

		if( canRun )
		{
			if( IsActive() && !preview && dltTime > 0.0f )
			{
				curTime += dltTime;

				if( node.moveMode == move_wait )
				{
					float len = node.len;

					if( len == 0.0f )
						len  = 1.0f;

					if( curTime >= node.time )
						curPos = len;
					else
						CheckLinks();
				}

				if( node.moveMode == move_step && animation )
				{
					const char *p = animation->CurrentNode();

					if( animationNode != p )
					{
						animationNode  = p;

						curPos = node.len;
					}
					else
						CheckLinks();
				}

				if( node.moveMode != move_step && animation )
				{
					animationNode = animation->CurrentNode();
				}
			}
		}
		else
			curTime = 0.0f;

		Matrix m;

		if((node.moveMode == move_arrive ||
			node.moveMode == move_linear ) && canRun )
		{
			if( IsActive() && !preview && dltTime > 0.0f)
			{
				if( nodeStarted )
				{
					curTime = 0.0f;

					nodeStarted = false;
				}

				float k;

				if( node.smooth > 0.0f )
				{
					k = curTime/node.smooth;

					if( k > 1.0f )
						k = 1.0f;
				}
				else
					k = 1.0f;

				if( node.moveMode == move_arrive )
				{
					Vector dPos(0.0f); float dAng = 0.0f;

					if( animation )
						animation->GetMovement(dPos);

				//	float dt = dPos.GetLength()*modelScale;
					float dt = dPos.GetLength();

					if( dt != 0.0f )
					{
						float vel = dt*lastScale/dltTime;

						velocity = Lerp(lastVelocity,vel,k);

						curPos += lastArrive = velocity*dltTime;

						delay = 0.0f;

						curVelocity = velocity;
					}
					else
					{
						if( delay > 0.01f )
						{
							curPos = node.len;

							velocity = curVelocity;
						}
						else
						{
							velocity = curVelocity;

							curPos += velocity*dltTime;

							delay  += dltTime;
						}
					}
				}
				else
				{
					if( node.len == 0.0f )
					{
						curPos = curTime/node.time;
					}
					else
					{
					/*	float vel = node.len/node.time;

						velocity = Lerp(lastVelocity,vel,k);

						if( prevVel == 0.0f )
							prevVel  = lastVelocity;

						curPos += prevVel*dltTime;

						prevVel = velocity;*/

						if( curTime >= node.smooth )
						{
							velocity = node.len/node.time;

							curPos += velocity*dltTime;
						}
						else
						{
							float a = (node.len/node.time - lastVelocity)/node.smooth;

							velocity = lastVelocity + a*curTime;

							curPos = lastVelocity*curTime + 0.5f*a*curTime*curTime;
						}
					}
				}
			}

			float t = curPos;

			float len = node.len;

			if( len == 0.0f )
				len  = 1.0f;

			if( t > len )
				t = len;

			Vector lookVec; Vector upVec = Vector(0.0f,1.0f,0.0f);
			Vector pos;

			float za;
			float ya;
			float xa;
		
			float sc;

			GetPosition(track,curNode,t,lookVec,upVec,za,ya,xa,sc,pos,track.lookMode);

			if( sc > 0.0f && sc <  0.1f )
				sc =  0.1f;

			if( sc < 0.0f && sc > -0.1f )
				sc = -0.1f;

			Vector look;

			if( track.lookMode != look_full )
				look = lookVec.GetXZ();
			else
				look = lookVec;

		//	look.Lerp(lastLook,look,0.1f);
		//	look.Lerp(lastLook,look,0.5f);

			m.BuildScale(modelScale*sc);

			Matrix n;

			if( track.lookMode >= look_follow )
				n.BuildOrient(look,upVec);

			lastLook = look;

			m.Rotate(Vector(  0,ya,  0));
			m.Rotate(Vector(-xa, 0,  0));
			m.Rotate(Vector(  0, 0,-za));

			m = m*n;

			if( track.lookMode == look_static )
			{
				m = m*matAngle;
			}

			m.Move(pos);

			lastScale = modelScale*fabsf(sc);
		}
		else
		{
			track.GetNodeMatrix(m,node,matWorld,matAngle,modelScale);

			lastScale = modelScale*node.sc;
		}

	//	if( curPos <= node.len )
		if( !preview )
		{
		//	Matrix z; z.BuildScale(1.0f/trackScale);

		//	z = z*m;

		//	matModel = z;
			matModel = m;

			if( !selected )
				model->SetUserColor(0x80000000);

			Matrix zz;

			MissionObject *p = master.Ptr();

			if( p )
				zz = p->GetMatrix(zz);

			model->SetTransform(m*zz);

		//	model->Draw();
			DrawModel();

			if( !selected )
				model->SetUserColor(color);

			events.Update(m*zz, dltTime);

			if( trackDraw )
				Render().Print(0.0f,0.0f,0xffffffff,"%f",node.timeTotal + curTime);
		}

		if( edit_mode && trackTime >= 0.0f )
			return;

		float len = node.len;

		if( len == 0.0f )
			len  = 1.0f;

		if( curPos >= len && canRun && (node.moveMode != move_wait || curTime >= node.time))
		{
			prevVel = lastVelocity = velocity;

			curNode = curNode < last ? curNode + 1 : 0;

			GotoNode(curTrack,curNode,curPos - node.len);

			CheckLinks();
		}

		if( !canRun && !preview )
		{
			CheckLinks();

			if( edit_mode )
			{
				restartDelay += dltTime;

				if( restartDelay > 1.5f )
				{
					started = true;

					Reset();
				}
			}
		}
	}
}

void AnimatedModel::DrawHandle(Vector &pt, Vector &tg, dword color)
{
	Vector node_pt = pt*matWorld;
	Vector node_tg = tg*matAngle;

	Render().DrawSphere(node_pt,0.05f,color);

	Vector t = node_pt + node_tg*0.5f;
	Vector u = node_pt - node_tg*0.5f;

	Render().DrawLine(t,0xa0ffff00,u,0xa0ffff00);

	Render().DrawSphere(t,0.04f,0xff00ff00);
	Render().DrawSphere(u,0.04f,0xff00ff00);
}

void AnimatedModel::GetPosition(
	Track &track, dword ni, float t, Vector &lookVec, Vector &upVec, float &za, float &ya, float &xa, float &sc, Vector &pos, LookMode lookMode)
{
//	Track &track = tracks[curTrack];

	array<Node> &nodes = track.nodes;

	dword last = nodes.Size() - 1;

	if( last == 0 )
	{
		Node &node = nodes[0];

		lookVec = node.tangent*matWorld;

		za = node.za;
		ya = node.ya;
		xa = node.xa;

		sc = node.sc;

		pos = node.pos*matWorld;

		if( lookMode == look_full )
		{
			upVec = (node.tangent^node.vx)*matAngle;
		}

		return;
	}

	Node &node = (nodes[ni]);
	Node &next = (ni != last ? nodes[ni + 1] : nodes[0]);
	Node &prev = (ni		 ? nodes[ni - 1] : nodes.LastE());

	float len = node.len;

	if( len == 0.0f )
		len  = 1.0f;

	float k = t/len;

	Vector a(node.tangent);
	Vector b(next.tangent);

	array<Bake> &bakes = node.bakes;

	if( bakes.Size())
	{
		if( t < bakes[0].time )
		{
			pos.Lerp(node.pos,bakes[0].pos,t/bakes[0].time);

			if( prev.bakes.Size() && (curNode || track.looped))
			{
				float u =
					(prev.len - prev.bakes.LastE().time + t)/
					(prev.len - prev.bakes.LastE().time + bakes[0].time);

				lookVec.Lerp(prev.bakes.LastE().look,bakes[0].look,u);

				za = Lerp(prev.bakes.LastE().za,bakes[0].za,u);
				ya = Lerp(prev.bakes.LastE().ya,bakes[0].ya,u);
				xa = Lerp(prev.bakes.LastE().xa,bakes[0].xa,u);

				sc = Lerp(prev.bakes.LastE().sc,bakes[0].sc,u);
			}
			else
			{
				float u = t/bakes[0].time;

				lookVec.Lerp(a,bakes[0].look,u);

				za = Lerp(node.za,bakes[0].za,u);
				ya = Lerp(node.ya,bakes[0].ya,u);
				xa = Lerp(node.xa,bakes[0].xa,u);

				sc = Lerp(node.sc,bakes[0].sc,u);
			}
		}
		else
		{
			for( dword i = 1 ; i < bakes.Size() ; i++ )
			{
				if( t < bakes[i].time )
					break;
			}

			if( i < bakes.Size())
			{
				float u =
					(t - bakes[i - 1].time)/
					(bakes[i].time - bakes[i - 1].time);

				pos.Lerp(bakes[i - 1].pos,bakes[i].pos,u);

				lookVec.Lerp(bakes[i - 1].look,bakes[i].look,u);

				za = Lerp(bakes[i - 1].za,bakes[i].za,u);
				ya = Lerp(bakes[i - 1].ya,bakes[i].ya,u);
				xa = Lerp(bakes[i - 1].xa,bakes[i].xa,u);

				sc = Lerp(bakes[i - 1].sc,bakes[i].sc,u);
			}
			else
			{
				pos.Lerp(bakes.LastE().pos,next.pos,
					(t - bakes.LastE().time)/(node.len - bakes.LastE().time));

				if( next.bakes.Size() && (curNode < last - 1 || track.looped))
				{
					float u =
						(t - bakes.LastE().time)/
						(node.len - bakes.LastE().time + next.bakes[0].time);

					lookVec.Lerp(bakes.LastE().look,next.bakes[0].look,u);

					za = Lerp(bakes.LastE().za,next.bakes[0].za,u);
					ya = Lerp(bakes.LastE().ya,next.bakes[0].ya,u);
					xa = Lerp(bakes.LastE().xa,next.bakes[0].xa,u);

					sc = Lerp(bakes.LastE().sc,next.bakes[0].sc,u);
				}
				else
				{
					float u =
						(t - bakes.LastE().time)/
						(node.len - bakes.LastE().time);

					lookVec.Lerp(bakes.LastE().look,b,u);

					za = Lerp(bakes.LastE().za,next.za,u);
					ya = Lerp(bakes.LastE().ya,next.ya,u);
					xa = Lerp(bakes.LastE().xa,next.xa,u);

					sc = Lerp(bakes.LastE().sc,next.sc,u);
				}
			}
		}
	}
	else
	{
		pos.Lerp(node.pos,next.pos,k);

		lookVec.Lerp(a,b,k);

		za = Lerp(node.za,next.za,k);
		ya = Lerp(node.ya,next.ya,k);
		xa = Lerp(node.xa,next.xa,k);

		sc = Lerp(node.sc,next.sc,k);
	}

	if( lookMode == look_full )
	{
		if((node.vx|next.vx) > (node.normal|next.normal))
		{
			Vector vx; vx.Lerp(node.vx,next.vx,k);

			upVec = lookVec^vx;
		}
		else
		{
			upVec.Lerp(node.normal,next.normal,k);
		}
	}

	pos = pos*matWorld;

	lookVec = lookVec*matAngle;

	if( lookMode == look_full )
		upVec = upVec*matAngle;
}

float AnimatedModel::DrawDebug(float y)
{
	if( !debugDraw )
		return y;

	const char *name = GetObjectID().c_str();

///////////////////////////

//	Matrix m; GetMatrix(m);

	Matrix zz;

	MissionObject *p = master.Ptr();

	if( p )
		zz = p->GetMatrix(zz);

	Matrix m(matModel,zz);

//////////////////////////

	Render().Print(m.pos,300.0f,2.0f,0xffffffff,name);

	const float ch = 17.0f;
	const float  x = 17.0f;

	Render().Print(x,y,0xffffff00,name);
	y += ch;

	Render().Print(x,y,0xffffffff,"Track: %d",curTrack);
	y += ch;

	Render().Print(x,y,0xffffffff,"Node: %d",curNode);
	y += ch;

	Render().Print(x,y,0xff00ff00,"Links:");
	y += ch;

	for( int i = 0 ; i < links ; i++ )
	{
		const Link &link = links[i];

		if( link.active )
		{
			if( link.id && link.id[0] )
				Render().Print(x,y,0xffffffff,"[%d,%d]>[%d,%d] - [%d]\"%s\"",
					link.srcTrack,link.srcItem,link.dstTrack,link.dstItem,i,link.id);
			else
				Render().Print(x,y,0xffffffff,"[%d,%d]>[%d,%d] - [%d]",
					link.srcTrack,link.srcItem,link.dstTrack,link.dstItem,i);
			y += ch;
		}
	}
	y += 8.0f;

	return y;
}

void AnimatedModel::InitParams(MOPReader &reader)
{
	saved = false;

	MOSafePointer sp;

	static const ConstString objectId("AnimatedModelsArbiter");
	Mission().CreateObject(sp,"AnimatedModelsArbiter",objectId);

	arbiter = (AnimatedModelsArbiter *)sp.Ptr();
	Assert(arbiter);

	switch( reader.Enum().c_str()[0] )
	{
		case 'P':
			editMode = edit_path;
			break;
		case 'Z':
			editMode = edit_rotation_z;
			break;
		case 'Y':
			editMode = edit_rotation_y;
			break;
		case 'X':
			editMode = edit_rotation_x;
			break;
		case 'S':
			editMode = edit_scale;
			break;
	}

	Vector pos = reader.Position();
		   ang = reader.Angles();

	trackScale = reader.Float();

	Matrix m(ang,pos);
	Matrix s;
		   s.BuildScale(trackScale);

	matWorld = s*m;

	matAngle = Matrix(ang);

	modelScale = reader.Float();

	const char * tmp_modelName = reader.String().c_str();

	bool isNewModel = false;
	bool isNewAnime = false;

	if( EditMode_IsOn())
	{
		/////////////////
		// Моделька грузится всегда, так как "...\..\name.gmx"(t) != "name"(model->GetFileName())
		/////////////////
	//	if( !model || !string::IsEqual(model->GetFileName(),t))
		if( !model || curModel != tmp_modelName )
		{
			if( model )
				model->Release();

			model = Geometry().CreateScene(tmp_modelName,&Animation(),&Particles(),&Sound(),_FL_);

			isNewModel = true; curModel = tmp_modelName;
		}
	}
	else
	{
		if( !model && !model_loadFailed )
		{
			model = Geometry().CreateScene(tmp_modelName,&Animation(),&Particles(),&Sound(),_FL_);

			if( !model )
				 model_loadFailed = true;

			isNewModel = true;
		}
	}

	const char * tmp_animationName = reader.String().c_str();

	if( !animation || !string::IsEqual(animation->GetName(),tmp_animationName))
	{
		if( animation )
		{
			events.DelAnimation(animation);
			animation->Release();
		}

		animation = Animation().Create(tmp_animationName, _FL_);

	//	if( model )
	//		model->SetAnimation(animation);

		isNewAnime = true;
	}

	if( isNewModel || isNewAnime )
	{
		if( model )
			model->SetAnimation(animation);
	}

	//////////////////////////////////////////////

	events.Init(&Sound(),&Particles(),&Mission());
	events.InitModels(this, model);

	matModel.Build(ang,pos);

	if( !EditMode_IsOn())
	{
		events.AddAnimation(animation);
		events.SetScene(model,matModel);
	}

	//////////////////////////////////////////////

	ConstString tmp_trackObject = reader.String(); // another object to read tracks from

	bool reset = false;

	if( EditMode_IsOn())
	{
		if( curTracks != tmp_trackObject )
		{
		//	Reset();
			reset = true;

			curTracks = tmp_trackObject;
		}
	}
	else
	{
		reset = true;
	}

	bool show = reader.Bool();
	bool acti = reader.Bool();

	dword start = reader.Long();

	dword editT = reader.Long();

	float ttt = reader.Float();
	trackDraw = reader.Bool();

	showRotation = reader.Bool();
	showScale	 = reader.Bool();

	showThrough = reader.Bool();
	capacity	= reader.Float();

	links.DelAll();

	long linksCount = reader.Array();

	links.AddElements(linksCount);

	for( long i = 0 ; i < linksCount ; i++ )
	{
		Link &link = links[i];

		link.id = reader.String().c_str();

		link.srcTrack = reader.Long(); link.srcItem = reader.Long();
		link.dstTrack = reader.Long(); link.dstItem = reader.Long();

		link.active = reader.Bool();
	}

	//// Render params ////

	drawLevel = reader.Long();

	color = reader.Colors();

	enableRender = reader.Bool();

	bool transparent = reader.Bool();

	//// уровень отрисовки

	long from = transparent ? ML_ALPHA1 : ML_GEOMETRY1;
	long to	  = transparent ? ML_ALPHA3 : ML_GEOMETRY3;

	drawLevel = from + (to - from)*drawLevel/100;

	////

	dynamicLighting = reader.Bool();

	shadowCast	  = reader.Bool();
	shadowReceive = reader.Bool();

	seaReflection = reader.Bool();
	seaRefraction = reader.Bool();

	hideInEditor = reader.Bool();

	noSwing = reader.Bool();


	if( model )
	{
		model->SetUserColor(color);
		model->SetDynamicLightState(dynamicLighting);
		model->SetShadowReceiveState(shadowReceive);
	}

	///////////////////////

	masterName = reader.String();

	FindObject(masterName,master);

	Show	(show);
	Activate(acti);

	MissionObject *p;

	FindObject(tmp_trackObject,sp);
	p = sp.Ptr();
	static const ConstString tid("AnimatedModel");
	if( tmp_trackObject.NotEmpty() && p && p->Is(tid))
	{
		AnimatedModel *q = (AnimatedModel *)p;

		tracks.setExtern(&q->tracks_native);
	}
	else
	{
		tracks.setExtern(null);

		CreateTrackData(reader);
	}

	if( !tracks.Size())
	{
		if( animation )
			animation->Goto(null,0.0f);
	}

	if( start != startTrack )
	{
		startTrack = start;

		if( startTrack >= tracks.Size())
			startTrack = 0;

		reset = true;
	}

	if( editT < tracks.Size())
		editing = true;
	else
		editing = false;

	if( editT != editTrack /*&& preview*/ )
		reset = true;

	editTrack = editT;

	if( reset )
	{
		trackTime = ttt;

		Reset();
	}
	else
	{
		if( trackTime != ttt )
		{
			trackTime  = ttt;
	
			dword tn = (EditMode_IsOn() && editing)? editTrack : startTrack;

			GotoNode(tn,0);
		}
	}
}
/*
struct Data
{
	float len;

	Vector xp;
	Vector yp;
	Vector zp;
	Vector sp;

	Vector xtg;
	Vector ytg;
	Vector ztg;
	Vector stg;

	Data()
	{
	}

	Data(const Node &node)
	{
		len = node.len;

		xp = node.xp;
		yp = node.yp;
		zp = node.zp;
		sp = node.sp;

		xtg = node.xtg;
		ytg = node.ytg;
		ztg = node.ztg;
		stg = node.stg;
	}
};
*/
void AnimatedModel::CreateTrackData(MOPReader &reader)
{
	dword tracksCount = reader.Array();

	tracks.DelAll();

//	if( tracksCount < tracks.Size())
//		tracks.DelAll();

	tracks.AddElements(tracksCount);

/*	IFile *file = null;

	if( !EditMode_IsOn())
	{
		string path = "resource\\missions\\";

		path += Mission().GetMissionName();
		path += "\\timelines\\";
		path += this->GetObjectID();
		path += ".tln";

		file = Files().OpenFile(path,file_open_existing_for_read,_FL_);
	}*/

	for( dword i = 0 ; i < tracksCount ; i++ )
	{
		Track &track = tracks[i];

		track.looped = reader.Bool();

		switch( reader.Enum().c_str()[0] )
		{
			case 'S':
				track.lookMode = look_static;
				break;
			case 'F':
				track.lookMode = look_follow;
				break;
			case 'U':
				track.lookMode = look_full;
				break;
		}

		long nodesCount = reader.Array();

		bool focused = true;

		track.focusedNode = -1;

		track.nodes.AddElements(nodesCount);

		float timeTotal = 0.0f;

		for( long j = 0 ; j < nodesCount ; j++ )
		{
			Node &node = track.nodes[j];

			node.Init(reader);

			node.timeTotal = timeTotal; timeTotal += node.time;
		}

		if( curTrack == i && curNode >= (dword)nodesCount )
		{
			GotoNode(i,0);
		}

		if( nodesCount < 2 )
		{
			//

			continue;
		}

	/*	if( file )
		{
			for( int n = 0 ; n < track.nodes ; n++ )
			{
				Node &node = track.nodes[n];

				Data  data; file->Read(&data,sizeof(Data));

				node.len = data.len;

				node.xp = data.xp;
				node.yp = data.yp;
				node.zp = data.zp;
				node.sp = data.sp;

				node.xtg = data.xtg;
				node.ytg = data.ytg;
				node.ztg = data.ztg;
				node.stg = data.stg;

				dword size; file->Read(&size,4);

				node.bakes.AddElements(size/sizeof(Bake));
			}

			for( int n = 0 ; n < track.nodes ; n++ )
			{
				array<Bake> &bakes = track.nodes[n].bakes;

				dword size = bakes.GetDataSize();

				if( file->Read(bakes.GetBuffer(),size) != size )
				{
					api->Error("AM Loader: invalid binary data");
				}
			}
		}
		else*/
			track.CreateData();
	}

/*	if( file )
		file->Release();*/
}
/*
bool AnimatedModel::EditMode_Export()
{
	if( !Mission().GetMissionName()[0] )
		return true;

	string path = "resource\\missions\\";

	path += Mission().GetMissionName();
	path += "\\timelines\\";
	path += this->GetObjectID();
	path += ".tln";

	IFile *file = Files().OpenFile(path,file_create_always,_FL_);

	if( file )
	{
		for( dword t = 0 ; t < tracks.Size() ; t++ )
		{
			for( int n = 0 ; n < tracks[t].nodes ; n++ )
			{
				const array<Bake> &bakes = tracks[t].nodes[n].bakes;

				Data data(tracks[t].nodes[n]);

				if( file->Write(&data,sizeof(Data)) != sizeof(Data))
					return false;

				dword size = bakes.GetDataSize();

				if( file->Write(&size,4) != 4 )
					return false;
			}

			for( int n = 0 ; n < tracks[t].nodes ; n++ )
			{
				const array<Bake> &bakes = tracks[t].nodes[n].bakes;

				dword size = bakes.GetDataSize();

				if( file->Write(bakes.GetBuffer(),size) != size )
					return false;
			}
		}

		file->Release(); return true;
	}
	else
	{
		api->Error("AM Export: can't create file %s",path.c_str());
		return false;
	}
}
*/
void AnimatedModel::Command(const char *id, dword numParams, const char **params)
{
	if( string::IsEmpty(id))
		return;

	if( string::IsEqual(id,"link"))
	{
		if( numParams < 2 )
		{
			LogicDebugError("Command <link> error. Not enought parameters.");
			return;
		}

		if( !params[0] )
		{
			LogicDebugError("Command <link> error. Index not specified.");
		//	return;
		}

		long index = atol(params[0]);

		if( dword(index) < links.Size())
		{
			bool newState = false;

			if( params[1] )
			{
				switch( params[1][0] )
				{
					case '1':
					case 't':
					case 'T':
						newState = true;
						break;
				}
			}

			links[index].active = newState;

			LogicDebug(
				"Command <link>. link #%i is %s",
				index,newState ? "activated" : "deactivated");
		}
		else
		{
			LogicDebugError(
				"Command <link> error. Index value %i not in range [0, %i].",
				index,links.Size() - 1);
			return;
		}
	}
	else
	if( string::IsEqual(id,"linkByID"))
	{
		if( numParams < 2 )
		{
			LogicDebugError("Command <link> error. Not enought parameters.");
			return;
		}

		if( !params[0] )
		{
			LogicDebugError("Command <link> error. Link ID not specified.");
			return;
		}

		bool newState = false;

		if( params[1] )
		{
			switch( params[1][0] )
			{
				case '1':
				case 't':
				case 'T':
					newState = true;
					break;
			}
		}

		const char *id = params[0];

		bool present = false;

		for( int i = 0 ; i < links ; i++ )
		{
			if( string::IsEqual(id,links[i].id))
			{
				links[i].active = newState;

				LogicDebug(
					"Command <link>. link by ID (%s) is %s",id,newState ? "activated" : "deactivated");

				present = true;
			}
		}

		if( !present )
		{
			LogicDebugError(
				"Command <link> error. Link not found by ID (%s).",id);
			return;
		}
	}
	else
	if( string::IsEqual(id,"reset"))
	{
		started = true;

		Reset();

		LogicDebug("Command <reset>. Reset complete.");
	}
	else
	if( string::IsEqual(id,"teleport"))
	{
		if( numParams < 1 )
		{
			LogicDebugError("Command <teleport> error. Not enought parameters.");
			return;
		}

		if( !params[0] )
		{
			LogicDebugError("Command <teleport> error. Locator name not specified.");
			return;
		}

		MOSafePointer sp;
		FindObject(ConstString(params[0]),sp);
		MissionObject * p = sp.Ptr();

		if( p )
		{
			Matrix m; p->GetMatrix(m);

		/*	Vector pos = reader.Position();
				   ang = reader.Angles();

			trackScale = reader.Float();

			Matrix m(ang,pos);
			Matrix s;
				   s.BuildScale(trackScale);

			matWorld = s*m;

			matAngle = Matrix(ang);*/

			Matrix s;
				   s.BuildScale(trackScale);

			matWorld = s*m;

			m.pos = 0.0f;

			ang = m.GetAngles();

			matAngle = Matrix(ang);

			LogicDebug("Command <teleport>. Teleport by locator \"%s\" complete.",params[0]);
		}
		else
		{
			LogicDebugError("Command <teleport> error. Locator \"%s\"not found.",params[0]);
			return;
		}
	}
	else
	if( string::IsEqual(id,"fadeout"))
	{
		fade = fade_out;

		LogicDebug("Command <fadeout>.");
	}
	else
	if( string::IsEqual(id,"fadein"))
	{
		fade = fade_in;

		LogicDebug("Command <fadein>.");
	}
	else
	{
		LogicDebugError("Unknown command \"%s\".",id);
	}
}

MOP_BEGINLISTCG(AnimatedModel, "Animated model", '1.00', 100, "Animated model\n\n    Use to animate models by tracks\n\nAviable commands list:\n\n    link     - enable/disable link\n\n        param[0] - link index\n        param[1] - 1/0\n\n    linkByID - enable/disable link by ID\n\n        param[0] - link ID\n        param[1] - 1/0\n\n    reset - reset animation\n\n        no parameters\n\n    teleport - get tracks plase from MissionLocator\n\n        param[0] - MissionLocator name", "Geometry")

	MOP_ENUMBEG("MovieCtrMode")
		MOP_ENUMELEMENT("Arrive to the next point")	// Перемещатся по треку со скоростью, считываемой из анимации
		MOP_ENUMELEMENT("Wait time out")			// Ожидать в позиции узла заданное время, затем сразу перейти на другой нод
		MOP_ENUMELEMENT("Linear time move")			// Перемещатся линейно во времени
		MOP_ENUMELEMENT("Step by change node")		// Перейти на другой узел трека как только сменится нод анимации
	MOP_ENUMEND

	MOP_ENUMBEG("EditMode")
		MOP_ENUMELEMENT("Path")
		MOP_ENUMELEMENT("X rotation")
		MOP_ENUMELEMENT("Y rotation")
		MOP_ENUMELEMENT("Z rotation")
		MOP_ENUMELEMENT("Scale")
	MOP_ENUMEND

	MOP_ENUMBEG("LookMode")

		MOP_ENUMELEMENT("Use the up vector")
		MOP_ENUMELEMENT("Follow the track")
		MOP_ENUMELEMENT("Static")

	MOP_ENUMEND

	MOP_ENUM("EditMode", "Editing mode")

	MOP_POSITIONC("Tracks position", Vector(0.0f), "Change tracks position")
	MOP_ANGLESC("Tracks angles", Vector(0.0f), "Rotate tracks")
	MOP_FLOATC("Tracks scale", 1.0f, "Change tracks scale")
	MOP_FLOATC("Model scale", 1.0f, "Change model scale")
	MOP_STRINGC("Model", "", "Model name")
	MOP_STRINGC("Animation", "", "Animation name")
	MOP_STRINGC("Animated model tracks", "", "Use tracks from another animated model")
	MOP_BOOLC("Show", true, "Show or hide geometry in time of mission start")
	MOP_BOOLC("Active", true, "Animate or pause in time of mission start")
	MOP_LONGC("Start track", 0, "Track to start")
	MOP_LONGC("Edited track", 0, "Track to edit")

	MOP_FLOAT("Debug time", -1.0f)
	MOP_BOOLC("Debug draw", false, "Print current in-track time")

	MOP_BOOLC("Show rotation", false, "Show model rotation graph")
	MOP_BOOLC("Show scale", false, "Show model scale graph")

	MOP_BOOLC("Show through", false, "Show multiple copies of model trough the track")
	MOP_FLOATEXC("Capacity", 1.0f, 0.0f, 10.0f, "Relative capacity of model copies")

	MOP_ARRAYBEG("Links", 0, 1000)

		MOP_STRING("Id", "")
		MOP_LONGC("Source track", 0, "Source track index")
		MOP_LONGC("Source item", 0, "Source item index")
		MOP_LONGC("Destination track", 0, "Source track index")
		MOP_LONGC("Destination item", 0, "Source item index")
		MOP_BOOL("Is active", false)

	MOP_ARRAYEND

	MOP_GROUPBEG("Render params")

		MOP_LONGEXC("Level", 0, 0, 100, "Order of geometry draw")	
		MOP_COLOR("Color", Color(0.0f, 0.0f, 0.0f, 1.0f))	
		MOP_BOOLC("Render", true, "Enable primary render")
		MOP_BOOLC("Transparency", false, "Geometry draw as transparency (i.e. light rays)")
		MOP_BOOL("Dynamic lighting", false)
		MOP_BOOLC("Shadow cast", false, "Geometry can is shadow cast by some objects")
		MOP_BOOLC("Shadow receive", false, "Geometry can is shadow receive from casting objects")
		MOP_BOOLC("Sea reflection", false, "Geometry can reflect in sea")
		MOP_BOOLC("Sea refraction", false, "Geometry can refract in sea")
		MOP_BOOLC("Hide in editor", false, "Hide invisible geometry in editor")				

	MOP_GROUPEND()

	MOP_BOOLC("No swing", true, "No swing model in swing machine")

	MOP_STRINGC("Master object", "", "Get global position from this object")

	MOP_ARRAYBEG("Tracks", 0, 100)

		MOP_BOOLC("Is loop", false, "Loop the track")

		MOP_ENUM("LookMode", "Looking mode")

		MOP_ARRAYBEG("Nodes", 1, 1000)

			MOP_POSITION("Position", Vector (0.0f));
			MOP_ANGLES("Tangent rotate", Vector (0.0f));
			MOP_FLOAT("Tangent len", 1.0f);

			MOP_FLOATC("Az", 0.0f, "Rotation angle by the OZ-axis")
			MOP_FLOATC("Az tangent rotate", 0.0f, "Tangent angle of the rotation graph")
			MOP_FLOATC("Az tangent len", 1.0f, "Tangent length of the rotation graph")

			MOP_FLOATC("Ay", 0.0f, "Rotation angle by the OY-axis")
			MOP_FLOATC("Ay tangent rotate", 0.0f, "Tangent angle of the rotation graph")
			MOP_FLOATC("Ay tangent len", 1.0f, "Tangent length of the rotation graph")

			MOP_FLOATC("Ax", 0.0f, "Rotation angle by the OX-axis")
			MOP_FLOATC("Ax tangent rotate", 0.0f, "Tangent angle of the rotation graph")
			MOP_FLOATC("Ax tangent len", 1.0f, "Tangent length of the rotation graph")

			MOP_FLOATC("Scale", 1.0f, "Scale modificator")
			MOP_FLOATC("Scale tangent rotate", 0.0f, "Tangent angle of the scale graph")
			MOP_FLOATC("Scale tangent len", 1.0f, "Tangent length of the scale graph")

			MOP_ENUM("MovieCtrMode", "Mode")
			MOP_FLOATC("Time", 1.0f, "Time to move to the next node")
			MOP_FLOATC("Morphing time", 0.0f, "Time to blend from current velocity to set")
			MOP_STRINGC("Node", "", "Animation node name")
			MOP_FLOATEXC("Blend time", 0.2f, 0.0f, 100.0f, "Blend time for move from current node to set")
			MOP_MISSIONTRIGGER("")

		MOP_ARRAYEND

	MOP_ARRAYEND

MOP_ENDLIST(AnimatedModel)
