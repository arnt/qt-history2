#include <qstyleinterface.h>
#include <qaquastyle.h>
#include <qobjectcleanuphandler.h>

class AquaStyle : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    AquaStyle();

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

AquaStyle::AquaStyle()
: ref( 0 )
{
}

QRESULT AquaStyle::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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

unsigned long AquaStyle::addRef()
{
    return ref++;
}

unsigned long AquaStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QStringList AquaStyle::featureList() const
{
    QStringList list;
    list << "Aqua";
    return list;
}

QStyle* AquaStyle::create( const QString& s )
{
    if ( s.lower() == "aqua" ) {
        QStyle *style = new QAquaStyle();
	styles.add( style );
	return style;
    }
    return 0;
}

bool AquaStyle::init()
{
    return TRUE;
}

void AquaStyle::cleanup()
{
    styles.clear();
}

bool AquaStyle::canUnload() const
{
    return styles.isEmpty();
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( AquaStyle )
}
