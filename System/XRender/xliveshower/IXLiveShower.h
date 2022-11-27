#ifndef IXLIVESHOWER_H
#define IXLIVESHOWER_H

class IXLiveShower
{
public:
	virtual ~IXLiveShower() {}
	virtual bool RenderFrame() = 0;
};

#endif
