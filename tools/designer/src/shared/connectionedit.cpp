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

#include "connectionedit.h"

#include <QMouseEvent>
#include <QPainter>
#include <QApplication>

#include <math.h>

#define LOOP_MARGIN 30
#define SELECTION_ALPHA 32
#define ENDPOINT_RADIUS 3

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static QRect expand(const QRect &r, int i)
{
    return QRect(r.x() - i, r.y() - i, r.width() + 2*i, r.height() + 2*i);
}

static QRect fixRect(const QRect &r)
{
    return QRect(r.x(), r.y(), r.width() - 1, r.height() - 1);
}

static double sqr(double x)
{
    return x*x;
}

// #include <algorithm> and use std::swap
/*
template <typename T>
static void swap(T &t1, T &t2)
{
    T tmp = t1;
    t1 = t2;
    t2 = tmp;
}
*/

class SignalDumper : public QObject
{
    Q_OBJECT
    public:
        SignalDumper(ConnectionEdit *edit);
    public slots:
        inline void added(Connection *c) { qDebug() << "SignalDumper::added()" << c; }
        inline void aboutToRemove(Connection *c) { qDebug() << "SignalDumper::aboutToRemove()" << c; }
        inline void selected(Connection *c) { qDebug() << "SignalDumper::selected()" << c; }
        inline void activated(Connection *c) { qDebug() << "SignalDumper::activated()" << c; }
};

SignalDumper::SignalDumper(ConnectionEdit *edit)
    : QObject(edit)
{
    connect(edit, SIGNAL(added(Connection*)), this, SLOT(added(Connection*)));
    connect(edit, SIGNAL(aboutToRemove(Connection*)), this, SLOT(aboutToRemove(Connection*)));
    connect(edit, SIGNAL(selected(Connection*)), this, SLOT(selected(Connection*)));
    connect(edit, SIGNAL(activated(Connection*)), this, SLOT(activated(Connection*)));
}

static CEEdgeItem *prevEdge(CEEdgeItem *edge)
{
    CEEndPointItem *ep = edge->endPoint1();
    if (ep == 0)
        return 0;
    return ep->otherEdge(edge);
}

static CEEdgeItem *nextEdge(CEEdgeItem *edge)
{
    CEEndPointItem *ep = edge->endPoint2();
    if (ep == 0)
        return 0;
    return ep->otherEdge(edge);
}

static CEEdgeItem *firstEdge(CEEdgeItem *edge)
{
    while (edge != 0) {
        CEEdgeItem *tmp = prevEdge(edge);
        if (tmp == 0)
            break;
        edge = tmp;
    }
    return edge;
}

static CEEdgeItem *lastEdge(CEEdgeItem *edge)
{
    while (edge != 0) {
        CEEdgeItem *tmp = nextEdge(edge);
        if (tmp == 0)
            break;
        edge = tmp;
    }
    return edge;
}

static CEEndPointItem *nextEndPoint(CEEndPointItem *item)
{
    CEEdgeItem *edge = item->destinationEdge();
    if (edge == 0)
        return 0;
    return edge->endPoint2();
}

static CEEndPointItem *prevEndPoint(CEEndPointItem *item)
{
    CEEdgeItem *edge = item->sourceEdge();
    if (edge == 0)
        return 0;
    return edge->endPoint1();
}

static CEEndPointItem *firstEndPoint(CEEndPointItem *item)
{
    for (;;) {
        if (item == 0)
            return 0;
        if (item->type() == CEItem::WidgetItem)
            break;
        item = prevEndPoint(item);
    }
    return item;
}

static CEEndPointItem *lastEndPoint(CEEndPointItem *item)
{
    for (;;) {
        if (item == 0)
            return 0;
        if (item->type() == CEItem::WidgetItem)
            break;
        item = nextEndPoint(item);
    }
    return item;
}

/*******************************************************************************
** CEItem
*/

CEItem::CEItem(ConnectionEdit *edit)
    : QObject(edit)
{
    m_visible = true;
    m_disable_select = false;
}

void CEItem::paint(QPainter *p)
{
    Q_UNUSED(p);
#if 0
    p->save();
    p->setClipping(false);
    QPoint pos = rect().center();
    QFontMetrics fm = edit()->fontMetrics();
    QString text = "0x" + QString::number((uint)this, 16);

    p->setPen(Qt::black);
    p->setBrush(Qt::black);
    p->drawRect(QRect(pos, fm.size(Qt::TextSingleLine, text)));
    p->setPen(Qt::white);
    p->drawText(pos + QPoint(0, fm.ascent()), text);
    p->restore();
#endif
}

QColor CEItem::colorForStatus()
{
    QColor color;

    switch (status()) {
        case Dragged:
            return Qt::red;
        case New:
            return Qt::red;
        case Selected:
            return Qt::red;
        case UnderMouse:
            return Qt::red;
        case Normal:
            return Qt::blue;
    }

    return Qt::blue;
}

void CEItem::update() const
{
    edit()->update(rect());
}

CEItem::Status CEItem::status() const
{
    if (edit()->draggedItem() == this)
        return Dragged;
    if (edit()->isNew(this))
        return New;
    if (edit()->isSelected(this))
        return Selected;
    if (edit()->isUnderMouse(this))
        return UnderMouse;
    return Normal;
}

void CEItem::setVisible(bool b)
{
    if (m_visible == b)
        return;
    m_visible = b;
    update();
}


/*******************************************************************************
** CELabelItem
*/

CELabelItem::CELabelItem(ConnectionEdit *edit)
    : CEItem(edit)
{
}

void CELabelItem::move(const QPoint &delta)
{
    update();

    QPoint new_pos = m_rect.topLeft() + delta;

    QRect constraint_rect(m_anchor_pos.x() - m_rect.width()*3/2, m_anchor_pos.y() - m_rect.height()*3/2,
                            m_rect.width()*2, m_rect.height()*2);
    if (new_pos.x() < constraint_rect.left())
        new_pos.setX(constraint_rect.left());
    if (new_pos.x() > constraint_rect.right())
        new_pos.setX(constraint_rect.right());
    if (new_pos.y() < constraint_rect.top())
        new_pos.setY(constraint_rect.top());
    if (new_pos.y() > constraint_rect.bottom())
        new_pos.setY(constraint_rect.bottom());

    m_rect.moveTopLeft(new_pos);

    update();
    emit moved();
}

void CELabelItem::setAnchorPos(const QPoint &pos)
{
    QPoint delta = pos - m_anchor_pos;

    update();
    m_rect.moveTopLeft(m_rect.topLeft() + pos - m_anchor_pos);
    update();

    m_anchor_pos = pos;
}

void CELabelItem::setText(const QString &text)
{
    m_text = text;

    update();
    QFontMetrics fm = edit()->fontMetrics();
    m_rect = QRect(m_rect.topLeft(), fm.size(Qt::TextSingleLine, text));
    m_rect = expand(m_rect, 2);
    update();
}

void CELabelItem::paint(QPainter *p)
{
    p->save();
    p->setPen(colorForStatus());
    p->setBrush(Qt::white);
    p->drawRect(fixRect(m_rect));
    QFontMetrics fm = edit()->fontMetrics();
    p->setPen(Qt::black);
    p->drawText(m_rect.topLeft() + QPoint(2, fm.ascent() + 2), m_text);
    p->restore();
    CEItem::paint(p);
}

/*******************************************************************************
** CEEndPointItem
*/

class EndPointLayout1D
{
public:
    EndPointLayout1D();
    void setPos(int a1, int a2, int b, int c1, int c2);
    int getPos(int a1, int a2, int c1, int c2);
    
private:
    enum Mode { OffsetFromMin, OffsetFromMax, Ratio };
    Mode m_mode;
    int m_offset;
    double m_ratio;
};

