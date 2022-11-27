#include <stdio.h>
#include <string.h>
#include "strutil.h"
#include "..\..\common_h\Storm3Crt.h"


char TBuffer[2048];


char* IntToStr (int val)
{
	crt_snprintf (TBuffer, 2000, "%d", val);
	return TBuffer;
}


char* FloatToStr (float val)
{
  crt_snprintf (TBuffer, 2000, "%3.4f", val);

	int y = strlen (TBuffer);
	for (int n = (y-1); n >= 0; n--)
	{
		if (n < (y-1))
		{
			if (TBuffer[n] != '0')
			{
				TBuffer[n+1] = 0;
				if (TBuffer[n] == '.')
				{
//					TBuffer[n] = '0';
					TBuffer[n] = 0;
				}
				return TBuffer;
			}
		}
	}
	return TBuffer;
}
