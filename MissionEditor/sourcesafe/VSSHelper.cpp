/*

#include "ssconnection.h"
#include "..\..\common_h\core.h"
#include "VSSHelper.h"

LPCTSTR gsz_root = _T("$/");


struct HackedData
{
	IVSSItemPtr vss_item;
};


HackedData* VSSAttachHackedData(SourceSafeItem* pItem)
{
	if (pItem->HidedVSS)
	{
		HackedData* pHackedData = (HackedData*)pItem->HidedVSS;
		delete pHackedData;
		pItem->HidedVSS = NULL;
	}

	pItem->HidedVSS = NEW HackedData;

	HackedData* pHackedData = (HackedData*)pItem->HidedVSS;
	return pHackedData;
}



void AddVSSItemRecursive (SourceSafeItem* pItem, SourceSafeItem* pParent, IVSSItemPtr vss_item, int level)
{
	

	CComBSTR str_name;
	HRESULT hr = vss_item->get_Name(&str_name);
	assert (hr == S_OK);
	CW2CT szMyString( str_name );
	pItem->Name = "$/";
	pItem->Name = (LPSTR)szMyString;

	HackedData* pHacked = VSSAttachHackedData(pItem);
	pHacked->vss_item = vss_item;

	if (pParent == NULL)
	{
		pItem->Name = "$/";
	}

	//api->Trace("SS : '%s'", pItem->Name.c_str());

	int i_type;
	vss_item->get_Type(&i_type);
	if (i_type == VSSITEM_PROJECT) pItem->Type = SourceSafeItem::VSS_PROJECT;
	if (i_type == VSSITEM_FILE) pItem->Type = SourceSafeItem::VSS_FILE;

	long l_checkOut;
	vss_item->get_IsCheckedOut(&l_checkOut);

	if (l_checkOut == VSSFILE_NOTCHECKEDOUT)
	{
		pItem->bCheckedOut = false;
	} else
	{
		pItem->bCheckedOut = true;
	}

	pItem->pParent = pParent;


	//recursive...
	if (i_type == VSSITEM_PROJECT)
	{
		long l_count;
		IVSSItemsPtr vss_items;
		vss_item->get_Items(_variant_t(false), &vss_items);
		vss_items->get_Count(&l_count);

		for (long i = 0; i < l_count; i++)
		{
			IVSSItemPtr vss_ChildItem;
			vss_items->get_Item(_variant_t(i+1L), &vss_ChildItem);


			//Дети рута...
			if (level == 0)
			{
				CComBSTR tmp_str_name;
				hr = vss_ChildItem->get_Name(&tmp_str_name);
				CW2CT tmp_szMyString( tmp_str_name );
				string temp_name = "$/";
				temp_name = (LPSTR)tmp_szMyString;

				if (temp_name != "MissionsSRC") continue;

			}

			SourceSafeItem* pNewItem = &pItem->Childs[pItem->Childs.Add()];
			AddVSSItemRecursive(pNewItem, pItem, vss_ChildItem, level+1);
		}
	}


}



SourceSafeItem::SourceSafeItem() : Childs (_FL_)
{
	HidedVSS = NULL;

	pParent = NULL;
	Type = VSS_UNKNOWN;
	bCheckedOut = false;
}

SourceSafeItem::~SourceSafeItem()
{
	if (HidedVSS != NULL)
	{
		HackedData* pHackedData = (HackedData*)HidedVSS;
		delete pHackedData;
		HidedVSS = NULL;
	}
}

void SourceSafeItem::Clear ()
{
	for (dword i = 0; i < Childs.Size(); i++)
	{
		Childs[i].Clear();
	}
	Childs.DelAll();
}

bool SourceSafeItem::AddToProject (const char* szFullPathName)
{
	if (Type != VSS_PROJECT) return false;

	CComBSTR bstr_comment("");
	CComBSTR bstr_localSpec(szFullPathName);

	IVSSItemPtr vss_newItem;

	HackedData* pHackedData = (HackedData*)HidedVSS;
	HRESULT hr = pHackedData->vss_item->Add(bstr_localSpec, bstr_comment, 0, &vss_newItem);
	if (hr != S_OK) return false;


	SourceSafeItem* pNewItem = &Childs[Childs.Add()];

	AddVSSItemRecursive(pNewItem, this, vss_newItem, 0);



	return true;

}

string SourceSafeItem::WhoCheckedOut()
{
	HackedData* pHackedData = (HackedData*)HidedVSS;

	string str_user("None");


	IVSSCheckoutsPtr checkOuts;
	pHackedData->vss_item->get_Checkouts(&checkOuts);

	IVSSCheckoutPtr vss_checkOutItem;
	checkOuts->get_Item(_variant_t(1L), &vss_checkOutItem);
	CComBSTR str_userName;

	try
	{
		HRESULT hr = vss_checkOutItem->get_Username(&str_userName);

		if (hr == S_OK)
		{
			CW2CT szMyString( str_userName );
			str_user = (LPSTR)szMyString;
		}
	}

	catch (...)
	{
	}

	return str_user;
}


bool SourceSafeItem::UndoCheckout (const char* szLocalPath)
{
	string fullname;
	fullname = szLocalPath;
	if (fullname[fullname.Size()-1] != '\\')
	{
		fullname+= "\\";
	}

	fullname+= Name.c_str();


	CComBSTR bstr_comment("");
	CComBSTR bstr_localSpec(fullname.c_str());
	CComBSTR bstr_localSpec_real(szLocalPath);

	HackedData* pHackedData = (HackedData*)HidedVSS;
	HRESULT hr = pHackedData->vss_item->get_LocalSpec(&bstr_localSpec_real);
	if (hr != S_OK) return false;


	hr = pHackedData->vss_item->UndoCheckout(bstr_localSpec, 0);

	if (hr != S_OK) return false;

	return true;
}

int SourceSafeItem::GetChildIndex (const char* szChildName)
{
	for (dword i = 0; i < Childs.Size(); i++)
	{
		if (Childs[i].Name == szChildName) return i;
	}

	return -1;
}

bool SourceSafeItem::GetLatestVersion (const char* szLocalPath)
{
	string fullname;
	fullname = szLocalPath;
	if (fullname[fullname.Size()-1] != '\\')
	{
		fullname+= "\\";
	}

	fullname+= Name.c_str();


	CComBSTR bstr_comment("");
	CComBSTR bstr_localSpec(fullname.c_str());
	CComBSTR bstr_localSpec_real(szLocalPath);

	HackedData* pHackedData = (HackedData*)HidedVSS;
	HRESULT hr = pHackedData->vss_item->get_LocalSpec(&bstr_localSpec_real);
	if (hr != S_OK) return false;


	hr = pHackedData->vss_item->Get(&bstr_localSpec, 0);

	if (hr != S_OK) return false;

	return true;
}


bool SourceSafeItem::Checkin (const char* szLocalPath)
{
	string fullname;
	fullname = szLocalPath;
	if (fullname[fullname.Size()-1] != '\\')
	{
		fullname+= "\\";
	}

	fullname+= Name.c_str();


	CComBSTR bstr_comment("");
	CComBSTR bstr_localSpec(fullname.c_str());
	CComBSTR bstr_localSpec_real(szLocalPath);

	HackedData* pHackedData = (HackedData*)HidedVSS;
	HRESULT hr = pHackedData->vss_item->get_LocalSpec(&bstr_localSpec_real);
	if (hr != S_OK) return false;


	hr = pHackedData->vss_item->Checkin(bstr_comment, bstr_localSpec, 0);

	if (hr != S_OK) return false;

	return true;
}


bool SourceSafeItem::Checkout (const char* szLocalPath)
{
	string fullname;
	fullname = szLocalPath;
	if (fullname[fullname.Size()-1] != '\\')
	{
		fullname+= "\\";
	}

	fullname+= Name.c_str();
	

	CComBSTR bstr_comment("");
	CComBSTR bstr_localSpec(fullname.c_str());
	CComBSTR bstr_localSpec_real(szLocalPath);

	HackedData* pHackedData = (HackedData*)HidedVSS;
	HRESULT hr = pHackedData->vss_item->get_LocalSpec(&bstr_localSpec_real);
	if (hr != S_OK) return false;

	hr = pHackedData->vss_item->Checkout(bstr_comment, bstr_localSpec, 0);

	if (hr != S_OK) return false;

	return true;
}



VSSAcess::VSSAcess()
{
}

VSSAcess::~VSSAcess()
{
}

bool VSSAcess::VSSEnumDatabase (SourceSafeItem& root, const char* szUserName)
{
	root.Clear ();

	CSSConnection VSS;
	bool bResult = VSS.ConnectToDB(szUserName, "", "\\\\sourceserver\\Sources\\Blood_src\\srcsafe.ini");

	if (bResult)
	{
		api->Trace("Connect to sourcesafe...done");
	} else
	{
		api->Trace("Connect to sourcesafe...error !");
	}
	
	if (bResult == false) return false;

	IVSSDatabasePtr pVssDB = VSS.GetSourceSafeDatabase();
	if (pVssDB)
	{
		IVSSItemPtr vssRootItem;
		pVssDB->get_VSSItem(CComBSTR(gsz_root), FALSE, &vssRootItem);

		AddVSSItemRecursive(&root, NULL, vssRootItem, 0);
	}

	return true;
}


*/