/****************************************************************************
**
** Implementation of the QCommonStyle class.
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

#include "qcommonstyle.h"

#ifndef QT_NO_STYLE

#include "private/qdialogbuttons_p.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qcheckbox.h"
#include "qdockarea.h"
#include "qdrawutil.h"
#include "qgroupbox.h"
#include "qheader.h"
#include "qmenu.h"
#include "qpainter.h"
#include "qprogressbar.h"
#include "qpushbutton.h"
#include "qradiobutton.h"
#include "qrangecontrol.h"
#include "qscrollbar.h"
#include "qslider.h"
#include "qspinbox.h"
#include "qstyleoption.h"
#include "qtabbar.h"
#include "qtoolbar.h"
#include "qtoolbox.h"
#include "qtoolbutton.h"

#include <limits.h>

/*!
    \class QCommonStyle qcommonstyle.h
    \brief The QCommonStyle class encapsulates the common Look and Feel of a GUI.

    \ingroup appearance

    This abstract class implements some of the widget's look and feel
    that is common to all GUI styles provided and shipped as part of
    Qt.

    All the functions are documented in \l QStyle.
*/

/*!
    \enum Qt::ArrowType

    \value UpArrow
    \value DownArrow
    \value LeftArrow
    \value RightArrow

*/

// the active painter, if any... this is used as an optimzation to
// avoid creating a painter if we have an active one (since
// QStyle::itemRect() needs a painter to operate correctly
static QPainter *activePainter = 0;

/*!
    Constructs a QCommonStyle.
*/
QCommonStyle::QCommonStyle() : QStyle()
{
    activePainter = 0;
}

/*! \reimp */
QCommonStyle::~QCommonStyle()
{
    activePainter = 0;
}


static const char * const check_list_controller_xpm[] = {
"16 16 4 1",
"        c None",
".        c #000000000000",
"X        c #FFFFFFFF0000",
"o        c #C71BC30BC71B",
"                ",
"                ",
" ..........     ",
" .XXXXXXXX.     ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" ..........oo   ",
"   oooooooooo   ",
"   oooooooooo   ",
"                ",
"                "};

static const char * const tree_branch_open_xpm[] = {
"9 9 2 1",
"  c None",
"# c #000000",
"#########",
"#       #",
"# ##### #",
"#  ###  #",
"#  ###  #",
"#   #   #",
"#   #   #",
"#       #",
"#########"};

static const char * const tree_branch_closed_xpm[] = {
"9 9 2 1",
"  c None",
"# c #000000",
"#########",
"#       #",
"# #     #",
"# ###   #",
"# ##### #",
"# ###   #",
"# #     #",
"#       #",
"#########"};

/*! \reimp */
void QCommonStyle::drawPrimitive(PrimitiveElement pe,
                                  QPainter *p,
                                  const QRect &r,
                                  const QPalette &pal,
                                  SFlags flags,
                                  const QStyleOption& ) const
{
    activePainter = p;

    switch (pe) {

    case PE_SpinWidgetPlus:
    case PE_SpinWidgetMinus: {
        p->save();
        int fw = pixelMetric(PM_DefaultFrameWidth, 0);
        QRect br;
        br.setRect(r.x() + fw, r.y() + fw, r.width() - fw*2,
                    r.height() - fw*2);

        p->fillRect(br, pal.brush(QPalette::Button));
        p->setPen(pal.buttonText());
        p->setBrush(pal.buttonText());

        int length;
        int x = r.x(), y = r.y(), w = r.width(), h = r.height();
        if (w <= 8 || h <= 6)
            length = qMin(w-2, h-2);
        else
            length = qMin(2*w / 3, 2*h / 3);

        if (!(length & 1))
            length -=1;
        int xmarg = (w - length) / 2;
        int ymarg = (h - length) / 2;

        p->drawLine(x + xmarg, (y + h / 2 - 1),
                     x + xmarg + length - 1, (y + h / 2 - 1));
        if (pe == PE_SpinWidgetPlus)
            p->drawLine((x+w / 2) - 1, y + ymarg,
                         (x+w / 2) - 1, y + ymarg + length - 1);
        p->restore();
        break; }

    case PE_SpinWidgetUp:
    case PE_SpinWidgetDown: {
        int fw = pixelMetric(PM_DefaultFrameWidth, 0);
        QRect br;
        br.setRect(r.x() + fw, r.y() + fw, r.width() - fw*2,
                    r.height() - fw*2);
        p->fillRect(br, pal.brush(QPalette::Button));
        int x = r.x(), y = r.y(), w = r.width(), h = r.height();
        int sw = w-4;
        if (sw < 3)
            break;
        else if (!(sw & 1))
            sw--;
        sw -= (sw / 7) * 2;        // Empty border
        int sh = sw/2 + 2;      // Must have empty row at foot of arrow

        int sx = x + w / 2 - sw / 2 - 1;
        int sy = y + h / 2 - sh / 2 - 1;

        QPointArray a;
        if (pe == PE_SpinWidgetDown)
            a.setPoints(3,  0, 1,  sw-1, 1,  sh-2, sh-1);
        else
            a.setPoints(3,  0, sh-1,  sw-1, sh-1,  sh-2, 1);
        int bsx = 0;
        int bsy = 0;
        if (flags & Style_Sunken) {
            bsx = pixelMetric(PM_ButtonShiftHorizontal);
            bsy = pixelMetric(PM_ButtonShiftVertical);
        }
        p->save();
        p->translate(sx + bsx, sy + bsy);
        p->setPen(pal.buttonText());
        p->setBrush(pal.buttonText());
        p->drawPolygon(a);
        p->restore();
        break; }

    case PE_DockWindowSeparator: {
        QPoint p1, p2;
        if (flags & Style_Horizontal) {
            p1 = QPoint(r.width()/2, 0);
            p2 = QPoint(p1.x(), r.height());
        } else {
            p1 = QPoint(0, r.height()/2);
            p2 = QPoint(r.width(), p1.y());
        }
        qDrawShadeLine(p, p1, p2, pal, 1, 1, 0);
        break; }


    default:
        break;
    }

    activePainter = 0;
}

/*!
    Draws the primitive element \a pe, with style options \a opt, on
    painter \a p, with parent widget \a w.
*/
void QCommonStyle::drawPrimitive(PrimitiveElement pe, const Q4StyleOption *opt, QPainter *p,
                                 const QWidget *widget) const
{
    switch (pe) {
    case PE_ButtonCommand:
    case PE_ButtonBevel:
    case PE_ButtonTool:
    case PE_ButtonDropDown:
    case PE_HeaderSection:
        qDrawShadePanel(p, opt->rect, opt->palette,
                        opt->state & (Style_Sunken | Style_Down | Style_On), 1,
                        &opt->palette.brush(QPalette::Button));
        break;
    case PE_Indicator:
        if (opt->state & Style_NoChange) {
            p->setPen(opt->palette.foreground());
            p->fillRect(opt->rect, opt->palette.brush(QPalette::Button));
            p->drawRect(opt->rect);
            p->drawLine(opt->rect.topLeft(), opt->rect.bottomRight());
        } else {
            qDrawShadePanel(p, opt->rect.x(), opt->rect.y(), opt->rect.width(), opt->rect.height(),
                            opt->palette, opt->state & (Style_Sunken | Style_On), 1,
                            &opt->palette.brush(QPalette::Button));
        }
        break;
    case PE_IndicatorMask:
        p->fillRect(opt->rect, color1);
        break;
    case PE_ExclusiveIndicator: {
        QRect ir = opt->rect;
        p->setPen(opt->palette.dark());
        p->drawArc(opt->rect, 0, 5760);
        if (opt->state & (Style_Sunken | Style_On)) {
            ir.addCoords(2, 2, -2, -2);
            p->setBrush(opt->palette.foreground());
            p->drawEllipse(ir);
        }
        break; }
    case PE_ExclusiveIndicatorMask:
        p->setPen(color1);
        p->setBrush(color1);
        p->drawEllipse(opt->rect);
        break;
    case PE_FocusRect:
        if (const Q4StyleOptionFocusRect *fropt = qt_cast<const Q4StyleOptionFocusRect *>(opt)) {
            QColor bg = fropt->backgroundColor;
            QPen oldPen = p->pen();
            if (bg.isValid()) {
                int h, s, v;
                bg.getHsv(&h, &s, &v);
                if (v >= 128)
                    p->setPen(Qt::black);
                else
                    p->setPen(Qt::white);
            } else {
                p->setPen(opt->palette.foreground());
            }
            if (opt->state & Style_FocusAtBorder)
                p->drawRect(QRect(opt->rect.x() + 1, opt->rect.y() + 1, opt->rect.width() - 2,
                            opt->rect.height() - 2));
            else
                p->drawRect(opt->rect);
            p->setPen(oldPen);
        }
        break;
    case PE_CheckMark: {
        const int markW = opt->rect.width() > 7 ? 7 : opt->rect.width();
        const int markH = markW;
        int posX = opt->rect.x() + (opt->rect.width() - markW)/2 + 1;
        int posY = opt->rect.y() + (opt->rect.height() - markH)/2;

        QPointArray a(markH * 2);
        int i, xx, yy;
        xx = posX;
        yy = 3 + posY;
        for (i = 0; i < markW / 2; ++i) {
            a.setPoint(2 * i, xx, yy);
            a.setPoint(2 * i + 1, xx, yy + 2);
            ++xx;
            ++yy;
        }
        yy -= 2;
        for (; i < markH; ++i) {
            a.setPoint(2 * i,   xx, yy);
            a.setPoint(2 * i + 1, xx, yy + 2);
            ++xx;
            --yy;
        }
        if (!(opt->state & Style_Enabled) && !(opt->state & Style_On)) {
            int pnt;
            p->setPen(opt->palette.highlightedText());
            QPoint offset(1, 1);
            for (pnt = 0; pnt < a.size(); ++pnt)
                a[pnt] += offset;
            p->drawLineSegments(a);
            for (pnt = 0; pnt < a.size(); ++pnt)
                a[pnt] -= offset;
        }
        p->setPen(opt->palette.text());
        p->drawLineSegments(a);
        break; }
    case PE_MenuFrame:
    case PE_Panel:
    case PE_PanelPopup:
        if (const Q4StyleOptionFrame *frame = qt_cast<const Q4StyleOptionFrame *>(opt))
            qDrawShadePanel(p, frame->rect, frame->palette, frame->state & Style_Sunken,
                            frame->lineWidth);
        break;
    case PE_MenuBarFrame:
    case PE_PanelMenuBar:
        if (const Q4StyleOptionFrame *frame = qt_cast<const Q4StyleOptionFrame *>(opt))
            qDrawShadePanel(p, frame->rect, frame->palette, false, frame->lineWidth,
                            &frame->palette.brush(QPalette::Button));
        break;
    case PE_ProgressBarChunk:
        p->fillRect(opt->rect.x(), opt->rect.y() + 3, opt->rect.width() -2, opt->rect.height() - 6,
                    opt->palette.brush(QPalette::Highlight));
        break;
    case PE_CheckListController:
        p->drawPixmap(opt->rect.topLeft(), QPixmap(check_list_controller_xpm));
        break;
    case PE_CheckListExclusiveIndicator:
        if (const Q4StyleOptionListView *lv = qt_cast<const Q4StyleOptionListView *>(opt)) {
            if (lv->items.isEmpty())
                return;
            int x = lv->rect.x(),
                y = lv->rect.y();
#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)
            static const QCOORD pts1[] = {                // dark lines
                1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
            static const QCOORD pts2[] = {                // black lines
                2,8, 1,7, 1,4, 2,3, 2,2, 3,2, 4,1, 7,1, 8,2, 9,2 };
            static const QCOORD pts3[] = {                // background lines
                2,9, 3,9, 4,10, 7,10, 8,9, 9,9, 9,8, 10,7, 10,4, 9,3 };
            static const QCOORD pts4[] = {                // white lines
                2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
                11,4, 10,3, 10,2 };
                // static const QCOORD pts5[] = {                // inner fill
                //    4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };
                //QPointArray a;

                if (lv->state & Style_Enabled)
                    p->setPen(lv->palette.text());
                else
                    p->setPen(QPen(lv->viewportPalette.color(QPalette::Disabled, QPalette::Text)));
                QPointArray a(QCOORDARRLEN(pts1), pts1);
                a.translate(x, y);
                //p->setPen(pal.dark());
                p->drawPolyline(a);
                a.setPoints(QCOORDARRLEN(pts2), pts2);
                a.translate(x, y);
                p->drawPolyline(a);
                a.setPoints(QCOORDARRLEN(pts3), pts3);
                a.translate(x, y);
                //                p->setPen(black);
                p->drawPolyline(a);
                a.setPoints(QCOORDARRLEN(pts4), pts4);
                a.translate(x, y);
                //                        p->setPen(blue);
                p->drawPolyline(a);
                //                a.setPoints(QCOORDARRLEN(pts5), pts5);
                //                a.translate(x, y);
                //        QColor fillColor = isDown() ? g.background() : g.base();
                //        p->setPen(fillColor);
                //        p->setBrush(fillColor);
                //        p->drawPolygon(a);
                if (opt->state & Style_On) {
                    p->setPen(NoPen);
                    p->setBrush(opt->palette.text());
                    p->drawRect(x + 5, y + 4, 2, 4);
                    p->drawRect(x + 4, y + 5, 4, 2);
                }
#undef QCOORDARRLEN
        }
        break;
    case PE_CheckListIndicator:
        if (const Q4StyleOptionListView *lv = qt_cast<const Q4StyleOptionListView *>(opt)) {
            if(lv->items.isEmpty())
                break;
            Q4StyleOptionListViewItem item = lv->items.at(0);
            int x = lv->rect.x(),
                y = lv->rect.y(),
                w = lv->rect.width(),
                h = lv->rect.width(),
                marg = lv->itemMargin;

            if (lv->state & Style_Enabled)
                p->setPen(QPen(lv->palette.text(), 2));
            else
                p->setPen(QPen(lv->viewportPalette.color(QPalette::Disabled, QPalette::Text), 2));
            if (opt->state & Style_Selected && !lv->rootIsDecorated
                    && !(item.extras & Q4StyleOptionListViewItem::ParentControl)) {
                p->fillRect(0, 0, x + marg + w + 4, item.height,
                            lv->palette.brush(QPalette::Highlight));
                if (item.state & Style_Enabled)
                    p->setPen(QPen(lv->palette.highlightedText(), 2));
            }

            if (lv->state & Style_NoChange)
                p->setBrush(lv->palette.brush(QPalette::Button));
            p->drawRect(x + marg, y + 2, w - 4, h - 4);
            /////////////////////
            ++x;
            ++y;
            if (lv->state & Style_On || lv->state & Style_NoChange) {
                QPointArray a(7 * 2);
                int i,
                    xx = x + 1 + marg,
                    yy = y + 5;
                for (i = 0; i < 3; ++i) {
                    a.setPoint(2 * i,   xx, yy);
                    a.setPoint(2 * i + 1, xx, yy + 2);
                    ++xx;
                    ++yy;
                }
                yy -= 2;
                for (i = 3; i < 7; ++i) {
                    a.setPoint(2 * i,   xx, yy);
                    a.setPoint(2 * i + 1, xx, yy + 2);
                    ++xx;
                    --yy;
                }
                p->drawLineSegments(a);
            }
        }
        break;
    case PE_RubberBandMask:
    case PE_RubberBand: {
        QPen oldPen = p->pen();
        QBrush oldBrush = p->brush();
        if (pe == PE_RubberBandMask)
            p->setPen(QPen(color1, 5));
        else
            p->setPen(QPen(opt->palette.foreground(), 5));
        if (opt->state & Style_Rectangle)
            p->setBrush(QBrush(NoBrush));
        else if (pe == PE_RubberBandMask)
            p->setBrush(QBrush(color1));
        else
            p->setBrush(QBrush(opt->palette.foreground()));
        p->drawRect(opt->rect);
        p->setPen(oldPen);
        p->setBrush(oldBrush);
        break; }
    case PE_TreeBranch: {
        static QPixmap open(tree_branch_open_xpm);
        static QPixmap closed(tree_branch_closed_xpm);
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
            p->drawPixmap(bef_h, bef_v, opt->state & Style_Open ? open : closed);
        }
        if (opt->state & Style_Item) {
            if (QApplication::reverseLayout())
                p->drawLine(opt->rect.left(), mid_v, bef_h, mid_v);
            else
                p->drawLine(aft_h, mid_v, opt->rect.right(), mid_v);
        }
        if (opt->state & Style_Sibling)
            p->drawLine(mid_h, aft_v, mid_h, opt->rect.bottom());
        if (opt->state & (Style_Open | Style_Children | Style_Item | Style_Sibling))
            p->drawLine(mid_h, opt->rect.y(), mid_h, bef_v);
        break; }
    case PE_SizeGrip: {
        p->save();
        int x, y, w, h;
        opt->rect.rect(&x, &y, &w, &h);

        int sw = qMin(h, w);
        if (h > w)
            p->translate(0, h - w);
        else
            p->translate(w - h, 0);

        int sx = x;
        int sy = y;
        int s = sw / 3;

        if (QApplication::reverseLayout()) {
            sx = x + sw;
            for (int i = 0; i < 4; ++i) {
                p->setPen(QPen(opt->palette.light(), 1));
                p->drawLine(x, sy - 1 , sx + 1,  sw);
                p->setPen(QPen(opt->palette.dark(), 1));
                p->drawLine(x, sy, sx, sw);
                p->setPen(QPen(opt->palette.dark(), 1));
                p->drawLine(x, sy + 1, sx - 1,  sw);
                sx -= s;
                sy += s;
            }
        } else {
            for (int i = 0; i < 4; ++i) {
                p->setPen(QPen(opt->palette.light(), 1));
                p->drawLine(sx - 1, sw, sw,  sy - 1);
                p->setPen(QPen(opt->palette.dark(), 1));
                p->drawLine(sx, sw, sw,  sy);
                p->setPen(QPen(opt->palette.dark(), 1));
                p->drawLine(sx + 1, sw, sw,  sy + 1);
                sx += s;
                sy += s;
            }
        }
        p->restore();
        break; }
    case PE_Separator:
        qDrawShadeLine(p, opt->rect.left(), opt->rect.top(), opt->rect.right(), opt->rect.bottom(),
                       opt->palette, opt->state & Style_Sunken, 1, 0);
        break;
    case PE_StatusBarSection:
        qDrawShadeRect(p, opt->rect, opt->palette, true, 1, 0, 0);
        break;
    case PE_HeaderArrow: {
        QPen oldPen = p->pen();
        if (opt->state & Style_Down) {
            QPointArray pa(3);
            p->setPen(opt->palette.light());
            p->drawLine(opt->rect.x() + opt->rect.width(), opt->rect.y(),
                        opt->rect.x() + opt->rect.width() / 2, opt->rect.height());
            p->setPen(opt->palette.dark());
            pa.setPoint(0, opt->rect.x() + opt->rect.width() / 2, opt->rect.height());
            pa.setPoint(1, opt->rect.x(), opt->rect.y());
            pa.setPoint(2, opt->rect.x() + opt->rect.width(), opt->rect.y());
            p->drawPolyline(pa);
        } else {
            QPointArray pa(3);
            p->setPen(opt->palette.light());
            pa.setPoint(0, opt->rect.x(), opt->rect.height());
            pa.setPoint(1, opt->rect.x() + opt->rect.width(), opt->rect.height());
            pa.setPoint(2, opt->rect.x() + opt->rect.width() / 2, opt->rect.y());
            p->drawPolyline(pa);
            p->setPen(opt->palette.dark());
            p->drawLine(opt->rect.x(), opt->rect.height(), opt->rect.x() + opt->rect.width() / 2, opt->rect.y());
        }
        p->setPen(oldPen);
        break; }
    case PE_PanelLineEdit:
    case PE_PanelTabWidget:
    case PE_WindowFrame:
        drawPrimitive(PE_Panel, opt, p, widget);
        break;
    case PE_PanelGroupBox: //We really do not need PE_GroupBoxFrame anymore, nasty holdover ###
        drawPrimitive(PE_GroupBoxFrame, opt, p, widget);
        break;
    case PE_GroupBoxFrame:
        if (const Q4StyleOptionFrame *frame = qt_cast<const Q4StyleOptionFrame *>(opt)) {
            int lwidth = frame->lineWidth,
                mlwidth = frame->midLineWidth;
            if (opt->state & (Style_Sunken | Style_Raised))
                qDrawShadeRect(p, frame->rect.x(), frame->rect.y(), frame->rect.width(),
                               frame->rect.height(), frame->palette, frame->state & Style_Sunken,
                               lwidth, mlwidth);
            else
                qDrawPlainRect(p, frame->rect.x(), frame->rect.y(), frame->rect.width(),
                               frame->rect.height(), frame->palette.foreground(), lwidth);
        }
        break;
    case PE_PanelDockWindow:
        if (const Q4StyleOptionFrame *frame = qt_cast<const Q4StyleOptionFrame *>(opt)) {
            int lw = frame->lineWidth;
            if (lw <= 0)
                lw = pixelMetric(PM_DockWindowFrameWidth);

            qDrawShadePanel(p, frame->rect, frame->palette, false, lw);
        }
        break;
    case PE_DockWindowHandle:
        if (const Q4StyleOptionDockWindow *dw = qt_cast<const Q4StyleOptionDockWindow *>(opt)) {
            bool highlight = dw->state & Style_On;

            p->save();
            p->translate(dw->rect.x(), dw->rect.y());
            if (dw->state & Style_Horizontal) {
                int x = dw->rect.width() / 3;
                if (dw->rect.height() > 4) {
                    qDrawShadePanel(p, x, 2, 3, dw->rect.height() - 4,
                                    dw->palette, highlight, 1, 0);
                    qDrawShadePanel(p, x+3, 2, 3, dw->rect.height() - 4,
                                    dw->palette, highlight, 1, 0);
                }
            } else {
                if (dw->rect.width() > 4) {
                    int y = dw->rect.height() / 3;
                    qDrawShadePanel(p, 2, y, dw->rect.width() - 4, 3,
                                    dw->palette, highlight, 1, 0);
                    qDrawShadePanel(p, 2, y+3, dw->rect.width() - 4, 3,
                                    dw->palette, highlight, 1, 0);
                }
            }
            p->restore();
        }
        break;
    default:
        qWarning("QCommonStyle::drawPrimitive not handled %d", pe);
    }
}

