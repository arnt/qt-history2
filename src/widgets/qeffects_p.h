/****************************************************************************
**
** Definition of QEffects functions.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QEFFECTS_P_H
#define QEFFECTS_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qeffects.cpp, qcombobox.cpp, qpopupmenu.cpp and qtooltip.cpp.
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qnamespace.h"
#endif // QT_H

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

#endif // QEFFECTS_P_H
