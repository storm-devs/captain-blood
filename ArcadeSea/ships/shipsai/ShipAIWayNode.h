

#ifndef _ShipAIWayNode_h_
#define _ShipAIWayNode_h_

#include "..\..\..\Common_h\Mission.h"
#include "ShipAIDebug.h"

enum NodeState
{
	NODE_STATE_CAN_PASS = 0,
	NODE_STATE_CANNOT_PASS,
	NODE_STATE_EXACTLY
};

// объявление класса узла пути
class WayNode 
{
public:

	WayNode ();

	WayNode ( float fX,  float fZ, NodeState eState = NODE_STATE_CAN_PASS );

	WayNode ( float fX, float fY, float fZ, NodeState eState = NODE_STATE_CAN_PASS );

	WayNode & operator=( const WayNode & tWayNode );

public:
	Vector					m_vPoint;

	NodeState			m_eState;
};

// объявление класса узла в редакторе
class EditorWayNode : public MissionObject, public  WayNode
{
public:

	EditorWayNode ();

	EditorWayNode ( float fX,  float fZ  );

	EditorWayNode ( float fX, float fY, float fZ  );

	~EditorWayNode ();

	void									SetPred ( EditorWayNode	*	pPredNode );

	EditorWayNode	*			GetPred ();

	void									SetNext ( EditorWayNode	*	pNextNode );

	EditorWayNode	*			GetNext ();

	void									SetPos ( Vector vNodePos );

	int										GetWayIndex ();

	// инициализация в списке нодов
	void									NodeInit ();
	//Инициализировать объект
	bool									Create ( MOPReader & reader );
	//Инициализировать объект
	bool									EditMode_Create ( MOPReader & reader );
	//Обновить параметры
	bool									EditMode_Update ( MOPReader & reader );

private:
	//Работа
	void _cdecl						Work  ( float dltTime, long level );
	//Работа в режиме редактирования
	void _cdecl						EditModeWork  ( float dltTime, long level );
	// отрисовка дебужной инфы
	void									DrawDebugInfo ();

	//ObjectFinder   					m_Finder;

	EditorWayNode	*			m_pNextNode;	

	EditorWayNode	*			m_pPredNode;

	int										m_iIndex;
};

#endif