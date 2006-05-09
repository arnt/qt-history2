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

#include "qwindowsstyle.h"
#include "qwindowsstyle_p.h"

#if !defined(QT_NO_STYLE_WINDOWS) || defined(QT_PLUGIN)

#include "qapplication.h"
#include "qbitmap.h"
#include "qdockwidget.h"
#include "qdrawutil.h" // for now
#include "qevent.h"
#include "qmenu.h"
#include "qmenubar.h"
#include "qpaintengine.h"
#include "qpainter.h"
#include "qprogressbar.h"
#include "qrubberband.h"
#include "qstyleoption.h"
#include "qtabbar.h"
#include "qwidget.h"
#include "qdebug.h"
#include "qmainwindow.h"

#if defined(Q_WS_WIN)
#include "qt_windows.h"
#  ifndef COLOR_GRADIENTACTIVECAPTION
#    define COLOR_GRADIENTACTIVECAPTION     27
#  endif
#  ifndef COLOR_GRADIENTINACTIVECAPTION
#    define COLOR_GRADIENTINACTIVECAPTION   28
#  endif
#endif

#include <limits.h>

static const int windowsItemFrame        =  2; // menu item frame width
static const int windowsSepHeight        =  9; // separator item height
static const int windowsItemHMargin      =  3; // menu item hor text margin
static const int windowsItemVMargin      =  2; // menu item ver text margin
static const int windowsArrowHMargin	 =  6; // arrow horizontal margin
static const int windowsTabSpacing	 = 12; // space between text and tab
static const int windowsCheckMarkHMargin =  2; // horiz. margins of check mark
static const int windowsRightBorder      = 15; // right border on windows
static const int windowsCheckMarkWidth   = 12; // checkmarks width on windows

static bool use2000style = true;

enum QSliderDirection { SlUp, SlDown, SlLeft, SlRight };

/*
    \internal
*/
QWindowsStylePrivate::QWindowsStylePrivate()
    : alt_down(false), menuBarTimer(0), animationFps(10), animateTimer(0), animateStep(0)
{
}

// Returns true if the toplevel parent of \a widget has seen the Alt-key
bool QWindowsStylePrivate::hasSeenAlt(const QWidget *widget) const
{
    widget = widget->window();
    return seenAlt.contains(widget);
}

/*!
    \reimp
*/
void QWindowsStyle::timerEvent(QTimerEvent *event)
{
#ifndef QT_NO_PROGRESSBAR
    Q_D(QWindowsStyle);
    if (event->timerId() == d->animateTimer) {
        Q_ASSERT(d->animationFps> 0);
        d->animateStep = d->startTime.elapsed() / (1000 / d->animationFps);
        foreach (QProgressBar *bar, d->bars) {
            if ((bar->minimum() == 0 && bar->maximum() == 0))
                bar->update();
        }
    }
#endif // QT_NO_PROGRESSBAR
    event->ignore();
}

/*!
    \reimp
*/
bool QWindowsStyle::eventFilter(QObject *o, QEvent *e)
{
    // Records Alt- and Focus events
    if (!o->isWidgetType())
        return QObject::eventFilter(o, e);

    QWidget *widget = ::qobject_cast<QWidget*>(o);
    Q_D(QWindowsStyle);
    switch(e->type()) {
    case QEvent::KeyPress:
        if (static_cast<QKeyEvent *>(e)->key() == Qt::Key_Alt) {
            widget = widget->window();

            // Alt has been pressed - find all widgets that care
            QList<QWidget *> l = qFindChildren<QWidget *>(widget);
            for (int pos=0 ; pos < l.size() ; ++pos) {
                QWidget *w = l.at(pos);
                if (w->isWindow() || !w->isVisible() ||
                    w->style()->styleHint(SH_UnderlineShortcut, 0, w))
                    l.removeAt(pos);
            }
            // Update states before repainting
            d->seenAlt.append(widget);
            d->alt_down = true;

            // Repaint all relevant widgets
            for (int pos = 0; pos < l.size(); ++pos)
                l.at(pos)->update();
        }
        break;
    case QEvent::KeyRelease:
	if (static_cast<QKeyEvent*>(e)->key() == Qt::Key_Alt) {
	    widget = widget->window();

	    // Update state and repaint the menubars.
	    d->alt_down = false;
#ifndef QT_NO_MENUBAR
            QList<QMenuBar *> l = qFindChildren<QMenuBar *>(widget);
            for (int i = 0; i < l.size(); ++i)
                l.at(i)->update();
#endif
	}
	break;
    case QEvent::Close:
        // Reset widget when closing
        d->seenAlt.removeAll(widget);
        d->seenAlt.removeAll(widget->window());
        break;
#ifndef QT_NO_PROGRESSBAR
    case QEvent::StyleChange:
    case QEvent::Show:
        if (QProgressBar *bar = qobject_cast<QProgressBar *>(o)) {
            d->bars << bar;
            if (d->bars.size() == 1) {
                Q_ASSERT(d->animationFps> 0);
                d->animateTimer = startTimer(1000 / d->animationFps);
            }
        }
        break;
    case QEvent::Destroy:
        d->bars.removeAll(reinterpret_cast<QProgressBar *>(o));
        break;
    case QEvent::Hide:
        if (QProgressBar *bar = qobject_cast<QProgressBar *>(o)) {
            d->bars.removeAll(bar);
            if (d->bars.isEmpty()) {
                killTimer(d->animateTimer);
                d->animateTimer = 0;
            }
        }
        break;
#endif // QT_NO_PROGRESSBAR
    default:
        break;
    }
    return QCommonStyle::eventFilter(o, e);
}

/*!
    \class QWindowsStyle qwindowsstyle.h
    \brief The QWindowsStyle class provides a Microsoft Windows-like look and feel.

    \ingroup appearance

    This style is Qt's default GUI style on Windows.

    \img qwindowsstyle.png
    \sa QWindowsXPStyle, QMacStyle, QPlastiqueStyle, QCDEStyle, QMotifStyle
*/

/*!
    Constructs a QWindowsStyle object.
*/
QWindowsStyle::QWindowsStyle() : QCommonStyle(*new QWindowsStylePrivate)
{
#if defined(Q_OS_WIN32)
    use2000style = QSysInfo::WindowsVersion != QSysInfo::WV_NT && QSysInfo::WindowsVersion != QSysInfo::WV_95;
#endif
}

/*!
    \internal

    Constructs a QWindowsStyle object.
*/
QWindowsStyle::QWindowsStyle(QWindowsStylePrivate &dd) : QCommonStyle(dd)
{
#if defined(Q_OS_WIN32)
    use2000style = QSysInfo::WindowsVersion != QSysInfo::WV_NT && QSysInfo::WindowsVersion != QSysInfo::WV_95;
#endif
}


/*! Destroys the QWindowsStyle object. */
QWindowsStyle::~QWindowsStyle()
{
}

#ifdef Q_WS_WIN
static inline QRgb colorref2qrgb(COLORREF col)
{
    return qRgb(GetRValue(col), GetGValue(col), GetBValue(col));
}
#endif

/*! \reimp */
void QWindowsStyle::polish(QApplication *app)
{
    QWindowsStylePrivate *d = const_cast<QWindowsStylePrivate*>(d_func());
    // We only need the overhead when shortcuts are sometimes hidden
    if (!styleHint(SH_UnderlineShortcut, 0) && app)
        app->installEventFilter(this);

    d->activeCaptionColor = app->palette().highlight().color();
    d->activeGradientCaptionColor = app->palette().highlight() .color();
    d->inactiveCaptionColor = app->palette().dark().color();
    d->inactiveGradientCaptionColor = app->palette().dark().color();
    d->inactiveCaptionText = app->palette().background().color();

#if defined(Q_WS_WIN) //fetch native titlebar colors
    if(app->desktopSettingsAware()){
        DWORD activeCaption = GetSysColor(COLOR_ACTIVECAPTION);
        DWORD gradientActiveCaption = GetSysColor(COLOR_GRADIENTACTIVECAPTION);
        DWORD inactiveCaption = GetSysColor(COLOR_INACTIVECAPTION);
        DWORD gradientInactiveCaption = GetSysColor(COLOR_GRADIENTINACTIVECAPTION);
        DWORD inactiveCaptionText = GetSysColor(COLOR_INACTIVECAPTIONTEXT);
        d->activeCaptionColor = colorref2qrgb(activeCaption);
        d->activeGradientCaptionColor = colorref2qrgb(gradientActiveCaption);
        d->inactiveCaptionColor = colorref2qrgb(inactiveCaption);
        d->inactiveGradientCaptionColor = colorref2qrgb(gradientInactiveCaption);
        d->inactiveCaptionText = colorref2qrgb(inactiveCaptionText);
    }
#endif
}

/*! \reimp */
void QWindowsStyle::unpolish(QApplication *app)
{
    app->removeEventFilter(this);
}

/*! \reimp */
void QWindowsStyle::polish(QWidget *widget)
{
    QCommonStyle::polish(widget);
#ifndef QT_NO_PROGRESSBAR
    if (qobject_cast<QProgressBar *>(widget))
        widget->installEventFilter(this);
#endif
}

/*! \reimp */
void QWindowsStyle::unpolish(QWidget *widget)
{
    QCommonStyle::unpolish(widget);
#ifndef QT_NO_PROGRESSBAR
    if (qobject_cast<QProgressBar *>(widget))
        widget->removeEventFilter(this);
#endif
}

/*! \reimp */
void QWindowsStyle::polish(QPalette &pal)
{
    QCommonStyle::polish(pal);
}

/*!
  \reimp
*/
int QWindowsStyle::pixelMetric(PixelMetric pm, const QStyleOption *opt, const QWidget *widget) const
{
    int ret;

    switch (pm) {
    case PM_ButtonDefaultIndicator:
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        ret = 1;
        break;
#ifndef QT_NO_TABBAR
    case PM_TabBarTabShiftHorizontal:
        ret = 0;
        break;
    case PM_TabBarTabShiftVertical:
        ret = 2;
        break;
#endif
    case PM_MaximumDragDistance:
        ret = 60;
        break;

#ifndef QT_NO_SLIDER
    case PM_SliderLength:
        ret = 11;
        break;

        // Returns the number of pixels to use for the business part of the
        // slider (i.e., the non-tickmark portion). The remaining space is shared
        // equally between the tickmark regions.
    case PM_SliderControlThickness:
        if (const QStyleOptionSlider *sl = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            int space = (sl->orientation == Qt::Horizontal) ? sl->rect.height() : sl->rect.width();
            int ticks = sl->tickPosition;
            int n = 0;
            if (ticks & QSlider::TicksAbove)
                ++n;
            if (ticks & QSlider::TicksBelow)
                ++n;
            if (!n) {
                ret = space;
                break;
            }

            int thick = 6;        // Magic constant to get 5 + 16 + 5
            if (ticks != QSlider::TicksBothSides && ticks != QSlider::NoTicks)
                thick += pixelMetric(PM_SliderLength, sl, widget) / 4;

            space -= thick;
            if (space > 0)
                thick += (space * 2) / (n + 2);
            ret = thick;
        } else {
            ret = 0;
        }
        break;
#endif // QT_NO_SLIDER

#ifndef QT_NO_MENU
    case PM_MenuBarHMargin:
        ret = 0;
        break;

    case PM_MenuBarVMargin:
        ret = 0;
        break;

    case PM_MenuBarPanelWidth:
        ret = 0;
        break;

    case PM_SmallIconSize:
        ret = 16;
        break;

    case PM_LargeIconSize:
        ret = 32;
        break;

    case PM_IconViewIconSize:
        ret = pixelMetric(PM_LargeIconSize, opt, widget);
        break;

    case PM_ToolBarIconSize:
        ret = 24;
        break;
    case PM_DockWidgetTitleMargin:
        ret = 3;
        break;
#if defined(Q_WS_WIN)
    case PM_DockWidgetFrameWidth:
        ret = GetSystemMetrics(SM_CXFRAME);
        break;
#else
    case PM_DockWidgetFrameWidth:
        ret = 4;
        break;
#endif // Q_WS_WIN
    break;

#endif // QT_NO_MENU


#if defined(Q_WS_WIN)
    case PM_TitleBarHeight:
        if (widget && (widget->windowType() == Qt::Tool)) {
            // MS always use one less than they say
#if defined(Q_OS_TEMP)
            ret = GetSystemMetrics(SM_CYCAPTION) - 1;
#else
            ret = GetSystemMetrics(SM_CYSMCAPTION) - 1;
#endif
        } else {
            ret = GetSystemMetrics(SM_CYCAPTION) - 1;
        }

        break;

    case PM_ScrollBarExtent:
        {
#ifndef Q_OS_TEMP
            NONCLIENTMETRICS ncm;
            ncm.cbSize = sizeof(NONCLIENTMETRICS);
            if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0))
                ret = qMax(ncm.iScrollHeight, ncm.iScrollWidth);
            else
#endif
                ret = QCommonStyle::pixelMetric(pm, opt, widget);
        }
        break;
#endif // Q_WS_WIN

    case PM_SplitterWidth:
        ret = qMax(6, QApplication::globalStrut().width());
        break;

#if defined(Q_WS_WIN)
    case PM_MDIFrameWidth:
        ret = GetSystemMetrics(SM_CYFRAME);
        break;
#endif
    case PM_ToolBarItemMargin:
        ret = 1;
        break;
    case PM_ToolBarItemSpacing:
        ret = 0;
        break;
    case PM_ToolBarHandleExtent:
        ret = 10;
        break;
    default:
        ret = QCommonStyle::pixelMetric(pm, opt, widget);
        break;
    }

    return ret;
}

#ifndef QT_NO_IMAGEFORMAT_XPM

