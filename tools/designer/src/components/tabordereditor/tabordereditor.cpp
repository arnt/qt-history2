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
#include <qextensionmanager.h>
#include <propertysheet.h>

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

TabOrderEditor::TabOrderEditor(AbstractFormWindow *form, QWidget *parent)
    : QWidget(parent), m_font_metrics(font())
{
    m_form_window = form;
    m_bg_widget = 0;
    m_undo_stack = form->commandHistory();
    connect(form, SIGNAL(widgetRemoved(QWidget*)), this, SLOT(widgetRemoved(QWidget*)));

    QFont font = this->font();
    font.setPointSize(font.pointSize()*2);
    font.setBold(true);
    setFont(font);
    m_font_metrics = QFontMetrics(font);
    m_current_index = 0;
    
    setAttribute(Qt::WA_MouseTracking, true);
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

QRect TabOrderEditor::indicatorRect(int index) const
{
    if (index < 0 || index >= m_tab_order_list.size())
        return QRect();

    QWidget *w = m_tab_order_list.at(index);
    QString text = QString::number(index + 1);
    
    QPoint center = mapFromGlobal(w->mapToGlobal(w->rect().center()));
    QSize size = m_font_metrics.size(Qt::TextSingleLine, text);
    QRect r(center - QPoint(size.width(), size.height())/2, size);
    r = QRect(r.left() - HBOX_MARGIN, r.top() - VBOX_MARGIN,
                r.width() + HBOX_MARGIN*2, r.height() + VBOX_MARGIN*2);

    return r;
}

void TabOrderEditor::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setClipRegion(e->region());

    if (m_bg_pixmap.isNull())
        updateBackground();
    p.drawPixmap(m_bg_pixmap.rect(), m_bg_pixmap);
    
    for (int i = 0; i < m_tab_order_list.size(); ++i) {
        QRect r = indicatorRect(i);

        QColor c = Qt::blue;
        p.setPen(c);
        c.setAlpha(BG_ALPHA);
        p.setBrush(c);
        p.drawRect(fixRect(r));

        p.setPen(Qt::white);
        p.drawText(r, QString::number(i + 1), QTextOption(Qt::AlignCenter));
    }
}

bool TabOrderEditor::skipWidget(QWidget *w) const
{
    if (qt_cast<QLayoutWidget*>(w)
            || w == formWindow()->mainContainer()
            || w->isExplicitlyHidden())
        return true;

    QExtensionManager *ext = formWindow()->core()->extensionManager();
    if (IPropertySheet *sheet = qt_extension<IPropertySheet*>(ext, w)) {
        int index = sheet->indexOf("focusPolicy");
        if (index != -1) {
            bool ok = false;
            Qt::FocusPolicy q = (Qt::FocusPolicy) Utils::valueOf(sheet->property(index), &ok);
            return !ok || q == Qt::NoFocus;
        }
    }

    return true;
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
        if (skipWidget(widget))
            continue;

        if (!m_tab_order_list.contains(widget))
            m_tab_order_list.append(widget);            
    }

    m_indicator_region = QRegion();
    for (int i = 0; i < m_tab_order_list.size(); ++i)
        m_indicator_region |= indicatorRect(i);
}

void TabOrderEditor::mouseMoveEvent(QMouseEvent *e)
{
    e->accept();
    if (m_indicator_region.contains(e->pos()))
        setCursor(Qt::PointingHandCursor);
    else
        setCursor(QCursor());
}

int TabOrderEditor::widgetIndexAt(const QPoint &pos) const
{
    int target_index = -1;
    for (int i = 0; i < m_tab_order_list.size(); ++i) {
        if (indicatorRect(i).contains(pos)) {
            target_index = i;
            break;
        }
    }

    return target_index;
}

void TabOrderEditor::mousePressEvent(QMouseEvent *e)
{
    e->accept();
    if (!m_indicator_region.contains(e->pos()))
        return;

    int target_index = widgetIndexAt(e->pos());
    if (target_index == -1)
        return;

    update(indicatorRect(target_index));
    update(indicatorRect(m_current_index));
    m_tab_order_list.swap(target_index, m_current_index);
    update(indicatorRect(target_index));
    update(indicatorRect(m_current_index));
    
    ++m_current_index;
    if (m_current_index == m_tab_order_list.size())
        m_current_index = 0;

    TabOrderCommand *cmd = new TabOrderCommand(formWindow());
    cmd->init(m_tab_order_list);
    formWindow()->commandHistory()->push(cmd);
}

void TabOrderEditor::mouseDoubleClickEvent(QMouseEvent *e)
{
    m_current_index = 0;
    mousePressEvent(e);
}
