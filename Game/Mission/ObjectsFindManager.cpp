//============================================================================================
// Spirenkov Maxim, 2004, 2008
//============================================================================================
// ObjectFindManager (quad tree)
//============================================================================================

#include "ObjectsFindManager.h"


#define ENABLE_NAN_CHECK
//#define ENABLE_NAN_STOP

//============================================================================================
//Abb
//============================================================================================


__forceinline ObjectsFindManager::Abb::Abb()
{
}

__forceinline ObjectsFindManager::Abb::Abb(const Abb & abb)
{
	minX = abb.minX; maxX = abb.maxX;
	minZ = abb.minZ; maxZ = abb.maxZ;
}

__forceinline ObjectsFindManager::Abb::Abb(long _minX, long _maxX, long _minZ, long _maxZ)
{
	minX = _minX; maxX = _maxX;
	minZ = _minZ; maxZ = _maxZ;
}

__forceinline bool ObjectsFindManager::Abb::IsInclude(const Abb & abb) const
{
	if(minX  <= abb.minX && maxX >= abb.maxX)
	{
		if(minZ <= abb.minZ && maxZ >= abb.maxZ)
		{
			return true;
		}
	}
	return false;
}

__forceinline bool ObjectsFindManager::Abb::IsIntersection(const Abb & abb) const
{
	if(minX <= abb.maxX && maxX >= abb.minX)
	{
		if(minZ <= abb.maxZ && maxZ >= abb.minZ)
		{
			return true;
		}
	}
	return false;
}

__forceinline long ObjectsFindManager::Abb::MinSize() const
{
	return coremin(maxX - minX, maxZ - minZ);
}


__forceinline bool ObjectsFindManager::Abb::Set(Vector min, Vector max, bool isChecker)
{
	//Границы
	const float roundValue = 0.999999f;
	if(isChecker)
	{
		if(min.x > max.x)
		{
			Swap(min.x, max.x);
		}
		if(min.z > max.z)
		{
			Swap(min.z, max.z);
		}
	}
	min.x -= roundValue;
	min.z -= roundValue;
	max.x += roundValue;
	max.z += roundValue;
	//Проверяем на попадание ректа в рабочий диапазон
	const float range = 1000000.0f;
	bool inRange = true;
	if(!isChecker)
	{
		if(min.x < -range || min.x > range) inRange = false;
		if(min.z < -range || min.z > range) inRange = false;
		if(max.x < -range || max.x > range) inRange = false;
		if(max.z < -range || max.z > range) inRange = false;
	}else{
		const float clampValue = range*1.1f;
		min.x = Clampf(min.x, -clampValue, clampValue);
		min.z = Clampf(min.z, -clampValue, clampValue);
		max.x = Clampf(max.x, -clampValue, clampValue);
		max.z = Clampf(max.z, -clampValue, clampValue);
	}
	//Преобразуем результат к целым числам
	if(inRange)
	{
		minX = long(min.x);
		maxX = long(max.x);
		minZ = long(min.z);
		maxZ = long(max.z);
/*
#ifndef STOP_ASSERTS
		if(AssertInRange("ObjectsFindManager::Abb::Set", true))
		{
			api->Trace("Inc abb: min = %f, %f; max = %f, %f", min.x, min.z, max.x, max.z);
		}
#endif		
*/
	}
	return inRange;
}

#ifndef STOP_ASSERTS
__forceinline bool ObjectsFindManager::Abb::AssertInRange(const char * prefix, bool noStop) const
{
	const long range = 2000000;
	bool outOfRange = false;
	if(minX < -range || minX > range)
	{
		outOfRange = true;
	}
	if(minZ < -range || minZ > range)
	{
		outOfRange = true;
	}
	if(maxX < -range || maxX > range)
	{
		outOfRange = true;
	}
	if(maxZ < -range || maxZ > range)
	{
		outOfRange = true;
	}
	if(outOfRange)
	{
		LogOut(prefix);
		Assert(noStop);
	}
	return outOfRange;
}
#endif

__forceinline void ObjectsFindManager::Abb::LogOut(const char * prefix) const
{
#ifndef STOP_DEBUG
	api->Trace("%s Abb: min(x: %i, z: %i) max(x: %i, z: %i)", prefix, minX, minZ, maxX, maxZ);
#endif
}



//============================================================================================
//TreeObject
//============================================================================================

