/****************************************************************************
** $Id: //depot/qt/main/src/opengl/qcolormap_mac.cpp#0 $
**
** Implementation of QColormap class
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

#include "qcolormap.h"
#include "qshared.h"
#include "qarray.h"
#include "qwidget.h"

class QColormapPrivate : public QShared
{
public:
    QColormapPrivate() { 
	valid  = FALSE; 
	size   = 0;
	topLevelWidget = 0;
    }
    
    ~QColormapPrivate() {
    }
    
    bool valid;
    int size;
    QWidget * topLevelWidget;
};


QColormap::QColormap( QWidget * w )
    : QObject( w )
{
}

QColormap::QColormap( const QColormap & map )
    : QObject( map.d->topLevelWidget )
{
}

QColormap::~QColormap()
{
}

void QColormap::create( QWidget * )
{
}

void QColormap::install( QWidget * )
{
}

QColormap & QColormap::operator=( const QColormap & )
{
    return *this;
}

void QColormap::detach()
{
}

void QColormap::setRgb( int, QRgb )
{    
}

void QColormap::setRgb( int, int, const QRgb * )
{    
}

void QColormap::setColor( int, const QColor & )
{    
}

QRgb QColormap::rgb( int ) const
{
    return Qt::black.rgb();
}

QColor QColormap::color( int ) const
{
    return Qt::black;
}

bool QColormap::isValid() const
{
    return FALSE;
}

int QColormap::size() const
{
    return 0;
}
