#include "../tools/designer/plugins/designerinterface.h"

#include <qaction.h>
#include <qapplication.h>
#include <qcleanuphandler.h>
#include <qthread.h>
#include <qdatetime.h>
#include <qdialog.h>
#include <qlcdnumber.h>
#include <qlayout.h>
#include <qlistbox.h>

#include <qwindowsstyle.h>
#include <qmotifstyle.h>
#include <qsgistyle.h>
#include <qcdestyle.h>
#include <qplatinumstyle.h>
#include <qmotifplusstyle.h>
#include <qsignalmapper.h>
#include <qstylefactory.h>

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
    TestInterface( QUnknownInterface *parent, const char *name = 0 );
    ~TestInterface();

    bool initialize( QApplicationInterface* );

    QStringList featureList() const;
    QAction* create( const QString &actionname, QObject* parent = 0 );
    QString group( const QString &actionname );

protected:
    bool event( QEvent* );
    bool eventFilter( QObject*, QEvent* );

private slots:
    void startThread();
    void countWidgets();
    void setStyle( const QString& );

private:
    QGuardedCleanupHandler<QAction> actions;
    QActionGroup *styleGroup;
    QSignalMapper styleMapper;

    TestThread* thread;
    QDialog* dialog;
    QLCDNumber* lcd;
};

TestInterface::TestInterface( QUnknownInterface *parent, const char *name )
: ActionInterface( parent, name ), styleMapper( this ), thread( 0 ), dialog( 0 )
{
    connect( &styleMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( setStyle( const QString& ) ) );
}

TestInterface::~TestInterface()
{
    if ( thread ) {
	thread->stop();
	thread->wait();
    }

    delete dialog;
}

bool TestInterface::initialize( QApplicationInterface* )
{
    if ( !applicationInterface() )
	return FALSE;

    return TRUE;    
}

QStringList TestInterface::featureList() const
{
    QStringList list;
    list << "Start Thread";
    list << "Count Widgets";
    list << "Set Style";
    return list;
}

/* XPM */
static const char * const editmark_xpm[] ={
"12 8 2 1",
". c None",
"c c #ff0000",
".........ccc",
"........ccc.",
".......ccc..",
"ccc...ccc...",
".ccc.ccc....",
"..ccccc.....",
"...ccc......",
"....c.......",
};


QAction* TestInterface::create( const QString& actionname, QObject* parent )
{
    if ( actionname == "Start Thread" ) {
	QAction* a = new QAction( actionname, QIconSet(), "St&art...", 0, parent, actionname );
	connect( a, SIGNAL(activated()), this, SLOT(startThread()) );
	actions.add( a );
	return a;
    } else if ( actionname == "Count Widgets" ) {
	QAction* a = new QAction( actionname, QIconSet(), "&Count", 0, parent, actionname );
	connect( a, SIGNAL(activated()), this, SLOT(countWidgets()) );
	actions.add( a );
	return a;
    } else if ( actionname == "Set Style" ) {
	QActionGroup *ag = new QActionGroup( parent, 0 );
	styleGroup = ag;
	ag->setIconSet( QIconSet( (const char**)editmark_xpm ) );
	ag->setMenuText( actionname );
	ag->setText( actionname );
	ag->setUsesDropDown( TRUE );
	ag->setExclusive( FALSE );

	QStringList list = QStyleFactory::styles();
	QAction *a;
	for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
	    QString style = *it;
	    a = new QAction( style, QIconSet(), "&"+style, 0, ag, 0, ag->isExclusive() );
	    connect( a, SIGNAL( activated() ), &styleMapper, SLOT(map()) );
	    styleMapper.setMapping( a, a->text() );
	}

	actions.add( ag );
	return ag;	
    }

    return 0;
}

void TestInterface::setStyle( const QString& style )
{
    QApplication::setStyle( style );
}

QString TestInterface::group( const QString & )
{
    return "Test";
}

void TestInterface::countWidgets()
{
    if ( !applicationInterface() )
	return;

    DesignerWidgetListInterface *wlIface = (DesignerWidgetListInterface*)applicationInterface()->queryInterface( "*DesignerWidgetListInterface" );
    if ( !wlIface )
	return;

    wlIface->selectAll();

    int c = wlIface->count();
    wlIface->release();

    DesignerStatusBarInterface *sbIface = (DesignerStatusBarInterface*)applicationInterface()->queryInterface( "*DesignerStatusBarInterface" );
    sbIface->requestSetProperty( "message", tr("There are %1 widgets in this form").arg( c ) );
    sbIface->release();
}

void TestInterface::startThread()
{
    if ( !thread )
	thread = new TestThread( this );

    if ( !thread->running() )
	thread->start();
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
	    if ( (int)t->length() != lcd->numDigits() )
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

class TestPlugIn : public QComponentInterface
{
public:
    TestPlugIn();
    ~TestPlugIn();

    QString name() const { return "Test plugin"; }
    QString description() const { return "PlugIn to show what kind of stupid things you can do here"; }
    QString author() const { return "Trolltech"; }
};

TestPlugIn::TestPlugIn()
: QComponentInterface( "Test PlugIn" )
{
    new TestInterface( this );
}

TestPlugIn::~TestPlugIn()
{
}

Q_EXPORT_INTERFACE(TestPlugIn)
