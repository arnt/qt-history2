#include <qstyleinterface.h>
#include <qmotifplusstyle.h>
#include <qcleanuphandler.h>

class MotifPlusStyle : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    MotifPlusStyle();

    QRESULT queryInterface( const QUuid&, QUnknownInterface ** );
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

MotifPlusStyle::MotifPlusStyle()
: ref( 0 )
{
}

QRESULT MotifPlusStyle::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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

unsigned long MotifPlusStyle::addRef()
{
    return ref++;
}

unsigned long MotifPlusStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QStringList MotifPlusStyle::featureList() const
{
    QStringList list;
    list << "MotifPlus";
    return list;
}

QStyle* MotifPlusStyle::create( const QString& s )
{
    if ( s.lower() == "motifplus" ) {
	QStyle *style = new QMotifPlusStyle();
	styles.add( style );
	return style;
    }
    return 0;
}

bool MotifPlusStyle::init()
{
    return TRUE;
}

void MotifPlusStyle::cleanup() 
{
    styles.clear();
}

bool MotifPlusStyle::canUnload() const
{
    return styles.isEmpty();
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( MotifPlusStyle )
}
