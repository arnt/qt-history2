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
#include "qtransform.h"

#include "qdatastream.h"
#include "qdebug.h"
#include "qmath_p.h"
#include "qmatrix.h"
#include "qregion.h"
#include "qpainterpath.h"
#include "qvariant.h"

#include <math.h>

#define MAPDOUBLE(x, y, nx, ny) \
{ \
    fx = x; \
    fy = y; \
    nx = m_11*fx + m_21*fy + m_31; \
    ny = m_12*fx + m_22*fy + m_32; \
    if (!isAffine()) { \
        qreal w = m_13*fx + m_23*fy + m_33; \
        w = 1/w; \
        nx *= w; \
        ny *= w; \
    }\
}

#define MAPINT(x, y, nx, ny) \
{ \
    fx = x; \
    fy = y; \
    nx = int(m_11*fx + m_21*fy + m_31); \
    ny = int(m_12*fx + m_22*fy + m_32); \
    if (!isAffine()) { \
        qreal w = m_13*fx + m_23*fy + m_33; \
        w = 1/w; \
        nx = int(nx/w); \
        ny = int(ny/w); \
    }\
}


QTransform::QTransform()
    : m_11(1), m_12(0), m_13(0),
      m_21(0), m_22(1), m_23(0),
      m_31(0), m_32(0), m_33(1)
{

}

QTransform::QTransform(qreal h11, qreal h12, qreal h13,
                       qreal h21, qreal h22, qreal h23,
                       qreal h31, qreal h32, qreal h33)
    : m_11(h11), m_12(h12), m_13(h13),
      m_21(h21), m_22(h22), m_23(h23),
      m_31(h31), m_32(h32), m_33(h33)
{

}

QTransform::QTransform(qreal h11, qreal h12, qreal h21,
                       qreal h22, qreal dx, qreal dy)
    : m_11(h11), m_12(h12), m_13(  0),
      m_21(h21), m_22(h22), m_23(  0),
      m_31( dx), m_32( dy), m_33(  1)
{

}

QTransform::QTransform(const QMatrix &mtx)
    : m_11(mtx.m11()), m_12(mtx.m12()), m_13(0),
      m_21(mtx.m21()), m_22(mtx.m22()), m_23(0),
      m_31(mtx.dx()) , m_32(mtx.dy()) , m_33(1)
{

}

QTransform QTransform::adjoint() const
{
    qreal h11, h12, h13,
        h21, h22, h23,
        h31, h32, h33;
    h11 = m_22*m_33 - m_23*m_32;
    h21 = m_23*m_31 - m_21*m_33;
    h31 = m_21*m_32 - m_22*m_31;
    h12 = m_13*m_32 - m_12*m_33;
    h22 = m_11*m_33 - m_13*m_31;
    h32 = m_12*m_31 - m_11*m_32;
    h13 = m_12*m_23 - m_13*m_22;
    h23 = m_13*m_21 - m_11*m_23;
    h33 = m_11*m_22 - m_12*m_21;
    //### not a huge fan of this simplification but
    //    i'd like to keep m33 as 1.0
    //return QTransform(h11, h12, h13,
    //                  h21, h22, h23,
    //                  h31, h32, h33);
    return QTransform(h11/h33, h12/h33, h13/h33,
                      h21/h33, h22/h33, h23/h33,
                      h31/h33, h32/h33, 1.0);
}

QTransform QTransform::transposed() const
{
    return QTransform(m_11, m_21, m_31,
                      m_12, m_22, m_32,
                      m_13, m_23, m_33);
}

QTransform QTransform::inverted(bool *invertible) const
{
    qreal det = determinant();
    if (qFuzzyCompare(det, qreal(0.0))) {
        if (invertible)
            *invertible = false;
        return QTransform();
    }
    if (invertible)
        *invertible = true;
    QTransform adjA = adjoint();
    QTransform invert = adjA / det;
    invert = QTransform(invert.m11()/invert.m33(), invert.m12()/invert.m33(), invert.m13()/invert.m33(),
                        invert.m21()/invert.m33(), invert.m22()/invert.m33(), invert.m23()/invert.m33(),
                        invert.m31()/invert.m33(), invert.m32()/invert.m33(), 1);
    return invert;
}

QTransform & QTransform::translate(qreal dx, qreal dy)
{
    m_31 += dx*m_11 + dy*m_21;
    m_32 += dy*m_22 + dx*m_12;
    return *this;
}

