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

#include "qmenu.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpalette.h"
#include "qwidget.h"
#include "qpushbutton.h"
#include "qscrollbar.h"
#include "qtabbar.h"
#include "qtabwidget.h"
#include "qlistview.h"
#include "qsplitter.h"
#include "qslider.h"
#include "qcombobox.h"
#include "qlineedit.h"
#include "qprogressbar.h"
#include "qimage.h"
#include "qfocusframe.h"
#include "qdebug.h"
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
    case PM_MenuBarPanelWidth:
    case PM_DefaultFrameWidth:
    case PM_FocusFrameVMargin:
    case PM_FocusFrameHMargin:
    case PM_MenuPanelWidth:
    case PM_SpinBoxFrameWidth:
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
                             const QWidget *widget) const
{

    switch(element) {
    case CE_MenuBarItem: {
        if (opt->state & State_Selected)  // active item
            qDrawShadePanel(p, opt->rect, opt->palette, true, 1,
                            &opt->palette.brush(QPalette::Button));
        else  // other item
            p->fillRect(opt->rect, opt->palette.brush(QPalette::Button));
        QCommonStyle::drawControl(element, opt, p, widget);
        break;  }
    default:
        QMotifStyle::drawControl(element, opt, p, widget);
    break;
    }


}

/*!
    \reimp
*/
void QCDEStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                        const QWidget *widget) const
{
    switch(pe) {
    case PE_IndicatorCheckBox: {
        bool down = opt->state & State_Down;
        bool on = opt->state & State_On;
        bool showUp = !(down ^ on);
        QBrush fill = showUp || opt->state & State_NoChange ? opt->palette.brush(QPalette::Button) : opt->palette.brush(QPalette::Mid);
        qDrawShadePanel(p, opt->rect,  opt->palette, !showUp, pixelMetric(PM_DefaultFrameWidth), &opt->palette.brush(QPalette::Button));

        if (!(opt->state & State_Off)) {
            QRect r = opt->rect;
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
            if (opt->state & State_NoChange)
                p->setPen(opt->palette.dark().color());
            else
                p->setPen(opt->palette.foreground().color());
            p->drawLineSegments(a);
        }
    }
        break;
    case PE_IndicatorRadioButton:
        {
            QRect r = opt->rect;
#define INTARRLEN(x) sizeof(x)/(sizeof(int)*2)
            static const int pts1[] = {              // up left  lines
                1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
            static const int pts4[] = {              // bottom right  lines
                2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
                11,4, 10,3, 10,2 };
            static const int pts5[] = {              // inner fill
                4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };
            bool down = opt->state & State_Down;
            bool on = opt->state & State_On;
            QPolygon a(INTARRLEN(pts1), pts1);
            a.translate(r.x(), r.y());
            p->setPen((down || on) ? opt->palette.dark().color() : opt->palette.light().color());
            p->drawPolyline(a);
            a.setPoints(INTARRLEN(pts4), pts4);
            a.translate(r.x(), r.y());
            p->setPen((down || on) ? opt->palette.light().color() : opt->palette.dark().color());
            p->drawPolyline(a);
            a.setPoints(INTARRLEN(pts5), pts5);
            a.translate(r.x(), r.y());
            QColor fillColor = on ? opt->palette.dark().color() : opt->palette.background().color();
            p->setPen(fillColor);
            p->setBrush(on ? opt->palette.brush(QPalette::Dark) :
                         opt->palette.brush(QPalette::Background));
            p->drawPolygon(a);
            break;
        }

    case PE_IndicatorRadioButtonMask:
        {
            static const int pts1[] = {
                // up left  lines
                1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1,
                // bottom right  lines
                10,2, 10,3, 11,4, 11,7, 10,8, 10,9, 9,10, 8,10, 7,11, 4,11, 3,10, 2,10
            };
            QRect r = opt->rect;
            QPolygon a(INTARRLEN(pts1), pts1);
            a.translate(r.x(), r.y());
            p->setPen(Qt::color1);
            p->setBrush(Qt::color1);
            p->drawPolygon(a);
            break;
        }
    case PE_IndicatorSpinUp:
    case PE_IndicatorSpinPlus:
    case PE_IndicatorSpinDown:
    case PE_IndicatorSpinMinus:
    case PE_IndicatorArrowUp:
    case PE_IndicatorArrowDown:
    case PE_IndicatorArrowRight:
    case PE_IndicatorArrowLeft: {
        QRect rect = opt->rect;
        QPolygon bFill;                          // fill polygon
        QPolygon bTop;                           // top shadow.
        QPolygon bBot;                           // bottom shadow.
        QPolygon bLeft;                          // left shadow.
        QMatrix    matrix;                         // xform matrix
        if (pe == PE_IndicatorSpinPlus || pe == PE_IndicatorSpinUp)
            pe = PE_IndicatorArrowUp;
        else if (pe == PE_IndicatorSpinMinus || pe == PE_IndicatorSpinDown)
            pe = PE_IndicatorArrowDown;
        bool vertical = pe == PE_IndicatorArrowUp || pe == PE_IndicatorArrowDown;
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

        if (pe == PE_IndicatorArrowUp || pe == PE_IndicatorArrowLeft) {
            matrix.translate(rect.x(), rect.y());
            if (vertical) {
                matrix.translate(0, rect.height() - 1);
                matrix.rotate(-90);
            } else {
                matrix.translate(rect.width() - 1, rect.height() - 1);
                matrix.rotate(180);
            }
            if (opt->state & State_Down)
                colspec = horizontal ? 0x2334 : 0x2343;
            else
                colspec = horizontal ? 0x1443 : 0x1434;
        } else if (pe == PE_IndicatorArrowDown || pe == PE_IndicatorArrowRight) {
            matrix.translate(rect.x(), rect.y());
            if (vertical) {
                matrix.translate(rect.width()-1, 0);
                matrix.rotate(90);
            }
            if (opt->state & State_Down)
                colspec = horizontal ? 0x2443 : 0x2434;
            else
                colspec = horizontal ? 0x1334 : 0x1343;
        }

        const QColor *cols[5];
        if (opt->state & State_Enabled) {
            cols[0] = 0;
            cols[1] = &opt->palette.button().color();
            cols[2] = &opt->palette.mid().color();
            cols[3] = &opt->palette.light().color();
            cols[4] = &opt->palette.dark().color();
        } else {
            cols[0] = 0;
            cols[1] = &opt->palette.button().color();
            cols[2] = &opt->palette.button().color();
            cols[3] = &opt->palette.button().color();
            cols[4] = &opt->palette.button().color();
        }

#define CMID    *cols[(colspec>>12) & 0xf]
#define CLEFT   *cols[(colspec>>8) & 0xf]
#define CTOP    *cols[(colspec>>4) & 0xf]
#define CBOT    *cols[colspec & 0xf]

        QPen     savePen   = p->pen();              // save current pen
        QBrush   saveBrush = p->brush();            // save current brush
        QMatrix wxm = p->worldMatrix();
        QPen     pen(Qt::NoPen);
        QBrush brush = opt->palette.brush(opt->state & State_Enabled ? QPalette::Button :
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
        QMotifStyle::drawPrimitive(pe, opt, p, widget);
    }
}

#endif
