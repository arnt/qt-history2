#include "qaccessiblecompat.h"
#include "q3simplewidgets.h"
#include "q3complexwidgets.h"

#include <qaccessibleplugin.h>
#include <qplugin.h>
#include <qstringlist.h>
#include <q3toolbar.h>

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
    list << "Q3GroupBox";
    list << "Q3ToolBar";
    list << "Q3ToolBarSeparator";
    list << "Q3DockWindowHandle";
    list << "Q3DockWindowResizeHandle";
    list << "Q3MainWindow";
    list << "Q3Header";

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
    } else if (classname == "Q3GroupBox") {
        iface = new Q3AccessibleDisplay(widget, Grouping);
    } else if (classname == "Q3ToolBar") {
        iface = new QAccessibleWidget(widget, ToolBar, static_cast<Q3ToolBar *>(widget)->label());
    } else if (classname == "Q3MainWindow") {
        iface = new QAccessibleWidget(widget, Application);
    } else if (classname == "Q3ToolBarSeparator") {
        iface = new QAccessibleWidget(widget, Separator);
    } else if (classname == "Q3DockWindowHandle") {
        iface = new QAccessibleWidget(widget, Grip);
    } else if (classname == "Q3DockWindowResizeHandle") {
        iface = new QAccessibleWidget(widget, Grip);
    } else if (classname == "Q3Header") {
        iface = new Q3AccessibleHeader(widget);
    }

    return iface;
}

Q_EXPORT_PLUGIN(CompatAccessibleFactory)
