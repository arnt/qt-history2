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

#include <QtGui/qabstractspinbox.h>
#include <QtGui/qicon.h>
#include <QtGui/qslider.h>
#include <QtGui/qstyle.h>
#include <QtGui/qtabbar.h>
#include <QtGui/qtabwidget.h>
#include <QtGui/qrubberband.h>

QT_MODULE(Gui)

class QDebug;

class Q_GUI_EXPORT QStyleOption
{
public:
    enum OptionType {
                      SO_Default, SO_FocusRect, SO_Button, SO_Tab, SO_MenuItem,
                      SO_Frame, SO_ProgressBar, SO_ToolBox, SO_Header, SO_Q3DockWindow,
                      SO_DockWidget, SO_Q3ListViewItem, SO_ViewItem, SO_TabWidgetFrame,
                      SO_TabBarBase, SO_RubberBand, SO_ToolBar,

                      SO_Complex = 0xf0000, SO_Slider, SO_SpinBox, SO_ToolButton, SO_ComboBox,
                      SO_Q3ListView, SO_TitleBar, SO_GroupBox, SO_SizeGrip,

                      SO_CustomBase = 0xf00,
                      SO_ComplexCustomBase = 0xf000000
                    };

    enum StyleOptionType { Type = SO_Default };
    enum StyleOptionVersion { Version = 1 };

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
    inline void initFrom(const QWidget *w) { init(w); }
    QStyleOption &operator=(const QStyleOption &other);
};

class Q_GUI_EXPORT QStyleOptionFocusRect : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_FocusRect };
    enum StyleOptionVersion { Version = 1 };

    QColor backgroundColor;

    QStyleOptionFocusRect();
    QStyleOptionFocusRect(const QStyleOptionFocusRect &other) : QStyleOption(Version, Type) { *this = other; }

protected:
    QStyleOptionFocusRect(int version);
};

class Q_GUI_EXPORT QStyleOptionFrame : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_Frame };
    enum StyleOptionVersion { Version = 1 };

    int lineWidth;
    int midLineWidth;

    QStyleOptionFrame();
    QStyleOptionFrame(const QStyleOptionFrame &other) : QStyleOption(Version, Type) { *this = other; }

protected:
    QStyleOptionFrame(int version);
};

class Q_GUI_EXPORT QStyleOptionFrameV2 : public QStyleOptionFrame
{
public:
    enum StyleOptionVersion { Version = 2 };
    enum FrameFeature {
        None = 0x00,
        Flat = 0x01
    };
    Q_DECLARE_FLAGS(FrameFeatures, FrameFeature)
    FrameFeatures features;

    QStyleOptionFrameV2();
    QStyleOptionFrameV2(const QStyleOptionFrameV2 &other) : QStyleOptionFrame(Version) { *this = other; }
    QStyleOptionFrameV2(const QStyleOptionFrame &other);
    QStyleOptionFrameV2 &operator=(const QStyleOptionFrame &other);

protected:
    QStyleOptionFrameV2(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionFrameV2::FrameFeatures)

#ifndef QT_NO_TABWIDGET
class Q_GUI_EXPORT QStyleOptionTabWidgetFrame : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_TabWidgetFrame };
    enum StyleOptionVersion { Version = 1 };

    int lineWidth;
    int midLineWidth;
    QTabBar::Shape shape;
    QSize tabBarSize;
    QSize rightCornerWidgetSize;
    QSize leftCornerWidgetSize;

    QStyleOptionTabWidgetFrame();
    inline QStyleOptionTabWidgetFrame(const QStyleOptionTabWidgetFrame &other)
        : QStyleOption(Version, Type) { *this = other; }

protected:
    QStyleOptionTabWidgetFrame(int version);
};
#endif

#ifndef QT_NO_TABBAR
class Q_GUI_EXPORT QStyleOptionTabBarBase : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_TabBarBase };
    enum StyleOptionVersion { Version = 1 };

    QTabBar::Shape shape;
    QRect tabBarRect;
    QRect selectedTabRect;

    QStyleOptionTabBarBase();
    QStyleOptionTabBarBase(const QStyleOptionTabBarBase &other) : QStyleOption(Version, Type) { *this = other; }

protected:
    QStyleOptionTabBarBase(int version);
};
#endif

class Q_GUI_EXPORT QStyleOptionHeader : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_Header };
    enum StyleOptionVersion { Version = 1 };

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
    Qt::Orientation orientation;

    QStyleOptionHeader();
    QStyleOptionHeader(const QStyleOptionHeader &other) : QStyleOption(Version, Type) { *this = other; }

