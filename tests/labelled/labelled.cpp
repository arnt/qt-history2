#include "labelled.h"
#include <qlabelled.h>
#include <qlabel.h>
#include <qapp.h>
#include <qgrid.h>
#include <qvbox.h>
#include <qchkbox.h>
#include <qpushbt.h>

main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    QVBox b;

    QLabelled m("Labelled widget", &b);
    //m.setAlignment(AlignLeft);
    QGrid g( 2, &m);
    for ( int i = 1; i < 10; i++ ) 
	new QCheckBox( "Press Me", &g );


    QHBox h( &b );
    new QPushButton( "OK", &h );
    new QPushButton( "Cancel", &h );

    b.resize(0,0); //############

    b.show();


    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
