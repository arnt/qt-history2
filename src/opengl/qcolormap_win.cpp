/****************************************************************************
** $Id: //depot/qt/main/src/opengl/qcolormap_win.cpp#0 $
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
	widget = 0;
    }
    
    ~QColormapPrivate() {
    }
    
    bool valid;
    int size;
    QWidget * widget;
};


QColormap::QColormap()
{
}

QColormap::QColormap( QWidget * w )
{
}

QColormap::QColormap( const QColormap & map )
{
}

QColormap::~QColormap()
{
}

QColormap & QColormap::operator=( const QColormap & map )
{
    QColormap dummy;
    return dummy;
}

void QColormap::detach()
{
}

void QColormap::setRgb( int idx, QRgb color )
{    
}

void QColormap::setColor( int idx, QColor color )
{    
}

QRgb QColormap::rgb( int idx ) const
{
}

QColor QColormap::color( int idx ) const
{
}

bool QColormap::isValid() const
{
    return FALSE;
}

Qt::HANDLE QColormap::colormap() const
{
    return 0;
}

int QColormap::size() const
{
    return 0;
}
