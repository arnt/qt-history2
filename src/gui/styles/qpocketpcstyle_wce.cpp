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
#ifndef QT_NO_STYLE_POCKETPC

#define QT_NO_DIALOGBUTTONS // ### For now...

#include "qpocketpcstyle_wce.h"
#include <windows.h>

// Other
#include "qpainter.h"
#include "qdrawutil.h"
#include "qapplication.h"
#include "qpixmap.h"
#include "qpalette.h"
#include "qslider.h"
#include "qtabbar.h"
#include "qpushbutton.h"
#include "qcheckbox.h"
#include "qradiobutton.h"
#include "q3popupmenu.h"
#include "qprogressbar.h"
#include "qcombobox.h"
#include "qlistview.h"
#include "qbitmap.h"
#include "qtoolbutton.h"
#include "qheader.h"
#include "q3menubar.h"
#include "qstatusbar.h"
#include "qtable.h"
#include "qtabwidget.h"

// Private headerfiles
#include "private/qtitlebar_p.h"
#include "private/qdialogbuttons_p.h"
#include "qcleanuphandler.h"

/*!
    \class QPocketPCStyle
    \brief The QPocketPCStyle class provides a Microsoft PocketPC-like look and feel.

    \ingroup appearance
    \internal

    This is Qt's default GUI style on the PocketPC.
*/

    #define CE_SEPHEIGHT                1 // separator item height
    #define CE_TABSPACING                6 // space between text and tab
    #define CE_ITEMFRAME                1 // menu item frame width
    #define CE_ITEMHMARGIN                2 // menu item hor text margin
    #define CE_ITEMVMARGIN                1 // menu item ver text margin
    #define CE_ARROWHMARGIN                4 // arrow horizontal margin
    #define CE_RIGHTBORDER                6 // right border on pocketpc
    #define CE_CHECKMARKHMARGIN          1 // horiz. margins of check mark
    #define CE_CHECKMARKWIDTH                6 // checkmarks width on windows

    #define CE_TITLEBAR_PAD                3
    #define CE_TITLEBAR_SEPARATION        1
    #define CE_TITLEBAR_PIXMAP_WIDTH        12
    #define CE_TITLEBAR_PIXMAP_HEIGHT        12
    #define CE_TITLEBAR_CONTROL_WIDTH        (CE_TITLEBAR_PAD + CE_TITLEBAR_PIXMAP_WIDTH)
    #define CE_TITLEBAR_CONTROL_HEIGHT        (CE_TITLEBAR_PAD + CE_TITLEBAR_PIXMAP_HEIGHT)

    // the active painter, if any... this is used as an optimzation to
    // avoid creating a painter if we have an active one (since
    // QStyle::itemRect() needs a painter to operate correctly
    static QPainter *activePainter = 0;

    // CheckList Controller XPM pixmap
    static const char * const check_list_controller_xpm[] = {
    "16 16 4 1",
    "        c None",
    ".        c #000000000000",
    "X        c #FFFFFFFF0000",
    "o        c #C71BC30BC71B",
    "                ",
    "                ",
    " ..........     ",
    " .XXXXXXXX.     ",
    " .XXXXXXXX.oo   ",
    " .XXXXXXXX.oo   ",
    " .XXXXXXXX.oo   ",
    " .XXXXXXXX.oo   ",
    " .XXXXXXXX.oo   ",
    " .XXXXXXXX.oo   ",
    " .XXXXXXXX.oo   ",
    " ..........oo   ",
    "   oooooooooo   ",
    "   oooooooooo   ",
    "                ",
    "                "};

    static const char * const qt_close_xpm[] = {
    "12 12 2 1",
    "# c #000000",
    ". c None",
    "............",
    "............",
    "..##....##..",
    "...##..##...",
    "....####....",
    ".....##.....",
    "....####....",
    "...##..##...",
    "..##....##..",
    "............",
    "............",
    "............"};

    static const char * const qt_maximize_xpm[]={
    "12 12 2 1",
    "# c #000000",
    ". c None",
    "............",
    "............",
    ".##########.",
    ".##########.",
    ".#........#.",
    ".#........#.",
    ".#........#.",
    ".#........#.",
    ".#........#.",
    ".#........#.",
    ".##########.",
    "............"};


    static const char * const qt_minimize_xpm[] = {
    "12 12 2 1",
    "# c #000000",
    ". c None",
    "............",
    "............",
    "............",
    "............",
    "............",
    "............",
    "............",
    "...######...",
    "...######...",
    "............",
    "............",
    "............"};

    #if 0 // ### not used???
    static const char * const qt_normalize_xpm[] = {
    "12 12 2 1",
    "# c #000000",
    ". c None",
    "............",
    "...#######..",
    "...#######..",
    "...#.....#..",
    ".#######.#..",
    ".#######.#..",
    ".#.....#.#..",
    ".#.....###..",
    ".#.....#....",
    ".#.....#....",
    ".#######....",
    "............"};
    #endif

    static const char * const qt_normalizeup_xpm[] = {
    "12 12 2 1",
    "# c #000000",
    ". c None",
    "............",
    "...#######..",
    "...#######..",
    "...#.....#..",
    ".#######.#..",
    ".#######.#..",
    ".#.....#.#..",
    ".#.....###..",
    ".#.....#....",
    ".#.....#....",
    ".#######....",
    "............"};


    static const char * const qt_shade_xpm[] = {
    "12 12 2 1",
    "# c #000000",
    ". c None",
    "............",
    "............",
    "............",
    "............",
    "............",
    ".....#......",
    "....###.....",
    "...#####....",
    "..#######...",
    "............",
    "............",
    "............"};

    static const char * const qt_unshade_xpm[] = {
    "12 12 2 1",
    "# c #000000",
    ". c None",
    "............",
    "............",
    "............",
    "............",
    "..#######...",
    "...#####....",
    "....###.....",
    ".....#......",
    "............",
    "............",
    "............",
    "............"};

    static const char * dock_window_close_xpm[] = {
    "8 8 2 1",
    "# c #000000",
    ". c None",
    "##....##",
    ".##..##.",
    "..####..",
    "...##...",
    "..####..",
    ".##..##.",
    "##....##",
    "........"};

    // >=========== Convenience functions [start] ==================<
    #ifndef QT_NO_RANGECONTROL
    static int qPositionFromValue(const QRangeControl * rc, int logical_val, int span)
    {
        if (span <= 0 || logical_val < rc->minValue() ||
            rc->maxValue() <= rc->minValue())
            return 0;
        if (logical_val > rc->maxValue())
            return span;

        uint range = rc->maxValue() - rc->minValue();
        uint p = logical_val - rc->minValue();

        if (range > (uint)INT_MAX/4096) {
            const int scale = 4096*2;
            return ((p/scale) * span) / (range/scale);
            // ### the above line is probably not 100% correct
            // ### but fixing it isn't worth the extreme pain...
        } else if (range > (uint)span) {
            return (2*p*span + range) / (2*range);
        } else {
            uint div = span / range;
            uint mod = span % range;
            return p*div + (2*p*mod + range) / (2*range);
        }
        //equiv. to (p*span)/range + 0.5
        // no overflow because of this implicit assumption:
        // span <= 4096
    }
    #endif // QT_NO_RANGECONTROL


    // -------------------------------------------------------
    Qt::Dock QPocketPCStyle::findLocation(QPainter *p) const
    {
        if (p && p->device()->devType() == QInternal::Widget) {
            QWidget *w = (QWidget*)p->device();
            QWidget *p = w->parentWidget();
            return findLocation(p);
        }
        return (Qt::Dock)0;
    }


    // -------------------------------------------------------
    Qt::Dock QPocketPCStyle::findLocation(QWidget *p) const
    {
        Qt::Dock placement = Qt::DockTop;

        // Is docked
        if (::qobject_cast<QToolBar*>(p) || ::qobject_cast<QDockWindow*>(p)) {
            QDockWindow *dw = ::qobject_cast<QDockWindow*>(p);

            // It's not docked, so no panel
            if (dw->place() == QDockWindow::OutsideDock)
                return (Qt::Dock)0;

            // Find main window, so we can determine where it's docked.
#ifndef Q_OS_TEMP
            if (::qobject_cast<QMainWindow*>(dw->window())) {
                QMainWindow *mw = ::qobject_cast<QMainWindow*>(dw->window());
#else
            if (::qobject_cast<QCEMainWindow*>(dw->window())) {
                QCEMainWindow *mw = ::qobject_cast<QCEMainWindow*>(dw->window());
#endif // Q_OS_TEMP
                if (mw) {
                    bool nl;
                    int index, extraOffset;
                    mw->getLocation(dw, placement, index, (bool)nl, extraOffset);
                }
            }
        } else if (::qobject_cast<QDockArea*>(p)) {
            QDockArea *da = ::qobject_cast<QDockArea*>(p);
            // Find main window, so we can determine where it's docked.
#ifndef Q_OS_TEMP
            if (::qobject_cast<QMainWindow*>(da->window())) {
                QMainWindow *mw = ::qobject_cast<QMainWindow*>(da->window());
#else
            if (::qobject_cast<QCEMainWindow*>(da->window())) {
                QCEMainWindow *mw = ::qobject_cast<QCEMainWindow*>(da->window());
#endif // Q_OS_TEMP
                if (mw) {
                    if (da == mw->topDock()) return Qt::DockTop;
                    if (da == mw->bottomDock()) return Qt::DockBottom;
                    if (da == mw->leftDock()) return Qt::DockLeft;
                    if (da == mw->rightDock()) return Qt::DockRight;
                    return Qt::DockBottom; // Probably MenuDock
                } else{
                    return (Qt::Dock)0;
                }
            }
        }
        return placement;
    }
    // >=========== Convenience functions [end] ====================<


/*!
        Constructs a QPocketPCStyle
*/
QPocketPCStyle::QPocketPCStyle()
{
    activePainter = 0;
    gotOriginal = 0;
}


QPocketPCStyle::~QPocketPCStyle()
{
    activePainter = 0;
}


void QPocketPCStyle::modifyOriginalPalette()
{
    originalPal = qApp->palette();

    QPalette pal = originalPal;
    pal.setColor(QPalette::Foreground,             QColor(0x00, 0x00, 0x00));
    pal.setColor(QPalette::Button,             QColor(0xd9, 0xcc, 0xc0));
    pal.setColor(QPalette::Light,             QColor(0xff, 0xff, 0xff));
    pal.setColor(QPalette::Dark,             QColor(0x80, 0x80, 0x80));
    pal.setColor(QPalette::Mid,             QColor(0x90, 0x88, 0x80));
    pal.setColor(QPalette::Text,             QColor(0x00, 0x00, 0x00));
    pal.setColor(QPalette::Base,             QColor(0xff, 0xff, 0xff));
    pal.setColor(QPalette::Background,             QColor(0xd9, 0xcc, 0xc0));
    pal.setColor(QPalette::Midlight,             QColor(0xff, 0xff, 0xff));
    pal.setColor(QPalette::BrightText,             QColor(0xff, 0xff, 0xff));
    pal.setColor(QPalette::ButtonText,             QColor(0x00, 0x00, 0x00));
    pal.setColor(QPalette::Shadow,             QColor(0x00, 0x00, 0x00));
    pal.setColor(QPalette::Highlight,             QColor(0x00, 0x00, 0x99));
    pal.setColor(QPalette::HighlightedText, QColor(0xff, 0xff, 0xff));
    //newPal.setActive(pal);
    //newPal.setInactive(pal);
    qApp->setPalette(pal, false);

    gotOriginal = true;
}


void QPocketPCStyle::polish(QApplication* app)
{
#ifndef Q_OS_TEMP
    modifyOriginalPalette();
#endif // Q_OS_TEMP
}


void QPocketPCStyle::polish(QWidget *w)
{
#ifndef Q_OS_TEMP
    if (!gotOriginal)
        modifyOriginalPalette();

    //Make widget grab "natural palette", which now
    //contains the colors typical to PocketPC 2003
    //if (!w->ownPalette())
        //w->unsetPalette();
#endif // Q_OS_TEMP

    // If widget has own palette, we assume the developer knows what
    // he's doing, so we don't mess around with the widget settings
    /*
    if (w && !w->ownPalette()) {
        if (::qobject_cast<QToolBar*>(w) ||
            ::qobject_cast<Q3MenuBar*>(w) ||
            ::qobject_cast<QStatusBar*>(w) ||
            ::qobject_cast<QDockWindow*>(w) ||
            ::qobject_cast<QDockArea*>(w) ||
            ::qobject_cast<QTable*>(w) ||
            ::qobject_cast<QTabWidget*>(w)) {
            w->setBackgroundMode(Qt::PaletteBackground);
            QToolBar *tb = 0;
            if ((tb = ::qobject_cast<QToolBar*>(w)))
                tb->boxLayout()->setAlignment(Qt::AlignLeft);
        } else if (::qobject_cast<QToolButton*>(w) &&
                    ::qobject_cast<QToolBar*>(w->parentWidget())) {
            w->setBackgroundMode(Qt::PaletteBackground);
        } else {
            w->setBackgroundMode(Qt::PaletteBase);
        }
    }
    */
}


void QPocketPCStyle::unPolish(QApplication* app)
{
#ifndef Q_OS_TEMP
    // Reset the palette to the original
    app->setPalette(originalPal, false);
    gotOriginal = false;
#endif // Q_OS_TEMP
}

void QPocketPCStyle::unpolish(QWidget *w)
{
#ifndef Q_OS_TEMP
    if (gotOriginal)
        unPolish(qApp);

    // Make widget grab "natural palette", which now
    // contains the colors typical to PocketPC 2003
    //if (!w->ownPalette())
        //w->unsetPalette();
#endif // Q_OS_TEMP

    //if (!w->ownPalette())
        //w->setBackgroundMode(Qt::PaletteBackground);
}


int QPocketPCStyle::pixelMetric(PixelMetric pixelmetric, const QStyleOption * /* option */,
                                const QWidget *widget) const
{
    int ret = 0;
    switch (pixelmetric) {

#ifndef QT_NO_SLIDER
    case PM_SliderLength:
        #define PM_SLIDERLENGTH 9
        ret = PM_SLIDERLENGTH;
        break;

    case PM_SliderThickness:
        #define PM_SLIDERTHICKNESS 17
        ret = PM_SLIDERTHICKNESS;
        break;

    // Returns the number of pixels to use for the business part of the
    // slider (i.e., the non-tickmark portion). The remaining space is shared
    // equally between the tickmark regions.
    case PM_SliderControlThickness:
        {
            if (!widget) break; // Don't know

            const QSlider *sl = (const QSlider *)widget;
            QSlider::TickPosition ticktype = sl->tickmarks();
            int n = 0;
            int space = (sl->orientation() == Qt::Horizontal)
                        ? sl->height() : sl->width();

            if (ticktype & QSlider::Above) n++;
            if (ticktype & QSlider::Below) n++;
            if (!n) { ret = space; break; } // Shortcut

            int thick = 6; // Magic constant to get 5 + 16 + 5
            if (ticktype != QSlider::Both &&
                 ticktype != QSlider::NoMarks)
                thick += PM_SLIDERLENGTH / 4; // pixelMetric(PM_SliderLength, sl) / 4;

            space -= thick;
            //### the two sides may be unequal in size
            if (space > 0)
                thick += (space * 2) / (n + 2);
            ret = thick;
            break;
        }

    case PM_SliderTickmarkOffset:
        {
            if (!widget) break; // Don't know

            const QSlider * sl = (const QSlider *) widget;
            QSlider::TickPosition ticktype = sl->tickmarks();
            int space = (sl->orientation() == Qt::Horizontal)
                        ? sl->height() : sl->width();
            int thickness = PM_SLIDERTHICKNESS; // pixelMetric(PM_SliderControlThickness, sl);
            if (ticktype == QSlider::Both)
                ret = (space - thickness) / 2;
            else if (ticktype == QSlider::Above)
                ret = space - thickness;
            else
                ret = 0;
            break;
        }

    case PM_SliderSpaceAvailable:
        {
            if (!widget) break; // Don't know

            const QSlider * sl = (const QSlider *) widget;
            if (sl->orientation() == Qt::Horizontal)
                ret = sl->width() - PM_SLIDERLENGTH; // pixelMetric(PM_SliderLength, sl);
            else
                ret = sl->height() - PM_SLIDERLENGTH; //pixelMetric(PM_SliderLength, sl);
            break;
        }
#endif // QT_NO_SLIDER

    case PM_SplitterWidth:
        #define PM_SPLITTERWIDTH 6
        ret = PM_SPLITTERWIDTH;
        break;

    case PM_ScrollBarSliderMin:
        #define PM_SCROLLBARSLIDERMIN 9
        ret = PM_SCROLLBARSLIDERMIN;
        break;

    case PM_ButtonMargin:
        #define PM_BUTTONMARGIN 8
        ret = PM_BUTTONMARGIN;
        break;

    case PM_ButtonDefaultIndicator:
        #define PM_BUTTONDEFAULTINDICATOR 0
        ret = PM_BUTTONDEFAULTINDICATOR;
        break;

    case PM_MenuButtonIndicator:
        #define PM_MENUBUTTONINDICATOR 8
        ret = PM_MENUBUTTONINDICATOR;
        break;

    case PM_ButtonShiftHorizontal:
        #define PM_BUTTONSHIFTHORIZONTAL 0
        ret = PM_BUTTONSHIFTHORIZONTAL;
        break;

    case PM_ButtonShiftVertical:
        #define PM_BUTTONSHIFTVERTICAL 0
        ret = PM_BUTTONSHIFTVERTICAL;
        break;

    case PM_SpinBoxFrameWidth:
        #define PM_SPINBOXFRAMEWIDTH 1
        ret = PM_SPINBOXFRAMEWIDTH;
        break;

    case PM_DefaultFrameWidth:
        #define PM_DEFAULTFRAMEWIDTH 1
        ret = PM_DEFAULTFRAMEWIDTH;
        break;

    case PM_ScrollBarExtent:
        #define PM_SCROLLBAREXTENT 13
        ret = PM_SCROLLBAREXTENT;
        break;

    case PM_MaximumDragDistance:
        #define PM_MAXIMUMDRAGDISTANCE 10
        ret = PM_MAXIMUMDRAGDISTANCE;
        break;

    case PM_DockWindowSeparatorExtent:
        #define PM_DOCKWINDOWSEPARATOREXTENT 1
        ret = PM_DOCKWINDOWSEPARATOREXTENT;
        break;

    // Size of the move handle, for movable dockwindows
    case PM_DockWindowHandleExtent:
        #define PM_DOCKWINDOWHANDLEEXTENT 5
        ret = PM_DOCKWINDOWHANDLEEXTENT;
        break;

    case PM_DockWindowFrameWidth:
        #define PM_DOCKWINDOWFRAMEWIDTH 1
        ret = PM_DOCKWINDOWFRAMEWIDTH;
        break;

    case PM_MenuBarFrameWidth:
        #define PM_MENUBARFRAMEWIDTH 1
        ret = PM_MENUBARFRAMEWIDTH;
        break;

#ifndef QT_NO_TABBAR
    case PM_TabBarTabOverlap:
        #define PM_TABBARTABOVERLAP 1
        ret = PM_TABBARTABOVERLAP;
        break;

    case PM_TabBarBaseHeight:
        #define PM_TABBARBASEHEIGHT 0
        ret = PM_TABBARBASEHEIGHT;
        break;

    case PM_TabBarBaseOverlap:
        #define PM_TABBARBASEOVERLAP 0
        ret = PM_TABBARBASEOVERLAP;
        break;

    case PM_TabBarTabHSpace:
        #define PM_TABBARTABHSPACE 20
        ret = PM_TABBARTABHSPACE;
        break;

    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
        #define PM_TABBARTABSHIFTHORIZONTAL 2
        #define PM_TABBARTABSHIFTVERTICAL PM_TABBARTABSHIFTHORIZONTAL
        ret = PM_TABBARTABSHIFTVERTICAL;
        break;

    case PM_TabBarTabVSpace:
        {
            const QTabBar * tb = (const QTabBar *) widget;
            if (tb->shape() == QTabBar::RoundedAbove ||
                 tb->shape() == QTabBar::RoundedBelow)
                ret = 10;
            else
                ret = 0;
            break;
        }
#endif

    case PM_ProgressBarChunkWidth:
        #define PM_PROGRESSBARCHUNKWIDTH 9
        ret = PM_PROGRESSBARCHUNKWIDTH;
        break;

    case PM_IndicatorWidth:
        #define PM_INDICATORWIDTH 15
        ret = PM_INDICATORWIDTH;
        break;

    case PM_IndicatorHeight:
        #define PM_INDICATORHEIGHT 15
        ret = PM_INDICATORHEIGHT;
        break;

    case PM_ExclusiveIndicatorWidth:
        #define PM_EXCLUSIVEINDICATORWIDTH 15
        ret = PM_EXCLUSIVEINDICATORWIDTH;
        break;

    case PM_ExclusiveIndicatorHeight:
        #define PM_EXCLUSIVEINDICATORHEIGHT 15
        ret = PM_EXCLUSIVEINDICATORHEIGHT;
        break;

#if defined(Q_WS_WIN)
    case PM_TitleBarHeight:
        if (widget && (widget->testWFlags(Qt::WA_WState_Tool) || ::qobject_cast<QDockWindow*>(widget))) {
            // MS always use one less than they say
            ret = GetSystemMetrics(SM_CYCAPTION) - 1;
        } else {
            ret = GetSystemMetrics(SM_CYCAPTION) - 1;
        }
        break;
#endif // Q_WS_WIN

    case PM_DialogButtonsSeparator:
        #define PM_DIALOGBUTTONSSEPARATOR 5
        ret = PM_DIALOGBUTTONSSEPARATOR;
        break;

    case PM_DialogButtonsButtonWidth:
        #define PM_DIALOGBUTTONSBUTTONWIDTH 70
        ret = PM_DIALOGBUTTONSBUTTONWIDTH;
        break;

    case PM_DialogButtonsButtonHeight:
        #define PM_DIALOGBUTTONSBUTTONHEIGHT 30
        ret = PM_DIALOGBUTTONSBUTTONHEIGHT;
        break;

    case PM_CheckListControllerSize:
    case PM_CheckListButtonSize:
        #define PM_CHECKLISTCONTROLLERSIZE 16
        #define PM_CHECKLISTBUTTONSIZE PM_CHECKLISTCONTROLLERSIZE
        ret = PM_CHECKLISTBUTTONSIZE;
        break;

    case PM_MenuHMargin:
    case PM_MenuVMargin:
        #define PM_POPUPMENUFRAMEHORIZONTALEXTRA 0
        #define PM_POPUPMENUFRAMEVERTICALEXTRA PM_POPUPMENUFRAMEHORIZONTALEXTRA
        ret = 0;
        break;

    case PM_HeaderMargin:
        #define PM_HEADERMARGIN 4
        ret = PM_HEADERMARGIN;
        break;

    case PM_HeaderMarkSize:
        #define PM_HEADERMARKSIZE 32
        ret = PM_HEADERMARKSIZE;
        break;

    case PM_HeaderGripMargin:
        #define PM_HEADERGRIPMARGIN 4
        ret = PM_HEADERGRIPMARGIN;
        break;

    case PM_MDIMinimizedWidth:
        #define PM_MDIMINIMIZEDWIDTH 196
        ret = PM_MDIMINIMIZEDWIDTH;
        break;

    case PM_MDIFrameWidth:
        #define PM_MDIFRAMEWIDTH 3
        ret = PM_MDIFRAMEWIDTH;
        break;

    // No opinion here, so return 0
    case PM_PopupMenuScrollerHeight:
        #define PM_POPUPMENUSCROLLERHEIGHT 5
        ret = PM_POPUPMENUSCROLLERHEIGHT;
        break;

    default:
        // This debug message is only here to make sure we handle the complete style properly
        // We might remove this debug message before a full release
        qDebug("*** Not Implemented Yet ***  pixelMetric(pixelmetric[0x%08x], w)", pixelmetric);
    }
    return ret;
}


int QPocketPCStyle::styleHint(StyleHint stylehint,
                              const QWidget *widget,
                              const QStyleOption &opt,
                              QStyleHintReturn *stylehint_return) const
{
    int ret = 0;

    switch (stylehint) {
    // Middle clicking on a scrollbar causes the thumb to jump to that position
    // This is only for convenience for when using the emulator, since that's
    // usually only the case when you use a mouse with three buttons.
    case SH_ScrollBar_MiddleClickAbsolutePosition:
    // PopupMenus must support scrolling, since the screen is so small
    case SH_Menu_Scrollable:
    // Cursor blinks when text is selected
    case SH_BlinkCursorWhenTextSelected:
    // Qt::RichText selections extend the full width of the document
    case SH_RichText_FullWidthSelection:
    // Toolbox page is bolded (rarely used on CE)
    case SH_ToolBox_SelectedPageTitleBold:
    // It's possible to still scroll when stylus is not ontop of the control
    case SH_ScrollBar_ScrollWhenPointerLeavesControl:
    // Use Mouse Tracking on the combobox dropdowns
    case SH_ComboBox_ListMouseTracking:
    // Sliders snaps to values on CE (no "in between" values)
    case SH_Slider_SnapToValue:
    // Diverse other settings defaulting to True/On
    case SH_EtchDisabledText:
    case SH_PrintDialog_RightAlignButtons:
    case SH_MainWindow_SpaceBelowMenuBar:
    case SH_FontDialog_SelectAssociatedText:
    case SH_Menu_AllowActiveAndDisabled:
    case SH_MenuBar_AltKeyNavigation:
    case SH_Workspace_FillSpaceOnMaximize:
        ret = 1;
        break;

#ifndef QT_NO_DIALOGBUTTONS
    // Default dialog button is the Accept button (OK)
    case SH_DialogButtons_DefaultButton:
        ret = QDialogButtons::Accept;
        break;
#endif

    // Label of the GroupBox is Centered vertically
    case SH_GroupBox_TextLabelVerticalAlignment:
        ret = Qt::AlignVCenter;
        break;

    // Label color of the GroupBox is the foreground color, or black
    case SH_GroupBox_TextLabelColor:
        ret = (int) (widget ? widget->palette().color(QPalette::Foreground).rgb() : 0);
        break;

    // Use the MouseButtonPress event to select Tabs and Expand ListViewItems
    case SH_ListViewExpand_SelectMouseType:
    case SH_TabBar_SelectMouseType:
        ret = QEvent::MouseButtonPress;
        break;

#ifdef QT3_SUPPORT
    // For special widget cases, use the windows behavior
    case SH_GUIStyle:
        ret = Qt::WindowsStyle;
        break;
#endif

    // Scrollbars have Qt::PaletteBase as their background
    case SH_ScrollBar_BackgroundMode:
        ret = QPalette::Base;
        break;

    // Both TabBar and Headers have Left alignment
    case SH_TabBar_Alignment:
    case SH_Header_ArrowAlignment:
        ret = Qt::AlignLeft;
        break;

    // Delay Sub Popupmenus for 256ms
    case SH_Menu_SubMenuPopupDelay:
        ret = 256;
        break;

    // Percentage is center aligned on Progress Bar
    case SH_ProgressDialog_TextLabelAlignment:
        ret = Qt::AlignCenter;
        break;

    // Use middle dark color of widget, or black
    case SH_Table_GridLineColor:
        ret = (int) (widget ? widget->palette().color(QPalette::Mid).rgb() : 0);
        break;

    // Use simple * character as password
    case SH_LineEdit_PasswordCharacter:
        ret = '*';
        break;

    // Scrollviews draws frame around whole view
    case SH_ScrollView_FrameOnlyAroundContents:
    // PopupMenus are not sloppy on CE
    case SH_Menu_SloppySubMenus:
    // Widgets do not share activation with floating modeless dialogs
    case SH_Widget_ShareActivation:
    // Selected items are not grayed out when the widget looses focus
    case SH_ItemView_ChangeHighlightOnFocus:
    // Styles outside of thumb will page, not move to absolute position
    case SH_ScrollBar_LeftClickAbsolutePosition:
    // Scrollbar does not stop when thumb reaches stylus point
    case SH_ScrollBar_StopMouseOverSlider:
    // TabBar should not suggest a size to prevent scroll arrows
    case SH_TabBar_PreferNoArrows:
    // Don't show underlines on accelerators
    case SH_UnderlineShortcut:
    // Comboboxes do not use popup menus
    case SH_ComboBox_Popup:
    // Menus and popups do not use mousetracking
    case SH_MenuBar_MouseTracking:
    case SH_Menu_MouseTracking:
    // Workspace children have borders
    case SH_TitleBar_NoBorder:
        break;

    default:
        // This debug message is only here to make sure we handle the complete style properly
        // We might remove this debug message before a full release
        qDebug("*** Not Implemented Yet ***  styleHint(stylehint[0x%08x], w, opt[0x%08x], stylehint_return)", stylehint, opt);
    }
    return ret;
}


QRect QPocketPCStyle::subRect(SubRect             subrect,
                              const QWidget *widget) const
{
#if defined(QT_CHECK_STATE)
    if (! widget) {
        qWarning("QPocketPCStyle::subRect: widget parameter cannot be zero!");
        return QRect();
    }
#endif
    QRect rect, wrect(widget->rect());

    switch (subrect) {

#ifndef QT_NO_SLIDER
    case SR_SliderFocusRect:
        {
            rect = widget->rect();
            rect.adjust(2, 2, -2, -2);
            break;
        }
#endif // QT_NO_SLIDER

#ifndef QT_NO_TOOLBOX
    case SR_ToolBoxTabContents:
        {
            rect = widget->rect();
            break;
        }
#endif // QT_NO_TOOLBOX

#ifndef QT_NO_DIALOGBUTTONS
    case SR_DialogButtonAbort:
    case SR_DialogButtonRetry:
    case SR_DialogButtonIgnore:
    case SR_DialogButtonAccept:
    case SR_DialogButtonReject:
    case SR_DialogButtonApply:
    case SR_DialogButtonHelp:
    case SR_DialogButtonAll:
    case SR_DialogButtonCustom:
        {
            QDialogButtons::Button srch = QDialogButtons::None;
            switch (r) {
            case SR_DialogButtonAccept:
                srch = QDialogButtons::Accept; break;
            case SR_DialogButtonReject:
                srch = QDialogButtons::Reject; break;
            case SR_DialogButtonAll:
                srch = QDialogButtons::All; break;
            case SR_DialogButtonApply:
                srch = QDialogButtons::Apply; break;
            case SR_DialogButtonHelp:
                srch = QDialogButtons::Help; break;
            case SR_DialogButtonRetry:
                srch = QDialogButtons::Retry; break;
            case SR_DialogButtonIgnore:
                srch = QDialogButtons::Ignore; break;
            case SR_DialogButtonAbort:
                srch = QDialogButtons::Abort; break;
            }

            const int bwidth = PM_DIALOGBUTTONSBUTTONWIDTH;; // pixelMetric(PM_DialogButtonsButtonWidth, widget),
                     bheight = PM_DIALOGBUTTONSBUTTONHEIGHT; // pixelMetric(PM_DialogButtonsButtonHeight, widget),
                      bspace = PM_DIALOGBUTTONSSEPARATOR; // pixelMetric(PM_DialogButtonsSeparator, widget),
                          fw = PM_DEFAULTFRAMEWIDTH; // pixelMetric(PM_DefaultFrameWidth, widget);
            const QDialogButtons *dbtns = (const QDialogButtons *) widget;
            int start = fw;

            if (dbtns->orientation() == Qt::Horizontal)
                start = wrect.right() - fw;

            QDialogButtons::Button btns[] =
                   {
                     QDialogButtons::All, QDialogButtons::Reject, QDialogButtons::Accept, //reverse order (right to left)
                     QDialogButtons::Apply, QDialogButtons::Retry, QDialogButtons::Ignore,
                     QDialogButtons::Abort, QDialogButtons::Help
                   };

            for (unsigned int i = 0, cnt = 0; i < (sizeof(btns)/sizeof(btns[0])); i++) {
                if (dbtns->isButtonVisible(btns[i])) {
                    QSize szH = dbtns->sizeHint(btns[i]);
                    int mwidth = qMax(bwidth, szH.width()), mheight = qMax(bheight, szH.height());
                    if(dbtns->orientation() == Qt::Horizontal) {
                        start -= mwidth;
                        if(cnt)
                            start -= bspace;
                    } else if(cnt) {
                        start += mheight;
                        start += bspace;
                    }
                    cnt++;
                    if (btns[i] == srch) {
                        if (dbtns->orientation() == Qt::Horizontal)
                            return QRect(start, wrect.bottom() - fw - mheight, mwidth, mheight);
                        else
                            return QRect(fw, start, mwidth, mheight);
                    }
                }
            }
            if(r == SR_DialogButtonCustom) {
                if(dbtns->orientation() == Qt::Horizontal)
                    return QRect(fw, fw, start - fw - bspace, wrect.height() - (fw*2));
                else
                    return QRect(fw, start, wrect.width() - (fw*2), wrect.height() - start - (fw*2));
            }
            return QRect();
        }
#endif //QT_NO_DIALOGBUTTONS

#ifndef QT_NO_PUSHBUTTON
    case SR_PushButtonContents:
        {
            const QPushButton *button = (const QPushButton *) widget;
            int dx = 1;
            if (button->isDefault() || button->autoDefault())
                ++dx;
            rect = wrect;
            rect.adjust(dx, dx, -dx, -dx);
            break;
        }

    case SR_PushButtonFocusRect:
        {
            const QPushButton *button = (const QPushButton *) widget;
            int dx = 1;
            if (button->isDefault() || button->autoDefault())
                ++dx;
            rect = wrect;
            rect.adjust(dx, dx, -dx, -dx);
            break;
        }
#endif // QT_NO_PUSHBUTTON

#ifndef QT_NO_CHECKBOX
    case SR_CheckBoxIndicator:
        {
            int h = PM_INDICATORHEIGHT; // pixelMetric(PM_IndicatorHeight, widget);
            rect.setRect(0, (wrect.height() - h) / 2,
                         PM_INDICATORWIDTH, h); // pixelMetric(PM_IndicatorWidth, widget)
            break;
        }

    case SR_CheckBoxContents:
        {
            QRect ir = subRect(SR_CheckBoxIndicator, widget);
            rect.setRect(ir.right() + 6, wrect.y(),
                         wrect.width() - ir.width() - 6, wrect.height());
            break;
        }

    case SR_CheckBoxFocusRect:
        {
            const QCheckBox *checkbox = (const QCheckBox *) widget;
            if (!checkbox->pixmap() && checkbox->text().isEmpty()) {
                rect = subRect(SR_CheckBoxIndicator, widget);
                rect.adjust(1, 1, -1, -1);
                break;
            }
            QRect cr = subRect(SR_CheckBoxContents, widget);

            // don't create a painter if we have an active one
            QPainter *p = activePainter ? 0 : new QPainter(checkbox);
            rect = itemRect((activePainter ? activePainter : p),
                            cr, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic,
                            checkbox->isEnabled(),
                            checkbox->pixmap(),
                            checkbox->text());
            delete p;

            rect.adjust(-1, -1, 1, 2);
            rect = rect.intersect(wrect);
            break;
        }
#endif // QT_NO_CHECKBOX

#ifndef QT_NO_RADIOBUTTON
    case SR_RadioButtonIndicator:
        {
            int h = PM_EXCLUSIVEINDICATORHEIGHT; //pixelMetric(PM_ExclusiveIndicatorHeight, widget);
            rect.setRect(0, (wrect.height() - h) / 2,
                         PM_EXCLUSIVEINDICATORWIDTH, h); //pixelMetric(PM_ExclusiveIndicatorWidth, widget)
            break;
        }

    case SR_RadioButtonContents:
        {
            QRect ir = subRect(SR_RadioButtonIndicator, widget);
            rect.setRect(ir.right() + 6, wrect.y(),
                         wrect.width() - ir.width() - 6, wrect.height());
            break;
        }

    case SR_RadioButtonFocusRect:
        {
            const QRadioButton *radiobutton = (const QRadioButton *) widget;
            // If only the radio button, with no extra pixmap, or text
            if (!radiobutton->pixmap() && radiobutton->text().isEmpty()) {
                rect = subRect(SR_RadioButtonIndicator, widget);
                rect.adjust(1, 1, -1, -2);
                break;
            }

            QRect cr = subRect(SR_RadioButtonContents, widget);

            // don't create a painter if we have an active one
            QPainter *p = activePainter ? 0 : new QPainter(radiobutton);
            rect = itemRect((activePainter ? activePainter : p),
                            cr, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic,
                            radiobutton->isEnabled(),
                            radiobutton->pixmap(),
                            radiobutton->text());
            delete p;

            rect.adjust(-2, -1, 1, 2);
            rect = rect.intersect(wrect);
            break;
        }
#endif // QT_NO_RADIOBUTTON

    case SR_ComboBoxFocusRect:
        rect.setRect(3, 3, widget->width()-6-16, widget->height()-6);
        break;

#ifndef QT_NO_MAINWINDOW
    case SR_DockWindowHandleRect:
        {
            if (! widget->parentWidget())
                break;

            const QDockWindow * dw = (const QDockWindow *) widget->parentWidget();

            if (!dw->area() || !dw->isCloseEnabled())
                rect.setRect(0, 0, widget->width(), widget->height());
            else {
                if (dw->area()->orientation() == Qt::Horizontal)
                    rect.setRect(0, 15, widget->width(), widget->height() - 15);
                else
                    rect.setRect(0, 1, widget->width() - 15, widget->height() - 1);
            }
            break;
        }
#endif // QT_NO_MAINWINDOW

#ifndef QT_NO_PROGRESSBAR
    case SR_ProgressBarGroove:
    case SR_ProgressBarContents:
        {
            QFontMetrics fm((widget ? widget->fontMetrics() :
                               QApplication::fontMetrics()));
            const QProgressBar *progressbar = (const QProgressBar *) widget;
            int textw = 0;
            if (progressbar->percentageVisible())
                textw = fm.width("100%") + 6;

            if (progressbar->indicatorFollowsStyle() ||
                ! progressbar->centerIndicator())
                rect.setCoords(wrect.left(), wrect.top(),
                               wrect.right() - textw, wrect.bottom());
            else
                rect = wrect;
            break;
        }

    case SR_ProgressBarLabel:
        {
            QFontMetrics fm((widget ? widget->fontMetrics() :
                               QApplication::fontMetrics()));
            const QProgressBar *progressbar = (const QProgressBar *) widget;
            int textw = 0;
            if (progressbar->percentageVisible())
                textw = fm.width("100%") + 6;

            if (progressbar->indicatorFollowsStyle() ||
                ! progressbar->centerIndicator())
                rect.setCoords(wrect.right() - textw, wrect.top(),
                               wrect.right(), wrect.bottom());
            else
                rect = wrect;
            break;
        }
#endif // QT_NO_PROGRESSBAR

    case SR_ToolButtonContents:
        rect = subControlRect(CC_ToolButton, widget, SC_ToolButton);
        break;

    default:
        rect = wrect;
        qDebug("*** Not Implemented Yet ***  subRect(subrect[0x%08x], w)", subrect);
        break;
    }
    return rect;
}


QSize QPocketPCStyle::sizeFromContents(ContentsType           contents,
                                       const QWidget          *widget,
                                       const QSize          &contentsSize,
                                       const QStyleOption &opt) const
{
    QSize sz(contentsSize);
#if defined(QT_CHECK_STATE)
    if (! widget) {
        qWarning("QPocketPCStyle::sizeFromContents: widget parameter cannot be zero!");
        return sz;
    }
#endif

    switch (contents) {
    case CT_Q3PopupMenuItem:
        {
            if (! widget || opt.isDefault())
                break;

            const Q3PopupMenu *popup = (const Q3PopupMenu *) widget;
            bool checkable = popup->isCheckable();
            Q3MenuItem *mi = opt.menuItem();
            int maxpmw = opt.maxIconWidth();
            int w = sz.width(), h = sz.height();

            if (mi->custom()) {
                w = mi->custom()->sizeHint().width();
                h = mi->custom()->sizeHint().height();
                if (! mi->custom()->fullSpan())
                    h += 2*CE_ITEMVMARGIN + 2*CE_ITEMFRAME;
            } else if (mi->widget()) {
            } else if (mi->isSeparator()) {
                w = 10; // arbitrary
                h = CE_SEPHEIGHT;
                break;
            } else {
                if (mi->pixmap())
                    h = qMax(h, mi->pixmap()->height() + 2*CE_ITEMFRAME);
                else if (! mi->text().isNull())
                    h = qMax(h, popup->fontMetrics().height() + 2*CE_ITEMVMARGIN +
                             2*CE_ITEMFRAME);

                if (mi->iconSet() != 0)
                    h = qMax(h, mi->iconSet()->pixmap(Qt::SmallIconSize,
                                                      QIcon::Normal).height() +
                             2*CE_ITEMFRAME);
            }

            if (! mi->text().isNull() && mi->text().indexOf('\t') >= 0)
                w += CE_TABSPACING;
            else if (mi->popup())
                w += 2*CE_ARROWHMARGIN;

            if (checkable && maxpmw < CE_CHECKMARKWIDTH)
                w += CE_CHECKMARKWIDTH - maxpmw;
            if (maxpmw)
                w += maxpmw + 6;
            if (checkable || maxpmw > 0)
                w += CE_CHECKMARKHMARGIN;
            w += CE_RIGHTBORDER;

            sz = QSize(w, h);
            break;
        }

    case CT_PushButton:
        {
            const QPushButton *button = (const QPushButton *) widget;
            int w = contentsSize.width(),
                h = contentsSize.height(),
               bm = PM_BUTTONMARGIN, // pixelMetric(PM_ButtonMargin, widget)
               fw = PM_DEFAULTFRAMEWIDTH; // pixelMetric(PM_DefaultFrameWidth, widget) * 2;

            w += bm + fw;
            h += bm + fw;

            if (button->isDefault() || button->autoDefault()) {
                int dbw = PM_ButtonDefaultIndicator * 2; // pixelMetric(PM_ButtonDefaultIndicator, widget) * 2;
                w += dbw;
                h += dbw;
            }

            sz = QSize(w, h);
            break;
        }

    case CT_CheckBox:
        {
            const QCheckBox *checkbox = (const QCheckBox *) widget;
            QRect irect = subRect(SR_CheckBoxIndicator, widget);
            int h = PM_INDICATORHEIGHT; // pixelMetric(PM_IndicatorHeight, widget);
            sz += QSize(irect.right() + (checkbox->text().isEmpty() ? 0 : 10), 4);
            sz.setHeight(qMax(sz.height(), h));
            break;
        }

    case CT_RadioButton:
        {
            const QRadioButton *radiobutton = (const QRadioButton *) widget;
            QRect irect = subRect(SR_RadioButtonIndicator, widget);
            int h = PM_EXCLUSIVEINDICATORHEIGHT; // pixelMetric(PM_ExclusiveIndicatorHeight, widget);
            sz += QSize(irect.right() + (radiobutton->text().isEmpty() ? 0 : 10), 4);
            sz.setHeight(qMax(sz.height(), h));
            break;
        }

    case CT_ToolButton:
        {
            sz = QSize(sz.width() + 4, sz.height() + 4);
            break;
        }

    case CT_ComboBox:
        {
            sz = QSize(sz.width() + 18, sz.height() + 5);
            break;
        }

#ifndef QT_NO_DIALOGBUTTONS
    case CT_DialogButtons:
        {
            const QDialogButtons *dbtns = (const QDialogButtons *)widget;
            int w = contentsSize.width(), h = contentsSize.height();
            const int bwidth = PM_DIALOGBUTTONSBUTTONWIDTH, // pixelMetric(PM_DialogButtonsButtonWidth, widget),
                    bspace = PM_DIALOGBUTTONSSEPARATOR, // pixelMetric(PM_DialogButtonsSeparator, widget),
                    bheight = PM_DIALOGBUTTONSBUTTONHEIGHT; // pixelMetric(PM_DialogButtonsButtonHeight, widget);
            if(dbtns->orientation() == Qt::Horizontal) {
                if(!w)
                    w = bwidth;
            } else {
                if(!h)
                    h = bheight;
            }
            QDialogButtons::Button btns[] = { QDialogButtons::All, QDialogButtons::Reject, QDialogButtons::Accept, //reverse order (right to left)
                                            QDialogButtons::Apply, QDialogButtons::Retry, QDialogButtons::Ignore, QDialogButtons::Abort,
                                            QDialogButtons::Help };
            for(unsigned int i = 0, cnt = 0; i < (sizeof(btns)/sizeof(btns[0])); i++) {
                if(dbtns->isButtonVisible(btns[i])) {
                    QSize szH = dbtns->sizeHint(btns[i]);
                    int mwidth = qMax(bwidth, szH.width()), mheight = qMax(bheight, szH.height());
                    if(dbtns->orientation() == Qt::Horizontal)
                        h = qMax(h, mheight);
                    else
                        w = qMax(w, mwidth);

                    if(cnt)
                        w += bspace;
                    cnt++;
                    if(dbtns->orientation() == Qt::Horizontal)
                        w += mwidth;
                    else
                        h += mheight;
                }
            }
            const int fw = PM_DEFAULTFRAMEWIDTH * 2; // pixelMetric(PM_DefaultFrameWidth, widget) * 2;
            sz = QSize(w + fw, h + fw);
            break;
        }
#endif //QT_NO_DIALOGBUTTONS

    case CT_LineEdit:
    case CT_Header:
    case CT_Slider:
    case CT_ProgressBar:
    case CT_TabBarTab:
    case CT_SpinBox:
    case CT_TabWidget:
        break;

    default:
        if (contents > CT_CustomBase)
            break;
        // This debug message is only here to make sure we handle the complete style properly
        // We might remove this debug message before a full release
        qDebug("*** Not Implemented Yet ***  sizeFromContents(contents[0x%08x], w, contentSize, opt[0x%08x])", contents, opt);
    }

    return sz;
}


QStyle::SubControl QPocketPCStyle::hitTestComplexControl(ComplexControl      complex,
                                                   const QWidget      *widget,
                                                   const QPoint              &point,
                                                   const QStyleOption &opt) const
{
    SubControl ret = SC_None;
    switch (complex) {

#ifndef QT_NO_LISTVIEW
    case CC_ListView:
        {
            if(point.x() >= 0 && point.x() <
               opt.listViewItem()->listView()->treeStepSize())
                ret = SC_ListViewExpand;
            break;
        }
#endif

#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar:
        {
            QRect r;
            uint ctrl = SC_ScrollBarAddLine;

            // we can do this because subcontrols were designed to be masks as well...
            while (ret == SC_None && ctrl <= SC_ScrollBarGroove) {
                r = subControlRect(complex, widget,
                                           (QStyle::SubControl) ctrl, opt);
                if (r.isValid() && r.contains(point))
                    ret = (QStyle::SubControl) ctrl;

                ctrl <<= 1;
            }

            break;
        }
#endif

#ifndef QT_NO_TITLEBAR
    case CC_TitleBar:
        {
            const QTitleBar *titlebar = (QTitleBar*)widget;
            QRect r;
            uint ctrl = SC_TitleBarLabel;

            // we can do this because subcontrols were designed to be masks as well...
            while (ret == SC_None && ctrl <= SC_TitleBarUnshadeButton) {
                r = visualRect(subControlRect(complex, widget, (QStyle::SubControl) ctrl, opt), widget);
                if (r.isValid() && r.contains(point))
                    ret = (QStyle::SubControl) ctrl;

                ctrl <<= 1;
            }
            if (titlebar->window()) {
                if (ret == SC_TitleBarMaxButton && titlebar->testWFlags(Qt::WA_WState_Tool)) {
                    if (titlebar->window()->isMinimized())
                        ret = SC_TitleBarUnshadeButton;
                    else
                        ret = SC_TitleBarShadeButton;
                } else if (ret == SC_TitleBarMinButton && !titlebar->testWFlags(Qt::WA_WState_Tool)) {
                    if (titlebar->window()->isMinimized())
                        ret = QStyle::SC_TitleBarNormalButton;
                }
            }
            break;
        }
#endif
    default:
        qDebug("*** Not Implemented Yet ***  hitTestComplexControl(control[0x%08x], w, point, opt[0x%08x])", complex, opt);
    }
    return ret;
}


QRect QPocketPCStyle::subControlRect(ComplexControl         complex,
                                             const QWidget        *widget,
                                             SubControl                 sc,
                                             const QStyleOption &opt) const
{
#if defined(QT_CHECK_STATE)
    if (! widget) {
        qWarning("QPocketPCStyle::subControlRect: widget parameter cannot be zero!");
        return QRect();
    }
#endif

    switch (complex)
    {
#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar:
        {
            const QScrollBar *scrollbar = (const QScrollBar *) widget;

            const int buttonWidth = 19;
            const int barStart = 0;
            const int grooveStart = buttonWidth;
            int sliderStart = scrollbar->sliderStart();
            int sliderEnd = 0;
            int grooveEnd = 0;
            int barEnd = 0;
            int height = 0;

            if (scrollbar->orientation() == Qt::Horizontal) {
                barEnd = scrollbar->width() - 1;
                height = scrollbar->height() - 1;
            } else {
                barEnd = scrollbar->height() - 1;
                height = scrollbar->width() - 1;
            }

            grooveEnd = barEnd - buttonWidth;

            int maxlen = grooveEnd - buttonWidth + 1;
            uint range = scrollbar->maxValue() - scrollbar->minValue();

            // calculate slider length
            if (range) {
                const int slidermin = 9;
                int sliderlen = (scrollbar->pageStep() * maxlen) / (range + scrollbar->pageStep());
                if (sliderlen < slidermin || range > INT_MAX / 2)
                    sliderlen = slidermin;
                if (sliderlen > maxlen)
                    sliderlen = maxlen;
                sliderEnd = sliderStart + sliderlen;
            } else
                sliderEnd = sliderStart + maxlen;

            int x1 = 0, x2 = 0;

            switch (sc) {
                case SC_ScrollBarSubLine: // top/left button
                    x1 = barStart; x2 = grooveStart; break;
                case SC_ScrollBarSubPage: // between top/left button and slider
                    x1 = grooveStart; x2 = sliderStart; break;
                case SC_ScrollBarSlider:
                    x1 = sliderStart; x2 = sliderEnd; break;
                case SC_ScrollBarAddPage: // between bottom/right button and slider
                    x1 = sliderEnd; x2 = grooveEnd; break;
                case SC_ScrollBarAddLine: // bottom/right button
                    x1 = grooveEnd; x2 = barEnd; break;
                case SC_ScrollBarGroove:
                    x1 = grooveStart; x2 = grooveEnd; break;
                default:
                    break;
            }

            // rotate the rectangle if it is vertical or not
            QRect rect;
            if (scrollbar->orientation() == Qt::Horizontal)
                rect.setCoords(x1, 0, x2, height);
            else
                rect.setCoords(0, x1, height, x2);
            return rect;
        }
#endif // QT_NO_SCROLLBAR

    case CC_SpinBox:
        {
            const int PM_SPINBOXBUTTONWIDTH = 12;
            const int fw  = PM_SPINBOXFRAMEWIDTH; // pixelMetric(PM_SpinBoxFrameWidth, widget);
            const int h   = widget->height();
            const int w   = widget->width();

            const int dh  = h  - PM_SPINBOXFRAMEWIDTH *2; // delta height (w/o frame)
            const int xD  = w  - PM_SPINBOXBUTTONWIDTH - PM_SPINBOXFRAMEWIDTH; // down button X
            const int xU  = xD - PM_SPINBOXBUTTONWIDTH; // up button X

            switch (sc) {
            case SC_SpinBoxUp:
                return QRect(xU, fw, PM_SPINBOXBUTTONWIDTH, dh);
            case SC_SpinBoxDown:
                return QRect(xD, fw, PM_SPINBOXBUTTONWIDTH, dh);
            case SC_SpinBoxEditField:
                return QRect(fw, fw, w-(w-xU)-PM_SPINBOXFRAMEWIDTH, dh);
            case SC_SpinBoxFrame:
                return widget->rect();
            }
            break;
        }

    case CC_ComboBox:
        {
            int x = 0, y = 0, wi = widget->width(), he = widget->height();
            int xpos = x;
            xpos += wi - 2 - 16;

            switch (sc) {
            case SC_ComboBoxFrame:
                return widget->rect();
                break;
            case SC_ComboBoxArrow:
                return QRect(xpos, y+2, 16, he-4);
            case SC_ComboBoxEditField:
                return QRect(x+3, y+3, wi-6-16, he-6);
            }
            break;
        }

#ifndef QT_NO_SLIDER
    case CC_Slider:
        {
            const QSlider * sl = (const QSlider *) widget;
            int tickOffset = pixelMetric(PM_SliderTickmarkOffset, sl);
            int thickness  = pixelMetric(PM_SliderControlThickness, sl);

            switch (sc) {
            case SC_SliderHandle:
                {
                    int sliderPos  = sl->sliderStart();
                    int len        = PM_SLIDERLENGTH; // pixelMetric(PM_SliderLength, sl);

                    if (sl->orientation() == Qt::Horizontal)
                        return QRect(sliderPos, tickOffset, len, thickness);
                    return QRect(tickOffset, sliderPos, thickness, len);
                }

            case SC_SliderGroove:
                {
                    if (sl->orientation() == Qt::Horizontal)
                        return QRect(0, tickOffset, sl->width(), thickness*2);
                    return QRect(tickOffset, 0, thickness, sl->height());
                }
            }
            break;
        }
#endif // QT_NO_SLIDER

        // COMMON STYLE ============================================================================================


#if !defined(QT_NO_TOOLBUTTON) && !defined(QT_NO_POPUPMENU)
    case CC_ToolButton:
        {
            const QToolButton *toolbutton = (const QToolButton *) widget;
            int mbi = PM_MENUBUTTONINDICATOR; // pixelMetric(PM_MenuButtonIndicator, widget);

            QRect rect = toolbutton->rect();
            //rect.adjust(0, 2, 0, -4);
            switch (sc) {
            case SC_ToolButton:
                if (toolbutton->popup() && ! toolbutton->popupDelay())
                    rect.adjust(0, 0, -mbi, 0);
                return rect;

            case SC_ToolButtonMenu:
                if (toolbutton->popup() && ! toolbutton->popupDelay())
                    rect.adjust(rect.width() - mbi, 0, 0, 0);
                return rect;
            }
            break;
        }
#endif // QT_NO_TOOLBUTTON && QT_NO_POPUPMENU

#ifndef QT_NO_TITLEBAR
    case CC_TitleBar:
        {
            const QTitleBar *titlebar = (const QTitleBar *) widget;
            const int controlTop = 2;
            const int controlHeight = widget->height() - controlTop * 2;

            switch (sc) {
            case SC_TitleBarLabel: {
                const QTitleBar *titlebar = (QTitleBar*)widget;
                QRect ir(0, 0, titlebar->width(), titlebar->height());
                if (titlebar->testWFlags(Qt::WA_WState_Tool)) {
                    if (titlebar->testWFlags(Qt::WA_WState_SysMenu))
                        ir.adjust(0, 0, -controlHeight-3, 0);
                    if (titlebar->testWFlags(Qt::WA_WState_MinMax))
                        ir.adjust(0, 0, -controlHeight-2, 0);
                } else {
                    if (titlebar->testWFlags(Qt::WA_WState_SysMenu))
                        ir.adjust(controlHeight+3, 0, -controlHeight-3, 0);
                    if (titlebar->testWFlags(Qt::WA_WState_Minimize))
                        ir.adjust(0, 0, -controlHeight-2, 0);
                    if (titlebar->testWFlags(Qt::WA_WState_Maximize))
                        ir.adjust(0, 0, -controlHeight-2, 0);
                }
                return ir; }

            case SC_TitleBarCloseButton:
                return QRect(titlebar->width() - (controlHeight + controlTop),
                              controlTop, controlHeight, controlHeight);

            case SC_TitleBarMaxButton:
            case SC_TitleBarShadeButton:
            case SC_TitleBarUnshadeButton:
                return QRect(titlebar->width() - ((controlHeight + controlTop) * 2),
                              controlTop, controlHeight, controlHeight);

            case SC_TitleBarMinButton:
            case SC_TitleBarNormalButton: {
                int offset = controlHeight + controlTop;
                if (!titlebar->testWFlags(Qt::WA_WState_Maximize))
                    offset *= 2;
                else
                    offset *= 3;
                return QRect(titlebar->width() - offset, controlTop, controlHeight, controlHeight);
            }

            case SC_TitleBarSysMenu:
                return QRect(3, controlTop, controlHeight, controlHeight);
            }
            break;
        }
#endif //QT_NO_TITLEBAR

    // --- Catch unknown elements ---
    default:
        // This debug message is only here to make sure we handle the complete style properly
        // We might remove this debug message before a full release
        qDebug("*** Not Implemented Yet ***  subControlRect(complex[0x%08x], w, subcontrol[0x%08x], opt[0x%08x])", complex, sc, opt);
    }

    return QRect();
}


void QPocketPCStyle::drawPrimitive(PrimitiveElement    primitive,
                                   QPainter              *p,
                                   const QRect              &r,
                                   const QPalette     &pal,
                                   SFlags               flags,
                                   const QStyleOption &opt) const
{
    // CE Shapes  ### Might be faster to use pixmaps instead
    #define CreateQPolygon(pts)        QPolygon(sizeof(pts)/(sizeof(int)*2), pts)
    static const int radioOutline[] = {  1, 3,    3, 1,    4, 1,    5, 0,    9, 0,   10, 1,   11, 1,   13, 3,   13, 4,   14, 5,   14, 9,   13,10,   13,11,   11,13,   10,13,   9,14,    5,14,    4,13,    3,13,    1,11,    1,10,    0, 9,    0, 5,    1,4,     1, 3 };
    static const int radioDot[]     = {  6, 3,    8, 3,    4, 4,   10, 4,    4, 5,   10, 5,    3, 6,   11, 6,    3, 7,   11, 7,    3, 8,   11, 8,    4, 9,   10, 9,    4,10,  10,10,    6,11,    8,11 };
    static const int tick[18]       = {  3, 6,    6, 9,   11, 4,   11, 5,    6,10,    3, 7,    3, 8,    6,11,   11,6  };
    static const int arrowUp[16]    = { -3, 1,    3, 1,   -2, 0,    2, 0,   -1,-1,    1,-1,    0,-2,    0,-2 };
    static const int arrowDown[16]  = { -3,-2,    3,-2,   -2,-1,    2,-1,   -1, 0,    1, 0,    0, 1,    0, 1 };

    // ### positioning needs fixing
    static const int arrowRight[16] = { -2,-3,   -2, 3,   -1,-2,   -1, 2,    0,-1,    0, 1,    1, 0,    1, 0 };
    static const int arrowLeft[16]  = {  0,-3,    0, 3,   -1,-2,   -1, 2,   -2,-1,   -2, 1,   -3, 0,   -3, 0 };
    static const int *arrows[4]     = { arrowUp, arrowDown, arrowRight, arrowLeft };

    // See PE_ArrowUp, PE_ArrowDown, PE_ArrowRight
    int arrow = 3;

    // Set active painter, for optimization
    activePainter = p;

    switch (primitive) {
    case PE_ButtonCommand:
    case PE_ButtonBevel:
    case PE_ButtonTool:
    case PE_ButtonDropDown:
    case PE_Splitter:
    case PE_DockWindowResizeHandle:
        {
            p->setBrush((flags & State_Sunken) ? pal.foreground() : pal.background());
            p->setPen(pal.foreground());
            p->drawRect(r);
            break;
        }

    case PE_DockWindowHandle:
        {
            p->fillRect(r, pal.background());
            p->setPen(pal.mid());
            if (flags & State_Horizontal) {
                p->drawLine(1, 2, 1, r.height() - 3);
                p->drawLine(3, 2, 3, r.height() - 3);
            } else {
                p->drawLine(2, 1, r.width() - 3, 1);
                p->drawLine(2, 3, r.width() - 3, 3);
            }
            break;
        }

    case PE_DockWindowSeparator:
        {
            p->setPen(pal.background());
            p->drawLine(r.topLeft(), r.bottomRight());
            p->setPen(pal.dark());
            if (flags & State_Horizontal)
                p->drawLine(0, 4, 0, r.height() - 5);
            else
                p->drawLine(4, 0, r.width() - 5, 0);
            break;
        }

    case PE_PanelDockWindow:
        {
            switch (findLocation(p)) {
            case Qt::DockTop:
                p->drawLine(r.bottomLeft(), r.bottomRight());
                break;
            case Qt::DockBottom:
                p->drawLine(r.topLeft(), r.topRight());
                break;
            case Qt::DockLeft:
                p->drawLine(r.topRight(), r.bottomRight());
                break;
            case Qt::DockRight:
                p->drawLine(r.topLeft(), r.bottomLeft());
                break;
            }
            break;
        }

    case PE_MenuBarFrame:
    case PE_PanelMenuBar:
        {
            switch (findLocation(p)) {
            case Qt::DockTop:
                p->drawLine(r.bottomLeft(), r.bottomRight());
                break;
            case Qt::DockBottom:
                p->drawLine(r.topLeft(), r.topRight());
                break;
            case Qt::DockLeft:
                p->drawLine(r.topRight(), r.bottomRight());
                break;
            case Qt::DockRight:
                p->drawLine(r.topLeft(), r.bottomLeft());
                break;
            }
            break;
        }

    case PE_HeaderSection:
        {
            const QColor &back = (flags & State_Down) ? pal.dark() : pal.background();
            const QColor &frnt = (flags & State_Down) ? pal.light() : pal.foreground();
            p->setBrush(back);
            p->setPen(frnt);
            p->drawRect(r);
            p->setPen(back);
            if (flags & State_Horizontal)
                p->drawLine(r.topLeft()+=QPoint(0,1), r.bottomLeft()+=QPoint(0,-1));
            else
                p->drawLine(r.topLeft()+=QPoint(1,0), r.topRight()+=QPoint(-1,0));
            break;
        }

    case PE_PanelTabWidget:
    case PE_WindowFrame:
        {
            qDrawShadePanel(p, r, pal, (flags & State_Sunken), opt.isDefault() ? PM_DEFAULTFRAMEWIDTH : opt.lineWidth()); // pixelMetric(PM_DefaultFrameWidth)
            break;
        }

    case PE_MenuFrame:
    case PE_PanelLineEdit:
    case PE_PanelPopup:
    case PE_PanelGroupBox:
    case PE_Panel:
        {
            //p->setBrush(pal.base());
            p->setPen(pal.foreground());
            p->drawRect(r);
            break;
        }

    case PE_Separator:
        {
            p->setBrush(Qt::NoBrush);
            p->setPen(pal.background());
            p->drawRect(r);
            break;
        }

    case PE_FocusRect:
        {
            // I'm not sure how much focus rects are used on pocketPC
            const QColor *bg = opt.isDefault()
                                ? 0 : &opt.color();

            QColor drawColor = pal.background();
            if (bg) {
                int h, s, v;
                bg->getHsv(&h, &s, &v);
                if (v < 128)
                    drawColor = pal.base();
            }
            QPen oldPen = p->pen();
            QBrush oldBrush = p->brush();

            p->setPen(drawColor);
            p->setBrush(Qt::NoBrush);
            p->drawRect(r);

            p->setPen(oldPen);
            p->setBrush(oldBrush);
            break;
        }

    case PE_StatusBarSection:
        {
            p->setBrush(pal.background());
            p->setPen(pal.foreground());
            p->drawRect(r);
            break;
        }

    case PE_Indicator:
        {
            p->setBrush(pal.base());
            p->setPen(pal.foreground());
            p->drawRect(r);                                        // Draw the box around the control
            if (flags & State_On)
                p->drawPolyline(CreateQPolygon(tick));   // Draw the tick if it is on
            break;
        }

    case PE_ExclusiveIndicator:
        {
            p->setPen(pal.foreground());
            p->drawPolyline(CreateQPolygon(radioOutline));
            if (flags & State_On)                                    // Radio button dot shown when it is selected
                p->drawPolyline(CreateQPolygon(radioDot));
            break;
        }

    case PE_ScrollBarSubLine:
        {
            drawPrimitive(PE_ButtonBevel, p, r, pal, (flags & State_Enabled) | ((flags & State_Down) ? State_Down : State_Raised));
            drawPrimitive(((flags & State_Horizontal) ? PE_ArrowLeft : PE_ArrowUp), p, r, pal, flags);
            break;
        }

    case PE_ScrollBarAddLine:
        {
            drawPrimitive(PE_ButtonBevel, p, r, pal, (flags & State_Enabled) | ((flags & State_Down) ? State_Down : State_Raised));
            drawPrimitive(((flags & State_Horizontal) ? PE_ArrowRight : PE_ArrowDown), p, r, pal, flags);
            break;
        }

    case PE_ScrollBarAddPage:
    case PE_ScrollBarSubPage:
        {
            p->setBrush(pal.base());            // On pocketPC it doesn't change appearence when pressed
            p->setPen(pal.foreground());
            p->drawRect(r);
            break;
        }

    case PE_ScrollBarSlider:
        {
            p->setBrush(pal.background());   // On pocketPC it doesn't change appearence when pressed
            p->setPen(pal.foreground());
            p->drawRect(r);
            // The 3 little lines on the slider button
            if (flags & State_Horizontal) {
                int midx = r.x() + r.width() / 2;
                p->drawLine(midx - 2, r.y() + 3, midx - 2, r.y() + r.height() - 4);
                p->drawLine(midx + 0, r.y() + 3, midx + 0, r.y() + r.height() - 4);
                p->drawLine(midx + 2, r.y() + 3, midx + 2, r.y() + r.height() - 4);
            } else {
                int midy = r.y() + r.height() / 2;
                p->drawLine(r.x() + 3, midy - 2, r.x() + r.width() - 4, midy - 2);
                p->drawLine(r.x() + 3, midy + 0, r.x() + r.width() - 4, midy + 0);
                p->drawLine(r.x() + 3, midy + 2, r.x() + r.width() - 4, midy + 2);
            }
            break;
        }

    case PE_ArrowUp:
        arrow--; // Fall-through intended
    case PE_ArrowDown:
        arrow--; // Fall-through intended
    case PE_ArrowRight:
        arrow--; // Fall-through intended
    case PE_ArrowLeft:
        {
            QPolygon a(8, arrows[arrow]);
            a.translate(r.x() + r.width() / 2, r.y() + r.height() / 2);
            p->setPen((flags & State_Down) ? pal.base() : pal.foreground());
            p->drawLineSegments(a);         // draw arrow
            break;
        }
    case PE_HeaderArrow:
	{
	    p->save();
	    if (flags & State_Up) { // invert logic to follow Windows style guide
		QPolygon pa(3);
		p->setPen(pal.light());
		p->drawLine(r.x() + r.width(), r.y(), r.x() + r.width() / 2, r.height());
		p->setPen(pal.dark());
		pa.setPoint(0, r.x() + r.width() / 2, r.height());
		pa.setPoint(1, r.x(), r.y());
		pa.setPoint(2, r.x() + r.width(), r.y());
		p->drawPolyline(pa);
	    } else {
		QPolygon pa(3);
		p->setPen(pal.light());
		pa.setPoint(0, r.x(), r.height());
		pa.setPoint(1, r.x() + r.width(), r.height());
		pa.setPoint(2, r.x() + r.width() / 2, r.y());
		p->drawPolyline(pa);
		p->setPen(pal.dark());
		p->drawLine(r.x(), r.height(), r.x() + r.width() / 2, r.y());
	    }
	    p->restore();
	    break;
	}

    case PE_ButtonDefault:
        {
            p->setPen(pal.foreground());
            p->drawRect(r);
            break;
        }

    case PE_CheckListIndicator:
        {
            if (flags & State_Enabled)
                p->setPen(QPen(pal.text(), 1));
            else
                p->setPen(QPen(pal.dark(), 1));

            if (flags & State_NoChange)
                p->setBrush(pal.brush(QPalette::Button));

            p->drawRect(r.x()+1, r.y()+1, 11, 11);

            if (! (flags & State_Off)) {
                QPolygon a(7*2);
                int i, xx, yy;
                xx = r.x() + 3;
                yy = r.y() + 5;

                for (i=0; i<3; i++) {
                    a.setPoint(2*i,   xx, yy);
                    a.setPoint(2*i+1, xx, yy+2);
                    xx++; yy++;
                }

                yy -= 2;
                for (i=3; i<7; i++) {
                    a.setPoint(2*i,   xx, yy);
                    a.setPoint(2*i+1, xx, yy+2);
                    xx++; yy--;
                }

                p->drawLineSegments(a);
            }
            break;
        }

    // Common Style =====================================================================
    // Not optimized yet...
    case PE_CheckListController:
        {
            p->drawPixmap(r, QPixmap((const char **)check_list_controller_xpm));
            break;
        }

#ifndef QT_NO_LISTVIEW
    case PE_CheckListExclusiveIndicator:
        {
            #define INTARRLEN(x) sizeof(x)/(sizeof(int)*2)

            QCheckListItem *item = opt.checkListItem();
            QListView *lv = item->listView();
            if(!item)
                return;
            int x = r.x(), y = r.y();

            static const int pts1[] = { 1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };    // dark lines
            static const int pts2[] = { 2,8, 1,7, 1,4, 2,3, 2,2, 3,2, 4,1, 7,1, 8,2, 9,2 };                    // black lines
            static const int pts3[] = { 2,9, 3,9, 4,10, 7,10, 8,9, 9,9, 9,8, 10,7, 10,4, 9,3 };            // background lines
            static const int pts4[] = { 2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7, 11,4, 10,3, 10,2 }; // white lines

            if (flags & State_Enabled)
                p->setPen(pal.text());
            else
                p->setPen(QPen(lv->palette().color(QPalette::Disabled, QPalette::Text)));
            QPolygon a(INTARRLEN(pts1), pts1);
            a.translate(x, y);
            p->drawPolyline(a);
            a.setPoints(INTARRLEN(pts2), pts2);
            a.translate(x, y);
            p->drawPolyline(a);
            a.setPoints(INTARRLEN(pts3), pts3);
            a.translate(x, y);
            p->drawPolyline(a);
            a.setPoints(INTARRLEN(pts4), pts4);
            a.translate(x, y);
            p->drawPolyline(a);
            if (flags & State_On) {
                p->setPen(Qt::NoPen);
                p->setBrush(pal.text());
                p->drawRect(x+5, y+4, 2, 4);
                p->drawRect(x+4, y+5, 4, 2);
            }
            break;
        }
#endif

    case PE_SpinBoxPlus:
    case PE_SpinBoxMinus:
        {
            p->save();
            int fw = PM_DEFAULTFRAMEWIDTH; // pixelMetric(PM_DefaultFrameWidth, 0);
            QRect br;
            br.setRect(r.x() + fw, r.y() + fw, r.width() - fw*2,
                        r.height() - fw*2);

            p->fillRect(br, pal.brush(QPalette::Button));
            p->setPen(pal.buttonText());
            p->setBrush(pal.buttonText());

            int length;
            int x = r.x(), y = r.y(), w = r.width(), h = r.height();
            if (w <= 8 || h <= 6)
                length = qMin(w-2, h-2);
            else
                length = qMin(2*w / 3, 2*h / 3);

            if (!(length & 1))
                length -=1;
            int xmarg = (w - length) / 2;
            int ymarg = (h - length) / 2;

            p->drawLine(x + xmarg, (y + h / 2 - 1),
                        x + xmarg + length - 1, (y + h / 2 - 1));
            if (primitive == PE_SpinBoxPlus)
                p->drawLine((x+w / 2) - 1, y + ymarg,
                            (x+w / 2) - 1, y + ymarg + length - 1);
            p->restore();
            break;
        }

    case PE_IndicatorMask:
        {
            p->fillRect(r, Qt::color1);
            break;
        }

    case PE_ExclusiveIndicatorMask:
        {
            p->setPen(Qt::color1);
            p->setBrush(Qt::color1);
            p->drawEllipse(r);
            break;
        }

    case PE_SizeGrip:
        {
            p->save();

            int x, y, w, h;
            r.rect(&x, &y, &w, &h);

            int sw = qMin(h,w);
            if (h > w)
                p->translate(0, h - w);
            else
                p->translate(w - h, 0);

            int sx = x;
            int sy = y;
            int s = sw / 3;

            if (QApplication::isRightToLeft()) {
                sx = x + sw;
                for (int i = 0; i < 4; ++i) {
                    p->setPen(QPen(pal.light(), 1));
                    p->drawLine( x, sy - 1 , sx + 1,  sw);
                    p->setPen(QPen(pal.dark(), 1));
                    p->drawLine( x, sy, sx,  sw);
                    p->setPen(QPen(pal.dark(), 1));
                    p->drawLine( x, sy + 1, sx - 1,  sw);
                    sx -= s;
                    sy += s;
                }
            } else {
                for (int i = 0; i < 4; ++i) {
                    p->setPen(QPen(pal.light(), 1));
                    p->drawLine( sx-1, sw, sw,  sy-1);
                    p->setPen(QPen(pal.dark(), 1));
                    p->drawLine( sx, sw, sw,  sy);
                    p->setPen(QPen(pal.dark(), 1));
                    p->drawLine( sx+1, sw, sw,  sy+1);
                    sx += s;
                    sy += s;
                }
            }

            p->restore();
            break;
        }

    case PE_CheckMark:
        {
            const int markW = r.width() > 7 ? 7 : r.width();
            const int markH = markW;
            int posX = r.x() + (r.width() - markW)/2 + 1;
            int posY = r.y() + (r.height() - markH)/2;

            // Could do with some optimizing/caching...
            QPolygon a(markH*2);
            int i, xx, yy;
            xx = posX;
            yy = 3 + posY;
            for (i=0; i<markW/2; i++) {
                a.setPoint(2*i,   xx, yy);
                a.setPoint(2*i+1, xx, yy+2);
                xx++; yy++;
            }
            yy -= 2;
            for (; i<markH; i++) {
                a.setPoint(2*i,   xx, yy);
                a.setPoint(2*i+1, xx, yy+2);
                xx++; yy--;
            }
            if (!(flags & State_Enabled) && !(flags & State_On)) {
                int pnt;
                p->setPen(pal.highlightedText());
                QPoint offset(1,1);
                for (pnt = 0; pnt < (int)a.size(); pnt++)
                    a[pnt] += offset;
                p->drawLineSegments(a);
                for (pnt = 0; pnt < (int)a.size(); pnt++)
                    a[pnt] -= offset;
            }
            p->setPen(pal.text());
            p->drawLineSegments(a);
            break;
        }

#ifndef QT_NO_FRAME
    case PE_GroupBoxFrame:
        {
            if (opt.isDefault())
                break;
            int lwidth = opt.lineWidth(), mlwidth = opt.midLineWidth();
            if (flags & (State_Sunken|State_Raised))
                qDrawShadeRect(p, r.x(), r.y(), r.width(), r.height(), pal, flags & State_Sunken, lwidth, mlwidth);
            else
                qDrawPlainRect(p, r.x(), r.y(), r.width(), r.height(), pal.foreground(), lwidth);
            break;
        }
#endif

    case PE_ProgressBarChunk:
        {
            p->fillRect(r.x(), r.y() + 3, r.width() -2, r.height() - 6,
                pal.brush(QPalette::Highlight));
            break;
        }

    case PE_TabBarBase:
        {
            p->fillRect(r, pal.background());
            break;
        }
    case PE_ScrollBarFirst:
    case PE_ScrollBarLast:
        break;

    // --- Catch unknown elements ---
    default:
        // This debug message is only here to make sure we handle the complete style properly
        // We might remove this debug message before a full release
        qDebug("*** Not Implemented Yet ***  drawPrimitive(primitive[0x%08x], p, r, pal, flags[0x%08x], opt[0x%08x])", primitive, flags, opt);
    }
    activePainter = 0;
}


void QPocketPCStyle::drawControl(ControlElement             control,
                                 QPainter            *p,
                                 const QWidget            *widget,
                                 const QRect            &r,
                                 const QPalette     &pal,
                                 SFlags                     flags,
                                 const QStyleOption &opt) const
{
#if defined(QT_CHECK_STATE)
    if (! widget) {
        qWarning("QPocketPCStyle::drawControl: widget parameter cannot be zero!");
        return;
    }
#endif

    // Set active painter, for optimization
    activePainter = p;

    switch (control) {
    case CE_PushButton:
        {
            const QPushButton *button = (const QPushButton *) widget;
            QRect br = r;

            SFlags flags = State_Default;
            if (button->isEnabled())
                flags |= State_Enabled;
            if (button->isDown())
                flags |= State_Down;
            if (button->isOn())
                flags |= State_On;
            if (! button->isFlat() && ! (flags & State_Down))
                flags |= State_Raised;

            if (button->isDefault()) {
                drawPrimitive(PE_ButtonDefault, p, br, pal, flags);
                br.adjust(1, 1, -1, -1);
            }
            drawPrimitive(PE_ButtonCommand, p, br, pal, flags);
            break;
        }
#ifndef QT_NO_TABBAR
    case CE_TabBarTab:
        {
            if (!widget || !widget->parentWidget())
                break;

            const QTabBar * tb  = (const QTabBar *) widget;
            const int current   = tb->currentTab();

            p->setPen(pal.foreground());
            if (flags & State_Selected) {
                // Draw full rect
                p->setBrush(pal.base());
                p->drawRect(r);
                // Now remove line
                p->setPen(pal.base());
                QPoint from = r.topLeft();
                QPoint   to = r.topRight();
                if (flags & State_Top) {
                    from = r.bottomLeft();
                    to = r.bottomRight();
                }
                from += QPoint(1, 0);
                to   += QPoint(-1, 0);
                p->drawLine(from, to);
            } else { // Not selected
                p->setBrush(pal.background());
                p->drawRect(r);
            }

            // If first tab, then remove left edge
            if (!tb->indexOf(opt.tab()->identifier())) {
                p->setPen(p->brush().color());
                QPoint from = r.topLeft();
                QPoint   to = r.bottomLeft();
                if (flags & State_Top) {
                    from += QPoint(0, 1);
                } else {
                    to   += QPoint(0,-1);
                }
                p->drawLine(from, to);
            }

            break;
        }
    case CE_TabBarLabel:
        {
            if (opt.isDefault())
                break;

            const QTabBar * tb = (const QTabBar *) widget;
            QTab * t = opt.tab();
            //bool has_focus = opt.hasFocus();

            QRect tr = r;
            if (t->identifier() == tb->currentTab())
                tr.setBottom(tr.bottom() - PM_DEFAULTFRAMEWIDTH); // pixelMetric(QStyle::PM_DefaultFrameWidth, tb));

            drawItem(p, tr, Qt::AlignCenter | Qt::TextShowMnemonic, pal, tb->isEnabled() &&
                      t->isEnabled(), 0, t->text());

            //if (has_focus)
                //drawPrimitive(PE_FocusRect, p, r, pal);
            break;
        }
#endif // QT_NO_TABBAR

#ifndef QT_NO_POPUPMENU
    case CE_Q3PopupMenuItem:
        {
            if (! widget || opt.isDefault())
                break;

            const Q3PopupMenu *popupmenu = (const Q3PopupMenu *) widget;
            Q3MenuItem *mi = opt.menuItem();
            if (!mi)
                break;

            int tab = opt.tabWidth();
            int maxpmw = opt.maxIconWidth();
            bool dis = ! mi->isEnabled();
            bool checkable = popupmenu->isCheckable();
            bool act = flags & State_Selected;
            int x, y, w, h;

            r.rect(&x, &y, &w, &h);

            if (checkable) {
                maxpmw = qMax(maxpmw, 12);
            }

            int checkcol = maxpmw;

            if (mi && mi->isSeparator()) {                    // draw separator
                p->setPen(pal.dark());
                p->drawLine(x, y, x+w, y);
                p->setPen(pal.light());
                p->drawLine(x, y+1, x+w, y+1);
                return;
            }

            QBrush fill = (act ?
                           pal.brush(QPalette::Highlight) :
                           pal.brush(QPalette::Button));
            p->fillRect(x, y, w, h, fill);

            if (!mi)
                return;

            int xpos = x;
            if (mi->isChecked()) {
                if (act && !dis)
                    qDrawShadePanel(p, xpos, y, checkcol, h,
                                     pal, true, 1, &pal.brush(QPalette::Button));
                else {
                    QBrush fill(pal.light(), Qt::Dense4Pattern);
                    qDrawShadePanel(p, xpos, y, checkcol, h, pal, true, 1,
                                     &fill);
                }
            } else if (! act)
                p->fillRect(xpos, y, checkcol , h, pal.brush(QPalette::Button));

            if (mi->iconSet()) {              // draw icon
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap;
                if (checkable && mi->isChecked())
                    pixmap = mi->iconSet()->pixmap(Qt::SmallIconSize, mode, QIcon::On);
                else
                    pixmap = mi->iconSet()->pixmap(Qt::SmallIconSize, mode);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                if (act && !dis && !mi->isChecked())
                    qDrawShadePanel(p, xpos, y, checkcol, h, pal, false, 1,
                                     &pal.brush(QPalette::Button));
                QRect cr(xpos, y, checkcol, h);
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(cr.center());
                p->setPen(pal.text());
                p->drawPixmap(pmr.topLeft(), pixmap);

                fill = (act ?
                        pal.brush(QPalette::Highlight) :
                        pal.brush(QPalette::Button));
                int xp;
                xp = xpos + checkcol + 1;
                p->fillRect(xp, y, w - checkcol - 1, h, fill);
            } else  if (checkable) {  // just "checking"...
                if (mi->isChecked()) {
                    int xp = xpos + CE_ITEMFRAME;

                    SFlags cflags = State_Default;
                    if (! dis)
                        cflags |= State_Enabled;
                    if (act)
                        cflags |= State_On;

                    drawPrimitive(PE_CheckMark, p,
                                  QRect(xp, y + CE_ITEMFRAME,
                                        checkcol - 2*CE_ITEMFRAME,
                                        h - 2*CE_ITEMFRAME), pal, cflags);
                }
            }

            p->setPen(act ? pal.highlightedText() : pal.buttonText());

            QColor discol;
            if (dis) {
                discol = pal.text();
                p->setPen(discol);
            }

            int xm = CE_ITEMFRAME + checkcol + CE_ITEMHMARGIN;
            xpos += xm;

            if (mi->custom()) {
                int m = CE_ITEMVMARGIN;
                p->save();
                if (dis && !act) {
                    p->setPen(pal.light());
                    mi->custom()->paint(p, pal, act, !dis,
                                         xpos+1, y+m+1, w-xm-tab+1, h-2*m);
                    p->setPen(discol);
                }
                mi->custom()->paint(p, pal, act, !dis,
                                     x+xm, y+m, w-xm-tab+1, h-2*m);
                p->restore();
            }
            QString s = mi->text();
            if (!s.isNull()) {                        // draw text
                int t = s.indexOf('\t');
                int m = CE_ITEMVMARGIN;
                const int text_flags = Qt::AlignVCenter|Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (t >= 0) {                         // draw tab text
                    int xp;
                    xp = x + w - tab - CE_RIGHTBORDER - CE_ITEMHMARGIN -
                         CE_ITEMFRAME + 1;
                    if (dis && !act) {
                        p->setPen(pal.light());
                        p->drawText(xp, y+m+1, tab, h-2*m, text_flags, s.mid(t+1));
                        p->setPen(discol);
                    }
                    p->drawText(xp, y+m, tab, h-2*m, text_flags, s.mid(t+1));
                    s = s.left(t);
                }
                if (dis && !act) {
                    p->setPen(pal.light());
                    p->drawText(xpos+1, y+m+1, w-xm-tab+1, h-2*m, text_flags, s, t);
                    p->setPen(discol);
                }
                p->drawText(xpos, y+m, w-xm-tab+1, h-2*m, text_flags, s, t);
            } else if (mi->pixmap()) {                        // draw pixmap
                QPixmap *pixmap = mi->pixmap();
                if (pixmap->depth() == 1)
                    p->setBackgroundMode(Qt::OpaqueMode);
                p->drawPixmap(xpos, y+CE_ITEMFRAME, *pixmap);
                if (pixmap->depth() == 1)
                    p->setBackgroundMode(Qt::TransparentMode);
            }
            if (mi->popup()) {                        // draw sub menu arrow
                int dim = (h-2*CE_ITEMFRAME) / 2;
                PrimitiveElement arrow;
                arrow = PE_ArrowRight;
                xpos = x+w - CE_ARROWHMARGIN - CE_ITEMFRAME - dim;
                if (act) {
                    if (!dis)
                        discol = white;
                    QPalette g2(discol, pal.highlight(),
                                    white, white,
                                    dis ? discol : white,
                                    discol, white);

                    drawPrimitive(arrow, p, QRect(xpos, y + h / 2 - dim / 2, dim, dim),
                                  g2, State_Enabled);
                } else {
                    drawPrimitive(arrow, p, QRect(xpos, y + h / 2 - dim / 2, dim, dim),
                                  pal, mi->isEnabled() ? State_Enabled : State_Default);
                }
            }

            break;
        }
#endif

    case CE_MenuBarItem:
        {
            bool active = flags & State_Active;
            bool hasFocus = flags & State_HasFocus;
            bool down = flags & State_Down;
            QRect pr = r;

            //p->fillRect(r, pal.brush(QPalette::Button));
            if (active || hasFocus) {
                QBrush b = pal.brush(QPalette::Button);
                if (active && down)
                    p->setBrushOrigin(p->brushOrigin() + QPoint(1,1));
                if (active && hasFocus)
                    qDrawShadeRect(p, r.x(), r.y(), r.width(), r.height(),
                                    pal, active && down, 1, 0, &b);
                if (active && down) {
                    pr.setRect(r.x() + 2 + PM_BUTTONSHIFTHORIZONTAL, //  pixelMetric(PM_ButtonShiftHorizontal, widget)
                                r.y() + 2 + PM_BUTTONSHIFTVERTICAL,   // pixelMetric(PM_ButtonShiftVertical, widget)
                                r.width()-4, r.height()-4);
                    p->setBrushOrigin(p->brushOrigin() - QPoint(1,1));
                }
            }
            if (opt.isDefault())
                break;

            QAction *mi = opt.action();
            QPixmap pix = mi->icon().pixmap(Qt::SmallIconSize, QIcon::Normal);
            drawItem(p, r, Qt::AlignCenter|Qt::TextShowMnemonic|Qt::TextDontClip|Qt::TextSingleLine, pal,
                      mi->isEnabled(), pix.isNull() ? 0 : &pix, mi->text(), -1,
                      &pal.color(QPalette::ButtonText));
            //QCommonStyle::drawControl(element, p, widget, pr, pal, how, opt);
            break;
        }

#ifdef QT3_SUPPORT
    case CE_Q3MenuBarItem:
        {
            bool active = flags & State_Active;
            bool hasFocus = flags & State_HasFocus;
            bool down = flags & State_Down;
            QRect pr = r;

            //p->fillRect(r, pal.brush(QPalette::Button));
            if (active || hasFocus) {
                QBrush b = pal.brush(QPalette::Button);
                if (active && down)
                    p->setBrushOrigin(p->brushOrigin() + QPoint(1,1));
                if (active && hasFocus)
                    qDrawShadeRect(p, r.x(), r.y(), r.width(), r.height(),
                                    pal, active && down, 1, 0, &b);
                if (active && down) {
                    pr.setRect(r.x() + 2 + PM_BUTTONSHIFTHORIZONTAL, //  pixelMetric(PM_ButtonShiftHorizontal, widget)
                                r.y() + 2 + PM_BUTTONSHIFTVERTICAL,   // pixelMetric(PM_ButtonShiftVertical, widget)
                                r.width()-4, r.height()-4);
                    p->setBrushOrigin(p->brushOrigin() - QPoint(1,1));
                }
            }
            if (opt.isDefault())
                break;

            Q3MenuItem *mi = opt.menuItem();
            drawItem(p, r, Qt::AlignCenter|Qt::TextShowMnemonic|Qt::TextDontClip|Qt::TextSingleLine, pal,
                      mi->isEnabled(), mi->pixmap(), mi->text(), -1,
                      &pal.color(QPalette::ButtonText));
            //QCommonStyle::drawControl(element, p, widget, pr, pal, how, opt);
            break;
        }
#endif

    case CE_CheckBox:
        {
            // many people expect to checkbox to be square, do that here.
            QRect ir = r;

            if (r.width() < r.height()) {
                ir.setTop(r.top() + (r.height() - r.width()) / 2);
                ir.setHeight(r.width());
            } else if (r.height() < r.width()) {
                ir.setLeft(r.left() + (r.width() - r.height()) / 2);
                ir.setWidth(r.height());
            }

            const QCheckBox *checkbox = (const QCheckBox *) widget;

            if (checkbox->isDown())
                flags |= State_Down;
            if (checkbox->state() == QButton::On)
                flags |= State_On;
            else if (checkbox->state() == QButton::Off)
                flags |= State_Off;
            else if (checkbox->state() == QButton::NoChange)
                flags |= State_NoChange;

            drawPrimitive(PE_Indicator, p, ir, pal, flags, opt);
            break;
        }

    case CE_CheckBoxLabel:
        {
            const QCheckBox *checkbox = (const QCheckBox *) widget;

            int alignment = QApplication::isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft;
            drawItem(p, r, alignment | Qt::AlignVCenter | Qt::TextShowMnemonic, pal,
                     flags & State_Enabled, checkbox->pixmap(), checkbox->text());

            if (checkbox->hasFocus()) {
                QRect fr = subRect(SR_CheckBoxFocusRect, widget);
                drawPrimitive(PE_FocusRect, p, fr, pal, flags);
            }
            break;
        }

    case CE_RadioButton:
        {
            // many people expect to checkbox to be square, do that here.
            QRect ir = r;

            if (r.width() < r.height()) {
                ir.setTop(r.top() + (r.height() - r.width()) / 2);
                ir.setHeight(r.width());
            } else if (r.height() < r.width()) {
                ir.setLeft(r.left() + (r.width() - r.height()) / 2);
                ir.setWidth(r.height());
            }

            const QRadioButton *radiobutton = (const QRadioButton *) widget;

            if (radiobutton->isDown())
                flags |= State_Down;
            if (radiobutton->state() == QButton::On)
                flags |= State_On;
            else if (radiobutton->state() == QButton::Off)
                flags |= State_Off;

            drawPrimitive(PE_ExclusiveIndicator, p, ir, pal, flags, opt);
            break;
        }

    case CE_RadioButtonLabel:
        {
            const QRadioButton *radiobutton = (const QRadioButton *) widget;

            int alignment = QApplication::isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft;
            drawItem(p, r, alignment | Qt::AlignVCenter | Qt::TextShowMnemonic, pal,
                     flags & State_Enabled, radiobutton->pixmap(), radiobutton->text());

            if (radiobutton->hasFocus()) {
                QRect fr = subRect(SR_RadioButtonFocusRect, widget);
                drawPrimitive(PE_FocusRect, p, fr, pal, flags);
            }
            break;
        }

    case CE_ProgressBarGroove:
        p->setPen(pal.foreground());
        p->drawRect(r);
        break;

#ifndef QT_NO_PROGRESSBAR
    case CE_ProgressBarContents:
        {
            const QProgressBar *progressbar = (const QProgressBar *) widget;

            bool reverse = QApplication::isRightToLeft();
            if (!progressbar->totalSteps()) {
                // draw busy indicator
                int w = r.width();
                int x = progressbar->progress() % (w * 2);
                if (x > w)
                    x = 2 * w - x;
                x = reverse ? r.right() - x : x + r.x();
                p->setPen(QPen(pal.highlight(), 4));
                p->drawLine(x, r.y() + 1, x, r.height() - 2);
            } else {
                const int unit_width = PM_PROGRESSBARCHUNKWIDTH; // pixelMetric(PM_ProgressBarChunkWidth, widget);
                int u = (r.width() - 4) / unit_width;
                int p_v = progressbar->progress();
                int t_s = progressbar->totalSteps();

                if (u > 0 && p_v >= INT_MAX / u && t_s >= u) {
                    // scale down to something usable.
                    p_v /= u;
                    t_s /= u;
                }

                int nu = (u * p_v + t_s / 2) / t_s;
                if (nu * unit_width > r.width() - 4)
                    nu--;

                // Draw nu units out of a possible u of unit_width width, each
                // a rectangle bordered by background color, all in a sunken panel
                // with a percentage text display at the end.
                int x = 0;
                int x0 = reverse ? r.right() - unit_width : r.x() + 2;
                for (int i=0; i<nu; i++) {
                    drawPrimitive(PE_ProgressBarChunk, p,
                                   QRect(x0+x, r.y(), unit_width, r.height()),
                                   pal, State_Default, opt);
                    x += reverse ? -unit_width: unit_width;
                }
            }
        }
        break;

    case CE_ProgressBarLabel:
        {
            const QProgressBar *progressbar = (const QProgressBar *) widget;
            drawItem(p, r, Qt::AlignCenter | Qt::TextSingleLine, pal, progressbar->isEnabled(), 0,
                     progressbar->progressString());
        }
        break;
#endif // QT_NO_PROGRESSBAR



    // Windows Style ====================================================================
    // ### Not optimized yet...

    case CE_ToolBoxTab:
        {
            drawPrimitive(PE_ButtonBevel, p, r, pal, flags);
            break;
        }


    // Common Style =====================================================================
    // ### Not optimized yet...

    case CE_MenuBarEmptyArea:
        {
            p->fillRect(r, widget->palette().color(QPalette::Background));
            break;
        }

#ifndef QT_NO_PUSHBUTTON
    case CE_PushButtonLabel:
        {
            const QPushButton *button = (const QPushButton *) widget;
            QRect ir = r;

            if (button->isDown() || button->isOn()) {
                flags |= State_Sunken;
                ir.moveBy(PM_BUTTONSHIFTHORIZONTAL, // pixelMetric(PM_ButtonShiftHorizontal, widget)
                          PM_BUTTONSHIFTVERTICAL);  // pixelMetric(PM_ButtonShiftVertical, widget));
            }

            if (button->isMenuButton()) {
                int mbi = PM_MENUBUTTONINDICATOR; // pixelMetric(PM_MenuButtonIndicator, widget);
                QRect ar(ir.right() - mbi, ir.y() + 2, mbi - 4, ir.height() - 4);
                drawPrimitive(PE_ArrowDown, p, ar, pal, flags, opt);
                ir.setWidth(ir.width() - mbi);
            }

            int tf=Qt::AlignVCenter | Qt::TextShowMnemonic;
#ifndef QT_NO_ICON
            if (button->iconSet() && ! button->iconSet()->isNull()) {
                QIcon::Mode mode =
                    button->isEnabled() ? QIcon::Normal : QIcon::Disabled;
                if (mode == QIcon::Normal && button->hasFocus())
                    mode = QIcon::Active;

                QIcon::State state = QIcon::Off;
                if (button->isToggleButton() && button->isOn())
                    state = QIcon::On;

                QPixmap pixmap = button->iconSet()->pixmap(Qt::SmallIconSize, mode, state);
                int pixw = pixmap.width();
                int pixh = pixmap.height();

                //Center the icon if there is neither text nor pixmap
                if (button->text().isEmpty() && !button->pixmap())
                    p->drawPixmap(ir.x() + ir.width() / 2 - pixw / 2, ir.y() + ir.height() / 2 - pixh / 2, pixmap);
                else
                    p->drawPixmap(ir.x() + 2, ir.y() + ir.height() / 2 - pixh / 2, pixmap);

                ir.moveBy(pixw + 4, 0);
                ir.setWidth(ir.width() - (pixw + 4));
                // left-align text if there is
                if (!button->text().isEmpty())
                    tf |= Qt::AlignLeft;
                else if (button->pixmap())
                    tf |= Qt::AlignHCenter;
            } else
#endif //QT_NO_ICON
                tf |= Qt::AlignHCenter;
            drawItem(p, ir, tf, pal,
                     flags & State_Enabled, button->pixmap(), button->text(),
                     button->text().length(), (flags & State_Sunken) ? &(pal.color(QPalette::Base)) : &(pal.color(QPalette::ButtonText)));

            if (flags & State_HasFocus)
                drawPrimitive(PE_FocusRect, p, subRect(SR_PushButtonFocusRect, widget),
                              pal, flags, QStyleOption(pal.foreground()));
            break;
        }
#endif // QT_NO_PUSHBUTTON

#ifndef QT_NO_TOOLBUTTON
    case CE_ToolButtonLabel:
        {
            const QToolButton *toolbutton = (const QToolButton *) widget;
            QRect rect = r;
            Qt::ArrowType arrowType = opt.isDefault()
                        ? Qt::DownArrow : opt.arrowType();

            if (flags & (State_Down | State_On))
                rect.moveBy(PM_BUTTONSHIFTHORIZONTAL, // pixelMetric(PM_ButtonShiftHorizontal, widget)
                            PM_BUTTONSHIFTVERTICAL);  // pixelMetric(PM_ButtonShiftVertical, widget));

            if (!opt.isDefault()) {
                PrimitiveElement pe;
                switch (arrowType) {
                case Qt::LeftArrow:  pe = PE_ArrowLeft;  break;
                case Qt::RightArrow: pe = PE_ArrowRight; break;
                case Qt::UpArrow:    pe = PE_ArrowUp;    break;
                case Qt::DownArrow:
                default:             pe = PE_ArrowDown;  break;
                }

                drawPrimitive(pe, p, rect, pal, flags, opt);
            } else {
                QColor btext = toolbutton->palette().color(QPalette::Foreground);

                if (toolbutton->iconSet().isNull() &&
                    ! toolbutton->text().isNull() &&
                    ! toolbutton->usesTextLabel()) {
                    drawItem(p, rect, Qt::AlignCenter | Qt::TextShowMnemonic, pal,
                             flags & State_Enabled, 0, toolbutton->text(),
                             toolbutton->text().length(), &btext);
                } else {
                    QPixmap pm;
                    Qt::IconSize size =
                        toolbutton->usesBigPixmap() ? Qt::LargeIconSize : Qt::SmallIconSize;
                    QIcon::State state =
                        toolbutton->isOn() ? QIcon::On : QIcon::Off;
                    QIcon::Mode mode;
                    if (! toolbutton->isEnabled())
                        mode = QIcon::Disabled;
                    else if (flags & (State_Down | State_On | State_Raised))
                        mode = QIcon::Active;
                    else
                        mode = QIcon::Normal;
                    pm = toolbutton->iconSet().pixmap(size, mode, state);

                    if (toolbutton->usesTextLabel()) {
                        if (toolbutton->textPosition() == QToolButton::Under) {
                            p->setFont(toolbutton->font());

                            QRect pr = rect, tr = rect;
                            int fh = p->fontMetrics().height();
                            pr.adjust(0, 1, 0, -fh-3);
                            tr.adjust(0, pr.bottom(), 0, -3);
                            drawItem(p, pr, Qt::AlignCenter, pal, true, &pm, QString::null);
                            drawItem(p, tr, Qt::AlignCenter | Qt::TextShowMnemonic, pal,
                                      flags & State_Enabled, 0, toolbutton->textLabel(),
                                      toolbutton->textLabel().length(), &btext);
                        } else {
                            p->setFont(toolbutton->font());

                            QRect pr = rect, tr = rect;
                            pr.setWidth(pm.width() + 8);
                            tr.adjust(pr.right(), 0, 0, 0);
                            drawItem(p, pr, Qt::AlignCenter, pal, true, &pm, QString::null);
                            drawItem(p, tr, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic, pal,
                                      flags & State_Enabled, 0, toolbutton->textLabel(),
                                      toolbutton->textLabel().length(), &btext);
                        }
                    } else {
                        drawItem(p, rect, Qt::AlignCenter, pal, true, &pm, QString::null);
                    }
                }
            }

            break;
        }
#endif // QT_NO_TOOLBUTTON

#ifndef QT_NO_HEADER
        case CE_HeaderLabel:
            {
                QRect rect = r;
                const QHeader* header = (const QHeader *) widget;
                int section = opt.headerSection();

                QIcon* icon = header->iconSet(section);
                if (icon) {
                    QPixmap pixmap = icon->pixmap(Qt::SmallIconSize,
                                                flags & State_Enabled ?
                                                QIcon::Normal : QIcon::Disabled);
                    int pixw = pixmap.width();
                    int pixh = pixmap.height();
                    // "pixh - 1" because of tricky integer division

                    QRect pixRect = rect;
                    pixRect.setY(rect.center().y() - (pixh - 1) / 2);
                    drawItem (p, pixRect, Qt::AlignVCenter, pal, flags & State_Enabled,
                            &pixmap, QString::null);
                    rect.setLeft(rect.left() + pixw + 2);
                }

                drawItem (p, rect, Qt::AlignVCenter, pal, flags & State_Enabled,
                        0, header->label(section), -1, &(pal.color(QPalette::ButtonText)));
                break;
            }
#endif // QT_NO_HEADER

#ifndef QT_NO_MAINWINDOW
        case CE_DockWindowEmptyArea:
            {
                p->fillRect(r, pal.background());
                switch (findLocation(p)) {
                case Qt::DockTop:
                    p->drawLine(r.bottomLeft(), r.bottomRight());
                    break;
                case Qt::DockBottom:
                    p->drawLine(r.topLeft(), r.topRight());
                    break;
                case Qt::DockLeft:
                    p->drawLine(r.topRight(), r.bottomRight());
                    break;
                case Qt::DockRight:
                    p->drawLine(r.topLeft(), r.bottomLeft());
                    break;
                }
                break;
            }
#endif // QT_NO_MAINWINDOW


    // --- Catch unknown elements ---
    default:
        // This debug message is only here to make sure we handle the complete style properly
        // We might remove this debug message before a full release
        qDebug("*** Not Implemented Yet ***  drawControl(control[0x%08x], p, w, r, pal, flags[0x%08x], opt[0x%08x])", control, flags, opt);
    }
    activePainter = 0;
}


void QPocketPCStyle::drawControlMask(ControlElement         control,
                                     QPainter                *p,
                                     const QWidget        *,
                                     const QRect        &r,
                                     const QStyleOption        &opt) const
{
    // Set active painter, for optimization
    activePainter = p;

    // Only use two colors to create a mask
    QPalette pal(Qt::color1, color1, color1, color1, color1, color1, color1, color1, Qt::color0);

    switch (control) {
    // Common Style =====================================================================
    // ### Not optimized yet...

    case CE_PushButton:
        drawPrimitive(PE_ButtonCommand, p, r, pal, State_Default, opt);
        break;

    case CE_CheckBox:
        drawPrimitive(PE_IndicatorMask, p, r, pal, State_Default, opt);
        break;

    case CE_RadioButton:
        drawPrimitive(PE_ExclusiveIndicatorMask, p, r, pal, State_Default, opt);
        break;

    // --- Catch unknown elements ---
    default:
        p->fillRect(r, Qt::color1);
        // This debug message is only here to make sure we handle the complete style properly
        // We might remove this debug message before a full release
        qDebug("*** Not Implemented Yet ***  drawControlMask(control[0x%08x], p, w, r, opt[0x%08x])", control, opt);
    }
    activePainter = 0;
}


void QPocketPCStyle::drawComplexControl(ComplexControl            complex,
                                        QPainter           *p,
                                        const QWidget           *widget,
                                        const QRect           &r,
                                        const QPalette     &pal,
                                        SFlags                    flags,
                                        SCFlags                    sub,
                                        SCFlags                    subActive,
                                        const QStyleOption &opt) const
{
    // Set subActive painter, for optimization
    activePainter = p;

    switch (complex) {
    case CC_SpinBox:
        {
            PrimitiveElement pe;
            const QSpinWidget * sw = (const QSpinWidget *) widget;

            if (sub & SC_SpinBoxFrame) {
                QPen old = p->pen();
                p->setPen(pal.foreground());
                p->drawRect(r);
                p->setPen(old);
            }

            if (sub & SC_SpinBoxUp) {
                flags = State_Default | State_Enabled;
                // Which button symbol?
                pe = sw->buttonSymbols() == QSpinWidget::PlusMinus
                        ? PE_SpinBoxPlus : PE_ArrowUp;

                QRect re = sw->upRect();
                // Button Up pressed?
                if (subActive == SC_SpinBoxUp) {
                    flags |= State_Down;
                    p->fillRect(re, pal.foreground());
                } else {
                    p->fillRect(re, pal.base());
                }

                // Draw separator
                QPen oldPen = p->pen();
                p->setPen(pal.foreground());
                p->drawLine(re.topLeft(), re.bottomLeft());
                p->setPen(oldPen);

                // Draw Arrow / Plus
                QPalette upal = pal;
                upal.setCurrentColorGroup(sw->isUpEnabled()?QPalette::Active:QPalette::Disabled);
                drawPrimitive(pe, p, re, upal, flags);
            }

            if (sub & SC_SpinBoxDown) {
                flags = State_Default | State_Enabled;
                // Which button symbol?
                pe = sw->buttonSymbols() == QSpinWidget::PlusMinus
                        ? PE_SpinBoxMinus : PE_ArrowDown;

                QRect re = sw->downRect();
                // Button Down pressed?
                if (subActive == SC_SpinBoxDown) {
                    flags |= State_Down;
                    p->fillRect(re, pal.foreground());
                } else {
                    p->fillRect(re, pal.base());
                }

                // Draw separator
                QPen oldPen = p->pen();
                p->setPen(pal.foreground());
                p->drawLine(re.topLeft(), re.bottomLeft());
                p->setPen(oldPen);

                // Draw Arrow / Minus
                QPalette dpal = pal;
                dpal.setCurrentColorGroup(sw->isDownEnabled()?QPalette::Active:QPalette::Disabled);
                drawPrimitive(pe, p, re, dpal, flags);
            }
/**/
            break;
        }

#ifndef QT_NO_COMBOBOX
    case CC_ComboBox:
        {
            if (sub & SC_ComboBoxArrow) {
                SFlags flags = State_Default;

                p->setBrush(pal.base());
                p->setPen(pal.foreground());
                p->drawRect(r);

                if (widget->isEnabled())
                    flags |= State_Enabled;

                if (subActive & State_Sunken)
                    flags |= State_Sunken;

                drawPrimitive(PE_ArrowDown, p, QRect(r.width() - 13, 0, 13, r.height()), pal, flags);
            }

            if (sub & SC_ComboBoxEditField) {
                const QComboBox * cb = (const QComboBox *) widget;

                if (cb->hasFocus() && !cb->editable()) {
                    p->fillRect(QRect(r.x() + 2, r.y() + 2, r.width() - 18, r.height() - 4), pal.brush(QPalette::Highlight));
                    p->setPen(pal.highlightedText());
                    p->setBackground(pal.highlight());
                }
            }
            break;
        }
#endif        // QT_NO_COMBOBOX

#ifndef QT_NO_LISTVIEW
    case CC_ListView:
        {
            const QListView *listview = (QListView*)widget;
            if (sub & SC_ListView) {
                p->fillRect(r, listview->viewport()->palette().background());
            }

            if (sub & (SC_ListViewExpand | SC_ListViewBranch)) {
                if (opt.isDefault())
                    break;

                QListViewItem *item = opt.listViewItem(),
                             *child = item->firstChild();

                int linetop = 0, linebot = 0, y = r.y();
                // each branch needs at most two lines, ie. four end points
                int dotoffset = (item->itemPos() + item->height() - y) %2;
                QPolygon dotlines(item->childCount() * 4);
                int c = 0;

                // skip the stuff above the exposed rectangle
                while (child && y + child->height() <= 0) {
                    y += child->totalHeight();
                    child = child->nextSibling();
                }

                int bx = r.width() / 2;

                // paint stuff in the magical area
                while (child && y < r.height()) {
                    linebot = y + child->height()/2;
                    if ((child->isExpandable() || child->childCount()) &&
                         (child->height() > 0)) {
                        // needs a box
                        p->setPen(pal.text());
                        p->drawRect(bx-4, linebot-4, 9, 9);
                        // plus or minus
                        p->drawLine(bx - 2, linebot, bx + 2, linebot);
                        if (!child->isOpen())
                            p->drawLine(bx, linebot - 2, bx, linebot + 2);
                        // dotlinery
                        dotlines[c++] = QPoint(bx, linetop);
                        dotlines[c++] = QPoint(bx, linebot - 5);
                        dotlines[c++] = QPoint(bx + 5, linebot);
                        dotlines[c++] = QPoint(r.width(), linebot);
                        linetop = linebot + 5;
                    } else {
                        // just dotlinery
                        dotlines[c++] = QPoint(bx+1, linebot);
                        dotlines[c++] = QPoint(r.width(), linebot);
                    }

                    y += child->totalHeight();
                    child = child->nextSibling();
                }

                // Expand line height to edge of rectangle if there's any
                // visible child below
                while (child && child->height() <= 0)
                    child = child->nextSibling();
                if (child)
                    linebot = r.height();

                if (linetop < linebot) {
                    dotlines[c++] = QPoint(bx, linetop);
                    dotlines[c++] = QPoint(bx, linebot);
                }

                p->setPen(pal.dark());

                static QBitmap *verticalLine = 0, *horizontalLine = 0;
                static QCleanupHandler<QBitmap> qlv_cleanup_bitmap;
                if (!verticalLine) {
                    // make 128*1 and 1*128 bitmaps that can be used for
                    // drawing the right sort of lines.
                    verticalLine = new QBitmap(1, 129, true);
                    horizontalLine = new QBitmap(128, 1, true);
                    QPolygon a(64);
                    QPainter p;
                    p.begin(verticalLine);
                    int i;
                    for(i=0; i<64; i++)
                        a.setPoint(i, 0, i*2+1);
                    p.setPen(Qt::color1);
                    p.drawPoints(a);
                    p.end();
                    //QApplication::flushX();
                    verticalLine->setMask(*verticalLine);
                    p.begin(horizontalLine);
                    for(i=0; i<64; i++)
                        a.setPoint(i, i*2+1, 0);
                    p.setPen(Qt::color1);
                    p.drawPoints(a);
                    p.end();
                    //QApplication::flushX();
                    horizontalLine->setMask(*horizontalLine);
                    qlv_cleanup_bitmap.add(&verticalLine);
                    qlv_cleanup_bitmap.add(&horizontalLine);
                }

                int line; // index into dotlines
                if (sub & SC_ListViewBranch) for(line = 0; line < c; line += 2) {
                    // assumptions here: lines are horizontal or vertical.
                    // lines always start with the numerically lowest
                    // coordinate.

                    // point ... relevant coordinate of current point
                    // end ..... same coordinate of the end of the current line
                    // other ... the other coordinate of the current point/line
                    if (dotlines[line].y() == dotlines[line+1].y()) {
                        int end = dotlines[line+1].x();
                        int point = dotlines[line].x();
                        int other = dotlines[line].y();
                        while(point < end) {
                            int i = 128;
                            if (i+point > end)
                                i = end-point;
                            p->drawPixmap(point, other, *horizontalLine,
                                           0, 0, i, 1);
                            point += i;
                        }
                    } else {
                        int end = dotlines[line+1].y();
                        int point = dotlines[line].y();
                        int other = dotlines[line].x();
                        int pixmapoffset = ((point & 1) != dotoffset) ? 1 : 0;
                        while(point < end) {
                            int i = 128;
                            if (i+point > end)
                                i = end-point;
                            p->drawPixmap(other, point, *verticalLine,
                                           0, pixmapoffset, 1, i);
                            point += i;
                        }
                    }
                }
            }
            break;
        }
#endif //QT_NO_LISTVIEW

#ifndef QT_NO_SLIDER
    case CC_Slider:
        {
            const QSlider *sl = (const QSlider *) widget;
            if (sub & SC_SliderGroove) {
                int tickOffset = pixelMetric(PM_SliderTickmarkOffset, sl);
                int thickness = pixelMetric(PM_SliderControlThickness, sl);
                int mid   = thickness / 2;
                int ticks = QSlider::Below; // sl->tickmarks();
                int len   = PM_SLIDERLENGTH; // pixelMetric(PM_SliderLength, sl);
                int x, y, wi, he;

                if (sl->orientation() == Qt::Horizontal) {
                    x = 0;
                    y = tickOffset;
                    wi = sl->width();
                    he = thickness;
                } else {
                    x = tickOffset;
                    y = 0;
                    wi = thickness*3;
                    he = sl->height();
                }

                if (ticks & QSlider::Above)
                    mid += len / 8;
                if (ticks & QSlider::Below)
                    mid -= len / 8;

                p->setPen(pal.foreground());
                if (sl->orientation() == Qt::Horizontal)
                    p->drawRect(x, y + mid - 2,  wi, 4);
                else
                    p->drawRect(x + mid - 2, y, 4, he);
            }

            if (sub & SC_SliderTickmarks) {
                int tickOffset = pixelMetric(PM_SliderTickmarkOffset, sl);
                int ticks = sl->tickmarks();
                int thickness = pixelMetric(PM_SliderControlThickness, sl);
                int len = pixelMetric(PM_SliderLength, sl);
                int available = pixelMetric(PM_SliderSpaceAvailable, sl);
                int interval = sl->tickInterval();
                int fudge = len / 2;
                int pos = 0;

                if (interval <= 0) {
                    interval = sl->lineStep();
                    if (qPositionFromValue(sl, interval, available) -
                        qPositionFromValue(sl, 0, available) < 3)
                        interval = sl->pageStep();
                }
                interval = qMax(interval, 1);

                p->setPen(pal.foreground());

                // Tickmarks below algorithm ---
                if (ticks & QSlider::Above) {
                    int v = sl->minValue();
                    while (v <= sl->maxValue() + 1) {
                        pos = qPositionFromValue(sl, v, available) + fudge;
                        if (sl->orientation() == Qt::Horizontal)
                            p->drawLine(pos, 0, pos, tickOffset-2);
                        else
                            p->drawLine(0, pos, tickOffset-2, pos);
                        v += interval;
                    }
                }

                if (ticks & QSlider::Below) {
                    int v = sl->minValue();
                    while (v <= sl->maxValue() + 1) {
                        pos = qPositionFromValue(sl, v, available) + fudge;
                        if (sl->orientation() == Qt::Horizontal)
                            p->drawLine(pos, tickOffset+thickness+1, pos, tickOffset+thickness+1 + available-2);
                        else
                            p->drawLine(tickOffset+thickness+1, pos, tickOffset+thickness+1 + available-2, pos);
                        v += interval;
                    }
                }
            }

            if (sub & SC_SliderHandle) {
                // 1111111
                // 1000001
                // 1000001
                // 1000001
                // 1000001
                // 1000001
                // *10001*
                // **101**
                // ***1***

                enum  SliderDir { SlUp, SlDown, SlLeft, SlRight };

                bool reverse       = QApplication::isRightToLeft();
                Qt::Orientation orient = sl->orientation();
                const QColor c0    = sl->palette().color(QPalette::Background);
                const QColor c1    = sl->palette().color(QPalette::Foreground);
                bool tickAbove     = sl->tickmarks() == QSlider::Above;
                bool tickBelow     = sl->tickmarks() == QSlider::Below;

                QRect re = subControlRect(CC_Slider, widget, SC_SliderHandle, opt);
                int x1 = re.x(),     y1 = re.y(),
                    wi = re.width(), he = re.height();
                int x2 = x1 + wi - 1;
                int y2 = y1 + he - 1;


                // Fill rect with background color (usually base)
                p->fillRect(x1, y1, wi, he, c0);

                if ((tickAbove && tickBelow) || (!tickAbove && !tickBelow)) {
                    p->drawRect(re);
                    return;
                }

                // If no tickmarks, then simply use plain box
                if (sl->hasFocus()) {
                    QRect re = subRect(SR_SliderFocusRect, sl);
                    drawPrimitive(PE_FocusRect, p, re, pal);
                }

                SliderDir dir;

                if (orient == Qt::Horizontal)
                    if (tickAbove)
                        dir = SlUp;
                    else
                        dir = SlDown;
                else
                    if (tickAbove)
                        dir = SlLeft;
                    else
                        dir = SlRight;

                QPolygon a;

                int d = 0;
                switch (dir) {
                case SlUp:
                    y1 = y1 + wi/2;
                    d =  (wi + 1) / 2 - 1;
                    a.setPoints(5, x1,y1, x1,y2, x2,y2, x2,y1, x1+d,y1-d);
                    break;
                case SlDown:
                    y2 = y2 - wi/2;
                    d =  (wi + 1) / 2 - 1;
                    a.setPoints(5, x1,y1, x1,y2, x1+d,y2+d, x2,y2, x2,y1);
                    break;
                case SlLeft:
                    d =  (he + 1) / 2 - 1;
                    x1 = x1 + he/2;
                    a.setPoints(5, x1,y1, x1-d,y1+d, x1,y2, x2,y2, x2,y1);
                    break;
                case SlRight:
                    d =  (he + 1) / 2 - 1;
                    x2 = x2 - he/2;
                    a.setPoints(5, x1,y1, x1,y2, x2,y2, x2+d,y1+d, x2,y1);
                    break;
                }

                //QBrush oldBrush = p->brush();
                //p->setBrush(pal.brush(QPalette::Button));
                //p->setPen(Qt::NoPen);
                //p->drawRect(x1, y1, x2-x1+1, y2-y1+1);
                p->drawPolygon(a);
//                p->setBrush(oldBrush);
/*
                if (dir != SlUp) {
                    p->setPen(c4);
                    p->drawLine(x1, y1, x2, y1);
                    p->setPen(c3);
                    p->drawLine(x1, y1+1, x2, y1+1);
                }
                if (dir != SlLeft) {
                    if (reverse)
                        p->setPen(c1);
                    else
                        p->setPen(c3);
                    p->drawLine(x1+1, y1+1, x1+1, y2);
                    if (reverse)
                        p->setPen(c0);
                    else
                        p->setPen(c4);
                    p->drawLine(x1, y1, x1, y2);
                }
                if (dir != SlRight) {
                    if (reverse)
                        p->setPen(c4);
                    else
                        p->setPen(c0);
                    p->drawLine(x2, y1, x2, y2);
                    if (reverse)
                        p->setPen(c3);
                    else
                        p->setPen(c1);
                    p->drawLine(x2-1, y1+1, x2-1, y2-1);
                }
                if (dir != SlDown) {
                    p->setPen(c0);
                    p->drawLine(x1, y2, x2, y2);
                    p->setPen(c1);
                    p->drawLine(x1+1, y2-1, x2-1, y2-1);
                }

                switch (dir) {
                case SlUp:
                    if (reverse)
                        p->setPen(c0);
                    else
                        p->setPen(c4);
                    p->drawLine(x1, y1, x1+d, y1-d);
                    if (reverse)
                        p->setPen(c4);
                    else
                        p->setPen(c0);
                    d = wi - d - 1;
                    p->drawLine(x2, y1, x2-d, y1-d);
                    d--;
                    if (reverse)
                        p->setPen(c1);
                    else
                        p->setPen(c3);
                    p->drawLine(x1+1, y1, x1+1+d, y1-d);
                    if (reverse)
                        p->setPen(c3);
                    else
                        p->setPen(c1);
                    p->drawLine(x2-1, y1, x2-1-d, y1-d);
                    break;
                case SlDown:
                    if (reverse)
                        p->setPen(c0);
                    else
                        p->setPen(c4);
                    p->drawLine(x1, y2, x1+d, y2+d);
                    if (reverse)
                        p->setPen(c4);
                    else
                        p->setPen(c0);
                    d = wi - d - 1;
                    p->drawLine(x2, y2, x2-d, y2+d);
                    d--;
                    if (reverse)
                        p->setPen(c1);
                    else
                        p->setPen(c3);
                    p->drawLine(x1+1, y2, x1+1+d, y2+d);
                    if (reverse)
                        p->setPen(c3);
                    else
                        p->setPen(c1);
                    p->drawLine(x2-1, y2, x2-1-d, y2+d);
                    break;
                case SlLeft:
                    p->setPen(c4);
                    p->drawLine(x1, y1, x1-d, y1+d);
                    p->setPen(c0);
                    d = he - d - 1;
                    p->drawLine(x1, y2, x1-d, y2-d);
                    d--;
                    p->setPen(c3);
                    p->drawLine(x1, y1+1, x1-d, y1+1+d);
                    p->setPen(c1);
                    p->drawLine(x1, y2-1, x1-d, y2-1-d);
                    break;
                case SlRight:
                    p->setPen(c4);
                    p->drawLine(x2, y1, x2+d, y1+d);
                    p->setPen(c0);
                    d = he - d - 1;
                    p->drawLine(x2, y2, x2+d, y2-d);
                    d--;
                    p->setPen(c3);
                    p->drawLine( x2, y1+1, x2+d, y1+1+d);
                    p->setPen(c1);
                    p->drawLine(x2, y2-1, x2+d, y2-1-d);
                    break;
                }
*/
            }
            break;
        }
#endif // QT_NO_SLIDER


    // Common Style =====================================================================
    // ### Not optimized yet...

#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar:
        {
            const QScrollBar *scrollbar = (const QScrollBar *) widget;
            QRect addline, subline, addpage, subpage, slider, first, last;
            bool maxedOut = (scrollbar->minValue() == scrollbar->maxValue());

            subline = subControlRect(complex, widget, SC_ScrollBarSubLine, opt);
            addline = subControlRect(complex, widget, SC_ScrollBarAddLine, opt);
            subpage = subControlRect(complex, widget, SC_ScrollBarSubPage, opt);
            addpage = subControlRect(complex, widget, SC_ScrollBarAddPage, opt);
            slider  = subControlRect(complex, widget, SC_ScrollBarSlider,  opt);
            first   = subControlRect(complex, widget, SC_ScrollBarFirst,   opt);
            last    = subControlRect(complex, widget, SC_ScrollBarLast,    opt);

            if ((sub & SC_ScrollBarSubLine) && subline.isValid())
                drawPrimitive(PE_ScrollBarSubLine, p, subline, pal,
                              ((maxedOut) ? State_Default : State_Enabled) |
                              ((subActive == SC_ScrollBarSubLine) ?
                               State_Down : State_Default) |
                              ((scrollbar->orientation() == Qt::Horizontal) ?
                               State_Horizontal : 0));
            if ((sub & SC_ScrollBarAddLine) && addline.isValid())
                drawPrimitive(PE_ScrollBarAddLine, p, addline, pal,
                              ((maxedOut) ? State_Default : State_Enabled) |
                              ((subActive == SC_ScrollBarAddLine) ?
                               State_Down : State_Default) |
                              ((scrollbar->orientation() == Qt::Horizontal) ?
                               State_Horizontal : 0));
            if ((sub & SC_ScrollBarSubPage) && subpage.isValid())
                drawPrimitive(PE_ScrollBarSubPage, p, subpage, pal,
                              ((maxedOut) ? State_Default : State_Enabled) |
                              ((subActive == SC_ScrollBarSubPage) ?
                               State_Down : State_Default) |
                              ((scrollbar->orientation() == Qt::Horizontal) ?
                               State_Horizontal : 0));
            if ((sub & SC_ScrollBarAddPage) && addpage.isValid())
                drawPrimitive(PE_ScrollBarAddPage, p, addpage, pal,
                              ((maxedOut) ? State_Default : State_Enabled) |
                              ((subActive == SC_ScrollBarAddPage) ?
                               State_Down : State_Default) |
                              ((scrollbar->orientation() == Qt::Horizontal) ?
                               State_Horizontal : 0));
            if ((sub & SC_ScrollBarFirst) && first.isValid())
                drawPrimitive(PE_ScrollBarFirst, p, first, pal,
                              ((maxedOut) ? State_Default : State_Enabled) |
                              ((subActive == SC_ScrollBarFirst) ?
                               State_Down : State_Default) |
                              ((scrollbar->orientation() == Qt::Horizontal) ?
                               State_Horizontal : 0));
            if ((sub & SC_ScrollBarLast) && last.isValid())
                drawPrimitive(PE_ScrollBarLast, p, last, pal,
                              ((maxedOut) ? State_Default : State_Enabled) |
                              ((subActive == SC_ScrollBarLast) ?
                               State_Down : State_Default) |
                              ((scrollbar->orientation() == Qt::Horizontal) ?
                               State_Horizontal : 0));
            if ((sub & SC_ScrollBarSlider) && slider.isValid()) {
                drawPrimitive(PE_ScrollBarSlider, p, slider, pal,
                              ((maxedOut) ? State_Default : State_Enabled) |
                              ((subActive == SC_ScrollBarSlider) ?
                               State_Down : State_Default) |
                              ((scrollbar->orientation() == Qt::Horizontal) ?
                               State_Horizontal : 0));

                // ### perhaps this should not be able to accept focus if maxedOut?
                if (scrollbar->hasFocus()) {
                    QRect fr(slider.x() + 2, slider.y() + 2,
                             slider.width() - 5, slider.height() - 5);
                    drawPrimitive(PE_FocusRect, p, fr, pal, State_Default);
                }
            }

            break;
        }
#endif // QT_NO_SCROLLBAR

#ifndef QT_NO_TOOLBUTTON
    case CC_ToolButton:
        {
            const QToolButton *toolbutton = (const QToolButton *) widget;

            QPalette c = pal;
            //if (toolbutton->backgroundMode() != QPalette::Button)
                //c.setBrush(QPalette::Button,
                            //toolbutton->palette().color(QPalette::Background));
            QRect button, menuarea;
            button   = visualRect(subControlRect(complex, widget, SC_ToolButton, opt), widget);
            menuarea = visualRect(subControlRect(complex, widget, SC_ToolButtonMenu, opt), widget);

            SFlags bflags = flags,
                   mflags = flags;

            if (subActive & SC_ToolButton)
                bflags |= State_Down;
            if (subActive & SC_ToolButtonMenu)
                mflags |= State_Down;

            if (sub & SC_ToolButton) {
                QToolBar *tb = ::qobject_cast<QToolBar*>(toolbutton->parentWidget());
                // Keep line, if docked and not movable (movable have frame)
                if (tb && !tb->isMovingEnabled()) {
                    switch (findLocation(toolbutton->parentWidget())) {
                    case Qt::DockTop:
                        p->drawLine(r.bottomLeft(), r.bottomRight());
                        break;
                    case Qt::DockBottom:
                        p->drawLine(r.topLeft(), r.topRight());
                        break;
                    case Qt::DockLeft:
                        p->drawLine(r.topRight(), r.bottomRight());
                        break;
                    case Qt::DockRight:
                        p->drawLine(r.topLeft(), r.bottomLeft());
                        break;
                    }
                }

                if (bflags & (State_Down | State_On | State_Raised)) {
                    drawPrimitive(PE_ButtonTool, p, button, c, bflags, opt);
                }
                /*
                else if (toolbutton->parentWidget() &&
                          toolbutton->parentWidget()->backgroundPixmap() &&
                          ! toolbutton->parentWidget()->backgroundPixmap()->isNull()) {
                    QPixmap pixmap =
                        *(toolbutton->parentWidget()->backgroundPixmap());

                    p->drawTiledPixmap(r, pixmap, toolbutton->pos());
                }
                */
            }

            if (sub & SC_ToolButtonMenu) {
                if (mflags & (State_Down | State_On | State_Raised))
                    drawPrimitive(PE_ButtonDropDown, p, menuarea, c, mflags, opt);
                drawPrimitive(PE_ArrowDown, p, menuarea, c, mflags, opt);
            }

            if (toolbutton->hasFocus() && !toolbutton->focusProxy()) {
                QRect fr = toolbutton->rect();
                fr.adjust(2, 2, -2, -2);
                drawPrimitive(PE_FocusRect, p, fr, c);
            }

            break;
        }
#endif // QT_NO_TOOLBUTTON

#ifndef QT_NO_TITLEBAR
    case CC_TitleBar:
        {
            const QTitleBar *titlebar = (const QTitleBar *) widget;
            if (sub & SC_TitleBarLabel) {
                QPalette titlePal = titlebar->palette();

                QColor left = titlePal.highlight();
                QColor right = titlePal.base();

                if (left != right) {
                    double rS = left.red();
                    double gS = left.green();
                    double bS = left.blue();

                    const double rD = double(right.red() - rS) / titlebar->width();
                    const double gD = double(right.green() - gS) / titlebar->width();
                    const double bD = double(right.blue() - bS) / titlebar->width();

                    const int w = titlebar->width();
                    for (int sx = 0; sx < w; sx++) {
                        rS+=rD;
                        gS+=gD;
                        bS+=bD;
                        p->setPen(QColor((int)rS, (int)gS, (int)bS));
                        p->drawLine(sx, 0, sx, titlebar->height());
                    }
                } else {
                    p->fillRect(titlebar->rect(), left);
                }

                QRect ir = visualRect(subControlRect(CC_TitleBar, widget, SC_TitleBarLabel), widget);

                p->setPen(titlePal.highlightedText());
                p->drawText(ir.x()+2, ir.y(), ir.width()-2, ir.height(),
                            Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, titlebar->visibleText());
            }

            QRect ir;
            bool down = false;
            QPixmap pm;

            if (sub & SC_TitleBarCloseButton) {
                ir = visualRect(subControlRect(CC_TitleBar, widget, SC_TitleBarCloseButton), widget);
                down = subActive & SC_TitleBarCloseButton;
                if (widget->testWFlags(Qt::WA_WState_Tool)
#ifndef QT_NO_MAINWINDOW
                     || ::qobject_cast<QDockWindow*>(widget)
#endif
                   )
                    pm = stylePixmap(SP_DockWindowCloseButton, widget);
                else
                    pm = stylePixmap(SP_TitleBarCloseButton, widget);
                drawPrimitive(PE_ButtonTool, p, ir, titlebar->palette(),
                              down ? State_Down : State_Raised);

                p->save();
                if(down)
                    p->translate(PM_BUTTONSHIFTHORIZONTAL, // pixelMetric(PM_ButtonShiftHorizontal, widget),
                                 PM_BUTTONSHIFTVERTICAL);  // pixelMetric(PM_ButtonShiftVertical, widget));
                drawItem(p, ir, Qt::AlignCenter, titlebar->palette(), true, &pm, QString::null);
                p->restore();
            }

            if (titlebar->window()) {
                if (sub & SC_TitleBarMaxButton) {
                    ir = visualRect(subControlRect(CC_TitleBar, widget, SC_TitleBarMaxButton), widget);

                    down = subActive & SC_TitleBarMaxButton;
                    pm = QPixmap(stylePixmap(SP_TitleBarMaxButton, widget));
                    drawPrimitive(PE_ButtonTool, p, ir, titlebar->palette(),
                                  down ? State_Down : State_Raised);

                    p->save();
                    if(down)
                        p->translate(PM_BUTTONSHIFTHORIZONTAL, // pixelMetric(PM_ButtonShiftHorizontal, widget),
                                     PM_BUTTONSHIFTVERTICAL);  // pixelMetric(PM_ButtonShiftVertical, widget));
                    drawItem(p, ir, Qt::AlignCenter, titlebar->palette(), true, &pm, QString::null);
                    p->restore();
                }

                if (sub & SC_TitleBarNormalButton || sub & SC_TitleBarMinButton) {
                    ir = visualRect(subControlRect(CC_TitleBar, widget, SC_TitleBarMinButton), widget);
                    QStyle::SubControl ctrl = (complex & SC_TitleBarNormalButton ?
                                               SC_TitleBarNormalButton :
                                               SC_TitleBarMinButton);
                    QStyle::StylePixmap spixmap = (complex & SC_TitleBarNormalButton ?
                                                   SP_TitleBarNormalButton :
                                                   SP_TitleBarMinButton);
                    down = subActive & ctrl;
                    pm = QPixmap(stylePixmap(spixmap, widget));
                    drawPrimitive(PE_ButtonTool, p, ir, titlebar->palette(),
                                  down ? State_Down : State_Raised);

                    p->save();
                    if(down)
                        p->translate(PM_BUTTONSHIFTHORIZONTAL, // pixelMetric(PM_ButtonShiftHorizontal, widget),
                                     PM_BUTTONSHIFTVERTICAL);  // pixelMetric(PM_ButtonShiftVertical, widget));
                    drawItem(p, ir, Qt::AlignCenter, titlebar->palette(), true, &pm, QString::null);
                    p->restore();
                }

                if (sub & SC_TitleBarShadeButton) {
                    ir = visualRect(subControlRect(CC_TitleBar, widget, SC_TitleBarShadeButton), widget);

                    down = subActive & SC_TitleBarShadeButton;
                    pm = QPixmap(stylePixmap(SP_TitleBarShadeButton, widget));
                    drawPrimitive(PE_ButtonTool, p, ir, titlebar->palette(),
                                  down ? State_Down : State_Raised);
                    p->save();
                    if(down)
                        p->translate(PM_BUTTONSHIFTHORIZONTAL, // pixelMetric(PM_ButtonShiftHorizontal, widget),
                                     PM_BUTTONSHIFTVERTICAL);  // pixelMetric(PM_ButtonShiftVertical, widget));
                    drawItem(p, ir, Qt::AlignCenter, titlebar->palette(), true, &pm, QString::null);
                    p->restore();
                }

                if (sub & SC_TitleBarUnshadeButton) {
                    ir = visualRect(subControlRect(CC_TitleBar, widget, SC_TitleBarUnshadeButton), widget);

                    down = subActive & SC_TitleBarUnshadeButton;
                    pm = QPixmap(stylePixmap(SP_TitleBarUnshadeButton, widget));
                    drawPrimitive(PE_ButtonTool, p, ir, titlebar->palette(),
                                  down ? State_Down : State_Raised);
                    p->save();
                    if(down)
                        p->translate(PM_BUTTONSHIFTHORIZONTAL, // pixelMetric(PM_ButtonShiftHorizontal, widget),
                                     PM_BUTTONSHIFTVERTICAL);  // pixelMetric(PM_ButtonShiftVertical, widget));
                    drawItem(p, ir, Qt::AlignCenter, titlebar->palette(), true, &pm, QString::null);
                    p->restore();
                }
            }
#ifndef QT_NO_WIDGET_TOPEXTRA
            if (sub & SC_TitleBarSysMenu) {
                if (!titlebar->windowIcon().isNull()) {
                    ir = visualRect(subControlRect(CC_TitleBar, widget, SC_TitleBarSysMenu), widget);
                    drawItem(p, ir, Qt::AlignCenter, titlebar->palette(), true, &(titlebar->windowIcon()), QString::null);
                }
            }
#endif
            break;
        }
#endif //QT_NO_TITLEBAR


    // --- Catch unknown elements ---
    default:
        // This debug message is only here to make sure we handle the complete style properly
        // We might remove this debug message before a full release
        qDebug("*** Not Implemented Yet ***  drawComplexControl(complex[0x%08x], p, w, r, pal, flags[0x%08x], sub[0x%08x], subsubActive[0x%08x], opt[0x%08x])", complex, flags, sub, subActive, opt);
    }
    activePainter = 0;
}


void QPocketPCStyle::drawComplexControlMask(ComplexControl      complex,
                                            QPainter               *p,
                                            const QWidget      *widget,
                                            const QRect               &r,
                                            const QStyleOption &opt) const
{
    // Set active painter, for optimization
    activePainter = p;

    switch (complex) {
    // --- Catch unknown elements ---
    default:
        p->fillRect(r, Qt::color1);
        // This debug message is only here to make sure we handle the complete style properly
        // We might remove this debug message before a full release
        qDebug("*** Not Implemented Yet ***  drawComplexControlMask(control[0x%08x], p, w, r, opt[0x%08x])", complex, opt);
    }
    activePainter = 0;
}

QPixmap QPocketPCStyle::stylePixmap(PixmapType                pixmapType,
                                    const QPixmap      &pix,
                                    const QPalette     &pal,
                                    const QStyleOption &opt) const
{
    return QPixmap();
}

QPixmap QPocketPCStyle::stylePixmap(StylePixmap         stylepixmap,
                                    const QWidget      *widget,
                                    const QStyleOption &opt) const
{
    switch (stylepixmap) {
    case SP_TitleBarShadeButton:
        return QPixmap((const char **)qt_shade_xpm);
    case SP_TitleBarUnshadeButton:
        return QPixmap((const char **)qt_unshade_xpm);
    case SP_TitleBarNormalButton:
        return QPixmap((const char **)qt_normalizeup_xpm);
    case SP_TitleBarMinButton:
        return QPixmap((const char **)qt_minimize_xpm);
    case SP_TitleBarMaxButton:
        return QPixmap((const char **)qt_maximize_xpm);
    case SP_TitleBarCloseButton:
        return QPixmap((const char **)qt_close_xpm);
    case SP_DockWindowCloseButton:
        return QPixmap((const char **)dock_window_close_xpm);
    }
    qDebug("*** Not Implemented Yet ***  stylePixmap(stylepixmap[0x%08x], w, opt[0x%08x])", stylepixmap, opt);
    return QPixmap();
}


#endif // QT_NO_STYLE_POCKETPC
