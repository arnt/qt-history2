#include <actioninterface.h>

#include <qaction.h>
#include <qapplication.h>
#include <qcleanuphandler.h>

#include <qsignalmapper.h>
#include <qstylefactory.h>

class TestComponent : public QObject, public ActionInterface, public QLibraryInterface
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

private slots:
    void setStyle( const QString& );

private:
    QGuardedCleanupHandler<QAction> actions;
    QActionGroup *styleGroup;
    QSignalMapper *styleMapper;

    unsigned long ref;
};

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
    else if ( uuid == IID_ActionInterface )
	iface = (ActionInterface*)this;
    else if ( uuid == IID_QLibraryInterface )
	iface = (QLibraryInterface*)this;

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

#include "main.moc"

Q_EXPORT_INTERFACE()
{
    QUnknownInterface *iface = (QUnknownInterface*)(ActionInterface*)new TestComponent;
    iface->addRef();
    return iface;
}
