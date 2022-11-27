//===========================================================================================================================
// Spirenkov Maxim, 2004
//===========================================================================================================================
// AnxEditor
//===========================================================================================================================
// AnxEditor Streams
//===========================================================================================================================

#include "AnxBase.h"

//===========================================================================================================================
//AnxStream
//===========================================================================================================================

AnxStream::AnxStream(bool isWriteMode, bool isBinary) : sections(_FL_)
{
	isWrite = isWriteMode;
	isError = false;
	this->isBinary = isBinary;
}

AnxStream::~AnxStream()
{
	if(!isError)
	{
		Assert(sections == 0);
	}
}

//Войти в секцию и сохранить о ней информацию
void AnxStream::EnterWriteSection(const char * id)
{
	Assert(isWrite);	
	Assert(!isBinary);
	char buf[5];
	sections.Add(GetPosition() + 1);
	buf[0] = 'S'; buf[1] = 0; buf[2] = 0; buf[3] = 0; buf[4] = 0;
	try
	{
		if(!WriteStreamData(buf, 5)) throw "";
		*this << id;
	}catch(...){
		isError = true;
		throw "Error stream write section header";
	}
}

//Выйти из секции
void AnxStream::ExitWriteSection()
{
	Assert(isWrite);	
	Assert(!isBinary);
	if(!WriteSizeToPosition(sections[sections - 1], GetPosition())){ isError = true; throw "Error stream write section size"; }
	sections.DelIndex(sections - 1);
}

//Сохранить bool
AnxStream & AnxStream::operator << (bool v)
{
	Assert(isWrite);
	char buf[2];
	buf[0] = 'b';
	buf[1] = v ? 1 : 0;
	if(!isBinary)
	{
		if(!WriteStreamData(buf, 2)){ isError = true; throw "Error stream write bool"; }
	}else{
		if(!WriteStreamData(buf + 1, 1)){ isError = true; throw "Error stream write bool"; }
	}
	return *this;
}

//Сохранить char
AnxStream & AnxStream::operator << (char v)
{
	Assert(isWrite);
	char buf[5];
	buf[0] = '1';
	buf[1] = v;
	if(!isBinary)
	{
		if(!WriteStreamData(buf, 2)){ isError = true;  throw "Error stream write char"; }
	}else{
		if(!WriteStreamData(buf + 1, 1)){ isError = true;  throw "Error stream write char"; }
	}
	return *this;
}

//Сохранить byte
AnxStream & AnxStream::operator << (byte v)
{
	return *this << char(v);
}

//Сохранить short
AnxStream & AnxStream::operator << (short v)
{
	Assert(isWrite);
	char buf[3];
	buf[0] = '2';
	*(short *)(buf + 1) = v;
	if(!isBinary)
	{
		if(!WriteStreamData(buf, 3)){ isError = true;  throw "Error stream write short"; }
	}else{
		if(!WriteStreamData(buf + 1, 2)){ isError = true;  throw "Error stream write short"; }
	}
	return *this;
}

//Сохранить int
AnxStream & AnxStream::operator << (int v)
{
	Assert(isWrite);
	char buf[5];
	buf[0] = 'i';
	*(dword *)(buf + 1) = v;
	if(!isBinary)
	{
		if(!WriteStreamData(buf, 5)){ isError = true;  throw "Error stream write int"; }
	}else{
		if(!WriteStreamData(buf + 1, 4)){ isError = true;  throw "Error stream write int"; }
	}
	return *this;
}

//Сохранить long
AnxStream & AnxStream::operator << (long v)
{
	Assert(isWrite);
	char buf[5];
	buf[0] = 'l';
	*(dword *)(buf + 1) = v;
	if(!isBinary)
	{
		if(!WriteStreamData(buf, 5)){ isError = true;  throw "Error stream write long"; }
	}else{
		if(!WriteStreamData(buf + 1, 4)){ isError = true;  throw "Error stream write long"; }
	}
	return *this;
}

//Сохранить dword
AnxStream & AnxStream::operator << (dword v)
{
	Assert(isWrite);
	char buf[5];
	buf[0] = 'd';
	*(dword *)(buf + 1) = v;
	if(!isBinary)
	{
		if(!WriteStreamData(buf, 5)){ isError = true;  throw "Error stream write dword"; }
	}else{
		if(!WriteStreamData(buf + 1, 4)){ isError = true;  throw "Error stream write dword"; }
	}
	return *this;
}