static const char * const qt_menu_xpm[] = {
"16 16 11 1",
"  c #000000",
", c #336600",
". c #99CC00",
"X c #666600",
"o c #999933",
"+ c #333300",
"@ c #669900",
"# c #999900",
"$ c #336633",
"% c #666633",
"& c #99CC33",
"................",
"................",
".....#,++X#.....",
"....X      X....",
"...X  Xo#%  X&..",
"..#  o..&@o  o..",
".., X..#+ @X X..",
"..+ o.o+ +o# +..",
"..+ #o+  +## +..",
".., %@ ++ +, X..",
"..#  o@oo+   #..",
"...X  X##$   o..",
"....X        X..",
"....&oX++X#oX...",
"................",
"................"};

static const char * const qt_close_xpm[] = {
"10 10 2 1",
"# c #000000",
". c None",
"..........",
".##....##.",
"..##..##..",
"...####...",
"....##....",
"...####...",
"..##..##..",
".##....##.",
"..........",
".........."};

static const char * const qt_maximize_xpm[]={
"10 10 2 1",
"# c #000000",
". c None",
"#########.",
"#########.",
"#.......#.",
"#.......#.",
"#.......#.",
"#.......#.",
"#.......#.",
"#.......#.",
"#########.",
".........."};

static const char * const qt_minimize_xpm[] = {
"10 10 2 1",
"# c #000000",
". c None",
"..........",
"..........",
"..........",
"..........",
"..........",
"..........",
"..........",
".#######..",
".#######..",
".........."};

static const char * const qt_normalizeup_xpm[] = {
"10 10 2 1",
"# c #000000",
". c None",
"...######.",
"...######.",
"...#....#.",
".######.#.",
".######.#.",
".#....###.",
".#....#...",
".#....#...",
".######...",
".........."};

static const char * const qt_help_xpm[] = {
"10 10 2 1",
". c None",
"# c #000000",
"..........",
"..######..",
".##....##.",
"......##..",
".....##...",
"....##....",
"....##....",
"..........",
"....##....",
".........."};

static const char * const qt_shade_xpm[] = {
"10 10 2 1",
"# c #000000",
". c None",
"..........",
"..........",
"..........",
"..........",
"....#.....",
"...###....",
"..#####...",
".#######..",
"..........",
".........."};

static const char * const qt_unshade_xpm[] = {
"10 10 2 1",
"# c #000000",
". c None",
"..........",
"..........",
"..........",
".#######..",
"..#####...",
"...###....",
"....#.....",
"..........",
"..........",
".........."};

static const char * dock_widget_close_xpm[] = {
"8 8 2 1",
"# c #000000",
". c None",
"........",
".##..##.",
"..####..",
"...##...",
"..####..",
".##..##.",
"........",
"........"};

/* XPM */
static const char * const information_xpm[]={
"32 32 5 1",
". c None",
"c c #000000",
"* c #999999",
"a c #ffffff",
"b c #0000ff",
"...........********.............",
"........***aaaaaaaa***..........",
"......**aaaaaaaaaaaaaa**........",
".....*aaaaaaaaaaaaaaaaaa*.......",
"....*aaaaaaaabbbbaaaaaaaac......",
"...*aaaaaaaabbbbbbaaaaaaaac.....",
"..*aaaaaaaaabbbbbbaaaaaaaaac....",
".*aaaaaaaaaaabbbbaaaaaaaaaaac...",
".*aaaaaaaaaaaaaaaaaaaaaaaaaac*..",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaaac*.",
"*aaaaaaaaaabbbbbbbaaaaaaaaaaac*.",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
".*aaaaaaaaaaabbbbbaaaaaaaaaac***",
".*aaaaaaaaaaabbbbbaaaaaaaaaac***",
"..*aaaaaaaaaabbbbbaaaaaaaaac***.",
"...caaaaaaabbbbbbbbbaaaaaac****.",
"....caaaaaaaaaaaaaaaaaaaac****..",
".....caaaaaaaaaaaaaaaaaac****...",
"......ccaaaaaaaaaaaaaacc****....",
".......*cccaaaaaaaaccc*****.....",
"........***cccaaaac*******......",
"..........****caaac*****........",
".............*caaac**...........",
"...............caac**...........",
"................cac**...........",
".................cc**...........",
"..................***...........",
"...................**..........."};
/* XPM */
static const char* const warning_xpm[]={
"32 32 4 1",
". c None",
"a c #ffff00",
"* c #000000",
"b c #999999",
".............***................",
"............*aaa*...............",
"...........*aaaaa*b.............",
"...........*aaaaa*bb............",
"..........*aaaaaaa*bb...........",
"..........*aaaaaaa*bb...........",
".........*aaaaaaaaa*bb..........",
".........*aaaaaaaaa*bb..........",
"........*aaaaaaaaaaa*bb.........",
"........*aaaa***aaaa*bb.........",
".......*aaaa*****aaaa*bb........",
".......*aaaa*****aaaa*bb........",
"......*aaaaa*****aaaaa*bb.......",
"......*aaaaa*****aaaaa*bb.......",
".....*aaaaaa*****aaaaaa*bb......",
".....*aaaaaa*****aaaaaa*bb......",
"....*aaaaaaaa***aaaaaaaa*bb.....",
"....*aaaaaaaa***aaaaaaaa*bb.....",
"...*aaaaaaaaa***aaaaaaaaa*bb....",
"...*aaaaaaaaaa*aaaaaaaaaa*bb....",
"..*aaaaaaaaaaa*aaaaaaaaaaa*bb...",
"..*aaaaaaaaaaaaaaaaaaaaaaa*bb...",
".*aaaaaaaaaaaa**aaaaaaaaaaa*bb..",
".*aaaaaaaaaaa****aaaaaaaaaa*bb..",
"*aaaaaaaaaaaa****aaaaaaaaaaa*bb.",
"*aaaaaaaaaaaaa**aaaaaaaaaaaa*bb.",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaa*bbb",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaa*bbb",
".*aaaaaaaaaaaaaaaaaaaaaaaaa*bbbb",
"..*************************bbbbb",
"....bbbbbbbbbbbbbbbbbbbbbbbbbbb.",
".....bbbbbbbbbbbbbbbbbbbbbbbbb.."};
/* XPM */
static const char* const critical_xpm[]={
"32 32 4 1",
". c None",
"a c #999999",
"* c #ff0000",
"b c #ffffff",
"...........********.............",
".........************...........",
".......****************.........",
"......******************........",
".....********************a......",
"....**********************a.....",
"...************************a....",
"..*******b**********b*******a...",
"..******bbb********bbb******a...",
".******bbbbb******bbbbb******a..",
".*******bbbbb****bbbbb*******a..",
"*********bbbbb**bbbbb*********a.",
"**********bbbbbbbbbb**********a.",
"***********bbbbbbbb***********aa",
"************bbbbbb************aa",
"************bbbbbb************aa",
"***********bbbbbbbb***********aa",
"**********bbbbbbbbbb**********aa",
"*********bbbbb**bbbbb*********aa",
".*******bbbbb****bbbbb*******aa.",
".******bbbbb******bbbbb******aa.",
"..******bbb********bbb******aaa.",
"..*******b**********b*******aa..",
"...************************aaa..",
"....**********************aaa...",
"....a********************aaa....",
".....a******************aaa.....",
"......a****************aaa......",
".......aa************aaaa.......",
".........aa********aaaaa........",
"...........aaaaaaaaaaa..........",
".............aaaaaaa............"};
/* XPM */
static const char *const question_xpm[] = {
"32 32 5 1",
". c None",
"c c #000000",
"* c #999999",
"a c #ffffff",
"b c #0000ff",
"...........********.............",
"........***aaaaaaaa***..........",
"......**aaaaaaaaaaaaaa**........",
".....*aaaaaaaaaaaaaaaaaa*.......",
"....*aaaaaaaaaaaaaaaaaaaac......",
"...*aaaaaaaabbbbbbaaaaaaaac.....",
"..*aaaaaaaabaaabbbbaaaaaaaac....",
".*aaaaaaaabbaaaabbbbaaaaaaaac...",
".*aaaaaaaabbbbaabbbbaaaaaaaac*..",
"*aaaaaaaaabbbbaabbbbaaaaaaaaac*.",
"*aaaaaaaaaabbaabbbbaaaaaaaaaac*.",
"*aaaaaaaaaaaaabbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaaabbbaaaaaaaaaaaac**",
"*aaaaaaaaaaaaabbaaaaaaaaaaaaac**",
"*aaaaaaaaaaaaabbaaaaaaaaaaaaac**",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaaac**",
".*aaaaaaaaaaaabbaaaaaaaaaaaac***",
".*aaaaaaaaaaabbbbaaaaaaaaaaac***",
"..*aaaaaaaaaabbbbaaaaaaaaaac***.",
"...caaaaaaaaaabbaaaaaaaaaac****.",
"....caaaaaaaaaaaaaaaaaaaac****..",
".....caaaaaaaaaaaaaaaaaac****...",
"......ccaaaaaaaaaaaaaacc****....",
".......*cccaaaaaaaaccc*****.....",
"........***cccaaaac*******......",
"..........****caaac*****........",
".............*caaac**...........",
"...............caac**...........",
"................cac**...........",
".................cc**...........",
"..................***...........",
"...................**..........."};
/* XPM */
static const char* const dir_open_xpm[]={
    "16 16 6 1",
    ". c None",
    "b c #ffff00",
    "d c #000000",
    "* c #999999",
    "c c #cccccc",
    "a c #ffffff",
    "................",
    "................",
    "...*****........",
    "..*aaaaa*.......",
    ".*abcbcba******.",
    ".*acbcbcaaaaaa*d",
    ".*abcbcbcbcbcb*d",
    "*************b*d",
    "*aaaaaaaaaa**c*d",
    "*abcbcbcbcbbd**d",
    ".*abcbcbcbcbcd*d",
    ".*acbcbcbcbcbd*d",
    "..*acbcbcbcbb*dd",
    "..*************d",
    "...ddddddddddddd",
    "................"};

/* XPM */
static const char * const dir_closed_xpm[]={
    "16 16 6 1",
    ". c None",
    "b c #ffff00",
    "d c #000000",
    "* c #999999",
    "a c #cccccc",
    "c c #ffffff",
    "................",
    "................",
    "..*****.........",
    ".*ababa*........",
    "*abababa******..",
    "*cccccccccccc*d.",
    "*cbababababab*d.",
    "*cabababababa*d.",
    "*cbababababab*d.",
    "*cabababababa*d.",
    "*cbababababab*d.",
    "*cabababababa*d.",
    "*cbababababab*d.",
    "**************d.",
    ".dddddddddddddd.",
    "................"};

/* XPM */
static const char * const dir_link_xpm[]={
    "16 16 10 1",
    "h c #808080",
    "g c #a0a0a0",
    "d c #000000",
    "b c #ffff00",
    "f c #303030",
    "# c #999999",
    "a c #cccccc",
    "e c #585858",
    "c c #ffffff",
    ". c None",
    "................",
    "................",
    "..#####.........",
    ".#ababa#........",
    "#abababa######..",
    "#cccccccccccc#d.",
    "#cbababababab#d.",
    "#cabababababa#d.",
    "#cbababdddddddd.",
    "#cababadccccccd.",
    "#cbababdcececcd.",
    "#cababadcefdfcd.",
    "#cbababdccgdhcd.",
    "#######dccchccd.",
    ".dddddddddddddd.",
    "................"};
/* XPM */
static const char* const file_xpm[]={
    "16 16 5 1",
    ". c #7f7f7f",
    "# c None",
    "c c #000000",
    "b c #bfbfbf",
    "a c #ffffff",
    "################",
    "..........######",
    ".aaaaaaaab.#####",
    ".aaaaaaaaba.####",
    ".aaaaaaaacccc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".bbbbbbbbbbbc###",
    "ccccccccccccc###"};
/* XPM */
static const char * const file_link_xpm[]={
    "16 16 10 1",
    "h c #808080",
    "g c #a0a0a0",
    "d c #c3c3c3",
    ". c #7f7f7f",
    "c c #000000",
    "b c #bfbfbf",
    "f c #303030",
    "e c #585858",
    "a c #ffffff",
    "# c None",
    "################",
    "..........######",
    ".aaaaaaaab.#####",
    ".aaaaaaaaba.####",
    ".aaaaaaaacccc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaadc###",
    ".aaaaaaaaaadc###",
    ".aaaacccccccc###",
    ".aaaacaaaaaac###",
    ".aaaacaeaeaac###",
    ".aaaacaefcfac###",
    ".aaaacaagchac###",
    ".ddddcaaahaac###",
    "ccccccccccccc###"};



#endif //QT_NO_IMAGEFORMAT_XPM

