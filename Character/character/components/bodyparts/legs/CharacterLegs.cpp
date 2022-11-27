#include "CharacterLegs.h"

#include "..\..\..\Character.h"

#include "..\..\..\Components\CharacterLogic.h"
#include "..\..\..\Components\CharacterPhysics.h"

static IConsole *console = null;

CharacterLegs:: CharacterLegs()
{
	chr = null;
	ani = null;

	console = (IConsole *)api->GetService("Console");

	enabled = false;

	key	= false;
}

CharacterLegs::~CharacterLegs()
{
	//
}

void CharacterLegs::Init(const InitData &data)
{
	if( data.chr->IsPlayer() == false )
		return;

	chr = data.chr;
	ani = data.animation;

	if( ani )
		Invalidate();
}

void CharacterLegs::Invalidate()
{
	lleg.th = ani->FindBone("Bedro_left_joint"  ,true);
	lleg.kn = ani->FindBone("Golen_left_joint"  ,true);
	lleg.fo = ani->FindBone("Foolt_left_joint"  ,true);
	lleg.to = ani->FindBone("Foolt_left_joint_2",true);

	rleg.th = ani->FindBone("Bedro_right_joint"  ,true);
	rleg.kn = ani->FindBone("Golen_right_joint"  ,true);
	rleg.fo = ani->FindBone( "Foot_right_joint"  ,true);
	rleg.to = ani->FindBone( "Foot_right_joint_2",true);

	if( lleg.th >= 0 && lleg.kn >= 0 && lleg.fo >= 0 && lleg.to >= 0 &&
		rleg.th >= 0 && rleg.kn >= 0 && lleg.fo >= 0 && lleg.to >= 0 )
	{
		RegBone(lleg.th,0);
		RegBone(lleg.kn,1);
		RegBone(lleg.fo,2);
		RegBone(lleg.to,3);

		RegBone(rleg.th,0);
		RegBone(rleg.kn,1);
		RegBone(rleg.fo,2);
		RegBone(rleg.to,3);

		lleg.thn = lleg.tho = ani->GetBoneMatrix(lleg.th);
		lleg.knn = lleg.kno = ani->GetBoneMatrix(lleg.kn);
		lleg.fon = lleg.foo = ani->GetBoneMatrix(lleg.fo);

		rleg.thn = rleg.tho = ani->GetBoneMatrix(rleg.th);
		rleg.knn = rleg.kno = ani->GetBoneMatrix(rleg.kn);
		rleg.fon = rleg.foo = ani->GetBoneMatrix(rleg.fo);

		lleg.found = false;
		lleg.k = 0.0f;
		lleg.pf = lleg.foo.pos;

		rleg.found = false;
		rleg.k = 0.0f;
		lleg.pf = lleg.foo.pos;

		RegBone(0,-1);

		rdy = 0.0f;
		pdy = 0.0f;

		cdy = 0.0f;

		lfd = 0.0f; lfc = 0.0f;
		rfd = 0.0f; rfc = 0.0f;

		blend = -1.0f;
	}
}

void CharacterLegs::ResetAnimation(IAnimation *animation)
{
	if( chr == null )
		return;

	ani = animation;

	if( ani )
		Invalidate();
}

float CharacterLegs::GetBoneBlend(long boneIndex)
{
//	if( GetAsyncKeyState('Z') < 0 )
//		return 0.0f;
	if( enabled == false )
		return 0.0f;

	if( chr )
	{
		if( chr->logic->IsActor() || chr->logic->GetState() != CharacterLogic::state_idle )
			return 0.0f;
	}

	if( boneIndex == 0 )
		return 1.0f*blend;

	float k = 0.0f;

	if( boneIndex == lleg.th ||
		boneIndex == lleg.kn ||
		boneIndex == lleg.fo )
		k = lleg.k;
	else
		k = rleg.k;

	return k*0.8f*blend;
}

const float footh = 0.13f; // высота ступни (расстояние от кости до уровня пола)
const float toe_h = 0.03f; // высота носка

