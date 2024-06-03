#ifndef NX_CHARACTER_SWEPTVOLUME
#define NX_CHARACTER_SWEPTVOLUME
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

	enum SweptVolumeType
	{
		SWEPT_BOX,
		SWEPT_CAPSULE,

		SWEPT_LAST
	};

	class SweepTest;
	#include "NxExtended.h"

	class SweptVolume
	{
		public:
										SweptVolume();
		virtual							~SweptVolume();

		virtual		void				ComputeTemporalBox(const SweepTest& test, NxExtendedBounds3& box, const NxExtendedVec3& center, const NxVec3& direction)	const	= 0;

		NX_INLINE	SweptVolumeType		GetType()	const	{ return mType;	}

					NxExtendedVec3		mCenter;
					NxF32				mHalfHeight;	// UBI
		protected:
					SweptVolumeType		mType;
	};

#endif

