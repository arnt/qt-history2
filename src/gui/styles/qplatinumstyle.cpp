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

#include "qplatinumstyle.h"

#if !defined(QT_NO_STYLE_PLATINUM) || defined(QT_PLUGIN)

#include "qapplication.h"
#include "qcombobox.h"
#include "qdrawutil.h"
#include "qpainter.h"
#include "qpalette.h"
#include "qpixmap.h"
#include "qpushbutton.h"
#include "qscrollbar.h"
#include "qslider.h"
#include <limits.h>

/*!
    \class QPlatinumStyle
    \brief The QPlatinumStyle class provides Mac/Platinum look and feel.

    \ingroup appearance

    This class implements the Platinum look and feel. It's an
    experimental class that tries to resemble a Macintosh-like GUI
    style with the QStyle system. The emulation is currently far from
    perfect.

    Most of the functions are documented in the base classes
    \l{QWindowsStyle}, \l{QCommonStyle}, and \l{QStyle}, but the
    QPlatinumStyle overloads of drawComplexControl(), drawControl(),
    drawPrimitive(), subControlRect(), and subRect(), are
    documented here.

    \sa QMacStyle
*/


/*!
    Constructs a QPlatinumStyle
*/
QPlatinumStyle::QPlatinumStyle()
{
}

/*!
    Destroys the style.
*/
QPlatinumStyle::~QPlatinumStyle()
{
}

/*void QPlatinumStyle::drawPrimitive(PrimitiveElement pe,
                                   QPainter *p,
                                   const QRect &r,
                                   const QPalette &pal,
                                   SFlags flags,
                                   const Q3StyleOption& opt) const*/

