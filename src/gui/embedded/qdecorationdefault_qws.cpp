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
#include <qapplication.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include "qdecorationdefault_qws.h"

#ifndef QT_NO_QWS_MANAGER
#if !defined(QT_NO_QWS_DECORATION_DEFAULT) || defined(QT_PLUGIN)

QPixmap * QDecorationDefault::staticHelpPixmap=0;
QPixmap * QDecorationDefault::staticMenuPixmap=0;
QPixmap * QDecorationDefault::staticClosePixmap=0;
QPixmap * QDecorationDefault::staticMinimizePixmap=0;
QPixmap * QDecorationDefault::staticMaximizePixmap=0;
QPixmap * QDecorationDefault::staticNormalizePixmap=0;

#ifndef QT_NO_IMAGEIO_XPM

/* XPM */
static const char * const default_menu_xpm[] = {
/* width height ncolors chars_per_pixel */
"16 16 11 1",
/* colors */
"  c #000000",
". c #336600",
"X c #666600",
"o c #99CC00",
"O c #999933",
"+ c #333300",
"@ c #669900",
"# c #999900",
"$ c #336633",
"% c #666633",
"& c #99CC33",
/* pixels */
"oooooooooooooooo",
"oooooooooooooooo",
"ooooo#.++X#ooooo",
"ooooX      Xoooo",
"oooX  XO#%  X&oo",
"oo#  Ooo&@O  Ooo",
"oo. Xoo#+ @X Xoo",
"oo+ OoO+ +O# +oo",
"oo+ #O+  +## +oo",
"oo. %@ ++ +. Xoo",
"oo#  O@OO+   #oo",
"oooX  X##$   Ooo",
"ooooX        Xoo",
"oooo&OX++X#OXooo",
"oooooooooooooooo",
"oooooooooooooooo"
};

static const char * const default_help_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"     ......     ",
"    .XXXXXXX    ",
"   .XX    .XX   ",
"  .XX      .XX  ",
"           .XX  ",
"          .XX   ",
"         .XX    ",
"       .XX      ",
"      .XX       ",
"      .XX       ",
"      ..        ",
"      .XX       ",
"      .XX       ",
"                ",
"                "};

static const char * const default_close_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"                ",
"  .X        .X  ",
"  .XX      .XX  ",
"   .XX    .XX   ",
"    .XX  .XX    ",
"     .XX.XX     ",
"      .XXX      ",
"      .XXX      ",
"     .XX.XX     ",
"    .XX  .XX    ",
"   .XX    .XX   ",
"  .XX      .XX  ",
"  .X        .X  ",
"                ",
"                "};

static const char * const default_maximize_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"                ",
"  ...........   ",
"  .XXXXXXXXXX   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X........X   ",
"  .XXXXXXXXXX   ",
"                ",
"                ",
"                "};

static const char * const default_minimize_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"       ...      ",
"       . X      ",
"       .XX      ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};

static const char * const default_normalize_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"                ",
"     ........   ",
"     .XXXXXXXX  ",
"     .X     .X  ",
"     .X     .X  ",
"  ....X...  .X  ",
"  .XXXXXXXX .X  ",
"  .X     .XXXX  ",
"  .X     .X     ",
"  .X     .X     ",
"  .X......X     ",
"  .XXXXXXXX     ",
"                ",
"                ",
"                "};

#endif // QT_NO_IMAGEIO_XPM

QDecorationDefault::QDecorationDefault()
    : QDecoration()
{
}

QDecorationDefault::~QDecorationDefault()
{
    delete staticMenuPixmap;
    delete staticClosePixmap;
    delete staticMinimizePixmap;
    delete staticMaximizePixmap;
    delete staticNormalizePixmap;

    // This makes it safe to delete and then create a QDecorationDefault
    staticMenuPixmap = 0;
    staticClosePixmap = 0;
    staticMinimizePixmap = 0;
    staticMaximizePixmap = 0;
    staticNormalizePixmap = 0;
}

#ifndef QT_NO_IMAGEIO_XPM
const char **QDecorationDefault::helpPixmap()
{
    return (const char **)default_help_xpm;
}

const char **QDecorationDefault::menuPixmap()
{
    return (const char **)default_menu_xpm;
}

const char **QDecorationDefault::closePixmap()
{
    return (const char **)default_close_xpm;
}

const char **QDecorationDefault::minimizePixmap()
{
    return (const char **)default_minimize_xpm;
}

const char **QDecorationDefault::maximizePixmap()
{
    return (const char **)default_maximize_xpm;
}

