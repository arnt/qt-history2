/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qframe.h#7 $
**
** Definition of QFrame widget class
**
** Author  : Haavard Nord
** Created : 950201
**
** Copyright (C) 1995 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#ifndef QFRAME_H
#define QFRAME_H

#include "qwidget.h"


class QFrame : public QWidget			// frame class
{
    Q_OBJECT
public:
    QFrame( QWidget *parent=0, const char *name=0, WFlags f=0 );

    const int	NoFrame = 0;			// no frame
    const int	Box	= 0x0001;		// rectangular box
    const int	Panel	= 0x0002;		// rectangular panel
    const int	HLine	= 0x0003;		// horizontal line
    const int	VLine	= 0x0004;		// vertical line
    const int	MType	= 0x000f;
    const int	Plain	= 0x0010;		// plain line
    const int	Raised	= 0x0020;		// raised shadow effect
    const int	Sunken	= 0x0030;		// sunken shadow effect
    const int	MStyle	= 0x00f0;

    QRect	contentsRect()	const;		// get rect inside frame

    int		frameStyle()	const { return fstyle; }
    void	setFrameStyle( int );

    int		lineWidth()	const { return lwidth; }
    void	setLineWidth( int );

    int		midLineWidth()	const { return mwidth; }
    void	setMidLineWidth( int );

    int		frameWidth()	const { return fwidth; }
    QRect	frameRect()	const;

protected:
    void	setFrameRect( const QRect & );
    void	paintEvent( QPaintEvent * );
    virtual void drawFrame( QPainter * );
    virtual void drawContents( QPainter * );	

private:
    void	updateFrameWidth();
    QRect	frect;				// frame rectangle
    int		fstyle;				// frame type/style
    short	lwidth;				// line width
    short	mwidth;				// mid line width
    short	fwidth;				// frame width
};


#endif // QFRAME_H
