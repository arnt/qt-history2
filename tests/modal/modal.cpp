#include <qapplication.h>
#include <qpushbutton.h>
#include <qmainwindow.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include "modal.h"

int main( int argc, char* argv[]  )
{
    QApplication a( argc, argv);
    QMainWindow m;
    QPopupMenu p;
    p.insertItem( "Hallo" );
    m.menuBar()->insertItem("Menu", &p );
    QPushButton l( "Main Application", &m);
    m.setCentralWidget( &l );
    a.setMainWidget( &m );
    m.show();
    ( new Dialog( &l ) )->exec();
    return a.exec();
}

