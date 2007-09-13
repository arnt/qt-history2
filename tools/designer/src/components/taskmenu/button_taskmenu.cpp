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
TRANSLATOR qdesigner_internal::ButtonTaskMenu
*/

#include "button_taskmenu.h"
#include "inplace_editor.h"

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormWindowCursorInterface>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

ButtonTaskMenu::ButtonTaskMenu(QAbstractButton *button, QObject *parent)
    : QDesignerTaskMenu(button, parent),
      m_button(button),
      m_preferredEditAction(new QAction(tr("Change text..."), this))
{
    connect(m_preferredEditAction, SIGNAL(triggered()), this, SLOT(editText()));
    m_taskActions.append(m_preferredEditAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

ButtonTaskMenu::~ButtonTaskMenu()
{
}

QAction *ButtonTaskMenu::preferredEditAction() const
{
    return m_preferredEditAction;
}

QList<QAction*> ButtonTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void ButtonTaskMenu::editText()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_button);
    if (!m_formWindow.isNull()) {
        connect(m_formWindow, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
        Q_ASSERT(m_button->parentWidget() != 0);
        
        QStyleOptionButton opt;
        opt.init(m_button);
        const QRect r = m_button->style()->subElementRect(QStyle::SE_PushButtonContents, &opt, m_button);

        m_editor = new InPlaceEditor(m_button, ValidationMultiLine, m_formWindow,m_button->text(),r);

        connect(m_editor, SIGNAL(textChanged(QString)), this, SLOT(updateText(QString)));
    }
}

void ButtonTaskMenu::editIcon()
{
}

void ButtonTaskMenu::updateText(const QString &text)
{
    m_formWindow->cursor()->setProperty(QLatin1String("text"), QVariant(text));
}

void ButtonTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

QT_END_NAMESPACE
