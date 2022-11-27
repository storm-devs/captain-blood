#ifndef BASE_ATTRIBUE
#define BASE_ATTRIBUE

#include "..\..\common_h\mission.h"
#include "..\..\common_h\gui.h"
#include "..\..\common_h\fileservice.h"
#include "..\missioneditor.h"
#include "..\textfile.h"
#include "..\..\common_h\tinyxml\tinyxml.h"

#include "..\NodesPool.h"



class GUIWindow;
class AttributeList;

class TextFile;

extern MissionEditor* sMission;

class BaseAttribute : public GUIControl
{

protected:
	const char* AttrHint;
  char Name[256];
  IMOParams::Type Type;
  bool IsLimit;

	void* MasterData;

public:


	bool RealTimeChanges;
	GUIWindow* pForm;


	virtual void SetMasterData (void* data)
	{
		MasterData = data;
	}

	virtual void* GetMasterData ()
	{
		return MasterData;
	}



	BaseAttribute () : GUIControl (NULL)
  {
	 AttrHint = NULL;
	 RealTimeChanges = false;
	 pForm = NULL;
	 MasterData = NULL;
   Name[0] = 0;
   IsLimit = false;
  };

protected:
  virtual ~BaseAttribute () {};
public:  

  bool GetIsLimit () const
  {
   return IsLimit;
  };
 
  void SetIsLimit (bool isLimit)
  {
   IsLimit = isLimit;
  }
  
  const char* GetName () const
  {
   return Name; 
  }
  
  void SetName (const char* name)
  {
	 crt_strncpy (Name, 255, name, 255);
  }
  
  
  IMOParams::Type GetType () const
  {
   return Type;
  }
  
  void SetType (IMOParams::Type type)
  {
   Type = type;  
  }
  
	/* показать окошко для редактирования... */
  virtual void PopupEdit (int pX, int pY) = 0;
  
  virtual void AddToWriter (MOPWriter& wrt) = 0;

	virtual void WriteToFile (IFile* pFile) = 0;

	virtual void LoadFromFile (IFile* pFile, const char* ClassName) = 0;


	/* Добавляем к ТриВиев*/
	virtual void Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v = NULL) = 0;

	virtual void UpdateTree(GUITreeNode * node, string * v = NULL) = 0;

	/* Вызываеться перед удалением из дерева */
	virtual void BeforeDelete () {};

	/* Скопировать парметры...*/
	virtual void Copy (BaseAttribute& dest, const BaseAttribute& from)
	{
		dest.SetName (from.GetName());
		dest.SetIsLimit (from.GetIsLimit());
		dest.RealTimeChanges = from.RealTimeChanges;
	}

	/* Вызываеться после загрузки с диска*/
	virtual void AfterLoad (BaseAttribute* baseattr, AttributeList* OriginalAttrList, const char* szClassName ) {};
	virtual void GroupHack (AttributeList* OriginalAttrList ) {};


	/* Начать изменение аттрибута в реалтайме... */
	virtual void BeginRTEdit (MissionEditor::tCreatedMO* MissionObject, GUIControl* parent) {};

	/* Подтвердить изменение аттрибута... */
	virtual void ApplyRTEdit () {};

	/* Отменить изменения в аттрибуте... */
	virtual void CancelRTEdit () {};

	/* Закончить редактирование... */
	virtual void CloseRTEdit () {};


	virtual bool IsSupportRTEdit () {return false;};

	virtual bool NeedApplyCancelButtonInRT ()  {return true;};


	void SetHint (const char* Hint)
	{
		AttrHint = Hint;
	}

	const char* GetHint ()
	{
		return AttrHint;
	}

	virtual void WriteToXML (TextFile &file, int level) = 0;
	virtual void ReadXML (TiXmlElement* Root, const char* szMasterClass) = 0;
};


#endif