/*! \reimp */
void QCommonStyle::drawControl(ControlElement element,
                                QPainter *p,
                                const QWidget *widget,
                                const QRect &r,
                                const QPalette &pal,
                                SFlags flags,
                                const QStyleOption& opt) const
{
    if (! widget) {
        qWarning("QCommonStyle::drawControl: widget parameter cannot be zero!");
        return;
    }

    activePainter = p;

    switch (element) {
    case CE_MenuBarEmptyArea: {
        if (!widget->testAttribute(QWidget::WA_NoSystemBackground))
            p->eraseRect(r);
        break; }
    case CE_PushButton:
        {
#ifndef QT_NO_PUSHBUTTON
            const QPushButton *button = (const QPushButton *) widget;
            QRect br = r;
            int dbi = pixelMetric(PM_ButtonDefaultIndicator, widget);

            if (button->isDefault() || button->autoDefault()) {
                if (button->isDefault())
                    drawPrimitive(PE_ButtonDefault, p, br, pal, flags);

                br.setCoords(br.left()   + dbi,
                             br.top()    + dbi,
                             br.right()  - dbi,
                             br.bottom() - dbi);
            }

            if (!button->isFlat() || button->isChecked() || button->isDown())
                drawPrimitive(PE_ButtonCommand, p, br, pal, flags);
#endif
            break;
        }

    case CE_PushButtonLabel:
        {
#ifndef QT_NO_PUSHBUTTON
            const QPushButton *button = (const QPushButton *) widget;
            QRect ir = r;

            if (button->isDown() || button->isChecked()) {
                flags |= Style_Sunken;
                ir.moveBy(pixelMetric(PM_ButtonShiftHorizontal, widget),
                          pixelMetric(PM_ButtonShiftVertical, widget));
            }
            // ### Please add another rect for QPushButton/AbstractButton that
            // takes care of this.  This is not the correct way to draw this
            // arrow, talk to TWS if you don't agree/want help.
            if (button->menu()) {
                int mbi = pixelMetric(PM_MenuButtonIndicator, widget);
                QRect ar(ir.right() - mbi, ir.y() + 2, mbi - 4, ir.height() - 4);
                drawPrimitive(PE_ArrowDown, p, ar, pal, flags, opt);
                ir.setWidth(ir.width() - mbi);
            }

            int tf=AlignVCenter | ShowPrefix;
            if (!styleHint(SH_UnderlineShortcut, widget, QStyleOption::Default, 0))
                tf |= NoAccel;

#ifndef QT_NO_ICONSET
            if (!button->icon().isNull()) {
                QIconSet::Mode mode =
                    button->isEnabled() ? QIconSet::Normal : QIconSet::Disabled;
                if (mode == QIconSet::Normal && button->hasFocus())
                    mode = QIconSet::Active;

                QIconSet::State state = QIconSet::Off;
                if (button->isCheckable() && button->isChecked())
                    state = QIconSet::On;

                QPixmap pixmap = button->icon().pixmap(QIconSet::Small, mode, state);
                int pixw = pixmap.width();
                int pixh = pixmap.height();

                //Center the icon if there is neither text nor pixmap
                if (button->text().isEmpty())
                    p->drawPixmap(ir.x() + ir.width() / 2 - pixw / 2, ir.y() + ir.height() / 2 - pixh / 2, pixmap);
                else
                    p->drawPixmap(ir.x() + 2, ir.y() + ir.height() / 2 - pixh / 2, pixmap);

                ir.moveBy(pixw + 4, 0);
                ir.setWidth(ir.width() - (pixw + 4));
                // left-align text if there is
                if (!button->text().isEmpty())
                    tf |= AlignLeft;
            } else
#endif //QT_NO_ICONSET
                tf |= AlignHCenter;
            drawItem(p, ir, tf, pal,
                     flags & Style_Enabled, QPixmap(), button->text(), -1, &(pal.buttonText().color()));

            if (flags & Style_HasFocus)
                drawPrimitive(PE_FocusRect, p, subRect(SR_PushButtonFocusRect, widget),
                              pal, flags);
#endif
            break;
        }

    case CE_CheckBox:
        drawPrimitive(PE_Indicator, p, r, pal, flags, opt);
        break;

#if 0
    case CE_TabBarTab:
        {
            const QTabBar * tb = (const QTabBar *) widget;

            if (tb->shape() == QTabBar::TriangularAbove ||
                 tb->shape() == QTabBar::TriangularBelow) {
                // triangular, above or below
                int y;
                int x;
                QPointArray a(10);
                a.setPoint(0, 0, -1);
                a.setPoint(1, 0, 0);
                y = r.height()-2;
                x = y/3;
                a.setPoint(2, x++, y-1);
                a.setPoint(3, x++, y);
                a.setPoint(3, x++, y++);
                a.setPoint(4, x, y);

                int i;
                int right = r.width() - 1;
                for (i = 0; i < 5; i++)
                    a.setPoint(9-i, right - a.point(i).x(), a.point(i).y());

                if (tb->shape() == QTabBar::TriangularAbove)
                    for (i = 0; i < 10; i++)
                        a.setPoint(i, a.point(i).x(),
                                    r.height() - 1 - a.point(i).y());

                a.translate(r.left(), r.top());

                if (flags & Style_Selected)
                    p->setBrush(pal.base());
                else
                    p->setBrush(pal.background());
                p->setPen(pal.foreground());
                p->drawPolygon(a);
                p->setBrush(NoBrush);
            }
            break;
        }

    case CE_TabBarLabel:
        {
            if (opt.isDefault())
                break;

            const QTabBar * tb = (const QTabBar *) widget;
            QTab * t = opt.tab();

            QRect tr = r;
            if (t->identifier() == tb->currentTab())
                tr.setBottom(tr.bottom() -
                              pixelMetric(QStyle::PM_DefaultFrameWidth, tb));

            int alignment = AlignCenter | ShowPrefix;
            if (!styleHint(SH_UnderlineShortcut, widget, QStyleOption::Default, 0))
                alignment |= NoAccel;
            drawItem(p, tr, alignment, pal, flags & Style_Enabled, t->text());

            if ((flags & Style_HasFocus) && !t->text().isEmpty())
                drawPrimitive(PE_FocusRect, p, r, pal);
            break;
        }
#endif
#ifndef QT_NO_TOOLBOX
    case CE_ToolBoxTab:
        {
            int d = 20 + r.height() - 3;
            QPointArray a(7);
            a.setPoint(0, -1, r.height() + 1);
            a.setPoint(1, -1, 1);
            a.setPoint(2, r.width() - d, 1);
            a.setPoint(3, r.width() - 20, r.height() - 2);
            a.setPoint(4, r.width() - 1, r.height() - 2);
            a.setPoint(5, r.width() - 1, r.height() + 1);
            a.setPoint(6, -1, r.height() + 1);

            const QToolBox *tb = (const QToolBox*)widget;

            if (flags & Style_Selected && tb->widget(tb->currentIndex())) {
                QWidget *tbW = tb->widget(tb->currentIndex());
                p->setBrush(tbW->palette().brush(tbW->backgroundRole()));
            } else {
                p->setBrush(pal.brush(tb->backgroundRole()));
            }

            p->setPen(pal.mid().color().dark(150));
            p->drawPolygon(a);
            p->setPen(pal.light());
            p->drawLine(0, 2, r.width() - d, 2);
            p->drawLine(r.width() - d - 1, 2, r.width() - 21, r.height() - 1);
            p->drawLine(r.width() - 20, r.height() - 1, r.width(), r.height() - 1);
            p->setBrush(NoBrush);
            break;
        }
#endif // QT_NO_TOOLBOX
    case CE_ProgressBarGroove:
        qDrawShadePanel(p, r, pal, true, 1, &pal.brush(QPalette::Background));
        break;

#ifndef QT_NO_PROGRESSBAR
    case CE_ProgressBarContents:
        {
            const QProgressBar *progressbar = (const QProgressBar *) widget;
            // Correct the highlight color if same as background,
            // or else we cannot see the progress...
            QPalette pal2 = pal;
            if (pal2.highlight() == pal2.background())
                pal2.setColor(QPalette::Highlight,
                               progressbar->palette().color(QPalette::Active,
                                                            QPalette::Highlight));
            bool reverse = QApplication::reverseLayout();
            int fw = 2;
            int w = r.width() - 2*fw;
            if (!progressbar->totalSteps()) {
                // draw busy indicator
                int x = progressbar->progress() % (w * 2);
                if (x > w)
                    x = 2 * w - x;
                x = reverse ? r.right() - x : x + r.x();
                p->setPen(QPen(pal2.highlight(), 4));
                p->drawLine(x, r.y() + 1, x, r.height() - fw);
            } else {
                const int unit_width = pixelMetric(PM_ProgressBarChunkWidth, widget);
                int u;
                if (unit_width > 1)
                    u = (r.width()+unit_width/3) / unit_width;
                else
                    u = w / unit_width;
                int p_v = progressbar->progress();
                int t_s = progressbar->totalSteps() ? progressbar->totalSteps() : 1;

                if (u > 0 && p_v >= INT_MAX / u && t_s >= u) {
                    // scale down to something usable.
                    p_v /= u;
                    t_s /= u;
                }

                // nu < tnu, if last chunk is only a partial chunk
                int tnu, nu;
                tnu = nu = p_v * u / t_s;

                if (nu * unit_width > w)
                    nu--;

                // Draw nu units out of a possible u of unit_width
                // width, each a rectangle bordered by background
                // color, all in a sunken panel with a percentage text
                // display at the end.
                int x = 0;
                int x0 = reverse ? r.right() - ((unit_width > 1) ?
                                                unit_width : fw) : r.x() + fw;
                for (int i=0; i<nu; i++) {
                    drawPrimitive(PE_ProgressBarChunk, p,
                                   QRect(x0+x, r.y(), unit_width, r.height()),
                                   pal2, Style_Default, opt);
                    x += reverse ? -unit_width: unit_width;
                }

                // Draw the last partial chunk to fill up the
                // progressbar entirely
                if (nu < tnu) {
                    int pixels_left = w - (nu*unit_width);
                    int offset = reverse ? x0+x+unit_width-pixels_left : x0+x;
                    drawPrimitive(PE_ProgressBarChunk, p,
                                   QRect(offset, r.y(), pixels_left,
                                          r.height()), pal2, Style_Default,
                                   opt);
                }
            }
        }
        break;

    case CE_ProgressBarLabel:
        {
            const QProgressBar *progressbar = (const QProgressBar *) widget;
            QColor penColor = pal.highlightedText();
            QColor *pcolor = 0;
            if (progressbar->centerIndicator() && !progressbar->indicatorFollowsStyle() &&
                 progressbar->progress()*2 >= progressbar->totalSteps())
                pcolor = &penColor;
            drawItem(p, r, AlignCenter | SingleLine, pal, flags & Style_Enabled,
                     progressbar->progressString(), -1, pcolor);
        }
        break;
#endif // QT_NO_PROGRESSBAR

    case CE_MenuTearoff: {
        if(flags & Style_Active)
            p->fillRect(r, pal.brush(QPalette::Highlight));
        else
            p->fillRect(r, pal.brush(QPalette::Button));
        p->setPen(QPen(pal.dark(), 1, DashLine));
        p->drawLine(r.x()+2, r.y()+r.height()/2-1, r.x()+r.width()-4, r.y()+r.height()/2-1);
        p->setPen(QPen(pal.light(), 1, DashLine));
        p->drawLine(r.x()+2, r.y()+r.height()/2, r.x()+r.width()-4, r.y()+r.height()/2);
        break; }

    case CE_MenuBarItem:
        {
            if (opt.isDefault())
                break;

            QAction *mi = opt.action();
            int alignment = AlignCenter|ShowPrefix|DontClip|SingleLine;
            if (!styleHint(SH_UnderlineShortcut, widget, QStyleOption::Default, 0))
                alignment |= NoAccel;
            QPixmap pix = mi->icon().pixmap(QIconSet::Small, QIconSet::Normal);
            drawItem(p, r, alignment, pal, flags & Style_Enabled, pix, mi->text(), -1,
                      &pal.buttonText().color());
            break;
        }

#ifndef QT_NO_TOOLBUTTON
    case CE_ToolButtonLabel:
        {
            const QToolButton *toolbutton = (const QToolButton *) widget;
            QRect rect = r;
            Qt::ArrowType arrowType = opt.isDefault()
                        ? Qt::DownArrow : opt.arrowType();

            int shiftX = 0;
            int shiftY = 0;
            if (flags & (Style_Down | Style_On)) {
                shiftX = pixelMetric(PM_ButtonShiftHorizontal, widget);
                shiftY = pixelMetric(PM_ButtonShiftVertical, widget);
            }

            if (!opt.isDefault()) {
                PrimitiveElement pe;
                switch (arrowType) {
                case Qt::LeftArrow:  pe = PE_ArrowLeft;  break;
                case Qt::RightArrow: pe = PE_ArrowRight; break;
                case Qt::UpArrow:    pe = PE_ArrowUp;    break;
                default:
                case Qt::DownArrow:  pe = PE_ArrowDown;  break;
                }

                rect.moveBy(shiftX, shiftY);
                drawPrimitive(pe, p, rect, pal, flags, opt);
            } else {
                QColor btext = toolbutton->palette().foreground();

                if (toolbutton->icon().isNull() &&
                    ! toolbutton->text().isNull() &&
                    ! toolbutton->usesTextLabel()) {
                    int alignment = AlignCenter | ShowPrefix;
                    if (!styleHint(SH_UnderlineShortcut, widget, QStyleOption::Default, 0))
                        alignment |= NoAccel;
                    rect.moveBy(shiftX, shiftY);
                    drawItem(p, rect, alignment, pal,
                             flags & Style_Enabled, toolbutton->text(), -1, &btext);
                } else {
                    QPixmap pm;
                    QIconSet::Size size =
                        toolbutton->usesBigPixmap() ? QIconSet::Large : QIconSet::Small;
                    QIconSet::State state =
                        toolbutton->isChecked() ? QIconSet::On : QIconSet::Off;
                    QIconSet::Mode mode;
                    if (! toolbutton->isEnabled())
                        mode = QIconSet::Disabled;
                    else if (flags & (Style_Down | Style_On) ||
                             ((flags & Style_Raised) && (flags & Style_AutoRaise)))
                        mode = QIconSet::Active;
                    else
                        mode = QIconSet::Normal;
                    pm = toolbutton->icon().pixmap(size, mode, state);

                    if (toolbutton->usesTextLabel()) {
                        p->setFont(toolbutton->font());
                        QRect pr = rect, tr = rect;
                        int alignment = ShowPrefix;
                        if (!styleHint(SH_UnderlineShortcut, widget, QStyleOption::Default, 0))
                            alignment |= NoAccel;

                        if (toolbutton->textPosition() == QToolButton::Under) {
                            int fh = p->fontMetrics().height();
                            pr.addCoords(0, 1, 0, -fh-3);
                            tr.addCoords(0, pr.bottom(), 0, -3);
                            pr.moveBy(shiftX, shiftY);
                            drawItem(p, pr, AlignCenter, pal,
                                      mode != QIconSet::Disabled || !toolbutton->icon().isGenerated(size, mode, state), pm);
                            alignment |= AlignCenter;
                        } else {
                            pr.setWidth(pm.width() + 8);
                            tr.addCoords(pr.right(), 0, 0, 0);
                            pr.moveBy(shiftX, shiftY);
                            drawItem(p, pr, AlignCenter, pal,
                                      mode != QIconSet::Disabled || !toolbutton->icon().isGenerated(size, mode, state), pm);
                            alignment |= AlignLeft | AlignVCenter;
                        }
                        tr.moveBy(shiftX, shiftY);
                        drawItem(p, tr, alignment, pal,
                                  flags & Style_Enabled, QPixmap(), toolbutton->text(),
                                  toolbutton->text().length(), &btext);
                    } else {
                        rect.moveBy(shiftX, shiftY);
                        drawItem(p, rect, AlignCenter, pal,
                                  mode != QIconSet::Disabled || !toolbutton->icon().isGenerated(size, mode, state), pm);
                    }
                }
            }

            break;
        }
#endif // QT_NO_TOOLBUTTON
    default:
        break;
    }

    activePainter = 0;
}


