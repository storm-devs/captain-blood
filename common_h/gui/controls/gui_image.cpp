#ifndef _XBOX

#include "gui_image.h"



GUIImage & GUIImage::operator = (const GUIImage & r)
{
	Load(r.GetName());
	return *this;
}

GUIImage::GUIImage ()
{
	FileName[0] = 0;
	image = NULL;
}

GUIImage::GUIImage (const char* name)
{
	memset (FileName, 0, MAX_PATH);
	image = NULL;
	Load (name);
}


GUIImage::~GUIImage ()
{
	if (image) image->Release ();
}

void GUIImage::Delete ()
{
	if (image) image->Release ();
	image = NULL;
}

void GUIImage::Load (const char* name)
{
	if (string::Len(name) == 0) return;

	memset (FileName, 0, MAX_PATH);
	crt_strncpy (FileName, MAX_PATH, name, MAX_PATH - 1);

	if (image) image->Release ();
	
	IRender* rs = (IRender*)api->GetService("DX9Render");
	image = rs->CreateTexture (_FL_, "GUI\\%s", name);
}

void GUIImage::DirectLoad (const char* name)
{
	memset (FileName, 0, MAX_PATH);
	crt_strncpy (FileName, MAX_PATH, name, MAX_PATH - 1);

	if (image) image->Release ();

	IRender* rs = (IRender*)api->GetService("DX9Render");
	image = rs->CreateTexture (_FL_, "%s", name);
}

bool GUIImage::DirectLoadTGA (const char* name)
{
	memset (FileName, 0, MAX_PATH);
	crt_strncpy (FileName, MAX_PATH, name, MAX_PATH - 1);

	if (image) image->Release ();

	IRender* rs = (IRender*)api->GetService("DX9Render");
	image = rs->CreateTextureUseD3DX (_FL_, "%s.tga", name);

	if (!image) return false;

	return true;
}



int GUIImage::GetWidth ()
{
	if (!image) return 0;
	return image->GetWidth ();
}

int GUIImage::GetHeight ()
{
	if (!image) return 0;
	return image->GetHeight ();
	
}


bool GUIImage::IsEmpty ()
{
	if (image == NULL) return true;
	return false;
}

const char* GUIImage::GetName () const
{
	return FileName;
}


#endif