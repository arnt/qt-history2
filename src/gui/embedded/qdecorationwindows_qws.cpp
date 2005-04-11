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
#include "qdecorationwindows_qws.h"

#if !defined(QT_NO_QWS_DECORATION_WINDOWS) || defined(QT_PLUGIN)

#ifndef QT_NO_IMAGEIO_XPM

/* XPM */
static const char * const win_close_xpm[] = {
"16 16 4 1",
"  s None  c None",
". c #000000",
"X c #FFFFFF",
"Y c #707070",
"                ",
"                ",
"                ",
"   Y.      .Y   ",
"    ..    ..    ",
"     ..  ..     ",
"      .YY.      ",
"      Y..Y      ",
"      .YY.      ",
"     ..  ..     ",
"    ..    ..    ",
"   Y.      .Y   ",
"                ",
"                ",
"                ",
"                "};

static const char * const win_help_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #000000",
"                ",
"                ",
"                ",
"     XXXXXX     ",
"    XX    XX    ",
"    XX    XX    ",
"          XX    ",
"         XX     ",
"       XX       ",
"       XX       ",
"                ",
"       XX       ",
"       XX       ",
"                ",
"                ",
"                "};

static const char * const win_maximize_xpm[] = {
"16 16 4 1",
"  s None  c None",
". c #000000",
"X c #FFFFFF",
"Y c #707070",
"                ",
"                ",
"                ",
"   ..........   ",
"   ..........   ",
"   .        .   ",
"   .        .   ",
"   .        .   ",
"   .        .   ",
"   .        .   ",
"   .        .   ",
"   ..........   ",
"                ",
"                ",
"                ",
"                "};

static const char * const win_minimize_xpm[] = {
"16 16 4 1",
"  s None  c None",
". c #000000",
"X c #FFFFFF",
"Y c #707070",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"    ........    ",
"    ........    ",
"                ",
"                ",
"                ",
"                "};

static const char * const win_normalize_xpm[] = {
"16 16 4 1",
"  s None  c None",
". c #000000",
"X c #FFFFFF",
"Y c #707070",
"                ",
"                ",
"     .........  ",
"     .........  ",
"     .       .  ",
"     .       .  ",
"  .........  .  ",
"  .........  .  ",
"  .       .  .  ",
"  .       ....  ",
"  .       .     ",
"  .       .     ",
"  .........     ",
"                ",
"                ",
"                "};

#endif // QT_NO_IMAGEIO_XPM


QDecorationWindows::QDecorationWindows()
    : QDecorationDefault()
{
    menu_width = 16;
    help_width = 18;
    minimize_width = 18;
    maximize_width = 18;
    close_width = 18;
}

QDecorationWindows::~QDecorationWindows()
{
}

const char **QDecorationWindows::xpmForRegion(int reg)
{
#ifndef QT_NO_IMAGEIO_XPM
    switch(reg)
    {
    case Close:
        return (const char **)win_close_xpm;
    case Help:
        return (const char **)win_help_xpm;
    case Minimize:
        return (const char **)win_minimize_xpm;
    case Maximize:
        return (const char **)win_maximize_xpm;
    case Normalize:
        return (const char **)win_normalize_xpm;
    default:
        return QDecorationDefault::xpmForRegion(reg);
    }
#endif
    return 0;
}

QRegion QDecorationWindows::region(const QWidget *widget, const QRect &rect, int type)
{
    Qt::WindowFlags flags = widget->windowFlags();
    bool hasTitle = flags & Qt::WindowTitleHint;
    bool hasSysMenu = flags & Qt::WindowSystemMenuHint;
    bool hasContextHelp = flags & Qt::WindowContextHelpButtonHint;
    bool hasMinimize = flags & Qt::WindowMinimizeButtonHint;
    bool hasMaximize = flags & Qt::WindowMaximizeButtonHint;
    int titleHeight = hasTitle ? 20 : 0;

    QRegion region;
    switch (type) {
        case Menu: {
                if (hasSysMenu) {
                    region = QRect(rect.left() + 2, rect.top() - titleHeight,
                                   menu_width, titleHeight);
                }
            }
            break;

        case Title: {
                QRect r(rect.left()
                        + (hasSysMenu ? menu_width + 4: 0),
                        rect.top() - titleHeight,
                        rect.width()
                        - (hasSysMenu ? menu_width : 0)
                        - close_width
                        - (hasMaximize ? maximize_width : 0)
                        - (hasMinimize ? minimize_width : 0)
                        - (hasContextHelp ? help_width : 0)
                        - 3,
                        titleHeight);
                if (r.width() > 0)
                    region = r;
            }
            break;
        case Help: {
                if (hasContextHelp) {
                    QRect r(rect.right()
                            - close_width
                            - (hasMaximize ? maximize_width : 0)
                            - (hasMinimize ? minimize_width : 0)
                            - help_width - 3, rect.top() - titleHeight,
                            help_width, titleHeight);
                    if (r.left() > rect.left() + titleHeight)
                        region = r;
                }
            }
            break;

        case Minimize: {
                if (hasMinimize) {
                    QRect r(rect.right() - close_width
                            - (hasMaximize ? maximize_width : 0)
                            - minimize_width - 3, rect.top() - titleHeight,
                            minimize_width, titleHeight);
                    if (r.left() > rect.left() + titleHeight)
                        region = r;
                }
            }
            break;

        case Normalize:
        case Maximize: {
                if (hasMaximize) {
                    QRect r(rect.right() - close_width - maximize_width - 3,
                            rect.top() - titleHeight, maximize_width, titleHeight);
                    if (r.left() > rect.left() + titleHeight)
                        region = r;
                }
            }
            break;

        case Close: {
                QRect r(rect.right() - close_width - 1, rect.top() - titleHeight,
                        close_width, titleHeight);
                if (r.left() > rect.left() + titleHeight)
                    region = r;
            }
            break;

        default:
            region = QDecorationDefault::region(widget, rect, type);
            break;
    }

    return region;
}

