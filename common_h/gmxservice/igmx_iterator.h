/*
#ifndef GMX_ITERATOR_INTERFACE
#define GMX_ITERATOR_INTERFACE


class IGMXIterator
{
protected:
	
	virtual ~IGMXIterator() {};

public:

	IGMXIterator() {};

	virtual void Begin (IGMXEntity* StartEntity = NULL, bool recursive = true) = 0;
	virtual void Begin (const char* szStartEntityName, bool recursive = true) = 0;
	virtual bool IsDone () = 0;
	virtual void Clear () = 0;
	virtual void Next () = 0;

	virtual IGMXEntity* Get () = 0;

	virtual void AddRef() = 0;
	virtual bool Release() = 0;
	virtual void ForceRelease() = 0;



	virtual dword GetCount() = 0;
	virtual IGMXEntity* GetByIndex(dword dwIndex) = 0;


	
};



#endif
*/