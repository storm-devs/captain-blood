#ifndef _XBOX

#include <stdio.h> 
#include <stdarg.h>
#include "gui_font.h"
#include "..\Common_h\FileService.h"




GUIFont::GUIFont (const char* FontName) : Chunks(_FL_)
{
	rs = (IRender*)api->GetService("DX9Render");

	rX = 0;
	rY = 0;
	
	sX = 0;
	sY = 0;
	
	pFont = NULL;
	SetName (FontName);
}

GUIFont::~GUIFont ()
{
	if (pFont) pFont->Release ();
	pFont = NULL;
}


void GUIFont::SetRect (int X, int Y, int sX, int sY)
{
	this->rX = X;
	this->rY = Y;
	
	this->sX = sX;
	this->sY = sY;
}

void GUIFont::CutTags (char* dest, const char* source)
{
	int l = strlen (source);
	int cpos = 0;
	for (int n =0 ; n < l; n++)
	{
		if ((source[n] == '#') && (source[n+1] == 'b'))
		{
			n++;
			continue;
		}
		if ((source[n] == '@') && (source[n+1] == 'b'))
		{
			n++;
			continue;
		}
		if ((source[n] == '@') && (source[n+1] == 'c'))
		{
			n++;
			continue;
		}
		if ((source[n] == '#') && (source[n+1] == 'c'))
		{
			n+=7;
			continue;
		}


		dest[cpos] = source[n];
		cpos++;

	}

	dest[cpos] = 0;
}


void _cdecl GUIFont::Print (int x, int y, unsigned long dwColor, const char * pFormat, ...)
{
	x-=rs->GetViewport().X;
	y-=rs->GetViewport().Y;
	if (!pFont) return;
	char cTmpBuffer[4096];
	
	va_list args;
	va_start(args, pFormat);
	crt_vsnprintf(cTmpBuffer, 4095, pFormat, args);
	va_end(args);

	Chunks.Empty();

	float curposx = (float)x;
	float curposy = (float)y;
	int beginfrom = 0;

	int total_len = strlen (cTmpBuffer);

	bool IsBold = false;
	bool CustomColor = false;
	DWORD Color = 0xFFFFFFFF;
	for (int n = 0; n < total_len; n++)
	{
		float char_height = pFont->GetHeight();

		if (cTmpBuffer[n] == '\n')
		{
			cTmpBuffer[n] = 0;
			TextChunk & tCh = Chunks[Chunks.Add()];
			tCh.Bold = IsBold;
			tCh.NewColor = CustomColor;
			tCh.Color = Color;
			tCh.x = curposx;
			tCh.y = curposy;
			tCh.Text = (cTmpBuffer + beginfrom);

			curposx = (float)x;
			curposy+=char_height;
			beginfrom = n+1;
		}


		if ((cTmpBuffer[n] == '#') && (cTmpBuffer[n+1] == 'b'))
		{
			cTmpBuffer[n] = 0;
			cTmpBuffer[n+1] = 0;
			TextChunk & tCh = Chunks[Chunks.Add()];			
			tCh.Bold = IsBold;
			tCh.NewColor = CustomColor;
			tCh.Color = Color;
			tCh.y = curposy;
			tCh.Text = (cTmpBuffer + beginfrom);
			tCh.x = curposx;
			float char_width  = pFont->GetLength("%s", tCh.Text);
			curposx += char_width;
			IsBold = true;
			n++;
			beginfrom = n+1;
		}
		
		if ((cTmpBuffer[n] == '@') && (cTmpBuffer[n+1] == 'b'))
		{
			cTmpBuffer[n] = 0;
			cTmpBuffer[n+1] = 0;
			TextChunk & tCh = Chunks[Chunks.Add()];
			tCh.Bold = IsBold;
			tCh.NewColor = CustomColor;
			tCh.Color = Color;
			tCh.y = curposy;
			tCh.Text = (cTmpBuffer + beginfrom);
			tCh.x = curposx;
			float char_width  = pFont->GetLength("%s", tCh.Text);
			curposx += char_width;
			IsBold = false;
			n++;
			beginfrom = n+1;
		}
		
		if ((cTmpBuffer[n] == '#') && (cTmpBuffer[n+1] == 'c'))
		{
			cTmpBuffer[n] = 0;
			cTmpBuffer[n+1] = 0;
			TextChunk & tCh = Chunks[Chunks.Add()];
			tCh.Bold = IsBold;
			tCh.Color = Color;
			tCh.NewColor = CustomColor;
			tCh.y = curposy;
			tCh.Text = (cTmpBuffer + beginfrom);
			tCh.x = curposx;
			float char_width  = pFont->GetLength("%s", tCh.Text);
			curposx += char_width;
			CustomColor = true;
			

			static char ColorCode[8];
			crt_strncpy (ColorCode, 8, (cTmpBuffer+n+2), 6);
			Color = strtol( ColorCode, NULL, 16);

			
			cTmpBuffer[n+7] = 0; // 6 - end string
			n+=7;
			beginfrom = n+1;
			
		}

		if ((cTmpBuffer[n] == '@') && (cTmpBuffer[n+1] == 'c'))
		{
			cTmpBuffer[n] = 0;
			cTmpBuffer[n+1] = 0;
			TextChunk & tCh = Chunks[Chunks.Add()];
			tCh.Bold = IsBold;
			tCh.Color = Color;
			tCh.NewColor = CustomColor;
			tCh.y = curposy;
			tCh.Text = (cTmpBuffer + beginfrom);
			tCh.x = curposx;
			float char_width  = pFont->GetLength("%s", tCh.Text);
			curposx += char_width;
			CustomColor = false;
			n++;
			beginfrom = n+1;
		}


		
		
	}

	// Добавляем последний кусочек...
	TextChunk & tCh = Chunks[Chunks.Add()];
	tCh.Bold = IsBold;
	tCh.NewColor = CustomColor;
	tCh.Color = Color;
	tCh.x = (float)curposx;
	tCh.y = (float)curposy;
	tCh.Text = (cTmpBuffer + beginfrom);
	
	pFont->SetColor (dwColor);
	//pFont->Print ((float)x, (float)y, "%s", cTmpBuffer);


	
	for (int i = 0; i < Chunks; i++)
	{
		TextChunk & tCh = Chunks[i];
		if (tCh.NewColor)
		{
			pFont->SetColor ((tCh.Color & 0xFFFFFF) | (dwColor & 0xFF000000));
		} else
		{
			pFont->SetColor (dwColor);
		}

		pFont->Print (tCh.x, tCh.y, "%s", tCh.Text);

		if (tCh.Bold)
		{
			pFont->Print (tCh.x+1, tCh.y, "%s", tCh.Text);
		}
	}
	
}

