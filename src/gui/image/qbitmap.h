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

#ifndef QBITMAP_H
#define QBITMAP_H

#include "QtGui/qpixmap.h"

class QVariant;

class Q_GUI_EXPORT QBitmap : public QPixmap
{
public:
    QBitmap();
    QBitmap(int w, int h, bool clear = false);
    explicit QBitmap(const QSize &, bool clear = false);
    QBitmap(int w, int h, const uchar *bits, bool isXbitmap=false);
    QBitmap(const QSize &, const uchar *bits, bool isXbitmap=false);
    QBitmap(const QBitmap &);
    QBitmap(const QPixmap &);
    QBitmap(const QImage &);
#ifndef QT_NO_IMAGEIO
    explicit QBitmap(const QString &fileName, const char *format=0);
#endif
    QBitmap &operator=(const QBitmap &);
    QBitmap &operator=(const QPixmap &);
    QBitmap &operator=(const QImage  &);
    operator QVariant() const;

#ifndef QT_NO_PIXMAP_TRANSFORMATION
    QBitmap transformed(const QMatrix &) const;
#ifdef QT3_SUPPORT
    inline QT3_SUPPORT QBitmap xForm(const QMatrix &matrix) const { return transformed(matrix); }
#endif
#endif
};
Q_DECLARE_SHARED(QBitmap);

#endif // QBITMAP_H
