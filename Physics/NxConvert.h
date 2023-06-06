#pragma once

using namespace physx;

__forceinline Vector Nx(const PxVec3 & v)
{
	return Vector((float)v.x, (float)v.y, (float)v.z);
}

__forceinline Vector Nx(const PxVec4& v)
{
	return Vector((float)v.x, (float)v.y, (float)v.z);
}

__forceinline PxVec3 Nx(const Vector & v)
{
	return PxVec3((PxReal)v.x, (PxReal)v.y, (PxReal)v.z);
}

__forceinline PxVec4 Nx(const Vector& v, float w)
{
	return PxVec4((PxReal)v.x, (PxReal)v.y, (PxReal)v.z, (PxReal)w);
}

__forceinline PxExtendedVec3 Nxe(const Vector & v)
{
	return PxExtendedVec3((PxReal)v.x, (PxReal)v.y, (PxReal)v.z);
}

__forceinline Vector Nxe(const PxExtendedVec3 & v)
{
	return Vector((float)v.x, (float)v.y, (float)v.z);
}





__forceinline Plane Nx(const PxPlane & v)
{
	Plane p;
	p.n = Nx(v.n);
	p.d = (float)v.d;
	return p;
}

__forceinline PxPlane Nx(const Plane & v)
{
	PxPlane p;
	p.n = Nx(v.n);
	p.d = (PxReal)v.d;
	return p;
}

__forceinline Matrix & Nx(Matrix & m, const PxMat33 & from)
{
	m.vx = Nx(from.column0);
	m.wx = 0.0f;
	m.vy = Nx(from.column1);
	m.wy = 0.0f;
	m.vz = Nx(from.column2);
	m.wz = 0.0f;
	m.pos = 0.0f;
	m.w = 1.0f;
	return m;
}

__forceinline PxMat33 & Nx(PxMat33 & m, const Matrix & from)
{
	m.column0 = Nx(from.vx);
	m.column1 = Nx(from.vy);
	m.column2 = Nx(from.vz);
	return m;
}

__forceinline Matrix & Nx(Matrix & m, const PxMat44 & from)
{
	m.vx = Nx(from.column0);
	m.wx = from.column0.w;
	m.vy = Nx(from.column1);
	m.wy = from.column1.w;
	m.vz = Nx(from.column2);
	m.wz = from.column2.w;
	m.pos = Nx(from.column3);
	m.w = from.column3.w;
	return m;
}

__forceinline PxMat44 & Nx(PxMat44 & m, const Matrix & from)
{
	m.column0 = Nx(from.vx, from.wx);
	m.column1 = Nx(from.vy, from.wy);
	m.column2 = Nx(from.vz, from.wz);
	m.column3 = Nx(from.pos, from.w);
	return m;
}

// FIX_PX3 rm NxMat34
/*
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
*/

__forceinline PxForceMode::Enum Nx(PhysForceMode forceMode)
{
	switch (forceMode)
	{
		case pfm_force :					return PxForceMode::eFORCE;
		case pfm_impulse :					return PxForceMode::eIMPULSE;
		case pfm_velocity_change :			return PxForceMode::eVELOCITY_CHANGE;
		// FIX_PX3 PxForceMode
		//case pfm_smooth_impulse :			return NX_SMOOTH_IMPULSE;
		//case pfm_smooth_velocity_change :	return NX_SMOOTH_VELOCITY_CHANGE;
		case pfm_acceleration :				return PxForceMode::eACCELERATION;
	}
	Assert(false);
	return PxForceMode::eFORCE;
}