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

#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtCore/qdebug.h>
#include <QtGui/QRegion>
#include <QtGui/qevent.h>
#include <QtGui/QFontMetrics>

#include <abstractformwindow.h>
#include <abstractformwindowcursor.h>
#include <abstractmetadatabase.h>
#include <abstractformeditor.h>

#include <qtundo.h>
#include <qdesigner_command.h>
#include <qdesigner_widget.h>

#define BG_ALPHA                32
#define VBOX_MARGIN             1
#define HBOX_MARGIN             4

static QRect fixRect(const QRect &r)
{
    return QRect(r.x(), r.y(), r.width() - 1, r.height() - 1);
}

static QRect expand(const QRect &r, int i)
{
    return QRect(r.x() - i, r.y() - i, r.width() + 2*i, r.height() + 2*i);
}

TabOrderEditor::TabOrderEditor(AbstractFormWindow *form, QWidget *parent)
    : QWidget(parent)
{
    m_form_window = form;
    m_bg_widget = 0;
    m_undo_stack = form->commandHistory();
    connect(form, SIGNAL(widgetRemoved(QWidget*)), this, SLOT(widgetRemoved(QWidget*)));
}

AbstractFormWindow *TabOrderEditor::formWindow() const
{
    return m_form_window;
}

void TabOrderEditor::setBackground(QWidget *background)
{
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

    initTabOrder();
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

    QFont font = p.font();
    font.setPointSize(font.pointSize()*2);
    font.setBold(true);
    p.setFont(font);
    QFontMetrics fm(font);
    
    for (int i = 0; i < m_tab_order_list.size(); ++i) {
        QWidget *w = m_tab_order_list.at(i);
        QPoint center = w->geometry().center();
        QString text = QString::number(i + 1);
        QSize size = fm.size(Qt::TextSingleLine, text);
        QRect r(center - QPoint(size.width(), size.height())/2, size);
        r = QRect(r.left() - HBOX_MARGIN, r.top() - VBOX_MARGIN,
                    r.width() + HBOX_MARGIN*2, r.height() + VBOX_MARGIN*2);
        
        QColor c = Qt::blue;
        p.setPen(c);
        c.setAlpha(BG_ALPHA);
        p.setBrush(c);
        p.drawRect(fixRect(r));

        p.setPen(Qt::white);
        p.drawText(r, text, QTextOption(Qt::AlignCenter));
    }
}

void TabOrderEditor::initTabOrder()
{
    m_tab_order_list.clear();

    AbstractFormEditor *core = formWindow()->core();

    if (AbstractMetaDataBaseItem *item = core->metaDataBase()->item(formWindow())) {
        m_tab_order_list = item->tabOrder();
    }
    
    AbstractFormWindowCursor *cursor = formWindow()->cursor();
    for (int i = 0; i < cursor->widgetCount(); ++i) {
        QWidget *widget = cursor->widget(i);
        if (qt_cast<QLayoutWidget*>(widget)
                || widget == formWindow()->mainContainer()
                || widget->isExplicitlyHidden())
            continue;

        if (!m_tab_order_list.contains(widget))
            m_tab_order_list.append(widget);            
    }
}