protected:
    QStyleOptionHeader(int version);
};

class Q_GUI_EXPORT QStyleOptionButton : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_Button };
    enum StyleOptionVersion { Version = 1 };

    enum ButtonFeature { None = 0x00, Flat = 0x01, HasMenu = 0x02, DefaultButton = 0x04,
                         AutoDefaultButton = 0x08 };
    Q_DECLARE_FLAGS(ButtonFeatures, ButtonFeature)

    ButtonFeatures features;
    QString text;
    QIcon icon;
    QSize iconSize;

    QStyleOptionButton();
    QStyleOptionButton(const QStyleOptionButton &other) : QStyleOption(Version, Type) { *this = other; }

protected:
    QStyleOptionButton(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionButton::ButtonFeatures)

#ifndef QT_NO_TABBAR
class Q_GUI_EXPORT QStyleOptionTab : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_Tab };
    enum StyleOptionVersion { Version = 1 };

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

protected:
    QStyleOptionTab(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionTab::CornerWidgets)

class Q_GUI_EXPORT QStyleOptionTabV2 : public QStyleOptionTab
{
public:
    enum StyleOptionVersion { Version = 2 };
    QSize iconSize;
    QStyleOptionTabV2();
    QStyleOptionTabV2(const QStyleOptionTabV2 &other) : QStyleOptionTab(Version) { *this = other; }
    QStyleOptionTabV2(const QStyleOptionTab &other);
    QStyleOptionTabV2 &operator=(const QStyleOptionTab &other);

protected:
    QStyleOptionTabV2(int version);
};

#endif


#ifndef QT_NO_TOOLBAR

class Q_GUI_EXPORT QStyleOptionToolBar : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_ToolBar };
    enum StyleOptionVersion { Version = 1 };
    enum ToolBarPosition { Beginning, Middle, End, OnlyOne };
    enum ToolBarFeature { None = 0x0, Movable = 0x1 };
    Q_DECLARE_FLAGS(ToolBarFeatures, ToolBarFeature)
    ToolBarPosition positionOfLine; // The toolbar line position
    ToolBarPosition positionWithinLine; // The position within a toolbar
    Qt::ToolBarArea toolBarArea; // The toolbar docking area
    ToolBarFeatures features;
    int lineWidth;
    int midLineWidth;
    QStyleOptionToolBar();
    QStyleOptionToolBar(const QStyleOptionToolBar &other) : QStyleOption(Version, Type) { *this = other; }

protected:
    QStyleOptionToolBar(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionToolBar::ToolBarFeatures)

#endif



class Q_GUI_EXPORT QStyleOptionProgressBar : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_ProgressBar };
    enum StyleOptionVersion { Version = 1 };

    int minimum;
    int maximum;
    int progress;
    QString text;
    Qt::Alignment textAlignment;
    bool textVisible;

    QStyleOptionProgressBar();
    QStyleOptionProgressBar(const QStyleOptionProgressBar &other) : QStyleOption(Version, Type) { *this = other; }

protected:
    QStyleOptionProgressBar(int version);
};

// Adds style info for vertical progress bars
class Q_GUI_EXPORT QStyleOptionProgressBarV2 : public QStyleOptionProgressBar
{
public:
    enum StyleOptionType { Type = SO_ProgressBar };
    enum StyleOptionVersion { Version = 2 };
    Qt::Orientation orientation;
    bool invertedAppearance;
    bool bottomToTop;

    QStyleOptionProgressBarV2();
    QStyleOptionProgressBarV2(const QStyleOptionProgressBar &other);
    QStyleOptionProgressBarV2(const QStyleOptionProgressBarV2 &other);
    QStyleOptionProgressBarV2 &operator=(const QStyleOptionProgressBar &other);

protected:
    QStyleOptionProgressBarV2(int version);
};

class Q_GUI_EXPORT QStyleOptionMenuItem : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_MenuItem };
    enum StyleOptionVersion { Version = 1 };

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

protected:
    QStyleOptionMenuItem(int version);
};

class Q_GUI_EXPORT QStyleOptionQ3ListViewItem : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_Q3ListViewItem };
    enum StyleOptionVersion { Version = 1 };

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

protected:
    QStyleOptionQ3ListViewItem(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionQ3ListViewItem::Q3ListViewItemFeatures)

