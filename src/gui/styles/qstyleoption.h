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

#include "qiconset.h"
#include "qslider.h"
#include "qstyle.h"
#include "qtabbar.h"

struct Q_GUI_EXPORT Q4StyleOption {
    int version;
    int type;
    QStyle::SFlags state;
    QRect rect;             // Rect has overloaded meanings.
    QPalette palette;
    enum { Default, FocusRect, Button, Tab, MenuItem, Complex, Slider, Frame, ProgressBar,
           ListView, ListViewItem };
    enum { Type = Default };
    Q4StyleOption(int optionversion, int optiontype = Default);
    void init(const QWidget *w);
};

struct Q4StyleOptionFocusRect  : public Q4StyleOption {
    enum { Type = FocusRect };
    QColor backgroundColor;
    Q4StyleOptionFocusRect(int version) : Q4StyleOption(version, FocusRect) {}
};

struct Q4StyleOptionFrame : public Q4StyleOption {
    enum { Type = Frame };
    int lineWidth;
    Q4StyleOptionFrame(int version) : Q4StyleOption(version, Frame) {}
};

struct Q4StyleOptionButton : public Q4StyleOption {
    enum { Type = Button };
    enum Extras { None = 0x00, Flat = 0x01, HasMenu = 0x02 };
    uint extras;
    QString text;
    QIconSet icon;
    Q4StyleOptionButton(int version) : Q4StyleOption(version, Button) {}
};

struct Q4StyleOptionTab : public Q4StyleOption {
    enum { Type = Tab };
    QTabBar::Shape tabshape;
    QString text;
    QIconSet icon;
    Q4StyleOptionTab(int version) : Q4StyleOption(version, Tab) {}
};

struct Q4StyleOptionProgressBar : public Q4StyleOption
{
    enum { Type = ProgressBar };
    enum Extras { None, CenterIndicator = 0x01, PercentageVisible = 0x02, 
                  IndicatorFollowsStyle = 0x03 };
    uint extras;
    QString progressString;
    int totalSteps;
    int progress;
    Q4StyleOptionProgressBar(int version) : Q4StyleOption(version, ProgressBar) {}
};

struct Q4StyleOptionMenuItem : public Q4StyleOption {
    enum { Type = MenuItem };
    enum MenuItemType { Normal, Separator, SubMenu, Scroller, TearOff, EmptyArea, Q3Custom };
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
    Q4StyleOptionMenuItem(int version) : Q4StyleOption(version, MenuItem) {}
};

struct Q4StyleOptionComplex : public Q4StyleOption
{
    enum { Type = Complex };
    QStyle::SCFlags parts;
    QStyle::SCFlags activeParts;
    Q4StyleOptionComplex(int version, int type) : Q4StyleOption(version, type) {}
};

struct Q4StyleOptionSlider : public Q4StyleOptionComplex {
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
    Q4StyleOptionSlider(int version) : Q4StyleOptionComplex(version, Slider) {}
};

struct Q4StyleOptionListViewItem : public Q4StyleOption
{
    enum { Type = ListViewItem };
    enum Extras { None = 0x00, Expandable = 0x01, MultiLine = 0x02, Visible = 0x04, Open = 0x08 };
    uint extras;
    int height;
    int totalHeight;
    int itemY;
    int childCount;
    Q4StyleOptionListViewItem(int version) : Q4StyleOption(version, ListViewItem) {}
};

struct Q4StyleOptionListView : public Q4StyleOptionComplex {
    enum { Type = ListView };
    // List of listview items. The first item corresponds to the old ListViewItem we passed,
    // all the other items are its children
    QList<Q4StyleOptionListViewItem> items;
    QPalette viewportPalette;
    QPalette::ColorRole viewportBGRole;
    int sortColumn;
    int itemMargin;
    Q4StyleOptionListView(int version) : Q4StyleOptionComplex(version, ListView) {}
};

template <typename T>
T qt_cast(const Q4StyleOption *opt)
{
    if (opt->type == static_cast<T>(0)->Type)
        return static_cast<T>(const_cast<Q4StyleOption *>(opt));
    return 0;
}
#endif
