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

Some::Some( QObject *p ) : QObject( p )
{
    proc = new QProcess( this );
#if defined(_OS_UNIX_)
    proc->addArgument( QDir::current().absFilePath( "some" ) );
    proc->addArgument( "-cat" );
#else
//    QDir dir = QDir::current();
//    dir.cd( "Debug" );
//    proc->addArgument( dir.absFilePath( "some" ) );
//    proc->addArgument( "-cat" );
    proc->addArgument( "p4" );
#endif

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

    QCheckBox *cb;
    // signal dataStdout( const QString& )
    cb = new QCheckBox( "Stdout", &main );
    QObject::connect( cb, SIGNAL(toggled(bool)),
	    this, SLOT(connectStdout(bool)) );
    // signal dataStderr( const QString& )
    cb = new QCheckBox( "Stderr", &main );
    QObject::connect( cb, SIGNAL(toggled(bool)),
	    this, SLOT(connectStderr(bool)) );
    // signal processExited()
    cb = new QCheckBox( "Exit Notify", &main );
    QObject::connect( cb, SIGNAL(toggled(bool)),
	    this, SLOT(connectExit(bool)) );

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
//	QMessageBox::information( &main, "kill()", "Successful!" );
    } else {
	qDebug( "kill() Fail!" );
//	QMessageBox::information( &main, "kill()", "Fail!" );
    }
    showInfo();
}

void Some::hup()
{
    if ( proc->hangUp() ) {
	qDebug( "hangUp() Successful!" );
//	QMessageBox::information( &main, "hangUp()", "Successful!" );
    } else {
	qDebug( "hangUp() Fail!" );
//	QMessageBox::information( &main, "hangUp()", "Fail!" );
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
#if defined(_OS_UNIX_)
    main.hide();
#endif
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

void SomeFactory::newProcess()
{
    new Some( parent );
}

void SomeFactory::quit()
{
    delete parent;
    parent = 0;
    emit quitted();
}
