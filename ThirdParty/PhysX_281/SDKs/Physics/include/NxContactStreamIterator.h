#ifndef NX_PHYSICS_NXCONTACTSTREAMITERATOR
#define NX_PHYSICS_NXCONTACTSTREAMITERATOR
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/
/** \addtogroup physics
  @{
*/

#include "Nxp.h"

class NxShape;

typedef NxU32 * NxContactStream;
typedef const NxU32 * NxConstContactStream;


/**
\brief Flags which describe a contact

@see NxContactStreamIterator
*/
enum NxShapePairStreamFlags
	{
	NX_SF_HAS_MATS_PER_POINT		= (1<<0),	//!< not used
	NX_SF_HAS_FEATURES_PER_POINT	= (1<<2),	//!< the stream includes per-point feature data
	//note: bits 8-15 are reserved for internal use (static ccd pullback counter)
	};

/**
\brief NxContactStreamIterator is for iterating through packed contact streams.

<p>The user code to use this iterator looks like this:
\code
void MyUserContactInfo::onContactNotify(NxContactPair & pair, NxU32 events)
	{
	NxContactStreamIterator i(pair.stream);
	
	while(i.goNextPair()) // user can call getNumPairs() here 
		{
		while(i.goNextPatch()) // user can also call getShape() and getNumPatches() here
			{
			while(i.goNextPoint()) //user can also call getPatchNormal() and getNumPoints() here
				{
				//user can also call getPoint() and getSeparation() here
				}
			}
		}
	}
\endcode
</p>

\note It is NOT OK to skip any of the iteration calls. For example, you may NOT put a break or a continue
statement in any of the above blocks, short of completely aborting the iteration and deleting the 
NxContactStreamIterator object.

\note The user should not rely on the exact geometry or positioning of contact points. The SDK is free
to re-organise, merge or move contact points as long as the overall physical simulation is not affected.</p>

<h3>Visualizations:</h3>
\li #NX_VISUALIZE_CONTACT_POINT
\li #NX_VISUALIZE_CONTACT_NORMAL
\li #NX_VISUALIZE_CONTACT_ERROR
\li #NX_VISUALIZE_CONTACT_FORCE

<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

@see NxConstContactStream NxUserContactReport
*/

