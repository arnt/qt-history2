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

#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QPixmap>
#include <QMatrix>
#include <qdebug.h>

#include <abstractformwindow.h>
#include "connectionedit.h"
#include "qtundo.h"

#define BG_ALPHA                32
#define LINE_PROXIMITY_RADIUS   3
#define LOOP_MARGIN             20
#define VLABEL_MARGIN            1
#define HLABEL_MARGIN            3

/*******************************************************************************
** Tools
*/

static QRect fixRect(const QRect &r)
{
    return QRect(r.x(), r.y(), r.width() - 1, r.height() - 1);
}

static QRect expand(const QRect &r, int i)
{
    return QRect(r.x() - i, r.y() - i, r.width() + 2*i, r.height() + 2*i);
}

static QRect endPointRect(const QPoint &pos)
{
    QRect r(pos + QPoint(-LINE_PROXIMITY_RADIUS, -LINE_PROXIMITY_RADIUS),
            QSize(2*LINE_PROXIMITY_RADIUS, 2*LINE_PROXIMITY_RADIUS));
    return r;
}

static void paintEndPoint(QPainter *p, const QPoint &pos)
{
    QRect r(pos + QPoint(-LINE_PROXIMITY_RADIUS, -LINE_PROXIMITY_RADIUS),
            QSize(2*LINE_PROXIMITY_RADIUS, 2*LINE_PROXIMITY_RADIUS));
    p->fillRect(fixRect(r), p->pen().color());
}

static CETypes::LineDir classifyLine(const QPoint &p1, const QPoint &p2)
{
    if (p1.x() == p2.x())
        return p1.y() < p2.y() ? CETypes::DownDir : CETypes::UpDir;
    Q_ASSERT(p1.y() == p2.y());
    return p1.x() < p2.x() ? CETypes::RightDir : CETypes::LeftDir;
}

static QPoint pointInsideRect(const QRect &r, QPoint p)
{
    if (p.x() < r.left())
        p.setX(r.left());
    else if (p.x() > r.right())
        p.setX(r.right());
        
    if (p.y() < r.top())
        p.setY(r.top());
    else if (p.y() > r.bottom())
        p.setY(r.bottom());
    
    return p;
}

/*******************************************************************************
** Commands
*/

class CECommand : public QtCommand, public CETypes
{
    Q_OBJECT
public:
    CECommand(ConnectionEdit *edit) 
        : m_edit(edit) { setCanMerge(false); }
    ConnectionEdit *edit() const { return m_edit; }
private:
    ConnectionEdit *m_edit;
};

class AddConnectionCommand : public CECommand
{
    Q_OBJECT
public:
    AddConnectionCommand(ConnectionEdit *edit, Connection *con);
    virtual void redo();
    virtual void undo();
private:
    Connection *m_con;
};

AddConnectionCommand::AddConnectionCommand(ConnectionEdit *edit, Connection *con)
    : CECommand(edit), m_con(con)
{
    setDescription(tr("Add connection"));
}

void AddConnectionCommand::redo()
{
    edit()->selectNone();
    edit()->m_con_list.append(m_con);
    m_con->inserted();
    edit()->setSelected(m_con, true);
}

void AddConnectionCommand::undo()
{
    edit()->setSelected(m_con, false);
    m_con->update();
    m_con->removed();
    edit()->m_con_list.removeAll(m_con);
}

class AdjustConnectionCommand : public CECommand
{
    Q_OBJECT
public:
    AdjustConnectionCommand(ConnectionEdit *edit, Connection *con,
                            const QPoint &old_source_pos,
                            const QPoint &old_target_pos,
                            const QPoint &new_source_pos, 
                            const QPoint &new_target_pos);
    virtual void redo();
    virtual void undo();
private:
    Connection *m_con;
    QPoint m_old_source_pos, m_old_target_pos,
            m_new_source_pos, m_new_target_pos;
};

AdjustConnectionCommand::AdjustConnectionCommand(ConnectionEdit *edit, Connection *con, 
                                                    const QPoint &old_source_pos,
                                                    const QPoint &old_target_pos,
                                                    const QPoint &new_source_pos, 
                                                    const QPoint &new_target_pos)
    : CECommand(edit)
{
    setDescription(tr("Adjust connection"));
    m_con = con;
    m_old_source_pos = old_source_pos;
    m_old_target_pos = old_target_pos;
    m_new_source_pos = new_source_pos;
    m_new_target_pos = new_target_pos;
}