class EndPointLayout
{
public:
    EndPointLayout();
    void setPos(const QRect &r1, const QPoint &p, const QRect &r2);
    QPoint getPos(const QRect &r1, const QRect &r2);

private:
    enum Mode { InR1, InR2, Layout };
    Mode m_mode;
    QPoint m_offset;
    EndPointLayout1D m_layout_hor, m_layout_ver;
};

EndPointLayout1D::EndPointLayout1D()
{
    m_mode = OffsetFromMin;
    m_offset = 0;
}

void EndPointLayout1D::setPos(int a1, int a2, int b, int c1, int c2)
{
    int min = qMin(a1, c1);
    int max = qMax(a2, c2);
    
    if (b <= min) {
        m_mode = OffsetFromMin;
        m_offset = b - min;
    } else if (b >= max) {
        m_mode = OffsetFromMax;
        m_offset = b - max;
    } else if (min == max) {
        m_mode = Ratio;
        m_ratio = 0.5;
    } else {
        m_mode = Ratio;
        m_ratio = (b - min + 0.0)/(max - min);
    }
}

int EndPointLayout1D::getPos(int a1, int a2, int c1, int c2)
{
    int min = qMin(a1, c1);
    int max = qMax(a2, c2);
    
    int result;
    
    switch (m_mode) {
        case OffsetFromMin:
            result = min + m_offset;
            break;
        case OffsetFromMax:
            result = max + m_offset;
            break;
        case Ratio:
            result = min + (int)(m_ratio*(max - min));
            break;
    }
    
    return result;
}

EndPointLayout::EndPointLayout()
{
    m_mode = InR1;
    m_offset = QPoint(0, 0);
}

void EndPointLayout::setPos(const QRect &r1, const QPoint &p, const QRect &r2)
{
    if (r1.contains(p)) {
        m_mode = InR1;
        m_offset = p - r1.topLeft();
    } else if (r2.contains(p)) {
        m_mode = InR2;
        m_offset = p - r2.topLeft();
    } else {
        m_mode = Layout;
        m_layout_hor.setPos(r1.left(), r1.right(), p.x(), r2.left(), r2.right());
        m_layout_ver.setPos(r1.top(), r1.bottom(), p.y(), r2.top(), r2.bottom());
    }
}

QPoint EndPointLayout::getPos(const QRect &r1, const QRect &r2)
{
    QPoint result;
    
    switch (m_mode) {
        case InR1:
            result = r1.topLeft() + m_offset;
            break;
        case InR2:
            result = r2.topLeft() + m_offset;
            break;
        case Layout: {
            int x = m_layout_hor.getPos(r1.left(), r1.right(), r2.left(), r2.right());
            int y = m_layout_ver.getPos(r1.top(), r1.bottom(), r2.top(), r2.bottom());
            result = QPoint(x, y);
            break;
        }
    }

    return result;
}

CEEndPointItem::CEEndPointItem(const QPoint &pos, ConnectionEdit *edit)
    : CEItem(edit), m_pos(pos), m_layout(new EndPointLayout) 
{}

CEEndPointItem::~CEEndPointItem()
{
    delete m_layout;
}

void CEEndPointItem::paint(QPainter *p)
{
    CEItem::paint(p);

    if (status() == Normal)
        return;

    QColor c = colorForStatus();
    p->fillRect(rect(), c);
}

void CEEndPointItem::move(const QPoint &delta)
{
    update();

    m_pos += delta;
    adjustLayout();
    update();
    emit moved();
}

void CEEndPointItem::adjustLayout()
{
    CEEndPointItem *first = firstEndPoint(this);
    CEEndPointItem *last = lastEndPoint(this);
    if (first == 0 || last == 0)
        return;
    
    m_layout->setPos(first->rect(), m_pos, last->rect());
}

void CEEndPointItem::adjustPos()
{
    CEEndPointItem *first = firstEndPoint(this);
    CEEndPointItem *last = lastEndPoint(this);
    if (first == 0 || last == 0)
        return;

    QPoint pos = m_layout->getPos(first->rect(), last->rect());
    if (pos == m_pos)
        return;

    update();
    m_pos = pos;
    update();
    emit moved();
}

void CEEndPointItem::addEdge(CEEdgeItem *edge)
{
    m_edge_list.append(edge);
    connect(edge, SIGNAL(destroyed(QObject*)), this, SLOT(edgeDestroyed(QObject*)));
}

void CEEndPointItem::edgeDestroyed(QObject *o)
{
    m_edge_list.removeAll(static_cast<CEEdgeItem*>(o));
}

CEEdgeItem *CEEndPointItem::edgeTo(CEEndPointItem *other) const
{
    foreach (CEEdgeItem *edge, m_edge_list) {
        if (edge->otherEndPoint(this) == other)
            return edge;
    }
    return 0;
}

QRect CEEndPointItem::rect() const
{
    return QRect(m_pos.x() - ENDPOINT_RADIUS, m_pos.y() - ENDPOINT_RADIUS,
                    ENDPOINT_RADIUS*2, ENDPOINT_RADIUS*2);
}

CEEdgeItem *CEEndPointItem::sourceEdge() const
{
    if (type() == WidgetItem)
        return 0;

    if (edgeCount() != 2)
        return 0;

    if (edge(0)->endPoint2() == this)
        return edge(0);
    Q_ASSERT(edge(1)->endPoint2() == this);
    return edge(1);
}

CEEdgeItem *CEEndPointItem::destinationEdge() const
{
    if (type() == WidgetItem)
        return 0;

    if (edgeCount() != 2)
        return 0;

    if (edge(0)->endPoint1() == this)
        return edge(0);
    Q_ASSERT(edge(1)->endPoint1() == this);
    return edge(1);
}

CEEdgeItem *CEEndPointItem::otherEdge(const CEEdgeItem *e) const
{
    if (type() == WidgetItem)
        return 0;

    if (edgeCount() != 2)
        return 0;
    if (edge(0) == e)
        return edge(1);
    else {
        Q_ASSERT(e == edge(1));
        return edge(0);
    }
}

/*******************************************************************************
** CEWidgetItem
*/

CEWidgetItem::CEWidgetItem(QWidget *w, ConnectionEdit *edit)
    : CEEndPointItem(QPoint(-1, -1), edit),
        m_widget(w)
{
    m_rect = widgetRect();
}
/*
static QString regionToString(const QRegion &region)
{
    QString result;

    QVector<QRect> rects = region.rects();
    for (int i = 0; i < rects.size(); ++i) {
        const QRect &r = rects.at(i);
        result += QString("[%1, %2, %3, %4] ").arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
    }

    return result;
}
*/
void CEWidgetItem::paint(QPainter *p)
{
    p->save();
    QColor c(colorForStatus());
    p->setPen(c);
    c.setAlpha(SELECTION_ALPHA);
    p->setBrush(c);
    p->drawRect(fixRect(rect()));
    p->restore();
    CEItem::paint(p);
}

QRect CEWidgetItem::widgetRect() const
{
    QRect r = m_widget->geometry();
    QPoint pos = m_widget->mapToGlobal(QPoint(0, 0));
    pos = edit()->mapFromGlobal(pos);
    r.moveTopLeft(pos);
    return r;
}

bool CEWidgetItem::updateGeometry()
{
    QRect new_rect = widgetRect();

    if (rect() == new_rect)
        return false;

    update();
    m_rect = new_rect;
    update();

    emit moved();
    
    return true;
}


