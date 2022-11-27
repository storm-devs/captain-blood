#ifndef _LOADER_H_
#define _LOADER_H_

#include "MissionEditor.h"
#include "..\common_h\tinyxml\tinyxml.h"
#include "NodesPool.h"

class MissionLoad
{
 struct ObjectJustLoaded
 {
	 MOSafePointer pObject;
	 GUITreeNode* node;
	 MissionEditor::tCreatedMO* pCreated;
 };

 array<ObjectJustLoaded> obj;

 struct TreeNodeJustLoaded
 {
	 GUITreeNode* node;
	 string node_fullname;
 };
 
 array<TreeNodeJustLoaded> JLLoadedNode;

 MissionEditor* editor;

public:

 MissionLoad (MissionEditor* _editor);
 ~MissionLoad ();
 
 MissionEditor::tCreatedMO* CreateObject (const char* ClassName, int level, DWORD version, const char* _ObjectName, AttributeList* ldAttrList, const char* PathInTree);

 AttributeList* CreateNecessaryAL (const char* ClassName, DWORD LoadedVersion, DWORD OriginalVersion, AttributeList* LoadedAttrList, AttributeList* OriginalAttrList);

 void LoadMSR (const char* filename, bool SetCameraPos = true);
 void Loading (const char* filename, bool SetCameraPos = true);

 void LoadTree (IFile* pFile, GUITreeView* TreeView);
 GUITreeNode* LoadNode (IFile* pFile, GUITreeNodes* nodes, int n = 0);

 void MakeLoadedObjectsUniqueNames ();

 void UpdateAllObjects ();


 void LoadXML (const char* filename, bool SetCameraPos, bool bAsReadOnly);

 bool ReadXMLTree (TiXmlElement* Root, GUITreeNodes* nodes, TreeNodesPool* nodesPool);
 bool ReadXMLObject (TiXmlElement* Root, GUITreeNode* pTreeNode, TreeNodesPool* nodesPool);
 AttributeList* ReadXMLAttributes (TiXmlElement* Root, const char* szMasterClass);

};


#endif