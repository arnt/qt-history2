#include <qstyleinterface.h>
#include <qcdestyle.h>
#include <qobjectcleanuphandler.h>

class CDEStyle : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    CDEStyle();

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

CDEStyle::CDEStyle()
{
}

QRESULT CDEStyle::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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

QStringList CDEStyle::featureList() const
{
    QStringList list;
    list << "CDE";
    return list;
}

QStyle* CDEStyle::create( const QString& s )
{
    if ( s.lower() == "cde" ) {
	QStyle *style = new QCDEStyle();
	styles.add( style );
	return style;
    }
    return 0;
}

bool CDEStyle::init()
{
    return TRUE;
}

void CDEStyle::cleanup() 
{
    styles.clear();
}

bool CDEStyle::canUnload() const
{
    return styles.isEmpty();
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( CDEStyle )
}
