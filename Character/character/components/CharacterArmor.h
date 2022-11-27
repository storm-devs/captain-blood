
#ifndef _CharacterArmor_h_
#define _CharacterArmor_h_


#include "..\Character.h"
#include "AnimationsBlender.h"
#include "CharacterLogic.h"

class CharacterArmor
{	
public:
	
	struct bbox
	{
		Vector bMin;
		Vector bMax;
	};

	dword bone_count;		
	bbox bboxes[32];

	struct item_data
	{
		bool used;

		bool visible;			
		int  item_index;		
	};
		
	array<item_data> data;
	
	CharacterArmor(Character * character);
	~CharacterArmor();	

	void Init(IGMXScene* model,IAnimationScene* animation);
	bool RegisterArmor(const char* bone, int index, Vector &bMin, Vector &bMax);
	void Draw(Matrix mtx);	

private:

	Character & chr;

	IGMXScene*			  armor_model;
	IAnimationProcedural* armor_anim;
};

#endif

