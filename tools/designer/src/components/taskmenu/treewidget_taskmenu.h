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

#ifndef TREEWIDGET_TASKMENU_H
#define TREEWIDGET_TASKMENU_H

#include <QtGui/QTreeWidget>
#include <QtCore/QPointer>

#include <qdesigner_taskmenu_p.h>
#include <extensionfactory_p.h>

QT_BEGIN_NAMESPACE

class QLineEdit;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class TreeWidgetTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    explicit TreeWidgetTaskMenu(QTreeWidget *button, QObject *parent = 0);
    virtual ~TreeWidgetTaskMenu();

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void editItems();
    void updateSelection();

private:
    QTreeWidget *m_treeWidget;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    QPointer<QLineEdit> m_editor;
    mutable QList<QAction*> m_taskActions;
    QAction *m_editItemsAction;
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QTreeWidget, TreeWidgetTaskMenu> TreeWidgetTaskMenuFactory;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // TREEWIDGET_TASKMENU_H
