/****************************************************************************
**
** Definition of QStyleOption class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef QSTYLEOPTION_H
#define QSTYLEOPTION_H

#include "qabstractspinbox.h"
#include "qiconset.h"
#include "qslider.h"
#include "qstyle.h"
#include "qtabbar.h"
#include "qtoolbutton.h"

struct Q_GUI_EXPORT QStyleOption {
    int version;
    int type;
    QStyle::SFlags state;
    QRect rect;             // Rect has overloaded meanings.
    QPalette palette;
    enum { Default, FocusRect, Button, Tab, MenuItem, Complex, Slider, Frame, ProgressBar,
           ListView, ListViewItem, Header, DockWindow, SpinBox, ToolButton, ComboBox, ToolBox,
           TitleBar, ViewItem, CustomBase = 0xf0000000 };
    enum { Type = Default };
    QStyleOption(int optionversion, int optiontype = Default);
    void init(const QWidget *w);
};

struct QStyleOptionFocusRect  : public QStyleOption {
    enum { Type = FocusRect };
    QColor backgroundColor;
    QStyleOptionFocusRect(int version) : QStyleOption(version, FocusRect) {}
};

struct QStyleOptionFrame : public QStyleOption {
    enum { Type = Frame };
    int lineWidth;
    int midLineWidth;
    QStyleOptionFrame(int version) : QStyleOption(version, Frame) {}
};

struct QStyleOptionHeader : public QStyleOption {
    enum { Type = Header };
    int section;
    QString text;
    QIconSet icon;
    QStyleOptionHeader(int version) : QStyleOption(version, Header) {}
};

struct QStyleOptionButton : public QStyleOption {
    enum { Type = Button };
    enum Extras { None = 0x00, Flat = 0x01, HasMenu = 0x02 };
    uint extras;
    QString text;
    QIconSet icon;
    QStyleOptionButton(int version) : QStyleOption(version, Button) {}
};

struct QStyleOptionTab : public QStyleOption {
    enum { Type = Tab };
    QTabBar::Shape tabshape;
    QString text;
    QIconSet icon;
    QStyleOptionTab(int version) : QStyleOption(version, Tab) {}
};

struct QStyleOptionProgressBar : public QStyleOption
{
    enum { Type = ProgressBar };
    enum Extras { None, CenterIndicator = 0x01, PercentageVisible = 0x02,
                  IndicatorFollowsStyle = 0x03 };
    uint extras;
    QString progressString;
    int totalSteps;
    int progress;
    QStyleOptionProgressBar(int version) : QStyleOption(version, ProgressBar) {}
};

struct QStyleOptionMenuItem : public QStyleOption {
    enum { Type = MenuItem };
    enum MenuItemType { Normal, Separator, SubMenu, Scroller, TearOff, Margin, EmptyArea, Q3Custom };
    enum CheckState { NotCheckable, Checked, Unchecked };
    MenuItemType menuItemType;
    CheckState checkState;
    QRect menuRect;
    QString text;
    QIconSet icon;
    int maxIconWidth;
    int tabWidth;
    QSize q3CustomItemSizeHint;
    bool q3CustomItemFullSpan;
    QStyleOptionMenuItem(int version) : QStyleOption(version, MenuItem) {}
};

struct QStyleOptionComplex : public QStyleOption
{
    enum { Type = Complex };
    QStyle::SCFlags parts;
    QStyle::SCFlags activeParts;
    QStyleOptionComplex(int version, int type = Complex) : QStyleOption(version, type) {}
};

struct QStyleOptionSlider : public QStyleOptionComplex {
    enum { Type = Slider };
    Qt::Orientation orientation;
    int minimum;
    int maximum;
    QSlider::TickSetting tickmarks;
    int tickInterval;
    bool useRightToLeft;
    // These two values are typically the same, but different if tracking is not enabled.
    int sliderPosition;
    int sliderValue;
    int singleStep;
    int pageStep;
    QStyleOptionSlider(int version) : QStyleOptionComplex(version, Slider) {}
};

struct QStyleOptionSpinBox : public QStyleOptionComplex {
    enum { Type = SpinBox };
    QAbstractSpinBox::ButtonSymbols buttonSymbols;
    QAbstractSpinBox::StepEnabled stepEnabled;
    double percentage;
    bool slider;
    QStyleOptionSpinBox(int version) : QStyleOptionComplex(version, SpinBox) {}
};

struct QStyleOptionListViewItem : public QStyleOption
{
    enum { Type = ListViewItem };
    enum Extras { None = 0x00, Expandable = 0x01, MultiLine = 0x02, Visible = 0x04,
                  ParentControl = 0x08 };
    uint extras;
    int height;
    int totalHeight;
    int itemY;
    int childCount;
    QStyleOptionListViewItem(int version) : QStyleOption(version, ListViewItem) {}
};

struct QStyleOptionListView : public QStyleOptionComplex {
    enum { Type = ListView };
    // List of listview items. The first item corresponds to the old ListViewItem we passed,
    // all the other items are its children
    QList<QStyleOptionListViewItem> items;
    QPalette viewportPalette;
    QPalette::ColorRole viewportBGRole;
    int sortColumn;
    int itemMargin;
    int treeStepSize;
    bool rootIsDecorated;
    QStyleOptionListView(int version) : QStyleOptionComplex(version, ListView) {}
};

struct QStyleOptionDockWindow : public QStyleOption
{
    enum { Type = DockWindow };
    bool docked;
    bool isCloseEnabled;
    QStyleOptionDockWindow(int version) : QStyleOption(version , DockWindow) {}
};

struct QStyleOptionToolButton : public QStyleOptionComplex
{
    enum { Type = ToolButton };
    enum Extras { None = 0x00, Arrow = 0x01, TextLabel = 0x02, Menu = 0x04, PopupDelay = 0x08,
                  BigPixmap = 0x10 };
    uint extras;
    QIconSet icon;
    QString text;
    Qt::ArrowType arrowType;
    QPalette::ColorRole bgRole;
    QPalette::ColorRole parentBGRole;
    QPalette parentPalette;
    QPoint pos;
    QFont font;
    QToolButton::TextPosition textPosition;
    QStyleOptionToolButton(int version) : QStyleOptionComplex(version, ToolButton) {}
};

struct QStyleOptionComboBox : public QStyleOptionComplex
{
    enum { Type = ComboBox };
    bool editable;
    QRect popupRect;
    QStyleOptionComboBox(int version) : QStyleOptionComplex(version, ComboBox) {}
};

struct QStyleOptionToolBox : public QStyleOption
{
    enum { Type = ToolBox };
    QString text;
    QIconSet icon;
    QPalette::ColorRole bgRole;
    QPalette::ColorRole currentWidgetBGRole;
    QPalette currentWidgetPalette;
    QStyleOptionToolBox(int version) : QStyleOption(version, ToolBox) {};
};

struct QStyleOptionTitleBar : public QStyleOptionComplex
{
    enum { Type = TitleBar };
    QString text;
    QPixmap icon;
    int titleBarState;
    Qt::WFlags titleBarFlags;
    QStyleOptionTitleBar(int version) : QStyleOptionComplex(version, TitleBar) {};
};

struct QStyleOptionViewItem : public QStyleOption
{
    enum { Type = ViewItem };
    enum Position { Left, Right, Top, Bottom };
    enum Size { Large, Small };
    int displayAlignment;
    int decorationAlignment;
    Position decorationPosition;
    Size decorationSize;
    QStyleOptionViewItem(int version) : QStyleOption(version, ViewItem) {}
};

template <typename T>
T qt_cast(const QStyleOption *opt)
{
    if (opt->type == static_cast<T>(0)->Type)
        return static_cast<T>(opt);
    return 0;
}

template <typename T>
T qt_cast(QStyleOption *opt)
{
    if (opt->type == static_cast<T>(0)->Type)
        return static_cast<T>(opt);
    return 0;
}
#endif
