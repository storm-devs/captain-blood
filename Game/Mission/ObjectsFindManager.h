//============================================================================================
// Spirenkov Maxim, 2004, 2008
//============================================================================================
// ObjectFindManager (quad tree)
//============================================================================================

#ifndef _MissionObjectFinderManager_h_
#define _MissionObjectFinderManager_h_

#include "..\..\Common_h\Mission.h"
#include "..\..\Common_h\templates\pulls.h"

class ObjectsFindManagerStorage;

class ObjectsFindManager
{
public:
	struct Abb
	{
		Abb();
		Abb(const Abb & abb);		
		Abb(long _minX, long _maxX, long _minZ, long _maxZ);
		bool IsInclude(const Abb & abb) const;
		bool IsIntersection(const Abb & abb) const;		
		long MinSize() const;
		bool Set(Vector min, Vector max, bool isChecker);
		bool AssertInRange(const char * prefix, bool noStop = false) const;
		void LogOut(const char * prefix) const;

		long minX, maxX;
		long minZ, maxZ;
	};

	struct Node;
	class TreeObject;

	class TreeObject : public IMissionQTObject
	{
	public:
		enum Flags
		{
			f_active = 1,
			f_isUpdatedAbb = 4,
			f_inUpdateList = 8,
			f_inInactiveList = 32,
			f_invalidateAbb = 64,
			f_deleted = 256,
		};
	public:
		TreeObject(MissionObject * _mo, ObjectsFindManager * _mng);

		//Освободить ресурсы
		virtual void Release();
		//Изменить активность
		virtual void Activate(bool isActive);
		//Активен ли объект
		virtual bool IsActivate();
		//Получить указатель на объект миссии
		virtual MissionObject & GetMissionObject();
		//Установить пользовательские данные
		virtual void SetUserData(void * data);
		//Получить пользовательские данные
		virtual void * GetUserData();
		//Установить матрицу
		virtual void SetMatrix(const Matrix & mtx);
		//Получить матрицу
		virtual const Matrix & GetMatrix() const;
		//Установить локальную позицию описывающего бокса
		virtual void SetBoxCenter(float centerX, float centerY, float centerZ);
		//Установить размер описывающего бокса
		virtual void SetBoxSize(float sizeX, float sizeY, float sizeZ);
		//Получить размеры коробки OBB
		virtual const Vector & GetBoxSize();
		//Получить центр OBB
		virtual const Vector & GetBoxCenter();
		//Получить квадрад, занимаемый на дереве
		virtual bool GetQTAbb(Vector & minAbb, Vector & maxAbb);

		//Следующий в списке нода
		TreeObject * NextInNode();
		//Следующий в мэнеджера списке
		TreeObject * NextInList();
		//Получить текущий abb
		const Abb & GetAbb();
		//Получить нод
		Node * GetNode();
		//Проверить и если надо пересчитать бокс
		bool CheckAbb();
		//Поставить в список обновления
		void SetToUpdate();

		
		//Добавить себя в список нода
		void AddToNode(Node * n);
		//Удалить себя из списка нода
		void RemoveFromNode();

		//Поместить объект в список обновления
		void MoveToUpdateList();
		//Поместить объект в список неактивных
		void MoveToInactiveList();
		//Удалить из списков
		void RemoveFromLists();
		//Шагнуть по списку обновления
		TreeObject * ExtractFirstFromUpdateList();

		//Создать объект дерева
		static TreeObject * Create(ObjectsFindManager * manager, MissionObject * mo);
		//Удалить объект дерева
		static void Delete(TreeObject * obj);

	
	private:
		//Добавить в список
		void AddToList(TreeObject * & first);
		//Удалить из списка
		void RemoveFromList(TreeObject * & first);


	private:
		dword flags;				//Флажки
		Node * node;				//Текущий нод
		ObjectsFindManager * mng;	//Менеджер, которому принадлежим
		TreeObject * nextInNode;	//Следующий в списке нода
		TreeObject * prevInNode;	//Предыдущий в ноде
		TreeObject * nextInList;	//Следующий в списке обновления
		TreeObject * prevInList;	//Предыдущий в списке обновления
		Abb abb;					//Ограничения в плоскости XZ
		Matrix matrix;				//Позиция колидера
		Vector center;				//Центр ящика колидера
		Vector size;				//Размер ящика колидера		
		MissionObject * mo;			//Объект миссии
		void * userData;			//Пользовательские данные
	public:
		const char * cppFile;		//Исходный файл где был создан объект
		long cppLine;				//Строка исходного файла где был создан объект
		dword pullCode;				//Код для пула памяти
		dword tmp;
	};

	friend class TreeObject;

	enum Consts
	{
		s_maxx_mask = 1,
		s_maxz_mask = 2,
		s_minx_minz = 0,	
		s_maxx_minz = s_maxx_mask,
		s_minx_maxz = s_maxz_mask,
		s_maxx_maxz = s_maxx_mask | s_maxz_mask,
		s_numChilds = 4,
		maxObjectsPerNode = 8,
		minNodeSizeForSplit = 2,
	};

	struct Node
	{
		Node(const Abb & nodeABB);
		~Node();

		//Найти объекты, касающиеся abb
		void FindObjects(const Abb & checkAbb, array<IMissionQTObject *> & list);
		//Получить нод для размещения объекта с заданным abb
		Node * GetTreeNode(const Abb & objAbb);
		//Получить нод для размещения объекта с заданным abb
		Node * AddChild(long index, long abb_minX, long abb_maxX, long abb_minZ, long abb_maxZ);
		//Нарисовать узел, детей и объекты
		void Draw(IRender * render, dword level, float levelScale);

		//Создать нод
		static Node * Create(const Abb & nodeABB);
		//Удалить нод
		static void Delete(Node * n);

		//Вывести ветку в лог
		void Dump(dword offset);
		

		Abb abb;						//Границы нода
		long cx, cz;					//Центр нода, являющийся началом s_maxx_maxz
		Node * child[4];				//Дети нода
		Node * parent;					//Родитель нода
		TreeObject * first;				//Первый элемент на ноде
		long count;						//Количество объектов в ноде
		dword pullCode;					//Данные для пула памяти		
	};

public:
	ObjectsFindManager();
	virtual ~ObjectsFindManager();

public:
	//Создать объект в дереве
	IMissionQTObject * CreateObject(MissionObject * mo, const char * cppFile, long cppLine);
	//Найти объекты, попадающие в область
	void FindObjects(const Vector & minVrt, const Vector & maxVrt, array<IMissionQTObject *> & list);
	//Нарисовать дерево и подписать объекты
	void Draw(IRender * render, float levelScale);
	//Вывести ноды дерева в лог
	void Dump();

protected:
	//Обновить дерево
	void UpdateTree();
	//Расширить дерево до размеров включающих abb
	void ExtrudeTree(const Abb & abb);


protected:
	Node * root;					//Корневой нод
	TreeObject * firstUpdate;		//Первый для обновления
	TreeObject * firstInactive;		//Первый не активный
};

//Хранилище для объектов 
class ObjectsFindManagerStorage : public Service
{
public:
	ObjectsFindManagerStorage();
	virtual ~ObjectsFindManagerStorage();
	
	Pulls<sizeof(ObjectsFindManager::Node)> nodes;
	Pulls<sizeof(ObjectsFindManager::TreeObject)> objects;
	static ObjectsFindManagerStorage * ptr;
};




#endif