/*!
    Draws the control \a ce, with style options \a opt, on painter \a
    p, with parent widget \a widget.
*/
void QCommonStyle::drawControl(ControlElement ce, const Q4StyleOption *opt,
                               QPainter *p, const QWidget *widget) const
{
    switch (ce) {
    case CE_PushButton:
        if (const Q4StyleOptionButton *btn = qt_cast<const Q4StyleOptionButton *>(opt)) {
            QRect br = btn->rect;
            int dbi = pixelMetric(PM_ButtonDefaultIndicator, widget);
            if (btn->state & Style_ButtonDefault) {
                drawPrimitive(PE_ButtonDefault, opt, p, widget);
                br.setCoords(br.left() + dbi, br.top() + dbi, br.right() - dbi, br.bottom() - dbi);
            }
            if (!(btn->extras & Q4StyleOptionButton::Flat)
                || btn->state & (Style_Down | Style_On)) {
                Q4StyleOptionButton tmpBtn = *btn;
                tmpBtn.rect = br;
                drawPrimitive(PE_ButtonCommand, &tmpBtn, p, widget);
            }
            if (btn->extras & Q4StyleOptionButton::HasMenu) {
                int mbi = pixelMetric(PM_MenuButtonIndicator, widget);
                QRect ir = btn->rect;
                Q4StyleOptionButton newBtn = *btn;
                newBtn.rect = QRect(ir.right() - mbi, ir.height() - 20, mbi, ir.height() - 4);
                drawPrimitive(PE_ArrowDown, &newBtn, p, widget);
            }
        }
        break;
    case CE_PushButtonLabel:
        if (const Q4StyleOptionButton *btn = qt_cast<const Q4StyleOptionButton *>(opt)) {
            QRect ir = btn->rect;
            uint tf = AlignVCenter | ShowPrefix | NoAccel;
            if (!btn->icon.isNull()) {
                QIconSet::Mode mode = btn->state & Style_Enabled ? QIconSet::Normal
                                                                 : QIconSet::Disabled;
                if (mode == QIconSet::Normal && btn->state & Style_HasFocus)
                    mode = QIconSet::Active;
                QIconSet::State state = QIconSet::Off;
                if (btn->state & Style_On)
                    state = QIconSet::On;
                QPixmap pixmap = btn->icon.pixmap(QIconSet::Small, mode, state);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                //Center the icon if there is neither text nor pixmap
                if (btn->text.isEmpty())
                    p->drawPixmap(ir.x() + ir.width() / 2 - pixw / 2,
                                  ir.y() + ir.height() / 2 - pixh / 2, pixmap);
                else
                    p->drawPixmap(ir.x() + 2, ir.y() + ir.height() / 2 - pixh / 2, pixmap);
                ir.moveBy(pixw + 4, 0);
                ir.setWidth(ir.width() - (pixw + 4));
                // left-align text if there is
                if (!btn->text.isEmpty())
                    tf |= AlignLeft;
            } else {
                tf |= AlignHCenter;
            }
            drawItem(p, ir, tf, btn->palette, btn->state & Style_Enabled, QPixmap(), btn->text, -1,
                     &(btn->palette.buttonText().color()));
        }
        break;
    case CE_RadioButton:
    case CE_CheckBox:
        drawPrimitive(ce == CE_RadioButton ? PE_ExclusiveIndicator : PE_Indicator, opt, p, widget);
        break;
    case CE_RadioButtonLabel:
    case CE_CheckBoxLabel:
        if (const Q4StyleOptionButton *btn = qt_cast<const Q4StyleOptionButton *>(opt)) {
            bool isRadio = (ce == CE_RadioButtonLabel);
            uint alignment = QApplication::reverseLayout() ? AlignRight : AlignLeft;
            if (styleHint(SH_UnderlineShortcut, widget, QStyleOption::Default, 0))
                alignment |= NoAccel;
            QPixmap pix;
            if (btn->icon.isNull())
                pix = btn->icon.pixmap(QIconSet::Small, QIconSet::Normal);
            drawItem(p, btn->rect, alignment | AlignVCenter | ShowPrefix, btn->palette,
                     btn->state & Style_Enabled, pix, btn->text);
            if (btn->state & Style_HasFocus) {
                Q4StyleOptionFocusRect fropt(0);
                fropt.state = btn->state;
                fropt.palette = btn->palette;
                fropt.rect = visualRect(subRect(isRadio ? SR_RadioButtonFocusRect
                                                        : SR_CheckBoxFocusRect, widget), widget);
                drawPrimitive(PE_FocusRect, &fropt, p, widget);
            }
        }
        break;
    case CE_MenuTearoff:
        if (opt->state & Style_Active)
            p->fillRect(opt->rect, opt->palette.brush(QPalette::Highlight));
        else
            p->fillRect(opt->rect, opt->palette.brush(QPalette::Button));
        p->setPen(QPen(opt->palette.dark(), 1, DashLine));
        p->drawLine(opt->rect.x() + 2, opt->rect.y() + opt->rect.height() / 2 - 1,
                    opt->rect.x() + opt->rect.width() - 4,
                    opt->rect.y() + opt->rect.height() / 2 - 1);
        p->setPen(QPen(opt->palette.light(), 1, DashLine));
        p->drawLine(opt->rect.x() + 2, opt->rect.y() + opt->rect.height() / 2,
                    opt->rect.x() + opt->rect.width() - 4, opt->rect.y() + opt->rect.height() / 2);
        break;
    case CE_MenuBarItem:
        if (const Q4StyleOptionMenuItem *mbi = qt_cast<const Q4StyleOptionMenuItem *>(opt)) {
            uint alignment = AlignCenter | ShowPrefix | DontClip | SingleLine;
            if (!styleHint(SH_UnderlineShortcut, widget, QStyleOption(), 0))
                alignment |= NoAccel;
            QPixmap pix = mbi->icon.pixmap(QIconSet::Small, QIconSet::Normal);
            drawItem(p, mbi->rect, alignment, mbi->palette, mbi->state & Style_Enabled,
                     pix, mbi->text, -1, &mbi->palette.buttonText().color());
        }
        break;
    case CE_ProgressBarGroove:
        qDrawShadePanel(p, opt->rect, opt->palette, true, 1,
                        &opt->palette.brush(QPalette::Background));
        break;
    case CE_ProgressBarLabel:
        if (const Q4StyleOptionProgressBar *pb = qt_cast<const Q4StyleOptionProgressBar *>(opt)) {
            QColor penColor = pb->palette.highlightedText();
            QColor *pColor = 0;
            if (pb->extras & Q4StyleOptionProgressBar::CenterIndicator
                && !(pb->extras & Q4StyleOptionProgressBar::IndicatorFollowsStyle)
                && pb->progress * 2 >= pb->totalSteps)
                pColor = &penColor;
            drawItem(p, pb->rect, AlignCenter | SingleLine, opt->palette,
                     opt->state & Style_Enabled, pb->progressString, -1, pColor);
        }
        break;
    case CE_ProgressBarContents:
        if (const Q4StyleOptionProgressBar *pb = qt_cast<const Q4StyleOptionProgressBar *>(opt)) {
            QPalette pal2 = pb->palette;
            // Correct the highlight color if it is the same as the background
            if (pal2.highlight() == pal2.background())
                pal2.setColor(QPalette::Highlight, pb->palette.color(QPalette::Active,
                                                                     QPalette::Highlight));
            bool reverse = QApplication::reverseLayout();
            int fw = 2;
            int w = pb->rect.width() - 2 * fw;
            if (!pb->totalSteps) {
                // draw busy indicator
                int x = pb->progress % (w * 2);
                if (x > w)
                    x = 2 * w - x;
                x = reverse ? pb->rect.right() - x : x + pb->rect.x();
                p->setPen(QPen(pal2.highlight(), 4));
                p->drawLine(x, pb->rect.y() + 1, x, pb->rect.height() - fw);
            } else {
                const int unit_width = pixelMetric(PM_ProgressBarChunkWidth, widget);
                int u;
                if (unit_width > 1)
                    u = (pb->rect.width() + unit_width / 3) / unit_width;
                else
                    u = w / unit_width;
                int p_v = pb->progress;
                int t_s = pb->totalSteps ? pb->totalSteps : 1;

                if (u > 0 && p_v >= INT_MAX / u && t_s >= u) {
                    // scale down to something usable.
                    p_v /= u;
                    t_s /= u;
                }

                // nu < tnu, if last chunk is only a partial chunk
                int tnu, nu;
                tnu = nu = p_v * u / t_s;

                if (nu * unit_width > w)
                    --nu;

                // Draw nu units out of a possible u of unit_width
                // width, each a rectangle bordered by background
                // color, all in a sunken panel with a percentage text
                // display at the end.
                int x = 0;
                int x0 = reverse ? pb->rect.right() - ((unit_width > 1) ? unit_width : fw)
                                 : pb->rect.x() + fw;
                Q4StyleOptionProgressBar pbBits = *pb;
                pbBits.palette = pal2;
                int myY = pbBits.rect.y();
                int myHeight = pbBits.rect.height();
                pbBits.state = Style_Default;
                for (int i = 0; i < nu; ++i) {
                    pbBits.rect.setRect(x0 + x, myY, unit_width, myHeight);
                    drawPrimitive(PE_ProgressBarChunk, &pbBits, p, widget);
                    x += reverse ? -unit_width : unit_width;
                }

                // Draw the last partial chunk to fill up the
                // progressbar entirely
                if (nu < tnu) {
                    int pixels_left = w - (nu * unit_width);
                    int offset = reverse ? x0 + x + unit_width-pixels_left : x0 + x;
                    pbBits.rect.setRect(offset, myY, pixels_left, myHeight);
                    drawPrimitive(PE_ProgressBarChunk, &pbBits, p, widget);
                }
            }
        }
        break;
    case CE_HeaderLabel:
        if (const Q4StyleOptionHeader *header = qt_cast<const Q4StyleOptionHeader *>(opt)) {
            QRect rect = header->rect;
            if (!header->icon.isNull()) {
                QPixmap pixmap
                    = header->icon.pixmap(QIconSet::Small,
                                          header->state & Style_Enabled ? QIconSet::Normal
                                                                        : QIconSet::Disabled);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                // "pixh - 1" because of tricky integer division

                QRect pixRect = rect;
                pixRect.setY(rect.center().y() - (pixh - 1) / 2);
                drawItem(p, pixRect, AlignVCenter, header->palette, (header->state & Style_Enabled)
                         || !header->icon.isGenerated(QIconSet::Small, QIconSet::Disabled), pixmap);
                rect.setLeft(rect.left() + pixw + 2);
            }

            drawItem(p, rect, AlignVCenter, header->palette, header->state & Style_Enabled,
                     header->text, -1, &(header->palette.buttonText().color()));
        }
        break;
    default:
        qWarning("QCommonStyle::drawControl not currently handled %d", ce);
    }
}

