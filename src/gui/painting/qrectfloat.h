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
#include "qsizefloat.h"
#include "qpointfloat.h"
#endif // QT_H

#if defined(topLeft)
#error "Macro definition of topLeft conflicts with QRectFloat"
// don't just silently undo people's defines: #undef topLeft
#endif

class Q_GUI_EXPORT QRectFloat                                        // rectangle class
{
public:
    QRectFloat() { xp = yp = 0.; w = h = 0.; }
    QRectFloat(const QPointFloat &topleft, const QPointFloat &bottomright);
    QRectFloat(const QPointFloat &topleft, const QSizeFloat &size);
    QRectFloat(float left, float top, float width, float height);
    QRectFloat(const QRect &rect);

    bool isNull() const;
    bool isEmpty() const;
    bool isValid() const;
    QRectFloat normalize() const;

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

    void setTopLeft(const QPointFloat &p);
    void setBottomRight(const QPointFloat &p);
    void setTopRight(const QPointFloat &p);
    void setBottomLeft(const QPointFloat &p);

    QPointFloat topLeft() const;
    QPointFloat bottomRight() const;
    QPointFloat topRight() const;
    QPointFloat bottomLeft() const;
    QPointFloat center() const;

    void moveLeft(float pos);
    void moveTop(float pos);
    void moveRight(float pos);
    void moveBottom(float pos);
    void moveTopLeft(const QPointFloat &p);
    void moveBottomRight(const QPointFloat &p);
    void moveTopRight(const QPointFloat &p);
    void moveBottomLeft(const QPointFloat &p);
    void moveCenter(const QPointFloat &p);
    void moveBy(float dx, float dy);
    void moveBy(const QPointFloat &p);

    void setRect(float x, float y, float w, float h);
    void getRect(float *x, float *y, float *w, float *h) const;

    void setCoords(float x1, float y1, float x2, float y2);
    void addCoords(float x1, float y1, float x2, float y2);
    void getCoords(float *x1, float *y1, float *x2, float *y2) const;

    QSizeFloat size() const;
    float width() const;
    float height() const;
    void setWidth(float w);
    void setHeight(float h);
    void setSize(const QSizeFloat &s);

    QRectFloat operator|(const QRectFloat &r) const;
    QRectFloat operator&(const QRectFloat &r) const;
    QRectFloat& operator|=(const QRectFloat &r);
    QRectFloat& operator&=(const QRectFloat &r);

    bool contains(const QPointFloat &p) const;
    bool contains(float x, float y) const;
    bool contains(const QRectFloat &r) const;
    QRectFloat unite(const QRectFloat &r) const;
    QRectFloat intersect(const QRectFloat &r) const;
    bool intersects(const QRectFloat &r) const;

    friend Q_GUI_EXPORT bool operator==(const QRectFloat &, const QRectFloat &);
    friend Q_GUI_EXPORT bool operator!=(const QRectFloat &, const QRectFloat &);

    QRect toRect() const;

private:
    float xp;
    float yp;
    float w;
    float h;
};

Q_GUI_EXPORT bool operator==(const QRectFloat &, const QRectFloat &);
Q_GUI_EXPORT bool operator!=(const QRectFloat &, const QRectFloat &);


/*****************************************************************************
  QRectFloat stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QRectFloat &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QRectFloat &);
#endif

/*****************************************************************************
  QRectFloat inline member functions
 *****************************************************************************/

inline QRectFloat::QRectFloat(float left, float top, float width, float height)
    : xp(left), yp(top), w(width), h(height)
{
}

inline QRectFloat::QRectFloat(const QPointFloat &topLeft, const QPointFloat &bottomRight)
{
    xp = topLeft.x();
    yp = topLeft.y();
    w = bottomRight.x() - xp;
    h = bottomRight.y() - yp;
}

inline QRectFloat::QRectFloat(const QPointFloat &topLeft, const QSizeFloat &size)
{
    xp = topLeft.x();
    yp = topLeft.y();
    w = size.width();
    h = size.height();
}

inline QRectFloat::QRectFloat(const QRect &r)
    : xp(r.x()), yp(r.y()), w(r.width()), h(r.height())
{
}

inline bool QRectFloat::isNull() const
{ return w == 0 && h == 0; }

inline bool QRectFloat::isEmpty() const
{ return w <= 0. || h <= 0.; }

inline bool QRectFloat::isValid() const
{ return w >= 0. && h >= 0.; }

inline float QRectFloat::left() const
{ return xp; }

inline float QRectFloat::top() const
{ return yp; }

inline float QRectFloat::right() const
{ return xp + w; }

inline float QRectFloat::bottom() const
{ return yp + h; }

inline float &QRectFloat::rLeft()
{ return xp; }

inline float & QRectFloat::rTop()
{ return yp; }

inline float & QRectFloat::rWidth()
{ return w; }

inline float & QRectFloat::rHeight()
{ return h; }

inline float QRectFloat::x() const
{ return xp; }

inline float QRectFloat::y() const
{ return yp; }

inline void QRectFloat::setLeft(float pos)
{ xp = pos; }

inline void QRectFloat::setTop(float pos)
{ yp = pos; }

inline void QRectFloat::setRight(float pos)
{ w = pos - xp; }

