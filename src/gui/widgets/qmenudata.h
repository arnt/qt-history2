/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMENUDATA_H
#define QMENUDATA_H

#include <qglobal.h>

#ifdef QT_COMPAT
#include <qaction.h>

class QSignalEmitter;

class Q_GUI_EXPORT QMenuItem : public QAction
{
public:
    QMenuItem();

    QT_COMPAT int id() const;
    QT_COMPAT QSignalEmitter *signal() const;
    QT_COMPAT int signalValue() const;
private:
    friend class QMenu;
    friend class QMenuBar;
    void setId(int);
    void setSignalValue(int);
};
#endif

#endif // QMENUDATA_H
