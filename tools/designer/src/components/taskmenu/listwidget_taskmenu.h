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

#ifndef LISTWIDGET_TASKMENU_H
#define LISTWIDGET_TASKMENU_H

#include "taskmenu_global.h"
#include <qdesigner_taskmenu.h>
#include <default_extensionfactory.h>

class QListWidget;
class ListWidgetEditor;

class QT_TASKMENU_EXPORT ListWidgetTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    ListWidgetTaskMenu(QListWidget *listWidget, QObject *parent = 0);
    virtual ~ListWidgetTaskMenu();

    QListWidget *listWidget() const;

    virtual QList<QAction*> taskActions() const;

private slots:
    void editItems();

private:
    QPointer<QListWidget> m_listWidget;
    QPointer<ListWidgetEditor> m_editor;
    QList<QAction*> m_actions;
};

class ListWidgetTaskMenuFactory: public DefaultExtensionFactory
{
    Q_OBJECT
public:
    ListWidgetTaskMenuFactory(QExtensionManager *extensionManager = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};


#endif // LISTWIDGET_TASKMENU_H
