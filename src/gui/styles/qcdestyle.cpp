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

#include "qcdestyle.h"

#if !defined(QT_NO_STYLE_CDE) || defined(QT_PLUGIN)

#include "qpainter.h"
#include "qdrawutil.h"
#include "qbutton.h"
#include <limits.h>

/*!
    \class QCDEStyle
    \brief The QCDEStyle class provides a CDE look and feel.

    \ingroup appearance

    This style provides a slightly improved Motif look similar to some
    versions of the Common Desktop Environment (CDE). The main
    differences are thinner frames and more modern radio buttons and
    checkboxes. Together with a dark background and a bright
    text/foreground color, the style looks quite attractive (at least
    for Motif fans).

    Note that most of the functions provided by QCDEStyle are
    reimplementations of QStyle functions; see QStyle for their
    documentation. QCDEStyle provides overloads for drawControl() and
    drawPrimitive() which are documented here.
*/

/*!
    Constructs a QCDEStyle.

    If \a useHighlightCols is false (the default), then the style will
    polish the application's color palette to emulate the Motif way of
    highlighting, which is a simple inversion between the base and the
    text color.
*/
QCDEStyle::QCDEStyle(bool useHighlightCols)
    : QMotifStyle(useHighlightCols)
{
}

/*!
    Destroys the style.
*/
QCDEStyle::~QCDEStyle()
{
}


/*!\reimp
*/
int QCDEStyle::pixelMetric(PixelMetric metric, const QStyleOption *option,
                           const QWidget *widget) const
/*
int QCDEStyle::pixelMetric(PixelMetric metric, const QStyleOption *option,
                           const QWidget *widget) const
                           */
{
    int ret;

    switch(metric) {
    case PM_MenuFrameWidth:
    case PM_DefaultFrameWidth:
        ret = 1;
        break;
    case PM_MenuBarFrameWidth:
        ret = 1;
        break;
    case PM_ScrollBarExtent:
        ret = 13;
        break;
    default:
        ret = QMotifStyle::pixelMetric(metric, option, widget);
        break;
    }
    return ret;
}

/*!
    \reimp
*/
void QCDEStyle::drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                             const QWidget *w) const
/*
void QCDEStyle::drawControl(ControlElement element,
                            QPainter *p,
                            const QWidget *widget,
                            const QRect &r,
                            const QPalette &pal,
                            SFlags how) const
                            */
{

    switch(element) {
    case CE_MenuBarItem:
        {
            if (how & Style_Active)  // active item
                qDrawShadePanel(p, r, pal, true, 1,
                                 &pal.brush(QPalette::Button));
            else  // other item
                p->fillRect(r, pal.brush(QPalette::Button));
            QCommonStyle::drawControl(element, p, widget, r, pal, how, opt);
            break;
        }
#ifdef QT_COMPAT
    case CE_Q3MenuBarItem:
        {
            if (how & Style_Active)  // active item
                qDrawShadePanel(p, r, pal, true, 1,
                                 &pal.brush(QPalette::Button));
            else  // other item
                p->fillRect(r, pal.brush(QPalette::Button));
            QCommonStyle::drawControl(element, p, widget, r, pal, how, opt);
            break;
        }
#endif
    default:
        QMotifStyle::drawControl(element, p, widget, r, pal, how, opt);
    break;
    }


}

