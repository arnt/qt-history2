#include <qstyleinterface.h>
#include <qcompactstyle.h>
#include <qobjectcleanuphandler.h>

class CompactStyle : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    CompactStyle();

    QRESULT queryInterface( const QUuid&, QUnknownInterface ** );
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

CompactStyle::CompactStyle()
: ref( 0 )
{
}

QRESULT CompactStyle::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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

unsigned long CompactStyle::addRef()
{
    return ref++;
}

unsigned long CompactStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QStringList CompactStyle::featureList() const
{
    QStringList list;
    list << "Compact";
    return list;
}

QStyle* CompactStyle::create( const QString& s )
{
    if ( s.lower() == "compact" ) {
        QStyle *style = new QCompactStyle();
	styles.add( style );
	return style;
    }
    return 0;
}

bool CompactStyle::init()
{
    return TRUE;
}

void CompactStyle::cleanup() 
{
    styles.clear();
}

bool CompactStyle::canUnload() const
{
    return styles.isEmpty();
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( CompactStyle )
}