void AdjustConnectionCommand::undo()
{
    m_con->setEndPoint(EndPoint::Source, m_con->widget(EndPoint::Source), m_old_source_pos);
    m_con->setEndPoint(EndPoint::Target, m_con->widget(EndPoint::Target), m_old_target_pos);
}

void AdjustConnectionCommand::redo()
{
    m_con->setEndPoint(EndPoint::Source, m_con->widget(EndPoint::Source), m_new_source_pos);
    m_con->setEndPoint(EndPoint::Target, m_con->widget(EndPoint::Target), m_new_target_pos);
}

class DeleteConnectionsCommand : public CECommand
{
public:
    DeleteConnectionsCommand(ConnectionEdit *edit, const ConnectionList &con_list);
    virtual void redo();
    virtual void undo();
private:
    ConnectionList m_con_list;
};

DeleteConnectionsCommand::DeleteConnectionsCommand(ConnectionEdit *edit, 
                                                    const ConnectionList &con_list)
    : CECommand(edit), m_con_list(con_list)
{
   setDescription(tr("Delete connections"));
}

void DeleteConnectionsCommand::redo()
{
    foreach (Connection *con, m_con_list) {
        Q_ASSERT(edit()->m_con_list.contains(con));
        edit()->setSelected(con, false);
        con->update();
        con->removed();
        edit()->m_con_list.removeAll(con);
    }
}

void DeleteConnectionsCommand::undo()
{
    foreach (Connection *con, m_con_list) {
        Q_ASSERT(!edit()->m_con_list.contains(con));
        edit()->m_con_list.append(con);
        edit()->setSelected(con, true);
        con->update();
        con->inserted();
    }
}

/*******************************************************************************
** Connection
*/

Connection::Connection(ConnectionEdit *edit)
{
    m_edit = edit;
    m_source = 0;
    m_target = 0;
    
    m_source_pos = QPoint(-1, -1);
    m_target_pos = QPoint(-1, -1);
}

Connection::Connection(ConnectionEdit *edit, QWidget *source, QWidget *target)
{
    m_edit = edit;
    m_source = source;
    m_target = target;

    m_source_pos = QPoint(-1, -1);
    m_target_pos = QPoint(-1, -1);
}

QPoint Connection::endPointPos(EndPoint::Type type) const 
{
    if (type == EndPoint::Source)
        return m_source_pos;
    else
        return m_target_pos;
}


void Connection::setSource(QWidget *source, const QPoint &pos)
{
    if (source == m_source && m_source_pos == pos)
        return;

    update(false);
    
    m_source = source;
    m_source_pos = pos;
    m_source_rect = m_edit->widgetRect(source);
    updateKneeList();
    
    update(false);
}

static QPoint lineEntryPos(const QPoint &p1, const QPoint &p2, const QRect &rect)
{
    Q_ASSERT(!rect.contains(p1));
    Q_ASSERT(rect.contains(p2));

    QPoint result;
    
    CETypes::LineDir dir = classifyLine(p1, p2);
    switch (dir) {
        case CETypes::UpDir:
            result = QPoint(p1.x(), rect.bottom());
            break;
        case CETypes::DownDir:
            result = QPoint(p1.x(), rect.top());
            break;
        case CETypes::LeftDir:
            result = QPoint(rect.right(), p1.y());
            break;
        case CETypes::RightDir:
            result = QPoint(rect.left(), p1.y());
            break;
    }

    return result;
}

static QPolygonF arrowHead(const QPoint &p1, const QPoint &p2)
{
    QPolygonF result;

    CETypes::LineDir dir = classifyLine(p1, p2);
    switch (dir) {
        case CETypes::UpDir:
            result.append(p2 + QPoint(0, 1));
            result.append(p2 + QPoint(LINE_PROXIMITY_RADIUS, LINE_PROXIMITY_RADIUS*2 + 1));
            result.append(p2 + QPoint(-LINE_PROXIMITY_RADIUS, LINE_PROXIMITY_RADIUS*2 + 1));
            break;
        case CETypes::DownDir:
            result.append(p2);
            result.append(p2 + QPoint(LINE_PROXIMITY_RADIUS, -LINE_PROXIMITY_RADIUS*2));
            result.append(p2 + QPoint(-LINE_PROXIMITY_RADIUS, -LINE_PROXIMITY_RADIUS*2));
            break;
        case CETypes::LeftDir:
            result.append(p2 + QPoint(1, 0));
            result.append(p2 + QPoint(2*LINE_PROXIMITY_RADIUS + 1, -LINE_PROXIMITY_RADIUS));
            result.append(p2 + QPoint(2*LINE_PROXIMITY_RADIUS + 1, LINE_PROXIMITY_RADIUS));
            break;
        case CETypes::RightDir:
            result.append(p2);
            result.append(p2 + QPoint(-2*LINE_PROXIMITY_RADIUS, -LINE_PROXIMITY_RADIUS));
            result.append(p2 + QPoint(-2*LINE_PROXIMITY_RADIUS, LINE_PROXIMITY_RADIUS));
            break;
    }

    return result;
}

