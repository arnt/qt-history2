#include <actioninterface.h>
#include <qaction.h>
#include <qapplication.h>
#include <qobjectcleanuphandler.h>
#include <qstylefactory.h>
#include <qsignalmapper.h>
#include <qsettings.h>

#include <qcomponentfactory.h>

class TestComponent : public QObject, 
		      public ActionInterface, 
		      public QLibraryInterface, 
		      public QComponentRegistrationInterface,
		      public QComponentInformationInterface,
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
    bool location( const QString &actionname, Location l ) const;

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

private:
    QObjectCleanupHandler actions;
    QActionGroup *styleGroup;
    QString selected;
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
    if ( !!selected ) {
	QSettings settings;
	settings.writeEntry( "/Trolltech/Qt Designer/3.0/AddOns/StyleSwitch/Style", selected );
    }
}

QRESULT TestComponent::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)(ActionInterface*)this;
    else if ( uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_Action )
	*iface = (ActionInterface*)this;
    else if ( uuid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;
    else if ( uuid == IID_QComponentRegistration )
	*iface = (QComponentRegistrationInterface*)this;
    else if ( uuid == IID_QComponentFactory )
	*iface = (QComponentFactoryInterface*)this;
    else
	return QE_NOINTERFACE;
    
    (*iface)->addRef();
    return QS_OK;
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
    QSettings setting;
    QString sel = setting.readEntry( "/Trolltech/Qt Designer/3.0/AddOns/StyleSwitch/Style" );
    if ( !!sel )
	setStyle( sel );
}

bool TestComponent::location( const QString &actionname, Location l ) const
{
    return TRUE;
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
    selected = style;
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
    return QComponentFactory::registerComponent( TestComponent::cid, filepath, "Test Component" );
}

bool TestComponent::unregisterComponents() const
{
    return QComponentFactory::unregisterComponent( TestComponent::cid );
}

QRESULT TestComponent::createInstance( const QUuid &cid, const QUuid &iid, QUnknownInterface **iface, QUnknownInterface * )
{
    if ( cid == TestComponent::cid ) {
	TestComponent *comp = new TestComponent();
	comp->queryInterface( iid, iface );
	if ( iface )
	    return QS_OK;

	delete comp;
    }
    return QE_NOINTERFACE;
}

#include "main.moc"

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( TestComponent )
}
