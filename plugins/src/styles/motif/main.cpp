#include <qstyleinterface.h>
#include <qmotifstyle.h>
#include <qobjectcleanuphandler.h>

class MotifStyle : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    MotifStyle();

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

MotifStyle::MotifStyle()
: ref( 0 )
{
}

QRESULT MotifStyle::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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

QStringList MotifStyle::featureList() const
{
    QStringList list;
    list << "Motif";
    return list;
}

QStyle* MotifStyle::create( const QString& s )
{
    if ( s.lower() == "motif" ) {
	QStyle *style = new QMotifStyle();
	styles.add( style );
	return style;
    }
    return 0;
}

bool MotifStyle::init()
{ 
    return TRUE;
}

void MotifStyle::cleanup() 
{
    styles.clear();
}

bool MotifStyle::canUnload() const
{
    return styles.isEmpty();
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( MotifStyle )
}
