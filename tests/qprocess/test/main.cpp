#include <iostream.h>
#include <qapplication.h>
#include <qvbox.h>
#include <qpushbutton.h>

#include "some.h"

int main( int argc, char **argv )
{
    if ( argc > 1 && QString( "-cat" ) == argv[1] ) {
	char ch;
	do
	{
	    cin >> ch;
	    cout << ch;
	    cout.flush();
	    cerr << (char)(ch +1);
	    cerr.flush();
	} while( !cin.eof() );
	return ch;
    } else {
	QApplication a( argc, argv );
	SomeFactory factory;
	QVBox vb;

	QPushButton *newProcess = new QPushButton( "New Process", &vb );
	QPushButton *quit = new QPushButton ( "Quit", &vb );

	QObject::connect( newProcess, SIGNAL(clicked()),
		&factory, SLOT(newProcess()) );
	QObject::connect( quit, SIGNAL(clicked()),
		&factory, SLOT(quit()) );
	QObject::connect( &factory, SIGNAL(quitted()),
		&a, SLOT(quit()) );

	a.setMainWidget( &vb );
	vb.show();
	return a.exec();
    }
}