QTransform & QTransform::scale(qreal sx, qreal sy)
{
    m_11 *= sx;
    m_12 *= sx;
    m_21 *= sy;
    m_22 *= sy;
    return *this;
}

QTransform & QTransform::shear(qreal sh, qreal sv)
{
    qreal tm11 = sv*m_21;
    qreal tm12 = sv*m_22;
    qreal tm21 = sh*m_11;
    qreal tm22 = sh*m_12;
    m_11 += tm11;
    m_12 += tm12;
    m_21 += tm21;
    m_22 += tm22;
    return *this;
}

const qreal deg2rad = qreal(0.017453292519943295769);        // pi/180
QTransform & QTransform::rotate(qreal a)
{
    qreal sina = 0;
    qreal cosa = 0;
    if (a == 90. || a == -270.)
        sina = 1.;
    else if (a == 270. || a == -90.)
        sina = -1.;
    else if (a == 180.)
        cosa = -1.;
    else{
        qreal b = deg2rad*a;                        // convert to radians
        sina = sin(b);                // fast and convenient
        cosa = cos(b);
    }
    qreal tm11 = cosa*m_11 + sina*m_21;
    qreal tm12 = cosa*m_12 + sina*m_22;
    qreal tm21 = -sina*m_11 + cosa*m_21;
    qreal tm22 = -sina*m_12 + cosa*m_22;
    m_11 = tm11; m_12 = tm12;
    m_21 = tm21; m_22 = tm22;
    return *this;
}

QTransform & QTransform::rotateRadians(qreal a)
{
    qreal sina = 0;
    qreal cosa = 0;
    if (a == 90. || a == -270.)
        sina = 1.;
    else if (a == 270. || a == -90.)
        sina = -1.;
    else if (a == 180.)
        cosa = -1.;
    else{
        sina = sin(a);
        cosa = cos(a);
    }
    qreal tm11 = cosa*m_11 + sina*m_21;
    qreal tm12 = cosa*m_12 + sina*m_22;
    qreal tm21 = -sina*m_11 + cosa*m_21;
    qreal tm22 = -sina*m_12 + cosa*m_22;
    m_11 = tm11; m_12 = tm12;
    m_21 = tm21; m_22 = tm22;
    return *this;
}

bool QTransform::operator==(const QTransform &o) const
{
#define qFZ qFuzzyCompare
    return qFZ(m_11, o.m_11) &&  qFZ(m_12, o.m_12) &&  qFZ(m_13, o.m_13)
        && qFZ(m_21, o.m_21) &&  qFZ(m_22, o.m_22) &&  qFZ(m_23, o.m_23)
        && qFZ(m_31, o.m_31) &&  qFZ(m_32, o.m_32) &&  qFZ(m_33, o.m_33);
#undef qFZ
}

bool QTransform::operator!=(const QTransform &o) const
{
    return !operator==(o);
}

QTransform & QTransform::operator*=(const QTransform &o)
{
    qreal m11 = m_11*o.m_11 + m_12*o.m_21 + m_13*o.m_31;
    qreal m12 = m_11*o.m_12 + m_12*o.m_22 + m_13*o.m_32;
    qreal m13 = m_11*o.m_13 + m_12*o.m_23 + m_13*o.m_33;

    qreal m21 = m_21*o.m_11 + m_22*o.m_21 + m_23*o.m_31;
    qreal m22 = m_21*o.m_12 + m_22*o.m_22 + m_23*o.m_32;
    qreal m23 = m_21*o.m_13 + m_22*o.m_23 + m_23*o.m_33;

    qreal m31 = m_31*o.m_11 + m_32*o.m_21 + m_33*o.m_31;
    qreal m32 = m_31*o.m_12 + m_32*o.m_22 + m_33*o.m_32;
    qreal m33 = m_31*o.m_13 + m_32*o.m_23 + m_33*o.m_33;

    m_11 = m11/m33; m_12 = m12/m33; m_13 = m13/m33;
    m_21 = m21/m33; m_22 = m22/m33; m_23 = m23/m33;
    m_31 = m31/m33; m_32 = m32/m33; m_33 = 1.0;

    return *this;
}

QTransform QTransform::operator*(const QTransform &m) const
{
    QTransform result = *this;
    result *= m;
    return result;
}

