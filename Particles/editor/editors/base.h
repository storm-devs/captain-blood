#ifndef _XBOX

#ifndef BASE_EDITOR
#define BASE_EDITOR 

#include "..\..\..\common_h\gui.h"
#include "..\..\..\common_h\core.h"

class BaseEditor : public Object
{

public:


	BaseEditor () {};
  virtual ~BaseEditor () {};
  
  virtual void Release () = 0;

	virtual void EndEdit () = 0;	
	virtual void Apply () = 0;
};


#endif

#endif