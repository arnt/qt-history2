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
    enum OptionType { SO_Default, SO_FocusRect, SO_Button, SO_Tab, SO_MenuItem,
                      SO_Frame, SO_ProgressBar, SO_ToolBox, SO_Header, SO_DockWindow,
                      SO_Complex, SO_Slider, SO_SpinBox, SO_ToolButton, SO_ComboBox,
                      SO_ListView, SO_ListViewItem, SO_TitleBar, SO_ViewItem,
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
    QDOC_PROPERTY(QColor backgroundColor);
};

struct QStyleOptionFrame : public QStyleOption {
    enum { Type = SO_Frame };
    int lineWidth;
    int midLineWidth;
    QStyleOptionFrame(int version)
        : QStyleOption(version, SO_Frame), lineWidth(0), midLineWidth(0) {}
    QDOC_PROPERTY(int lineWidth);
    QDOC_PROPERTY(int midLineWidth);
};

struct QStyleOptionHeader : public QStyleOption {
    enum { Type = SO_Header };
    int section;
    QString text;
    QIconSet icon;
    QStyleOptionHeader(int version) : QStyleOption(version, SO_Header), section(0) {}
    QDOC_PROPERTY(int section);
    QDOC_PROPERTY(QString text);
    QDOC_PROPERTY(QIconSet icon);
};

struct QStyleOptionButton : public QStyleOption {
    enum { Type = SO_Button };
    enum ButtonFeature { None = 0x00, Flat = 0x01, HasMenu = 0x02 };
    Q_DECLARE_FLAGS(ButtonFeatures, ButtonFeature);
    ButtonFeatures features;
    QString text;
    QIconSet icon;
    QStyleOptionButton(int version) : QStyleOption(version, SO_Button), features(None) {}
    QDOC_PROPERTY(ButtonFeatures features);
    QDOC_PROPERTY(QString text);
    QDOC_PROPERTY(QIconSet icon);
};

struct QStyleOptionTab : public QStyleOption {
    enum { Type = SO_Tab };
    QTabBar::Shape shape;
    QString text;
    QIconSet icon;
    int row;
    QStyleOptionTab(int version) : QStyleOption(version, SO_Tab), row(0) {}
    QDOC_PROPERTY(QTabBar::Shape shape);
    QDOC_PROPERTY(QString text);
    QDOC_PROPERTY(QIconSet icon);
    QDOC_PROPERTY(int row);
};

struct QStyleOptionProgressBar : public QStyleOption
{
    enum { Type = SO_ProgressBar };
    enum ProgressBarFeature { None, CenterIndicator = 0x01, PercentageVisible = 0x02,
                              IndicatorFollowsStyle = 0x03 };
    Q_DECLARE_FLAGS(ProgressBarFeatures, ProgressBarFeature);
    ProgressBarFeatures features;
    QString progressString;
    int totalSteps;
    int progress;
    QStyleOptionProgressBar(int version)
        : QStyleOption(version, SO_ProgressBar), features(None), totalSteps(0), progress(0) {}
    QDOC_PROPERTY(ProgressBarFeatures features);
    QDOC_PROPERTY(QString progressString);
    QDOC_PROPERTY(int totalSteps);
    QDOC_PROPERTY(int progress);
};

struct QStyleOptionMenuItem : public QStyleOption {
    enum { Type = SO_MenuItem };
    enum MenuItemType { Normal, Separator, SubMenu, Scroller, TearOff, Margin,
			EmptyArea, Q3Custom };
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
    QFont font;
    QStyleOptionMenuItem(int version) : QStyleOption(version, SO_MenuItem), menuItemType(Normal),
    checkState(NotCheckable), maxIconWidth(0), tabWidth(0), q3CustomItemFullSpan(false) {}
    QDOC_PROPERTY(MenuItemType menuItemType);
    QDOC_PROPERTY(CheckState checkState);
    QDOC_PROPERTY(QRect menuRect);
    QDOC_PROPERTY(QString text);
    QDOC_PROPERTY(QIconSet icon);
    QDOC_PROPERTY(int maxIconWidth);
    QDOC_PROPERTY(int tabWidth);
    QDOC_PROPERTY(QSize q3CustomItemSizeHint);
    QDOC_PROPERTY(bool q3CustomItemFullSpan);
    QDOC_PROPERTY(QFont font);
};

