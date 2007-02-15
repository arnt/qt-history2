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

#include "grid_p.h"

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QPainter>
#include <QtGui/QWidget>
#include <QtGui/qevent.h>

static const bool defaultSnap = true;
static const bool defaultVisible = true;
static const int DEFAULT_GRID = 10;
static const char* KEY_VISIBLE = "gridVisible";
static const char* KEY_SNAPX =  "gridSnapX";
static const char* KEY_SNAPY =  "gridSnapY";
static const char* KEY_DELTAX =  "gridDeltaX";
static const char* KEY_DELTAY =  "gridDeltaY";

// Insert a value into the serialization map unless default
template <class T>
    static inline void valueToVariantMap(T value, T defaultValue, const QString &key, QVariantMap &v) {
        if (value != defaultValue)
            v.insert(key, QVariant(value));
    }

// Obtain a value form QVariantMap
template <class T>
    static inline void valueFromVariantMap(const QVariantMap &v, const QString &key, T &value) {
        const QVariantMap::const_iterator it = v.constFind(key);
        if (it != v.constEnd())
            value = qVariantValue<T>(it.value());
    }

namespace qdesigner_internal
{

Grid::Grid() :
    m_visible(defaultVisible),
    m_snapX(defaultSnap),
    m_snapY(defaultSnap),
    m_deltaX(DEFAULT_GRID),
    m_deltaY(DEFAULT_GRID)
{
}

void  Grid::fromVariantMap(const QVariantMap& vm)
{
    *this = Grid();
    valueFromVariantMap(vm, QLatin1String(KEY_VISIBLE), m_visible);
    valueFromVariantMap(vm, QLatin1String(KEY_SNAPX), m_snapX);
    valueFromVariantMap(vm, QLatin1String(KEY_SNAPY), m_snapY);
    valueFromVariantMap(vm, QLatin1String(KEY_DELTAX), m_deltaX);
    valueFromVariantMap(vm, QLatin1String(KEY_DELTAY), m_deltaY);
}

QVariantMap Grid::toVariantMap() const
{
    QVariantMap rc;
    addToVariantMap(rc);
    return rc;
}

void  Grid::addToVariantMap(QVariantMap& vm) const
{
    valueToVariantMap(m_visible, defaultVisible, QLatin1String(KEY_VISIBLE), vm);
    valueToVariantMap(m_snapX, defaultSnap, QLatin1String(KEY_SNAPX), vm);
    valueToVariantMap(m_snapY, defaultSnap, QLatin1String(KEY_SNAPY), vm);
    valueToVariantMap(m_deltaX, DEFAULT_GRID, QLatin1String(KEY_DELTAX), vm);
    valueToVariantMap(m_deltaY, DEFAULT_GRID, QLatin1String(KEY_DELTAY), vm);
}

void Grid::paint(QWidget *widget, QPaintEvent *e, bool needFrame) const
{
    QPainter p(widget);
    p.fillRect(e->rect(), widget->palette().brush(widget->backgroundRole()));

    if (m_visible) {
        const int xstart = (e->rect().x() / m_deltaX) * m_deltaX;
        const int ystart = (e->rect().y() / m_deltaY) * m_deltaY;

        const int xend = (e->rect().right()  / m_deltaX) * m_deltaX;
        const int yend = (e->rect().bottom() / m_deltaY) * m_deltaY;

        int pointCount = ((xend - xstart) / m_deltaX) * ((yend - ystart) * m_deltaY);

        typedef QVector<QPoint> Points;
        static Points points;
        if (points.empty()) {
            points.reserve(4096);
        } else {
            points.clear();
        }

        int i = 0;
        int x = xstart;
        int y = ystart;
        while (pointCount > 0) {
            while (i < pointCount) {
                points.push_back(QPoint(x, y));
                ++i;
                x += m_deltaX;
                if (x > xend) {
                    x = xstart;
                    y += m_deltaY;
                    if (y > yend) // probably never reached..
                        break;
                }
            }
            p.drawPoints( &(*points.begin()), i);
            pointCount -= i;
            i = 0;
        }
    }
    if (needFrame) {
        p.setPen(widget->palette().dark().color());
        p.drawRect(e->rect());
    }
}

QPoint Grid::snapPoint(const QPoint &p) const
{
    const int sx = m_snapX ? ((p.x() + m_deltaX / 2) / m_deltaX) * m_deltaX : p.x();
    const int sy = m_snapY ? ((p.y() + m_deltaY / 2) / m_deltaY) * m_deltaY : p.y();
    return QPoint(sx, sy);
}

int Grid::widgetHandleAdjustX(int x) const
{
    return m_snapX ? (x / m_deltaX) * m_deltaX + 1 : x;
}

int Grid::widgetHandleAdjustY(int y) const
{
    return m_snapY ? (y / m_deltaY) * m_deltaY + 1 : y;
}
}
