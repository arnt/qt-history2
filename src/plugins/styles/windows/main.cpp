#include <qstyleplugin.h>
#include <qwindowsstyle.h>

class WindowsStyle : public QStylePlugin
{
public:
    WindowsStyle();

    QStringList keys();
    QStyle *create(const QString&);
};

WindowsStyle::WindowsStyle()
: QStylePlugin()
{
}

QStringList WindowsStyle::keys()
{
    QStringList list;
    list << "Windows";
    return list;
}

QStyle* WindowsStyle::create(const QString& s)
{
    if (s.toLower() == "windows")
        return new QWindowsStyle();

    return 0;
}

Q_EXPORT_PLUGIN(WindowsStyle)