/*******************************************************************************
** CEEdgeItem
*/

CEEdgeItem::CEEdgeItem(CEEndPointItem *ep1, CEEndPointItem *ep2, ConnectionEdit *edit)
    : CEItem(edit), m_ep1(ep1), m_ep2(ep2)
{
    connect(m_ep1, SIGNAL(destroyed(QObject*)), this, SLOT(endPointDestroyed(QObject*)));
    connect(m_ep2, SIGNAL(destroyed(QObject*)), this, SLOT(endPointDestroyed(QObject*)));
    connect(m_ep1, SIGNAL(moved()), this, SLOT(endPointMoved()));
    connect(m_ep2, SIGNAL(moved()), this, SLOT(endPointMoved()));

    m_ep1->addEdge(this);
    m_ep2->addEdge(this);

    m_exit_pos = QPoint(-1, -1);
    m_enter_pos = QPoint(-1, -1);

    recalculate();
}

void CEEdgeItem::endPointDestroyed(QObject *o)
{
    if (o == m_ep1)
        m_ep1 = 0;
    else if (o == m_ep2)
        m_ep2 = 0;
}

static QPoint rotate(QPoint pos, double alpha)
{
    return QPoint((int)(pos.x()*cos(alpha) + pos.y()*sin(alpha)),
                    (int)(-pos.x()*sin(alpha) + pos.y()*cos(alpha)));
}

enum EdgeType { HorEdge, VerEdge };
static EdgeType classifyEdge(QPoint p1, QPoint p2, const QRect &rect)
{
    if (rect.width() > 0 && rect.height() > 0) {
        Q_ASSERT(rect.contains(p2));
    }

    int dx = p1.x() - p2.x();
    int dy = p2.y() - p1.y();

    if (dx == 0)
        return HorEdge;
    if (dy == 0)
        return VerEdge;

    double r = (dy + 0.0)/dx;

    if (dx > 0) {
        int d = rect.right() - p2.x();
        if (d == 0)
            return VerEdge;
        double r2 = (p2.y() - rect.top() + 0.0)/d; // +
        double r3 = (p2.y() - rect.bottom() + 0.0)/d; // -
        return r2 > r && r > r3 ? VerEdge : HorEdge;
    }

    int d = p2.x() - rect.left();
    if (d == 0)
        return VerEdge;
    double r1 = (p2.y() - rect.top() + 0.0)/d; // +
    double r4 = (p2.y() - rect.bottom() + 0.0)/d; // -
    return r1 > -r && -r > r4 ? VerEdge : HorEdge;
}

static QPoint enterPos(const QPoint &p1, const QPoint &p2, const QRect &rect)
{
    if (rect.width() > 0 && rect.height() > 0) {
        Q_ASSERT(rect.contains(p2));
    }

    int dx = p1.x() - p2.x();
    int dy = p2.y() - p1.y();

    if (dx == 0) {
        if (p1.y() < p2.y())
            return QPoint(p2.x(), rect.top());
        else
            return QPoint(p2.x(), rect.bottom());
    }

    if (dy == 0) {
        if (p1.x() > p2.x())
            return QPoint(rect.right(), p2.y());
        else
            return QPoint(rect.left(), p2.y());
    }

    EdgeType exit_edge = classifyEdge(p1, p2, rect);

    double r = (dx + 0.0)/dy;

    if (exit_edge == HorEdge) {
        if (p1.y() < p2.y()) {
            return QPoint(p2.x(), rect.top())
                        + QPoint((int)((p2.y() - rect.top())*r), 0);
        } else {
            return QPoint(p2.x(), rect.bottom())
                        - QPoint((int)((rect.bottom() - p2.y())*r), 0);
        }
    }  else {
        if (p1.x() > p2.x()) {
            return QPoint(rect.right(), p2.y())
                        - QPoint(0, (int)((rect.right() - p2.x())/r));
        } else {
            return QPoint(rect.left(), p2.y())
                        + QPoint(0, (int)((p2.x() - rect.left())/r));
        }
    }
}

static double angle(const QPoint &p1, const QPoint &p2)
{
    int dx = p1.x() - p2.x();
    int dy = p2.y() - p1.y();

    if (dx == 0)
        return dy < 0 ? -M_PI/2 : M_PI/2;

    double result = atan((dy + 0.0)/dx);
    if (dx < 0)
        result += M_PI;

    return result;
}

void CEEdgeItem::recalculate()
{
    if (m_ep1 == 0 || m_ep1 == 0)
        return;

    m_pos1 = m_ep1->pos();
    m_pos2 = m_ep2->pos();

    QPoint d;

    if (m_pos1.y() == m_pos2.y()) {
        d = QPoint(0, ENDPOINT_RADIUS);
        if (m_pos1.x() <= m_pos2.x())
            d = -d;
    } else if (m_pos1.x() == m_pos2.x()) {
        d = QPoint(ENDPOINT_RADIUS, 0);
        if (m_pos1.y() >= m_pos2.y())
            d = -d;
    } else {
        double r = (m_pos2.y() - m_pos1.y() + 0.0)/(m_pos2.x() - m_pos1.x());
        double u = (ENDPOINT_RADIUS + 0.0)/sqrt(1 + sqr(1/r));
        d = QPoint((int)u, (int)(-u/r));

        if (m_pos1.y() > m_pos2.y())
            d = -d;
    }

    if (m_pos1.x() < m_pos2.x() && m_pos1.y() <= m_pos2.y()) {
        m_top = m_pos1 + d;
        m_side1 = m_pos1 - d;
        m_side2 = m_pos2 + d;
        m_bottom = m_pos2 - d;
    } else if (m_pos1.x() >= m_pos2.x() && m_pos1.y() < m_pos2.y()) {
        m_top = m_pos1 - d;
        m_side1 = m_pos2 - d;
        m_side2 = m_pos1 + d;
        m_bottom = m_pos2 + d;
    } else if (m_pos1.x() > m_pos2.x() && m_pos1.y() >= m_pos2.y()) {
        m_top = m_pos2 - d;
        m_side1 = m_pos2 + d;
        m_side2 = m_pos1 - d;
        m_bottom = m_pos1 + d;
    } else if (m_pos1.x() <= m_pos2.x() && m_pos1.y() > m_pos2.y()) {
        m_top = m_pos2 + d;
        m_side1 = m_pos1 + d;
        m_side2 = m_pos2 - d;
        m_bottom = m_pos1 - d;
    }
}

QRect CEEdgeItem::rect() const
{
    QRect result = QRect(qMin(m_pos1.x(), m_pos2.x()), qMin(m_pos1.y(), m_pos2.y()),
                        qAbs(m_pos1.x() - m_pos2.x()) + 1, qAbs(m_pos1.y() - m_pos2.y()) + 1);
    result = expand(result, ENDPOINT_RADIUS);
    QRectF br = m_arrow_head.boundingRect();
    result |= QRect((int)br.x(), (int)br.y(), (int)br.width(), (int)br.height());
    return expand(result, ENDPOINT_RADIUS);
}

static bool belowLine(const QPoint &a, const QPoint &b, const QPoint &p)
{
    return p.y() >= (int)(a.y() + (p.x() - a.x() + 0.0)*(b.y() - a.y())/(b.x() - a.x()));
}

static bool aboveLine(const QPoint &a, const QPoint &b, const QPoint &p)
{
    return p.y() <= (int)(a.y() + (p.x() - a.x() + 0.0)*(b.y() - a.y())/(b.x() - a.x()));
}