static CETypes::LineDir closestEdge(const QPoint &p, const QRect &r)
{
    Q_ASSERT(r.contains(p));

    CETypes::LineDir result = CETypes::UpDir;
    int min = p.y() - r.top();

    int d = p.x() - r.left();
    if (d < min) {
        min = d;
        result = CETypes::LeftDir;
    }

    d = r.bottom() - p.y();
    if (d < min) {
        min = d;
        result = CETypes::DownDir;
    }

    d = r.right() - p.x();
    if (d < min) {
        min = d;
        result = CETypes::RightDir;
    }

    return result;
}

void Connection::updateKneeList()
{
    LineDir old_source_label_dir = labelDir(EndPoint::Source);
    LineDir old_target_label_dir = labelDir(EndPoint::Target);
        
    QPoint s = endPointPos(EndPoint::Source);
    QPoint t = endPointPos(EndPoint::Target);
    QRect sr = m_source_rect;
    QRect tr = m_target_rect;
    
    m_knee_list.clear();
    m_arrow_head.clear();

    if (m_source == 0 || s == QPoint(-1, -1) || t == QPoint(-1, -1))
        return;

    QRect r = sr | tr;

    m_knee_list.append(s);
    if (m_target == 0) {
        m_knee_list.append(QPoint(t.x(), s.y()));
    } else if (tr.contains(sr) || sr.contains(tr)) {
        LineDir dir = closestEdge(t, tr);
        switch (dir) {
            case UpDir:
                m_knee_list.append(QPoint(s.x(), r.top() - LOOP_MARGIN));
                m_knee_list.append(QPoint(t.x(), r.top() - LOOP_MARGIN));
                break;
            case DownDir:
                m_knee_list.append(QPoint(s.x(), r.bottom() + LOOP_MARGIN));
                m_knee_list.append(QPoint(t.x(), r.bottom() + LOOP_MARGIN));
                break;
            case LeftDir:
                m_knee_list.append(QPoint(r.left() - LOOP_MARGIN, s.y()));
                m_knee_list.append(QPoint(r.left() - LOOP_MARGIN, t.y()));
                break;
            case RightDir:
                m_knee_list.append(QPoint(r.right() + LOOP_MARGIN, s.y()));
                m_knee_list.append(QPoint(r.right() + LOOP_MARGIN, t.y()));
                break;
        }
    } else {
        if (r.height() < sr.height() + tr.height()) {
            if (s.y() >= tr.top() && s.y() <= tr.bottom() || t.y() >= sr.bottom() || t.y() <= sr.top())
                m_knee_list.append(QPoint(s.x(), t.y()));
            else
                m_knee_list.append(QPoint(t.x(), s.y()));
        } else if (r.width() < sr.width() + tr.width()) {
            if (s.x() >= tr.left() && s.x() <= tr.right() || t.x() >= sr.right() || t.x() <= sr.left())
                m_knee_list.append(QPoint(t.x(), s.y()));
            else
                m_knee_list.append(QPoint(s.x(), t.y()));
        } else {
            if (sr.topLeft() == r.topLeft()) {
                if (tr.right() - t.x() < tr.bottom() - t.y())
                    m_knee_list.append(QPoint(t.x(), s.y()));
                else
                    m_knee_list.append(QPoint(s.x(), t.y()));
            } else if (sr.topRight() == r.topRight()) {
                if (t.x() - r.left() < r.bottom() - t.y())
                    m_knee_list.append(QPoint(t.x(), s.y()));
                else
                    m_knee_list.append(QPoint(s.x(), t.y()));
            } else if (sr.bottomRight() == r.bottomRight()) {
                if (t.x() - r.left() < t.y() - r.top())
                    m_knee_list.append(QPoint(t.x(), s.y()));
                else
                    m_knee_list.append(QPoint(s.x(), t.y()));
            } else {
                if (r.right() - t.x() < t.y() - r.top())
                    m_knee_list.append(QPoint(t.x(), s.y()));
                else
                    m_knee_list.append(QPoint(s.x(), t.y()));
            }
        }
    }
    m_knee_list.append(t);

    if (m_knee_list.size() == 2)
        m_knee_list.clear();
            
    trimLine();

    LineDir new_source_label_dir = labelDir(EndPoint::Source);
    LineDir new_target_label_dir = labelDir(EndPoint::Target);
    if (new_source_label_dir != old_source_label_dir)
        updatePixmap(EndPoint::Source);
    if (new_target_label_dir != old_target_label_dir)
        updatePixmap(EndPoint::Target);
}

