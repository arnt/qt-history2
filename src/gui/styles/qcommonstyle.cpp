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

#include "qcommonstyle.h"

#ifndef QT_NO_STYLE

#include "private/qdialogbuttons_p.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qcheckbox.h"
#include "qdockwindow.h"
#include "qdrawutil.h"
#include "qgroupbox.h"
#include "qmenu.h"
#include "qpainter.h"
#include "qprogressbar.h"
#include "qpushbutton.h"
#include "qradiobutton.h"
#include "qscrollbar.h"
#include "qslider.h"
#include "qspinbox.h"
#include "qstyleoption.h"
#include "qtabbar.h"
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

    All the functions are full documented in \l{QStyle}, although the
    extra functions that QCommonStyle provides, e.g.
    drawComplexControl(), drawComplexControlMask(), drawControl(),
    drawControlMask(), drawPrimitive(), querySubControl(),
    querySubControlMetrics(), sizeFromContents(), and subRect() are
    documented here.
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

/*!
    \overload

    Destroys the style
*/
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

/*!
    Draws the primitive element \a pe, with style options \a opt, on
    painter \a p. The \a widget is optional and may contain a widget
    that is useful for drawing the primitive.
*/
void QCommonStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
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
        p->fillRect(opt->rect, Qt::color1);
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
        p->setPen(Qt::color1);
        p->setBrush(Qt::color1);
        p->drawEllipse(opt->rect);
        break;
    case PE_FocusRect:
        if (const QStyleOptionFocusRect *fropt = qt_cast<const QStyleOptionFocusRect *>(opt)) {
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
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt))
            qDrawShadePanel(p, frame->rect, frame->palette, frame->state & Style_Sunken,
                            frame->lineWidth);
        break;
    case PE_MenuBarFrame:
    case PE_PanelMenuBar:
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt))
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
        if (const QStyleOptionListView *lv = qt_cast<const QStyleOptionListView *>(opt)) {
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
                p->setPen(Qt::NoPen);
                p->setBrush(opt->palette.text());
                p->drawRect(x + 5, y + 4, 2, 4);
                p->drawRect(x + 4, y + 5, 4, 2);
            }
#undef QCOORDARRLEN
        }
        break;
    case PE_CheckListIndicator:
        if (const QStyleOptionListView *lv = qt_cast<const QStyleOptionListView *>(opt)) {
            if(lv->items.isEmpty())
                break;
            QStyleOptionListViewItem item = lv->items.at(0);
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
                && !(item.features & QStyleOptionListViewItem::ParentControl)) {
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
	p->save();
        if (pe == PE_RubberBandMask) {
            p->setBrush(Qt::color1);
            p->setPen(Qt::NoPen);
            p->drawRect(opt->rect);
	    if (opt->state & Style_Rectangle) {
                p->setBrush(Qt::color0);
                p->drawRect(opt->rect.x() + 4, opt->rect.y() + 4,
                            opt->rect.width() - 8, opt->rect.height() - 8);
            }
	} else {
	    QRect r = opt->rect;
 	    p->setBrush(Qt::Dense4Pattern);
	    p->setBackground(QBrush(opt->palette.base()));
	    p->setBackgroundMode(Qt::OpaqueMode);
            p->setPen(opt->palette.color(QPalette::Active, QPalette::Foreground));
	    p->drawRect(r);
	    if (opt->state & Style_Rectangle) {
		r.addCoords(3,3, -3,-3);
		p->drawRect(r);
	    }
	}
	p->restore();
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
        opt->rect.getRect(&x, &y, &w, &h);

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
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt)) {
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
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt)) {
            int lw = frame->lineWidth;
            if (lw <= 0)
                lw = pixelMetric(PM_DockWindowFrameWidth);

            qDrawShadePanel(p, frame->rect, frame->palette, false, lw);
        }
        break;
    case PE_DockWindowHandle:
        if (const QStyleOptionDockWindow *dw = qt_cast<const QStyleOptionDockWindow *>(opt)) {
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
    case PE_DockWindowSeparator: {
        QPoint p1, p2;
        if (opt->state & Style_Horizontal) {
            p1 = QPoint(opt->rect.width()/2, 0);
            p2 = QPoint(p1.x(), opt->rect.height());
        } else {
            p1 = QPoint(0, opt->rect.height()/2);
            p2 = QPoint(opt->rect.width(), p1.y());
        }
        qDrawShadeLine(p, p1, p2, opt->palette, 1, 1, 0);
        break; }
    case PE_SpinBoxPlus:
    case PE_SpinBoxMinus:
        if (const QStyleOptionSpinBox *sb = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            p->save();
            QRect r = sb->rect;
            int fw = pixelMetric(PM_DefaultFrameWidth, widget);
            QRect br;
            br.setRect(r.x() + fw, r.y() + fw, r.width() - fw*2,
                       r.height() - fw*2);

            p->fillRect(br, sb->palette.brush(QPalette::Button));
            QPen pen(sb->palette.buttonText());
            if (sb->rect.height() > 30)
                pen.setWidth((int)(sb->rect.height() / 20));

            p->setPen(pen);
            p->setBrush(sb->palette.buttonText());

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
            if (pe == PE_SpinBoxPlus)
                p->drawLine((x+w / 2) - 1, y + ymarg,
                            (x+w / 2) - 1, y + ymarg + length - 1);
            p->restore();
            break;
        }

    case PE_SpinBoxUp:
    case PE_SpinBoxDown:
        if (const QStyleOptionSpinBox *sb = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            QRect r = sb->rect;
            int fw = pixelMetric(PM_DefaultFrameWidth, widget);
            QRect br;
            br.setRect(r.x() + fw, r.y() + fw, r.width() - fw*2,
                       r.height() - fw*2);
            p->fillRect(br, sb->palette.brush(QPalette::Button));
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
            if (pe == PE_SpinBoxDown)
                a.setPoints(3,  0, 1,  sw-1, 1,  sh-2, sh-1);
            else
                a.setPoints(3,  0, sh-1,  sw-1, sh-1,  sh-2, 1);
            int bsx = 0;
            int bsy = 0;
            if (sb->state & Style_Sunken) {
                bsx = pixelMetric(PM_ButtonShiftHorizontal);
                bsy = pixelMetric(PM_ButtonShiftVertical);
            }
            p->save();
            p->translate(sx + bsx, sy + bsy);
            p->setPen(sb->palette.buttonText());
            p->setBrush(sb->palette.buttonText());
            p->drawPolygon(a);
            p->restore();
            break;
        }
    case PE_SpinBoxSlider:
        if (const QStyleOptionSpinBox *sb = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            QRect re = sb->rect;
            re.setWidth((int)((double)re.width() * sb->percentage));
            p->fillRect(re, sb->palette.brush(QPalette::Highlight));
            break;
        }
    default:
        break;
    }
}