/*!
    \reimp
*/
void QPlatinumStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                                   const QWidget *w) const
{
    switch (pe) {
    case PE_HeaderSection:
        {
            // adjust the sunken flag, otherwise headers are drawn
            // sunken...
            if (flags & State_Sunken)
                flags ^= State_Sunken;
            drawPrimitive(PE_ButtonBevel, p, r, pal, flags, opt);
            break;
        }
    case PE_ButtonTool:
        {
            // tool buttons don't change color when pushed in platinum,
            // so we need to make the mid and button color the same
            QPalette pal2 = pal;
            QBrush fill;

            // quick trick to make sure toolbuttons drawn sunken
            // when they are activated...
            if (flags & State_On)
                flags |= State_Sunken;

            fill = pal2.brush(QPalette::Button);
            pal2.setBrush(QPalette::Mid, fill);
            drawPrimitive(PE_ButtonBevel, p, r, pal2, flags, opt);
            break;
        }
    case PE_ButtonBevel:
        {
            int x,
                y,
                w,
                h;
            r.rect(&x, &y, &w, &h);

            QPen oldPen = p->pen();
            if (w * h < 1600 ||
                 qAbs(w - h) > 10) {
                // small buttons

                if (!(flags & State_Sunken)) {
                    p->fillRect(x + 2, y + 2, w - 4, h - 4,
                                 pal.brush(QPalette::Button));
                    // the bright side
                    p->setPen(pal.dark());
                    p->drawLine(x, y, x + w - 1, y);
                    p->drawLine(x, y, x, y + h - 1);

                    p->setPen(pal.light());
                    p->drawLine(x + 1, y + 1, x + w - 2, y + 1);
                    p->drawLine(x + 1, y + 1, x + 1, y + h - 2);

                    // the dark side
                    p->setPen(pal.mid());
                    p->drawLine(x + 2, y + h - 2, x + w - 2, y + h - 2);
                    p->drawLine(x + w - 2, y + 2, x + w - 2, y + h - 3);

                    p->setPen(pal.dark().color().dark());
                    p->drawLine(x + 1, y + h - 1, x + w - 1,
                                 y + h - 1);
                    p->drawLine(x + w - 1, y + 1,
                                 x + w - 1,
                                 y + h - 2);
                } else {
                    p->fillRect(x + 2, y + 2,
                                w - 4, h - 4,
                                pal.brush(QPalette::Mid));

                    // the dark side
                    p->setPen(pal.dark().color().dark());
                    p->drawLine(x, y, x + w - 1, y);
                    p->drawLine(x, y, x, y + h - 1);

                    p->setPen(pal.mid().color().dark());
                    p->drawLine(x + 1, y + 1,
                                 x + w-2, y + 1);
                    p->drawLine(x + 1, y + 1,
                                 x + 1, y + h - 2);


                    // the bright side!

                    p->setPen(pal.button());
                    p->drawLine(x + 1, y + h - 2,
                                 x + w - 2,
                                 y + h - 2);
                    p->drawLine(x + w - 2, y + 1,
                                 x + w - 2,
                                 y + h - 2);
                    p->setPen(pal.dark());
                    p->drawLine(x, y + h - 1,
                                x + w - 1,
                                y + h - 1);
                    p->drawLine(x + w - 1, y,
                                x + w - 1,
                                y + h - 1);
                }
            } else {
                // big ones
                if (!(flags & State_Sunken)) {
                    p->fillRect(x + 3, y + 3, w - 6,
                                 h - 6,
                                 pal.brush(QPalette::Button));

                    // the bright side
                    p->setPen(pal.button().color().dark());
                    p->drawLine(x, y, x + w - 1, y);
                    p->drawLine(x, y, x, y + h - 1);

                    p->setPen(pal.button());
                    p->drawLine(x + 1, y + 1,
                                 x + w - 2, y + 1);
                    p->drawLine(x + 1, y + 1,
                                 x + 1, y + h - 2);

                    p->setPen(pal.light());
                    p->drawLine(x + 2, y + 2,
                                 x + 2, y + h - 2);
                    p->drawLine(x + 2, y + 2,
                                 x + w - 2, y + 2);
                    // the dark side!

                    p->setPen(pal.mid());
                    p->drawLine(x + 3, y + h - 3,
                                 x + w - 3,
                                 y + h - 3);
                    p->drawLine(x + w - 3, y + 3,
                                 x + w - 3,
                                 y + h - 3);
                    p->setPen(pal.dark());
                    p->drawLine(x + 2, y + h - 2,
                                 x + w - 2,
                                 y + h - 2);
                    p->drawLine(x + w - 2, y + 2,
                                 x + w - 2,
                                 y + h - 2);

                    p->setPen(pal.dark().color().dark());
                    p->drawLine(x + 1, y + h - 1,
                                 x + w - 1,
                                 y + h - 1);
                    p->drawLine(x + w - 1, y + 1,
                                 x + w - 1,
                                 y + h - 1);
                } else {
                    p->fillRect(x + 3, y + 3, w - 6,
                                 h - 6,
                                 pal.brush(QPalette::Mid));

                    // the dark side
                    p->setPen(pal.dark().color().dark().dark());
                    p->drawLine(x, y, x + w - 1, y);
                    p->drawLine(x, y, x, y + h - 1);

                    p->setPen(pal.dark().color().dark());
                    p->drawLine(x + 1, y + 1,
                                 x + w - 2, y + 1);
                    p->drawLine(x + 1, y + 1,
                                 x + 1, y + h - 2);

                    p->setPen(pal.mid().color().dark());
                    p->drawLine(x + 2, y + 2,
                                 x + 2, y + w - 2);
                    p->drawLine(x + 2, y + 2,
                                 x + w - 2, y + 2);


                    // the bright side!

                    p->setPen(pal.button());
                    p->drawLine(x + 2, y + h - 3,
                                 x + w - 3,
                                 y + h - 3);
                    p->drawLine(x + w - 3, y + 3,
                                 x + w - 3,
                                 y + h - 3);

                    p->setPen(pal.midlight());
                    p->drawLine(x + 1, y + h - 2,
                                 x + w - 2,
                                 y + h - 2);
                    p->drawLine(x + w - 2, y + 1,
                                 x + w - 2,
                                 y + h - 2);

                    p->setPen(pal.dark());
                    p->drawLine(x, y + h - 1,
                                 x + w - 1,
                                 y + h - 1);
                    p->drawLine(x + w - 1, y,
                                 x + w - 1,
                                 y + h - 1);


                    // corners
                    p->setPen(mixedColor(pal.dark().color().dark().dark(),
                                          pal.dark()));
                    p->drawPoint(x, y + h - 1);
                    p->drawPoint(x + w - 1, y);

                    p->setPen(mixedColor(pal.dark().color().dark(), pal.midlight()));
                    p->drawPoint(x + 1, y + h - 2);
                    p->drawPoint(x + w - 2, y + 1);

                    p->setPen(mixedColor(pal.mid().color().dark(), pal.button()));
                    p->drawPoint(x + 2, y + h - 3);
                    p->drawPoint(x + w - 3, y + 2);
                }
            }
            p->setPen(oldPen);
            break;
        }
    case PE_ButtonCommand:
        {
            QPen oldPen = p->pen();
            int x,
                y,
                w,
                h;
            r.rect(&x, &y, &w, &h);

            if (!(flags & (State_Sunken | State_On))) {
                p->fillRect(x+3, y+3, w-6, h-6,
                             pal.brush(QPalette::Button));
                // the bright side
                p->setPen(pal.shadow());
                p->drawLine(x, y, x+w-1, y);
                p->drawLine(x, y, x, y + h - 1);

                p->setPen(pal.button());
                p->drawLine(x + 1, y + 1, x + w - 2, y + 1);
                p->drawLine(x + 1, y + 1, x + 1, y + h - 2);

                p->setPen(pal.light());
                p->drawLine(x + 2, y + 2, x + 2, y + h - 2);
                p->drawLine(x + 2, y + 2, x + w - 2, y + 2);


                // the dark side!

                p->setPen(pal.mid());
                p->drawLine(x + 3, y + h - 3 ,x + w - 3, y + h - 3);
                p->drawLine(x + w - 3, y + 3, x + w - 3, y + h - 3);

                p->setPen(pal.dark());
                p->drawLine(x + 2, y + h - 2, x + w - 2, y + h - 2);
                p->drawLine(x + w - 2, y + 2, x + w - 2, y + h - 2);

                p->setPen(pal.shadow());
                p->drawLine(x + 1, y + h - 1, x + w - 1, y + h - 1);
                p->drawLine(x + w - 1, y, x + w - 1, y + h - 1);


                // top left corner:
                p->setPen(pal.background());
                p->drawPoint(x, y);
                p->drawPoint(x + 1, y);
                p->drawPoint(x, y+1);
                p->setPen(pal.shadow());
                p->drawPoint(x + 1, y + 1);
                p->setPen(pal.button());
                p->drawPoint(x + 2, y + 2);
                p->setPen(white);
                p->drawPoint(x + 3, y + 3);
                // bottom left corner:
                p->setPen(pal.background());
                p->drawPoint(x, y + h - 1);
                p->drawPoint(x + 1, y + h - 1);
                p->drawPoint(x, y + h - 2);
                p->setPen(pal.shadow());
                p->drawPoint(x + 1, y + h - 2);
                p->setPen(pal.dark());
                p->drawPoint(x + 2, y + h - 3);
                // top right corner:
                p->setPen(pal.background());
                p->drawPoint(x + w -1, y);
                p->drawPoint(x + w - 2, y);
                p->drawPoint(x + w - 1, y + 1);
                p->setPen(pal.shadow());
                p->drawPoint(x + w - 2, y + 1);
                p->setPen(pal.dark());
                p->drawPoint(x + w - 3, y + 2);
                // bottom right corner:
                p->setPen(pal.background());
                p->drawPoint(x + w - 1, y + h - 1);
                p->drawPoint(x + w - 2, y + h - 1);
                p->drawPoint(x + w - 1, y + h - 2);
                p->setPen(pal.shadow());
                p->drawPoint(x + w - 2, y + h - 2);
                p->setPen(pal.dark());
                p->drawPoint(x + w - 3, y + h - 3);
                p->setPen(pal.mid());
                p->drawPoint(x + w - 4, y + h - 4);

            } else {
                p->fillRect(x + 2, y + 2, w - 4, h - 4,
                             pal.brush(QPalette::Dark));

                // the dark side
                p->setPen(pal.shadow());
                p->drawLine(x, y, x + w - 1, y);
                p->drawLine(x, y, x, y + h - 1);

                p->setPen(pal.dark().color().dark());
                p->drawLine(x + 1, y + 1, x + w - 2, y + 1);
                p->drawLine(x + 1, y + 1, x + 1, y + h - 2);

                // the bright side!

                p->setPen(pal.button());
                p->drawLine(x + 1, y + h - 2, x + w - 2, y + h - 2);
                p->drawLine(x + w - 2, y + 1, x + w - 2, y + h - 2);

                p->setPen(pal.dark());
                p->drawLine(x, y + h - 1, x + w - 1, y + h - 1);
                p->drawLine(x + w - 1, y, x + w - 1, y + h - 1);

                // top left corner:
                p->setPen(pal.background());
                p->drawPoint(x, y);
                p->drawPoint(x + 1, y);
                p->drawPoint(x, y + 1);
                p->setPen(pal.shadow());
                p->drawPoint(x + 1, y + 1);
                p->setPen(pal.dark().color().dark());
                p->drawPoint(x + 3, y + 3);
                // bottom left corner:
                p->setPen(pal.background());
                p->drawPoint(x, y + h - 1);
                p->drawPoint(x + 1, y + h - 1);
                p->drawPoint(x, y + h - 2);
                p->setPen(pal.shadow());
                p->drawPoint(x + 1, y + h - 2);
                // top right corner:
                p->setPen(pal.background());
                p->drawPoint(x + w - 1, y);
                p->drawPoint(x + w - 2, y);
                p->drawPoint(x + w - 1, y + 1);
                p->setPen(pal.shadow());
                p->drawPoint(x + w - 2, y + 1);
                // bottom right corner:
                p->setPen(pal.background());
                p->drawPoint(x + w - 1, y + h - 1);
                p->drawPoint(x + w - 2, y + h - 1);
                p->drawPoint(x + w - 1, y + h - 2);
                p->setPen(pal.shadow());
                p->drawPoint(x + w - 2, y + h - 2);
                p->setPen(pal.dark());
                p->drawPoint(x + w - 3, y + h - 3);
                p->setPen(pal.mid());
                p->drawPoint(x + w - 4, y + h - 4);
            }
            p->setPen(oldPen);
            break;
        }
    case PE_Indicator:
        {
            drawPrimitive(PE_ButtonBevel, p, QRect(r.x(), r.y(),
                                                    r.width() - 2, r.height()),
                           pal, flags);
            p->fillRect(r.x() + r.width() - 2, r.y(), 2, r.height(),
                         pal.brush(QPalette::Background));
            p->setPen(pal.shadow());
            p->drawRect(r.x(), r.y(), r.width() - 2, r.height());

            static const int nochange_mark[] = { 3,5, 9,5,  3,6, 9,6 };
            static const int check_mark[] = {
                3,5, 5,5,  4,6, 5,6,  5,7, 6,7,  5,8, 6,8,      6,9, 9,9,
                6,10, 8,10, 7,11, 8,11,  7,12, 7,12,  8,8, 9,8,  8,7, 10,7,
                9,6, 10,6, 9,5, 11,5,  10,4, 11,4,  10,3, 12,3,
                11,2, 12,2, 11,1, 13,1,  12,0, 13,0 };
            if (!(flags & State_Off)) {
                QPen oldPen = p->pen();
                int x1 = r.x();
                int y1 = r.y();
                if (flags & State_Sunken) {
                    x1++;
                    y1++;
                }
                QPolygon amark;
                if (flags & State_On) {
                    amark = QPolygon(sizeof(check_mark)/(sizeof(int)*2),
                                         check_mark);
                    // ### KLUDGE!!
                    flags ^= State_On;
                    flags ^= State_Sunken;
                } else if (flags & State_NoChange) {
                    amark = QPolygon(sizeof(nochange_mark)
                                         / (sizeof(int) * 2),
                                         nochange_mark);
                }

                amark.translate(x1 + 1, y1 + 1);
                p->setPen(pal.dark());
                p->drawLineSegments(amark);
                amark.translate(-1, -1);
                p->setPen(pal.foreground());
                p->drawLineSegments(amark);
                p->setPen(oldPen);
            }
            break;
        }
    case PE_IndicatorMask:
        {
            int x,
                y,
                w,
                h;
            r.rect(&x, &y, &w, &h);
            p->fillRect(x, y, w - 2, h, Qt::color1);
            if (flags & State_Off) {
                QPen oldPen = p->pen();
                p->setPen (QPen(Qt::color1, 2));
                p->drawLine(x + 2, y + h / 2 - 1,
                             x + w / 2 - 1, y + h - 4);
                p->drawLine(x + w / 2 - 1, y + h - 4,
                             x + w, 0);
                p->setPen(oldPen);
            }
            break;
        }
    case PE_ExclusiveIndicator:
        {
#define INTARRLEN(x) sizeof(x) / (sizeof(int) * 2)
            bool down = flags & State_Sunken;
            bool on = flags & State_On;

            static const int pts1[] = {                // normal circle
                5,0, 8,0, 9,1, 10,1, 11,2, 12,3, 12,4, 13,5,
                13,8, 12,9, 12,10, 11,11, 10,12, 9,12, 8,13,
                5,13, 4,12, 3,12, 2,11, 1,10, 1,9, 0,8, 0,5,
                1,4, 1,3, 2,2, 3,1, 4,1 };
            static const int pts2[] = {                // top left shadow
                5,1, 8,1, 3,2, 7,2, 2,3, 5,3,  2,4, 4,4,
                1,5, 3,5, 1,6, 1,8, 2,6, 2,7 };
            static const int pts3[] = {                // bottom right, dark
                5,12, 8,12, 7,11, 10,11, 8,10, 11,10,
                9,9, 11,9, 10,8, 12,8, 11,7, 11,7,
                12,5, 12,7 };
            static const int pts4[] = {                // bottom right, light
                5,12, 8,12, 7,11, 10,11, 9,10, 11,10,
                10,9, 11,9, 11,7, 11,8, 12,5, 12,8 };
            static const int pts5[] = {                // check mark
                6,4, 8,4, 10,6, 10,8, 8,10, 6,10, 4,8, 4,6 };
            static const int pts6[] = {                // check mark extras
                4,5, 5,4, 9,4, 10,5, 10,9, 9,10, 5,10, 4,9 };
            int x, y;
            x = r.x();
            y = r.y();
            p->setBrush((down||on) ? pal.brush(QPalette::Dark)
                         : pal.brush(QPalette::Button));
            p->setPen(Qt::NoPen);
            p->drawEllipse(x, y, 13, 13);
            p->setPen(pal.shadow());
            QPolygon a(INTARRLEN(pts1), pts1);
            a.translate(x, y);
            p->drawPolyline(a);        // draw normal circle
            QColor tc, bc;
            const int *bp;
            int        bl;
            if (down || on) {                        // pressed down or on
                tc = pal.dark().color().dark();
                bc = pal.light();
                bp = pts4;
                bl = INTARRLEN(pts4);
            } else {                                        // released
                tc = pal.light();
                bc = pal.dark();
                bp = pts3;
                bl = INTARRLEN(pts3);
            }
            p->setPen(tc);
            a.setPoints(INTARRLEN(pts2), pts2);
            a.translate(x, y);
            p->drawLineSegments(a);                // draw top shadow
            p->setPen(bc);
            a.setPoints(bl, bp);
            a.translate(x, y);
            p->drawLineSegments(a);
            if (on) {                                // draw check mark
                int x1 = x,
                    y1 = y;
                p->setBrush(pal.foreground());
                p->setPen(pal.foreground());
                a.setPoints(INTARRLEN(pts5), pts5);
                a.translate(x1, y1);
                p->drawPolygon(a);
                p->setBrush(Qt::NoBrush);
                p->setPen(pal.dark());
                a.setPoints(INTARRLEN(pts6), pts6);
                a.translate(x1, y1);
                p->drawLineSegments(a);
            }
            break;
        }

    case PE_ExclusiveIndicatorMask:
        {
            static const int pts1[] = {                // normal circle
                5,0, 8,0, 9,1, 10,1, 11,2, 12,3, 12,4, 13,5,
                13,8, 12,9, 12,10, 11,11, 10,12, 9,12, 8,13,
                5,13, 4,12, 3,12, 2,11, 1,10, 1,9, 0,8, 0,5,
                1,4, 1,3, 2,2, 3,1, 4,1 };
            QPolygon a(INTARRLEN(pts1), pts1);
            a.translate(r.x(), r.y());
            p->setPen(Qt::color1);
            p->setBrush(Qt::color1);
            p->drawPolygon(a);
            break;
        }
    case PE_ScrollBarAddLine:
        {
            drawPrimitive(PE_ButtonBevel, p, r, pal,
                           (flags & State_Enabled) | ((flags & State_Sunken)
                                                      ? State_Sunken
                                                      : State_Raised));
            p->setPen(pal.shadow());
            p->drawRect(r);
            drawPrimitive(((flags & State_Horizontal) ? PE_ArrowRight
                            : PE_ArrowDown), p, QRect(r.x() + 2,
                                                      r.y() + 2,
                                                      r.width() - 4,
                                                      r.height() - 4),
                           pal, flags);
            break;
        }
    case PE_ScrollBarSubLine:
        {
            drawPrimitive(PE_ButtonBevel, p, r, pal,
                           (flags & State_Enabled) | ((flags & State_Sunken)
                                                      ? State_Sunken
                                                      : State_Raised));
            p->setPen(pal.shadow());
            p->drawRect(r);
            drawPrimitive(((flags & State_Horizontal) ? PE_ArrowLeft
                            : PE_ArrowUp), p, QRect(r.x() + 2,
                                                     r.y() + 2,
                                                     r.width() - 4,
                                                     r.height() - 4),
                           pal, flags);
            break;
        }
    case PE_ScrollBarAddPage:
    case PE_ScrollBarSubPage:
        {
            QPen oldPen = p->pen();
            if (r.width() < 3 || r.height() < 3) {
                p->fillRect(r, pal.brush(QPalette::Mid));
                p->setPen(pal.shadow());
                p->drawRect(r);
                p->setPen(oldPen);
            } else {
                int x,
                    y,
                    w,
                    h;
                r.rect(&x, &y, &w, &h);
                if (flags & State_Horizontal) {
                    p->fillRect(x + 2, y + 2, w - 2,
                                 h - 4,
                                 pal.brush(QPalette::Mid));
                    // the dark side
                    p->setPen(pal.dark().color().dark());
                    p->drawLine(x, y, x + w - 1, y);
                    p->setPen(pal.shadow());
                    p->drawLine(x, y, x, y + h - 1);

                    p->setPen(pal.mid().color().dark());
                    p->drawLine(x + 1, y + 1, x + w - 1,
                                 y + 1);
                    p->drawLine(x + 1, y + 1, x + 1,
                                 y + h - 2);

                    // the bright side!

                    p->setPen(pal.button());
                    p->drawLine(x + 1, y + h - 2,
                                 x + w - 1,
                                 y + h - 2);
                    p->setPen(pal.shadow());
                    p->drawLine(x, y + h - 1,
                                 x + w - 1,
                                 y + h - 1);

                } else {
                    p->fillRect(x + 2, y + 2, w - 4,
                                 h - 2,
                                 pal.brush(QPalette::Mid));

                    // the dark side
                    p->setPen(pal.dark().color().dark());
                    p->drawLine(x, y, x + w - 1, y);
                    p->setPen(pal.shadow());
                    p->drawLine(x, y, x, y + h - 1);

                    p->setPen(pal.mid().color().dark());
                    p->drawLine(x + 1, y + 1, x + w - 2,
                                 y + 1);
                    p->drawLine(x + 1, y + 1, x + 1,
                                 y + h - 1);

                    // the bright side!
                    p->setPen(pal.button());
                    p->drawLine(x + w - 2, y + 1,
                                 x + w - 2,
                                 y + h - 1);

                    p->setPen(pal.shadow());
                    p->drawLine(x + w - 1, y,
                                 x + w - 1,
                                 y + h - 1);

                }
            }
            p->setPen(oldPen);
            break;
        }
    case PE_ScrollBarSlider:
        {
            QPoint bo = p->brushOrigin();
            p->setBrushOrigin(r.topLeft());
            drawPrimitive(PE_ButtonBevel, p, r, pal, State_Raised);
            p->setBrushOrigin(bo);
            drawRiffles(p, r.x(), r.y(), r.width(), r.height(), pal,
                         flags & State_Horizontal);
            p->setPen(pal.shadow());
            p->drawRect(r);
            if (flags & State_HasFocus) {
                drawPrimitive(PE_FocusRect, p, QRect(r.x() + 2, r.y() + 2,
                                                      r.width() - 5,
                                                      r.height() - 5),
                               pal, flags);
            }
            break;
        }
    default:
        QWindowsStyle::drawPrimitive(pe, p, r, pal, flags, opt);
        break;
    }

}

