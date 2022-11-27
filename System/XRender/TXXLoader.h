
#ifndef RENDER_TXX_TEXTURES_LOADER
#define RENDER_TXX_TEXTURES_LOADER

#include "TXX.h"
#include "Render.h"
#include "Textures/BaseTexture.h"
#include "Textures/Texture.h"


#ifndef _XBOX


//------------- загрузчик для PC

class IRead;

class TXXLoader
{
	string strFullPath;

public:

	struct SD_TEXFORMAT
	{
		TXXFormat	txFormat;
		D3DFORMAT	d3dFormat;
		bool		isSwizzled;
		dword		pitchDivider;
		const char	* pFormatName;
	};


private:

	bool bSkipDegradation;
	IFileService * pFS;
	NGRender* pRS;


	bool LoadBox(IDataFile * pRF, IBaseTexture * pTexture, RENDERLOCKED_BOX * pLB, const TxxFileHeader & TXHead, const SD_TEXFORMAT & TexFormat);
	bool LoadSurface(IDataFile * pRF, IBaseTexture * pTexture, dword dwFace, dword dwLevel, RENDERLOCKED_RECT * pLR, const TxxFileHeader & TXHead, const SD_TEXFORMAT & TexFormat);



	CDX8Texture* LoadTexture(IDataFile* file, const TxxFileHeader& head, SD_TEXFORMAT texFormat);


	CDX8Texture* CreateMipMapColoredTexture(TxxFileHeader TXHead, const SD_TEXFORMAT & TexFormat);



	static SD_TEXFORMAT TexFormats[];
	static SD_TEXFORMAT* GetFormat(TXXFormat txFormat);



public:

	TXXLoader(NGRender* r_srv);
	virtual ~TXXLoader();



	IBaseTexture* Load_TXX (const char* fileName, const char* sourceFile, long sourceLine);

	void IgnoreDegradation (bool bIgnore);


};

#else

//------------- загрузчик для X360

class TXXLoader
{

private:

	string strFullPath;


	IFileService * pFS;
	NGRender* pRS;

public:

	TXXLoader(NGRender* r_srv);
	virtual ~TXXLoader();

	IBaseTexture* Load_TXX (const char* fileName, const char* sourceFile, long sourceLine);

	void IgnoreDegradation (bool bIgnore)
	{

	}

};

#endif

#endif