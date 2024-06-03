#ifndef NX_INTERFACE_H
#define NX_INTERFACE_H
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

enum NxInterfaceType
{
	NX_INTERFACE_STATS,
	NX_INTERFACE_LAST
};


class NxInterface
{
public:
  virtual int getVersionNumber(void) const = 0;
  virtual NxInterfaceType getInterfaceType(void) const = 0;

protected:
  virtual ~NxInterface(){};
};

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