void Connection::trimLine()
{
    if (m_source == 0 || m_source_pos == QPoint(-1, -1) || m_target_pos == QPoint(-1, -1))
        return;
    int cnt = m_knee_list.size();
    if (cnt < 2)
        return;
    
    QRect sr = m_source_rect;
    QRect tr = m_target_rect;

    if (sr.contains(m_knee_list.at(1)))
        m_knee_list.removeFirst();
    
    cnt = m_knee_list.size();
    if (cnt < 2)
        return;
    
    if (sr.contains(m_knee_list.at(0)) && !sr.contains(m_knee_list.at(1)))
        m_knee_list[0] = lineEntryPos(m_knee_list.at(1), m_knee_list.at(0), sr);

    if (tr.contains(m_knee_list.at(cnt - 1)) && !tr.contains(m_knee_list.at(cnt - 2))) {
        m_knee_list[cnt - 1]
            = lineEntryPos(m_knee_list.at(cnt - 2), m_knee_list.at(cnt - 1), tr);
        m_arrow_head = arrowHead(m_knee_list.at(cnt - 2), m_knee_list.at(cnt - 1));
    }
}

void Connection::setTarget(QWidget *target, const QPoint &pos)
{
    if (target == m_target && m_target_pos == pos)
        return;
    
    update(false);
    
    m_target = target; 
    m_target_pos = pos;
    m_target_rect = m_edit->widgetRect(target);
    updateKneeList();
    
    update(false);
}

static QRect lineRect(const QPoint &a, const QPoint &b)
{
    QPoint c(qMin(a.x(), b.x()), qMin(a.y(), b.y()));
    QPoint d(qMax(a.x(), b.x()), qMax(a.y(), b.y()));
    
    QRect result(c, d);
    return expand(result, LINE_PROXIMITY_RADIUS);
}

QRegion Connection::region() const
{
    QRegion result;
    
    for (int i = 0; i < m_knee_list.size() - 1; ++i)
        result = result.unite(lineRect(m_knee_list.at(i), m_knee_list.at(i + 1)));

    if (!m_arrow_head.isEmpty()) {
        QRect r = m_arrow_head.boundingRect().toRect();
        r = expand(r, 1);
        result = result.unite(r);
    }
    
    result = result.unite(labelRect(EndPoint::Source));
    result = result.unite(labelRect(EndPoint::Target));
    
    return result;
}

void Connection::update(bool update_widgets) const
{
    m_edit->update(region());
    if (update_widgets) {
        if (m_source != 0)
            m_edit->update(m_source_rect);
        if (m_target != 0)
            m_edit->update(m_target_rect);
    }
    
    m_edit->update(endPointRect(EndPoint::Source));
    m_edit->update(endPointRect(EndPoint::Target));
}

void Connection::paint(QPainter *p) const
{
    for (int i = 0; i < m_knee_list.size() - 1; ++i)
        p->drawLine(m_knee_list.at(i), m_knee_list.at(i + 1));
    
    if (!m_arrow_head.isEmpty()) {
        p->save();
        p->setBrush(p->pen().color());
        p->drawPolygon(m_arrow_head);
        p->restore();
    }    
}

bool Connection::contains(const QPoint &pos) const
{
    return region().contains(pos);
}

QRect Connection::endPointRect(EndPoint::Type type) const
{
    if (type == EndPoint::Source) {
        if (m_source_pos != QPoint(-1, -1))
            return ::endPointRect(m_source_pos);
    } else {
        if (m_target_pos != QPoint(-1, -1))
            return ::endPointRect(m_target_pos);
    }
    return QRect();
}