/*!
    Draws the control \a ce, with style options \a opt, on painter \a
    p. The \a widget is optional and may contain a widget that is
    useful for drawing the control.
*/
void QCommonStyle::drawControl(ControlElement ce, const QStyleOption *opt,
                               QPainter *p, const QWidget *widget) const
{
    switch (ce) {
    case CE_PushButton:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            QRect br = btn->rect;
            int dbi = pixelMetric(PM_ButtonDefaultIndicator, widget);
            if (btn->state & Style_ButtonDefault) {
                drawPrimitive(PE_ButtonDefault, opt, p, widget);
                br.setCoords(br.left() + dbi, br.top() + dbi, br.right() - dbi, br.bottom() - dbi);
            }
            if (!(btn->features & QStyleOptionButton::Flat)
                || btn->state & (Style_Down | Style_On)) {
                QStyleOptionButton tmpBtn = *btn;
                tmpBtn.rect = br;
                drawPrimitive(PE_ButtonCommand, &tmpBtn, p, widget);
            }
            if (btn->features & QStyleOptionButton::HasMenu) {
                int mbi = pixelMetric(PM_MenuButtonIndicator, widget);
                QRect ir = btn->rect;
                QStyleOptionButton newBtn = *btn;
                newBtn.rect = QRect(ir.right() - mbi, ir.height() - 20, mbi, ir.height() - 4);
                drawPrimitive(PE_ArrowDown, &newBtn, p, widget);
            }
        }
        break;
    case CE_PushButtonLabel:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            QRect ir = btn->rect;
            uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
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
                    tf |= Qt::AlignLeft;
            } else {
                tf |= Qt::AlignHCenter;
            }
            drawItem(p, ir, tf, btn->palette, (btn->state & Style_Enabled), QPixmap(), btn->text, -1,
                     &(btn->palette.buttonText().color()));
        }
        break;
    case CE_RadioButton:
    case CE_CheckBox:
        drawPrimitive(ce == CE_RadioButton ? PE_ExclusiveIndicator : PE_Indicator, opt, p, widget);
        break;
    case CE_RadioButtonLabel:
    case CE_CheckBoxLabel:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            bool isRadio = (ce == CE_RadioButtonLabel);
            uint alignment = QApplication::reverseLayout() ? Qt::AlignRight : Qt::AlignLeft;
            if (!styleHint(SH_UnderlineShortcut, widget, Q3StyleOption::Default, 0))
                alignment |= Qt::TextHideMnemonic;
            QPixmap pix;
            if (btn->icon.isNull())
                pix = btn->icon.pixmap(QIconSet::Small, QIconSet::Normal);
            drawItem(p, btn->rect, alignment | Qt::AlignVCenter | Qt::TextShowMnemonic, btn->palette,
                     btn->state & Style_Enabled, pix, btn->text);
            if (btn->state & Style_HasFocus) {
                QStyleOptionFocusRect fropt(0);
                fropt.state = btn->state;
                fropt.palette = btn->palette;
                fropt.rect = visualRect(subRect(isRadio ? SR_RadioButtonFocusRect
                                                        : SR_CheckBoxFocusRect, btn, widget),
                                                widget);
                drawPrimitive(PE_FocusRect, &fropt, p, widget);
            }
        }
        break;
    case CE_MenuTearoff:
        if (opt->state & Style_Active)
            p->fillRect(opt->rect, opt->palette.brush(QPalette::Highlight));
        else
            p->fillRect(opt->rect, opt->palette.brush(QPalette::Button));
        p->setPen(QPen(opt->palette.dark(), 1, Qt::DashLine));
        p->drawLine(opt->rect.x() + 2, opt->rect.y() + opt->rect.height() / 2 - 1,
                    opt->rect.x() + opt->rect.width() - 4,
                    opt->rect.y() + opt->rect.height() / 2 - 1);
        p->setPen(QPen(opt->palette.light(), 1, Qt::DashLine));
        p->drawLine(opt->rect.x() + 2, opt->rect.y() + opt->rect.height() / 2,
                    opt->rect.x() + opt->rect.width() - 4, opt->rect.y() + opt->rect.height() / 2);
        break;
    case CE_MenuBarItem:
        if (const QStyleOptionMenuItem *mbi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            uint alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
            if (!styleHint(SH_UnderlineShortcut, widget, Q3StyleOption(), 0))
                alignment |= Qt::TextHideMnemonic;
            QPixmap pix = mbi->icon.pixmap(QIconSet::Small, QIconSet::Normal);
            drawItem(p, mbi->rect, alignment, mbi->palette, mbi->state & Style_Enabled,
                     pix, mbi->text, -1, &mbi->palette.buttonText().color());
        }
        break;
    case CE_MenuBarEmptyArea:
        if (widget && !widget->testAttribute(Qt::WA_NoSystemBackground))
            p->eraseRect(opt->rect);
        break;
    case CE_ProgressBarGroove:
        qDrawShadePanel(p, opt->rect, opt->palette, true, 1,
                        &opt->palette.brush(QPalette::Background));
        break;
    case CE_ProgressBarLabel:
        if (const QStyleOptionProgressBar *pb = qt_cast<const QStyleOptionProgressBar *>(opt)) {
            QColor penColor = pb->palette.highlightedText();
            QColor *pColor = 0;
            if (pb->features & QStyleOptionProgressBar::CenterIndicator
                && !(pb->features & QStyleOptionProgressBar::IndicatorFollowsStyle)
                && pb->progress * 2 >= pb->totalSteps)
                pColor = &penColor;
            drawItem(p, pb->rect, Qt::AlignCenter | Qt::TextSingleLine, opt->palette,
                     opt->state & Style_Enabled, pb->progressString, -1, pColor);
        }
        break;
    case CE_ProgressBarContents:
        if (const QStyleOptionProgressBar *pb = qt_cast<const QStyleOptionProgressBar *>(opt)) {
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
                QStyleOptionProgressBar pbBits = *pb;
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
        if (const QStyleOptionHeader *header = qt_cast<const QStyleOptionHeader *>(opt)) {
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
                drawItem(p, pixRect, Qt::AlignVCenter, header->palette, (header->state & Style_Enabled)
                         || !header->icon.isGenerated(QIconSet::Small, QIconSet::Disabled), pixmap);
                rect.setLeft(rect.left() + pixw + 2);
            }

            drawItem(p, rect, Qt::AlignVCenter, header->palette, header->state & Style_Enabled,
                     header->text, -1, &(header->palette.buttonText().color()));
        }
        break;
    case CE_ToolButtonLabel:
        if (const QStyleOptionToolButton *tb = qt_cast<const QStyleOptionToolButton *>(opt)) {
            QRect rect = tb->rect;
            int shiftX = 0;
            int shiftY = 0;
            if (tb->state & (Style_Down | Style_On)) {
                shiftX = pixelMetric(PM_ButtonShiftHorizontal, widget);
                shiftY = pixelMetric(PM_ButtonShiftVertical, widget);
            }
            if (tb->features & QStyleOptionToolButton::Arrow) {
                PrimitiveElement pe;
                switch (tb->arrowType) {
                case Qt::LeftArrow:
                    pe = PE_ArrowLeft;
                    break;
                case Qt::RightArrow:
                    pe = PE_ArrowRight;
                    break;
                case Qt::UpArrow:
                    pe = PE_ArrowUp;
                    break;
                case Qt::DownArrow:
                    pe = PE_ArrowDown;
                    break;
                default:
                    return;
                }
                rect.moveBy(shiftX, shiftY);
                QStyleOption arrowOpt(0);
                arrowOpt.rect = rect;
                arrowOpt.palette = tb->palette;
                arrowOpt.state = tb->state;
                drawPrimitive(pe, &arrowOpt, p, widget);
            } else {
                QColor btext = tb->palette.foreground();

                if (tb->icon.isNull() && !tb->text.isEmpty()
                        && !(tb->features & QStyleOptionToolButton::TextLabel)) {
                    int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
                    if (!styleHint(SH_UnderlineShortcut, widget, Q3StyleOption::Default, 0))
                        alignment |= Qt::TextHideMnemonic;
                    rect.moveBy(shiftX, shiftY);
                    drawItem(p, rect, alignment, tb->palette,
                             opt->state & Style_Enabled, tb->text, -1, &btext);
                } else {
                    QPixmap pm;
                    QIconSet::Size size =
                        tb->features & QStyleOptionToolButton::BigPixmap ? QIconSet::Large
                                                                        : QIconSet::Small;
                    QIconSet::State state =
                        tb->state & Style_On ? QIconSet::On : QIconSet::Off;
                    QIconSet::Mode mode;
                    if (!(tb->state & Style_Enabled))
                        mode = QIconSet::Disabled;
                    else if (opt->state & (Style_Down | Style_On) ||
                             ((opt->state & Style_Raised) && (opt->state & Style_AutoRaise)))
                        mode = QIconSet::Active;
                    else
                        mode = QIconSet::Normal;
                    pm = tb->icon.pixmap(size, mode, state);

                    if (tb->features & QStyleOptionToolButton::TextLabel) {
                        p->setFont(tb->font);
                        QRect pr = rect,
                        tr = rect;
                        int alignment = Qt::TextShowMnemonic;
                        if (!styleHint(SH_UnderlineShortcut, widget, Q3StyleOption::Default, 0))
                            alignment |= Qt::TextHideMnemonic;

                        if (tb->textPosition == QToolButton::Under) {
                            int fh = p->fontMetrics().height();
                            pr.addCoords(0, 1, 0, -fh - 3);
                            tr.addCoords(0, pr.bottom(), 0, -3);
                            pr.moveBy(shiftX, shiftY);
                            drawItem(p, pr, Qt::AlignCenter, tb->palette,
                                     mode != QIconSet::Disabled
                                     || !tb->icon.isGenerated(size, mode, state), pm);
                            alignment |= Qt::AlignCenter;
                        } else {
                            pr.setWidth(pm.width() + 8);
                            tr.addCoords(pr.right(), 0, 0, 0);
                            pr.moveBy(shiftX, shiftY);
                            drawItem(p, pr, Qt::AlignCenter, tb->palette,
                                      mode != QIconSet::Disabled
                                      || !tb->icon.isGenerated(size, mode, state), pm);
                            alignment |= Qt::AlignLeft | Qt::AlignVCenter;
                        }
                        tr.moveBy(shiftX, shiftY);
                        drawItem(p, tr, alignment, tb->palette,
                                  tb->state & Style_Enabled, QPixmap(), tb->text,
                                  tb->text.length(), &btext);
                    } else {
                        rect.moveBy(shiftX, shiftY);
                        drawItem(p, rect, Qt::AlignCenter, tb->palette,
                                  mode != QIconSet::Disabled
                                  || !tb->icon.isGenerated(size, mode, state), pm);
                    }
                }
            }
        }
        break;
    case CE_ToolBoxTab:
        if (const QStyleOptionToolBox *tb = qt_cast<const QStyleOptionToolBox *>(opt)) {
            int d = 20 + tb->rect.height() - 3;
            QPointArray a(7);
            a.setPoint(0, -1, tb->rect.height() + 1);
            a.setPoint(1, -1, 1);
            a.setPoint(2, tb->rect.width() - d, 1);
            a.setPoint(3, tb->rect.width() - 20, tb->rect.height() - 2);
            a.setPoint(4, tb->rect.width() - 1, tb->rect.height() - 2);
            a.setPoint(5, tb->rect.width() - 1, tb->rect.height() + 1);
            a.setPoint(6, -1, tb->rect.height() + 1);

            if (tb->state & Style_Selected) {
                p->setBrush(tb->currentWidgetPalette.brush(tb->currentWidgetBGRole));
            } else {
                p->setBrush(tb->palette.brush(tb->bgRole));
            }

            p->setPen(tb->palette.mid().color().dark(150));
            p->drawPolygon(a);
            p->setPen(tb->palette.light());
            p->drawLine(0, 2, tb->rect.width() - d, 2);
            p->drawLine(tb->rect.width() - d - 1, 2, tb->rect.width() - 21, tb->rect.height() - 1);
            p->drawLine(tb->rect.width() - 20, tb->rect.height() - 1,
                        tb->rect.width(), tb->rect.height() - 1);
            p->setBrush(Qt::NoBrush);
        }
        break;
    case CE_TabBarTab:
        if (const QStyleOptionTab *tab = qt_cast<const QStyleOptionTab *>(opt)) {
            if (tab->shape == QTabBar::TriangularAbove || tab->shape == QTabBar::TriangularBelow) {
                QBrush oldBrush = p->brush();
                QPen oldPen = p->pen();
                // triangular, above or below
                int y;
                int x;
                QPointArray a(10);
                a.setPoint(0, 0, -1);
                a.setPoint(1, 0, 0);
                y = tab->rect.height() - 2;
                x = y / 3;
                a.setPoint(2, x++, y - 1);
                a.setPoint(3, x++, y);
                a.setPoint(3, x++, y++);
                a.setPoint(4, x, y);

                int i;
                int right = tab->rect.width() - 1;
                for (i = 0; i < 5; ++i)
                    a.setPoint(9 - i, right - a.point(i).x(), a.point(i).y());

                if (tab->shape == QTabBar::TriangularAbove)
                    for (i = 0; i < 10; ++i)
                        a.setPoint(i, a.point(i).x(), tab->rect.height() - 1 - a.point(i).y());

                a.translate(tab->rect.left(), tab->rect.top());

                if (tab->state & Style_Selected)
                    p->setBrush(tab->palette.base());
                else
                    p->setBrush(tab->palette.background());
                p->setPen(tab->palette.foreground());
                p->drawPolygon(a);
                p->setPen(oldPen);
                p->setBrush(oldBrush);
            }
        }
        break;
    case CE_TabBarLabel:
        if (const QStyleOptionTab *tab = qt_cast<const QStyleOptionTab *>(opt)) {
            QRect tr = tab->rect;
            if (tab->state & Style_Selected)
                tr.setBottom(tr.bottom() - pixelMetric(QStyle::PM_DefaultFrameWidth, widget));

            int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
            if (!styleHint(SH_UnderlineShortcut, widget))
                alignment |= Qt::TextHideMnemonic;
            drawItem(p, tr, alignment, tab->palette, tab->state & Style_Enabled, tab->text);

            if (tab->state & Style_HasFocus && !tab->text.isEmpty()) {
                QStyleOptionFocusRect fropt(0);
                fropt.rect = tab->rect;
                fropt.palette = tab->palette;
                fropt.state = Style_Default;
                drawPrimitive(PE_FocusRect, &fropt, p, widget);
            }
        }
        break;
    case CE_ToolBarButton:
        if (const QStyleOptionButton * const button = qt_cast<const QStyleOptionButton *>(opt)) {
            const QRect cr = subRect(SR_ToolBarButtonContents, opt, widget);

            QIconSet::Mode iconMode = (button->state & Style_Enabled)
                                      ? QIconSet::Normal
                                      : QIconSet::Disabled;
            if (button->state & Style_Down)
                iconMode = QIconSet::Active;
            QIconSet::State iconState = (button->state & Style_On)
                                        ? QIconSet::On
                                        : QIconSet::Off;

            const QPixmap pixmap = button->icon.pixmap(QIconSet::Automatic, iconMode, iconState);

            if (button->state & (Style_On | Style_Down)) {
                qDrawShadePanel(p, cr, button->palette, true);
            } else if (button->state & (Style_MouseOver | Style_Open)) {
                qDrawShadePanel(p, cr, button->palette, false);
            }

            const QRect ir(cr.topLeft() + QPoint(3, 3), QSize(pixmap.width(), cr.height() - 6));
            drawItem(p, ir, Qt::AlignCenter, button->palette,
                     (button->state & Style_Enabled), pixmap);
            if (!button->text.isEmpty()) {
                const QRect tr(ir.topRight() + QPoint(2 + 1, 0),
                               QSize(cr.width() - ir.width() - 9, ir.height()));
                drawItem(p, tr, Qt::AlignLeft | Qt::AlignVCenter,
                         button->palette, (button->state & Style_Enabled), button->text);
            }

            if (button->features & QStyleOptionButton::HasMenu) {
                QStyleOption mopt(0);
                mopt.rect = subRect(SR_ToolBarButtonMenu, opt, widget);
                mopt.palette = button->palette;
                mopt.state = QStyle::Style_Enabled;
                if (button->state & (Style_Down | Style_MouseOver | Style_Open))
                    qDrawShadePanel(p, mopt.rect, button->palette, button->state & Style_Open);
                drawPrimitive(QStyle::PE_ArrowDown, &mopt, p, widget);
            }
        }
        break;
    default:
        break;
    }
}