ObjectsFindManager::TreeObject::TreeObject(MissionObject * _mo, ObjectsFindManager * _mng)
{
	//Заполняем поля
	Assert(_mo);
	Assert(_mng);
	flags = f_invalidateAbb;
	node = null;
	mng = _mng;
	nextInNode = null;
	prevInNode = null;
	nextInList = null;
	prevInList = null;
	abb.minX = 0;
	abb.maxX = 0;
	abb.minZ = 0;
	abb.maxZ = 0;
	center = 0.0f;
	size = 0.0f;
	mo = _mo;
	userData = null;
	cppFile = null;
	cppLine = 0;
	pullCode = 0;
	tmp = 0;
	//Добавляемся в список неактивных
	MoveToInactiveList();
}

//Освободить ресурсы
void ObjectsFindManager::TreeObject::Release()
{
	//Исключаемся из всех возможных списков
	Assert(mng);
	RemoveFromNode();
	RemoveFromLists();
	//Чистим нужное
	flags = f_deleted;
	node = null;
	mo = null;
	userData = null;
	//Удаляем объект
	Delete(this);
//	api->Trace("Release object = %x", this);
}

//Изменить активность
void ObjectsFindManager::TreeObject::Activate(bool isActive)
{
	if(isActive)
	{
		//Если активны то ничего не делаем
		if(flags & f_active)
		{
			return;
		}
		//Пометим флажёк что хотим быть активными
		flags |= f_active;
		//Если бокс кривой то больше ничего не делаем
		if(flags & f_invalidateAbb)
		{
			return;
		}
		//Переходим в список обновления
		MoveToUpdateList();
	}else{
		if(!(flags & f_active))
		{
			return;
		}
		//Переходим в список неактивных
		RemoveFromNode();
		MoveToInactiveList();
	}
}

//Активен ли объект
bool ObjectsFindManager::TreeObject::IsActivate()
{
	return (flags & f_active) != 0;
}

//Получить указатель на объект миссии
MissionObject & ObjectsFindManager::TreeObject::GetMissionObject()
{
	Assert(mo);
	return *mo;
}

//Установить пользовательские данные
void ObjectsFindManager::TreeObject::SetUserData(void * data)
{
	userData = data;
}

//Получить пользовательские данные
void * ObjectsFindManager::TreeObject::GetUserData()
{
	return userData;
}

//Установить матрицу
void ObjectsFindManager::TreeObject::SetMatrix(const Matrix & mtx)
{
#ifdef ENABLE_NAN_CHECK
#ifndef STOP_DEBUG
	if(_isnan(mtx.pos.x) || _isnan(mtx.pos.y) || _isnan(mtx.pos.z) ||
		_isnan(mtx.vx.x) || _isnan(mtx.vx.y) || _isnan(mtx.vx.z) ||
		_isnan(mtx.vy.x) || _isnan(mtx.vy.y) || _isnan(mtx.vy.z) ||
		_isnan(mtx.vz.x) || _isnan(mtx.vz.y) || _isnan(mtx.vz.z))
	{
		api->Trace("%x, matrix = pos(%f, %f, %f), 3x3 = (%f, %f, %f, %f, %f, %f, %f, %f, %f)", this, mtx.pos.x, mtx.pos.y, mtx.pos.z, mtx.vx.x, mtx.vx.y, mtx.vx.z, mtx.vy.x, mtx.vy.y, mtx.vy.z, mtx.vz.x, mtx.vz.y, mtx.vz.z);
#ifdef ENABLE_NAN_STOP
		Assert(false);
#endif
		return;
	}
#endif
#endif
	matrix = mtx;
	SetToUpdate();
}

//Получить матрицу
const Matrix & ObjectsFindManager::TreeObject::GetMatrix() const
{
	return matrix;
}

//Установить локальную позицию описывающего бокса
void ObjectsFindManager::TreeObject::SetBoxCenter(float centerX, float centerY, float centerZ)
{
#ifdef ENABLE_NAN_CHECK
#ifndef STOP_DEBUG
	if(_isnan(centerX) || _isnan(centerY) || _isnan(centerZ))
	{
		api->Trace("%x, ObjectsFindManager::TreeObject::SetBoxCenter(%f, %f, %f)", this, centerX, centerY, centerZ);
#ifdef ENABLE_NAN_STOP
		Assert(false);
#endif
		return;
	}
#endif
#endif
	center.x = centerX;
	center.y = centerY;
	center.z = centerZ;
	SetToUpdate();
}