/*!
    \reimp
*/
void QPlatinumStyle::drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                                 const QWidget *w = 0) const
/*
void QPlatinumStyle::drawControl(ControlElement element,
                                 QPainter *p,
                                 const QWidget *widget,
                                 const QRect &r,
                                 const QPalette &pal,
                                 SFlags how,
                                 const Q3StyleOption& opt) const
                                 */
{
    switch(element) {
    case CE_PushButton:
        {
#ifndef QT_NO_PUSHBUTTON
            QPalette pal2 = pal;
            const QPushButton *btn;
            int x1, y1, x2, y2;
            bool useBevelButton;
            SFlags flags;
            flags = State_Default;
            btn = (const QPushButton*)widget;

            // take care of the flags based on what we know...
            if (btn->isDown())
                flags |= State_Sunken;
            if (btn->isChecked())
                flags |= State_On;
            if (btn->isEnabled())
                flags |= State_Enabled;
            if (btn->isDefault())
                flags |= State_Default;
            if (! btn->isFlat() && !(flags & State_Sunken))
                flags |= State_Raised;

            r.coords(&x1, &y1, &x2, &y2);

            p->setPen(pal.foreground());
            p->setBrush(QBrush(pal.button(), Qt::NoBrush));

            QBrush fill;
            if (btn->isDown()) {
                fill = pal.brush(QPalette::Dark);
                // this could be done differently, but this
                // makes a down Bezel drawn correctly...
                pal2.setBrush(QPalette::Mid, fill);
            } else if (btn->isChecked()) {
                fill = QBrush(pal.mid(), Qt::Dense4Pattern);
                pal2.setBrush(QPalette::Mid, fill);
            }
            // to quote the old QPlatinumStlye drawPushButton...
            // small or square image buttons as well as toggle buttons are
            // bevel buttons (what a heuristic....)
            if (btn->isCheckable() || (btn->pixmap() &&
                      (btn->width() * btn->height() < 1600 ||
                       qAbs(btn->width() - btn->height()) < 10)))
                useBevelButton = true;
            else
                useBevelButton = false;

            int diw = pixelMetric(PM_ButtonDefaultIndicator, widget);
            if (btn->isDefault()) {
                x1 += 1;
                y1 += 1;
                x2 -= 1;
                y2 -= 1;
                QPalette buttonPal(pal2);
                buttonPal.setColor(QPalette::Button, pal.mid());

                SFlags myFlags = flags;
                // don't draw the default button sunken, unless it's necessary.
                if (myFlags & State_Sunken)
                    myFlags ^= State_Sunken;
                if (useBevelButton) {
                    drawPrimitive(PE_ButtonBevel, p, QRect(x1, y1,
                                                             x2 - x1 + 1,
                                                             y2 - y1 + 1),
                                   pal2, myFlags, opt);
                } else {
                    drawPrimitive(PE_ButtonCommand, p, QRect(x1, y1,
                                                               x2 - x1 + 1,
                                                               y2 - y1 + 1),
                                   buttonPal, myFlags, opt);
                }
            }

            if (btn->isDefault() || btn->autoDefault()) {
                x1 += diw;
                y1 += diw;
                x2 -= diw;
                y2 -= diw;
            }

            if (!btn->isFlat() || btn->isChecked() || btn->isDown()) {
                if (useBevelButton) {
                    // fix for toggle buttons...
                    if (flags & (State_Sunken | State_On))
                        flags |= State_Sunken;
                    drawPrimitive(PE_ButtonBevel, p, QRect(x1, y1,
                                                             x2 - x1 + 1,
                                                             y2 - y1 + 1),
                                   pal2, flags, opt);
                } else {

                    drawPrimitive(PE_ButtonCommand, p, QRect(x1, y1,
                                                               x2 - x1 + 1,
                                                               y2 - y1 + 1),
                                   pal2, flags, opt);
                }
            }


            if (p->brush().style() != Qt::NoBrush)
                p->setBrush(Qt::NoBrush);
            break;
#endif
        }
    case CE_PushButtonLabel:
        {
#ifndef QT_NO_PUSHBUTTON
            const QPushButton *btn;
            bool on;
            int x, y, w, h;
            SFlags flags;
            flags = State_Default;
            btn = (const QPushButton*)widget;
            on = btn->isDown() || btn->isChecked();
            r.rect(&x, &y, &w, &h);
            if (btn->menu()) {
                int dx = pixelMetric(PM_MenuButtonIndicator, widget);

                int xx = x + w - dx - 4;
                int yy = y - 3;
                int hh = h + 6;

                if (!on) {
                    p->setPen(pal.mid());
                    p->drawLine(xx, yy + 2, xx, yy + hh - 3);
                    p->setPen(pal.button());
                    p->drawLine(xx + 1, yy + 1, xx + 1, yy + hh - 2);
                    p->setPen(pal.light());
                    p->drawLine(xx + 2, yy + 2, xx + 2, yy + hh - 2);
                }
                if (btn->isEnabled())
                    flags |= State_Enabled;
                drawPrimitive(PE_ArrowDown, p, QRect(x + w - dx - 1, y + 2,
                                                      dx, h - 4),
                               pal, flags, opt);
                w -= dx;
            }
#ifndef QT_NO_ICON
            if (!btn->icon().isNull()) {
                QIcon::Mode mode = btn->isEnabled()
                                   ? QIcon::Normal : QIcon::Disabled;
                if (mode == QIcon::Normal && btn->hasFocus())
                    mode = QIcon::Active;
                QIcon::State state = QIcon::Off;
                if (btn->isCheckable() && btn->isChecked())
                    state = QIcon::On;
                QPixmap pixmap = btn->icon().pixmap(Qt::SmallIconSize, mode, state);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                p->drawPixmap(x + 2, y + h / 2 - pixh / 2, pixmap);
                x += pixw + 4;
                w -= pixw + 4;
            }
#endif
            drawItemText(p, QRect(x, y, w, h),
                      Qt::AlignCenter | Qt::TextShowMnemonic,
                      btn->palette(), btn->isEnabled(),
                      btn->text(), -1,
                      on ? &btn->palette().brightText().color()
                      : &btn->palette().buttonText().color());
            if (btn->hasFocus())
                drawPrimitive(PE_FocusRect, p,
                               subRect(SR_PushButtonFocusRect, widget),
                               pal, flags);
            break;
#endif
        }
    default:
        QWindowsStyle::drawControl(element, p, widget, r, pal, how, opt);
        break;
    }
}

