#include <qapplication.h>
#include "qdockwidget.h"
#include "qdockarea.h"
#include <qtoolbar.h>
#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qmainwindow.h>
#include <qhbox.h>
#include <qvbox.h>

class MyMainWindow : public QMainWindow
{
public:
    MyMainWindow() : QMainWindow() {
	QHBox *hbox = new QHBox( this );
	new QDockArea( Qt::Vertical, hbox );
	QVBox *vbox = new QVBox( hbox );
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
    dw->show();

    dw = new QDockWidget( 0 );
    dw->setGeometry( 40, 40, 200, 70 );
    dw->show();

    dw = new QDockWidget( 0 );
    dw->setGeometry( 60, 60, 200, 70 );

    QToolBar *b = new QToolBar( "", 0, dw );
    new QPushButton( "1", b );
    new QPushButton( "2", b );
    new QPushButton( "3", b );
    new QPushButton( "4", b );
    new QPushButton( "5", b );
    dw->setWidget( b );
    dw->setResizeEnabled( FALSE );
    dw->setCloseEnabled( FALSE );
    dw->show();

    return a.exec();
}
