#ifndef NX_PHYSICS_NXSLIDINGJOINT
#define NX_PHYSICS_NXSLIDINGJOINT
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

#include "NxJoint.h"
class NxCylindricalJointDesc;

/**
\brief Cylindrical Joints permit relative translational movement between two bodies along
 an axis, and also relative rotation along the axis.


 \image html cylinderJoint.png

 <h3>Creation</h3>

 Example:

 \include NxCylindricalJoint_Create.cpp

<h3>Visualizations:</h3>
\li #NX_VISUALIZE_JOINT_LOCAL_AXES
\li #NX_VISUALIZE_JOINT_WORLD_AXES
\li #NX_VISUALIZE_JOINT_LIMITS

 @see NxCylindricalJointDesc NxJoint NxScene.createJoint()
*/
class NxCylindricalJoint : public NxJoint
	{
	public:
	/**
	\brief Use this for changing a significant number of joint parameters at once.

	Use the set() methods for changing only a single property at once.

	Please note that you can not change the actor pointers using this function, if you do so the joint will be marked as broken and will stop working.
	
	Calling the loadFromDesc() method on a broken joint will result in an error message.

	<b>Sleeping:</b> This call wakes the actor if it is sleeping.

	\param[in] desc The descriptor used to set the state of the object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes

	@see saveToDesc() NxCylindricalJointDesc
	*/
	virtual void loadFromDesc(const NxCylindricalJointDesc& desc) = 0;

	/**
	\brief Writes all of the object's attributes to the desc struct

	\param[out] desc The descriptor used to retrieve the state of the object.

	<b>Platform:</b>
	\li PC SW: Yes
	\li PPU  : Yes
	\li PS3  : Yes
	\li XB360: Yes
	
	@see loadFromDesc() NxCylindricalJointDesc
	*/
	virtual void saveToDesc(NxCylindricalJointDesc& desc) = 0;
	};

/** @} */
#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
