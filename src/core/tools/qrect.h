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

#ifndef QRECT_H
#define QRECT_H

#include "QtCore/qsize.h"
#include "QtCore/qpoint.h"

#ifdef topLeft
#error "qrect.h must be included before any header file that defines topLeft"
#endif

class Q_CORE_EXPORT QRect                                        // rectangle class
{
public:
    QRect() { x1 = y1 = 0; x2 = y2 = -1; }
    QRect(const QPoint &topleft, const QPoint &bottomright);
    QRect(const QPoint &topleft, const QSize &size);
    QRect(int left, int top, int width, int height);

    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;
    QRect normalize() const;

    int left() const;
    int top() const;
    int right() const;
    int bottom() const;

#ifdef QT_COMPAT
    QT_COMPAT int &rLeft() { return x1; }
    QT_COMPAT int &rTop() { return y1; }
    QT_COMPAT int &rRight() { return x2; }
    QT_COMPAT int &rBottom() { return y2; }
#endif

    int x() const;
    int y() const;
    void setLeft(int pos);
    void setTop(int pos);
    void setRight(int pos);
    void setBottom(int pos);
    void setX(int x);
    void setY(int y);

    void setTopLeft(const QPoint &p);
    void setBottomRight(const QPoint &p);
    void setTopRight(const QPoint &p);
    void setBottomLeft(const QPoint &p);

    QPoint topLeft() const;
    QPoint bottomRight() const;
    QPoint topRight() const;
    QPoint bottomLeft() const;
    QPoint center() const;

    void moveLeft(int pos);
    void moveTop(int pos);
    void moveRight(int pos);
    void moveBottom(int pos);
    void moveTopLeft(const QPoint &p);
    void moveBottomRight(const QPoint &p);
    void moveTopRight(const QPoint &p);
    void moveBottomLeft(const QPoint &p);
    void moveCenter(const QPoint &p);

    inline void translate(int dx, int dy);
    inline void translate(const QPoint &p);

    void moveTo(int x, int t);
    void moveTo(const QPoint &p);

#ifdef QT_COMPAT
    QT_COMPAT void moveBy(int dx, int dy) { translate(dx, dy); }
    QT_COMPAT void moveBy(const QPoint &p) { translate(p); }
#endif

    void setRect(int x, int y, int w, int h);
    inline void getRect(int *x, int *y, int *w, int *h) const;

    void setCoords(int x1, int y1, int x2, int y2);
    void addCoords(int x1, int y1, int x2, int y2);
    inline void getCoords(int *x1, int *y1, int *x2, int *y2) const;

    QRect adjusted(int x1, int y1, int x2, int y2) const;

    QSize size() const;
    int width() const;
    int height() const;
    void setWidth(int w);
    void setHeight(int h);
    void setSize(const QSize &s);

    QRect operator|(const QRect &r) const;
    QRect operator&(const QRect &r) const;
    QRect& operator|=(const QRect &r);
    QRect& operator&=(const QRect &r);

    bool contains(const QPoint &p, bool proper=false) const;
    bool contains(int x, int y) const; // inline methods, _don't_ merge these
    bool contains(int x, int y, bool proper) const;
    bool contains(const QRect &r, bool proper=false) const;
    QRect unite(const QRect &r) const;
    QRect intersect(const QRect &r) const;
    bool intersects(const QRect &r) const;

    friend Q_CORE_EXPORT inline bool operator==(const QRect &, const QRect &);
    friend Q_CORE_EXPORT inline bool operator!=(const QRect &, const QRect &);

#ifdef QT_COMPAT
    inline QT_COMPAT void rect(int *x, int *y, int *w, int *h) const { getRect(x, y, w, h); }
    inline QT_COMPAT void coords(int *x1, int *y1, int *x2, int *y2) const { getCoords(x1, y1, x2, y2); }
#endif

private:
#if defined(Q_WS_X11) || defined(Q_OS_TEMP)
    friend void qt_setCoords(QRect *r, int xp1, int yp1, int xp2, int yp2);
#endif
#if defined(Q_OS_MAC)
    int y1;
    int x1;
    int y2;
    int x2;
#else
    int x1;
    int y1;
    int x2;
    int y2;
#endif

};
Q_DECLARE_TYPEINFO(QRect, Q_MOVABLE_TYPE);

Q_CORE_EXPORT inline bool operator==(const QRect &, const QRect &);
Q_CORE_EXPORT inline bool operator!=(const QRect &, const QRect &);


/*****************************************************************************
  QRect stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QRect &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QRect &);
#endif

/*****************************************************************************
  QRect inline member functions
 *****************************************************************************/

inline QRect::QRect(int left, int top, int width, int height)
{
    x1 = left;
    y1 = top;
    x2 = (left + width - 1);
    y2 = (top + height - 1);
}

