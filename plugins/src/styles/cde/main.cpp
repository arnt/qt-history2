#include <qstyleinterface.h>
#include <qcdestyle.h>
#include <qobjectcleanuphandler.h>

class CDEStyle : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    CDEStyle();

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

CDEStyle::CDEStyle()
: ref( 0 )
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

unsigned long CDEStyle::addRef()
{
    return ref++;
}

unsigned long CDEStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
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

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( CDEStyle )
}
