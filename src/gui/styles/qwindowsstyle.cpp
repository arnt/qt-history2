/****************************************************************************
**
** Implementation of Windows-like style class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qwindowsstyle.h"

#if !defined(QT_NO_STYLE_WINDOWS) || defined(QT_PLUGIN)

#include "qapplication.h"
#include "qbitmap.h"
#include "qcleanuphandler.h"
#include "qcombobox.h"
#include "qdrawutil.h" // for now
#include "qevent.h"
#include "qlabel.h"
#include "qlistbox.h"
#include "qmenu.h"
#include "qpainter.h"
#include "qpushbutton.h"
#include "qscrollbar.h"
#include "qslider.h"
#include "qstyleoption.h"
#include "qtabbar.h"
#include "qtabwidget.h"
#include "qwidget.h"


#if defined(Q_WS_WIN)
#include "qt_windows.h"

extern HDC qt_winHDC(const QPaintDevice *dev);
#endif

#include <limits.h>

static const int windowsItemFrame                =  2; // menu item frame width
static const int windowsSepHeight                =  2; // separator item height
static const int windowsItemHMargin                =  3; // menu item hor text margin
static const int windowsItemVMargin                =  2; // menu item ver text margin
static const int windowsArrowHMargin                =  6; // arrow horizontal margin
static const int windowsTabSpacing                = 12; // space between text and tab
static const int windowsCheckMarkHMargin        =  2; // horiz. margins of check mark
static const int windowsRightBorder                = 12; // right border on windows
static const int windowsCheckMarkWidth                = 12; // checkmarks width on windows

static bool use2000style = true;

enum QSliderDirection { SlUp, SlDown, SlLeft, SlRight };

// A friendly class providing access to Q3MenuData's protected member.
#if 0 // For now..
class FriendlyMenuData : public Q3MenuData
{
    friend class QWindowsStyle;
};
#endif // For now..

// Private class
class QWindowsStyle::Private : public QObject
{
public:
    Private(QWindowsStyle *parent);

    bool hasSeenAlt(const QWidget *widget) const;
    bool altDown() const { return alt_down; }

protected:
    bool eventFilter(QObject *o, QEvent *e);

private:
    QList<const QWidget *> seenAlt;
    bool alt_down;
    int menuBarTimer;
};

QWindowsStyle::Private::Private(QWindowsStyle *parent)
: QObject(parent, "QWindowsStylePrivate"), alt_down(false), menuBarTimer(0)
{
}

// Returns true if the toplevel parent of \a widget has seen the Alt-key
bool QWindowsStyle::Private::hasSeenAlt(const QWidget *widget) const
{
    widget = widget->topLevelWidget();
    return seenAlt.contains(widget);
}

// Records Alt- and Focus events
bool QWindowsStyle::Private::eventFilter(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return QObject::eventFilter(o, e);

    QWidget *widget = ::qt_cast<QWidget*>(o);

    switch(e->type()) {
    case QEvent::KeyPress:
        if (((QKeyEvent*)e)->key() == Qt::Key_Alt) {
            widget = widget->topLevelWidget();

            // Alt has been pressed - find all widgets that care
            QList<QWidget *> l = qFindChildren<QWidget *>(widget);
            for (int pos=0; pos<l.size(); ++pos) {
                QWidget *w = l.at(pos);
                if (w->isTopLevel() || !w->isVisible() ||
                    w->style().styleHint(SH_UnderlineShortcut, w))
                    l.removeAt(pos);
            }
            // Update states before repainting
            seenAlt.append(widget);
            alt_down = true;

            // Repaint all relevant widgets
            for (int pos=0; pos<l.size(); ++pos) {
                QWidget *w = static_cast<QWidget*>(l.at(pos));
                w->repaint();
            }
        }
        break;
#if 0 // For now..
    case QEvent::KeyRelease:
        if (((QKeyEvent*)e)->key() == Qt::Key_Alt) {
            widget = widget->topLevelWidget();

            // Update state
            alt_down = false;
            // Repaint only menubars
            QList<Q3MenuBar *> l = qFindChildren<Q3MenuBar *>(widget);
            for (int pos=0; pos<l.size(); ++pos) {
                Q3MenuBar *menuBar  = l.at(pos);
                menuBar->repaint();
            }
        }
        break;

    case QEvent::FocusIn:
    case QEvent::FocusOut:
        {
            // Menubars toggle based on focus
            Q3MenuBar *menuBar = qt_cast<Q3MenuBar*>(o);
            if (menuBar && !menuBarTimer) // delayed repaint to avoid flicker
                menuBarTimer = menuBar->startTimer(0);
        }
        break;
    case QEvent::Timer:
        {
            Q3MenuBar *menuBar = qt_cast<Q3MenuBar*>(o);
            QTimerEvent *te = (QTimerEvent*)e;
            if (menuBar && te->timerId() == menuBarTimer) {
                menuBar->killTimer(te->timerId());
                menuBarTimer = 0;
                menuBar->repaint();
                return true;
            }
        }
        break;
#endif // For now..
    case QEvent::Close:
        // Reset widget when closing
        seenAlt.removeAll(widget);
        seenAlt.removeAll(widget->topLevelWidget());
        break;
    default:
        break;
    }

    return QObject::eventFilter(o, e);
}

/*!
    \class QWindowsStyle qwindowsstyle.h
    \brief The QWindowsStyle class provides a Microsoft Windows-like look and feel.

    \ingroup appearance

    This style is Qt's default GUI style on Windows.
*/

/*!
    Constructs a QWindowsStyle
*/
QWindowsStyle::QWindowsStyle() : QCommonStyle(), d(0)
{
#if defined(Q_OS_WIN32)
    use2000style = QSysInfo::WindowsVersion != QSysInfo::WV_NT && QSysInfo::WindowsVersion != QSysInfo::WV_95;
#endif
}

/*! \reimp */
QWindowsStyle::~QWindowsStyle()
{
    delete d;
}

/*! \reimp */
void QWindowsStyle::polish(QApplication *app)
{
    // We only need the overhead when shortcuts are sometimes hidden
    if (!styleHint(SH_UnderlineShortcut, 0)) {
        d = new Private(this);
        app->installEventFilter(d);
    }
}

/*! \reimp */
void QWindowsStyle::unPolish(QApplication *)
{
    delete d;
    d = 0;
}

/*! \reimp */
void QWindowsStyle::polish(QWidget *widget)
{
    QCommonStyle::polish(widget);
    if(QMenu *menu = qt_cast<QMenu*>(widget))
        menu->setCheckable(true);
#if 0 // For now..
#ifdef QT_COMPAT
    if(Q3PopupMenu *popup = qt_cast<Q3PopupMenu*>(widget))
        popup->setCheckable(true);
#endif
#endif // For now..
}

/*! \reimp */
void QWindowsStyle::unPolish(QWidget *widget)
{
    QCommonStyle::polish(widget);
}

/*! \reimp */
void QWindowsStyle::polish(QPalette &pal)
{
    QCommonStyle::polish(pal);
}

