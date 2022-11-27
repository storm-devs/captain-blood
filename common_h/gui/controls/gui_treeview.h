#ifndef _XBOX
#ifndef GUI_TREEVIEW
#define GUI_TREEVIEW

#include "gui_control.h"
#include "gui_font.h"
#include "gui_stringlist.h"
#include "gui_scrollbar.h"

#include "gui_events.h"
#include "gui_image.h"

#include "..\..\Templates\string.h"



#define TREE_VIEW_ITEM_LEN (128)


// Для навешивания обработчика приравниванием :)
#define OnChange \
t_OnChange->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)


#define OnBeforeChange \
	t_OnBeforeChange->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
	*(hack_temp_ptr)





class GUITreeNode;
//-----------------------------------------------------------

extern GUIEventHandler* hack_temp_ptr;

// Для навешивания обработчика приравниванием :)
#define OnAdd \
t_OnAdd->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

#define OnClear \
t_OnClear->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

#define OnCopyNode \
t_OnCopyNode->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

#define OnDblClick \
t_OnDblClick->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)


#define AfterDrop \
	t_AfterDrop->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)

#define BeforeDrop \
	t_BeforeDrop->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)


#define OnFind \
t_OnFind->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)


#define OnExpand \
t_OnExpand->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)


#define OnColapse \
t_OnColapse->GetThis (hack_temp_ptr); \
hack_temp_ptr->SetObject (this); \
*(hack_temp_ptr)


#define OnRightSelect \
	t_OnRightSelect->GetThis (hack_temp_ptr); \
	hack_temp_ptr->SetObject (this); \
	*(hack_temp_ptr)



class GUITreeNodes
{
	bool bSystemClear;
	array<GUITreeNode*> Items;


public:
 
	GUITreeNodes ();
  ~GUITreeNodes ();
	

	GUITreeNode* FindItemByName (const char* name);
	GUITreeNode* FindItemByData (void* Data);

  int Add (GUITreeNode* n);
	
	void Clear ();
	
	int GetCount ();
	
	GUITreeNode* Get(int index);
	
	void Insert (int index, GUITreeNode* n);
	
	void Delete (int index);
	
	GUITreeNode* operator[] (int index);


	GUIEventHandler _add;
	GUIEventHandler _clear;

	GUIEventHandler* t_OnAdd;  
	GUIEventHandler* t_OnClear;  

	void ReleaseChilds ();

	void Sort (bool (*FuncCompare)(GUITreeNode* const &a1, GUITreeNode* const &a2));

	void SortEx (bool (*FuncCompare)(GUITreeNode* const &a1, GUITreeNode* const &a2), int (*NeedToSortChilds)(GUITreeNode* const &a1));

	



};



//-----------------------------------------------------------

class GUITreeNode : public GUIControl
{

	friend class GUITreeNodes;

protected:

	GUITreeNodes* OwnerList;

  virtual ~GUITreeNode ();
	
	void GetFullNodeProcess (GUITreeNode* node);

public:


	const char* GetFullPath();

	bool bUseCustomColor;
	DWORD Color;
	bool CanDrag;
	bool CanDrop;
	bool CanCopy;
  void* Data;
	int Tag;
  bool Expanded;
	bool Selected;
	bool bReadOnly;
  
	virtual void Release ();
	GUIImage* Image;
	GUIImage* ExpandedImage;
	GUIImage* SelectedImage;

	GUIImage _Image;
	GUIImage _ExpandedImage;
	GUIImage _SelectedImage;


  GUITreeNode ();

  
  GUITreeNode* Parent;
  GUITreeNodes Childs;
  
  bool HasChildren ();
  
  int GetChildsCount ();
  
  //string Text;
	const char* GetText();
	void SetText(const char* value);

	void _cdecl OnStrAdd (GUIControl* sender);


	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUITreeNode") == 0) return true;
		return GUIControl::Is (className);
	}

protected:

	char text_data[TREE_VIEW_ITEM_LEN];
};

//-----------------------------------------------------------

#define MAX_IMAGES 9

class GUITreeView : public GUIControl
{
	
	
protected:


	bool NeedHorizScroll;
	


  int nTop;
	int nHeight;

	virtual bool ProcessMessages (GUIMessage message, DWORD lparam, DWORD hparam);

