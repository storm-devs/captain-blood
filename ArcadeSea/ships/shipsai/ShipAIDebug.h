
#ifndef _ShipAIDebug_h_
#define _ShipAIDebug_h_

#include "..\..\..\Common_h\Math3D.h"

/*#define DEBUG_DRAW
#define DEBUG_DRAW_TARGET_LINES
#define DEBUG_DRAW_OBSTACLE_RADIUS
#define DEBUG_DRAW_OBSTACLES_FORCES*/

// дебажная стрелка
struct DebugArrow
{
	DebugArrow::DebugArrow () : dwColor( 0xffffffff ) {}

	Vector					vStart;
	Vector					vEnd;
	dword					dwColor;
};

#endif
