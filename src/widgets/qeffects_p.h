/****************************************************************************
**
**
** Definition of QEffects functions
**
** Created : 2000.06.21
**
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the widgets
** module and therefore may only be used if the widgets module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QEFFECTS_H
#define QEFFECTS_H

#include "qnamespace.h"

#ifndef QT_NO_EFFECTS
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
#endif // QT_NO_EFFECTS
#endif // QEFFECTS_H