/*!
    \reimp
*/
void QPlatinumStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                        QPainter *p, const QWidget *widget) const
/*
void QPlatinumStyle::drawComplexControl(ComplexControl control,
                                        QPainter *p,
                                        const QWidget *widget,
                                        const QRect &r,
                                        const QPalette &pal,
                                        SFlags how,
                                        SCFlags sub,
                                        SCFlags subActive,
                                        const Q3StyleOption& opt) const
                                        */
{
    switch (control) {
    case CC_ComboBox:
        {
            int x,
                y,
                w,
                h;
            r.rect(&x, &y, &w, &h);
            p->fillRect(x + 2,  y + 2, w - 4,
                         h - 4, pal.brush(QPalette::Button));
            // the bright side
            p->setPen(pal.shadow());
            p->drawLine(x, y, x + w - 1, y);
            p->drawLine(x, y, x, y + h - 1);

            p->setPen(pal.light());
            p->drawLine(x + 1, y + 1,
                         x + w - 2, y + 1);
            p->drawLine(x + 1, y + 1,
                         x + 1, y + h - 2);

            // the dark side!


            p->setPen(pal.mid());
            p->drawLine(x + 2, y + h - 2,
                         x + w - 2, y + h - 2);
            p->drawLine(x + w - 2, y + 2,
                         x + w - 2, y + h - 2);

            p->setPen (pal.shadow());
            p->drawLine(x + 1, y + h - 1,
                         x + w - 1, y + h - 1);
            p->drawLine(x + w - 1, y,
                         x + w - 1, y + h - 1);

            // top left corner:
            p->setPen(pal.background());
            p->drawPoint(x, y);
            p->drawPoint(x + 1, y);
            p->drawPoint(x, y + 1);
            p->setPen(pal.shadow());
            p->drawPoint(x + 1, y + 1);
            p->setPen(white);
            p->drawPoint(x + 3, y + 3);
            // bottom left corner:
            p->setPen(pal.background());
            p->drawPoint(x, y + h - 1);
            p->drawPoint(x + 1, y + h - 1);
            p->drawPoint(x, y + h - 2);
            p->setPen(pal.shadow());
            p->drawPoint(x + 1, y + h - 2);
            // top right corner:
            p->setPen(pal.background());
            p->drawPoint(x + w - 1, y);
            p->drawPoint(x + w - 2, y);
            p->drawPoint(x + w - 1, y + 1);
            p->setPen(pal.shadow());
            p->drawPoint(x + w - 2, y + 1);
            // bottom right corner:
            p->setPen(pal.background());
            p->drawPoint(x + w - 1, y + h - 1);
            p->drawPoint(x + w - 2, y + h - 1);
            p->drawPoint(x + w - 1, y + h - 2);
            p->setPen(pal.shadow());
            p->drawPoint(x + w - 2, y + h - 2);
            p->setPen(pal.dark());
            p->drawPoint(x + w - 3, y + h - 3);

            if (sub & SC_ComboBoxArrow) {
                QRect rTmp = subControlRect(CC_ComboBox, widget,
                                                     SC_ComboBoxArrow, opt);
                int xx = rTmp.x(),
                    yy = rTmp.y(),
                    ww = rTmp.width(),
                    hh = rTmp.height();
                // the bright side

                p->setPen(pal.mid());
                p->drawLine(xx, yy+2, xx, yy+hh-3);

                p->setPen(pal.button());
                p->drawLine(xx+1, yy+1, xx+ww-2, yy+1);
                p->drawLine(xx+1, yy+1, xx+1, yy+hh-2);

                p->setPen(pal.light());
                p->drawLine(xx+2, yy+2, xx+2, yy+hh-2);
                p->drawLine(xx+2, yy+2, xx+ww-2, yy+2);


                // the dark side!

                p->setPen(pal.mid());
                p->drawLine(xx+3, yy+hh-3 ,xx+ww-3, yy+hh-3);
                p->drawLine(xx+ww-3, yy+3, xx+ww-3, yy+hh-3);

                p->setPen(pal.dark());
                p->drawLine(xx+2, yy+hh-2 ,xx+ww-2, yy+hh-2);
                p->drawLine(xx+ww-2, yy+2, xx+ww-2, yy+hh-2);

                p->setPen(pal.shadow());
                p->drawLine(xx+1, yy+hh-1,xx+ww-1, yy+hh-1);
                p->drawLine(xx+ww-1, yy, xx+ww-1, yy+hh-1);

                // top right corner:
                p->setPen(pal.background());
                p->drawPoint(xx + ww - 1, yy);
                p->drawPoint(xx + ww - 2, yy);
                p->drawPoint(xx + ww - 1, yy + 1);
                p->setPen(pal.shadow());
                p->drawPoint(xx + ww - 2, yy + 1);
                // bottom right corner:
                p->setPen(pal.background());
                p->drawPoint(xx + ww - 1, yy + hh - 1);
                p->drawPoint(xx + ww - 2, yy + hh - 1);
                p->drawPoint(xx + ww - 1, yy + hh - 2);
                p->setPen(pal.shadow());
                p->drawPoint(xx + ww - 2, yy + hh - 2);
                p->setPen(pal.dark());
                p->drawPoint(xx + ww - 3, yy + hh - 3);
                p->setPen(pal.mid());
                p->drawPoint(xx + ww - 4, yy + hh - 4);

                // and the arrows
                p->setPen(pal.foreground());
                QPolygon a ;
                a.setPoints( 7, -3,1, 3,1, -2,0, 2,0, -1,-1, 1,-1, 0,-2 );
                a.translate(xx + ww / 2, yy + hh / 2 - 3);
                p->drawLineSegments(a, 0, 3);                // draw arrow
                p->drawPoint(a[6]);
                a.setPoints(7, -3,-1, 3,-1, -2,0, 2,0, -1,1, 1,1, 0,2);
                a.translate(xx + ww / 2, yy + hh / 2 + 2);
                p->drawLineSegments(a, 0, 3);                // draw arrow
                p->drawPoint(a[6]);

            }
#ifndef QT_NO_COMBOBOX
            if (sub & SC_ComboBoxEditField) {
                const QComboBox *cmb;
                cmb = (const QComboBox*)widget;
                // sadly this is pretty much the windows code, except
                // for the first fillRect call...
                QRect re =
                    QStyle::visualRect(subControlRect(CC_ComboBox,
                                                                widget,
                                                                SC_ComboBoxEditField),
                                        widget);
                if (cmb->hasFocus() && !cmb->editable())
                    p->fillRect(re.x() + 1, re.y() + 1,
                                 re.width() - 2, re.height() - 2,
                                 pal.brush(QPalette::Highlight));

                if (cmb->hasFocus()) {
                    p->setPen(pal.highlightedText());
                    p->setBackground(pal.highlight());
                } else {
                    p->setPen(pal.text());
                    p->setBackground(pal.background());
                }

                if (cmb->hasFocus() && !cmb->editable()) {
                    QRect re =
                        QStyle::visualRect(subRect(SR_ComboBoxFocusRect,
                                                     cmb),
                                            widget);
                    drawPrimitive(PE_FocusRect, p, re, pal,
                                   State_FocusAtBorder,
                                   Q3StyleOption(pal.highlight()));
                }
                if (cmb->editable()) {
                    // need this for the moment...
                    // was the code in comboButton rect
                    QRect ir(x + 3, y + 3,
                              w - 6 - 16, h - 6);
                    if (QApplication::isRightToLeft())
                        ir.translate(16, 0);
                    // end comboButtonRect...
                    ir.setRect(ir.left() - 1, ir.top() - 1, ir.width() + 2,
                                ir.height() + 2);
                    qDrawShadePanel(p, ir, pal, true, 2, 0);
                }
            }
#endif
            break;
        }
    case CC_Slider:
        {
#ifndef QT_NO_SLIDER
            const QSlider *slider = (const QSlider *) widget;
            int thickness = pixelMetric(PM_SliderControlThickness, widget);
            int len = pixelMetric(PM_SliderLength, widget);
            int ticks = slider->tickmarks();

            QRect groove = subControlRect(CC_Slider, widget, SC_SliderGroove,
                                                  opt),
                  handle = subControlRect(CC_Slider, widget, SC_SliderHandle,
                                                  opt);

            if ((sub & SC_SliderGroove) && groove.isValid()) {
                p->fillRect(groove, pal.brush(QPalette::Background));

                int x, y, w, h;
                int mid = thickness / 2;

                if (ticks & QSlider::Above)
                    mid += len / 8;
                if (ticks & QSlider::Below)
                    mid -= len / 8;

                if (slider->orientation() == Qt::Horizontal) {
                    x = 0;
                    y = groove.y() + mid - 3;
                    w = slider->width();
                    h = 7;
                } else {
                    x = groove.x() + mid - 3;
                    y = 0;
                    w = 7;
                    h = slider->height();
                }

                p->fillRect(x, y, w, h, pal.brush(QPalette::Dark));
                // the dark side
                p->setPen(pal.dark());
                p->drawLine(x, y, x + w - 1, y);
                p->drawLine(x, y, x, y + h - 1);
                p->setPen(pal.shadow());
                p->drawLine(x + 1, y + 1, x + w - 2, y + 1);
                p->drawLine(x + 1, y + 1, x + 1, y + h - 2);
                // the bright side!
                p->setPen(pal.shadow());
                p->drawLine(x + 1,  y + h - 2, x + w - 2,  y + h - 2);
                p->drawLine(x + w - 2, y + 1, x + w - 2, y + h - 2);
                p->setPen(pal.light());
                p->drawLine(x, y + h - 1, x + w - 1, y + h - 1);
                p->drawLine(x + w - 1, y, x + w - 1, y + h - 1);
                // top left corner:
                p->setPen(pal.background());
                p->drawPoint(x, y);
                p->drawPoint(x + 1, y);
                p->drawPoint(x, y + 1);
                p->setPen(pal.shadow());
                p->drawPoint(x + 1, y + 1);
                // bottom left corner:
                p->setPen(pal.background());
                p->drawPoint(x, y + h - 1);
                p->drawPoint(x + 1, y + h - 1);
                p->drawPoint(x, y + h - 2);
                p->setPen(pal.light());
                p->drawPoint(x + 1, y + h - 2);
                // top right corner:
                p->setPen(pal.background());
                p->drawPoint(x + w - 1, y);
                p->drawPoint(x + w - 2, y);
                p->drawPoint(x + w - 1, y + 1);
                p->setPen(pal.dark());
                p->drawPoint(x + w - 2, y + 1);
                // bottom right corner:
                p->setPen(pal.background());
                p->drawPoint(x + w - 1, y + h - 1);
                p->drawPoint(x + w - 2, y + h - 1);
                p->drawPoint(x + w - 1, y + h - 2);
                p->setPen(pal.light());
                p->drawPoint(x + w - 2, y + h - 2);
                p->setPen(pal.dark());
                p->drawPoint(x + w - 3, y + h - 3);
                // ### end slider groove

                if (how & State_HasFocus)
                    drawPrimitive(PE_FocusRect, p, groove, pal);
            }

            if ((sub & SC_SliderHandle) && handle.isValid()) {
                const QColor c0 = pal.shadow();
                const QColor c1 = pal.dark();
                const QColor c3 = pal.light();

                int x1 = handle.x();
                int x2 = handle.x() + handle.width() - 1;
                int y1 = handle.y();
                int y2 = handle.y() + handle.height() - 1;
                int mx = handle.width() / 2;
                int my = handle.height() / 2;

                if (slider->orientation() == Qt::Vertical) {
                    // Background
                    QBrush oldBrush = p->brush();
                    p->setBrush(pal.brush(QPalette::Button));
                    p->setPen(Qt::NoPen);
                    QPolygon a(6);
                    a.setPoint(0, x1 + 1, y1 + 1);
                    a.setPoint(1, x2 - my + 2, y1 + 1);
                    a.setPoint(2, x2 - 1, y1 + my - 1);
                    a.setPoint(3, x2 - 1, y2 - my + 1);
                    a.setPoint(4, x2 - my + 2, y2 - 1);
                    a.setPoint(5, x1 + 1, y2 - 1);
                    p->drawPolygon(a);
                    p->setBrush(oldBrush);

                    // shadow border
                    p->setPen(c0);
                    p->drawLine(x1, y1 + 1, x1,y2 - 1);
                    p->drawLine(x2 - my + 2, y1, x2, y1 + my - 2);
                    p->drawLine(x2 - my + 2, y2, x2, y1 + my + 2);
                    p->drawLine(x2, y1 + my - 2, x2, y1 + my + 2);
                    p->drawLine(x1 + 1, y1, x2 - my + 2, y1);
                    p->drawLine(x1 + 1, y2, x2 - my + 2, y2);

                    // light shadow
                    p->setPen(c3);
                    p->drawLine(x1 + 1, y1 + 2, x1 + 1, y2 - 2);
                    p->drawLine(x1 + 1, y1 + 1, x2 - my + 2, y1 + 1);
                    p->drawLine(x2 - my + 2, y1 + 1, x2 - 1, y1 + my - 2);

                    // dark shadow
                    p->setPen(c1);
                    p->drawLine(x2 - 1, y1 + my - 2, x2 - 1, y1 + my + 2);
                    p->drawLine(x2 - my + 2, y2 - 1, x2 - 1, y1 + my + 2);
                    p->drawLine(x1 + 1, y2 - 1, x2 -my + 2, y2 - 1);

                    drawRiffles(p, handle.x(), handle.y() + 2, handle.width() - 3,
                                 handle.height() - 4, pal, true);
                } else {  // Qt::Horizontal
                    QBrush oldBrush = p->brush();
                    p->setBrush(pal.brush(QPalette::Button));
                    p->setPen(Qt::NoPen);
                    QPolygon a(6);
                    a.setPoint(0, x2 - 1, y1 + 1);
                    a.setPoint(1, x2 - 1, y2 - mx + 2);
                    a.setPoint(2, x2 - mx + 1, y2 - 1);
                    a.setPoint(3, x1 + mx - 1, y2 - 1);
                    a.setPoint(4, x1 + 1, y2 - mx + 2);
                    a.setPoint(5, x1 + 1, y1 + 1);
                    p->drawPolygon(a);
                    p->setBrush(oldBrush);

                    // shadow border
                    p->setPen(c0);
                    p->drawLine(x1 + 1, y1, x2 - 1, y1);
                    p->drawLine(x1, y2 - mx + 2, x1 + mx - 2, y2);
                    p->drawLine(x2, y2 - mx + 2, x1 + mx + 2, y2);
                    p->drawLine(x1 + mx - 2, y2, x1 + mx + 2, y2);
                    p->drawLine(x1, y1 + 1, x1, y2 - mx + 2);
                    p->drawLine(x2, y1 + 1, x2, y2 - mx + 2);

                    // light shadow
                    p->setPen(c3);
                    p->drawLine(x1 + 1, y1 + 1, x2 - 1, y1 + 1);
                    p->drawLine(x1 + 1, y1 + 1, x1 + 1, y2 - mx + 2);

                    // dark shadow
                    p->setPen(c1);
                    p->drawLine(x2 - 1, y1 + 1, x2 - 1, y2 - mx + 2);
                    p->drawLine(x1 + 1, y2 - mx + 2, x1 + mx - 2, y2 - 1);
                    p->drawLine(x2 - 1, y2 - mx + 2, x1 + mx + 2, y2 - 1);
                    p->drawLine(x1 + mx - 2, y2 - 1, x1 + mx + 2, y2 - 1);

                    drawRiffles(p, handle.x() + 2, handle.y(), handle.width() - 4,
                                 handle.height() - 5, pal, false);
                }
            }

            if (sub & SC_SliderTickmarks)
                QCommonStyle::drawComplexControl(control, p, widget, r,
                                                  pal, how, SC_SliderTickmarks,
                                                  subActive, opt);
#endif
            break;
        }
    default:
        QWindowsStyle::drawComplexControl(control, p, widget, r, pal,
                                           how, sub, subActive, opt);
        break;
    }
}



