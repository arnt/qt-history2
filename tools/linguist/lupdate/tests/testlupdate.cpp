#include "testlupdate.h"
#include <stdlib.h>
#include <QtGui/QApplication>
#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtCore/QDir>

#ifdef Q_OS_WIN32
#   include <windows.h>
#endif

#include <QtTest/QtTest>




TestLUpdate::TestLUpdate()
{
    childProc = 0;
    m_cmdLupdate = QLatin1String("lupdate");
    m_cmdQMake = QLatin1String("qmake");
}

TestLUpdate::~TestLUpdate()
{
    if (childProc)
	delete childProc;
}

void TestLUpdate::setWorkingDirectory(const QString &workDir)
{
    m_workDir = workDir;
    QDir::setCurrent(m_workDir);
}

void TestLUpdate::addMakeResult( const QString &result )
{
    make_result.append( result );
}

bool TestLUpdate::runChild( bool showOutput, const QString &program, const QStringList &argList)
{
    exit_ok = FALSE;
    if (childProc)
	    delete childProc;

    child_show = showOutput;
    if ( showOutput ) {
	    QString S = argList.join(" ");
	        addMakeResult( program + QLatin1String(" ") + S );
    }

    childProc = new QProcess();
    //childProc = new QProcess(argList, this, argList.join(" ").toLatin1());
    Q_ASSERT(childProc);

    childProc->setWorkingDirectory(m_workDir);
    connect(childProc, SIGNAL(finished(int)), this, SLOT(childReady(int)));

    childProc->start( program, argList );

    bool ok;

    ok = childProc->waitForStarted();

    if (ok) ok = childProc->waitForFinished();

    if (!ok) {
	    addMakeResult( "Error executing '" + program + "'." );
    }

    childReady(ok ? 0 : -1);

    return ok;
}

void TestLUpdate::childReady(int /*exitCode*/)
{
    if (childProc != 0) {
	    childHasData();
        exit_ok = childProc->state() == QProcess::NotRunning && childProc->exitStatus() == 0;
	    childProc->deleteLater();
    }
    childProc = 0;
}

void TestLUpdate::childHasData()
{
    QString stderror(childProc->readAllStandardError());
    stderror = stderror.replace("\t", "    ");
    if (child_show)
        addMakeResult( stderror );

    return;
}


bool TestLUpdate::updateProFile( const QString &pathProFile)
{
    QDir D;
    if (!D.exists(pathProFile)) {
        addMakeResult( "Directory '" + pathProFile + "' doesn't exist" );
        return false;
    }

	QStringList args;
	args.append(pathProFile);

	return runChild( true, m_cmdLupdate, args );
}

bool TestLUpdate::qmake()
{
    QStringList args;
    return runChild(true, m_cmdQMake, args);

}
