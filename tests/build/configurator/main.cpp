/*
** Main startup code for the configurator tool
*/

#include <stdio.h>
#include <qapplication.h>
#include <qmainwindow.h>

#include "dialogwidget.h"
#include "menu.h"

QApplication* pApp;

int main( int argc, char** argv )
{
	QMainWindow* pMainWindow;
	CDialogWidget* pDlgWidget;
	CConfiguratorMenu* pMenu;

	pApp = new QApplication( argc, argv );
	QObject::connect( pApp, SIGNAL( lastWindowClosed() ), pApp, SLOT( quit() ) );

	pMainWindow = new QMainWindow();
	pMenu = new CConfiguratorMenu( pMainWindow );

	pDlgWidget = new CDialogWidget( pMainWindow );

	pMainWindow->setCentralWidget( pDlgWidget );
	pMainWindow->show();
	pApp->setMainWidget( pMainWindow );

	pApp->exec();
	
	delete pApp;

	return 0;
}