/*!
  \reimp
*/
void QWindowsStyle::drawControl(ControlElement element,
                                 QPainter *p,
                                 const QWidget *widget,
                                 const QRect &r,
                                 const QPalette &pal,
                                 SFlags flags,
                                 const QStyleOption& opt) const
{
    switch (element) {
#if 0
    case CE_TabBarTab:
        {
            if (!widget || !opt.tab())
                break;

            const QTabBar * tb = (const QTabBar *) widget;
            const QTab * t = opt.tab();
            bool selected = flags & Style_Selected;
            bool lastTab = (tb->indexOf(t->identifier()) == tb->count()-1) ?
                           true : false;
            QRect r2(r);
            if (tb->shape()  == QTabBar::RoundedAbove) {
                p->setPen(pal.midlight());

                p->drawLine(r2.left(), r2.bottom(), r2.right(), r2.bottom());
                p->setPen(pal.light());
                p->drawLine(r2.left(), r2.bottom()-1, r2.right(), r2.bottom()-1);
                if (r2.left() == 0)
                    p->drawPoint(tb->rect().bottomLeft());

                if (selected) {
                    p->fillRect(QRect(r2.left()+1, r2.bottom()-1, r2.width()-3, 2),
                                 pal.brush(QPalette::Background));
                    p->setPen(pal.background());
                    p->drawLine(r2.left()+1, r2.bottom(), r2.left()+1, r2.top()+2);
                    p->setPen(pal.light());
                } else {
                    p->setPen(pal.light());
                    r2.setRect(r2.left() + 2, r2.top() + 2,
                               r2.width() - 4, r2.height() - 2);
                }

                int x1, x2;
                x1 = r2.left();
                x2 = r2.right() - 2;
                p->drawLine(x1, r2.bottom()-1, x1, r2.top() + 2);
                x1++;
                p->drawPoint(x1, r2.top() + 1);
                x1++;
                p->drawLine(x1, r2.top(), x2, r2.top());
                if (r2.left() > 0) {
                    p->setPen(pal.midlight());
                }
                x1 = r2.left();
                p->drawPoint(x1, r2.bottom());

                p->setPen(pal.midlight());
                x1++;
                p->drawLine(x1, r2.bottom(), x1, r2.top() + 2);
                x1++;
                p->drawLine(x1, r2.top()+1, x2, r2.top()+1);

                p->setPen(pal.dark());
                x2 = r2.right() - 1;
                p->drawLine(x2, r2.top() + 2, x2, r2.bottom() - 1 +
                             (selected ? 1:-1));
                p->setPen(pal.shadow());
                p->drawPoint(x2, r2.top() + 1);
                p->drawPoint(x2, r2.top() + 1);
                x2++;
                p->drawLine(x2, r2.top() + 2, x2, r2.bottom() -
                             (selected ? (lastTab ? 0:1) :2));
            } else if (tb->shape() == QTabBar::RoundedBelow) {
                bool rightAligned = styleHint(SH_TabBar_Alignment, tb) == Qt::AlignRight;
                bool firstTab = tb->indexOf(t->identifier()) == 0;
                if (selected) {
                    p->fillRect(QRect(r2.left()+1, r2.top(), r2.width()-3, 1),
                                 pal.brush(QPalette::Background));
                    p->setPen(pal.background());
                    p->drawLine(r2.left()+1, r2.top(), r2.left()+1, r2.bottom()-2);
                    p->setPen(pal.dark());
                } else {
                    p->setPen(pal.shadow());
                    p->drawLine(r2.left() +
                                 (rightAligned && firstTab ? 0 : 1),
                                 r2.top() + 1,
                                 r2.right() - (lastTab ? 0 : 2),
                                 r2.top() + 1);

                    if (rightAligned && lastTab)
                        p->drawPoint(r2.right(), r2.top());
                    p->setPen(pal.dark());
                    p->drawLine(r2.left(), r2.top(), r2.right() - 1,
                                 r2.top());
                    r2.setRect(r2.left() + 2, r2.top(),
                                r2.width() - 4, r2.height() - 2);
                }

                p->drawLine(r2.right() - 1, r2.top() + (selected ? 0: 2),
                             r2.right() - 1, r2.bottom() - 2);
                p->drawPoint(r2.right() - 2, r2.bottom() - 2);
                p->drawLine(r2.right() - 2, r2.bottom() - 1,
                             r2.left() + 1, r2.bottom() - 1);

                p->setPen(pal.midlight());
                p->drawLine(r2.left() + 1, r2.bottom() - 2,
                             r2.left() + 1, r2.top() + (selected ? 0 : 2));

                p->setPen(pal.shadow());
                p->drawLine(r2.right(),
                             r2.top() + (lastTab && rightAligned &&
                                         selected) ? 0 : 1,
                             r2.right(), r2.bottom() - 1);
                p->drawPoint(r2.right() - 1, r2.bottom() - 1);
                p->drawLine(r2.right() - 1, r2.bottom(),
                             r2.left() + 2, r2.bottom());

                p->setPen(pal.light());
                p->drawLine(r2.left(), r2.top() + (selected ? 0 : 2),
                             r2.left(), r2.bottom() - 2);
            } else {
                QCommonStyle::drawControl(element, p, widget, r, pal, flags, opt);
            }
            break;
        }
#endif
    case CE_ToolBoxTab:
        {
            qDrawShadePanel(p, r, pal, flags & (Style_Sunken | Style_Down | Style_On) , 1,
                             &pal.brush(QPalette::Button));
            break;
        }

    case CE_MenuBarItem: {
        bool active = flags & Style_Active;
        bool hasFocus = flags & Style_HasFocus;
        bool down = flags & Style_Down;
        QRect pr = r;

        p->fillRect(r, pal.brush(QPalette::Button));
        if (active || hasFocus) {
            QBrush b = pal.brush(QPalette::Button);
            if (active && down)
                p->setBrushOrigin(p->brushOrigin() + QPoint(1,1));
            if (active && hasFocus)
                qDrawShadeRect(p, r.x(), r.y(), r.width(), r.height(),
                               pal, active && down, 1, 0, &b);
            if (active && down) {
                pr.moveBy(pixelMetric(PM_ButtonShiftHorizontal, widget),
                          pixelMetric(PM_ButtonShiftVertical, widget));
                p->setBrushOrigin(p->brushOrigin() - QPoint(1,1));
            }
        }
        QCommonStyle::drawControl(element, p, widget, pr, pal, flags, opt);
        break; }
    default:
        QCommonStyle::drawControl(element, p, widget, r, pal, flags, opt);
    }
}


/*!
  \reimp
*/
int QWindowsStyle::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    int ret;

    switch (metric) {
    case PM_ButtonDefaultIndicator:
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        ret = 1;
        break;

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
        {
            const QSlider * sl = (const QSlider *) widget;
            int space = (sl->orientation() == Qt::Horizontal) ? sl->height()
                        : sl->width();
            int ticks = sl->tickmarks();
            int n = 0;
            if (ticks & QSlider::Above) n++;
            if (ticks & QSlider::Below) n++;
            if (!n) {
                ret = space;
                break;
            }

            int thick = 6;        // Magic constant to get 5 + 16 + 5
            if (ticks != QSlider::Both && ticks != QSlider::NoMarks)
                thick += pixelMetric(PM_SliderLength, sl) / 4;

            space -= thick;
            //### the two sides may be unequal in size
            if (space > 0)
                thick += (space * 2) / (n + 2);
            ret = thick;
            break;
        }
#endif // QT_NO_SLIDER

    case PM_MenuBarFrameWidth:
        ret = 0;
        break;

#if defined(Q_WS_WIN)
    case PM_TitleBarHeight:
#ifndef QT_NO_MAINWINDOW
        if (widget && (widget->testWFlags(Qt::WStyle_Tool) || qt_cast<QDockWindow*>(widget))) {
            // MS always use one less than they say
#if defined(Q_OS_TEMP)
            ret = GetSystemMetrics(SM_CYCAPTION) - 1;
#else
            ret = GetSystemMetrics(SM_CYSMCAPTION) - 1;
#endif
        } else
#endif // QT_NO_MAINWINDOW
        {
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
                ret = QCommonStyle::pixelMetric(metric, widget);
        }
        break;
#endif

    case PM_SplitterWidth:
        ret = qMax(6, QApplication::globalStrut().width());
        break;

#if defined(Q_WS_WIN)
    case PM_MDIFrameWidth:
        ret = GetSystemMetrics(SM_CYFRAME);
        break;
#endif

    default:
        ret = QCommonStyle::pixelMetric(metric, widget);
        break;
    }

    return ret;
}


