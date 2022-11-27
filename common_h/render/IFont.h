#ifndef DX8_IFONT_HPP
#define DX8_IFONT_HPP

#include "IResource.h"

class IFont : public IResource
{
protected:
	IFont() {};
	virtual ~IFont() {};

public:
	virtual void			SetColor(dword dwColor) = 0;
	virtual void			SetHeight(float fHeight) = 0;
	virtual float			GetHeight() const = 0;
	virtual void			SetTechnique(const char * pTechniqueName) = 0;
	virtual float _cdecl	GetLength(const char * pFormatString, ...) const = 0;
	virtual float _cdecl	GetLength(dword dwLength, const char * pFormatString, ...) const = 0;

	// 2d print 
	virtual void _cdecl		Print(float x, float y, const char * pFormatString, ...) = 0;
	// 
	virtual void _cdecl		Print(const Vector & vPos, float fViewDistance, float fLine, const char * pFormatString, ...) = 0;
	virtual float _cdecl	GetHeight(const char * pFormatString, ...) const = 0;

	virtual void SetKerning (float kerningValue) = 0;
};

#endif