/*!
    Draws the mask for the given control \a ce, with style options
    \a opt, on painter \a p, with parent widget \a w.
*/
void QCommonStyle::drawControlMask(ControlElement ce, const Q4StyleOption *opt, QPainter *p,
                                   const QWidget *w) const
{
    QPalette pal(color1,color1,color1,color1,color1,color1,color1,color1,color0);
    switch (ce) {
    case CE_PushButton:
        if (const Q4StyleOptionButton *btn = qt_cast<const Q4StyleOptionButton *>(opt)) {
            Q4StyleOptionButton newBtn = *btn;
            newBtn.palette = pal;
            drawPrimitive(PE_ButtonCommand, &newBtn, p, w);
        }
        break;
    default:
        p->fillRect(opt->rect, color1);
    }

}

/*! \reimp */
void QCommonStyle::drawControlMask(ControlElement control,
                                    QPainter *p,
                                    const QWidget *widget,
                                    const QRect &r,
                                    const QStyleOption& opt) const
{
    Q_UNUSED(widget);

    activePainter = p;

    QPalette pal(color1,color1,color1,color1,color1,color1,color1,color1,color0);

    switch (control) {
    case CE_PushButton:
        drawPrimitive(PE_ButtonCommand, p, r, pal, Style_Default, opt);
        break;

    case CE_CheckBox:
        drawPrimitive(PE_IndicatorMask, p, r, pal, Style_Default, opt);
        break;

    case CE_RadioButton:
        drawPrimitive(PE_ExclusiveIndicatorMask, p, r, pal, Style_Default, opt);
        break;

    default:
        p->fillRect(r, color1);
        break;
    }

    activePainter = 0;
}

/*! \reimp */
QRect QCommonStyle::subRect(SubRect r, const QWidget *widget) const
{
    if (! widget) {
        qWarning("QCommonStyle::subRect: widget parameter cannot be zero!");
        return QRect();
    }

    QRect rect, wrect(widget->rect());

    switch (r) {
#ifndef QT_NO_DIALOGBUTTONS
    case SR_DialogButtonAbort:
    case SR_DialogButtonRetry:
    case SR_DialogButtonIgnore:
    case SR_DialogButtonAccept:
    case SR_DialogButtonReject:
    case SR_DialogButtonApply:
    case SR_DialogButtonHelp:
    case SR_DialogButtonAll:
    case SR_DialogButtonCustom: {
        QDialogButtons::Button srch = QDialogButtons::None;
        if(r == SR_DialogButtonAccept)
            srch = QDialogButtons::Accept;
        else if(r == SR_DialogButtonReject)
            srch = QDialogButtons::Reject;
        else if(r == SR_DialogButtonAll)
            srch = QDialogButtons::All;
        else if(r == SR_DialogButtonApply)
            srch = QDialogButtons::Apply;
        else if(r == SR_DialogButtonHelp)
            srch = QDialogButtons::Help;
        else if(r == SR_DialogButtonRetry)
            srch = QDialogButtons::Retry;
        else if(r == SR_DialogButtonIgnore)
            srch = QDialogButtons::Ignore;
        else if(r == SR_DialogButtonAbort)
            srch = QDialogButtons::Abort;

        const int bwidth = pixelMetric(PM_DialogButtonsButtonWidth, widget),
                 bheight = pixelMetric(PM_DialogButtonsButtonHeight, widget),
                  bspace = pixelMetric(PM_DialogButtonsSeparator, widget),
                      fw = pixelMetric(PM_DefaultFrameWidth, widget);
        const QDialogButtons *dbtns = (const QDialogButtons *) widget;
        int start = fw;
        if(dbtns->orientation() == Horizontal)
            start = wrect.right() - fw;
        QDialogButtons::Button btns[] = { QDialogButtons::All, QDialogButtons::Reject, QDialogButtons::Accept, //reverse order (right to left)
                                          QDialogButtons::Apply, QDialogButtons::Retry, QDialogButtons::Ignore, QDialogButtons::Abort,
                                          QDialogButtons::Help };
        for(unsigned int i = 0, cnt = 0; i < (sizeof(btns)/sizeof(btns[0])); i++) {
            if(dbtns->isButtonVisible(btns[i])) {
                QSize szH = dbtns->sizeHint(btns[i]);
                int mwidth = qMax(bwidth, szH.width()), mheight = qMax(bheight, szH.height());
                if(dbtns->orientation() == Horizontal) {
                    start -= mwidth;
                    if(cnt)
                        start -= bspace;
                } else if(cnt) {
                    start += mheight;
                    start += bspace;
                }
                cnt++;
                if(btns[i] == srch) {
                    if(dbtns->orientation() == Horizontal)
                        return QRect(start, wrect.bottom() - fw - mheight, mwidth, mheight);
                    else
                        return QRect(fw, start, mwidth, mheight);
                }
            }
        }
        if(r == SR_DialogButtonCustom) {
            if(dbtns->orientation() == Horizontal)
                return QRect(fw, fw, start - fw - bspace, wrect.height() - (fw*2));
            else
                return QRect(fw, start, wrect.width() - (fw*2), wrect.height() - start - (fw*2));
        }
        return QRect(); }
#endif //QT_NO_DIALOGBUTTONS
    case SR_PushButtonContents:
        {
#ifndef QT_NO_PUSHBUTTON
            const QPushButton *button = (const QPushButton *) widget;
            int dx1, dx2;

            dx1 = pixelMetric(PM_DefaultFrameWidth, widget);
            if (button->isDefault() || button->autoDefault())
                dx1 += pixelMetric(PM_ButtonDefaultIndicator, widget);
            dx2 = dx1 * 2;

            rect.setRect(wrect.x()      + dx1,
                         wrect.y()      + dx1,
                         wrect.width()  - dx2,
                         wrect.height() - dx2);
#endif
            break;
        }

    case SR_PushButtonFocusRect:
        {
#ifndef QT_NO_PUSHBUTTON
            const QPushButton *button = (const QPushButton *) widget;
            int dbw1 = 0, dbw2 = 0;
            if (button->isDefault() || button->autoDefault()) {
                dbw1 = pixelMetric(PM_ButtonDefaultIndicator, widget);
                dbw2 = dbw1 * 2;
            }

            int dfw1 = pixelMetric(PM_DefaultFrameWidth, widget) * 2,
                dfw2 = dfw1 * 2;

            rect.setRect(wrect.x()      + dfw1 + dbw1,
                         wrect.y()      + dfw1 + dbw1,
                         wrect.width()  - dfw2 - dbw2,
                         wrect.height() - dfw2 - dbw2);
#endif
            break;
        }

    case SR_CheckBoxIndicator:
        {
            int h = pixelMetric(PM_IndicatorHeight, widget);
            rect.setRect(0, (wrect.height() - h) / 2,
                         pixelMetric(PM_IndicatorWidth, widget), h);
            break;
        }

    case SR_CheckBoxContents:
        {
#ifndef QT_NO_CHECKBOX
            QRect ir = subRect(SR_CheckBoxIndicator, widget);
            rect.setRect(ir.right() + 6, wrect.y(),
                         wrect.width() - ir.width() - 6, wrect.height());
#endif
            break;
        }

    case SR_CheckBoxFocusRect:
        {
#ifndef QT_NO_CHECKBOX
            const QCheckBox *checkbox = (const QCheckBox *) widget;
            if (checkbox->text().isEmpty()) {
                rect = subRect(SR_CheckBoxIndicator, widget);
                rect.addCoords(1, 1, -1, -1);
                break;
            }
            QRect cr = subRect(SR_CheckBoxContents, widget);

            if(!checkbox->icon().isNull()) {
                rect = itemRect(cr, AlignLeft | AlignVCenter | ShowPrefix,
                                checkbox->icon().pixmap(QIconSet::Small, QIconSet::Normal));
            } else {
                QFontMetrics fm = widget->fontMetrics();
                rect = itemRect(fm, cr, AlignLeft | AlignVCenter | ShowPrefix,
                                checkbox->isEnabled(), checkbox->text());
            }


            rect.addCoords(-3, -2, 3, 2);
            rect = rect.intersect(wrect);
#endif
            break;
        }

    case SR_RadioButtonIndicator:
        {
            int h = pixelMetric(PM_ExclusiveIndicatorHeight, widget);
            rect.setRect(0, (wrect.height() - h) / 2,
                         pixelMetric(PM_ExclusiveIndicatorWidth, widget), h);
            break;
        }

    case SR_RadioButtonContents:
        {
            QRect ir = subRect(SR_RadioButtonIndicator, widget);
            rect.setRect(ir.right() + 6, wrect.y(),
                         wrect.width() - ir.width() - 6, wrect.height());
            break;
        }

    case SR_RadioButtonFocusRect:
        {
#ifndef QT_NO_RADIOBUTTON
            const QRadioButton *radiobutton = (const QRadioButton *) widget;
            if (!radiobutton->icon().isNull() && radiobutton->text().isEmpty()) {
                rect = subRect(SR_RadioButtonIndicator, widget);
                rect.addCoords(1, 1, -1, -1);
                break;
            }
            QRect cr = subRect(SR_RadioButtonContents, widget);

            if(!radiobutton->icon().isNull()) {
                rect = itemRect(cr, AlignLeft | AlignVCenter | ShowPrefix,
                                radiobutton->icon().pixmap(QIconSet::Small, QIconSet::Normal));
            } else {
                QFontMetrics fm = widget->fontMetrics();
                rect = itemRect(fm, cr, AlignLeft | AlignVCenter | ShowPrefix,
                                radiobutton->isEnabled(),
                                radiobutton->text());
            }

            rect.addCoords(-3, -2, 3, 2);
            rect = rect.intersect(wrect);
#endif
            break;
        }

    case SR_ComboBoxFocusRect:
        rect.setRect(3, 3, widget->width()-6-16, widget->height()-6);
        break;

#ifndef QT_NO_SLIDER
    case SR_SliderFocusRect:
        {
            const QSlider * sl = (const QSlider *) widget;
            int tickOffset = pixelMetric(PM_SliderTickmarkOffset, sl);
            int thickness  = pixelMetric(PM_SliderControlThickness, sl);

            if (sl->orientation() == Horizontal)
                rect.setRect(0, tickOffset-1, sl->width(), thickness+2);
            else
                rect.setRect(tickOffset-1, 0, thickness+2, sl->height());
            rect = rect.intersect(sl->rect()); // ## is this really necessary?
            break;
        }
#endif // QT_NO_SLIDER


    case SR_ToolButtonContents:
        rect = querySubControlMetrics(CC_ToolButton, widget, SC_ToolButton);
        break;

    case SR_ToolBoxTabContents:
        rect = wrect;
        rect.addCoords(0, 0, -30, 0);
        break;

    default:
        rect = wrect;
        break;
    }

    return rect;
}

