/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qwindowsstyle.h#5 $
**
** Definition of compact style class, good for small displays
**
** Created : 000623
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

#ifndef QCOMPACTSTYLE_H
#define QCOMPACTSTYLE_H

#ifndef QT_H
#include "qwindowsstyle.h"
#endif // QT_H

#ifndef QT_NO_STYLE_COMPACT


class Q_EXPORT QCompactStyle : public QWindowsStyle
{
public:
    QCompactStyle();
    virtual ~QCompactStyle();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QCompactStyle( const QCompactStyle & );
    QCompactStyle& operator=( const QCompactStyle & );
#endif
};

#endif // QT_NO_STYLE_WINDOWS

#endif // QCOMPACTSTYLE_H
