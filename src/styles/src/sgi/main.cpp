#include <qstyleinterface.h>
#include <qsgistyle.h>
#include <qobjectcleanuphandler.h>

class SGIStyle : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    SGIStyle();

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

SGIStyle::SGIStyle()
: ref( 0 )
{
}

QRESULT SGIStyle::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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
	*iface = 0;

    if ( *iface )
	(*iface)->addRef();
}

unsigned long SGIStyle::addRef()
{
    return ref++;
}

unsigned long SGIStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
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

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( SGIStyle )
}
