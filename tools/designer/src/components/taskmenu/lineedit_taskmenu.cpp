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

#include "lineedit_taskmenu.h"
#include "inplace_editor.h"

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

LineEditTaskMenu::LineEditTaskMenu(QLineEdit *lineEdit, QObject *parent)
    : QDesignerTaskMenu(lineEdit, parent),
      m_lineEdit(lineEdit)
{
    m_editTextAction = new QAction(this);
    m_editTextAction->setText(tr("Edit Text"));
    connect(m_editTextAction, SIGNAL(triggered()), this, SLOT(editText()));
    m_taskActions.append(m_editTextAction);
}

LineEditTaskMenu::~LineEditTaskMenu()
{
}

QAction *LineEditTaskMenu::preferredEditAction() const
{
    return m_editTextAction;
}

QList<QAction*> LineEditTaskMenu::taskActions() const
{
    return QDesignerTaskMenu::taskActions() + m_taskActions;
}

void LineEditTaskMenu::editText()
{
    m_formWindow = AbstractFormWindow::findFormWindow(m_lineEdit);
    if (!m_formWindow.isNull()) {
        connect(m_formWindow, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
        Q_ASSERT(m_lineEdit->parentWidget() != 0);

        m_editor = new InPlaceEditor(m_lineEdit);
        m_editor->setObjectName("__qt__passive_m_editor");

        m_editor->setFrame(false);
        m_editor->setText(m_lineEdit->text());
        m_editor->selectAll();
        m_editor->setBackgroundRole(m_lineEdit->backgroundRole());
        connect(m_editor, SIGNAL(returnPressed()), m_editor, SLOT(deleteLater()));
        connect(m_editor, SIGNAL(textChanged(const QString &)), this, SLOT(updateText(const QString&)));

        QStyleOption opt;
        opt.init(m_lineEdit);
        QRect r = opt.rect;

        m_editor->setGeometry(QRect(m_lineEdit->mapTo(m_lineEdit->window(), r.topLeft()), r.size()));
        m_editor->setFocus();
        m_editor->show();
    }
}

void LineEditTaskMenu::editIcon()
{
    qDebug() << "edit icon";
}

LineEditTaskMenuFactory::LineEditTaskMenuFactory(QExtensionManager *extensionManager)
    : DefaultExtensionFactory(extensionManager)
{
}

QObject *LineEditTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(object)) {
        if (iid == Q_TYPEID(ITaskMenu)) {
            return new LineEditTaskMenu(lineEdit, parent);
        }
    }

    return 0;
}

void LineEditTaskMenu::updateText(const QString &text)
{
    m_formWindow->cursor()->setWidgetProperty(m_lineEdit, QLatin1String("text"), QVariant(text));
}

void LineEditTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