/*!
    \reimp
*/
QRect QPlatinumStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                                             SubControl sc, const QWidget *widget) const
/*
QRect QPlatinumStyle::subControlRect(ComplexControl control,
                                             const QWidget *widget,
                                             SubControl sc,
                                             const Q3StyleOption& opt) const
                                             */
{
    switch(control) {
#ifndef QT_NO_COMBOBOX
    case CC_ComboBox:
        const QComboBox *cb;
        cb = (const QComboBox *)widget;
        switch(sc) {
        case SC_ComboBoxArrow: {
            QRect ir = cb->rect();
            int xx;
            if(QApplication::isRightToLeft())
                xx = ir.x();
            else
                xx = ir.x() + ir.width() - 20;
            return QRect(xx, ir.y(), 20, ir.height()); }
        default:
            break;
        }
        break;
#endif
#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar: {
        const QScrollBar *sb;
        sb = (const QScrollBar *)widget;
        int sbextent = pixelMetric(PM_ScrollBarExtent, widget);
        int maxlen = ((sb->orientation() == Qt::Horizontal) ?
                      sb->width() : sb->height()) - (sbextent * 2);

        int sliderlen;

        // calculate length
        if (sb->maximum() != sb->minimum()) {
            uint range = sb->maximum() - sb->minimum();
            sliderlen = (sb->pageStep() * maxlen) /
                        (range + sb->pageStep());

            int slidermin = pixelMetric(PM_ScrollBarSliderMin, widget);
            if (sliderlen < slidermin || range > INT_MAX / 2)
                sliderlen = slidermin;
            if (sliderlen > maxlen)
                sliderlen = maxlen;
        } else {
            sliderlen = maxlen;
        }
        int sliderstart = positionFromValue(sb->minimum(), sb->maximum(),
                                            sb->sliderPosition(), maxlen - sliderlen,
                                            sb->invertedAppearance());

        switch (sc) {
        case SC_ScrollBarSubLine:
            if (sb->orientation() == Qt::Horizontal) {
                int buttonw = qMin(sb->width() / 2, sbextent);
                return QRect(sb->width() - 2 * buttonw, 0, buttonw, sbextent);
            } else {
                int buttonh = qMin(sb->height() / 2, sbextent);
                return QRect(0, sb->height() - 2 * buttonh, sbextent, buttonh);
            }
        case SC_ScrollBarAddLine:
            if (sb->orientation() == Qt::Horizontal) {
                int buttonw = qMin(sb->width() / 2, sbextent);
                return QRect(sb->width() - buttonw, 0, sbextent, buttonw);
            } else {
                int buttonh = qMin(sb->height() / 2, sbextent);
                return QRect(0, sb->height() - buttonh, sbextent, buttonh);
            }
        case SC_ScrollBarSubPage:
            if (sb->orientation() == Qt::Horizontal)
                return QRect(1, 0, sliderstart, sbextent);
            return QRect(0, 1, sbextent, sliderstart);
        case SC_ScrollBarAddPage:
            if (sb->orientation() == Qt::Horizontal)
                return QRect(sliderstart + sliderlen, 0, maxlen - sliderstart - sliderlen, sbextent);
            return QRect(0, sliderstart + sliderlen, sbextent, maxlen - sliderstart - sliderlen);
        case SC_ScrollBarGroove:
            if (sb->orientation() == Qt::Horizontal)
                return QRect(1, 0, sb->width() - sbextent * 2, sb->height());
            return QRect(0, 1, sb->width(), sb->height() - sbextent * 2);
        case SC_ScrollBarSlider:
            if (sb->orientation() == Qt::Horizontal)
                return QRect(sliderstart, 0, sliderlen, sbextent);
            return QRect(0, sliderstart, sbextent, sliderlen);
        default:
            break;
        }
        break; }
#endif
#ifndef QT_NO_SLIDER
    case CC_Slider: {

        const QSlider *slider = (const QSlider *) widget;
        int tickOffset = pixelMetric(PM_SliderTickmarkOffset, widget);
        int thickness = pixelMetric(PM_SliderControlThickness, widget);
        int mid = thickness / 2;
        int ticks = slider->tickmarks();
        int len = pixelMetric(PM_SliderLength, widget);

        switch (sc) {
        case SC_SliderGroove:
            if (ticks & QSlider::Above)
                mid += len / 8;
            if (ticks & QSlider::Below)
                mid -= len / 8;
            if (slider->orientation() == QSlider::Horizontal)
                return QRect(0, tickOffset, slider->width(), thickness);
            return QRect(tickOffset, 0, thickness, slider->height());
        default:
            break;
        }
        break; }
#endif
    default:
        break;
    }
    return QWindowsStyle::subControlRect(control, widget, sc, opt);
}