/*!
    Draws the mask for the given control \a ce, with style options
    \a opt, on painter \a p. The widget \a w is optional and may contain
    a widget that is useful for drawing the primitive.
*/
void QCommonStyle::drawControlMask(ControlElement ce, const QStyleOption *opt, QPainter *p,
                                   const QWidget *w) const
{
    QPalette pal(Qt::color1,Qt::color1,Qt::color1,Qt::color1,Qt::color1,Qt::color1,Qt::color1,Qt::color1,Qt::color0);
    switch (ce) {
    case CE_PushButton:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            QStyleOptionButton newBtn = *btn;
            newBtn.palette = pal;
            drawPrimitive(PE_ButtonCommand, &newBtn, p, w);
        }
        break;
    default:
        p->fillRect(opt->rect, Qt::color1);
    }

}

/*!
    Returns the rectangle occupied by sub-rectangle \a sr, with style
    options \a opt. The widget \a w is optional and may contain a widget
    that is useful for drawing the sub-rectangle.
*/
QRect QCommonStyle::subRect(SubRect sr, const QStyleOption *opt, const QWidget *w) const
{
    QRect r;
    switch (sr) {
    case SR_PushButtonContents:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
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
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
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
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            if (btn->text.isEmpty()) {
                r = subRect(SR_CheckBoxIndicator, opt, w);
                r.addCoords(1, 1, -1, -1);
                break;
            }
            QRect cr = subRect(SR_CheckBoxContents, opt, w);

            if (!btn->icon.isNull()) {
                r = itemRect(cr, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic,
                             btn->icon.pixmap(QIconSet::Small, QIconSet::Normal));
            } else {
                QFontMetrics fm = w->fontMetrics();
                r = itemRect(fm, cr, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic,
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
            QRect ir = subRect(SR_RadioButtonIndicator, opt, w);
            r.setRect(ir.right() + 6, opt->rect.y(),
                      opt->rect.width() - ir.width() - 6, opt->rect.height());
            break;
        }

    case SR_RadioButtonFocusRect:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            if (!btn->icon.isNull() && btn->text.isEmpty()) {
                r = subRect(SR_RadioButtonIndicator, opt, w);
                r.addCoords(1, 1, -1, -1);
                break;
            }
            QRect cr = subRect(SR_RadioButtonContents, opt, w);

            if(!btn->icon.isNull()) {
                r = itemRect(cr, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic,
                             btn->icon.pixmap(QIconSet::Small, QIconSet::Normal));
            } else {
                QFontMetrics fm = w->fontMetrics();
                r = itemRect(fm, cr, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic,
                             btn->state & Style_Enabled, btn->text);
            }
            r.addCoords(-3, -2, 3, 2);
            r = r.intersect(btn->rect);
        }
        break;
    case SR_SliderFocusRect:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            int tickOffset = pixelMetric(PM_SliderTickmarkOffset, w);
            int thickness  = pixelMetric(PM_SliderControlThickness, w);
            if (slider->orientation == Qt::Horizontal)
                r.setRect(0, tickOffset - 1, slider->rect.width(), thickness + 2);
            else
                r.setRect(tickOffset - 1, 0, thickness + 2, slider->rect.height());
            r = r.intersect(slider->rect);
        }
        break;
    case SR_ProgressBarGroove:
    case SR_ProgressBarContents:
    case SR_ProgressBarLabel:
        if (const QStyleOptionProgressBar *pb = qt_cast<const QStyleOptionProgressBar *>(opt)) {
            QFontMetrics fm(w ? w->fontMetrics() : QApplication::fontMetrics());
            int textw = 0;
            if (pb->features & QStyleOptionProgressBar::PercentageVisible)
                textw = fm.width("100%") + 6;

            if (pb->features & QStyleOptionProgressBar::IndicatorFollowsStyle
                || !(pb->features & QStyleOptionProgressBar::CenterIndicator)) {
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
        if (const QStyleOptionDockWindow *dw = qt_cast<const QStyleOptionDockWindow *>(opt)) {
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
    case SR_ToolButtonContents:
        if (const QStyleOptionToolButton *tb = qt_cast<const QStyleOptionToolButton *>(opt))
            r = querySubControlMetrics(CC_ToolButton, tb, SC_ToolButton, w);
        break;
    case SR_ComboBoxFocusRect:
        r.setRect(3, 3, opt->rect.width() - 6 - 16, opt->rect.height() - 6);
        break;
    case SR_ToolBoxTabContents:
        r = opt->rect;
        r.addCoords(0, 0, -30, 0);
        break;
    case SR_ToolBarButtonContents:
        if (const QStyleOptionButton * const button = qt_cast<const QStyleOptionButton *>(opt)) {
            r = opt->rect;
            if (button->features & QStyleOptionButton::HasMenu) {
                r.setWidth(r.width() - 12);
            }
        }
        break;
    case SR_ToolBarButtonMenu:
        if (const QStyleOptionButton * const button = qt_cast<const QStyleOptionButton *>(opt)) {
            if (button->features & QStyleOptionButton::HasMenu) {
                r = opt->rect;
                r.setLeft(r.right() - (12 - 1));
            }
        }
        break;
    default:
        break;
    }
    return r;
}

/*!
    Draws the complex control \a cc, with style options \a opt, on painter
    \a p. The \a widget is optional and may contain a widget that is
    useful for drawing the primitive.
*/
void QCommonStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                      QPainter *p, const QWidget *widget) const
{
    switch (cc) {
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
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
                p->setPen(slider->palette.foreground());
                int v = slider->minimum;
                while (v <= slider->maximum + 1) {
                    pos = QStyle::positionFromValue(slider->minimum, slider->maximum,
                                                    v, available) + fudge;
                    if (slider->orientation == Qt::Horizontal) {
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
        if (const QStyleOptionSlider *scrollbar = qt_cast<const QStyleOptionSlider *>(opt)) {
            // Make a copy here and reset it for each primitive.
            QStyleOptionSlider newScrollbar = *scrollbar;
            SFlags saveFlags = scrollbar->state;
            if (scrollbar->minimum == scrollbar->maximum)
                saveFlags |= Style_Enabled;

            if (scrollbar->parts & SC_ScrollBarSubLine) {
                newScrollbar.state = saveFlags;
                newScrollbar.rect = visualRect(querySubControlMetrics(cc, &newScrollbar,
                                                                      SC_ScrollBarSubLine, widget),
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
                newScrollbar.rect = visualRect(querySubControlMetrics(cc, &newScrollbar,
                                                                      SC_ScrollBarAddLine, widget),
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
                newScrollbar.rect = visualRect(querySubControlMetrics(cc, &newScrollbar,
                                                                      SC_ScrollBarSubPage, widget),
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
                newScrollbar.rect = visualRect(querySubControlMetrics(cc, &newScrollbar,
                                                                      SC_ScrollBarAddPage, widget),
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
                newScrollbar.rect = visualRect(querySubControlMetrics(cc, &newScrollbar,
                                                                      SC_ScrollBarFirst, widget),
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
                newScrollbar.rect = visualRect(querySubControlMetrics(cc, &newScrollbar,
                                                                      SC_ScrollBarLast, widget),
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
                newScrollbar.rect = visualRect(querySubControlMetrics(cc, &newScrollbar,
                                                                      SC_ScrollBarSlider, widget),
                                               widget);
                if (newScrollbar.rect.isValid()) {
                    if (scrollbar->activeParts & SC_ScrollBarSlider)
                        newScrollbar.state |= Style_Down;
                    drawPrimitive(PE_ScrollBarSlider, &newScrollbar, p, widget);

                    if (scrollbar->state & Style_HasFocus) {
                        QStyleOptionFocusRect fropt(0);
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
        if (const QStyleOptionListView *lv = qt_cast<const QStyleOptionListView *>(opt)) {
            if (lv->parts & SC_ListView)
                p->fillRect(lv->rect, lv->viewportPalette.brush(lv->viewportBGRole));
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            QStyleOptionSpinBox copy = *sb;
            PrimitiveElement pe;

            if (sb->parts & SC_SpinBoxFrame)
                qDrawWinPanel(p, sb->rect, sb->palette, true); //cstyle == Sunken);

            if (sb->parts & SC_SpinBoxUp) {
                copy.parts = SC_SpinBoxUp;
                QPalette pal2 = sb->palette;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepUpEnabled)) {
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                }

                copy.palette = pal2;

                if (sb->activeParts == SC_SpinBoxUp) {
                    copy.state |= Style_On;
                    copy.state |= Style_Sunken;
                } else {
                    copy.state |= Style_Raised;
                }
                pe = (sb->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_SpinBoxPlus : PE_SpinBoxUp);

                copy.rect = querySubControlMetrics(CC_SpinBox, &copy, SC_SpinBoxUp, widget);
                drawPrimitive(PE_ButtonBevel, &copy, p, widget);
                drawPrimitive(pe, &copy, p, widget);
            }

            if (sb->parts & SC_SpinBoxDown) {
                copy.parts = SC_SpinBoxDown;
                copy.state = sb->state;
                QPalette pal2 = sb->palette;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepDownEnabled)) {
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                }
                copy.palette = pal2;

                if (sb->activeParts == SC_SpinBoxDown) {
                    copy.state |= Style_On;
                    copy.state |= Style_Sunken;
                } else {
                    copy.state |= Style_Raised;
                }
                pe = (sb->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_SpinBoxMinus : PE_SpinBoxDown);

                copy.rect = sb->rect;
                copy.rect = querySubControlMetrics(CC_SpinBox, &copy, SC_SpinBoxDown, widget);
                drawPrimitive(PE_ButtonBevel, &copy, p, widget);
                drawPrimitive(pe, &copy, p, widget);
            }

            if (sb->parts & PE_SpinBoxSlider) {
                copy.state = sb->state;
                pe = PE_SpinBoxSlider;
                copy.parts = SC_SpinBoxSlider;
                copy.rect = sb->rect;
                copy.rect = querySubControlMetrics(CC_SpinBox, &copy, SC_SpinBoxSlider, widget);
                drawPrimitive(pe, &copy, p, widget);
            }

        }
        break;
    case CC_ToolButton:
        if (const QStyleOptionToolButton *toolbutton
                = qt_cast<const QStyleOptionToolButton *>(opt)) {
            QRect button, menuarea;
            button = visualRect(querySubControlMetrics(cc, toolbutton, SC_ToolButton, widget),
                                widget);
            menuarea = visualRect(querySubControlMetrics(cc, toolbutton, SC_ToolButtonMenu, widget),
                                  widget);

            SFlags bflags = toolbutton->state,
                   mflags = toolbutton->state;

            if (toolbutton->activeParts & SC_ToolButton)
                bflags |= Style_Down;
            if (toolbutton->activeParts & SC_ToolButtonMenu)
                mflags |= Style_Down;

            QStyleOption tool(0);
            tool.palette = toolbutton->palette;
            if (toolbutton->parts & SC_ToolButton) {
                if (bflags & (Style_Down | Style_On | Style_Raised)) {
                    tool.rect = button;
                    tool.state = bflags;
                    drawPrimitive(PE_ButtonTool, &tool, p, widget);
                }
            }

            if (toolbutton->parts & SC_ToolButtonMenu) {
                tool.rect = menuarea;
                tool.state = mflags;
                if (mflags & (Style_Down | Style_On | Style_Raised))
                    drawPrimitive(PE_ButtonDropDown, &tool, p, widget);
                drawPrimitive(PE_ArrowDown, &tool, p, widget);
            }

            if (toolbutton->state & Style_HasFocus) {
                QStyleOptionFocusRect fr(0);
                fr.rect = toolbutton->rect;
                fr.rect.addCoords(3, 3, -3, -3);
                fr.palette = toolbutton->palette;
                fr.state = Style_Default;
                drawPrimitive(PE_FocusRect, &fr, p, widget);
            }
        }
        break;
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *tb = qt_cast<const QStyleOptionTitleBar *>(opt)) {
            QRect ir;
            if (opt->parts & SC_TitleBarLabel) {
                QColor left = tb->palette.highlight();
                QColor right = tb->palette.base();

                if (left != right) {
                    double rS = left.red();
                    double gS = left.green();
                    double bS = left.blue();

                    const double rD = double(right.red() - rS) / tb->rect.width();
                    const double gD = double(right.green() - gS) / tb->rect.width();
                    const double bD = double(right.blue() - bS) / tb->rect.width();

                    const int w = tb->rect.width();
                    for (int sx = 0; sx < w; ++sx) {
                        rS+=rD;
                        gS+=gD;
                        bS+=bD;
                        p->setPen(QColor(int(rS), int(gS), int(bS)));
                        p->drawLine(sx, 0, sx, tb->rect.height());
                    }
                } else {
                    p->fillRect(opt->rect, left);
                }

                ir = visualRect(querySubControlMetrics(CC_TitleBar, tb, SC_TitleBarLabel,
                                                       widget), widget);

                p->setPen(tb->palette.highlightedText());
                p->drawText(ir.x() + 2, ir.y(), ir.width() - 2, ir.height(),
                            Qt::AlignAuto | Qt::AlignVCenter | Qt::TextSingleLine, tb->text);
            }

            bool down = false;
            QPixmap pm;

            QStyleOption tool(0);
            tool.palette = tb->palette;
            if (tb->parts & SC_TitleBarCloseButton) {
                ir = visualRect(querySubControlMetrics(CC_TitleBar, tb, SC_TitleBarCloseButton,
                                                       widget), widget);
                down = tb->activeParts & SC_TitleBarCloseButton;
                if (tb->titleBarFlags & Qt::WStyle_Tool
#ifndef QT_NO_MAINWINDOW
                     || qt_cast<const QDockWindow *>(widget)
#endif
                   )
                    pm = stylePixmap(SP_DockWindowCloseButton, widget);
                else
                    pm = stylePixmap(SP_TitleBarCloseButton, widget);
                tool.rect = ir;
                tool.state = down ? Style_Down : Style_Raised;
                drawPrimitive(PE_ButtonTool, &tool, p, widget);

                p->save();
                if (down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, widget),
                                 pixelMetric(PM_ButtonShiftVertical, widget));
                drawItem(p, ir, Qt::AlignCenter, tb->palette, true, pm);
                p->restore();
            }

            if (tb->parts & SC_TitleBarMaxButton) {
                ir = visualRect(querySubControlMetrics(CC_TitleBar, tb, SC_TitleBarMaxButton,
                                                       widget), widget);

                down = tb->activeParts & SC_TitleBarMaxButton;
                pm = QPixmap(stylePixmap(SP_TitleBarMaxButton, widget));
                tool.rect = ir;
                tool.state = down ? Style_Down : Style_Raised;
                drawPrimitive(PE_ButtonTool, &tool, p, widget);

                p->save();
                if (down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, widget),
                                 pixelMetric(PM_ButtonShiftVertical, widget));
                drawItem(p, ir, Qt::AlignCenter, tb->palette, true, pm);
                p->restore();
            }

            if (tb->parts & SC_TitleBarNormalButton || tb->parts & SC_TitleBarMinButton) {
                ir = visualRect(querySubControlMetrics(CC_TitleBar, tb, SC_TitleBarMinButton,
                                                       widget), widget);
                QStyle::SubControl ctrl = (tb->parts & SC_TitleBarNormalButton ?
                                           SC_TitleBarNormalButton :
                                           SC_TitleBarMinButton);
                QStyle::StylePixmap spixmap = (tb->parts & SC_TitleBarNormalButton ?
                                               SP_TitleBarNormalButton :
                                               SP_TitleBarMinButton);
                down = tb->activeParts & ctrl;
                pm = QPixmap(stylePixmap(spixmap, widget));
                tool.rect = ir;
                tool.state = down ? Style_Down : Style_Raised;
                drawPrimitive(PE_ButtonTool, &tool, p, widget);

                p->save();
                if (down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, widget),
                                 pixelMetric(PM_ButtonShiftVertical, widget));
                drawItem(p, ir, Qt::AlignCenter, tb->palette, true, pm);
                p->restore();
            }

            if (tb->parts & SC_TitleBarShadeButton) {
                ir = visualRect(querySubControlMetrics(CC_TitleBar, tb, SC_TitleBarShadeButton,
                                                       widget), widget);

                down = tb->activeParts & SC_TitleBarShadeButton;
                pm = QPixmap(stylePixmap(SP_TitleBarShadeButton, widget));
                tool.rect = ir;
                tool.state = down ? Style_Down : Style_Raised;
                drawPrimitive(PE_ButtonTool, &tool, p, widget);
                p->save();
                if (down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, widget),
                                 pixelMetric(PM_ButtonShiftVertical, widget));
                drawItem(p, ir, Qt::AlignCenter, tb->palette, true, pm);
                p->restore();
            }

            if (tb->parts & SC_TitleBarUnshadeButton) {
                ir = visualRect(querySubControlMetrics(CC_TitleBar, tb, SC_TitleBarUnshadeButton,
                                                       widget), widget);

                down = tb->activeParts & SC_TitleBarUnshadeButton;
                pm = QPixmap(stylePixmap(SP_TitleBarUnshadeButton, widget));
                tool.rect = ir;
                tool.state = down ? Style_Down : Style_Raised;
                drawPrimitive(PE_ButtonTool, &tool, p, widget);
                p->save();
                if (down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, widget),
                                 pixelMetric(PM_ButtonShiftVertical, widget));
                drawItem(p, ir, Qt::AlignCenter, tb->palette, true, pm);
                p->restore();
            }
#ifndef QT_NO_WIDGET_TOPEXTRA
            if (tb->parts & SC_TitleBarSysMenu) {
                if (!tb->icon.isNull()) {
                    ir = visualRect(querySubControlMetrics(CC_TitleBar, tb, SC_TitleBarSysMenu,
                                                           widget), widget);
                    drawItem(p, ir, Qt::AlignCenter, tb->palette, true, tb->icon);
                }
            }
#endif
        }
        break;
    default:
        qWarning("drawComplexControl control not handled %d", cc);
    }
}

/*!
    Draws the mask for the given complex control \a cc, with style
    options \a opt, on painter \a p. The widget \a w is optional and may
    contain a widget that is useful for drawing the mask.
*/
void QCommonStyle::drawComplexControlMask(ComplexControl , const QStyleOptionComplex *opt,
                                          QPainter *p, const QWidget *) const
{
    p->fillRect(opt->rect, Qt::color1);
}

/*!
    Returns the sub-widget in the complex control \a cc, with style
    options \a opt, at point \a pt, and with parent widget \a widget.

    \sa querySubControlMetrics()
*/
QStyle::SubControl QCommonStyle::querySubControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                                 const QPoint &pt, const QWidget *widget) const
{
    SubControl sc = SC_None;
    switch (cc) {
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            QRect r = visualRect(querySubControlMetrics(cc, slider, SC_SliderHandle, widget),
                                 widget);
            if (r.isValid() && r.contains(pt)) {
                sc = SC_SliderHandle;
            } else {
                r = visualRect(querySubControlMetrics(cc, slider, SC_SliderGroove ,widget), widget);
                if (r.isValid() && r.contains(pt))
                    sc = SC_SliderGroove;
            }
        }
        break;
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollbar = qt_cast<const QStyleOptionSlider *>(opt)) {
            QRect r;
            uint ctrl = SC_ScrollBarAddLine;
            while (ctrl <= SC_ScrollBarGroove) {
                r = visualRect(querySubControlMetrics(cc, scrollbar, QStyle::SubControl(ctrl),
                                                      widget), widget);
                if (r.isValid() && r.contains(pt)) {
                    sc = QStyle::SubControl(ctrl);
                    break;
                }
                ctrl <<= 1;
            }
        }
        break;
    case CC_ListView:
        if (const QStyleOptionListView *lv = qt_cast<const QStyleOptionListView *>(opt)) {
            if (pt.x() >= 0 && pt.x() < lv->treeStepSize)
                sc = SC_ListViewExpand;
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinbox = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            QRect r;
            uint ctrl = SC_SpinBoxUp;
            while (ctrl <= SC_SpinBoxSlider) {
                r = visualRect(querySubControlMetrics(cc, spinbox, QStyle::SubControl(ctrl), widget), widget);
                if (r.isValid() && r.contains(pt)) {
                    sc = QStyle::SubControl(ctrl);
                    break;
                }
                ctrl <<= 1;
            }
        }
        break;

    case CC_TitleBar:
        if (const QStyleOptionTitleBar *tb = qt_cast<const QStyleOptionTitleBar *>(opt)) {
            QRect r;
            uint ctrl = SC_TitleBarLabel;
            bool isMinimized = tb->titleBarState & Qt::WindowMinimized;

            while (ctrl <= SC_TitleBarUnshadeButton) {
                r = visualRect(querySubControlMetrics(cc, tb, QStyle::SubControl(ctrl), widget),
                                                      widget);
                if (r.isValid() && r.contains(pt)) {
                    sc = QStyle::SubControl(ctrl);
                    break;
                }
                ctrl <<= 1;
            }
            if (tb->titleBarFlags & Qt::WStyle_Tool) {
                if (sc == SC_TitleBarMinButton || sc == SC_TitleBarMaxButton) {
                    if (isMinimized)
                        sc = SC_TitleBarUnshadeButton;
                    else
                        sc = SC_TitleBarShadeButton;
                }
            } else if (sc == SC_TitleBarMinButton && isMinimized) {
                sc = QStyle::SC_TitleBarNormalButton;
            }
        }
        break;
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qt_cast<const QStyleOptionComboBox *>(opt)) {
            QRect r;
            uint ctrl = SC_ComboBoxArrow;  // Start here and go down.
            while (ctrl > 0) {
                r = visualRect(querySubControlMetrics(cc, cmb, QStyle::SubControl(ctrl), widget),
                                                      widget);
                if (r.isValid() && r.contains(pt)) {
                    sc = QStyle::SubControl(ctrl);
                    break;
                }
                ctrl >>= 1;
            }
        }
        break;
    default:
        qWarning("QCommonStyle::querySubControl case not handled %d", cc);
    }
    return sc;
}

