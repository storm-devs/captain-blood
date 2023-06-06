#pragma once

class physx::NxActor;
class physx::PxCloth;
class physx::PxJoint;
struct PhysInternal
{
	PhysInternal() : actor(NULL), cloth(NULL), joint(NULL) {}
	physx::NxActor* actor;
	physx::PxCloth* cloth;
	physx::PxJoint* joint;
};