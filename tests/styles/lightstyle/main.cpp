#include <qstyleinterface.h>
#include "lightstyle.h"
#include <qobjectcleanuphandler.h>

class LightStyleIface : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    LightStyleIface();
    virtual ~LightStyleIface();

    QRESULT queryInterface( const QUuid&, QUnknownInterface **);
    Q_REFCOUNT;

    QStringList featureList() const;
    QStyle *create( const QString& );

    bool init();
    void cleanup();
    bool canUnload() const;

private:
    QObjectCleanupHandler styles;
};

LightStyleIface::LightStyleIface()
{
}

LightStyleIface::~LightStyleIface()
{
}

QRESULT LightStyleIface::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)(QStyleFactoryInterface*)this;
    else if ( uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_QStyleFactory )
	*iface = (QStyleFactoryInterface*)this;
    else if ( uuid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;
    else
	return QE_NOINTERFACE;

    if ( *iface )
	(*iface)->addRef();
    return QS_OK;
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

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( LightStyleIface )
}
