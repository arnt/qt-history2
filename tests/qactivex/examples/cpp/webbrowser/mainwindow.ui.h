/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <qprogressbar.h>
#include <qstatusbar.h>

void MainWindow::go()
{
    WebBrowser->dynamicCall( "Navigate", addressEdit->text() );
}

void MainWindow::newWindow()
{
    MainWindow *window = new MainWindow;
    window->show();
    if ( addressEdit->text().isEmpty() )
	return;
    window->addressEdit->setText( addressEdit->text() );
    window->go();
}


void MainWindow::setProgress( int a, int b )
{
    if ( a <= 0 || b <= 0 ) {
	pb->hide();
	return;
    }
    pb->show();
    pb->setTotalSteps( b );
    pb->setProgress( a );
}

void MainWindow::init()
{
    connect( WebBrowser, SIGNAL(StatusTextChange(const QString&)), statusBar(), SLOT(message(const QString&)) );
    
    pb = new QProgressBar( statusBar() );
    pb->setPercentageVisible( FALSE );
    statusBar()->addWidget( pb, 0, TRUE );
    pb->hide();

    connect( WebBrowser, SIGNAL(ProgressChange(int,int)), this, SLOT(setProgress(int,int)) );
    
    WebBrowser->dynamicCall( "GoHome" );
}