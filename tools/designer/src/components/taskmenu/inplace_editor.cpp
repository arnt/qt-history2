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

#include "inplace_editor.h"

#include <QtGui/QResizeEvent>
#include <QtCore/qdebug.h>

InPlaceEditor::InPlaceEditor(QWidget *widget)
    : QLineEdit(),
      m_widget(widget)
{
    m_noChildEvent = widget->testAttribute(Qt::WA_NoChildEventsForParent);
    setParent(widget->window());
    m_widget->installEventFilter(this);
}

InPlaceEditor::~InPlaceEditor()
{
    m_widget->setAttribute(Qt::WA_NoChildEventsForParent, m_noChildEvent);
}

bool InPlaceEditor::eventFilter(QObject *object, QEvent *e)
{
    Q_ASSERT(object == m_widget);

    if (e->type() == QEvent::Resize) {
        QResizeEvent *event = static_cast<QResizeEvent*>(e);
        resize(event->size().width() - 2, height());
    }

    return false;
}

void InPlaceEditor::focusOutEvent(QFocusEvent *e)
{
    QLineEdit::focusOutEvent(e);
    deleteLater();
}