#ifdef Q_WS_WIN
QPixmap convertHIconToPixmap( const HICON icon)
{
    bool foundAlpha = false;
    HDC screenDevice = qt_win_display_dc();
    HDC hdc = CreateCompatibleDC(screenDevice);

    ICONINFO iconinfo;
    GetIconInfo(icon, &iconinfo); //x and y Hotspot describes the icon center

    //create image
    HBITMAP winBitmap = CreateBitmap(iconinfo.xHotspot * 2, iconinfo.yHotspot * 2, 1, 32, 0);
    HGDIOBJ oldhdc = SelectObject(hdc, winBitmap);
    DrawIconEx( hdc, 0, 0, icon, iconinfo.xHotspot * 2, iconinfo.yHotspot * 2, 0, 0, DI_NORMAL);
    QPixmap::HBitmapFormat alphaType = QPixmap::PremultipliedAlpha;

    BITMAP bitmapData;
    GetObject(iconinfo.hbmColor, sizeof(BITMAP), &bitmapData);

    QPixmap iconpixmap = QPixmap::fromWinHBITMAP(winBitmap, alphaType);
    QImage img = iconpixmap.toImage();

    if ( bitmapData.bmBitsPixel == 32 ) { //only check 32 bit images for alpha
        for (int y = 0 ; y < iconpixmap.height() && !foundAlpha ; y++) {
            QRgb *scanLine= reinterpret_cast<QRgb *>(img.scanLine(y));
            for (int x = 0; x < img.width() ; x++) {
                if (qAlpha(scanLine[x]) != 0) {
                    foundAlpha = true;
                    break;
                }
            }
        }
    }

    if (!foundAlpha) {
        //If no alpha was found, we use the mask to set alpha values
        HBITMAP winMask = CreateBitmap(iconinfo.xHotspot * 2, iconinfo.yHotspot * 2, 1, 32, 0);
        SelectObject(hdc, winMask);
        DrawIconEx( hdc, 0, 0, icon, iconinfo.xHotspot * 2, iconinfo.yHotspot * 2, 0, 0, DI_MASK);

        QPixmap maskPixmap = QPixmap::fromWinHBITMAP(winMask, alphaType);
        QImage mask = maskPixmap.toImage();

        for (int y = 0 ; y< iconpixmap.height() ; y++){
            QRgb *scanlineImage = reinterpret_cast<QRgb *>(img.scanLine(y));
            QRgb *scanlineMask = reinterpret_cast<QRgb *>(mask.scanLine(y));
            for (int x = 0; x < img.width() ; x++){
                if (qRed(scanlineMask[x]) != 0)
                    scanlineImage[x] = 0; //mask out this pixel
                else
                    scanlineImage[x] |= 0xff000000; // set the alpha channel to 255
            }
        }
        DeleteObject(winMask);
    }

    //dispose resources created by iconinfo call
    DeleteObject(iconinfo.hbmMask);
    DeleteObject(iconinfo.hbmColor);

    SelectObject(hdc, oldhdc); //restore state
    DeleteDC(hdc);
    DeleteObject(winBitmap);
    return QPixmap::fromImage(img);
}

QPixmap loadIconFromShell32( int resourceId, int size )
{
    HMODULE hmod = LoadLibraryA("shell32.dll");
    if( hmod ) {
        HICON iconHandle = (HICON)LoadImage(hmod, MAKEINTRESOURCE(resourceId), IMAGE_ICON, size, size, 0);
        if( iconHandle ) {
            QPixmap iconpixmap = convertHIconToPixmap( iconHandle );
            DestroyIcon(iconHandle);
            return iconpixmap;
        }
    }
    return QPixmap();
}
#endif

/*!
 \reimp
 */
QPixmap QWindowsStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                                      const QWidget *widget) const
{
#ifdef Q_WS_WIN
    QPixmap desktopIcon;
    switch(standardPixmap) {
    case SP_DriveCDIcon:
    case SP_DriveDVDIcon:
        {
            desktopIcon = loadIconFromShell32(12, 16);
            break;
        }
    case SP_DriveNetIcon:
        {
            desktopIcon = loadIconFromShell32(10, 16);
            break;
        }
    case SP_DriveHDIcon:
        {
            desktopIcon = loadIconFromShell32(9, 16);
            break;
        }
    case SP_DriveFDIcon:
        {
            desktopIcon = loadIconFromShell32(7, 16);
            break;
        }
    case SP_FileIcon:
        {
            desktopIcon = loadIconFromShell32(1, 16);
            break;
        }
    case SP_FileLinkIcon:
        {
            desktopIcon = loadIconFromShell32(1, 16);
            QPainter painter(&desktopIcon);
            QPixmap link = loadIconFromShell32(30, 16);
            painter.drawPixmap(0, 0, 16, 16, link);
            break;
        }
    case SP_DirLinkIcon:
        {
            desktopIcon = loadIconFromShell32(4, 16);
            QPainter painter(&desktopIcon);
            QPixmap link = loadIconFromShell32(30, 16);
            painter.drawPixmap(0, 0, 16, 16, link);
            break;
        }
    case SP_DirClosedIcon:
        {
            desktopIcon = loadIconFromShell32(4, 16);
            break;
        }
    case SP_DesktopIcon:
        {
            desktopIcon = loadIconFromShell32(35, 16);
            break;
        }
    case SP_ComputerIcon:
        {
            desktopIcon = loadIconFromShell32(16, 16);
            break;
        }
    case SP_DirOpenIcon:
        {
            desktopIcon = loadIconFromShell32(5, 16);
            break;
        }
    case SP_FileDialogNewFolder:
        {
            desktopIcon = loadIconFromShell32(319, 16);
            break;
        }
    case SP_FileDialogToParent:
        {
            desktopIcon = loadIconFromShell32(255, 16);
            break;
        }
    case SP_TrashIcon:
        {
            desktopIcon = loadIconFromShell32(191, 16);
            break;
        }
    case SP_MessageBoxInformation:
        {
            HICON iconHandle = LoadIcon(NULL, IDI_INFORMATION);
            desktopIcon = convertHIconToPixmap( iconHandle );
            DestroyIcon(iconHandle);
            break;
        }
    case SP_MessageBoxWarning:
        {
            HICON iconHandle = LoadIcon(NULL, IDI_WARNING);
            desktopIcon = convertHIconToPixmap( iconHandle );
            DestroyIcon(iconHandle);
            break;
        }
    case SP_MessageBoxCritical:
        {
            HICON iconHandle = LoadIcon(NULL, IDI_ERROR);
            desktopIcon = convertHIconToPixmap( iconHandle );
            DestroyIcon(iconHandle);
            break;
        }
    case SP_MessageBoxQuestion:
        {
            HICON iconHandle = LoadIcon(NULL, IDI_QUESTION);
            desktopIcon = convertHIconToPixmap( iconHandle );
            DestroyIcon(iconHandle);
            break;
        }
    }
    if (!desktopIcon.isNull()) {
        return desktopIcon;
    }
#endif
#ifndef QT_NO_IMAGEFORMAT_XPM
    switch (standardPixmap) {
    case SP_TitleBarMenuButton:
        return QPixmap((const char **)qt_menu_xpm);
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
    case SP_TitleBarContextHelpButton:
        return QPixmap((const char **)qt_help_xpm);
    case SP_DockWidgetCloseButton:
        return QPixmap((const char **)dock_widget_close_xpm);
    case SP_MessageBoxInformation:
        return QPixmap((const char **)information_xpm);
    case SP_MessageBoxWarning:
        return QPixmap((const char **)warning_xpm);
    case SP_MessageBoxCritical:
        return QPixmap((const char **)critical_xpm);
    case SP_MessageBoxQuestion:
        return QPixmap((const char **)question_xpm);
    default:
        break;
    }
#endif //QT_NO_IMAGEFORMAT_XPM
    return QCommonStyle::standardPixmap(standardPixmap, opt, widget);
}

/*! \reimp */
int QWindowsStyle::styleHint(StyleHint hint, const QStyleOption *opt, const QWidget *widget,
                             QStyleHintReturn *returnData) const
{
    int ret = 0;

    switch (hint) {
    case SH_EtchDisabledText:
    case SH_Slider_SnapToValue:
    case SH_PrintDialog_RightAlignButtons:
    case SH_FontDialog_SelectAssociatedText:
    case SH_Menu_AllowActiveAndDisabled:
    case SH_MenuBar_AltKeyNavigation:
    case SH_MenuBar_MouseTracking:
    case SH_Menu_MouseTracking:
    case SH_ComboBox_ListMouseTracking:
    case SH_ScrollBar_StopMouseOverSlider:
    case SH_MainWindow_SpaceBelowMenuBar:
        ret = 1;

        break;
    case SH_ItemView_ChangeHighlightOnFocus:
#if defined(Q_WS_WIN)
        if (QSysInfo::WindowsVersion != QSysInfo::WV_95 && QSysInfo::WindowsVersion != QSysInfo::WV_NT)
            ret = 1;
        else
#endif
            ret = 0;
        break;
    case SH_ToolBox_SelectedPageTitleBold:
        ret = 0;
        break;

#if defined(Q_WS_WIN)
    case SH_UnderlineShortcut:
        ret = 1;
        if (QSysInfo::WindowsVersion != QSysInfo::WV_95
            && QSysInfo::WindowsVersion != QSysInfo::WV_98
            && QSysInfo::WindowsVersion != QSysInfo::WV_NT) {
            BOOL cues;
            SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &cues, 0);
            ret = int(cues);
            // Do nothing if we always paint underlines
            Q_D(const QWindowsStyle);
            if (!ret && widget && d) {
#ifndef QT_NO_MENUBAR
                const QMenuBar *menuBar = ::qobject_cast<const QMenuBar*>(widget);
                if (!menuBar && ::qobject_cast<const QMenu *>(widget)) {
                    QWidget *w = QApplication::activeWindow();
                    if (w && w != widget)
                        menuBar = qFindChild<QMenuBar *>(w);
                }
                // If we paint a menubar draw underlines if it has focus, or if alt is down,
                // or if a popup menu belonging to the menubar is active and paints underlines
                if (menuBar) {
                    if (menuBar->hasFocus() || d->altDown())
                        ret = 1;
                    // Otherwise draw underlines if the toplevel widget has seen an alt-press
                } else
#endif // QT_NO_MENUBAR
                if (d->hasSeenAlt(widget)) {
                    ret = 1;
                }
            }
        }
        break;
#endif
#ifndef QT_NO_RUBBERBAND
    case SH_RubberBand_Mask:
        if (const QStyleOptionRubberBand *rbOpt = qstyleoption_cast<const QStyleOptionRubberBand *>(opt)) {
            ret = 0;
            if (rbOpt->shape == QRubberBand::Rectangle) {
                ret = true;
                if(QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask*>(returnData)) {
                    mask->region = opt->rect;
                    int size = 1;
                    if (widget && widget->isWindow())
                        size = 4;
                    mask->region -= opt->rect.adjusted(size, size, -size, -size);
                }
            }
        }
        break;
#endif // QT_NO_RUBBERBAND
    default:
        ret = QCommonStyle::styleHint(hint, opt, widget, returnData);
        break;
    }
    return ret;
}