class Q_GUI_EXPORT QStyleOptionQ3DockWindow : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_Q3DockWindow };
    enum StyleOptionVersion { Version = 1 };

    bool docked;
    bool closeEnabled;

    QStyleOptionQ3DockWindow();
    QStyleOptionQ3DockWindow(const QStyleOptionQ3DockWindow &other) : QStyleOption(Version, Type) { *this = other; }

protected:
    QStyleOptionQ3DockWindow(int version);
};

class Q_GUI_EXPORT QStyleOptionDockWidget : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_DockWidget };
    enum StyleOptionVersion { Version = 1 };

    QString title;
    bool closable;
    bool movable;
    bool floatable;

    QStyleOptionDockWidget();
    QStyleOptionDockWidget(const QStyleOptionDockWidget &other) : QStyleOption(Version, Type) { *this = other; }

protected:
    QStyleOptionDockWidget(int version);
};

class Q_GUI_EXPORT QStyleOptionViewItem : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_ViewItem };
    enum StyleOptionVersion { Version = 1 };

    enum Position { Left, Right, Top, Bottom };

    Qt::Alignment displayAlignment;
    Qt::Alignment decorationAlignment;
    Qt::TextElideMode textElideMode;
    Position decorationPosition;
    QSize decorationSize;
    QFont font;
    bool showDecorationSelected;

    QStyleOptionViewItem();
    QStyleOptionViewItem(const QStyleOptionViewItem &other) : QStyleOption(Version, Type) { *this = other; }

protected:
    QStyleOptionViewItem(int version);
};

class Q_GUI_EXPORT QStyleOptionToolBox : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_ToolBox };
    enum StyleOptionVersion { Version = 1 };

    QString text;
    QIcon icon;

    QStyleOptionToolBox();
    QStyleOptionToolBox(const QStyleOptionToolBox &other) : QStyleOption(Version, Type) { *this = other; }

protected:
    QStyleOptionToolBox(int version);
};

#ifndef QT_NO_RUBBERBAND
class Q_GUI_EXPORT QStyleOptionRubberBand : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_RubberBand };
    enum StyleOptionVersion { Version = 1 };

    QRubberBand::Shape shape;
    bool opaque;

    QStyleOptionRubberBand();
    QStyleOptionRubberBand(const QStyleOptionRubberBand &other) : QStyleOption(Version, Type) { *this = other; }

protected:
    QStyleOptionRubberBand(int version);
};
#endif // QT_NO_RUBBERBAND

// -------------------------- Complex style options -------------------------------
class Q_GUI_EXPORT QStyleOptionComplex : public QStyleOption
{
public:
    enum StyleOptionType { Type = SO_Complex };
    enum StyleOptionVersion { Version = 1 };

    QStyle::SubControls subControls;
    QStyle::SubControls activeSubControls;

    QStyleOptionComplex(int version = QStyleOptionComplex::Version, int type = SO_Complex);
    QStyleOptionComplex(const QStyleOptionComplex &other) : QStyleOption(Version, Type) { *this = other; }
};

#ifndef QT_NO_SLIDER
class Q_GUI_EXPORT QStyleOptionSlider : public QStyleOptionComplex
{
public:
    enum StyleOptionType { Type = SO_Slider };
    enum StyleOptionVersion { Version = 1 };

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

protected:
    QStyleOptionSlider(int version);
};
#endif // QT_NO_SLIDER

#ifndef QT_NO_SPINBOX
class Q_GUI_EXPORT QStyleOptionSpinBox : public QStyleOptionComplex
{
public:
    enum StyleOptionType { Type = SO_SpinBox };
    enum StyleOptionVersion { Version = 1 };

    QAbstractSpinBox::ButtonSymbols buttonSymbols;
    QAbstractSpinBox::StepEnabled stepEnabled;
    bool frame;

    QStyleOptionSpinBox();
    QStyleOptionSpinBox(const QStyleOptionSpinBox &other) : QStyleOptionComplex(Version, Type) { *this = other; }

protected:
    QStyleOptionSpinBox(int version);
};
#endif // QT_NO_SPINBOX

class Q_GUI_EXPORT QStyleOptionQ3ListView : public QStyleOptionComplex
{
public:
    enum StyleOptionType { Type = SO_Q3ListView };
    enum StyleOptionVersion { Version = 1 };

    QList<QStyleOptionQ3ListViewItem> items;
    QPalette viewportPalette;
    QPalette::ColorRole viewportBGRole;
    int sortColumn;
    int itemMargin;
    int treeStepSize;
    bool rootIsDecorated;

