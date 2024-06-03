/*----------------------------------------------------------------------------*\
|
|								NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include <new>
#include "CharacterControllerManager.h"
#include "NxController.h"
#include "BoxController.h"
#include "CapsuleController.h"

	// "Next Largest Power of 2
	// Given a binary integer value x, the next largest power of 2 can be computed by a SWAR algorithm
	// that recursively "folds" the upper bits into the lower bits. This process yields a bit vector with
	// the same most significant 1 as x, but all 1's below it. Adding 1 to that value yields the next
	// largest power of 2. For a 32-bit value:"
	NX_INLINE NxU32	NextPowerOfTwo(NxU32 x)
	{
		x |= (x >> 1);
		x |= (x >> 2);
		x |= (x >> 4);
		x |= (x >> 8);
		x |= (x >> 16);
		return x+1;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Public factory methods

NxUserAllocator* CCTAllocator::mAllocator = NULL;

NX_C_EXPORT NXCHARACTER_API NxControllerManager* NX_CALL_CONV NxCreateControllerManager(NxUserAllocator* allocator) 
	{
	NX_ASSERT(allocator);
	if (!allocator) return NULL;

	CCTAllocator::mAllocator = allocator;

	CharacterControllerManager* CM = (CharacterControllerManager*)allocator->malloc(sizeof(CharacterControllerManager), NX_MEMORY_PERSISTENT);
	new(CM)CharacterControllerManager(allocator);
	return CM;
	}

NX_C_EXPORT NXCHARACTER_API void NX_CALL_CONV NxReleaseControllerManager(NxControllerManager* manager) 
	{
	manager->release();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Implementation of userallocator array
class ControllerArray {
public:
	ControllerArray(NxUserAllocator* userAlloc) : allocator(userAlloc), count(0), capacity(0), data(NULL) {}
	~ControllerArray() { if (data) allocator->free(data); }
	NxU32 size() const { return count; }
	Controller** begin() const { return data; }
	void pushBack(Controller* controller) 
		{
		reserve();
		data[count] = controller;
		count++;
		}
	void replaceWithLast(NxU32 index) 
		{
		NX_ASSERT(index < count && count);
		data[index] = data[count-1];
		count--;
		}
	Controller* operator[](NxU32 index) const { return data[index]; }
protected:
	NxUserAllocator* allocator;
	Controller** data;
	NxU32 count;
	NxU32 capacity;
	void reserve() 
		{
		if (count+1 > capacity) 
			{
			NxU32 oldCapacity = capacity;
			capacity = NextPowerOfTwo(capacity+1);
			if (data)
				data = (Controller**)allocator->realloc(data, capacity*sizeof(Controller*));
			else
				data = (Controller**)allocator->malloc(capacity*sizeof(Controller*), NX_MEMORY_PERSISTENT);
			}
		}
	};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CharacterControllerManager::CharacterControllerManager(NxUserAllocator* userAlloc) : allocator(userAlloc)
	{
	controllers = (ControllerArray*)allocator->malloc(sizeof(ControllerArray), NX_MEMORY_PERSISTENT);
	new(controllers)ControllerArray(allocator);

	debugData = NULL;
	}

CharacterControllerManager::~CharacterControllerManager()
	{
	controllers->~ControllerArray();
	allocator->free(controllers);
	controllers = NULL;

	NX_DELETE_SINGLE(debugData);
	}

void CharacterControllerManager::release() 
	{
	while (getNbControllers()!= 0) releaseController(*getController(0));
	NxUserAllocator* a = allocator;
	this->~CharacterControllerManager();
	a->free(this);
	}

NxDebugRenderable CharacterControllerManager::getDebugData()
	{
	if(!debugData)
		debugData = new CCTDebugData;	// ###

	return NxDebugRenderable(	debugData->getNbPoints(), debugData->getPoints(),
								debugData->getNbLines(), debugData->getLines(),
								debugData->getNbTriangles(), debugData->getTriangles());
	}

void CharacterControllerManager::resetDebugData()
	{
	if(debugData)
		debugData->clear();	// Preserves the pointers, so it's fine
	}

NxU32 CharacterControllerManager::getNbControllers() const { return controllers->size(); }

Controller** CharacterControllerManager::getControllers() 
	{
	return controllers->begin();
	}

NxController*	CharacterControllerManager::getController(NxU32 index) 
	{
	NX_ASSERT(index < controllers->size());
	NX_ASSERT((*controllers)[index]);
	return (*controllers)[index]->getNxController();
	}

NxController* CharacterControllerManager::createController(NxScene* scene, const NxControllerDesc& desc)
	{
	NX_ASSERT(desc.isValid());
	NX_ASSERT(allocator);
	if (!desc.isValid()) return NULL;
	if (!allocator) return NULL;
	Controller* newController = NULL;

	NxController* N = NULL;
	if(desc.getType()==NX_CONTROLLER_BOX)
		{
		BoxController* BC = (BoxController*)allocator->malloc(sizeof(BoxController), NX_MEMORY_PERSISTENT);
		new(BC)BoxController(desc, scene);
		newController = BC;
		N = BC;
		}
	else if(desc.getType()==NX_CONTROLLER_CAPSULE)
		{
		CapsuleController* CC = (CapsuleController*)allocator->malloc(sizeof(CapsuleController), NX_MEMORY_PERSISTENT);
		new(CC)CapsuleController(desc, scene);
		newController = CC;
		N = CC;
		}
	else NX_ASSERT(0);

	if(newController)
		{
		controllers->pushBack(newController);
		newController->manager = this;
		}

	return N;
	}

void CharacterControllerManager::releaseController(NxController& controller)
	{
	for (NxU32 i = 0; i<controllers->size(); i++)
		if ((*controllers)[i]->getNxController() == &controller)
			{
			controllers->replaceWithLast(i);
			break;
			}
	if (controller.getType() == NX_CONTROLLER_CAPSULE) 
		{
		CapsuleController* cc = (CapsuleController*)&controller;
		cc->~CapsuleController();
		allocator->free(cc);
		} 
	else if (controller.getType() == NX_CONTROLLER_BOX) 
		{
		BoxController* bc = (BoxController*)&controller;
		bc->~BoxController();
		allocator->free(bc);
		} 
	else NX_ASSERT(0);
	}

void CharacterControllerManager::purgeControllers()
	{
	while(controllers->size())
		releaseController(*(*controllers)[0]->getNxController());
	}

void CharacterControllerManager::updateControllers()
	{
	for (NxU32 i = 0; i<controllers->size(); i++)
		{
		(*controllers)[i]->exposedPosition = (*controllers)[i]->filteredPosition;
		}
//	printStats();
	}
