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
#include "qdecorationkde_qws.h"

#ifndef QT_NO_QWS_DECORATION_KDE

QDecorationKDE::QDecorationKDE()
    : QDecorationDefault()
{
}

QDecorationKDE::~QDecorationKDE()
{
}

/*
    If rect is empty, no frame is added. (a hack, really)
*/
QRegion QDecorationKDE::region(const QWidget *widget, const QRect &rect, QDecoration::Region type)
{
    int titleHeight = getTitleHeight(widget);
//  int titleWidth = getTitleWidth(widget);
//  int bw = rect.isEmpty() ? 0 : BORDER_WIDTH;
    QRegion region;

    switch (type) {
        case Maximize: {
            QRect r(rect.right() - 2*titleHeight, rect.top() - titleHeight,
                            titleHeight, titleHeight);
            if (r.left() > rect.left() + titleHeight)
                    region = r;
            break;
        }
        case Minimize: {
            QRect r(rect.right() - 3*titleHeight, rect.top() - titleHeight,
                            titleHeight, titleHeight);
            if (r.left() > rect.left() + titleHeight)
                    region = r;
            break;
        }
        case Menu:
        case Close:
        case All:
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
            region = QDecorationDefault::region(widget, rect, type);
            break;
    }

    return region;
}

void QDecorationKDE::paint(QPainter *painter, const QWidget *widget)
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
    tr = QRect(titleHeight, -titleHeight,  titleWidth, titleHeight - 1);

    QRegion oldClip = painter->clipRegion();
    painter->setClipRegion(oldClip - QRegion(tr));        // reduce flicker

#ifndef QT_NO_PALETTE
    //QPalette pal = QApplication::palette();
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

void QDecorationKDE::paintButton(QPainter *painter, const QWidget *w,
                        QDecoration::Region type, int state)
{
#ifndef QT_NO_PALETTE

    //QPalette pal = QApplication::palette();
    QPalette pal = w->palette();
    pal.setCurrentColorGroup(QPalette::Active);

    QRect brect(region(w, w->rect(), type).boundingRect());

    int xoff=2;
    int yoff=2;

    const QPixmap pm=pixmapFor(w,type,state & QWSButton::On, xoff, yoff);

    {

        if ((state & QWSButton::MouseOver) && (state & QWSButton::Clicked)) {
#if !defined(QT_NO_DRAWUTIL)
            qDrawShadePanel(painter, brect.x(), brect.y(), brect.width()-1,
                            brect.height()-1, pal, true, 2,
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

#endif // QT_NO_QWS_DECORATION_KDE
