#include <qstyleinterface.h>
#include <qwindowsstyle.h>
#include <qobjectcleanuphandler.h>

class WindowsStyle : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    WindowsStyle();

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

WindowsStyle::WindowsStyle()
{
}

QRESULT WindowsStyle::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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

QStringList WindowsStyle::featureList() const
{
    QStringList list;
    list << "Windows";
    return list;
}

QStyle* WindowsStyle::create( const QString& s )
{
    if ( s.lower() == "windows" ) {
	QStyle *style = new QWindowsStyle();
	styles.add( style );
	return style;
    }
    return 0;
}

bool WindowsStyle::init()
{
    return TRUE;
}

void WindowsStyle::cleanup()
{
    styles.clear();
}

bool WindowsStyle::canUnload() const
{
    return styles.isEmpty();
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( WindowsStyle )
}