inline QRect::QRect(const QPoint &topLeft, const QPoint &bottomRight)
{
    x1 = topLeft.x();
    y1 = topLeft.y();
    x2 = bottomRight.x();
    y2 = bottomRight.y();
}

inline QRect::QRect(const QPoint &topLeft, const QSize &size)
{
    x1 = topLeft.x();
    y1 = topLeft.y();
    x2 = (x1+size.width() - 1);
    y2 = (y1+size.height() - 1);
}

inline bool QRect::isNull() const
{ return x2 == x1 - 1 && y2 == y1 - 1; }

inline bool QRect::isEmpty() const
{ return x1 > x2 || y1 > y2; }

inline bool QRect::isValid() const
{ return x1 <= x2 && y1 <= y2; }

inline int QRect::left() const
{ return x1; }

inline int QRect::top() const
{ return y1; }

inline int QRect::right() const
{ return x2; }

inline int QRect::bottom() const
{ return y2; }

inline int QRect::x() const
{ return x1; }

inline int QRect::y() const
{ return y1; }

inline void QRect::setLeft(int pos)
{ x1 = pos; }

inline void QRect::setTop(int pos)
{ y1 = pos; }

inline void QRect::setRight(int pos)
{ x2 = pos; }

inline void QRect::setBottom(int pos)
{ y2 = pos; }

inline void QRect::setTopLeft(const QPoint &p)
{ x1 = p.x(); y1 = p.y(); }

inline void QRect::setBottomRight(const QPoint &p)
{ x2 = p.x(); y2 = p.y(); }

inline void QRect::setTopRight(const QPoint &p)
{ x2 = p.x(); y1 = p.y(); }

inline void QRect::setBottomLeft(const QPoint &p)
{ x1 = p.x(); y2 = p.y(); }

inline void QRect::setX(int x)
{ x1 = x; }

inline void QRect::setY(int y)
{ y1 = y; }

inline QPoint QRect::topLeft() const
{ return QPoint(x1, y1); }

inline QPoint QRect::bottomRight() const
{ return QPoint(x2, y2); }

inline QPoint QRect::topRight() const
{ return QPoint(x2, y1); }

inline QPoint QRect::bottomLeft() const
{ return QPoint(x1, y2); }

inline QPoint QRect::center() const
{ return QPoint((x1+x2)/2, (y1+y2)/2); }

inline int QRect::width() const
{ return  x2 - x1 + 1; }

inline int QRect::height() const
{ return  y2 - y1 + 1; }

inline QSize QRect::size() const
{ return QSize(width(), height()); }

inline void QRect::translate(int dx, int dy)
{
    x1 += dx;
    y1 += dy;
    x2 += dx;
    y2 += dy;
}

inline void QRect::translate(const QPoint &p)
{
    x1 += p.x();
    y1 += p.y();
    x2 += p.x();
    y2 += p.y();
}

inline void QRect::moveTo(int x, int y)
{
    x2 += x - x1;
    y2 += y - y1;
    x1 = x;
    y1 = y;
}

inline void QRect::moveTo(const QPoint &p)
{
    x2 += p.x() - x1;
    y2 += p.y() - y1;
    x1 = p.x();
    y1 = p.y();
}

inline void QRect::moveLeft(int pos)
{ x2 += (pos - x1); x1 = pos; }

inline void QRect::moveTop(int pos)
{ y2 += (pos - y1); y1 = pos; }

inline void QRect::moveRight(int pos)
{
    x1 += (pos - x2);
    x2 = pos;
}

inline void QRect::moveBottom(int pos)
{
    y1 += (pos - y2);
    y2 = pos;
}

inline void QRect::moveTopLeft(const QPoint &p)
{
    moveLeft(p.x());
    moveTop(p.y());
}

inline void QRect::moveBottomRight(const QPoint &p)
{
    moveRight(p.x());
    moveBottom(p.y());
}

inline void QRect::moveTopRight(const QPoint &p)
{
    moveRight(p.x());
    moveTop(p.y());
}

inline void QRect::moveBottomLeft(const QPoint &p)
{
    moveLeft(p.x());
    moveBottom(p.y());
}

inline void QRect::getRect(int *x, int *y, int *w, int *h) const
{
    *x = x1;
    *y = y1;
    *w = x2 - x1 + 1;
    *h = y2 - y1 + 1;
}

inline void QRect::setRect(int x, int y, int w, int h)
{
    x1 = x;
    y1 = y;
    x2 = (x + w - 1);
    y2 = (y + h - 1);
}

