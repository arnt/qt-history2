/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbrush.h#30 $
**
** Definition of QBrush class
**
** Created : 940112
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QBRUSH_H
#define QBRUSH_H

#include "qcolor.h"
#include "qshared.h"


enum BrushStyle					// brush style
      { NoBrush, SolidPattern,
	Dense1Pattern, Dense2Pattern, Dense3Pattern, Dense4Pattern,
	Dense5Pattern, Dense6Pattern, Dense7Pattern,
	HorPattern, VerPattern, CrossPattern,
	BDiagPattern, FDiagPattern, DiagCrossPattern, CustomPattern=24 };


class QBrush
{
friend class QPainter;
public:
    QBrush();
    QBrush( BrushStyle );
    QBrush( const QColor &, BrushStyle=SolidPattern );
    QBrush( const QColor &, const QPixmap & );
    QBrush( const QBrush & );
   ~QBrush();
    QBrush &operator=( const QBrush & );

    BrushStyle	style()	 const		{ return data->style; }
    void	setStyle( BrushStyle );
    const QColor &color()const		{ return data->color; }
    void	setColor( const QColor & );
    QPixmap    *pixmap() const		{ return data->pixmap; }
    void	setPixmap( const QPixmap & );

    bool	operator==( const QBrush &p ) const;
    bool	operator!=( const QBrush &b ) const
					{ return !(operator==(b)); }

private:
    QBrush	copy()	const;
    void	detach();
    void	init( const QColor &, BrushStyle );
    struct QBrushData : public QShared {	// brush data
	BrushStyle style;
	QColor	  color;
	QPixmap	 *pixmap;
    } *data;
};


/*****************************************************************************
  QBrush stream functions
 *****************************************************************************/

QDataStream &operator<<( QDataStream &, const QBrush & );
QDataStream &operator>>( QDataStream &, QBrush & );


#endif // QBRUSH_H
