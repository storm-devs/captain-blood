#ifndef DX8_IRESOURCE_HPP
#define DX8_IRESOURCE_HPP

enum DX8RESOURCETYPE
{
	DX8TYPE_UNKNOWN	= 0,
	DX8TYPE_SURFACE,
	DX8TYPE_VOLUME,
	DX8TYPE_TEXTURE,
/*
	DX8TYPE_ANIMTEXTURE,
	DX8TYPE_VOLUMETEXTURE,
	DX8TYPE_CUBETEXTURE,
*/
	DX8TYPE_VERTEXBUFFER,
	DX8TYPE_INDEXBUFFER,
	DX8TYPE_LIGHT,
	DX8TYPE_FONT,
	DX8TYPE_ADVANCED_FONT,

	DX8TYPE_PARAMETERS,
	DX8TYPE_DEPTHSTENCIL,
	DX8TYPE_ALREADYRELEASED,

	DX8TYPE_FORCE_DWORD	= 0x7fffffff
};

class IResource
{
protected:
	IResource() {};
	virtual ~IResource() {};

public:

	virtual long GetRef() = 0;
	virtual void AddRef() = 0;
	virtual bool Release() = 0;
	virtual bool ForceRelease() = 0;

	// release inner resource object
	virtual bool Reset() { return false; };
	// recreate inner resource object
	virtual bool Recreate() { return false; };
	// return true if resource is loaded
	virtual bool IsLoaded() { return false; };
	// return true if resource can't load or not created
	virtual bool IsError() { return false; };
	/*// return true if resource lost after reset device
	virtual bool IsLost() { return false; }
	// return true if resource lost after reset device
	virtual void DropLost() { return false; }*/

	virtual DX8RESOURCETYPE		GetType() const = 0;
	virtual dword				GetSize() const = 0;

	virtual const char *		GetFileName() const = 0;
	virtual long				GetFileLine() const = 0;
};

#endif