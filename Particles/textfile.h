#ifndef TEXT_FILE_WRITER
#define TEXT_FILE_WRITER


class TextFile
{

  char OutBuffer[16384];
	FILE* fp;

public:

  TextFile(const char* filename)
  {
   fp = crt_fopen( filename, "w" );
  }
  
  ~TextFile()
  {
	 if (!fp) return;
   fclose (fp);
  }
  
  void Write (int tabs, const char * format, ...)
  {
	 if (!fp) return;
 	 va_list args;
	 va_start(args, format);
	 crt_vsnprintf(OutBuffer, sizeof(OutBuffer) - 4, format, args);
   va_end(args);
   
   for (int i = 0; i < tabs; i++)
   {
    fprintf(fp, "  ");
   }
   
   fprintf(fp, "%s", OutBuffer);
  }
  
  
};


#endif