//Установить размер описывающего бокса
void ObjectsFindManager::TreeObject::SetBoxSize(float sizeX, float sizeY, float sizeZ)
{
#ifdef ENABLE_NAN_CHECK
#ifndef STOP_DEBUG
	if(_isnan(sizeX) || _isnan(sizeY) || _isnan(sizeZ))
	{
		api->Trace("%x, ObjectsFindManager::TreeObject::SetBoxSize(%f, %f, %f)", this, sizeX, sizeY, sizeZ);
#ifdef ENABLE_NAN_STOP
		Assert(false);
#endif
		return;
	}
#endif
#endif
	size.x = sizeX;
	size.y = sizeY;
	size.z = sizeZ;
	SetToUpdate();
}

//Получить размеры коробки OBB
const Vector & ObjectsFindManager::TreeObject::GetBoxSize()
{
	return size;
}

//Получить центр OBB
const Vector & ObjectsFindManager::TreeObject::GetBoxCenter()
{
	return center;
}

//Получить квадрад, занимаемый на дереве
bool ObjectsFindManager::TreeObject::GetQTAbb(Vector & minAbb, Vector & maxAbb)
{
	if(CheckAbb())
	{
		minAbb.x = (float)abb.minX;
		minAbb.y = 0.0f;
		minAbb.z = (float)abb.minZ;
		maxAbb.x = (float)abb.maxX;
		maxAbb.y = 0.0f;
		maxAbb.z = (float)abb.maxZ;
		return true;
	}
	return false;
}

//Следующий в списке нода
__forceinline ObjectsFindManager::TreeObject * ObjectsFindManager::TreeObject::NextInNode()
{
	return nextInNode;
}

//Следующий в мэнеджера списке
__forceinline ObjectsFindManager::TreeObject * ObjectsFindManager::TreeObject::NextInList()
{
	return nextInList;
}

//Получить текущий abb
__forceinline const ObjectsFindManager::Abb & ObjectsFindManager::TreeObject::GetAbb()
{
	return abb;
}

//Получить нод
__forceinline ObjectsFindManager::Node * ObjectsFindManager::TreeObject::GetNode()
{
	return node;
}

//Поставить в список обновления
__forceinline void ObjectsFindManager::TreeObject::SetToUpdate()
{
	flags &= ~(f_invalidateAbb | f_isUpdatedAbb);
	if(flags & f_active)
	{
		//Встанем в очередь обновления
		MoveToUpdateList();
	}
}

//Проверить и если надо пересчитать бокс
__forceinline bool ObjectsFindManager::TreeObject::CheckAbb()
{
	if((flags & f_isUpdatedAbb) == 0)
	{		
		Vector min, max;
		Box::FindABBforOBB(matrix, center - size*0.5f, center + size*0.5f, min, max, false);		
		bool inRange = abb.Set(min, max, false);
/*
#ifndef STOP_ASSERTS
		if(abb.AssertInRange("ObjectsFindManager::TreeObject::CheckAbb - 2", true))
		{
			api->Trace("object = %x", this);
			api->Trace("f_active = %i", (flags & f_active) != 0);
			api->Trace("f_isUpdatedAbb = %i", (flags & f_isUpdatedAbb) != 0);
			api->Trace("f_inUpdateList = %i", (flags & f_inUpdateList) != 0);
			api->Trace("f_inInactiveList = %i", (flags & f_inInactiveList) != 0);
			api->Trace("f_invalidateAbb = %i", (flags & f_invalidateAbb) != 0);
			api->Trace("f_deleted = %i", (flags & f_deleted) != 0);
			api->Trace("center = %f, %f, %f", center.x, center.y, center.z);
			api->Trace("size = %f, %f, %f", size.x, size.y, size.z);
			api->Trace("matrix = pos(%f, %f, %f), 3x3 = (%f, %f, %f, %f, %f, %f, %f, %f, %f)", matrix.pos.x, matrix.pos.y, matrix.pos.z, matrix.vx.x, matrix.vx.y, matrix.vx.z, matrix.vy.x, matrix.vy.y, matrix.vy.z, matrix.vz.x, matrix.vz.y, matrix.vz.z);
			api->Trace("CheckAbb: min = %f, %f; max = %f, %f", min.x, min.z, max.x, max.z);
			Assert(false);
		}
#endif		
*/
		flags |= f_isUpdatedAbb | f_invalidateAbb;
		if(inRange)
		{
			flags &= ~f_invalidateAbb;
		}
	}
	return (flags & f_invalidateAbb) == 0;
}

