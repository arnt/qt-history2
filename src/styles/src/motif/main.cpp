#include <qstyleinterface.h>
#include <qmotifstyle.h>
#include <qcleanuphandler.h>

class MotifStyle : public QStyleInterface, public QLibraryInterface
{
public:
    MotifStyle();

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

MotifStyle::MotifStyle()
: ref( 0 )
{
}

QUnknownInterface *MotifStyle::queryInterface( const QUuid &uuid )
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

unsigned long MotifStyle::addRef()
{
    return ref++;
}

unsigned long MotifStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
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

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( MotifStyle )
}
