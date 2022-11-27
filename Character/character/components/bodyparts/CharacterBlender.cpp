#include "CharacterBodyParts.h"
#include "..\..\Character.h"

CharacterBlender::CharacterBlender()
{
	parent = null;

	enabled = true;
	active	= true;
}

void CharacterBlender::RegBone(int boneIndex, int i)
{
	Assert(parent)

	parent->map[boneIndex].p = this;
	parent->map[boneIndex].i = i;
}

void CharacterBlender::ReleaseBones()
{
	Assert(parent)

	for( int i = 0 ; i < parent->map ; i++ )
	{
		if( parent->map[i].p == this )
			parent->map[i].p  = null;
	}
}
