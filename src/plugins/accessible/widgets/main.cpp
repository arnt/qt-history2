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

#ifndef QT_NO_ACCESSIBILITY

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
#ifndef QT_NO_LINEEDIT
    list << QLatin1String("QLineEdit");
#endif
#ifndef QT_NO_COMBOBOX
    list << QLatin1String("QComboBox");
#endif
#ifndef QT_NO_SPINBOX
    list << QLatin1String("QSpinBox");
    list << QLatin1String("QDoubleSpinBox");
#endif
#ifndef QT_NO_SCROLLBAR
    list << QLatin1String("QScrollBar");
#endif
#ifndef QT_NO_SLIDER
    list << QLatin1String("QSlider");
#endif
#ifndef QT_NO_TOOLBUTTON
    list << QLatin1String("QToolButton");
#endif
    list << QLatin1String("QCheckBox");
    list << QLatin1String("QRadioButton");
    list << QLatin1String("QPushButton");
    list << QLatin1String("QButton");
    list << QLatin1String("QDialog");
    list << QLatin1String("QMessageBox");
    list << QLatin1String("QMainWindow");
    list << QLatin1String("QLabel");
    list << QLatin1String("QLCDNumber");
    list << QLatin1String("QGroupBox");
    list << QLatin1String("QStatusBar");
    list << QLatin1String("QProgressBar");
    list << QLatin1String("QMenuBar");
    list << QLatin1String("Q3PopupMenu");
    list << QLatin1String("QMenu");
    list << QLatin1String("QHeaderView");
    list << QLatin1String("QTabBar");
    list << QLatin1String("QToolBar");
    list << QLatin1String("QWorkspaceChild");
    list << QLatin1String("QSizeGrip");
    list << QLatin1String("QAbstractItemView");
#ifndef QT_NO_SPLITTER
    list << QLatin1String("QSplitter");
    list << QLatin1String("QSplitterHandle");
#endif
#ifndef QT_NO_TEXTEDIT
    list << QLatin1String("QTextEdit");
#endif
    list << QLatin1String("QTipLabel");
    list << QLatin1String("QFrame");
    list << QLatin1String("QStackedWidget");
    list << QLatin1String("QToolBox");
    list << QLatin1String("QMdiArea");
    list << QLatin1String("QMdiSubWindow");
    list << QLatin1String("QWorkspace");
    list << QLatin1String("QDialogButtonBox");
#ifndef QT_NO_DIAL
    list << QLatin1String("QDial");
#endif
#ifndef QT_NO_RUBBERBAND
    list << QLatin1String("QRubberBand");
#endif

    return list;
}

