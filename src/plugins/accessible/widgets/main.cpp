/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qaccessiblewidgets.h"
#include "qaccessiblemenu.h"
#include "simplewidgets.h"
#include "rangecontrols.h"
#include "complexwidgets.h"

#include <qaccessibleplugin.h>
#include <qplugin.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qvariant.h>
#include <qaccessible.h>

class AccessibleFactory : public QAccessiblePlugin
{
public:
    AccessibleFactory();

    QStringList keys() const;
    QAccessibleInterface *create(const QString &classname, QObject *object);
};

AccessibleFactory::AccessibleFactory()
{
}

QStringList AccessibleFactory::keys() const
{
    QStringList list;
    list << "QLineEdit";
    list << "QComboBox";
    list << "QSpinBox";
    list << "QScrollBar";
    list << "QSlider";
    list << "QToolButton";
    list << "QCheckBox";
    list << "QRadioButton";
    list << "QPushButton";
    list << "QButton";
    list << "QViewportWidget";
    list << "QClipperWidget";
    list << "QDialog";
    list << "QMessageBox";
    list << "QMainWindow";
    list << "QLabel";
    list << "QLCDNumber";
    list << "QGroupBox";
    list << "QStatusBar";
    list << "QProgressBar";
    list << "QMenuBar";
    list << "QPopupMenu";
    list << "QMenu";
    list << "QHeaderView";
    list << "QTabBar";
    list << "QToolBar";
    list << "QWorkspaceChild";
    list << "QSizeGrip";
    list << "QSplitter";
    list << "QSplitterHandle";
    list << "QTipLabel";
    list << "QFrame";
    list << "QWidgetStack";

    return list;
}

QAccessibleInterface *AccessibleFactory::create(const QString &classname, QObject *object)
{
    QAccessibleInterface *iface = 0;
    if (!object || !object->isWidgetType())
        return iface;
    QWidget *widget = static_cast<QWidget*>(object);

    if (classname == "QLineEdit") {
        iface = new QAccessibleLineEdit(widget);
    } else if (classname == "QComboBox") {
        iface = new QAccessibleComboBox(widget);
    } else if (classname == "QSpinBox") {
        iface = new QAccessibleSpinBox(widget);
    } else if (classname == "QScrollBar") {
        iface = new QAccessibleScrollBar(widget);
    } else if (classname == "QSlider") {
        iface = new QAccessibleSlider(widget);
    } else if (classname == "QToolButton") {
        Role role = NoRole;
        QToolButton *tb = qt_cast<QToolButton*>(widget);
        if (!tb->menu())
            role = tb->isCheckable() ? CheckBox : PushButton;
        else if (!tb->popupMode() != QToolButton::DelayedPopupMode)
            role = ButtonDropDown;
        else
            role = ButtonMenu;
        iface = new QAccessibleToolButton(widget, role);
    } else if (classname == "QCheckBox") {
        iface = new QAccessibleButton(widget, CheckBox);
    } else if (classname == "QRadioButton") {
        iface = new QAccessibleButton(widget, RadioButton);
    } else if (classname == "QPushButton") {
        Role role = NoRole;
        QPushButton *pb = qt_cast<QPushButton*>(widget);
        if (pb->menu())
            role = ButtonMenu;
        else if (pb->isCheckable())
            role = CheckBox;
        else
            role = PushButton;
        iface = new QAccessibleButton(widget, role);
    } else if (classname == "QButton") {
        iface = new QAccessibleButton(widget, PushButton);
    } else if (classname == "QViewportWidget") {
        iface = new QAccessibleViewport(widget, widget->parentWidget());
    } else if (classname == "QClipperWidget") {
        iface = new QAccessibleViewport(widget, widget->parentWidget()->parentWidget());
    } else if (classname == "QDialog") {
        iface = new QAccessibleWidget(widget, Dialog);
    } else if (classname == "QMessageBox") {
        iface = new QAccessibleWidget(widget, AlertMessage);
    } else if (classname == "QMainWindow") {
        iface = new QAccessibleWidget(widget, Application);
    } else if (classname == "QLabel" || classname == "QLCDNumber") {
        iface = new QAccessibleDisplay(widget);
    } else if (classname == "QGroupBox") {
        iface = new QAccessibleDisplay(widget, Grouping);
    } else if (classname == "QStatusBar") {
        iface = new QAccessibleWidget(widget, StatusBar);
    } else if (classname == "QProgressBar") {
        iface = new QAccessibleDisplay(widget);
    } else if (classname == "QToolBar") {
        iface = new QAccessibleWidget(widget, ToolBar, widget->windowTitle());
    } else if (classname == "QMenuBar") {
        iface = new QAccessibleMenuBar(widget);
    } else if (classname == "QMenu") {
        iface = new QAccessibleMenu(widget);
    } else if (classname == "QPopupMenu") {
        iface = new QAccessibleMenu(widget);
    } else if (classname == "QHeaderView") {
        iface = new QAccessibleHeader(widget);
    } else if (classname == "QTabBar") {
        iface = new QAccessibleTabBar(widget);
    } else if (classname == "QWorkspaceChild") {
        iface = new QAccessibleWidget(widget, Window);
    } else if (classname == "QSizeGrip") {
        iface = new QAccessibleWidget(widget, Grip);
    } else if (classname == "QSplitter") {
        iface = new QAccessibleWidget(widget, Splitter);
    } else if (classname == "QSplitterHandle") {
        iface = new QAccessibleWidget(widget, Grip);
    } else if (classname == "QTipLabel") {
        iface = new QAccessibleWidget(widget, ToolTip);
    } else if (classname == "QFrame") {
        iface = new QAccessibleWidget(widget, Border);
    }

    return iface;
}

Q_EXPORT_PLUGIN(AccessibleFactory)
