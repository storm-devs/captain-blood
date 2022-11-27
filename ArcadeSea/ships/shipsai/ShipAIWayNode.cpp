
#include "ShipAIWayNode.h"

#define MG_LOGIC			"logic"

//============================================================================================
// реализация класса узла пути
//============================================================================================
WayNode::WayNode ()
: m_vPoint ( 0, 0, 0 )
, m_eState ( NODE_STATE_CAN_PASS )
{
};

WayNode::WayNode ( float fX,  float fZ, NodeState eState )
: m_vPoint ( fX, 0, fZ )
, m_eState ( eState )
{
};

WayNode::WayNode ( float fX, float fY, float fZ, NodeState eState )
: m_vPoint ( fX, fY, fZ )
, m_eState ( eState )
{	
};

WayNode & WayNode::operator=( const WayNode & tWayNode )
{
	m_vPoint = tWayNode.m_vPoint;
	m_eState = tWayNode.m_eState;

	return *this;
};


//============================================================================================
// реализация класса узла в редакторе
//============================================================================================

EditorWayNode::EditorWayNode ()
: m_pNextNode ( NULL )
, m_pPredNode ( NULL )
, m_iIndex ( 0 )
{
};

EditorWayNode::EditorWayNode ( float fX,  float fZ  )
: WayNode (  fX, fZ )
, m_pNextNode ( NULL )
, m_pPredNode ( NULL )
, m_iIndex ( 0 )
{
};

EditorWayNode::EditorWayNode ( float fX, float fY, float fZ  )
: WayNode (  fX, fY, fZ )
, m_pNextNode ( NULL )
, m_pPredNode ( NULL )
, m_iIndex ( 0 )
{
};

EditorWayNode::~EditorWayNode ()
{
	if  ( m_pPredNode )
		m_pPredNode->SetNext ( m_pNextNode );

	if  ( m_pNextNode )
		m_pNextNode->SetPred ( m_pPredNode );
};

void	EditorWayNode::SetPred ( EditorWayNode	*	pPredNode )
{
	m_pPredNode = pPredNode;
};

EditorWayNode	*	EditorWayNode::GetPred ()
{
	return m_pPredNode;
};

void	EditorWayNode::SetNext ( EditorWayNode	*	pNextNode )
{
	m_pNextNode = pNextNode;
};

EditorWayNode	*	EditorWayNode::GetNext ()
{
	return m_pNextNode;
};

void	EditorWayNode::SetPos ( Vector vNodePos )
{
	m_vPoint = vNodePos;
};

int	EditorWayNode::GetWayIndex ()
{
	return m_iIndex;
};

void EditorWayNode::NodeInit ()
{
	if ( m_pNextNode || m_pPredNode )
		return;

	Vector vMin ( -1e+3f, -1e+3f, -1e+3f  );
	Vector vMax ( 1e+3f, 1e+3f, 1e+3f  );
	/*int iExistNodesCount = FindObjects ( MG_LOGIC , vMin, vMax );

	for ( int i = 0; i <  iExistNodesCount; ++i )
	{
		ObjectFinder * pObjFinder = GetFindObject  ( i );
		Assert  ( pObjFinder );

		MissionObject * pMissionObject = pObjFinder->GetMissionObject  ();
		Assert  ( pMissionObject );

		EditorWayNode * pEditorNode = static_cast<EditorWayNode *>  ( pMissionObject );

		if  ( !pEditorNode )
			continue;

		// добавление нода в соответствующую группу нодов
		if  ( pEditorNode != this &&  NULL == pEditorNode->GetNext  () &&  m_iIndex == pEditorNode->GetWayIndex  () )
		{
			SetPred  ( pEditorNode );
			pEditorNode->SetNext  ( this );
			break;
		};
	};*/

};

bool	EditorWayNode::Create ( MOPReader & reader )
{
	SetUpdate ( ( MOF_UPDATE )&EditorWayNode::Work, ML_EXECUTE2 );

	bool bRes = EditMode_Update ( reader );

	//Registry ( MG_LOGIC, m_Finder );
	//m_Finder.SetMissionObject ( this );

	NodeInit ();

	return bRes;
};

bool	EditorWayNode::EditMode_Create ( MOPReader & reader  )
{
	SetUpdate ( ( MOF_UPDATE ) &EditorWayNode::EditModeWork, ML_GEOMETRY1 );

	bool bRes = EditMode_Update ( reader );

	//Registry ( MG_LOGIC, m_Finder );
	//m_Finder.SetMissionObject ( this );

	NodeInit ();

	return bRes;
}

bool EditorWayNode::EditMode_Update ( MOPReader & reader )
{
	int iNodeIndex = reader.Long ();
	Vector vPosition  = reader.Position ();
	bool bCanPass = reader.Bool ();

	m_iIndex  = iNodeIndex;
	m_vPoint = vPosition;
	m_eState = bCanPass  ? NODE_STATE_CAN_PASS : NODE_STATE_CANNOT_PASS;

	Activate ( true );
	return true;
};

//Работа
void _cdecl	EditorWayNode::Work  ( float dltTime, long level )
{
	DrawDebugInfo ();
};

//Работа в режиме редактирования
void _cdecl	EditorWayNode::EditModeWork  ( float dltTime, long level )
{
	DrawDebugInfo ();
};

// отрисовка дебужной инфы
void	EditorWayNode::DrawDebugInfo ()
{
#ifndef DEBUG_DRAW
	return;
#endif

	int iCirclePointsCount = 12;
	float fNodeRadius = 0.2f;

	IRender & tRender = Render ();

	for ( int i = 0; i < iCirclePointsCount; ++i )
	{
		Vector vStart = m_vPoint + Vector ( fNodeRadius*sinf  ( 2*i*PI / iCirclePointsCount ), 0, fNodeRadius*cosf  ( 2*i*PI / iCirclePointsCount ) );
		Vector vEnd = m_vPoint + Vector ( fNodeRadius*sinf  ( 2*( i+1 )*PI / iCirclePointsCount ), 0, fNodeRadius*cosf  ( 2*( i+1 )*PI / iCirclePointsCount ) );
		tRender.DrawBufferedLine ( vStart, 0xffffff00, vEnd, 0xffffff00, false );
	};

	EditorWayNode * pNext = GetNext ();

	if  ( pNext )
	{
		Vector vNextPoint = pNext->m_vPoint;
		tRender.DrawBufferedLine  ( m_vPoint, 0xffffff00, vNextPoint, 0xffffff00, false );
	};
};


MOP_BEGINLISTG ( EditorWayNode , "Editor Way Node", '1.00', 0, "Default" )
MOP_LONGC  ( "Way Index", 0, "Index of corresponding way" )
MOP_POSITIONC ( "Node position", Vector(0.0f, 0.0f, 0.0f), " default position  ")
MOP_BOOLC ( "Can Pass", false, " node can be passed" )
MOP_ENDLIST ( EditorWayNode )