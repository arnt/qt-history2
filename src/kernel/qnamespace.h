/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qnamespace.h#2 $
**
** Definition of Qt namespace (as class for compiler compatibility)
**
** Created : 980927
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef QNAMESPACE_H
#define QNAMESPACE_H

#if !defined(QT_H)
#include "qglobal.h"
#endif


class QColor;


#if defined(QT_MAYBE_CONST)
#error "what happened?  I am dazed and confused."
#elif defined(_CC_MSC_)
#define QT_MAYBE_CONST
#else
#define QT_MAYBE_CONST const
#endif


class Q_EXPORT Qt {
public:
    static QT_MAYBE_CONST QColor & color0;
    static QT_MAYBE_CONST QColor & color1;
    static QT_MAYBE_CONST QColor & black;
    static QT_MAYBE_CONST QColor & white;
    static QT_MAYBE_CONST QColor & darkGray;
    static QT_MAYBE_CONST QColor & gray;
    static QT_MAYBE_CONST QColor & lightGray;
    static QT_MAYBE_CONST QColor & red;
    static QT_MAYBE_CONST QColor & green;
    static QT_MAYBE_CONST QColor & blue;
    static QT_MAYBE_CONST QColor & cyan;
    static QT_MAYBE_CONST QColor & magenta;
    static QT_MAYBE_CONST QColor & yellow;
    static QT_MAYBE_CONST QColor & darkRed;
    static QT_MAYBE_CONST QColor & darkGreen;
    static QT_MAYBE_CONST QColor & darkBlue;
    static QT_MAYBE_CONST QColor & darkCyan;
    static QT_MAYBE_CONST QColor & darkMagenta;
    static QT_MAYBE_CONST QColor & darkYellow;

    enum ButtonState {				// mouse/keyboard state values
	NoButton	= 0x00,
	LeftButton	= 0x01,
	RightButton	= 0x02,
	MidButton	= 0x04,
	MouseButtonMask = 0x07,
	ShiftButton	= 0x08,
	ControlButton   = 0x10,
	AltButton	= 0x20,
	KeyButtonMask   = 0x38
    };
};


#endif