/*!
  \reimp
*/
QSize QWindowsStyle::sizeFromContents(ContentsType contents,
                                       const QWidget *widget,
                                       const QSize &contentsSize,
                                       const QStyleOption& opt) const
{
    QSize sz(contentsSize);

    switch (contents) {
    case CT_PushButton:
        {
#ifndef QT_NO_PUSHBUTTON
            const QPushButton *button = (const QPushButton *) widget;
            sz = QCommonStyle::sizeFromContents(contents, widget, contentsSize, opt);
            int w = sz.width(), h = sz.height();

            int defwidth = 0;
            if (button->isDefault() || button->autoDefault())
                defwidth = 2*pixelMetric(PM_ButtonDefaultIndicator, widget);

            if (w < 80+defwidth)
                w = 80+defwidth;
            if (h < 23+defwidth)
                h = 23+defwidth;

            sz = QSize(w, h);
#endif
            break;
        }

    case CT_MenuBarItem: {
        if(!sz.isEmpty())
            sz = QSize(sz.width()+(windowsItemVMargin*2), sz.height()+(windowsItemHMargin*2));
        break; }

    case CT_MenuItem:
        {
#ifndef QT_NO_MENU
            if (opt.isDefault())
                break;

            const QMenu *menu = (const QMenu *) widget;
            bool checkable = menu->isCheckable();
            QAction *act = opt.action();
            int maxpmw = opt.maxIconWidth();
            int w = sz.width(), h = sz.height();

            if (act->isSeparator()) {
                w = 10;
                h = 2;
            } else {
                h = qMax(h, menu->fontMetrics().height() + 8);
                if(!act->icon().isNull())
                    h = qMax(h,act->icon().pixmap(QIconSet::Small, QIconSet::Normal).height() + 2*windowsItemFrame);
            }

            if (!act->text().isNull()) {
                if (act->text().contains('\t'))
                    w += 12;
            }

            if (maxpmw)
                w += maxpmw + 6;
            if (checkable && maxpmw < 20)
                w += 20 - maxpmw;
            if (checkable || maxpmw > 0)
                w += 2;
            w += 12;

            sz = QSize(w, h);
#endif
            break;
        }

    default:
        sz = QCommonStyle::sizeFromContents(contents, widget, sz, opt);
        break;
    }

    return sz;
}

#ifndef QT_NO_IMAGEIO_XPM

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
".#########..",
".#########..",
".#.......#..",
".#.......#..",
".#.......#..",
".#.......#..",
".#.......#..",
".#.......#..",
".#########..",
"............",
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
"............",
"..######....",
"..######....",
"............",
"............"};

static const char * const qt_normalizeup_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"....######..",
"....######..",
"....#....#..",
"..######.#..",
"..######.#..",
"..#....###..",
"..#....#....",
"..#....#....",
"..######....",
"............",
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
"...................**...........",
};
#endif //QT_NO_IMAGEIO_XPM

/*!
 \reimp
 */
QPixmap QWindowsStyle::stylePixmap(StylePixmap stylepixmap,
                                   const QWidget *widget,
                                   const QStyleOption& opt) const
{
#ifndef QT_NO_IMAGEIO_XPM
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
#endif //QT_NO_IMAGEIO_XPM
    return QCommonStyle::stylePixmap(stylepixmap, widget, opt);
}

/*!\reimp
*/
void QWindowsStyle::drawComplexControl(ComplexControl ctrl, QPainter *p,
                                        const QWidget *widget,
                                        const QRect &r,
                                        const QPalette &pal,
                                        SFlags flags,
                                        SCFlags sub,
                                        SCFlags subActive,
                                        const QStyleOption& opt) const
{
    switch (ctrl) {
    default:
        QCommonStyle::drawComplexControl(ctrl, p, widget, r, pal, flags, sub,
                                          subActive, opt);
        break;
    }
}


/*! \reimp */
int QWindowsStyle::styleHint(StyleHint hint,
                              const QWidget *widget,
                              const QStyleOption &opt,
                              QStyleHintReturn *returnData) const
{
    int ret;

    switch (hint) {
    case SH_EtchDisabledText:
    case SH_Slider_SnapToValue:
    case SH_PrintDialog_RightAlignButtons:
    case SH_MainWindow_SpaceBelowMenuBar:
    case SH_FontDialog_SelectAssociatedText:
    case SH_Menu_AllowActiveAndDisabled:
    case SH_MenuBar_AltKeyNavigation:
    case SH_MenuBar_MouseTracking:
    case SH_Menu_MouseTracking:
    case SH_ComboBox_ListMouseTracking:
    case SH_ScrollBar_StopMouseOverSlider:
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
            ret = cues ? 1 : 0;
            // Do nothing if we always paint underlines
            if (!ret && widget && d) {
#if 0 // For now
                Q3MenuBar *menuBar = ::qt_cast<Q3MenuBar*>(widget);
                Q3PopupMenu *popupMenu = 0;
                if (!menuBar)
                    popupMenu = ::qt_cast<Q3PopupMenu*>(widget);

                // If we paint a menubar draw underlines if it has focus, or if alt is down,
                // or if a popup menu belonging to the menubar is active and paints underlines
                if (menuBar) {
                    if (menuBar->hasFocus()) {
                        ret = 1;
                    } else if (d->altDown()) {
                        ret = 1;
                    } else if (qApp->focusWidget() && qApp->focusWidget()->isPopup()) {
                        popupMenu = ::qt_cast<Q3PopupMenu*>(qApp->focusWidget());
                        Q3MenuData *pm = popupMenu ? static_cast<Q3MenuData*>(popupMenu) : 0;
                        if (pm && ((FriendlyMenuData*)pm)->parentMenu == menuBar) {
                            if (d->hasSeenAlt(menuBar))
                                ret = 1;
                        }
                    }
                // If we paint a popup menu draw underlines if the respective menubar does
                } else if (popupMenu) {
                    Q3MenuData *pm = static_cast<Q3MenuData*>(popupMenu);
                    while (pm) {
                        if (((FriendlyMenuData*)pm)->isMenuBar) {
                            menuBar = (Q3MenuBar*)pm;
                            if (d->hasSeenAlt(menuBar))
                                ret = 1;
                            break;
                        }
                        pm = ((FriendlyMenuData*)pm)->parentMenu;
                    }
                // Otherwise draw underlines if the toplevel widget has seen an alt-press
                } else if (d->hasSeenAlt(widget)) {
                    ret = 1;
                }
#else
                ret = 1;
#endif // For now..
            }

        }
        break;
#endif

    default:
        ret = QCommonStyle::styleHint(hint, widget, opt, returnData);
        break;
    }

    return ret;
}

/*! \reimp */
QRect QWindowsStyle::subRect(SubRect r, const QWidget *widget) const
{
    QRect rect;

    switch (r) {
#ifndef QT_NO_SLIDER
    case SR_SliderFocusRect:
        {
            rect = widget->rect();
            break;
        }
#endif // QT_NO_SLIDER
    case SR_ToolBoxTabContents:
        rect = widget->rect();
        break;
    default:
        rect = QCommonStyle::subRect(r, widget);
        break;
    }

    return rect;
}

