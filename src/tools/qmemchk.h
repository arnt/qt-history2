/****************************************************************************
** $Id: //depot/qt/main/src/tools/qmemchk.h#1 $
**
** Definition of memory checking routines
**
** Author  : Haavard Nord
** Created : 920624
**
** Copyright (C) 1992-1996 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** Define the CHECK_MEMORY flag and recompile tools to enable memory checking.
*****************************************************************************/

#ifndef QMEMCHK_H
#define QMEMCHK_H


#if !defined(QGLOBAL_H)
#include "qglobal.h"
#endif


void  memchkSetLogFile( const char * );		// default is "MEMCHK.LOG"
void  memchkSetBufSize( int );			// default is 8192
bool  memchkSetReporting( bool );		// default is TRUE

int   memchkAllocCount();			// get # allocs

void  memchkStart();				// start memory check
void  memchkStop();				// stop checking


#if defined(CHECK_MEMORY)

void *operator new( uint size );		// replace new operator
void  operator delete( void *p );		// replace delete operator

#endif


#endif // QMEMCHK_H
