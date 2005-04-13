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

#include "QtGui/qabstractspinbox.h"
#include "QtGui/qicon.h"
#include "QtGui/qslider.h"
#include "QtGui/qstyle.h"
#include "QtGui/qtabbar.h"
#include "QtGui/qtabwidget.h"
#include "QtGui/qrubberband.h"

class Q_GUI_EXPORT QStyleOption
{
public:
    enum OptionType {
                      SO_Default, SO_FocusRect, SO_Button, SO_Tab, SO_MenuItem,
                      SO_Frame, SO_ProgressBar, SO_ToolBox, SO_Header, SO_Q3DockWindow,
                      SO_DockWidget, SO_Q3ListViewItem, SO_ViewItem, SO_TabWidgetFrame,
                      SO_TabBarBase, SO_RubberBand,

                      SO_Complex = 0xf0000, SO_Slider, SO_SpinBox, SO_ToolButton, SO_ComboBox,
                      SO_Q3ListView, SO_TitleBar,

                      SO_CustomBase = 0xf00,
                      SO_ComplexCustomBase = 0xf000000
                    };

    enum { Type = SO_Default };
    enum { Version = 1 };

    int version;
    int type;
    QStyle::State state;
    Qt::LayoutDirection direction;
    QRect rect;
    QFontMetrics fontMetrics;
    QPalette palette;

    QStyleOption(int version = QStyleOption::Version, int type = SO_Default);
    QStyleOption(const QStyleOption &other);
    ~QStyleOption();

    void init(const QWidget *w);
    QStyleOption &operator=(const QStyleOption &other);

    QDOC_PROPERTY(int version)
    QDOC_PROPERTY(int type)
    QDOC_PROPERTY(QStyle::State state)
    QDOC_PROPERTY(QRect rect)
    QDOC_PROPERTY(QPalette palette)

};

class Q_GUI_EXPORT QStyleOptionFocusRect  : public QStyleOption
{
public:
    enum { Type = SO_FocusRect };
    enum { Version = 1 };

    QColor backgroundColor;

    QStyleOptionFocusRect();
    QStyleOptionFocusRect(const QStyleOptionFocusRect &other) : QStyleOption(Version, Type) { *this = other; }

    QDOC_PROPERTY(QColor backgroundColor)

protected:
    QStyleOptionFocusRect(int version);
};

class Q_GUI_EXPORT QStyleOptionFrame : public QStyleOption
{
public:
    enum { Type = SO_Frame };
    enum { Version = 1 };

    int lineWidth;
    int midLineWidth;

    QStyleOptionFrame();
    QStyleOptionFrame(const QStyleOptionFrame &other) : QStyleOption(Version, Type) { *this = other; }

    QDOC_PROPERTY(int lineWidth)
    QDOC_PROPERTY(int midLineWidth)

protected:
    QStyleOptionFrame(int version);
};

class Q_GUI_EXPORT QStyleOptionTabWidgetFrame : public QStyleOption
{
public:
    enum { Type = SO_TabWidgetFrame };
    enum { Version = 1 };

    int lineWidth;
    int midLineWidth;
    QTabBar::Shape shape;
    QSize tabBarSize;
    QSize rightCornerWidgetSize;
    QSize leftCornerWidgetSize;

    QStyleOptionTabWidgetFrame();
    QStyleOptionTabWidgetFrame(const QStyleOptionTabWidgetFrame &other) : QStyleOption(Version, Type) { *this = other; }

    QDOC_PROPERTY(int lineWidth)
    QDOC_PROPERTY(int midLineWidth)

protected:
    QStyleOptionTabWidgetFrame(int version);
};

class Q_GUI_EXPORT QStyleOptionTabBarBase : public QStyleOption
{
public:
    enum { Type = SO_TabBarBase };
    enum { Version = 1 };

    QTabBar::Shape shape;
    QRect tabBarRect;
    QRect selectedTabRect;

