#ifndef NX_CHARACTER_SWEPTBOX
#define NX_CHARACTER_SWEPTBOX
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "SweptVolume.h"

	class SweptBox : public SweptVolume
	{
		public:
										SweptBox();
		virtual							~SweptBox();

		virtual		void				ComputeTemporalBox(const SweepTest&, NxExtendedBounds3& box, const NxExtendedVec3& center, const NxVec3& direction)	const;

					NxVec3				mExtents;
	};

#endif

