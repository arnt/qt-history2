#include <qstyleinterface.h>
#include <qcomponentinterface.h>
#include <qwindowsstyle.h>

class WindowsStyleInterface : public QStyleInterface
{
public:
    WindowsStyleInterface( QUnknownInterface *parent, const char *name = 0 );

    QStringList featureList() const;
    QStyle *create( const QString& );
};

WindowsStyleInterface::WindowsStyleInterface( QUnknownInterface *parent, const char *name )
: QStyleInterface( parent, name )
{
}

QStringList WindowsStyleInterface::featureList() const
{
    QStringList list;
    list << "Windows";
    return list;
}

QStyle* WindowsStyleInterface::create( const QString& style )
{
    if ( style == "Windows" )
	return new QWindowsStyle();
    return 0;
}

class PlugInInterface : public QComponentInterface
{
public:
    PlugInInterface();
};

PlugInInterface::PlugInInterface()
{
    new WindowsStyleInterface( this );
}

Q_EXPORT_INTERFACE(PlugInInterface)
