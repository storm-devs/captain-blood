#ifndef _XBOX
#ifndef GUI_FONT
#define GUI_FONT

#include "..\..\render.h"
#include "..\..\core.h"

class IFont;

class GUIFont
{
	struct TextChunk
	{
		bool Bold;
		bool NewColor;
		DWORD Color;
		float x;
		float y;
		char *Text;
	};

	array<TextChunk> Chunks;

	IRender* rs;
	int rX;
	int rY;
	
	int sX;
	int sY;
	
	int height;
	
	char cFontName[256];
	float cFontSize;
	
	
	IFont* pFont;
	
public:

	void CutTags (char* dest, const char* source);
	
	GUIFont (const char* FontName);
	~GUIFont ();
	
	void SetRect (int X, int Y, int sX, int sY);
	
	void _cdecl Print (int x, int y, unsigned long dwColor, const char * pFormat, ...);
	
	void _cdecl PrintWidth (int x, int y, int width, unsigned long dwColor, const char * pFormat, ...);
	
	
	int GetWidth (const char* string);
	
	int GetHeight ();
	int GetHeight (const char* string);
	
	
	void SetName (const char* name);
	
	void SetSize (float size);
	
};



#endif

#endif