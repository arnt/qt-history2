/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpicture.h#14 $
**
** Definition of QPicture class
**
** Created : 940729
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPICTURE_H
#define QPICTURE_H

#include "qpaintd.h"
#include "qbuffer.h"


class QPicture : public QPaintDevice		// picture class
{
public:
    QPicture();
   ~QPicture();

    bool	play( QPainter * );

    bool	load( const char *fileName );
    bool	save( const char *fileName );

protected:
    bool	cmd( int, QPainter *, QPDevCmdParam * );
    int		metric( int ) const;

private:
    bool	exec( QPainter *, QDataStream &, int );
    QBuffer	pictb;
    int		trecs;
    bool	formatOk;

private:	// Disabled copy constructor and operator=
    QPicture( const QPicture & ) :QPaintDevice(0) {}
    QPicture &operator=( const QPicture & ) { return *this; }
};


#endif // QPICTURE_H
