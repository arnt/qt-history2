#include <qstyleinterface.h>
#include <qplatinumstyle.h>
#include <qcleanuphandler.h>

class PlatinumStyle : public QStyleInterface, public QLibraryInterface
{
public:
    PlatinumStyle();

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

PlatinumStyle::PlatinumStyle()
: ref( 0 )
{
}

QUnknownInterface *PlatinumStyle::queryInterface( const QUuid &uuid )
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

unsigned long PlatinumStyle::addRef()
{
    return ref++;
}

unsigned long PlatinumStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
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
    styles.clear();;
}

bool PlatinumStyle::canUnload() const
{
    return styles.isEmpty();
}

Q_EXPORT_INTERFACE()
{
    QUnknownInterface *iface = (QUnknownInterface*)(QStyleInterface*)new PlatinumStyle;
    iface->addRef();
    return iface;
}
