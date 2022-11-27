#pragma once


__forceinline Vector Nx(const NxVec3 & v)
{
	return Vector((float)v.x, (float)v.y, (float)v.z);
}

__forceinline NxVec3 Nx(const Vector & v)
{
	return NxVec3((NxReal)v.x, (NxReal)v.y, (NxReal)v.z);
}

__forceinline NxExtendedVec3 Nxe(const Vector & v)
{
	return NxExtendedVec3((NxReal)v.x, (NxReal)v.y, (NxReal)v.z);
}

__forceinline Vector Nxe(const NxExtendedVec3 & v)
{
	return Vector((float)v.x, (float)v.y, (float)v.z);
}





__forceinline Plane Nx(const NxPlane & v)
{
	Plane p;
	p.n = Nx(v.normal);
	p.d = (float)v.d;
	return p;
}

__forceinline NxPlane Nx(const Plane & v)
{
	NxPlane p;
	p.normal = Nx(v.n);
	p.d = (NxReal)v.d;
	return p;
}

__forceinline Matrix & Nx(Matrix & m, const NxMat33 & from)
{
	m.vx = Nx(from.getColumn(0));
	m.wx = 0.0f;
	m.vy = Nx(from.getColumn(1));
	m.wy = 0.0f;
	m.vz = Nx(from.getColumn(2));
	m.wz = 0.0f;
	m.pos = 0.0f;
	m.w = 1.0f;
	return m;
}

__forceinline NxMat33 & Nx(NxMat33 & m, const Matrix & from)
{
	m.setColumn(0, Nx(from.vx));
	m.setColumn(1, Nx(from.vy));
	m.setColumn(2, Nx(from.vz));
	return m;
}

__forceinline Matrix & Nx(Matrix & m, const NxMat34 & from)
{
	m.vx = Nx(from.M.getColumn(0));
	m.wx = 0.0f;
	m.vy = Nx(from.M.getColumn(1));
	m.wy = 0.0f;
	m.vz = Nx(from.M.getColumn(2));
	m.wz = 0.0f;
	m.pos = Nx(from.t);	
	m.w = 1.0f;
	return m;
}

__forceinline NxMat34 & Nx(NxMat34 & m, const Matrix & from)
{
	m.M.setColumn(0, Nx(from.vx));
	m.M.setColumn(1, Nx(from.vy));
	m.M.setColumn(2, Nx(from.vz));
	m.t = Nx(from.pos);
	return m;
}

__forceinline NxForceMode Nx(PhysForceMode forceMode)
{
	switch (forceMode)
	{
		case pfm_force :					return NX_FORCE;
		case pfm_impulse :					return NX_IMPULSE;
		case pfm_velocity_change :			return NX_VELOCITY_CHANGE;
		case pfm_smooth_impulse :			return NX_SMOOTH_IMPULSE;
		case pfm_smooth_velocity_change :	return NX_SMOOTH_VELOCITY_CHANGE;
		case pfm_acceleration :				return NX_ACCELERATION;
	}
	Assert(false);
	return NX_FORCE;
}