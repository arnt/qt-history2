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

#include "qobject.h"
#include "qpixmap.h"
#include "qrect.h"
#include "qsize.h"

#ifndef QT_NO_STYLE

class QAction;
class QTab;
class QFontMetrics;

class QStyleHintReturn; // not defined yet

class QStyleOption;
class QStyleOptionComplex;
class Q_GUI_EXPORT QStyle : public QObject
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

    virtual QRect itemRect(const QRect &r, int flags, const QPixmap &pixmap) const;

    QRect itemRect(QPainter *p, const QRect &r, int flags, bool enabled,
                   const QPixmap &pixmap, const QString &text, int len = -1) const;

    virtual void drawItem(QPainter *p, const QRect &r,
                          int flags, const QPalette &pal, bool enabled,
                          const QString &text, int len = -1,
                          const QColor *penColor = 0) const;

    virtual void drawItem(QPainter *p, const QRect &r,
                          int flags, const QPalette &pal, bool enabled,
                          const QPixmap &pixmap,
                          const QColor *penColor = 0) const;

    inline void drawItem(QPainter *p, const QRect &r,
                  int flags, const QPalette &pal, bool enabled,
                  const QPixmap &pixmap,
                  const QString &text, int len = -1,
                  const QColor *penColor = 0) const {
        if (!pixmap.isNull())
            drawItem(p, r, flags, pal, enabled, pixmap, penColor);
        else
            drawItem(p, r, flags, pal, enabled, text, len, penColor);
    }

    enum StyleFlag {
        Style_None    =       0x00000000,
#ifdef QT_COMPAT
        Style_Default = Style_None,
#endif
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
        Style_Rectangle =     0x00020000,
        Style_Open =          0x00040000,
        Style_Children =      0x00080000,
        Style_Item =          0x00100000,
        Style_Sibling =       0x00200000,
        Style_Editing =       0x00400000
    };
    Q_DECLARE_FLAGS(StyleFlags, StyleFlag)

#ifdef QT_COMPAT
    typedef StyleFlags SFlags;
#endif

    enum PrimitiveElement {
        PE_Q3CheckListController,
        PE_Q3CheckListExclusiveIndicator,
        PE_Q3CheckListIndicator,
        PE_Q3DockWindowSeparator,
        PE_Q3Separator,

        PE_Frame,
        PE_FrameDefaultButton,
        PE_FrameDockWindow,
        PE_FrameFocusRect,
        PE_FrameGroupBox,
        PE_FrameLineEdit,
        PE_FrameMenu,
        PE_FrameStatusBar,
        PE_FrameTabWidget,
        PE_FrameWindow,
        PE_FrameButtonBevel,
        PE_FrameButtonTool,

        PE_PanelButtonCommand,
        PE_PanelButtonBevel,
        PE_PanelButtonTool,
        PE_PanelHeader,
        PE_PanelMenuBar,
        PE_PanelToolBar,

        PE_IndicatorArrowDown,
        PE_IndicatorArrowLeft,
        PE_IndicatorArrowRight,
        PE_IndicatorArrowUp,
        PE_IndicatorBranch,
        PE_IndicatorButtonDropDown,
        PE_IndicatorCheckBox,
        PE_IndicatorCheckBoxMask,
        PE_IndicatorDockWindowResizeHandle,
        PE_IndicatorHeaderArrow,
        PE_IndicatorMenuCheckMark,
        PE_IndicatorProgressChunk,
        PE_IndicatorRadioButton,
        PE_IndicatorRadioButtonMask,
        PE_IndicatorSpinDown,
        PE_IndicatorSpinMinus,
        PE_IndicatorSpinPlus,
        PE_IndicatorSpinUp,
        PE_IndicatorToolBarHandle,
        PE_IndicatorToolBarSeparator,

        // do not add any values below/greater this
        PE_CustomBase = 0xf000000
    };

    virtual void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                               const QWidget *w = 0) const = 0;
    enum ControlElement {
        CE_PushButton,
        CE_PushButtonBevel,
        CE_PushButtonLabel,

        CE_CheckBox,
        CE_CheckBoxLabel,

        CE_RadioButton,
        CE_RadioButtonLabel,

        CE_TabBar,
        CE_TabBarTab,
        CE_TabBarLabel,

        CE_ProgressBar,
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
        CE_SpinBoxSlider,
        CE_SizeGrip,
        CE_Splitter,
        CE_RubberBand,
        CE_DockWindowTitle,

        CE_ScrollBarAddLine,
        CE_ScrollBarSubLine,
        CE_ScrollBarAddPage,
        CE_ScrollBarSubPage,
        CE_ScrollBarSlider,
        CE_ScrollBarFirst,
        CE_ScrollBarLast,

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

        SR_Q3DockWindowHandleRect,

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

        SR_HeaderLabel,
        SR_HeaderArrow,

        SR_PanelTab,

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
        SC_TitleBarContextHelpButton = 0x00000100,

        SC_ListView =              0x00000001,
        SC_ListViewBranch =        0x00000002,
        SC_ListViewExpand =        0x00000004,

        SC_All =                   0xffffffff
    };
    Q_DECLARE_FLAGS(SubControls, SubControl)
