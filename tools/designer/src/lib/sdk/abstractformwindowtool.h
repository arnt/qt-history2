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

#ifndef ABSTRACTFORMWINDOWTOOL_H
#define ABSTRACTFORMWINDOWTOOL_H

#include "sdk_global.h"

#include <QObject>

class AbstractFormEditor;
class AbstractFormWindow;
class QWidget;

class QT_SDK_EXPORT AbstractFormWindowTool: public QObject
{
    Q_OBJECT
public:
    AbstractFormWindowTool(QObject *parent = 0);
    virtual ~AbstractFormWindowTool();

    virtual AbstractFormEditor *core() const = 0;
    virtual AbstractFormWindow *formWindow() const = 0;

    virtual bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event) = 0;
};

#endif // ABSTRACTFORMWINDOWTOOL_H