/*!
    \reimp
*/
void QCDEStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                        const QWidget *w) const
/*
void QCDEStyle::drawPrimitive(PrimitiveElement pe,
                              QPainter *p,
                              const QRect &r,
                              const QPalette &pal,
                              SFlags flags) const
                              */
{
    switch(pe) {
    case PE_Indicator: {
#ifndef QT_NO_BUTTON
        bool down = flags & Style_Down;
        bool on = flags & Style_On;
        bool showUp = !(down ^ on);
        QBrush fill = showUp || flags & Style_NoChange ? pal.brush(QPalette::Button) : pal.brush(QPalette::Mid);
        qDrawShadePanel(p, r, pal, !showUp, pixelMetric(PM_DefaultFrameWidth), &pal.brush(QPalette::Button));

        if (!(flags & Style_Off)) {
            QPolygon a(7 * 2);
            int i, xx, yy;
            xx = r.x() + 3;
            yy = r.y() + 5;
            for (i = 0; i < 3; i++) {
                a.setPoint(2 * i,   xx, yy);
                a.setPoint(2 * i + 1, xx, yy + 2);
                xx++; yy++;
            }
            yy -= 2;
            for (i = 3; i < 7; i++) {
                a.setPoint(2 * i, xx, yy);
                a.setPoint(2 * i + 1, xx, yy + 2);
                xx++; yy--;
            }
            if (flags & Style_NoChange)
                p->setPen(pal.dark());
            else
                p->setPen(pal.foreground());
            p->drawLineSegments(a);
        }
#endif
    }
        break;
    case PE_ExclusiveIndicator:
        {
#define INTARRLEN(x) sizeof(x)/(sizeof(int)*2)
            static const int pts1[] = {              // up left  lines
                1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
            static const int pts4[] = {              // bottom right  lines
                2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
                11,4, 10,3, 10,2 };
            static const int pts5[] = {              // inner fill
                4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };
            bool down = flags & Style_Down;
            bool on = flags & Style_On;
            QPolygon a(INTARRLEN(pts1), pts1);
            a.translate(r.x(), r.y());
            p->setPen((down || on) ? pal.dark() : pal.light());
            p->drawPolyline(a);
            a.setPoints(INTARRLEN(pts4), pts4);
            a.translate(r.x(), r.y());
            p->setPen((down || on) ? pal.light() : pal.dark());
            p->drawPolyline(a);
            a.setPoints(INTARRLEN(pts5), pts5);
            a.translate(r.x(), r.y());
            QColor fillColor = on ? pal.dark() : pal.background();
            p->setPen(fillColor);
            p->setBrush(on ? pal.brush(QPalette::Dark) :
                         pal.brush(QPalette::Background));
            p->drawPolygon(a);
            break;
        }

    case PE_ExclusiveIndicatorMask:
        {
            static const int pts1[] = {
                // up left  lines
                1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1,
                // bottom right  lines
                10,2, 10,3, 11,4, 11,7, 10,8, 10,9, 9,10, 8,10, 7,11, 4,11, 3,10, 2,10
            };
            QPolygon a(INTARRLEN(pts1), pts1);
            a.translate(r.x(), r.y());
            p->setPen(Qt::color1);
            p->setBrush(Qt::color1);
            p->drawPolygon(a);
            break;
        }
    case PE_ArrowUp:
    case PE_ArrowDown:
    case PE_ArrowRight:
    case PE_ArrowLeft: {
        QRect rect = r;
        QPolygon bFill;                          // fill polygon
        QPolygon bTop;                           // top shadow.
        QPolygon bBot;                           // bottom shadow.
        QPolygon bLeft;                          // left shadow.
        QMatrix    matrix;                         // xform matrix
        bool vertical = pe == PE_ArrowUp || pe == PE_ArrowDown;
        bool horizontal = !vertical;
        int  dim = rect.width() < rect.height() ? rect.width() : rect.height();
        int  colspec = 0x0000;                      // color specification array

        if (dim < 2)                              // too small arrow
            return;

        // adjust size and center (to fix rotation below)
        if (rect.width() >  dim) {
            rect.setX(rect.x() + ((rect.width() - dim) / 2));
            rect.setWidth(dim);
        }
        if (rect.height() > dim) {
            rect.setY(rect.y() + ((rect.height() - dim) / 2));
            rect.setHeight(dim);
        }

        if (dim > 3) {
            bFill.resize(dim & 1 ? 3 : 4);
            bTop.resize(2);
            bBot.resize(2);
            bLeft.resize(2);
            bLeft.putPoints(0, 2, 0, 0, 0, dim-1);
            bTop.putPoints(0, 2, 1, 0, dim-1, dim/2);
            bBot.putPoints(0, 2, 1, dim-1, dim-1, dim/2);

            if (dim > 6) {                        // dim>6: must fill interior
                bFill.putPoints(0, 2, 1, dim-1, 1, 1);
                if (dim & 1)                      // if size is an odd number
                    bFill.setPoint(2, dim - 2, dim / 2);
                else
                    bFill.putPoints(2, 2, dim-2, dim/2-1, dim-2, dim/2);
            }
        } else {
            if (dim == 3) {                       // 3x3 arrow pattern
                bLeft.setPoints(4, 0,0, 0,2, 1,1, 1,1);
                bTop .setPoints(2, 1,0, 1,0);
                bBot .setPoints(2, 1,2, 2,1);
            } else {                                  // 2x2 arrow pattern
                bLeft.setPoints(2, 0,0, 0,1);
                bTop .setPoints(2, 1,0, 1,0);
                bBot .setPoints(2, 1,1, 1,1);
            }
        }

        if (pe == PE_ArrowUp || pe == PE_ArrowLeft) {
            matrix.translate(rect.x(), rect.y());
            if (vertical) {
                matrix.translate(0, rect.height() - 1);
                matrix.rotate(-90);
            } else {
                matrix.translate(rect.width() - 1, rect.height() - 1);
                matrix.rotate(180);
            }
            if (flags & Style_Down)
                colspec = horizontal ? 0x2334 : 0x2343;
            else
                colspec = horizontal ? 0x1443 : 0x1434;
        } else if (pe == PE_ArrowDown || pe == PE_ArrowRight) {
            matrix.translate(rect.x(), rect.y());
            if (vertical) {
                matrix.translate(rect.width()-1, 0);
                matrix.rotate(90);
            }
            if (flags & Style_Down)
                colspec = horizontal ? 0x2443 : 0x2434;
            else
                colspec = horizontal ? 0x1334 : 0x1343;
        }

        const QColor *cols[5];
        if (flags & Style_Enabled) {
            cols[0] = 0;
            cols[1] = &pal.button().color();
            cols[2] = &pal.mid().color();
            cols[3] = &pal.light().color();
            cols[4] = &pal.dark().color();
        } else {
            cols[0] = 0;
            cols[1] = &pal.button().color();
            cols[2] = &pal.button().color();
            cols[3] = &pal.button().color();
            cols[4] = &pal.button().color();
        }

#define CMID    *cols[(colspec>>12) & 0xf]
#define CLEFT   *cols[(colspec>>8) & 0xf]
#define CTOP    *cols[(colspec>>4) & 0xf]
#define CBOT    *cols[colspec & 0xf]

        QPen     savePen   = p->pen();              // save current pen
        QBrush   saveBrush = p->brush();            // save current brush
        QMatrix wxm = p->worldMatrix();
        QPen     pen(Qt::NoPen);
        QBrush brush = pal.brush(flags & Style_Enabled ? QPalette::Button :
                                  QPalette::Mid);

        p->setPen(pen);
        p->setBrush(brush);
        p->setWorldMatrix(matrix, true);          // set transformation matrix
        p->drawPolygon(bFill);                    // fill arrow
        p->setBrush(Qt::NoBrush);                     // don't fill

        p->setPen(CLEFT);
        p->drawLineSegments(bLeft);
        p->setPen(CBOT);
        p->drawLineSegments(bBot);
        p->setPen(CTOP);
        p->drawLineSegments(bTop);

        p->setWorldMatrix(wxm);
        p->setBrush(saveBrush);                   // restore brush
        p->setPen(savePen);                       // restore pen

#undef CMID
#undef CLEFT
#undef CTOP
#undef CBOT

    }
        break;
    default:
        QMotifStyle::drawPrimitive(pe, p, r, pal, flags, opt);
    }
}

#endif
