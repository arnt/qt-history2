#include <qapplication.h>
#include "qdockwidget.h"
#include "qdockarea.h"
#include <qtoolbar.h>
#include <qpushbutton.h>

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    QDockArea *da1 = new QDockArea( Qt::Horizontal );
    QDockArea *da2 = new QDockArea( Qt::Vertical );

    da1->setGeometry( 100, 100, 800, 200 );
    da1->show();
    da2->setGeometry( 100, 325, 200, 800 );
    da2->show();

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
    
    dw->show();

    
    
    return a.exec();
}
