#include <qapplication.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

#include "some.h"


/*
 * Some
*/
static QLabel *out;
static QLabel *err;

Some::Some( QObject *p, bool cStdout, bool cStderr, bool cExit, int com )
	: QObject( p )
{
    proc = new QProcess( this );
    switch ( com ) {
    case 1:
	proc->addArgument( QDir::current().absFilePath( "some" ) );
	proc->addArgument( "-guicat" );
	hideAfterExit = TRUE;
	break;
#if 0
	// for windows compiled from a dsp file
	proc = new QProcess( this );
	QDir dir = QDir::current();
	dir.cd( "Debug" );
	proc->addArgument( dir.absFilePath( "some" ) );
	proc->addArgument( "-cat" );
#endif
    case 2:
	// other external program
	proc->addArgument( "p4" );
	proc->addArgument( "help" );
	proc->addArgument( "commands" );
	hideAfterExit = FALSE;
	break;
    default:
	proc->addArgument( QDir::current().absFilePath( "some" ) );
	proc->addArgument( "-cat" );
	hideAfterExit = TRUE;
	break;
    }

    // io stuff
    QLineEdit *in = new QLineEdit( &main );
    out = new QLabel( &main );
    err = new QLabel( &main );
    QPushButton *close = new QPushButton( "Close Stdin", &main );

    // hup, kill
    QPushButton *hupButton  = new QPushButton( "Hang up", &main );
    QPushButton *killButton = new QPushButton( "Kill", &main );
    QObject::connect( hupButton, SIGNAL(clicked()),
	    this, SLOT(hup()) );
    QObject::connect( killButton, SIGNAL(clicked()),
	    this, SLOT(kill()) );

    // info stuff
    isRunningInfo  = new QLabel( &info );
    normalExitInfo = new QLabel( &info );
    exitStatusInfo = new QLabel( &info );
    QPushButton *closeInfo = new QPushButton( "Close", &info );
    QPushButton *updateInfoButton = new QPushButton( "Update Info", &info );
    QPushButton *showInfoButton = new QPushButton( "Show Info", &main );
    QObject::connect( closeInfo, SIGNAL(clicked()),
	    &info, SLOT(hide()) );
    QObject::connect( updateInfoButton, SIGNAL(clicked()),
	    this, SLOT(updateInfo()) );
    QObject::connect( showInfoButton, SIGNAL(clicked()),
	    this, SLOT(showInfo()) );

    // slot dataStdin( const QString& )
    QObject::connect( in, SIGNAL(textChanged(const QString&)),
	    proc, SLOT(dataStdin(const QString&)) );
    // slot closeStdin()
    QObject::connect( close, SIGNAL(clicked()),
	    proc, SLOT(closeStdin()) );

    // signal dataStdout( const QString& )
    QCheckBox *cb1 = new QCheckBox( "Stdout", &main );
    QObject::connect( cb1, SIGNAL(toggled(bool)),
	    this, SLOT(connectStdout(bool)) );
    cb1->setChecked( cStdout );
    connectStdout( cStdout );
    // signal dataStderr( const QString& )
    QCheckBox *cb2 = new QCheckBox( "Stderr", &main );
    QObject::connect( cb2, SIGNAL(toggled(bool)),
	    this, SLOT(connectStderr(bool)) );
    cb2->setChecked( cStderr );
    connectStderr( cStderr );
    // signal processExited()
    QCheckBox *cb3 = new QCheckBox( "Exit Notify", &main );
    QObject::connect( cb3, SIGNAL(toggled(bool)),
	    this, SLOT(connectExit(bool)) );
    cb3->setChecked( cExit );
    connectExit( cExit );
    // signal wroteStdin()
    QObject::connect( proc, SIGNAL(wroteStdin()),
		this, SLOT(wroteStdin()) );

    if ( !proc->start() ) {
	qWarning( "Could not start process" );
	return;
    }
    main.show();
}

void Some::kill()
{
    if ( proc->kill() ) {
	qDebug( "kill() Successful!" );
    } else {
	qDebug( "kill() Fail!" );
    }
    showInfo();
}

void Some::hup()
{
    if ( proc->hangUp() ) {
	qDebug( "hangUp() Successful!" );
    } else {
	qDebug( "hangUp() Fail!" );
    }
    showInfo();
}

void Some::updateInfo()
{
    isRunningInfo->setText( ( proc->isRunning() ?
		"isRunning: TRUE" : "isRunning: FALSE" ) );
    normalExitInfo->setText( ( proc->normalExit() ?
		"normalExit: TRUE" : "normalExit: FALSE" ) );
    exitStatusInfo->setText(
	    QString( "exitStatus: %1").arg( proc->exitStatus() ) );
}

void Some::showInfo()
{
    updateInfo();
    info.show();
}

void Some::procExited()
{
    showInfo();
    if ( hideAfterExit )
	main.hide();
}

void Some::wroteStdin()
{
    qDebug( "wroteStdin()" );
}

void Some::connectStdout( bool enable )
{
    if ( enable )
	QObject::connect( proc, SIGNAL(dataStdout(const QString&)),
		out, SLOT(setText(const QString&)) );
    else
	QObject::disconnect( proc, SIGNAL(dataStdout(const QString&)),
		out, SLOT(setText(const QString&)) );
}

void Some::connectStderr( bool enable )
{
    if ( enable )
	QObject::connect( proc, SIGNAL(dataStderr(const QString&)),
		err, SLOT(setText(const QString&)) );
    else
	QObject::disconnect( proc, SIGNAL(dataStderr(const QString&)),
		err, SLOT(setText(const QString&)) );
}

void Some::connectExit( bool enable )
{
    if ( enable )
	QObject::connect( proc, SIGNAL(processExited()),
		this, SLOT(procExited()) );
    else
	QObject::disconnect( proc, SIGNAL(processExited()),
		this, SLOT(procExited()) );
}


/*
 * SomeFactory
*/

void SomeFactory::newProcess0()
{
    new Some( parent, cStdout, cStderr, cExit, 0 );
}

void SomeFactory::newProcess1()
{
    new Some( parent, cStdout, cStderr, cExit, 1 );
}

void SomeFactory::newProcess2()
{
    new Some( parent, cStdout, cStderr, cExit, 2 );
}

void SomeFactory::quit()
{
    delete parent;
    parent = 0;
    emit quitted();
}

void SomeFactory::connectStdout( bool enable )
{
    cStdout = enable;
}

void SomeFactory::connectStderr( bool enable )
{
    cStderr = enable;
}

void SomeFactory::connectExit( bool enable )
{
    cExit = enable;
}
