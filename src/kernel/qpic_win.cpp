/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpic_win.cpp#2 $
**
** Implementation of QMetaFile class for Windows + NT
**
** Author  : Haavard Nord
** Created : 940802
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qmetafil.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpic_win.cpp#2 $";
#endif


QMetaFile::QMetaFile()
{
    setDevType( PDT_METAFILE | PDF_EXTDEV );	// set device type
}

QMetaFile::~QMetaFile()
{
}
