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

#ifndef QSTYLE_H
#define QSTYLE_H

#ifndef QT_H
#include "qobject.h"
#include "qpixmap.h"
#include "qrect.h"
#include "qsize.h"
#endif // QT_H

#ifndef QT_NO_STYLE

class QAction;
class QTab;
class QCheckListItem;
class QFontMetrics;

// This class goes away after the tech preview
class Q3StyleOption {
public:
    enum StyleOptionDefault { Default };

    Q3StyleOption(StyleOptionDefault=Default) : def(true) {}
    // Note: we don't use default arguments since that is unnecessary
    // initialization.
    Q3StyleOption(int in1) : def(false), i1(in1) {}
    Q3StyleOption(int in1, int in2) : def(false), i1(in1), i2(in2) {}
    Q3StyleOption(int in1, int in2, int in3, int in4) :
        def(false), i1(in1), i2(in2), i3(in3), i4(in4) {}
    Q3StyleOption(QAction *a) : def(false), act(a) {}
    Q3StyleOption(QAction *a, int in1) : def(false), act(a), i1(in1) {}
    Q3StyleOption(QAction *a, int in1, int in2) : def(false), act(a), i1(in1), i2(in2) {}
    Q3StyleOption(const QColor& c) : def(false), cl(&c) {}
    Q3StyleOption(QTab *t) : def(false), tb(t) {}
    Q3StyleOption(QCheckListItem *i) : def(false), cli(i) {}
    Q3StyleOption(Qt::ArrowType a) : def(false), i1((int)a) {}
    Q3StyleOption(const QRect &r) : def(false), i1(r.x()), i2(r.y()), i3(r.width()),i4(r.height()){}
    Q3StyleOption(QWidget *w) : def(false), p1((void*)w) {}

    bool isDefault() const { return def; }

    int day() const { return i1; }

    int lineWidth() const { return i1; }
    int midLineWidth() const { return i2; }
    int frameShape() const { return i3; }
    int frameShadow() const { return i4; }

    int titleBarState() const { return i1; }

    int headerSection() const { return i1; }
    QAction *action() const { return act; }
    int maxIconWidth() const { return i1; }
    int tabWidth() const { return i2; }

    const QColor &color() const { return *cl; }

    QTab *tab() const { return tb; }

    QCheckListItem *checkListItem() const { return cli; }

    Qt::ArrowType arrowType() const { return (Qt::ArrowType)i1; }
    QRect rect() const { return QRect(i1, i2, i3, i4); }
    QWidget *widget() const { return (QWidget*)p1; }

private:
    // NOTE: none of these components have constructors.
    bool def;
    bool b1,b2,b3; // reserved
    QAction *act;
    QTab* tb;
    const QColor* cl;
    int i1, i2, i3, i4, i5, i6; // reserved
    QCheckListItem* cli;
    void *p1, *p2, *p3, *p4; // reserved
    // (padded to 64 bytes on some architectures)
};

class QStyleHintReturn; // not defined yet

struct QStyleOption;
struct QStyleOptionComplex;
class Q_GUI_EXPORT QStyle: public QObject
{
    Q_OBJECT

public:
    QStyle();
    virtual ~QStyle();

    virtual void polish(QWidget *);
    virtual void unPolish(QWidget *);

    virtual void polish(QApplication *);
    virtual void unPolish(QApplication *);

    virtual void polish(QPalette &);

    virtual QRect itemRect(const QFontMetrics &fm, const QRect &r,
                           int flags, bool enabled,
                           const QString &text, int len = -1) const;

    virtual QRect itemRect(const QRect &r,
                            int flags, const QPixmap &pixmap) const;

    QRect itemRect(QPainter *p, const QRect &r,
                   int flags, bool enabled,
                   const QPixmap &pixmap,
                   const QString &text, int len = -1) const;

    virtual void drawItem(QPainter *p, const QRect &r,
                          int flags, const QPalette &pal, bool enabled,
                          const QString &text, int len = -1,
                          const QColor *penColor = 0) const;

    virtual void drawItem(QPainter *p, const QRect &r,
                          int flags, const QPalette &pal, bool enabled,
                          const QPixmap &pixmap,
                          const QColor *penColor = 0) const;

