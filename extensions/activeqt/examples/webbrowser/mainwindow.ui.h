/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qaxwidget.h>
#include <qmessagebox.h>
#include <qprogressbar.h>
#include <qstatusbar.h>

void MainWindow::init()
{
    pb = new QProgressBar( statusBar() );
    pb->setPercentageVisible( FALSE );
    pb->hide();
    statusBar()->addWidget( pb, 0, TRUE );

    connect( WebBrowser, SIGNAL(ProgressChange(int,int)), this, SLOT(setProgress(int,int)) );
    connect( WebBrowser, SIGNAL(StatusTextChange(const QString&)), statusBar(), SLOT(message(const QString&)) );

    WebBrowser->dynamicCall( "GoHome()" );
}

void MainWindow::go()
{
    actionStop->setEnabled( TRUE );
    WebBrowser->dynamicCall( "Navigate(const QString&)", addressEdit->text() );
}

void MainWindow::setTitle( const QString &title )
{
    setCaption( "Qt WebBrowser - " + title );
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


void MainWindow::setCommandState( int cmd, bool on )
{
    switch ( cmd ) {
    case 1:
	actionForward->setEnabled( on );
	break;
    case 2:
	actionBack->setEnabled( on );
	break;
    }
}

void MainWindow::navigateBegin()
{
    actionStop->setEnabled( TRUE );
}

void MainWindow::navigateComplete()
{
    actionStop->setEnabled( FALSE );
}

void MainWindow::newWindow()
{
    MainWindow *window = new MainWindow;
    window->show();
    if ( addressEdit->text().isEmpty() )
	return;
    window->addressEdit->setText( addressEdit->text() );
    window->actionStop->setEnabled( TRUE );
    window->go();
}

void MainWindow::aboutSlot()
{
    QMessageBox::about(this, tr("About WebBrowser"),
		tr("This Example has been created using the ActiveQt integration into Qt Designer.\n"
		   "It demonstrates the use of QAxWidget to embed the Internet Explorer ActiveX\n"
		   "control into a Qt application."));
}

void MainWindow::aboutQtSlot()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}
