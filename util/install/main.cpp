#include <qapplication.h>
#include "setupwizardimpl.h"


int main( int argc, char** argv )
{
	QApplication app( argc, argv );
	SetupWizardImpl w;

	w.show();

	app.setMainWidget( &w );

	int res = app.exec();

	w.stopProcesses();
	return res;
}
