/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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
#include "gltexobj.h"


GLObjectWindow::GLObjectWindow( QWidget* parent, const char* name )
    : QWidget( parent, name )
{

    // Create nice frames to put around the OpenGL widgets
    QFrame* f1 = new QFrame( this, "frame1" );
    f1->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    f1->setLineWidth( 2 );

    // Create an OpenGL widget
    GLTexobj* c = new GLTexobj( f1, "glbox1");

    // Create a menu
    QPopupMenu *file = new QPopupMenu( this );
    file->insertItem( "Toggle Animation", c, SLOT(toggleAnimation()),
		      CTRL+Key_A );
    file->insertSeparator();
    file->insertItem( "Exit",  qApp, SLOT(quit()), CTRL+Key_Q );

    // Create a menu bar
    QMenuBar *m = new QMenuBar( this );
    m->setSeparator( QMenuBar::InWindowsStyle );
    m->insertItem("&File", file );

    // Create the three sliders; one for each rotation axis
    QSlider* x = new QSlider (this);
    x->setObjectName("xsl");
    x->setOrientation(QSlider::Vertical);
    x->setMinimum(0);
    x->setMaximum(360);
    x->setPageStep(60);
    x->setValue(0);
    x->setTickmarks( QSlider::Left );
    connect( x, SIGNAL(valueChanged(int)), c, SLOT(setXRotation(int)) );

    QSlider* y = new QSlider (this);
    y->setObjectName("ysl");
    y->setOrientation(QSlider::Vertical);
    y->setMinimum(0);
    y->setMaximum(360);
    y->setPageStep(60);
    y->setValue(0);
    y->setTickmarks( QSlider::Left );
    connect( y, SIGNAL(valueChanged(int)), c, SLOT(setYRotation(int)) );

    QSlider* z = new QSlider (this);
    z->setObjectName("zsl");
    z->setOrientation(QSlider::Vertical);
    z->setMinimum(0);
    z->setMaximum(360);
    z->setPageStep(60);
    z->setValue(0);
    z->setTickmarks( QSlider::Left );
    connect( z, SIGNAL(valueChanged(int)), c, SLOT(setZRotation(int)) );

    // Now that we have all the widgets, put them into a nice layout

    // Put the sliders on top of each other
    QVBoxLayout* vlayout = new QVBoxLayout( 20, "vlayout");
    vlayout->addWidget( x );
    vlayout->addWidget( y );
    vlayout->addWidget( z );

    // Put the GL widget inside the frame
    QHBoxLayout* flayout1 = new QHBoxLayout( f1, 2, 2, "flayout1");
    flayout1->addWidget( c, 1 );

    // Top level layout, puts the sliders to the left of the frame/GL widget
    QHBoxLayout* hlayout = new QHBoxLayout( this, 20, 20, "hlayout");
    hlayout->setMenuBar( m );
    hlayout->addLayout( vlayout );
    hlayout->addWidget( f1, 1 );

}
