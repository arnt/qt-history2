/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpicture.h#3 $
**
** Definition of QMetaFile class
**
** Author  : Haavard Nord
** Created : 940729
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QMETAFIL_H
#define QMETAFIL_H

#include "qpaintd.h"
#include "qbuffer.h"


class QMetaFile : public QPaintDevice		// metafile class
{
public:
    QMetaFile();
   ~QMetaFile();

    bool play( QPainter * );

    bool load( const char *fileName );		// read from file
    bool save( const char *fileName );		// write to file

public:
    bool cmd( int, QPDevCmdParam * );

private:
    QBuffer mfbuf;
    long    trecs;
    bool exec( QPainter *, QDataStream &s, long );
};


#endif // QMETAFIL_H
