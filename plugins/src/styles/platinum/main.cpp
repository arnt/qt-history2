#include <qstyleinterface.h>
#include <qplatinumstyle.h>
#include <qobjectcleanuphandler.h>

class PlatinumStyle : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    PlatinumStyle();

    QRESULT queryInterface( const QUuid&, QUnknownInterface ** );
    Q_REFCOUNT;

    QStringList featureList() const;
    QStyle *create( const QString& );

    bool init();
    void cleanup();
    bool canUnload() const;

private:
    QObjectCleanupHandler styles;
};

PlatinumStyle::PlatinumStyle()
{
}

QRESULT PlatinumStyle::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;

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

    (*iface)->addRef();
    return QS_OK;
}

QStringList PlatinumStyle::featureList() const
{
    QStringList list;
    list << "Platinum";
    return list;
}

QStyle* PlatinumStyle::create( const QString& s )
{
    if ( s.lower() == "platinum" ) {
        QStyle *style = new QPlatinumStyle();
	styles.add( style );
	return style;
    }
    return 0;
}

bool PlatinumStyle::init()
{
    return TRUE;
}

void PlatinumStyle::cleanup() 
{
    styles.clear();
}

bool PlatinumStyle::canUnload() const
{
    return styles.isEmpty();
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( PlatinumStyle )
}
