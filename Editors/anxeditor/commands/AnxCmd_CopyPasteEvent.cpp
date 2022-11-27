//===========================================================================================================================
// Spirenkov Maxim, 2008
//===========================================================================================================================//	
//
//===========================================================================================================================
// AnxCmd_CopyPasteEvent
//============================================================================================


#include "AnxCmd_CopyPasteEvent.h"
#include "..\forms\NodeAnimationForm.h"

//============================================================================================

AnxCmd_CopyPasteEvent::AnxCmd_CopyPasteEvent(AnxOptions & options, NodeAnimationForm * _form, bool isCopy) : Command(options)
{
	Assert(_form);
	form = _form;
	isCopyEvent = isCopy;
}

AnxCmd_CopyPasteEvent::~AnxCmd_CopyPasteEvent()
{
}

//============================================================================================

bool AnxCmd_CopyPasteEvent::IsEnable()
{
	if(!opt.project) return false;
	if(isCopyEvent)
	{
		return form->IsEnableEventCopy();
	}
	return opt.project->eventCopyBuffer.GetDataSize() > 0;
}

void AnxCmd_CopyPasteEvent::Do()
{
	Assert(opt.project);
	Assert(form);
	if(isCopyEvent)
	{
		opt.project->eventCopyBuffer.Reset();
		form->CopyEvent(opt.project->eventCopyBuffer);
		opt.project->eventCopyBuffer.EndOfWrite();
	}else{
		Assert(opt.project->eventCopyBuffer.GetDataSize() > 0);
		opt.project->eventCopyBuffer.ResetPosition();
		form->PasteEvent(opt.project->eventCopyBuffer);
	}
}

Command * AnxCmd_CopyPasteEvent::CreateThisObject(AnxOptions & opt)
{
	return NEW AnxCmd_CopyPasteEvent(opt, form, isCopyEvent);
}


