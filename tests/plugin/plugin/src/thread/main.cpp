#include "../../../../../tools/designer/plugins/designerinterface.h"

#include <qaction.h>
#include <qapplicationinterface.h>
#include <qapplication.h>
#include <qcleanuphandler.h>
#include <qthread.h>
#include <qdatetime.h>
#include <qdialog.h>
#include <qlcdnumber.h>
#include <qlayout.h>

class TestInterface;

class TestThread : public QThread
{
public:
    TestThread( TestInterface* );
    void stop();

protected:
    void run();

private:
    QMutex mtx;
    bool stopped;
    TestInterface* iface;
};

class TestInterface : public QObject, public ActionInterface
{
    Q_OBJECT

public:
    TestInterface();
    ~TestInterface();

    QString name() { return "Test Interface"; }
    QString description() { return "Mostly harmless stuff"; }
    QString author() { return "Trolltech"; }

    bool connectNotify( QApplication* );
    bool disconnectNotify( QApplication* );

    QStringList featureList();
    QAction* create( const QString &actionname, QObject* parent = 0 );
    QString group( const QString &actionname );

protected:
    bool event( QEvent* );
    bool eventFilter( QObject*, QEvent* );

private slots:
    void startThread();

private:
    QGuardedCleanUpHandler<QAction> actions;
    QGuardedPtr<QApplicationInterface> appInterface;
    TestThread* thread;
    QDialog* dialog;
    QLCDNumber* lcd;
};

TestInterface::TestInterface()
{
    dialog = 0;
    thread = 0;
}

TestInterface::~TestInterface()
{
    delete dialog;
}

bool TestInterface::connectNotify( QApplication* theApp )
{
    if ( !theApp )
	return FALSE;

    appInterface = theApp->requestApplicationInterface();
    if ( !appInterface )
	return FALSE;

    thread = new TestThread( this );
    return TRUE;
}

bool TestInterface::disconnectNotify( QApplication* )
{
    thread->stop();
    thread->wait();
    delete thread;

    return TRUE;
}

QStringList TestInterface::featureList()
{
    QStringList list;
    list << "Start...";
    return list;
}

QAction* TestInterface::create( const QString& actionname, QObject* parent )
{
    if ( actionname == "Start..." ) {
	QAction* a = new QAction( actionname, QIconSet(), "St&art...", 0, parent, actionname );
	connect( a, SIGNAL(activated()), this, SLOT(startThread()) );
	actions.addCleanUp( a );
	return a;
    } 

    return 0;
}

QString TestInterface::group( const QString & )
{
    return "Test";
}

bool TestInterface::event( QEvent* e )
{
    if ( e->type() == QEvent::User ) {
	if ( !dialog ) {
	    dialog = new QDialog( 0, 0, FALSE, WStyle_StaysOnTop );
	    dialog->setCaption( "Watch the time!" );
	    QBoxLayout *box = new QHBoxLayout( dialog );
	    lcd = new QLCDNumber( dialog );
	    box->insertWidget( 0, lcd, 1 );
	    lcd->setSegmentStyle( QLCDNumber::Filled );
	    dialog->installEventFilter( this );
	}
	if ( !dialog->isVisible() && thread->running() ) {
	    dialog->show();
	}
	QString *t = (QString*)((QCustomEvent*)e)->data();
	if ( t ) {
	    if ( t->length() != lcd->numDigits() )
		lcd->setNumDigits( t->length() );
	    lcd->display( *t );
	    delete t;
	}
    }
    return QObject::event( e );
}

bool TestInterface::eventFilter( QObject *o, QEvent *e )
{
    if ( o == dialog && e->type() == QEvent::Close ) {
	thread->stop();
    }
    return QObject::eventFilter( o, e );
}

void TestInterface::startThread()
{
    if ( !thread->running() )
	thread->start();
}

TestThread::TestThread( TestInterface *f )
: QThread()
{
    iface = f;
    stopped = FALSE;
}

void TestThread::stop()
{
    mtx.lock();
    stopped = TRUE;
    mtx.unlock();
}

void TestThread::run()
{
    stopped = FALSE;

    bool s = FALSE;
    while ( !s ) {
	sleep( 1 );

	qApp->lock();
	QString *t = new QString;
	*t = QDate::currentDate().toString() + " | " + QTime::currentTime().toString();
	qApp->unlock();

	mtx.lock();
	s = stopped;
	mtx.unlock();
	if ( !s )
	    QThread::postEvent( iface, new QCustomEvent( QEvent::User, (void*)t ) );
    }
}

#include "main.moc"

Q_EXPORT_INTERFACE(ActionInterface, TestInterface)
