#include <qapplication.h>
#include "setupwizardimpl.h"


void main( int argc, char** argv )
{
	QApplication* app = new QApplication( argc, argv );
	SetupWizardImpl* w = new SetupWizardImpl;

	w->show();

	app->setMainWidget( w );

	app->exec();
}