/*! \reimp */
void QWindowsStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                                  const QWidget *w) const
{
    // Used to restore across fallthrough cases. Currently only used in PE_IndicatorCheckBox
    bool doRestore = false;

    switch (pe) {
#ifndef QT_NO_TOOLBAR
  case PE_IndicatorToolBarSeparator:
        {
            QRect rect = opt->rect;
            const int margin = 2;
            if(opt->state & State_Horizontal){
                const int offset = rect.width()/2;
                p->setPen(QPen(opt->palette.dark().color()));
                p->drawLine(rect.bottomLeft().x() + offset,
                            rect.bottomLeft().y() - margin,
                            rect.topLeft().x() + offset,
                            rect.topLeft().y() + margin);
                p->setPen(QPen(opt->palette.light().color()));
                p->drawLine(rect.bottomLeft().x() + offset + 1,
                            rect.bottomLeft().y() - margin,
                            rect.topLeft().x() + offset + 1,
                            rect.topLeft().y() + margin);
            }
            else{ //Draw vertical separator
                const int offset = rect.height()/2;
                p->setPen(QPen(opt->palette.dark().color()));
                p->drawLine(rect.topLeft().x() + margin ,
                            rect.topLeft().y() + offset,
                            rect.topRight().x() - margin,
                            rect.topRight().y() + offset);
                p->setPen(QPen(opt->palette.light().color()));
                p->drawLine(rect.topLeft().x() + margin ,
                            rect.topLeft().y() + offset + 1,
                            rect.topRight().x() - margin,
                            rect.topRight().y() + offset + 1);
            }
        }
        break;
    case PE_IndicatorToolBarHandle:
        p->save();
        p->translate(opt->rect.x(), opt->rect.y());
        if (opt->state & State_Horizontal) {
            int x = opt->rect.width() / 2 - 4;
            if (QApplication::layoutDirection() == Qt::RightToLeft)
                x -= 2;
            if (opt->rect.height() > 4) {
                qDrawShadePanel(p, x, 2, 3, opt->rect.height() - 4,
                                opt->palette, false, 1, 0);
                qDrawShadePanel(p, x + 3, 2, 3, opt->rect.height() - 4,
                                opt->palette, false, 1, 0);
            }
        } else {
            if (opt->rect.width() > 4) {
                int y = opt->rect.height() / 2 - 4;
                qDrawShadePanel(p, 2, y, opt->rect.width() - 4, 3,
                                opt->palette, false, 1, 0);
                qDrawShadePanel(p, 2, y + 3, opt->rect.width() - 4, 3,
                                opt->palette, false, 1, 0);
            }
        }
        p->restore();
        break;

#endif // QT_NO_TOOLBAR
    case PE_FrameButtonTool:
    case PE_PanelButtonTool: {
#ifndef QT_NO_DOCKWIDGET
        if (w && w->inherits("QDockWidgetTitleButton")) {
           if (const QDockWidget *dw = qobject_cast<const QDockWidget *>(w->parent()))
                if (dw->isFloating()){
                    qDrawWinButton(p, opt->rect.adjusted(1, 1, 0, 0), opt->palette, opt->state & (State_Sunken | State_On),
                           &opt->palette.button());

                    return;
                }
        }
#endif // QT_NO_DOCKWIDGET
        QBrush fill;
        bool stippled;
        bool panel = (pe == PE_PanelButtonTool);
        if ((!(opt->state & State_Sunken ))
            && (!(opt->state & State_Enabled)
                || ((opt->state & State_Enabled ) && !(opt->state & State_MouseOver)))
            && (opt->state & State_On) && use2000style) {
            fill = QBrush(opt->palette.light().color(), Qt::Dense4Pattern);
            stippled = true;
        } else {
            fill = opt->palette.brush(QPalette::Button);
            stippled = false;
        }

        if (opt->state & (State_Raised | State_Sunken | State_On)) {
            if (opt->state & State_AutoRaise) {
                if(opt->state & (State_Enabled | State_Sunken | State_On)){
                    if (panel)
                        qDrawShadePanel(p, opt->rect, opt->palette,
                                        opt->state & (State_Sunken | State_On), 1, &fill);
                    else
                        qDrawShadeRect(p, opt->rect, opt->palette,
                                       opt->state & (State_Sunken | State_On), 1);
                }
                if (stippled) {
                    p->setPen(opt->palette.button().color());
                    p->drawRect(opt->rect.adjusted(1,1,-2,-2));
                }
            } else {
                qDrawWinButton(p, opt->rect, opt->palette,
                               opt->state & (State_Sunken | State_On), panel ? &fill : 0);
            }
        } else {
            p->fillRect(opt->rect, fill);
        }
        break; }
    case PE_PanelButtonCommand:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QBrush fill;
            State flags = opt->state;
            QPalette pal = opt->palette;
            QRect r = opt->rect;
            if (! (flags & State_Sunken) && (flags & State_On))
                fill = QBrush(pal.light().color(), Qt::Dense4Pattern);
            else
                fill = pal.brush(QPalette::Button);

            if (btn->features & QStyleOptionButton::DefaultButton && flags & State_Sunken) {
                p->setPen(pal.dark().color());
                p->setBrush(fill);
                p->drawRect(r.adjusted(0, 0, -1, -1));
            } else if (flags & (State_Raised | State_Sunken | State_On | State_Sunken)) {
                qDrawWinButton(p, r, pal, flags & (State_Sunken | State_On),
                               &fill);
            } else {
                p->fillRect(r, fill);
            }
        }
        break;
    case PE_FrameDefaultButton: {
        p->setPen(opt->palette.shadow().color());
        QRect rect = opt->rect;
        rect.adjust(0, 0, -1, -1);
        p->drawRect(rect);
        break;
    }
    case PE_IndicatorArrowUp:
    case PE_IndicatorArrowDown:
    case PE_IndicatorArrowRight:
    case PE_IndicatorArrowLeft: {
        QPoint points[7];
        switch (pe) {
            case PE_IndicatorArrowUp:
                points[0] = QPoint(-4, 1);
                points[1] = QPoint(2, 1);
                points[2] = QPoint(-3, 0);
                points[3] = QPoint(1, 0);
                points[4] = QPoint(-2, -1);
                points[5] = QPoint(0, -1);
                points[6] = QPoint(-1, -2);
                break;
            case PE_IndicatorArrowDown:
                points[0] = QPoint(-4, -2);
                points[1] = QPoint(2, -2);
                points[2] = QPoint(-3, -1);
                points[3] = QPoint(1, -1);
                points[4] = QPoint(-2, 0);
                points[5] = QPoint(0, 0);
                points[6] = QPoint(-1, 1);
                break;
            case PE_IndicatorArrowRight:
                points[0] = QPoint(-2, -3);
                points[1] = QPoint(-2, 3);
                points[2] = QPoint(-1, -2);
                points[3] = QPoint(-1, 2);
                points[4] = QPoint(0, -1);
                points[5] = QPoint(0, 1);
                points[6] = QPoint(1, 0);
                break;
            case PE_IndicatorArrowLeft:
                points[0] = QPoint(0, -3);
                points[1] = QPoint(0, 3);
                points[2] = QPoint(-1, -2);
                points[3] = QPoint(-1, 2);
                points[4] = QPoint(-2, -1);
                points[5] = QPoint(-2, 1);
                points[6] = QPoint(-3, 0);
                break;
            default:
                break;
        }
        p->save();
        if (opt->state & State_Sunken)
            p->translate(pixelMetric(PM_ButtonShiftHorizontal),
                         pixelMetric(PM_ButtonShiftVertical));
        if (opt->state & State_Enabled) {
            p->translate(opt->rect.x() + opt->rect.width() / 2,
                         opt->rect.y() + opt->rect.height() / 2);
            p->setPen(opt->palette.buttonText().color());
            p->drawLine(points[0], points[1]);
            p->drawLine(points[2], points[3]);
            p->drawLine(points[4], points[5]);
            p->drawPoint(points[6]);
        } else {
            p->translate(opt->rect.x() + opt->rect.width() / 2 + 1,
                         opt->rect.y() + opt->rect.height() / 2 + 1);
            p->setPen(opt->palette.light().color());
            p->drawLine(points[0], points[1]);
            p->drawLine(points[2], points[3]);
            p->drawLine(points[4], points[5]);
            p->drawPoint(points[6]);
            p->translate(-1, -1);
            p->setPen(opt->palette.mid().color());
            p->drawLine(points[0], points[1]);
            p->drawLine(points[2], points[3]);
            p->drawLine(points[4], points[5]);
            p->drawPoint(points[6]);
        }
        p->restore();
        break; }
    case PE_IndicatorCheckBox: {
        QBrush fill;
        if (opt->state & State_NoChange)
            fill = QBrush(opt->palette.base().color(), Qt::Dense4Pattern);
        else if (opt->state & State_Sunken)
            fill = opt->palette.button();
        else if (opt->state & State_Enabled)
            fill = opt->palette.base();
        else
            fill = opt->palette.background();
        p->save();
        doRestore = true;
        qDrawWinPanel(p, opt->rect, opt->palette, true, &fill);
        if (opt->state & State_NoChange)
            p->setPen(opt->palette.dark().color());
        else
            p->setPen(opt->palette.text().color());
        } // Fall through!
    case PE_IndicatorViewItemCheck:
    case PE_Q3CheckListIndicator:
        if (!doRestore) {
            p->save();
            doRestore = true;
        }
        if (pe == PE_Q3CheckListIndicator || pe == PE_IndicatorViewItemCheck) {
            const QStyleOptionViewItem *itemViewOpt = qstyleoption_cast<const QStyleOptionViewItem *>(opt);
            p->setPen(itemViewOpt
                      && itemViewOpt->showDecorationSelected
                      && opt->state & State_Selected
                        ? opt->palette.highlightedText().color()
                        : opt->palette.text().color());
            if (opt->state & State_NoChange)
                p->setBrush(opt->palette.brush(QPalette::Button));
            p->drawRect(opt->rect.x() + 1, opt->rect.y() + 1, 11, 11);
        }
        if (!(opt->state & State_Off)) {
            QLineF lines[7];
            int i, xx, yy;
            xx = opt->rect.x() + 3;
            yy = opt->rect.y() + 5;
            for (i = 0; i < 3; ++i) {
                lines[i] = QLineF(xx, yy, xx, yy + 2);
                ++xx;
                ++yy;
            }
            yy -= 2;
            for (i = 3; i < 7; ++i) {
                lines[i] = QLineF(xx, yy, xx, yy + 2);
                ++xx;
                --yy;
            }
            p->drawLines(lines, 7);
        }
        if (doRestore)
            p->restore();
        break;
    case PE_FrameFocusRect:
        if (const QStyleOptionFocusRect *fropt = qstyleoption_cast<const QStyleOptionFocusRect *>(opt)) {
            if (!(fropt->state & State_KeyboardFocusChange))
                return;
            QRect r = opt->rect;
            p->save();
            p->setBackgroundMode(Qt::TransparentMode);
            QColor bg_col = fropt->backgroundColor;
            if (!bg_col.isValid())
                bg_col = p->background().color();
            // Create an "XOR" color.
            QColor patternCol((bg_col.red() ^ 0xff) & 0xff,
                              (bg_col.green() ^ 0xff) & 0xff,
                              (bg_col.blue() ^ 0xff) & 0xff);
            p->setBrush(QBrush(patternCol, Qt::Dense4Pattern));
            p->setBrushOrigin(r.topLeft());
            p->setPen(Qt::NoPen);
            p->drawRect(r.left(), r.top(), r.width(), 1);    // Top
            p->drawRect(r.left(), r.bottom(), r.width(), 1); // Bottom
            p->drawRect(r.left(), r.top(), 1, r.height());   // Left
            p->drawRect(r.right(), r.top(), 1, r.height());  // Right
            p->restore();
        }
        break;
    case PE_IndicatorRadioButton:
        {
#define PTSARRLEN(x) sizeof(x)/(sizeof(QPoint))
            static const QPoint pts1[] = {              // dark lines
                QPoint(1, 9), QPoint(1, 8), QPoint(0, 7), QPoint(0, 4), QPoint(1, 3), QPoint(1, 2),
                QPoint(2, 1), QPoint(3, 1), QPoint(4, 0), QPoint(7, 0), QPoint(8, 1), QPoint(9, 1)
            };
            static const QPoint pts2[] = {              // black lines
                QPoint(2, 8), QPoint(1, 7), QPoint(1, 4), QPoint(2, 3), QPoint(2, 2), QPoint(3, 2),
                QPoint(4, 1), QPoint(7, 1), QPoint(8, 2), QPoint(9, 2)
            };
            static const QPoint pts3[] = {              // background lines
                QPoint(2, 9), QPoint(3, 9), QPoint(4, 10), QPoint(7, 10), QPoint(8, 9), QPoint(9, 9),
                QPoint(9, 8), QPoint(10, 7), QPoint(10, 4), QPoint(9, 3)
            };
            static const QPoint pts4[] = {              // white lines
                QPoint(2, 10), QPoint(3, 10), QPoint(4, 11), QPoint(7, 11), QPoint(8, 10),
                QPoint(9, 10), QPoint(10, 9), QPoint(10, 8), QPoint(11, 7), QPoint(11, 4),
                QPoint(10, 3), QPoint(10, 2)
            };
            static const QPoint pts5[] = {              // inner fill
                QPoint(4, 2), QPoint(7, 2), QPoint(9, 4), QPoint(9, 7), QPoint(7, 9), QPoint(4, 9),
                QPoint(2, 7), QPoint(2, 4)
            };

            // make sure the indicator is square
            QRect ir = opt->rect;

            if (opt->rect.width() < opt->rect.height()) {
                ir.setTop(opt->rect.top() + (opt->rect.height() - opt->rect.width()) / 2);
                ir.setHeight(opt->rect.width());
            } else if (opt->rect.height() < opt->rect.width()) {
                ir.setLeft(opt->rect.left() + (opt->rect.width() - opt->rect.height()) / 2);
                ir.setWidth(opt->rect.height());
            }

            p->save();
            bool down = opt->state & State_Sunken;
            bool enabled = opt->state & State_Enabled;
            bool on = opt->state & State_On;
            QPolygon a;
            p->translate(ir.x(), ir.y());

            p->setPen(opt->palette.dark().color());
            p->drawPolyline(pts1, PTSARRLEN(pts1));

            p->setPen(opt->palette.shadow().color());
            p->drawPolyline(pts2, PTSARRLEN(pts2));

            p->setPen(opt->palette.midlight().color());
            p->drawPolyline(pts3, PTSARRLEN(pts3));

            p->setPen(opt->palette.light().color());
            p->drawPolyline(pts4, PTSARRLEN(pts4));

            QColor fillColor = (down || !enabled)
                               ? opt->palette.button().color()
                               : opt->palette.base().color();
            p->setPen(fillColor);
            p->setBrush(fillColor) ;
            p->drawPolygon(pts5, PTSARRLEN(pts5));

            p->translate(-ir.x(), -ir.y()); // restore translate

            if (on) {
                p->setPen(Qt::NoPen);
                p->setBrush(opt->palette.text());
                p->drawRect(ir.x() + 5, ir.y() + 4, 2, 4);
                p->drawRect(ir.x() + 4, ir.y() + 5, 4, 2);
            }
            p->restore();
            break;
        }
#ifndef QT_NO_FRAME
    case PE_Frame:
    case PE_FrameMenu:
        if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            if (frame->lineWidth == 2 || pe == PE_Frame) {
                QPalette popupPal = frame->palette;
                if (pe == PE_FrameMenu) {
                    popupPal.setColor(QPalette::Light, frame->palette.background().color());
                    popupPal.setColor(QPalette::Midlight, frame->palette.light().color());
                }
                if (use2000style && pe == PE_Frame && (frame->state & State_Raised))
                    qDrawWinButton(p, frame->rect, popupPal, frame->state & State_Sunken);
                else
                    qDrawWinPanel(p, frame->rect, popupPal, frame->state & State_Sunken);
            } else {
                QCommonStyle::drawPrimitive(pe, opt, p, w);
            }
        }
        break;
