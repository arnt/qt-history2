/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpen.h#31 $
**
** Definition of QPen class
**
** Created : 940112
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QPEN_H
#define QPEN_H

#ifndef QT_H
#include "qcolor.h"
#include "qshared.h"
#endif // QT_H


class Q_EXPORT QPen: public Qt
{
friend class QPainter;
public:
    QPen();
    QPen( PenStyle );
    QPen( const QColor &color, uint width=0, PenStyle style=SolidLine );
    QPen( const QColor &cl, uint w, PenStyle s, PenCapStyle c, PenJoinStyle j);
    QPen( const QPen & );
   ~QPen();
    QPen &operator=( const QPen & );

    PenStyle	style() const		{ return data->style; }
    void	setStyle( PenStyle );
    uint	width() const		{ return data->width; }
    void	setWidth( uint );
    const QColor &color() const		{ return data->color; }
    void	setColor( const QColor & );
    PenCapStyle	capStyle() const;
    void	setCapStyle( PenCapStyle );
    PenJoinStyle joinStyle() const;
    void	setJoinStyle( PenJoinStyle );

    bool	operator==( const QPen &p ) const;
    bool	operator!=( const QPen &p ) const
					{ return !(operator==(p)); }

private:
    QPen	copy()	const;
    void	detach();
    void	init( const QColor &, uint, uint );
    struct QPenData : public QShared {		// pen data
	PenStyle  style;
	uint	  width;
	QColor	  color;
	Q_UINT16  linest;
    } *data;
};


/*****************************************************************************
  QPen stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QPen & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QPen & );
#endif

#endif // QPEN_H