//Добавить себя в список нода
__forceinline void ObjectsFindManager::TreeObject::AddToNode(Node * n)
{
	Assert(n);
	RemoveFromNode();
	nextInNode = n->first;
	if(nextInNode)
	{
		nextInNode->prevInNode = this;
	}
	prevInNode = null;
	n->first = this;
	n->count++;
	node = n;
}

//Удалить себя из списка нода
__forceinline void ObjectsFindManager::TreeObject::RemoveFromNode()
{
	if(node)
	{
		if(nextInNode)
		{
			nextInNode->prevInNode = prevInNode;
		}
		if(prevInNode)
		{
			prevInNode->nextInNode = nextInNode;
			prevInNode = null;
		}else{
			Assert(node->first == this);
			node->first = nextInNode;
		}
		nextInNode = null;
		node->count--;
		Assert(node->count >= 0);
		node = null;		
	}
}

//Поместить объект в список обновления
__forceinline void ObjectsFindManager::TreeObject::MoveToUpdateList()
{
	if(flags & f_inInactiveList)
	{
		Assert((flags & f_inUpdateList) == 0);
		RemoveFromList(mng->firstInactive);
		flags &= ~f_inInactiveList;
	}
	if((flags & f_inUpdateList) == 0)
	{
		AddToList(mng->firstUpdate);
		flags |= f_inUpdateList;
	}
}

//Поместить объект в список неактивных
__forceinline void ObjectsFindManager::TreeObject::MoveToInactiveList()
{
	if(flags & f_inUpdateList)
	{
		Assert((flags & f_inInactiveList) == 0);
		RemoveFromList(mng->firstUpdate);
		flags &= ~f_inUpdateList;
	}
	if((flags & f_inInactiveList) == 0)
	{
		AddToList(mng->firstInactive);
		flags |= f_inInactiveList;
	}
}

//Удалить из списков
__forceinline void ObjectsFindManager::TreeObject::RemoveFromLists()
{
	if(flags & f_inInactiveList)
	{
		Assert((flags & f_inUpdateList) == 0);
		RemoveFromList(mng->firstInactive);
		flags &= ~f_inInactiveList;
	}
	if(flags & f_inUpdateList)
	{
		Assert((flags & f_inInactiveList) == 0);
		RemoveFromList(mng->firstUpdate);
		flags &= ~f_inUpdateList;
	}
}

//Добавить в список
__forceinline void ObjectsFindManager::TreeObject::AddToList(TreeObject * & first)
{
	nextInList = first;
	if(nextInList)
	{
		nextInList->prevInList = this;
	}
	prevInList = null;
	first = this;
}

//Удалить из списка
void ObjectsFindManager::TreeObject::RemoveFromList(TreeObject * & first)
{
	if(nextInList)
	{
		nextInList->prevInList = prevInList;
	}
	if(prevInList)
	{
		prevInList->nextInList = nextInList;
		prevInList = null;
	}else{
		Assert(first == this);
		first = nextInList;
	}
	nextInList = null;
}

//Шагнуть по списку обновления
__forceinline ObjectsFindManager::TreeObject * ObjectsFindManager::TreeObject::ExtractFirstFromUpdateList()
{
	Assert(!prevInList);
	TreeObject * obj = nextInList;
	if(nextInList)
	{
		nextInList->prevInList = null;
	}
	nextInList = null;	
	Assert(flags & f_inUpdateList);
	flags &= ~f_inUpdateList;
	return obj;
}

//Создать объект дерева
__forceinline ObjectsFindManager::TreeObject * ObjectsFindManager::TreeObject::Create(ObjectsFindManager * manager, MissionObject * mo)
{
	Assert(ObjectsFindManagerStorage::ptr);
	dword code;
	void * ptr = ObjectsFindManagerStorage::ptr->objects.Alloc(code);
	TreeObject * obj = new('a', ptr) TreeObject(mo, manager);
	obj->pullCode = code;
	return obj;	
}

