#ifndef _SAVER_H_
#define _SAVER_H_

#include "MissionEditor.h"

class TextFile;

class MissionSave
{

 MissionEditor* editor;


 

public:

 MissionSave (MissionEditor* _editor);
 ~MissionSave ();
 

 void SaveMSR (const char* input_filename);

 void WriteTree (IFile* pFile, GUITreeView* TreeView);
 void WriteNode (IFile* pFile, GUITreeNode* node);


 void SaveXML (const char* filename);
 void WriteTreeToXML (TextFile &file, GUITreeView* TreeView);
 void WriteTreeNodeToXML (TextFile &file, GUITreeNode* node, int level);

 void WriteObject(TextFile &file, MissionEditor::tCreatedMO* pObject, int level);

};


#endif