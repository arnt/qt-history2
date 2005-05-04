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

#include "abstractformwindow.h"
#include "inplace_editor.h"

#include <QtGui/QResizeEvent>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtGui/QShortcut>

#include <QtCore/QMetaProperty>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

InPlaceEditor::InPlaceEditor(QWidget *widget, QDesignerFormWindowInterface *fw)
    : QLineEdit(),
      m_widget(widget)
{
    (void) new QShortcut(Qt::Key_Escape, this, SLOT(deleteLater()));

    m_noChildEvent = widget->testAttribute(Qt::WA_NoChildEventsForParent);
    setParent(widget->window());
    m_widget->installEventFilter(this);
    connect(this, SIGNAL(destroyed()), fw->mainContainer(), SLOT(setFocus()));

    if (m_widget->metaObject()->indexOfProperty("alignment") != -1) {
        Qt::Alignment alignment = Qt::Alignment(m_widget->property("alignment").toInt());
        setAlignment(alignment);
    } else if (qobject_cast<QPushButton *>(widget)
            || qobject_cast<QToolButton *>(widget) /* tool needs to be more complex */) {
        setAlignment(Qt::AlignHCenter);
    }
    // ### more ...
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