    QStyleOptionTabBarBase();
    QStyleOptionTabBarBase(const QStyleOptionTabBarBase &other) : QStyleOption(Version, Type) { *this = other; }

protected:
    QStyleOptionTabBarBase(int version);
};

class Q_GUI_EXPORT QStyleOptionHeader : public QStyleOption
{
public:
    enum { Type = SO_Header };
    enum { Version = 1 };

    enum SectionPosition { Beginning, Middle, End, OnlyOneSection };
    enum SelectedPosition { NotAdjacent, NextIsSelected, PreviousIsSelected,
                            NextAndPreviousAreSelected };
    enum SortIndicator { None, SortUp, SortDown };

    int section;
    QString text;
    Qt::Alignment textAlignment;
    QIcon icon;
    Qt::Alignment iconAlignment;
    SectionPosition position;
    SelectedPosition selectedPosition;
    SortIndicator sortIndicator;

    QStyleOptionHeader();
    QStyleOptionHeader(const QStyleOptionHeader &other) : QStyleOption(Version, Type) { *this = other; }

    QDOC_PROPERTY(int section)
    QDOC_PROPERTY(QString text)
    QDOC_PROPERTY(Qt::Alignment textAlignment)
    QDOC_PROPERTY(QIcon icon)
    QDOC_PROPERTY(Qt::Alignment iconAlignment)

protected:
    QStyleOptionHeader(int version);
};

class Q_GUI_EXPORT QStyleOptionButton : public QStyleOption
{
public:
    enum { Type = SO_Button };
    enum { Version = 1 };

    enum ButtonFeature { None = 0x00, Flat = 0x01, HasMenu = 0x02, DefaultButton = 0x04,
                         AutoDefaultButton = 0x08 };
    Q_DECLARE_FLAGS(ButtonFeatures, ButtonFeature)

    ButtonFeatures features;
    QString text;
    QIcon icon;

    QStyleOptionButton();
    QStyleOptionButton(const QStyleOptionButton &other) : QStyleOption(Version, Type) { *this = other; }

    QDOC_PROPERTY(ButtonFeatures features)
    QDOC_PROPERTY(QString text)
    QDOC_PROPERTY(QIcon icon)

protected:
    QStyleOptionButton(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionButton::ButtonFeatures);

class Q_GUI_EXPORT QStyleOptionTab : public QStyleOption
{
public:
    enum { Type = SO_Tab };
    enum { Version = 1 };

    enum TabPosition { Beginning, Middle, End, OnlyOneTab };
    enum SelectedPosition { NotAdjacent, NextIsSelected, PreviousIsSelected };
    enum CornerWidget { NoCornerWidgets = 0x00, LeftCornerWidget = 0x01,
                        RightCornerWidget = 0x02 };
    Q_DECLARE_FLAGS(CornerWidgets, CornerWidget)

    QTabBar::Shape shape;
    QString text;
    QIcon icon;
    int row;
    TabPosition position;
    SelectedPosition selectedPosition;
    CornerWidgets cornerWidgets;

    QStyleOptionTab();
    QStyleOptionTab(const QStyleOptionTab &other) : QStyleOption(Version, Type) { *this = other; }

    QDOC_PROPERTY(QTabBar::Shape shape)
    QDOC_PROPERTY(QString text)
    QDOC_PROPERTY(QIcon icon)
    QDOC_PROPERTY(int row)
    QDOC_PROPERTY(TabPosition position)
    QDOC_PROPERTY(SelectedPosition selectedPosition)
    QDOC_PROPERTY(CornerWidgets cornerWidgets)

protected:
    QStyleOptionTab(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionTab::CornerWidgets);

class Q_GUI_EXPORT QStyleOptionProgressBar : public QStyleOption
{
public:
    enum { Type = SO_ProgressBar };
    enum { Version = 1 };

    int minimum;
    int maximum;
    int progress;

    QString text;
    Qt::Alignment textAlignment;
    bool textVisible;


