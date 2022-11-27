#include "InterfaceUtils.h"


CREATE_SERVICE(InterfaceUtils, 10000)


ICoreStorageLong * InterfaceUtils::largeshot = null;
ICoreStorageLong * InterfaceUtils::hideHUD = null;

InterfaceUtils::InterfaceUtils()
{
	if(!largeshot)
	{
		largeshot = api->Storage().GetItemLong("system.screenshot.Largeshot", _FL_);
		largeshot->Set(0);
	}
	if(!hideHUD)
	{
		hideHUD = api->Storage().GetItemLong("system.screenshot.hidegui", _FL_);
		hideHUD->Set(0);
	}	
}

InterfaceUtils::~InterfaceUtils()
{
	RELEASE(largeshot);
	RELEASE(hideHUD);
}

dword InterfaceUtils::WordWrapString(string &str, IFont *font, float outputPixelWidth)
{
	if( !font )
		return 0;

	dword linesCount = 1;
	float length = font->GetLength(str);

	if( length > outputPixelWidth )
	{
		float outputWidth = outputPixelWidth;

		dword potentialWrapPos = 0;	// возможное место переноса
		dword  prevRealWrapPos = 0;	// место где был сделан последний перенос

		dword strLength = str.Len();

		for( dword i = 0 ; i < strLength ; i++ )
		{
			if( str[i] == ' ' )
			{
				str.GetDataBuffer()[i] = 0;

				float length = font->GetLength(str.GetBuffer() + prevRealWrapPos);

				str.GetDataBuffer()[i] = ' ';

				if( length >= outputWidth )
				{
					if( potentialWrapPos == 0 )
						potentialWrapPos = i;

					linesCount++;

					str.GetDataBuffer()[potentialWrapPos] = '\n';

					prevRealWrapPos = potentialWrapPos;
				}

				potentialWrapPos = i;
			}
		}

		// обработка переноса последней строки
		float length = font->GetLength(str.GetBuffer() + prevRealWrapPos);

	//	if( length >= outputWidth )
		if( length >= outputWidth && (str.GetBuffer()[i] || potentialWrapPos))
		{
			if( potentialWrapPos == 0 )
				potentialWrapPos = i;

			linesCount++;

			str.GetDataBuffer()[potentialWrapPos] = '\n';
		}
	}

	str += "\n";

	return linesCount;
}

dword InterfaceUtils::WordWrapString(char text[], IFont *font, float outputPixelWidth, dword textMaxSize)
{
	if( !font )
		return 0;

	dword linesCount = 1;
	float length = font->GetLength(text);

	if( length > outputPixelWidth )
	{
		float outputWidth = outputPixelWidth;

		dword potentialWrapPos = 0;	// возможное место переноса
		dword  prevRealWrapPos = 0;	// место где был сделан последний перенос

		dword strLength = strlen(text);

		for( dword i = 0 ; i < strLength ; i++ )
		{
			if( text[i] == ' ' )
			{
				text[i] = 0;

				float length = font->GetLength(text + prevRealWrapPos);

				text[i] = ' ';

				if( length >= outputWidth )
				{
					if( potentialWrapPos == 0 )
						potentialWrapPos = i;

					linesCount++;

					text[potentialWrapPos] = '\n';

					prevRealWrapPos = potentialWrapPos;
				}

				potentialWrapPos = i;
			}
		}

		// обработка переноса последней строки
		float length = font->GetLength(text + prevRealWrapPos);

	//	if( length >= outputWidth )
		if( length >= outputWidth && (text[i] || potentialWrapPos))
		{
			if( potentialWrapPos == 0 )
				potentialWrapPos = i;

			linesCount++;

			text[potentialWrapPos] = '\n';
		}
	}

	dword strLength = strlen(text);

	Assert(strLength < textMaxSize - 1)

	text[strLength]		= '\n';
	text[strLength + 1] = 0;

	return linesCount;
}
