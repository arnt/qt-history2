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
        if (!(opt->state & State_Enabled) && styleHint(SH_DitherDisabledText))
            p->fillRect(opt->rect, QBrush(p->background().color(), Qt::Dense5Pattern));
    } break;
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
            if (!(opt->state & State_Enabled) && styleHint(SH_DitherDisabledText))
                p->fillRect(opt->rect, QBrush(p->background().color(), Qt::Dense5Pattern));
        } break;

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
    default:
        QMotifStyle::drawPrimitive(pe, opt, p, widget);
    }
}

#endif