    QStyleOptionProgressBar();
    QStyleOptionProgressBar(const QStyleOptionProgressBar &other) : QStyleOption(Version, Type) { *this = other; }

    QDOC_PROPERTY(int minimum)
    QDOC_PROPERTY(int maximum)
    QDOC_PROPERTY(int progress)
    QDOC_PROPERTY(QString text)
    QDOC_PROPERTY(Qt::Alignment textAlignment)
    QDOC_PROPERTY(bool textVisible)

protected:
    QStyleOptionProgressBar(int version);
};

class Q_GUI_EXPORT QStyleOptionMenuItem : public QStyleOption
{
public:
    enum { Type = SO_MenuItem };
    enum { Version = 1 };

    enum MenuItemType { Normal, DefaultItem, Separator, SubMenu, Scroller, TearOff, Margin,
                        EmptyArea };
    enum CheckType { NotCheckable, Exclusive, NonExclusive };

    MenuItemType menuItemType;
    CheckType checkType;
    bool checked;
    bool menuHasCheckableItems;
    QRect menuRect;
    QString text;
    QIcon icon;
    int maxIconWidth;
    int tabWidth;
    QFont font;

    QStyleOptionMenuItem();
    QStyleOptionMenuItem(const QStyleOptionMenuItem &other) : QStyleOption(Version, Type) { *this = other; }

    QDOC_PROPERTY(MenuItemType menuItemType)
    QDOC_PROPERTY(CheckType checkType)
    QDOC_PROPERTY(bool checked)
    QDOC_PROPERTY(bool menuHasCheckableItems)
    QDOC_PROPERTY(QRect menuRect)
    QDOC_PROPERTY(QString text)
    QDOC_PROPERTY(QIcon icon)
    QDOC_PROPERTY(int maxIconWidth)
    QDOC_PROPERTY(int tabWidth)
    QDOC_PROPERTY(QFont font)

protected:
    QStyleOptionMenuItem(int version);
};

class Q_GUI_EXPORT QStyleOptionQ3ListViewItem : public QStyleOption
{
public:
    enum { Type = SO_Q3ListViewItem };
    enum { Version = 1 };

    enum Q3ListViewItemFeature { None = 0x00, Expandable = 0x01, MultiLine = 0x02, Visible = 0x04,
                                 ParentControl = 0x08 };
    Q_DECLARE_FLAGS(Q3ListViewItemFeatures, Q3ListViewItemFeature)

    Q3ListViewItemFeatures features;
    int height;
    int totalHeight;
    int itemY;
    int childCount;

    QStyleOptionQ3ListViewItem();
    QStyleOptionQ3ListViewItem(const QStyleOptionQ3ListViewItem &other) : QStyleOption(Version, Type) { *this = other; }

    QDOC_PROPERTY(ListViewItemFeatures features)
    QDOC_PROPERTY(int height)
    QDOC_PROPERTY(int totalHeight)
    QDOC_PROPERTY(int itemY)
    QDOC_PROPERTY(int childCount)

protected:
    QStyleOptionQ3ListViewItem(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionQ3ListViewItem::Q3ListViewItemFeatures);

class Q_GUI_EXPORT QStyleOptionQ3DockWindow : public QStyleOption
{
public:
    enum { Type = SO_Q3DockWindow };
    enum { Version = 1 };

    bool docked;
    bool closeEnabled;

    QStyleOptionQ3DockWindow();
    QStyleOptionQ3DockWindow(const QStyleOptionQ3DockWindow &other) : QStyleOption(Version, Type) { *this = other; }

    QDOC_PROPERTY(bool docked)
    QDOC_PROPERTY(bool closeEnabled)

protected:
    QStyleOptionQ3DockWindow(int version);
};

class Q_GUI_EXPORT QStyleOptionDockWidget : public QStyleOption
{
public:
    enum { Type = SO_DockWidget };
    enum { Version = 1 };

    QString title;
    bool closable;
    bool moveable;
    bool floatable;

