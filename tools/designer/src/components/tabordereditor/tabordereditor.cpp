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

#include "tabordereditor.h"

#include <abstractformwindow.h>

#include <qtundo.h>
#include <qdesigner_command.h>

#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtCore/qdebug.h>
#include <QtGui/QRegion>
#include <QtGui/qevent.h>

TabOrderEditor::TabOrderEditor(AbstractFormWindow *form, QWidget *parent)
    : QWidget(parent)
{
    m_formWindow = form;
    m_bg_widget = 0;
    m_undo_stack = form->commandHistory();
    connect(form, SIGNAL(widgetRemoved(QWidget*)), this, SLOT(widgetRemoved(QWidget*)));
}

AbstractFormWindow *TabOrderEditor::formWindow() const
{
    return m_formWindow;
}

void TabOrderEditor::setBackground(QWidget *background)
{
    qDebug() << "TabOrderEditor::setBackground()";

    if (background == m_bg_widget) {
        return;
    }

    m_bg_widget = background;
    updateBackground();
}

void TabOrderEditor::updateBackground()
{
    if (m_bg_widget == 0) {
        // nothing to do
        return;
    }

    m_bg_pixmap = QPixmap::grabWidget(m_bg_widget);

    QPainter p(&m_bg_pixmap);
    p.setPen(QColor(0, 0, 255, 22));
    for (int y = 0; y < m_bg_pixmap.height(); y += 2)
        p.drawLine(0, y, m_bg_pixmap.width(), y);

    update();
}

void TabOrderEditor::widgetRemoved(QWidget*)
{
}

void TabOrderEditor::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setClipRegion(e->region());

    if (m_bg_pixmap.isNull())
        updateBackground();
    p.drawPixmap(m_bg_pixmap.rect(), m_bg_pixmap);
}

