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
__forceinline void SolvePhysic (Vector& Position, Vector& Velocity, const Vector& Forces, float UMass, float TimeScale)
{
/*
	Drag = 1.0f - Drag;
	if (Drag < 0.0f) Drag = 0.0f;
	if (Drag > 1.0f) Drag = 1.0f;
*/
	Vector Acceleration = Vector (0.0f);
	if (UMass>0.001f) Acceleration = (Forces / UMass);

	//Velocity.y -= 9.8f * TimeScale;

	Position += (Velocity + TimeScale*0.5f*Acceleration) * TimeScale;

	Velocity += Acceleration * TimeScale;
}

/// Добавить к текущим силам силу гравитации...
__forceinline void AddGravityForce (Vector& Forces, float fGrav, float Mass)
{
	Forces.y += fGrav*Mass;
}

#endif