/*!
    Returns the rectangle occupied by sub-rectangle \a sr, with style
    options \a opt, and parent widget \a w.
*/
QRect QCommonStyle::subRect(SubRect sr, const Q4StyleOption *opt, const QWidget *w) const
{
    QRect r;
    switch (sr) {
    case SR_PushButtonContents:
        if (const Q4StyleOptionButton *btn = qt_cast<const Q4StyleOptionButton *>(opt)) {
            int dx1, dx2;
            dx1 = pixelMetric(PM_DefaultFrameWidth, w);
            if (btn->state & Style_ButtonDefault)
                dx1 += pixelMetric(PM_ButtonDefaultIndicator, w);
            dx2 = dx1 * 2;
            r.setRect(opt->rect.x() + dx1, opt->rect.y() + dx1, opt->rect.width() - dx2,
                      opt->rect.height() - dx2);
        }
        break;
    case SR_PushButtonFocusRect:
        if (const Q4StyleOptionButton *btn = qt_cast<const Q4StyleOptionButton *>(opt)) {
            int dbw1 = 0, dbw2 = 0;
            if (btn->state & Style_ButtonDefault) {
                dbw1 = pixelMetric(PM_ButtonDefaultIndicator, w);
                dbw2 = dbw1 * 2;
            }

            int dfw1 = pixelMetric(PM_DefaultFrameWidth, w) * 2,
                dfw2 = dfw1 * 2;

            r.setRect(btn->rect.x() + dfw1 + dbw1, btn->rect.y() + dfw1 + dbw1,
                      btn->rect.width() - dfw2 - dbw2, btn->rect.height()- dfw2 - dbw2);
        }
        break;

    case SR_CheckBoxIndicator:
        {
            int h = pixelMetric(PM_IndicatorHeight, w);
            r.setRect(0, (opt->rect.height() - h) / 2,
                      pixelMetric(PM_IndicatorWidth, w), h);
        }
        break;

    case SR_CheckBoxContents:
        {
            QRect ir = subRect(SR_CheckBoxIndicator, opt, w);
            r.setRect(ir.right() + 6, opt->rect.y(), opt->rect.width() - ir.width() - 6,
                      opt->rect.height());
        }
        break;

    case SR_CheckBoxFocusRect:
        if (const Q4StyleOptionButton *btn = qt_cast<const Q4StyleOptionButton *>(opt)) {
            if (btn->text.isEmpty()) {
                r = subRect(SR_CheckBoxIndicator, opt, w);
                r.addCoords(1, 1, -1, -1);
                break;
            }
            QRect cr = subRect(SR_CheckBoxContents, opt, w);

            if (!btn->icon.isNull()) {
                r = itemRect(cr, AlignLeft | AlignVCenter | ShowPrefix,
                             btn->icon.pixmap(QIconSet::Small, QIconSet::Normal));
            } else {
                QFontMetrics fm = w->fontMetrics();
                r = itemRect(fm, cr, AlignLeft | AlignVCenter | ShowPrefix,
                             btn->state & Style_Enabled, btn->text);
            }
            r.addCoords(-3, -2, 3, 2);
            r = r.intersect(btn->rect);
        }
        break;

    case SR_RadioButtonIndicator:
        {
            int h = pixelMetric(PM_ExclusiveIndicatorHeight, w);
            r.setRect(0, (opt->rect.height() - h) / 2,
                    pixelMetric(PM_ExclusiveIndicatorWidth, w), h);
        }
        break;

    case SR_RadioButtonContents:
        {
            QRect ir = subRect(SR_RadioButtonIndicator, w);
            r.setRect(ir.right() + 6, opt->rect.y(),
                      opt->rect.width() - ir.width() - 6, opt->rect.height());
            break;
        }

    case SR_RadioButtonFocusRect:
        if (const Q4StyleOptionButton *btn = qt_cast<const Q4StyleOptionButton *>(opt)) {
            if (!btn->icon.isNull() && btn->text.isEmpty()) {
                r = subRect(SR_RadioButtonIndicator, opt, w);
                r.addCoords(1, 1, -1, -1);
                break;
            }
            QRect cr = subRect(SR_RadioButtonContents, w);

            if(!btn->icon.isNull()) {
                r = itemRect(cr, AlignLeft | AlignVCenter | ShowPrefix,
                             btn->icon.pixmap(QIconSet::Small, QIconSet::Normal));
            } else {
                QFontMetrics fm = w->fontMetrics();
                r = itemRect(fm, cr, AlignLeft | AlignVCenter | ShowPrefix,
                             btn->state & Style_Enabled, btn->text);
            }
            r.addCoords(-3, -2, 3, 2);
            r = r.intersect(btn->rect);
        }
        break;
    case SR_SliderFocusRect:
        if (const Q4StyleOptionSlider *slider = qt_cast<const Q4StyleOptionSlider *>(opt)) {
            int tickOffset = pixelMetric(PM_SliderTickmarkOffset, w);
            int thickness  = pixelMetric(PM_SliderControlThickness, w);
            if (slider->orientation == Horizontal)
                r.setRect(0, tickOffset - 1, slider->rect.width(), thickness + 2);
            else
                r.setRect(tickOffset - 1, 0, thickness + 2, slider->rect.height());
            r = r.intersect(slider->rect);
        }
        break;
    case SR_ProgressBarGroove:
    case SR_ProgressBarContents:
    case SR_ProgressBarLabel:
        if (const Q4StyleOptionProgressBar *pb = qt_cast<const Q4StyleOptionProgressBar *>(opt)) {
            QFontMetrics fm(w ? w->fontMetrics() : QApplication::fontMetrics());
            int textw = 0;
            if (pb->extras & Q4StyleOptionProgressBar::PercentageVisible)
                textw = fm.width("100%") + 6;

            if (pb->extras & Q4StyleOptionProgressBar::IndicatorFollowsStyle
                || !(pb->extras & Q4StyleOptionProgressBar::CenterIndicator)) {
                if (sr != SR_ProgressBarLabel)
                    r.setCoords(pb->rect.left(), pb->rect.top(),
                                pb->rect.right() - textw, pb->rect.bottom());
                else
                    r.setCoords(pb->rect.right() - textw, pb->rect.top(),
                                pb->rect.right(), pb->rect.bottom());
            }
            else
                r = pb->rect;
        }
        break;
    case SR_DockWindowHandleRect:
        if (const Q4StyleOptionDockWindow *dw = qt_cast<const Q4StyleOptionDockWindow *>(opt)) {
            if (!dw->docked || !dw->isCloseEnabled)
                r.setRect(0, 0, dw->rect.width(), dw->rect.height());
            else {
                if (dw->state & Style_Horizontal)
                    r.setRect(0, 15, dw->rect.width(), dw->rect.height() - 15);
                else
                    r.setRect(0, 1, dw->rect.width() - 15, dw->rect.height() - 1);
            }
        }
        break;
    default:
        qWarning("QCommonStyle::SubRect case not handled %d", sr);
    }
    return r;
}

/*!
    Draws the complex control \a cc, with style options \a opt, on painter
    \a p, with parent widget \a widget.
*/
void QCommonStyle::drawComplexControl(ComplexControl cc, const Q4StyleOptionComplex *opt,
                                      QPainter *p, const QWidget *widget) const
{
    switch (cc) {
    case CC_Slider:
        if (const Q4StyleOptionSlider *slider = qt_cast<const Q4StyleOptionSlider *>(opt)) {
            if (slider->parts == SC_SliderTickmarks) {
                int tickOffset = pixelMetric(PM_SliderTickmarkOffset, widget);
                int ticks = slider->tickmarks;
                int thickness = pixelMetric(PM_SliderControlThickness, widget);
                int len = pixelMetric(PM_SliderLength, widget);
                int available = pixelMetric(PM_SliderSpaceAvailable, widget);
                int interval = slider->tickInterval;
                if (interval <= 0) {
                    interval = slider->singleStep;
                    if (QStyle::positionFromValue(slider->minimum, slider->maximum, interval,
                                                  available)
                            - QStyle::positionFromValue(slider->minimum, slider->maximum,
                                                        0, available) < 3)
                        interval = slider->pageStep;
                }
                if (!interval)
                    interval = 1;
                int fudge = len / 2;
                int pos;
                if (slider->orientation == Horizontal) {
                    if (ticks & QSlider::Above)
                        p->fillRect(0, 0, slider->rect.width(), tickOffset,
                                    slider->palette.brush(QPalette::Background));
                    if (ticks & QSlider::Below)
                        p->fillRect(0, tickOffset + thickness, slider->rect.width(), tickOffset,
                                    slider->palette.brush(QPalette::Background));
                } else {
                    if (ticks & QSlider::Above)
                        p->fillRect(0, 0, tickOffset, slider->rect.width(),
                                    slider->palette.brush(QPalette::Background));
                    if (ticks & QSlider::Below)
                        p->fillRect(tickOffset + thickness, 0, tickOffset, slider->rect.height(),
                                    slider->palette.brush(QPalette::Background));
                }
                p->setPen(slider->palette.foreground());
                int v = slider->minimum;
                while (v <= slider->maximum + 1) {
                    pos = QStyle::positionFromValue(slider->minimum, slider->maximum,
                            v, available) + fudge;
                    if (slider->orientation == Horizontal) {
                        if (ticks & QSlider::Above)
                            p->drawLine(pos, 0, pos, tickOffset - 2);
                        if (ticks & QSlider::Below)
                            p->drawLine(pos, tickOffset + thickness + 1, pos,
                                        tickOffset + thickness + 1 + available - 2);
                    } else {
                        if (ticks & QSlider::Above)
                            p->drawLine(0, pos, tickOffset - 2, pos);
                        if (ticks & QSlider::Below)
                            p->drawLine(tickOffset + thickness + 1, pos,
                                        tickOffset + thickness + 1 + available - 2, pos);
                    }
                    v += interval;
                }
            }
        }
        break;
    case CC_ScrollBar:
        if (const Q4StyleOptionSlider *scrollbar = qt_cast<const Q4StyleOptionSlider *>(opt)) {
            // Since we really get this thing is a const it is not correct to be making
            // changes to it. So make a copy here and reset it for each primitive.
            Q4StyleOptionSlider newScrollbar = *scrollbar;
            SFlags saveFlags = scrollbar->state;
            if (scrollbar->minimum == scrollbar->maximum)
                saveFlags |= Style_Enabled;

            if (scrollbar->parts & SC_ScrollBarSubLine) {
                newScrollbar.state = saveFlags;
                newScrollbar.parts = SC_ScrollBarSubLine;
                newScrollbar.rect = visualRect(querySubControlMetrics(cc, &newScrollbar, widget),
                                               widget);
                if (newScrollbar.rect.isValid()) {
                    if (scrollbar->activeParts & SC_ScrollBarSubLine)
                        newScrollbar.state |= Style_Down;
                    drawPrimitive(PE_ScrollBarSubLine, &newScrollbar, p, widget);
                }
            }
            if (scrollbar->parts & SC_ScrollBarAddLine) {
                newScrollbar.rect = scrollbar->rect;
                newScrollbar.state = saveFlags;
                newScrollbar.parts = SC_ScrollBarAddLine;
                newScrollbar.rect = visualRect(querySubControlMetrics(cc, &newScrollbar, widget),
                                               widget);
                if (newScrollbar.rect.isValid()) {
                    if (scrollbar->activeParts & SC_ScrollBarAddLine)
                        newScrollbar.state |= Style_Down;
                    drawPrimitive(PE_ScrollBarAddLine, &newScrollbar, p, widget);
                }
            }
            if (scrollbar->parts & SC_ScrollBarSubPage) {
                newScrollbar.rect = scrollbar->rect;
                newScrollbar.state = saveFlags;
                newScrollbar.parts = SC_ScrollBarSubPage;
                newScrollbar.rect = visualRect(querySubControlMetrics(cc, &newScrollbar, widget),
                                               widget);
                if (newScrollbar.rect.isValid()) {
                    if (scrollbar->activeParts & SC_ScrollBarSubPage)
                        newScrollbar.state |= Style_Down;
                    drawPrimitive(PE_ScrollBarSubPage, &newScrollbar, p, widget);
                }
            }
            if (scrollbar->parts & SC_ScrollBarAddPage) {
                newScrollbar.rect = scrollbar->rect;
                newScrollbar.state = saveFlags;
                newScrollbar.parts = SC_ScrollBarAddPage;
                newScrollbar.rect = visualRect(querySubControlMetrics(cc, &newScrollbar, widget),
                                               widget);
                if (newScrollbar.rect.isValid()) {
                    if (scrollbar->activeParts & SC_ScrollBarAddPage)
                        newScrollbar.state |= Style_Down;
                    drawPrimitive(PE_ScrollBarAddPage, &newScrollbar, p, widget);
                }
            }
            if (scrollbar->parts & SC_ScrollBarFirst) {
                newScrollbar.rect = scrollbar->rect;
                newScrollbar.state = saveFlags;
                newScrollbar.parts = SC_ScrollBarFirst;
                newScrollbar.rect = visualRect(querySubControlMetrics(cc, &newScrollbar, widget),
                                               widget);
                if (newScrollbar.rect.isValid()) {
                    if (scrollbar->activeParts & SC_ScrollBarFirst)
                        newScrollbar.state |= Style_Down;
                    drawPrimitive(PE_ScrollBarFirst, &newScrollbar, p, widget);
                }
            }
            if (scrollbar->parts & SC_ScrollBarLast) {
                newScrollbar.rect = scrollbar->rect;
                newScrollbar.state = saveFlags;
                newScrollbar.parts = SC_ScrollBarLast;
                newScrollbar.rect = visualRect(querySubControlMetrics(cc, &newScrollbar, widget),
                                               widget);
                if (newScrollbar.rect.isValid()) {
                    if (scrollbar->activeParts & SC_ScrollBarLast)
                        newScrollbar.state |= Style_Down;
                    drawPrimitive(PE_ScrollBarLast, &newScrollbar, p, widget);
                }
            }
            if (scrollbar->parts & SC_ScrollBarSlider) {
                newScrollbar.rect = scrollbar->rect;
                newScrollbar.state = saveFlags;
                newScrollbar.parts = SC_ScrollBarSlider;
                newScrollbar.rect = visualRect(querySubControlMetrics(cc, &newScrollbar, widget),
                                               widget);
                if (newScrollbar.rect.isValid()) {
                    if (scrollbar->activeParts & SC_ScrollBarSlider)
                        newScrollbar.state |= Style_Down;
                    drawPrimitive(PE_ScrollBarSlider, &newScrollbar, p, widget);

                    if (scrollbar->state & Style_HasFocus) {
                        Q4StyleOptionFocusRect fropt(0);
                        fropt.rect.setRect(newScrollbar.rect.x() + 2, newScrollbar.rect.y() + 2,
                                newScrollbar.rect.width() - 5, newScrollbar.rect.height() - 5);
                        fropt.palette = newScrollbar.palette;
                        fropt.state = Style_Default;
                        drawPrimitive(PE_FocusRect, &fropt, p, widget);
                    }
                }
            }
        }
        break;
    case CC_ListView:
        if (const Q4StyleOptionListView *lv = qt_cast<const Q4StyleOptionListView *>(opt)) {
            if (lv->parts & SC_ListView)
                p->fillRect(lv->rect, lv->viewportPalette.brush(lv->viewportBGRole));
        }
        break;
    default:
        qWarning("drawComplexControl control not handled %d", cc);
    }
}

/*!
    Draws the mask for the given complex control \a cc, with style
    options \a opt, on painter \a p, with parent widget \a w.
*/
void QCommonStyle::drawComplexControlMask(ComplexControl , const Q4StyleOptionComplex *opt,
                                          QPainter *p, const QWidget *) const
{
    p->fillRect(opt->rect, color1);
}

/*!
    Returns the sub-widget in the complex control \a cc, with style
    options \a opt, at point \a pt, and with parent widget \a widget.

    \sa querySubControlMetrics()
*/
QStyle::SubControl QCommonStyle::querySubControl(ComplexControl cc, const Q4StyleOptionComplex *opt,
                                                 const QPoint &pt, const QWidget *widget) const
{
    SubControl sc = SC_None;
    switch (cc) {
    case CC_Slider:
        if (const Q4StyleOptionSlider *slider = qt_cast<const Q4StyleOptionSlider *>(opt)) {
            Q4StyleOptionSlider tmpSlider = *slider;
            tmpSlider.parts = SC_SliderHandle;
            QRect r = visualRect(querySubControlMetrics(cc, &tmpSlider, widget), widget);
            if (r.isValid() && r.contains(pt)) {
                sc = SC_SliderHandle;
            } else {
                tmpSlider.parts = SC_SliderGroove;
                r = visualRect(querySubControlMetrics(cc, &tmpSlider, widget), widget);
                if (r.isValid() && r.contains(pt))
                    sc = SC_SliderGroove;
            }
        }
        break;
    case CC_ScrollBar:
        if (const Q4StyleOptionSlider *scrollbar = qt_cast<const Q4StyleOptionSlider *>(opt)) {
            Q4StyleOptionSlider tmpScrollbar = *scrollbar;
            QRect r;
            uint ctrl = SC_ScrollBarAddLine;
            while (sc == SC_None && ctrl <= SC_ScrollBarGroove) {
                tmpScrollbar.parts = (QStyle::SubControl)ctrl;
                r = visualRect(querySubControlMetrics(cc, &tmpScrollbar, widget), widget);
                if (r.isValid() && r.contains(pt)) {
                    sc = (QStyle::SubControl)ctrl;
                    break;
                }
                ctrl <<= 1;
            }
        }
        break;
    case CC_ListView:
        if (const Q4StyleOptionListView *lv = qt_cast<const Q4StyleOptionListView *>(opt)) {
            if (pt.x() >= 0 && pt.x() < lv->treeStepSize)
                sc = SC_ListViewExpand;
        }
        break;
    default:
        qWarning("QCommonStyle::querySubControl case not handled %d", cc);
    }
    return sc;
}

