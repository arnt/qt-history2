#include "qaccessiblecompat.h"

#include <qaccessibleplugin.h>

class CompatAccessibleFactory : public QAccessiblePlugin, QAccessible
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
    list << "QTextEdit";
#ifndef QT_NO_ICONVIEW
    list << "QIconView";
#endif
    list << "QListView";
    list << "QWidgetStack";

    return list;
}

QAccessibleInterface *CompatAccessibleFactory::create(const QString &classname, QObject *object)
{
    QAccessibleInterface *iface = 0;
    if (!object || !object->isWidgetType())
        return iface;
    QWidget *widget = static_cast<QWidget*>(object);

    if (classname == "QTextEdit") {
        iface = new QAccessibleTextEdit(widget);
#ifndef QT_NO_ICONVIEW
    } else if (classname == "QIconView") {
        iface = new QAccessibleIconView(widget);
#endif
    } else if (classname == "QListView") {
        iface = new QAccessibleListView(widget);
    } else if (classname == "QWidgetStack") {
        iface = new QAccessibleWidgetStack(widget);
    }
    return iface;
}

Q_EXPORT_PLUGIN(CompatAccessibleFactory)
