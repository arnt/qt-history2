/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
    list << QLatin1String("Q3TextEdit");
    list << QLatin1String("Q3IconView");
    list << QLatin1String("Q3ListView");
    list << QLatin1String("Q3WidgetStack");
    list << QLatin1String("Q3GroupBox");
    list << QLatin1String("Q3ToolBar");
    list << QLatin1String("Q3ToolBarSeparator");
    list << QLatin1String("Q3DockWindowHandle");
    list << QLatin1String("Q3DockWindowResizeHandle");
    list << QLatin1String("Q3MainWindow");
    list << QLatin1String("Q3Header");
    list << QLatin1String("Q3ListBox");
    list << QLatin1String("Q3Table");
    list << QLatin1String("Q3TitleBar");

    return list;
}

QAccessibleInterface *CompatAccessibleFactory::create(const QString &classname, QObject *object)
{
    QAccessibleInterface *iface = 0;
    if (!object || !object->isWidgetType())
        return iface;
    QWidget *widget = static_cast<QWidget*>(object);

    if (classname == QLatin1String("Q3TextEdit")) {
        iface = new Q3AccessibleTextEdit(widget);
    } else if (classname == QLatin1String("Q3IconView")) {
        iface = new QAccessibleIconView(widget);
    } else if (classname == QLatin1String("Q3ListView")) {
        iface = new QAccessibleListView(widget);
    } else if (classname == QLatin1String("Q3WidgetStack")) {
        iface = new QAccessibleWidgetStack(widget);
    } else if (classname == QLatin1String("Q3ListBox")) {
        iface = new QAccessibleListBox(widget);
    } else if (classname == QLatin1String("Q3Table")) {
        iface = new Q3AccessibleScrollView(widget, Table);
    } else if (classname == QLatin1String("Q3GroupBox")) {
        iface = new Q3AccessibleDisplay(widget, Grouping);
    } else if (classname == QLatin1String("Q3ToolBar")) {
        iface = new QAccessibleWidget(widget, ToolBar, static_cast<Q3ToolBar *>(widget)->label());
    } else if (classname == QLatin1String("Q3MainWindow")) {
        iface = new QAccessibleWidget(widget, Application);
    } else if (classname == QLatin1String("Q3ToolBarSeparator")) {
        iface = new QAccessibleWidget(widget, Separator);
    } else if (classname == QLatin1String("Q3DockWindowHandle")) {
        iface = new QAccessibleWidget(widget, Grip);
    } else if (classname == QLatin1String("Q3DockWindowResizeHandle")) {
        iface = new QAccessibleWidget(widget, Grip);
    } else if (classname == QLatin1String("Q3Header")) {
        iface = new Q3AccessibleHeader(widget);
    } else if (classname == QLatin1String("Q3TitleBar")) {
        iface = new Q3AccessibleTitleBar(widget);
    }

    return iface;
}

Q_EXPORT_STATIC_PLUGIN(CompatAccessibleFactory)
Q_EXPORT_PLUGIN2(qtaccessiblecompatwidgets, CompatAccessibleFactory)
