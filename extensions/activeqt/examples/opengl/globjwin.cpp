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

#include <qpushbutton.h>
#include <qslider.h>
#include <qlayout.h>
#include <qframe.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include "globjwin.h"
#include "glbox.h"


GLObjectWindow::GLObjectWindow( QWidget* parent, const char* name )
    : QWidget( parent, name )
{

    // Create a menu
    QMenu *file = new QMenu( this );
    file->addAction( "Exit",  qApp, SLOT(quit())/*, CTRL+Key_Q*/);

    // Create a menu bar
    QMenuBar *m = new QMenuBar( this );
    m->addMenu("&File", file );

    // Create a nice frame to put around the OpenGL widget
    QFrame* f = new QFrame(this);
    f->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    f->setLineWidth( 2 );

    // Create our OpenGL widget
    GLBox* c = new GLBox( f, "glbox");

    // Create the three sliders; one for each rotation axis
    QSlider* x = new QSlider(QSlider::Vertical, this);
    x->setMaximum(360);
    x->setPageStep(60);
    x->setTickmarks( QSlider::Left );
    QObject::connect( x, SIGNAL(valueChanged(int)),c,SLOT(setXRotation(int)) );

    QSlider* y = new QSlider(QSlider::Vertical, this);
    y->setMaximum(360);
    y->setPageStep(60);
    y->setTickmarks( QSlider::Left );
    QObject::connect( y, SIGNAL(valueChanged(int)),c,SLOT(setYRotation(int)) );

    QSlider* z = new QSlider(QSlider::Vertical, this);
    z->setMaximum(360);
    z->setPageStep(60);
    z->setTickmarks( QSlider::Left );
    QObject::connect( z, SIGNAL(valueChanged(int)),c,SLOT(setZRotation(int)) );


    // Now that we have all the widgets, put them into a nice layout

    // Put the sliders on top of each other
    QVBoxLayout* vlayout = new QVBoxLayout( 20, "vlayout");
    vlayout->addWidget( x );
    vlayout->addWidget( y );
    vlayout->addWidget( z );

    // Put the GL widget inside the frame
    QHBoxLayout* flayout = new QHBoxLayout( f, 2, 2, "flayout");
    flayout->addWidget( c, 1 );

    // Top level layout, puts the sliders to the left of the frame/GL widget
    QHBoxLayout* hlayout = new QHBoxLayout( this, 20, 20, "hlayout");
    hlayout->setMenuBar( m );
    hlayout->addLayout( vlayout );
    hlayout->addWidget( f, 1 );
}
