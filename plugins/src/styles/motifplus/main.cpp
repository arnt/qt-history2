#include <qstyleinterface.h>
#include <qmotifplusstyle.h>
#include <qobjectcleanuphandler.h>

class MotifPlusStyle : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    MotifPlusStyle();

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

MotifPlusStyle::MotifPlusStyle()
{
}

QRESULT MotifPlusStyle::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( MotifPlusStyle )
}
