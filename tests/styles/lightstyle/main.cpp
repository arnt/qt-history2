#include <qstyleinterface.h>
#include "lightstyle.h"
#include <qobjectcleanuphandler.h>

class LightStyleIface : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    LightStyleIface();
    virtual ~LightStyleIface();

    void queryInterface( const QUuid&, QUnknownInterface **);
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QStyle *create( const QString& );

    bool init();
    void cleanup();
    bool canUnload() const;

private:
    QObjectCleanupHandler styles;

    unsigned long ref;
};

LightStyleIface::LightStyleIface()
: ref( 0 )
{
}

LightStyleIface::~LightStyleIface()
{
}

void LightStyleIface::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)(QStyleFactoryInterface*)this;
    else if ( uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_QStyleFactory )
	*iface = (QStyleFactoryInterface*)this;
    else if ( uuid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;

    if ( *iface )
	(*iface)->addRef();
}

unsigned long LightStyleIface::addRef()
{
    return ref++;
}

unsigned long LightStyleIface::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QStringList LightStyleIface::featureList() const
{
    QStringList list;
    list << "Light";
    return list;
}

QStyle* LightStyleIface::create( const QString& s )
{
    if ( s.lower() == "light" ) {
	QStyle *style = new LightStyle();
	styles.add( style );
	return style;
    }
    return 0;
}

bool LightStyleIface::init()
{
    return TRUE;
}

void LightStyleIface::cleanup()
{
    styles.clear();
}

bool LightStyleIface::canUnload() const
{
    return styles.isEmpty();
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( LightStyleIface )
}
