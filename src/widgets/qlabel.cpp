/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.cpp#4 $
**
** Implementation of QLineEdit class
**
** Author  : Eirik Eng
** Created : 941215
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qlabel.h"
#include "qpainter.h"
#include "qfontmet.h"
#include "qpixmap.h"
#include "qkeycode.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qlabel.cpp#4 $";
#endif


QLabel::QLabel( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
}

QLabel::QLabel( const char *text, QWidget *parent, const char *name )
	: QWidget( parent, name ), t(text)
{
}

void QLabel::setText( const char *s )
{
    if ( t == s )				// no change
	return;
    t = s;
    update();
}

void QLabel::setText( long l )
{
    QString tmp;
    tmp.sprintf( "%i", l );
    if ( tmp != t ) {
	t = tmp;
	update();
    }
}

void QLabel::setText( double d )
{
    QString tmp;
    tmp.sprintf( "%g", d );
    if ( tmp != t ) {
	t = tmp;
	update();
    }
}

const char *QLabel::text() const
{
    return t.data();
}


void QLabel::paintEvent( QPaintEvent * )
{
    QPainter p;
    p.begin( this );
    p.drawText( clientRect(), AlignLeft | AlignVCenter | DontClip, t );
    p.end();
}


