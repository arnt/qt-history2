/****************************************************************************
**
**
** Definition of QEffects functions
**
** Created : 2000.06.21
**
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
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

#ifndef QEFFECTS_H
#define QEFFECTS_H

#include "qnamespace.h"

class QWidget;

struct QEffects
{
    enum Direction {
	LeftScroll	= 0x0001,
	RightScroll	= 0x0002,
	UpScroll	= 0x0004,
	DownScroll	= 0x0008
    };

    typedef uint DirFlags;
};

extern void Q_EXPORT qScrollEffect( QWidget*, QEffects::DirFlags dir = QEffects::DownScroll, int time = -1 );
extern void Q_EXPORT qFadeEffect( QWidget*, int time = -1 );

#endif // QEFFECTS_H
