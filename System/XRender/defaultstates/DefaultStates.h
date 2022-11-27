#ifndef RENDER_DEFAULT_STATES
#define RENDER_DEFAULT_STATES


#include "..\GraphicsApi.h"
#include "..\..\..\common_h\core.h"


class DefaultRenderStates
{

public:

	static bool GetDefaultValue (D3DRENDERSTATETYPE state, DWORD &defaultValue);
};


#endif