/*!\reimp
 */
int QPlatinumStyle::pixelMetric(PixelMetric metric, const QStyleOption *option,
                                const QWidget *widget) const
{
    int ret;
    switch(metric) {
    case PM_ButtonDefaultIndicator:
        ret = 3;
        break;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        ret = 0;
        break;
    case PM_IndicatorWidth:
        ret = 15;
        break;
    case PM_IndicatorHeight:
        ret = 13;
        break;
    case PM_ExclusiveIndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
        ret = 15;
        break;
    case PM_SliderLength:
        ret = 17;
        break;
    case PM_MaximumDragDistance:
        ret = -1;
        break;
    default:
        ret = QWindowsStyle::pixelMetric(metric, option, widget);
        break;
    }
    return ret;
}

/*!
    \reimp
*/
QRect QPlatinumStyle::subRect(SubRect r, const QStyleOption *opt, const QWidget *widget) const
//QRect QPlatinumStyle::subRect(SubRect r, const QWidget *widget) const
{
    QRect rect;
    switch (r) {
    case SR_ComboBoxFocusRect:
        {
            QRect tmpR = widget->rect();
            rect = QRect(tmpR.x() + 4, tmpR.y() + 4, tmpR.width() - 8 - 16,
                          tmpR.height() - 8);
            break;
        }
    default:
        rect = QWindowsStyle::subRect(r, widget);
        break;
    }
    return rect;
}

