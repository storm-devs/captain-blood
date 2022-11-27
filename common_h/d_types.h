#ifndef _D_TYPES_H_
#define _D_TYPES_H_



typedef unsigned int dword;
typedef unsigned short word;
typedef unsigned char byte;

#ifndef null
#define null 0
#endif

#define _FL_			__FILE__, __LINE__
#define _FILELINE_		_FL_

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif


#endif