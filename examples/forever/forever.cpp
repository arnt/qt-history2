//depot/qt/main/examples/forever/forever.cpp#18 - integrate change 112192 (text)
/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qtimer.h>
#include <qpainter.h>
#include <qapplication.h>
#include <stdlib.h>				// defines rand() function

#include "forever.h"

using namespace Qt;

//
// Forever - a widget that draws rectangles forever.
//

//
// Constructs a Forever widget.
//

Forever::Forever( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    setAttribute(WA_NoBackground);
    setAttribute(WA_PaintOnScreen);
    for (int a=0; a<numColors; a++) {
	colors[a] = QColor( rand()&255,
			    rand()&255,
			    rand()&255 );
    }
    rectangles = 0;

    QTimer * animation = new QTimer( this );
    connect( animation, SIGNAL(timeout()),
	     this, SLOT(repaint()) );
    animation->start( 0 );

    QTimer * counter = new QTimer( this );
    connect( counter, SIGNAL(timeout()),
	     this, SLOT(updateCaption()) );
    counter->start( 1000 );
}


void Forever::updateCaption()
{
    QString s;
    s.sprintf( "Qt Example - Forever - %d rectangles/second", rectangles );
    rectangles = 0;
    setWindowTitle( s );
}


//
// Handles paint events for the Forever widget.
//

void Forever::paintEvent( QPaintEvent * )
{
    int w = width();
    int h = height();
    if(w <= 0 || h <= 0)
	return;
    QPainter paint( this );			// painter object
    paint.setPen( NoPen );			// do not draw outline
    for (int i = 0; i < 100; ++i) {
	paint.setBrush(colors[rand() % numColors]);// set random brush color

	QPoint p1( rand()%w, rand()%h );	// p1 = top left
	QPoint p2( rand()%w, rand()%h );	// p2 = bottom right

	QRect r( p1, p2 );
	paint.drawRect( r );			// draw filled rectangle
	rectangles++;
    }
}

//
// Create and display Forever widget.
//

int main( int argc, char **argv )
{
    QApplication a( argc, argv );		// create application object
    Forever always;				// create widget
    always.resize( 400, 250 );			// start up with size 400x250
    a.setMainWidget( &always );			// set as main widget
    always.setWindowTitle("Qt Example - Forever");
    always.show();				// show widget
    return a.exec();				// run event loop
}