/*!
    Mixes two colors \a c1 and \a c2 to a new color.
*/
QColor QPlatinumStyle::mixedColor(const QColor &c1, const QColor &c2) const
{
    int h1,s1,v1,h2,s2,v2;
    c1.getHsv(&h1,&s1,&v1);
    c2.getHsv(&h2,&s2,&v2);
    return QColor((h1+h2)/2, (s1+s2)/2, (v1+v2)/2, QColor::Hsv);
}

/*!
    Draws the nifty Macintosh decoration used on sliders using painter
    \a p and colorgroup \a pal. \a x, \a y, \a w, \a h and \a horizontal
    specify the geometry and orientation of the riffles.
*/
void QPlatinumStyle::drawRiffles(QPainter* p,  int x, int y, int w, int h,
                                  const QPalette &pal, bool horizontal) const
{
    if (!horizontal) {
        if (h > 20) {
            y += (h-20)/2 ;
            h = 20;
        }
        if (h > 8) {
            int n = h / 4;
            int my = y+h/2-n;
            int i ;
            p->setPen(pal.light());
            for (i=0; i<n; i++) {
                p->drawLine(x+3, my+2*i, x+w-5, my+2*i);
            }
            p->setPen(pal.dark());
            my++;
            for (i=0; i<n; i++) {
                p->drawLine(x+4, my+2*i, x+w-4, my+2*i);
            }
        }
    }
    else {
        if (w > 20) {
            x += (w-20)/2 ;
            w = 20;
        }
        if (w > 8) {
            int n = w / 4;
            int mx = x+w/2-n;
            int i ;
            p->setPen(pal.light());
            for (i=0; i<n; i++) {
                p->drawLine(mx+2*i, y+3, mx + 2*i, y+h-5);
            }
            p->setPen(pal.dark());
            mx++;
            for (i=0; i<n; i++) {
                p->drawLine(mx+2*i, y+4, mx + 2*i, y+h-4);
            }
        }
    }
}

#endif
