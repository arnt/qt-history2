#include <qstyleplugin.h>
#include <qwindowsstyle.h>

class Qt::WindowsStyle : public QStylePlugin
{
public:
    Qt::WindowsStyle();

    QStringList keys() const;
    QStyle *create(const QString&);
};

Qt::WindowsStyle::WindowsStyle()
: QStylePlugin()
{
}

QStringList Qt::WindowsStyle::keys() const
{
    QStringList list;
    list << "Windows";
    return list;
}

QStyle* Qt::WindowsStyle::create(const QString& s)
{
    if (s.toLower() == "windows")
        return new QWindowsStyle();

    return 0;
}

Q_EXPORT_PLUGIN(Qt::WindowsStyle)

