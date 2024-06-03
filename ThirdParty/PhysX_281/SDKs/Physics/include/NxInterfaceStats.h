#ifndef NX_INTERFACE_STATS_H
#define NX_INTERFACE_STATS_H
/*----------------------------------------------------------------------------*\
|
|						Public Interface to Ageia PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#include "NxInterface.h"

#define NX_INTERFACE_STATS_VERSION 100

class NxInterfaceStats : public NxInterface
{
public:
  virtual int             getVersionNumber(void) const { return NX_INTERFACE_STATS_VERSION; };
  virtual NxInterfaceType getInterfaceType(void) const { return NX_INTERFACE_STATS; };
	virtual bool	          getHeapSize(int &used,int &unused) = 0;

};

#endif
//AGCOPYRIGHTBEGIN
///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 AGEIA Technologies.
// All rights reserved. www.ageia.com
///////////////////////////////////////////////////////////////////////////
//AGCOPYRIGHTEND