    QStyleOptionDockWidget();
    QStyleOptionDockWidget(const QStyleOptionDockWidget &other) : QStyleOption(Version, Type) { *this = other; }

    QDOC_PROPERTY(QString title)
    QDOC_PROPERTY(bool closable)
    QDOC_PROPERTY(bool moveable)
    QDOC_PROPERTY(bool floatable)

protected:
    QStyleOptionDockWidget(int version);
};

class Q_GUI_EXPORT QStyleOptionViewItem : public QStyleOption
{
public:
    enum { Type = SO_ViewItem };
    enum { Version = 1 };

    enum Position { Left, Right, Top, Bottom };

    Qt::Alignment displayAlignment;
    Qt::Alignment decorationAlignment;
    Position decorationPosition;
    QSize decorationSize;
    QFont font;

    QStyleOptionViewItem();
    QStyleOptionViewItem(const QStyleOptionViewItem &other) : QStyleOption(Version, Type) { *this = other; }

    QDOC_PROPERTY(Qt::Alignment displayAlignment)
    QDOC_PROPERTY(Qt::Alignment decorationAlignment)
    QDOC_PROPERTY(Position decorationPosition)
    QDOC_PROPERTY(QSize decorationSize)
    QDOC_PROPERTY(QFont font)

protected:
    QStyleOptionViewItem(int version);
};

class Q_GUI_EXPORT QStyleOptionToolBox : public QStyleOption
{
public:
    enum { Type = SO_ToolBox };
    enum { Version = 1 };

    QString text;
    QIcon icon;

    QStyleOptionToolBox();
    QStyleOptionToolBox(const QStyleOptionToolBox &other) : QStyleOption(Version, Type) { *this = other; }

    QDOC_PROPERTY(QString text)
    QDOC_PROPERTY(QIcon icon)

protected:
    QStyleOptionToolBox(int version);
};

class Q_GUI_EXPORT QStyleOptionRubberBand : public QStyleOption
{
public:
    enum { Type = SO_RubberBand };
    enum { Version = 1 };

    QRubberBand::Shape shape;
    bool opaque;

    QStyleOptionRubberBand();
    QStyleOptionRubberBand(const QStyleOptionRubberBand &other) : QStyleOption(Version, Type) { *this = other; }

    QDOC_PROPERTY(QRubberBand::Shape shape)
    QDOC_PROPERTY(bool opaque)

protected:
    QStyleOptionRubberBand(int version);
};

// -------------------------- Complex style options -------------------------------
class Q_GUI_EXPORT QStyleOptionComplex : public QStyleOption
{
public:
    enum { Type = SO_Complex };
    enum { Version = 1 };

    QStyle::SubControls subControls;
    QStyle::SubControls activeSubControls;

    QStyleOptionComplex(int version = QStyleOptionComplex::Version, int type = SO_Complex);
    QStyleOptionComplex(const QStyleOptionComplex &other) : QStyleOption(Version, Type) { *this = other; }

    QDOC_PROPERTY(QStyle::SubControls subControls)
    QDOC_PROPERTY(QStyle::SubControls activeSubControls)
};

class Q_GUI_EXPORT QStyleOptionSlider : public QStyleOptionComplex
{
public:
    enum { Type = SO_Slider };
    enum { Version = 1 };

    Qt::Orientation orientation;
    int minimum;
    int maximum;
    QSlider::TickPosition tickPosition;
    int tickInterval;
    bool upsideDown;
    int sliderPosition;
    int sliderValue;
    int singleStep;
    int pageStep;
    qreal notchTarget;
    bool dialWrapping;

    QStyleOptionSlider();
    QStyleOptionSlider(const QStyleOptionSlider &other) : QStyleOptionComplex(Version, Type) { *this = other; }

