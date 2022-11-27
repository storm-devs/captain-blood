#ifndef _SystemMgs_h_
#define _SystemMgs_h_


#define CONTROLS_MESSAGE	"ctrlmsg"

struct ControlsMessage
{
	unsigned long dwMsgID;
	unsigned long wParam;
	unsigned long lParam;
};



#endif
