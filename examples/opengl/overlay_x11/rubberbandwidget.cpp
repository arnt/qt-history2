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

#include "rubberbandwidget.h"
#include <qpainter.h>


RubberbandWidget::RubberbandWidget( QColor transparentColor, QWidget *parent, 
				    const char *name, WFlags f )
    : QWidget( parent, name, f )
{
    setBackgroundColor( transparentColor );
    on = FALSE;
}


void RubberbandWidget::mousePressEvent( QMouseEvent* e )
{
    p1 = e->pos();
    p2 = p1;
    p3 = p1;
    on = TRUE;
    setMouseTracking( TRUE );
}


void RubberbandWidget::mouseMoveEvent( QMouseEvent* e )
{
    if ( on ) {
	p2 = e->pos();
	QPainter p( this );
	// Erase last drawn rubberband:
	p.setPen( QPen( backgroundColor(), 3 ) );
	p.drawRect( QRect( p1, p3 ) );
	// Draw the new one:
	p.setPen( QPen( white, 3 ) );
	p.drawRect( QRect(p1, p2) );
	p3 = p2;
    }
}

void RubberbandWidget::mouseReleaseEvent( QMouseEvent* )
{
    if ( on ) {
	QPainter p ( this );
	p.eraseRect( rect() );
    }
    on = FALSE;
    setMouseTracking( FALSE );
}
