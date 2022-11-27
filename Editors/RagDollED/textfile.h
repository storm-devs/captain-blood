#pragma once

class TextFile
{
	char OutBuffer[16384]; FILE *fp;

public:

	TextFile(const char *filename)
	{
		fp = crt_fopen(filename,"w");
	}

	~TextFile()
	{
		if( fp )
			fclose (fp);
	}
  
	void Write(int tabs, const char *format, ...)
	{
		if( fp )
		{
			va_list args;
			va_start(args,format);

			crt_vsnprintf(OutBuffer,sizeof(OutBuffer) - 4,format,args);

			va_end(args);
	   
			for (int i = 0; i < tabs; i++)
			{
				fprintf(fp,"   ");
			}

			fprintf(fp,"%s",OutBuffer);
		}
	}

};
