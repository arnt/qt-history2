#include <qstyleinterface.h>
#include <qmotifplusstyle.h>
#include <qcleanuphandler.h>

class MotifPlusStyle : public QStyleInterface, public QLibraryInterface
{
public:
    MotifPlusStyle();

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

MotifPlusStyle::MotifPlusStyle()
: ref( 0 )
{
}

QUnknownInterface *MotifPlusStyle::queryInterface( const QUuid &uuid )
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
    QUnknownInterface *iface = (QUnknownInterface*)(QStyleInterface*)new MotifPlusStyle;
    iface->addRef();
    return iface;
}