//Сохранить float
AnxStream & AnxStream::operator << (float v)
{
	Assert(isWrite);
	char buf[5];
	buf[0] = 'f';
	*(float *)(buf + 1) = v;
	if(!isBinary)
	{
		if(!WriteStreamData(buf, 5)){ isError = true;  throw "Error stream write float"; }
	}else{
		if(!WriteStreamData(buf + 1, 4)){ isError = true;  throw "Error stream write float"; }
	}
	return *this;
}

//Сохранить string
AnxStream & AnxStream::operator << (const char * v)
{
	Assert(isWrite);
	if(!v) v = "";
	long len = strlen(v);
	char buf[5];
	buf[0] = 's';
	*(dword *)(buf + 1) = len;
	if(!isBinary)
	{
		if(!WriteStreamData(buf, 5)){ isError = true;  throw "Error stream write string size"; }
	}else{
		if(!WriteStreamData(buf + 1, 4)){ isError = true;  throw "Error stream write string size"; }
	}
	if(len > 0) if(!WriteStreamData(v, len)){ isError = true;  throw "Error stream write string data"; }
	return *this;
}

//Сохранить данные
AnxStream & AnxStream::Write(const void * data, dword size)
{
	Assert(isWrite);
	Assert(!isBinary);
	char buf[5];
	buf[0] = 'B';
	*(dword *)(buf + 1) = size;
	if(!WriteStreamData(&buf, 5)){ isError = true;  throw "Error stream write binary data"; }
	if(!WriteStreamData(data, size)){ isError = true;  throw "Error stream write"; }
	return *this;
}

//Сохранить версию
void AnxStream::WriteVersion(dword ver)
{
	Assert(isWrite);
	Assert(!isBinary);
	char buf[5];
	buf[0] = 'v';
	*(dword *)(buf + 1) = ver;
	if(!WriteStreamData(buf, 5)){ isError = true;  throw "Error stream write version"; }
}

//Сохранить идентификатор
void AnxStream::WriteID(dword id)
{
	Assert(isWrite);
	Assert(!isBinary);
	if(GetPosition() != 0){ isError = true;  throw "Stream write position is not zero, can't write id"; }
	if(!WriteStreamData(&id, 4)){ isError = true;  throw "Error stream write id"; }
}


//Войти в секцию и получить(сохранить) о ней информацию
void AnxStream::EnterReadSection(string & id)
{
	Assert(!isWrite)
	Assert(!isBinary);
	char buf[5];
	if(!SysReadStreamData(buf, 5)){ isError = true; throw "Error stream read"; }
	if(buf[0] != 'S'){ isError = true; throw "Error stream read format section"; }
	*this >> id;
	sections.Add(*(dword *)(buf + 1));
}

void AnxStream::EnterReadSection(const char * checkID)
{
	string id;
	EnterReadSection(id);
	if(id != checkID) throw "Invalidate section id";
}

//Выйти из секции
void AnxStream::ExitReadSection()
{
	Assert(!isWrite);
	Assert(!isBinary);
	if(!SkipReadPositionTo(sections[sections - 1])){ isError = true; throw "Error stream skip section"; }
	sections.DelIndex(sections - 1);
}

//Прочитать bool
AnxStream & AnxStream::operator >> (bool & v)
{
	Assert(!isWrite);
	char buf[2];
	if(!isBinary)
	{
		if(!SysReadStreamData(buf, 2)){ isError = true; throw "Error stream read data"; }
		if(buf[0] != 'b'){ isError = true;  throw "Error stream read format bool"; }		
	}else{
		if(!SysReadStreamData(buf + 1, 1)){ isError = true; throw "Error stream read data"; }
	}
	v = (buf[1] != 0);
	return *this;
}

//Прочитать char
AnxStream & AnxStream::operator >> (char & v)
{
	Assert(!isWrite);
	char buf[2];
	if(!isBinary)
	{
		if(!SysReadStreamData(buf, 2)){ isError = true; throw "Error stream read data"; }
		if(buf[0] != '1'){ isError = true;  throw "Error stream read format char"; }		
	}else{
		if(!SysReadStreamData(buf + 1, 1)){ isError = true; throw "Error stream read data"; }
	}
	v = buf[1];
	return *this;
}

//Прочитать byte
AnxStream & AnxStream::operator >> (byte & v)
{
	char & c = *(char *)&v;
	return *this >> c;
}

//Прочитать short
AnxStream & AnxStream::operator >> (short & v)
{
	Assert(!isWrite);
	char buf[3];
	if(!isBinary)
	{
		if(!SysReadStreamData(buf, 3)){ isError = true;  throw "Error stream read data"; }
		if(buf[0] != '2'){ isError = true; throw "Error stream read format short"; }
	}else{
		if(!SysReadStreamData(buf + 1, 2)){ isError = true;  throw "Error stream read data"; }
	}
	v = *(short *)(buf + 1);
	return *this;
}

