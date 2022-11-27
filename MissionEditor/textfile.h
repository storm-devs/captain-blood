#ifndef TEXT_FILE_WRITER
#define TEXT_FILE_WRITER

#include "zlib\zipme.h"


#define SaveFileAsZip true

class TextFile
{
public:

	TextFile(const char* filename) : data(_FL_, 65536*32)
	{
		ptr = 0;
		fileName = filename;
	}

	~TextFile()
	{
		bool isUseZip = SaveFileAsZip;
		if(ptr > 0)
		{
			ZipMe::WriteFile(fileName.c_str(), data.GetBuffer(), ptr, isUseZip);
		}		
	}

	void Write (int tabs, const char * format, ...)
	{
		tabs *= 2;
		va_list args;
		va_start(args, format);
		long size = data.Size() - ptr;
		Assert(size >= 0);
		if(size < 16384 + tabs)
		{
			data.AddElements(65536*4 + tabs);
		}
		for (int i = 0; i < tabs; i++, ptr++)
		{
			data[ptr] = ' ';
		}
		int result = crt_vsnprintf((char *)&data[ptr], 16384, format, args);
		va_end(args);
		if(result < 0)
		{
			ptr += 16384;
		}else{
			ptr += result;
		}
	}

private:
	array<byte> data;
	dword ptr;
	string fileName;
};


#endif