#endif // QT_NO_FRAME
    case PE_IndicatorBranch: {
        // This is _way_ too similar to the common style.
        static const int decoration_size = 9;
        int mid_h = opt->rect.x() + opt->rect.width() / 2;
        int mid_v = opt->rect.y() + opt->rect.height() / 2;
        int bef_h = mid_h;
        int bef_v = mid_v;
        int aft_h = mid_h;
        int aft_v = mid_v;
        if (opt->state & State_Children) {
            int delta = decoration_size / 2;
            bef_h -= delta;
            bef_v -= delta;
            aft_h += delta;
            aft_v += delta;
            p->drawLine(bef_h + 2, bef_v + 4, bef_h + 6, bef_v + 4);
            if (!(opt->state & State_Open))
                p->drawLine(bef_h + 4, bef_v + 2, bef_h + 4, bef_v + 6);
            QPen oldPen = p->pen();
            p->setPen(opt->palette.dark().color());
            p->drawRect(bef_h, bef_v, decoration_size - 1, decoration_size - 1);
            p->setPen(oldPen);
        }
        QBrush brush(opt->palette.dark().color(), Qt::Dense4Pattern);
        if (opt->state & State_Item) {
            if (opt->direction == Qt::RightToLeft)
                p->fillRect(opt->rect.left(), mid_v, bef_h - opt->rect.left(), 1, brush);
            else
                p->fillRect(aft_h, mid_v, opt->rect.right() - aft_h + 1, 1, brush);
        }
        if (opt->state & State_Sibling)
            p->fillRect(mid_h, aft_v, 1, opt->rect.bottom() - aft_v + 1, brush);
        if (opt->state & (State_Open | State_Children | State_Item | State_Sibling))
            p->fillRect(mid_h, opt->rect.y(), 1, bef_v - opt->rect.y(), brush);
        break; }
    case PE_FrameButtonBevel:
    case PE_PanelButtonBevel: {
        QBrush fill;
        bool panel = pe != PE_FrameButtonBevel;
        p->setBrushOrigin(opt->rect.topLeft());
        if (!(opt->state & State_Sunken) && (opt->state & State_On))
            fill = QBrush(opt->palette.light().color(), Qt::Dense4Pattern);
        else
            fill = opt->palette.brush(QPalette::Button);

        if (opt->state & (State_Raised | State_On | State_Sunken)) {
            qDrawWinButton(p, opt->rect, opt->palette, opt->state & (State_Sunken | State_On),
                           panel ? &fill : 0);
        } else {
            if (panel)
                p->fillRect(opt->rect, fill);
            else
                p->drawRect(opt->rect);
        }
        break; }
    case PE_FrameWindow: {
         QPalette popupPal = opt->palette;
         popupPal.setColor(QPalette::Light, opt->palette.background().color());
         popupPal.setColor(QPalette::Midlight, opt->palette.light().color());
         qDrawWinPanel(p, opt->rect, popupPal, opt->state & State_Sunken);
        break; }
#ifndef QT_NO_DOCKWIDGET
    case PE_IndicatorDockWidgetResizeHandle: {
        QPen oldPen = p->pen();
        p->setPen(opt->palette.light().color());
        if (opt->state & State_Horizontal) {
            p->drawLine(opt->rect.left(),          opt->rect.top(),
                        opt->rect.right(), opt->rect.top());
            p->setPen(opt->palette.dark().color());
            p->drawLine(opt->rect.left(),          opt->rect.bottom() - 1,
                        opt->rect.right(), opt->rect.bottom() - 1);
            p->setPen(opt->palette.shadow().color());
            p->drawLine(opt->rect.left(),          opt->rect.bottom(),
                        opt->rect.right(), opt->rect.bottom());
        } else {
            p->drawLine(opt->rect.left(), opt->rect.top(),
                        opt->rect.left(), opt->rect.bottom());
            p->setPen(opt->palette.dark().color());
            p->drawLine(opt->rect.right() - 1, opt->rect.top(),
                        opt->rect.right() - 1, opt->rect.bottom());
            p->setPen(opt->palette.shadow().color());
            p->drawLine(opt->rect.right(), opt->rect.top(),
                        opt->rect.right(), opt->rect.bottom());
        }
        p->setPen(oldPen);
        break; }
case PE_FrameDockWidget:
        if (qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            drawPrimitive(QStyle::PE_FrameWindow, opt, p, w);
        }
    break;
#endif // QT_NO_DOCKWIDGET

    case PE_FrameTabWidget:
        if (use2000style) {
            QRect rect = opt->rect;
            QPalette pal = opt->palette;
            qDrawWinButton(p, opt->rect, opt->palette, false, 0);
            break;
       }
    default:
        QCommonStyle::drawPrimitive(pe, opt, p, w);
    }
}

/*! \reimp */
void QWindowsStyle::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                                const QWidget *widget) const
{
    switch (ce) {
#ifndef QT_NO_RUBBERBAND
    case CE_RubberBand:
        if (qstyleoption_cast<const QStyleOptionRubberBand *>(opt)) {
            // ### workaround for slow general painter path
            QPixmap tiledPixmap(16, 16);
            QPainter pixmapPainter(&tiledPixmap);
            pixmapPainter.setPen(Qt::NoPen);
            pixmapPainter.setBrush(Qt::Dense4Pattern);
            pixmapPainter.setBackground(Qt::white);
            pixmapPainter.setBackgroundMode(Qt::OpaqueMode);
            pixmapPainter.drawRect(0, 0, tiledPixmap.width(), tiledPixmap.height());
            pixmapPainter.end();
            tiledPixmap = QPixmap::fromImage(tiledPixmap.toImage());
            p->save();
            QRect r = opt->rect;
            QStyleHintReturnMask mask;
            if (styleHint(QStyle::SH_RubberBand_Mask, opt, widget, &mask))
                p->setClipRegion(mask.region);
            p->drawTiledPixmap(r.x(), r.y(), r.width(), r.height(), tiledPixmap);
            p->restore();
            return;
        }
        break;
#endif // QT_NO_RUBBERBAND

#if !defined(QT_NO_MENU) && !defined(QT_NO_MAINWINDOW)
    case CE_MenuBarEmptyArea:
        if (widget && qobject_cast<const QMainWindow *>(widget->parentWidget())) {
            QPen oldPen = p->pen();
            p->setPen(QPen(opt->palette.dark().color()));
            p->drawLine(opt->rect.bottomLeft(), opt->rect.bottomRight());
        }
        break;
#endif
#ifndef QT_NO_MENU
    case CE_MenuItem:
        if (const QStyleOptionMenuItem *menuitem = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            int x, y, w, h;
            menuitem->rect.getRect(&x, &y, &w, &h);
            int tab = menuitem->tabWidth;
            bool dis = !(menuitem->state & State_Enabled);
            bool checked = menuitem->checkType != QStyleOptionMenuItem::NotCheckable
                            ? menuitem->checked : false;
            bool act = menuitem->state & State_Selected;

            // windows always has a check column, regardless whether we have an icon or not
            int checkcol = qMax(menuitem->maxIconWidth, use2000style ? 20 : windowsCheckMarkWidth);


            QBrush fill = menuitem->palette.brush(act ? QPalette::Highlight : QPalette::Button);
            p->fillRect(menuitem->rect, fill);

            if (menuitem->menuItemType == QStyleOptionMenuItem::Separator){
                int yoff = y-1 + h / 2;
                p->setPen(menuitem->palette.dark().color());
                p->drawLine(x + 2, yoff, x + w - 4, yoff);
                p->setPen(menuitem->palette.light().color());
                p->drawLine(x + 2, yoff + 1, x + w - 4, yoff + 1);
                return;
            }

            QRect vCheckRect = visualRect(opt->direction, menuitem->rect, QRect(menuitem->rect.x(), menuitem->rect.y(), checkcol, menuitem->rect.height()));
            if (checked) {
                if (act && !dis) {
                    qDrawShadePanel(p, vCheckRect,
                                    menuitem->palette, true, 1,
                                    &menuitem->palette.brush(QPalette::Button));
                } else {
                    QBrush fill(menuitem->palette.light().color(), Qt::Dense4Pattern);
                    qDrawShadePanel(p, vCheckRect, menuitem->palette, true, 1, &fill);
                }
            } else if (!act) {
                p->fillRect(vCheckRect, menuitem->palette.brush(QPalette::Button));
            }

            // On Windows Style, if we have a checkable item and an icon we
            // draw the icon recessed to indicate an item is checked. If we
            // have no icon, we draw a checkmark instead.
            if (!menuitem->icon.isNull()) {
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap;
                if (checked)
                    pixmap = menuitem->icon.pixmap(pixelMetric(PM_SmallIconSize), mode, QIcon::On);
                else
                    pixmap = menuitem->icon.pixmap(pixelMetric(PM_SmallIconSize), mode);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                if (act && !dis && !checked)
                    qDrawShadePanel(p, vCheckRect,  menuitem->palette, false, 1,
                                    &menuitem->palette.brush(QPalette::Button));
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(vCheckRect.center());
                p->setPen(menuitem->palette.text().color());
                p->drawPixmap(pmr.topLeft(), pixmap);
            } else if (checked) {
                QStyleOptionMenuItem newMi = *menuitem;
                newMi.state = State_None;
                if (!dis)
                    newMi.state |= State_Enabled;
                if (act)
                    newMi.state |= State_On;
                newMi.rect = visualRect(opt->direction, menuitem->rect, QRect(menuitem->rect.x() + windowsItemFrame, menuitem->rect.y() + windowsItemFrame,
                                                                              checkcol - 2 * windowsItemFrame, menuitem->rect.height() - 2*windowsItemFrame));
                drawPrimitive(PE_IndicatorMenuCheckMark, &newMi, p, widget);
            }
            p->setPen(act ? menuitem->palette.highlightedText().color() : menuitem->palette.buttonText().color());

            QColor discol;
            if (dis) {
                discol = menuitem->palette.text().color();
                p->setPen(discol);
            }

            int xm = windowsItemFrame + checkcol + windowsItemHMargin;
            int xpos = menuitem->rect.x() + xm;
            QRect textRect(xpos, y + windowsItemVMargin, w - xm - windowsRightBorder - tab + 1, h - 2 * windowsItemVMargin);
            QRect vTextRect = visualRect(opt->direction, menuitem->rect, textRect);
            QString s = menuitem->text;
            if (!s.isEmpty()) {                     // draw text
                p->save();
                int t = s.indexOf('\t');
                int text_flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (!styleHint(SH_UnderlineShortcut, menuitem, widget))
                    text_flags |= Qt::TextHideMnemonic;
                text_flags |= Qt::AlignLeft;
                if (t >= 0) {
                    QRect vShortcutRect = visualRect(opt->direction, menuitem->rect, QRect(textRect.topRight(), menuitem->rect.bottomRight()));
                    if (dis && !act) {
                        p->setPen(menuitem->palette.light().color());
                        p->drawText(vShortcutRect.adjusted(1,1,1,1), text_flags, s.mid(t + 1));
                        p->setPen(discol);
                    }
                    p->drawText(vShortcutRect, text_flags, s.mid(t + 1));
                    s = s.left(t);
                }
                QFont font = menuitem->font;
                if (menuitem->menuItemType == QStyleOptionMenuItem::DefaultItem)
                    font.setBold(true);
                p->setFont(font);
                if (dis && !act) {
                    p->setPen(menuitem->palette.light().color());
                    p->drawText(vTextRect.adjusted(1,1,1,1), text_flags, s.left(t));
                    p->setPen(discol);
                }
                p->drawText(vTextRect, text_flags, s.left(t));
                p->restore();
            }
            if (menuitem->menuItemType == QStyleOptionMenuItem::SubMenu) {// draw sub menu arrow
                int dim = (h - 2 * windowsItemFrame) / 2;
                PrimitiveElement arrow;
                arrow = (opt->direction == Qt::RightToLeft) ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
                xpos = x + w - windowsArrowHMargin - windowsItemFrame - dim;
                QRect  vSubMenuRect = visualRect(opt->direction, menuitem->rect, QRect(xpos, y + h / 2 - dim / 2, dim, dim));
                QStyleOptionMenuItem newMI = *menuitem;
                newMI.rect = vSubMenuRect;
                newMI.state = dis ? State_None : State_Enabled;
                if (act)
                    newMI.palette.setColor(QPalette::ButtonText,
                                           newMI.palette.highlightedText().color());
                drawPrimitive(arrow, &newMI, p, widget);
            }

        }
        break;
#endif // QT_NO_MENU
#ifndef QT_NO_MENUBAR
    case CE_MenuBarItem:
        if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            bool active = mbi->state & State_Selected;
            bool hasFocus = mbi->state & State_HasFocus;
            bool down = mbi->state & State_Sunken;
            QStyleOptionMenuItem newMbi = *mbi;
            p->fillRect(mbi->rect, mbi->palette.brush(QPalette::Button));
            if (active || hasFocus) {
                QBrush b = mbi->palette.brush(QPalette::Button);
                if (active && down)
                    p->setBrushOrigin(p->brushOrigin() + QPoint(1, 1));
                if (active && hasFocus)
                    qDrawShadeRect(p, mbi->rect.x(), mbi->rect.y(), mbi->rect.width(),
                                   mbi->rect.height(), mbi->palette, active && down, 1, 0, &b);
                if (active && down) {
                    newMbi.rect.translate(pixelMetric(PM_ButtonShiftHorizontal, mbi, widget),
                                       pixelMetric(PM_ButtonShiftVertical, mbi, widget));
                    p->setBrushOrigin(p->brushOrigin() - QPoint(1, 1));
                }
            }
            QCommonStyle::drawControl(ce, &newMbi, p, widget);
        }
        break;
#endif // QT_NO_MENUBAR
#ifndef QT_NO_TABBAR
    case CE_TabBarTabShape:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            bool rtlHorTabs = (tab->direction == Qt::RightToLeft
                               && (tab->shape == QTabBar::RoundedNorth
                                   || tab->shape == QTabBar::RoundedSouth));
            bool selected = tab->state & State_Selected;
            bool lastTab = ((!rtlHorTabs && tab->position == QStyleOptionTab::End)
                            || (rtlHorTabs
                                && tab->position == QStyleOptionTab::Beginning));
            bool firstTab = ((!rtlHorTabs
                               && tab->position == QStyleOptionTab::Beginning)
                             || (rtlHorTabs
                                 && tab->position == QStyleOptionTab::End));
            bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
            bool previousSelected =
                ((!rtlHorTabs
                  && tab->selectedPosition == QStyleOptionTab::PreviousIsSelected)
                || (rtlHorTabs
                    && tab->selectedPosition == QStyleOptionTab::NextIsSelected));
            bool nextSelected =
                ((!rtlHorTabs
                  && tab->selectedPosition == QStyleOptionTab::NextIsSelected)
                 || (rtlHorTabs
                     && tab->selectedPosition
                            == QStyleOptionTab::PreviousIsSelected));
            int tabBarAlignment = styleHint(SH_TabBar_Alignment, tab, widget);
            bool leftAligned = (!rtlHorTabs && tabBarAlignment == Qt::AlignLeft)
                                || (rtlHorTabs
                                    && tabBarAlignment == Qt::AlignRight);

            bool rightAligned = (!rtlHorTabs && tabBarAlignment == Qt::AlignRight)
                                 || (rtlHorTabs
                                         && tabBarAlignment == Qt::AlignLeft);

            QColor light = tab->palette.light().color();
            QColor midlight = tab->palette.midlight().color();
            QColor dark = tab->palette.dark().color();
            QColor shadow = tab->palette.shadow().color();
            QColor background = tab->palette.background().color();
            int borderThinkness = pixelMetric(PM_TabBarBaseOverlap, tab, widget);
            if (selected)
                borderThinkness /= 2;
            QRect r2(opt->rect);
            int x1 = r2.left();
            int x2 = r2.right();
            int y1 = r2.top();
            int y2 = r2.bottom();
            switch (tab->shape) {
            default:
                QCommonStyle::drawControl(ce, tab, p, widget);
                break;
            case QTabBar::RoundedNorth: {
                if (!selected) {
                    y1 += 2;
                    x1 += firstTab ? borderThinkness : 0;
                    x2 -= lastTab ? borderThinkness : 0;
                }
                // Delete border
                if (selected) {
                    p->setPen(background);
                    p->drawLine(x1, y2 - 1, x2, y2 - 1);
                    p->drawLine(x1, y2, x2, y2);
                }
                // Left
                if (firstTab || selected || onlyOne || !previousSelected) {
                    p->setPen(light);
                    p->drawLine(x1, y1 + 2, x1, y2 - ((onlyOne || firstTab) && selected && leftAligned ? 0 : borderThinkness));
                    p->drawPoint(x1 + 1, y1 + 1);
                    if (!use2000style) {
                        p->setPen(midlight);
                        p->drawLine(x1 + 1, y1 + 2, x1 + 1, y2 - ((onlyOne || firstTab) && selected && leftAligned ? 0 : borderThinkness));
                    }
                }
                // Top
                {
                    int beg = x1 + (previousSelected ? 0 : 2);
                    int end = x2 - (nextSelected ? 0 : 2);
                    p->setPen(light);
                    p->drawLine(beg, y1, end, y1);
                    if (!use2000style) {
                        p->setPen(midlight);
                        p->drawLine(beg, y1 + 1, end, y1 + 1);
                    }
                }
                // Right
                if (lastTab || selected || onlyOne || !nextSelected) {
                    p->setPen(shadow);
                    p->drawLine(x2, y1 + 2, x2, y2 - ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness));
                    p->drawPoint(x2 - 1, y1 + 1);
                    p->setPen(dark);
                    p->drawLine(x2 - 1, y1 + 2, x2 - 1, y2 - ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness));
                }
                break; }
            case QTabBar::RoundedSouth: {
                if (!selected) {
                    y2 -= 2;
                    x1 += firstTab ? borderThinkness : 0;
                    x2 -= lastTab ? borderThinkness : 0;
                }
                // Delete border
                if (selected) {
                    p->setPen(background);
                    p->drawLine(x1, y1 + 1, x2, y1 + 1);
                    p->drawLine(x1, y1, x2, y1);
                }
                // Left
                if (firstTab || selected || onlyOne || !previousSelected) {
                    p->setPen(light);
                    p->drawLine(x1, y2 - 2, x1, y1 + ((onlyOne || firstTab) && selected && leftAligned ? 0 : borderThinkness));
                    p->drawPoint(x1 + 1, y2 - 1);
                    if (!use2000style) {
                        p->setPen(midlight);
                        p->drawLine(x1 + 1, y2 - 2, x1 + 1, y1 + ((onlyOne || firstTab) && selected && leftAligned ? 0 : borderThinkness));
                    }
                }
                // Bottom
                {
                    int beg = x1 + (previousSelected ? 0 : 2);
                    int end = x2 - (nextSelected ? 0 : 2);
                    p->setPen(shadow);
                    p->drawLine(beg, y2, end, y2);
                    p->setPen(dark);
                    p->drawLine(beg, y2 - 1, end, y2 - 1);
                }
                // Right
                if (lastTab || selected || onlyOne || !nextSelected) {
                    p->setPen(shadow);
                    p->drawLine(x2, y2 - 2, x2, y1 + ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness));
                    p->drawPoint(x2 - 1, y2 - 1);
                    p->setPen(dark);
                    p->drawLine(x2 - 1, y2 - 2, x2 - 1, y1 + ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness));
                }
                break; }
            case QTabBar::RoundedWest: {
                if (!selected) {
                    x1 += 2;
                    y1 += firstTab ? borderThinkness : 0;
                    y2 -= lastTab ? borderThinkness : 0;
                }
                // Delete border
                if (selected) {
                    p->setPen(background);
                    p->drawLine(x2 - 1, y1, x2 - 1, y2);
                    p->drawLine(x2, y1, x2, y2);
                }
                // Top
                if (firstTab || selected || onlyOne || !previousSelected) {
                    p->setPen(light);
                    p->drawLine(x1 + 2, y1, x2 - ((onlyOne || firstTab) && selected && leftAligned ? 0 : borderThinkness), y1);
                    p->drawPoint(x1 + 1, y1 + 1);
                    if (!use2000style) {
                        p->setPen(midlight);
                        p->drawLine(x1 + 2, y1 + 1, x2 - ((onlyOne || firstTab) && selected && leftAligned ? 0 : borderThinkness), y1 + 1);
                    }
                }
                // Left
                {
                    int beg = y1 + (previousSelected ? 0 : 2);
                    int end = y2 - (nextSelected ? 0 : 2);
                    p->setPen(light);
                    p->drawLine(x1, beg, x1, end);
                    if (!use2000style) {
                        p->setPen(midlight);
                        p->drawLine(x1 + 1, beg, x1 + 1, end);
                    }
                }
                // Bottom
                if (lastTab || selected || onlyOne || !nextSelected) {
                    p->setPen(shadow);
                    p->drawLine(x1 + 3, y2, x2 - ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness), y2);
                    p->drawPoint(x1 + 2, y2 - 1);
                    p->setPen(dark);
                    p->drawLine(x1 + 3, y2 - 1, x2 - ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness), y2 - 1);
                    p->drawPoint(x1 + 1, y2 - 1);
                    p->drawPoint(x1 + 2, y2);
                }
                break; }
            case QTabBar::RoundedEast: {
                if (!selected) {
                    x2 -= 2;
                    y1 += firstTab ? borderThinkness : 0;
                    y2 -= lastTab ? borderThinkness : 0;
                }
                // Delete border
                if (selected) {
                    p->setPen(background);
                    p->drawLine(x1 + 1, y1, x1 + 1, y2);
                    p->drawLine(x1, y1, x1, y2);
                }
                // Top
                if (firstTab || selected || onlyOne || !previousSelected) {
                    p->setPen(light);
                    p->drawLine(x2 - 2, y1, x1 + ((onlyOne || firstTab) && selected && leftAligned ? 0 : borderThinkness), y1);
                    p->drawPoint(x2 - 1, y1 + 1);
                    if (!use2000style) {
                        p->setPen(midlight);
                        p->drawLine(x2 - 3, y1 + 1, x1 + ((onlyOne || firstTab) && selected && leftAligned ? 0 : borderThinkness), y1 + 1);
                        p->drawPoint(x2 - 1, y1);
                    }
                }
                // Right
                {
                    int beg = y1 + (previousSelected ? 0 : 2);
                    int end = y2 - (nextSelected ? 0 : 2);
                    p->setPen(shadow);
                    p->drawLine(x2, beg, x2, end);
                    p->setPen(dark);
                    p->drawLine(x2 - 1, beg, x2 - 1, end);
                }
                // Bottom
                if (lastTab || selected || onlyOne || !nextSelected) {
                    p->setPen(shadow);
                    p->drawLine(x2 - 2, y2, x1 + ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness), y2);
                    p->drawPoint(x2 - 1, y2 - 1);
                    p->setPen(dark);
                    p->drawLine(x2 - 2, y2 - 1, x1 + ((onlyOne || lastTab) && selected && rightAligned ? 0 : borderThinkness), y2 - 1);
                }
                break; }
            }
        }
        break;
