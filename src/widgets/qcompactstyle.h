/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qwindowsstyle.h#5 $
**
** Definition of compact style class, good for small displays
**
** Created : 000623
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
    int extraPopupMenuItemWidth( bool checkable, int maxpmw, QMenuItem*, const QFontMetrics& );
    int popupMenuItemHeight( bool checkable, QMenuItem*, const QFontMetrics& );
    void drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw, int tab, QMenuItem* mi,
				       const QPalette& pal,
				       bool act, bool enabled, int x, int y, int w, int h);

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QCompactStyle( const QCompactStyle & );
    QCompactStyle& operator=( const QCompactStyle & );
#endif
};

#endif // QT_NO_STYLE_WINDOWS

#endif // QCOMPACTSTYLE_H