    QStyleOptionQ3ListView();
    QStyleOptionQ3ListView(const QStyleOptionQ3ListView &other) : QStyleOptionComplex(Version, Type) { *this = other; }

protected:
    QStyleOptionQ3ListView(int version);
};

class Q_GUI_EXPORT QStyleOptionToolButton : public QStyleOptionComplex
{
public:
    enum StyleOptionType { Type = SO_ToolButton };
    enum StyleOptionVersion { Version = 1 };

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

protected:
    QStyleOptionToolButton(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionToolButton::ToolButtonFeatures)

class Q_GUI_EXPORT QStyleOptionComboBox : public QStyleOptionComplex
{
public:
    enum StyleOptionType { Type = SO_ComboBox };
    enum StyleOptionVersion { Version = 1 };

    bool editable;
    QRect popupRect;
    bool frame;
    QString currentText;
    QIcon currentIcon;
    QSize iconSize;

    QStyleOptionComboBox();
    QStyleOptionComboBox(const QStyleOptionComboBox &other) : QStyleOptionComplex(Version, Type) { *this = other; }

protected:
    QStyleOptionComboBox(int version);
};

class Q_GUI_EXPORT QStyleOptionTitleBar : public QStyleOptionComplex
{
public:
    enum StyleOptionType { Type = SO_TitleBar };
    enum StyleOptionVersion { Version = 1 };

    QString text;
    QIcon icon;
    int titleBarState;
    Qt::WFlags titleBarFlags;

    QStyleOptionTitleBar();
    QStyleOptionTitleBar(const QStyleOptionTitleBar &other) : QStyleOptionComplex(Version, Type) { *this = other; }

protected:
    QStyleOptionTitleBar(int version);
};

class Q_GUI_EXPORT QStyleOptionGroupBox : public QStyleOptionComplex
{
public:
    enum StyleOptionType { Type = SO_GroupBox };
    enum StyleOptionVersion { Version = 1 };

    QStyleOptionFrameV2::FrameFeatures features;
    QString text;
    Qt::Alignment textAlignment;
    QColor textColor;
    int lineWidth;
    int midLineWidth;

    QStyleOptionGroupBox();
    QStyleOptionGroupBox(const QStyleOptionGroupBox &other) : QStyleOptionComplex(Version, Type) { *this = other; }
protected:
    QStyleOptionGroupBox(int version);
};

class Q_GUI_EXPORT QStyleOptionSizeGrip : public QStyleOptionComplex
{
public:
    enum StyleOptionType { Type = SO_SizeGrip };
    enum StyleOptionVersion { Version = 1 };

    Qt::Corner corner;

    QStyleOptionSizeGrip();
    QStyleOptionSizeGrip(const QStyleOptionSizeGrip &other) : QStyleOptionComplex(Version, Type) { *this = other; }
protected:
    QStyleOptionSizeGrip(int version);
};

template <typename T>
T qstyleoption_cast(const QStyleOption *opt)
{
    if (opt && opt->version >= static_cast<T>(0)->Version && (opt->type == static_cast<T>(0)->Type
        || int(static_cast<T>(0)->Type) == QStyleOption::SO_Default
        || (int(static_cast<T>(0)->Type) == QStyleOption::SO_Complex
            && opt->type > QStyleOption::SO_Complex)))
        return static_cast<T>(opt);
    return 0;
}

template <typename T>
T qstyleoption_cast(QStyleOption *opt)
{
    if (opt && opt->version >= static_cast<T>(0)->Version && (opt->type == static_cast<T>(0)->Type
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

    enum StyleOptionType { Type = SH_Default };
    enum StyleOptionVersion { Version = 1 };

    QStyleHintReturn(int version = QStyleOption::Version, int type = SH_Default);
    ~QStyleHintReturn();

    int version;
    int type;
};

class Q_GUI_EXPORT QStyleHintReturnMask : public QStyleHintReturn {
public:
    enum StyleOptionType { Type = SH_Mask };
    enum StyleOptionVersion { Version = 1 };

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

#if !defined(QT_NO_DEBUG_STREAM) && !defined(QT_NO_DEBUG)
Q_GUI_EXPORT QDebug operator<<(QDebug debug, const QStyleOption::OptionType &optionType);
Q_GUI_EXPORT QDebug operator<<(QDebug debug, const QStyleOption &option);
#endif

#endif // QSTYLEOPTION_H