const char **QDecorationDefault::normalizePixmap()
{
    return (const char **)default_normalize_xpm;
}
#endif


QPixmap QDecorationDefault::pixmapFor(const QWidget *widget, int decorationRegion, bool on,
                                      int &xoff, int &/*yoff*/)
{
#ifndef QT_NO_IMAGEIO_XPM
    static const char** staticHelpPixmapXPM=0;
    static const char** staticMenuPixmapXPM=0;
    static const char** staticClosePixmapXPM=0;
    static const char** staticMinimizePixmapXPM=0;
    static const char** staticMaximizePixmapXPM=0;
    static const char** staticNormalizePixmapXPM=0;
    const char** xpm;

    // Why don't we just use/extend the enum type...

    if (staticHelpPixmapXPM != (xpm=helpPixmap()) || !staticHelpPixmap) {
        staticHelpPixmapXPM = xpm;
        staticHelpPixmap = new QPixmap(xpm);
    }
    if (staticMenuPixmapXPM != (xpm=menuPixmap()) || !staticMenuPixmap) {
        staticMenuPixmapXPM = xpm;
        staticMenuPixmap = new QPixmap(xpm);
    }
    if (staticClosePixmapXPM != (xpm=closePixmap()) || !staticClosePixmap) {
        staticClosePixmapXPM = xpm;
        staticClosePixmap = new QPixmap(xpm);
    }
    if (staticMinimizePixmapXPM != (xpm=minimizePixmap()) || !staticMinimizePixmap) {
        staticMinimizePixmapXPM = xpm;
        staticMinimizePixmap = new QPixmap(xpm);
    }
    if (staticMaximizePixmapXPM != (xpm=maximizePixmap()) || !staticMaximizePixmap) {
        staticMaximizePixmapXPM = xpm;
        staticMaximizePixmap = new QPixmap(xpm);
    }
    if (staticNormalizePixmapXPM != (xpm=normalizePixmap()) || staticNormalizePixmap) {
        staticNormalizePixmapXPM = xpm;
        staticNormalizePixmap = new QPixmap(xpm);
    }

    const QPixmap *pm = 0;

    switch (decorationRegion) {
        case Help:
            pm = staticHelpPixmap;
            break;
        case Menu:
#ifndef QT_NO_WIDGET_TOPEXTRA
            if (!widget->windowIcon().isNull())
                return widget->windowIcon();
#endif
            if (!pm) {
                xoff = 1;
                pm = staticMenuPixmap;
            }
            break;
        case Close:
            pm = staticClosePixmap;
            break;
        case Maximize:
            if (on)
                pm = staticNormalizePixmap;
            else
                pm = staticMaximizePixmap;
            break;
        case Minimize:
            pm = staticMinimizePixmap;
            break;
        default:
            break;
    }
    return *pm;
#else
    return 0;
#endif
}

int QDecorationDefault::getTitleWidth(const QWidget *widget)
{
    return widget->width() - 4 * getTitleHeight(widget) - 4;
}

int QDecorationDefault::getTitleHeight(const QWidget *)
{
    return 20;
}

