//===========================================================================================================================
// Spirenkov Maxim, 2004
//===========================================================================================================================
// AnxEditor
//===========================================================================================================================
// AnxEditor Streams
//===========================================================================================================================

#ifndef _AnxEditorStreams_h_
#define _AnxEditorStreams_h_


//===========================================================================================================================
//AnxStream
//===========================================================================================================================

class AnxStream
{
protected:
	AnxStream(bool isWriteMode, bool isBinary);
	virtual ~AnxStream();

public:
	//Войти в секцию и сохранить о ней информацию
	void EnterWriteSection(const char * id);
	//Выйти из секции
	void ExitWriteSection();
	//Сохранить bool
	AnxStream & operator << (bool v);
	//Сохранить char
	AnxStream & operator << (char v);
	//Сохранить byte
	AnxStream & operator << (byte v);
	//Сохранить short
	AnxStream & operator << (short v);
	//Сохранить int
	AnxStream & operator << (int v);
	//Сохранить long
	AnxStream & operator << (long v);
	//Сохранить dword
	AnxStream & operator << (dword v);
	//Сохранить float
	AnxStream & operator << (float v);
	//Сохранить string
	AnxStream & operator << (const char * v);
	//Сохранить данные
	AnxStream & Write(const void * data, dword size);
	//Сохранить версию
	void WriteVersion(dword ver);
	//Сохранить идентификатор
	void WriteID(dword id);

	//Войти в секцию и получить о ней информацию
	void EnterReadSection(string & id);
	void EnterReadSection(const char * checkID);
	//Выйти из секции
	void ExitReadSection();
	//Прочитать bool
	AnxStream & operator >> (bool & v);
	//Прочитать char
	AnxStream & operator >> (char & v);
	//Прочитать byte
	AnxStream & operator >> (byte & v);
	//Прочитать short
	AnxStream & operator >> (short & v);
	//Прочитать int
	AnxStream & operator >> (int & v);
	//Прочитать long
	AnxStream & operator >> (long & v);
	//Прочитать dword
	AnxStream & operator >> (dword & v);
	//Прочитать float
	AnxStream & operator >> (float & v);
	//Прочитать string
	AnxStream & operator >> (string & v);
	//Прочитать данные
	AnxStream & Read(void * & data, dword & size);
	//Прочитать версию
	dword ReadVersion();
	//Прочитать идентификатор
	dword ReadID();

public:
	//Сохранить данные
	virtual bool WriteStreamData(const void * data, dword size) = null;
	//Прочитать данные
	virtual bool ReadStreamData(void * data, dword size) = null;
	//Получить пекущую позицию в потоке
	virtual dword GetPosition() = null;
protected:
	//Получить пекущую позицию в потоке
	virtual bool WriteSizeToPosition(dword pos, dword size) = null;
	//При чтении перейти к позиции пропустив часть данных
	virtual bool SkipReadPositionTo(dword pos) = null;

private:
	//Чтение с проверкой секции
	bool SysReadStreamData(void * data, dword size);

protected:
	bool isWrite;
	bool isError;
	bool isBinary;
	array<dword> sections;
};


//===========================================================================================================================
//AnxStreamMemory
//===========================================================================================================================

class AnxStreamMemory : public AnxStream
{
public:
	AnxStreamMemory(bool isBinary = false);
	virtual ~AnxStreamMemory();

	void EndOfWrite();
	void Reset();
	void ResetPosition();

	//Получить данне
	void * GetMemData();
	//Получить размер данных
	dword GetDataSize();
	//Получить копию данных
	void * GetMemDataCopy(dword & size);
	

public:
	//Сохранить данные
	virtual bool WriteStreamData(const void * data, dword size);
	//Прочитать данные
	virtual bool ReadStreamData(void * data, dword size);
	//Получить пекущую позицию в потоке
	virtual dword GetPosition();
protected:
	//Получить пекущую позицию в потоке
	virtual bool WriteSizeToPosition(dword pos, dword size);
	//При чтении перейти к позиции пропустив часть данных
	virtual bool SkipReadPositionTo(dword pos);

private:
	array<byte> memdata;
	dword position;	
};

//===========================================================================================================================
//AnxStreamFile
//===========================================================================================================================

class IFile;

class AnxStreamFile : public AnxStream
{
public:
	AnxStreamFile(bool write, bool isBinary = false);
	virtual ~AnxStreamFile();

	bool Open(const char * fileName);

public:
	//Сохранить данные
	virtual bool WriteStreamData(const void * data, dword size);
	//Прочитать данные
	virtual bool ReadStreamData(void * data, dword size);
protected:
	//Получить пекущую позицию в потоке
	virtual dword GetPosition();
	//Получить пекущую позицию в потоке
	virtual bool WriteSizeToPosition(dword pos, dword size);
	//При чтении перейти к позиции пропустив часть данных
	virtual bool SkipReadPositionTo(dword pos);
private:
	IFile * file;
};



#endif












