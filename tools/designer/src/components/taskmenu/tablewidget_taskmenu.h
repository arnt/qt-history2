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

#ifndef TABLEWIDGET_TASKMENU_H
#define TABLEWIDGET_TASKMENU_H

#include <qdesigner_taskmenu_p.h>
#include <extensionfactory_p.h>

#include <QtGui/QTableWidget>
#include <QtCore/QPointer>

class QLineEdit;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class TableWidgetTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    explicit TableWidgetTaskMenu(QTableWidget *button, QObject *parent = 0);
    virtual ~TableWidgetTaskMenu();

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void editItems();
    void updateSelection();

private:
    QTableWidget *m_tableWidget;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    QPointer<QLineEdit> m_editor;
    mutable QList<QAction*> m_taskActions;
    QAction *m_editItemsAction;
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QTableWidget, TableWidgetTaskMenu> TableWidgetTaskMenuFactory;
}  // namespace qdesigner_internal

#endif // TABLEWIDGET_TASKMENU_H