inline void QRectFloat::setBottom(float pos)
{ h = pos - yp; }

inline void QRectFloat::setTopLeft(const QPointFloat &p)
{ xp = p.x(); yp = p.y(); }

inline void QRectFloat::setBottomRight(const QPointFloat &p)
{ w = p.x() - xp; h = p.y() - yp; }

inline void QRectFloat::setTopRight(const QPointFloat &p)
{ w = p.x() - xp; yp = p.y(); }

inline void QRectFloat::setBottomLeft(const QPointFloat &p)
{ xp = p.x(); h = p.y() - yp; }

inline void QRectFloat::setX(float x)
{ xp = x; }

inline void QRectFloat::setY(float y)
{ yp = y; }

inline QPointFloat QRectFloat::topLeft() const
{ return QPointFloat(xp, yp); }

inline QPointFloat QRectFloat::bottomRight() const
{ return QPointFloat(xp + w, yp + h); }

inline QPointFloat QRectFloat::topRight() const
{ return QPointFloat(xp + w, yp); }

inline QPointFloat QRectFloat::bottomLeft() const
{ return QPointFloat(xp, yp + h); }

inline QPointFloat QRectFloat::center() const
{ return QPointFloat(xp + w/2, yp + h/2); }

inline float QRectFloat::width() const
{ return w; }

inline float QRectFloat::height() const
{ return h; }

inline QSizeFloat QRectFloat::size() const
{ return QSizeFloat(w, h); }

inline void QRectFloat::moveBy(float dx, float dy)
{
    xp += dx;
    yp += dy;
}

inline void QRectFloat::moveBy(const QPointFloat &p)
{
    xp += p.x();
    yp += p.y();
}

inline void QRectFloat::moveLeft(float pos)
{ xp = pos; }

inline void QRectFloat::moveTop(float pos)
{ yp = pos; }

inline void QRectFloat::moveRight(float pos)
{
    xp = pos - w;
}

inline void QRectFloat::moveBottom(float pos)
{
    yp = pos - h;
}

inline void QRectFloat::moveTopLeft(const QPointFloat &p)
{
    xp = p.x();
    yp = p.y();
}

inline void QRectFloat::moveBottomRight(const QPointFloat &p)
{
    xp = p.x() - w;
    yp = p.y() - h;
}

inline void QRectFloat::moveTopRight(const QPointFloat &p)
{
    xp = p.x() - w;
    yp = p.y();
}

inline void QRectFloat::moveBottomLeft(const QPointFloat &p)
{
    xp = p.x();
    yp = p.y() - h;
}

inline void QRectFloat::getRect(float *x, float *y, float *w, float *h) const
{
    *x = this->xp;
    *y = this->yp;
    *w = this->w;
    *h = this->h;
}

inline void QRectFloat::setRect(float x, float y, float w, float h)
{
    this->xp = x;
    this->yp = y;
    this->w = w;
    this->h = h;
}

inline void QRectFloat::getCoords(float *xp1, float *yp1, float *xp2, float *yp2) const
{
    *xp1 = xp;
    *yp1 = yp;
    *xp2 = xp + w;
    *yp2 = yp + h;
}

inline void QRectFloat::setCoords(float xp1, float yp1, float xp2, float yp2)
{
    xp = xp1;
    yp = yp1;
    w = xp2 - xp1;
    h = yp2 - yp1;
}

inline void QRectFloat::addCoords(float xp1, float yp1, float xp2, float yp2)
{
    xp += xp1;
    yp += yp1;
    w += xp2 - xp1;
    h += yp2 - yp1;
}

inline void QRectFloat::setWidth(float w)
{ this->w = w; }

inline void QRectFloat::setHeight(float h)
{ this->h = h; }

inline void QRectFloat::setSize(const QSizeFloat &s)
{
    w = s.width();
    h = s.height();
}

inline bool QRectFloat::contains(float x, float y) const
{
    return x >= xp && x < xp + w &&
           y >= yp && y < yp + h;
}

inline bool QRectFloat::contains(const QPointFloat &p) const
{
    return p.x() >= xp && p.x() < xp + w &&
           p.y() >= yp && p.y() < yp + h;
}

inline QRectFloat& QRectFloat::operator|=(const QRectFloat &r)
{
    *this = *this | r;
    return *this;
}

inline QRectFloat& QRectFloat::operator&=(const QRectFloat &r)
{
    *this = *this & r;
    return *this;
}

inline QRectFloat QRectFloat::intersect(const QRectFloat &r) const
{
    return *this & r;
}
inline QRectFloat QRectFloat::unite(const QRectFloat &r) const
{
    return *this | r;
}

inline bool operator==(const QRectFloat &r1, const QRectFloat &r2)
{
    return r1.xp == r2.xp && r1.yp == r2.yp && r1.w == r2.w && r1.h == r2.h;
}

inline bool operator!=(const QRectFloat &r1, const QRectFloat &r2)
{
    return r1.xp != r2.xp || r1.yp != r2.yp || r1.w != r2.w || r1.h != r2.h;
}

inline QRect QRectFloat::toRect() const
{
    return QRect(qRound(xp), qRound(yp), qRound(w), qRound(h));
}

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug, const QRectFloat &);
#endif

#endif // QRECT_H
