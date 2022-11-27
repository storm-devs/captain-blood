#ifndef TREE_NODES_POOL
#define TREE_NODES_POOL


#include "..\common_h\core.h"
#include "..\common_h\gui.h"

//#include "gmx_Pool.h"

//class GUITreeNode;


class EditorTreeNode : public GUITreeNode
{

	
	friend class objectsPool<EditorTreeNode, 4096>;

protected:
	virtual ~EditorTreeNode ()
	{
	}

public:

	EditorTreeNode()
	{
	}

	virtual void Release ();

};




class TreeNodesPool
{
public:
	objectsPool<EditorTreeNode, 4096> nodesPool;

private:
	bool bIsReadOnlyMode;

public:


	TreeNodesPool();

	~TreeNodesPool();

	GUITreeNode* CreateNode ();

	void DefaultReadOnly(bool bVal);


};


#endif