QTransform & QTransform::operator=(const QTransform &matrix)
{
    m_11 = matrix.m_11;
    m_12 = matrix.m_12;
    m_13 = matrix.m_13;
    m_21 = matrix.m_21;
    m_22 = matrix.m_22;
    m_23 = matrix.m_23;
    m_31 = matrix.m_31;
    m_32 = matrix.m_32;
    m_33 = matrix.m_33;

    return *this;
}

void QTransform::reset()
{
    m_11 = m_22 = m_33 = 1.0;
    m_12 = m_13 = m_21 = m_23 = m_31 = m_32 = 0;
}

#ifndef QT_NO_DATASTREAM
QDataStream & operator<<(QDataStream &s, const QTransform &m)
{
    s << m.m11()
      << m.m12()
      << m.m13()
      << m.m21()
      << m.m22()
      << m.m23()
      << m.m31()
      << m.m32()
      << m.m33();
    return s;
}

QDataStream & operator>>(QDataStream &s, QTransform &t)
{
     double m11, m12, m13,
         m21, m22, m23,
         m31, m32, m33;

     s >> m11;
     s >> m12;
     s >> m13;
     s >> m21;
     s >> m22;
     s >> m23;
     s >> m31;
     s >> m32;
     s >> m33;
     t.setMatrix(m11, m12, m13,
                 m21, m22, m23,
                 m31, m32, m33);
     return s;
}

#endif // QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QTransform &m)
{
    dbg.nospace() << "QTransform("
                  << "11="  << m.m11()
                  << " 12=" << m.m12()
                  << " 13=" << m.m13()
                  << " 21=" << m.m21()
                  << " 22=" << m.m22()
                  << " 23=" << m.m23()
                  << " 31=" << m.m31()
                  << " 32=" << m.m32()
                  << " 33=" << m.m33()
                  << ")";
    return dbg.space();
}
#endif

QPoint QTransform::map(const QPoint &p) const
{
    qreal fx = p.x();
    qreal fy = p.y();

    qreal x = m_11 * fx + m_21 * fy + m_31;
    qreal y = m_12 * fx + m_22 * fy + m_32;

    if (isAffine()) {
        return QPoint(qRound(x), qRound(y));
    } else {
        qreal w = m_13 * fx + m_23 * fy + m_33;
        return QPoint(qRound(x/w), qRound(y/w));
    }
}

QPointF QTransform::map(const QPointF &p) const
{
    qreal fx = p.x();
    qreal fy = p.y();

    qreal x = m_11 * fx + m_21 * fy + m_31;
    qreal y = m_12 * fx + m_22 * fy + m_32;

    if (isAffine()) {
        return QPointF(x, y);
    } else {
        qreal w = m_13 * fx + m_23 * fy + m_33;
        return QPointF(x/w, y/w);
    }
}

QLine QTransform::map(const QLine &l) const
{
    return QLine(map(l.p1()), map(l.p2()));
}

QLineF QTransform::map(const QLineF &l) const
{
    return QLineF(map(l.p1()), map(l.p2()));
}

QPolygonF QTransform::map(const QPolygonF &a) const
{
    int size = a.size();
    int i;
    QPolygonF p(size);
    const QPointF *da = a.constData();
    QPointF *dp = p.data();

    qreal fx, fy;
    for(i = 0; i < size; ++i) {
        MAPDOUBLE(da[i].xp, da[i].yp, dp[i].xp, dp[i].yp);
    }
    return p;
}

QPolygon QTransform::map(const QPolygon &a) const
{
    int size = a.size();
    int i;
    QPolygon p(size);
    const QPoint *da = a.constData();
    QPoint *dp = p.data();

    int fx, fy;
    for(i = 0; i < size; ++i) {
        MAPINT(da[i].xp, da[i].yp, dp[i].xp, dp[i].yp);
    }
    return p;
}

QRegion QTransform::map(const QRegion &r) const
{
    if (isAffine() && !isScaling() && !isRotating()) { // translate or identity
        if (!isTranslating()) // Identity
            return r;
        QRegion copy(r);
        copy.translate(qRound(m_31), qRound(m_32));
        return copy;
    }

    QPainterPath p;
    p.addRegion(r);
    p = map(p);
    return p.toFillPolygon(QTransform()).toPolygon();
}