CETypes::LineDir Connection::labelDir(EndPoint::Type type) const
{
    int cnt = m_knee_list.size();
    if (cnt < 2)
        return RightDir;
    
    LineDir dir;
    if (type == EndPoint::Source)
        dir = classifyLine(m_knee_list.at(0), m_knee_list.at(1));
    else
        dir = classifyLine(m_knee_list.at(cnt - 2), m_knee_list.at(cnt - 1));
    
    if (dir == LeftDir)
        dir = RightDir;
    if (dir == UpDir)
        dir = DownDir;
        
    return dir;
}

QRect Connection::labelRect(EndPoint::Type type) const
{
    int cnt = m_knee_list.size();
    if (cnt < 2)
        return QRect();
    QString text = label(type);
    if (text.isEmpty())
        return QRect();
        
    QSize size = labelPixmap(type).size();
    QPoint p1, p2;
    if (type == EndPoint::Source) {
        p1 = m_knee_list.at(0);
        p2 = m_knee_list.at(1);
    } else {
        p1 = m_knee_list.at(cnt - 1);
        p2 = m_knee_list.at(cnt - 2);
    }
    LineDir dir = classifyLine(p1, p2);
            
    QRect result;
    switch (dir) {
        case UpDir:
            result = QRect(p1 + QPoint(-size.width()/2, 0), size);
            break;
        case DownDir:
            result = QRect(p1 + QPoint(-size.width()/2, -size.height()), size);
            break;
        case LeftDir:
            result = QRect(p1 + QPoint(0, -size.height()/2), size);
            break;
        case RightDir:
            result = QRect(p1 + QPoint(-size.width(), -size.height()/2), size);
            break;
    }
    
    return result;
}

void Connection::setLabel(EndPoint::Type type, const QString &text)
{ 
    if (text == label(type))
        return;

    if (type == EndPoint::Source)
        m_source_label = text;
    else
        m_target_label = text;
        
    updatePixmap(type);
}

void Connection::updatePixmap(EndPoint::Type type)
{
    QPixmap *pm = type == EndPoint::Source ? &m_source_label_pm : &m_target_label_pm;
    *pm = QPixmap();
    
    QString text = label(type);
    if (text.isEmpty())
        return;
    
    QFontMetrics fm = m_edit->fontMetrics();
    QSize size = fm.size(Qt::TextSingleLine, text) + QSize(HLABEL_MARGIN*2, VLABEL_MARGIN*2);
    pm->resize(size);
    pm->fill(m_edit->palette().color(QPalette::Base));
    
    QPainter p(pm);
    p.setPen(m_edit->palette().color(QPalette::Text));
    p.drawText(-fm.leftBearing(text.at(0)) + HLABEL_MARGIN, fm.ascent() + VLABEL_MARGIN, text);
    p.end();
    
    LineDir dir = labelDir(type);
    
    if (dir == DownDir)
        *pm = pm->transform(QMatrix(0.0, -1.0, 1.0, 0.0, 0.0, 0.0));
}

void Connection::checkWidgets()
{
    bool changed = false;

    if (m_source != 0) {
        QRect r = m_edit->widgetRect(m_source);
        if (r != m_source_rect) {
            m_edit->update(m_source_rect);
            m_source_rect = r;
            if (m_source_pos != QPoint(-1, -1))
                m_source_pos = pointInsideRect(m_source_rect, m_source_pos);
            changed = true;
        }
    }

    if (m_target != 0) {
        QRect r = m_edit->widgetRect(m_target);
        if (r != m_target_rect) {
            m_edit->update(m_target_rect);
            m_target_rect = r;
            if (m_target_pos != QPoint(-1, -1))
                m_target_pos = pointInsideRect(m_target_rect, m_target_pos);
            changed = true;
        }
    }

    if (changed) {
        update();
        updateKneeList();
        update();
    }
}

/*******************************************************************************
** ConnectionEdit
*/

ConnectionEdit::ConnectionEdit(QWidget *parent, AbstractFormWindow *form)
    : QWidget(parent)
{
    m_bg_widget = 0;
    m_widget_under_mouse = 0;
    m_tmp_con = 0;
    m_start_connection_on_drag = true;
    m_undo_stack = form->commandHistory();
    m_active_color = Qt::red;
    m_inactive_color = Qt::blue;
    setAttribute(Qt::WA_MouseTracking, true);
    setFocusPolicy(Qt::ClickFocus);

    connect(form, SIGNAL(widgetsChanged()), this, SLOT(updateBackground()));
    connect(form, SIGNAL(widgetRemoved(QWidget*)), this, SLOT(widgetRemoved(QWidget*)));
}


