#include <qstyleinterface.h>
#include <qcleanuphandler.h>

#include "windowsxpstyle.h"

class WindowsXPStyle : public QStyleFactoryInterface, public QLibraryInterface
{
public:
    WindowsXPStyle();

    QRESULT queryInterface( const QUuid&, QUnknownInterface ** );
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

QRESULT WindowsXPStyle::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)(QStyleFactoryInterface*)this;
    else if ( uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_QStyleFactory )
	*iface = (QStyleFactoryInterface*)this;
    else if ( uuid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;

    if ( *iface )
	(*iface)->addRef();
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