bool CEEdgeItem::contains(const QPoint &p) const
{
    if (m_pos1.x() == m_pos2.x() || m_pos1.y() == m_pos2.y())
        return rect().contains(p);

    bool b1 = m_top.x() == m_side1.x() ? p.x() > m_top.x() : belowLine(m_top, m_side1, p);
    bool b2 = m_top.x() == m_side2.x() ? p.x() < m_top.x() : belowLine(m_top, m_side2, p);
    bool a1 = m_side1.x() == m_bottom.x() ? p.x() > m_bottom.x() : aboveLine(m_side1, m_bottom, p);
    bool a2 = m_side2.x() == m_bottom.x() ? p.x() < m_bottom.x() : aboveLine(m_side2, m_bottom, p);

/*    qDebug() << "CEEdgeItem::contains():"
                << "top" << m_top
                << "side1" << m_side1
                << "side2" << m_side2
                << "bottom" << m_bottom
                << "b1" << b1
                << "b2" << b2
                << "a1" << a1
                << "a2" << a2; */

    return b1 && b2 && a1 && a2;
}

void CEEdgeItem::paint(QPainter *p)
{
    if (m_ep1 == 0 || m_ep1 == 0)
        return;

    p->save();

    QPoint p1 = m_pos1;
    QPoint p2 = m_pos2;
    if (m_exit_pos != QPoint(-1, -1))
        p1 = m_exit_pos;
    if (m_enter_pos != QPoint(-1, -1))
        p2 = m_enter_pos;

    p->setPen(colorForStatus());
    p->drawLine(p1, p2);

    if (!m_arrow_head.isEmpty()) {
        p->setBrush(colorForStatus());
        p->drawPolygon(m_arrow_head);
    }
/*
    p->setPen(Qt::green);
    p->drawLine(m_top, m_side1);
    p->drawLine(m_top, m_side2);
    p->setPen(Qt::red);
    p->drawLine(m_bottom, m_side1);
    p->drawLine(m_bottom, m_side2);
*/
    p->restore();
    CEItem::paint(p);
}

void CEEdgeItem::move(const QPoint &delta)
{
    if (m_ep1 == 0 || m_ep2 == 0)
        return;

    m_ep1->move(delta);
    m_ep2->move(delta);
}

void CEEdgeItem::endPointMoved()
{
    update();
    recalculate();
    update();
}

CEEndPointItem *CEEdgeItem::otherEndPoint(const CEEndPointItem *ep) const
{
    if (m_ep1 == 0 || m_ep2 == 0)
        return 0;

    Q_ASSERT(ep == m_ep1 || ep == m_ep2);

    if (ep == m_ep1)
        return m_ep2;
    else
        return m_ep1;
}

void CEEdgeItem::setExitPos(const QPoint &pos)
{
    if (pos == m_exit_pos)
        return;
    m_exit_pos = pos;
    
    update();
}

void CEEdgeItem::setEnterPos(const QPoint &pos)
{
    if (pos == m_enter_pos)
        return;

    m_enter_pos = pos;
    m_arrow_head.clear();

    if (m_enter_pos != QPoint(-1, -1)) {
        double alpha = angle(m_pos1, m_pos2);

        QPoint pos1(3*ENDPOINT_RADIUS, -(int)(ENDPOINT_RADIUS*1.5));
        QPoint pos2(3*ENDPOINT_RADIUS, (int)(ENDPOINT_RADIUS*1.5));
        pos1 = rotate(pos1, alpha);
        pos2 = rotate(pos2, alpha);
        m_arrow_head.append(m_enter_pos);
        m_arrow_head.append(m_enter_pos + pos1);
        m_arrow_head.append(m_enter_pos + pos2);
    }

    update();
}

bool CEEdgeItem::visibleAt(const QPoint &pos) const
{
    QPoint p1 = m_exit_pos;
    if (p1.x() == -1)
        p1 = m_pos1;
    QPoint p2 = m_enter_pos;
    if (p2.x() == -1)
        p2 = m_pos2;
        
    QPoint p3(qMin(p1.x(), p2.x()), qMin(p1.y(), p2.y()));
    QPoint p4(qMax(p1.x(), p2.x()), qMax(p1.y(), p2.y()));
        
    QRect r(p3, p4);
    r = expand(r, ENDPOINT_RADIUS);
    return r.contains(pos);
}

/*******************************************************************************
** Connection
*/

Connection::Connection(ConnectionEdit *edit)
{
    m_source = 0;
    m_destination = 0;
    m_edit = edit;
    m_source_label_item = 0;
    m_destination_label_item = 0;
}

Connection::HintList Connection::hints() const
{
    HintList result;

    QList<CEItem*> item_list = m_edit->connectionItems(this);

    CEEdgeItem *some_edge = 0;
    for (int i = 0; i < item_list.size(); ++i) {
        CEItem *item = item_list.at(i);
        if (item->type() == CEItem::EdgeItem) {
            some_edge = qt_cast<CEEdgeItem*>(item);
            break;
        }
    }
    if (some_edge == 0)
        return result;

    for (CEEdgeItem *edge = firstEdge(some_edge); edge != 0; edge = nextEdge(edge)) {
        CEEndPointItem *item = edge->endPoint1();
        if (item == 0)
            continue;
        if (item->type() == CEItem::WidgetItem)
            continue;
        QPoint pos = item->pos();
        result.append(ConnectionHint(ConnectionHint::EndPoint, item->pos()));
    }

    if (CELabelItem *label_item = sourceLabelItem())
        result.append(ConnectionHint(ConnectionHint::SourceLabel, label_item->pos()));

    if (CELabelItem *label_item = destinationLabelItem())
        result.append(ConnectionHint(ConnectionHint::DestinationLabel, label_item->pos()));

    return result;
}

void Connection::setLabelItems(CELabelItem *source_label, CELabelItem *destination_label)
{
    m_source_label_item = source_label;
    m_destination_label_item = destination_label;

    if (m_source_label_item != 0)
        m_source_label_item->setText(m_source_label_data.value(DisplayRole).toString());

    if (m_destination_label_item != 0)
        m_destination_label_item->setText(m_destination_label_data.value(DisplayRole).toString());
}

void Connection::setDestinationLabel(LabelRole role, const QVariant &v)
{
    m_destination_label_data[role] = v;
    if (m_destination_label_item != 0 && role == DisplayRole)
        m_destination_label_item->setText(v.toString());
}

void Connection::setSourceLabel(LabelRole role, const QVariant &v)
{
    m_source_label_data[role] = v;
    if (m_source_label_item != 0 && role == DisplayRole)
        m_source_label_item->setText(v.toString());
}

/*******************************************************************************
** ConnectionEdit
*/

ConnectionEdit::ConnectionEdit(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_MouseTracking, true);
    setFocusPolicy(Qt::ClickFocus);

    m_bg_widget = 0;
    m_dragged_item = 0;
    m_start_drag_on_move = false;
    m_start_draw_on_move = false;
    m_current_line = NoLine;
    m_old_target = 0;

    new SignalDumper(this);
}

ConnectionEdit::~ConnectionEdit()
{
    clear();
}

void ConnectionEdit::paintEvent(QPaintEvent *e)
{
    if (m_bg_pixmap.isNull())
        updateBackground();

    QPainter p(this);
    p.setClipRegion(e->region());
    
    p.drawPixmap(m_bg_pixmap.rect(), m_bg_pixmap);

    foreach (CEItem *item, m_item_list) {
        if (item->visible() && e->region().contains(item->rect()))
            item->paint(&p);
    }
}

CEItem *ConnectionEdit::itemUnderMouse() const
{
    if (m_items_under_mouse.isEmpty())
        return 0;
    return m_items_under_mouse.last();
}