//Прочитать int
AnxStream & AnxStream::operator >> (int & v)
{
	Assert(!isWrite);
	char buf[5];
	if(!isBinary)
	{
		if(!SysReadStreamData(buf, 5)){ isError = true;  throw "Error stream read data"; }
		if(buf[0] != 'i'){ isError = true; throw "Error stream read format int"; }
	}else{
		if(!SysReadStreamData(buf + 1, 4)){ isError = true;  throw "Error stream read data"; }
	}
	v = *(dword *)(buf + 1);
	return *this;
}

//Прочитать long
AnxStream & AnxStream::operator >> (long & v)
{
	Assert(!isWrite);
	char buf[5];
	if(!isBinary)
	{
		if(!SysReadStreamData(buf, 5)){ isError = true;  throw "Error stream read data"; }
		if(buf[0] != 'l'){ isError = true; throw "Error stream read format long"; }
	}else{
		if(!SysReadStreamData(buf + 1, 4)){ isError = true;  throw "Error stream read data"; }
	}
	v = *(dword *)(buf + 1);
	return *this;
}

//Прочитать dword
AnxStream & AnxStream::operator >> (dword & v)
{
	Assert(!isWrite);
	char buf[5];
	if(!isBinary)
	{
		if(!SysReadStreamData(buf, 5)){ isError = true;  throw "Error stream read data"; }
		if(buf[0] != 'd'){ isError = true; throw "Error stream read format dword"; }
	}else{
		if(!SysReadStreamData(buf + 1, 4)){ isError = true;  throw "Error stream read data"; }
	}
	v = *(dword *)(buf + 1);
	return *this;
}

//Прочитать float
AnxStream & AnxStream::operator >> (float & v)
{
	Assert(!isWrite);
	char buf[5];
	if(!isBinary)
	{
		if(!SysReadStreamData(buf, 5)){ isError = true; throw "Error stream read data"; }
		if(buf[0] != 'f'){ isError = true; throw "Error stream read format float"; }
	}else{
		if(!SysReadStreamData(buf + 1, 4)){ isError = true; throw "Error stream read data"; }
	}
	v = *(float *)(buf + 1);
	return *this;
}

//Прочитать string
AnxStream & AnxStream::operator >> (string & v)
{
	Assert(!isWrite);
	char buf[5];
	if(!isBinary)
	{
		if(!SysReadStreamData(buf, 5)){ isError = true; throw "Error stream read data"; }
		if(buf[0] != 's'){ isError = true; throw "Error stream read format string"; }
	}else{
		if(!SysReadStreamData(buf + 1, 4)){ isError = true; throw "Error stream read data"; }
	}
	dword len = *(dword *)(buf + 1);
	char * sbuf = NEW char[len + 1];
	if(!SysReadStreamData(sbuf, len))
	{
		delete sbuf;
		isError = true;
		throw "Error stream read string data";
	}
	sbuf[len] = 0;
	v = sbuf;
	delete sbuf;
	return *this;
}

//Прочитать данные
AnxStream & AnxStream::Read(void * & data, dword & size)
{
	Assert(!isWrite);
	Assert(!isBinary);
	data = null;
	size = 0;
	char buf[5];
	if(!SysReadStreamData(buf, 5)){ isError = true; throw "Error stream read"; }
	if(buf[0] != 'B'){ isError = true; throw "Error stream read format binary data"; }
	dword len = *(dword *)(buf + 1);
	if(len > 0)
	{
		data = NEW byte[len];
		if(!SysReadStreamData(data, len))
		{
			delete data; data = null;
			isError = true; 
			throw "Error stream read";
		}
		size = len;
	}
	return *this;
}

//Прочитать версию
dword AnxStream::ReadVersion()
{
	Assert(!isWrite);
	Assert(!isBinary);
	char buf[5];
	if(!SysReadStreamData(buf, 5)){ isError = true;  throw "Error stream read data"; }
	if(buf[0] != 'v'){ isError = true; throw "Error stream read format version"; }
	return *(dword *)(buf + 1);
}

//Прочитать идентификатор
dword AnxStream::ReadID()
{
	Assert(!isWrite);
	Assert(!isBinary);
	if(GetPosition() != 0){ isError = true;  throw "Stream read position is not zero, can't read id"; }
	dword id;
	if(!SysReadStreamData(&id, 4)){ isError = true;  throw "Error stream read data"; }
	return id;
}

