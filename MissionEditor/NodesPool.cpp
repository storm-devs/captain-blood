#include "NodesPool.h"
#include "..\common_h\gui.h"


TreeNodesPool * pool = NULL;


TreeNodesPool::TreeNodesPool()
{
	Assert(pool == NULL);
	pool = this;
	bIsReadOnlyMode = false;
}

TreeNodesPool::~TreeNodesPool()
{
	pool = NULL;
	nodesPool.Destroy();
}


void TreeNodesPool::DefaultReadOnly(bool bVal)
{
	bIsReadOnlyMode = bVal;

}

GUITreeNode* TreeNodesPool::CreateNode ()
{
	EditorTreeNode* nd = nodesPool.Allocate();

	nd->bReadOnly = bIsReadOnlyMode;

	if (bIsReadOnlyMode)
	{
		nd->CanDrag = false;
		nd->CanDrop = false;
	}

	return nd;
	//return NEW GUITreeNode;
}


//---------------------------------

void EditorTreeNode::Release ()
{
	// Если есть список-владелец
	if (OwnerList)
	{
		// Ищем себя в этом списке
		for (int n =0; n < OwnerList->GetCount (); n++)
		{
			// Нашли
			if (OwnerList->Get (n) == this)
			{
				// Удалили себя из списка...
				OwnerList->Delete (n);
				pool->nodesPool.Free(this);
				//delete this;
				break;
			}
		}
	} else
	{
		pool->nodesPool.Free(this);
		//delete this;
	}


	
}

