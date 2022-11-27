
#ifndef _TipsManager_h_
#define _TipsManager_h_


#include "..\..\..\Common_h\Mission.h"
#include "..\..\..\Common_h\ITips.h"
#include "..\..\..\Common_h\IGUIElement.h"
#include "Tips.h"
#include "..\..\utils\SafePointerEx.h"

class TipsManager : public ITipsManager
{	

public:

	TipsManager();
	~TipsManager();

	struct TipDescr
	{
		ConstString id;
		MOSafePointerTypeEx<IGUIElement> widget;
		ConstString widgetName;

		bool  is3D;

		bool update;
	};

	array<TipDescr> tips_types;

	array<Tip*> tips;

	//--------------------------------------------------------------------------------------------
public:
		
	//Инициализировать объект
	virtual bool Create(MOPReader & reader);	
	//Обновить параметры
	virtual bool EditMode_Update(MOPReader & reader);	
	//Нарисовать модельку
	void _cdecl Draw(float dltTime, long level);	

	virtual ITip* CreateTip(const ConstString & id, Vector pos,void* ptr);
	virtual void  DeleteTip(ITip* tip);

	virtual void  Enable(bool enable);	

	virtual void  SetTipState(ITip::TState _state, void* ptr);	

	virtual bool  IsAnyActiveTip();

private:	
	bool isEnable;

	void InitParams(MOPReader & reader);
};

#endif











