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

#ifndef QT_H
#include "qwindowdefs.h"
#include "qpointarray.h"
#include "qrect.h"
#include "qregion.h"
#endif // QT_H

#ifndef QT_NO_MATRIX


class Q_GUI_EXPORT QMatrix                                        // 2D transform matrix
{
public:
    QMatrix();
    QMatrix(double m11, double m12, double m21, double m22,
              double dx, double dy);
    QMatrix(const QMatrix &matrix);

    void        setMatrix(double m11, double m12, double m21, double m22,
                           double dx,  double dy);

    double        m11() const { return _m11; }
    double        m12() const { return _m12; }
    double        m21() const { return _m21; }
    double        m22() const { return _m22; }
    double        dx()  const { return _dx; }
    double        dy()  const { return _dy; }

    void        map(int x, int y, int *tx, int *ty)              const;
    void        map(double x, double y, double *tx, double *ty) const;
    QRect        mapRect(const QRect &)        const;

    QPoint        map(const QPoint &p)        const { return operator *(p); }
    QRect        map(const QRect &r)        const { return mapRect (r); }
    QPointArray map(const QPointArray &a) const { return operator * (a); }
    QRegion     map(const QRegion &r) const { return operator *(r); }
    QRegion     mapToRegion(const QRect &r) const { return operator *(r); }
    QPointArray        mapToPolygon(const QRect &r)        const;

    void        reset();
    inline bool isIdentity() const;

    QMatrix   &translate(double dx, double dy);
    QMatrix   &scale(double sx, double sy);
    QMatrix   &shear(double sh, double sv);
    QMatrix   &rotate(double a);

    bool isInvertible() const { return (_m11*_m22 - _m12*_m21) != 0; }
    double det() const { return _m11*_m22 - _m12*_m21; }

    QMatrix        invert(bool * = 0) const;

    bool        operator==(const QMatrix &) const;
    bool        operator!=(const QMatrix &) const;
    QMatrix   &operator*=(const QMatrix &);

    /* we use matrix multiplication semantics here */
    QPoint operator * (const QPoint &) const;
    QPointArray operator *  (const QPointArray &a) const;
    QRegion operator*(const QRect &) const;
    QRegion operator*(const QRegion &) const;

    QMatrix &operator=(const QMatrix &);

private:
    double        _m11, _m12;
    double        _m21, _m22;
    double        _dx,  _dy;
};

inline bool QMatrix::isIdentity() const
{
    return _m11 == 1.0 && _m22 == 1.0 && _m12 == 0.0 && _m21 == 0.0
        && _dx == 0.0 && _dy == 0.0;
}

Q_GUI_EXPORT QMatrix operator*(const QMatrix &, const QMatrix &);


/*****************************************************************************
  QMatrix stream functions
 *****************************************************************************/

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QMatrix &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QMatrix &);

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug, const QMatrix &);
#endif

#endif // QT_NO_MATRIX

#endif // QMATRIX_H
