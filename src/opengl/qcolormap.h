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
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QCOLORMAP_H
#define QCOLORMAP_H

#ifndef QT_H
#include "qobject.h"
#include "qcolor.h"
#endif // QT_H

class QColormapPrivate;
class QWidget;
class Q_EXPORT QColormap : public QObject
{
public:
    QColormap( QWidget * w );
    QColormap( const QColormap & );
    virtual   ~QColormap();
    
    QColormap &operator=( const QColormap & );
    bool      isValid() const;
    int       size() const;
    
    void   setRgb( int base, int count, const QRgb * colors );
    void   setRgb( int idx, QRgb color );
    QRgb   rgb( int idx ) const;
    void   setColor( int idx, const QColor & color );
    QColor color( int idx ) const;
        
    void   install( QWidget * w );
    
private:
    void detach();
    void create( QWidget * w );
    QColormapPrivate * d;
};

#endif
