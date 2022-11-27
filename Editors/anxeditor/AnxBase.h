
#ifndef _ANX_EDITOR_BASE_H_
#define _ANX_EDITOR_BASE_H_


enum AnxNodeTypes
{
	anxnt_error = 0,		//Недопустимое значение
	anxnt_bnode = 1,		//Базовый для всех нод
	anxnt_anode = 2,		//Нод с анимацией
	anxnt_hnode = 3,		//Нод включающий иерархию
	anxnt_gnode = 4,		//Нод групирующий линки
	anxnt_inode = 5,		//Нод принимающий на себя линк с иерархического нода
	anxnt_onode = 6,		//Нод выводящий линк из иерархического нода
};

#define ANX_STREAM_CATCH
#define _ANI_EVENTS_NO_MISSION_

#include "..\..\Common_h\math3d.h"
#include "..\..\Common_h\core.h"
#include "..\..\Common_h\render.h"
#include "..\..\Common_h\gmx.h"
#include "..\..\Common_h\gui.h"
#include "..\..\Common_h\FileService.h"
#include "..\..\Common_h\Templates\Array.h"
#include "..\..\Common_h\Templates\Stack.h"
#include "..\..\Common_h\Templates\String.h"

#include "AnxOptions.h"
#include "AnxStreams.h"


#endif
