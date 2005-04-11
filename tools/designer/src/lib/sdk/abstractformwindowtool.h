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

#include <QtDesigner/sdk_global.h>

#include <QtCore/QObject>

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QWidget;
class QAction;
class DomUI;

class QT_SDK_EXPORT QDesignerFormWindowToolInterface: public QObject
{
    Q_OBJECT
public:
    QDesignerFormWindowToolInterface(QObject *parent = 0);
    virtual ~QDesignerFormWindowToolInterface();

    virtual QDesignerFormEditorInterface *core() const = 0;
    virtual QDesignerFormWindowInterface *formWindow() const = 0;
    virtual QWidget *editor() const = 0;

    virtual QAction *action() const = 0;

    virtual void activated() = 0;
    virtual void deactivated() = 0;

    virtual void saveToDom(DomUI*, QWidget*) {}
    virtual void loadFromDom(DomUI*, QWidget*) {}

    virtual bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event) = 0;
};

#endif // ABSTRACTFORMWINDOWTOOL_H
