/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbitmap.h#37 $
**
** Definition of QBitmap class
**
** Created : 941020
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QBITMAP_H
#define QBITMAP_H

#ifndef QT_H
#include "qpixmap.h"
#endif // QT_H


class Q_EXPORT QBitmap : public QPixmap
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
    QBitmap( const QString &fileName, const char *format=0 );

    QBitmap &operator=( const QBitmap & );
    QBitmap &operator=( const QPixmap & );
    QBitmap &operator=( const QImage  & );

#if QT_FEATURE_TRANSFORMATIONS
    QBitmap  xForm( const QWMatrix & ) const;
#endif
};


#endif // QBITMAP_H
