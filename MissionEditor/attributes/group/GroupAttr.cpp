#include "GroupAttr.h"
#include "..\strutil.h"

#include "..\bool\BoolAttr.h"
#include "..\long\LongAttr.h"
#include "..\float\FloatAttr.h"
#include "..\position\positionAttr.h"
#include "..\Rotation\RotationAttr.h"
#include "..\String\StringAttr.h"
#include "..\Color\ColorAttr.h"
#include "..\Enumerate\EnumAttr.h"
#include "..\attrcreator.h"

#include "..\..\missioneditor.h"

#include "..\..\forms\globalParams.h"


GroupAttribute& GroupAttribute::operator = (const GroupAttribute & source)
{
	GroupName = source.GroupName;
	SetName(GroupName.c_str());

	Childs.DelAll();

	for (DWORD i = 0; i < source.Childs.Size(); i++)
	{
		BaseAttribute* GroupParam = source.Childs[i];

		BaseAttribute* pAttr = CreateAttributeCopy(GroupParam);

		Childs.Add(pAttr);
	}

	return *this;
}

GroupAttribute& GroupAttribute::operator = (const IMOParams::Group& source)
{
	GroupName = source.name;
	SetName(GroupName.c_str());


	Childs.DelAll();

	for (DWORD i = 0; i < source.element.Size(); i++)
	{
		IMOParams::Param* GroupParam = source.element[i];

		BaseAttribute* pAttr = CreateFromMissionParam(GroupParam);

		Childs.Add(pAttr);
	}
	

	return *this;
}


GroupAttribute::GroupAttribute () : Childs (_FL_, 64)
{
	RootNode = NULL;
	bExpanded = false;
	Type = IMOParams::t_group;
}

GroupAttribute::~GroupAttribute ()
{
	for (DWORD j = 0; j < Childs.Size(); j++)
	{
		DeleteAttribute(Childs[j]);
		//delete ;
	}

	

}

void GroupAttribute::AddToWriter (MOPWriter& wrt)
{
	for (DWORD i = 0; i < Childs.Size(); i++)
	{
		Childs[i]->AddToWriter(wrt);
	}
}

void GroupAttribute::UpdateTree(GUITreeNode * node, string * v)
{
	gp->__tmpText = GroupName;
	gp->__tmpText += "#c808080";


	node->Image->Load("folder");
	node->Tag = TAG_ATTRIBUTE;
	node->CanDrag = false;
	node->CanDrop = false;
	node->CanCopy = false;
	node->Data = this;
	//nNode->Text = gp->__tmpText;
	node->SetText(gp->__tmpText);
	node->Expanded = bExpanded;

	RootNode = node;
}

void GroupAttribute::Add2Tree (GUITreeNodes* nodes, TreeNodesPool* nodesPool, string * v)
{
	GUITreeNode* nNode = nodesPool->CreateNode();

	UpdateTree(nNode);

	nodes->Add(nNode);

	


	for (DWORD i = 0; i < Childs.Size(); i++)
	{
		Childs[i]->Add2Tree(&nNode->Childs, nodesPool);
	}
	
}


void GroupAttribute::SetMasterData (void* data)
{
	for (DWORD i = 0; i < Childs.Size(); i++)
	{
		Childs[i]->SetMasterData(data);
	}

}


void GroupAttribute::BeforeDelete ()
{
	if (RootNode)
	{
		bExpanded = RootNode->Expanded;
	}

	

	for (DWORD i = 0; i < Childs.Size(); i++)
	{
		Childs[i]->BeforeDelete();
	}
}

void GroupAttribute::WriteToFile (IFile* pFile)
{
	DWORD written;
	for (DWORD i = 0; i < Childs.Size(); i++)
	{
		IMOParams::Type t = Childs[i]->GetType ();
		DWORD dwType = (DWORD)t;

		written = pFile->Write(&dwType, sizeof (DWORD));
		Assert (written == sizeof (DWORD));


		Childs[i]->WriteToFile(pFile);
	}


}

void GroupAttribute::GroupHack (AttributeList* OriginalAttrList )
{
	for (DWORD i = 0; i < Childs.Size(); i++)
	{
		const char* pAttrName = Childs[i]->GetName();
		IMOParams::Type type = Childs[i]->GetType();

		BaseAttribute* pAttr = OriginalAttrList->FindInAttrList(pAttrName, type);

		if (pAttr)
		{
			CreateAttributeCopy(pAttr, Childs[i]);
		}

		if (Childs[i]->GetType() == IMOParams::t_group)
		{
			Childs[i]->GroupHack(OriginalAttrList );
		}
	}
}


void GroupAttribute::DebugLog (int deep)
{
	string mar;
	for (int q = 0; q <= deep; q++)
	{
		mar += "   ";
	}


	for (dword j = 0; j < Childs.Size(); j++)
	{
		IMOParams::Type t = Childs[j]->GetType();

		api->Trace("%s%s - %s", mar.c_str(), Childs[j]->GetName(), AttributeList::GetTextType(t));

		if (t == IMOParams::t_array)
		{
			ArrayAttribute* pArray = (ArrayAttribute*)Childs[j];
			pArray->DebugLog (deep+1);

		}

		if (t == IMOParams::t_group)
		{
			GroupAttribute* pGroup = (GroupAttribute*)Childs[j];
			pGroup->DebugLog (deep+1);

		}
	}

}


void GroupAttribute::GenerateFlatList(array<BaseAttribute*> &FlatList)
{
	for (dword j = 0; j < Childs.Size(); j++)
	{
		IMOParams::Type t = Childs[j]->GetType();

		if (t == IMOParams::t_group)
		{
			GroupAttribute* pGroup = (GroupAttribute*)Childs[j];
			pGroup->GenerateFlatList (FlatList);
			continue;
		}

		FlatList.Add(Childs[j]);


	}

}


void GroupAttribute::WriteToXML (TextFile &file, int level)
{
	for (DWORD i = 0; i < Childs.Size(); i++)
	{
		Childs[i]->WriteToXML(file, level);
	}

	//file.Write(level, "<enum val = \"%s\">\n", GetName ());
	//file.Write(level+1, "<selected val = \"s\" />\n", curvalue.c_str());
	//file.Write(level, "</enum>\n")
}

void GroupAttribute::ReadXML (TiXmlElement* Root, const char* szMasterClass)
{
}