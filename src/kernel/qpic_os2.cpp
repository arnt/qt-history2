/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpic_os2.cpp#1 $
**
** Implementation of QMetaFile class for OS/2 PM
**
** Author  : Haavard Nord
** Created : 940802
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qmetafil.h"
#define	 INCL_PM
#include <os2.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpic_os2.cpp#1 $";
#endif


QMetaFile::QMetaFile()
{
    devType = PDT_METAFILE;			// set device type
}

QMetaFile::~QMetaFile()
{
}