void ConnectionEdit::clear()
{
    m_con_list.clear();
    m_sel_con_set.clear();
    m_bg_widget = 0;
    m_widget_under_mouse = 0;
    m_tmp_con = 0;
}

void ConnectionEdit::setBackground(QWidget *background)
{
    m_bg_widget = background; 
    updateBackground(); 
}
    
void ConnectionEdit::updateBackground()
{    
    if (m_bg_widget != 0) {
        m_bg_pixmap = QPixmap::grabWidget(m_bg_widget);
        updateLines();
        update();
    }
}

QWidget *ConnectionEdit::widgetAt(const QPoint &pos) const
{
    if (m_bg_widget == 0)
        return 0;
    QWidget *widget = m_bg_widget->childAt(pos);
    if (widget == 0)
        widget = m_bg_widget;
    return widget;
}


QRect ConnectionEdit::widgetRect(QWidget *w) const
{
    if (w == 0)
        return QRect();
    QRect r = w->geometry();
    QPoint pos = w->mapToGlobal(QPoint(0, 0));
    pos = mapFromGlobal(pos);
    r.moveTopLeft(pos);
    return r;
}

ConnectionEdit::State ConnectionEdit::state() const
{
    if (m_tmp_con != 0)
        return Connecting;
    if (!m_drag_end_point.isNull())
        return Dragging;
    return Editing;
}

void ConnectionEdit::paintLabel(QPainter *p, EndPoint::Type type, Connection *con)
{
    if (con->label(type).isEmpty())
        return;

    bool heavy = selected(con) || con == m_tmp_con;
    p->setPen(heavy ? m_active_color : m_inactive_color);
    p->setBrush(Qt::NoBrush);
    QRect r = con->labelRect(type);
    p->drawPixmap(r.topLeft(), con->labelPixmap(type));
    p->drawRect(fixRect(r));
}

void ConnectionEdit::paintConnection(QPainter *p, Connection *con,
                                        WidgetSet *heavy_highlight_set,
                                        WidgetSet *light_highlight_set) const
{
    bool heavy = selected(con) || con == m_tmp_con;
    p->setPen(heavy ? m_active_color : m_inactive_color);
    con->paint(p);

    WidgetSet *set = heavy ? heavy_highlight_set : light_highlight_set;
    QWidget *source = con->widget(EndPoint::Source);
    QWidget *target = con->widget(EndPoint::Target);
    if (source != 0)
        set->insert(source, source);
    if (target != 0)
        set->insert(target, target);
}

void ConnectionEdit::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setClipRegion(e->region());

    if (m_bg_pixmap.isNull())
        updateBackground();
    p.drawPixmap(m_bg_pixmap.rect(), m_bg_pixmap);

    WidgetSet heavy_highlight_set, light_highlight_set;

    foreach (Connection *con, m_con_list)
        paintConnection(&p, con, &heavy_highlight_set, &light_highlight_set);
    if (m_tmp_con != 0)
        paintConnection(&p, m_tmp_con, &heavy_highlight_set, &light_highlight_set);

    if (m_widget_under_mouse != 0)
        heavy_highlight_set.insert(m_widget_under_mouse, m_widget_under_mouse);

    QColor c = m_active_color;
    p.setPen(c);
    c.setAlpha(BG_ALPHA);
    p.setBrush(c);
    foreach (QWidget *w, heavy_highlight_set) {
        p.drawRect(fixRect(widgetRect(w)));
        light_highlight_set.remove(w);
    }

    c = m_inactive_color;
    p.setPen(c);
    c.setAlpha(BG_ALPHA);
    p.setBrush(c);
    foreach (QWidget *w, light_highlight_set)
        p.drawRect(fixRect(widgetRect(w)));

    p.setBrush(palette().color(QPalette::Base));
    p.setPen(palette().color(QPalette::Text));
    foreach (Connection *con, m_con_list) {
        paintLabel(&p, EndPoint::Source, con);
        paintLabel(&p, EndPoint::Target, con);
    }

    p.setPen(m_active_color);
    p.setBrush(m_active_color);
    foreach (Connection *con, m_con_list) {
        if (!selected(con))
            continue;
        paintEndPoint(&p, con->endPointPos(EndPoint::Source));
        if (con->widget(EndPoint::Target) != 0)
            paintEndPoint(&p, con->endPointPos(EndPoint::Target));
    }
}