#endif // QT_NO_TABBAR
    case CE_ToolBoxTab:
        qDrawShadePanel(p, opt->rect, opt->palette,
                        opt->state & (State_Sunken | State_On), 1,
                        &opt->palette.brush(QPalette::Button));
        break;
#ifndef QT_NO_SPLITTER
case CE_Splitter: {
        QPen oldPen = p->pen();
        p->setPen(opt->palette.light().color());
        if (opt->state & State_Horizontal) {
            p->drawLine(opt->rect.x() + 1, opt->rect.y(), opt->rect.x() + 1, opt->rect.height());
            p->setPen(opt->palette.dark().color());
            p->drawLine(opt->rect.x(), opt->rect.y(), opt->rect.x(), opt->rect.height());
            p->drawLine(opt->rect.right() - 1, opt->rect.y(), opt->rect.right() - 1,
                        opt->rect.height());
            p->setPen(opt->palette.shadow().color());
            p->drawLine(opt->rect.right(), opt->rect.y(), opt->rect.right(), opt->rect.height());
        } else {
            p->drawLine(opt->rect.x(), opt->rect.y() + 1, opt->rect.width(), opt->rect.y() + 1);
            p->setPen(opt->palette.dark().color());
            p->drawLine(opt->rect.x(), opt->rect.bottom() - 1, opt->rect.width(),
                        opt->rect.bottom() - 1);
            p->setPen(opt->palette.shadow().color());
            p->drawLine(opt->rect.x(), opt->rect.bottom(), opt->rect.width(), opt->rect.bottom());
        }
        p->setPen(oldPen);
        break; }
#endif // QT_NO_SPLITTER
#ifndef QT_NO_SCROLLBAR
    case CE_ScrollBarSubLine:
    case CE_ScrollBarAddLine: {
        if (use2000style && (opt->state & State_Sunken)) {
            p->setPen(opt->palette.dark().color());
            p->setBrush(opt->palette.brush(QPalette::Button));
            p->drawRect(opt->rect.adjusted(0, 0, -1, -1));
        } else {
            QStyleOption buttonOpt = *opt;
            if (!(buttonOpt.state & State_Sunken))
                buttonOpt.state |= State_Raised;
            drawPrimitive(PE_PanelButtonBevel, &buttonOpt, p, widget);
        }
        PrimitiveElement arrow;
        if (opt->state & State_Horizontal) {
            if (ce == CE_ScrollBarAddLine)
                arrow = opt->direction == Qt::LeftToRight ? PE_IndicatorArrowRight : PE_IndicatorArrowLeft;
            else
                arrow = opt->direction == Qt::LeftToRight ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
        } else {
            if (ce == CE_ScrollBarAddLine)
                arrow = PE_IndicatorArrowDown;
            else
                arrow = PE_IndicatorArrowUp;
        }
        drawPrimitive(arrow, opt, p, widget);
        break; }
    case CE_ScrollBarAddPage:
    case CE_ScrollBarSubPage: {
            QBrush br;
            QBrush bg = p->background();
            Qt::BGMode bg_mode = p->backgroundMode();
            p->setPen(Qt::NoPen);
            p->setBackgroundMode(Qt::OpaqueMode);

            if (opt->state & State_Sunken) {
                br = QBrush(opt->palette.shadow().color(), Qt::Dense4Pattern);
                p->setBackground(opt->palette.dark().color());
                p->setBrush(br);
            } else {
                QPixmap pm = opt->palette.brush(QPalette::Light).texture();
                br = !pm.isNull() ? QBrush(pm) : QBrush(opt->palette.light().color(), Qt::Dense4Pattern);
                p->setBackground(opt->palette.background().color());
                p->setBrush(br);
            }
            p->drawRect(opt->rect);
            p->setBackground(bg);
            p->setBackgroundMode(bg_mode);
            break; }
    case CE_ScrollBarSlider:
        if (!(opt->state & State_Enabled)) {
            QPixmap pm = opt->palette.brush(QPalette::Light).texture();
            QBrush br = !pm.isNull() ? QBrush(pm) : QBrush(opt->palette.light().color(), Qt::Dense4Pattern);
            p->setPen(Qt::NoPen);
            p->setBrush(br);
            p->setBackgroundMode(Qt::OpaqueMode);
            p->drawRect(opt->rect);
        } else {
            QStyleOptionButton buttonOpt;
            buttonOpt.QStyleOption::operator=(*opt);
            buttonOpt.state = State_Enabled | State_Raised;
            drawPrimitive(PE_PanelButtonBevel, &buttonOpt, p, widget);
        }
        break;
#endif // QT_NO_SCROLLBAR
    case CE_HeaderSection: {
        QBrush fill;
        if (opt->state & State_On)
            fill = QBrush(opt->palette.light().color(), Qt::Dense4Pattern);
        else
            fill = opt->palette.brush(QPalette::Button);

        if (opt->state & (State_Raised | State_Sunken)) {
            qDrawWinButton(p, opt->rect, opt->palette, opt->state & State_Sunken, &fill);
        } else {
            p->fillRect(opt->rect, fill);
        }
        break; }
