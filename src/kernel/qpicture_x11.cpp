/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpicture_x11.cpp#4 $
**
** Implementation of QMetaFile class for X11
**
** Author  : Haavard Nord
** Created : 940729
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qmetafil.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpicture_x11.cpp#4 $";
#endif


QMetaFile::QMetaFile()
{
    setDevType( PDT_METAFILE | PDF_EXTDEV );	// set device type
}

QMetaFile::~QMetaFile()
{
}