CEItem *ConnectionEdit::itemUnderMouse(CEItem::Type type) const
{
    if (m_items_under_mouse.isEmpty())
        return 0;

    int i = m_items_under_mouse.size();
    do {
        --i;
        CEItem *item = m_items_under_mouse.at(i);
        if (item->type() == type)
            return item;
    } while (i > 0);

    return 0;
}

void ConnectionEdit::insertEndPoint(const QPoint &pos)
{
    if (mode() != EditMode)
        return;

    selectNone();

    CEItem *item = itemUnderMouse(CEItem::EdgeItem);
    CEEdgeItem *edge = qt_cast<CEEdgeItem*>(item);

    if (edge == 0)
        return;

    CEEndPointItem *ep = new CEEndPointItem(pos, this);
    insertItem(ep);
    CEEdgeItem *new_edge1 = new CEEdgeItem(edge->endPoint1(), ep, this);
    CEEdgeItem *new_edge2 = new CEEdgeItem(ep, edge->endPoint2(), this);
    insertItem(new_edge1);
    insertItem(new_edge2);

    Connection *con = m_connection_map.value(edge, 0);
    if (con != 0) {
        m_connection_map.insert(ep, con);
        m_connection_map.insert(new_edge1, con);
        m_connection_map.insert(new_edge2, con);
    }

    updateUnderMouse(pos);
    m_start_drag_on_move = true;
    deleteItem(edge);

    updateLine(new_edge1);
//    update();
    ep->adjustLayout();
}

void ConnectionEdit::initConnection(Connection *con, const Connection::HintList &hint_list)
{
    QWidget *source = con->source();
    Q_ASSERT(source != 0);
    QWidget *destination = con->destination();
    Q_ASSERT(destination != 0);

    CEWidgetItem *source_item = widgetItem(source);
    if (source_item == 0) {
        source_item = new CEWidgetItem(source, this);
        insertItem(source_item);
    }
    QList<CEItem*> item_list;
    item_list.append(source_item);

    CEWidgetItem *dest_item = widgetItem(destination);
    if (dest_item == 0) {
        dest_item = new CEWidgetItem(destination, this);
        insertItem(dest_item);
    }

    CEEndPointItem *last_ep = source_item;
    ConnectionHint source_label_hint, destination_label_hint;

    foreach (ConnectionHint hint, hint_list) {
        switch (hint.type) {
            case ConnectionHint::EndPoint: {
                CEEndPointItem *ep = new CEEndPointItem(hint.pos, this);
                CEEdgeItem *edge = new CEEdgeItem(last_ep, ep, this);
                insertItem(edge);
                insertItem(ep);
                item_list.append(edge);
                item_list.append(ep);
                last_ep = ep;
                break;
            }
            case ConnectionHint::SourceLabel: {
                source_label_hint = hint;
                break;
            }
            case ConnectionHint::DestinationLabel: {
                destination_label_hint = hint;
                break;
            }
        }
    }

    CEEdgeItem *edge = new CEEdgeItem(last_ep, dest_item, this);
    insertItem(edge);
    item_list.append(edge);
    item_list.append(dest_item);

    initConnection(con, item_list);

    CELabelItem *source_label = con->sourceLabelItem();
    CELabelItem *destination_label = con->destinationLabelItem();
    if (source_label != 0)
        source_label->move(source_label_hint.pos - source_label->pos());
    if (destination_label != 0)
        destination_label->move(destination_label_hint.pos - destination_label->pos());
}

void ConnectionEdit::initConnection(Connection *con, const ItemList &item_list)
{
    int n = item_list.size();
    Q_ASSERT(n > 1);

    connect(con, SIGNAL(aboutToDelete(Connection*)), this, SIGNAL(aboutToRemove(Connection*)));
    connect(con, SIGNAL(selected(Connection*)), this, SIGNAL(selected(Connection*)));

    foreach (CEItem *item, item_list)
        m_connection_map.insert(item, con);

    CELabelItem *source_label = con->sourceLabelItem();
    CELabelItem *destination_label = con->destinationLabelItem();
    if (source_label != 0) {
        insertItem(source_label);
        m_connection_map.insert(source_label, con);
    }
    if (destination_label != 0) {
        insertItem(destination_label);
        m_connection_map.insert(destination_label, con);
    }
        
    m_connection_list.append(con);

    updateLine(con);

    foreach (CEItem *item, item_list) {
        if (item->type() == CEItem::EndPointItem)
            qt_cast<CEEndPointItem*>(item)->adjustLayout();
        item->update();
    }

    emit added(con);
}

Connection *ConnectionEdit::createConnection(QWidget *source, QWidget *destination)
{
    Connection *con = new Connection(this);
    if (source != 0)
        con->setSource(source);
    if (destination != 0)
        con->setDestination(destination);
    return con;
}


void ConnectionEdit::checkConnection(Connection *con)
{
    ItemList item_list = m_connection_map.keys(con);
    if (item_list.size() > 2)
        return;

    // remove if only widget items are left associated with this connection
    foreach (CEItem *item, item_list) {
        if (item->type() != CEItem::WidgetItem)
            return;
    }

    for (int i = 0; i < item_list.size(); ++i) {
        CEItem *item = item_list.at(i);

        ConnectionMap::iterator it = m_connection_map.lowerBound(item);
        while (it != m_connection_map.end() && it.key() == item) {
            if (it.value() == con) {
                m_connection_map.erase(it);
                break;
            }
            ++it;
        }
    }

    emit aboutToRemove(con);
    m_connection_list.removeAll(con);
    delete con;
}

void ConnectionEdit::deleteItem(CEItem *item)
{
    Q_ASSERT(item != m_dragged_item);
    Q_ASSERT(!m_selected_item_set.contains(item));

    m_item_list.removeAll(item);
    m_items_under_mouse.removeAll(item);

    QList<Connection*> connection_list = m_connection_map.values(item);
    m_connection_map.remove(item);
    CEEndPointItem *ep_item = 0;
    if (item->type() == CEItem::EndPointItem)
        ep_item = qt_cast<CEEndPointItem*>(item);
    foreach (Connection *con, connection_list)
        checkConnection(con);

    item->update();

    delete item;
}

static bool itemLessThan(CEItem *item1, CEItem *item2)
{
    CEItem::Type t1 = item1->type();
    CEItem::Type t2 = item2->type();
    if (t1 != t2)
        return t1 < t2;
    if (t1 != CEItem::WidgetItem)
        return true;
    if (item1->rect().contains(item2->rect()))  // two nested widgets -
        return true;                            // outer is "less than" inner
    return false;
}

void ConnectionEdit::insertItem(CEItem *item)
{
    // sorted insert
    for (int i = 0; i < m_item_list.size(); ++i) {
        if (itemLessThan(item, m_item_list.at(i))) {
            m_item_list.insert(i, item);
            return;
        }
    }
    m_item_list.append(item);
    item->update();
}

CEWidgetItem *ConnectionEdit::widgetItem(QWidget *widget) const
{
    if (widget == 0)
        return 0;

    for (int i = 0; i < m_item_list.size(); ++i) {
        CEWidgetItem *widget_item = qt_cast<CEWidgetItem*>(m_item_list.at(i));
        if (widget_item == 0)
            break;
        if (widget_item->widget() == widget)
            return widget_item;
    }

    return 0;
}

