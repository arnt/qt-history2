/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qframe.h#33 $
**
** Definition of QFrame widget class
**
** Created : 950201
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QFRAME_H
#define QFRAME_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H


class Q_EXPORT QFrame : public QWidget			// frame class
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
	   StyledPanel = 0x0006,			// rectangular panel depending on the GUI style
	   MShape   = 0x000f,
	   Plain    = 0x0010,			// plain line
	   Raised   = 0x0020,			// raised shadow effect
	   Sunken   = 0x0030,			// sunken shadow effect
	   MShadow  = 0x00f0 };

    int		frameStyle()	const;
    int		frameShape()	const;
    int		frameShadow()	const;
    virtual void setFrameStyle( int );

    bool	lineShapesOk()	const;

    int		lineWidth()	const;
    virtual void setLineWidth( int );

    int		margin()	const;
    virtual void setMargin( int );

    int		midLineWidth()	const;
    virtual void setMidLineWidth( int );

    int		frameWidth()	const;
    QRect	frameRect()	const;
    QRect	contentsRect()	const;

    QSize	sizeHint() const;
    QSizePolicy sizePolicy() const;

protected:
    virtual void setFrameRect( const QRect & );
    void	paintEvent( QPaintEvent * );
    void	resizeEvent( QResizeEvent * );
    virtual void drawFrame( QPainter * );
    virtual void drawContents( QPainter * );
    virtual void frameChanged();
    void	updateMask();
    virtual void drawFrameMask( QPainter * );
    virtual void drawContentsMask( QPainter * );

private:
    void	updateFrameWidth();
    QRect	frect;
    int		fstyle;
    short	lwidth;
    short	mwidth;
    short	fwidth;
    short	lineok;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QFrame( const QFrame & );
    QFrame &operator=( const QFrame & );
#endif
};


inline int QFrame::frameStyle() const
{ return fstyle; }

inline int QFrame::frameShape() const
{ return fstyle & MShape; }

inline int QFrame::frameShadow() const
{ return fstyle & MShadow; }

inline bool QFrame::lineShapesOk() const
{ return lineok; }				// ### Qt 2.0: bool

inline int QFrame::lineWidth() const
{ return lwidth; }

inline int QFrame::midLineWidth() const
{ return mwidth & 0x00ff; }

inline int QFrame::margin() const
{ return ((int)mwidth) >> 8; }

inline int QFrame::frameWidth() const
{ return fwidth; }


#endif // QFRAME_H
