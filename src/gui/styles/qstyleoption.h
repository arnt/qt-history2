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
#ifndef QSTYLEOPTION_H
#define QSTYLEOPTION_H

#include "qabstractspinbox.h"
#include "qiconset.h"
#include "qslider.h"
#include "qstyle.h"
#include "qtabbar.h"
#include "qtoolbutton.h"

class Q_GUI_EXPORT QStyleOption
{
public:
    enum OptionType {
                      // Standard controls
                      SO_Default, SO_FocusRect, SO_Button, SO_Tab, SO_MenuItem,
                      SO_Frame, SO_ProgressBar, SO_ToolBox, SO_Header, SO_DockWindow,
                      SO_ListViewItem, SO_ViewItem,

                      // Complex controls
                      SO_Complex = 0xf000, SO_Slider, SO_SpinBox, SO_ToolButton, SO_ComboBox,
                      SO_ListView, SO_TitleBar,

                      // base for custom standard controls
                      SO_CustomBase = 0xf00000,
                      // base for custom complex controls
                      SO_ComplexCustomBase = 0xf000000
                    };
    enum { Type = SO_Default };

    int version;
    int type;
    QStyle::StyleFlags state;
    QRect rect;
    QPalette palette;

    QStyleOption(int optionversion, int optiontype = SO_Default);
    ~QStyleOption();

    void init(const QWidget *w);

    QDOC_PROPERTY(int version);
    QDOC_PROPERTY(int type);
    QDOC_PROPERTY(QStyle::SFlags state);
    QDOC_PROPERTY(QRect rect);
    QDOC_PROPERTY(QPalette palette);
};

class QStyleOptionFocusRect  : public QStyleOption
{
public:
    enum { Type = SO_FocusRect };

    QColor backgroundColor;

    QStyleOptionFocusRect(int version);

    QDOC_PROPERTY(QColor backgroundColor);
};

class QStyleOptionFrame : public QStyleOption
{
public:
    enum { Type = SO_Frame };

    int lineWidth;
    int midLineWidth;

    QStyleOptionFrame(int version);

    QDOC_PROPERTY(int lineWidth);
    QDOC_PROPERTY(int midLineWidth);
};

class QStyleOptionHeader : public QStyleOption
{
public:
    enum { Type = SO_Header };

    int section;
    QString text;
    QIconSet icon;

    QStyleOptionHeader(int version);

    QDOC_PROPERTY(int section);
    QDOC_PROPERTY(QString text);
    QDOC_PROPERTY(QIconSet icon);
};

class QStyleOptionButton : public QStyleOption
{
public:
    enum { Type = SO_Button };
    enum ButtonFeature { None = 0x00, Flat = 0x01, HasMenu = 0x02 };
    Q_DECLARE_FLAGS(ButtonFeatures, ButtonFeature);

    ButtonFeatures features;
    QString text;
    QIconSet icon;

    QStyleOptionButton(int version);

    QDOC_PROPERTY(ButtonFeatures features);
    QDOC_PROPERTY(QString text);
    QDOC_PROPERTY(QIconSet icon);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionButton::ButtonFeatures);

class QStyleOptionTab : public QStyleOption
{
public:
    enum { Type = SO_Tab };
    enum TabPosition { Beginning, Middle, End, OnlyOneTab };

    QTabBar::Shape shape;
    QString text;
    QIconSet icon;
    int row;
    TabPosition position;

    QStyleOptionTab(int version);

    QDOC_PROPERTY(QTabBar::Shape shape);
    QDOC_PROPERTY(QString text);
    QDOC_PROPERTY(QIconSet icon);
    QDOC_PROPERTY(int row);
    QDOC_PROPERTY(TabPosition position);
};

class QStyleOptionProgressBar : public QStyleOption
{
public:
    enum { Type = SO_ProgressBar };
    enum ProgressBarFeature { None, CenterIndicator = 0x01, PercentageVisible = 0x02,
                              IndicatorFollowsStyle = 0x03 };
    Q_DECLARE_FLAGS(ProgressBarFeatures, ProgressBarFeature);

    ProgressBarFeatures features;
    QString progressString;
    int totalSteps;
    int progress;

    QStyleOptionProgressBar(int version);

    QDOC_PROPERTY(ProgressBarFeatures features);
    QDOC_PROPERTY(QString progressString);
    QDOC_PROPERTY(int totalSteps);
    QDOC_PROPERTY(int progress);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionProgressBar::ProgressBarFeatures);

