#include <qstyleinterface.h>
#include <qwindowsxpstyle.h>
#include <qcleanuphandler.h>

class WindowsXPStyle : public QStyleInterface, public QLibraryInterface
{
public:
    WindowsXPStyle();

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

WindowsXPStyle::WindowsXPStyle()
: ref( 0 )
{
}

QUnknownInterface *WindowsXPStyle::queryInterface( const QUuid &uuid )
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

unsigned long WindowsXPStyle::addRef()
{
    return ref++;
}

unsigned long WindowsXPStyle::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QStringList WindowsXPStyle::featureList() const
{
    QStringList list;
    list << "Windows XP";
    return list;
}

QStyle* WindowsXPStyle::create( const QString& s )
{
    if ( s.lower() == "windows xp" ) {
	QStyle *style = new QWindowsXPStyle();
	styles.add( style );
	return style;
    }
    return 0;
}

bool WindowsXPStyle::init()
{
    return TRUE;
}

void WindowsXPStyle::cleanup()
{
    styles.clear();
}

bool WindowsXPStyle::canUnload() const
{
    return styles.isEmpty();
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( WindowsXPStyle )
}
