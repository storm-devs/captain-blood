

#include "zipme.h"
#include "zlib.h"



ZipMe::ZipMe() : data(_FL_)
{
}

ZipMe::~ZipMe()
{
	data.Empty();
}

//Записать файл с архивацией, добавляеться последний символ в конец имени 'z'
bool ZipMe::WriteFile(const char * fileName, const void * data, dword dataSize, bool isCompress)
{
	if(!data || !dataSize) return false;
	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	if(isCompress)
	{
		string zipName(fileName);
		for(const char * c = fileName; *c; c++);
		if(c[-1] != 'z' && c[-1] != 'Z')
		{
			zipName += 'z';
		}	
		fs->Delete(zipName.c_str());
		gzFile file = gzopen(zipName.c_str(), "w+b");
		if(!file) return false;
		gzsetparams(file, 2, Z_DEFAULT_STRATEGY);
		if(gzwrite(file, data, dataSize) != dataSize)
		{
			gzclose(file);
			fs->Delete(zipName.c_str());
			return false;
		}
		gzclose(file);
	}else{
		if(!fs->SaveData(fileName, data, dataSize))
		{
			return false;
		}
	}
	return true;
}

//Прочитать файл в память, если последний яимвол 'z' то подрузамеваеться что файл с компрессией
bool ZipMe::ReadFile(const char * fileName)
{
	if(!fileName || !fileName[0]) return false;
	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	//Сначала пробуем открыть zip файл, тк он по определению более новый
	string zipName = fileName;
	if(zipName.Last() != 'z' && zipName.Last() != 'Z')
	{
		zipName += 'z';
	}
	gzFile zipFile = gzopen(fileName, "rb");
	if(!zipFile)
	{
		zipName.Delete(zipName.Len() - 1, 1);
		IFile * file = fs->OpenFile(zipName, file_open_existing_for_read, _FL_);
		if(!file)
		{
			return false;
		}
		if(file->Size() > 0)
		{
			data.AddElements(file->Size());
			if(file->Read(data.GetBuffer(), data.Size()) != data.Size())
			{
				data.Empty();
				file->Release();
				return false;
			}
		}
		file->Release();
		return true;
	}
	long chank = 65536*4;
	while(true)
	{
		dword offset = data.Size();
		data.AddElements(chank);
		long readBytes = gzread(zipFile, &data[offset], chank);
		if(readBytes < 0)
		{
			gzclose(zipFile);
			data.Empty();
			return false;
		}
		if(readBytes < chank)
		{
			data.DelRange(offset + readBytes, data.Last());
			gzclose(zipFile);
			break;
		}
	}
	return true;
}

//Получить буфер
const byte * ZipMe::Data()
{
	return data.GetBuffer();
}

//Размер буфера
dword ZipMe::Size()
{
	return data.Size();
}

//Заменить в загруженном буфере все \r на \n и убрать двойные \n
void ZipMe::ClearText()
{
	long size = data.Size();
	if(!size) return;
	byte * buf = data.GetBuffer();
	for(long src = 1, dst = 1; src < data; src++)
	{
		if(buf[src] != '\r' && buf[src] != 0)
		{
			buf[dst++] = buf[src];
		}
	}
	if(dst < data)
	{
		data.DelRange(dst, data.Last());
	}
	data.Add(0);
}