class NxContactStreamIterator
	{
	public:
	/**
	\brief Starts the iteration, and returns the number of pairs.

	\param[in] stream

	@see NxConstContactStream
	*/
	NX_INLINE NxContactStreamIterator(NxConstContactStream stream);

//iteration:


	/**
	\brief Goes on to the next pair, silently skipping invalid pairs.
	
	Returns false if there are no more pairs. Note that getNumPairs() also includes invalid pairs in the count.
	
	Once goNextPoint() returns false, the user should not call it again.

	\return True if there are more pairs.

	@see getNumPairs() getShape()
	*/
	NX_INLINE bool goNextPair();	

	/**
	\brief Goes on to the next patch (contacts with the same normal).
	
	Returns false if there are no more. Once goNextPatch() returns false, the user should
	not call it again until they move to the next pair.

	\return True if there are more patches.

	@see getPatchNormal()
	*/
	NX_INLINE bool goNextPatch();

	/**
	\brief Goes on to the next contact point.
	
	Returns false if there are no more. Once goNextPoint() returns false, the user should
	not call it again unil they move to the next patch.

	\return True if there are more contact points.

	@see getPoint()
	*/
	NX_INLINE bool goNextPoint();

//accessors:

	/**
	\brief Returns the number of pairs in the structure. 
	
	May be called at any time.
	
	\return The number of pairs in the struct (including invalid pairs).

	@see goNextPair()
	*/
	NX_INLINE NxU32 getNumPairs();

	/**
	\brief Retrieves the shapes for the current pair.
	
	May be called after goNextPair() returned true. ShapeIndex is 0 or 1.

	\param[in] shapeIndex Used to choose which of the pair of shapes to retrieve(set to 0 or 1).
	\return The shape specified by shapeIndex.

	@see goNextPair() NxShape
	*/
	NX_INLINE NxShape * getShape(NxU32 shapeIndex);

	/**
	\brief Retrieves the shape flags for the current pair.
	
	May be called after goNextPair() returned true

	\return The shape flags for the current pair. See #NxShapeFlag.

	@see NxShapeFlag NxShape goNextPair()
	*/
	NX_INLINE NxU16 getShapeFlags(); 

	/**
	\brief Retrieves the number of patches for the current pair.
	
	May be called after goNextPair() returned true

	\return The number of patches in this pair.

	@see goNextPatch()
	*/
	NX_INLINE NxU32 getNumPatches();

	/**
	\brief Retrieves the number of remaining patches.
	
	May be called after goNextPair() returned true

	\return The number of patches remaining in this pair.

	@see goNextPatch() getNumPatches()
	*/
	NX_INLINE NxU32 getNumPatchesRemaining();

	/**
	\brief Retrieves the patch normal.
	
	May be called after goNextPatch() returned true

	\return The patch normal.

	@see goNextPatch()
	*/
	NX_INLINE const NxVec3 & getPatchNormal();

	/**
	\brief Retrieves the number of points in the current patch.
	
	May be called after goNextPatch() returned true

	\return The number of points in the current patch.

	@see goNextPoint() getNumPointsRemaining()
	*/
	NX_INLINE NxU32 getNumPoints();

	/**
	\brief Retrieves the number of points remaining in the current patch.
	
	May be called after goNextPatch() returned true

	\return The number of points remaining in the current patch.

	@see goNextPoint() getNumPoints()
	*/
	NX_INLINE NxU32 getNumPointsRemaining();

	/**
	\brief Returns the contact point position.
	
	May be called after goNextPoint() returned true
	
	\return the current contact point

	@see getShapeFlags() goNextPoint() getNumPoints() getSeparation() getFeatureIndex0()
	*/
	NX_INLINE const NxVec3 & getPoint();
	
	/**
	\brief Return the separation for the contact point.

	May be called after goNextPoint() returned true
	
	\return the seperation distance for the current point.

	@see goNextPoint() getPoint()
	*/
	NX_INLINE NxReal getSeparation();

	/**
	\brief Retrieves the feature index.

	Feature indices are only defined for triangle mesh and heightfield shapes. 
	
	A feature index for a triangle mesh shape is the pre cooked triangle index. For a
	heightfield shape a feature index is a triangle index as specified on creation, including
	holes in the index.
	
	May be called after goNextPoint() returned true
	If getShapeFlags()&NX_SF_HAS_FEATURES_PER_POINT is specified, this method returns a feature belonging to shape 0,
	
	\return The feature index on shape 0 for the current point.

	@see NX_SF_FEATURE_INDICES goNextPoint() getPoint() getSeparation() getFeatureIndex1()
	*/
	NX_INLINE NxU32 getFeatureIndex0();

	/**
	\brief Retrieves the feature index.

	may be called after goNextPoint() returned true
	If getShapeFlags()&NX_SF_HAS_FEATURES_PER_POINT is specified, this method returns a feature belonging to shape 1,
	
	\return The feature index on shape1 for the current point.

	@see NX_SF_FEATURE_INDICES goNextPoint() getPoint() getSeparation() getFeatureIndex0()
	*/
	NX_INLINE NxU32 getFeatureIndex1();

	
	/**
	\brief Retrieves the point normal force.

	May be called after goNextPoint() returned true

	If getShapeFlags()&NX_SF_POINT_CONTACT_FORCE is true (this is the case if this flag is raised for either shape in the pair), 
	this method returns the contact force at this contact point.
	Returns 0 otherwise.

	\return The contact force for the current point.

	@see getShapeFlags goNextPoint() getPoint()
	*/
	NX_INLINE NxReal getPointNormalForce();

	private:
	//iterator variables -- are only initialized by stream iterator calls:
	//Note: structs are used so that we can leave the iterators vars on the stack as they were
	//and the user's iteration code doesn't have to change when we add members.

	//The number of pairs in the structure
	NxU32 numPairs;

	//The shapes for the current pair
	NxShape * shapes[2];
	//The shape flags for the current pair.
	NxU16 shapeFlags;
	//The number of patches for the current pair.
	NxU16 numPatches;
	
	//The patch normal.
	const NxVec3 * patchNormal;
	//The number of points in the current patch.
	NxU32  numPoints;

	//The contact point position.
	const NxVec3 * point;
	//The separation for the contact point.
	NxReal separation;
	//The feature index on shape 0.
	NxU32 featureIndex0;
	//The feature index on shape 1.
	NxU32 featureIndex1;

	//Number of pairs remaining in the stream
	NxU32 numPairsRemaining;
	//Number of contact patches remaining for the current pair
	NxU32 numPatchesRemaining;
	//Number of contact points remaining in the current patch
	NxU32 numPointsRemaining;

	protected:
	/**
	\brief Normal force for the current point

	Only exists if (shapeFlags & NX_SF_POINT_CONTACT_FORCE)
	*/
	const NxReal * pointNormalForce;

	/**
	\brief The associated stream
	*/
	NxConstContactStream stream;
	};

NX_INLINE NxContactStreamIterator::NxContactStreamIterator(NxConstContactStream s)
	{
	stream = s;
	numPairsRemaining = numPairs = stream ? *stream++ : 0;
	}

