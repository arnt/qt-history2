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

#ifndef QRECTFLOAT_H
#define QRECTFLOAT_H

#ifndef QT_H
#include "qrect.h"
#include "qsizefloat.h"
#include "qpointfloat.h"
#endif // QT_H

#if defined(topLeft)
#error "Macro definition of topLeft conflicts with QRectF"
// don't just silently undo people's defines: #undef topLeft
#endif

class Q_GUI_EXPORT QRectF                                        // rectangle class
{
public:
    QRectF() { xp = yp = 0.; w = h = 0.; }
    QRectF(const QPointF &topleft, const QPointF &bottomright);
    QRectF(const QPointF &topleft, const QSizeF &size);
    QRectF(float left, float top, float width, float height);
    QRectF(const QRect &rect);

    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;
    QRectF normalize() const;

    float left() const;
    float top() const;
    float right() const;
    float bottom() const;

    float &rLeft();
    float &rTop();
    float &rWidth();
    float &rHeight();

    float x() const;
    float y() const;
    void setLeft(float pos);
    void setTop(float pos);
    void setRight(float pos);
    void setBottom(float pos);
    void setX(float x);
    void setY(float y);

    void setTopLeft(const QPointF &p);
    void setBottomRight(const QPointF &p);
    void setTopRight(const QPointF &p);
    void setBottomLeft(const QPointF &p);

    QPointF topLeft() const;
    QPointF bottomRight() const;
    QPointF topRight() const;
    QPointF bottomLeft() const;
    QPointF center() const;

    void moveLeft(float pos);
    void moveTop(float pos);
    void moveRight(float pos);
    void moveBottom(float pos);
    void moveTopLeft(const QPointF &p);
    void moveBottomRight(const QPointF &p);
    void moveTopRight(const QPointF &p);
    void moveBottomLeft(const QPointF &p);
    void moveCenter(const QPointF &p);
    void moveBy(float dx, float dy);
    void moveBy(const QPointF &p);

    void setRect(float x, float y, float w, float h);
    void getRect(float *x, float *y, float *w, float *h) const;

    void setCoords(float x1, float y1, float x2, float y2);
    void addCoords(float x1, float y1, float x2, float y2);
    void getCoords(float *x1, float *y1, float *x2, float *y2) const;

    QSizeF size() const;
    float width() const;
    float height() const;
    void setWidth(float w);
    void setHeight(float h);
    void setSize(const QSizeF &s);

    QRectF operator|(const QRectF &r) const;
    QRectF operator&(const QRectF &r) const;
    QRectF& operator|=(const QRectF &r);
    QRectF& operator&=(const QRectF &r);

    bool contains(const QPointF &p) const;
    bool contains(float x, float y) const;
    bool contains(const QRectF &r) const;
    QRectF unite(const QRectF &r) const;
    QRectF intersect(const QRectF &r) const;
    bool intersects(const QRectF &r) const;

    friend Q_GUI_EXPORT bool operator==(const QRectF &, const QRectF &);
    friend Q_GUI_EXPORT bool operator!=(const QRectF &, const QRectF &);

    QRect toRect() const;

private:
    float xp;
    float yp;
    float w;
    float h;
};

Q_GUI_EXPORT bool operator==(const QRectF &, const QRectF &);
Q_GUI_EXPORT bool operator!=(const QRectF &, const QRectF &);


/*****************************************************************************
  QRectF stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QRectF &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QRectF &);
#endif

/*****************************************************************************
  QRectF inline member functions
 *****************************************************************************/

inline QRectF::QRectF(float left, float top, float width, float height)
    : xp(left), yp(top), w(width), h(height)
{
}