QAccessibleInterface *AccessibleFactory::create(const QString &classname, QObject *object)
{
    QAccessibleInterface *iface = 0;
    if (!object || !object->isWidgetType())
        return iface;
    QWidget *widget = static_cast<QWidget*>(object);

    if (false) {
#ifndef QT_NO_LINEEDIT
    } else if (classname == QLatin1String("QLineEdit")) {
        iface = new QAccessibleLineEdit(widget);
#endif
#ifndef QT_NO_COMBOBOX
    } else if (classname == QLatin1String("QComboBox")) {
        iface = new QAccessibleComboBox(widget);
#endif
#ifndef QT_NO_SPINBOX
    } else if (classname == QLatin1String("QSpinBox")) {
        iface = new QAccessibleSpinBox(widget);
    } else if (classname == QLatin1String("QDoubleSpinBox")) {
        iface = new QAccessibleDoubleSpinBox(widget);
#endif
#ifndef QT_NO_SCROLLBAR
    } else if (classname == QLatin1String("QScrollBar")) {
        iface = new QAccessibleScrollBar(widget);
#endif
#ifndef QT_NO_SLIDER
    } else if (classname == QLatin1String("QSlider")) {
        iface = new QAccessibleSlider(widget);
#endif
#ifndef QT_NO_TOOLBUTTON
    } else if (classname == QLatin1String("QToolButton")) {
        Role role = NoRole;
        QToolButton *tb = qobject_cast<QToolButton*>(widget);
#ifndef QT_NO_MENU
        if (!tb->menu())
            role = tb->isCheckable() ? CheckBox : PushButton;
        else if (!tb->popupMode() != QToolButton::DelayedPopup)
            role = ButtonDropDown;
        else
#endif
            role = ButtonMenu;
        iface = new QAccessibleToolButton(widget, role);
#endif // QT_NO_TOOLBUTTON
    } else if (classname == QLatin1String("QCheckBox")) {
        iface = new QAccessibleButton(widget, CheckBox);
    } else if (classname == QLatin1String("QRadioButton")) {
        iface = new QAccessibleButton(widget, RadioButton);
    } else if (classname == QLatin1String("QPushButton")) {
        Role role = NoRole;
        QPushButton *pb = qobject_cast<QPushButton*>(widget);
#ifndef QT_NO_MENU
        if (pb->menu())
            role = ButtonMenu;
        else
#endif
        if (pb->isCheckable())
            role = CheckBox;
        else
            role = PushButton;
        iface = new QAccessibleButton(widget, role);
    } else if (classname == QLatin1String("QButton")) {
        iface = new QAccessibleButton(widget, PushButton);
    } else if (classname == QLatin1String("QDialog")) {
        iface = new QAccessibleWidget(widget, Dialog);
    } else if (classname == QLatin1String("QMessageBox")) {
        iface = new QAccessibleWidget(widget, AlertMessage);
    } else if (classname == QLatin1String("QMainWindow")) {
        iface = new QAccessibleWidget(widget, Application);
    } else if (classname == QLatin1String("QLabel") || classname == QLatin1String("QLCDNumber")) {
        iface = new QAccessibleDisplay(widget);
    } else if (classname == QLatin1String("QGroupBox")) {
        iface = new QAccessibleDisplay(widget, Grouping);
    } else if (classname == QLatin1String("QStatusBar")) {
        iface = new QAccessibleWidget(widget, StatusBar);
    } else if (classname == QLatin1String("QProgressBar")) {
        iface = new QAccessibleDisplay(widget);
    } else if (classname == QLatin1String("QToolBar")) {
        iface = new QAccessibleWidget(widget, ToolBar, widget->windowTitle());
#ifndef QT_NO_MENUBAR
    } else if (classname == QLatin1String("QMenuBar")) {
        iface = new QAccessibleMenuBar(widget);
#endif
#ifndef QT_NO_MENU
    } else if (classname == QLatin1String("QMenu")) {
        iface = new QAccessibleMenu(widget);
    } else if (classname == QLatin1String("Q3PopupMenu")) {
        iface = new QAccessibleMenu(widget);
#endif
#ifndef QT_NO_ITEMVIEWS
    } else if (classname == QLatin1String("QHeaderView")) {
        iface = new QAccessibleHeader(widget);
    } else if (classname == QLatin1String("QAbstractItemView")) {
        iface = new QAccessibleItemView(widget);
#endif
#ifndef QT_NO_TABBAR
    } else if (classname == QLatin1String("QTabBar")) {
        iface = new QAccessibleTabBar(widget);
#endif
    } else if (classname == QLatin1String("QWorkspaceChild")) {
        iface = new QAccessibleWidget(widget, Window);
    } else if (classname == QLatin1String("QSizeGrip")) {
        iface = new QAccessibleWidget(widget, Grip);
#ifndef QT_NO_SPLITTER
    } else if (classname == QLatin1String("QSplitter")) {
        iface = new QAccessibleWidget(widget, Splitter);
    } else if (classname == QLatin1String("QSplitterHandle")) {
        iface = new QAccessibleWidget(widget, Grip);
#endif
#ifndef QT_NO_TEXTEDIT
    } else if (classname == QLatin1String("QTextEdit")) {
        iface = new QAccessibleTextEdit(widget);
#endif
    } else if (classname == QLatin1String("QTipLabel")) {
        iface = new QAccessibleWidget(widget, ToolTip);
    } else if (classname == QLatin1String("QFrame")) {
        iface = new QAccessibleWidget(widget, Border);
    } else if (classname == QLatin1String("QStackedWidget")) {
        iface = new QAccessibleStackedWidget(widget);
    } else if (classname == QLatin1String("QToolBox")) {
        iface = new QAccessibleToolBox(widget);
    } else if (classname == QLatin1String("QMdiArea")) {
        iface = new QAccessibleMdiArea(widget);
    } else if (classname == QLatin1String("QMdiSubWindow")) {
        iface = new QAccessibleMdiSubWindow(widget);
    } else if (classname == QLatin1String("QWorkspace")) {
        iface = new QAccessibleWorkspace(widget);
    } else if (classname == QLatin1String("QDialogButtonBox")) {
        iface = new QAccessibleDialogButtonBox(widget);
#ifndef QT_NO_DIAL
    } else if (classname == QLatin1String("QDial")) {
        iface = new QAccessibleDial(widget);
#endif
#ifndef QT_NO_RUBBERBAND
    } else if (classname == QLatin1String("QRubberBand")) {
        iface = new QAccessibleRubberBand(widget);
#endif
    }

    return iface;
}

Q_EXPORT_STATIC_PLUGIN(AccessibleFactory)
Q_EXPORT_PLUGIN2(qtaccessiblewidgets, AccessibleFactory)

#endif // QT_NO_ACCESSIBILITY