NX_INLINE NxU32 NxContactStreamIterator::getNumPairs()
	{
	return numPairs;
	}

NX_INLINE NxShape * NxContactStreamIterator::getShape(NxU32 shapeIndex)
	{
	NX_ASSERT(shapeIndex<=1);
	return shapes[shapeIndex];
	}

NX_INLINE NxU16 NxContactStreamIterator::getShapeFlags()
	{
	return shapeFlags;
	}

NX_INLINE NxU32 NxContactStreamIterator::getNumPatches()
	{
	return numPatches;
	}

NX_INLINE NxU32 NxContactStreamIterator::getNumPatchesRemaining()
	{
	return numPatchesRemaining;
	}

NX_INLINE const NxVec3 & NxContactStreamIterator::getPatchNormal()
	{
	return *patchNormal;
	}

NX_INLINE NxU32 NxContactStreamIterator::getNumPoints()
	{
	return numPoints;
	}

NX_INLINE NxU32 NxContactStreamIterator::getNumPointsRemaining()
	{
	return numPointsRemaining;
	}

NX_INLINE const NxVec3 & NxContactStreamIterator::getPoint()
	{
	return *point;
	}

NX_INLINE NxReal NxContactStreamIterator::getSeparation()
	{
	return separation;
	}

NX_INLINE NxU32 NxContactStreamIterator::getFeatureIndex0()
	{
	return featureIndex0;
	}
NX_INLINE NxU32 NxContactStreamIterator::getFeatureIndex1()
	{
	return featureIndex1;
	}

NX_INLINE NxReal NxContactStreamIterator::getPointNormalForce()
	{
	return pointNormalForce ? *pointNormalForce : 0;
	}

NX_INLINE bool NxContactStreamIterator::goNextPair()
	{
	while (numPairsRemaining--)
		{
#ifdef NX32
		size_t bin0 = *stream++;
		size_t bin1 = *stream++;
		shapes[0] = (NxShape*)bin0;
		shapes[1] = (NxShape*)bin1;
//		shapes[0] = (NxShape*)*stream++;
//		shapes[1] = (NxShape*)*stream++;
#else
		NxU64 low = (NxU64)*stream++;
		NxU64 high = (NxU64)*stream++;
		NxU64 bits = low|(high<<32);
		shapes[0] = (NxShape*)bits;
		low = (NxU64)*stream++;
		high = (NxU64)*stream++;
		bits = low|(high<<32);
		shapes[1] = (NxShape*)bits;
#endif
		NxU32 t = *stream++;
		numPatches = (NxU16) (t & 0xffff);
		numPatchesRemaining = (NxU32) numPatches;
		shapeFlags = (NxU16) (t >> 16);	
		return true;

		}
	return false;
	}

NX_INLINE bool NxContactStreamIterator::goNextPatch()
	{
	if (numPatchesRemaining--)
		{
		patchNormal = reinterpret_cast<const NxVec3 *>(stream);
		stream += 3;
		numPointsRemaining = numPoints = *stream++;
		return true;
		}
	else
		return false;
	}

NX_INLINE bool NxContactStreamIterator::goNextPoint()
	{
	if (numPointsRemaining--)
		{
		// Get contact point
		point = reinterpret_cast<const NxVec3 *>(stream);
		stream += 3;

		// Get separation
		NxU32 binary = *stream++;
		NxU32 is32bits = binary & NX_SIGN_BITMASK;
		binary |= NX_SIGN_BITMASK;	// PT: separation is always negative, but the sign bit is used
									// for other purposes in the stream.
		// To avoid strict-aliasing warnings on gcc, unions are used to read from the stream.
		// Note: You will still get a warning if gcc -Wstrict-aliasing=2 is used but this is a false positive.
		NxU32F32* sep = reinterpret_cast<NxU32F32*>(&binary);
		separation = sep->f;

		if (shapeFlags & NX_SF_POINT_CONTACT_FORCE)
			pointNormalForce = reinterpret_cast<const NxReal *>(stream++);
		else
			pointNormalForce = 0;	//there is no contact force.


		if (shapeFlags & NX_SF_HAS_FEATURES_PER_POINT)
			{
			if(is32bits)
				{
				featureIndex0 = *stream++;
				featureIndex1 = *stream++;
				}
			else
				{
				featureIndex0 = *stream++;
				featureIndex1 = featureIndex0>>16;
				featureIndex0 &= 0xffff;
				}
			}
		else
			{
			featureIndex0 = 0xffffffff;
			featureIndex1 = 0xffffffff;
			}

		//bind = *stream++;
		//materialIDs[0] = bind & 0xffff;
		//materialIDs[1] = bind >> 16;

		return true;
		}
	else
		return false;
	}

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