//Удалить объект дерева
__forceinline void ObjectsFindManager::TreeObject::Delete(TreeObject * obj)
{
	Assert(ObjectsFindManagerStorage::ptr);
	ObjectsFindManagerStorage::ptr->objects.Delete(obj->pullCode);
}


//============================================================================================
//Node
//============================================================================================


__forceinline ObjectsFindManager::Node::Node(const Abb & nodeABB)
{
	abb = nodeABB;
	cx = (abb.minX + abb.maxX + 1)/2;
	cz = (abb.minZ + abb.maxZ + 1)/2;
	child[s_minx_minz] = null;
	child[s_maxx_minz] = null;
	child[s_minx_maxz] = null;
	child[s_maxx_maxz] = null;
	parent = null;
	first = null;
	count = 0;
	pullCode = 0;
}

__forceinline ObjectsFindManager::Node::~Node()
{
	Assert(false);
}

//Найти объекты, касающиеся abb
void ObjectsFindManager::Node::FindObjects(const Abb & checkAbb, array<IMissionQTObject *> & list)
{
	if(checkAbb.IsIntersection(abb))
	{
		//Объекты
		for(TreeObject * obj = first; obj; obj = obj->NextInNode())
		{
			if(checkAbb.IsIntersection(obj->GetAbb()))
			{
				list.Add(obj);
			}			
		}
		//Дети
		if(child)
		{
			if(child[s_minx_minz])
			{
				child[s_minx_minz]->FindObjects(checkAbb, list);
			}
			if(child[s_maxx_minz])
			{
				child[s_maxx_minz]->FindObjects(checkAbb, list);
			}
			if(child[s_minx_maxz])
			{
				child[s_minx_maxz]->FindObjects(checkAbb, list);
			}
			if(child[s_maxx_maxz])
			{
				child[s_maxx_maxz]->FindObjects(checkAbb, list);
			}			
		}
	}
}

//Получить нод для размещения объекта с заданным abb
ObjectsFindManager::Node * ObjectsFindManager::Node::GetTreeNode(const Abb & objAbb)
{
	//Уже точно известно что objAbb внутри данного нода, смотрим на попадание в детей
	if(objAbb.maxX < cx)
	{
		//Объект лежит полностью в левой половине нода
		if(objAbb.maxZ < cz)
		{
			//Объект лежит полностью в левой-задней половине нода
			if(child[s_minx_minz]) return child[s_minx_minz]->GetTreeNode(objAbb);
			//Если объектов на ноде мало, то не рыпаемся
			if(count < maxObjectsPerNode) return this;
			//Пробуем углубляться
			return AddChild(s_minx_minz, abb.minX, cx - 1, abb.minZ, cz - 1);
		}else
		if(objAbb.minZ >= cz)
		{
			//Объект лежит полностью в левой-передней половине нода
			if(child[s_minx_maxz]) return child[s_minx_maxz]->GetTreeNode(objAbb);
			//Если объектов на ноде мало, то не рыпаемся
			if(count < maxObjectsPerNode) return this;
			//Пробуем углубляться
			return AddChild(s_minx_maxz, abb.minX, cx - 1, cz, abb.maxZ);
		}
	}else
	if(objAbb.minX >= cx)
	{
		//Объект лежит полностью в правой половине нода
		if(objAbb.maxZ < cz)
		{
			//Объект лежит полностью в правой-задней половине нода
			if(child[s_maxx_minz]) return child[s_maxx_minz]->GetTreeNode(objAbb);
			//Если объектов на ноде мало, то не рыпаемся
			if(count < maxObjectsPerNode) return this;
			//Пробуем углубляться
			return AddChild(s_maxx_minz, cx, abb.maxX, abb.minZ, cz - 1);
		}else
		if(objAbb.minZ >= cz)
		{
			//Объект лежит полностью в правой-передней половине нода
			if(child[s_maxx_maxz]) return child[s_maxx_maxz]->GetTreeNode(objAbb);
			//Если объектов на ноде мало, то не рыпаемся
			if(count < maxObjectsPerNode) return this;
			//Пробуем углубляться
			return AddChild(s_maxx_maxz, cx, abb.maxX, cz, abb.maxZ);
		}
	}
	//Объект пересекает границы детей и не может быть положен глубже
	return this;
}

