#include "qaccessiblewidgets.h"
#include "qaccessiblemenu.h"
#include "simplewidgets.h"
#include "rangecontrols.h"

#include <qaccessibleplugin.h>
#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qvariant.h>

class AccessibleFactory : public QAccessiblePlugin, QAccessible
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
    list << "QSpinWidget";
    list << "QDial";
    list << "QScrollBar";
    list << "QSlider";
    list << "QToolButton";
    list << "QCheckBox";
    list << "QRadioButton";
    list << "QButton";
    list << "QViewportWidget";
    list << "QClipperWidget";
    list << "QTextEdit";
#ifndef QT_NO_ICONVIEW
    list << "QIconView";
#endif
    list << "QListView";
    list << "QListBox";
    list << "QTable";
    list << "QDialog";
    list << "QMessageBox";
    list << "QMainWindow";
    list << "QLabel";
    list << "QGroupBox";
    list << "QStatusBar";
    list << "QProgressBar";
    list << "QToolBar";
    list << "QMenuBar";
    list << "QPopupMenu";
    list << "QHeader";
    list << "QTabBar";
    list << "QTitleBar";
    list << "QWorkspaceChild";
    list << "QSizeGrip";
    list << "QSplitterHandle";
    list << "QToolBarSeparator";
    list << "QDockWindowHandle";
    list << "QDockWindowResizeHandle";
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
	iface = new QAccessibleText(widget, EditableText);
    }
    else if (classname == "QComboBox") {
	iface = new QAccessibleComboBox(widget);
    } else if (classname == "QSpinBox") {
	iface = new QAccessibleRangeControl(widget, SpinBox);
    } else if (classname == "QSpinWidget") {
	iface = new QAccessibleSpinWidget(widget);
    } else if (classname == "QDial") {
	iface = new QAccessibleRangeControl(widget, Dial);
    } else if (classname == "QScrollBar") {
	iface = new QAccessibleScrollBar(widget);
    } else if (classname == "QSlider") {
	iface = new QAccessibleSlider(widget);
    } else if (classname == "QToolButton") {
	QToolButton *tb = (QToolButton*)widget;
	if (!tb->popup())
	    iface = new QAccessibleButton(widget, PushButton);
	else if (!tb->popupDelay())
	    iface = new QAccessibleButton(widget, ButtonDropDown);
	else
	    iface = new QAccessibleButton(widget, ButtonMenu);
    } else if (classname == "QCheckBox") {
	iface = new QAccessibleButton(widget, CheckBox);
    } else if (classname == "QRadioButton") {
	iface = new QAccessibleButton(widget, RadioButton);
    } else if (classname == "QButton") {
	iface = new QAccessibleButton(widget, PushButton);
    } else if (classname == "QViewportWidget") {
	iface = new QAccessibleViewport(widget, widget->parentWidget());
    } else if (classname == "QClipperWidget") {
	iface = new QAccessibleViewport(widget, widget->parentWidget()->parentWidget());
    } else if (classname == "QTextEdit") {
	iface = new QAccessibleTextEdit(widget);
#ifndef QT_NO_ICONVIEW
    } else if (classname == "QIconView") {
	iface = new QAccessibleIconView(widget);
#endif
    } else if (classname == "QListView") {
	iface = new QAccessibleListView(widget);
    } else if (classname == "QListBox") {
	iface = new QAccessibleListBox(widget);
    } else if (classname == "QTable") {
	iface = new QAccessibleScrollView(widget, Table);
    } else if (classname == "QDialog") {
	iface = new QAccessibleWidget(widget, Dialog);
    } else if (classname == "QMessageBox") {
	iface = new QAccessibleWidget(widget, AlertMessage);
    } else if (classname == "QMainWindow") {
	iface = new QAccessibleWidget(widget, Application);
    } else if (classname == "QLabel" || classname == "QLCDNumber") {
	iface = new QAccessibleDisplay(widget, StaticText);
    } else if (classname == "QGroupBox") {
	iface = new QAccessibleDisplay(widget, Grouping);
    } else if (classname == "QStatusBar") {
	iface = new QAccessibleWidget(widget, StatusBar);
    } else if (classname == "QProgressBar") {
	iface = new QAccessibleRangeControl(widget, ProgressBar);
    } else if (classname == "QToolBar") {
	QToolBar *tb = (QToolBar*)widget;
	iface = new QAccessibleWidget(widget, ToolBar, tb->label());
    } else if (classname == "QMenuBar") {
	iface = new QAccessibleMenuBar(widget);
    } else if (classname == "QPopupMenu") {
	iface = new QAccessiblePopup(widget);
    } else if (classname == "QHeader") {
	iface = new QAccessibleHeader(widget);
    } else if (classname == "QTabBar") {
	iface = new QAccessibleTabBar(widget);
    } else if (classname == "QTitleBar") {
	iface = new QAccessibleTitleBar(widget);
    } else if (classname == "QWorkspaceChild") {
	iface = new QAccessibleWidget(widget, Window);
    } else if (classname == "QSizeGrip") {
	iface = new QAccessibleWidget(widget, Grip);
    } else if (classname == "QSplitterHandle") {
	iface = new QAccessibleWidget(widget, Separator, QString::null, 
					QString::null, QString::null, QString::null, NoAction,
					QString::null, QString::null, Moveable);
    } else if (classname == "QToolBarSeparator") {
	iface = new QAccessibleWidget(widget, QAccessible::Separator, QString::null, 
					QString::null, QString::null, QString::null, NoAction,
					QString::null, QString::null, Unavailable);
    } else if (classname == "QDockWindowHandle") {
	iface = new QAccessibleWidget(widget, QAccessible::Grip, widget->property( "caption").toString(),
					QString::null, QString::null, QString::null, NoAction,
					QString::null, QString::null, Moveable);
    } else if (classname == "QDockWindowResizeHandle") {
	iface = new QAccessibleWidget(widget, QAccessible::Separator, QString::null,
					QString::null, QString::null, QString::null, NoAction,
					QString::null, QString::null, Moveable);
    } else if (classname == "QTipLabel") {
	iface = new QAccessibleWidget(widget, ToolTip);
    } else if (classname == "QFrame") {
	iface = new QAccessibleWidget(widget, Border);
    } else if (classname == "QWidgetStack") {
	iface = new QAccessibleWidgetStack(widget);
    }

    return iface;
}

Q_EXPORT_PLUGIN(AccessibleFactory)