#ifdef QT_COMPAT
    typedef SubControls SCFlags;
#endif

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
        PM_MenuPanelWidth,
        PM_MenuTearoffHeight,
        PM_MenuDesktopFrameWidth,

        PM_MenuBarPanelWidth,
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

        PM_ToolBarFrameWidth,
        PM_ToolBarHandleExtent,
        PM_ToolBarItemSpacing,
        PM_ToolBarSeparatorExtent,
        PM_ToolBarExtensionExtent,

        PM_SpinBoxSliderHeight,

        PM_DefaultToplevelMargin,
        PM_DefaultChildMargin,
        PM_DefaultLayoutSpacing,

        // do not add any values below/greater than this
        PM_CustomBase = 0xf0000000
    };

    virtual int pixelMetric(PixelMetric metric, const QStyleOption *option = 0,
                            const QWidget *widget = 0) const = 0;

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
        CT_HeaderSection,
        // do not add any values below/greater than this
        CT_CustomBase = 0xf0000000
    };

    virtual QSize sizeFromContents(ContentsType ct, const QStyleOption *opt,
                                   const QSize &contentsSize, const QWidget *w = 0) const = 0;

    enum StyleHint {
        // ...
        // the general hints
        // ...
        // disabled text should be etched, ala Windows
        SH_EtchDisabledText,

#ifdef QT_COMPAT
        // the GUI style enum, argh!
        SH_GUIStyle = 1,
#endif

        // ...
        // widget specific hints
        // ...
        SH_ScrollBar_BackgroundMode = 2, // deprecated
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
#ifdef QT_COMPAT
        SH_UnderlineAccelerator = SH_UnderlineShortcut,
#endif

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

        // bool - if the menubar should have a menubar or not.
        SH_DrawMenuBarSeparator,

        // bool - If the titlebar should show a * for modified Windows
        SH_TitlebarModifyNotification,

        // FocusPolicy - The default focus policy for a button.
        SH_Button_FocusPolicy,

        // int - The border of the selection square
        SH_ColorDialog_SelectedColorBorder,

        // bool - Whether or not to dismiss the menubar on a second click or not
        SH_MenuBar_DismissOnSecondClick,

        // bool - Use the button border as the button spacing for a message box
        SH_MessageBox_UseBorderForButtonSpacing,

        // bool - Auto Raise for title bars
        SH_TitleBar_AutoRaise,

        // int - the popup delay for menus attached to tool buttons
        SH_ToolButton_PopupDelay,

        // Qt::IconSize - the preferred icon size for tool bars
        SH_ToolBar_IconSize,

        // do not add any values below/greater than this
        SH_CustomBase = 0xf0000000
    };

    virtual int styleHint(StyleHint stylehint, const QStyleOption *opt = 0,
                          const QWidget *widget = 0, QStyleHintReturn* returnData = 0) const = 0;


    enum StandardPixmap {
        SP_TitleBarMenuButton,
        SP_TitleBarMinButton,
        SP_TitleBarMaxButton,
        SP_TitleBarCloseButton,
        SP_TitleBarNormalButton,
        SP_TitleBarShadeButton,
        SP_TitleBarUnshadeButton,
        SP_TitleBarContextHelpButton,
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
        SP_ItemChecked,
        SP_ItemUnchecked,
        SP_ToolBarHorizontalExtensionButton,
        SP_ToolBarVerticalExtensionButton,
        SP_FileDialogStart,
        SP_FileDialogEnd,
        SP_FileDialogToParent,
        SP_FileDialogNewFolder,
        SP_FileDialogDetailedView,
        SP_FileDialogInfoView,
        SP_FileDialogContentsView,
        SP_FileDialogListView,
        SP_FileDialogBack,

        // do not add any values below/greater than this
        SP_CustomBase = 0xf0000000
    };

    virtual QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt = 0,
                                   const QWidget *widget = 0) const = 0;

    enum IconMode {
        IM_Disabled,
        IM_Active,

        // do not add any values below/greater than this
        IM_CustomBase = 0xf0000000
    };

    virtual QPixmap generatedIconPixmap(IconMode iconMode, const QPixmap &pixmap,
                                        const QStyleOption *opt) const = 0;

    static QRect visualRect(Qt::LayoutDirection direction, const QRect &boundingRect, const QRect &logicalRect);
    static QPoint visualPos(Qt::LayoutDirection direction, const QRect &boundingRect, const QPoint &logicalPos);
    static int positionFromValue(int min, int max, int val, int space, bool upsideDown = false);
    static int valueFromPosition(int min, int max, int pos, int space, bool upsideDown = false);
    static Qt::Alignment horizontalAlignment(Qt::LayoutDirection direction, Qt::Alignment align);

private:
    Q_DISABLE_COPY(QStyle)

protected:
#if defined(QT_COMPAT) && !defined(QT_NO_UNRESOLVED_EXTERNALS)
    // Cause a compile error when trying to use style functions that
    // accept QColorGroup arguments. Remove in Qt 5.x.
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
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QStyle::StyleFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QStyle::SubControls)

#endif // QT_NO_STYLE

#endif // QSTYLE_H