void ConnectionEdit::updateUnderMouse(const QPoint &pos)
{
    QWidget *widget = widgetAt(pos);
    if (widget != 0 && widgetItem(widget) == 0) {
        // create a temporary widget under the mouse
        CEItem *item = new CEWidgetItem(widget, this);
        insertItem(item);
        item->update();
    }
/*
    foreach (CEItem *item, m_items_under_mouse)
        item->update(); */

    m_items_under_mouse.clear();

    int i = 0;
    while (i < m_item_list.size()) {
        CEItem *item = m_item_list.at(i);
        CEWidgetItem *witem = qt_cast<CEWidgetItem*>(item);
        if (witem != 0 && witem->widget() != widget && witem->edgeCount() == 0) {
            // delete an old temporary widget
            setSelected(witem, false);
            deleteItem(witem);
            continue;
        }

        if (item->selectable() && item->rect().contains(pos) && item->contains(pos)) {
            CEEdgeItem *edge_item = qt_cast<CEEdgeItem*>(item);
            if (edge_item == 0 || edge_item->visibleAt(pos))
                m_items_under_mouse.append(item);
        }

        ++i;
    }
/*
    foreach (CEItem *item, m_items_under_mouse)
        item->update(); */
}

void ConnectionEdit::mousePressEvent(QMouseEvent *e)
{
    e->accept();
    updateUnderMouse(e->pos());

    CEItem *item = itemUnderMouse();

    if (mode() == EditMode) {
        if (e->modifiers() & Qt::ControlModifier) {
            insertEndPoint(e->pos());
            item = 0;
        } else {
            if (item != 0) {
                if (item->type() == CEItem::WidgetItem)
                    m_start_draw_on_move = true;
                else
                    m_start_drag_on_move = true;
            }
        }
    }

    m_last_mouse_pos = e->pos();
    if (!(e->modifiers() & Qt::ShiftModifier))
        selectNone();
    if (item != 0)
        setSelected(item, true);
}

void ConnectionEdit::addEdgeTo(const QPoint &pos)
{
    CEEndPointItem *ep = new CEEndPointItem(pos, this);
    CEEdgeItem *edge = new CEEdgeItem(lastEndPoint(), ep, this);
    insertItem(edge);
    insertItem(ep);
    m_new_item_list.append(edge);
    m_new_item_list.append(ep);
}

static bool isDescendant(QWidget *child, QWidget *parent)
{
    while (child != 0 && child != parent)
        child = child->parentWidget();

    return child != 0;
}

ConnectionEdit::LineType ConnectionEdit::classifyLine(CEWidgetItem *source, CEWidgetItem *target,
                                                        const QPoint &pos) const
{
    if (target == 0 ||
        (!isDescendant(source->widget(), target->widget())
            && !isDescendant(target->widget(), source->widget()))) {
        QRect r = source->rect();
        if (r.width() > 0 && r.height() > 0) {
            Q_ASSERT(!r.contains(pos));
        }

        if (pos.x() < r.left()) {
            return LeftLine;
        } else if (pos.x() > r.right()) {
            return RightLine;
        } else {
            if (pos.y() < r.top())
                return TopLine;
            else
                return BottomLine;
        }
    } else {
        QRect r = target->rect();
        if (r.width() > 0 && r.height() > 0) {
            Q_ASSERT(r.contains(pos));
        }

        int min_dist = pos.y() - r.top();
        LineType result = TopLoopLine;

        int dist = pos.x() - r.left();
        if (dist < min_dist) {
            min_dist = dist;
            result = LeftLoopLine;
        }

        dist = r.bottom() - pos.y();
        if (dist < min_dist) {
            min_dist = dist;
            result = BottomLoopLine;
        }

        dist = r.right() - pos.x();
        if (dist < min_dist) {
            min_dist = dist;
            result = RightLoopLine;
        }

        return result;
    }
}

void ConnectionEdit::createLine(LineType type, CEWidgetItem *target, const QPoint &pos)
{
    // delete all items except source
    ItemList delete_items;
    while (m_new_item_list.size() > 1)
        delete_items.append(m_new_item_list.takeLast());
    delete_items.removeAll(target);
    delete_items.removeAll(m_new_item_list.first());
    deleteItems(delete_items);

    CEWidgetItem *start = firstEndPoint();
    QPoint c = start->rect().center();

    QRect sr = start->rect();
    QRect tr;
    if (target != 0) {
        tr = target->rect();
        if (sr.contains(tr))
            tr = sr;
    }

    switch (type) {
        case LeftLine:
        case RightLine:
            addEdgeTo(QPoint(pos.x(), c.y()));
            break;
        case TopLine:
            addEdgeTo(QPoint(pos.x(), sr.top()));
            break;
        case BottomLine:
            addEdgeTo(QPoint(pos.x(), sr.bottom()));
            break;
        case TopLoopLine:
            addEdgeTo(QPoint(c.x(), tr.top() - LOOP_MARGIN));
            addEdgeTo(QPoint(pos.x(), tr.top() - LOOP_MARGIN));
            break;
        case BottomLoopLine:
            addEdgeTo(QPoint(c.x(), tr.bottom() + LOOP_MARGIN));
            addEdgeTo(QPoint(pos.x(), tr.bottom() + LOOP_MARGIN));
            break;
        case RightLoopLine:
            addEdgeTo(QPoint(tr.right() + LOOP_MARGIN, c.y()));
            addEdgeTo(QPoint(tr.right() + LOOP_MARGIN, pos.y()));
            break;
        case LeftLoopLine:
            addEdgeTo(QPoint(tr.left() - LOOP_MARGIN, c.y()));
            addEdgeTo(QPoint(tr.left() - LOOP_MARGIN, pos.y()));
            break;
        default:
            Q_ASSERT(false);
    }

    addEdgeTo(pos);
    if (target != 0) {
        CEEdgeItem *edge = new CEEdgeItem(lastEndPoint(), target, this);
        insertItem(edge);
        m_new_item_list.append(edge);
        m_new_item_list.append(target);
    }
}

void ConnectionEdit::mouseMoveEvent(QMouseEvent *e)
{
    CEItem *under_mouse = itemUnderMouse();

    if (m_start_drag_on_move) {
        Q_ASSERT(under_mouse != 0);
        m_dragged_item = under_mouse;
        m_start_drag_on_move = false;
        m_dragged_item->update();
    } else if (m_start_draw_on_move) {
        Q_ASSERT(under_mouse != 0);
        Q_ASSERT(under_mouse->type() == CEItem::WidgetItem);
        m_new_item_list.append(under_mouse);
        m_current_line = NoLine;
        m_old_target = 0;
        m_start_draw_on_move = false;
        selectNone();
    }

    if (mode() != DragMode)
        updateUnderMouse(e->pos());

    if (mode() == DrawMode) {
        QPoint pos = e->pos();
        CEWidgetItem *start = firstEndPoint();
        CEWidgetItem *target = qt_cast<CEWidgetItem*>(itemUnderMouse(CEItem::WidgetItem));

        LineType needed_line = classifyLine(start, target, pos);
        if (needed_line != m_current_line || target != m_old_target) {
            createLine(needed_line, target, pos);
            m_current_line = needed_line;
            m_old_target = target;
        }

        int cnt = m_new_item_list.size();
        Q_ASSERT(cnt >= 5);
        --cnt;
        CEEndPointItem *ep1 = qt_cast<CEEndPointItem*>(m_new_item_list.at(cnt));
        if (ep1->type() == CEItem::WidgetItem)
            ep1 = qt_cast<CEEndPointItem*>(m_new_item_list.at(cnt -= 2));
        CEEndPointItem *ep2 = qt_cast<CEEndPointItem*>(m_new_item_list.at(cnt -= 2));

        ep1->move(pos - ep1->pos());
        if (m_current_line == LeftLoopLine || m_current_line == RightLoopLine)
            ep2->move(QPoint(ep2->pos().x(), pos.y()) - ep2->pos());
        else
            ep2->move(QPoint(pos.x(), ep2->pos().y()) - ep2->pos());
        updateLine(qt_cast<CEEdgeItem*>(m_new_item_list.at(cnt + 1)));
    } else if (mode() == DragMode) {
        m_dragged_item->move(e->pos() - m_last_mouse_pos);
        Connection *con = m_connection_map.value(m_dragged_item, 0);
        if (con != 0)
            updateLine(con);
    }

    m_last_mouse_pos = e->pos();
}

