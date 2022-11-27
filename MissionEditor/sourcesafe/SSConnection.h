#if !defined(_SSCONNECTION_H_INCLUDED_)
#define _SSCONNECTION_H_INCLUDED_


#include <atlbase.h>
#include <comdef.h>
#include <atlwin.h>
#include <assert.h>
#include "ssauto.h"


extern "C" const GUID __declspec(selectany) LIBID_SourceSafeTypeLib =
{0x783cd4e0,0x9d54,0x11cf,{0xb8,0xee,0x00,0x60,0x8c,0xc9,0xa7,0x1f}};
extern "C" const GUID __declspec(selectany) IID_IVSSItem =
{0x783cd4e1,0x9d54,0x11cf,{0xb8,0xee,0x00,0x60,0x8c,0xc9,0xa7,0x1f}};
extern "C" const GUID __declspec(selectany) IID_IVSSVersions =
{0x783cd4e7,0x9d54,0x11cf,{0xb8,0xee,0x00,0x60,0x8c,0xc9,0xa7,0x1f}};
extern "C" const GUID __declspec(selectany) IID_IVSSVersion =
{0x783cd4e8,0x9d54,0x11cf,{0xb8,0xee,0x00,0x60,0x8c,0xc9,0xa7,0x1f}};
extern "C" const GUID __declspec(selectany) IID_IVSSItems =
{0x783cd4e5,0x9d54,0x11cf,{0xb8,0xee,0x00,0x60,0x8c,0xc9,0xa7,0x1f}};
extern "C" const GUID __declspec(selectany) IID_IVSSCheckouts =
{0x8903a770,0xf55f,0x11cf,{0x92,0x27,0x00,0xaa,0x00,0xa1,0xeb,0x95}};
extern "C" const GUID __declspec(selectany) IID_IVSSCheckout =
{0x783cd4e6,0x9d54,0x11cf,{0xb8,0xee,0x00,0x60,0x8c,0xc9,0xa7,0x1f}};
extern "C" const GUID __declspec(selectany) IID_IVSSDatabase =
{0x783cd4e2,0x9d54,0x11cf,{0xb8,0xee,0x00,0x60,0x8c,0xc9,0xa7,0x1f}};
extern "C" const GUID __declspec(selectany) CLSID_VSSItem =
{0x783cd4e3,0x9d54,0x11cf,{0xb8,0xee,0x00,0x60,0x8c,0xc9,0xa7,0x1f}};
extern "C" const GUID __declspec(selectany) CLSID_VSSVersion =
{0x783cd4ec,0x9d54,0x11cf,{0xb8,0xee,0x00,0x60,0x8c,0xc9,0xa7,0x1f}};
extern "C" const GUID __declspec(selectany) CLSID_VSSDatabase =
{0x783cd4e4,0x9d54,0x11cf,{0xb8,0xee,0x00,0x60,0x8c,0xc9,0xa7,0x1f}};
extern "C" const GUID __declspec(selectany) IID_IVSSEvents =
{0x783cd4e9,0x9d54,0x11cf,{0xb8,0xee,0x00,0x60,0x8c,0xc9,0xa7,0x1f}};
extern "C" const GUID __declspec(selectany) IID_IVSS =
{0x783cd4eb,0x9d54,0x11cf,{0xb8,0xee,0x00,0x60,0x8c,0xc9,0xa7,0x1f}};
extern "C" const GUID __declspec(selectany) IID_IVSSEventHandler =
{0x783cd4ea,0x9d54,0x11cf,{0xb8,0xee,0x00,0x60,0x8c,0xc9,0xa7,0x1f}};


_COM_SMARTPTR_TYPEDEF(IVSSItem, IID_IVSSItem);
_COM_SMARTPTR_TYPEDEF(IVSSVersions, IID_IVSSVersions);
_COM_SMARTPTR_TYPEDEF(IVSSVersion, IID_IVSSVersion);
_COM_SMARTPTR_TYPEDEF(IVSSItems, IID_IVSSItems);
_COM_SMARTPTR_TYPEDEF(IVSSCheckouts, IID_IVSSCheckouts);
_COM_SMARTPTR_TYPEDEF(IVSSCheckout, IID_IVSSCheckout);
_COM_SMARTPTR_TYPEDEF(IVSSDatabase, IID_IVSSDatabase);
_COM_SMARTPTR_TYPEDEF(IVSSEvents,IID_IVSSEvents);
_COM_SMARTPTR_TYPEDEF(IVSS, IID_IVSS);
_COM_SMARTPTR_TYPEDEF(IVSSEventHandler, IID_IVSSEventHandler);



bool b_COM_AlreadyInited = false;


class CSSConnection
{
	bool bConnected;
	IVSSDatabasePtr mp_vssDatabase;

public:

	CSSConnection()
	{
		bConnected = false;
		mp_vssDatabase = NULL;

		if (!b_COM_AlreadyInited)
		{
			HRESULT hRes = ::CoInitialize(NULL);
			//assert (hRes == S_OK);

			b_COM_AlreadyInited = true;
		}
	}

	virtual ~CSSConnection()
	{
	}


	bool ConnectToDB(LPCTSTR psz_User, LPCTSTR psz_Password, LPCTSTR psz_VSSini=NULL)
	{
		CLSID clsid;
		IClassFactory *pClf;
		bConnected = false;
     
		try
		{

			HRESULT hres = CLSIDFromProgID(L"SourceSafe", &clsid );
			if (hres != S_OK) return false;
			hres = CoGetClassObject( clsid, CLSCTX_ALL, NULL, IID_IClassFactory, (void**)&pClf );
			if (hres != S_OK) return false;
			hres = pClf->CreateInstance( NULL, IID_IVSSDatabase, (void **) &mp_vssDatabase);
			if (hres != S_OK) return false;
			hres = mp_vssDatabase->Open((CComBSTR)psz_VSSini, (CComBSTR)psz_User, (CComBSTR)psz_Password);
			if (hres != S_OK) return false;

			bConnected = true;
			return true;
		}
		catch (...)
		{
		
		}

		bConnected = false;
		return false;
	}


	IVSSDatabasePtr GetSourceSafeDatabase ()
	{
		if (bConnected) return mp_vssDatabase;
		return NULL;
	}


};


#endif // _SSCONNECTION_H_INCLUDED_