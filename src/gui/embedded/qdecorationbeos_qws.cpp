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
#include "qdecorationbeos_qws.h"

#if !defined(QT_NO_QWS_DECORATION_BEOS) || defined(QT_PLUGIN)

QDecorationBeOS::QDecorationBeOS()
    : QDecorationDefault()
{
}

QDecorationBeOS::~QDecorationBeOS()
{
}

const char **QDecorationBeOS::menuPixmap()
{
    return (const char **)0;
}

int QDecorationBeOS::getTitleWidth(const QWidget *w)
{
    int titleWidth = w->width();
    return (titleWidth > 100) ? titleWidth : 100;
}

int QDecorationBeOS::getTitleHeight(const QWidget *)
{
    return 15;
}

/*
    If rect is empty, no frame is added. (a hack, really)
*/
QRegion QDecorationBeOS::region(const QWidget *widget, const QRect &rect, QDecoration::DecorItem type)
{
//    int titleWidth = getTitleWidth(widget);
    int titleHeight = getTitleHeight(widget);
    int bw = rect.isEmpty() ? 0 : BORDER_WIDTH;
    QRegion region;

    switch (type) {
        case All: {
            int w = rect.width() + 2 * bw;
            QRect tr(rect.left() - bw,
                    rect.top() - titleHeight - bw,
                    (w < 110) ? w : 110,
                    titleHeight);
            QRect r(rect.left() - bw,
                    rect.top() - bw,
                    rect.width() + 2 * bw,
                    rect.height() + 2 * bw);
            region = r;
            region += tr;
            region -= rect;
            break;
        }
/*
        case Title: {
            int w = rect.width();
            QRect r(rect.left() + titleHeight, rect.top() - titleHeight,
                    (w < 110) ? w : 110, titleHeight);
            if (r.width() > 0)
                region = r;
            break;
        }
        case Top: {
            int w = rect.width();
            QRect r(rect.left() + CORNER_GRAB,
                    rect.top() - titleHeight - bw,
                    (w < 110) ? w : 110,
                    bw);
            if (r.width() > 0)
                region = r;
            break;
        }
*/
        case Menu:
            break;
        case Close: {
                QRect r;
                r = QRect(rect.left(), rect.top() - titleHeight - 4,
                    titleHeight, titleHeight);
                region = r;
                break;
        }
        case Maximize: {
                QRect r;
                int w = rect.width();
                r = QRect(((w < 100) ? w : 100) - 16, rect.top() - titleHeight - 4,
                    titleHeight, titleHeight);
                if (r.left() > rect.left() + titleHeight)
                    region = r;
            break;
        }
        case Minimize: {
                QRect r;
                int w = rect.width();
                r = QRect(((w < 100) ? w : 100) - 32, rect.top() - titleHeight - 4,
                    titleHeight, titleHeight);
                if (r.left() > rect.left() + titleHeight)
                    region = r;
            break;
        }
        case Title:
        case Top:
        case Left:
        case Right:
        case Bottom:
        case TopLeft:
        case TopRight:
        case BottomLeft:
        case BottomRight:
        default:
            region = QDecorationBeOS::region(widget, rect, type);
            break;
    }

    return region;
}

void QDecorationBeOS::paintItem(QPainter *painter, const QWidget *widget, DecorItem item,
                                   DecoreState state)
{
    if (item == None)
        return;

    if (item == All) {
        paintItem(painter, widget, Border, state);
        paintItem(painter, widget, Title, state);
        paintItem(painter, widget, Menu, state);
        paintItem(painter, widget, Help, state);
        paintItem(painter, widget, Minimize, state);
        paintItem(painter, widget, Maximize, state);
        paintItem(painter, widget, Normalize, state);
        paintItem(painter, widget, Close, state);
        return;
    }

    switch(item) {
    case Border:
    {
        qWarning("QDecorationBeOS::paintEvent(): Border - NYI!");
        break;
    }
    case Title:
    {
        qWarning("QDecorationBeOS::paintEvent(): Title - NYI!");
        break;
    }
    case Menu:
    {
        qWarning("QDecorationBeOS::paintEvent(): Menu - NYI!");
        break;
    }
    case Help:
    {
        qWarning("QDecorationBeOS::paintEvent(): Help - NYI!");
        break;
    }
    case Minimize:
    {
        qWarning("QDecorationBeOS::paintEvent(): Minimize - NYI!");
        break;
    }
    case Maximize:
    {
        qWarning("QDecorationBeOS::paintEvent(): Maximize - NYI!");
        break;
    }
    case Normalize:
    {
        qWarning("QDecorationBeOS::paintEvent(): Normalize - NYI!");
        break;
    }
    case Close:
    {
        qWarning("QDecorationBeOS::paintEvent(): Close - NYI!");
        break;
    }
    }
}


#if 0
void QDecorationBeOS::paint(QPainter *painter, const QWidget *widget)
{
    int titleWidth = getTitleWidth(widget);
    int titleHeight = getTitleHeight(widget);

    QRect rect(widget->rect());

    // Border rect
    QRect br(rect.left() - BORDER_WIDTH,
                rect.top() - BORDER_WIDTH - titleHeight,
                rect.width() + 2 * BORDER_WIDTH,
                rect.height() + BORDER_WIDTH + BOTTOM_BORDER_WIDTH + titleHeight);

    // title bar rect
    QRect tr;

    br = QRect();
    tr = QRect(titleHeight, -titleHeight,  titleWidth, titleHeight - 1);

    QRegion oldClip = painter->clipRegion();
    painter->setClipRegion(oldClip - QRegion(tr));        // reduce flicker

#ifndef QT_NO_PALETTE
    // const QPalette &pal = QApplication::palette();
    QPalette pal = widget->palette();
    pal.setCurrentColorGroup(QPalette::Active);

#if !defined(QT_NO_DRAWUTIL)
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

            QColor c1;
            if (widget == qApp->activeWindow()) {
                c1 = QColor(0xC0, 0xB0, 0x40);
            } else {
                c1 = QColor(0x20, 0x30, 0x50);
                titlePen = QPen(QColor(0x90, 0x90, 0x90));
            }
            painter->fillRect(rect.left() - 4, rect.top() - titleHeight - 4,
                            rect.width() + 8, rect.height() + titleHeight + 8, QBrush(c1));

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

void QDecorationBeOS::paintButton(QPainter *painter, const QWidget *w,
                        QDecoration::Region type, int state)
{
#ifndef QT_NO_PALETTE
//    const QPalette &pal = w->palette();

    QRect brect(region(w, w->rect(), type).boundingRect());

    int xoff=2;
    int yoff=2;

    const QPixmap pm=pixmapFor(w,type,state & QWSButton::On, xoff, yoff);

    xoff++;

    if ((state & QWSButton::MouseOver) && (state & QWSButton::Clicked)) {
        if (!pm.isNull()) painter->drawPixmap(brect.x()+1+xoff, brect.y()+1+yoff, pm);
    } else {
        if (!pm.isNull()) painter->drawPixmap(brect.x()+xoff, brect.y()+yoff, pm);
    }
#endif
}

#endif // 0 --------------------------------------------------------------

#endif // QT_NO_QWS_DECORATION_BEOS