#ifndef QT_NO_TOOLBAR
    case CE_ToolBar:
        if (const QStyleOptionToolBar *toolbar = qstyleoption_cast<const QStyleOptionToolBar *>(opt)) {
            QRect rect = opt->rect;

            bool paintLeftBorder = true;
            bool paintRightBorder = true;
            bool paintBottomBorder = true;

            switch (toolbar->toolBarArea){
            case Qt::BottomToolBarArea :
                switch(toolbar->positionOfLine){
                case QStyleOptionToolBar::Beginning:
                case QStyleOptionToolBar::OnlyOne:
                    paintBottomBorder = false;
                default:
                    break;
                }
            case Qt::TopToolBarArea :
                switch(toolbar->positionWithinLine){
                case QStyleOptionToolBar::Beginning:
                    paintLeftBorder = false;
                    break;
                case QStyleOptionToolBar::End:
                    paintRightBorder = false;
                    break;
                case QStyleOptionToolBar::OnlyOne:
                    paintRightBorder = false;
                    paintLeftBorder = false;
                default:
                    break;
                }
                if(QApplication::layoutDirection() == Qt::RightToLeft){ //reverse layout changes the order of Beginning/end
                    bool tmp = paintLeftBorder;
                    paintRightBorder=paintLeftBorder;
                    paintLeftBorder=tmp;
                }
                break;
            case Qt::RightToolBarArea :
                switch (toolbar->positionOfLine){
                case QStyleOptionToolBar::Beginning:
                case QStyleOptionToolBar::OnlyOne:
                    paintRightBorder = false;
                    break;
                default:
                    break;
                }
                break;
            case Qt::LeftToolBarArea :
                switch (toolbar->positionOfLine){
                case QStyleOptionToolBar::Beginning:
                case QStyleOptionToolBar::OnlyOne:
                    paintLeftBorder = false;
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }


            //draw top border
            p->setPen(QPen(opt->palette.light().color()));
            p->drawLine(rect.topLeft().x(),
                        rect.topLeft().y(),
                        rect.topRight().x(),
                        rect.topRight().y());

            if (paintLeftBorder){
                p->setPen(QPen(opt->palette.light().color()));
                p->drawLine(rect.topLeft().x(),
                            rect.topLeft().y(),
                            rect.bottomLeft().x(),
                            rect.bottomLeft().y());
            }

            if (paintRightBorder){
                p->setPen(QPen(opt->palette.dark().color()));
                p->drawLine(rect.topRight().x(),
                            rect.topRight().y(),
                            rect.bottomRight().x(),
                            rect.bottomRight().y());
            }

            if (paintBottomBorder){
                p->setPen(QPen(opt->palette.dark().color()));
                p->drawLine(rect.bottomLeft().x(),
                            rect.bottomLeft().y(),
                            rect.bottomRight().x(),
                            rect.bottomRight().y());
            }
        }
        break;


#endif // QT_NO_TOOLBAR
#ifndef QT_NO_PROGRESSBAR
    case CE_ProgressBarContents:
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {

            QRect rect = pb->rect;
            bool vertical = false;
            bool inverted = false;

            // Get extra style options if version 2
            const QStyleOptionProgressBarV2 *pb2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(opt);
            if (pb2) {
                vertical = (pb2->orientation == Qt::Vertical);
                inverted = pb2->invertedAppearance;
            }
            QMatrix m;
            if (vertical) {
                rect = QRect(rect.left(), rect.top(), rect.height(), rect.width()); // flip width and height
                m.translate(rect.height(), 0.0);
                m.rotate(90);
            }
            QPalette pal2 = pb->palette;
            // Correct the highlight color if it is the same as the background
            if (pal2.highlight() == pal2.background())
                pal2.setColor(QPalette::Highlight, pb->palette.color(QPalette::Active,
                                                                     QPalette::Highlight));
            bool reverse = ((!vertical && (pb->direction == Qt::RightToLeft)) || vertical);
            if (inverted)
                reverse = !reverse;
            int fw = 2;
            int w = rect.width() - 2 * fw;
            if (pb->minimum == 0 && pb->maximum == 0) {
                Q_D(const QWindowsStyle);
                const int unit_width = pixelMetric(PM_ProgressBarChunkWidth, pb, widget);
                QStyleOptionProgressBarV2 pbBits = *pb;
                Q_ASSERT(unit_width >0);

                pbBits.rect = rect;
                pbBits.palette = pal2;

                int chunkCount = w / unit_width + 1;
                int step = d->animateStep%chunkCount;
                int margin = 3;
                int chunksInRow = 5;
                int myY = pbBits.rect.y();
                int myHeight = pbBits.rect.height();
                int chunksToDraw = chunksInRow;

                if(step > chunkCount - 5)chunksToDraw = (chunkCount - step);
                QRegion prevClip = p->clipRegion(); //save state
                QRect clip = rect;
                clip.setLeft(clip.left() + margin);
                clip.setRight(clip.right() - margin);
                QRegion intersection = prevClip.intersect(clip);

                int x0 = reverse ? rect.right() - unit_width*(step) - unit_width  : margin + unit_width * step;
                int x = 0;

                //Make sure the cliprect is also rotated if vertical
                if(vertical)clip = m.mapRect(clip);

                if(!prevClip.isEmpty())p->setClipRegion(intersection);
                else p->setClipRect(clip);

                for (int i = 0; i < chunksToDraw ; ++i) {
                    pbBits.rect.setRect(x0 + x, myY, unit_width, myHeight);
                    pbBits.rect = m.mapRect(pbBits.rect);
                    drawPrimitive(PE_IndicatorProgressChunk, &pbBits, p, widget);
                    x += reverse ? -unit_width : unit_width;
                }
                //Draw wrap-around chunks
                if( step > chunkCount-5){
                    x0 = reverse ? rect.right() - unit_width : margin ;
                    x = 0;
                    int chunksToDraw = step - (chunkCount - chunksInRow);
                    for (int i = 0; i < chunksToDraw ; ++i) {
                        pbBits.rect.setRect(x0 + x, myY, unit_width, myHeight);
                        pbBits.rect = m.mapRect(pbBits.rect);
                        drawPrimitive(PE_IndicatorProgressChunk, &pbBits, p, widget);
                        x += reverse ? -unit_width : unit_width;
                    }
                }
                p->setClipRegion(prevClip); //restore state
            }
            else {
                QCommonStyle::drawControl(ce, opt, p, widget);
            }
        }
        break;
#endif // QT_NO_PROGRESSBAR

#ifndef QT_NO_DOCKWIDGET
    case CE_DockWidgetTitle:

        if (const QStyleOptionDockWidget *dwOpt = qstyleoption_cast<const QStyleOptionDockWidget *>(opt)) {
            Q_D(const QWindowsStyle);
            QRect r = dwOpt->rect;
            bool floating = false;
            int menuOffset = 0; //used to center text when floated
            QColor inactiveCaptionTextColor = d->inactiveCaptionText;
            if (dwOpt->movable) {
                const QDockWidget *dockWidget = qobject_cast<const QDockWidget *>(widget);
                QColor left, right;

                //Titlebar gradient
                if (dockWidget && dockWidget->isFloating()) {
                    floating = true;
                    if (widget && widget->isActiveWindow()) {
                        left = d->activeCaptionColor;
                        right = d->activeGradientCaptionColor;
                    } else {
                        left = d->inactiveCaptionColor;
                        right = d->inactiveGradientCaptionColor;
                    }
                    menuOffset = 2;
                    QBrush fillBrush(left);
                    if (left != right) {
                        QPoint p1(dwOpt->rect.x(), dwOpt->rect.top() + dwOpt->rect.height()/2);
                        QPoint p2(dwOpt->rect.right(), dwOpt->rect.top() + dwOpt->rect.height()/2);
                        QLinearGradient lg(p1, p2);
                        lg.setColorAt(0, left);
                        lg.setColorAt(1, right);
                        fillBrush = lg;
                    }
                    p->fillRect(opt->rect.adjusted(0, 0, 0, -3), fillBrush);
                }
                p->setPen(dwOpt->palette.color(QPalette::Light));
                if (!dockWidget || !dockWidget->isFloating()) {
                    p->drawLine(r.topLeft(), r.topRight());
                    p->setPen(dwOpt->palette.color(QPalette::Dark));
                    p->drawLine(r.bottomLeft(), r.bottomRight());            }
            }
            if (!dwOpt->title.isEmpty()) {
                QFont oldFont = p->font();
                if (floating) {
                    QFont font = oldFont;
                    font.setBold(true);
                    p->setFont(font);
                }
                QPalette palette = dwOpt->palette;
                palette.setColor(QPalette::Background, inactiveCaptionTextColor);
                bool active = dwOpt->state & State_Active;
                const int indent = p->fontMetrics().descent();
                drawItemText(p, r.adjusted(indent + 1, - menuOffset, -indent - 1, -1),
                            Qt::AlignLeft | Qt::AlignVCenter, palette,
                            dwOpt->state & State_Enabled, dwOpt->title,
                            floating ? (active ? QPalette::BrightText : QPalette::Background) : QPalette::Foreground);
                p->setFont(oldFont);
            }
        }
        return;
#endif // QT_NO_DOCKWIDGET
    default:
        QCommonStyle::drawControl(ce, opt, p, widget);
    }
}

/*! \reimp */
QRect QWindowsStyle::subElementRect(SubElement sr, const QStyleOption *opt, const QWidget *w) const
{
    QRect r;
    switch (sr) {
    case SE_SliderFocusRect:
    case SE_ToolBoxTabContents:
        r = visualRect(opt->direction, opt->rect, opt->rect);
        break;
    default:
        r = QCommonStyle::subElementRect(sr, opt, w);
    }
    return r;
}

#ifdef QT3_SUPPORT
Q_GLOBAL_STATIC_WITH_ARGS(QBitmap, globalVerticalLine, (1, 129))
Q_GLOBAL_STATIC_WITH_ARGS(QBitmap, globalHorizontalLine, (128, 1))
#endif

/*! \reimp */
void QWindowsStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                       QPainter *p, const QWidget *widget) const
{
    switch (cc) {
#ifndef QT_NO_SLIDER
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            int thickness  = pixelMetric(PM_SliderControlThickness, slider, widget);
            int len        = pixelMetric(PM_SliderLength, slider, widget);
            int ticks = slider->tickPosition;
            QRect groove = QCommonStyle::subControlRect(CC_Slider, slider,
                                                                SC_SliderGroove, widget);
            QRect handle = QCommonStyle::subControlRect(CC_Slider, slider,
                                                                SC_SliderHandle, widget);

            if ((slider->subControls & SC_SliderGroove) && groove.isValid()) {
                int mid = thickness / 2;

                if (ticks & QSlider::TicksAbove)
                    mid += len / 8;
                if (ticks & QSlider::TicksBelow)
                    mid -= len / 8;

                p->setPen(slider->palette.shadow().color());
                if (slider->orientation == Qt::Horizontal) {
                    qDrawWinPanel(p, groove.x(), groove.y() + mid - 2,
                                   groove.width(), 4, slider->palette, true);
                    p->drawLine(groove.x() + 1, groove.y() + mid - 1,
                                groove.x() + groove.width() - 3, groove.y() + mid - 1);
                } else {
                    qDrawWinPanel(p, groove.x() + mid - 2, groove.y(),
                                  4, groove.height(), slider->palette, true);
                    p->drawLine(groove.x() + mid - 1, groove.y() + 1,
                                groove.x() + mid - 1, groove.y() + groove.height() - 3);
                }
            }

            if (slider->subControls & SC_SliderTickmarks) {
                QStyleOptionSlider tmpSlider = *slider;
                tmpSlider.subControls = SC_SliderTickmarks;
                QCommonStyle::drawComplexControl(cc, &tmpSlider, p, widget);
            }

            if (slider->subControls & SC_SliderHandle) {
                // 4444440
                // 4333310
                // 4322210
                // 4322210
                // 4322210
                // 4322210
                // *43210*
                // **410**
                // ***0***
                const QColor c0 = slider->palette.shadow().color();
                const QColor c1 = slider->palette.dark().color();
                // const QColor c2 = g.button();
                const QColor c3 = slider->palette.midlight().color();
                const QColor c4 = slider->palette.light().color();
                QBrush handleBrush;

                if (slider->state & State_Enabled) {
                    handleBrush = slider->palette.color(QPalette::Button);
                } else {
                    handleBrush = QBrush(slider->palette.color(QPalette::Button),
                                         Qt::Dense4Pattern);
                }


                int x = handle.x(), y = handle.y(),
                   wi = handle.width(), he = handle.height();

                int x1 = x;
                int x2 = x+wi-1;
                int y1 = y;
                int y2 = y+he-1;

                Qt::Orientation orient = slider->orientation;
                bool tickAbove = slider->tickPosition == QSlider::TicksAbove;
                bool tickBelow = slider->tickPosition == QSlider::TicksBelow;

                if (slider->state & State_HasFocus) {
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*slider);
                    fropt.rect = subElementRect(SE_SliderFocusRect, slider, widget);
                    drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
                }

                if ((tickAbove && tickBelow) || (!tickAbove && !tickBelow)) {
                    Qt::BGMode oldMode = p->backgroundMode();
                    p->setBackgroundMode(Qt::OpaqueMode);
                    qDrawWinButton(p, QRect(x, y, wi, he), slider->palette, false,
                                   &handleBrush);
                    p->setBackgroundMode(oldMode);
                    return;
                }

                QSliderDirection dir;

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

                QBrush oldBrush = p->brush();
                p->setPen(Qt::NoPen);
                p->setBrush(handleBrush);
                Qt::BGMode oldMode = p->backgroundMode();
                p->setBackgroundMode(Qt::OpaqueMode);
                p->drawRect(x1, y1, x2-x1+1, y2-y1+1);
                p->drawPolygon(a);
                p->setBrush(oldBrush);
                p->setBackgroundMode(oldMode);

                if (dir != SlUp) {
                    p->setPen(c4);
                    p->drawLine(x1, y1, x2, y1);
                    p->setPen(c3);
                    p->drawLine(x1, y1+1, x2, y1+1);
                }
                if (dir != SlLeft) {
                    p->setPen(c3);
                    p->drawLine(x1+1, y1+1, x1+1, y2);
                    p->setPen(c4);
                    p->drawLine(x1, y1, x1, y2);
                }
                if (dir != SlRight) {
                    p->setPen(c0);
                    p->drawLine(x2, y1, x2, y2);
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
                    p->setPen(c4);
                    p->drawLine(x1, y1, x1+d, y1-d);
                    p->setPen(c0);
                    d = wi - d - 1;
                    p->drawLine(x2, y1, x2-d, y1-d);
                    d--;
                    p->setPen(c3);
                    p->drawLine(x1+1, y1, x1+1+d, y1-d);
                    p->setPen(c1);
                    p->drawLine(x2-1, y1, x2-1-d, y1-d);
                    break;
                case SlDown:
                    p->setPen(c4);
                    p->drawLine(x1, y2, x1+d, y2+d);
                    p->setPen(c0);
                    d = wi - d - 1;
                    p->drawLine(x2, y2, x2-d, y2+d);
                    d--;
                    p->setPen(c3);
                    p->drawLine(x1+1, y2, x1+1+d, y2+d);
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
                    p->drawLine(x2, y1+1, x2+d, y1+1+d);
                    p->setPen(c1);
                    p->drawLine(x2, y2-1, x2+d, y2-1-d);
                    break;
                }
            }
        }
        break;
