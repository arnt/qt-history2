#include <qstyleinterface.h>
#include <qaquastyle.h>
#include <qobjectcleanuphandler.h>

class AquaStyle : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    AquaStyle();

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

AquaStyle::AquaStyle()
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

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( AquaStyle )
}