void ConnectionEdit::mousePressEvent(QMouseEvent *e)
{
    Connection *con_under_mouse = connectionAt(e->pos());
    m_start_connection_on_drag = false;
    
    switch (state()) {
        case Connecting:
        case Dragging:
            break;
        case Editing:
            if (!m_end_point_under_mouse.isNull()) {
                if (!(e->modifiers() & Qt::ShiftModifier)) {
                    startDrag(m_end_point_under_mouse, e->pos());
                }
            } else if (con_under_mouse != 0) {
                if (!(e->modifiers() & Qt::ShiftModifier)) {
                    selectNone();
                    setSelected(con_under_mouse, true);
                } else {
                    setSelected(con_under_mouse, !selected(con_under_mouse));
                }
            } else {
                if (!(e->modifiers() & Qt::ShiftModifier)) {
                    selectNone();
                    if (m_widget_under_mouse != 0)
                        m_start_connection_on_drag = true;
                }
            }
            break;
    }
    
    e->accept();
}

void ConnectionEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
    switch (state()) {
        case Connecting:
            break;
        case Dragging:
            break;
        case Editing:
            if (m_sel_con_set.size() == 1) {
                Connection *con = m_sel_con_set.keys().first();
                modifyConnection(con);
            }
            break;
    }
    
}

void ConnectionEdit::mouseReleaseEvent(QMouseEvent *e)
{
    switch (state()) {
        case Connecting:
            if (m_widget_under_mouse == 0) {
                m_tmp_con->update();
                delete m_tmp_con;
                m_tmp_con = 0;
            } else {
                endConnection(m_widget_under_mouse, e->pos());
            }    
            setCursor(QCursor());
            break;
        case Editing:
            break;
        case Dragging:
            endDrag(e->pos());
            break;
    }
        
    e->accept();
}

void ConnectionEdit::findObjectsUnderMouse(const QPoint &pos)
{
    Connection *con_under_mouse = connectionAt(pos);
    QWidget *w = con_under_mouse != 0 ? 0 : widgetAt(pos);
    if (w == m_bg_widget)
        w = 0;
    if (w != m_widget_under_mouse) {
        if (m_widget_under_mouse != 0)
            update(widgetRect(m_widget_under_mouse));
        m_widget_under_mouse = w;
        if (m_widget_under_mouse != 0)
            update(widgetRect(m_widget_under_mouse));
    }

    EndPoint hs = endPointAt(pos);
    if (hs != m_end_point_under_mouse) {
        if (m_end_point_under_mouse.isNull())
            setCursor(Qt::PointingHandCursor);
        else
            setCursor(QCursor());
        m_end_point_under_mouse = hs;
    }
    
}

void ConnectionEdit::mouseMoveEvent(QMouseEvent *e)
{
    findObjectsUnderMouse(e->pos());

    switch (state()) {
        case Connecting:
            continueConnection(m_widget_under_mouse, e->pos());
            break;
        case Editing:
            if ((e->buttons() & Qt::LeftButton)
                    && m_start_connection_on_drag
                    && m_widget_under_mouse != 0) { 
                m_start_connection_on_drag = false;
                startConnection(m_widget_under_mouse, e->pos());
                setCursor(Qt::CrossCursor);
            }
            break;
        case Dragging:
            continueDrag(e->pos());
            break;
    }
        
    e->accept();
}

void ConnectionEdit::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_Delete:
            if (state() == Editing)
                deleteSelected();
            break;
    }
    
    e->accept();
}

void ConnectionEdit::startConnection(QWidget *source, const QPoint &pos)
{
    Q_ASSERT(m_tmp_con == 0);
    
    m_tmp_con = new Connection(this);
    m_tmp_con->setEndPoint(EndPoint::Source, source, pos);
}

void ConnectionEdit::endConnection(QWidget *target, const QPoint &pos)
{
    Q_ASSERT(m_tmp_con != 0);
    
    m_tmp_con->setEndPoint(EndPoint::Target, target, pos);
    
    QWidget *source = m_tmp_con->widget(EndPoint::Source);
    Q_ASSERT(source != 0);
    Q_ASSERT(target != 0);
    Connection *new_con = createConnection(source, target);
    if (new_con != 0) {
        new_con->setEndPoint(EndPoint::Source, source, m_tmp_con->endPointPos(EndPoint::Source));
        new_con->setEndPoint(EndPoint::Target, target, m_tmp_con->endPointPos(EndPoint::Target));
        m_undo_stack->push(new AddConnectionCommand(this, new_con));
    }
    findObjectsUnderMouse(mapFromGlobal(QCursor::pos()));
            
    delete m_tmp_con;
    m_tmp_con = 0;
}

