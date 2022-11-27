#pragma once

/*
Настройка студии:
Includes:
"C:\Program Files\Ageia Technologies\AGEIA PhysX SDK\SDKs\Foundation\include";"C:\Program Files\Ageia Technologies\AGEIA PhysX SDK\SDKs\Physics\include";"C:\Program Files\Ageia Technologies\AGEIA PhysX SDK\SDKs\NxCharacter\include";"C:\Program Files\Ageia Technologies\AGEIA PhysX SDK\SDKs\Cooking\include";"C:\Program Files\Ageia Technologies\AGEIA PhysX SDK\SDKs\PhysXLoader\include"
или "C:\Program Files (x86)\Ageia Technologies\AGEIA PhysX SDK\SDKs\Foundation\include";"C:\Program Files (x86)\Ageia Technologies\AGEIA PhysX SDK\SDKs\Physics\include";"C:\Program Files (x86)\Ageia Technologies\AGEIA PhysX SDK\SDKs\NxCharacter\include";"C:\Program Files (x86)\Ageia Technologies\AGEIA PhysX SDK\SDKs\Cooking\include";"C:\Program Files (x86)\Ageia Technologies\AGEIA PhysX SDK\SDKs\PhysXLoader\include"


Libs:
win32
"C:\Program Files\Ageia Technologies\AGEIA PhysX SDK\SDKs\lib\win32";
или "C:\Program Files (x86)\Ageia Technologies\AGEIA PhysX SDK\SDKs\lib\win32";
xbox360
"C:\Program Files\Ageia Technologies\AGEIA PhysX SDK\SDKs\lib\xbox360";
или "C:\Program Files (x86)\Ageia Technologies\AGEIA PhysX SDK\SDKs\lib\xbox360";



либо если номер версии есть, то:


Includes:
"C:\Program Files\AGEIA Technologies\SDK\v2.8.0\SDKs\Foundation\include";"C:\Program Files\AGEIA Technologies\SDK\v2.8.0\SDKs\Physics\include";"C:\Program Files\AGEIA Technologies\SDK\v2.8.0\SDKs\NxCharacter\include";"C:\Program Files\AGEIA Technologies\SDK\v2.8.0\SDKs\Cooking\include";"C:\Program Files\AGEIA Technologies\SDK\v2.8.0\SDKs\PhysXLoader\include"
или "C:\Program Files (x86)\AGEIA Technologies\SDK\v2.8.0\SDKs\Foundation\include";"C:\Program Files (x86)\AGEIA Technologies\SDK\v2.8.0\SDKs\Physics\include";"C:\Program Files (x86)\AGEIA Technologies\SDK\v2.8.0\SDKs\NxCharacter\include";"C:\Program Files (x86)\AGEIA Technologies\SDK\v2.8.0\SDKs\Cooking\include";"C:\Program Files (x86)\AGEIA Technologies\SDK\v2.8.0\SDKs\PhysXLoader\include"


Libs:
win32
"C:\Program Files\AGEIA Technologies\SDK\v2.8.0\SDKs\lib\Win32";
или "C:\Program Files (x86)\AGEIA Technologies\SDK\v2.8.0\SDKs\lib\Win32";
xbox360
"C:\Program Files\AGEIA Technologies\SDK\v2.8.0\SDKs\lib\xbox360";
или "C:\Program Files (x86)\AGEIA Technologies\SDK\v2.8.0\SDKs\lib\xbox360";

*/



#ifndef _Physics_includes_h_
#define _Physics_includes_h_

#include "core.h"

enum PhysForceMode
{
	pfm_force = 0,
	pfm_impulse,
	pfm_velocity_change,
	pfm_smooth_impulse,
	pfm_smooth_velocity_change,
	pfm_acceleration,

	pfm_forcedword = 0x7FFFFFFF
};

// группы для материалов
enum PhysMaterialGroup
{
	mtl_default = 0,	// дефолтовый материал
	mtl_ships,			// материал для кораблей
	mtl_last			//
};

//Материал, определяющий свойство треугольника
enum PhysTriangleMaterialID
{
	pmtlid_air = 0,
	pmtlid_ground = 1,
	pmtlid_stone = 2,
	pmtlid_sand = 3,
	pmtlid_wood = 4,
	pmtlid_grass = 5,
	pmtlid_water = 6,
	pmtlid_iron = 7,
	pmtlid_fabrics = 8,
	pmtlid_other1 = 9,
	pmtlid_other2 = 10,
	pmtlid_other3 = 11,
	pmtlid_other4 = 12,
};


//Группы для коллижена
enum PhysicsCollisionGroup
{
	phys_world = 1,				//Коллизия с текущем миром
	phys_ragdoll = 2,			//Группа для рэгдолов
	phys_character = 3,			//Группа с которой коллизется НПС	
	phys_player = 4,			//Группа с которой коллизется игрок	
	phys_particles = 5,			//Группа для партиклов
	phys_pair = 6,				//Группа для парных анимаций
	phys_ship = 7,				//Группа для кораблей
	phys_grp1 = 8,				//Злой шутер
	phys_grp2 = 9,				//использует
	phys_grp3 = 10,				//их
	phys_grp4 = 11,				//всех (phys_grpN)
	phys_nocollision = 12,		//Группа для отключения коллижена статических объектов
	phys_playerctrl = 13,		//
	phys_physobjects = 14,		//Группа в которой находятся игровые физические объекты
	phys_charitems = 15,		//Группа в которой находятся игровые физические объекты
	phys_bloodpatch = 16,		//Группа для кровянного патча
	phys_enemy = 17,			//Группа с которой коллизятся враги
	phys_ally = 18,				//Группа с которой коллизятся друзья
	phys_boss = 19,				//Группа с которой коллизятся боссы
	phys_grass = 20,			//Группа с которой рэйкастится трава
	phys_max
};

inline dword phys_mask(PhysicsCollisionGroup grp)
{
	Assert(grp < phys_max);
	return 1 << grp;
};

inline dword phys_mask(PhysicsCollisionGroup grp_1, PhysicsCollisionGroup grp_2)
{
	Assert(grp_1 < phys_max);
	Assert(grp_2 < phys_max);
	return (1 << grp_1) | (1 << grp_2);
}

inline dword phys_mask(PhysicsCollisionGroup grp_1, PhysicsCollisionGroup grp_2, PhysicsCollisionGroup grp_3)
{
	Assert(grp_1 < phys_max);
	Assert(grp_2 < phys_max);
	Assert(grp_3 < phys_max);
	return (1 << grp_1) | (1 << grp_2) | (1 << grp_3);
}

inline dword phys_mask(PhysicsCollisionGroup grp_1, PhysicsCollisionGroup grp_2, PhysicsCollisionGroup grp_3, PhysicsCollisionGroup grp_4)
{
	Assert(grp_1 < phys_max);
	Assert(grp_2 < phys_max);
	Assert(grp_3 < phys_max);
	Assert(grp_4 < phys_max);
	return (1 << grp_1) | (1 << grp_2) | (1 << grp_3) | (1 << grp_4);
}

typedef void * PhysRaycastId;

#include "math3D.h"
#include "Physics\IPhysBase.h"
#include "Physics\IPhysMaterial.h"
#include "Physics\IPhysCharacter.h"
#include "Physics\IPhysCloth.h"
#include "Physics\IPhysRagdoll.h"
#include "Physics\IPhysRigidbody.h"
#include "Physics\IPhysTriangleMesh.h"
#include "Physics\IPhysics.h"

#endif