#endif // QT_NO_SLIDER
#ifdef QT3_SUPPORT
    case CC_Q3ListView:
        if (const QStyleOptionQ3ListView *lv = qstyleoption_cast<const QStyleOptionQ3ListView *>(opt)) {
            int i;
            if (lv->subControls & SC_Q3ListView)
                QCommonStyle::drawComplexControl(cc, lv, p, widget);
            if (lv->subControls & (SC_Q3ListViewBranch | SC_Q3ListViewExpand)) {
                if (lv->items.isEmpty())
                    break;
                QStyleOptionQ3ListViewItem item = lv->items.at(0);
                int y = lv->rect.y();
                int c;
                int dotoffset = 0;
                QPolygon dotlines;
                if ((lv->activeSubControls & SC_All) && (lv->subControls & SC_Q3ListViewExpand)) {
                    c = 2;
                    dotlines.resize(2);
                    dotlines[0] = QPoint(lv->rect.right(), lv->rect.top());
                    dotlines[1] = QPoint(lv->rect.right(), lv->rect.bottom());
                } else {
                    int linetop = 0, linebot = 0;
                    // each branch needs at most two lines, ie. four end points
                    dotoffset = (item.itemY + item.height - y) % 2;
                    dotlines.resize(item.childCount * 4);
                    c = 0;

                    // skip the stuff above the exposed rectangle
                    for (i = 1; i < lv->items.size(); ++i) {
                        QStyleOptionQ3ListViewItem child = lv->items.at(i);
                        if (child.height + y > 0)
                            break;
                        y += child.totalHeight;
                    }
                    int bx = lv->rect.width() / 2;

                    // paint stuff in the magical area
                    while (i < lv->items.size() && y < lv->rect.height()) {
                        QStyleOptionQ3ListViewItem child = lv->items.at(i);
                        if (child.features & QStyleOptionQ3ListViewItem::Visible) {
                            int lh;
                            if (!(item.features & QStyleOptionQ3ListViewItem::MultiLine))
                                lh = child.height;
                            else
                                lh = p->fontMetrics().height() + 2 * lv->itemMargin;
                            lh = qMax(lh, QApplication::globalStrut().height());
                            if (lh % 2 > 0)
                                ++lh;
                            linebot = y + lh / 2;
                            if (child.features & QStyleOptionQ3ListViewItem::Expandable
                                || child.childCount > 0 && child.height > 0) {
                                // needs a box
                                p->setPen(lv->palette.mid().color());
                                p->drawRect(bx - 4, linebot - 4, 8, 8);
                                // plus or minus
                                p->setPen(lv->palette.text().color());
                                p->drawLine(bx - 2, linebot, bx + 2, linebot);
                                if (!(child.state & State_Open))
                                    p->drawLine(bx, linebot - 2, bx, linebot + 2);
                                // dotlinery
                                p->setPen(lv->palette.mid().color());
                                dotlines[c++] = QPoint(bx, linetop);
                                dotlines[c++] = QPoint(bx, linebot - 4);
                                dotlines[c++] = QPoint(bx + 5, linebot);
                                dotlines[c++] = QPoint(lv->rect.width(), linebot);
                                linetop = linebot + 5;
                            } else {
                                // just dotlinery
                                dotlines[c++] = QPoint(bx+1, linebot -1);
                                dotlines[c++] = QPoint(lv->rect.width(), linebot -1);
                            }
                            y += child.totalHeight;
                        }
                        ++i;
                    }

                    // Expand line height to edge of rectangle if there's any
                    // visible child below
                    while (i < lv->items.size() && lv->items.at(i).height <= 0)
                        ++i;
                    if (i < lv->items.size())
                        linebot = lv->rect.height();

                    if (linetop < linebot) {
                        dotlines[c++] = QPoint(bx, linetop);
                        dotlines[c++] = QPoint(bx, linebot);
                    }
                }
                p->setPen(lv->palette.text().color());
                QBitmap *verticalLine = globalVerticalLine();
                QBitmap *horizontalLine = globalHorizontalLine();
                static bool isInit = false;
                if (!isInit) {
                    isInit = true;
                    // make 128*1 and 1*128 bitmaps that can be used for
                    // drawing the right sort of lines.
                    verticalLine->clear();
                    horizontalLine->clear();
                    QPolygon a(64);
                    QPainter p;
                    p.begin(verticalLine);
                    for(i = 0; i < 64; ++i)
                        a.setPoint(i, 0, i * 2 + 1);
                    p.setPen(Qt::color1);
                    p.drawPoints(a);
                    p.end();
                    QApplication::flush();
                    verticalLine->setMask(*verticalLine);
                    p.begin(horizontalLine);
                    for(i = 0; i < 64; ++i)
                        a.setPoint(i, i * 2 + 1, 0);
                    p.setPen(Qt::color1);
                    p.drawPoints(a);
                    p.end();
                    QApplication::flush();
                    horizontalLine->setMask(*horizontalLine);
                }

                int line; // index into dotlines
                if (lv->subControls & SC_Q3ListViewBranch) for(line = 0; line < c; line += 2) {
                    // assumptions here: lines are horizontal or vertical.
                    // lines always start with the numerically lowest
                    // coordinate.

                    // point ... relevant coordinate of current point
                    // end ..... same coordinate of the end of the current line
                    // other ... the other coordinate of the current point/line
                    if (dotlines[line].y() == dotlines[line+1].y()) {
                        int end = dotlines[line + 1].x();
                        int point = dotlines[line].x();
                        int other = dotlines[line].y();
                        while (point < end) {
                            int i = 128;
                            if (i + point > end)
                                i = end-point;
                            p->drawPixmap(point, other, *horizontalLine, 0, 0, i, 1);
                            point += i;
                        }
                    } else {
                        int end = dotlines[line + 1].y();
                        int point = dotlines[line].y();
                        int other = dotlines[line].x();
                        int pixmapoffset = ((point & 1) != dotoffset) ? 1 : 0;
                        while(point < end) {
                            int i = 128;
                            if (i + point > end)
                                i = end-point;
                            p->drawPixmap(other, point, *verticalLine, 0, pixmapoffset, 1, i);
                            point += i;
                        }
                    }
                }
            }
        }
        break;
#endif // QT3_SUPPORT
#ifndef QT_NO_COMBOBOX
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            QBrush editBrush = (cmb->state & State_Enabled) ? cmb->palette.brush(QPalette::Base)
                                : cmb->palette.brush(QPalette::Background);
            if ((cmb->subControls & SC_ComboBoxFrame) && cmb->frame)
                qDrawWinPanel(p, opt->rect, opt->palette, true, &editBrush);
            else
                p->fillRect(opt->rect, editBrush);

            if (cmb->subControls & SC_ComboBoxArrow) {
                State flags = State_None;

                QRect ar = subControlRect(CC_ComboBox, cmb, SC_ComboBoxArrow, widget);
                if (cmb->activeSubControls == SC_ComboBoxArrow) {
                    p->setPen(cmb->palette.dark().color());
                    p->setBrush(cmb->palette.brush(QPalette::Button));
                    p->drawRect(ar.adjusted(0,0,-1,-1));
                } else {
                    // Make qDrawWinButton use the right colors for drawing the shade of the button
                    QPalette pal(cmb->palette);
                    pal.setColor(QPalette::Button, cmb->palette.light().color());
                    pal.setColor(QPalette::Light, cmb->palette.button().color());
                    qDrawWinButton(p, ar, pal, false,
                                   &cmb->palette.brush(QPalette::Button));
                }

                ar.adjust(2, 2, -2, -2);
                if (opt->state & State_Enabled)
                    flags |= State_Enabled;

                if (cmb->activeSubControls == SC_ComboBoxArrow)
                    flags |= State_Sunken;
                QStyleOption arrowOpt(0);
                arrowOpt.rect = ar;
                arrowOpt.palette = cmb->palette;
                arrowOpt.state = flags;
                drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, p, widget);
            }

            if (cmb->subControls & SC_ComboBoxEditField) {
                QRect re = subControlRect(CC_ComboBox, cmb, SC_ComboBoxEditField, widget);
                if (cmb->state & State_HasFocus && !cmb->editable)
                    p->fillRect(re.x(), re.y(), re.width(), re.height(),
                                cmb->palette.brush(QPalette::Highlight));

                if (cmb->state & State_HasFocus) {
                    p->setPen(cmb->palette.highlightedText().color());
                    p->setBackground(cmb->palette.highlight());

                } else {
                    p->setPen(cmb->palette.text().color());
                    p->setBackground(cmb->palette.background());
                }

                if (cmb->state & State_HasFocus && !cmb->editable) {
                    QStyleOptionFocusRect focus;
                    focus.QStyleOption::operator=(*cmb);
                    focus.rect = subElementRect(SE_ComboBoxFocusRect, cmb, widget);
                    focus.state |= State_FocusAtBorder;
                    focus.backgroundColor = cmb->palette.highlight().color();
                    drawPrimitive(PE_FrameFocusRect, &focus, p, widget);
                }
            }
        }
        break;
#endif // QT_NO_COMBOBOX
    default:
        QCommonStyle::drawComplexControl(cc, opt, p, widget);
    }
}

/*! \reimp */
QSize QWindowsStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt,
                                      const QSize &csz, const QWidget *widget) const
{
    QSize sz(csz);
    switch (ct) {
    case CT_PushButton:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            sz = QCommonStyle::sizeFromContents(ct, opt, csz, widget);
            int w = sz.width(),
                h = sz.height();
            int defwidth = 0;
            if (btn->features & QStyleOptionButton::AutoDefaultButton)
                defwidth = 2 * pixelMetric(PM_ButtonDefaultIndicator, btn, widget);
            if (w < 75 + defwidth && btn->icon.isNull())
                w = 75 + defwidth;
            if (h < 23 + defwidth)
                h = 23 + defwidth;
            sz = QSize(w, h);
        }
        break;
#ifndef QT_NO_MENU
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            int w = sz.width();
            sz = QCommonStyle::sizeFromContents(ct, opt, csz, widget);

            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                sz = QSize(10, windowsSepHeight);
            }
            else if (mi->icon.isNull()) {
                sz.setHeight(sz.height() - 2);
                w -= 6;
            }

            if (mi->menuItemType != QStyleOptionMenuItem::Separator && !mi->icon.isNull())
                 sz.setHeight(qMax(sz.height(),
                              mi->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal).height()
                              + 2 * windowsItemFrame));
            int maxpmw = mi->maxIconWidth;
            int tabSpacing = use2000style ? 20 :windowsTabSpacing;
            if (mi->text.contains('\t'))
                w += tabSpacing;
            else if (mi->menuItemType == QStyleOptionMenuItem::SubMenu)
                w += 2 * windowsArrowHMargin;
            else if (mi->menuItemType == QStyleOptionMenuItem::DefaultItem) {
                // adjust the font and add the difference in size.
                // it would be better if the font could be adjusted in the getStyleOptions qmenu func!!
                QFontMetrics fm(mi->font);
                QFont fontBold = mi->font;
                fontBold.setBold(true);
                QFontMetrics fmBold(fontBold);
                w += fmBold.width(mi->text) - fm.width(mi->text);
            }

            int checkcol = qMax(maxpmw, use2000style ? 20 : windowsCheckMarkWidth); // Windows always shows a check column
            w += checkcol;
            w += windowsRightBorder + 10;
            sz.setWidth(w);
        }
        break;
#endif // QT_NO_MENU
#ifndef QT_NO_MENUBAR
    case CT_MenuBarItem:
        if (!sz.isEmpty())
            sz += QSize(windowsItemHMargin * 4, windowsItemVMargin * 2);
        break;
#endif
                // Otherwise, fall through
    case CT_ToolButton:
        if (qstyleoption_cast<const QStyleOptionToolButton *>(opt)) 
            return sz += QSize(7, 6);
        // Otherwise, fall through

    default:
        sz = QCommonStyle::sizeFromContents(ct, opt, csz, widget);
    }
    return sz;
}
#endif

QIcon QWindowsStyle::standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *option,
                                         const QWidget *widget) const
{
    QPixmap iconPixmap = standardPixmap(standardIcon, option, widget);
    QIcon icon(iconPixmap);

    //Include a translated icon for use with pressed buttons
    QPixmap pressedIconPixmap(iconPixmap.size());
    pressedIconPixmap.fill(Qt::transparent);
    QPainter p(&pressedIconPixmap);
    p.drawPixmap(1, 1, iconPixmap.size().width(), iconPixmap.size().height(), iconPixmap);
    icon.addPixmap(pressedIconPixmap, QIcon::Normal, QIcon::On);
    return icon;
}
