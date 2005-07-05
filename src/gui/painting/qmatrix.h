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

#include "QtCore/qline.h"
#include "QtCore/qpoint.h"
#include "QtGui/qpolygon.h"
#include "QtCore/qrect.h"
#include "QtGui/qregion.h"
#include "QtGui/qwindowdefs.h"

class QPainterPath;

class Q_GUI_EXPORT QMatrix // 2D transform matrix
{
public:
    QMatrix();
    QMatrix(qreal m11, qreal m12, qreal m21, qreal m22,
            qreal dx, qreal dy);
    QMatrix(const QMatrix &matrix);

    void setMatrix(qreal m11, qreal m12, qreal m21, qreal m22,
                   qreal dx, qreal dy);

    qreal m11() const { return _m11; }
    qreal m12() const { return _m12; }
    qreal m21() const { return _m21; }
    qreal m22() const { return _m22; }
    qreal dx() const { return _dx; }
    qreal dy() const { return _dy; }

    void map(int x, int y, int *tx, int *ty) const;
    void map(qreal x, qreal y, qreal *tx, qreal *ty) const;
    QRect mapRect(const QRect &) const;
    QRectF mapRect(const QRectF &) const;

    QPoint map(const QPoint &p) const;
    QPointF map(const QPointF&p) const;
    QLine map(const QLine &l) const;
    QLineF map(const QLineF &l) const;
    QPolygonF map(const QPolygonF &a) const;
    QPolygon map(const QPolygon &a) const;
    QRegion map(const QRegion &r) const;
    QPainterPath map(const QPainterPath &p) const;
    QPolygon mapToPolygon(const QRect &r) const;

    void reset();
    inline bool isIdentity() const;

    QMatrix &translate(qreal dx, qreal dy);
    QMatrix &scale(qreal sx, qreal sy);
    QMatrix &shear(qreal sh, qreal sv);
    QMatrix &rotate(qreal a);

    bool isInvertible() const { return (_m11*_m22 - _m12*_m21) != 0; }
    qreal det() const { return _m11*_m22 - _m12*_m21; }

    QMatrix inverted(bool *invertible = 0) const;

    bool operator==(const QMatrix &) const;
    bool operator!=(const QMatrix &) const;

    QMatrix &operator*=(const QMatrix &);
    QMatrix operator*(const QMatrix &o) const;

    QMatrix &operator=(const QMatrix &);

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT QMatrix invert(bool *invertible=0) const { return inverted(invertible); }
    inline QT3_SUPPORT QRect map(const QRect &r) const { return mapRect(r); }
    QT3_SUPPORT QRegion mapToRegion(const QRect &r) const;
#endif

private:
    qreal _m11, _m12;
    qreal _m21, _m22;
    qreal _dx, _dy;
};
Q_DECLARE_TYPEINFO(QMatrix, Q_MOVABLE_TYPE);

// mathematical semantics
inline Q_GUI_EXPORT QPoint operator*(const QPoint &p, const QMatrix &m)
{ return m.map(p); }
inline Q_GUI_EXPORT QPointF operator*(const QPointF &p, const QMatrix &m)
{ return m.map(p); }
inline Q_GUI_EXPORT QLineF operator*(const QLineF &l, const QMatrix &m)
{ return m.map(l); }
inline Q_GUI_EXPORT QLine operator*(const QLine &l, const QMatrix &m)
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

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const QMatrix &);
#endif


#ifdef QT3_SUPPORT
#include "QtGui/qwmatrix.h"
#endif

#endif // QMATRIX_H
