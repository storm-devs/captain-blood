

#ifndef _MissionEditorExport_h_
#define _MissionEditorExport_h_


#include "..\common_h\core.h"


class MissionEditorExport : public RegObject
{
public:

	virtual void ConvertXML2MIS (const char* xmlName) = null;

};


#endif

