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
#include "qicon.h"
#include "qslider.h"
#include "qstyle.h"
#include "qtabbar.h"
#include "qtabwidget.h"

class Q_GUI_EXPORT QStyleOption
{
public:
    enum OptionType {
                      // Standard controls
                      SO_Default, SO_FocusRect, SO_Button, SO_Tab, SO_MenuItem,
                      SO_Frame, SO_ProgressBar, SO_ToolBox, SO_Header, SO_Q3DockWindow,
                      SO_DockWindow, SO_ListViewItem, SO_ViewItem, SO_TabWidgetFrame,

                      // Complex controls
                      SO_Complex = 0xf000, SO_Slider, SO_SpinBox, SO_ToolButton, SO_ComboBox,
                      SO_ListView, SO_TitleBar,

                      // base for custom standard controls
                      SO_CustomBase = 0xf00000,
                      // base for custom complex controls
                      SO_ComplexCustomBase = 0xf000000
                    };

    enum { Type = SO_Default };
    enum { Version = 1 };

    int version;
    int type;
    QStyle::StyleFlags state;
    Qt::LayoutDirection direction;
    QRect rect;
    QFontMetrics fontMetrics;
    QPalette palette;

    QStyleOption(int version = QStyleOption::Version, int type = SO_Default);
    ~QStyleOption();

    void init(const QWidget *w);

    QDOC_PROPERTY(int version)
    QDOC_PROPERTY(int type)
    QDOC_PROPERTY(QStyle::SFlags state)
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

    QStyleOptionTabWidgetFrame();

    QDOC_PROPERTY(int lineWidth)
    QDOC_PROPERTY(int midLineWidth)

protected:
    QStyleOptionTabWidgetFrame(int version);
};

class Q_GUI_EXPORT QStyleOptionHeader : public QStyleOption
{
public:
    enum { Type = SO_Header };
    enum { Version = 1 };

    int section;
    QString text;
    Qt::Alignment textAlignment;
    QIcon icon;
    Qt::Alignment iconAlignment;

    QStyleOptionHeader();

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

    enum ButtonFeature { None = 0x00, Flat = 0x01, HasMenu = 0x02, DefaultButton = 0x04, AutoDefaultButton = 0x08 };
    Q_DECLARE_FLAGS(ButtonFeatures, ButtonFeature)

    ButtonFeatures features;
    QString text;
    QIcon icon;

    QStyleOptionButton();

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
    enum SelectedPosition { NotAdjacent, NextIsSelected, PreviousIsSelected};

    QTabBar::Shape shape;
    Qt::Orientation orientation;
    QString text;
    QIcon icon;
    int row;
    TabPosition position;
    SelectedPosition selectedPosition;

    QStyleOptionTab();

    QDOC_PROPERTY(QTabBar::Shape shape)
    QDOC_PROPERTY(Qt::Orientation Orientation)
    QDOC_PROPERTY(QString text)
    QDOC_PROPERTY(QIcon icon)
    QDOC_PROPERTY(int row)
    QDOC_PROPERTY(TabPosition position)
    QDOC_PROPERTY(SelectedPosition selectedPosition)

protected:
    QStyleOptionTab(int version);
};

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
    QRect menuRect;
    QString text;
    QIcon icon;
    int maxIconWidth;
    int tabWidth;
    QFont font;

    QStyleOptionMenuItem();

    QDOC_PROPERTY(MenuItemType menuItemType)
    QDOC_PROPERTY(CheckType checkType)
    QDOC_PROPERTY(bool checked)
    QDOC_PROPERTY(QRect menuRect)
    QDOC_PROPERTY(QString text)
    QDOC_PROPERTY(QIcon icon)
    QDOC_PROPERTY(int maxIconWidth)
    QDOC_PROPERTY(int tabWidth)
    QDOC_PROPERTY(QFont font)

protected:
    QStyleOptionMenuItem(int version);
};

class Q_GUI_EXPORT QStyleOptionListViewItem : public QStyleOption
{
public:
    enum { Type = SO_ListViewItem };
    enum { Version = 1 };

    enum ListViewItemFeature { None = 0x00, Expandable = 0x01, MultiLine = 0x02, Visible = 0x04,
                               ParentControl = 0x08 };
    Q_DECLARE_FLAGS(ListViewItemFeatures, ListViewItemFeature)

    ListViewItemFeatures features;
    int height;
    int totalHeight;
    int itemY;
    int childCount;

    QStyleOptionListViewItem();

    QDOC_PROPERTY(ListViewItemFeatures features)
    QDOC_PROPERTY(int height)
    QDOC_PROPERTY(int totalHeight)
    QDOC_PROPERTY(int itemY)
    QDOC_PROPERTY(int childCount)

protected:
    QStyleOptionListViewItem(int version);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyleOptionListViewItem::ListViewItemFeatures);

class Q_GUI_EXPORT QStyleOptionQ3DockWindow : public QStyleOption
{
public:
    enum { Type = SO_Q3DockWindow };
    enum { Version = 1 };

    bool docked;
    bool closeEnabled;

    QStyleOptionQ3DockWindow();

    QDOC_PROPERTY(bool docked)
    QDOC_PROPERTY(bool closeEnabled)

protected:
    QStyleOptionQ3DockWindow(int version);
};

class Q_GUI_EXPORT QStyleOptionDockWindow : public QStyleOption
{
public:
    enum { Type = SO_DockWindow };
    enum { Version = 1 };

