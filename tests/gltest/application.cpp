/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "application.h"
#include <qworkspace.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qmultilineedit.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qaccel.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qwhatsthis.h>
#include <qobjectlist.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qgl.h>

#include "glinfo.h"

ApplicationWindow::ApplicationWindow()
    : QMainWindow( 0, "example application main window", WDestructiveClose )
{
    QPopupMenu * file = new QPopupMenu( this );
    menuBar()->insertItem( "&File", file );

    file->insertItem( "&Info", this, SLOT( info() ), CTRL+Key_I );
    file->insertSeparator();
    file->insertItem( "&Quit", qApp, SLOT( closeAllWindows() ), CTRL+Key_Q );

    menuBar()->insertSeparator();
    QPopupMenu * help = new QPopupMenu( this );
    menuBar()->insertItem( "&Help", help );

    help->insertItem( "&About", this, SLOT(about()), Key_F1);
    help->insertItem( "About &Qt", this, SLOT(aboutQt()));

    QVBox* vb = new QVBox( this );
    vb->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    ws = new QWorkspace( vb );
    ws->setScrollBarsEnabled( TRUE );
    setCentralWidget( vb );
    ControlWindow *cw = new ControlWindow( ws, 0 );
    
    connect( cw, SIGNAL( newCreate( const QGLFormat & )), this, SLOT( newDoc( const QGLFormat & )));
    
    cw->show();
}


ApplicationWindow::~ApplicationWindow()
{
}



MDIWindow* ApplicationWindow::newDoc( const QGLFormat &f )
{
    MDIWindow* w = new MDIWindow( f, ws, 0 );
    w->setCaption("GL Test");
    w->show();
    return w;
}

void ApplicationWindow::info()
{
    GLInfo *info = new GLInfo( this );
    info->exec();
    delete info;
    
}


void ApplicationWindow::about()
{
    QMessageBox::about( this, "Qt Application Example",
			"This example demonstrates simple use of\n "
			"Qt's Multiple Document Interface (MDI).");
}


void ApplicationWindow::aboutQt()
{
    QMessageBox::aboutQt( this, "Qt Application Example" );
}


MDIWindow::MDIWindow( const QGLFormat &f, QWidget* parent, const char* name )
    : QWidget( parent, name )
{
    QString str;
    QVBoxLayout *layout = new QVBoxLayout( this );
    gl = new QGLWidget( f, this );
    str.sprintf("Double Buffer: %d", gl->format().doubleBuffer());
    QLabel *l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Depth Buffer: %d", gl->format().depth());
    l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Rgba: %d", gl->format().rgba());
    l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Alpha Buffer: %d", gl->format().alpha());
    l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Accum Buffer: %d", gl->format().accum());
    l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Stencil Buffer: %d", gl->format().stencil());
    l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Stereo: %d", gl->format().stereo());
    l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Direct rendering: %d", gl->format().directRendering());
    l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Has Overlay: %d", gl->format().hasOverlay());
    l = new QLabel( str, this);
    layout->addWidget( l );
}

MDIWindow::~MDIWindow()
{
}

ControlWindow::ControlWindow( QWidget *parent, const char* name )
    : QWidget( parent, name )
{
    setCaption( "Control" );
    QVBoxLayout *bl = new QVBoxLayout( this );
    db = new QCheckBox( "Double Buffer", this );
    bl->addWidget( db );
    depth = new QCheckBox( "Depth Buffer", this );
    bl->addWidget( depth );
    rgba = new QCheckBox( "RGBA", this );
    bl->addWidget( rgba );
    alpha = new QCheckBox( "Alpha Buffer", this );
    bl->addWidget( alpha );
    accum = new QCheckBox( "Accum Buffer", this );
    bl->addWidget( accum );
    stencil = new QCheckBox( "Stencil Buffer", this );
    bl->addWidget( stencil );
    
    QPushButton *button = new QPushButton( "Create", this );
    bl->addWidget( button );
    connect( button, SIGNAL( clicked() ), this, SLOT( create()));
};
    

void ControlWindow::create()
{
    QGLFormat f;
    f.setDoubleBuffer( db->isChecked() );
    f.setDepth( depth->isChecked() );
    f.setRgba( rgba->isChecked() );
    f.setAlpha( alpha->isChecked() );
    f.setAccum( accum->isChecked() );
    f.setStencil( stencil->isChecked() );
    emit newCreate( f );
}