    QDOC_PROPERTY(Qt::Orientation orientation)
    QDOC_PROPERTY(int minimum)
    QDOC_PROPERTY(int maximum)
    QDOC_PROPERTY(QSlider::TickPosition tickPosition)
    QDOC_PROPERTY(int tickInterval)
    QDOC_PROPERTY(bool upsideDown)
    QDOC_PROPERTY(int sliderPosition)
    QDOC_PROPERTY(int sliderValue)
    QDOC_PROPERTY(int singleStep)
    QDOC_PROPERTY(int pageStep)
    QDOC_PROPERTY(double notchTarget)
    QDOC_PROPERTY(bool dialWrapping)

protected:
    QStyleOptionSlider(int version);
};

class Q_GUI_EXPORT QStyleOptionSpinBox : public QStyleOptionComplex
{
public:
    enum { Type = SO_SpinBox };
    enum { Version = 1 };

    QAbstractSpinBox::ButtonSymbols buttonSymbols;
    QAbstractSpinBox::StepEnabled stepEnabled;
    bool frame;

    QStyleOptionSpinBox();
    QStyleOptionSpinBox(const QStyleOptionSpinBox &other) : QStyleOptionComplex(Version, Type) { *this = other; }

    QDOC_PROPERTY(QAbstractSpinBox::ButtonSymbols buttonSymbols)
    QDOC_PROPERTY(QAbstractSpinBox::StepEnabled stepEnabled)
    QDOC_PROPERTY(bool frame)

protected:
    QStyleOptionSpinBox(int version);
};

class Q_GUI_EXPORT QStyleOptionQ3ListView : public QStyleOptionComplex
{
public:
    enum { Type = SO_Q3ListView };
    enum { Version = 1 };

    QList<QStyleOptionQ3ListViewItem> items;
    QPalette viewportPalette;
    QPalette::ColorRole viewportBGRole;
    int sortColumn;
    int itemMargin;
    int treeStepSize;
    bool rootIsDecorated;

    QStyleOptionQ3ListView();
    QStyleOptionQ3ListView(const QStyleOptionQ3ListView &other) : QStyleOptionComplex(Version, Type) { *this = other; }

    QDOC_PROPERTY(QList<QStyleOptionQ3ListViewItem> items)
    QDOC_PROPERTY(QPalette viewportPalette)
    QDOC_PROPERTY(QPalette::ColorRole viewportBGRole)
    QDOC_PROPERTY(int sortColumn)
    QDOC_PROPERTY(int itemMargin)
    QDOC_PROPERTY(int treeStepSize)
    QDOC_PROPERTY(bool rootIsDecorated)

protected:
    QStyleOptionQ3ListView(int version);
};

class Q_GUI_EXPORT QStyleOptionToolButton : public QStyleOptionComplex
{
public:
    enum { Type = SO_ToolButton };
    enum { Version = 1 };

    enum ToolButtonFeature { None = 0x00, Arrow = 0x01, Menu = 0x04, PopupDelay = 0x08 };
    Q_DECLARE_FLAGS(ToolButtonFeatures, ToolButtonFeature)

    ToolButtonFeatures features;
    QIcon icon;
    QSize iconSize;
    QString text;
    Qt::ArrowType arrowType;
    Qt::ToolButtonStyle toolButtonStyle;
    QPoint pos;
    QFont font;

    QStyleOptionToolButton();
    QStyleOptionToolButton(const QStyleOptionToolButton &other) : QStyleOptionComplex(Version, Type) { *this = other; }

    QDOC_PROPERTY(ToolButtonFeatures features)
    QDOC_PROPERTY(QIcon icon)
    QDOC_PROPERTY(QString text)
    QDOC_PROPERTY(Qt::ArrowType arrowType)
    QDOC_PROPERTY(Qt::ToolButtonStyle toolButtonStyle)
    QDOC_PROPERTY(QPoint pos)
    QDOC_PROPERTY(QFont font)
    QDOC_PROPERTY(QSize iconSize)

protected:
    QStyleOptionToolButton(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionToolButton::ToolButtonFeatures);

class Q_GUI_EXPORT QStyleOptionComboBox : public QStyleOptionComplex
{
public:
    enum { Type = SO_ComboBox };
    enum { Version = 1 };

