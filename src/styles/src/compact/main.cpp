#include <qstyleinterface.h>
#include <qcompactstyle.h>
#include <qcleanuphandler.h>

class CompactStyle : public QStyleInterface, public QLibraryInterface
{
public:
    CompactStyle();

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

CompactStyle::CompactStyle()
: ref( 0 )
{
}

QUnknownInterface *CompactStyle::queryInterface( const QUuid &uuid )
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
    styles.clear();;
}

bool CompactStyle::canUnload() const
{
    return styles.isEmpty();
}

Q_EXPORT_INTERFACE()
{
    QUnknownInterface *iface = (QUnknownInterface*)(QStyleInterface*)new CompactStyle;
    iface->addRef();
    return iface;
}