void CharacterLegs::GetBoneTransform(int i, BlendData &data)
{
	data.position = data.prevPosition;
	data.rotation = data.prevRotation;
	data.scale	  = data.prevScale;

	if( data.boneIndex == 0 )
	{
		Matrix m; chr->physics->GetModelMatrix(m);

		Vector p = m.MulVertex(data.prevPosition);
		Vector n(0.0f,-pdy,0.0f);

	//	chr->Render().DrawSphere(p	  ,0.010f,0xffff0000);
	//	chr->Render().DrawSphere(p + n,0.005f,0xff0000ff);

	//	data.position += n*0.8f;
		data.position += n;

		pdy = cdy;

		return;
	}

	Matrix a; data.prevRotation.GetMatrix(a);

	Matrix b = ani->GetBoneMatrix(ani->GetBoneParent(data.boneIndex));

	///////////////////////////////

	if( data.boneIndex == lleg.th )
	{
	//	data.position = b.MulVertexByInverse(lleg.thn.pos);

		Matrix v;

		Vector p = b.MulVertexByInverse(lleg.thn.pos);
		Vector q = b.MulVertexByInverse(lleg.knn.pos);

		v.BuildView(p,q,b.MulNormalByInverse(lleg.thn.vz));

		Matrix r;

		r.BuildRotateX(PI*0.5f);

		v *= r;

		v.Inverse();

		data.rotation.Set(v);

		a.pos = data.prevPosition;

		lleg.tho = a*b;
	
		return;
	}

	if( data.boneIndex == rleg.th )
	{
	//	data.position = b.MulVertexByInverse(rleg.thn.pos);

		Matrix v;

		Vector p = b.MulVertexByInverse(rleg.thn.pos);
		Vector q = b.MulVertexByInverse(rleg.knn.pos);

		v.BuildView(p,q,b.MulNormalByInverse(rleg.thn.vz));

		Matrix r;

		r.BuildRotateX(PI*0.5f);

		v *= r;

		v.Inverse();

		data.rotation.Set(v);

		a.pos = data.prevPosition;

		rleg.tho = a*b;
	
		return;
	}

	///////////////////////////////

	if( data.boneIndex == lleg.kn )
	{
		data.position = lleg.thn.MulVertexByInverse(lleg.knn.pos);

		Matrix v;

		Vector p = b.MulVertexByInverse(lleg.knn.pos);
		Vector q = b.MulVertexByInverse(lleg.fon.pos);

		v.BuildView(p,q,lleg.thn.MulNormalByInverse(lleg.knn.vz));

		Matrix r;

		r.BuildRotateX(PI*0.5f);

		v *= r;

		v.Inverse();

		data.rotation.Set(v);

		a.pos = data.prevPosition;

		lleg.kno = a*lleg.tho;
	
		return;
	}

	if( data.boneIndex == rleg.kn )
	{
		data.position = rleg.thn.MulVertexByInverse(rleg.knn.pos);

		Matrix v;

		Vector p = b.MulVertexByInverse(rleg.knn.pos);
		Vector q = b.MulVertexByInverse(rleg.fon.pos);

		v.BuildView(p,q,rleg.thn.MulNormalByInverse(rleg.knn.vz));

		Matrix r;

		r.BuildRotateX(PI*0.5f);

		v *= r;

		v.Inverse();

		data.rotation.Set(v);

		a.pos = data.prevPosition;

		rleg.kno = a*rleg.tho;
	
		return;
	}

	/////////////////////////////////

	Matrix m; chr->physics->GetModelMatrix(m);

//	chr->Render().DrawMatrix(b,0.2f);

//	Vector foot = b.MulVertex(data.prevPosition);
	Vector foot;

	if( data.boneIndex == lleg.fo )
		foot = lleg.fon.pos;
	else
	if( data.boneIndex == rleg.fo )
		foot = rleg.fon.pos;
	else
		foot = b.MulVertex(data.prevPosition);

	foot = m.MulVertex(foot);

//	chr->Render().DrawSphere(foot,0.01f,0xffff0000);

	IPhysicsScene::RaycastResult hd;

	IPhysBase *r = chr->Physics().Raycast(
		foot + Vector(0.0f, 0.5f,0.0f),
		foot + Vector(0.0f,-0.5f,0.0f),phys_mask(/*phys_world*/phys_bloodpatch),&hd);

	Vector pos;

	if( data.boneIndex == lleg.fo )
		pos = lleg.knn.MulVertexByInverse(lleg.fon.pos);
	else
	if( data.boneIndex == rleg.fo )
		pos = rleg.knn.MulVertexByInverse(rleg.fon.pos);
	else
		pos = data.prevPosition;

	if( r )
	{
		Vector n = b.MulNormalByInverse(m.MulNormalByInverse(hd.normal));

		if( data.boneIndex == lleg.to ||
			data.boneIndex == rleg.to )
		{
			Vector to = foot - hd.position;

			float len = to.Normalize();
			float sig = sign(to|n);

			to *= sig;

			len *= sig;

			if( len < toe_h )
			{
				if( data.boneIndex == lleg.to )
				{
					lfd = to*(toe_h - len);
				}
				if( data.boneIndex == rleg.to )
				{
					rfd = to*(toe_h - len);
				}
			}
		}

	//	chr->Render().DrawSphere(hd.position,0.01f);
	//	chr->Render().DrawVector(hd.position,hd.position + hd.normal*0.2,-1);

	//	chr->Render().DrawVector(foot,foot + m.MulNormal(b.MulNormal(a.vx))*0.2f,0xff00ff00);
	//	chr->Render().DrawVector(foot,foot + m.MulNormal(b.MulNormal(a.vy))*0.2f,0xffff0000);
	//	chr->Render().DrawVector(foot,foot + m.MulNormal(b.MulNormal(a.vz))*0.2f,0xff0000ff);

		float k;
		float h;

		if( data.boneIndex == lleg.fo ||
			data.boneIndex == rleg.fo )
		{
		//	h = 0.16f;
			h = footh;
		}
		else
		{
		//	h = 0.01f;
			h = 0.03f;
		}

	//	if( foot.y > hd.position.y/* + h*/ )
		if( foot.y > hd.position.y + h )
		{
		//	k = 1.0f - ((foot - hd.position).GetLength()/* - h*/)/0.25f/*0.16f*/;
			k = 1.0f - ((foot - hd.position).GetLength() - h)/footh;

			if( k > 1.0f )
				k = 1.0f;
			if( k < 0.0f )
				k = 0.0f;
		}
		else
		{
			k = 1.0f;
		}

		k *= 1.0f - fabsf(a.vx|n);

		float xk = (a.vz|n);

		if( xk > 0.0f )
			xk = 1.0f - xk;
		else
			xk = 1.0f;

		if((a.vy|n) < 0.1f )
			k = 0.0f;

		n.Lerp(a.vy,n,0.5f*k*xk);

		Matrix v;

		v.BuildView(pos,pos - n,a.vz);

		Matrix r;

		r.BuildRotateX(PI*0.5f);

		v *= r;

		v.Inverse();

	//	if( data.boneIndex == lleg.fo ||
	//		data.boneIndex == rleg.fo )
		{
			data.rotation.Set(v);
		}
	}

	if( data.boneIndex == lleg.fo )
	{
		a.pos = data.prevPosition;

		lleg.foo = a*lleg.kno;
	}

	if( data.boneIndex == rleg.fo )
	{
		a.pos = data.prevPosition;

		rleg.foo = a*rleg.kno;
	}

//	data.position = pos;
}