	GUITreeNode* sel_node;
	GUITreeNode* exp_node;
	float ClickTime;

	int printed_lines;
	int printed_width;

	int OffsetY;
	int OffsetX;

	IControls * ctrl;
	
	GUITreeNode* FindItemProcess (GUITreeNode* root, char* path);


	bool CenterItemProcess (GUITreeNodes& items, int PrintedLines);

	bool IsParentIs (GUITreeNode* NeedParent, GUITreeNode* Node);

	GUITreeNode* item_to_center;

	GUITreeNode* DragedNode;

	bool SystemMDown;


	GUITreeNode* NodeUnderCursor;
	bool CursorUnderNodeButton;
	float TimeFromLastPressed;

	bool NeedScrolToSelection;
	int SelectionDrawY;

	bool inDragInDrop;

	char _finishChar;
	
public:


	bool Flat;
	
	GUIImage* ImageForCopyIcon;

	GUITreeNode* CopyBranch (GUITreeNode* destination, GUITreeNode* from);

	GUIScrollBar* sbHoriz;
	GUIScrollBar* sbVert;


	void ResetSelection (GUITreeNodes& items);

	GUIImage* ImagesArray[MAX_IMAGES];
	void SetImagesArray (const char* filename);
	
  int DrawText (GUITreeNodes& items, int fromY, int formX, int level, DWORD BitMask);

	bool NeedHorizScrollBar (GUITreeNodes& items, int fromY, int formX, int level);

	GUITreeNodes* Items;
	
	DWORD FontColor;
	GUITreeView (GUIControl* pParent, int Left, int Top, int Width, int Height);
	virtual ~GUITreeView ();
	
	virtual void SetFinishingChar (char finishChar);

	
	virtual void Draw ();

	virtual bool OnMDown (int MouseKey, const GUIPoint& pt);

	int ProcessMDown (const GUIPoint& pt, GUITreeNodes& items, int fromY, int formX, int level);


	virtual GUITreeNode* GetSelectedNode ();
	virtual void SetSelectedNode (GUITreeNode* node, int NeedCenter = true);

	virtual GUITreeNode* GetExpandedOrCollapsedNode ();
	virtual void SetExpandedOrCollapsedNode (GUITreeNode* node);


	void MoveSelectionToDown ();
	void MoveSelectionToUp ();


	void ProcessKeyDown (int key);
	void UpdateScrollBars ();


	void _cdecl sbVertChange (GUIControl* sender);
	void _cdecl sbHorizChange (GUIControl* sender);

	
	GUIEventHandler* t_OnBeforeChange;  
	GUIEventHandler* t_OnChange;  

	GUIEventHandler* t_OnExpand;  
	GUIEventHandler* t_OnColapse;  

	GUIEventHandler* t_AfterDrop;  
	GUIEventHandler* t_BeforeDrop;  

	GUIEventHandler* t_OnDblClick;  

	GUIEventHandler* t_OnRightSelect;  

	void _cdecl ItemsBeforeClear (GUIControl* sender);


	GUIEventHandler* t_OnCopyNode;  

	GUITreeNode* FindItem (const char* path);

	GUIEventHandler* t_OnFind;  
	

	void CenterItem (GUITreeNode* item);

	virtual bool DragBegin ();	
	virtual bool DragOver (int MouseX, int MouseY, GUIControl* dragfrom);
	virtual void DragDrop (int MouseX, int MouseY, GUIControl* dragfrom);
	bool _cdecl CustomDraw (int mouseX, int mouseY);


	GUITreeNode* GetDragedItem ();


	virtual void MakeFlat (bool enable);


	virtual bool Is (const char* className)
	{
		if (strcmp(className, "GUITreeView") == 0) return true;
		return GUIControl::Is (className);
	}


	GUITreeNode* GetNodeAtPos (const GUIPoint& pt);


	int utilGetIndexInNodes (GUITreeNode* node, GUITreeNodes* nodes);


	bool BeforeDrop_CanDrop;


	void Sort (bool (*FuncCompare)(GUITreeNode* const &a1, GUITreeNode* const &a2));
	void SortEx (bool (*FuncCompare)(GUITreeNode* const &a1, GUITreeNode* const &a2), int (*NeedToSortChilds)(GUITreeNode* const &a1));



	void ChangeWidth(int newWidth);



};




#endif

#endif