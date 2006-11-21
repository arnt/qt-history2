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

/*
TRANSLATOR qdesigner_internal::LineEditTaskMenu
*/

#include "lineedit_taskmenu.h"
#include "inplace_editor.h"

#include <QtDesigner/QtDesigner>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

LineEditTaskMenu::LineEditTaskMenu(QLineEdit *lineEdit, QObject *parent)
    : QDesignerTaskMenu(lineEdit, parent),
      m_lineEdit(lineEdit),
      m_editTextAction(new QAction(tr("Change text..."), this))
{
    connect(m_editTextAction, SIGNAL(triggered()), this, SLOT(editText()));
    m_taskActions.append(m_editTextAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
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
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void LineEditTaskMenu::editText()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_lineEdit);
    if (!m_formWindow.isNull()) {
        connect(m_formWindow, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
        Q_ASSERT(m_lineEdit->parentWidget() != 0);
        
        QStyleOption opt;
        opt.init(m_lineEdit);

        m_editor = new InPlaceEditor(m_lineEdit, ValidationSingleLine, m_formWindow,m_lineEdit->text(),opt.rect);

        connect(m_editor, SIGNAL(textChanged(QString)), this, SLOT(updateText(QString)));
    }
}

void LineEditTaskMenu::editIcon()
{
}

LineEditTaskMenuFactory::LineEditTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *LineEditTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(object)) {
        if (iid == Q_TYPEID(QDesignerTaskMenuExtension)) {
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