//Получить нод для размещения объекта с заданным abb
__forceinline ObjectsFindManager::Node * ObjectsFindManager::Node::AddChild(long index, long abb_minX, long abb_maxX, long abb_minZ, long abb_maxZ)
{	
	if(abb_maxX - abb_minX >= minNodeSizeForSplit && abb_maxZ - abb_minZ >= minNodeSizeForSplit)
	{
		//Размеры позволяют создать ребёнка		
		Node * n = Create(Abb(abb_minX, abb_maxX, abb_minZ, abb_maxZ));
		Assert(!child[index]);
		child[index] = n;
		n->parent = this;
		//Маркируем объекты что необходимо будет обновиться объектам
		for(TreeObject * obj = first; obj; obj = obj->NextInNode())
		{
			obj->SetToUpdate();
		}
		return n;
	}
	return this;
}

//Нарисовать узел, детей и объекты
void ObjectsFindManager::Node::Draw(IRender * render, dword level, float levelScale)
{
#ifndef STOP_DEBUG
	static const Vector endAbb(0.99999f, 0.0f, 0.99999f);
	//Рисуем детей
	for(dword i = 0; i < s_numChilds; i++)
	{
		if(child[i])
		{
			child[i]->Draw(render, level + 1, levelScale);
		}
	}
	//Рисуем себя
	float flevel = level*levelScale;
	Vector vmin(float(abb.minX), (parent ? level - 1 : level)*levelScale, float(abb.minZ));
	Vector vmax(float(abb.maxX), flevel, float(abb.maxZ));
	render->DrawBox(vmin, vmax + endAbb, Matrix(), first ? 0xff00ff00 : 0xff0000ff);
	//Рисуем объекты
	for(TreeObject * obj = first; obj; obj = obj->NextInNode())
	{
		const Matrix & mtx = obj->GetMatrix();
		//Рисуем мировой ящик объекта
		Vector vmin = obj->GetBoxCenter() - obj->GetBoxSize()*0.5f;
		Vector vmax = obj->GetBoxCenter() + obj->GetBoxSize()*0.5f;
		render->DrawBox(vmin, vmax + endAbb, mtx, 0xffffff00);
		Vector worldCenter = mtx*((vmin + vmax)*0.5f);
		//Рисуем квадрат на дереве
		Vector nodeCenter;
		if(obj->GetQTAbb(vmin, vmax))
		{
			vmin.y = flevel;
			vmax.y = flevel;
			vmax += endAbb;
			render->DrawBox(vmin, vmax, Matrix(), 0xff909f20);
			nodeCenter = (vmin + vmax)*0.5f;
			render->Print(nodeCenter, 30.0f, 0.0f, 0xff909f20, "%u", level);			
		}else{
			nodeCenter = worldCenter;
			nodeCenter.y = flevel;
		}
		//Линия до нода
		render->DrawLine(nodeCenter, 0xff0000ff, worldCenter, 0xffffff00);
		//Подпись
		render->Print(worldCenter, 100000.0f, 0.0f, 0xff00ff00, "MO: %s%s, 0x%.8x", 
			obj->GetMissionObject().GetObjectID().c_str(),
			obj->CheckAbb() ? "" : " [invalidate box]",
			obj);

	}
#endif
}

//Создать нод
__forceinline ObjectsFindManager::Node * ObjectsFindManager::Node::Create(const Abb & nodeABB)
{
	Assert(ObjectsFindManagerStorage::ptr);
	dword code;
	void * ptr = ObjectsFindManagerStorage::ptr->nodes.Alloc(code);
	Node * n = new('a', ptr) Node(nodeABB);
	n->pullCode = code;
	return n;	
}

//Удалить нод
void ObjectsFindManager::Node::Delete(Node * n)
{	
	if(!n) return;
	Assert(ObjectsFindManagerStorage::ptr);
	//Удаляем детей
	Delete(n->child[s_minx_minz]);
	Delete(n->child[s_maxx_minz]);
	Delete(n->child[s_minx_maxz]);
	Delete(n->child[s_maxx_maxz]);
	//Отписываем от нода объекты
	while(n->first)
	{
		n->first->RemoveFromNode();
	}
	//Удаляем себя
	ObjectsFindManagerStorage::ptr->nodes.Delete(n->pullCode);
}