class QStyleOptionMenuItem : public QStyleOption
{
public:
    enum { Type = SO_MenuItem };
    enum MenuItemType { Normal, Separator, SubMenu, Scroller, TearOff, Margin,
			EmptyArea };
    enum CheckState { NotCheckable, Checked, Unchecked };

    MenuItemType menuItemType;
    CheckState checkState;
    QRect menuRect;
    QString text;
    QIconSet icon;
    int maxIconWidth;
    int tabWidth;
    QFont font;

    QStyleOptionMenuItem(int version);

    QDOC_PROPERTY(MenuItemType menuItemType);
    QDOC_PROPERTY(CheckState checkState);
    QDOC_PROPERTY(QRect menuRect);
    QDOC_PROPERTY(QString text);
    QDOC_PROPERTY(QIconSet icon);
    QDOC_PROPERTY(int maxIconWidth);
    QDOC_PROPERTY(int tabWidth);
    QDOC_PROPERTY(QFont font);
};

class QStyleOptionListViewItem : public QStyleOption
{
public:
    enum { Type = SO_ListViewItem };
    enum ListViewItemFeature { None = 0x00, Expandable = 0x01, MultiLine = 0x02, Visible = 0x04,
                               ParentControl = 0x08 };
    Q_DECLARE_FLAGS(ListViewItemFeatures, ListViewItemFeature);

    ListViewItemFeatures features;
    int height;
    int totalHeight;
    int itemY;
    int childCount;

    QStyleOptionListViewItem(int version);

    QDOC_PROPERTY(int height);
    QDOC_PROPERTY(int totalHeight);
    QDOC_PROPERTY(int itemY);
    QDOC_PROPERTY(int childCount);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionListViewItem::ListViewItemFeatures);

class QStyleOptionDockWindow : public QStyleOption
{
public:
    enum { Type = SO_DockWindow };

    bool docked;
    bool isCloseEnabled;

    QStyleOptionDockWindow(int version);

    QDOC_PROPERTY(bool docked);
    QDOC_PROPERTY(bool isCloseEnabled);
};

class QStyleOptionViewItem : public QStyleOption
{
public:
    enum { Type = SO_ViewItem };
    enum Position { Left, Right, Top, Bottom };
    enum Size { Small, Large };

    int displayAlignment;
    int decorationAlignment;
    Position decorationPosition;
    Size decorationSize;
    QFont font;

    QStyleOptionViewItem(int version);

    QDOC_PROPERTY(int displayAlignment);
    QDOC_PROPERTY(int decorationAlignment);
    QDOC_PROPERTY(Position decorationPosition);
    QDOC_PROPERTY(Size decorationSize);
};

class QStyleOptionToolBox : public QStyleOption
{
public:
    enum { Type = SO_ToolBox };

    QString text;
    QIconSet icon;
    QPalette::ColorRole bgRole;
    QPalette::ColorRole currentWidgetBGRole;
    QPalette currentWidgetPalette;

    QStyleOptionToolBox(int version);

    QDOC_PROPERTY(QString text);
    QDOC_PROPERTY(QIconSet icon);
    QDOC_PROPERTY(QPalette::ColorRole bgRole);
    QDOC_PROPERTY(QPalette::ColorRole currentWidgetBGRole);
    QDOC_PROPERTY(QPalette currentWidgetPalette);
};

// -------------------------- Complex style options -------------------------------
class QStyleOptionComplex : public QStyleOption
{
public:
    enum { Type = SO_Complex };

    QStyle::SubControls parts;
    QStyle::SubControls activeParts;

    QStyleOptionComplex(int version, int type = SO_Complex);

    QDOC_PROPERTY(QStyle::SubControls parts);
    QDOC_PROPERTY(QStyle::SubControls activeParts);
};

class QStyleOptionSlider : public QStyleOptionComplex
{
public:
    enum { Type = SO_Slider };

    Qt::Orientation orientation;
    int minimum;
    int maximum;
    QSlider::TickSetting tickmarks;
    int tickInterval;
    bool useRightToLeft;
    int sliderPosition;
    int sliderValue;
    int singleStep;
    int pageStep;

    QStyleOptionSlider(int version);

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

class QStyleOptionSpinBox : public QStyleOptionComplex
{
public:
    enum { Type = SO_SpinBox };

    QAbstractSpinBox::ButtonSymbols buttonSymbols;
    QAbstractSpinBox::StepEnabled stepEnabled;
    double percentage;
    bool slider;
    bool frame;

