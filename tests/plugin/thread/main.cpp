#include <actioninterface.h>

#include <qaction.h>
#include <qapplication.h>
#include <qcleanuphandler.h>

#include <qsignalmapper.h>
#include <qstylefactory.h>

#include <qsettings.h>

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

    QUnknownInterface *queryInterface( const QUuid& );
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

    QUnknownInterface *createInstance( const QUuid &, const QUuid & );

    static QUuid cid;


private slots:
    void setStyle( const QString& );

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

QUnknownInterface *TestComponent::queryInterface( const QUuid &uuid )
{
    QUnknownInterface *iface = 0;
    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)(ActionInterface*)this;
    else if ( uuid == IID_QFeatureListInterface )
	iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_ActionInterface )
	iface = (ActionInterface*)this;
    else if ( uuid == IID_QLibraryInterface )
	iface = (QLibraryInterface*)this;
    else if ( uuid == IID_QComponentServerInterface )
	iface = (QComponentServerInterface*)this;
    else if ( uuid == IID_QComponentFactoryInterface )
	iface = (QComponentFactoryInterface*)this;
    
    if ( iface )
	iface->addRef();
    return iface;
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
    }

    return 0;
}

void TestComponent::setStyle( const QString& style )
{
    QApplication::setStyle( style );
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

QUnknownInterface *TestComponent::createInstance( const QUuid &iid, const QUuid &cid )
{
    if ( cid == TestComponent::cid ) {
	TestComponent *comp = new TestComponent();
	QUnknownInterface *iface = comp->queryInterface( iid );
	if ( iface )
	    return iface;

	delete comp;
    }
    return 0;
}

#include "main.moc"

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( TestComponent )
}