bool QDecorationWindows::paint(QPainter *painter, const QWidget *widget, int decorationRegion,
                               DecorationState state)
{
    if (decorationRegion == None)
        return false;

    const QRect titleRect = QDecoration::region(widget, Title).boundingRect();
    const QPalette pal = widget->palette();
    QRegion oldClipRegion = painter->clipRegion();

    bool paintAll = (decorationRegion == int(All));
    if ((paintAll || decorationRegion & Title && titleRect.width() > 0) && state == Normal
        && (widget->windowFlags() & Qt::WindowTitleHint) ) {
        painter->setClipRegion(oldClipRegion);
        QColor fromBrush, toBrush;
        QPen   titlePen;

        if (widget == qApp->activeWindow() || qApp->activeWindow() == qApp->activePopupWidget()) {
            fromBrush = pal.color(QPalette::Highlight);
            titlePen   = pal.color(QPalette::HighlightedText);
        } else {
            fromBrush = pal.color(QPalette::Background);
            titlePen   = pal.color(QPalette::Text);
        }
        toBrush = fromBrush.light(300);

        painter->setPen(Qt::NoPen);
        QPoint p1(titleRect.x(), titleRect.y() + titleRect.height()/2);
        QPoint p2(titleRect.right(), titleRect.y() + titleRect.height()/2);
        QLinearGradient lg(p1, p2);
        lg.setColorAt(0, fromBrush);
        lg.setColorAt(1, toBrush);
        painter->fillRect(titleRect, lg);

        painter->setPen(titlePen);
        painter->setFont(widget->font());
        painter->drawText(titleRect, Qt::AlignVCenter, widget->windowTitle());
        decorationRegion ^= Title;
    }

    return QDecorationDefault::paint(painter, widget, decorationRegion, state);
}

void QDecorationWindows::paintButton(QPainter *painter, const QWidget *widget, int buttonRegion,
                                     DecorationState state, const QPalette &pal)
{
    QBrush fromBrush, toBrush;
    QPen   titlePen;

    if (widget == qApp->activeWindow() || qApp->activeWindow() == qApp->activePopupWidget()) {
        fromBrush = pal.brush(QPalette::Highlight);
        titlePen   = pal.color(QPalette::HighlightedText);
    } else {
        fromBrush = pal.brush(QPalette::Background);
        titlePen   = pal.color(QPalette::Text);
    }
    toBrush = fromBrush.color().light(300);

    QRect brect(QDecoration::region(widget, buttonRegion).boundingRect());
    if (buttonRegion != Close && buttonRegion != Menu)
        painter->fillRect(brect, toBrush);
    else
        painter->fillRect(brect.x() - 2, brect.y(), brect.width() + 4, brect.height(),
                          buttonRegion == Menu ? fromBrush : toBrush);

    int xoff = 1;
    int yoff = 2;
    const QPixmap pm = pixmapFor(widget, buttonRegion, xoff, yoff);
    if (buttonRegion != Menu) {
        if (state & Normal) {
            qDrawWinPanel(painter, brect.x(), brect.y() + 2, brect.width(),
                          brect.height() - 4, pal, false, &pal.brush(QPalette::Background));
        } else if (state & Pressed) {
            qDrawWinPanel(painter, brect.x(), brect.y() + 2, brect.width(),
                          brect.height() - 4, pal, true, &pal.brush(QPalette::Background));
            ++xoff;
            ++yoff;
        }
    } else {
        xoff = 0;
        yoff = 2;
    }

    if (!pm.isNull())
        painter->drawPixmap(brect.x() + xoff, brect.y() + yoff, pm);
}

#endif // QT_NO_QWS_DECORATION_WINDOWS || QT_PLUGIN
