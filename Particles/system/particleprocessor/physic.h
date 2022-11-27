#ifndef SIMPLE_PHYSIC_MATH
#define SIMPLE_PHYSIC_MATH



///
/// ================================================================
/// Poisition - Позиция в которой находиться партикл
/// Velocity - Текущая скорость
/// Forces - силы действующие на партикл
/// UMass - fabsf(Массы)
/// Drag - сопротивление среды (0 нет сопротивления .. 1 полная остановка)
/// TimeScale - скалирование времени...
__forceinline void SolvePhysic (Vector& Position, Vector& Velocity, const Vector& Forces, float Mass, float Drag, float TimeScale)
{
	float k = 0.0f;
	if (Drag > 0.0001f)
	{
		k = (TimeScale / Drag);
		if (k > 1.0f) k = 1.0f;
	}

	Vector Acceleration = (Forces / Mass);

	Velocity += Acceleration * TimeScale;

	Velocity -= (Velocity * k);

	Position += (Velocity * TimeScale);
}

/// Добавить к текущим силам силу гравитации...
__forceinline void AddGravityForce (Vector& Forces, float Mass, float GravK)
{
	//if (GravK < 0) GravK = 0.0f;
	//if (GravK > 1.0f) GravK = 1.0f;

	Forces.y += -0.098f * Mass * GravK;
}

#endif