/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcursor.cpp#3 $
**
** Implementation of QCursor class
**
** Author  : Haavard Nord
** Created : 940220
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcursor.h"
#include "qdstream.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcursor.cpp#3 $";
#endif


QDataStream &operator<<( QDataStream &s, const QCursor &c )
{
    return s << (INT16)c.shape();		// write shape id to stream
}

QDataStream &operator>>( QDataStream &s, QCursor &c )
{
    INT16 shape;
    s >> shape;					// read shape id from stream
    c.setShape( (int)shape );			// create cursor with shape
    return s;
}
