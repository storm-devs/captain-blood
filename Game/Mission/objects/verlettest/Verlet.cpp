#include "Verlet.h"


#define NUM_ITERATIONS 15

VerletSolver::VerletSolver() : particles(_FL_), constraints (_FL_)
{

}


VerletSolver::~VerletSolver()
{

}


void VerletSolver::Solve()
{
	float fTimeStep = api->GetDeltaTime();

	for(dword i = 0; i < particles.Size(); i++)
	{
		Particle & clothPoint = particles[i];

		if (clothPoint.bIsStatic) continue;

		Vector& x = clothPoint.vPos;
		Vector temp = x;
		Vector& oldx = clothPoint.vOldPos;
		Vector& a = clothPoint.vForce;
		x += x - oldx + a * fTimeStep * fTimeStep;

		oldx = temp;

	}
}

void VerletSolver::SatisfyConstraints()
{
	float scale1 = 0.5f;
	float scale2 = 0.5f;

	for(dword j = 0; j < NUM_ITERATIONS; j++)
	{
		for(dword i = 0; i < constraints.Size(); i++)
		{
				Constraint& c = constraints[i];

				Particle & clothPoint1 = particles[c.particleA];
				Particle & clothPoint2 = particles[c.particleB];

				if (clothPoint1.bIsStatic && clothPoint2.bIsStatic) continue;

				Vector& x1 = clothPoint1.vPos;
				Vector& x2 = clothPoint2.vPos;
				Vector delta = x2 - x1;


				float deltalength = ~delta;

				float fdelta = deltalength - c.restlength*c.restlength;

				float diff = 2.0f*fdelta;

				if(diff > c.restlength*0.3f) diff = c.restlength*0.3f;
				if(diff < -c.restlength*0.3f) diff = -c.restlength*0.3f;

				//float diff = (deltalength - c.restlength) / deltalength;

				if (clothPoint1.bIsStatic && !clothPoint2.bIsStatic)
				{
					x2 -= delta * 1.0 * diff;
				} else
				{
					if (!clothPoint1.bIsStatic && clothPoint2.bIsStatic)
					{
						x1 += delta * 1.0 * diff;
					} else
					{
						x1 += delta * 0.5 * diff;
						x2 -= delta * 0.5 * diff;
					}
				}

				
				
		}
	}


}

void VerletSolver::ApplyForces()
{
	for(dword i = 0; i < particles.Size(); i++)
	{
		//particles[i].vForce = Vector (0.0f, -9.8f, 0.0f);
		particles[i].vForce = Vector (0.0f, -0.098f, -0.04f);
	}
}


void VerletSolver::Work()
{
	Solve();
	SatisfyConstraints();
}

dword VerletSolver::AddParticle (const Vector& pos, bool bIsStatic)
{
	dword index = particles.Add();
	Particle* p = &particles[index];
	p->vForce = 0.0f;
	p->vOldPos = pos;
	p->vPos = pos;
	p->bIsStatic = bIsStatic;

	return index;
}

void VerletSolver::AddConstraint (dword from, dword to)
{
	Constraint* c = &constraints[constraints.Add()];
	c->particleA = from;
	c->particleB = to;
	c->restlength = (particles[from].vPos - particles[to].vPos).GetLength();
}


dword VerletSolver::GetParticlesCount ()
{
	return particles.Size();
}

const Vector& VerletSolver::GetParticlePos (dword index)
{
	return particles[index].vPos;
}

bool VerletSolver::IsParticlesStatic (dword index)
{
	return particles[index].bIsStatic;
}



dword VerletSolver::GetConstraintsCount ()
{
	return constraints.Size();
}

void VerletSolver::GetConstraintsData (dword index, dword& i1, dword& i2)
{
	i1 = constraints[index].particleA;
	i2 = constraints[index].particleB;
}



//Инициализировать объект
bool Verlet::EditMode_Create(MOPReader & reader)
{
	for(long i = 0; i < 16; i++)
	{
		for(long j = 0; j < 16; j++)
		{
			dword n = vs.AddParticle(Vector(-2.5f + 2.5f*j/15.0f, 5.0f - 5.0f*i/15.0f, 0.0f), i == 0);
			if(j > 0)
			{
				vs.AddConstraint(n, n - 1);
			}
			if(i > 0)
			{
				vs.AddConstraint(n, n - 16);
			}
		}
	}
	vs.ApplyForces();
	SetUpdate(&Verlet::Draw, ML_ALPHA3 + 1);
	return true;
}

//Нарисовать модельку
void _cdecl Verlet::Draw(float dltTime, long level)
{
	vs.Work();
	for(dword i = 0; i < vs.GetParticlesCount(); i++)
	{
		Render().DrawSphere(vs.GetParticlePos(i), 0.1f);
	}
	Render().FlushBufferedLines();
	for(dword i = 1; i < vs.GetConstraintsCount(); i++)
	{
		dword i1, i2;
		vs.GetConstraintsData(i, i1, i2);
		Render().DrawBufferedLine(vs.GetParticlePos(i1), 0xff0000ff, vs.GetParticlePos(i2), 0xff0000ff);
	}
}


//MOP_BEGINLIST(Verlet, "Verlet", '1.00', 0)
//MOP_ENDLIST(Verlet)

