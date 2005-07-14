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

#include <QtCore/qglobal.h>

#ifdef QT3_SUPPORT
#include <QtGui/qaction.h>

QT_MODULE(Gui)

class Q_GUI_EXPORT QMenuItem : public QAction
{
public:
    QMenuItem();

    QT3_SUPPORT int id() const;
    QT3_SUPPORT int signalValue() const;
private:
    friend class QMenu;
    friend class QMenuBar;
    void setId(int);
    void setSignalValue(int);
};
#endif

#endif // QMENUDATA_H
