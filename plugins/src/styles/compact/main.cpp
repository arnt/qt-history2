#include <qstyleinterface.h>
#include <qcompactstyle.h>
#include <qobjectcleanuphandler.h>

class CompactStyle : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    CompactStyle();

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

CompactStyle::CompactStyle()
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

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( CompactStyle )
}
