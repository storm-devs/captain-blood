/*
#ifndef GMX_COLLISION_INFO_INTERFACE
#define GMX_COLLISION_INFO_INTERFACE

#include "..\math3d.h"

class IGMXScene;
class IGMXEntity;
class IGMXMesh;
class IGMXSubset;

class ICollisionInfo
{

public:

 ICollisionInfo() {};
 virtual ~ICollisionInfo() {};

 virtual const Triangle& GetTriangleInModelSpace () const = 0;
 virtual const Triangle& GetTriangleInColliderSpace () const = 0;
 
 virtual int GetTriangleIndex () const = 0;

 virtual IGMXScene* GetScene() const = 0;
 virtual IGMXEntity* GetEntity () const = 0;
 virtual IGMXMesh* GetMesh () const = 0;
 virtual IGMXSubset* GetSubset () const = 0;
};




#endif


*/