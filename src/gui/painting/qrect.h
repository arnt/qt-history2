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

#ifndef QT_H
#include "qsize.h"
#include "qpoint.h"
#endif // QT_H

#if defined(topLeft)
#error "Macro definition of topLeft conflicts with QRect"
// don't just silently undo people's defines: #undef topLeft
#endif

class Q_GUI_EXPORT QRect                                        // rectangle class
{
public:
    QRect()        { x1 = y1 = 0; x2 = y2 = -1; }
    QRect(const QPoint &topleft, const QPoint &bottomright);
    QRect(const QPoint &topleft, const QSize &size);
    QRect(int left, int top, int width, int height);

    bool   isNull()        const;
    bool   isEmpty()        const;
    bool   isValid()        const;
    QRect  normalize()        const;

    int           left()        const;
    int           top()        const;
    int           right()        const;
    int           bottom()        const;

    QCOORD &rLeft();
    QCOORD &rTop();
    QCOORD &rRight();
    QCOORD &rBottom();

    int           x()                const;
    int           y()                const;
    void   setLeft(int pos);
    void   setTop(int pos);
    void   setRight(int pos);
    void   setBottom(int pos);
    void   setX(int x);
    void   setY(int y);

    void   setTopLeft(const QPoint &p);
    void   setBottomRight(const QPoint &p);
    void   setTopRight(const QPoint &p);
    void   setBottomLeft(const QPoint &p);

    QPoint topLeft()         const;
    QPoint bottomRight() const;
    QPoint topRight()         const;
    QPoint bottomLeft()         const;
    QPoint center()         const;

    void   rect(int *x, int *y, int *w, int *h) const;
    void   coords(int *x1, int *y1, int *x2, int *y2) const;

    void   moveLeft(int pos);
    void   moveTop(int pos);
    void   moveRight(int pos);
    void   moveBottom(int pos);
    void   moveTopLeft(const QPoint &p);
    void   moveBottomRight(const QPoint &p);
    void   moveTopRight(const QPoint &p);
    void   moveBottomLeft(const QPoint &p);
    void   moveCenter(const QPoint &p);
    void   moveBy(int dx, int dy);
    void   moveBy(const QPoint &p);

    void   setRect(int x, int y, int w, int h);
    void   setCoords(int x1, int y1, int x2, int y2);
    void   addCoords(int x1, int y1, int x2, int y2);

    QSize  size()        const;
    int           width()        const;
    int           height()        const;
    void   setWidth(int w);
    void   setHeight(int h);
    void   setSize(const QSize &s);

    QRect  operator|(const QRect &r) const;
    QRect  operator&(const QRect &r) const;
    QRect&  operator|=(const QRect &r);
    QRect&  operator&=(const QRect &r);

    bool   contains(const QPoint &p, bool proper=false) const;
    bool   contains(int x, int y) const; // inline methods, _don't_ merge these
    bool   contains(int x, int y, bool proper) const;
    bool   contains(const QRect &r, bool proper=false) const;
    QRect  unite(const QRect &r) const;
    QRect  intersect(const QRect &r) const;
    bool   intersects(const QRect &r) const;

    friend Q_GUI_EXPORT bool operator==(const QRect &, const QRect &);
    friend Q_GUI_EXPORT bool operator!=(const QRect &, const QRect &);

    enum RectangleMode {
        ExclusiveRectangles = 0,
        InclusiveRectangles = 1
    };

    static void setRectangleMode(RectangleMode mode) { static_rect_mode = mode; }
    static RectangleMode rectangleMode() { return static_rect_mode; }

private:
#if defined(Q_WS_X11) || defined(Q_OS_TEMP)
    friend void qt_setCoords(QRect *r, int xp1, int yp1, int xp2, int yp2);
#endif
#if defined(Q_OS_MAC)
    QCOORD y1;
    QCOORD x1;
    QCOORD y2;
    QCOORD x2;
#else
    QCOORD x1;
    QCOORD y1;
    QCOORD x2;
    QCOORD y2;
#endif

    static RectangleMode static_rect_mode;

};

Q_GUI_EXPORT bool operator==(const QRect &, const QRect &);
Q_GUI_EXPORT bool operator!=(const QRect &, const QRect &);


/*****************************************************************************
  QRect stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QRect &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QRect &);
#endif

/*****************************************************************************
  QRect inline member functions
 *****************************************************************************/

inline QRect::QRect(int left, int top, int width, int height)
{
    x1 = (QCOORD)left;
    y1 = (QCOORD)top;
    x2 = (QCOORD)(left + width - static_rect_mode);
    y2 = (QCOORD)(top + height - static_rect_mode);
}

inline QRect::QRect(const QPoint &topLeft, const QPoint &bottomRight)
{
    x1 = (QCOORD)topLeft.x();
    y1 = (QCOORD)topLeft.y();
    x2 = (QCOORD)bottomRight.x();
    y2 = (QCOORD)bottomRight.y();
}