QRegion QDecorationDefault::region(const QWidget *widget, const QRect &rect, int decorationRegion)
{
    int titleHeight = getTitleHeight(widget);
    int bw = BORDER_WIDTH;
    int bbw = BOTTOM_BORDER_WIDTH;

    QRegion region;
    switch (decorationRegion) {
        case All: {
                if (widget->isMaximized()) {
                    QRect r(rect.left(), rect.top() - titleHeight,
                            rect.width(), rect.height() + titleHeight);
                    region = r;
                } else {
                    QRect r(rect.left() - bw,
                            rect.top() - titleHeight - bw,
                            rect.width() + 2 * bw,
                            rect.height() + titleHeight + bw + bbw);
                    region = r;
                }
                region -= rect;
            }
            break;

        case Title: {
                    QRect r(rect.left() + titleHeight, rect.top() - titleHeight,
                            rect.width() - 4*titleHeight, titleHeight);
                    if (r.width() > 0)
                        region = r;
            }
            break;

        case Top: {
                    QRect r(rect.left() + CORNER_GRAB,
                            rect.top() - titleHeight - bw,
                            rect.width() - 2 * CORNER_GRAB,
                            bw);
                    region = r;
            }
            break;

        case Left: {
                QRect r(rect.left() - bw,
                        rect.top() - titleHeight + CORNER_GRAB,
                        bw,
                        rect.height() + titleHeight - 2 * CORNER_GRAB);
                region = r;
            }
            break;

        case Right: {
                QRect r(rect.right() + 1,
                        rect.top() - titleHeight + CORNER_GRAB,
                        bw,
                        rect.height() + titleHeight - 2 * CORNER_GRAB);
                region = r;
            }
            break;

        case Bottom: {
                QRect r(rect.left() + CORNER_GRAB,
                        rect.bottom() + 1,
                        rect.width() - 2 * CORNER_GRAB,
                        bw);
                region = r;
            }
            break;

        case TopLeft: {
                QRect r1(rect.left() - bw,
                        rect.top() - bw - titleHeight,
                        CORNER_GRAB + bw,
                        bw);

                QRect r2(rect.left() - bw,
                        rect.top() - bw - titleHeight,
                        bw,
                        CORNER_GRAB + bw);

                region = QRegion(r1) + r2;
            }
            break;

        case TopRight: {
                QRect r1(rect.right() - CORNER_GRAB,
                        rect.top() - bw - titleHeight,
                        CORNER_GRAB + bw,
                        bw);

                QRect r2(rect.right() + 1,
                        rect.top() - bw - titleHeight,
                        bw,
                        CORNER_GRAB + bw);

                region = QRegion(r1) + r2;
            }
            break;

        case BottomLeft: {
                QRect r1(rect.left() - bw,
                        rect.bottom() + 1,
                        CORNER_GRAB + bw,
                        bw);

                QRect r2(rect.left() - bw,
                        rect.bottom() - CORNER_GRAB,
                        bw,
                        CORNER_GRAB + bw);
                region = QRegion(r1) + r2;
            }
            break;

        case BottomRight: {
                QRect r1(rect.right() - CORNER_GRAB,
                        rect.bottom() + 1,
                        CORNER_GRAB + bw,
                        bw);

                QRect r2(rect.right() + 1,
                        rect.bottom() - CORNER_GRAB,
                        bw,
                        CORNER_GRAB + bw);
                region = QRegion(r1) + r2;
            }
            break;

        case Help: {
                QRect r(rect.right() - 4*titleHeight, rect.top() - titleHeight,
                        titleHeight, titleHeight);
                if (r.left() > rect.left() + titleHeight)
                    region = r;
            }
            break;

        case Menu: {
                    QRect r(rect.left(), rect.top() - titleHeight,
                            titleHeight, titleHeight);
                    region = r;
            }
            break;

        case Close: {
                QRect r(rect.right() - titleHeight, rect.top() - titleHeight,
                        titleHeight, titleHeight);
                if (r.left() > rect.left() + titleHeight)
                    region = r;
            }
            break;

        case Maximize: {
                QRect r(rect.right() - 2*titleHeight, rect.top() - titleHeight,
                        titleHeight, titleHeight);
                if (r.left() > rect.left() + titleHeight)
                    region = r;
            }
            break;

        case Minimize: {
                QRect r(rect.right() - 3*titleHeight, rect.top() - titleHeight,
                        titleHeight, titleHeight);
                if (r.left() > rect.left() + titleHeight)
                    region = r;
            }
            break;

        default:
            break;
    }

    return region;
}

bool QDecorationDefault::paint(QPainter *painter, const QWidget *widget, int decorationRegion,
                               DecorationState state)
{
    if (decorationRegion == None)
        return false;

    const QPalette pal = widget->palette();
    int titleHeight = getTitleHeight(widget);
    int titleWidth = getTitleWidth(widget);

    bool paintAll = (decorationRegion == All);
    bool handled = false;
    if (paintAll || decorationRegion & Borders) {
        QRegion oldClip = painter->clipRegion();
        painter->setClipRegion(oldClip - // reduce flicker
                               QRect(titleHeight, -titleHeight,  titleWidth, titleHeight - 1));

        QRect br = QDecoration::region(widget).boundingRect();
        qDrawWinPanel(painter, br.x(), br.y(), br.width(),
                    br.height(), pal, false,
                    &pal.brush(QPalette::Background));

        painter->setClipRegion(oldClip);

        handled |= true;
    }
    if (paintAll || decorationRegion & Title) {
        qWarning("QDecorationDefault::paint(): Title - NYI!");
    }
    if (paintAll || decorationRegion & Menu) {
        qWarning("QDecorationDefault::paint(): Menu - NYI!");
    }
    if (paintAll || decorationRegion & Help) {
        qWarning("QDecorationDefault::paint(): Help - NYI!");
    }
    if (paintAll || decorationRegion & Minimize) {
        qWarning("QDecorationDefault::paint(): Minimize - NYI!");
    }
    if (paintAll || decorationRegion & Maximize) {
        qWarning("QDecorationDefault::paint(): Maximize - NYI!");
    }
    if (paintAll || decorationRegion & Close) {
        qWarning("QDecorationDefault::paint(): Close - NYI!");
    }
    return handled;
}

