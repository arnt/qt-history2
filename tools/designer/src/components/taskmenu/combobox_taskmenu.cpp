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

#include "combobox_taskmenu.h"
#include "inplace_editor.h"
#include "listwidgeteditor.h"

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

ComboBoxTaskMenu::ComboBoxTaskMenu(QComboBox *button, QObject *parent)
    : QDesignerTaskMenu(button, parent),
      m_comboBox(button)
{
    m_editItemsAction = new QAction(this);
    m_editItemsAction->setText(tr("Edit Items..."));
    connect(m_editItemsAction, SIGNAL(triggered()), this, SLOT(editItems()));
    m_taskActions.append(m_editItemsAction);
}

ComboBoxTaskMenu::~ComboBoxTaskMenu()
{
}

QAction *ComboBoxTaskMenu::preferredEditAction() const
{
    return m_editItemsAction;
}

QList<QAction*> ComboBoxTaskMenu::taskActions() const
{
    return QDesignerTaskMenu::taskActions() + m_taskActions;
}

void ComboBoxTaskMenu::editItems()
{
    m_formWindow = AbstractFormWindow::findFormWindow(m_comboBox);
    if (m_formWindow.isNull())
        return;

    Q_ASSERT(m_comboBox != 0);

    ListWidgetEditor dlg(m_formWindow, m_comboBox->window());
    dlg.fillContentsFromComboBox(m_comboBox);
    if (dlg.exec() == QDialog::Accepted) {
        m_comboBox->clear();
        for (int i=0; i<dlg.count(); ++i) {
            m_comboBox->addItem(dlg.icon(i), dlg.text(i));
        }
    }
}

ComboBoxTaskMenuFactory::ComboBoxTaskMenuFactory(QExtensionManager *extensionManager)
    : DefaultExtensionFactory(extensionManager)
{
}

QObject *ComboBoxTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QComboBox *button = qobject_cast<QComboBox*>(object)) {
        if (iid == Q_TYPEID(ITaskMenu)) {
            return new ComboBoxTaskMenu(button, parent);
        }
    }

    return 0;
}

void ComboBoxTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

