/****************************************************************************
** $Id: $
**
** Definition of compact style class, good for small displays
**
** Created : 000623
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QCOMPACTSTYLE_H
#define QCOMPACTSTYLE_H

#ifndef QT_H
#include "qwindowsstyle.h"
#endif // QT_H

#ifndef QT_NO_STYLE_COMPACT

#if defined(QT_PLUGIN_STYLE_COMPACT)
#define Q_EXPORT_STYLE_COMPACT
#else
#define Q_EXPORT_STYLE_COMPACT Q_EXPORT
#endif

class Q_EXPORT_STYLE_COMPACT QCompactStyle : public QWindowsStyle
{
public:
    QCompactStyle();
    virtual ~QCompactStyle();
    int extraPopupMenuItemWidth( bool checkable, int maxpmw, QMenuItem*,
				 const QFontMetrics& ) const;
    int popupMenuItemHeight( bool checkable, QMenuItem*,
			     const QFontMetrics& ) const;
    void drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw, int tab, QMenuItem* mi,
				       const QPalette& pal,
				       bool act, bool enabled, int x, int y, int w, int h);

    int buttonMargin() const;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QCompactStyle( const QCompactStyle & );
    QCompactStyle& operator=( const QCompactStyle & );
#endif
};

#endif // QT_NO_STYLE_WINDOWS

#endif // QCOMPACTSTYLE_H
