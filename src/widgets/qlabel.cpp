/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.cpp#2 $
**
** Implementation of QLineEdit class
**
** Author  : Eirik Eng
** Created : 941215
**
** Copyright (c) 1994 by Troll Tech AS.	 All rights reserved.
**
***********************************************************************/

#include "qlabel.h"
#include "qpainter.h"
#include "qfontmet.h"
#include "qpixmap.h"
#include "qkeycode.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qlabel.cpp#2 $";
#endif


QLabel::QLabel( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
}

QLabel::QLabel( const char *text, QWidget *parent, const char *name )
	: QWidget( parent, name )
{
    t = text;
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
    t.sprintf( "%i", l );
    update();
}

void QLabel::setText( double d )
{
    t.sprintf( "%f", d );
    update();
}

char *QLabel::text() const
{
    return t.data();
}


void QLabel::paintEvent( QPaintEvent * )
{
    QPainter p;
    QFontMetrics fm( font() );

    p.begin( this );
    p.drawText( 2, clientHeight() - fm.descent(), t );
    p.end();
}


