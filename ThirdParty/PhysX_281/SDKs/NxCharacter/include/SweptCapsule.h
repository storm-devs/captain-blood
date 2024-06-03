#ifndef NX_CHARACTER_SWEPTCAPSULE
#define NX_CHARACTER_SWEPTCAPSULE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "SweptVolume.h"
#include "NxCapsule.h"

	class SweptCapsule : public SweptVolume
	{
		public:
										SweptCapsule();
		virtual							~SweptCapsule();

		virtual		void				ComputeTemporalBox(const SweepTest&, NxExtendedBounds3& box, const NxExtendedVec3& center, const NxVec3& direction)	const;

		NX_INLINE	void				GetCapsule(const NxExtendedVec3& center, NxExtendedCapsule& capsule, NxU32 axis) const
										{
											capsule.radius		= mRadius;
											capsule.p0			= center;
											capsule.p1			= center;
											capsule.p0[axis]	+= mHeight*0.5f;
											capsule.p1[axis]	-= mHeight*0.5f;
										}

		NX_INLINE	void				GetLocalCapsule(NxCapsule& capsule, NxU32 axis) const
										{
											capsule.radius		= mRadius;
											capsule.p0.zero();
											capsule.p1.zero();
											capsule.p0[axis]	+= mHeight*0.5f;
											capsule.p1[axis]	-= mHeight*0.5f;
										}

					NxF32				mRadius;
					NxF32				mHeight;
	};

#endif