/*!
    Returns the rectangle occupied by the complex control \a cc, with
    style options \a opt, and with parent widget \a widget.

    \sa querySubControl()
*/
QRect QCommonStyle::querySubControlMetrics(ComplexControl cc, const Q4StyleOptionComplex *opt,
                                           const QWidget *widget) const
{
    QRect ret;
    switch (cc) {
    case CC_Slider:
        if (const Q4StyleOptionSlider *slider = qt_cast<const Q4StyleOptionSlider *>(opt)) {
            int tickOffset = pixelMetric(PM_SliderTickmarkOffset, widget);
            int thickness = pixelMetric(PM_SliderControlThickness, widget);

            switch (slider->parts) {
            case SC_SliderHandle: {
                int sliderPos = 0;
                int len = pixelMetric(PM_SliderLength, widget);
                bool horizontal = slider->orientation == Horizontal;
                sliderPos = positionFromValue(slider->minimum, slider->maximum,
                                              slider->sliderPosition,
                                              (horizontal ? slider->rect.width()
                                                          : slider->rect.height()) - len,
                                              slider->useRightToLeft);
                if (horizontal)
                    ret.setRect(sliderPos, tickOffset, len, thickness);
                else
                    ret.setRect(tickOffset, sliderPos, thickness, len);
                break; }
            case SC_SliderGroove:
                if (slider->orientation == Horizontal)
                    ret.setRect(0, tickOffset, slider->rect.width(), thickness);
                else
                    ret.setRect(tickOffset, 0, thickness, slider->rect.height());
                break;
            default:
                break;
            }
        }
        break;
    case CC_ScrollBar:
        if (const Q4StyleOptionSlider *scrollbar = qt_cast<const Q4StyleOptionSlider *>(opt)) {
            int sbextent = pixelMetric(PM_ScrollBarExtent, widget);
            int maxlen = ((scrollbar->orientation == Qt::Horizontal) ?
                    scrollbar->rect.width() : scrollbar->rect.height()) - (sbextent * 2);
            int sliderlen;

            // calculate slider length
            if (scrollbar->maximum != scrollbar->minimum) {
                uint range = scrollbar->maximum - scrollbar->minimum;
                sliderlen = (scrollbar->pageStep * maxlen) / (range + scrollbar->pageStep);

                int slidermin = pixelMetric(PM_ScrollBarSliderMin, widget);
                if (sliderlen < slidermin || range > INT_MAX / 2)
                    sliderlen = slidermin;
                if (sliderlen > maxlen)
                    sliderlen = maxlen;
            } else {
                sliderlen = maxlen;
            }

            int sliderstart = sbextent + positionFromValue(scrollbar->minimum, scrollbar->maximum,
                                                           scrollbar->sliderPosition,
                                                           maxlen - sliderlen,
                                                           scrollbar->useRightToLeft);
            switch (scrollbar->parts) {
            case SC_ScrollBarSubLine:            // top/left button
                if (scrollbar->orientation == Qt::Horizontal) {
                    int buttonWidth = qMin(scrollbar->rect.width() / 2, sbextent);
                    ret.setRect(0, 0, buttonWidth, sbextent);
                } else {
                    int buttonHeight = qMin(scrollbar->rect.height() / 2, sbextent);
                    ret.setRect(0, 0, sbextent, buttonHeight);
                }
                break;
            case SC_ScrollBarAddLine:            // bottom/right button
                if (scrollbar->orientation == Qt::Horizontal) {
                    int buttonWidth = qMin(scrollbar->rect.width()/2, sbextent);
                    ret.setRect(scrollbar->rect.width() - buttonWidth, 0, buttonWidth, sbextent);
                } else {
                    int buttonHeight = qMin(scrollbar->rect.height()/2, sbextent);
                    ret.setRect(0, scrollbar->rect.height() - buttonHeight, sbextent, buttonHeight);
                }
                break;
            case SC_ScrollBarSubPage:            // between top/left button and slider
                if (scrollbar->orientation == Qt::Horizontal)
                    ret.setRect(sbextent, 0, sliderstart - sbextent, sbextent);
                else
                    ret.setRect(0, sbextent, sbextent, sliderstart - sbextent);
                break;
            case SC_ScrollBarAddPage:            // between bottom/right button and slider
                if (scrollbar->orientation == Qt::Horizontal)
                    ret.setRect(sliderstart + sliderlen, 0,
                            maxlen - sliderstart - sliderlen + sbextent, sbextent);
                else
                    ret.setRect(0, sliderstart + sliderlen, sbextent,
                            maxlen - sliderstart - sliderlen + sbextent);
                break;
            case SC_ScrollBarGroove:
                if (scrollbar->orientation == Qt::Horizontal)
                    ret.setRect(sbextent, 0, scrollbar->rect.width() - sbextent * 2,
                            scrollbar->rect.height());
                else
                    ret.setRect(0, sbextent, scrollbar->rect.width(),
                            scrollbar->rect.height() - sbextent * 2);
                break;
            case SC_ScrollBarSlider:
                if (scrollbar->orientation == Qt::Horizontal)
                    ret.setRect(sliderstart, 0, sliderlen, sbextent);
                else
                    ret.setRect(0, sliderstart, sbextent, sliderlen);
                break;
            }
        }
        break;
    default:
        qWarning("QCommonStyle::querySubControlMetrics case not handled %d", cc);
    }
    return ret;
}

/*! \reimp */
void QCommonStyle::drawComplexControl(ComplexControl control,
                                       QPainter *p,
                                       const QWidget *widget,
                                       const QRect &r,
                                       const QPalette &pal,
                                       SFlags flags,
                                       SCFlags controls,
                                       SCFlags active,
                                       const QStyleOption& opt) const
{
    if (! widget) {
        qWarning("QCommonStyle::drawComplexControl: widget parameter cannot be zero!");
        return;
    }

    activePainter = p;

    switch (control) {
#ifndef QT_NO_TOOLBUTTON
    case CC_ToolButton:
        {
            const QToolButton *toolbutton = (const QToolButton *) widget;

            QPalette pal2 = pal;
            if (toolbutton->backgroundRole() != QPalette::Button)
                pal2.setBrush(QPalette::Button,
                            toolbutton->palette().brush(toolbutton->backgroundRole()));
            QRect button, menuarea;
            button   = visualRect(querySubControlMetrics(control, widget, SC_ToolButton, opt), widget);
            menuarea = visualRect(querySubControlMetrics(control, widget, SC_ToolButtonMenu, opt), widget);

            SFlags bflags = flags,
                   mflags = flags;

            if (active & SC_ToolButton)
                bflags |= Style_Down;
            if (active & SC_ToolButtonMenu)
                mflags |= Style_Down;

            if (controls & SC_ToolButton) {
                QWidget *tbPW = static_cast<QWidget *>(toolbutton->parent());
                if (bflags & (Style_Down | Style_On | Style_Raised)) {
                    drawPrimitive(PE_ButtonTool, p, button, pal2, bflags, opt);
                } else if (tbPW &&
                            tbPW->palette().brush(tbPW->backgroundRole()).pixmap() &&
                          ! tbPW->palette().brush(tbPW->backgroundRole()).pixmap()->isNull()) {
                    QPixmap pixmap =
                        *(tbPW->palette().brush(tbPW->backgroundRole()).pixmap());

                    p->drawTiledPixmap(r, pixmap, toolbutton->pos());
                }
            }

            if (controls & SC_ToolButtonMenu) {
                if (mflags & (Style_Down | Style_On | Style_Raised))
                    drawPrimitive(PE_ButtonDropDown, p, menuarea, pal2, mflags, opt);
                drawPrimitive(PE_ArrowDown, p, menuarea, pal2, mflags, opt);
            }

            if (toolbutton->hasFocus() && !toolbutton->focusProxy()) {
                QRect fr = toolbutton->rect();
                fr.addCoords(3, 3, -3, -3);
                drawPrimitive(PE_FocusRect, p, fr, pal2);
            }

            break;
        }
#endif // QT_NO_TOOLBUTTON

#ifndef QT_NO_TITLEBAR
    case CC_TitleBar:
        {
            if (controls & SC_TitleBarLabel) {
                QColor left = pal.highlight();
                QColor right = pal.base();

                if (left != right) {
                    double rS = left.red();
                    double gS = left.green();
                    double bS = left.blue();

                    const double rD = double(right.red() - rS) / r.width();
                    const double gD = double(right.green() - gS) / r.width();
                    const double bD = double(right.blue() - bS) / r.width();

                    const int w = r.width();
                    for (int sx = 0; sx < w; sx++) {
                        rS+=rD;
                        gS+=gD;
                        bS+=bD;
                        p->setPen(QColor((int)rS, (int)gS, (int)bS));
                        p->drawLine(sx, 0, sx, r.height());
                    }
                } else {
                    p->fillRect(r, left);
                }

                QRect ir = visualRect(querySubControlMetrics(CC_TitleBar, widget, SC_TitleBarLabel), widget);

                p->setPen(pal.highlightedText());
                QString caption(widget->windowTitle());
                p->drawText(ir.x()+2, ir.y(), ir.width()-2, ir.height(),
                            AlignAuto | AlignVCenter | SingleLine, caption);
            }

            QRect ir;
            bool down = false;
            QPixmap pm;

            if (controls & SC_TitleBarCloseButton) {
                ir = visualRect(querySubControlMetrics(CC_TitleBar, widget, SC_TitleBarCloseButton), widget);
                down = active & SC_TitleBarCloseButton;
                if (widget->testWFlags(WStyle_Tool)
#ifndef QT_NO_MAINWINDOW
                     || qt_cast<QDockWindow*>(widget)
#endif
                   )
                    pm = stylePixmap(SP_DockWindowCloseButton, widget);
                else
                    pm = stylePixmap(SP_TitleBarCloseButton, widget);
                drawPrimitive(PE_ButtonTool, p, ir, pal,
                              down ? Style_Down : Style_Raised);

                p->save();
                if(down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, widget),
                                  pixelMetric(PM_ButtonShiftVertical, widget));
                drawItem(p, ir, AlignCenter, pal, true, pm);
                p->restore();
            }

            if (controls & SC_TitleBarMaxButton) {
                ir = visualRect(querySubControlMetrics(CC_TitleBar, widget, SC_TitleBarMaxButton), widget);

                down = active & SC_TitleBarMaxButton;
                pm = QPixmap(stylePixmap(SP_TitleBarMaxButton, widget));
                drawPrimitive(PE_ButtonTool, p, ir, pal,
                              down ? Style_Down : Style_Raised);

                p->save();
                if(down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, widget),
                                  pixelMetric(PM_ButtonShiftVertical, widget));
                drawItem(p, ir, AlignCenter, pal, true, pm);
                p->restore();
            }

            if (controls & SC_TitleBarNormalButton || controls & SC_TitleBarMinButton) {
                ir = visualRect(querySubControlMetrics(CC_TitleBar, widget, SC_TitleBarMinButton), widget);
                QStyle::SubControl ctrl = (controls & SC_TitleBarNormalButton ?
                                           SC_TitleBarNormalButton :
                                           SC_TitleBarMinButton);
                QStyle::StylePixmap spixmap = (controls & SC_TitleBarNormalButton ?
                                               SP_TitleBarNormalButton :
                                               SP_TitleBarMinButton);
                down = active & ctrl;
                pm = QPixmap(stylePixmap(spixmap, widget));
                drawPrimitive(PE_ButtonTool, p, ir, pal, down ? Style_Down : Style_Raised);

                p->save();
                if(down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, widget),
                                  pixelMetric(PM_ButtonShiftVertical, widget));
                drawItem(p, ir, AlignCenter, pal, true, pm);
                p->restore();
            }

            if (controls & SC_TitleBarShadeButton) {
                ir = visualRect(querySubControlMetrics(CC_TitleBar, widget, SC_TitleBarShadeButton), widget);

                down = active & SC_TitleBarShadeButton;
                pm = QPixmap(stylePixmap(SP_TitleBarShadeButton, widget));
                drawPrimitive(PE_ButtonTool, p, ir, pal, down ? Style_Down : Style_Raised);
                p->save();
                if(down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, widget),
                                  pixelMetric(PM_ButtonShiftVertical, widget));
                drawItem(p, ir, AlignCenter, pal, true, pm);
                p->restore();
            }

            if (controls & SC_TitleBarUnshadeButton) {
                ir = visualRect(querySubControlMetrics(CC_TitleBar, widget, SC_TitleBarUnshadeButton), widget);

                down = active & SC_TitleBarUnshadeButton;
                pm = QPixmap(stylePixmap(SP_TitleBarUnshadeButton, widget));
                drawPrimitive(PE_ButtonTool, p, ir, pal, down ? Style_Down : Style_Raised);
                p->save();
                if(down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, widget),
                                  pixelMetric(PM_ButtonShiftVertical, widget));
                drawItem(p, ir, AlignCenter, pal, true, pm);
                p->restore();
            }
#ifndef QT_NO_WIDGET_TOPEXTRA
            if (controls & SC_TitleBarSysMenu) {
                if (!widget->windowIcon().isNull()) {
                    ir = visualRect(querySubControlMetrics(CC_TitleBar, widget, SC_TitleBarSysMenu), widget);
                    drawItem(p, ir, AlignCenter, pal, true, widget->windowIcon());
                }
            }
#endif
            break;
        }
#endif //QT_NO_TITLEBAR

    case CC_SpinWidget: {
#ifndef QT_NO_SPINWIDGET
        const QSpinWidget * sw = (const QSpinWidget *) widget;
        SFlags flags;
        PrimitiveElement pe;

        if (controls & SC_SpinWidgetFrame)
            qDrawWinPanel(p, r, pal, true); //cstyle == Sunken);

        if (controls & SC_SpinWidgetUp) {
            flags = Style_Default | Style_Enabled;
            if (active == SC_SpinWidgetUp) {
                flags |= Style_On;
                flags |= Style_Sunken;
            } else
                flags |= Style_Raised;
            if (sw->buttonSymbols() == QSpinWidget::PlusMinus)
                pe = PE_SpinWidgetPlus;
            else
                pe = PE_SpinWidgetUp;

            QRect re = sw->upRect();
            QPalette pal2 = pal;
            if(!sw->isUpEnabled())
                pal2.setCurrentColorGroup(QPalette::Disabled);
            drawPrimitive(PE_ButtonBevel, p, re, pal2, flags);
            drawPrimitive(pe, p, re, pal2, flags);
        }

        if (controls & SC_SpinWidgetDown) {
            flags = Style_Default | Style_Enabled;
            if (active == SC_SpinWidgetDown) {
                flags |= Style_On;
                flags |= Style_Sunken;
            } else
                flags |= Style_Raised;
            if (sw->buttonSymbols() == QSpinWidget::PlusMinus)
                pe = PE_SpinWidgetMinus;
            else
                pe = PE_SpinWidgetDown;

            QRect re = sw->downRect();
            QPalette pal2 = pal;
            if(!sw->isDownEnabled())
                pal2.setCurrentColorGroup(QPalette::Disabled);
            drawPrimitive(PE_ButtonBevel, p, re, pal2, flags);
            drawPrimitive(pe, p, re, pal2, flags);
        }
#endif
        break; }