void ConnectionEdit::resizeEvent(QResizeEvent *)
{
    updateBackground();
    if (m_bg_widget != 0 && m_bg_widget->layout() != 0)
        updateAllItems();
}

void ConnectionEdit::mouseReleaseEvent(QMouseEvent *e)
{
    updateUnderMouse(e->pos());

    if (mode() == DragMode) {
        m_dragged_item->update();
        m_dragged_item = 0;
    } else if (mode() == DrawMode) {
        // Released over a widget without an associated item, create the item
        CEWidgetItem *widget_item = qt_cast<CEWidgetItem*>(itemUnderMouse(CEItem::WidgetItem));
        if (widget_item == 0) {
            abortNewItems();
        } else {
            // End-point
            CEWidgetItem *source_item = qt_cast<CEWidgetItem*>(m_new_item_list.first());
            Q_ASSERT(source_item != 0);
            CEWidgetItem *destination_item = qt_cast<CEWidgetItem*>(m_new_item_list.last());
            Q_ASSERT(destination_item != 0);
            Connection *con = createConnection(source_item->widget(), destination_item->widget());
            if (con == 0) {
                abortNewItems();
            } else {
                initConnection(con, m_new_item_list);
                m_new_item_list.clear();
            }
        }
    }
    e->accept();

    m_start_drag_on_move = false;
    m_start_draw_on_move = false;
    m_current_line = NoLine;
    m_old_target = 0;
}

void ConnectionEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
    e->accept();

    if (mode() != EditMode)
        return;

    CEEdgeItem *edge = qt_cast<CEEdgeItem*>(itemUnderMouse(CEItem::EdgeItem));
    if (edge == 0)
        return;


    Connection *con = m_connection_map.value(edge, 0);
    if (con == 0)
        return;

    emit activated(con);
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
    QRect r = w->rect();
    r.moveTopLeft(w->mapTo(m_bg_widget, QPoint(0, 0)));

    return r;
}

void ConnectionEdit::clear()
{
    foreach (CEItem *item, m_item_list)
        delete item;

    m_item_list.clear();
    m_selected_item_set.clear();
    m_new_item_list.clear();

    m_dragged_item = 0;
    m_start_drag_on_move = false;
    m_start_draw_on_move = false;
    m_current_line = NoLine;
    m_old_target = 0;
}

void ConnectionEdit::abortNewItems()
{
    if (mode() != DrawMode)
        return;

    deleteItems(m_new_item_list);
    m_new_item_list.clear();

    update();
}

void ConnectionEdit::keyPressEvent(QKeyEvent *e)
{
    e->accept();
    switch (e->key()) {
        case Qt::Key_D:
            dumpItems();
            break;
        case Qt::Key_Escape:
            abortNewItems();
            e->accept();
            break;
        case Qt::Key_Delete: {
            deleteItems();
            e->accept();
            break;
        }
        default:
            break;
    }

//    dumpItems();
    update();
}

void ConnectionEdit::deleteEndPoint(CEEndPointItem *ep)
{
    if (ep->edgeCount() == 2) {
        CEEndPointItem *ep1 = ep->sourceEdge()->endPoint1();
        CEEndPointItem *ep2 = ep->destinationEdge()->endPoint2();

        if (ep1 != 0 && ep2 != 0) {
            CEEdgeItem *new_edge = new CEEdgeItem(ep1, ep2, this);
            insertItem(new_edge);

            Connection *con = m_connection_map.value(ep, 0);
            if (con != 0)
                m_connection_map.insert(new_edge, con);

            deleteItem(ep->edge(0));
            deleteItem(ep->edge(0));
            deleteItem(ep);
            updateLine(new_edge);
        }
    } else {
        deleteItem(ep);
    }
}

void ConnectionEdit::updateBackground()
{
    if (m_bg_widget != 0)
        m_bg_pixmap = QPixmap::grabWidget(m_bg_widget);
}

void ConnectionEdit::selectNone()
{
    foreach (CEItem *item, m_selected_item_set) {
        item->update();
    }
    m_selected_item_set.clear();
}

void ConnectionEdit::deleteWidgetItem(CEWidgetItem *widget)
{
    m_selected_item_set.clear();

    QMap<CEItem*, CEItem*> item_set;

    QList<Connection*> con_list = m_connection_map.values(widget);
    foreach (Connection *con, con_list) {
        QList<CEItem*> item_list = m_connection_map.keys(con);
        foreach (CEItem *item, item_list)
            item_set.insert(item, item);
    }

    deleteItems(item_set.keys());
}

void ConnectionEdit::deleteWidgetItem(QWidget *w)
{
    CEWidgetItem *widget_item = widgetItem(w);
    if (widget_item != 0)
        deleteWidgetItem(widget_item);
    
    QList<QWidget*> child_list = qFindChildren<QWidget*>(w);
    foreach (QWidget *child, child_list)
        deleteWidgetItem(child);
}

void ConnectionEdit::updateAllItems()
{
    for (int i = 0; i < m_item_list.size(); ++i) {
        CEWidgetItem *widget_item = qt_cast<CEWidgetItem*>(m_item_list.at(i));
        if (widget_item == 0)
            continue;

        if (!widget_item->updateGeometry())
            continue;

        QList<Connection*> con_list = m_connection_map.values(widget_item);
        foreach (Connection *con, con_list) {
            QList<CEItem*> item_list = m_connection_map.keys(con);
            foreach (CEItem *item, item_list) {
                if (item->type() == CEItem::EndPointItem)
                    qt_cast<CEEndPointItem*>(item)->adjustPos();
            }
            updateLine(con);
        }
    }
}

void ConnectionEdit::deleteItems(ItemList item_list)
{
    // If a single endpoint is selected, just delete that and fix the line
    bool single_ep_selected = item_list.size() == 1
                                && item_list.first()->type() == CEItem::EndPointItem;

    QMap<CEItem*, CEItem*> deleted_set;

    if (single_ep_selected) {
        CEEndPointItem *ep = qt_cast<CEEndPointItem*>(item_list.first());
        deleteEndPoint(ep);
    } else {
        // First delete the edges
        int i = 0;
        while (i < item_list.size()) {
            CEItem *item = item_list.at(i);
            if (deleted_set.contains(item)) {
                item_list.removeAt(i);
                continue;
            }

            CEItem::Type type = item->type();

            if (type != CEItem::EndPointItem
                    && type != CEItem::WidgetItem) {
                deleteItem(item);
                item_list.removeAt(i);
                deleted_set.insert(item, item);
                continue;
            }

            ++i;
        }

        // Then delete those endpoints which no longer have any edges attached
        i = 0;
        while (i < item_list.size()) {
            CEItem *item = item_list.at(i);
            if (deleted_set.contains(item)) {
                item_list.removeAt(i);
                continue;
            }
            CEEndPointItem *ep_item = qt_cast<CEEndPointItem*>(item);
            Q_ASSERT(ep_item != 0);

            if (ep_item->edgeCount() == 0) {
                deleteItem(ep_item);
                item_list.removeAt(i);
                deleted_set.insert(item, item);
                continue;
            }

            ++i;
        }
    }
}