const float maxd = 0.5f; // максимальное расстояние, на которое можно сместить ногу

void CharacterLegs::Update(float dltTime)
{
	if( chr == null )
		return;

	#ifndef _XBOX

	if( !chr->EditMode_IsOn())
	if( !console || console->IsHided())
	{
		bool pressed = api->DebugKeyState('L');

		if( pressed && !key )
		{
			enabled = !enabled;

			if( console )
				console->Trace(COL_ALL,"Character legs IK is %s",enabled ? "ON" : "OFF");
		}

		key = pressed;
	}

	#endif

//	Matrix m; chr->physics->GetModelMatrix(m);

//	chr->Render().DrawSphere(m.pos,0.01f,0xff00ff00);
//	chr->Render().DrawMatrix(m,0.1f);

//	lfc.Lerp(lfc,lfd,0.5f);
//	rfc.Lerp(rfc,rfd,0.5f);

	lfc = 0.0f;//lfd;
	rfc = 0.0f;//rfd;

	rdy = 0.0f; moved = 0.0f; upd = 0.0f; UpdateLeg(dltTime,lleg); float lld = rdy; float lm = moved; float lup = upd;
	rdy = 0.0f; moved = 0.0f; upd = 0.0f; UpdateLeg(dltTime,rleg); float rld = rdy; float rm = moved; float rup = upd;

	if( lld > rld && lleg.fon.pos.y < rleg.fon.pos.y )
	{
		rdy = lld;

		moved = maxd - rm;

		if( moved < 0.0f )
			moved = 0.0f;

		if( rdy > moved )
			rdy = moved;
	}
	else
	if( rld > lld && rleg.fon.pos.y < lleg.fon.pos.y )
	{
		rdy = rld;

		moved = maxd - lm;

		if( moved < 0.0f )
			moved = 0.0f;

		if( rdy > moved )
			rdy = moved;
	}
	else
	{
		rdy = 0.0f;
	}

/*	if( lm > 0.0f && rm > 0.0f )
	{
		pdy -= coremin(lm,rm);
	}*/

	rdy -= coremax(lup,rup);

	float t = dltTime;

	if( t > 1.0f )
		t = 1.0f;

	cdy = Lerp(cdy,rdy,t);

//	float k = 20.0f;
	float k = 30.0f;

	if( lleg.found )
	{
		lleg.k += k*dltTime;

		if( lleg.k > 1.0f )
			lleg.k = 1.0f;
	}
	else
	{
		lleg.k -= k*dltTime;

		if( lleg.k < 0.0f )
			lleg.k = 0.0f;
	}

	if( rleg.found )
	{
		rleg.k += k*dltTime;

		if( rleg.k > 1.0f )
			rleg.k = 1.0f;
	}
	else
	{
		rleg.k -= k*dltTime;

		if( rleg.k < 0.0f )
			rleg.k = 0.0f;
	}

	if( blend < 0.0f )
	{
		chr->physics->GetModelMatrix(prev); blend = 0.0f;
	}
	else
	{
		Matrix m; chr->physics->GetModelMatrix(m);

		float d = (m.pos - prev.pos).GetLength();

		blend = 1.0f/(1.0f + d*25.f);

	//	if( enabled )
	//		chr->Render().Print(0.0f,0.0f,-1,"%f",blend);

		prev = m;
	}
}

