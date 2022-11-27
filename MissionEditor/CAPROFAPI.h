//**************************************************************************
//
// Copyright 2005 ADVANCED MICRO DEVICES, INC. All Rights Reserved.
//
// This software and any related documentation (the 'Materials') are the
// confidential proprietary information of AMD. Unless otherwise provided
// in a software agreement specifically licensing the Materials, the Materials
// are provided in confidence and may not to be distributed, modified, or 
// reproduced in whole or in part by any means.
// 
// LIMITATION OF LIABILITY: THE MATERIALS ARE PROVIDED 'AS IS' WITHOUT ANY
// EXPRESS OR IMPLIED WARRANTY OF ANY KIND, INCLUDING BUT NOT LIMITED TO 
// WARRANTIES OF MERCHANTABILITY, NONINFRINGEMENT, TITLE, FITNESS FOR ANY
// PARTICULAR PURPOSE, OR WARRANTIES ARISING FORM CONDUCT, COURSE OF DEALING,
// OR USAGE OF TRADE.  IN NO EVENT SHALL AMD OR ITS LICENSORS BE LIABLE FOR ANY
// DAMAGES WHATSOEVER (INCLUDING,WITHOUT LIMITATION, DAMAGES FOR LOSS OF PROFITS,
// BUSINESS INTERRUPTION, OR LOSS OF INFORMATION) ARISING OUT OF THE USE OF OR
// INABILITY TO USE THE MATERIALS, EVEN IF AMD HAS BEEN ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGES.  BECAUSE SOME JURISDICTIONS PROHIBIT THE 
// EXCLUSION OR LIMITATION OF LIABILITY FOR CONSEQUENTIAL OR INCIDENTAL DAMAGES,
// THE ABOVE LIMITATION MAY NOT APPLY TO YOU.  
// 
// AMD does not assume any responsibility for any errors which may appear in the 
// Materials nor any responsibility to support or update the
// Materials.  AMD retains the right to modify the Materials at any time, 
// without notice, and is not obligated to provide such modified Materials to
// you.
// 
// NO SUPPORT OBLIGATION: AMD is not obligated to furnish, support, or make any
// further information, software, technical information, know-how, or show-how
// available to you.
// 
// U.S. GOVERNMENT RESTRICTED RIGHTS: The Materials are provided with
// 'RESTRICTED RIGHTS.' Use, duplication, or disclosure by the Government is 
// subject to the restrictions as set forth in FAR 52.227-14 and DFAR 
// 252.227-7013, et seq., or its successor.  Use of the Materials by the 
// Government constitutes acknowledgement of AMD's proprietary rights in them.
// ============================================================================
//
///////////////////////////////////////////////////////////////////////////////
//
// CAProfAPI.h 08-25-2004
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _CAPROFAPI_H_
#define _CAPROFAPI_H_

#ifdef CAPROFAPI_EXPORTS
#define CAPROFAPI __declspec(dllexport)
#else
#define CAPROFAPI __declspec(dllimport)
#endif


#define CAPROFAPICALL __cdecl


#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
// CAProfPause()
//	Pause profiling if current profile is in profiling state. If current 
//	profile is in other state, the function does not take any action.
//
///////////////////////////////////////////////////////////////////////////////
CAPROFAPI
VOID CAPROFAPICALL CAProfPause(void);

///////////////////////////////////////////////////////////////////////////////
// CAProfResume()
//	Resume profiling if and only if current profile is in pause state.
//
///////////////////////////////////////////////////////////////////////////////
CAPROFAPI
VOID CAPROFAPICALL CAProfResume(void);

#ifdef __cplusplus
}
#endif

#endif  _CAPROFAPI_H_