    void drawItem(QPainter *p, const QRect &r,
                  int flags, const QPalette &pal, bool enabled,
                  const QPixmap &pixmap,
                  const QString &text, int len = -1,
                  const QColor *penColor = 0) const {
        if (!pixmap.isNull())
            drawItem(p, r, flags, pal, enabled, pixmap, penColor);
        else
            drawItem(p, r, flags, pal, enabled, text, len, penColor);
    }

    enum StyleFlags {
        Style_Default =       0x00000000,
        Style_Enabled =       0x00000001,
        Style_Raised =        0x00000002,
        Style_Sunken =        0x00000004,
        Style_Off =           0x00000008,
        Style_NoChange =      0x00000010,
        Style_On =            0x00000020,
        Style_Down =          0x00000040,
        Style_Horizontal =    0x00000080,
        Style_HasFocus =      0x00000100,
        Style_Top =           0x00000200,
        Style_Bottom =        0x00000400,
        Style_FocusAtBorder = 0x00000800,
        Style_AutoRaise =     0x00001000,
        Style_MouseOver =     0x00002000,
        Style_Up =            0x00004000,
        Style_Selected =      0x00008000,
        Style_Active =        0x00010000,
        Style_ButtonDefault = 0x00020000,
        Style_Rectangle =     0x00040000,
        Style_Open =          0x00100000,
        Style_Children =      0x00200000,
        Style_Item =          0x00400000,
        Style_Sibling =       0x00800000,
        Style_Editing =       0x01000000
    };
    typedef uint SFlags;

    enum PrimitiveElement {
        PE_ButtonCommand,
        PE_ButtonDefault,
        PE_ButtonBevel,
        PE_ButtonTool,
        PE_ButtonDropDown,

        PE_FocusRect,

        PE_ArrowUp,
        PE_ArrowDown,
        PE_ArrowRight,
        PE_ArrowLeft,

        PE_SpinBoxUp,
        PE_SpinBoxDown,
        PE_SpinBoxPlus,
        PE_SpinBoxMinus,
        PE_SpinBoxSlider,

        PE_Indicator,
        PE_IndicatorMask,
        PE_ExclusiveIndicator,
        PE_ExclusiveIndicatorMask,

        PE_DockWindowHandle,
        PE_DockWindowSeparator,
        PE_DockWindowResizeHandle,

        PE_Splitter,

        PE_Panel,
        PE_PanelPopup,
        PE_PanelMenuBar,
        PE_PanelDockWindow,

        PE_TabBarBase,

        PE_HeaderSection,
        PE_HeaderArrow,
        PE_StatusBarSection,

        PE_GroupBoxFrame,

        PE_Separator,

        PE_SizeGrip,

        PE_CheckMark,

        PE_ScrollBarAddLine,
        PE_ScrollBarSubLine,
        PE_ScrollBarAddPage,
        PE_ScrollBarSubPage,
        PE_ScrollBarSlider,
        PE_ScrollBarFirst,
        PE_ScrollBarLast,

        PE_ProgressBarChunk,

        PE_PanelLineEdit,
        PE_PanelTabWidget,

        PE_WindowFrame,

        PE_CheckListController,
        PE_CheckListIndicator,
        PE_CheckListExclusiveIndicator,

        PE_PanelGroupBox,

        PE_TreeBranch,

        PE_MenuFrame,
        PE_MenuBarFrame,

        PE_RubberBand,
        PE_RubberBandMask,

        // do not add any values below/greater this
        PE_CustomBase = 0xf000000
    };

