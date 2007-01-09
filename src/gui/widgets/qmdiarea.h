/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMDIAREA_H
#define QMDIAREA_H

#include <QtGui/qworkspace.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class Q_GUI_EXPORT QMdiArea : public QWorkspace
{
public:
    explicit QMdiArea(QWidget *parent = 0) : QWorkspace(parent) {}
};

QT_END_HEADER

#endif // QMDIAREA_H
