#include <qapplication.h>
#include <qvbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

#include "some.h"


//#define LOGGING

#ifdef LOGGING
QFile Some::logFile( "outputLog" );
#endif

/*
 * Some
*/
Some::Some( QObject *p, bool start, bool uOwnEnvironment, int comm, bool cStdout, bool cStderr, bool cExit, int com )
    : QObject( p ), stdoutConnected( FALSE ), stderrConnected( FALSE ), exitConnected( FALSE )
{
    proc = new QProcess( this );
    proc->setCommunication( comm );

    // io stuff
    QLineEdit *in = new QLineEdit( &main );
    out = new QLabel( &main );
    err = new QLabel( &main );
    QPushButton *writeMuch = new QPushButton( "Write much", &main );
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

    // slot writeToStdin( const QString& )
    QObject::connect( in, SIGNAL(textChanged(const QString&)),
	    proc, SLOT(writeToStdin(const QString&)) );
    // slot write much
    QObject::connect( writeMuch, SIGNAL(clicked()),
	    this, SLOT(writeMuch()) );
    // slot closeStdin()
    QObject::connect( close, SIGNAL(clicked()),
	    proc, SLOT(closeStdin()) );

    // signal readyReadStdout()
    QCheckBox *cb1 = new QCheckBox( "Stdout", &main );
    QObject::connect( cb1, SIGNAL(toggled(bool)),
	    this, SLOT(connectStdout(bool)) );
    cb1->setChecked( cStdout );
    connectStdout( cStdout );
    // signal readyReadStderr()
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
    // signal wroteToStdin()
    QObject::connect( proc, SIGNAL(wroteToStdin()),
		this, SLOT(wroteToStdin()) );

    // start cat in the same QProcess class
    QPushButton *startButton = new QPushButton( "Start cat", &main );
    QObject::connect( startButton, SIGNAL(clicked()),
	    this, SLOT(startCat()) );

    main.show();

    protocolReadStdout = 0;
    protocolReadStderr = 0;
    protocol.show();

    if ( uOwnEnvironment ) {
	QStringList env;
	env << "SNAFU=FNORD";
//	env << "LD_LIBRARY_PATH=FNORD";
	procInit( start, com, &env );
    } else {
	procInit( start, com, 0 );
    }
}

void Some::procInit( bool start, int com, QStringList *env )
{
    proc->setArguments( QStringList() );
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
    case 3:
	proc->addArgument( QDir::current().absFilePath( "some" ) );
	proc->addArgument( "-much" );
	hideAfterExit = FALSE;
	break;
    default:
	proc->addArgument( QDir::current().absFilePath( "some" ) );
	proc->addArgument( "-cat" );
	hideAfterExit = TRUE;
	break;
    }
    if ( start ) {
	if ( !proc->start( env ) ) {
	    qWarning( "Could not start process" );
	    return;
	}
    } else {
	hideAfterExit = FALSE;
	if ( !proc->launch( "Foo Bla Fnord Test Hmpfl", env ) ) {
	    qWarning( "Could not start process" );
	    return;
	}
    }
}

void Some::writeMuch()
{
    QString big;
    big.fill( 'c', 16*4096 );
    proc->writeToStdin( big );
}

void Some::kill()
{
    proc->kill();
    showInfo();
}

void Some::hup()
{
    proc->tryTerminate();
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
    protocol.setText( protocol.text() +
	    QString(
		"total read on stdout: %1 bytes\n"
		"total read on stderr: %2 bytes\n"
		).arg( protocolReadStdout).arg( protocolReadStderr) );
    showInfo();
    if ( hideAfterExit )
	main.hide();
}

void Some::startCat()
{
    procInit( TRUE, 0, 0 );
}

void Some::wroteToStdin()
{
    qDebug( "wroteToStdin()" );
}

void Some::connectStdout( bool enable )
{
    if ( enable ) {
	if ( !stdoutConnected )
	    QObject::connect( proc, SIGNAL(readyReadStdout()),
		    this, SLOT(readyReadStdout()) );
	stdoutConnected = TRUE;
    } else {
	QObject::disconnect( proc, SIGNAL(readyReadStdout()),
		this, SLOT(readyReadStdout()) );
	stdoutConnected = FALSE;
    }
}