/*!
    Returns the rectangle occupied by the complex control \a cc, with
    style options \a opt, sub-control \a sc. The \a widget is optional
    and may contain a widget that is useful for drawing the
    sub-control.

    \sa querySubControl()
*/
QRect QCommonStyle::querySubControlMetrics(ComplexControl cc, const QStyleOptionComplex *opt,
                                           SubControl sc, const QWidget *widget) const
{
    QRect ret;
    switch (cc) {
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            int tickOffset = pixelMetric(PM_SliderTickmarkOffset, widget);
            int thickness = pixelMetric(PM_SliderControlThickness, widget);

            switch (sc) {
            case SC_SliderHandle: {
                int sliderPos = 0;
                int len = pixelMetric(PM_SliderLength, widget);
                bool horizontal = slider->orientation == Qt::Horizontal;
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
                if (slider->orientation == Qt::Horizontal)
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
        if (const QStyleOptionSlider *scrollbar = qt_cast<const QStyleOptionSlider *>(opt)) {
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
            switch (sc) {
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
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinbox = qt_cast<const QStyleOptionSpinBox *>(opt)) {

            int fw = spinbox->frame ? pixelMetric(PM_SpinBoxFrameWidth, widget) : 0;
            int slider = spinbox->slider ? qMax((int)(spinbox->rect.height() / 20),
                                                pixelMetric(PM_SpinBoxSliderHeight, widget)) : 0;
            QSize bs;

            bs.setHeight(qMax(8, spinbox->rect.height()/2 - fw));
            bs.setWidth(qMax(14, qMin(bs.height() * 8 / 5, spinbox->rect.width() / 4))); // 1.6 -approximate golden mean
            bs = bs.expandedTo(QApplication::globalStrut());
            int y = fw;
            int x, lx, rx;
            x = (QApplication::reverseLayout() ? fw : spinbox->rect.width() - y - bs.width());
            lx = (QApplication::reverseLayout() ? bs.width() + fw : fw);
            rx = x - fw;
            switch (sc) {
            case SC_SpinBoxUp:
                ret = QRect(x, y, bs.width(), bs.height()); break;
            case SC_SpinBoxDown:
                ret = QRect(x, y + bs.height(), bs.width(), bs.height()); break;
            case SC_SpinBoxButtonField:
                ret = QRect(x, y, bs.width(), spinbox->rect.height() - 2*fw); break;
            case SC_SpinBoxEditField:
                ret = QRect(lx, fw, rx, spinbox->rect.height() - 2*fw - slider); break;
            case SC_SpinBoxSlider:
                ret = (slider > 0 ? QRect(lx, spinbox->rect.height() - fw - slider, rx, slider) : QRect()); break;
            case SC_SpinBoxFrame:
                ret = spinbox->frame ? spinbox->rect : QRect();
            default:
                break;
            }
        }
        break;
    case CC_ToolButton:
        if (const QStyleOptionToolButton *tb = qt_cast<const QStyleOptionToolButton *>(opt)) {
            int mbi = pixelMetric(PM_MenuButtonIndicator, widget);
            ret = tb->rect;
            switch (sc) {
            case SC_ToolButton:
                if ((tb->features
                     & (QStyleOptionToolButton::Menu | QStyleOptionToolButton::PopupDelay))
                    == QStyleOptionToolButton::Menu)
                    ret.addCoords(0, 0, -mbi, 0);
                break;
            case SC_ToolButtonMenu:
                if ((tb->features
                     & (QStyleOptionToolButton::Menu | QStyleOptionToolButton::PopupDelay))
                    == QStyleOptionToolButton::Menu)
                    ret.addCoords(ret.width() - mbi, 0, 0, 0);
                break;
            default:
                break;
            }
        }
        break;
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qt_cast<const QStyleOptionComboBox *>(opt)) {
            int x = 0,
                y = 0,
                wi = cmb->rect.width(),
                he = cmb->rect.height();
            int xpos = x;
            xpos += wi - 2 - 16;

            switch (sc) {
            case SC_ComboBoxFrame:
                ret = cmb->rect;
                break;
            case SC_ComboBoxArrow:
                ret.setRect(xpos, y + 2, 16, he - 4);
                break;
            case SC_ComboBoxEditField:
                ret.setRect(x + 3, y + 3, wi - 6 - 16, he - 6);
                break;
            case SC_ComboBoxListBoxPopup:
                ret = cmb->popupRect;
                break;
            default:
                break;
            }
        }
        break;
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *tb = qt_cast<const QStyleOptionTitleBar *>(opt)) {
            const int controlTop = 2;
            const int controlHeight = tb->rect.height() - controlTop * 2;

            switch (sc) {
            case SC_TitleBarLabel:
                ret.setRect(0, 0, tb->rect.width(), tb->rect.height());
                if (tb->titleBarFlags & Qt::WStyle_Tool) {
                    if (tb->titleBarFlags & Qt::WStyle_SysMenu)
                        ret.addCoords(0, 0, -controlHeight - 3, 0);
                    if (tb->titleBarFlags & Qt::WStyle_MinMax)
                        ret.addCoords(0, 0, -controlHeight - 2, 0);
                } else {
                    if (tb->titleBarFlags & Qt::WStyle_SysMenu)
                        ret.addCoords(controlHeight + 3, 0, -controlHeight - 3, 0);
                    if (tb->titleBarFlags & Qt::WStyle_Minimize)
                        ret.addCoords(0, 0, -controlHeight - 2, 0);
                    if (tb->titleBarFlags & Qt::WStyle_Maximize)
                        ret.addCoords(0, 0, -controlHeight - 2, 0);
                }
                break;
            case SC_TitleBarCloseButton:
                ret.setRect(tb->rect.width() - (controlHeight + controlTop),
                            controlTop, controlHeight, controlHeight);
                break;
            case SC_TitleBarMaxButton:
            case SC_TitleBarShadeButton:
            case SC_TitleBarUnshadeButton:
                ret.setRect(tb->rect.width() - ((controlHeight + controlTop) * 2),
                            controlTop, controlHeight, controlHeight);
                break;
            case SC_TitleBarMinButton:
            case SC_TitleBarNormalButton: {
                int offset = controlHeight + controlTop;
                if (!(tb->titleBarFlags & Qt::WStyle_Maximize))
                    offset *= 2;
                else
                    offset *= 3;
                ret.setRect(tb->rect.width() - offset, controlTop, controlHeight, controlHeight);
                break;
            }
            case SC_TitleBarSysMenu:
                ret.setRect(3, controlTop, controlHeight, controlHeight);
                break;
            default:
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
            if (widget->testWFlags(Qt::WStyle_Tool)) {
                ret = qMax(widget->fontMetrics().lineSpacing(), 16);
#ifndef QT_NO_MAINWINDOW
            } else if (qt_cast<const QDockWindow*>(widget)) {
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
            int space = (sl->orientation() == Qt::Horizontal) ? sl->height() :
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
            if (sl->orientation() == Qt::Horizontal)
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

    case PM_SpinBoxSliderHeight:
    case PM_MenuBarFrameWidth:
        ret = 2;
        break;

    case PM_MenuBarItemSpacing:
        ret = 8;
        break;

    case PM_ToolBarItemSpacing:
        ret = 4;
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

    case PM_DefaultToplevelMargin:
        ret = 11;
        break;
    case PM_DefaultChildMargin:
        ret = 9;
        break;
    case PM_DefaultLayoutSpacing:
        ret = 6;
        break;

    default:
        ret = 0;
        break;
    }

    return ret;
}

/*!
    Returns the size required by the contents of type \a ct, with
    style options \a opt, original size \a csz, font metrics \a fm.
    The \a widget is optional and may contain a widget that is useful
    for calculating the size.
*/
QSize QCommonStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &csz,
                                     const QFontMetrics &fm, const QWidget *widget) const
{
    QSize sz(csz);
    switch (ct) {
    case CT_PushButton:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
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
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
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
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            bool checkable = mi->checkState != QStyleOptionMenuItem::NotCheckable;
            int maxpmw = mi->maxIconWidth;
            int w = sz.width(),
                h = sz.height();
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
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
    case CT_ToolButton:
        sz = QSize(sz.width() + 6, sz.height() + 5);
        break;
    case CT_ComboBox: {
        int dfw = pixelMetric(PM_DefaultFrameWidth, widget) * 2;
        sz = QSize(sz.width() + dfw + 21, sz.height() + dfw);
        break; }
    case CT_ToolBarButton:
        if (const QStyleOptionButton * const button = qt_cast<const QStyleOptionButton *>(opt)) {
            sz += QSize(6, 6); // for the icon
            if (!button->text.isEmpty())
                sz.rwidth() += 3; // between the text and the icon
            if (button->features & QStyleOptionButton::HasMenu)
                sz.rwidth() += 12;
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
int QCommonStyle::styleHint(StyleHint sh, const QWidget * w, const Q3StyleOption &, QStyleHintReturn *) const
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
        ret = Qt::WindowsStyle;
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

    case SH_TipLabel_Opacity:
        ret = 255;
        break;

    default:
        ret = 0;
        break;
    }

    return ret;
}

/*! \reimp */
QPixmap QCommonStyle::stylePixmap(StylePixmap, const QWidget *, const Q3StyleOption&) const
{
    return QPixmap();
}

/*! \reimp */
QPixmap QCommonStyle::stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap,
                                   const QPalette &pal, const Q3StyleOption &) const
{
    switch(pixmaptype) {
    case PT_Disabled: {
        QBitmap pixmapMask;
        if (pixmap.mask()) {
            pixmapMask = *pixmap.mask();
        } else {
            QImage img = pixmap.convertToImage();
            pixmapMask.convertFromImage(img.createHeuristicMask(),
                                         Qt::MonoOnly | Qt::ThresholdDither);
        }
        QPixmap ret(pixmap.width() + 1, pixmap.height() + 1);
        ret.fill(pal.color(QPalette::Disabled, QPalette::Background));

        QPainter painter;
        painter.begin(&ret);
        painter.setPen(pal.color(QPalette::Disabled, QPalette::Light));
        painter.drawPixmap(1, 1, pixmapMask);
        painter.setPen(pal.color(QPalette::Disabled, QPalette::Foreground));
        painter.drawPixmap(0, 0, pixmapMask);
        painter.end();

        QBitmap mask(ret.size());
        mask.fill(Qt::color0);
        painter.begin(&mask);
        painter.drawPixmap(0, 0, pixmapMask);
        painter.drawPixmap(1, 1, pixmapMask);
        painter.end();
        ret.setMask(mask);
        return ret;
    }
    case PT_Active:
        return pixmap;
    default:
        break;
    }
    return pixmap;
}

#endif // QT_NO_STYLE
