#ifndef DX8RESOURCE_HPP
#define DX8RESOURCE_HPP

#include "..\..\common_h\Render.h"
#include "..\..\common_h\templates\array.h"
//#include "..\..\common_h\templates\string.h"

class CDX8Resource : public IResource
{
public:
	CDX8Resource(DX8RESOURCETYPE _Type, dword _dwSize);
	virtual ~CDX8Resource();

	virtual bool				Release();
	virtual bool				ForceRelease();
	virtual void				AddRef()
	{
		iRefCount++;
	}
	virtual long				GetRef() { return iRefCount; }
	virtual void				SetRef(long iRefCount) { this->iRefCount = iRefCount; }

	virtual DX8RESOURCETYPE		GetType() const { return Type; }
	virtual dword				GetSize() const { return dwSize; }

	virtual void				SetType(DX8RESOURCETYPE _Type)	{ Type = _Type; }
	virtual void				SetSize(dword _dwSize)			{ dwSize = _dwSize; }

	virtual bool				IsLoaded() { return iLoadRef == 0; }
	virtual void				AddLoadRef();
	virtual void				DecLoadRef();

	virtual bool				IsError() { return bError; }
	virtual void				SetError(bool bError) { this->bError = bError; }

#ifndef STOP_DEBUG
	virtual void				SetFileLine(const char * pFileName, long iLine) 
	{ 
		this->FileName = pFileName;
		this->iLine = iLine;
	}; 
	virtual const char *		GetFileName() const { return FileName; }
	virtual long				GetFileLine() const { return iLine; }
#else
	virtual void				SetFileLine(const char * pFileName, long iLine) { }
	virtual const char *		GetFileName() const { return ""; }
	virtual long				GetFileLine() const { return -1; }
#endif

private:
	DX8RESOURCETYPE		Type;
	dword				dwSize;
	long				iRefCount;
	bool				bLoaded, bError;
	long				iLoadRef;

#ifndef STOP_DEBUG
	const char *		FileName;
	long				iLine;
#endif
};

// ================================================================================================
#define DX8_RESOURCE_IMPLEMENT																	\
public: \
		virtual void				AddRef()		{ Resource.AddRef(); };						\
		virtual DX8RESOURCETYPE		GetType() const { return Resource.GetType(); };				\
		virtual dword				GetSize() const { return Resource.GetSize(); };				\
		virtual void				SetSize(dword _dwSize)	{ Resource.SetSize(_dwSize); };		\
		virtual long				GetRef() { return Resource.GetRef(); };						\
		virtual void				SetRef(long iRefCount) { Resource.SetRef(iRefCount); };		\
		virtual void				SetFileLine(const char * pFileName, long iLine) { Resource.SetFileLine(pFileName, iLine); };	\
		virtual const char *		GetFileName() const { return Resource.GetFileName(); };		\
		virtual long				GetFileLine() const { return Resource.GetFileLine(); };		\
		virtual bool				IsLoaded() { return Resource.IsLoaded(); };					\
		virtual bool				IsError() { return Resource.IsError(); };					\
		virtual void				SetError(bool bError) { Resource.SetError(bError); };		\
		virtual void				AddLoadRef() { Resource.AddLoadRef(); }						\
		virtual void				DecLoadRef() { Resource.DecLoadRef(); }						\
public:																							\
		CDX8Resource	Resource;										

#endif