void ConnectionEdit::continueConnection(QWidget *target, const QPoint &pos)
{
    Q_ASSERT(m_tmp_con != 0);

    m_tmp_con->setEndPoint(EndPoint::Target, target, pos);    
}
    
void ConnectionEdit::modifyConnection(Connection *)
{
}

Connection *ConnectionEdit::createConnection(QWidget *source, QWidget *target)
{
    Connection *con = new Connection(this, source, target);
    return con;
}

void ConnectionEdit::widgetRemoved(QWidget *widget)
{
    QList<QWidget*> child_list = qFindChildren<QWidget*>(widget);
    child_list.prepend(widget);
    
    ConnectionSet remove_set;
    foreach (QWidget *w, child_list) {
        foreach (Connection *con, m_con_list) {
            if (con->widget(EndPoint::Source) == w || con->widget(EndPoint::Target) == w)
                remove_set.insert(con, con);
        }
    }
                    
    if (!remove_set.isEmpty())
        m_undo_stack->push(new DeleteConnectionsCommand(this, remove_set.keys()));
    
    updateBackground();
}

void ConnectionEdit::setSelected(Connection *con, bool sel)
{
    if (sel == m_sel_con_set.contains(con))
        return;

    if (sel)
        m_sel_con_set.insert(con, con);
    else
        m_sel_con_set.remove(con);
    
    con->update();
}
    
bool ConnectionEdit::selected(const Connection *con) const
{
    return m_sel_con_set.contains(const_cast<Connection*>(con));
}

void ConnectionEdit::selectNone()
{
    foreach (Connection *con, m_sel_con_set)
        con->update();

    m_sel_con_set.clear();
}

Connection *ConnectionEdit::connectionAt(const QPoint &pos) const
{
    foreach (Connection *con, m_con_list) {
        if (con->contains(pos))
            return con;
    }
    return 0;
}

CETypes::EndPoint ConnectionEdit::endPointAt(const QPoint &pos) const
{
    foreach (Connection *con, m_con_list) {
        if (!selected(con))
            continue;
        QRect sr = con->endPointRect(EndPoint::Source);
        QRect tr = con->endPointRect(EndPoint::Target);
        
        if (sr.contains(pos))
            return EndPoint(con, EndPoint::Source);
        if (tr.contains(pos))
            return EndPoint(con, EndPoint::Target);
    }
    return EndPoint();
}

void ConnectionEdit::startDrag(const EndPoint &end_point, const QPoint &pos)
{
    Q_ASSERT(m_drag_end_point.isNull());
    m_drag_end_point = end_point;
    m_old_source_pos = m_drag_end_point.con->endPointPos(EndPoint::Source);
    m_old_target_pos = m_drag_end_point.con->endPointPos(EndPoint::Target);
    adjustHotSopt(m_drag_end_point, pos);
}

void ConnectionEdit::continueDrag(const QPoint &pos)
{
    Q_ASSERT(!m_drag_end_point.isNull());
    adjustHotSopt(m_drag_end_point, pos);
}

void ConnectionEdit::endDrag(const QPoint &pos)
{
    Q_ASSERT(!m_drag_end_point.isNull());
    adjustHotSopt(m_drag_end_point, pos);
    
    Connection *con = m_drag_end_point.con;
    QPoint new_source_pos = con->endPointPos(EndPoint::Source);
    QPoint new_target_pos = con->endPointPos(EndPoint::Target);
    m_undo_stack->push(new AdjustConnectionCommand(this, con, m_old_source_pos, m_old_target_pos,
                                                    new_source_pos, new_target_pos));
            
    m_drag_end_point = EndPoint();
}

void ConnectionEdit::adjustHotSopt(const EndPoint &end_point, const QPoint &pos)
{
    QWidget *w = end_point.con->widget(end_point.type);
    end_point.con->setEndPoint(end_point.type, w, pointInsideRect(widgetRect(w), pos));
}

void ConnectionEdit::deleteSelected()
{
    m_undo_stack->push(new DeleteConnectionsCommand(this, m_sel_con_set.keys()));
}

void ConnectionEdit::addConnection(Connection *con)
{
    m_con_list.append(con);
}

void ConnectionEdit::updateLines()
{
    foreach (Connection *con, m_con_list)
        con->checkWidgets();
}

#include "connectionedit.moc"
