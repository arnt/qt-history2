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

#ifndef QWMATRIX_H
#define QWMATRIX_H

#ifndef QT_H
#include "qmatrix.h"
#endif // QT_H

#ifndef QT_NO_MATRIX


class Q_GUI_EXPORT QWMatrix : public QMatrix
{
public:
    QWMatrix() : QMatrix() {}
    QWMatrix(double m11, double m12, double m21, double m22,
              double dx, double dy)
        : QMatrix(m11, m12, m21, m22, dx, dy) {}
    QWMatrix(const QWMatrix &matrix) : QMatrix(matrix) {}
    QWMatrix(const QMatrix &matrix) : QMatrix(matrix) {}

    QWMatrix   &translate(double dx, double dy) { QMatrix::translate(dx, dy); return *this; }
    QWMatrix   &scale(double sx, double sy) { QMatrix::scale(sx, sy); return *this; }
    QWMatrix   &shear(double sh, double sv) { QMatrix::shear(sh, sv); return *this; }
    QWMatrix   &rotate(double a) { QMatrix::rotate(a); return *this; }

    QWMatrix   &operator*=(const QWMatrix &o) { QMatrix::operator*=(o); return *this; }

    QWMatrix &operator=(const QWMatrix &o) { QMatrix::operator=(o); return *this; }
    QWMatrix &operator=(const QMatrix &o) { QMatrix::operator=(o); return *this; }
};

#endif // QT_NO_WMATRIX

#endif // QWMATRIX_H
