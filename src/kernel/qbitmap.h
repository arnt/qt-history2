/****************************************************************************
**
** Definition of QBitmap class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QBITMAP_H
#define QBITMAP_H

#ifndef QT_H
#include "qpixmap.h"
#endif // QT_H


class Q_GUI_EXPORT QBitmap : public QPixmap
{
public:
    QBitmap();
    QBitmap( int w, int h,  bool clear = FALSE,
	     QPixmap::Optimization = QPixmap::DefaultOptim );
    QBitmap( const QSize &, bool clear = FALSE,
	     QPixmap::Optimization = QPixmap::DefaultOptim );
    QBitmap( int w, int h,  const uchar *bits, bool isXbitmap=FALSE );
    QBitmap( const QSize &, const uchar *bits, bool isXbitmap=FALSE );
    QBitmap( const QBitmap & );
#ifndef QT_NO_IMAGEIO
    QBitmap( const QString &fileName, const char *format=0 );
#endif
    QBitmap &operator=( const QBitmap & );
    QBitmap &operator=( const QPixmap & );
    QBitmap &operator=( const QImage  & );

#ifndef QT_NO_PIXMAP_TRANSFORMATION
    QBitmap  xForm( const QWMatrix & ) const;
#endif
};


#endif // QBITMAP_H
