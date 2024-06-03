/*----------------------------------------------------------------------------*\
|
|								NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "SweptCapsule.h"
#include "CharacterController.h"

SweptCapsule::SweptCapsule()
{
	mType = SWEPT_CAPSULE;
}

SweptCapsule::~SweptCapsule()
{
}

void SweptCapsule::ComputeTemporalBox(const SweepTest& test, NxExtendedBounds3& box, const NxExtendedVec3& center, const NxVec3& direction) const
{
	NxVec3 mExtents(mRadius, mRadius, mRadius);
	mExtents[test.mUpDirection] += mHeight*0.5f;
//mExtents *= 2.0f;

	const NxVec3 SkinExtents = mExtents + NxVec3(test.mSkinWidth, test.mSkinWidth, test.mSkinWidth);

	NxExtendedVec3 tmp = center;
	tmp += direction;

	NxExtendedBounds3 DestBox;
	DestBox.setCenterExtents(tmp, SkinExtents);

	box.setCenterExtents(center, SkinExtents);
	box.add(DestBox);
}
