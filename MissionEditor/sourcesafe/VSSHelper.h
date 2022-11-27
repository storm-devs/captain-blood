/*


#ifndef VSS_STORM3_ACCESSOR
#define VSS_STORM3_ACCESSOR

#include "..\..\common_h\core.h"
#include "ssauto.h"





struct SourceSafeItem
{
	enum ItemType
	{
		VSS_UNKNOWN = 0,
		VSS_PROJECT,
		VSS_FILE,

		VSS_FORCE_DWORD = 0x7fffffff
	};

	ItemType Type;
	bool bCheckedOut;
	string Name;
	SourceSafeItem* pParent;

	
	void* HidedVSS;

	array<SourceSafeItem> Childs;

	//Залить на сервак
	bool Checkin (const char* szLocalPath);

	//Взять с сервака
	bool Checkout (const char* szLocalPath);

	//Отменить взятие с сервака
	bool UndoCheckout (const char* szLocalPath);

	//Получить последнюю версию
	bool GetLatestVersion (const char* szLocalPath);

	//Добавить к проекту (нод должен иметь тип VSS_PROJECT)
	bool AddToProject (const char* szFullPathName);

	void Clear ();


	int GetChildIndex (const char* szChildName);


	string WhoCheckedOut();

	SourceSafeItem();
	~SourceSafeItem();
	
	
};


class VSSAcess
{


public:

	VSSAcess();
	~VSSAcess();

	static bool VSSEnumDatabase (SourceSafeItem& root, const char* szUserName);
};


#endif

*/