#include "qaccessiblecompat.h"

#include <qaccessibleplugin.h>
#include <qplugin.h>
#include <qstringlist.h>

class CompatAccessibleFactory : public QAccessiblePlugin
{
public:
    CompatAccessibleFactory();

    QStringList keys() const;
    QAccessibleInterface *create(const QString &classname, QObject *object);
};

CompatAccessibleFactory::CompatAccessibleFactory()
{
}

QStringList CompatAccessibleFactory::keys() const
{
    QStringList list;
    list << "Q3TextEdit";
#ifndef QT_NO_ICONVIEW
    list << "QIconView";
#endif
    list << "Q3ListView";
    list << "QWidgetStack";

    return list;
}

QAccessibleInterface *CompatAccessibleFactory::create(const QString &classname, QObject *object)
{
    QAccessibleInterface *iface = 0;
    if (!object || !object->isWidgetType())
        return iface;
    QWidget *widget = static_cast<QWidget*>(object);

    if (classname == "Q3TextEdit") {
        iface = new QAccessibleTextEdit(widget);
#ifndef QT_NO_ICONVIEW
    } else if (classname == "QIconView") {
        iface = new QAccessibleIconView(widget);
#endif
    } else if (classname == "Q3ListView") {
        iface = new QAccessibleListView(widget);
    } else if (classname == "QWidgetStack") {
        iface = new QAccessibleWidgetStack(widget);
    }
    return iface;
}

Q_EXPORT_PLUGIN(CompatAccessibleFactory)
