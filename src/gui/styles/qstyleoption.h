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
    QRect rect;
    QPalette palette;
    enum OptionType { SO_Default, SO_FocusRect, SO_Button, SO_Tab, SO_MenuItem, SO_Complex,
                      SO_Slider, SO_Frame, SO_ProgressBar, SO_ListView, SO_ListViewItem,
                      SO_Header, SO_DockWindow, SO_SpinBox, SO_ToolButton, SO_ComboBox,
                      SO_ToolBox, SO_TitleBar, SO_ViewItem,
                      SO_CustomBase = 0xf000000 };
    enum { Type = SO_Default };
    QStyleOption(int optionversion, int optiontype = SO_Default);
    void init(const QWidget *w);
    QDOC_PROPERTY(int version);
    QDOC_PROPERTY(int type);
    QDOC_PROPERTY(QStyle::SFlags state);
    QDOC_PROPERTY(QRect rect);
    QDOC_PROPERTY(QPalette palette);
};

struct QStyleOptionFocusRect  : public QStyleOption {
    enum { Type = SO_FocusRect };
    QColor backgroundColor;
    QStyleOptionFocusRect(int version) : QStyleOption(version, SO_FocusRect) {}
};

struct QStyleOptionFrame : public QStyleOption {
    enum { Type = SO_Frame };
    int lineWidth;
    int midLineWidth;
    QStyleOptionFrame(int version) : QStyleOption(version, SO_Frame) {}
};

struct QStyleOptionHeader : public QStyleOption {
    enum { Type = SO_Header };
    int section;
    QString text;
    QIconSet icon;
    QStyleOptionHeader(int version) : QStyleOption(version, SO_Header) {}
};

struct QStyleOptionButton : public QStyleOption {
    enum { Type = SO_Button };
    enum Extras { None = 0x00, Flat = 0x01, HasMenu = 0x02 };
    uint extras;
    QString text;
    QIconSet icon;
    QStyleOptionButton(int version) : QStyleOption(version, SO_Button) {}
};

struct QStyleOptionTab : public QStyleOption {
    enum { Type = SO_Tab };
    QTabBar::Shape tabshape;
    QString text;
    QIconSet icon;
    QStyleOptionTab(int version) : QStyleOption(version, SO_Tab) {}
};

struct QStyleOptionProgressBar : public QStyleOption
{
    enum { Type = SO_ProgressBar };
    enum Extras { None, CenterIndicator = 0x01, PercentageVisible = 0x02,
                  IndicatorFollowsStyle = 0x03 };
    uint extras;
    QString progressString;
    int totalSteps;
    int progress;
    QStyleOptionProgressBar(int version) : QStyleOption(version, SO_ProgressBar) {}
};

struct QStyleOptionMenuItem : public QStyleOption {
    enum { Type = SO_MenuItem };
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
    QStyleOptionMenuItem(int version) : QStyleOption(version, SO_MenuItem) {}
};

struct QStyleOptionComplex : public QStyleOption
{
    enum { Type = SO_Complex };
    QStyle::SCFlags parts;
    QStyle::SCFlags activeParts;
    QStyleOptionComplex(int version, int type = SO_Complex) : QStyleOption(version, type) {}
};

struct QStyleOptionSlider : public QStyleOptionComplex {
    enum { Type = SO_Slider };
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
    QStyleOptionSlider(int version) : QStyleOptionComplex(version, SO_Slider) {}
};

struct QStyleOptionSpinBox : public QStyleOptionComplex {
    enum { Type = SO_SpinBox };
    QAbstractSpinBox::ButtonSymbols buttonSymbols;
    QAbstractSpinBox::StepEnabled stepEnabled;
    double percentage;
    bool slider;
    QStyleOptionSpinBox(int version) : QStyleOptionComplex(version, SO_SpinBox) {}
};

struct QStyleOptionListViewItem : public QStyleOption
{
    enum { Type = SO_ListViewItem };
    enum Extras { None = 0x00, Expandable = 0x01, MultiLine = 0x02, Visible = 0x04,
                  ParentControl = 0x08 };
    uint extras;
    int height;
    int totalHeight;
    int itemY;
    int childCount;
    QStyleOptionListViewItem(int version) : QStyleOption(version, SO_ListViewItem) {}
};

struct QStyleOptionListView : public QStyleOptionComplex {
    enum { Type = SO_ListView };
    // List of listview items. The first item corresponds to the old ListViewItem we passed,
    // all the other items are its children
    QList<QStyleOptionListViewItem> items;
    QPalette viewportPalette;
    QPalette::ColorRole viewportBGRole;
    int sortColumn;
    int itemMargin;
    int treeStepSize;
    bool rootIsDecorated;
    QStyleOptionListView(int version) : QStyleOptionComplex(version, SO_ListView) {}
};

struct QStyleOptionDockWindow : public QStyleOption
{
    enum { Type = SO_DockWindow };
    bool docked;
    bool isCloseEnabled;
    QStyleOptionDockWindow(int version) : QStyleOption(version , SO_DockWindow) {}
};

struct QStyleOptionToolButton : public QStyleOptionComplex
{
    enum { Type = SO_ToolButton };
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
    QStyleOptionToolButton(int version) : QStyleOptionComplex(version, SO_ToolButton) {}
};

struct QStyleOptionComboBox : public QStyleOptionComplex
{
    enum { Type = SO_ComboBox };
    bool editable;
    QRect popupRect;
    QStyleOptionComboBox(int version) : QStyleOptionComplex(version, SO_ComboBox) {}
};

struct QStyleOptionToolBox : public QStyleOption
{
    enum { Type = SO_ToolBox };
    QString text;
    QIconSet icon;
    QPalette::ColorRole bgRole;
    QPalette::ColorRole currentWidgetBGRole;
    QPalette currentWidgetPalette;
    QStyleOptionToolBox(int version) : QStyleOption(version, SO_ToolBox) {};
};

struct QStyleOptionTitleBar : public QStyleOptionComplex
{
    enum { Type = SO_TitleBar };
    QString text;
    QPixmap icon;
    int titleBarState;
    Qt::WFlags titleBarFlags;
    QStyleOptionTitleBar(int version) : QStyleOptionComplex(version, SO_TitleBar) {};
};

struct QStyleOptionViewItem : public QStyleOption
{
    enum { Type = SO_ViewItem };
    enum Position { Left, Right, Top, Bottom };
    enum Size { Large, Small };
    int displayAlignment;
    int decorationAlignment;
    Position decorationPosition;
    Size decorationSize;
    QStyleOptionViewItem(int version) : QStyleOption(version, SO_ViewItem) {}
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
