/****************************************************************************
** $Id: //depot/qt/main/src/opengl/qcolormap.h#0 $
**
** Definition of QColormap class
**
** Created : 20010326
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
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

#ifndef QCOLORMAP_H
#define QCOLORMAP_H

#ifndef QT_H
#include "qcolor.h"
#endif // QT_H

class QColormapPrivate;
class QWidget;
class Q_EXPORT QColormap {
public:
    QColormap();
    QColormap( QWidget * w );
    QColormap( const QColormap & );
    virtual ~QColormap();
    QColormap        &operator=( const QColormap & );
    bool             isValid() const;
    void             setEntry( int idx, QRgb color );
    void             setEntry( int idx, const QColor & color );
    const Qt::HANDLE colormap() const;
    
private:
    void detach();
    QColormapPrivate * d;
};

#endif