#ifndef QT_NO_SLIDER
    case CC_Slider:
        switch (controls) {
        case SC_SliderTickmarks: {
            const QSlider * sl = static_cast<const QSlider *>(widget);
            int tickOffset = pixelMetric(PM_SliderTickmarkOffset, sl);
            int ticks = sl->tickmarks();
            int thickness = pixelMetric(PM_SliderControlThickness, sl);
            int len = pixelMetric(PM_SliderLength, sl);
            int available = pixelMetric(PM_SliderSpaceAvailable, sl);
            int interval = sl->tickInterval();

            if (interval <= 0) {
                interval = sl->singleStep();
                if (QStyle::positionFromValue(sl->minimum(), sl->maximum(), interval, available)
                    - QStyle::positionFromValue(sl->minimum(), sl->maximum(), 0, available) < 3)
                    interval = sl->pageStep();
            }

            int fudge = len / 2;
            int pos;

            if (ticks & QSlider::Above) {
                if (sl->orientation() == Horizontal)
                    p->fillRect(0, 0, sl->width(), tickOffset, pal.brush(QPalette::Background));
                else
                    p->fillRect(0, 0, tickOffset, sl->width(), pal.brush(QPalette::Background));
                p->setPen(pal.foreground());
                int v = sl->minimum();
                if (!interval)
                    interval = 1;
                while (v <= sl->maximum() + 1) {
                    pos = QStyle::positionFromValue(sl->minimum(), sl->maximum(), v, available)
                          + fudge;
                    if (sl->orientation() == Horizontal)
                        p->drawLine(pos, 0, pos, tickOffset-2);
                    else
                        p->drawLine(0, pos, tickOffset-2, pos);
                    v += interval;
                }
            }

            if (ticks & QSlider::Below) {
                if (sl->orientation() == Horizontal)
                    p->fillRect(0, tickOffset + thickness, sl->width(), tickOffset,
                                pal.brush(QPalette::Background));
                else
                    p->fillRect(tickOffset + thickness, 0, tickOffset, sl->height(),
                                pal.brush(QPalette::Background));
                p->setPen(pal.foreground());
                int v = sl->minimum();
                if (!interval)
                    interval = 1;
                while (v <= sl->maximum() + 1) {
                    pos = QStyle::positionFromValue(sl->minimum(), sl->maximum(), v, available)
                          + fudge;
                    if (sl->orientation() == Horizontal)
                        p->drawLine(pos, tickOffset+thickness+1, pos,
                                    tickOffset+thickness+1 + available-2);
                    else
                        p->drawLine(tickOffset+thickness+1, pos,
                                    tickOffset+thickness+1 + available-2,
                                    pos);
                    v += interval;
                }

            }

            break; }
        }
        break;
#endif // QT_NO_SLIDER
    default:
        break;
    }

    activePainter = 0;
}


/*! \reimp */
void QCommonStyle::drawComplexControlMask(ComplexControl control,
                                           QPainter *p,
                                           const QWidget *widget,
                                           const QRect &r,
                                           const QStyleOption& opt) const
{
    Q_UNUSED(control);
    Q_UNUSED(widget);
    Q_UNUSED(opt);

    p->fillRect(r, color1);
}


/*! \reimp */
QRect QCommonStyle::querySubControlMetrics(ComplexControl control,
                                            const QWidget *widget,
                                            SubControl sc,
                                            const QStyleOption &opt) const
{
    if (! widget) {
        qWarning("QCommonStyle::querySubControlMetrics: widget parameter cannot be zero!");
        return QRect();
    }

    switch (control) {
    case CC_SpinWidget: {
        int fw = pixelMetric(PM_SpinBoxFrameWidth, widget);
        QSize bs;
        bs.setHeight(widget->height()/2 - fw);
        if (bs.height() < 8)
            bs.setHeight(8);
        bs.setWidth(qMin(bs.height() * 8 / 5, widget->width() / 4)); // 1.6 -approximate golden mean
        bs = bs.expandedTo(QApplication::globalStrut());
        int y = fw;
        int x, lx, rx;
        x = widget->width() - y - bs.width();
        lx = fw;
        rx = x - fw;
        switch (sc) {
        case SC_SpinWidgetUp:
            return QRect(x, y, bs.width(), bs.height());
        case SC_SpinWidgetDown:
            return QRect(x, y + bs.height(), bs.width(), bs.height());
        case SC_SpinWidgetButtonField:
            return QRect(x, y, bs.width(), widget->height() - 2*fw);
        case SC_SpinWidgetEditField:
            return QRect(lx, fw, rx, widget->height() - 2*fw);
        case SC_SpinWidgetFrame:
            return widget->rect();
        default:
            break;
        }
        break; }

    case CC_ComboBox: {
        int x = 0, y = 0, wi = widget->width(), he = widget->height();
        int xpos = x;
        xpos += wi - 2 - 16;

        switch (sc) {
        case SC_ComboBoxFrame:
            return widget->rect();
        case SC_ComboBoxArrow:
            return QRect(xpos, y+2, 16, he-4);
        case SC_ComboBoxEditField:
            return QRect(x+3, y+3, wi-6-16, he-6);
        case SC_ComboBoxListBoxPopup:
            return opt.rect();
        default:
            break;
        }
        break; }

#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar: {
        const QScrollBar *scrollbar = (const QScrollBar *) widget;
        int sbextent = pixelMetric(PM_ScrollBarExtent, widget);
        int maxlen = ((scrollbar->orientation() == Qt::Horizontal) ?
                      scrollbar->width() : scrollbar->height()) - (sbextent * 2);
        int sliderlen;

        // calculate slider length
        if (scrollbar->maximum() != scrollbar->minimum()) {
            uint range = scrollbar->maximum() - scrollbar->minimum();
            sliderlen = (scrollbar->pageStep() * maxlen) /
                        (range + scrollbar->pageStep());

            int slidermin = pixelMetric(PM_ScrollBarSliderMin, widget);
            if (sliderlen < slidermin || range > INT_MAX / 2)
                sliderlen = slidermin;
            if (sliderlen > maxlen)
                sliderlen = maxlen;
        } else
            sliderlen = maxlen;

        int sliderstart = sbextent + positionFromValue(scrollbar->minimum(), scrollbar->maximum(),
                                                       scrollbar->sliderPosition(),
                                                       maxlen - sliderlen,
                                                       scrollbar->invertedAppearance());
        switch (sc) {
        case SC_ScrollBarSubLine:            // top/left button
            if (scrollbar->orientation() == Qt::Horizontal) {
                int buttonWidth = qMin(scrollbar->width()/2, sbextent);
                return QRect(0, 0, buttonWidth, sbextent);
            } else {
                int buttonHeight = qMin(scrollbar->height()/2, sbextent);
                return QRect(0, 0, sbextent, buttonHeight);
            }

        case SC_ScrollBarAddLine:            // bottom/right button
            if (scrollbar->orientation() == Qt::Horizontal) {
                int buttonWidth = qMin(scrollbar->width()/2, sbextent);
                return QRect(scrollbar->width() - buttonWidth, 0, buttonWidth, sbextent);
            } else {
                int buttonHeight = qMin(scrollbar->height()/2, sbextent);
                return QRect(0, scrollbar->height() - buttonHeight, sbextent, buttonHeight);
            }

        case SC_ScrollBarSubPage:            // between top/left button and slider
            if (scrollbar->orientation() == Qt::Horizontal)
                return QRect(sbextent, 0, sliderstart - sbextent, sbextent);
            return QRect(0, sbextent, sbextent, sliderstart - sbextent);

        case SC_ScrollBarAddPage:            // between bottom/right button and slider
            if (scrollbar->orientation() == Qt::Horizontal)
                return QRect(sliderstart + sliderlen, 0,
                             maxlen - sliderstart - sliderlen + sbextent, sbextent);
            return QRect(0, sliderstart + sliderlen,
                         sbextent, maxlen - sliderstart - sliderlen + sbextent);

        case SC_ScrollBarGroove:
            if (scrollbar->orientation() == Qt::Horizontal)
                return QRect(sbextent, 0, scrollbar->width() - sbextent * 2,
                             scrollbar->height());
            return QRect(0, sbextent, scrollbar->width(),
                         scrollbar->height() - sbextent * 2);

        case SC_ScrollBarSlider:
            if (scrollbar->orientation() == Qt::Horizontal)
                return QRect(sliderstart, 0, sliderlen, sbextent);
            return QRect(0, sliderstart, sbextent, sliderlen);

        default: break;
        }

        break; }
#endif // QT_NO_SCROLLBAR

#ifndef QT_NO_SLIDER
    case CC_Slider: {
        const QSlider * sl = static_cast<const QSlider *>(widget);
        int tickOffset = pixelMetric(PM_SliderTickmarkOffset, sl);
        int thickness = pixelMetric(PM_SliderControlThickness, sl);

        switch (sc) {
        case SC_SliderHandle: {
            int sliderPos = 0;
            int len = pixelMetric(PM_SliderLength, sl);
            bool horizontal = sl->orientation() == Horizontal;
            sliderPos = positionFromValue(sl->minimum(), sl->maximum(), sl->sliderPosition(),
                                          (horizontal ? sl->width() : sl->height()) - len,
                                           (!horizontal == !sl->invertedAppearance()));
            if (horizontal)
                return QRect(sliderPos, tickOffset, len, thickness);
            return QRect(tickOffset, sliderPos, thickness, len); }
        case SC_SliderGroove: {
            if (sl->orientation() == Horizontal)
                return QRect(0, tickOffset, sl->width(), thickness);
            return QRect(tickOffset, 0, thickness, sl->height()); }
        default:
            break;
        }
        break; }
#endif // QT_NO_SLIDER

#if !defined(QT_NO_TOOLBUTTON) && !defined(QT_NO_POPUPMENU)
    case CC_ToolButton: {
            const QToolButton *toolbutton = (const QToolButton *) widget;
            int mbi = pixelMetric(PM_MenuButtonIndicator, widget);

            QRect rect = toolbutton->rect();
            switch (sc) {
            case SC_ToolButton:
                if (toolbutton->menu() && ! toolbutton->popupDelay())
                    rect.addCoords(0, 0, -mbi, 0);
                return rect;

            case SC_ToolButtonMenu:
                if (toolbutton->menu() && ! toolbutton->popupDelay())
                    rect.addCoords(rect.width() - mbi, 0, 0, 0);
                return rect;

            default: break;
            }
            break;
        }
#endif // QT_NO_TOOLBUTTON && QT_NO_POPUPMENU

#ifndef QT_NO_TITLEBAR
    case CC_TitleBar: {
            const int controlTop = 2;
            const int controlHeight = widget->height() - controlTop * 2;

            switch (sc) {
            case SC_TitleBarLabel: {
                QRect ir(0, 0, widget->width(), widget->height());
                if (widget->testWFlags(WStyle_Tool)) {
                    if (widget->testWFlags(WStyle_SysMenu))
                        ir.addCoords(0, 0, -controlHeight-3, 0);
                    if (widget->testWFlags(WStyle_MinMax))
                        ir.addCoords(0, 0, -controlHeight-2, 0);
                } else {
                    if (widget->testWFlags(WStyle_SysMenu))
                        ir.addCoords(controlHeight+3, 0, -controlHeight-3, 0);
                    if (widget->testWFlags(WStyle_Minimize))
                        ir.addCoords(0, 0, -controlHeight-2, 0);
                    if (widget->testWFlags(WStyle_Maximize))
                        ir.addCoords(0, 0, -controlHeight-2, 0);
                }
                return ir; }

            case SC_TitleBarCloseButton:
                return QRect(widget->width() - (controlHeight + controlTop),
                              controlTop, controlHeight, controlHeight);

            case SC_TitleBarMaxButton:
            case SC_TitleBarShadeButton:
            case SC_TitleBarUnshadeButton:
                return QRect(widget->width() - ((controlHeight + controlTop) * 2),
                              controlTop, controlHeight, controlHeight);

            case SC_TitleBarMinButton:
            case SC_TitleBarNormalButton: {
                int offset = controlHeight + controlTop;
                if (!widget->testWFlags(WStyle_Maximize))
                    offset *= 2;
                else
                    offset *= 3;
                return QRect(widget->width() - offset, controlTop, controlHeight, controlHeight);
            }

            case SC_TitleBarSysMenu:
                return QRect(3, controlTop, controlHeight, controlHeight);

            default: break;
            }
            break; }
#endif //QT_NO_TITLEBAR

    default:
        break;
    }
    return QRect();
}


/*! \reimp */
QStyle::SubControl QCommonStyle::querySubControl(ComplexControl control,
                                                 const QWidget *widget,
                                                 const QPoint &pos,
                                                 const QStyleOption& opt) const
{
    SubControl ret = SC_None;

    switch (control) {
#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar:
        {
            QRect r;
            uint ctrl = SC_ScrollBarAddLine;

            // we can do this because subcontrols were designed to be masks as well...
            while (ret == SC_None && ctrl <= SC_ScrollBarGroove) {
                r = visualRect(querySubControlMetrics(control, widget,
                                                      (QStyle::SubControl) ctrl, opt), widget);
                if (r.isValid() && r.contains(pos))
                    ret = (QStyle::SubControl) ctrl;

                ctrl <<= 1;
            }

            break;
        }
#endif
    case CC_TitleBar:
        {
#ifndef QT_NO_TITLEBAR
            QRect r;
            uint ctrl = SC_TitleBarLabel;
            bool isMinimized = false;
            if (!opt.isDefault())
                isMinimized= opt.titleBarState() & QWidget::WindowMinimized;

            // we can do this because subcontrols were designed to be masks as well...
            while (ret == SC_None && ctrl <= SC_TitleBarUnshadeButton) {
                r = visualRect(querySubControlMetrics(control, widget, (QStyle::SubControl) ctrl, opt), widget);
                if (r.isValid() && r.contains(pos))
                    ret = (QStyle::SubControl) ctrl;

                ctrl <<= 1;
            }
            if (widget->testWFlags(WStyle_Tool)) {
                if (ret == SC_TitleBarMinButton || ret == SC_TitleBarMaxButton) {
                    if (isMinimized)
                        ret = SC_TitleBarUnshadeButton;
                    else
                        ret = SC_TitleBarShadeButton;
                }
            } else if (ret == SC_TitleBarMinButton && isMinimized) {
                ret = QStyle::SC_TitleBarNormalButton;
            }
#endif
            break;
        }
    case CC_Slider:
        {
            QRect r;
            // Check for handle first, then the groove.
            r = visualRect(querySubControlMetrics(control, widget, SC_SliderHandle, opt), widget);
            if (r.isValid() && r.contains(pos)) {
                ret = SC_SliderHandle;
            } else {
                r = visualRect(querySubControlMetrics(control, widget, SC_SliderGroove, opt),
                               widget);
                if (r.isValid() && r.contains(pos))
                    ret = SC_SliderGroove;
            }
            break;
        }
    default:
        break;
    }

    return ret;
}

