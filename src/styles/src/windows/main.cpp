#include <qstyleinterface.h>
#include <qwindowsstyle.h>
#include <qobjectcleanuphandler.h>

class WindowsStyle : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    WindowsStyle();

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

WindowsStyle::WindowsStyle()
: ref( 0 )
{
}

QRESULT WindowsStyle::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)(QStyleFactoryInterface*)this;
    else if ( uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_QStyleFactory )
	*iface = (QStyleFactoryInterface*)this;
    else if ( uuid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;
    else
	*iface = 0;

    if ( *iface )
	(*iface)->addRef();
    return;
}

unsigned long WindowsStyle::addRef()
{
    return ref++;
}

unsigned long WindowsStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
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

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( WindowsStyle )
}
