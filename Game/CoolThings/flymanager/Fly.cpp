#include "Fly.h"

//int Swarm::total = 0;
//int Cloud::total = 0;

const float fly_time  = 0.5f;
const float fly_vel	  = 1.2f;
const float fly_delay = 2.5f;

void Swarm::Init(float radius, dword count, bool playSound)
{
	if( flys.Size() != count || r != radius )
	{
		flys.DelAll();
		flys.AddElements(count);

		for( dword i = 0 ; i < count ; i++ )
		{
			Fly &fly = flys[i];

			fly.pos.Rand(0.0f,r);

			fly.beg.Rand(); fly.beg.y *= 0.2f;
			fly.end.Rand(); fly.end.y *= 0.2f;

			fly.time = 0.0f;

			fly.in = true;
		}
	}

	pos = 0.0f;

	r = radius;
	a = 1.0f;

	k = 1.0f;

	if( playSound )
	{
		if( sound )
		{
			sound->SetVolume(0.0f);
			sound->Play();
		}

		play = true;
	}
	else
	{
		play = false;
	}

	start = 0.0f;
	delay = 0.0f;
}

void Swarm::Update(float dltTime)
{
	if( delay > 0.0f )
	{
		delay -= dltTime;

		k = delay/fly_delay;

		if( sound && play )
			sound->SetVolume(k);

		if( delay < 0.0f )
		{
			busy = false;

			if( sound )
				sound->Stop();

		//	total--;
		//	Assert(total >= 0)

			return;
		}
	}
	else
	if( start < fly_delay )
	{
		start += dltTime;

		if( start > fly_delay )
			start = fly_delay;

		k = start/fly_delay;

		if( sound && play )
			sound->SetVolume(k);
	}
}

void Swarm::Move(float dltTime)
{
	const float r_fly_time = 1.0f/fly_time;

	float dt_vel = dltTime*fly_vel;
	float r_05 = r*0.5f;

	for( int i = 0 ; i < flys ; i++ )
	{
		Fly &fly = flys[i];

		Vector v;

		float k = fly.time*r_fly_time;

		if( k > 1.0f )
			k = 1.0f;

		v.Lerp(fly.beg,fly.end,k);

		fly.pos += v*dt_vel;

		if( !fly.pos.InSphere(0.0f,r) || fabsf(fly.pos.y) > r_05 )
		{
			if( fly.in || fly.time > fly_time )
			{
				fly.beg =  v;
				fly.end = -fly.pos;

				fly.time = 0.0f;

				fly.beg.y = fly.end.y;

				if( fly.pos.y > 0.0f )
					fly.end.y = RRnd(-0.5f,-0.2f);
				else
					fly.end.y = RRnd( 0.2f, 0.5f);

				fly.in = false;
			}
			else
				fly.time += dltTime;
		}
		else
		{
			fly.in = true;

			if( fly.time > fly_time )
			{
				fly.beg = fly.end;

				fly.end.Rand(); fly.end.y *= 0.2f;

				fly.time = 0.0f;
			}
			else
				fly.time += dltTime;
		}
	}
}

void Swarm::SetPosition(const Vector &pos)
{
	this->pos = pos;

	if( sound && play )
		sound->SetPosition(pos);
}

void Swarm::SetAlpha(float alpha)
{
	a = alpha;
}

void Swarm::Release(bool isFade)
{
//	if( isFade )
//	{
	//	delay = fly_delay;

		if( start < 0.1f )
			start = 0.1f;

		delay = start;
//	}
//	else
//		busy = false;
}

void Cloud::SetPosition(const Vector &pos)
{
	this->pos = pos;

	if( swarm )
		swarm->SetPosition(pos);
}

void Cloud::SetAlpha(float alpha)
{
	a = alpha;

	if( swarm )
		swarm->SetAlpha(alpha);
}

void Cloud::Release(bool isFade)
{
	if( swarm )
		swarm->Release(isFade);

	swarm = null;

	busy = false;

//	total--;
//	Assert(total >= 0)
}