void Some::readyReadStdout()
{
    QString s = proc->readStdout();
    out->setText( s.mid( 0, 1000 ) );

    logMessage( s );

    protocol.setText( protocol.text() +
	    QString( "read on stdout: %1 bytes\n" ).arg( s.length() ) );
    protocolReadStdout += s.length();
//qDebug( "out: %d", protocolReadStdout );
}

void Some::connectStderr( bool enable )
{
    if ( enable ) {
	if ( !stderrConnected )
	    QObject::connect( proc, SIGNAL(readyReadStderr()),
		    this, SLOT(readyReadStderr()) );
	stderrConnected = TRUE;
    } else {
	QObject::disconnect( proc, SIGNAL(readyReadStderr()),
		this, SLOT(readyReadStderr()) );
	stderrConnected = FALSE;
    }
}

void Some::readyReadStderr()
{
    QString s = proc->readStderr();
    err->setText( s.mid( 0, 1000 ) );

    logMessage( s );

    protocol.setText( protocol.text() +
	    QString( "read on stderr: %1 bytes\n" ).arg( s.length() ) );
    protocolReadStderr += s.length();
//qDebug( "err: %d", protocolReadStderr );
}

void Some::connectExit( bool enable )
{
    if ( enable ) {
	if ( !exitConnected )
	    QObject::connect( proc, SIGNAL(processExited()),
		    this, SLOT(procExited()) );
	exitConnected = TRUE;
    } else {
	QObject::disconnect( proc, SIGNAL(processExited()),
		this, SLOT(procExited()) );
	exitConnected = FALSE;
    }
}


#ifdef LOGGING
void Some::logMessage( const QString& buf )
{
    if ( !logFile.isOpen() ) {
	if ( logFile.open( IO_ReadWrite | IO_Append ) ) {
	    logFile.writeBlock( buf.latin1(), buf.length() );
	    logFile.close();
	} else {
	    qWarning( "error open file" );
	}
    }
#else
void Some::logMessage( const QString& )
{
#endif
}


/*
 * SomeFactory
*/

void SomeFactory::startProcess0()
{
    new Some( parent, TRUE, uOwnEnvironment, communication(), cStdout, cStderr, cExit, 0 );
}

void SomeFactory::startProcess1()
{
    new Some( parent, TRUE, uOwnEnvironment, communication(), cStdout, cStderr, cExit, 1 );
}

void SomeFactory::startProcess2()
{
    new Some( parent, TRUE, uOwnEnvironment, communication(), cStdout, cStderr, cExit, 2 );
}

void SomeFactory::startProcess3()
{
    new Some( parent, TRUE, uOwnEnvironment, communication(), cStdout, cStderr, cExit, 3 );
}

void SomeFactory::launchProcess0()
{
    new Some( parent, FALSE, uOwnEnvironment, communication(), cStdout, cStderr, cExit, 0 );
}

void SomeFactory::launchProcess1()
{
    new Some( parent, FALSE, uOwnEnvironment, communication(), cStdout, cStderr, cExit, 1 );
}

void SomeFactory::launchProcess2()
{
    new Some( parent, FALSE, uOwnEnvironment, communication(), cStdout, cStderr, cExit, 2 );
}

void SomeFactory::quit()
{
    delete parent;
    parent = 0;
    emit quitted();
}

int SomeFactory::communication()
{
    int comm = 0;
    if ( commStdin )
	comm |= QProcess::Stdin;
    if ( commStdout )
	comm |= QProcess::Stdout;
    if ( commStderr )
	comm |= QProcess::Stderr;
    if ( commDupStderr )
	comm |= QProcess::DupStderr;
    return comm;
}

void SomeFactory::useOwnEnvironment( bool enable )
{
    uOwnEnvironment = enable;
}

void SomeFactory::communicationStdout( bool enable )
{
    commStdout = enable;
}

void SomeFactory::communicationStderr( bool enable )
{
    commStderr = enable;
}

void SomeFactory::communicationDupStderr( bool enable )
{
    commDupStderr = enable;
}

void SomeFactory::communicationStdin( bool enable )
{
    commStdin = enable;
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