QPainterPath QTransform::map(const QPainterPath &path) const
{

    if (path.isEmpty())
        return QPainterPath();

    QPainterPath copy = path;

    // Translate or identity
    if (isAffine() && !isScaling() && !isRotating()) {

        // Translate
        if (isTranslating()) {
            copy.detach();
            for (int i=0; i<path.elementCount(); ++i) {
                QPainterPath::Element &e = copy.d_ptr->elements[i];
                e.x += m_31;
                e.y += m_32;
            }
        }

    // Full xform
    } else {
        copy.detach();
        qreal fx, fy;
        for (int i=0; i<path.elementCount(); ++i) {
            QPainterPath::Element &e = copy.d_ptr->elements[i];
            MAPDOUBLE(e.x, e.y, e.x, e.y);
        }
    }

    return copy;
}

QPolygon QTransform::mapToPolygon(const QRect &rect) const
{

    QPolygon a(4);
    qreal x[4], y[4];
    if (isAffine() && !isRotating()) {
        x[0] = m_11*rect.x() + m_31;
        y[0] = m_22*rect.y() + m_32;
        qreal w = m_11*rect.width();
        qreal h = m_22*rect.height();
        if (w < 0) {
            w = -w;
            x[0] -= w;
        }
        if (h < 0) {
            h = -h;
            y[0] -= h;
        }
        x[1] = x[0]+w;
        x[2] = x[1];
        x[3] = x[0];
        y[1] = y[0];
        y[2] = y[0]+h;
        y[3] = y[2];
    } else {
        qreal right = rect.x() + rect.width();
        qreal bottom = rect.y() + rect.height();
        qreal fx, fy;
        MAPDOUBLE(rect.x(), rect.y(), x[0], y[0]);
        MAPDOUBLE(right, rect.y(), x[1], y[1]);
        MAPDOUBLE(right, bottom, x[2], y[2]);
        MAPDOUBLE(rect.x(), bottom, x[3], y[3]);
    }

    // all coordinates are correctly, tranform to a pointarray
    // (rounding to the next integer)
    a.setPoints(4, qRound(x[0]), qRound(y[0]),
                qRound(x[1]), qRound(y[1]),
                qRound(x[2]), qRound(y[2]),
                qRound(x[3]), qRound(y[3]));
    return a;
}

QTransform QTransform::operator/(qreal div)
{
    div = 1/div;
    m_11 *= div;
    m_12 *= div;
    m_13 *= div;
    m_21 *= div;
    m_22 *= div;
    m_23 *= div;
    m_31 *= div;
    m_32 *= div;
    m_33 *= div;
    return *this;
}

bool QTransform::squareToQuad(const QPolygonF &quad, QTransform &trans)
{
    qreal dx0 = quad[0].x();
    qreal dx1 = quad[1].x();
    qreal dx2 = quad[2].x();
    qreal dx3 = quad[3].x();

    qreal dy0 = quad[0].y();
    qreal dy1 = quad[1].y();
    qreal dy2 = quad[2].y();
    qreal dy3 = quad[3].y();

    double ax  = dx0 - dx1 + dx2 - dx3;
    double ay  = dy0 - dy1 + dy2 - dy3;

    if (!ax && !ay) { //afine transform
        trans.setMatrix(dx1 - dx0, dy1 - dy0,  0,
                        dx2 - dx1, dy2 - dy1,  0,
                        dx0,       dy0,  1);
    } else {
        double ax1 = dx1 - dx2;
        double ax2 = dx3 - dx2;
        double ay1 = dy1 - dy2;
        double ay2 = dy3 - dy2;

        /*determinants */
        double gtop    =  ax  * ay2 - ax2 * ay;
        double htop    =  ax1 * ay  - ax  * ay1;
        double bottom  =  ax1 * ay2 - ax2 * ay1;

        double a, b, c, d, e, f, g, h;  /*i is always 1*/

        if (!bottom)
            return false;

        g = gtop/bottom;
        h = htop/bottom;

        a = dx1 - dx0 + g * dx1;
        b = dx3 - dx0 + h * dx3;
        c = dx0;
        d = dy1 - dy0 + g * dy1;
        e = dy3 - dy0 + h * dy3;
        f = dy0;

        trans.setMatrix(a, d, g,
                        b, e, h,
                        c, f, 1.0);
    }

    return true;
}


bool QTransform::quadToSquare(const QPolygonF &quad, QTransform &trans)
{
    if (!squareToQuad(quad, trans))
        return false;

    bool invertible = false;
    trans = trans.inverted(&invertible);

    return invertible;
}