inline QRect::QRect(const QPoint &topLeft, const QSize &size)
{
    x1 = (QCOORD)topLeft.x();
    y1 = (QCOORD)topLeft.y();
    x2 = (QCOORD)(x1+size.width() - static_rect_mode);
    y2 = (QCOORD)(y1+size.height() - static_rect_mode);
}

inline bool QRect::isNull() const
{ return x2 == x1 - static_rect_mode && y2 == y1 - static_rect_mode; }

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

inline QCOORD &QRect::rLeft()
{ return x1; }

inline QCOORD & QRect::rTop()
{ return y1; }

inline QCOORD & QRect::rRight()
{ return x2; }

inline QCOORD & QRect::rBottom()
{ return y2; }

inline int QRect::x() const
{ return x1; }

inline int QRect::y() const
{ return y1; }

inline void QRect::setLeft(int pos)
{ x1 = (QCOORD)pos; }

inline void QRect::setTop(int pos)
{ y1 = (QCOORD)pos; }

inline void QRect::setRight(int pos)
{ x2 = (QCOORD)pos; }

inline void QRect::setBottom(int pos)
{ y2 = (QCOORD)pos; }

inline void QRect::setTopLeft(const QPoint &p)
{ x1 = p.x(); y1 = p.y(); }

inline void QRect::setBottomRight(const QPoint &p)
{ x2 = p.x(); y2 = p.y(); }

inline void QRect::setTopRight(const QPoint &p)
{ x2 = p.x(); y1 = p.y(); }

inline void QRect::setBottomLeft(const QPoint &p)
{ x1 = p.x(); y2 = p.y(); }

inline void QRect::setX(int x)
{ x1 = (QCOORD)x; }

inline void QRect::setY(int y)
{ y1 = (QCOORD)y; }

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
{ return  x2 - x1 + static_rect_mode; }

inline int QRect::height() const
{ return  y2 - y1 + static_rect_mode; }

inline QSize QRect::size() const
{ return QSize(x2 - x1 + static_rect_mode, y2-y1 + static_rect_mode); }

inline void QRect::moveBy(int dx, int dy)
{
    x1 += (QCOORD)dx;
    y1 += (QCOORD)dy;
    x2 += (QCOORD)dx;
    y2 += (QCOORD)dy;
}

inline void QRect::moveBy(const QPoint &p)
{
    x1 += (QCOORD)p.x();
    y1 += (QCOORD)p.y();
    x2 += (QCOORD)p.x();
    y2 += (QCOORD)p.y();
}

inline void QRect::moveLeft(int pos)
{ x2 += (QCOORD)(pos - x1); x1 = (QCOORD)pos; }

inline void QRect::moveTop(int pos)
{ y2 += (QCOORD)(pos - y1); y1 = (QCOORD)pos; }

inline void QRect::moveRight(int pos)
{
    x1 += (QCOORD)(pos - x2);
    x2 = (QCOORD)pos;
}

inline void QRect::moveBottom(int pos)
{
    y1 += (QCOORD)(pos - y2);
    y2 = (QCOORD)pos;
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

inline void QRect::rect(int *x, int *y, int *w, int *h) const
{
    *x = x1;
    *y = y1;
    *w = x2-x1 + static_rect_mode;
    *h = y2-y1 + static_rect_mode;
}

inline void QRect::setRect(int x, int y, int w, int h)
{
    x1 = (QCOORD)x;
    y1 = (QCOORD)y;
    x2 = (QCOORD)(x+w - static_rect_mode);
    y2 = (QCOORD)(y+h - static_rect_mode);
}

inline void QRect::coords(int *xp1, int *yp1, int *xp2, int *yp2) const
{
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

inline void QRect::setCoords(int xp1, int yp1, int xp2, int yp2)
{
    x1 = (QCOORD)xp1;
    y1 = (QCOORD)yp1;
    x2 = (QCOORD)xp2;
    y2 = (QCOORD)yp2;
}

inline void QRect::addCoords(int xp1, int yp1, int xp2, int yp2)
{
    x1 += (QCOORD)xp1;
    y1 += (QCOORD)yp1;
    x2 += (QCOORD)xp2;
    y2 += (QCOORD)yp2;
}

inline void QRect::setWidth(int w)
{ x2 = (QCOORD)(x1 + w - static_rect_mode); }

inline void QRect::setHeight(int h)
{ y2 = (QCOORD)(y1 + h - static_rect_mode); }

inline void QRect::setSize(const QSize &s)
{
    x2 = (QCOORD)(s.width()  + x1 - static_rect_mode);
    y2 = (QCOORD)(s.height() + y1 - static_rect_mode);
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

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug, const QRect &);
#endif

#endif // QRECT_H