void ConnectionEdit::dumpItems()
{
    qDebug() << "========== Items ============";
    foreach (CEItem *item, m_item_list) {
        if (CEEndPointItem *ep_item = qt_cast<CEEndPointItem*>(item))
            qDebug() << ep_item << ep_item->edgeList() << ep_item->rect() << ep_item->visible();
        else if (CEEdgeItem *edge_item = qt_cast<CEEdgeItem*>(item))
            qDebug() << edge_item << edge_item->endPoint1() << edge_item->endPoint2() << edge_item->visible();
        else
            qDebug() << item << item->visible();
    }
}

void ConnectionEdit::deleteItems()
{
    ItemList item_list = m_selected_item_set.keys();
    m_selected_item_set.clear();
    deleteItems(item_list);
}

void ConnectionEdit::setSelected(CEItem *item, bool selected)
{
    ItemList item_list;
    item_list.append(item);

    if (item->type() == CEItem::EdgeItem || item->type() == CEItem::LabelItem) {
        Connection *con = m_connection_map.value(item, 0);
        if (con != 0) {
            Q_ASSERT(m_connection_map.values(item).count() == 1);
            ItemList connection_item_list = m_connection_map.keys(con);
            foreach (CEItem *item, connection_item_list)
                item_list.append(item);
        }
    }

    setSelectedItems(item_list, selected);
}

void ConnectionEdit::setSelectedItems(const ItemList &item_list, bool selected)
{
    foreach (CEItem *item, item_list) {
        SelectedSet::iterator it = m_selected_item_set.find(item);
        bool found = it != m_selected_item_set.end();

        if (selected) {
            if (!found) {
                m_selected_item_set.insert(item, item);
                item->update();
            }
        } else {
            if (found) {
                m_selected_item_set.erase(it);
                item->update();
            }
        }
    }
}

CEWidgetItem *ConnectionEdit::firstEndPoint() const
{
    if (m_new_item_list.isEmpty())
        return 0;

    CEWidgetItem *widget_item = qt_cast<CEWidgetItem*>(m_new_item_list.first());
    Q_ASSERT(widget_item != 0);
    return widget_item;
}

CEEndPointItem *ConnectionEdit::lastEndPoint() const
{
    if (m_new_item_list.isEmpty())
        return 0;
    CEEndPointItem *ep = qt_cast<CEEndPointItem*>(m_new_item_list.last());
    Q_ASSERT(ep != 0);
    return ep;
}

static QWidget *otherWidget(Connection *con, QWidget *w)
{
    if (con->source() == w)
        return con->destination();
    else
        return con->source();
}

void ConnectionEdit::updateLine(Connection *con)
{
    ItemList item_list = m_connection_map.keys(con);

    if (!con->source()->isShown() || !con->destination()->isShown()) {
        foreach (CEItem *item, item_list) {
            item->setVisible(false);
            if (CEWidgetItem *widget_item = qt_cast<CEWidgetItem*>(item)) {
                if (!widget_item->widget()->isVisible())
                    item->setVisible(false);
                else {
                    // Don't hide the widget item if it is visible and it has a connection
                    // to another visible widget.
                    QList<Connection*> con_list = m_connection_map.values(item);
                    bool visible = false;
                    for (int i = 0; i < con_list.size(); ++i) {
                        Connection *con2 = con_list.at(i);
                        if (otherWidget(con2, widget_item->widget())->isVisible()) {
                            visible = true;
                            break;
                        }
                    }
                    item->setVisible(visible);
                }
            } else {
                item->setVisible(false);
            }
        }
    } else {
        // find any edge
        CEEdgeItem *some_edge = 0;
        for (int i = 0; i < item_list.size(); ++i) {
            foreach (CEItem *item, item_list) {
            item->setVisible(true);
                if (some_edge == 0)
                    some_edge = qt_cast<CEEdgeItem*>(item_list.at(i));
            }
        }

        if (some_edge != 0)
            updateLine(some_edge);
    }
}
/*
static int rectDist(const QRect &r1, const QRect &r2)
{
    int hdist = 0, vdist = 0;
    if (r1.right() < r2.left())
        hdist = r2.left() - r2.right();
    else if (r2.right() < r1.left())
        hdist = r1.left() - r2.right();
    if (r1.bottom() < r2.top())
        vdist = r2.top() - r1.bottom();
    else if (r2.bottom() < r1.top())
        vdist = r1.top() - r2.bottom();

    return qMax(hdist, vdist);
}
*/

void ConnectionEdit::updateLine(CEEdgeItem *e)
{
    Connection *con = m_connection_map.value(e, 0);

    CEEdgeItem *first_edge = firstEdge(e);
    Q_ASSERT(first_edge != 0);
    CEWidgetItem *source = qt_cast<CEWidgetItem*>(first_edge->endPoint1());
    Q_ASSERT(source != 0);
    QRect sr = source->rect();

    if (sr.width() <= 0 || sr.height() <= 0)
        return;
    
    for (CEEdgeItem *edge = first_edge; edge != 0; edge = nextEdge(edge)) {
        edge->setVisible(true);
        edge->setEnterPos(QPoint(-1, -1));
        edge->setExitPos(QPoint(-1, -1));
    }

    for (CEEdgeItem *edge = first_edge; edge != 0; edge = nextEdge(edge)) {
        QPoint pos1 = edge->endPoint1()->pos();
        QPoint pos2 = edge->endPoint2()->pos();
        bool b1 = sr.contains(pos1);
        bool b2 = sr.contains(pos2);

        if (b1 && b2) {
            edge->setVisible(false);
            continue;
        }

        Q_ASSERT(b1 && !b2);

        QPoint pos = enterPos(pos2, pos1, sr);
        edge->setExitPos(pos);
        if (con != 0) {
            CELabelItem *sl = con->sourceLabelItem();
            if (sl)
                sl->setAnchorPos(pos);
        }
        break;
    }

    CEEdgeItem *last_edge = lastEdge(e);
    Q_ASSERT(last_edge != 0);
    CEWidgetItem *target = qt_cast<CEWidgetItem*>(last_edge->endPoint2());
    if (target != 0) {
        QRect tr = target->rect();

        if (tr.width() <= 0 || tr.height() <= 0)
            return;
        
        for (CEEdgeItem *edge = last_edge; edge != 0; edge = prevEdge(edge)) {
            QPoint pos1 = edge->endPoint1()->pos();
            QPoint pos2 = edge->endPoint2()->pos();
            bool b1 = tr.contains(pos1);
            bool b2 = tr.contains(pos2);

            if (b1 && b2) {
                edge->setVisible(false);
                continue;
            }

            Q_ASSERT(!b1 && b2);

            QPoint pos = enterPos(pos1, pos2, tr);

            edge->setEnterPos(pos);
            if (con != 0) {
                CELabelItem *dl = con->destinationLabelItem();
                if (dl)
                    dl->setAnchorPos(pos);
            }
            break;
        }
    }

/*    CEEndPointItem *some_ep = e->endPoint1();
    if (some_ep->type() == CEItem::WidgetItem)
        some_ep = e->endPoint2();
    if (some_ep->type() == CEItem::EndPointItem) {
        for (CEEndPointItem *ep = some_ep; ep != 0 && ep->type() == CEItem::EndPointItem; ep = prevEndPoint(ep))
            ep->adjustLayout();
        for (CEEndPointItem *ep = nextEndPoint(ep); ep != 0 && ep->type() == CEItem::EndPointItem; ep = nextEndPoint(ep))
            ep->adjustLayout();
    }*/
}

#include "connectionedit.moc"