//Вывести ветку в лог
void ObjectsFindManager::Node::Dump(dword offset = 0)
{		
#ifndef STOP_DEBUG
	char buffer[256];
	memset(buffer, ' ', sizeof(buffer));
	if(offset > sizeof(buffer) - 1) offset = sizeof(buffer) - 1;
	buffer[offset] = 0;
	abb.LogOut(buffer);
	api->Trace("%sCenter: (x: %i, z: %i)", buffer, cx, cz);
	api->Trace("%sObjects count: %i", buffer, count);
	const char * names[4];
	names[s_minx_minz] = "s_minx_minz";
	names[s_maxx_minz] = "s_maxx_minz";
	names[s_minx_maxz] = "s_minx_maxz";
	names[s_maxx_maxz] = "s_maxx_maxz";
	for(long i = 0; i < 4; i++)
	{
		if(child[i])
		{
			api->Trace("%s%s = ", buffer, names[i]);
			api->Trace("%s{", buffer);
			child[i]->Dump(offset + 2);
			api->Trace("%s", buffer);
			api->Trace("%s}", buffer);
		}else{
			api->Trace("%s%s = null", buffer, names[i]);
		}
	}
#endif
}

//============================================================================================
//ObjectsFindManager
//============================================================================================

ObjectsFindManager::ObjectsFindManager()
{
	root = null;
	firstUpdate = null;
	firstInactive = null;
}

ObjectsFindManager::~ObjectsFindManager()
{
	if(root)
	{
		Node::Delete(root);
		root = null;
	}	
}

//Создать объект в дереве
IMissionQTObject * ObjectsFindManager::CreateObject(MissionObject * mo, const char * cppFile, long cppLine)
{
	TreeObject * obj = TreeObject::Create(this, mo);
	obj->cppFile = cppFile;
	obj->cppLine = cppLine;
	return obj;
}

//Найти объекты, попадающие в область
void ObjectsFindManager::FindObjects(const Vector & minVrt, const Vector & maxVrt, array<IMissionQTObject *> & list)
{
	//Обновим дерево
	UpdateTree();
	//Получаем целочисленный объём
	Abb abb;
	abb.Set(minVrt, maxVrt, true);
	//Очистим список
	list.Empty();
	//Собираем попадающих в область
	if(!root) return;
	root->FindObjects(abb, list);
}

//Нарисовать дерево и подписать объекты
void ObjectsFindManager::Draw(IRender * render, float levelScale)
{
#ifndef STOP_DEBUG
	//Обновим дерево
	UpdateTree();
	//Рисуем активные объекты
	if(root)
	{
		root->Draw(render, 0, levelScale);
	}
	//Пишем список неактивных объектов под 0
	if(firstInactive)
	{
		float h = 3.0f;
		float k = Clampf((h - render->GetView().GetCamPos().y)/h);
		dword color = (dword(k*255.0f) << 24) | 0x00ff4040;
		Vector drawPoint = Vector(0.0f, -1.0f, 0.0f);
		float line = 0.0f;
		float step = 1.25f;		
		render->Print(drawPoint, 100000.0f, line, color, "Inactive list:");
		for(TreeObject * obj = firstInactive; obj; obj = obj->NextInList())
		{
			line += step;			
			render->Print(drawPoint, 100000.0f, line, color, "MO: %s%s, 0x%.8x", 
				obj->GetMissionObject().GetObjectID().c_str(),
				obj->CheckAbb() ? "" : " [invalidate box]",
				obj);
		}
	}
#endif
}

//Вывести ноды дерева в лог
void ObjectsFindManager::Dump()
{
#ifndef STOP_DEBUG
	//Обновим дерево
	UpdateTree();
	//Выводим в лог
	if(root)
	{
		root->Dump();
	}
#endif
}

