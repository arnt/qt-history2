#include <qapplication.h>
#include "qdockwidget.h"
#include "qdockarea.h"
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qmultilineedit.h>
#include <qmainwindow.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qlistview.h>
#include "fileopen.xpm"
#include <qdial.h>

class MyMainWindow : public QMainWindow
{
public:
    MyMainWindow() : QMainWindow() {
	QHBox *hbox = new QHBox( this, "1" );
	new QDockArea( Qt::Vertical, hbox );
	QVBox *vbox = new QVBox( hbox, "2" );
	new QDockArea( Qt::Horizontal, vbox );
	new QMultiLineEdit( vbox );
	new QDockArea( Qt::Horizontal, vbox );
	new QDockArea( Qt::Vertical, hbox );
	setCentralWidget( hbox );

	resize( 500, 500 );
    };

};

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    MyMainWindow mw;
    a.setMainWidget( &mw );
    mw.show();

    QDockWidget *dw = new QDockWidget( 0 );
    dw->setGeometry( 20, 20, 200, 70 );
    dw->setWidget( new QDial( dw ) );
    dw->show();

    dw = new QDockWidget( 0 );
    dw->setWidget( new QListView( dw ) );
    dw->setGeometry( 40, 40, 200, 70 );
    dw->show();

    dw = new QDockWidget( 0 );
    dw->setGeometry( 60, 60, 200, 70 );
    
    QToolBar *b = new QToolBar( "", 0, dw );
    b->setShowHandle( FALSE );
    QToolButton *t = new QToolButton( b );
    t->setPixmap( QPixmap( fileopen ) );
    t = new QToolButton( b );
    t->setPixmap( QPixmap( fileopen ) );
    t = new QToolButton( b );
    t->setPixmap( QPixmap( fileopen ) );
    t = new QToolButton( b );
    t->setPixmap( QPixmap( fileopen ) );
    t = new QToolButton( b );
    t->setPixmap( QPixmap( fileopen ) );
    dw->setWidget( b );
    dw->setResizeEnabled( FALSE );
    dw->setCloseEnabled( FALSE );
    dw->show();
    dw->adjustSize();

    return a.exec();
}
