#pragma once
#include "../../Common_h/Render.h"

// контроллер движения партиклов буруна
class ParticleController
{
	Vector vs_, ve_, vel_;			// начало конец траектории, начальная скорость 
	float height_;					// высота подъема
	float curTime_;					// текущее время
	float endTime_;					// полное время жизни
	float parabolTrackCoef_;		// соотношение времени полета по параболе и плавания на воде

public:

	ParticleController() : height_(0.0f), curTime_(0.0f), endTime_(0.0f) {}
	ParticleController(	const Vector& vs, const Vector& ve, const Vector& vel,
						float height, float endTime, float parabolTrackCoef	) :
		vs_(vs),
		ve_(ve),
		vel_(vel),
		height_(height),
		endTime_(endTime),
		curTime_(endTime*rand()/RAND_MAX),
		parabolTrackCoef_(parabolTrackCoef) {}

		void SetParams(	const Vector& vs, const Vector& ve, const Vector& vel,
						float height, float endTime, float parabolTrackCoef )
		{
			vs_ = vs;
			ve_ = ve;
			vel_ = vel;
			height_ = height;
			curTime_ = 0.0f;
			endTime_ = endTime;
			parabolTrackCoef_ = parabolTrackCoef;
		}

		Vector GetPosition(float t) const
		{
			Vector v;

			if (t < parabolTrackCoef_) // параболическое движение
			{
				t = t/parabolTrackCoef_;

				v = vs_ + (ve_-vs_)*t + vel_*curTime_;

				t = 2.0f*t - 1.0f;
				v.y = (1.0f - t*t)*height_;
			}
			else
			{
				t = t-parabolTrackCoef_;
//				float s = sin(v.GetAngle(vel_)) > 0.0f ? 1.0f : -1.0f;
				v = ve_ + vel_*endTime_*parabolTrackCoef_;
			}


			return v;
		}

		Vector GetCurrentPosition() const
		{
			return GetPosition(curTime_/endTime_);
		}

		float GetRelTiming() { return curTime_/endTime_; }
		void Step(float deltaTime) { curTime_ += deltaTime;	}
		bool IsFinished() const { return curTime_ >= endTime_; }
};

class BowWaveEmitter;
class BowParticles
{
	ShaderId BowParticles_id;

	// вершина первого стрима (неизменяемые данные)
	struct PGeometry
	{
		float	corner;		// номер угла квада партикла
		float	size;		// размер партикла
	};

	// вершина второго стрима (изменяющиеся во времени параметры партикла)
	struct PTransform
	{
		Vector	vCenter;	// центр партикла
		float	angle;		// угол поворота партикла
		float	ttl;		// время жизни (альфа) партикла
	};

	// данные для моделирования движения партикла
	struct PData
	{
		PData() : isActive(false), ttl(0.0f) {}
		ParticleController	solver;				// контроллер траектории движения
		bool				isActive;			// флаг активности
		float				angle;				// угол поворота
		float				rotateSpeed;		// скорость вращения
		float				ttl;				// время жизни (альфа)
	};

	BowWaveEmitter&				main_;

	IBaseTexture	*bowParticlesTexture_;		//	текстура на партиклах
	IVariable		*varPartTexture_;			//	--//--
	IVariable		*varMInvView_;				//	матрица билбординга
	IVariable		*varMWorldViewProj_;		//	мир*труд*май
	

	array<PData>	partData_;					//	частицы

	IVBuffer		*particlesVB_;				//	вертекс буфер для стрим0 партиклов
	IVBuffer		*particlesTVB_;				//	вертекс буфер для стрим1 партиклов
	IIBuffer		*particlesIB_;				//	индекс буфер для партиклов

	void CreateBuffers();
	void EmitParticles(float deltaTime);
	void UpdatePTransforms(float deltaTime);
	void RenderPSystem();

public:
	
	void Init();
	void Work(float deltaTime);

	BowParticles(BowWaveEmitter& main);
	~BowParticles(void);
};

