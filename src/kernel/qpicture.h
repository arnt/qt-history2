/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpicture.h#30 $
**
** Definition of QPicture class
**
** Created : 940729
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QPICTURE_H
#define QPICTURE_H

#ifndef QT_H
#include "qpaintdevice.h"
#include "qbuffer.h"
#endif // QT_H


class Q_EXPORT QPicture : public QPaintDevice		// picture class
{
public:
    QPicture( int formatVersion = 0 );
   ~QPicture();

    bool	isNull() const;

    uint	size() const;
    const char* data() const;
    virtual void setData( const char* data, uint size );

    bool	play( QPainter * );

    bool	load( const QString &fileName );
    bool	save( const QString &fileName );

    QPicture& operator= (const QPicture&);

protected:
    bool	cmd( int, QPainter *, QPDevCmdParam * );
    int		metric( int ) const;

private:
    bool	exec( QPainter *, QDataStream &, int );
    void	resetFormat();
    QBuffer	pictb;
    int		trecs;
    bool	formatOk;
    int		formatMajor;
    int		formatMinor;

private:       // Disabled copy constructor
#if defined(Q_DISABLE_COPY)
    QPicture( const QPicture & );
#endif
};


inline bool QPicture::isNull() const
{
    return pictb.buffer().isNull();
}

inline uint QPicture::size() const
{
    return pictb.buffer().size();
}

inline const char* QPicture::data() const
{
    return pictb.buffer().data();
}


#endif // QPICTURE_H
