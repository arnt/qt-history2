#include <qstyleinterface.h>
#include "lightstyle.h"
#include <qcleanuphandler.h>

class LightStyleIface : public QStyleInterface, public QLibraryInterface
{
public:
    LightStyleIface();
    virtual ~LightStyleIface();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

    QStringList featureList() const;
    QStyle *create( const QString& );

    bool init();
    void cleanup();
    bool canUnload() const;

private:
    QGuardedCleanupHandler<QStyle> styles;

    unsigned long ref;
};

LightStyleIface::LightStyleIface()
: ref( 0 )
{
}

LightStyleIface::~LightStyleIface()
{
}

QUnknownInterface *LightStyleIface::queryInterface( const QUuid &uuid )
{
    QUnknownInterface *iface = 0;
    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)(QStyleInterface*)this;
    else if ( uuid == IID_QFeatureListInterface )
	iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_QStyleInterface )
	iface = (QStyleInterface*)this;
    else if ( uuid == IID_QLibraryInterface )
	iface = (QLibraryInterface*)this;

    if ( iface )
	iface->addRef();
    return iface;
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
    list << "LightStyle";
    return list;
}

QStyle* LightStyleIface::create( const QString& s )
{
    if ( s.lower() == "lightstyle" ) {
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