//Обновить дерево
void ObjectsFindManager::UpdateTree()
{	
	//Проходим по списку объектов, ожидающих обновление
	while(firstUpdate)
	{
		TreeObject * obj = firstUpdate;
		//Исключим из списка
		firstUpdate = obj->ExtractFirstFromUpdateList();
		//Посчитаем границы объекта
		if(obj->CheckAbb())
		{
			const Abb & objAbb = obj->GetAbb();
#ifndef STOP_ASSERTS
			objAbb.AssertInRange("ObjectsFindManager::UpdateTree");
#endif
			//Надо подобрать объекту нод
			Node * n = obj->GetNode();
			if(n)
			{				
				if(n->abb.IsInclude(objAbb))
				{
					n = n->GetTreeNode(objAbb);
					obj->AddToNode(n);
				}else{
					//Поднимаемся на верх
					n = n->parent;
					//Поднимаемся от нода, пока не начнём влазить
					while(n)
					{
						if(n->abb.IsInclude(objAbb))
						{
							break;
						}
						n = n->parent;
					}
					if(n)
					{
						//Добавляем
						n = n->GetTreeNode(objAbb);
						obj->AddToNode(n);
						continue;
					}
				}
			}
			//Надо добавлять начиная от корня дерева
			ExtrudeTree(objAbb);
			Assert(root);
			n = root->GetTreeNode(objAbb);
			Assert(n);
			obj->AddToNode(n);
		}else{
			//Объект имеет неправильный ящик
			obj->MoveToInactiveList();
		}
	}
}

//Расширить дерево до размеров включающих abb
void ObjectsFindManager::ExtrudeTree(const Abb & abb)
{
	if(root)
	{		
		while(!root->abb.IsInclude(abb))
		{
			//Определяем размер стартового нода
			Abb newAbb(root->abb);
			long index = 0;
			if(newAbb.maxZ < abb.maxZ)
			{
				//Вперёд
				newAbb.maxZ += newAbb.maxZ - newAbb.minZ + 1;
			}else{
				//Назад
				newAbb.minZ -= newAbb.maxZ - newAbb.minZ + 1;
				index |= s_maxz_mask;
			}
			if(newAbb.maxX < abb.maxX)
			{
				//Вправо			
				newAbb.maxX += newAbb.maxX - newAbb.minX + 1;
			}else{
				//Влево
				newAbb.minX -= newAbb.maxX - newAbb.minX + 1;
				index |= s_maxx_mask;
			}
			//Добавляем уровень иерархии
			Node * n = root;
			root = Node::Create(newAbb);
			root->child[index] = n;
			n->parent = root;			
			//Проверить, чтобы при увеличении были правильные размеры ребёнка
#ifndef STOP_DEBUG
			if(index & s_maxx_mask)
			{
				if(root->cx != root->child[index]->abb.minX)
				{
					abb.LogOut("ObjectsFindManager::Extrude ");
					root->Dump();
					Assert(root->cx == root->child[index]->abb.minX);
				}
			}else{
				if(root->cx != root->child[index]->abb.maxX + 1)
				{
					abb.LogOut("ObjectsFindManager::Extrude ");
					root->Dump();
					Assert(root->cx == root->child[index]->abb.maxX + 1);
				}
				
			}
			if(index & s_maxz_mask)
			{
				if(root->cz != root->child[index]->abb.minZ)
				{
					abb.LogOut("ObjectsFindManager::Extrude ");
					root->Dump();					
					Assert(root->cz == root->child[index]->abb.minZ);
				}				
			}else{
				if(root->cz != root->child[index]->abb.maxZ + 1)
				{
					abb.LogOut("ObjectsFindManager::Extrude ");
					root->Dump();
					Assert(root->cz == root->child[index]->abb.maxZ + 1);
				}				
			}
#endif
		}
	}else{
		//Добавляем квадратный нод
		float centerX = (abb.minX + abb.maxX)*0.5f;
		float centerZ = (abb.minZ + abb.maxZ)*0.5f;
		long dX = abb.maxX - abb.minX;
		long dZ = abb.maxZ - abb.minZ;
		float size = (coremax(dX, dZ))*0.5f + 0.999999f;
		Abb quadAbb;
		quadAbb.minX = long(centerX - size);
		quadAbb.maxX = long(centerX + size);
		quadAbb.minZ = long(centerZ - size);
		quadAbb.maxZ = long(centerZ + size);
		root = Node::Create(quadAbb);
	}
}



//============================================================================================
//ObjectsFindManagerStorage
//============================================================================================

CREATE_SERVICE(ObjectsFindManagerStorage, 100000)

ObjectsFindManagerStorage * ObjectsFindManagerStorage::ptr = null;

ObjectsFindManagerStorage::ObjectsFindManagerStorage()
{
	Assert(!ptr);
	ptr = this;
}

ObjectsFindManagerStorage::~ObjectsFindManagerStorage()
{
	Assert(ptr == this);
	ptr = null;
	nodes.Clear();
	objects.Clear();
}





