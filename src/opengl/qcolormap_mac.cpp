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
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

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

void QColormap::setEntry( int idx, QRgb color )
{    
}

void QColormap::setEntry( int idx, const QColor & color )
{
}

bool QColormap::isValid() const
{
    return FALSE;
}

const Qt::HANDLE QColormap::colormap() const
{
    return 0;
}
