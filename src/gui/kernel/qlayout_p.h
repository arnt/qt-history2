/****************************************************************************
**
** Definition of some Qt private functions.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef QLAYOUT_P_H
#define QLAYOUT_P_H
//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qlayout*.cpp, and qabstractlayout.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//
//
#ifndef QT_H
#include <private/qobject_p.h>
#endif // QT_H

class Q_GUI_EXPORT QLayoutPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QLayout)
public:
    QLayoutPrivate();

    int insideSpacing;
    int outsideBorder;
    uint topLevel : 1;
    uint enabled : 1;
    uint frozen : 1;
    uint activated : 1;
    uint autoMinimum : 1;
    uint autoResizeMode : 1;
    uint autoNewChild : 1;
    QRect rect;
#ifndef QT_NO_MENUBAR
    QMenuBar *menubar;
#endif
};

#endif
