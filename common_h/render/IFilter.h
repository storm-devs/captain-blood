#ifndef IRENDER_FILTER_HPP
#define IRENDER_FILTER_HPP


#include "..\core.h"


class IRenderTarget;
class IBaseTexture;
class IRender;
class ITexture;

//Фильтр для построцессинга....
class IRenderFilter
{
	float fExecuteLevel;
	bool bEnabled;

	IRender* pRS;

public:

	IRenderFilter()
	{
		bEnabled = true;
		fExecuteLevel = 1.0f;
	};

	virtual ~IRenderFilter()
	{
	};

	virtual void Init ()
	{
		pRS = (IRender*)api->GetService("DX9Render");

		Assert(pRS);
	}


	virtual void FilterImage(IBaseTexture* source, IRenderTarget* destination) = 0; 

	IRender& Render()
	{
		return *pRS;
	}

	bool Enable(bool bValue)
	{
		bool bOldVal = bEnabled;
		bEnabled = bValue;
		return bOldVal;
	}

	bool IsEnabled ()
	{
		return bEnabled;
	}

	void SetExecuteLevel (float _fExecuteLevel)
	{ 
		fExecuteLevel = _fExecuteLevel;
	};

	float GetExecuteLevel() const
	{ 
		return fExecuteLevel;
	};




};



#endif