//Чтение с проверкой секции
inline bool AnxStream::SysReadStreamData(void * data, dword size)
{
	if(sections > 0)
	{
		if(GetPosition() + size > sections[sections - 1]) return false;
	}
	return ReadStreamData(data, size);
}


//===========================================================================================================================
//AnxStreamMemory
//===========================================================================================================================


AnxStreamMemory::AnxStreamMemory(bool isBinary) : AnxStream(true, isBinary), memdata(_FL_, 65536)
{
	position = 0;
}

AnxStreamMemory::~AnxStreamMemory()
{
}

void AnxStreamMemory::EndOfWrite()
{
	Assert(sections == 0);
	isWrite = false;
	position = 0;
}

void AnxStreamMemory::Reset()
{
	isWrite = true;
	position = 0;
	memdata.Empty();
}

void AnxStreamMemory::ResetPosition()
{
	isWrite = false;
	position = 0;
}

//Получить данне
void * AnxStreamMemory::GetMemData()
{
	return memdata.GetBuffer();
}

//Получить размер данных
dword AnxStreamMemory::GetDataSize()
{
	return memdata.GetDataSize();
}

//Получить копию данных
void * AnxStreamMemory::GetMemDataCopy(dword & size)
{
	void * data = NEW byte[memdata + 1];
	if(memdata > 0) memcpy(data, &memdata[0], memdata);
	size = (dword)memdata;
	return data;
}

//Сохранить данные
bool AnxStreamMemory::WriteStreamData(const void * data, dword size)
{	
	if(!size) return true;
	for(const byte * d = (const byte *)data; size; size--, d++) memdata.Add(*d);
	position = memdata;
	return true;
}

//Прочитать данные
bool AnxStreamMemory::ReadStreamData(void * data, dword size)
{
	if(!size) return true;
	for(byte * d = (byte *)data; size; size--, d++, position)
	{
		if(position >= (dword)memdata) return false;
		*d = memdata[position++];
	}
	return true;
}

//Получить пекущую позицию в потоке
dword AnxStreamMemory::GetPosition()
{
	return position;
}

//Получить пекущую позицию в потоке
bool AnxStreamMemory::WriteSizeToPosition(dword pos, dword size)
{
	if(pos + 4 > (dword)memdata) return false;
	*(dword *)&(memdata[pos]) = size;
	return true;
}

//При чтении перейти к позиции пропустив часть данных
bool AnxStreamMemory::SkipReadPositionTo(dword pos)
{
	if(pos > (dword)memdata) return false;
	position = pos;
	return true;
}

//===========================================================================================================================
//AnxStreamFile
//===========================================================================================================================

AnxStreamFile::AnxStreamFile(bool write, bool isBinary) : AnxStream(write, isBinary)
{
	file = null;
}

AnxStreamFile::~AnxStreamFile()
{
	if(file) file->Release(); file = null;
}

bool AnxStreamFile::Open(const char * fileName)
{
	if(file) file->Release(); file = null;
	IFileService * fs = (IFileService *)api->GetService("FileService");
	Assert(fs);
	bool res = false;
	if(isWrite)
	{
		file = fs->OpenFile(fileName, file_create_always, _FL_);
		res = (file != null);
	}else{
		file = fs->OpenFile(fileName, file_open_existing_for_read, _FL_);
		res = (file != null);
	}
	return res;
}

//Сохранить данные
bool AnxStreamFile::WriteStreamData(const void * data, dword size)
{
	if(!file) return false;
	if(!size) return true;
	return file->Write(data, size) == size;
}

//Прочитать данные
bool AnxStreamFile::ReadStreamData(void * data, dword size)
{
	if(!file) return false;
	if(!size) return true;
	return file->Read(data, size) == size;
}

//Получить пекущую позицию в потоке
dword AnxStreamFile::GetPosition()
{
	if(file) return file->GetPos();
	return 0;
}

//Получить пекущую позицию в потоке
bool AnxStreamFile::WriteSizeToPosition(dword pos, dword size)
{
	if(!file) return false;
	dword position = file->GetPos();
	if(file->SetPos(pos) != pos)
	{
		file->SetPos(position);
		return false;
	}
	if(!WriteStreamData(&size, sizeof(size)))return false;
	return file->SetPos(position) == position;
}

//При чтении перейти к позиции пропустив часть данных
bool AnxStreamFile::SkipReadPositionTo(dword pos)
{
	if(file) return file->SetPos(pos) == pos;
	return false;
}