    QStyleOptionSpinBox(int version);

    QDOC_PROPERTY(QAbstractSpinBox::ButtonSymbols buttonSymbols);
    QDOC_PROPERTY(QAbstractSpinBox::StepEnabled stepEnabled);
    QDOC_PROPERTY(double percentage);
    QDOC_PROPERTY(bool slider);
    QDOC_PROPERTY(bool frame);
};

class QStyleOptionListView : public QStyleOptionComplex
{
public:
    enum { Type = SO_ListView };

    QList<QStyleOptionListViewItem> items;
    QPalette viewportPalette;
    QPalette::ColorRole viewportBGRole;
    int sortColumn;
    int itemMargin;
    int treeStepSize;
    bool rootIsDecorated;

    QStyleOptionListView(int version);

    QDOC_PROPERTY(QList<QStyleOptionListViewItem> items);
    QDOC_PROPERTY(QPalette viewportPalette);
    QDOC_PROPERTY(QPalette::ColorRole viewportBGRole);
    QDOC_PROPERTY(int sortColumn);
    QDOC_PROPERTY(int itemMargin);
    QDOC_PROPERTY(int treeStepSize);
    QDOC_PROPERTY(bool rootIsDecorated);
};

class QStyleOptionToolButton : public QStyleOptionComplex
{
public:
    enum { Type = SO_ToolButton };
    enum ToolButtonFeature { None = 0x00, Arrow = 0x01, TextLabel = 0x02, Menu = 0x04,
                             PopupDelay = 0x08, BigPixmap = 0x10 };
    Q_DECLARE_FLAGS(ToolButtonFeatures, ToolButtonFeature);

    ToolButtonFeatures features;
    QIconSet icon;
    QString text;
    Qt::ArrowType arrowType;
    QPalette::ColorRole bgRole;
    QPoint pos;
    QFont font;
    QToolButton::TextPosition textPosition;

    QStyleOptionToolButton(int version);

    QDOC_PROPERTY(ToolButtonFeatures features);
    QDOC_PROPERTY(QIconSet icon);
    QDOC_PROPERTY(QString text);
    QDOC_PROPERTY(Qt::ArrowType arrowType);
    QDOC_PROPERTY(QPalette::ColorRole bgRole);
    QDOC_PROPERTY(QPoint pos);
    QDOC_PROPERTY(QFont font);
    QDOC_PROPERTY(QToolButton::TextPosition textPosition);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionToolButton::ToolButtonFeatures);

class QStyleOptionComboBox : public QStyleOptionComplex
{
public:
    enum { Type = SO_ComboBox };

    bool editable;
    QRect popupRect;

    QStyleOptionComboBox(int version);

    QDOC_PROPERTY(bool editable);
    QDOC_PROPERTY(QRect popupRect);
};

class QStyleOptionTitleBar : public QStyleOptionComplex
{
public:
    enum { Type = SO_TitleBar };

    QString text;
    QPixmap icon;
    int titleBarState;
    Qt::WFlags titleBarFlags;

    QStyleOptionTitleBar(int version);

    QDOC_PROPERTY(QString text);
    QDOC_PROPERTY(QPixmap icon);
    QDOC_PROPERTY(int titleBarState);
    QDOC_PROPERTY(Qt::WFlags titleBarFlags);
};

template <typename T>
T qt_cast(const QStyleOption *opt)
{
    if (opt && opt->type == static_cast<T>(0)->Type
        || int(static_cast<T>(0)->Type) == QStyleOption::SO_Default
        || (int(static_cast<T>(0)->Type) == QStyleOption::SO_Complex
            && ((opt->type > QStyleOption::SO_Complex && opt->type < QStyleOption::SO_CustomBase)
                || opt->type >= QStyleOption::SO_ComplexCustomBase)))
        return static_cast<T>(opt);
    return 0;
}

template <typename T>
T qt_cast(QStyleOption *opt)
{
    if (opt && opt->type == static_cast<T>(0)->Type
        || int(static_cast<T>(0)->Type) == QStyleOption::SO_Default
        || (int(static_cast<T>(0)->Type) == QStyleOption::SO_Complex
            && ((opt->type > QStyleOption::SO_Complex && opt->type < QStyleOption::SO_CustomBase)
                || opt->type >= QStyleOption::SO_ComplexCustomBase)))
        return static_cast<T>(opt);
    return 0;
}
#endif
