/****************************************************************************
**
** Definition of QPen class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPEN_H
#define QPEN_H

#ifndef QT_H
#include "qcolor.h"
#include "qshared.h"
#endif // QT_H


class Q_EXPORT QPen: public Qt
{
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
    friend class QPainter;
#ifdef Q_WS_WIN
    friend class QFontEngineWin;
#endif

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
