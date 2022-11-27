#ifndef _XBOX
#ifndef GUI_IMAGE
#define GUI_IMAGE

#include "..\..\core.h"
#include "..\..\render.h"


class GUIImage
{
	
	IBaseTexture* image;
	
	char FileName[MAX_PATH];
public:
	
	GUIImage ();
	GUIImage (const char* name);
	~GUIImage ();
	
	void Load (const char* name);
	void DirectLoad (const char* name);
	bool DirectLoadTGA (const char* name);
	
	int GetWidth ();
	int GetHeight ();
	
	bool IsEmpty ();

	IBaseTexture* GetTexture () {return image;};

	const char* GetName () const;

	void Delete ();

	GUIImage & operator = (const GUIImage & r);
	
};


#endif


#endif