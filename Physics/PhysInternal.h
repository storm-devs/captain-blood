#pragma once

class NxActor;
class NxCloth;
class NxJoint;
struct PhysInternal
{
	PhysInternal() : actor(NULL), cloth(NULL), joint(NULL) {}
	NxActor* actor;
	NxCloth* cloth;
	NxJoint* joint;
};