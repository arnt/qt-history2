/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcursor.cpp#1 $
**
** Implementation of QCursor class
**
** Author  : Haavard Nord
** Created : 940220
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
** --------------------------------------------------------------------------
** This file containts the platform independent implementation of the QCursor
** class, which is mainly stream functions.  Platform dependent functions are
** found in the qcur_xxx.C files.
*****************************************************************************/

#include "qcursor.h"
#include "qstream.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcursor.cpp#1 $";
#endif


QStream &operator<<( QStream &s, const QCursor &c )
{
    return s << c.shape();			// write shape id to stream
}

QStream &operator>>( QStream &s, QCursor &c )
{
    int shape;
    s >> shape;					// read shape id from stream
    c.setShape( shape );			// create cursor with shape
    return s;
}
