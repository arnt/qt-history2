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
#ifndef QTRANSFORM_H
#define QTRANSFORM_H

#include <QtGui/qpainterpath.h>
#include <QtGui/qpolygon.h>
#include <QtGui/qregion.h>
#include <QtGui/qwindowdefs.h>
#include <QtCore/qline.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QVariant;

class Q_GUI_EXPORT QTransform
{
public:
    enum TransformationCodes {
        TxNone = 0,
        TxTranslate = 1,
        TxScale = 2,
        TxRotShear = 3
    };
public:
    QTransform();
    QTransform(qreal h11, qreal h12, qreal h13,
               qreal h21, qreal h22, qreal h23,
               qreal h31, qreal h32, qreal h33 = 1.0);
    QTransform(qreal h11, qreal h12, qreal h13,
               qreal h21, qreal h22, qreal h23);
    explicit QTransform(const QMatrix &mtx);

    bool isAffine() const;
    bool isIdentity() const;
    bool isInvertible() const;
    bool isScaling() const;
    bool isRotating() const;
    bool isTranslating() const;

    int type() const;
    
    qreal determinant() const;
    qreal det() const;

    qreal m11() const;
    qreal m12() const;
    qreal m13() const;
    qreal m21() const;
    qreal m22() const;
    qreal m23() const;
    qreal m31() const;
    qreal m32() const;
    qreal m33() const;
    qreal dx() const;
    qreal dy() const;

    void setMatrix(qreal m11, qreal m12, qreal m13,
                   qreal m21, qreal m22, qreal m23,
                   qreal m31, qreal m32, qreal m33);
    
    QTransform inverted(bool *invertible = 0) const;
    QTransform adjoint() const;
    QTransform transposed() const;
    
    QTransform &translate(qreal dx, qreal dy);
    QTransform &scale(qreal sx, qreal sy);
    QTransform &shear(qreal sh, qreal sv);
    QTransform &rotate(qreal a);
    QTransform &rotateRadians(qreal a);

    static bool squareToQuad(const QPolygonF &square, QTransform &result);
    static bool quadToSquare(const QPolygonF &quad, QTransform &result);
    static bool quadToQuad(const QPolygonF &one,
                           const QPolygonF &two,
                           QTransform &result);
    
    bool operator==(const QTransform &) const;
    bool operator!=(const QTransform &) const;

    QTransform &operator*=(const QTransform &);
    QTransform operator*(const QTransform &o) const;
    QTransform operator/(qreal o);

    QTransform &operator=(const QTransform &);

    operator QVariant() const;
    
    void reset();
    QPoint       map(const QPoint &p) const;
    QPointF      map(const QPointF &p) const;
    QLine        map(const QLine &l) const;
    QLineF       map(const QLineF &l) const;
    QPolygonF    map(const QPolygonF &a) const;
    QPolygon     map(const QPolygon &a) const;
    QRegion      map(const QRegion &r) const;
    QPainterPath map(const QPainterPath &p) const;
    QPolygon     mapToPolygon(const QRect &r) const;
    QRect mapRect(const QRect &) const;
    QRectF mapRect(const QRectF &) const;
    void map(int x, int y, int *tx, int *ty) const;
    void map(qreal x, qreal y, qreal *tx, qreal *ty) const;

    QMatrix toAffine() const;
    
private:
    class Private;
    Private *d;
    qreal m_11, m_12, m_13;
    qreal m_21, m_22, m_23;
    qreal m_31, m_32, m_33;
    
};
Q_DECLARE_TYPEINFO(QTransform, Q_MOVABLE_TYPE);

/******* inlines *****/
inline bool QTransform::isAffine() const
{
    return qFuzzyCompare(m_13, 0) && qFuzzyCompare(m_23, 0);
}
inline bool QTransform::isIdentity() const
{
#define qFZ qFuzzyCompare
    return qFZ(m_11, 1) && qFZ(m_12, 0) && qFZ(m_13, 0)
        && qFZ(m_21, 0) && qFZ(m_22, 1) && qFZ(m_23, 0)
        && qFZ(m_31, 0) && qFZ(m_32, 0) && qFZ(m_33, 1);
#undef qFZ
}

inline bool QTransform::isInvertible() const
{
    return !qFuzzyCompare(determinant(), 0);
}
#if 1
inline bool QTransform::isScaling() const
{
    return !qFuzzyCompare(m_11, 1.0) ||
           !qFuzzyCompare(m_22, 1.0);
}
inline bool QTransform::isRotating() const
{
    return !qFuzzyCompare(m_12, 0.0) ||
           !qFuzzyCompare(m_21, 0.0);
}
#endif
inline bool QTransform::isTranslating() const
{
    return !qFuzzyCompare(m_31, 0.0) ||
           !qFuzzyCompare(m_32, 0.0);
}

inline qreal QTransform::determinant() const
{ 
    return m_11*(m_33*m_22-m_32*m_23) -
        m_21*(m_33*m_12-m_32*m_13)+m_31*(m_23*m_12-m_22*m_13);   
}
inline qreal QTransform::det() const
{
    return determinant();
}
inline qreal QTransform::m11() const
{
    return m_11;
}
inline qreal QTransform::m12() const
{
    return m_12;
}
inline qreal QTransform::m13() const
{
    return m_13;
}
inline qreal QTransform::m21() const
{
    return m_21;
}
inline qreal QTransform::m22() const
{
    return m_22;
}
inline qreal QTransform::m23() const
{
    return m_23;
}
inline qreal QTransform::m31() const
{
    return m_31;
}
inline qreal QTransform::m32() const
{
    return m_32;
}
inline qreal QTransform::m33() const
{
    return m_33;
}
inline qreal QTransform::dx() const
{
    return m_31;
}
inline qreal QTransform::dy() const
{
    return m_32;
}

/****** stream functions *******************/
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QTransform &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QTransform &);

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const QTransform &);
#endif
/****** end stream functions *******************/

// mathematical semantics
Q_GUI_EXPORT_INLINE QPoint operator*(const QPoint &p, const QTransform &m)
{ return m.map(p); }
Q_GUI_EXPORT_INLINE QPointF operator*(const QPointF &p, const QTransform &m)
{ return m.map(p); }
Q_GUI_EXPORT_INLINE QLineF operator*(const QLineF &l, const QTransform &m)
{ return m.map(l); }
Q_GUI_EXPORT_INLINE QLine operator*(const QLine &l, const QTransform &m)
{ return m.map(l); }
Q_GUI_EXPORT_INLINE QPolygon operator *(const QPolygon &a, const QTransform &m)
{ return m.map(a); }
Q_GUI_EXPORT_INLINE QPolygonF operator *(const QPolygonF &a, const QTransform &m)
{ return m.map(a); }
Q_GUI_EXPORT_INLINE QRegion operator *(const QRegion &r, const QTransform &m)
{ return m.map(r); }
Q_GUI_EXPORT_INLINE QPainterPath operator *(const QPainterPath &p, const QTransform &m)
{ return m.map(p); }

QT_END_HEADER

#endif