void QTransform::setMatrix(qreal m11, qreal m12, qreal m13,
                           qreal m21, qreal m22, qreal m23,
                           qreal m31, qreal m32, qreal m33)
{
    m_11 = m11; m_12 = m12; m_13 = m13;
    m_21 = m21; m_22 = m22; m_23 = m23;
    m_31 = m31; m_32 = m32; m_33 = m33;
}

bool QTransform::quadToQuad(const QPolygonF &one,
                            const QPolygonF &two,
                            QTransform &trans)
{
    QTransform stq;
    if (!quadToSquare(one, trans))
        return false;
    if (!squareToQuad(two, stq))
        return false;
    trans *= stq;
    //qDebug()<<"Final = "<<trans;
    return true;
}

QRect QTransform::mapRect(const QRect &rect) const
{
    QRect result;
    if (isAffine() && !isRotating()) {
        int x = qRound(m_11*rect.x() + m_31);
        int y = qRound(m_22*rect.y() + m_32);
        int w = qRound(m_11*rect.width());
        int h = qRound(m_22*rect.height());
        if (w < 0) {
            w = -w;
            x -= w;
        }
        if (h < 0) {
            h = -h;
            y -= h;
        }
        result = QRect(x, y, w, h);
    } else {
        // see mapToPolygon for explanations of the algorithm.
        qreal x0, y0;
        qreal x, y, fx, fy;
        MAPDOUBLE(rect.left(), rect.top(), x0, y0);
        qreal xmin = x0;
        qreal ymin = y0;
        qreal xmax = x0;
        qreal ymax = y0;
        MAPDOUBLE(rect.right() + 1, rect.top(), x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        MAPDOUBLE(rect.right() + 1, rect.bottom() + 1, x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        MAPDOUBLE(rect.left(), rect.bottom() + 1, x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        qreal w = xmax - xmin;
        qreal h = ymax - ymin;
        xmin -= (xmin - x0) / w;
        ymin -= (ymin - y0) / h;
        xmax -= (xmax - x0) / w;
        ymax -= (ymax - y0) / h;
        result = QRect(qRound(xmin), qRound(ymin),
                       qRound(xmax)-qRound(xmin)+1, qRound(ymax)-qRound(ymin)+1);
    }
    return result;
}

QRectF QTransform::mapRect(const QRectF &rect) const
{
      QRectF result;
    if (isAffine() && !isRotating()) {
        qreal x = m_11*rect.x() + m_31;
        qreal y = m_22*rect.y() + m_32;
        qreal w = m_11*rect.width();
        qreal h = m_22*rect.height();
        if (w < 0) {
            w = -w;
            x -= w;
        }
        if (h < 0) {
            h = -h;
            y -= h;
        }
        result = QRectF(x, y, w, h);
    } else {
        qreal x0, y0;
        qreal x, y, fx, fy;
        MAPDOUBLE(rect.x(), rect.y(), x0, y0);
        qreal xmin = x0;
        qreal ymin = y0;
        qreal xmax = x0;
        qreal ymax = y0;
        MAPDOUBLE(rect.x() + rect.width(), rect.y(), x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        MAPDOUBLE(rect.x() + rect.width(), rect.y() + rect.height(), x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        MAPDOUBLE(rect.x(), rect.y() + rect.height(), x, y);
        xmin = qMin(xmin, x);
        ymin = qMin(ymin, y);
        xmax = qMax(xmax, x);
        ymax = qMax(ymax, y);
        result = QRectF(xmin, ymin, xmax-xmin, ymax - ymin);
    }
    return result;
}

void QTransform::map(int x, int y, int *tx, int *ty) const
{
    int fx, fy;
    MAPINT(x, y, *tx, *ty);
}

void QTransform::map(qreal x, qreal y, qreal *tx, qreal *ty) const
{
    qreal fx, fy;
    MAPDOUBLE(x, y, *tx, *ty);
}

QMatrix QTransform::toAffine() const
{
    return QMatrix(m_11, m_12,
                   m_21, m_22,
                   m_31, m_32);
}

int QTransform::type() const
{
    if (m_12 != 0 || m_21 != 0)
        return TxRotShear;
    else if (m_11 != 1 || m_22 != 1)
        return TxScale;
    else if (m_31 != 0 || m_32 != 0)
        return TxTranslate;
    else
        return TxNone;
}

/*!

    Returns the transform as a QVariant.
*/
QTransform::operator QVariant() const
{
    return QVariant(QVariant::Transform, this);
}
