#include "labelled.h"
#include <qlabelled.h>
#include <qlabel.h>
#include <qapp.h>
#include <qgrid.h>
#include <qvbox.h>
#include <qchkbox.h>
#include <qpushbt.h>
#include <qbuttonrow.h>


main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication::setFont( QFont("Helvetica") );
    QApplication app(argc, argv);

    QVBox b;

    QLabelled m("Labelled widget", &b);
    //m.setAlignment(AlignLeft);
    QGrid g( 2, &m);
    for ( int i = 1; i < 10; i++ ) 
	new QCheckBox( "Pressing me is fun!", &g );


    QButtonRow h( &b );
    QPushButton p1( "OK", &h );
    QObject::connect( &p1, SIGNAL(clicked()), qApp, SLOT(quit()));
    QPushButton pp( "I'll think about it", &h );
    QPushButton p2( "Cancel", &h );
    QObject::connect( &p2, SIGNAL(clicked()), qApp, SLOT(quit()));
    //    b.resize(0,0); //############

    //QObject::connect( &pp, SIGNAL(clicked()), &h, SLOT(dump()));

    b.show();


    //    QApplication::setFont( QFont("Helvetica",32),TRUE );

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
