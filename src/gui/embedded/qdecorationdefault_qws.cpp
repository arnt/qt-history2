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

QPixmap *QDecorationDefault::staticHelpPixmap = 0;
QPixmap *QDecorationDefault::staticMenuPixmap = 0;
QPixmap *QDecorationDefault::staticClosePixmap = 0;
QPixmap *QDecorationDefault::staticMinimizePixmap = 0;
QPixmap *QDecorationDefault::staticMaximizePixmap = 0;
QPixmap *QDecorationDefault::staticNormalizePixmap = 0;

#ifndef QT_NO_IMAGEIO_XPM

/* XPM */

static int menu_width = 20;
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

static int help_width = 20;
static const char * const default_help_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"                ",
"    ......      ",
"   ..XXXXXX     ",
"   .XX   .XX    ",
"   .XX   .XX    ",
"        ..XX    ",
"       ..XX     ",
"      ..XX      ",
"      .XX       ",
"      .XX       ",
"      ..        ",
"      .XX       ",
"      .XX       ",
"                ",
"                "};

static int close_width = 20;
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

static int maximize_width = 20;
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

static int minimize_width = 20;
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

static int normalize_width = 20;
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


QPixmap QDecorationDefault::pixmapFor(const QWidget *widget, int decorationRegion,
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
            if (widget->testWState(Qt::WState_Maximized))
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
                QRect r(rect.left() + titleHeight, rect.top() - titleHeight, rect.width()
                        - menu_width - close_width - maximize_width - minimize_width - help_width,
                        titleHeight);
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

        case Menu: {
                QRect r(rect.left(), rect.top() - titleHeight,
                        menu_width, titleHeight);
                region = r;
            }
            break;

        case Help: {
                QRect r(rect.right() - bw - close_width - maximize_width
                        - minimize_width - help_width, rect.top() - titleHeight,
                        help_width, titleHeight);
                if (r.left() > rect.left() + titleHeight)
                    region = r;
            }
            break;


        case Minimize: {
                QRect r(rect.right() - bw - close_width - maximize_width
                        - minimize_width, rect.top() - titleHeight,
                        minimize_width, titleHeight);
                if (r.left() > rect.left() + titleHeight)
                    region = r;
            }
            break;

        case Maximize: {
                QRect r(rect.right() - bw - close_width - maximize_width,
                        rect.top() - titleHeight, maximize_width, titleHeight);
                if (r.left() > rect.left() + titleHeight)
                    region = r;
            }
            break;

        case Close: {
                QRect r(rect.right() - bw - close_width, rect.top() - titleHeight,
                        close_width, titleHeight);
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
    const QRect titleRect = QDecoration::region(widget, Title).boundingRect();
    int titleHeight = titleRect.height();
    int titleWidth = titleRect.width();
    QRegion oldClipRegion = painter->clipRegion();

    bool paintAll = (decorationRegion == All);
    bool handled = false;

    if ((paintAll || decorationRegion & Borders) && state == Normal) {
        painter->setClipRegion(oldClipRegion - titleRect); // reduce flicker
        QRect br = QDecoration::region(widget).boundingRect();
        qDrawWinPanel(painter, br.x(), br.y(), br.width(),
                    br.height(), pal, false,
                    &pal.brush(QPalette::Background));
        handled |= true;
    }

    if ((paintAll || decorationRegion & Title && titleWidth > 0) && state == Normal) {
        painter->setClipRegion(oldClipRegion);
        QBrush titleBrush;
        QPen   titlePen;

        if (widget == qApp->activeWindow()) {
            titleBrush = pal.brush(QPalette::Highlight);
            titlePen   = pal.color(QPalette::HighlightedText);
        } else {
            titleBrush = pal.brush(QPalette::Background);
            titlePen   = pal.color(QPalette::Text);
        }

        qDrawShadePanel(painter,
                        titleRect.x(), titleRect.y(), titleRect.width(), titleRect.height(),
                        pal, true, 1, &titleBrush);

        painter->setPen(titlePen);
        painter->setFont(widget->font());
        painter->drawText(titleRect.x() + 4, titleRect.y(),
                          titleRect.width() - 8, titleRect.height(),
                          Qt::AlignVCenter, widget->windowTitle());
        handled |= true;
    }

    if (state != Hover) {
        painter->setClipRegion(oldClipRegion);
        if (paintAll || decorationRegion & Menu) {
            paintButton(painter, widget, Menu, state, pal);
            handled |= true;
        }

        if (paintAll || decorationRegion & Help) {
            paintButton(painter, widget, Help, state, pal);
            handled |= true;
        }

        if (paintAll || decorationRegion & Minimize) {
            paintButton(painter, widget, Minimize, state, pal);
            handled |= true;
        }

        if (paintAll || decorationRegion & Maximize) {
            paintButton(painter, widget, Maximize, state, pal);
            handled |= true;
        }

        if (paintAll || decorationRegion & Close) {
            paintButton(painter, widget, Close, state, pal);
            handled |= true;
        }
    }
    return handled;
}

void QDecorationDefault::paintButton(QPainter *painter, const QWidget *widget,
                                     int buttonRegion, DecorationState state, const QPalette &pal)
{
    int xoff = 2;
    int yoff = 2;

    const QPixmap pm = pixmapFor(widget, buttonRegion, xoff, yoff);
    QRect brect(QDecoration::region(widget, buttonRegion).boundingRect());

    if (state & QDecoration::Pressed) {
        qDrawWinPanel(painter, brect.x(), brect.y(), brect.width()-1, brect.height()-1,
                    pal, true, &pal.brush(QPalette::Background));
        ++xoff;
        ++yoff;
    } else {
        painter->fillRect(brect.x(), brect.y(), brect.width() - 1, brect.height() - 1,
                        pal.brush(QPalette::Background));
    }

    if (!pm.isNull())
        painter->drawPixmap(brect.x() + xoff, brect.y() + yoff, pm);
}


#endif // QT_NO_QWS_DECORATION_DEFAULT

#endif // QT_NO_QWS_MANAGER