    bool editable;
    QRect popupRect;
    bool frame;
    QString currentText;
    QIcon currentIcon;
    QSize iconSize;

    QStyleOptionComboBox();
    QStyleOptionComboBox(const QStyleOptionComboBox &other) : QStyleOptionComplex(Version, Type) { *this = other; }

    QDOC_PROPERTY(bool editable)
    QDOC_PROPERTY(QRect popupRect)
    QDOC_PROPERTY(bool frame)
    QDOC_PROPERTY(QString currentText)
    QDOC_PROPERTY(QIcon currentIcon)
    QDOC_PROPERTY(QSize iconSize)

protected:
    QStyleOptionComboBox(int version);
};

class Q_GUI_EXPORT QStyleOptionTitleBar : public QStyleOptionComplex
{
public:
    enum { Type = SO_TitleBar };
    enum { Version = 1 };

    QString text;
    QIcon icon;
    int titleBarState;
    Qt::WFlags titleBarFlags;

    QStyleOptionTitleBar();
    QStyleOptionTitleBar(const QStyleOptionTitleBar &other) : QStyleOptionComplex(Version, Type) { *this = other; }

    QDOC_PROPERTY(QString text)
    QDOC_PROPERTY(QIcon icon)
    QDOC_PROPERTY(int titleBarState)
    QDOC_PROPERTY(Qt::WFlags titleBarFlags)

protected:
    QStyleOptionTitleBar(int version);
};

template <typename T>
T qstyleoption_cast(const QStyleOption *opt)
{
    if (opt && opt->version <= static_cast<T>(0)->Version && (opt->type == static_cast<T>(0)->Type
        || int(static_cast<T>(0)->Type) == QStyleOption::SO_Default
        || (int(static_cast<T>(0)->Type) == QStyleOption::SO_Complex
            && opt->type > QStyleOption::SO_Complex)))
        return static_cast<T>(opt);
    return 0;
}

template <typename T>
T qstyleoption_cast(QStyleOption *opt)
{
    if (opt && opt->version <= static_cast<T>(0)->Version && (opt->type == static_cast<T>(0)->Type
        || int(static_cast<T>(0)->Type) == QStyleOption::SO_Default
        || (int(static_cast<T>(0)->Type) == QStyleOption::SO_Complex
            && opt->type > QStyleOption::SO_Complex)))
        return static_cast<T>(opt);
    return 0;
}

// -------------------------- QStyleHintReturn -------------------------------
class Q_GUI_EXPORT QStyleHintReturn {
public:
    enum HintReturnType {
        SH_Default=0xf000, SH_Mask
    };

    enum { Type = SH_Default };
    enum { Version = 1 };

    QStyleHintReturn(int version = QStyleOption::Version, int type = SH_Default);
    ~QStyleHintReturn();

    int version;
    int type;
};

class Q_GUI_EXPORT QStyleHintReturnMask : public QStyleHintReturn {
public:
    enum { Type = SH_Mask };
    enum { Version = 1 };

    QStyleHintReturnMask();

    QRegion region;
};

template <typename T>
T qstyleoption_cast(const QStyleHintReturn *hint)
{
    if (hint && hint->version <= static_cast<T>(0)->Version &&
        (hint->type == static_cast<T>(0)->Type || int(static_cast<T>(0)->Type) == QStyleHintReturn::SH_Default))
        return static_cast<T>(hint);
    return 0;
}

template <typename T>
T qstyleoption_cast(QStyleHintReturn *hint)
{
    if (hint && hint->version <= static_cast<T>(0)->Version &&
        (hint->type == static_cast<T>(0)->Type || int(static_cast<T>(0)->Type) == QStyleHintReturn::SH_Default))
        return static_cast<T>(hint);
    return 0;
}

#endif
