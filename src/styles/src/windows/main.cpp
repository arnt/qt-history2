#include <qstyleinterface.h>
#include <qcomponentinterface.h>
#include <qwindowsstyle.h>

class WindowsStyleInterface : public QStyleInterface
{
public:
    WindowsStyleInterface( QUnknownInterface *parent );

    QStringList featureList() const;
    QStyle *create( const QString& );
};

WindowsStyleInterface::WindowsStyleInterface( QUnknownInterface *parent )
: QStyleInterface( parent )
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

class PlugInInterface : public QUnknownInterface
{
public:
    PlugInInterface();
};

PlugInInterface::PlugInInterface()
: QUnknownInterface()
{
    new WindowsStyleInterface( this );
}

Q_EXPORT_INTERFACE(PlugInInterface)