/*! \reimp */
void QWindowsStyle::drawPrimitive(PrimitiveElement pe, const Q4StyleOption *opt, QPainter *p,
                                  const QWidget *w) const
{
    switch (pe) {
    case PE_ButtonTool: {
        QBrush fill;
        bool stippled;
        if (!(opt->state & (Style_Down | Style_MouseOver)) && (opt->state & Style_On)
                && use2000style) {
            fill = QBrush(opt->palette.light(), Qt::Dense4Pattern);
            stippled = true;
        } else {
            fill = opt->palette.brush(QPalette::Button);
            stippled = false;
        }

        if (opt->state & (Style_Raised | Style_Down | Style_On)) {
            if (opt->state & Style_AutoRaise) {
                qDrawShadePanel(p, opt->rect, opt->palette,
                                opt->state & (Style_Down | Style_On), 1, &fill);
                if (stippled) {
                    p->setPen(opt->palette.button());
                    p->drawRect(opt->rect.x() + 1, opt->rect.y() + 1, opt->rect.width() - 2,
                                opt->rect.height() - 2);
                }
            } else
                qDrawWinButton(p, opt->rect, opt->palette,
                               opt->state & (Style_Down | Style_On), &fill);
        } else {
            p->fillRect(opt->rect, fill);
        }
        break; }
    case PE_ButtonCommand: {
        QBrush fill;
        SFlags flags = opt->state;
        QPalette pal = opt->palette;
        QRect r = opt->rect;
        if (! (flags & Style_Down) && (flags & Style_On))
            fill = QBrush(pal.light(), Qt::Dense4Pattern);
        else
            fill = pal.brush(QPalette::Button);

        if (flags & Style_ButtonDefault && flags & Style_Down) {
            p->setPen(pal.dark());
            p->setBrush(fill);
            p->drawRect(r);
        } else if (flags & (Style_Raised | Style_Down | Style_On | Style_Sunken))
            qDrawWinButton(p, r, pal, flags & (Style_Sunken | Style_Down | Style_On), &fill);
        else
            p->fillRect(r, fill);
        break; }
    case PE_ButtonDefault:
        p->setPen(opt->palette.shadow());
        p->drawRect(opt->rect);
        break;
    case PE_ArrowUp:
    case PE_ArrowDown:
    case PE_ArrowRight:
    case PE_ArrowLeft: {
        QPointArray a;
        switch (pe) {
            case PE_ArrowUp:
                a.setPoints(7, -4, 1, 2, 1, -3, 0, 1, 0, -2, -1, 0, -1, -1, -2);
                break;

            case PE_ArrowDown:
                a.setPoints(7, -4, -2, 2, -2, -3, -1, 1, -1, -2, 0, 0, 0, -1, 1);
                break;

            case PE_ArrowRight:
                a.setPoints(7, -2, -3, -2, 3, -1, -2, -1, 2, 0, -1, 0, 1, 1, 0);
                break;

            case PE_ArrowLeft:
                a.setPoints(7, 0, -3, 0, 3, -1, -2, -1, 2, -2, -1, -2, 1, -3, 0);
                break;

            default:
                break;
        }
        p->save();
        if (opt->state & Style_Down)
            p->translate(pixelMetric(PM_ButtonShiftHorizontal),
                         pixelMetric(PM_ButtonShiftVertical));
        if (opt->state & Style_Enabled) {
            a.translate(opt->rect.x() + opt->rect.width() / 2,
                        opt->rect.y() + opt->rect.height() / 2);
            p->setPen(opt->palette.buttonText());
            p->drawLineSegments(a, 0, 3);         // draw arrow
            p->drawPoint(a[6]);
        } else {
            a.translate(opt->rect.x() + opt->rect.width() / 2 + 1,
                        opt->rect.y() + opt->rect.height() / 2 + 1);
            p->setPen(opt->palette.light());
            p->drawLineSegments(a, 0, 3);         // draw arrow
            p->drawPoint(a[6]);
            a.translate(-1, -1);
            p->setPen(opt->palette.mid());
            p->drawLineSegments(a, 0, 3);         // draw arrow
            p->drawPoint(a[6]);
        }
        p->restore();
        break; }
    case PE_Indicator: {
        QBrush fill;
        if (opt->state & Style_NoChange)
            fill = QBrush(opt->palette.base(), Qt::Dense4Pattern);
        else if (opt->state & Style_Down)
            fill = opt->palette.button();
        else if (opt->state & Style_Enabled)
            fill = opt->palette.base();
        else
            fill = opt->palette.background();
        qDrawWinPanel(p, opt->rect, opt->palette, true, &fill);
        if (opt->state & Style_NoChange)
            p->setPen(opt->palette.dark());
        else
            p->setPen(opt->palette.text());
        } // Fall through!
    case PE_CheckListIndicator:
        if (pe == PE_CheckListIndicator) {
            if (opt->state & Style_Enabled)
                p->setPen(QPen(opt->palette.text(), 1));
            else
                p->setPen(QPen(opt->palette.dark(), 1));
            if (opt->state & Style_NoChange)
                p->setBrush(opt->palette.brush(QPalette::Button));
            p->drawRect(opt->rect.x() + 1, opt->rect.y() + 1, 11, 11);
        }
        if (!(opt->state & Style_Off)) {
            QPointArray a(7 * 2);
            int i, xx, yy;
            xx = opt->rect.x() + 3;
            yy = opt->rect.y() + 5;
            for (i = 0; i < 3; ++i) {
                a.setPoint(2 * i, xx, yy);
                a.setPoint(2 * i + 1, xx, yy + 2);
                ++xx;
                ++yy;
            }
            yy -= 2;
            for (i = 3; i < 7; ++i) {
                a.setPoint(2 * i, xx, yy);
                a.setPoint(2 * i + 1, xx, yy + 2);
                ++xx;
                --yy;
            }
            p->drawLineSegments(a);
        }
        break;
    case PE_FocusRect:
        if (const Q4StyleOptionFocusRect *fropt = qt_cast<const Q4StyleOptionFocusRect *>(opt)) {
#if defined (Q_WS_WIN) && !defined(QT_GDIPLUS_SUPPORT)
            {
                HDC hdc = qt_winHDC(p->device());
                RECT rect = { opt->rect.left(), opt->rect.top(), opt->rect.right() + 1,
                    opt->rect.bottom() + 1 };
                    DrawFocusRect(hdc, &rect);
            }
#else
            QRect r = opt->rect;
            p->save();
            p->setBackgroundMode(Qt::TransparentMode);
            QColor bg_col = fropt->backgroundColor;
            if (!bg_col.isValid())
                bg_col = p->background().color();
            if (qGray(bg_col.rgb()) < 128)
                p->setBrush(QBrush(Qt::white, Qt::Dense4Pattern));
            else
                p->setBrush(QBrush(Qt::black, Qt::Dense4Pattern));
            p->setPen(Qt::NoPen);
            p->drawRect(r.left(), r.top(), r.width(), 1);    // Top
            p->drawRect(r.left(), r.bottom(), r.width(), 1); // Bottom
            p->drawRect(r.left(), r.top(), 1, r.height());   // Left
            p->drawRect(r.right(), r.top(), 1, r.height());  // Right
            p->restore();
#endif
        }
        break;
    case PE_ExclusiveIndicator:
        {
#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)
            static const QCOORD pts1[] = {              // dark lines
                1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
            static const QCOORD pts2[] = {              // black lines
                2,8, 1,7, 1,4, 2,3, 2,2, 3,2, 4,1, 7,1, 8,2, 9,2 };
            static const QCOORD pts3[] = {              // background lines
                2,9, 3,9, 4,10, 7,10, 8,9, 9,9, 9,8, 10,7, 10,4, 9,3 };
            static const QCOORD pts4[] = {              // white lines
                2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
                11,4, 10,3, 10,2 };
            static const QCOORD pts5[] = {              // inner fill
                4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };

            // make sure the indicator is square
            QRect ir = opt->rect;

            if (opt->rect.width() < opt->rect.height()) {
                ir.setTop(opt->rect.top() + (opt->rect.height() - opt->rect.width()) / 2);
                ir.setHeight(opt->rect.width());
            } else if (opt->rect.height() < opt->rect.width()) {
                ir.setLeft(opt->rect.left() + (opt->rect.width() - opt->rect.height()) / 2);
                ir.setWidth(opt->rect.height());
            }

            bool down = opt->state & Style_Down;
            bool enabled = opt->state & Style_Enabled;
            bool on = opt->state & Style_On;
            QPointArray a;
            a.setPoints(QCOORDARRLEN(pts1), pts1);
            a.translate(ir.x(), ir.y());
            p->setPen(opt->palette.dark());
            p->drawPolyline(a);
            a.setPoints(QCOORDARRLEN(pts2), pts2);
            a.translate(ir.x(), ir.y());
            p->setPen(opt->palette.shadow());
            p->drawPolyline(a);
            a.setPoints(QCOORDARRLEN(pts3), pts3);
            a.translate(ir.x(), ir.y());
            p->setPen(opt->palette.midlight());
            p->drawPolyline(a);
            a.setPoints(QCOORDARRLEN(pts4), pts4);
            a.translate(ir.x(), ir.y());
            p->setPen(opt->palette.light());
            p->drawPolyline(a);
            a.setPoints(QCOORDARRLEN(pts5), pts5);
            a.translate(ir.x(), ir.y());
            QColor fillColor = (down || !enabled) ? opt->palette.button() : opt->palette.base();
            p->setPen(fillColor);
            p->setBrush(fillColor) ;
            p->drawPolygon(a);
            if (on) {
                p->setPen(Qt::NoPen);
                p->setBrush(opt->palette.text());
                p->drawRect(ir.x() + 5, ir.y() + 4, 2, 4);
                p->drawRect(ir.x() + 4, ir.y() + 5, 4, 2);
            }
            break;
        }
    case PE_ScrollBarSubLine:
    case PE_ScrollBarAddLine: {
        if (use2000style && opt->state & Style_Down) {
            p->setPen(opt->palette.dark());
            p->setBrush(opt->palette.brush(QPalette::Button));
            p->drawRect(opt->rect);
        } else {
            Q4StyleOption buttonOpt = *opt;
            if (!(buttonOpt.state & Style_Down))
                buttonOpt.state = Style_Raised;
            drawPrimitive(PE_ButtonBevel, &buttonOpt, p, w);
        }
        PrimitiveElement arrow;
        if (opt->state & Style_Horizontal) {
            if (pe == PE_ScrollBarAddLine)
                arrow = PE_ArrowRight;
            else
                arrow = PE_ArrowLeft;
        } else {
            if (pe == PE_ScrollBarAddLine)
                arrow = PE_ArrowDown;
            else
                arrow = PE_ArrowUp;
        }
        drawPrimitive(arrow, opt, p, w);
        break; }
    case PE_ScrollBarAddPage:
    case PE_ScrollBarSubPage: {
            QBrush br;
            QColor c = p->background().color();
            p->setPen(Qt::NoPen);
            p->setBackgroundMode(Qt::OpaqueMode);

            if (opt->state & Style_Down) {
                br = QBrush(opt->palette.shadow(), Qt::Dense4Pattern);
                p->setBackground(opt->palette.dark());
                p->setBrush(QBrush(opt->palette.shadow(), Qt::Dense4Pattern));
            } else {
                br = opt->palette.brush(QPalette::Light).pixmap()
                     ? opt->palette.brush(QPalette::Light)
                     : QBrush(opt->palette.light(), Qt::Dense4Pattern);
                p->setBrush(br);
            }
            p->drawRect(opt->rect);
            p->setBackground(c);
            break; }
    case PE_ScrollBarSlider:
        if (!(opt->state & Style_Enabled)) {
            QBrush br = opt->palette.brush(QPalette::Light).pixmap()
                        ?  opt->palette.brush(QPalette::Light)
                        : QBrush(opt->palette.light(), Qt::Dense4Pattern);
            p->setPen(Qt::NoPen);
            p->setBrush(br);
            p->setBackgroundMode(Qt::OpaqueMode);
            p->drawRect(opt->rect);
        } else {
            Q4StyleOption buttonOpt = *opt;
            buttonOpt.state = Style_Enabled | Style_Raised;
            drawPrimitive(PE_ButtonBevel, &buttonOpt, p, w);
        }
        break;
    case PE_MenuFrame:
    case PE_Panel:
    case PE_PanelPopup:
        if (const Q4StyleOptionFrame *frame = qt_cast<const Q4StyleOptionFrame *>(opt)) {
            if (frame->lineWidth == 2) {
                QPalette popupPal = frame->palette;
                if (pe == PE_PanelPopup) {
                    popupPal.setColor(QPalette::Light, frame->palette.background());
                    popupPal.setColor(QPalette::Midlight, frame->palette.light());
                }
                qDrawWinPanel(p, frame->rect, popupPal, frame->state & Style_Sunken);
            } else {
                QCommonStyle::drawPrimitive(pe, opt, p, w);
            }
        }
        break;
    case PE_Splitter: {
        QPen oldPen = p->pen();
        p->setPen(opt->palette.light());
        if (opt->state & Style_Horizontal) {
            p->drawLine(opt->rect.x() + 1, opt->rect.y(), opt->rect.x() + 1, opt->rect.height());
            p->setPen(opt->palette.dark());
            p->drawLine(opt->rect.x(), opt->rect.y(), opt->rect.x(), opt->rect.height());
            p->drawLine(opt->rect.right() - 1, opt->rect.y(), opt->rect.right() - 1,
                        opt->rect.height());
            p->setPen(opt->palette.shadow());
            p->drawLine(opt->rect.right(), opt->rect.y(), opt->rect.right(), opt->rect.height());
        } else {
            p->drawLine(opt->rect.x(), opt->rect.y() + 1, opt->rect.width(), opt->rect.y() + 1);
            p->setPen(opt->palette.dark());
            p->drawLine(opt->rect.x(), opt->rect.bottom() - 1, opt->rect.width(),
                        opt->rect.bottom() - 1);
            p->setPen(opt->palette.shadow());
            p->drawLine(opt->rect.x(), opt->rect.bottom(), opt->rect.width(), opt->rect.bottom());
        }
        p->setPen(oldPen);
        break; }
    case PE_TreeBranch: {
        // This is _way_ too similar to the common style.
        static const int decoration_size = 9;
        int mid_h = opt->rect.x() + opt->rect.width() / 2;
        int mid_v = opt->rect.y() + opt->rect.height() / 2;
        int bef_h = mid_h;
        int bef_v = mid_v;
        int aft_h = mid_h;
        int aft_v = mid_v;
        if (opt->state & Style_Children) {
            int delta = decoration_size / 2;
            bef_h -= delta;
            bef_v -= delta;
            aft_h += delta;
            aft_v += delta;
            p->drawLine(bef_h + 2, bef_v + 4, bef_h + 6, bef_v + 4);
            if (!(opt->state & Style_Open))
                p->drawLine(bef_h + 4, bef_v + 2, bef_h + 4, bef_v + 6);
            QPen oldPen = p->pen();
            p->setPen(opt->palette.dark());
            p->drawRect(bef_h, bef_v, decoration_size, decoration_size);
            p->setPen(oldPen);
        }
        QBrush brush(opt->palette.dark(), Qt::Dense4Pattern);
        if (opt->state & Style_Item) {
            if (QApplication::reverseLayout())
                p->fillRect(opt->rect.left(), mid_v, bef_h - opt->rect.left(), 1, brush);
            else
                p->fillRect(aft_h, mid_v, opt->rect.right() - aft_h + 1, 1, brush);
        }
        if (opt->state & Style_Sibling)
            p->fillRect(mid_h, aft_v, 1, opt->rect.bottom() - aft_v + 1, brush);
        if (opt->state & (Style_Open | Style_Children | Style_Item | Style_Sibling))
            p->fillRect(mid_h, opt->rect.y(), 1, bef_v - opt->rect.y(), brush);
        break; }
    case PE_ButtonBevel:
    case PE_HeaderSection: {
        QBrush fill;
        if (!(opt->state & Style_Down) && (opt->state & Style_On))
            fill = QBrush(opt->palette.light(), Qt::Dense4Pattern);
        else
            fill = opt->palette.brush(QPalette::Button);

        if (opt->state & (Style_Raised | Style_Down | Style_On | Style_Sunken))
            qDrawWinButton(p, opt->rect, opt->palette, opt->state & (Style_Down | Style_On), &fill);
        else
            p->fillRect(opt->rect, fill);
        break; }
#if defined(Q_WS_WIN)
    case PE_HeaderArrow:
        if (const Q4StyleOptionHeader *header = qt_cast<const Q4StyleOptionHeader *>(opt)) {
            QPen oldPen = p->pen();
            if (header->state & Style_Up) { // invert logic to follow Windows style guide
                QPointArray pa(3);
                p->setPen(header->palette.light());
                p->drawLine(header->rect.x() + header->rect.width(), header->rect.y(),
                            header->rect.x() + header->rect.width() / 2, header->rect.height());
                p->setPen(header->palette.dark());
                pa.setPoint(0, header->rect.x() + header->rect.width() / 2, header->rect.height());
                pa.setPoint(1, header->rect.x(), header->rect.y());
                pa.setPoint(2, header->rect.x() + header->rect.width(), header->rect.y());
                p->drawPolyline(pa);
            } else {
                QPointArray pa(3);
                p->setPen(header->palette.light());
                pa.setPoint(0, header->rect.x(), header->rect.height());
                pa.setPoint(1, header->rect.x() + header->rect.width(), header->rect.height());
                pa.setPoint(2, header->rect.x() + header->rect.width() / 2, header->rect.y());
                p->drawPolyline(pa);
                p->setPen(header->palette.dark());
                p->drawLine(header->rect.x(), header->rect.height(),
                            header->rect.x() + header->rect.width() / 2, header->rect.y());
            }
            p->setPen(oldPen);
        }
        break;
#endif
    case PE_WindowFrame: {
         QPalette popupPal = opt->palette;
         popupPal.setColor(QPalette::Light, opt->palette.background());
         popupPal.setColor(QPalette::Midlight, opt->palette.light());
         qDrawWinPanel(p, opt->rect, popupPal, opt->state & Style_Sunken);
        break; }
    case PE_DockWindowResizeHandle: {
        QPen oldPen = p->pen();
        p->setPen(opt->palette.light());
        if (opt->state & Style_Horizontal) {
            p->drawLine(opt->rect.x(), opt->rect.y(), opt->rect.width(), opt->rect.y());
            p->setPen(opt->palette.dark());
            p->drawLine(opt->rect.x(), opt->rect.bottom() - 1, opt->rect.width(),
                        opt->rect.bottom() - 1);
            p->setPen(opt->palette.shadow());
            p->drawLine(opt->rect.x(), opt->rect.bottom(), opt->rect.width(), opt->rect.bottom());
        } else {
            p->drawLine(opt->rect.x(), opt->rect.y(), opt->rect.x(), opt->rect.height());
            p->setPen(opt->palette.dark());
            p->drawLine(opt->rect.right() - 1, opt->rect.y(), opt->rect.right() - 1,
                        opt->rect.height());
            p->setPen(opt->palette.shadow());
            p->drawLine(opt->rect.right(), opt->rect.y(), opt->rect.right(), opt->rect.height());
        }
        p->setPen(oldPen);
        break; }
    default:
        QCommonStyle::drawPrimitive(pe, opt, p, w);
    }
}

/*! \reimp */
void QWindowsStyle::drawControl(ControlElement ce, const Q4StyleOption *opt, QPainter *p,
                                const QWidget *widget) const
{
    switch (ce) {
    case CE_MenuItem:
        if (const Q4StyleOptionMenuItem *menuitem = qt_cast<const Q4StyleOptionMenuItem *>(opt)) {
            int tab = menuitem->tabWidth;
            int maxpmw = menuitem->maxIconWidth;
            bool dis = !(menuitem->state & Style_Enabled);
            bool checked = menuitem->checkState == Q4StyleOptionMenuItem::Checked;
            bool act = menuitem->state & Style_Active;

            int x, y, w, h;
            menuitem->rect.rect(&x, &y, &w, &h);

            if (menuitem->checkState != Q4StyleOptionMenuItem::NotCheckable) {
                // space for the checkmarks
                if (use2000style)
                    maxpmw = qMax(maxpmw, 20);
                else
                    maxpmw = qMax(maxpmw, 12);
            }

            int checkcol = maxpmw;
            if (menuitem->menuItemType == Q4StyleOptionMenuItem::Separator) {
                p->setPen(menuitem->palette.dark());
                p->drawLine(x, y, x + w, y);
                p->setPen(menuitem->palette.light());
                p->drawLine(x, y + 1, x + w, y + 1);
                return;
            }

            QBrush fill = menuitem->palette.brush(act ? QPalette::Highlight : QPalette::Button);
            p->fillRect(x, y, w, h, fill);

            int xpos = x;
            QRect vrect = visualRect(QRect(xpos, y, checkcol, h), menuitem->rect);
            int xvis = vrect.x();
            if (checked) {
                if (act && !dis) {
                    qDrawShadePanel(p, xvis, y, checkcol, h,
                                    menuitem->palette, true, 1,
                                    &menuitem->palette.brush(QPalette::Button));
                } else {
                    QBrush fill(menuitem->palette.light(), Qt::Dense4Pattern);
                    // set the brush origin for the hash pattern to the x/y coordinate
                    // of the menu item's checkmark... this way, the check marks have
                    // a consistent look
                    QPoint origin = p->brushOrigin();
                    p->setBrushOrigin(xvis, y);
                    qDrawShadePanel(p, xvis, y, checkcol, h, menuitem->palette, true, 1, &fill);
                    // restore the previous brush origin
                    p->setBrushOrigin(origin);
                }
            } else if (!act) {
                p->fillRect(xvis, y, checkcol , h, menuitem->palette.brush(QPalette::Button));
            }

            // On Windows Style, if we have a checkable item and an icon we
            // draw the icon recessed to indicate an item is checked. If we
            // have no icon, we draw a checkmark instead.
            if (!menuitem->icon.isNull()) {
                QIconSet::Mode mode = dis ? QIconSet::Disabled : QIconSet::Normal;
                if (act && !dis)
                    mode = QIconSet::Active;
                QPixmap pixmap;
                if (checked)
                    pixmap = menuitem->icon.pixmap(QIconSet::Small, mode, QIconSet::On);
                else
                    pixmap = menuitem->icon.pixmap(QIconSet::Small, mode);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                if (act && !dis && menuitem->checkState == Q4StyleOptionMenuItem::Unchecked)
                    qDrawShadePanel(p, xvis, y, checkcol, h, menuitem->palette, false, 1,
                                    &menuitem->palette.brush(QPalette::Button));
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(vrect.center());
                p->setPen(menuitem->palette.text());
                p->drawPixmap(pmr.topLeft(), pixmap);

                fill = menuitem->palette.brush(act ? QPalette::Highlight : QPalette::Button);
                int xp = xpos + checkcol + 1;
                p->fillRect(visualRect(QRect(xp, y, w - checkcol - 1, h), menuitem->rect), fill);
            } else if (checked) {
                Q4StyleOptionMenuItem newMi = *menuitem;
                newMi.state = Style_Default;
                if (!dis)
                    newMi.state |= Style_Enabled;
                if (act)
                    newMi.state |= Style_On;
                int xp = xpos + windowsItemFrame;
                newMi.rect = visualRect(QRect(xp, y + windowsItemFrame,
                                        checkcol - 2 * windowsItemFrame, h - 2*windowsItemFrame),
                                        menuitem->rect);
                drawPrimitive(PE_CheckMark, &newMi, p, widget);
            }
            p->setPen(act ? menuitem->palette.highlightedText() : menuitem->palette.buttonText());

            QColor discol;
            if (dis) {
                discol = menuitem->palette.text();
                p->setPen(discol);
            }

            int xm = windowsItemFrame + checkcol + windowsItemHMargin;
            xpos += xm;
            vrect = visualRect(QRect(xpos, y + windowsItemVMargin, w - xm - tab + 1,
                                     h - 2 * windowsItemVMargin), menuitem->rect);
            xvis = vrect.x();
            if (menuitem->menuItemType == Q4StyleOptionMenuItem::Q3Custom) {
                // Grr... how do we paint it...
                /*
                p->save();
                if (dis && !act) {
                    p->setPen(pal.light());
                    mi->custom()->paint(p, pal, act, !dis,
                                         xvis+1, y+windowsItemVMargin+1, w-xm-tab+1, h-2*windowsItemVMargin);
                    p->setPen(discol);
                }
                mi->custom()->paint(p, pal, act, !dis,
                                     xvis, y+windowsItemVMargin, w-xm-tab+1, h-2*windowsItemVMargin);
                p->restore();
                */
            }
            QString s = menuitem->text;
            if (!s.isEmpty()) {                     // draw text
                int t = s.indexOf('\t');
                int text_flags = Qt::AlignVCenter | Qt::ShowPrefix | Qt::DontClip | Qt::SingleLine;
                if (!styleHint(SH_UnderlineShortcut, widget))
                    text_flags |= Qt::NoAccel;
                text_flags |= (QApplication::reverseLayout() ? Qt::AlignRight : Qt::AlignLeft);
                if (t >= 0) {
                    int xp = x + w - tab - windowsItemHMargin - windowsItemFrame + 1;
                    if (use2000style)
                        xp -= 20;
                    else
                        xp -= windowsRightBorder;
                    int xoff = visualRect(QRect(xp, y + windowsItemVMargin, tab,
                                                h - 2 * windowsItemVMargin), menuitem->rect).x();
                    if (dis && !act) {
                        p->setPen(menuitem->palette.light());
                        p->drawText(xoff + 1, y + windowsItemVMargin + 1, tab,
                                    h - 2 * windowsItemVMargin, text_flags, s.mid(t + 1));
                        p->setPen(discol);
                    }
                    p->drawText(xoff, y + windowsItemVMargin, tab, h - 2 * windowsItemVMargin,
                                text_flags, s.mid(t + 1));
                    s = s.left(t);
                }
                if (dis && !act) {
                    p->setPen(menuitem->palette.light());
                    p->drawText(xvis + 1, y + windowsItemVMargin + 1, w - xm - tab + 1,
                                h - 2 * windowsItemVMargin, text_flags, s, t);
                    p->setPen(discol);
                }
                p->drawText(xvis, y + windowsItemVMargin, w - xm - tab + 1,
                            h - 2 * windowsItemVMargin, text_flags, s, t);
            }
            if (menuitem->menuItemType == Q4StyleOptionMenuItem::SubMenu) {// draw sub menu arrow
                int dim = (h - 2 * windowsItemFrame) / 2;
                PrimitiveElement arrow;
                arrow = QApplication::reverseLayout() ? PE_ArrowLeft : PE_ArrowRight;
                xpos = x + w - windowsArrowHMargin - windowsItemFrame - dim;
                vrect = visualRect(QRect(xpos, y + h / 2 - dim / 2, dim, dim), menuitem->rect);
                Q4StyleOptionMenuItem newMI = *menuitem;
                newMI.rect = vrect;
                newMI.state = dis ? Style_Default : Style_Enabled;
                if (act)
                    newMI.palette.setColor(QPalette::ButtonText, newMI.palette.highlightedText());
                drawPrimitive(arrow, &newMI, p, widget);
            }

        }
        break;
    case CE_MenuBarItem:
        if (const Q4StyleOptionMenuItem *mbi = qt_cast<const Q4StyleOptionMenuItem *>(opt)) {
            bool active = mbi->state & Style_Active;
            bool hasFocus = mbi->state & Style_HasFocus;
            bool down = mbi->state & Style_Down;
            Q4StyleOptionMenuItem newMbi = *mbi;
            p->fillRect(mbi->rect, mbi->palette.brush(QPalette::Button));
            if (active || hasFocus) {
                QBrush b = mbi->palette.brush(QPalette::Button);
                if (active && down)
                    p->setBrushOrigin(p->brushOrigin() + QPoint(1, 1));
                if (active && hasFocus)
                    qDrawShadeRect(p, mbi->rect.x(), mbi->rect.y(), mbi->rect.width(),
                                   mbi->rect.height(), mbi->palette, active && down, 1, 0, &b);
                if (active && down) {
                    newMbi.rect.moveBy(pixelMetric(PM_ButtonShiftHorizontal, widget),
                                       pixelMetric(PM_ButtonShiftVertical, widget));
                    p->setBrushOrigin(p->brushOrigin() - QPoint(1, 1));
                }
            }
            QCommonStyle::drawControl(ce, &newMbi, p, widget);
        }
        break;
    default:
        QCommonStyle::drawControl(ce, opt, p, widget);
    }
}

/*! \reimp */
QRect QWindowsStyle::subRect(SubRect sr, const Q4StyleOption *opt, const QWidget *w) const
{
    QRect r;
    switch (sr) {
    default:
        r = QCommonStyle::subRect(sr, opt, w);
    }
    return r;
}

/*! \reimp */
void QWindowsStyle::drawComplexControl(ComplexControl cc, const Q4StyleOptionComplex *opt,
                                       QPainter *p, const QWidget *widget) const
{
    switch (cc) {
    case CC_Slider:
        if (const Q4StyleOptionSlider *slider = qt_cast<const Q4StyleOptionSlider *>(opt)) {
            int thickness  = pixelMetric(PM_SliderControlThickness, widget);
            int len        = pixelMetric(PM_SliderLength, widget);
            int ticks = slider->tickmarks;
            Q4StyleOptionSlider querySlider = *slider;
            querySlider.parts = SC_SliderGroove;
            QRect groove = QCommonStyle::querySubControlMetrics(CC_Slider, &querySlider, widget);
            querySlider.parts = SC_SliderHandle;
            QRect handle = QCommonStyle::querySubControlMetrics(CC_Slider, &querySlider, widget);

            if ((slider->parts & SC_SliderGroove) && groove.isValid()) {
                int mid = thickness / 2;

                if (ticks & QSlider::Above)
                    mid += len / 8;
                if (ticks & QSlider::Below)
                    mid -= len / 8;

                p->setPen(slider->palette.shadow());
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

            if (slider->parts & SC_SliderTickmarks) {
                querySlider.parts = SC_SliderTickmarks;
                QCommonStyle::drawComplexControl(cc, &querySlider, p, widget);
            }

            if (slider->parts & SC_SliderHandle) {
                // 4444440
                // 4333310
                // 4322210
                // 4322210
                // 4322210
                // 4322210
                // *43210*
                // **410**
                // ***0***
                const QColor c0 = slider->palette.shadow();
                const QColor c1 = slider->palette.dark();
                // const QColor c2 = g.button();
                const QColor c3 = slider->palette.midlight();
                const QColor c4 = slider->palette.light();

                int x = handle.x(), y = handle.y(),
                   wi = handle.width(), he = handle.height();

                int x1 = x;
                int x2 = x+wi-1;
                int y1 = y;
                int y2 = y+he-1;

                Qt::Orientation orient = slider->orientation;
                bool tickAbove = slider->tickmarks == QSlider::Above;
                bool tickBelow = slider->tickmarks == QSlider::Below;

                p->fillRect(x, y, wi, he, slider->palette.brush(QPalette::Background));

                if (slider->state & Style_HasFocus) {
                    Q4StyleOptionFocusRect fropt(0);
                    fropt.rect = subRect(SR_SliderFocusRect, slider, widget);
                    fropt.palette = slider->palette;
                    fropt.state = Style_Default;
                    drawPrimitive(PE_FocusRect, &fropt, p, widget);
                }

                if ((tickAbove && tickBelow) || (!tickAbove && !tickBelow)) {
                    qDrawWinButton(p, QRect(x, y, wi, he), slider->palette, false,
                                   &slider->palette.brush(QPalette::Button));
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

                QPointArray a;

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
                p->setBrush(slider->palette.brush(QPalette::Button));
                p->setPen(Qt::NoPen);
                p->drawRect(x1, y1, x2-x1+1, y2-y1+1);
                p->drawPolygon(a);
                p->setBrush(oldBrush);

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
    case CC_ListView:
        if (const Q4StyleOptionListView *lv = qt_cast<const Q4StyleOptionListView *>(opt)) {
            int i;
            if (lv->parts & SC_ListView)
                QCommonStyle::drawComplexControl(cc, lv, p, widget);
            if (lv->parts & (SC_ListViewBranch | SC_ListViewExpand)) {
                if (lv->items.isEmpty())
                    break;
                Q4StyleOptionListViewItem item = lv->items.at(0);
                int y = lv->rect.y();
                int c;
                int dotoffset = 0;
                QPointArray dotlines;
                if (lv->activeParts == SC_All && lv->parts == SC_ListViewExpand) {
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
                        Q4StyleOptionListViewItem child = lv->items.at(i);
                        if (child.height + y > 0)
                            break;
                        y += child.totalHeight;
                    }
                    int bx = lv->rect.width() / 2;

                    // paint stuff in the magical area
                    while (i < lv->items.size() && y < lv->rect.height()) {
                        Q4StyleOptionListViewItem child = lv->items.at(i);
                        if (child.extras & Q4StyleOptionListViewItem::Visible) {
                            int lh;
                            if (!(item.extras & Q4StyleOptionListViewItem::MultiLine))
                                lh = child.height;
                            else
                                lh = p->fontMetrics().height() + 2 * lv->itemMargin;
                            lh = qMax(lh, QApplication::globalStrut().height());
                            if (lh % 2 > 0)
                                ++lh;
                            linebot = y + lh / 2;
                            if (child.extras & Q4StyleOptionListViewItem::Expandable
                                || child.childCount > 0 && child.height > 0) {
                                // needs a box
                                p->setPen(lv->palette.mid());
                                p->drawRect(bx - 4, linebot - 4, 9, 9);
                                // plus or minus
                                p->setPen(lv->palette.text());
                                p->drawLine(bx - 2, linebot, bx + 2, linebot);
                                if (!(child.state & Style_Open))
                                    p->drawLine(bx, linebot - 2, bx, linebot + 2);
                                // dotlinery
                                p->setPen(lv->palette.mid());
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
                p->setPen(lv->palette.text());

                static QBitmap *verticalLine = 0, *horizontalLine = 0;
                static QCleanupHandler<QBitmap> qlv_cleanup_bitmap;
                if (!verticalLine) {
                    // make 128*1 and 1*128 bitmaps that can be used for
                    // drawing the right sort of lines.
                    verticalLine = new QBitmap(1, 129, true);
                    horizontalLine = new QBitmap(128, 1, true);
                    QPointArray a(64);
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
                    qlv_cleanup_bitmap.add(&verticalLine);
                    qlv_cleanup_bitmap.add(&horizontalLine);
                }

                int line; // index into dotlines
                if (lv->parts & SC_ListViewBranch) for(line = 0; line < c; line += 2) {
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
    case CC_ComboBox:
        if (const Q4StyleOptionComboBox *cmb = qt_cast<const Q4StyleOptionComboBox *>(opt)) {
            Q4StyleOptionComboBox newCmb = *cmb;
            if (cmb->parts & SC_ComboBoxArrow) {
                SFlags flags = Style_Default;
                
                qDrawWinPanel(p, opt->rect, opt->palette, true,
                              cmb->state & Style_Enabled ? &cmb->palette.brush(QPalette::Base)
                                                         : &cmb->palette.brush(QPalette::Background));
                
                newCmb.parts = SC_ComboBoxArrow;
                QRect ar =
                    QStyle::visualRect(QCommonStyle::querySubControlMetrics(CC_ComboBox, &newCmb,
                                                                            widget), widget);
                if (cmb->activeParts == SC_ComboBoxArrow) {
                    p->setPen(cmb->palette.dark());
                    p->setBrush(cmb->palette.brush(QPalette::Button));
                    p->drawRect(ar);
                } else {
                    qDrawWinPanel(p, ar, cmb->palette, false,
                                  &cmb->palette.brush(QPalette::Button));
                }
                
                ar.addCoords(2, 2, -2, -2);
                if (opt->state & Style_Enabled)
                    flags |= Style_Enabled;
                
                if (cmb->activeParts == SC_ComboBoxArrow)
                    flags |= Style_Sunken;
                Q4StyleOption arrowOpt(0, Q4StyleOption::Default);
                arrowOpt.rect = ar;
                arrowOpt.palette = cmb->palette;
                arrowOpt.state = flags;
                drawPrimitive(PE_ArrowDown, &arrowOpt, p, widget);
            }
            if (cmb->parts & SC_ComboBoxEditField) {
                newCmb.parts = SC_ComboBoxEditField;
                QRect re =
                    QStyle::visualRect(QCommonStyle::querySubControlMetrics(CC_ComboBox, &newCmb,
                                                                            widget), widget);
                if (cmb->state & Style_HasFocus && !cmb->editable)
                    p->fillRect(re.x(), re.y(), re.width(), re.height(),
                                cmb->palette.brush(QPalette::Highlight));
                
                if (cmb->state & Style_HasFocus) {
                    p->setPen(cmb->palette.highlightedText());
                    p->setBackground(cmb->palette.highlight());
                    
                } else {
                    p->setPen(cmb->palette.text());
                    p->setBackground(cmb->palette.background());
                }
                
                if (cmb->state & Style_HasFocus && !cmb->editable) {
                    Q4StyleOptionFocusRect focus(0);
                    focus.rect = QStyle::visualRect(subRect(SR_ComboBoxFocusRect, cmb, widget),
                                                    widget);
                    focus.palette = cmb->palette;
                    focus.state = Style_FocusAtBorder;
                    focus.backgroundColor = cmb->palette.highlight();
                    drawPrimitive(PE_FocusRect, &focus, p, widget);
                }
            }
        }
        break;
    default:
        QCommonStyle::drawComplexControl(cc, opt, p, widget);
    }
}

/*! \reimp */
QSize QWindowsStyle::sizeFromContents(ContentsType ct, const Q4StyleOption *opt, const QSize &csz,
                                      const QFontMetrics &fm, const QWidget *widget) const
{
    QSize sz(csz);
    switch (ct) {
    case CT_PushButton:
        if (const Q4StyleOptionButton *btn = qt_cast<const Q4StyleOptionButton *>(opt)) {
            sz = QCommonStyle::sizeFromContents(ct, opt, csz, fm, widget);
            int w = sz.width(),
                h = sz.height();
            int defwidth = 0;
            if (btn->state & Style_ButtonDefault)
                defwidth = 2 * pixelMetric(PM_ButtonDefaultIndicator, widget);
            if (w < 80 + defwidth && btn->icon.isNull())
                w = 80 + defwidth;
            if (h < 23 + defwidth)
                h = 23 + defwidth;
            sz = QSize(w, h);
        }
        break;
    case CT_MenuItem:
        if (const Q4StyleOptionMenuItem *mi = qt_cast<const Q4StyleOptionMenuItem *>(opt)) {
            sz = QCommonStyle::sizeFromContents(ct, opt, csz, fm, widget);
            if ((mi->menuItemType != Q4StyleOptionMenuItem::Separator
                 && mi->menuItemType != Q4StyleOptionMenuItem::Q3Custom) && !mi->icon.isNull())
                 sz.setHeight(qMax(csz.height(),
                              mi->icon.pixmap(QIconSet::Small, QIconSet::Normal).height()
                              + 2 * windowsItemFrame));
        }
        break;
    case CT_MenuBarItem:
        if (!sz.isEmpty())
            sz = QSize(sz.width() + windowsItemVMargin * 2, sz.height() + windowsItemHMargin * 2);
        break;
    default:
        sz = QCommonStyle::sizeFromContents(ct, opt, csz, fm, widget);
    }
    return sz;
}
#endif
