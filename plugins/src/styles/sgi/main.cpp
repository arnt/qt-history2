#include <qstyleinterface.h>
#include <qsgistyle.h>
#include <qobjectcleanuphandler.h>

class SGIStyle : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    SGIStyle();

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

SGIStyle::SGIStyle()
{
}

QRESULT SGIStyle::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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

QStringList SGIStyle::featureList() const
{
    QStringList list;
    list << "SGI";
    return list;
}

QStyle* SGIStyle::create( const QString& s )
{
    if ( s.lower() == "sgi" ) {
        QStyle *style = new QSGIStyle();
	styles.add( style );
	return style;
    }
    return 0;
}

bool SGIStyle::init()
{
    return TRUE;
}

void SGIStyle::cleanup() 
{
    styles.clear();
}

bool SGIStyle::canUnload() const
{
    return styles.isEmpty();
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( SGIStyle )
}