inline void QRect::getCoords(int *xp1, int *yp1, int *xp2, int *yp2) const
{
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

inline void QRect::setCoords(int xp1, int yp1, int xp2, int yp2)
{
    x1 = xp1;
    y1 = yp1;
    x2 = xp2;
    y2 = yp2;
}

inline void QRect::addCoords(int xp1, int yp1, int xp2, int yp2)
{
    x1 += xp1;
    y1 += yp1;
    x2 += xp2;
    y2 += yp2;
}

inline QRect QRect::adjusted(int xp1, int yp1, int xp2, int yp2) const
{ return QRect(QPoint(x1 + xp1, y1 + yp1), QPoint(x2 + xp2, y2 + yp2)); }

inline void QRect::setWidth(int w)
{ x2 = (x1 + w - 1); }

inline void QRect::setHeight(int h)
{ y2 = (y1 + h - 1); }

inline void QRect::setSize(const QSize &s)
{
    x2 = (s.width()  + x1 - 1);
    y2 = (s.height() + y1 - 1);
}

inline bool QRect::contains(int x, int y, bool proper) const
{
    if (proper)
        return x > x1 && x < x2 &&
               y > y1 && y < y2;
    else
        return x >= x1 && x <= x2 &&
               y >= y1 && y <= y2;
}

inline bool QRect::contains(int x, int y) const
{
    return x >= x1 && x <= x2 &&
           y >= y1 && y <= y2;
}

inline bool QRect::contains(const QPoint &p, bool proper) const
{
    if (proper)
        return p.x() > x1 && p.x() < x2 &&
               p.y() > y1 && p.y() < y2;
    else
        return p.x() >= x1 && p.x() <= x2 &&
               p.y() >= y1 && p.y() <= y2;
}

inline QRect& QRect::operator|=(const QRect &r)
{
    *this = *this | r;
    return *this;
}

inline QRect& QRect::operator&=(const QRect &r)
{
    *this = *this & r;
    return *this;
}

inline QRect QRect::intersect(const QRect &r) const
{
    return *this & r;
}
inline QRect QRect::unite(const QRect &r) const
{
    return *this | r;
}

inline bool operator==(const QRect &r1, const QRect &r2)
{
    return r1.x1==r2.x1 && r1.x2==r2.x2 && r1.y1==r2.y1 && r1.y2==r2.y2;
}

inline bool operator!=(const QRect &r1, const QRect &r2)
{
    return r1.x1!=r2.x1 || r1.x2!=r2.x2 || r1.y1!=r2.y1 || r1.y2!=r2.y2;
}

#ifndef QT_NO_DEBUG_OUTPUT
Q_CORE_EXPORT QDebug operator<<(QDebug, const QRect &);
#endif


class Q_CORE_EXPORT QRectF                                        // rectangle class
{
public:
    QRectF() { xp = yp = 0.; w = h = 0.; }
    QRectF(const QPointF &topleft, const QSizeF &size);
    QRectF(qReal left, qReal top, qReal width, qReal height);
    QRectF(const QRect &rect);

    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;
    QRectF normalize() const;

    qReal x() const;
    qReal y() const;

    QPointF center() const;

    void translate(qReal dx, qReal dy);
    void translate(const QPointF &p);

    void moveTo(qReal x, qReal t);
    void moveTo(const QPointF &p);

    void setRect(qReal x, qReal y, qReal w, qReal h);
    void getRect(qReal *x, qReal *y, qReal *w, qReal *h) const;

    void setCoords(qReal x1, qReal y1, qReal x2, qReal y2);
    void addCoords(qReal x1, qReal y1, qReal x2, qReal y2);
    void getCoords(qReal *x1, qReal *y1, qReal *x2, qReal *y2) const;

    QRectF adjusted(qReal x1, qReal y1, qReal x2, qReal y2) const;

    QPointF origin() const;
    QSizeF size() const;
    qReal width() const;
    qReal height() const;
    void setWidth(qReal w);
    void setHeight(qReal h);
    void setSize(const QSizeF &s);

    QRectF operator|(const QRectF &r) const;
    QRectF operator&(const QRectF &r) const;
    QRectF& operator|=(const QRectF &r);
    QRectF& operator&=(const QRectF &r);

    bool contains(const QPointF &p) const;
    bool contains(qReal x, qReal y) const;
    bool contains(const QRectF &r) const;
    QRectF unite(const QRectF &r) const;
    QRectF intersect(const QRectF &r) const;
    bool intersects(const QRectF &r) const;

    friend Q_CORE_EXPORT inline bool operator==(const QRectF &, const QRectF &);
    friend Q_CORE_EXPORT inline bool operator!=(const QRectF &, const QRectF &);

    QRect toRect() const;

private:
    qReal xp;
    qReal yp;
    qReal w;
    qReal h;
};
Q_DECLARE_TYPEINFO(QRectF, Q_MOVABLE_TYPE);

Q_CORE_EXPORT inline bool operator==(const QRectF &, const QRectF &);
Q_CORE_EXPORT inline bool operator!=(const QRectF &, const QRectF &);


/*****************************************************************************
  QRectF stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QRectF &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QRectF &);
#endif

/*****************************************************************************
  QRectF inline member functions
 *****************************************************************************/

inline QRectF::QRectF(qReal left, qReal top, qReal width, qReal height)
    : xp(left), yp(top), w(width), h(height)
{
}

inline QRectF::QRectF(const QPointF &topLeft, const QSizeF &size)
{
    xp = topLeft.x();
    yp = topLeft.y();
    w = size.width();
    h = size.height();
}

inline QRectF::QRectF(const QRect &r)
    : xp(r.x()), yp(r.y()), w(r.width()), h(r.height())
{
}

inline bool QRectF::isNull() const
{ return w == 0 && h == 0; }

inline bool QRectF::isEmpty() const
{ return w <= 0. || h <= 0.; }

inline bool QRectF::isValid() const
{ return w > 0. && h > 0.; }

inline qReal QRectF::x() const
{ return xp; }

inline qReal QRectF::y() const
{ return yp; }

inline QPointF QRectF::center() const
{ return QPointF(xp + w/2, yp + h/2); }

inline qReal QRectF::width() const
{ return w; }

inline qReal QRectF::height() const
{ return h; }

inline QSizeF QRectF::size() const
{ return QSizeF(w, h); }

inline QPointF QRectF::origin() const
{ return QPointF(xp, yp); }

inline void QRectF::translate(qReal dx, qReal dy)
{
    xp += dx;
    yp += dy;
}

inline void QRectF::translate(const QPointF &p)
{
    xp += p.x();
    yp += p.y();
}


inline void QRectF::getRect(qReal *x, qReal *y, qReal *w, qReal *h) const
{
    *x = this->xp;
    *y = this->yp;
    *w = this->w;
    *h = this->h;
}

inline void QRectF::setRect(qReal x, qReal y, qReal w, qReal h)
{
    this->xp = x;
    this->yp = y;
    this->w = w;
    this->h = h;
}

inline void QRectF::getCoords(qReal *xp1, qReal *yp1, qReal *xp2, qReal *yp2) const
{
    *xp1 = xp;
    *yp1 = yp;
    *xp2 = xp + w;
    *yp2 = yp + h;
}

inline void QRectF::setCoords(qReal xp1, qReal yp1, qReal xp2, qReal yp2)
{
    xp = xp1;
    yp = yp1;
    w = xp2 - xp1;
    h = yp2 - yp1;
}

inline void QRectF::addCoords(qReal xp1, qReal yp1, qReal xp2, qReal yp2)
{
    xp += xp1;
    yp += yp1;
    w += xp2 - xp1;
    h += yp2 - yp1;
}

inline QRectF QRectF::adjusted(qReal xp1, qReal yp1, qReal xp2, qReal yp2) const
{ return QRectF(xp + xp1, yp + yp2, w + xp2 - xp1, h + yp2 - yp1); }

inline void QRectF::setWidth(qReal w)
{ this->w = w; }

inline void QRectF::setHeight(qReal h)
{ this->h = h; }

inline void QRectF::setSize(const QSizeF &s)
{
    w = s.width();
    h = s.height();
}

inline bool QRectF::contains(qReal x, qReal y) const
{
    return x >= xp && x < xp + w &&
           y >= yp && y < yp + h;
}

inline bool QRectF::contains(const QPointF &p) const
{
    return p.x() >= xp && p.x() < xp + w &&
           p.y() >= yp && p.y() < yp + h;
}

inline QRectF& QRectF::operator|=(const QRectF &r)
{
    *this = *this | r;
    return *this;
}

inline QRectF& QRectF::operator&=(const QRectF &r)
{
    *this = *this & r;
    return *this;
}

inline QRectF QRectF::intersect(const QRectF &r) const
{
    return *this & r;
}
inline QRectF QRectF::unite(const QRectF &r) const
{
    return *this | r;
}

inline bool operator==(const QRectF &r1, const QRectF &r2)
{
    return r1.xp == r2.xp && r1.yp == r2.yp && r1.w == r2.w && r1.h == r2.h;
}

inline bool operator!=(const QRectF &r1, const QRectF &r2)
{
    return r1.xp != r2.xp || r1.yp != r2.yp || r1.w != r2.w || r1.h != r2.h;
}

inline QRect QRectF::toRect() const
{
    return QRect(qRound(xp), qRound(yp), qRound(w), qRound(h));
}

#ifndef QT_NO_DEBUG_OUTPUT
Q_CORE_EXPORT QDebug operator<<(QDebug, const QRectF &);
#endif

#endif // QRECT_H
