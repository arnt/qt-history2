#include <qstyleinterface.h>
#include <qsgistyle.h>
#include <qcleanuphandler.h>

class SGIStyle : public QStyleInterface, public QLibraryInterface
{
public:
    SGIStyle();

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

SGIStyle::SGIStyle()
: ref( 0 )
{
}

QUnknownInterface *SGIStyle::queryInterface( const QUuid &uuid )
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
