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

#include "formwindowcursor.h"
#include "formwindow.h"

// sdk
#include <QtDesigner/propertysheet.h>
#include <QtDesigner/QExtensionManager>
#include <qdesigner_command_p.h>

#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

FormWindowCursor::FormWindowCursor(FormWindow *fw, QObject *parent)
    : QObject(parent),
      m_formWindow(fw),
      m_iterator(0)
{
    update();
    connect(fw, SIGNAL(changed()), this, SLOT(update()));
}

FormWindowCursor::~FormWindowCursor()
{
}

QDesignerFormWindowInterface *FormWindowCursor::formWindow() const
{
    return m_formWindow;
}

bool FormWindowCursor::movePosition(MoveOperation op, MoveMode mode)
{
    if (widgetCount() == 0)
        return false;

    m_iterator = m_formWindow->widgets().indexOf(selectedWidget(0));

    if (mode == MoveAnchor)
        m_formWindow->clearSelection(false);

    switch (op) {
    case Next:
        ++m_iterator;
        if (m_iterator >= widgetCount())
            m_iterator = 0;

        m_formWindow->selectWidget(m_formWindow->widgetAt(m_iterator), true);
        return true;

    case Prev:
        --m_iterator;
        if (m_iterator < 0)
            m_iterator = widgetCount() - 1;

        if (m_iterator < 0)
            return false;

        m_formWindow->selectWidget(m_formWindow->widgetAt(m_iterator), true);
        return true;

    default:
        return false;
    }
}

int FormWindowCursor::position() const
{
    return m_iterator;
}

void FormWindowCursor::setPosition(int pos, MoveMode mode)
{
    if (!widgetCount())
        return;

    if (mode == MoveAnchor)
        m_formWindow->clearSelection(false);

    m_iterator = pos;
    if (m_iterator >= widgetCount())
        m_iterator = 0;

    m_formWindow->selectWidget(m_formWindow->widgetAt(m_iterator), true);
}

QWidget *FormWindowCursor::current() const
{
    if (m_iterator < widgetCount())
        return m_formWindow->widgetAt(m_iterator);
    return 0;
}

bool FormWindowCursor::hasSelection() const
{
    return !m_formWindow->selectedWidgets().isEmpty();
}

int FormWindowCursor::selectedWidgetCount() const
{
    int N = m_formWindow->selectedWidgets().count();
    return N ? N : 1;
}

QWidget *FormWindowCursor::selectedWidget(int index) const
{
    return hasSelection()
        ? m_formWindow->selectedWidgets().at(index)
        : m_formWindow->mainContainer();
}

void FormWindowCursor::update()
{
    // ### todo
}

int FormWindowCursor::widgetCount() const
{
    return m_formWindow->widgetCount();
}

QWidget *FormWindowCursor::widget(int index) const
{
    return m_formWindow->widgetAt(index);
}

void FormWindowCursor::setProperty(const QString &name, const QVariant &value)
{
    int N = selectedWidgetCount();
    Q_ASSERT(N);

    if (N > 1)
        m_formWindow->commandHistory()->push(new QtCommand(QtCommand::MacroBegin, tr("changed '%1'").arg(name)));

    for (int i=0; i<N; ++i) {
        QWidget *widget = selectedWidget(i);
        setWidgetProperty(widget, name, value);
    }

    if (N > 1)
        m_formWindow->commandHistory()->push(new QtCommand(QtCommand::MacroEnd));
}

void FormWindowCursor::setWidgetProperty(QWidget *widget, const QString &name, const QVariant &value)
{
    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(m_formWindow->core()->extensionManager(), widget);
    Q_ASSERT(sheet);

    SetPropertyCommand *cmd = new SetPropertyCommand(m_formWindow);
    cmd->init(widget, name, value);
    m_formWindow->commandHistory()->push(cmd);
}

void FormWindowCursor::resetWidgetProperty(QWidget *widget, const QString &name)
{
    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(m_formWindow->core()->extensionManager(), widget);
    Q_ASSERT(sheet);

    ResetPropertyCommand *cmd = new ResetPropertyCommand(m_formWindow);
    cmd->init(widget, name);
    m_formWindow->commandHistory()->push(cmd);
}
