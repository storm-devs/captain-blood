/*----------------------------------------------------------------------------*\
|
|								NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "SweptBox.h"
#include "CharacterController.h"

SweptBox::SweptBox()
{
	mType = SWEPT_BOX;
}

SweptBox::~SweptBox()
{
}

void SweptBox::ComputeTemporalBox(const SweepTest& test, NxExtendedBounds3& box, const NxExtendedVec3& center, const NxVec3& direction) const
{
	const NxVec3 SkinExtents = mExtents + NxVec3(test.mSkinWidth, test.mSkinWidth, test.mSkinWidth);

	NxExtendedVec3 tmp = center;
	tmp += direction;

	NxExtendedBounds3 DestBox;
	DestBox.setCenterExtents(tmp, SkinExtents);

	box.setCenterExtents(center, SkinExtents);
	box.add(DestBox);
}