    virtual void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                               const QWidget *w = 0) const = 0;
    enum ControlElement {
        CE_PushButton,
        CE_PushButtonLabel,

        CE_CheckBox,
        CE_CheckBoxLabel,

        CE_RadioButton,
        CE_RadioButtonLabel,

        CE_TabBarTab,
        CE_TabBarLabel,

        CE_ProgressBarGroove,
        CE_ProgressBarContents,
        CE_ProgressBarLabel,

        CE_MenuItem,
        CE_MenuScroller,
        CE_MenuVMargin,
        CE_MenuHMargin,
        CE_MenuTearoff,
        CE_MenuEmptyArea,

        CE_MenuBarItem,
        CE_MenuBarEmptyArea,

        CE_ToolButtonLabel,

        CE_DockWindowEmptyArea,
        CE_ToolBoxTab,
        CE_HeaderLabel,

        CE_ToolBarButton,

        // do not add any values below/greater than this
        CE_CustomBase = 0xf0000000
    };

    virtual void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                             const QWidget *w = 0) const = 0;
    virtual void drawControlMask(ControlElement element, const QStyleOption *opt, QPainter *p,
                                 const QWidget *w = 0) const = 0;

    enum SubRect {
        SR_PushButtonContents,
        SR_PushButtonFocusRect,

        SR_CheckBoxIndicator,
        SR_CheckBoxContents,
        SR_CheckBoxFocusRect,

        SR_RadioButtonIndicator,
        SR_RadioButtonContents,
        SR_RadioButtonFocusRect,

        SR_ComboBoxFocusRect,

        SR_SliderFocusRect,

        SR_DockWindowHandleRect,

        SR_ProgressBarGroove,
        SR_ProgressBarContents,
        SR_ProgressBarLabel,

        SR_ToolButtonContents,

        SR_DialogButtonAccept,
        SR_DialogButtonReject,
        SR_DialogButtonApply,
        SR_DialogButtonHelp,
        SR_DialogButtonAll,
        SR_DialogButtonAbort,
        SR_DialogButtonIgnore,
        SR_DialogButtonRetry,
        SR_DialogButtonCustom,

        SR_ToolBoxTabContents,

        SR_ToolBarButtonContents,
        SR_ToolBarButtonMenu,

        // do not add any values below/greater than this
        SR_CustomBase = 0xf0000000
    };

    virtual QRect subRect(SubRect r, const QStyleOption *opt, const QWidget *widget = 0) const = 0;


    enum ComplexControl {
        CC_SpinBox,
        CC_ComboBox,
        CC_ScrollBar,
        CC_Slider,
        CC_ToolButton,
        CC_TitleBar,
        CC_ListView,

        // do not add any values below/greater than this
        CC_CustomBase = 0xf0000000
    };

    enum SubControl {
        SC_None =                  0x00000000,

        SC_ScrollBarAddLine =      0x00000001,
        SC_ScrollBarSubLine =      0x00000002,
        SC_ScrollBarAddPage =      0x00000004,
        SC_ScrollBarSubPage =      0x00000008,
        SC_ScrollBarFirst =        0x00000010,
        SC_ScrollBarLast =         0x00000020,
        SC_ScrollBarSlider =       0x00000040,
        SC_ScrollBarGroove =       0x00000080,

        SC_SpinBoxUp =             0x00000001,
        SC_SpinBoxDown =           0x00000002,
        SC_SpinBoxFrame =          0x00000004,
        SC_SpinBoxEditField =      0x00000008,
        SC_SpinBoxButtonField =    0x00000010,
        SC_SpinBoxSlider =         0x00000020,

        SC_ComboBoxFrame =         0x00000001,
        SC_ComboBoxEditField =     0x00000002,
        SC_ComboBoxArrow =         0x00000004,
        SC_ComboBoxListBoxPopup =  0x00000008,

        SC_SliderGroove =          0x00000001,
        SC_SliderHandle =          0x00000002,
        SC_SliderTickmarks =       0x00000004,

        SC_ToolButton =            0x00000001,
        SC_ToolButtonMenu =        0x00000002,

        SC_TitleBarLabel =         0x00000001,
        SC_TitleBarSysMenu =       0x00000002,
        SC_TitleBarMinButton =     0x00000004,
        SC_TitleBarMaxButton =     0x00000008,
        SC_TitleBarCloseButton =   0x00000010,
        SC_TitleBarNormalButton =  0x00000020,
        SC_TitleBarShadeButton =   0x00000040,
        SC_TitleBarUnshadeButton = 0x00000080,

        SC_ListView =              0x00000001,
        SC_ListViewBranch =        0x00000002,
        SC_ListViewExpand =        0x00000004,

        SC_All =                   0xffffffff
    };
    typedef uint SCFlags;


    virtual void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                    const QWidget *widget = 0) const = 0;
    virtual void drawComplexControlMask(ComplexControl cc, const QStyleOptionComplex *opt,
                                        QPainter *p, const QWidget *widget = 0) const = 0;
    virtual SubControl querySubControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                       const QPoint &pt, const QWidget *widget = 0) const = 0;
    virtual QRect querySubControlMetrics(ComplexControl cc, const QStyleOptionComplex *opt,
                                         SubControl sc, const QWidget *widget = 0) const = 0;
    enum PixelMetric {
        PM_ButtonMargin,
        PM_ButtonDefaultIndicator,
        PM_MenuButtonIndicator,
        PM_ButtonShiftHorizontal,
        PM_ButtonShiftVertical,

        PM_DefaultFrameWidth,
        PM_SpinBoxFrameWidth,

        PM_MaximumDragDistance,

        PM_ScrollBarExtent,
        PM_ScrollBarSliderMin,

        PM_SliderThickness,             // total slider thickness
        PM_SliderControlThickness,      // thickness of the business part
        PM_SliderLength,                // total length of slider
        PM_SliderTickmarkOffset,        //
        PM_SliderSpaceAvailable,        // available space for slider to move

        PM_DockWindowSeparatorExtent,
        PM_DockWindowHandleExtent,
        PM_DockWindowFrameWidth,

        PM_TabBarTabOverlap,
        PM_TabBarTabHSpace,
        PM_TabBarTabVSpace,
        PM_TabBarBaseHeight,
        PM_TabBarBaseOverlap,

        PM_ProgressBarChunkWidth,

        PM_SplitterWidth,
        PM_TitleBarHeight,

        PM_MenuScrollerHeight,
        PM_MenuHMargin,
        PM_MenuVMargin,
        PM_MenuFrameWidth,
        PM_MenuTearoffHeight,
        PM_MenuDesktopFrameWidth,

        PM_MenuBarFrameWidth,
        PM_MenuBarItemSpacing,
        PM_MenuBarVMargin,
        PM_MenuBarHMargin,

        PM_IndicatorWidth,
        PM_IndicatorHeight,
        PM_ExclusiveIndicatorWidth,
        PM_ExclusiveIndicatorHeight,
        PM_CheckListButtonSize,
        PM_CheckListControllerSize,

        PM_DialogButtonsSeparator,
        PM_DialogButtonsButtonWidth,
        PM_DialogButtonsButtonHeight,

        PM_MDIFrameWidth,
        PM_MDIMinimizedWidth,
        PM_HeaderMargin,
        PM_HeaderMarkSize,
        PM_HeaderGripMargin,
        PM_TabBarTabShiftHorizontal,
        PM_TabBarTabShiftVertical,
        PM_TabBarScrollButtonWidth,

        PM_ToolBarItemSpacing,
        PM_SpinBoxSliderHeight,

        PM_DefaultToplevelMargin,
        PM_DefaultChildMargin,
        PM_DefaultLayoutSpacing,

        // do not add any values below/greater than this
        PM_CustomBase = 0xf0000000
    };

    virtual int pixelMetric(PixelMetric metric, const QWidget *widget = 0) const = 0;

    enum ContentsType {
        CT_PushButton,
        CT_CheckBox,
        CT_RadioButton,
        CT_ToolButton,
        CT_ComboBox,
        CT_Splitter,
        CT_DockWindow,
        CT_ProgressBar,
        CT_MenuItem,
        CT_MenuBarItem,
        CT_MenuBar,
        CT_Menu,
        CT_TabBarTab,
        CT_Slider,
        CT_Header,
        CT_LineEdit,
        CT_SpinBox,
        CT_SizeGrip,
        CT_TabWidget,
        CT_DialogButtons,
        CT_ToolBarButton,
        // do not add any values below/greater than this
        CT_CustomBase = 0xf0000000
    };

    virtual QSize sizeFromContents(ContentsType ct, const QStyleOption *opt,
                                   const QSize &contentsSize, const QFontMetrics &fm,
                                   const QWidget *w = 0) const = 0;

    enum StyleHint {
        // ...
        // the general hints
        // ...
        // disabled text should be etched, ala Windows
        SH_EtchDisabledText,

        // the GUI style enum, argh!
        SH_GUIStyle,

        // ...
        // widget specific hints
        // ...
        SH_ScrollBar_BackgroundMode, // deprecated
        SH_ScrollBar_MiddleClickAbsolutePosition,
        SH_ScrollBar_ScrollWhenPointerLeavesControl,

        // QEvent::Type - which mouse event to select a tab
        SH_TabBar_SelectMouseType,

        SH_TabBar_Alignment,

        SH_Header_ArrowAlignment,

        // bool - sliders snap to values while moving, ala Windows
        SH_Slider_SnapToValue,

        // bool - key presses handled in a sloppy manner - ie. left on a vertical
        // slider subtracts a line
        SH_Slider_SloppyKeyEvents,

        // bool - center button on progress dialogs, ala Motif, else right aligned
        // perhaps this should be a Qt::Alignment value
        SH_ProgressDialog_CenterCancelButton,

        // Qt::Alignment - text label alignment in progress dialogs
        // Center on windows, Auto|VCenter otherwise
        SH_ProgressDialog_TextLabelAlignment,

        // bool - right align buttons on print dialog, ala Windows
        SH_PrintDialog_RightAlignButtons,

        // bool - 1 or 2 pixel space between the menubar and the dockarea, ala Windows
        // this *REALLY* needs a better name
        SH_MainWindow_SpaceBelowMenuBar,

        // bool - select the text in the line edit about the listbox when selecting
        // an item from the listbox, or when the line edit receives focus, ala Windows
        SH_FontDialog_SelectAssociatedText,

        // bool - allows disabled menu items to be active
        SH_Menu_AllowActiveAndDisabled,

        // bool - pressing space activates item, ala Motif
        SH_Menu_SpaceActivatesItem,

        // int - number of milliseconds to wait before opening a submenu
        // 256 on windows, 96 on motif
        SH_Menu_SubMenuPopupDelay,

        // bool - should scrollviews draw their frame only around contents (ala Motif),
        // or around contents, scrollbars and corner widgets (ala Windows) ?
        SH_ScrollView_FrameOnlyAroundContents,

        // bool - menubars items are navigatable by pressing alt, followed by using
        // the arrow keys to select the desired item
        SH_MenuBar_AltKeyNavigation,

        // bool - mouse tracking in combobox dropdown lists
        SH_ComboBox_ListMouseTracking,

        // bool - mouse tracking in popupmenus
        SH_Menu_MouseTracking,

        // bool - mouse tracking in menubars
        SH_MenuBar_MouseTracking,

        // bool - gray out selected items when loosing focus
        SH_ItemView_ChangeHighlightOnFocus,

        // bool - supports shared activation among modeless widgets
        SH_Widget_ShareActivation,

        // bool - workspace should just maximize the client area
        SH_Workspace_FillSpaceOnMaximize,

        // bool - supports popup menu comboboxes
        SH_ComboBox_Popup,

        // bool - title bar has no border
        SH_TitleBar_NoBorder,

        // bool - stop scrollbar at mouse
        SH_ScrollBar_StopMouseOverSlider,

        //bool - blink cursort with selected text
        SH_BlinkCursorWhenTextSelected,

        //bool - richtext selections extend the full width of the docuemnt
        SH_RichText_FullWidthSelection,

        //bool - popupmenu supports scrolling instead of multicolumn mode
        SH_Menu_Scrollable,

        // Qt::Alignment - text label vertical alignment in groupboxes
        // Center on windows, Auto|VCenter otherwise
        SH_GroupBox_TextLabelVerticalAlignment,

        // Qt::QRgb - text label color in groupboxes
        SH_GroupBox_TextLabelColor,

        // bool - popupmenu supports sloppy submenus
        SH_Menu_SloppySubMenus,

        // Qt::QRgb - table grid color
        SH_Table_GridLineColor,

        // QChar - Unicode character for password char
        SH_LineEdit_PasswordCharacter,

        // QDialogButtons::Button - default button
        SH_DialogButtons_DefaultButton,

        // QToolBox - Boldness of the selected page title
        SH_ToolBox_SelectedPageTitleBold,

        //bool - if a tabbar prefers not to have scroller arrows
        SH_TabBar_PreferNoArrows,

        //bool - if left button should cause an absolute position
        SH_ScrollBar_LeftClickAbsolutePosition,

        // QEvent::Type - which mouse event to select a list view expansion
        SH_ListViewExpand_SelectMouseType,

        //bool - if underline for shortcuts
        SH_UnderlineShortcut,

        // bool - if tool buttons should use a 3D frame
        // when the mouse is over the button
        SH_ToolButton_Uses3D,

        SH_ScrollBar_BackgroundRole,

        //bool - animate a click when up or down is pressed in a spin box
        SH_SpinBox_AnimateButton,

        //int - autorepeat interval for keyboard on spinboxes
        SH_SpinBox_KeyPressAutoRepeatRate,

        //int - autorepeat interval for mouse clicks on spinboxes
        SH_SpinBox_ClickAutoRepeatRate,

        //bool - small scrolling popups should fill the screen as scrolled
        SH_Menu_FillScreenWithScroll,

        // int - a scale of 0 (transparent) to 255 (solid) indicating opacity of tip label.
        SH_TipLabel_Opacity,

#ifdef QT_COMPAT
        SH_UnderlineAccelerator = SH_UnderlineShortcut,
#endif

        // do not add any values below/greater than this
        SH_CustomBase = 0xf0000000
    };

    virtual int styleHint(StyleHint stylehint,
                          const QWidget *widget = 0,
                          const Q3StyleOption& = Q3StyleOption::Default,
                          QStyleHintReturn* returnData = 0) const = 0;


    enum StylePixmap {
        SP_TitleBarMinButton,
        SP_TitleBarMaxButton,
        SP_TitleBarCloseButton,
        SP_TitleBarNormalButton,
        SP_TitleBarShadeButton,
        SP_TitleBarUnshadeButton,
        SP_DockWindowCloseButton,
        SP_MessageBoxInformation,
        SP_MessageBoxWarning,
        SP_MessageBoxCritical,
        SP_MessageBoxQuestion,
        SP_DesktopIcon,
        SP_TrashIcon,
        SP_ComputerIcon,
        SP_DriveFDIcon,
        SP_DriveHDIcon,
        SP_DriveCDIcon,
        SP_DriveDVDIcon,
        SP_DriveNetIcon,
        SP_DirOpenIcon,
        SP_DirClosedIcon,
        SP_DirLinkIcon,
        SP_FileIcon,
        SP_FileLinkIcon,

        // do not add any values below/greater than this
        SP_CustomBase = 0xf0000000
    };

    virtual QPixmap stylePixmap(StylePixmap stylepixmap,
                                const QWidget *widget = 0,
                                const Q3StyleOption& = Q3StyleOption::Default) const = 0;

    enum PixmapType {
        PT_Disabled,
        PT_Active,

        // do not add any values below/greater than this
        PT_CustomBase = 0xf0000000
    };

    virtual QPixmap stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap,
                                const QPalette &pal,
                                const Q3StyleOption& = Q3StyleOption::Default) const = 0;

    static QRect visualRect(const QRect &logical, const QWidget *w);
    static QRect visualRect(const QRect &logical, const QRect &bounding);
    static QPoint visualPos(const QPoint &logical, const QWidget *w);
    static QPoint visualPos(const QPoint &logical, const QRect &bounding);
    static int positionFromValue(int min, int max, int val, int space, bool upsideDown = false);
    static int valueFromPosition(int min, int max, int pos, int space, bool upsideDown = false);

private:
#ifdef QT_COMPAT
    // cause a compile error when trying to use style functions that
    // accept QColorGroup arguments... remove 5.x
    void QT_COMPAT drawItem(QPainter *p, const QRect &r,
                   int flags, const QColorGroup &colorgroup, bool enabled,
                   const QString &text, int len = -1,
                   const QColor *penColor = 0) const;

    void QT_COMPAT drawItem(QPainter *p, const QRect &r,
                   int flags, const QColorGroup colorgroup, bool enabled,
                   const QPixmap &pixmap,
                   const QColor *penColor = 0) const;

    void QT_COMPAT drawItem(QPainter *p, const QRect &r,
                   int flags, const QColorGroup colorgroup, bool enabled,
                   const QPixmap *pixmap,
                   const QString &text, int len = -1,
                   const QColor *penColor = 0) const;
#endif // QT_COMPAT

#if defined(Q_DISABLE_COPY)
    QStyle(const QStyle &);
    QStyle& operator=(const QStyle &);
#endif
};
#endif // QT_NO_STYLE
#endif // QSTYLE_H
