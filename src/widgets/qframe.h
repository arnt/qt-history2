/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qframe.h#10 $
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
    QFrame( QWidget *parent=0, const char *name=0, WFlags f=0,
	    bool allowLines=TRUE );

    enum { NoFrame  = 0,			// no frame
	   Box	    = 0x0001,			// rectangular box
	   Panel    = 0x0002,			// rectangular panel
	   HLine    = 0x0003,			// horizontal line
	   VLine    = 0x0004,			// vertical line
	   MShape   = 0x000f,
	   Plain    = 0x0010,			// plain line
	   Raised   = 0x0020,			// raised shadow effect
	   Sunken   = 0x0030,			// sunken shadow effect
	   MShadow  = 0x00f0 };

    int		frameStyle()	const { return fstyle; }
    void	setFrameStyle( int );

    int		lineWidth()	const { return lwidth; }
    void	setLineWidth( int );

    int		midLineWidth()	const { return mwidth; }
    void	setMidLineWidth( int );

    int		frameWidth()	const { return fwidth; }
    QRect	frameRect()	const;
    QRect	contentsRect()	const;		// get rectangle inside frame

protected:
    void	setFrameRect( const QRect & );
    void	paintEvent( QPaintEvent * );
    void	resizeEvent( QResizeEvent * );
    virtual void drawFrame( QPainter * );
    virtual void drawContents( QPainter * );
    virtual void frameChanged();

private:
    void	updateFrameWidth();
    QRect	frect;
    int		fstyle;
    short	lwidth;
    short	mwidth;
    short	fwidth;
    short	lineok;
};


#endif // QFRAME_H
