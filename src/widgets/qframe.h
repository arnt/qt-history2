/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qframe.h#19 $
**
** Definition of QFrame widget class
**
** Created : 950201
**
** Copyright (C) 1995-1996 by Troll Tech AS.  All rights reserved.
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
	   WinPanel = 0x0003,			// rectangular panel (Windows)
	   HLine    = 0x0004,			// horizontal line
	   VLine    = 0x0005,			// vertical line
	   MShape   = 0x000f,
	   Plain    = 0x0010,			// plain line
	   Raised   = 0x0020,			// raised shadow effect
	   Sunken   = 0x0030,			// sunken shadow effect
	   MShadow  = 0x00f0 };

    int		frameStyle()	const;
    int		frameShape()	const;
    int		frameShadow()	const;
    void	setFrameStyle( int );

    bool	lineShapesOk()	const;

    int		lineWidth()	const;
    void	setLineWidth( int );

    int		midLineWidth()	const;
    void	setMidLineWidth( int );

    int		frameWidth()	const;
    QRect	frameRect()	const;
    QRect	contentsRect()	const;

    QSize	sizeHint() const;

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

private:	// Disabled copy constructor and operator=
    QFrame( const QFrame & ) {}
    QFrame &operator=( const QFrame & ) { return *this; }
};


inline int QFrame::frameStyle() const
{ return fstyle; }

inline int QFrame::frameShape() const
{ return fstyle & MShape; }

inline int QFrame::frameShadow() const
{ return fstyle & MShadow; }

inline bool QFrame::lineShapesOk() const
{ return lineok; }

inline int QFrame::lineWidth() const
{ return lwidth; }

inline int QFrame::midLineWidth() const
{ return mwidth; }

inline int QFrame::frameWidth() const
{ return fwidth; }


#endif // QFRAME_H
