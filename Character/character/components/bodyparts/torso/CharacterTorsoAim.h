#pragma once

#include "..\Head\CharacterHeadAim.h"

class CharacterTorsoAim : public CharacterHeadAim
{
public:

	virtual void Init(const InitData &data);
	virtual void ResetAnimation(IAnimation *animation);
	virtual void SetRotation(float rotation);
};
