#include <qapplication.h>
#include <qeditorfactory.h>
#include <qsqlform.h>
#include <qsqltable.h>
#include <qpixmap.h>
#include <qlabel.h>


int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    
    QLabel l( 0 );
    l.setProperty( "text", QVariant( QString("test") ) );
    a.setMainWidget( &l );
    l.show();
    
    return a.exec();
}
