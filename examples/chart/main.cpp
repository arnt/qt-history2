#include <qapplication.h>
#include "chartform.h"
#include "element.h"


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    QString filename;
    if ( app.argc() > 1 ) {
	filename = app.argv()[1];
	if ( !filename.endsWith( ".cht" ) )
	    filename = QString::null;
    }

    ChartForm *cf = new ChartForm( filename );
    app.setMainWidget( cf );
    cf->setCaption( "Chart" );
    cf->show();
    app.connect( &app, SIGNAL(lastWindowClosed()), cf, SLOT(fileQuit()) );

    return app.exec();
}