    QString title;
    bool closable;
    bool moveable;
    bool floatable;

    QStyleOptionDockWindow();

    QDOC_PROPERTY(QString title)
    QDOC_PROPERTY(bool closable)
    QDOC_PROPERTY(bool moveable)
    QDOC_PROPERTY(bool floatable)

protected:
    QStyleOptionDockWindow(int version);
};

class Q_GUI_EXPORT QStyleOptionViewItem : public QStyleOption
{
public:
    enum { Type = SO_ViewItem };
    enum { Version = 1 };

    enum Position { Left, Right, Top, Bottom };
    enum Size { Small, Large };

    int displayAlignment;
    int decorationAlignment;
    Position decorationPosition;
    Size decorationSize;
    QFont font;

    QStyleOptionViewItem();

    QDOC_PROPERTY(int displayAlignment)
    QDOC_PROPERTY(int decorationAlignment)
    QDOC_PROPERTY(Position decorationPosition)
    QDOC_PROPERTY(Size decorationSize)
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

    QDOC_PROPERTY(QString text)
    QDOC_PROPERTY(QIcon icon)

protected:
    QStyleOptionToolBox(int version);
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

    QStyleOptionSlider();

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
    double percentage;
    bool showSliderIndicator;
    bool showFrame;

    QStyleOptionSpinBox();

    QDOC_PROPERTY(QAbstractSpinBox::ButtonSymbols buttonSymbols)
    QDOC_PROPERTY(QAbstractSpinBox::StepEnabled stepEnabled)
    QDOC_PROPERTY(double percentage)
    QDOC_PROPERTY(bool showSliderIndicator)
    QDOC_PROPERTY(bool showFrame)

protected:
    QStyleOptionSpinBox(int version);
};

class Q_GUI_EXPORT QStyleOptionListView : public QStyleOptionComplex
{
public:
    enum { Type = SO_ListView };
    enum { Version = 1 };

    QList<QStyleOptionListViewItem> items;
    QPalette viewportPalette;
    QPalette::ColorRole viewportBGRole;
    int sortColumn;
    int itemMargin;
    int treeStepSize;
    bool rootIsDecorated;

    QStyleOptionListView();

    QDOC_PROPERTY(QList<QStyleOptionListViewItem> items)
    QDOC_PROPERTY(QPalette viewportPalette)
    QDOC_PROPERTY(QPalette::ColorRole viewportBGRole)
    QDOC_PROPERTY(int sortColumn)
    QDOC_PROPERTY(int itemMargin)
    QDOC_PROPERTY(int treeStepSize)
    QDOC_PROPERTY(bool rootIsDecorated)

protected:
    QStyleOptionListView(int version);
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
    QString text;
    Qt::ArrowType arrowType;
    Qt::IconSize iconSize;
    Qt::ToolButtonStyle toolButtonStyle;
    QPoint pos;
    QFont font;

    QStyleOptionToolButton();

    QDOC_PROPERTY(ToolButtonFeatures features)
    QDOC_PROPERTY(QIcon icon)
    QDOC_PROPERTY(QString text)
    QDOC_PROPERTY(Qt::ArrowType arrowType)
    QDOC_PROPERTY(Qt::IconSize iconSize)
    QDOC_PROPERTY(Qt::ToolButtonStyle toolButtonStyle)
    QDOC_PROPERTY(QPoint pos)
    QDOC_PROPERTY(QFont font)

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

    QStyleOptionComboBox();

    QDOC_PROPERTY(bool editable)
    QDOC_PROPERTY(QRect popupRect)

protected:
    QStyleOptionComboBox(int version);
};

class Q_GUI_EXPORT QStyleOptionTitleBar : public QStyleOptionComplex
{
public:
    enum { Type = SO_TitleBar };
    enum { Version = 1 };

    QString text;
    QPixmap icon;
    int titleBarState;
    Qt::WFlags titleBarFlags;

    QStyleOptionTitleBar();

    QDOC_PROPERTY(QString text)
    QDOC_PROPERTY(QPixmap icon)
    QDOC_PROPERTY(int titleBarState)
    QDOC_PROPERTY(Qt::WFlags titleBarFlags)

protected:
    QStyleOptionTitleBar(int version);
};

template <typename T>
T qt_cast(const QStyleOption *opt)
{
    if (opt && opt->version <= static_cast<T>(0)->Version && (opt->type == static_cast<T>(0)->Type
        || int(static_cast<T>(0)->Type) == QStyleOption::SO_Default
        || (int(static_cast<T>(0)->Type) == QStyleOption::SO_Complex
            && ((opt->type > QStyleOption::SO_Complex && opt->type < QStyleOption::SO_CustomBase)
                || opt->type >= QStyleOption::SO_ComplexCustomBase))))
        return static_cast<T>(opt);
    return 0;
}

template <typename T>
T qt_cast(QStyleOption *opt)
{
    if (opt && opt->version <= static_cast<T>(0)->Version && (opt->type == static_cast<T>(0)->Type
        || int(static_cast<T>(0)->Type) == QStyleOption::SO_Default
        || (int(static_cast<T>(0)->Type) == QStyleOption::SO_Complex
            && ((opt->type > QStyleOption::SO_Complex && opt->type < QStyleOption::SO_CustomBase)
                || opt->type >= QStyleOption::SO_ComplexCustomBase))))
        return static_cast<T>(opt);
    return 0;
}
#endif
