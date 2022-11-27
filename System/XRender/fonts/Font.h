#ifndef CDX8FONT_HPP
#define CDX8FONT_HPP

#include "..\..\..\common_h\Render.h"
#include "..\..\..\common_h\templates\array.h"
#include "..\..\..\common_h\templates\stack.h"
#include "..\..\..\common_h\templates\string.h"
#include "..\..\..\common_h\templates\htable.h"
#include "..\..\..\common_h\templates\map.h"
#include "..\Resource.h"

class CDX8Fonts;

class RDX8Font
{
private:
	struct FONT_VERTEX
	{
		Vector	vPos;
		float	tu, tv;
	};
public:
	RDX8Font();
	~RDX8Font();

	dword AddRef();
	dword Release();
	void Create(const char * pFontName);
	const string & GetName() const { return sName; };
	IIBuffer * GetIndexBuffer() const;
	IVBuffer * PrintBuffer(float x, float y, float z, const char * pString, float fHeight, float kerning);
	dword GetNumPrimitives() const;
	dword GetNumVertices() const;
	IBaseTexture * GetTexture() const;
	float GetLength(float fHeight, float kerning, const char * pString) const;
	float GetOriginalHeight() const;

	static IRender * pRS;
private:
	struct symbol_t
	{
		float			fStartU, fStartV, fDU; 
	};

	string				sName;
	symbol_t			Symbols[256];
	IBaseTexture		* pFntTexture;
	dword				dwRefCount;
	dword				dwLastLen;
	float				fDV;
	float				fTexSizeX, fTexSizeY;
	float				fOriginalHeight;
	float fDefaultKerning;

	static IVBuffer		* pVBuffer;
	static IIBuffer		* pIBuffer;
	static dword		dwVBufferVertices, dwVBufferRefCount;

	bool			ReserveBuffer(dword dwVertices);
};

class CDX8Font : public IFont
{
public:
	CDX8Font(RDX8Font * pRFont, float fHeight, dword dwColor);
	virtual ~CDX8Font();

	virtual void		SetColor(dword dwColor);
	virtual void		SetHeight(float fHeight);
	virtual float		GetHeight() const;
	virtual float _cdecl GetHeight(const char * pFormatString, ...) const;
	virtual void		SetTechnique(const char * pTechniqueName);
	virtual void _cdecl	Print(float x, float y, const char * pFormatString, ...);
	virtual void _cdecl	Print(const Vector & vPos, float fViewDistance, float fLine, const char * pFormatString, ...);

	virtual float _cdecl	GetLength(const char * pFormatString, ...) const;
	virtual float _cdecl	GetLength(dword dwLength, const char * pFormatString, ...) const;

	virtual void SetKerning (float kerningValue);

	IBaseTexture* GetTexture();

	static IRender		* pRS;

private:
	static CDX8Fonts	* pDX8Fonts;

	RDX8Font			* pRFont;
	float				fHeight;
	dword				dwColor;
	ShaderId shaderID;
	float				fCurrentZ;
	float kerning_value;

	IVariable* pFontTexture;
	IVariable* pFontColor;

	DX8_RESOURCE_IMPLEMENT

	virtual bool	Release();
	virtual bool	ForceRelease();
};

class CDX8Fonts
{ 
public:
	CDX8Fonts(IRender *);
	virtual ~CDX8Fonts();

	IFont	* CreateFont(const char * pFontName, float fHeight, dword dwColor);
	bool	Release(IFont * pFont);

private:

	IRender				* pRS;
	array<CDX8Font*>	aCFonts;
	array<RDX8Font*>	aRFonts;

};

#endif