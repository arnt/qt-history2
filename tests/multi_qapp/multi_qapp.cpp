#include "qapplication.h"
#include "qpushbutton.h"

int app_main (int argc, char* argv[] )
{
        QApplication myapp( argc, argv );

        QPushButton* mybutton = new QPushButton( "Hello world", 0 );
        mybutton->resize( 80, 30 );

        myapp.setMainWidget( mybutton );
        mybutton->show();
        return myapp.exec();
}

int main( int argc, char* argv[] )
{
        app_main( argc, argv );
        app_main( argc, argv );
        app_main( argc, argv );
        return 0;
}
