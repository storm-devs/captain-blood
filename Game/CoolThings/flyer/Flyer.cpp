#include "Flyer.h"

const float flyer_time =  0.5f;
const float flyer_vel  = 50.0f;

Flyer:: Flyer() : modes(_FL_)
{
	minDelay = 0.0f;
	maxDelay = 0.0f;

	time  = 0.0f;
	delay = 0.0f;

	run = false;

	sound = null;
}

Flyer::~Flyer()
{
	if( sound )
		sound->Release();
}

void Flyer::Restart()
{
	Show(false);

	run = false; time = 0.0f;

	Show(m_show);
}

bool Flyer::Create(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

bool Flyer::EditMode_Update(MOPReader &reader)
{
	InitParams(reader);

	return true;
}

void Flyer::Activate(bool isActive)
{
	//
}

void Flyer::Show(bool isShow)
{
	MissionObject::Show(isShow);

	if( isShow )
		SetUpdate(&Flyer::Draw,ML_PARTICLES3);
	else
		DelUpdate(&Flyer::Draw);

	if( isShow )
		LogicDebug("Show");
	else
		LogicDebug("Hide");
}

void Flyer::BuildTrack()
{
	Matrix view = Render().GetView();

	Matrix inve = view;
		   inve.Inverse();

	Vector pos = view.GetCamPos() + inve.vz*1.0f + inve.vy*RRnd(-0.7f,0.7f);

	Vector v;

	v.RandXZ(); v.Normalize();

	const Plane *p = Render().GetFrustum();

	Plane pl(-inve.vz,pos + inve.vz*5.0f);

	float da = 100000.0f; Vector a = pos + v*da;
	float db = 100000.0f; Vector b = pos - v*db;

	Vector r; float d;

	for( int i = 0 ; i < 5 ; i++ )
	{
		if( p[i].Intersection(pos,a,r))
		{
			d = (r - pos).GetLength();

			if( da > d )
			{
				da = d; beg = r;
			}
		}
	}

	if( pl.Intersection(pos,a,r))
	{
		d = (r - pos).GetLength();

		if( da > d )
		{
			da = d; beg = r;
		}
	}

	for( int i = 0 ; i < 5 ; i++ )
	{
		if( p[i].Intersection(pos,b,r))
		{
			d = (r - pos).GetLength();

			if( db > d )
			{
				db = d; end = r;
			}
		}
	}

	if( pl.Intersection(pos,b,r))
	{
		d = (r - pos).GetLength();

		if( db > d )
		{
			db = d; end = r;
		}
	}

	if((beg - pos).GetLength() < 2.0f )
		beg += v*2.5f;

	if((end - pos).GetLength() < 2.0f )
		end -= v*2.5f;

	off = RRnd(0.0f,PI*2.0f);
	fre = RRnd(2.0f,4.0f);
	rad = 0.2f;

	time = 0.0f;

	run = true;

	d = (end - beg).GetLength();

	delay = 0.3f*d;

	if( !sound )
	{
		sound = Sound().Create3D("flyer",beg,_FL_,false,false);

	//	Assert(sound)
	}

	curSound = sound; curMode = -1;

	if( modes )
	{
		float f = RRnd(0,float(modes));

		curMode = int(f);

		if( modes[curMode].sound )
			curSound = modes[curMode].sound;
	}

//	curSound->SetVelocity(-v*d/delay);

	curSound->SetPosition(beg);
	curSound->Play();

	lastPos = beg;
}

void _cdecl Flyer::Draw(float dltTime, long level)
{
	if( !run )
	{
		if( time > delay )
		{
			BuildTrack();
		}
		else
			time += dltTime;

		return;
	}

	Vector pos;

	float t = time/delay;

	pos.Lerp(beg,end,t);

	Vector n = end - beg; n.Normalize();
	Vector v = n^Vector(0.0f,1.0f,0.0f);

	v.Normalize();

//	v *= rad*sinf(t*fre + off);
	v *= rad*sinf(time*fre + off);

	pos += v;

	if( EditMode_IsOn() && Mission().EditMode_IsAdditionalDraw())
	{
		Render().DrawLine(beg,0xffffffff,end,0xffffffff);
		Render().DrawLine(pos,0xffffffff,pos,0xffffffff);
	}

	Matrix view = Render().GetView();
	Vector camp = view.GetCamPos();

	float a = 1.0f;
	float d = (pos - camp).GetLength();

	if( d > 4.0f )
		a = 0.5f*(6.0f - d);

	if( curMode >= 0 && modes[curMode].model )
	{
		Matrix m;
			   m.BuildOriented(pos,pos + pos - lastPos,Vector(0.0f,1.0f,0.0f));

		Color c(0xff000000); c.a = a;

		modes[curMode].model->SetUserColor(c);

		modes[curMode].model->SetTransform(m);
		modes[curMode].model->Draw();

		modes[curMode].model->SetUserColor(0xff000000);
	}
	else
		Render().DrawSphere(pos,0.005f,0xff000000);

	curSound->SetPosition(pos);

	lastPos = pos;

	time += dltTime;

	if( time > delay )
	{
		run = false;

		delay = RRnd(minDelay,maxDelay);

		curSound->Stop();
	}
}

void Flyer::InitParams(MOPReader &reader)
{
	modes.DelAll();

	long n = reader.Array();

	for( long i = 0 ; i < n ; i++ )
	{
		const char *model = reader.String().c_str();
		ConstString sound = reader.String();

		Mode &mode = modes[modes.Add()];

		mode.model = Geometry().CreateScene(model,&Animation(),&Particles(),&Sound(),_FL_);
		mode.sound = Sound().Create3D(sound,0.0f,_FL_,false,false);
	}

	minDelay = reader.Float();
	maxDelay = reader.Float();

	Show(m_show = reader.Bool());
}

MOP_BEGINLISTCG(Flyer, "Flyer", '1.00', 100, "", "Default")

	MOP_ARRAYBEG("Modes", 0, 100)

		MOP_STRING("Model", "")
		MOP_STRING("Sound", "")

	MOP_ARRAYEND

	MOP_FLOAT("Min delay", 1.0f)
	MOP_FLOAT("Max delay", 5.0f)

	MOP_BOOL("Show", true)

MOP_ENDLIST(Flyer)
