#ifndef NX_PHYSICS_NXFORCEFIELDKERNELDEFS
#define NX_PHYSICS_NXFORCEFIELDKERNELDEFS

#include "Nxp.h"
#include "NxTargets.h"

#define NxBoolean			typename NxTarget::BVarType
#define NxFloat				typename NxTarget::FVarType
#define NxVector			typename NxTarget::VVarType
#define NxFailIf(_x)		if(NxTarget::testFailure(_x)) return false;
#define NxFinishIf(_x)		if(NxTarget::testFinish(_x))  return true;
#define NxSelect(_x,_y,_z)	typename NxTarget::BVarType(_x).select((_y),(_z))

#define NxFConst(name)															\
private:																		\
	typename NxTarget::FConstType name;											\
public:																			\
NxReal get##name() { return NxForceFieldInternals::NxSw::getFloatVal(name); }	\
void set##name(NxReal x) { mUpdateCounter++; name = x; }						\

#define NxVConst(name)															\
private:																		\
	typename NxTarget::VConstType name;											\
public:																			\
NxVec3 get##name() { return NxForceFieldInternals::NxSw::getVecVal(name); }		\
void set##name(const NxVec3&  x) { mUpdateCounter++; name = x; }				\

#define NxBConst(name)															\
private:																		\
	typename NxTarget::BConstType name;											\
public:																			\
NxVec3 get##name() { return NxForceFieldInternals::NxSw::getBoolVal(name); }	\
void set##name(bool x) {  mUpdateCounter++; name = x; }							\


#define NX_START_FORCEFIELD(name)												\
template<class NxTarget>														\
class NxForceFieldKernelTemplate##name											\
{																				\
	friend class NxForceFieldKernel##name;										\
protected:																		\
	NxU32 mUpdateCounter;														\

#define NX_START_FUNCTION														\
protected:																		\
bool eval(typename NxTarget::VVarType& force,									\
	      typename NxTarget::VVarType& torque,									\
		  const typename NxTarget::VConstType& Position,						\
		  const typename NxTarget::VConstType& Velocity) const					\
{																				\

#define NX_END_FUNCTION															\
	return true;																\
}																				\
	
#if NX_ENABLE_HW_PARSER 
	#define NX_END_FORCEFIELD(name)												\
	};																			\
																				\
	class NxForceFieldKernel##name :											\
	public NxForceFieldKernel,													\
		public NxForceFieldKernelTemplate##name<NxForceFieldInternals::NxSw>	\
	{																			\
	public:																		\
		void parse() const														\
		{																		\
			NxForceFieldInternals::NxHw::VVarType force, torque;				\
			const NxForceFieldInternals::NxHw::VConstType position, velocity;	\
			NxForceFieldKernelTemplate##name<NxForceFieldInternals::NxHw> hwField;\
			hwField.eval(force, torque, position, velocity);					\
		}																		\
																				\
		bool evaluate(	NxVec3& force,											\
						NxVec3& torque,											\
						const NxVec3& position,									\
						const NxVec3& velocity) const							\
		{																		\
			return eval((NxForceFieldInternals::NxSwVecVar&)force,(NxForceFieldInternals::NxSwVecVar&)torque,	\
						position, velocity);									\
		}																		\
																				\
		NxU32 getType() const { return NX_FFK_CUSTOM_KERNEL; }					\
		NxForceFieldKernel* clone()	const		{ return NULL; }				\
		void update(NxForceFieldKernel& in)	const {}							\
		void setEpsilon(NxReal eps) {}											\
																				\
		void* operator new(size_t size)											\
		{																		\
			return NxGetPhysicsSDKAllocator()->malloc(size);					\
		}																		\
																				\
		void  operator delete(void* p)											\
		{																		\
			NxGetPhysicsSDKAllocator()->free(p);								\
		}																		\
	};																			
#else
	#define NX_END_FORCEFIELD(name)												\
	};																			\
																				\
	class NxForceFieldKernel##name :											\
		public NxForceFieldKernel,												\
		public NxForceFieldKernelTemplate##name<NxForceFieldInternals::NxSw>	\
	{																			\
	public:																		\
		void parse() const {}													\
		bool evaluate(	NxVec3& force,											\
						NxVec3& torque,											\
						const NxVec3& position,									\
						const NxVec3& velocity)	const							\
		{																		\
			return eval((NxForceFieldInternals::NxSwVecVar&)force,(NxForceFieldInternals::NxSwVecVar&)torque,	\
						position, velocity);									\
		}																		\
																				\
		NxU32 getType() const { return NX_FFK_CUSTOM_KERNEL; }					\
																				\
		NxForceFieldKernel* clone()	const										\
		{																		\
			NxForceFieldKernel##name* clone; 									\
			clone = new NxForceFieldKernel##name();								\
			*clone = *this;														\
			return clone;														\
		}																		\
																				\
		void update(NxForceFieldKernel& in) const								\
		{																		\
			NxForceFieldKernel##name* dest; 									\
			dest = static_cast<NxForceFieldKernel##name*>(&in);					\
			if(dest->mUpdateCounter == mUpdateCounter) return;					\
			*dest = *this;														\
		}																		\
																				\
		void setEpsilon(NxReal eps) 											\
		{																		\
			NxForceFieldInternals::NxSwFloat::setEpsilon(eps);					\
		}																		\
																				\
		void* operator new(size_t size)											\
		{																		\
			return NxGetPhysicsSDKAllocator()->malloc(size);					\
		}																		\
																				\
		void  operator delete(void* p)											\
		{																		\
			NxGetPhysicsSDKAllocator()->free(p);								\
		}																		\
	};																	
#endif

#endif

//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2007 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
