/****************************************************************************
** $Id: $
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** This example program may be used, distributed and modified without 
** limitation.
**
*****************************************************************************/


#include <qprogressbar.h>
#include <qstatusbar.h>

void MainWindow::go()
{
    actionStop->setEnabled( TRUE );
    WebBrowser->dynamicCall( "Navigate(const QString&)", addressEdit->text() );
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
    
    actionStop->setEnabled( TRUE );
    WebBrowser->dynamicCall( "GoHome()" );
}

void MainWindow::setTitle( const QString &title )
{
    setCaption( "Qt WebBrowser - " + title );
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

void MainWindow::navigateComplete()
{
    actionStop->setEnabled( FALSE );
}

void MainWindow::navigateBegin()
{
    actionStop->setEnabled( TRUE );
}