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

#ifndef QMATRIX_H
#define QMATRIX_H

#include "qline.h"
#include "qpoint.h"
#include "qpolygon.h"
#include "qrect.h"
#include "qregion.h"
#include "qwindowdefs.h"
#ifndef QT_NO_MATRIX

class QPainterPath;

class Q_GUI_EXPORT QMatrix // 2D transform matrix
{
public:
    QMatrix();
    QMatrix(qReal m11, qReal m12, qReal m21, qReal m22,
            qReal dx, qReal dy);
    QMatrix(const QMatrix &matrix);

    void setMatrix(qReal m11, qReal m12, qReal m21, qReal m22,
                   qReal dx, qReal dy);

    qReal m11() const { return _m11; }
    qReal m12() const { return _m12; }
    qReal m21() const { return _m21; }
    qReal m22() const { return _m22; }
    qReal dx() const { return _dx; }
    qReal dy() const { return _dy; }

    void map(int x, int y, int *tx, int *ty) const;
    void map(qReal x, qReal y, qReal *tx, qReal *ty) const;
    QRect mapRect(const QRect &) const;
    QRectF mapRect(const QRectF &) const;

    QPoint map(const QPoint &p) const;
    QPointF map(const QPointF&p) const;
    QLineF map(const QLineF &l) const;
    QPolygonF map(const QPolygonF &a) const;
    QPolygon map(const QPolygon &a) const;
    QRegion map(const QRegion &r) const;
    QPainterPath map(const QPainterPath &p) const;
    QPolygon mapToPolygon(const QRect &r) const;

    void reset();
    inline bool isIdentity() const;

    QMatrix &translate(qReal dx, qReal dy);
    QMatrix &scale(qReal sx, qReal sy);
    QMatrix &shear(qReal sh, qReal sv);
    QMatrix &rotate(qReal a);

    bool isInvertible() const { return (_m11*_m22 - _m12*_m21) != 0; }
    qReal det() const { return _m11*_m22 - _m12*_m21; }

    QMatrix invert(bool * = 0) const;

    bool operator==(const QMatrix &) const;
    bool operator!=(const QMatrix &) const;

    QMatrix &operator*=(const QMatrix &);
    QMatrix operator*(const QMatrix &o) const;

    QMatrix &operator=(const QMatrix &);

#ifdef QT_COMPAT
    inline QT_COMPAT QRect map(const QRect &r) const { return mapRect(r); }
    QT_COMPAT QRegion mapToRegion(const QRect &r) const;
#endif

private:
    qReal _m11, _m12;
    qReal _m21, _m22;
    qReal _dx, _dy;
};
Q_DECLARE_TYPEINFO(QMatrix, Q_MOVABLE_TYPE);

// mathematical semantics
inline Q_GUI_EXPORT QPoint operator*(const QPoint &p, const QMatrix &m)
{ return m.map(p); }
inline Q_GUI_EXPORT QPointF operator*(const QPointF &p, const QMatrix &m)
{ return m.map(p); }
inline Q_GUI_EXPORT QLineF operator*(const QLineF &l, const QMatrix &m)
{ return m.map(l); }
inline Q_GUI_EXPORT QPolygon operator *(const QPolygon &a, const QMatrix &m)
{ return m.map(a); }
inline Q_GUI_EXPORT QPolygonF operator *(const QPolygonF &a, const QMatrix &m)
{ return m.map(a); }
inline Q_GUI_EXPORT QRegion operator *(const QRegion &r, const QMatrix &m)
{ return m.map(r); }
Q_GUI_EXPORT QPainterPath operator *(const QPainterPath &p, const QMatrix &m);

inline bool QMatrix::isIdentity() const
{
    return _m11 == 1.0 && _m22 == 1.0 && _m12 == 0.0 && _m21 == 0.0 && _dx == 0.0 && _dy == 0.0;
}

/*****************************************************************************
 QMatrix stream functions
 *****************************************************************************/

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QMatrix &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QMatrix &);

#ifndef QT_NO_DEBUG_OUTPUT
Q_GUI_EXPORT QDebug operator<<(QDebug, const QMatrix &);
#endif

#endif // QT_NO_MATRIX

#ifdef QT_COMPAT
#include "qwmatrix.h"
#endif

#endif // QMATRIX_H