struct QStyleOptionComplex : public QStyleOption
{
    enum { Type = SO_Complex };
    QStyle::SCFlags parts;
    QStyle::SCFlags activeParts;
    QStyleOptionComplex(int version, int type = SO_Complex)
        : QStyleOption(version, type), parts(QStyle::SC_All), activeParts(QStyle::SC_None) {}
    QDOC_PROPERTY(QStyle::SCFlags parts);
    QDOC_PROPERTY(QStyle::SCFlags activeParts);
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
    QStyleOptionSlider(int version)
        : QStyleOptionComplex(version, SO_Slider), minimum(0), maximum(0),
          tickmarks(QSlider::NoMarks), tickInterval(0), useRightToLeft(false), sliderPosition(0),
          sliderValue(0), singleStep(0), pageStep(0) {}
    QDOC_PROPERTY(Qt::Orientation orientation);
    QDOC_PROPERTY(int maximum);
    QDOC_PROPERTY(int minimum);
    QDOC_PROPERTY(QSlider::TickSetting tickmarks);
    QDOC_PROPERTY(int tickInterval);
    QDOC_PROPERTY(bool useRightToLeft);
    QDOC_PROPERTY(int sliderPosition);
    QDOC_PROPERTY(int sliderValue);
    QDOC_PROPERTY(int singleStep);
    QDOC_PROPERTY(int pageStep);
};

struct QStyleOptionSpinBox : public QStyleOptionComplex {
    enum { Type = SO_SpinBox };
    QAbstractSpinBox::ButtonSymbols buttonSymbols;
    QAbstractSpinBox::StepEnabled stepEnabled;
    double percentage;
    bool slider;
    QStyleOptionSpinBox(int version)
        : QStyleOptionComplex(version, SO_SpinBox), buttonSymbols(QAbstractSpinBox::UpDownArrows),
          stepEnabled(QAbstractSpinBox::StepNone), percentage(0.0), slider(false) {}
    QDOC_PROPERTY(QAbstractSpinBox::ButtonSymbols buttonSymbols);
    QDOC_PROPERTY(QAbstractSpinBox::StepEnabled stepEnabled);
    QDOC_PROPERTY(double percentage);
    QDOC_PROPERTY(bool slider);
};

struct QStyleOptionListViewItem : public QStyleOption
{
    enum { Type = SO_ListViewItem };
    enum ListViewItemFeature { None = 0x00, Expandable = 0x01, MultiLine = 0x02, Visible = 0x04,
                               ParentControl = 0x08 };
    Q_DECLARE_FLAGS(ListViewItemFeatures, ListViewItemFeature);
    ListViewItemFeatures features;
    int height;
    int totalHeight;
    int itemY;
    int childCount;
    QStyleOptionListViewItem(int version)
        : QStyleOption(version, SO_ListViewItem), features(None), height(0), totalHeight(0),
          itemY(0), childCount(0) {}
    QDOC_PROPERTY(ListViewItemFeatures features);
    QDOC_PROPERTY(int height);
    QDOC_PROPERTY(int totalHeight);
    QDOC_PROPERTY(int itemY);
    QDOC_PROPERTY(int childCount);
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
    QStyleOptionListView(int version) : QStyleOptionComplex(version, SO_ListView), sortColumn(0),
    itemMargin(0), treeStepSize(0), rootIsDecorated(false) {}
    QDOC_PROPERTY(QList<QStyleOptionListViewItem> items);
    QDOC_PROPERTY(QPalette viewportPalette);
    QDOC_PROPERTY(QPalette::ColorRole viewportBGRole);
    QDOC_PROPERTY(int sortColumn);
    QDOC_PROPERTY(int itemMargin);
    QDOC_PROPERTY(int treeStepSize);
    QDOC_PROPERTY(bool rootIsDecorated);
};

struct QStyleOptionDockWindow : public QStyleOption
{
    enum { Type = SO_DockWindow };
    bool docked;
    bool isCloseEnabled;
    QStyleOptionDockWindow(int version) : QStyleOption(version , SO_DockWindow),
    docked(false), isCloseEnabled(false) {}
    QDOC_PROPERTY(bool docked);
    QDOC_PROPERTY(bool isCloseEnabled);
};