inline QRectF::QRectF(const QPointF &topLeft, const QPointF &bottomRight)
{
    xp = topLeft.x();
    yp = topLeft.y();
    w = bottomRight.x() - xp;
    h = bottomRight.y() - yp;
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
{ return w >= 0. && h >= 0.; }

inline float QRectF::left() const
{ return xp; }

inline float QRectF::top() const
{ return yp; }

inline float QRectF::right() const
{ return xp + w; }

inline float QRectF::bottom() const
{ return yp + h; }

inline float &QRectF::rLeft()
{ return xp; }

inline float & QRectF::rTop()
{ return yp; }

inline float & QRectF::rWidth()
{ return w; }

inline float & QRectF::rHeight()
{ return h; }

inline float QRectF::x() const
{ return xp; }

inline float QRectF::y() const
{ return yp; }

inline void QRectF::setLeft(float pos)
{ xp = pos; }

inline void QRectF::setTop(float pos)
{ yp = pos; }

inline void QRectF::setRight(float pos)
{ w = pos - xp; }

inline void QRectF::setBottom(float pos)
{ h = pos - yp; }

inline void QRectF::setTopLeft(const QPointF &p)
{ xp = p.x(); yp = p.y(); }

inline void QRectF::setBottomRight(const QPointF &p)
{ w = p.x() - xp; h = p.y() - yp; }

inline void QRectF::setTopRight(const QPointF &p)
{ w = p.x() - xp; yp = p.y(); }

inline void QRectF::setBottomLeft(const QPointF &p)
{ xp = p.x(); h = p.y() - yp; }

inline void QRectF::setX(float x)
{ xp = x; }

inline void QRectF::setY(float y)
{ yp = y; }

inline QPointF QRectF::topLeft() const
{ return QPointF(xp, yp); }

inline QPointF QRectF::bottomRight() const
{ return QPointF(xp + w, yp + h); }

inline QPointF QRectF::topRight() const
{ return QPointF(xp + w, yp); }

inline QPointF QRectF::bottomLeft() const
{ return QPointF(xp, yp + h); }

inline QPointF QRectF::center() const
{ return QPointF(xp + w/2, yp + h/2); }

inline float QRectF::width() const
{ return w; }

inline float QRectF::height() const
{ return h; }

inline QSizeF QRectF::size() const
{ return QSizeF(w, h); }

inline void QRectF::moveBy(float dx, float dy)
{
    xp += dx;
    yp += dy;
}

inline void QRectF::moveBy(const QPointF &p)
{
    xp += p.x();
    yp += p.y();
}

inline void QRectF::moveLeft(float pos)
{ xp = pos; }

inline void QRectF::moveTop(float pos)
{ yp = pos; }

inline void QRectF::moveRight(float pos)
{
    xp = pos - w;
}

inline void QRectF::moveBottom(float pos)
{
    yp = pos - h;
}

inline void QRectF::moveTopLeft(const QPointF &p)
{
    xp = p.x();
    yp = p.y();
}

inline void QRectF::moveBottomRight(const QPointF &p)
{
    xp = p.x() - w;
    yp = p.y() - h;
}

inline void QRectF::moveTopRight(const QPointF &p)
{
    xp = p.x() - w;
    yp = p.y();
}

inline void QRectF::moveBottomLeft(const QPointF &p)
{
    xp = p.x();
    yp = p.y() - h;
}

inline void QRectF::getRect(float *x, float *y, float *w, float *h) const
{
    *x = this->xp;
    *y = this->yp;
    *w = this->w;
    *h = this->h;
}

inline void QRectF::setRect(float x, float y, float w, float h)
{
    this->xp = x;
    this->yp = y;
    this->w = w;
    this->h = h;
}

inline void QRectF::getCoords(float *xp1, float *yp1, float *xp2, float *yp2) const
{
    *xp1 = xp;
    *yp1 = yp;
    *xp2 = xp + w;
    *yp2 = yp + h;
}

inline void QRectF::setCoords(float xp1, float yp1, float xp2, float yp2)
{
    xp = xp1;
    yp = yp1;
    w = xp2 - xp1;
    h = yp2 - yp1;
}

inline void QRectF::addCoords(float xp1, float yp1, float xp2, float yp2)
{
    xp += xp1;
    yp += yp1;
    w += xp2 - xp1;
    h += yp2 - yp1;
}

inline void QRectF::setWidth(float w)
{ this->w = w; }

inline void QRectF::setHeight(float h)
{ this->h = h; }

inline void QRectF::setSize(const QSizeF &s)
{
    w = s.width();
    h = s.height();
}

inline bool QRectF::contains(float x, float y) const
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

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug, const QRectF &);
#endif

#endif // QRECT_H