void _cdecl GUIFont::PrintWidth (int x, int y, int width, unsigned long dwColor, const char * pFormat, ...)
{
	if (!pFont) return;
	char cTmpBuffer[4096];
	
	va_list args;
	va_start(args, pFormat);
	crt_vsnprintf(cTmpBuffer, 4095, pFormat, args);
	va_end(args);
	
	
	int points = false;
	for (;;)
	{
		float tLen =  pFont->GetLength (cTmpBuffer);
		float tLen2 =  pFont->GetLength (". . .");
		if ((tLen+tLen2) <= width) break;
		
		points = true;
		int a = strlen (cTmpBuffer);
		
		if (a == 1) return;
		
		cTmpBuffer[a-1] = 0;
	}
	
	
	pFont->SetColor (dwColor);
	
	if (points)
	pFont->Print  ((float)x, (float)y, "%s. . .", cTmpBuffer);
	else
	pFont->Print  ((float)x, (float)y, cTmpBuffer);
	
	
	
}


int GUIFont::GetWidth (const char* string)
{
	static char Temp[8192];
	static char Temp2[8192];
	crt_strncpy (Temp, 8192, string, 8191);
	CutTags (Temp2, Temp);

	if (!pFont) return 0;
	float width =  pFont->GetLength ("%s", Temp2);

/*
	float width = 0.0f;
	int slen = strlen (string);
	for (int n = 0; n < slen; n++)
	{
		if (string[n] == '\n') continue;
		if (string[n] == '#' && string[n+1] == 'b') {n+=1; continue;}
		if (string[n] == '#' && string[n+1] == 'c') {n+=7; continue;}
		if (string[n] == '@' && string[n+1] == 'b') {n+=1; continue;}
		if (string[n] == '@' && string[n+1] == 'c') {n+=1; continue;}
		width += pFont->GetLength ("%c", string+n);
	}
*/

	if (width == 0) return 1;
	return (int)(width+0.5f);
}

int GUIFont::GetHeight (const char* string)
{
	if (!pFont) return 0;
	return (int)pFont->GetHeight("%s", string);
}

int GUIFont::GetHeight ()
{
	if (height == 0) return 1;
	return height;
}


void GUIFont::SetName (const char* name)
{
	IFileService* fs = (IFileService*)api->GetService("FileService");

	if (pFont) pFont->Release ();
	pFont = NULL;

	crt_strncpy (cFontName, 256, name, 255);
	IRender* rs = (IRender*)api->GetService("DX9Render");
	pFont = rs->CreateFont (cFontName);
	height = 0;
	cFontSize = 0;
	if(pFont)
	{
		cFontSize = pFont->GetHeight();
		height = (long)pFont->GetHeight();
		pFont->SetTechnique ("GUIFont");
	}
}

void GUIFont::SetSize (float size)
{
	if (!pFont) return;	
	cFontSize = size;
	height = (int)size;
	pFont->SetHeight (size);
}


#endif