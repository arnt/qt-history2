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

#include "listwidget_taskmenu.h"
#include "inplace_editor.h"
#include "ui_listwidgeteditor.h"

#include <abstractformeditor.h>
#include <abstractformwindow.h>
#include <abstractformwindowcursor.h>
#include <abstractformwindowmanager.h>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

ListWidgetTaskMenu::ListWidgetTaskMenu(QListWidget *button, QObject *parent)
    : QDesignerTaskMenu(button, parent),
      m_listWidget(button)
{
    QAction *action = 0;

    action = new QAction(this);
    action->setText(tr("Edit Items..."));
    connect(action, SIGNAL(triggered()), this, SLOT(editItems()));
    m_taskActions.append(action);
}

ListWidgetTaskMenu::~ListWidgetTaskMenu()
{
}

QList<QAction*> ListWidgetTaskMenu::taskActions() const
{
    return QDesignerTaskMenu::taskActions() + m_taskActions;
}

void ListWidgetTaskMenu::editItems()
{
    m_formWindow = AbstractFormWindow::findFormWindow(m_listWidget);
    if (m_formWindow.isNull())
        return;

    Q_ASSERT(m_listWidget != 0);

    QDialog dlg(m_listWidget->window());
    Ui::ListWidgetEditor ui;
    ui.setupUi(&dlg);
    dlg.exec();
}

ListWidgetTaskMenuFactory::ListWidgetTaskMenuFactory(QExtensionManager *extensionManager)
    : DefaultExtensionFactory(extensionManager)
{
}

QObject *ListWidgetTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QListWidget *button = qobject_cast<QListWidget*>(object)) {
        if (iid == Q_TYPEID(ITaskMenu)) {
            return new ListWidgetTaskMenu(button, parent);
        }
    }

    return 0;
}

void ListWidgetTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