/*! \reimp */
int QCommonStyle::pixelMetric(PixelMetric m, const QWidget *widget) const
{
    int ret;

    switch (m) {
    case PM_MenuBarVMargin:
    case PM_MenuBarHMargin:
        ret = 2;
        break;
    case PM_DialogButtonsSeparator:
        ret = 5;
        break;
    case PM_DialogButtonsButtonWidth:
        ret = 70;
        break;
    case PM_DialogButtonsButtonHeight:
        ret = 30;
        break;
    case PM_CheckListControllerSize:
    case PM_CheckListButtonSize:
        ret = 16;
        break;
    case PM_TitleBarHeight: {
        if (widget) {
            if (widget->testWFlags(WStyle_Tool)) {
                ret = qMax(widget->fontMetrics().lineSpacing(), 16);
#ifndef QT_NO_MAINWINDOW
            } else if (qt_cast<QDockWindow*>(widget)) {
                ret = qMax(widget->fontMetrics().lineSpacing(), 13);
#endif
            } else {
                ret = qMax(widget->fontMetrics().lineSpacing(), 18);
            }
        } else {
            ret = 0;
        }

        break; }
    case PM_ScrollBarSliderMin:
        ret = 9;
        break;

    case PM_ButtonMargin:
        ret = 6;
        break;

    case PM_ButtonDefaultIndicator:
        ret = 0;
        break;

    case PM_MenuButtonIndicator:
        if (! widget)
            ret = 12;
        else
            ret = qMax(12, (widget->height() - 4) / 3);
        break;

    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        ret = 0;
        break;

    case PM_MenuFrameWidth:
    case PM_SpinBoxFrameWidth:
    case PM_DefaultFrameWidth:
        ret = 2;
        break;

    case PM_MDIFrameWidth:
        ret = 4;
        break;

    case PM_MDIMinimizedWidth:
        ret = 196;
        break;

#ifndef QT_NO_SCROLLBAR
    case PM_ScrollBarExtent:
        if (!widget) {
            ret = 16;
        } else {
            const QScrollBar *bar = (const QScrollBar*)widget;
            int s = bar->orientation() == Qt::Horizontal ?
                    QApplication::globalStrut().height()
                    : QApplication::globalStrut().width();
            ret = qMax(16, s);
        }
        break;
#endif
    case PM_MaximumDragDistance:
        ret = -1;
        break;

#ifndef QT_NO_SLIDER
    case PM_SliderThickness:
        ret = 16;
        break;

    case PM_SliderTickmarkOffset:
        {
            if (! widget) {
                ret = 0;
                break;
            }

            const QSlider * sl = (const QSlider *) widget;
            int space = (sl->orientation() == Horizontal) ? sl->height() :
                        sl->width();
            int thickness = pixelMetric(PM_SliderControlThickness, sl);
            int ticks = sl->tickmarks();

            if (ticks == QSlider::Both)
                ret = (space - thickness) / 2;
            else if (ticks == QSlider::Above)
                ret = space - thickness;
            else
                ret = 0;
            break;
        }

    case PM_SliderSpaceAvailable:
        {
            const QSlider * sl = (const QSlider *) widget;
            if (sl->orientation() == Horizontal)
                ret = sl->width() - pixelMetric(PM_SliderLength, sl);
            else
                ret = sl->height() - pixelMetric(PM_SliderLength, sl);
            break;
        }
#endif // QT_NO_SLIDER

    case PM_DockWindowSeparatorExtent:
        ret = 6;
        break;

    case PM_DockWindowHandleExtent:
        ret = 8;
        break;

    case PM_DockWindowFrameWidth:
        ret = 1;
        break;

    case PM_MenuBarFrameWidth:
        ret = 2;
        break;

    case PM_MenuBarItemSpacing:
    case PM_ToolBarItemSpacing:
        ret = 0;
        break;

    case PM_TabBarTabOverlap:
        ret = 3;
        break;

    case PM_TabBarBaseHeight:
        ret = 0;
        break;

    case PM_TabBarBaseOverlap:
        ret = 0;
        break;

    case PM_TabBarTabHSpace:
        ret = 24;
        break;

    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
        ret = 2;
        break;

#ifndef QT_NO_TABBAR
    case PM_TabBarTabVSpace:
        {
            const QTabBar * tb = (const QTabBar *) widget;
            if (tb && (tb->shape() == QTabBar::RoundedAbove ||
                         tb->shape() == QTabBar::RoundedBelow))
                ret = 10;
            else
                ret = 0;
            break;
        }
#endif

    case PM_ProgressBarChunkWidth:
        ret = 9;
        break;

    case PM_IndicatorWidth:
        ret = 13;
        break;

    case PM_IndicatorHeight:
        ret = 13;
        break;

    case PM_ExclusiveIndicatorWidth:
        ret = 12;
        break;

    case PM_ExclusiveIndicatorHeight:
        ret = 12;
        break;

    case PM_MenuTearoffHeight:
        ret = 6;
        break;

    case PM_MenuDesktopFrameWidth:
    case PM_MenuScrollerHeight:
    case PM_MenuHMargin:
    case PM_MenuVMargin:
        ret = 0;
        break;

    case PM_HeaderMargin:
        ret = 4;
        break;
    case PM_HeaderMarkSize:
        ret = 32;
        break;
    case PM_HeaderGripMargin:
        ret = 4;
        break;
    case PM_TabBarScrollButtonWidth:
        ret = 16;
        break;
    default:
        ret = 0;
        break;
    }

    return ret;
}


/*! \reimp */
QSize QCommonStyle::sizeFromContents(ContentsType contents,
                                     const QWidget *widget,
                                     const QSize &contentsSize,
                                     const QStyleOption& opt) const
{
    QSize sz(contentsSize);

    if (! widget) {
        qWarning("QCommonStyle::sizeFromContents: widget parameter cannot be zero!");
        return sz;
    }

    switch (contents) {
#ifndef QT_NO_DIALOGBUTTONS
    case CT_DialogButtons: {
        const QDialogButtons *dbtns = (const QDialogButtons *)widget;
        int w = contentsSize.width(), h = contentsSize.height();
        const int bwidth = pixelMetric(PM_DialogButtonsButtonWidth, widget),
                  bspace = pixelMetric(PM_DialogButtonsSeparator, widget),
                 bheight = pixelMetric(PM_DialogButtonsButtonHeight, widget);
        if(dbtns->orientation() == Horizontal) {
            if(!w)
                w = bwidth;
        } else {
            if(!h)
                h = bheight;
        }
        QDialogButtons::Button btns[] = { QDialogButtons::All, QDialogButtons::Reject, QDialogButtons::Accept, //reverse order (right to left)
                                          QDialogButtons::Apply, QDialogButtons::Retry, QDialogButtons::Ignore, QDialogButtons::Abort,
                                          QDialogButtons::Help };
        for(unsigned int i = 0, cnt = 0; i < (sizeof(btns)/sizeof(btns[0])); i++) {
            if(dbtns->isButtonVisible(btns[i])) {
                QSize szH = dbtns->sizeHint(btns[i]);
                int mwidth = qMax(bwidth, szH.width()), mheight = qMax(bheight, szH.height());
                if(dbtns->orientation() == Horizontal)
                    h = qMax(h, mheight);
                else
                    w = qMax(w, mwidth);

                if(cnt)
                    w += bspace;
                cnt++;
                if(dbtns->orientation() == Horizontal)
                    w += mwidth;
                else
                    h += mheight;
            }
        }
        const int fw = pixelMetric(PM_DefaultFrameWidth, widget) * 2;
        sz = QSize(w + fw, h + fw);
        break; }
#endif //QT_NO_DIALOGBUTTONS
    case CT_PushButton:
        {
#ifndef QT_NO_PUSHBUTTON
            const QPushButton *button = (const QPushButton *) widget;
            int w = contentsSize.width(),
                h = contentsSize.height(),
               bm = pixelMetric(PM_ButtonMargin, widget),
               fw = pixelMetric(PM_DefaultFrameWidth, widget) * 2;

            w += bm + fw;
            h += bm + fw;

            if (button->isDefault() || button->autoDefault()) {
                int dbw = pixelMetric(PM_ButtonDefaultIndicator, widget) * 2;
                w += dbw;
                h += dbw;
            }

            sz = QSize(w, h);
#endif
            break;
        }

    case CT_CheckBox:
        {
#ifndef QT_NO_CHECKBOX
            const QCheckBox *checkbox = (const QCheckBox *) widget;
            QRect irect = subRect(SR_CheckBoxIndicator, widget);
            int h = pixelMetric(PM_IndicatorHeight, widget);
            int margins = (!checkbox->icon().isNull() && checkbox->text().isEmpty()) ? 0 : 10;
            sz += QSize(irect.right() + margins, 4);
            sz.setHeight(qMax(sz.height(), h));
#endif
            break;
        }

    case CT_RadioButton:
        {
#ifndef QT_NO_RADIOBUTTON
            const QRadioButton *radiobutton = (const QRadioButton *) widget;
            QRect irect = subRect(SR_RadioButtonIndicator, widget);
            int h = pixelMetric(PM_ExclusiveIndicatorHeight, widget);
            int margins = (!radiobutton->icon().isNull() && radiobutton->text().isEmpty()) ? 0 : 10;
            sz += QSize(irect.right() + margins, 4);
            sz.setHeight(qMax(sz.height(), h));
#endif
            break;
        }

    case CT_ToolButton:
        {
            sz = QSize(sz.width() + 6, sz.height() + 5);
            break;
        }

    case CT_ComboBox:
        {
            int dfw = pixelMetric(PM_DefaultFrameWidth, widget) * 2;
            sz = QSize(sz.width() + dfw + 21, sz.height() + dfw);
            break;
        }

    case CT_MenuItem:
        {
#ifndef QT_NO_MENU
            if (opt.isDefault())
                break;
            const QMenu *menu = (const QMenu *)widget;
            bool checkable = menu->isCheckable();
            QAction *act = opt.action();
            int maxpmw = opt.maxIconWidth();

            int w = sz.width(), h = sz.height();
            if(act->isSeparator()) {
                w = 10;
                h = 2;
            } else {
                h = qMax(h, menu->fontMetrics().height() + 8);
                if(act->icon().isNull())
                    h = qMax(h, act->icon().pixmap(QIconSet::Small, QIconSet::Normal).height() + 4);
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

    case CT_SpinBox:
        sz.setWidth(qMin(sz.height(), int(sz.width() * 0.2)));
        break;

    case CT_MenuBar:
    case CT_Menu:
    case CT_MenuBarItem:
    case CT_LineEdit:
    case CT_Header:
    case CT_Slider:
    case CT_ProgressBar:
        // just return the contentsSize for now
        // fall through intended

    default:
        break;
    }

    return sz;
}

/*!
    Returns the size required by the contents of type \a ct, with
    style options \a opt, original size \a csz, font metrics \a fm,
    and parent widget \a widget.
*/
QSize QCommonStyle::sizeFromContents(ContentsType ct, const Q4StyleOption *opt, const QSize &csz,
                                     const QFontMetrics &fm, const QWidget *widget) const
{
    QSize sz(csz);
    switch (ct) {
    case CT_PushButton:
        if (const Q4StyleOptionButton *btn = qt_cast<const Q4StyleOptionButton *>(opt)) {
            int w = csz.width(),
                h = csz.height(),
                bm = pixelMetric(PM_ButtonMargin, widget),
            fw = pixelMetric(PM_DefaultFrameWidth, widget) * 2;
            w += bm + fw;
            h += bm + fw;
            if (btn->state & Style_ButtonDefault) {
                int dbw = pixelMetric(PM_ButtonDefaultIndicator, widget) * 2;
                w += dbw;
                h += dbw;
            }
            sz = QSize(w, h);
        }
        break;
    case CT_RadioButton:
    case CT_CheckBox:
        if (const Q4StyleOptionButton *btn = qt_cast<const Q4StyleOptionButton *>(opt)) {
            bool isRadio = (ct == CT_RadioButton);
            QRect irect = subRect(isRadio ? SR_RadioButtonIndicator : SR_CheckBoxIndicator,
                                  btn, widget);
            int h = pixelMetric(isRadio ? PM_ExclusiveIndicatorHeight : PM_IndicatorHeight, widget);
            int margins = (!btn->icon.isNull() && btn->text.isEmpty()) ? 0 : 10;
            sz += QSize(irect.right() + margins, 4);
            sz.setHeight(qMax(sz.height(), h));
        }
        break;
    case CT_MenuItem:
        if (const Q4StyleOptionMenuItem *mi = qt_cast<const Q4StyleOptionMenuItem *>(opt)) {
            bool checkable = mi->checkState != Q4StyleOptionMenuItem::NotCheckable;
            int maxpmw = mi->maxIconWidth;
            int w = sz.width(),
                h = sz.height();
            if (mi->menuItemType == Q4StyleOptionMenuItem::Q3Custom) {
                w = mi->q3CustomItemSizeHint.width();
                h = mi->q3CustomItemSizeHint.height();
                if (!mi->q3CustomItemFullSpan)
                    h += 8;
            } else if (mi->menuItemType == Q4StyleOptionMenuItem::Separator) {
                w = 10;
                h = 2;
            } else {
                h = qMax(h, fm.height() + 8);
                if (!mi->icon.isNull())
                    h = qMax(h, mi->icon.pixmap(QIconSet::Small, QIconSet::Normal).height() + 4);
            }
            if (mi->text.contains('\t'))
                w += 12;
            if (maxpmw > 0)
                w += maxpmw + 6;
            if (checkable && maxpmw < 20)
                w += 20 - maxpmw;
            if (checkable || maxpmw > 0)
                w += 2;
            w += 12;
            sz = QSize(w, h);
        }
        break;
    case CT_MenuBar:
    case CT_Menu:
    case CT_MenuBarItem:
    case CT_LineEdit:
    case CT_Header:
    case CT_Slider:
    case CT_ProgressBar:
        // just return the contentsSize for now
        // fall through intended
    default:
        break;
    }
    return sz;
}


/*! \reimp */
int QCommonStyle::styleHint(StyleHint sh, const QWidget * w, const QStyleOption &, QStyleHintReturn *) const
{
    int ret;

    switch (sh) {
#ifndef QT_NO_DIALOGBUTTONS
    case SH_DialogButtons_DefaultButton:
        ret = QDialogButtons::Accept;
        break;
#endif
    case SH_GroupBox_TextLabelVerticalAlignment:
        ret = Qt::AlignVCenter;
        break;

    case SH_GroupBox_TextLabelColor:
        ret = (int) (w ? w->palette().color(w->foregroundRole()).rgb() : 0);
        break;

    case SH_ListViewExpand_SelectMouseType:
    case SH_TabBar_SelectMouseType:
        ret = QEvent::MouseButtonPress;
        break;

    case SH_GUIStyle:
        ret = WindowsStyle;
        break;

#ifndef QT_NO_PALETTE
    case SH_ScrollBar_BackgroundRole:
        ret = QPalette::Background;
        break;
#endif

    case SH_TabBar_Alignment:
    case SH_Header_ArrowAlignment:
        ret = Qt::AlignLeft;
        break;

    case SH_Menu_SubMenuPopupDelay:
        ret = 256;
        break;

    case SH_ProgressDialog_TextLabelAlignment:
        ret = Qt::AlignCenter;
        break;

    case SH_BlinkCursorWhenTextSelected:
        ret = 1;
        break;

    case SH_Table_GridLineColor:
        ret = -1;
        break;

    case SH_LineEdit_PasswordCharacter:
        ret = '*';
        break;

    case SH_ToolBox_SelectedPageTitleBold:
        ret = 1;
        break;

    case SH_UnderlineShortcut:
        ret = 1;
        break;

    case SH_ToolButton_Uses3D:
        ret = 1;
        break;

    case SH_SpinBox_ClickAutoRepeatRate:
        ret = 150;
        break;

    case SH_SpinBox_KeyPressAutoRepeatRate:
        ret = 75;
        break;

    case SH_Menu_FillScreenWithScroll:
        ret = true;
        break;

    default:
        ret = 0;
        break;
    }

    return ret;
}

/*! \reimp */
QPixmap QCommonStyle::stylePixmap(StylePixmap, const QWidget *, const QStyleOption&) const
{
    return QPixmap();
}

/*! \reimp */
QPixmap QCommonStyle::stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap,
                                   const QPalette &pal, const QStyleOption &) const
{
    switch(pixmaptype) {
    case PT_Disabled: {
        QImage img;
        QBitmap pixmapMask;
        if (pixmap.mask()) {
            pixmapMask = *pixmap.mask();
        } else {
            img = pixmap.convertToImage();
            pixmapMask.convertFromImage(img.createHeuristicMask(),
                                         Qt::MonoOnly | Qt::ThresholdDither);
        }
        QPixmap ret(pixmap.width() + 1, pixmap.height() + 1);
        ret.fill(pal.color(QPalette::Disabled, QPalette::Background));

        QPainter painter;
        painter.begin(&ret);
        painter.setPen(pal.color(QPalette::Disabled, QPalette::Base));
        painter.drawPixmap(1, 1, pixmapMask);
        painter.setPen(pal.color(QPalette::Disabled, QPalette::Foreground));
        painter.drawPixmap(0, 0, pixmapMask);
        painter.end();

        if (!pixmapMask.mask())
            pixmapMask.setMask(pixmapMask);

        QBitmap mask(ret.size());
        mask.fill(Qt::color0);
        painter.begin(&mask);
        painter.drawPixmap(0, 0, pixmapMask);
        painter.drawPixmap(1, 1, pixmapMask);
        painter.end();
        ret.setMask(mask);
        return ret;
    }
    case PT_Pressed:
        return pixmap;
    default:
        break;
    }
    return pixmap;
}

#endif // QT_NO_STYLE