#if 0
void QDecorationDefault::paint(QPainter *painter, const QWidget *widget)
{
    int titleWidth = getTitleWidth(widget);
    int titleHeight = getTitleHeight(widget);

    QRect rect(widget->rect());

    // title bar rect
    QRect tr(titleHeight, -titleHeight,  titleWidth, titleHeight - 1);

    QRegion oldClip = painter->clipRegion();
    painter->setClipRegion(oldClip - QRegion(tr));        // reduce flicker

#ifndef QT_NO_PALETTE
    QPalette pal = QApplication::palette();
//    const QPalette pal = widget->palette();
    pal.setCurrentColorGroup(QPalette::Active);

#if !defined(QT_NO_DRAWUTIL)
    // Border rect
    QRect br(rect.left() - BORDER_WIDTH,
                rect.top() - BORDER_WIDTH - titleHeight,
                rect.width() + 2 * BORDER_WIDTH,
                rect.height() + BORDER_WIDTH + BOTTOM_BORDER_WIDTH + titleHeight);

    qDrawWinPanel(painter, br.x(), br.y(), br.width(),
                  br.height() - 4, pal, false,
                  &pal.brush(QPalette::Background));
#endif

    painter->setClipRegion(oldClip);

    if (titleWidth > 0) {
        QBrush titleBrush;
        QPen   titlePen;
        int    titleLeft = titleHeight + 4;

        if (widget == qApp->activeWindow()) {
            titleBrush = pal.brush(QPalette::Highlight);
            titlePen   = pal.color(QPalette::HighlightedText);
        } else {
            titleBrush = pal.brush(QPalette::Background);
            titlePen   = pal.color(QPalette::Text);
        }

#define CLAMP(x, y)            (((x) > (y)) ? (y) : (x))

        {

#if !defined(QT_NO_DRAWUTIL)
            qDrawShadePanel(painter, tr.x(), tr.y(), tr.width(), tr.height(),
                            pal, true, 1, &titleBrush);
#endif

#ifndef QT_NO_WIDGET_TOPEXTRA
            painter->setPen(titlePen);
            painter->setFont(widget->font());
            painter->drawText(titleLeft, -titleHeight,
                            titleWidth-5, titleHeight - 1,
                            Qt::AlignVCenter, widget->windowTitle());
#endif
            return;
        }

#ifndef QT_NO_WIDGET_TOPEXTRA
        painter->setPen(titlePen);
        painter->setFont(widget->font());
        painter->drawText(titleLeft, -titleHeight,
                        rect.width() - titleHeight - 10, titleHeight-1,
                        Qt::AlignVCenter, widget->windowTitle());
#endif
    }

#endif //QT_NO_PALETTE

}

void QDecorationDefault::paintButton(QPainter *painter, const QWidget *w,
                        QDecoration::Region type, int state)
{
#ifndef QT_NO_PALETTE
    QPalette pal = QApplication::palette();
//    QPalette pal = w->palette();
    pal.setCurrentColorGroup(QPalette::Active);

    QRect brect(region(w, w->rect(), type).boundingRect());

    int xoff=2;
    int yoff=2;

    const QPixmap pm=pixmapFor(w,type,state & QWSButton::On, xoff, yoff);

    {

        if ((state & QWSButton::MouseOver) && (state & QWSButton::Clicked)) {
#if !defined(QT_NO_DRAWUTIL)
            qDrawWinPanel(painter, brect.x(), brect.y(), brect.width()-1,
                        brect.height()-1, pal, true,
                        &pal.brush(QPalette::Background));
#endif
            if (!pm.isNull()) painter->drawPixmap(brect.x()+xoff+1, brect.y()+yoff+1, pm);
        } else {
            painter->fillRect(brect.x(), brect.y(), brect.width()-1,
                        brect.height()-1, pal.brush(QPalette::Background));
            if (!pm.isNull()) painter->drawPixmap(brect.x()+xoff, brect.y()+yoff, pm);
        }
    }

#endif

}
#endif // 0 -------------------------------------------------



#endif // QT_NO_QWS_DECORATION_DEFAULT

#endif // QT_NO_QWS_MANAGER
