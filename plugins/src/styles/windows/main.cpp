#include <qstyleplugin.h>
#include <qwindowsstyle.h>

class WindowsStyle : public QStylePlugin
{
public:
    WindowsStyle();

    QStringList featureList() const;
    QStyle *create( const QString& );
};

WindowsStyle::WindowsStyle()
: QStylePlugin()
{
}

QStringList WindowsStyle::featureList() const
{
    QStringList list;
    list << "Windows";
    return list;
}

QStyle* WindowsStyle::create( const QString& s )
{
    if ( s.lower() == "windows" )
	return new QWindowsStyle();

    return 0;
}

Q_EXPORT_PLUGIN( WindowsStyle )

