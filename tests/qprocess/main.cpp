#include <iostream.h>
#include <stdlib.h>

#include <qapplication.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include "some.h"
#include "guicat.h"

int main( int argc, char **argv )
{
    if ( argc > 1 ) {
	if ( QString( "-cat" ) == argv[1] ) {
	    char *envString;
	    envString = getenv( "QTDIR" );
	    if ( envString )
		cout << envString << endl;
	    envString = getenv( "SNAFU" );
	    if ( envString )
		cout << envString << endl;

	    char ch;
	    while( cin >> ch )
	    {
		cout << ch;
		cout.flush();
		cerr << (char)(ch +1);
		cerr.flush();
	    }
	    return ch;
	} else if ( QString( "-guicat" ) == argv[1] ) {
	    QApplication a( argc, argv );
	    GuiCat gc;
	    a.setMainWidget( &gc );
	    gc.show();
	    return a.exec();
	} else if ( QString( "-much" ) == argv[1] ) {
	    {
		const char mod = 59;
		for ( int i=0; i<100000; i++ ) {
		    char j = i % mod;
		    if ( j == mod-1 ) {
			cout << endl;
		    } else {
			cout << (char)( 'A' + j );
		    }
		}
	    }
	} else if ( QString( "-readline" ) == argv[1] ) {
	    // ### I should make a real test for the QProcess::readLine...()
	    QApplication a( argc, argv );

	    QProcess proc;
	    proc.addArgument( "p4" );
	    proc.addArgument( "help" );
	    proc.addArgument( "commands" );
	    proc.start();
	    while ( TRUE ) {
		while ( proc.canReadLineStdout() ) {
		    qDebug( proc.readLineStdout() );
		}
	    }
	}
    } else {
	QApplication a( argc, argv );
	SomeFactory factory;
	QVBox vb;

	QPushButton *newProcess;
	// start process
	newProcess = new QPushButton( "Start Process (cat)", &vb );
	QObject::connect( newProcess, SIGNAL(clicked()),
		&factory, SLOT(startProcess0()) );
	newProcess = new QPushButton( "Start Process (guicat)", &vb );
	QObject::connect( newProcess, SIGNAL(clicked()),
		&factory, SLOT(startProcess1()) );
	newProcess = new QPushButton( "Start Process (p4)", &vb );
	QObject::connect( newProcess, SIGNAL(clicked()),
		&factory, SLOT(startProcess2()) );
	newProcess = new QPushButton( "Start Process (much)", &vb );
	QObject::connect( newProcess, SIGNAL(clicked()),
		&factory, SLOT(startProcess3()) );
	// launch process
	newProcess = new QPushButton( "Launch Process (cat)", &vb );
	QObject::connect( newProcess, SIGNAL(clicked()),
		&factory, SLOT(launchProcess0()) );
	newProcess = new QPushButton( "Launch Process (guicat)", &vb );
	QObject::connect( newProcess, SIGNAL(clicked()),
		&factory, SLOT(launchProcess1()) );
	newProcess = new QPushButton( "Launch Process (p4)", &vb );
	QObject::connect( newProcess, SIGNAL(clicked()),
		&factory, SLOT(launchProcess2()) );

	QPushButton *quit = new QPushButton ( "Quit", &vb );
	QObject::connect( quit, SIGNAL(clicked()),
		&factory, SLOT(quit()) );
	QObject::connect( &factory, SIGNAL(quitted()),
		&a, SLOT(quit()) );

	QCheckBox *cb;
	cb = new QCheckBox( "Use own environment", &vb );
	QObject::connect( cb, SIGNAL(toggled(bool)),
		&factory, SLOT(useOwnEnvironment(bool)) );
	cb->setChecked( FALSE );
	cb = new QCheckBox( "Connect Stdout", &vb );
	QObject::connect( cb, SIGNAL(toggled(bool)),
		&factory, SLOT(connectStdout(bool)) );
	cb->setChecked( TRUE );
	cb = new QCheckBox( "Connect Stderr", &vb );
	QObject::connect( cb, SIGNAL(toggled(bool)),
		&factory, SLOT(connectStderr(bool)) );
	cb->setChecked( TRUE );
	cb = new QCheckBox( "Communication Stdin", &vb );
	QObject::connect( cb, SIGNAL(toggled(bool)),
		&factory, SLOT(communicationStdin(bool)) );
	cb->setChecked( TRUE );
	cb = new QCheckBox( "Communication Stdout", &vb );
	QObject::connect( cb, SIGNAL(toggled(bool)),
		&factory, SLOT(communicationStdout(bool)) );
	cb->setChecked( TRUE );
	cb = new QCheckBox( "Communication Stderr", &vb );
	QObject::connect( cb, SIGNAL(toggled(bool)),
		&factory, SLOT(communicationStderr(bool)) );
	cb->setChecked( TRUE );
	cb = new QCheckBox( "Communication DupStderr", &vb );
	QObject::connect( cb, SIGNAL(toggled(bool)),
		&factory, SLOT(communicationDupStderr(bool)) );
	cb->setChecked( FALSE );
	cb = new QCheckBox( "Exit Notify", &vb );
	QObject::connect( cb, SIGNAL(toggled(bool)),
		&factory, SLOT(connectExit(bool)) );
	cb->setChecked( TRUE );

	a.setMainWidget( &vb );
	vb.show();
	return a.exec();
    }
    return 0;
}
