#ifndef GMX_ENTITY_INTERFACE
#define GMX_ENTITY_INTERFACE

#include "..\core.h"
#include "..\templates.h"
#include "..\d_types.h"
#include "..\math3d.h"
#include "igmx_scene.h"

/*
typedef enum _GMXCURVETYPE
{
	GMXCT_OPEN,
	GMXCT_CLOSED,
	GMXCT_NONCURVE,

	GMXET_FORCE_DWORD = 0x7fffffff
} GMXCURVETYPE;
*/

typedef void (_cdecl Object::*GMX_EVENT)();

class IGMXMesh;


class IGMXEntity
{
protected:
	//Деструктор
	virtual ~IGMXEntity() {};

public:

	//Конструктор
	IGMXEntity() {};

	//Уменьшить счетчик ссылок -> Удаление объекта
	virtual bool Release () = 0;

	//Увеличить счетчик ссылок
	//virtual void AddRef () = 0;

	//Увеличить счетчик ссылок, тоже самое, что и AddRef
	virtual IGMXEntity* Clone() = 0;


	//Получить имя
	virtual const char* GetName () = 0;

	//Получить родителя
	//virtual IGMXEntity* GetParent () = 0;

	//Получить кол-во детей
	//virtual dword ChildsCount () = 0;

	//Получить указатель на ребенка
	//virtual IGMXEntity* Child(dword dwIndex) = 0;

	//Получить матрицу трансформации Local
	//virtual const Matrix& GetTransform () = 0;

	//Установить матрицу трансформации Local
	//virtual void SetTransform (const Matrix& m, bool bUpdateBoundBox = true) = 0;

	//"Получить" мировую матрицу (если есть непротухший кэш, то из кеша, иначе рассчитываеться)
	virtual const Matrix& GetLocalTransform () = 0;
	virtual const Matrix& GetWorldTransform() = 0;

	//Получить указатель на сцену хозяина
	//virtual IGMXScene* Scene () = 0;


	//virtual GMXENTITYTYPE GetType() = 0;

	virtual void SubscribeDeletionEvent (Object* _class, GMX_EVENT _method) = 0;
	virtual void UnSubscribeDeletionEvent (Object* _class) = 0;


	//virtual string GetFullName () = 0;

	//virtual IGMXMesh* GetMesh() = 0;


	//virtual bool SetVisible (bool __bVisible) = 0;
	//virtual bool GetVisible () = 0;
	//virtual bool SwitchVisible () = 0;
	//virtual bool IsVisible() = 0;


	//virtual bool SetEnabled (bool __bEnabled) = 0;
	//virtual bool GetEnabled () = 0;
	//virtual bool SwitchEnabled () = 0;
	//virtual bool IsEnabled() = 0;

	//virtual bool SetCollision (bool __bCollision) = 0;
	//virtual bool GetCollision () = 0;
	//virtual bool SwitchCollision () = 0;
	//virtual bool IsCollision() = 0;


	//virtual void Draw() = 0;

	virtual const GMXBoundBox& GetBound() = 0;

	//virtual dword GetCurvePointsCount () = 0;
	//virtual const Vector& GetCurvePoint(dword dwIndex) = 0;
	//virtual GMXCURVETYPE GetCurveType () = 0;


	//virtual void ForceRelease () = 0;

	virtual void TransformationIsSpoiledOnAllParentAnimationLocators() = 0;

	virtual void AnimationIsSpoil () = 0;

};

#endif

