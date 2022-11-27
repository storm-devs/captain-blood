

#include "Archivator.h"


//---------------------------------------------------------------------------------------------
//Сompressor
//---------------------------------------------------------------------------------------------


PackArchivator::FileInfo::FileInfo()
{
	data = null;
	size = 0;
	offset = 0;
	compressedSize = 0;
}

PackArchivator::FileInfo::~FileInfo()
{
	if(data)
	{
		delete data;
		data = null;
	}
}


PackArchivator::PackArchivator() : files(_FL_, 256),
									packedData(_FL_, 65536)
{
}

PackArchivator::~PackArchivator()
{
}

//Добавить файл
void PackArchivator::AddFile(const char * path, void * buffer, dword size)
{
	const char * fileName = string::GetFileName(path);
	if(string::IsEmpty(fileName))
	{
		return;
	}
	if(size == 0)
	{
		api->Trace("Error pack builder: file \"%s\" have got zero size and not include to pack", path);
		return;
	}	
	dword hash = string::HashNoCase(fileName);
	for(dword i = 0; i < files.Size(); i++)
	{
		if(files[i].hash == hash)
		{
			if(files[i].name == fileName)
			{
				api->Trace("Error pack builder: file name \"%s\" of path \"%s\" is repeat and not include to pack", fileName, path);
				return;
			}
		}
	}
	FileInfo & fi = files[files.Add()];
	fi.name = fileName;
	fi.hash = hash;
	fi.data = (byte *)buffer;
	fi.size = size;
}

//Процес подготовки данных
void PackArchivator::Process(Method method)
{
	for(dword i = 0; i < files.Size(); i++)
	{
		FileInfo & fi = files[i];
		const byte * fileData = fi.data;
		dword fileSize = fi.size;
		//Сжимаем
		if(fi.size > 0 && method == m_archive)
		{
			if(compressor.Compress(fi.data, fi.size))
			{
				Assert(compressor.Size() < fi.size);
				fileData = compressor.Buffer();
				fileSize = compressor.Size();
			}
		}
		fi.offset = packedData.Size();
		fi.compressedSize = fileSize;
		packedData.AddElements(fileSize);
		if(fileSize > 0)
		{
			memcpy(&packedData[fi.offset], fileData, fileSize);
		}
	}
}

//Получить количество файлов
dword PackArchivator::GetFilesCount()
{
	return files.Size();
}

//Получить общий размер таблицы имён
dword PackArchivator::GetNamesSize()
{
	dword size = 0;
	for(dword i = 0; i < files.Size(); i++)
	{
		FileInfo & fi = files[i];
		size += fi.name.Len() + 1;
	}
	return size;
}

//Получить путь архивированого файла
const char * PackArchivator::GetFileName(dword index, dword & len, dword & hash)
{
	len = files[index].name.Len();
	hash = files[index].hash;
	return files[index].name.c_str();
}

//Получить смещение от начала массива данных
dword PackArchivator::GetFileOffset(dword index)
{
	return files[index].offset;
}

//Получить размер несжатого файла
dword PackArchivator::GetFileSize(dword index)
{
	return files[index].size;
}

//Получить размер сжатого файла
dword PackArchivator::GetFileCompressedSize(dword index)
{
	return files[index].compressedSize;
}

//Получить буфер с данными
const void * PackArchivator::GetDataBuffer()
{
	return packedData.GetBuffer();
}

//Получить размер данных
dword PackArchivator::GetDataSize()
{
	return packedData.Size();
}


