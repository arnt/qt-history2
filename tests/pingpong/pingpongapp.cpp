#include "pingpongapp.h"

#include <qlayout.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qsqltable.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qframe.h>
#include <qsplitter.h>
#include <qapplication.h>
#include <qfiledialog.h>

PingPongApp::PingPongApp( QWidget * parent, const char * name )
    : QMainWindow( parent, name )
{
    init();
}

void PingPongApp::init()
{
    setCaption( "PingPong" );

    QWidget* w = new QWidget();
    
    
    setCentralWidget( w );

    // Setup menus
    QPopupMenu * menu = new QPopupMenu( this );
    menu->insertSeparator();
    menu->insertItem( "&Quit", qApp, SLOT( quit() ), CTRL+Key_Q );
    menuBar()->insertItem( "&File", menu );

    resize( 700, 400 );
}
