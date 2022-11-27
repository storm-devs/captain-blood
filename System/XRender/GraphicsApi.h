#ifndef STORM3_NEXTGEN_RENDER_GRAPHICS_API
#define STORM3_NEXTGEN_RENDER_GRAPHICS_API







#ifndef _XBOX
#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "GdiPlus.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dxguid.lib ")



inline void PIXBeginNamedEvent (unsigned long color_not_used, const char* eventName) {};
inline void PIXEndNamedEvent () {};

#else

#include <xtl.h>
#include <Xgraphics.h>
#include <fxl.h>




#endif


#endif