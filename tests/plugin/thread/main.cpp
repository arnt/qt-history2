#include <actioninterface.h>

#include <qaction.h>
#include <qapplication.h>
#include <qcleanuphandler.h>

#include <qsignalmapper.h>
#include <qstylefactory.h>

#include <qsettings.h>
#include <qthread.h>
#include <qlabel.h>

#include <qt_windows.h>

class TestThread : public QThread
{
public:
    TestThread() : QThread() {}

protected:
    void run();
};

void TestThread::run()
{
    QLabel label( 0 );
    label.setText( "Text" );
    label.show();

    while ( 1 )
    {
	qApp->processOneEvent();
    }
}

class TestComponent : public QObject, 
		      public ActionInterface, 
		      public QLibraryInterface, 
		      public QComponentServerInterface,
		      public QComponentInterface,
		      public QComponentFactoryInterface
{
    Q_OBJECT

public:
    TestComponent();
    ~TestComponent();

    QRESULT queryInterface( const QUuid&, QUnknownInterface ** );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QAction* create( const QString &actionname, QObject* parent = 0 );
    QString group( const QString &actionname ) const;
    void connectTo( QUnknownInterface *ai );

    bool init();
    void cleanup();
    bool canUnload() const;

    bool registerComponents( const QString & ) const;
    bool unregisterComponents() const;

    QString name() const { return "Test Component"; }
    QString description() const { return "Guess what..."; }
    QString version() const { return "beta"; }
    QString author() const { return "vohi@trolltech.com"; }

    QRESULT createInstance( const QUuid &, const QUuid &, QUnknownInterface **, QUnknownInterface *outer );

    static QUuid cid;


private slots:
    void setStyle( const QString& );
    void startThread();

private:
    QGuardedCleanupHandler<QAction> actions;
    QActionGroup *styleGroup;
    QSignalMapper *styleMapper;

    unsigned long ref;
};

QUuid TestComponent::cid( 0xDD19964B, 0xA2C8, 0x42AE, 0xAA, 0xF9, 0x8A, 0xDC, 0x50, 0x9B, 0xCA, 0x03 );

TestComponent::TestComponent()
: styleMapper( 0 ), ref( 0 )
{
}

TestComponent::~TestComponent()
{
}

QRESULT TestComponent::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( uuid == IID_QUnknownInterface )
	*iface = (QUnknownInterface*)(ActionInterface*)this;
    else if ( uuid == IID_QFeatureListInterface )
	*iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_ActionInterface )
	*iface = (ActionInterface*)this;
    else if ( uuid == IID_QLibraryInterface )
	*iface = (QLibraryInterface*)this;
    else if ( uuid == IID_QComponentServerInterface )
	*iface = (QComponentServerInterface*)this;
    else if ( uuid == IID_QComponentFactoryInterface )
	*iface = (QComponentFactoryInterface*)this;
    
    if ( *iface )
	(*iface)->addRef();
}

unsigned long TestComponent::addRef()
{
    return ref++;
}

unsigned long TestComponent::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QStringList TestComponent::featureList() const
{
    QStringList list;
    list << "Set Style";
    list << "Thread widget";
    return list;
}

void TestComponent::connectTo( QUnknownInterface * )
{
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

QAction* TestComponent::create( const QString& actionname, QObject* parent )
{
    if ( actionname == "Set Style" ) {
	QActionGroup *ag = new QActionGroup( parent, 0 );
	styleGroup = ag;
	ag->setIconSet( QIconSet( (const char**)editmark_xpm ) );
	ag->setMenuText( actionname );
	ag->setText( actionname );
	ag->setUsesDropDown( TRUE );
	ag->setExclusive( TRUE );

	QStringList list = QStyleFactory::styles();
	list.sort();

	if ( !styleMapper ) {
	    styleMapper = new QSignalMapper( this );
	    connect( styleMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( setStyle( const QString& ) ) );
	}

	QAction *a = 0;
	for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
	    QString style = *it;
	    a = new QAction( style, QIconSet(), "&"+style, 0, ag, 0, ag->isExclusive() );
	    connect( a, SIGNAL( activated() ), styleMapper, SLOT(map()) );
	    styleMapper->setMapping( a, a->text() );
	}

	actions.add( ag );
	return ag;
    } else if ( actionname == "Thread widget" ) {
	QAction *a = new QAction( "Start Thread", QIconSet(), "Start &Thread", 0, parent );
	connect( a, SIGNAL( activated() ), this, SLOT( startThread() ) );

	actions.add( a );
	return a;
    }

    return 0;
}

void TestComponent::setStyle( const QString& style )
{
    QApplication::setStyle( style );
}

void TestComponent::startThread()
{
    TestThread *thread = new TestThread;
    thread->start();
}

QString TestComponent::group( const QString & ) const
{
    return "Test";
}

bool TestComponent::init()
{
    return TRUE;
}

void TestComponent::cleanup()
{
    actions.clear();
}

bool TestComponent::canUnload() const
{
    return actions.isEmpty();
}

bool TestComponent::registerComponents( const QString &filepath ) const
{
    QString cidStr = TestComponent::cid.toString();
    QSettings settings;
    bool ok;

    settings.insertSearchPath( QSettings::Windows, "/Classes" );
    ok = settings.writeEntry( "/CLSID/" + cidStr + "/Default", "Test Component" );
    ok = ok && settings.writeEntry( "/CLSID/" + cidStr + "/InprocServer32/Default", filepath );

    return ok;
}

bool TestComponent::unregisterComponents() const
{
    QString cidStr = TestComponent::cid.toString();
    QSettings settings;
    bool ok;

    settings.insertSearchPath( QSettings::Windows, "/Classes" );
    ok = settings.removeEntry( "/CLSID/" + cidStr + "/InprocServer32/Default" );
    ok = ok && settings.removeEntry( "/CLSID/" + cidStr + "/Default" );

    return ok;
}

QRESULT TestComponent::createInstance( const QUuid &iid, const QUuid &cid, QUnknownInterface **iface, QUnknownInterface * )
{
    if ( cid == TestComponent::cid ) {
	TestComponent *comp = new TestComponent();
	comp->queryInterface( iid, iface );
	if ( iface )
	    return;

	delete comp;
    }
}

#include "main.moc"

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( TestComponent )
}