void CharacterLegs::UpdateLeg(float dltTime, LegInfo &leg)
{
	if( leg.foo.pos.y > leg.pf.y && !leg.found && leg.k < 0.1f )
	{
		leg.found = false;
		leg.pf = leg.foo.pos;
		return;
	}

	leg.found = true;
	leg.pf = leg.foo.pos;

	Matrix m; chr->physics->GetModelMatrix(m);

	leg.thn = leg.tho*m;
	leg.knn = leg.kno*m;
	leg.fon = leg.foo*m;

//	Matrix org = ani->GetBoneMatrix(0)*m;

//	chr->Render().DrawSphere(leg.thn.pos,0.01f,0xffff0000);
//	chr->Render().DrawSphere(leg.knn.pos,0.01f,0xffff0000);
//	chr->Render().DrawSphere(leg.fon.pos,0.01f,0xffff0000);

	Vector foot = leg.fon.pos;

	IPhysicsScene::RaycastResult hd;

//	IPhysBase *res = chr->Physics().Raycast(
//		foot + Vector(0.0f, 0.5f,0.0f),
//		foot + Vector(0.0f,-0.5f,0.0f),phys_mask(/*phys_world*/phys_bloodpatch),&hd);

	Vector to = leg.fon.pos - leg.thn.pos; to.Normalize(); to *= 0.5f;

	IPhysBase *res = chr->Physics().Raycast(
		foot - to,
		foot + to,phys_mask(/*phys_world*/phys_bloodpatch),&hd);

	if( res )
	{
		float dy = foot.y - hd.position.y;

		if( rdy < dy - footh )
			rdy = dy - footh;

	/*	Vector up(0.0f,1.0f,0.0f);

		float ak = hd.normal|up;

		if( ak < 0.5f )
		{
			leg.found = false;
			return;
		}

		float dist = dy*ak;*/

		to = foot - hd.position;

		float dist = to.Normalize();

		float sig = sign(to|hd.normal);
		
		dist *= sig;

	//	chr->Render().DrawVector(hd.position,hd.position + hd.normal*dist,0xffffff00);
	//	chr->Render().DrawVector(foot,foot - hd.normal*dist,0xffff00ff);

	/*	if( foot.y < hd.position.y + 0.16f )
		{
			leg.fon.pos.y = hd.position.y + 0.16f;*/

/*	//	float footh = 0.16f;
	//	float footh = 0.13f;
		float footh = 0.11f;*/

		if( dist < footh )
		{
			float move = footh - dist;	// на сколько нужно сместить ступню

			if( leg.fo == lleg.fo )
			{
			//	lfc.Lerp(lfc,lfd,0.1f);
				move += lfc.y;
			}
			else
			{
			//	rfc.Lerp(rfc,rfd,0.1f);
				move += rfc.y;
			}

			upd = move;

			if( move > maxd )
				move = maxd;

			//////////
			to *= sig;
			//////////

			upd = upd - move;			// на сколько нужно сместить тело
			upd = (to*upd).y;

		//	leg.fon.pos += hd.normal*(move);
			leg.fon.pos += to*(move);

			moved = move;				// на сколько сместили ступню
			moved = (to*moved).y;

			Vector t = leg.thn.pos;
			Vector k = leg.knn.pos;
			Vector f = leg.fon.pos;

			float a = (k - t).GetLength();
			float b = (foot - k).GetLength();
			float c = (f - t).GetLength();

		//	float lk = a/(a + b);
			float lk = 0.5f*(1.0f + (a*a - b*b)/(c*c));

			Vector n = f - t; n *= lk;

			Vector h = t + n;

		//	chr->Render().DrawVector(t,t + n,0xff0000ff);

			Vector d = k - h;

		//	Vector z = n^d;
			Vector z = n^leg.knn.vz;

		//	chr->Render().DrawVector(h,k,0xffff00ff);

			Vector x = z^n;

			x.Normalize();

			float len = c*lk;

			len = a*a - len*len;

			if( len < 0.0f )
				len = 0.0f;

			x *= sqrt(len);

		/*	{
				Vector p = f - t;
			//	Vector d = h + x - t;
				Vector d = k - t;
				Vector q;

				IKSolver::solve(a,b,p.v,d.v,q.v);

				x = t + q - h;
			}*/

		//	chr->Render().DrawVector(h,h + x,0xffffff00);

			leg.knn.pos = h + x;
		}
		else
		{
			moved = 0.0f;
		}
	}

	Matrix r; r.BuildRotateX(PI*0.5f);

	Vector th = m.MulVertexByInverse(leg.thn.pos);
	Vector kn = m.MulVertexByInverse(leg.knn.pos);
	Vector fo = m.MulVertexByInverse(leg.fon.pos);

	leg.thn.BuildOriented(kn,th,m.MulNormalByInverse(leg.thn.vz));
	leg.thn.pos = th;
	leg.thn = r*leg.thn;

	leg.knn.BuildOriented(fo,kn,m.MulNormalByInverse(leg.knn.vz));
	leg.knn.pos = kn;
	leg.knn = r*leg.knn;

	leg.fon.pos = fo;

//	chr->Render().DrawMatrix(leg.thn*m,0.2f);
//	chr->Render().DrawMatrix(leg.knn*m,0.2f);

//	chr->Render().DrawMatrix(org,0.5f);

//	chr->Render().DrawVector(m.MulVertex(leg.thn.pos),m.MulVertex(leg.fon.pos),-1);

//	chr->Render().DrawSphere(m.MulVertex(leg.thn.pos),0.02f);
//	chr->Render().DrawSphere(m.MulVertex(leg.knn.pos),0.02f);
//	chr->Render().DrawSphere(m.MulVertex(leg.fon.pos),0.02f);

//	chr->Render().DrawVector(m.MulVertex(leg.thn.pos),m.MulVertex(leg.knn.pos),-1);
//	chr->Render().DrawVector(m.MulVertex(leg.knn.pos),m.MulVertex(leg.fon.pos),-1);
}

void CharacterLegs::setEnabled(bool en)
{
	enabled = en;

	if( console )
		console->Trace(COL_ALL,"Character legs IK is %s",enabled ? "ON" : "OFF");
}