struct QStyleOptionToolButton : public QStyleOptionComplex
{
    enum { Type = SO_ToolButton };
    enum ToolButtonFeature { None = 0x00, Arrow = 0x01, TextLabel = 0x02, Menu = 0x04,
                             PopupDelay = 0x08, BigPixmap = 0x10 };
    Q_DECLARE_FLAGS(ToolButtonFeatures, ToolButtonFeature);
    ToolButtonFeatures features;
    QIconSet icon;
    QString text;
    Qt::ArrowType arrowType;
    QPalette::ColorRole bgRole;
    QPalette::ColorRole parentBGRole;
    QPalette parentPalette;
    QPoint pos;
    QFont font;
    QToolButton::TextPosition textPosition;
    QStyleOptionToolButton(int version)
        : QStyleOptionComplex(version, SO_ToolButton), features(None), arrowType(Qt::DownArrow),
          textPosition(QToolButton::BesideIcon) {}
    QDOC_PROPERTY(ToolButtonFeatures features);
    QDOC_PROPERTY(QIconSet icon);
    QDOC_PROPERTY(QString text);
    QDOC_PROPERTY(Qt::ArrowType arrowType);
    QDOC_PROPERTY(QPalette::ColorRole bgRole);
    QDOC_PROPERTY(QPalette::ColorRole parentBGRole);
    QDOC_PROPERTY(QPalette parentPalette);
    QDOC_PROPERTY(QPoint pos);
    QDOC_PROPERTY(QFont font);
    QDOC_PROPERTY(QToolButton::TextPosition textPosition);
};

struct QStyleOptionComboBox : public QStyleOptionComplex
{
    enum { Type = SO_ComboBox };
    bool editable;
    QRect popupRect;
    QStyleOptionComboBox(int version)
        : QStyleOptionComplex(version, SO_ComboBox), editable(false) {}
    QDOC_PROPERTY(bool editable);
    QDOC_PROPERTY(QRect popupRect);
};

struct QStyleOptionToolBox : public QStyleOption
{
    enum { Type = SO_ToolBox };
    QString text;
    QIconSet icon;
    QPalette::ColorRole bgRole;
    QPalette::ColorRole currentWidgetBGRole;
    QPalette currentWidgetPalette;
    QStyleOptionToolBox(int version)
        : QStyleOption(version, SO_ToolBox), bgRole(QPalette::Foreground) {};
    QDOC_PROPERTY(QString text);
    QDOC_PROPERTY(QIconSet icon);
    QDOC_PROPERTY(QPalette::ColorRole bgRole);
    QDOC_PROPERTY(QPalette::ColorRole currentWidgetBGRole);
    QDOC_PROPERTY(QPalette currentWidgetPalette);
};

struct QStyleOptionTitleBar : public QStyleOptionComplex
{
    enum { Type = SO_TitleBar };
    QString text;
    QPixmap icon;
    int titleBarState;
    Qt::WFlags titleBarFlags;
    QStyleOptionTitleBar(int version)
        : QStyleOptionComplex(version, SO_TitleBar), titleBarState(0), titleBarFlags(0) {};
    QDOC_PROPERTY(QString text);
    QDOC_PROPERTY(QPixmap icon);
    QDOC_PROPERTY(int titleBarState);
    QDOC_PROPERTY(Qt::WFlags titleBarFlags);
};

struct QStyleOptionViewItem : public QStyleOption
{
    enum { Type = SO_ViewItem };
    enum Position { Left, Right, Top, Bottom };
    enum Size { Small, Large };
    int displayAlignment;
    int decorationAlignment;
    Position decorationPosition;
    Size decorationSize;
    QStyleOptionViewItem(int version) : QStyleOption(version, SO_ViewItem),
    displayAlignment(0), decorationAlignment(0), decorationPosition(Left), decorationSize(Small) {}
    QDOC_PROPERTY(int displayAlignment);
    QDOC_PROPERTY(int decorationAlignment);
    QDOC_PROPERTY(Position decorationPosition);
    QDOC_PROPERTY(Size decorationSize);
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
