/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlabel.cpp#10 $
**
** Implementation of QLabel widget class
**
** Author  : Eirik Eng
** Created : 941215
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#include "qlabel.h"
#include "qpainter.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qlabel.cpp#10 $";
#endif


QLabel::QLabel( QWidget *parent, const char *name )
	: QFrame( parent, name )
{
    initMetaObject();
    align = AlignLeft | AlignVCenter | ExpandTabs;
}

QLabel::QLabel( const char *label, QWidget *parent, const char *name )
	: QFrame( parent, name ), str(label)
{
    initMetaObject();
    align = AlignLeft | AlignVCenter | ExpandTabs;
}


void QLabel::setLabel( const char *s )
{
    if ( str == s )				// no change
	return;
    str = s;
    updateLabel();
}

void QLabel::setLabel( long l )
{
    QString tmp;
    tmp.sprintf( "%ld", l );
    if ( tmp != str ) {
	str = tmp;
	updateLabel();
    }
}

void QLabel::setLabel( double d )
{
    QString tmp;
    tmp.sprintf( "%g", d );
    if ( tmp != str ) {
	str = tmp;
	updateLabel();
    }
}


void QLabel::setAlignment( int alignment )
{
    align = alignment;
    updateLabel();
}


void QLabel::updateLabel()			// update label, not frame
{
    QPainter paint;
    paint.begin( this );
    paint.eraseRect( contentsRect() );
    drawContents( &paint );
    paint.end();
}


void QLabel::drawContents( QPainter *p )
{
    p->drawText( contentsRect(), align, str );
}
