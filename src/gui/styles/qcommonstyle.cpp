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
#include "qdockwindow.h"
#include "qdrawutil.h"
#include "qgroupbox.h"
#include "qmenu.h"
#include "qpainter.h"
#include "qslider.h"
#include "qstyleoption.h"
#include "qtabbar.h"
#include "qtoolbutton.h"

#ifndef QT_NO_DEBUG
#include <QtCore/qdebug.h>
#endif

#include <limits.h>

/*!
    \class QCommonStyle
    \brief The QCommonStyle class encapsulates the common Look and Feel of a GUI.

    \ingroup appearance

    This abstract class implements some of the widget's look and feel
    that is common to all GUI styles provided and shipped as part of
    Qt.

    All the functions are full documented in \l{QStyle}, although the
    extra functions that QCommonStyle provides, e.g.
    drawComplexControl(), drawControl(), drawPrimitive(),
    hitTestComplexControl(), subControlRect(), sizeFromContents(), and
    subRect() are documented here.
*/

/*!
    \enum Qt::ArrowType

    \value UpArrow
    \value DownArrow
    \value LeftArrow
    \value RightArrow

*/

/*!
    Constructs a QCommonStyle.
*/
QCommonStyle::QCommonStyle()
    : QStyle()
{ }

/*!
    \overload

    Destroys the style
*/
QCommonStyle::~QCommonStyle()
{ }


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

static const char * const tb_extension_arrow_v_xpm[] = {
    "7 9 3 1",
    "            c None",
    ".            c #000000",
    "+            c none",
    ".+++++.",
    "..+++..",
    "+..+..+",
    "++...++",
    ".++.++.",
    "..+++..",
    "+..+..+",
    "++...++",
    "+++.+++"
};

static const char * const tb_extension_arrow_h_xpm[] = {
    "9 7 3 1",
    "            c None",
    ".            c #000000",
    "+            c none",
    "..++..+++",
    "+..++..++",
    "++..++..+",
    "+++..++..",
    "++..++..+",
    "+..++..++",
    "..++..+++"
};

static const char * const filedialog_start_xpm[]={
    "16 15 8 1",
    "a c #cec6bd",
    "# c #000000",
    "e c #ffff00",
    "b c #999999",
    "f c #cccccc",
    "d c #dcdcdc",
    "c c #ffffff",
    ". c None",
    ".....######aaaaa",
    "...bb#cccc##aaaa",
    "..bcc#cccc#d#aaa",
    ".bcef#cccc#dd#aa",
    ".bcfe#cccc#####a",
    ".bcef#ccccccccc#",
    "bbbbbbbbbbbbccc#",
    "bccccccccccbbcc#",
    "bcefefefefee#bc#",
    ".bcefefefefef#c#",
    ".bcfefefefefe#c#",
    "..bcfefefefeeb##",
    "..bbbbbbbbbbbbb#",
    "...#############",
    "................"};

static const char * const filedialog_end_xpm[]={
    "16 15 9 1",
    "d c #a0a0a0",
    "c c #c3c3c3",
    "# c #cec6bd",
    ". c #000000",
    "f c #ffff00",
    "e c #999999",
    "g c #cccccc",
    "b c #ffffff",
    "a c None",
    "......####aaaaaa",
    ".bbbb..###aaaaaa",
    ".bbbb.c.##aaaaaa",
    ".bbbb....ddeeeea",
    ".bbbbbbb.bbbbbe.",
    ".bbbbbbb.bcfgfe.",
    "eeeeeeeeeeeeefe.",
    "ebbbbbbbbbbeege.",
    "ebfgfgfgfgff.ee.",
    "aebfgfgfgfgfg.e.",
    "aebgfgfgfgfgf.e.",
    "aaebgfgfgfgffe..",
    "aaeeeeeeeeeeeee.",
    "aaa.............",
    "aaaaaaaaaaaaaaaa"};

static const char* const filedialog_toparent_xpm[]={
    "15 13 3 1",
    ". c None",
    "* c #000000",
    "a c #ffff99",
    "..*****........",
    ".*aaaaa*.......",
    "***************",
    "*aaaaaaaaaaaaa*",
    "*aaaa*aaaaaaaa*",
    "*aaa***aaaaaaa*",
    "*aa*****aaaaaa*",
    "*aaaa*aaaaaaaa*",
    "*aaaa*aaaaaaaa*",
    "*aaaa******aaa*",
    "*aaaaaaaaaaaaa*",
    "*aaaaaaaaaaaaa*",
    "***************"};

static const char* const filedialog_newfolder_xpm[] = {
    "15 14 4 1",
    "        c None",
    ".        c #000000",
    "+        c #FFFF00",
    "@        c #FFFFFF",
    "          .    ",
    "               ",
    "          .    ",
    "       .     . ",
    "  ....  . . .  ",
    " .+@+@.  . .   ",
    "..........  . .",
    ".@+@+@+@+@..   ",
    ".+@+@+@+@+. .  ",
    ".@+@+@+@+@.  . ",
    ".+@+@+@+@+.    ",
    ".@+@+@+@+@.    ",
    ".+@+@+@+@+.    ",
    "...........    "};

static const char* const filedialog_detailedview_xpm[]={
    "14 11 3 1",
    ". c None",
    "* c #000000",
    "a c #000099",
    ".****.***.***.",
    "..............",
    "aaaaaaaaaaaaaa",
    "..............",
    ".****.***.***.",
    "..............",
    ".****.***.***.",
    "..............",
    ".****.***.***.",
    "..............",
    ".****.***.***."};

static const char* const filedialog_previewinfoview_xpm[]={
    "13 13 4 1",
    ". c #00007f",
    "a c black",
    "# c #cec6bd",
    "b c #000000",
    "..#####aaaaaa",
    ".#.#bb#a#####",
    "...####a#bbb#",
    "#######a#####",
    "#######a#bb##",
    "..#####a#####",
    ".#.#bb#a#bbb#",
    "...####a#####",
    "#######a#bb##",
    "#######a#####",
    "..#####a#bbb#",
    ".#.#bb#a#####",
    "...####aaaaaa"};

static const char* const filedialog_previewcontentsview_xpm[]={
    "14 13 5 1",
    ". c #00007f",
    "a c black",
    "c c #7f007f",
    "# c #cec6bd",
    "b c #000000",
    "..#####aaaaaaa",
    ".#.#bb#a#####a",
    "...####a#ccc#a",
    "#######a#ccc#a",
    "#######a#####a",
    "..#####a#bbb#a",
    ".#.#bb#a#####a",
    "...####a#bbb#a",
    "#######a#####a",
    "#######a#bbb#a",
    "..#####a#####a",
    ".#.#bb#a#####a",
    "...####aaaaaaa"};

static const char* const filedialog_mclistview_xpm[]={
    "15 11 4 1",
    "* c None",
    "b c #000000",
    ". c #000099",
    "a c #ffffff",
    "...*****...****",
    ".a.*bbb*.a.*bbb",
    "...*****...****",
    "***************",
    "...*****...****",
    ".a.*bbb*.a.*bbb",
    "...*****...****",
    "***************",
    "...*****...****",
    ".a.*bbb*.a.*bbb",
    "...*****...****"};

static const char * const filedialog_back_xpm [] = {
    "13 11 3 1",
    "a c #00ffff",
    "# c #000000",
    ". c None",
    ".....#.......",
    "....##.......",
    "...#a#.......",
    "..#aa########",
    ".#aaaaaaaaaa#",
    "#aaaaaaaaaaa#",
    ".#aaaaaaaaaa#",
    "..#aa########",
    "...#a#.......",
    "....##.......",
    ".....#......."};

/*!
    Draws the primitive element \a pe, with style options \a opt, on
    painter \a p. The \a widget is optional and may contain a widget
    that is useful for drawing the primitive.
*/
void QCommonStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                                 const QWidget *widget) const
{
    switch (pe) {
    case PE_FrameButtonBevel:
    case PE_FrameButtonTool:
        qDrawShadeRect(p, opt->rect, opt->palette,
                       opt->state & (State_Sunken | State_Down | State_On), 1, 0);
        break;
    case PE_PanelButtonCommand:
    case PE_PanelButtonBevel:
    case PE_PanelButtonTool:
    case PE_IndicatorButtonDropDown:
        qDrawShadePanel(p, opt->rect, opt->palette,
                        opt->state & (State_Sunken | State_Down | State_On), 1,
                        &opt->palette.brush(QPalette::Button));
        break;
    case PE_IndicatorCheckBox:
        if (opt->state & State_NoChange) {
            p->setPen(opt->palette.foreground().color());
            p->fillRect(opt->rect, opt->palette.brush(QPalette::Button));
            p->drawRect(opt->rect);
            p->drawLine(opt->rect.topLeft(), opt->rect.bottomRight());
        } else {
            qDrawShadePanel(p, opt->rect.x(), opt->rect.y(), opt->rect.width(), opt->rect.height(),
                            opt->palette, opt->state & (State_Sunken | State_On), 1,
                            &opt->palette.brush(QPalette::Button));
        }
        break;
    case PE_IndicatorRadioButton: {
        QRect ir = opt->rect;
        p->setPen(opt->palette.dark().color());
        p->drawArc(opt->rect, 0, 5760);
        if (opt->state & (State_Sunken | State_On)) {
            ir.addCoords(2, 2, -2, -2);
            p->setBrush(opt->palette.foreground());
            p->drawEllipse(ir);
        }
        break; }
    case PE_FrameFocusRect:
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
                p->setPen(opt->palette.foreground().color());
            }
            p->drawRect(QRect(opt->rect.x() + 1, opt->rect.y() + 1, opt->rect.width() - 2,
                              opt->rect.height() - 2));
            p->setPen(oldPen);
        }
        break;
    case PE_IndicatorMenuCheckMark: {
        const int markW = opt->rect.width() > 7 ? 7 : opt->rect.width();
        const int markH = markW;
        int posX = opt->rect.x() + (opt->rect.width() - markW)/2 + 1;
        int posY = opt->rect.y() + (opt->rect.height() - markH)/2;

        QVector<QLineF> a(markH);
        int i, xx, yy;
        xx = posX;
        yy = 3 + posY;
        for (i = 0; i < markW; ++i) {
            a << QLineF(xx, yy, xx, yy + 2);
            ++xx;
            ++yy;
        }
        yy -= 2;
        for (; i < markH; ++i) {
            a << QLineF(xx, yy, xx, yy + 2);
            ++xx;
            --yy;
        }
        if (!(opt->state & State_Enabled) && !(opt->state & State_On)) {
            int pnt;
            p->setPen(opt->palette.highlightedText().color());
            QPoint offset(1, 1);
            for (pnt = 0; pnt < a.size(); ++pnt)
                a[pnt] += offset;
            p->drawLines(a);
            for (pnt = 0; pnt < a.size(); ++pnt)
                a[pnt] -= offset;
        }
        p->setPen(opt->palette.text().color());
        p->drawLines(a);
        break; }
    case PE_Frame:
    case PE_FrameMenu:
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt))
            qDrawShadePanel(p, frame->rect, frame->palette, frame->state & State_Sunken,
                            frame->lineWidth);
        break;
    case PE_PanelMenuBar:
    case PE_PanelToolBar:
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt))
            qDrawShadePanel(p, frame->rect, frame->palette, false, frame->lineWidth,
                            &frame->palette.brush(QPalette::Button));
        break;
    case PE_IndicatorProgressChunk:
        p->fillRect(opt->rect.x(), opt->rect.y() + 3, opt->rect.width() -2, opt->rect.height() - 6,
                    opt->palette.brush(QPalette::Highlight));
        break;
    case PE_Q3CheckListController:
        p->drawPixmap(opt->rect.topLeft(), QPixmap(check_list_controller_xpm));
        break;
    case PE_Q3CheckListExclusiveIndicator:
        if (const QStyleOptionQ3ListView *lv = qt_cast<const QStyleOptionQ3ListView *>(opt)) {
            if (lv->items.isEmpty())
                return;
            int x = lv->rect.x(),
                y = lv->rect.y();
#define INTARRLEN(x) sizeof(x)/(sizeof(int)*2)
            static const int pts1[] = {                // dark lines
                1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
            static const int pts2[] = {                // black lines
                2,8, 1,7, 1,4, 2,3, 2,2, 3,2, 4,1, 7,1, 8,2, 9,2 };
            static const int pts3[] = {                // background lines
                2,9, 3,9, 4,10, 7,10, 8,9, 9,9, 9,8, 10,7, 10,4, 9,3 };
            static const int pts4[] = {                // white lines
                2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
                11,4, 10,3, 10,2 };
            // static const int pts5[] = {                // inner fill
            //    4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };
            //QPolygon a;

            if (lv->state & State_Enabled)
                p->setPen(lv->palette.text().color());
            else
                p->setPen(QPen(lv->viewportPalette.color(QPalette::Disabled, QPalette::Text)));
            QPolygon a(INTARRLEN(pts1), pts1);
            a.translate(x, y);
            //p->setPen(pal.dark());
            p->drawPolyline(a);
            a.setPoints(INTARRLEN(pts2), pts2);
            a.translate(x, y);
            p->drawPolyline(a);
            a.setPoints(INTARRLEN(pts3), pts3);
            a.translate(x, y);
            //                p->setPen(black);
            p->drawPolyline(a);
            a.setPoints(INTARRLEN(pts4), pts4);
            a.translate(x, y);
            //                        p->setPen(blue);
            p->drawPolyline(a);
            //                a.setPoints(INTARRLEN(pts5), pts5);
            //                a.translate(x, y);
            //        QColor fillColor = isDown() ? g.background() : g.base();
            //        p->setPen(fillColor);
            //        p->setBrush(fillColor);
            //        p->drawPolygon(a);
            if (opt->state & State_On) {
                p->setPen(Qt::NoPen);
                p->setBrush(opt->palette.text());
                p->drawRect(x + 5, y + 4, 2, 4);
                p->drawRect(x + 4, y + 5, 4, 2);
            }
#undef INTARRLEN
        }
        break;
    case PE_Q3CheckListIndicator:
        if (const QStyleOptionQ3ListView *lv = qt_cast<const QStyleOptionQ3ListView *>(opt)) {
            if(lv->items.isEmpty())
                break;
            QStyleOptionQ3ListViewItem item = lv->items.at(0);
            int x = lv->rect.x(),
                y = lv->rect.y(),
                w = lv->rect.width(),
                h = lv->rect.width(),
             marg = lv->itemMargin;

            if (lv->state & State_Enabled)
                p->setPen(QPen(lv->palette.text().color(), 2));
            else
                p->setPen(QPen(lv->viewportPalette.color(QPalette::Disabled, QPalette::Text), 2));
            if (opt->state & State_Selected && !lv->rootIsDecorated
                && !(item.features & QStyleOptionQ3ListViewItem::ParentControl)) {
                p->fillRect(0, 0, x + marg + w + 4, item.height,
                            lv->palette.brush(QPalette::Highlight));
                if (item.state & State_Enabled)
                    p->setPen(QPen(lv->palette.highlightedText().color(), 2));
            }

            if (lv->state & State_NoChange)
                p->setBrush(lv->palette.brush(QPalette::Button));
            p->drawRect(x + marg, y + 2, w - 4, h - 4);
            /////////////////////
                ++x;
                ++y;
                if (lv->state & State_On || lv->state & State_NoChange) {
                    QLineF lines[7];
                    int i,
                        xx = x + 1 + marg,
                        yy = y + 5;
                    for (i = 0; i < 3; ++i) {
                        lines[i] = QLineF(xx, yy, xx, yy + 2);
                        ++xx;
                        ++yy;
                    }
                    yy -= 2;
                    for (i = 3; i < 7; ++i) {
                        lines[i] = QLineF(xx, yy, xx, yy + 2);
                        ++xx;
                        --yy;
                    }
                    p->drawLines(lines, 7);
                }
        }
        break;
    case PE_IndicatorBranch: {
        static QPixmap open(tree_branch_open_xpm);
        static QPixmap closed(tree_branch_closed_xpm);
        static const int decoration_size = 9;
        int mid_h = opt->rect.x() + opt->rect.width() / 2;
        int mid_v = opt->rect.y() + opt->rect.height() / 2;
        int bef_h = mid_h;
        int bef_v = mid_v;
        int aft_h = mid_h;
        int aft_v = mid_v;
        if (opt->state & State_Children) {
            int delta = decoration_size / 2;
            bef_h -= delta;
            bef_v -= delta;
            aft_h += delta;
            aft_v += delta;
            p->drawPixmap(bef_h, bef_v, opt->state & State_Open ? open : closed);
        }
        if (opt->state & State_Item) {
            if (opt->direction == Qt::RightToLeft)
                p->drawLine(opt->rect.left(), mid_v, bef_h, mid_v);
            else
                p->drawLine(aft_h, mid_v, opt->rect.right(), mid_v);
        }
        if (opt->state & State_Sibling)
            p->drawLine(mid_h, aft_v, mid_h, opt->rect.bottom());
        if (opt->state & (State_Open | State_Children | State_Item | State_Sibling))
            p->drawLine(mid_h, opt->rect.y(), mid_h, bef_v);
        break; }
    case PE_Q3Separator:
        qDrawShadeLine(p, opt->rect.left(), opt->rect.top(), opt->rect.right(), opt->rect.bottom(),
                       opt->palette, opt->state & State_Sunken, 1, 0);
        break;
    case PE_FrameStatusBar:
        qDrawShadeRect(p, opt->rect, opt->palette, true, 1, 0, 0);
        break;
    case PE_IndicatorHeaderArrow: {
        QPen oldPen = p->pen();
        if (opt->state & State_Up) {
            QPolygon pa(3);
            p->setPen(opt->palette.light().color());
            p->drawLine(opt->rect.x() + opt->rect.width(), opt->rect.y(),
                        opt->rect.x() + opt->rect.width() / 2, opt->rect.height());
            p->setPen(opt->palette.dark().color());
            pa.setPoint(0, opt->rect.x() + opt->rect.width() / 2, opt->rect.height());
            pa.setPoint(1, opt->rect.x(), opt->rect.y());
            pa.setPoint(2, opt->rect.x() + opt->rect.width(), opt->rect.y());
            p->drawPolyline(pa);
        } else if (opt->state & State_Down) {
            QPolygon pa(3);
            p->setPen(opt->palette.light().color());
            pa.setPoint(0, opt->rect.x(), opt->rect.height());
            pa.setPoint(1, opt->rect.x() + opt->rect.width(), opt->rect.height());
            pa.setPoint(2, opt->rect.x() + opt->rect.width() / 2, opt->rect.y());
            p->drawPolyline(pa);
            p->setPen(opt->palette.dark().color());
            p->drawLine(opt->rect.x(), opt->rect.height(),
                        opt->rect.x() + opt->rect.width() / 2, opt->rect.y());
        }
        p->setPen(oldPen);
        break; }
    case PE_FrameTabWidget:
        if (const QStyleOptionTabWidgetFrame *twf
                = qt_cast<const QStyleOptionTabWidgetFrame *>(opt)) {
            QPen oldPen = p->pen();
            p->setPen(opt->palette.light().color());
            switch (twf->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                p->drawLine(opt->rect.left(), opt->rect.top(),
                            opt->rect.left(), opt->rect.bottom() - 1);
                p->setPen(opt->palette.shadow().color());
                p->drawLine(opt->rect.left(), opt->rect.bottom() + 1,
                            opt->rect.right(), opt->rect.bottom() + 1);
                p->setPen(opt->palette.dark().color());
                p->drawLine(opt->rect.left(), opt->rect.bottom(),
                            opt->rect.right() - 1, opt->rect.bottom());
                p->drawLine(opt->rect.right(), opt->rect.bottom() - 1,
                            opt->rect.right(), opt->rect.top());
                break;
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth: {
                int top = opt->rect.top();
                p->drawLine(opt->rect.left(), opt->rect.bottom(), opt->rect.left(), top + 1);
                p->drawLine(opt->rect.left(), top, opt->rect.right(), top);
                p->drawLine(opt->rect.right(), top + 1, opt->rect.right(), opt->rect.bottom());
                break; }
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                p->drawLine(opt->rect.left() + 1, opt->rect.top(),
                            opt->rect.right() - 1, opt->rect.top());
                p->setPen(opt->palette.shadow().color());
                p->drawLine(opt->rect.left() + 1, opt->rect.bottom(),
                            opt->rect.right(), opt->rect.bottom());
                p->setPen(opt->palette.dark().color());
                p->drawLine(opt->rect.left() + 1, opt->rect.bottom() - 1,
                            opt->rect.right() - 1, opt->rect.bottom() - 1);
                p->drawLine(opt->rect.right(), opt->rect.bottom() - 2,
                            opt->rect.right(), opt->rect.top());
                break;
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                p->drawLine(opt->rect.left() + 1, opt->rect.top(),
                            opt->rect.right() - 2, opt->rect.top());
                p->drawLine(opt->rect.left(), opt->rect.top(),
                            opt->rect.left(), opt->rect.bottom() - 1);
                p->drawLine(opt->rect.left(), opt->rect.bottom(),
                            opt->rect.right() - 2, opt->rect.bottom());
                p->setPen(opt->palette.shadow().color());
                p->drawLine(opt->rect.left(), opt->rect.bottom(),
                            opt->rect.right(), opt->rect.bottom());
                p->setPen(opt->palette.dark().color());
                p->drawLine(opt->rect.left(), opt->rect.bottom() -1,
                            opt->rect.right() - 1, opt->rect.bottom() - 1);
                break;
            }
            p->setPen(oldPen);
        }
        break;
    case PE_FrameTabBarBase:
        if (const QStyleOptionTabWidgetFrame *twf
                = qt_cast<const QStyleOptionTabWidgetFrame *>(opt)) {
            QPen oldPen = p->pen();
            QRect r2 = opt->rect;
            switch (twf->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                p->setPen(opt->palette.light().color());
                p->drawLine(r2.left(), r2.bottom() - 1, r2.left(), r2.bottom());
                p->drawLine(r2.left(), r2.bottom() - 1, r2.right(), r2.bottom() - 1);
                p->setPen(opt->palette.dark().color());
                p->drawLine(r2.right(), r2.bottom() - 1, r2.right(), r2.bottom());
                break;
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                p->setPen(opt->palette.shadow().color());
                p->drawLine(r2.left(), r2.top()+ 1, r2.right(), r2.top() + 1);
                p->setPen(opt->palette.dark().color());
                p->drawLine(r2.left(), r2.top(), r2.right()- 1, r2.top());
                break;
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                p->setPen(opt->palette.shadow().color());
                p->drawLine(r2.right(), r2.bottom(), r2.left(), r2.bottom());
                p->setPen(opt->palette.dark().color());
                p->drawLine(r2.right(), r2.bottom() - 1, r2.left(), r2.bottom() - 1);
                p->setPen(opt->palette.light().color());
                p->drawLine(r2.left(), r2.bottom() - 2, r2.left(), r2.top() - 1);
                p->drawLine(r2.right(), r2.top(), r2.left(), r2.top());
                break;
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                p->setPen(opt->palette.shadow().color());
                p->drawLine(r2.left(), r2.bottom(), r2.right() + 1, r2.bottom());
                p->setPen(opt->palette.dark().color());
                p->drawLine(r2.left(), r2.bottom() - 1, r2.right(), r2.bottom() - 1);
                p->drawLine(r2.right() + 1, r2.bottom() - 2, r2.right() + 1, r2.top());
                p->setPen(opt->palette.light().color());
                p->drawLine(r2.left(), r2.top(), r2.right(), r2.top());
                break;
            }
            p->setPen(oldPen);
        }
        break;
    case PE_FrameLineEdit:
    case PE_FrameWindow:
        drawPrimitive(PE_Frame, opt, p, widget);
        break;
    case PE_FrameGroupBox:
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt)) {
            int lwidth = frame->lineWidth,
               mlwidth = frame->midLineWidth;
            if (opt->state & (State_Sunken | State_Raised))
                qDrawShadeRect(p, frame->rect.x(), frame->rect.y(), frame->rect.width(),
                               frame->rect.height(), frame->palette, frame->state & State_Sunken,
                               lwidth, mlwidth);
            else
                qDrawPlainRect(p, frame->rect.x(), frame->rect.y(), frame->rect.width(),
                               frame->rect.height(), frame->palette.foreground().color(), lwidth);
        }
        break;
    case PE_FrameDockWindow:
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt)) {
            int lw = frame->lineWidth;
            if (lw <= 0)
                lw = pixelMetric(PM_DockWindowFrameWidth);

            qDrawShadePanel(p, frame->rect, frame->palette, false, lw);
        }
        break;

    case PE_IndicatorToolBarHandle:
        p->save();
        p->translate(opt->rect.x(), opt->rect.y());
        if (opt->state & State_Horizontal) {
            int x = opt->rect.width() / 3;
            if (QApplication::layoutDirection() == Qt::RightToLeft)
                x -= 2;
            if (opt->rect.height() > 4) {
                qDrawShadePanel(p, x, 2, 3, opt->rect.height() - 4,
                                opt->palette, false, 1, 0);
                qDrawShadePanel(p, x+3, 2, 3, opt->rect.height() - 4,
                                opt->palette, false, 1, 0);
            }
        } else {
            if (opt->rect.width() > 4) {
                int y = opt->rect.height() / 3;
                qDrawShadePanel(p, 2, y, opt->rect.width() - 4, 3,
                                opt->palette, false, 1, 0);
                qDrawShadePanel(p, 2, y+3, opt->rect.width() - 4, 3,
                                opt->palette, false, 1, 0);
            }
        }
        p->restore();
        break;
    case PE_IndicatorToolBarSeparator:
        {
            QPoint p1, p2;
            if (opt->state & State_Horizontal) {
                p1 = QPoint(opt->rect.width()/2, 0);
                p2 = QPoint(p1.x(), opt->rect.height());
            } else {
                p1 = QPoint(0, opt->rect.height()/2);
                p2 = QPoint(opt->rect.width(), p1.y());
            }
            qDrawShadeLine(p, p1, p2, opt->palette, 1, 1, 0);
            break;
        }
    case PE_IndicatorSpinPlus:
    case PE_IndicatorSpinMinus: {
        QRect r = opt->rect;
        int fw = pixelMetric(PM_DefaultFrameWidth, opt, widget);
        QRect br = r.adjusted(fw, fw, -fw, -fw);

        int offset = (opt->state & State_Sunken) ? 1 : 0;
        int step = (br.width() + 4) / 5;
        p->fillRect(br.x() + offset, br.y() + offset +br.height() / 2 - step / 2,
                    br.width(), step,
                    opt->palette.buttonText());
        if (pe == PE_IndicatorSpinPlus)
            p->fillRect(br.x() + br.width() / 2 - step / 2 + offset, br.y() + offset,
                        step, br.height(),
                        opt->palette.buttonText());

        break; }
    case PE_IndicatorSpinUp:
    case PE_IndicatorSpinDown: {
        QRect r = opt->rect;
        int fw = pixelMetric(PM_DefaultFrameWidth, opt, widget);
        // QRect br = r.adjusted(fw, fw, -fw, -fw);
        int x = r.x(), y = r.y(), w = r.width(), h = r.height();
        int sw = w-4;
        if (sw < 3)
            break;
        else if (!(sw & 1))
            sw--;
        sw -= (sw / 7) * 2;        // Empty border
        int sh = sw/2 + 2;      // Must have empty row at foot of arrow

        int sx = x + w / 2 - sw / 2;
        int sy = y + h / 2 - sh / 2;

        if (pe == PE_IndicatorSpinUp && fw)
            --sy;

        QPolygon a;
        if (pe == PE_IndicatorSpinDown)
            a.setPoints(3,  0, 1,  sw-1, 1,  sh-2, sh-1);
        else
            a.setPoints(3,  0, sh-1,  sw-1, sh-1,  sh-2, 1);
        int bsx = 0;
        int bsy = 0;
        if (opt->state & State_Sunken) {
            bsx = pixelMetric(PM_ButtonShiftHorizontal);
            bsy = pixelMetric(PM_ButtonShiftVertical);
        }
        p->save();
        p->translate(sx + bsx, sy + bsy);
        p->setPen(opt->palette.buttonText().color());
        p->setBrush(opt->palette.buttonText());
        p->drawPolygon(a);
        p->restore();
        break; }
    default:
        break;
    }
}

#include <qdebug.h>
/*!
    Draws the control \a ce, with style options \a opt, on painter \a
    p. The \a widget is optional and may contain a widget that is
    useful for drawing the control.
*/
void QCommonStyle::drawControl(ControlElement element, const QStyleOption *opt,
                               QPainter *p, const QWidget *widget) const
{
    switch (element) {
    case CE_PushButton:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            drawControl(CE_PushButtonBevel, btn, p, widget);
            QStyleOptionButton subopt = *btn;
            subopt.rect = QStyle::visualRect(btn->direction, btn->rect,
                                             subRect(SR_PushButtonContents, btn, widget));
            drawControl(CE_PushButtonLabel, &subopt, p, widget);
            if (btn->state & State_HasFocus) {
                QStyleOptionFocusRect fropt;
                fropt.state = btn->state;
                fropt.palette = btn->palette;
                fropt.rect = visualRect(opt->direction, opt->rect,
                                        subRect(SR_PushButtonFocusRect, btn, widget));
                drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
            }
        }
        break;
    case CE_PushButtonBevel:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            QRect br = btn->rect;
            int dbi = pixelMetric(PM_ButtonDefaultIndicator, btn, widget);
            if (btn->features & QStyleOptionButton::DefaultButton)
                drawPrimitive(PE_FrameDefaultButton, opt, p, widget);
            if (btn->features & QStyleOptionButton::AutoDefaultButton)
                br.setCoords(br.left() + dbi, br.top() + dbi, br.right() - dbi, br.bottom() - dbi);
            if (!(btn->features & QStyleOptionButton::Flat)
                || btn->state & (State_Down | State_On)) {
                QStyleOptionButton tmpBtn = *btn;
                tmpBtn.rect = br;
                drawPrimitive(PE_PanelButtonCommand, &tmpBtn, p, widget);
            }
            if (btn->features & QStyleOptionButton::HasMenu) {
                int mbi = pixelMetric(PM_MenuButtonIndicator, btn, widget);
                QRect ir = btn->rect;
                QStyleOptionButton newBtn = *btn;
                newBtn.rect = QRect(ir.right() - mbi, ir.height() - 20, mbi, ir.height() - 4);
                drawPrimitive(PE_IndicatorArrowDown, &newBtn, p, widget);
            }
        }
        break;
    case CE_PushButtonLabel:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            QRect ir = btn->rect;
            uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
            if (btn->state & (State_On | State_Down))
                ir.translate(pixelMetric(PM_ButtonShiftHorizontal, opt, widget),
                             pixelMetric(PM_ButtonShiftVertical, opt, widget));
            if (!btn->icon.isNull()) {
                QIcon::Mode mode = btn->state & State_Enabled ? QIcon::Normal
                                                              : QIcon::Disabled;
                if (mode == QIcon::Normal && btn->state & State_HasFocus)
                    mode = QIcon::Active;
                QIcon::State state = QIcon::Off;
                if (btn->state & State_On)
                    state = QIcon::On;
                QPixmap pixmap = btn->icon.pixmap(pixelMetric(PM_SmallIconSize), mode, state);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                //Center the icon if there is no text
                if (btn->text.isEmpty())
                    p->drawPixmap(ir.x() + ir.width() / 2 - pixw / 2,
                                  ir.y() + ir.height() / 2 - pixh / 2, pixmap);
                else
                    p->drawPixmap(ir.x() + 2, ir.y() + ir.height() / 2 - pixh / 2, pixmap);
                ir.translate(pixw + 4, 0);
                ir.setWidth(ir.width() - (pixw + 4));
                // left-align text if there is
                if (!btn->text.isEmpty())
                    tf |= Qt::AlignLeft;
            } else {
                tf |= Qt::AlignHCenter;
            }
            drawItemText(p, ir, tf, btn->palette, (btn->state & State_Enabled),
                         btn->text, &(btn->palette.buttonText().color()));
        }
        break;
    case CE_RadioButton:
    case CE_CheckBox:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            bool isRadio = (element == CE_RadioButton);
            QStyleOptionButton subopt = *btn;
            subopt.rect = QStyle::visualRect(btn->direction, btn->rect,
                                             subRect(isRadio ? QStyle::SR_RadioButtonIndicator
                                                             : SR_CheckBoxIndicator, btn, widget));
            drawPrimitive(isRadio ? PE_IndicatorRadioButton : PE_IndicatorCheckBox,
                          &subopt, p, widget);
            subopt.rect = QStyle::visualRect(btn->direction, btn->rect,
                                             subRect(isRadio ? QStyle::SR_RadioButtonContents
                                                             : SR_CheckBoxContents, btn, widget));
            drawControl(isRadio ? CE_RadioButtonLabel : CE_CheckBoxLabel, &subopt, p, widget);
            if (btn->state & State_HasFocus) {
                QStyleOptionFocusRect fropt;
                fropt.state = btn->state;
                fropt.palette = btn->palette;
                fropt.rect = visualRect(btn->direction, btn->rect,
                                        subRect(isRadio ? QStyle::SR_RadioButtonFocusRect
                                                        : SR_CheckBoxFocusRect, btn, widget));
                drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
            }
        }
        break;
    case CE_RadioButtonLabel:
    case CE_CheckBoxLabel:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            uint alignment = Qt::AlignLeft | Qt::AlignVCenter;
            if (!styleHint(SH_UnderlineShortcut, btn, widget))
                alignment |= Qt::TextHideMnemonic;
            QPixmap pix;
            if (!btn->icon.isNull()) {
                pix = btn->icon.pixmap(pixelMetric(PM_SmallIconSize), btn->state & State_Enabled ? QIcon::Normal : QIcon::Disabled);
                drawItemPixmap(p, btn->rect, alignment, btn->palette, pix);
            } else {
                drawItemText(p, btn->rect, alignment | Qt::TextShowMnemonic,
                             btn->palette, btn->state & State_Enabled, btn->text);
            }
        }
        break;
    case CE_MenuTearoff:
        if (opt->state & State_Selected)
            p->fillRect(opt->rect, opt->palette.brush(QPalette::Highlight));
        else
            p->fillRect(opt->rect, opt->palette.brush(QPalette::Button));
        p->setPen(QPen(opt->palette.dark().color(), 1, Qt::DashLine));
        p->drawLine(opt->rect.x() + 2, opt->rect.y() + opt->rect.height() / 2 - 1,
                    opt->rect.x() + opt->rect.width() - 4,
                    opt->rect.y() + opt->rect.height() / 2 - 1);
        p->setPen(QPen(opt->palette.light().color(), 1, Qt::DashLine));
        p->drawLine(opt->rect.x() + 2, opt->rect.y() + opt->rect.height() / 2,
                    opt->rect.x() + opt->rect.width() - 4, opt->rect.y() + opt->rect.height() / 2);
        break;
    case CE_MenuBarItem:
        if (const QStyleOptionMenuItem *mbi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            uint alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip
                            | Qt::TextSingleLine;
            if (!styleHint(SH_UnderlineShortcut, mbi, widget))
                alignment |= Qt::TextHideMnemonic;
            QPixmap pix = mbi->icon.pixmap(pixelMetric(PM_SmallIconSize),  (mbi->state & State_Enabled) ? QIcon::Normal : QIcon::Disabled);
            if (!pix.isNull())
                drawItemPixmap(p,mbi->rect, alignment, mbi->palette, pix, &mbi->palette.buttonText().color());
            else
                drawItemText(p, mbi->rect, alignment, mbi->palette, mbi->state & State_Enabled,
                             mbi->text, &mbi->palette.buttonText().color());
        }
        break;
    case CE_MenuBarEmptyArea:
        if (widget && !widget->testAttribute(Qt::WA_NoSystemBackground))
            p->eraseRect(opt->rect);
        break;
    case CE_ProgressBar:
        if (const QStyleOptionProgressBar *pb = qt_cast<const QStyleOptionProgressBar *>(opt)) {
            QStyleOptionProgressBar subopt = *pb;
            subopt.rect = QStyle::visualRect(pb->direction, pb->rect,
                                             subRect(SR_ProgressBarGroove, pb, widget));
            drawControl(CE_ProgressBarGroove, &subopt, p, widget);
            subopt.rect = QStyle::visualRect(pb->direction, pb->rect,
                                             subRect(SR_ProgressBarContents, pb, widget));
            drawControl(CE_ProgressBarContents, &subopt, p, widget);
            if (pb->textVisible) {
                subopt.rect = QStyle::visualRect(pb->direction, pb->rect,
                                                 subRect(SR_ProgressBarLabel, pb, widget));
                drawControl(CE_ProgressBarLabel, &subopt, p, widget);
            }
        }
        break;
    case CE_ProgressBarGroove:
        qDrawShadePanel(p, opt->rect, opt->palette, true, 1,
                        &opt->palette.brush(QPalette::Background));
        break;
    case CE_ProgressBarLabel:
        if (const QStyleOptionProgressBar *pb = qt_cast<const QStyleOptionProgressBar *>(opt)) {
            QColor penColor = pb->palette.highlightedText().color();
            QColor *pColor = 0;
            if ((pb->textAlignment & Qt::AlignCenter) && pb->textVisible
                && pb->progress * 2 >= pb->maximum)
                pColor = &penColor;
            drawItemText(p, pb->rect, Qt::AlignCenter | Qt::TextSingleLine, pb->palette,
                         pb->state & State_Enabled, pb->text, pColor);
        }
        break;
    case CE_ProgressBarContents:
        if (const QStyleOptionProgressBar *pb = qt_cast<const QStyleOptionProgressBar *>(opt)) {
            QPalette pal2 = pb->palette;
            // Correct the highlight color if it is the same as the background
            if (pal2.highlight() == pal2.background())
                pal2.setColor(QPalette::Highlight, pb->palette.color(QPalette::Active,
                                                                     QPalette::Highlight));
            bool reverse = pb->direction == Qt::RightToLeft;
            int fw = 2;
            int w = pb->rect.width() - 2 * fw;
            if (pb->minimum == 0 && pb->maximum == 0) {
                // draw busy indicator
                int x = pb->progress % (w * 2);
                if (x > w)
                    x = 2 * w - x;
                x = reverse ? pb->rect.right() - x : x + pb->rect.x();
                p->setPen(QPen(pal2.highlight().color(), 4));
                p->drawLine(x, pb->rect.y() + 1, x, pb->rect.height() - fw);
            } else {
                const int unit_width = pixelMetric(PM_ProgressBarChunkWidth, pb, widget);
                int u;
                if (unit_width > 1)
                    u = (pb->rect.width() + unit_width / 3) / unit_width;
                else
                    u = w / unit_width;
                int p_v = pb->progress;
                int t_s = pb->maximum ? pb->maximum : 1;

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
                pbBits.state = State_None;
                for (int i = 0; i < nu; ++i) {
                    pbBits.rect.setRect(x0 + x, myY, unit_width, myHeight);
                    drawPrimitive(PE_IndicatorProgressChunk, &pbBits, p, widget);
                    x += reverse ? -unit_width : unit_width;
                }

                // Draw the last partial chunk to fill up the
                // progressbar entirely
                if (nu < tnu) {
                    int pixels_left = w - (nu * unit_width);
                    int offset = reverse ? x0 + x + unit_width-pixels_left : x0 + x;
                    pbBits.rect.setRect(offset, myY, pixels_left, myHeight);
                    drawPrimitive(PE_IndicatorProgressChunk, &pbBits, p, widget);
                }
            }
        }
        break;
    case CE_HeaderLabel:
        if (const QStyleOptionHeader *header = qt_cast<const QStyleOptionHeader *>(opt)) {
            QRect rect = header->rect;
            if (!header->icon.isNull()) {
                QPixmap pixmap
                    = header->icon.pixmap(pixelMetric(PM_SmallIconSize), (header->state & State_Enabled) ? QIcon::Normal : QIcon::Disabled);
                int pixw = pixmap.width();
                // "pixh - 1" because of tricky integer division
                drawItemPixmap(p, rect, header->iconAlignment, header->palette, pixmap);
                if (header->direction == Qt::LeftToRight)
                    rect.setLeft(rect.left() + pixw + 2);
                else
                    rect.setRight(rect.right() - pixw - 2);
            }
            drawItemText(p, rect, header->textAlignment, header->palette,
                         (header->state & State_Enabled), header->text,
                         &(header->palette.buttonText().color()));
        }
        break;

    case CE_ToolButtonLabel:
        if (const QStyleOptionToolButton *toolbutton
                = qt_cast<const QStyleOptionToolButton *>(opt)) {
            QRect rect = toolbutton->rect;
            int shiftX = 0;
            int shiftY = 0;
            if (toolbutton->state & (State_Down | State_On)) {
                shiftX = pixelMetric(PM_ButtonShiftHorizontal, toolbutton, widget);
                shiftY = pixelMetric(PM_ButtonShiftVertical, toolbutton, widget);
            }
            if (toolbutton->features & QStyleOptionToolButton::Arrow) {
                PrimitiveElement pe;
                switch (toolbutton->arrowType) {
                case Qt::LeftArrow:
                    pe = PE_IndicatorArrowLeft;
                    break;
                case Qt::RightArrow:
                    pe = PE_IndicatorArrowRight;
                    break;
                case Qt::UpArrow:
                    pe = PE_IndicatorArrowUp;
                    break;
                case Qt::DownArrow:
                    pe = PE_IndicatorArrowDown;
                    break;
                default:
                    return;
                }
                rect.translate(shiftX, shiftY);
                QStyleOption arrowOpt(0);
                arrowOpt.rect = rect;
                arrowOpt.palette = toolbutton->palette;
                arrowOpt.state = toolbutton->state;
                drawPrimitive(pe, &arrowOpt, p, widget);
            } else {
                QColor btext = toolbutton->palette.foreground().color();

                // remove the space used for the menu button when
                // positioning the text label and/or icon
                if (toolbutton->features & QStyleOptionToolButton::Menu)
                    rect.addCoords(0, 0, -pixelMetric(QStyle::PM_MenuButtonIndicator,
                                                      toolbutton, widget), 0);

                if (toolbutton->icon.isNull() && !toolbutton->text.isEmpty()
                    || toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly) {
                    int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
                    if (!styleHint(SH_UnderlineShortcut, opt, widget))
                        alignment |= Qt::TextHideMnemonic;
                    rect.translate(shiftX, shiftY);
                    drawItemText(p, rect, alignment, toolbutton->palette,
                                 opt->state & State_Enabled, toolbutton->text, &btext);
                } else {
                    QPixmap pm;
                    QIcon::State state = toolbutton->state & State_On ? QIcon::On : QIcon::Off;
                    QIcon::Mode mode;
                    if (!(toolbutton->state & State_Enabled))
                        mode = QIcon::Disabled;
                    else if ((opt->state & State_MouseOver) && (opt->state & State_AutoRaise))
                        mode = QIcon::Active;
                    else
                        mode = QIcon::Normal;
                    pm = toolbutton->icon.pixmap(toolbutton->rect.size(), mode, state);

                    if (toolbutton->toolButtonStyle != Qt::ToolButtonIconOnly) {
                        p->setFont(toolbutton->font);
                        QRect pr = rect,
                        tr = rect;
                        int alignment = Qt::TextShowMnemonic;
                        if (!styleHint(SH_UnderlineShortcut, opt, widget))
                            alignment |= Qt::TextHideMnemonic;

                        if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                            int fh = p->fontMetrics().height();
                            pr.addCoords(0, 3, 0, -fh - 3);
                            tr.addCoords(0, pr.bottom(), 0, -3);
                            pr.translate(shiftX, shiftY);
                            drawItemPixmap(p, pr, Qt::AlignCenter, toolbutton->palette, pm);
                            alignment |= Qt::AlignCenter;
                        } else {
                            pr.setWidth(pm.width() + 8);
                            tr.addCoords(pr.right(), 0, 0, 0);
                            pr.translate(shiftX, shiftY);
                            drawItemPixmap(p, pr, Qt::AlignCenter, toolbutton->palette, pm);
                            alignment |= Qt::AlignLeft | Qt::AlignVCenter;
                        }
                        tr.translate(shiftX, shiftY);
                        drawItemText(p, tr, alignment, toolbutton->palette,
                                     toolbutton->state & State_Enabled, toolbutton->text, &btext);
                    } else {
                        rect.translate(shiftX, shiftY);
                        drawItemPixmap(p, rect, Qt::AlignCenter, toolbutton->palette, pm);
                    }
                }
            }
        }
        break;
    case CE_ToolBoxTab:
        if (const QStyleOptionToolBox *tb = qt_cast<const QStyleOptionToolBox *>(opt)) {
            int d = 20 + tb->rect.height() - 3;
            QPolygon a(7);
            a.setPoint(0, -1, tb->rect.height() + 1);
            a.setPoint(1, -1, 1);
            a.setPoint(2, tb->rect.width() - d, 1);
            a.setPoint(3, tb->rect.width() - 20, tb->rect.height() - 2);
            a.setPoint(4, tb->rect.width() - 1, tb->rect.height() - 2);
            a.setPoint(5, tb->rect.width() - 1, tb->rect.height() + 1);
            a.setPoint(6, -1, tb->rect.height() + 1);

            p->setPen(tb->palette.mid().color().dark(150));
            p->drawPolygon(a);
            p->setPen(tb->palette.light().color());
            p->drawLine(0, 2, tb->rect.width() - d, 2);
            p->drawLine(tb->rect.width() - d - 1, 2, tb->rect.width() - 21, tb->rect.height() - 1);
            p->drawLine(tb->rect.width() - 20, tb->rect.height() - 1,
                        tb->rect.width(), tb->rect.height() - 1);
            p->setBrush(Qt::NoBrush);
        }
        break;
    case CE_TabBarTab:
        if (const QStyleOptionTab *tab = qt_cast<const QStyleOptionTab *>(opt)) {
            drawControl(CE_TabBarTabShape, tab, p, widget);
            drawControl(CE_TabBarTabLabel, tab, p, widget);
        }
        break;
    case CE_TabBarTabShape:
        if (const QStyleOptionTab *tab = qt_cast<const QStyleOptionTab *>(opt)) {
            QBrush oldBrush = p->brush();
            QPen oldPen = p->pen();
            if (tab->state & State_Selected)
                p->setBrush(tab->palette.base());
            else
                p->setBrush(tab->palette.background());
            p->setPen(tab->palette.foreground().color());
            int y;
            int x;
            QPolygon a(10);
            switch (tab->shape) {
            case QTabBar::TriangularNorth:
            case QTabBar::TriangularSouth: {
                a.setPoint(0, 0, -1);
                a.setPoint(1, 0, 0);
                y = tab->rect.height() - 2;
                x = y / 3;
                a.setPoint(2, x++, y - 1);
                ++x;
                a.setPoint(3, x++, y++);
                a.setPoint(4, x, y);

                int i;
                int right = tab->rect.width() - 1;
                for (i = 0; i < 5; ++i)
                    a.setPoint(9 - i, right - a.point(i).x(), a.point(i).y());
                if (tab->shape == QTabBar::TriangularNorth)
                    for (i = 0; i < 10; ++i)
                        a.setPoint(i, a.point(i).x(), tab->rect.height() - 1 - a.point(i).y());

                a.translate(tab->rect.left(), tab->rect.top());
                p->drawPolygon(a);
                break; }
            case QTabBar::TriangularEast:
            case QTabBar::TriangularWest: {
                a.setPoint(0, -1, 0);
                a.setPoint(1, 0, 0);
                x = tab->rect.width() - 2;
                y = x / 3;
                a.setPoint(2, x - 1, y++);
                ++y;
                a.setPoint(3, x++, y++);
                a.setPoint(4, x, y);
                int i;
                int bottom = tab->rect.height() - 1;
                for (i = 0; i < 5; ++i)
                    a.setPoint(9 - i, a.point(i).x(), bottom - a.point(i).y());
                if (tab->shape == QTabBar::TriangularWest)
                    for (i = 0; i < 10; ++i)
                        a.setPoint(i, tab->rect.width() - 1 - a.point(i).x(), a.point(i).y());
                a.translate(tab->rect.left(), tab->rect.top());
                p->drawPolygon(a);
                break; }
            default:
                break;
            }
            p->setPen(oldPen);
            p->setBrush(oldBrush);
        }
        break;
    case CE_TabBarTabLabel:
        if (const QStyleOptionTab *tab = qt_cast<const QStyleOptionTab *>(opt)) {
            QRect tr = tab->rect;
            bool verticalTabs = tab->shape == QTabBar::RoundedEast
                                || tab->shape == QTabBar::RoundedWest
                                || tab->shape == QTabBar::TriangularEast
                                || tab->shape == QTabBar::TriangularWest;
            bool selected = tab->state & State_Selected;
            if (verticalTabs) {
                p->save();
                int newX, newY, newRot;
                if (tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::TriangularEast) {
                    newX = tr.width();
                    newY = tr.y();
                    newRot = 90;
                } else {
                    newX = 0;
                    newY = tr.y() + tr.height();
                    newRot = -90;
                }
                tr.setRect(0, 0, tr.height(), tr.width());
                QMatrix m;
                m.translate(newX, newY);
                m.rotate(newRot);
                p->setMatrix(m, true);
            }
            if (selected) {
                tr.setBottom(tr.bottom() - pixelMetric(QStyle::PM_TabBarTabShiftVertical, tab,
                                                       widget));
                tr.setRight(tr.right() - pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, tab,
                                                     widget));
            }

            int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
            if (!styleHint(SH_UnderlineShortcut, opt, widget))
                alignment |= Qt::TextHideMnemonic;
            if (!tab->icon.isNull()) {
                QPixmap tabIcon = tab->icon.pixmap(pixelMetric(PM_SmallIconSize), (tab->state & State_Enabled) ? QIcon::Normal : QIcon::Disabled);
                p->drawPixmap(tr.left() + 6, tr.center().y() - tabIcon.height() / 2, tabIcon);
                tr.setLeft(tr.left() + tabIcon.width() + 4);
            }
            drawItemText(p, tr, alignment, tab->palette, tab->state & State_Enabled, tab->text);

            if (verticalTabs)
                p->restore();

            if (tab->state & State_HasFocus && !tab->text.isEmpty()) {
                QStyleOptionFocusRect fropt;
                const int OFFSET = 3;

                int x1, x2;
                x1 = tab->rect.left();
                x2 = tab->rect.right() - 1;

                if (selected) {
                    x1 -= 2;
                    x2 += 2;
                }
                if (tab->position == QStyleOptionTab::Beginning)
                    x1 += 2;
                if (tab->position == QStyleOptionTab::End)
                    x2 -= 2;

                fropt.rect.setRect(x1 + OFFSET, tab->rect.y() + OFFSET,
                                   x2 - x1 - 2*OFFSET + 2, tab->rect.height() - 2*OFFSET);
                fropt.palette = tab->palette;
                fropt.state = State_None;
                drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
            }
        }
        break;
    case CE_SpinBoxSlider:
        if (const QStyleOptionSpinBox *sb = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            QRect re = sb->rect;
            re.setWidth((int)((double)re.width() * sb->percentage));
            p->fillRect(re, sb->palette.brush(QPalette::Highlight));
            break;
        }
    case CE_SizeGrip: {
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

        if (opt->direction == Qt::RightToLeft) {
            sx = x + sw;
            for (int i = 0; i < 4; ++i) {
                p->setPen(QPen(opt->palette.light().color(), 1));
                p->drawLine(x, sy - 1 , sx + 1,  sw);
                p->setPen(QPen(opt->palette.dark().color(), 1));
                p->drawLine(x, sy, sx, sw);
                p->setPen(QPen(opt->palette.dark().color(), 1));
                p->drawLine(x, sy + 1, sx - 1,  sw);
                sx -= s;
                sy += s;
            }
        } else {
            for (int i = 0; i < 4; ++i) {
                p->setPen(QPen(opt->palette.light().color(), 1));
                p->drawLine(sx - 1, sw, sw,  sy - 1);
                p->setPen(QPen(opt->palette.dark().color(), 1));
                p->drawLine(sx, sw, sw,  sy);
                p->setPen(QPen(opt->palette.dark().color(), 1));
                p->drawLine(sx + 1, sw, sw,  sy + 1);
                sx += s;
                sy += s;
            }
        }
        p->restore();
        break; }
    case CE_RubberBand: {
        p->save();
        QRect r = opt->rect.adjusted(0,0,-1,-1);
        p->setBrush(Qt::Dense4Pattern);
        p->setBackground(QBrush(opt->palette.base()));
        p->setBackgroundMode(Qt::OpaqueMode);
        p->setPen(opt->palette.color(QPalette::Active, QPalette::Foreground));
        p->drawRect(r);
        if (opt->state & State_Rectangle) {
            r.addCoords(3,3, -3,-3);
            p->drawRect(r);
        }
        p->restore();
        break; }
    case CE_DockWindowTitle:
        if (const QStyleOptionDockWindow *dwOpt = qt_cast<const QStyleOptionDockWindow *>(opt)) {
            QRect r = dwOpt->rect.adjusted(0, 0, -1, -1);
            if (dwOpt->moveable) {
                p->setPen(dwOpt->palette.color(QPalette::Dark));
                p->drawRect(r);
            }
            if (!dwOpt->title.isEmpty()) {
                const int indent = p->fontMetrics().descent();
                drawItemText(p, r.adjusted(indent + 1, 1, -indent - 1, -1),
                             Qt::AlignLeft | Qt::AlignVCenter, dwOpt->palette,
                             dwOpt->state & State_Enabled, dwOpt->title);
            }
        }
        break;
    case CE_Header:
        if (const QStyleOptionHeader *header = qt_cast<const QStyleOptionHeader *>(opt)) {
            drawControl(CE_HeaderSection, header, p, widget);
            QStyleOptionHeader subopt = *header;
            subopt.rect = visualRect(header->direction, header->rect, subRect(SR_HeaderLabel, header, widget));
            drawControl(CE_HeaderLabel, &subopt, p, widget);
            if (header->state & (State_Up | State_Down)) {
                subopt.rect = visualRect(header->direction, header->rect,
                                         subRect(SR_HeaderArrow, opt, widget));
                drawPrimitive(PE_IndicatorHeaderArrow, &subopt, p, widget);
            }
        }
        break;
    case CE_FocusFrame:
        p->fillRect(opt->rect, opt->palette.foreground());
        break;
    case CE_HeaderSection:
        qDrawShadePanel(p, opt->rect, opt->palette,
                        opt->state & State_Sunken, 1,
                        &opt->palette.brush(QPalette::Button));
        break;
    default:
        break;
    }
}


/*!
    Returns the rectangle occupied by sub-rectangle \a sr, with style
    options \a opt. The \a widget parameter is optional and may
    contain a widget that is useful for drawing the sub-rectangle.
*/
QRect QCommonStyle::subRect(SubRect sr, const QStyleOption *opt, const QWidget *widget) const
{
    QRect r;
    switch (sr) {
    case SR_PushButtonContents:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            int dx1, dx2;
            dx1 = pixelMetric(PM_DefaultFrameWidth, btn, widget);
            if (btn->features & QStyleOptionButton::AutoDefaultButton)
                dx1 += pixelMetric(PM_ButtonDefaultIndicator, btn, widget);
            dx2 = dx1 * 2;
            r.setRect(opt->rect.x() + dx1, opt->rect.y() + dx1, opt->rect.width() - dx2,
                      opt->rect.height() - dx2);
        }
        break;
    case SR_PushButtonFocusRect:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            int dbw1 = 0, dbw2 = 0;
            if (btn->features & QStyleOptionButton::AutoDefaultButton){
                dbw1 = pixelMetric(PM_ButtonDefaultIndicator, btn, widget);
                dbw2 = dbw1 * 2;
            }

            int dfw1 = pixelMetric(PM_DefaultFrameWidth, btn, widget) + 1,
                dfw2 = dfw1 * 2;

            r.setRect(btn->rect.x() + dfw1 + dbw1, btn->rect.y() + dfw1 + dbw1,
                      btn->rect.width() - dfw2 - dbw2, btn->rect.height()- dfw2 - dbw2);
        }
        break;

    case SR_CheckBoxIndicator:
        {
            int h = pixelMetric(PM_IndicatorHeight, opt, widget);
            r.setRect(0, (opt->rect.height() - h) / 2,
                      pixelMetric(PM_IndicatorWidth, opt, widget), h);
        }
        break;

    case SR_CheckBoxContents:
        {
            QRect ir = subRect(SR_CheckBoxIndicator, opt, widget);
            r.setRect(ir.right() + 6, opt->rect.y(), opt->rect.width() - ir.width() - 6,
                      opt->rect.height());
        }
        break;

    case SR_CheckBoxFocusRect:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            if (btn->text.isEmpty()) {
                r = subRect(SR_CheckBoxIndicator, opt, widget);
                r.addCoords(1, 1, -1, -1);
                break;
            }
            QRect cr = subRect(SR_CheckBoxContents, opt, widget);

            if (!btn->icon.isNull()) {
                r = itemPixmapRect(cr,  Qt::AlignAbsolute | Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic,
                                   btn->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal));
            } else {
                r = itemTextRect(opt->fontMetrics, cr, Qt::AlignAbsolute | Qt::AlignLeft | Qt::AlignVCenter
                                 | Qt::TextShowMnemonic, btn->state & State_Enabled, btn->text);
            }
            r.addCoords(-3, -2, 3, 2);
            r = r.intersect(btn->rect);
        }
        break;

    case SR_RadioButtonIndicator:
        {
            int h = pixelMetric(PM_ExclusiveIndicatorHeight, opt, widget);
            r.setRect(0, (opt->rect.height() - h) / 2,
                    pixelMetric(PM_ExclusiveIndicatorWidth, opt, widget), h);
        }
        break;

    case SR_RadioButtonContents:
        {
            QRect ir = subRect(SR_RadioButtonIndicator, opt, widget);
            r.setRect(ir.right() + 6, opt->rect.y(),
                      opt->rect.width() - ir.width() - 6, opt->rect.height());
            break;
        }

    case SR_RadioButtonFocusRect:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            if (!btn->icon.isNull() && btn->text.isEmpty()) {
                r = subRect(SR_RadioButtonIndicator, opt, widget);
                r.addCoords(1, 1, -1, -1);
                break;
            }
            QRect cr = subRect(SR_RadioButtonContents, opt, widget);

            if(!btn->icon.isNull()) {
                r = itemPixmapRect(cr,  Qt::AlignAbsolute | Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic,
                                   btn->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal));
            } else {
                r = itemTextRect(opt->fontMetrics, cr,  Qt::AlignAbsolute | Qt::AlignLeft | Qt::AlignVCenter
                                 | Qt::TextShowMnemonic, btn->state & State_Enabled, btn->text);
            }
            r.addCoords(-3, -2, 3, 2);
            r = r.intersect(btn->rect);
        }
        break;
    case SR_SliderFocusRect:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            int tickOffset = pixelMetric(PM_SliderTickmarkOffset, slider, widget);
            int thickness  = pixelMetric(PM_SliderControlThickness, slider, widget);
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
            int textw = 0;
            if (pb->textVisible)
                textw = qMax(pb->fontMetrics.width(pb->text), pb->fontMetrics.width("100%")) + 6;

            if ((pb->textAlignment & Qt::AlignCenter) == 0) {
                if (sr != SR_ProgressBarLabel)
                    r.setCoords(pb->rect.left(), pb->rect.top(),
                                pb->rect.right() - textw, pb->rect.bottom());
                else
                    r.setCoords(pb->rect.right() - textw, pb->rect.top(),
                                pb->rect.right(), pb->rect.bottom());
            } else {
                r = pb->rect;
            }
        }
        break;
    case SR_Q3DockWindowHandleRect:
        if (const QStyleOptionQ3DockWindow *dw = qt_cast<const QStyleOptionQ3DockWindow *>(opt)) {
            if (!dw->docked || !dw->closeEnabled)
                r.setRect(0, 0, dw->rect.width(), dw->rect.height());
            else {
                if (dw->state & State_Horizontal)
                    r.setRect(0, 15, dw->rect.width(), dw->rect.height() - 15);
                else
                    r.setRect(0, 1, dw->rect.width() - 15, dw->rect.height() - 1);
            }
        }
        break;
    case SR_ComboBoxFocusRect:
        r.setRect(3, 3, opt->rect.width() - 6 - 16, opt->rect.height() - 6);
        break;
    case SR_ToolBoxTabContents:
        r = opt->rect;
        r.addCoords(0, 0, -30, 0);
        break;
    case SR_HeaderLabel: {
        int margin = pixelMetric(QStyle::PM_HeaderMargin, opt, widget);
        r.setRect(opt->rect.x() + margin, opt->rect.y() + margin,
                  opt->rect.width() - margin * 2, opt->rect.height() - margin * 2);

        break; }
    case SR_HeaderArrow: {
        int h = opt->rect.height();
        int w = opt->rect.width();
        int x = opt->rect.x();
        int y = opt->rect.y();
        int margin = pixelMetric(QStyle::PM_HeaderMargin, opt, widget);
        if (opt->state & State_Horizontal)
            r.setRect(x + w - margin * 2 - (h / 2), y + 5, h / 2, h - margin * 2);
        else
            r.setRect(x + 5, y, h / 2, h - margin * 2);
        break; }

    case SR_RadioButtonClickRect:
        return subRect(SR_RadioButtonFocusRect, opt, widget);
    case SR_CheckBoxClickRect:
        return subRect(SR_CheckBoxFocusRect, opt, widget);
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
            if (slider->subControls == SC_SliderTickmarks) {
                int tickOffset = pixelMetric(PM_SliderTickmarkOffset, slider, widget);
                int ticks = slider->tickPosition;
                int thickness = pixelMetric(PM_SliderControlThickness, slider, widget);
                int len = pixelMetric(PM_SliderLength, slider, widget);
                int available = pixelMetric(PM_SliderSpaceAvailable, slider, widget);
                int interval = slider->tickInterval;
                if (interval <= 0) {
                    interval = slider->singleStep;
                    if (QStyle::sliderPositionFromValue(slider->minimum, slider->maximum, interval,
                                                        available)
                        - QStyle::sliderPositionFromValue(slider->minimum, slider->maximum,
                                                          0, available) < 3)
                        interval = slider->pageStep;
                }
                if (!interval)
                    interval = 1;
                int fudge = len / 2;
                int pos;
                p->setPen(slider->palette.foreground().color());
                int v = slider->minimum;
                while (v <= slider->maximum + 1) {
                    pos = QStyle::sliderPositionFromValue(slider->minimum, slider->maximum + 1,
                                                          v, available) + fudge;
                    if (slider->orientation == Qt::Horizontal) {
                        if (ticks & QSlider::TicksAbove)
                            p->drawLine(pos, 0, pos, tickOffset - 2);
                        if (ticks & QSlider::TicksBelow)
                            p->drawLine(pos, tickOffset + thickness + 1, pos,
                                        tickOffset + thickness + 1 + available - 2);
                    } else {
                        if (ticks & QSlider::TicksAbove)
                            p->drawLine(0, pos, tickOffset - 2, pos);
                        if (ticks & QSlider::TicksBelow)
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
            State saveFlags = scrollbar->state;
            if (scrollbar->minimum == scrollbar->maximum)
                saveFlags |= State_Enabled;

            if (scrollbar->subControls & SC_ScrollBarSubLine) {
                newScrollbar.state = saveFlags;
                newScrollbar.rect = visualRect(opt->direction, opt->rect,
                                               subControlRect(cc, &newScrollbar,
                                                                      SC_ScrollBarSubLine, widget));
                if (newScrollbar.rect.isValid()) {
                    if (scrollbar->activeSubControls & SC_ScrollBarSubLine)
                        newScrollbar.state |= State_Down;
                    drawControl(CE_ScrollBarSubLine, &newScrollbar, p, widget);
                }
            }
            if (scrollbar->subControls & SC_ScrollBarAddLine) {
                newScrollbar.rect = scrollbar->rect;
                newScrollbar.state = saveFlags;
                newScrollbar.rect = visualRect(opt->direction, opt->rect,
                                               subControlRect(cc, &newScrollbar,
                                                                      SC_ScrollBarAddLine, widget));
                if (newScrollbar.rect.isValid()) {
                    if (scrollbar->activeSubControls & SC_ScrollBarAddLine)
                        newScrollbar.state |= State_Down;
                    drawControl(CE_ScrollBarAddLine, &newScrollbar, p, widget);
                }
            }
            if (scrollbar->subControls & SC_ScrollBarSubPage) {
                newScrollbar.rect = scrollbar->rect;
                newScrollbar.state = saveFlags;
                newScrollbar.rect = visualRect(opt->direction, opt->rect,
                                               subControlRect(cc, &newScrollbar,
                                                                      SC_ScrollBarSubPage, widget));
                if (newScrollbar.rect.isValid()) {
                    if (scrollbar->activeSubControls & SC_ScrollBarSubPage)
                        newScrollbar.state |= State_Down;
                    drawControl(CE_ScrollBarSubPage, &newScrollbar, p, widget);
                }
            }
            if (scrollbar->subControls & SC_ScrollBarAddPage) {
                newScrollbar.rect = scrollbar->rect;
                newScrollbar.state = saveFlags;
                newScrollbar.rect = visualRect(opt->direction, opt->rect,
                                               subControlRect(cc, &newScrollbar,
                                                                      SC_ScrollBarAddPage, widget));
                if (newScrollbar.rect.isValid()) {
                    if (scrollbar->activeSubControls & SC_ScrollBarAddPage)
                        newScrollbar.state |= State_Down;
                    drawControl(CE_ScrollBarAddPage, &newScrollbar, p, widget);
                }
            }
            if (scrollbar->subControls & SC_ScrollBarFirst) {
                newScrollbar.rect = scrollbar->rect;
                newScrollbar.state = saveFlags;
                newScrollbar.rect = visualRect(opt->direction, opt->rect,
                                               subControlRect(cc, &newScrollbar,
                                                                      SC_ScrollBarFirst, widget));
                if (newScrollbar.rect.isValid()) {
                    if (scrollbar->activeSubControls & SC_ScrollBarFirst)
                        newScrollbar.state |= State_Down;
                    drawControl(CE_ScrollBarFirst, &newScrollbar, p, widget);
                }
            }
            if (scrollbar->subControls & SC_ScrollBarLast) {
                newScrollbar.rect = scrollbar->rect;
                newScrollbar.state = saveFlags;
                newScrollbar.rect = visualRect(opt->direction, opt->rect,
                                               subControlRect(cc, &newScrollbar,
                                                                      SC_ScrollBarLast, widget));
                if (newScrollbar.rect.isValid()) {
                    if (scrollbar->activeSubControls & SC_ScrollBarLast)
                        newScrollbar.state |= State_Down;
                    drawControl(CE_ScrollBarLast, &newScrollbar, p, widget);
                }
            }
            if (scrollbar->subControls & SC_ScrollBarSlider) {
                newScrollbar.rect = scrollbar->rect;
                newScrollbar.state = saveFlags;
                newScrollbar.rect = visualRect(opt->direction, opt->rect,
                                               subControlRect(cc, &newScrollbar,
                                                                      SC_ScrollBarSlider, widget));
                if (newScrollbar.rect.isValid()) {
                    if (scrollbar->activeSubControls & SC_ScrollBarSlider)
                        newScrollbar.state |= State_Down;
                    drawControl(CE_ScrollBarSlider, &newScrollbar, p, widget);

                    if (scrollbar->state & State_HasFocus) {
                        QStyleOptionFocusRect fropt;
                        fropt.rect.setRect(newScrollbar.rect.x() + 2, newScrollbar.rect.y() + 2,
                                           newScrollbar.rect.width() - 5,
                                           newScrollbar.rect.height() - 5);
                        fropt.palette = newScrollbar.palette;
                        fropt.state = State_None;
                        drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
                    }
                }
            }
        }
        break;
    case CC_Q3ListView:
        if (const QStyleOptionQ3ListView *lv = qt_cast<const QStyleOptionQ3ListView *>(opt)) {
            if (lv->subControls & SC_Q3ListView)
                p->fillRect(lv->rect, lv->viewportPalette.brush(lv->viewportBGRole));
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            QStyleOptionSpinBox copy = *sb;
            PrimitiveElement pe;

            if (sb->subControls & SC_SpinBoxFrame) {
                QRect r = visualRect(opt->direction, opt->rect,
                                     subControlRect(CC_SpinBox, sb, SC_SpinBoxFrame, widget));
                qDrawWinPanel(p, r, sb->palette, true); //cstyle == Sunken);
            }

            if (sb->subControls & SC_SpinBoxUp) {
                copy.subControls = SC_SpinBoxUp;
                QPalette pal2 = sb->palette;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepUpEnabled)) {
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                    copy.state &= ~State_Enabled;
                }

                copy.palette = pal2;

                if (sb->activeSubControls == SC_SpinBoxUp) {
                    copy.state |= State_On;
                    copy.state |= State_Sunken;
                } else {
                    copy.state |= State_Raised;
                }
                pe = (sb->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_IndicatorSpinPlus
                                                                       : PE_IndicatorSpinUp);

                copy.rect = visualRect(opt->direction, opt->rect,
                                       subControlRect(CC_SpinBox, sb, SC_SpinBoxUp,
                                                              widget));
                drawPrimitive(PE_PanelButtonBevel, &copy, p, widget);
                copy.rect.addCoords(3, 0, -4, 0);
                drawPrimitive(pe, &copy, p, widget);
            }

            if (sb->subControls & SC_SpinBoxDown) {
                copy.subControls = SC_SpinBoxDown;
                copy.state = sb->state;
                QPalette pal2 = sb->palette;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepDownEnabled)) {
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                    copy.state &= ~State_Enabled;
                }
                copy.palette = pal2;

                if (sb->activeSubControls == SC_SpinBoxDown) {
                    copy.state |= State_On;
                    copy.state |= State_Sunken;
                } else {
                    copy.state |= State_Raised;
                }
                pe = (sb->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_IndicatorSpinMinus
                                                                       : PE_IndicatorSpinDown);

                copy.rect = visualRect(opt->direction, opt->rect,
                                       subControlRect(CC_SpinBox, sb, SC_SpinBoxDown,
                                                              widget));
                drawPrimitive(PE_PanelButtonBevel, &copy, p, widget);
                copy.rect.addCoords(3, 0, -4, 0);
                drawPrimitive(pe, &copy, p, widget);
            }

            if (sb->subControls & CE_SpinBoxSlider) {
                copy.state = sb->state;
                copy.subControls = SC_SpinBoxSlider;
                copy.rect = visualRect(opt->direction, opt->rect,
                                       subControlRect(CC_SpinBox, sb, SC_SpinBoxSlider,
                                       widget));
                drawControl(CE_SpinBoxSlider, &copy, p, widget);
            }

        }
        break;
    case CC_ToolButton:
        if (const QStyleOptionToolButton *toolbutton
                = qt_cast<const QStyleOptionToolButton *>(opt)) {
            QRect button, menuarea;
            button = visualRect(opt->direction, opt->rect,
                                subControlRect(cc, toolbutton, SC_ToolButton, widget));
            menuarea = visualRect(opt->direction, opt->rect,
                                  subControlRect(cc, toolbutton, SC_ToolButtonMenu,
                                                         widget));

            State bflags = toolbutton->state;

            if (bflags & State_AutoRaise) {
                if (!(bflags & State_MouseOver)) {
                    bflags &= ~State_Raised;
                }
            }
            State mflags = bflags;

            if (toolbutton->activeSubControls & SC_ToolButton)
                bflags |= State_Down;
            if (toolbutton->activeSubControls & SC_ToolButtonMenu)
                mflags |= State_Down;

            QStyleOption tool(0);
            tool.palette = toolbutton->palette;
            if (toolbutton->subControls & SC_ToolButton) {
                if (bflags & (State_Down | State_On | State_Raised)) {
                    tool.rect = button;
                    tool.state = bflags;
                    drawPrimitive(PE_PanelButtonTool, &tool, p, widget);
                }
            }

            if (toolbutton->subControls & SC_ToolButtonMenu) {
                tool.rect = menuarea;
                tool.state = mflags;
                if (mflags & (State_Down | State_On | State_Raised))
                    drawPrimitive(PE_IndicatorButtonDropDown, &tool, p, widget);
                drawPrimitive(PE_IndicatorArrowDown, &tool, p, widget);
            }

            if (toolbutton->state & State_HasFocus) {
                QStyleOptionFocusRect fr;
                fr.rect = toolbutton->rect;
                fr.rect.addCoords(3, 3, -3, -3);
                fr.palette = toolbutton->palette;
                fr.state = State_None;
                drawPrimitive(PE_FrameFocusRect, &fr, p, widget);
            }
            QStyleOptionToolButton label = *toolbutton;
            int fw = pixelMetric(PM_DefaultFrameWidth, opt, widget);
            label.rect = button.adjusted(fw, fw, -fw, -fw);
            drawControl(CE_ToolButtonLabel, &label, p, widget);
        }
        break;
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *tb = qt_cast<const QStyleOptionTitleBar *>(opt)) {
            QRect ir;
            if (opt->subControls & SC_TitleBarLabel) {
                QColor left = tb->palette.highlight().color();
                QColor right = tb->palette.base().color();

                QBrush fillBrush(left);
                if (left != right) {
                    QPoint p1(tb->rect.x(), tb->rect.top() + tb->rect.height()/2);
                    QPoint p2(tb->rect.right(), tb->rect.top() + tb->rect.height()/2);
                    fillBrush = QBrush(p1, left, p2, right);
                }

                p->fillRect(opt->rect, fillBrush);

                ir = visualRect(opt->direction, opt->rect,
                        subControlRect(CC_TitleBar, tb, SC_TitleBarLabel, widget));

                p->setPen(tb->palette.highlightedText().color());
                p->drawText(ir.x() + 2, ir.y(), ir.width() - 2, ir.height(),
                            Qt::AlignAuto | Qt::AlignVCenter | Qt::TextSingleLine, tb->text);
            }

            bool down = false;
            QPixmap pm;

            QStyleOption tool(0);
            tool.palette = tb->palette;
            if (tb->subControls & SC_TitleBarCloseButton) {
                ir = visualRect(opt->direction, opt->rect,
                        subControlRect(CC_TitleBar, tb, SC_TitleBarCloseButton, widget));
                down = tb->activeSubControls & SC_TitleBarCloseButton;
                if (tb->titleBarFlags & Qt::WStyle_Tool
#ifndef QT_NO_MAINWINDOW
                     || qt_cast<const QDockWindow *>(widget)
#endif
                   )
                    pm = standardPixmap(SP_DockWindowCloseButton, &tool, widget);
                else
                    pm = standardPixmap(SP_TitleBarCloseButton, &tool, widget);
                tool.rect = ir;
                tool.state = down ? State_Down : State_Raised;
                drawPrimitive(PE_PanelButtonTool, &tool, p, widget);

                p->save();
                if (down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, tb, widget),
                                 pixelMetric(PM_ButtonShiftVertical, tb, widget));
                drawItemPixmap(p, ir, Qt::AlignCenter, tb->palette, pm);
                p->restore();
            }

            if (tb->subControls & SC_TitleBarMaxButton
                && tb->titleBarFlags & Qt::WStyle_Maximize) {
                ir = visualRect(opt->direction, opt->rect,
                        subControlRect(CC_TitleBar, tb, SC_TitleBarMaxButton, widget));

                down = tb->activeSubControls & SC_TitleBarMaxButton;
                pm = standardPixmap(SP_TitleBarMaxButton, &tool, widget);
                tool.rect = ir;
                tool.state = down ? State_Down : State_Raised;
                drawPrimitive(PE_PanelButtonTool, &tool, p, widget);

                p->save();
                if (down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, tb, widget),
                                 pixelMetric(PM_ButtonShiftVertical, tb, widget));
                drawItemPixmap(p, ir, Qt::AlignCenter, tb->palette, pm);
                p->restore();
            }

            if ((tb->subControls & SC_TitleBarNormalButton
                 || tb->subControls & SC_TitleBarMinButton)
                && tb->titleBarFlags & Qt::WStyle_Minimize) {
                ir = visualRect(opt->direction, opt->rect,
                        subControlRect(CC_TitleBar, tb, SC_TitleBarMinButton, widget));

                QStyle::SubControl ctrl = (tb->subControls & SC_TitleBarNormalButton ?
                                           SC_TitleBarNormalButton :
                                           SC_TitleBarMinButton);
                QStyle::StandardPixmap spixmap = (tb->subControls & SC_TitleBarNormalButton ?
                                               SP_TitleBarNormalButton :
                                               SP_TitleBarMinButton);
                down = tb->activeSubControls & ctrl;
                pm = standardPixmap(spixmap, &tool, widget);
                tool.rect = ir;
                tool.state = down ? State_Down : State_Raised;
                drawPrimitive(PE_PanelButtonTool, &tool, p, widget);

                p->save();
                if (down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, tb, widget),
                                 pixelMetric(PM_ButtonShiftVertical, tb, widget));
                drawItemPixmap(p, ir, Qt::AlignCenter, tb->palette, pm);
                p->restore();
            }

            if (tb->subControls & SC_TitleBarShadeButton) {
                ir = visualRect(opt->direction, opt->rect,
                        subControlRect(CC_TitleBar, tb, SC_TitleBarShadeButton, widget));

                down = tb->activeSubControls & SC_TitleBarShadeButton;
                pm = standardPixmap(SP_TitleBarShadeButton, &tool, widget);
                tool.rect = ir;
                tool.state = down ? State_Down : State_Raised;
                drawPrimitive(PE_PanelButtonTool, &tool, p, widget);
                p->save();
                if (down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, tb, widget),
                                 pixelMetric(PM_ButtonShiftVertical, tb, widget));
                drawItemPixmap(p, ir, Qt::AlignCenter, tb->palette, pm);
                p->restore();
            }

            if (tb->subControls & SC_TitleBarUnshadeButton) {
                ir = visualRect(opt->direction, opt->rect,
                        subControlRect(CC_TitleBar, tb, SC_TitleBarUnshadeButton, widget));

                down = tb->activeSubControls & SC_TitleBarUnshadeButton;
                pm = standardPixmap(SP_TitleBarUnshadeButton, &tool, widget);
                tool.rect = ir;
                tool.state = down ? State_Down : State_Raised;
                drawPrimitive(PE_PanelButtonTool, &tool, p, widget);
                p->save();
                if (down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, tb, widget),
                                 pixelMetric(PM_ButtonShiftVertical, tb, widget));
                drawItemPixmap(p, ir, Qt::AlignCenter, tb->palette, pm);
                p->restore();
            }
            if (tb->subControls & SC_TitleBarContextHelpButton
                && tb->titleBarFlags & Qt::WStyle_ContextHelp) {
                ir = visualRect(opt->direction, opt->rect,
                        subControlRect(CC_TitleBar, tb, SC_TitleBarContextHelpButton,
                                               widget));

                down = tb->activeSubControls & SC_TitleBarContextHelpButton;
                pm = standardPixmap(SP_TitleBarContextHelpButton, &tool, widget);
                tool.rect = ir;
                tool.state = down ? State_Down : State_Raised;
                drawPrimitive(PE_PanelButtonTool, &tool, p, widget);
                p->save();
                if (down)
                    p->translate(pixelMetric(PM_ButtonShiftHorizontal, tb, widget),
                                 pixelMetric(PM_ButtonShiftVertical, tb, widget));
                drawItemPixmap(p, ir, Qt::AlignCenter, tb->palette, pm);
                p->restore();
            }
#ifndef QT_NO_WIDGET_TOPEXTRA
            if (tb->subControls & SC_TitleBarSysMenu && tb->titleBarFlags & Qt::WStyle_SysMenu) {
                ir = visualRect(opt->direction, opt->rect,
                        subControlRect(CC_TitleBar, tb, SC_TitleBarSysMenu, widget));
                if (!tb->icon.isNull()) {
                    drawItemPixmap(p, ir, Qt::AlignCenter, tb->palette, tb->icon);
                } else {
                    pm = standardPixmap(SP_TitleBarMenuButton, &tool, widget);
                    tool.rect = ir;
                    p->save();
                    drawItemPixmap(p, ir, Qt::AlignCenter, tb->palette, pm);
                    p->restore();
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
    Returns the sub-widget in the complex control \a cc, with style
    options \a opt, at point \a pt, and with parent widget \a widget.

    \sa subControlRect()
*/
QStyle::SubControl QCommonStyle::hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                                 const QPoint &pt, const QWidget *widget) const
{
    SubControl sc = SC_None;
    switch (cc) {
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            QRect r = visualRect(opt->direction, opt->rect,
                                 subControlRect(cc, slider, SC_SliderHandle, widget));
            if (r.isValid() && r.contains(pt)) {
                sc = SC_SliderHandle;
            } else {
                r = visualRect(opt->direction, opt->rect,
                               subControlRect(cc, slider, SC_SliderGroove ,widget));
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
                r = visualRect(opt->direction, opt->rect,
                               subControlRect(cc, scrollbar, QStyle::SubControl(ctrl),
                                                      widget));
                if (r.isValid() && r.contains(pt)) {
                    sc = QStyle::SubControl(ctrl);
                    break;
                }
                ctrl <<= 1;
            }
        }
        break;
    case CC_Q3ListView:
        if (const QStyleOptionQ3ListView *lv = qt_cast<const QStyleOptionQ3ListView *>(opt)) {
            if (pt.x() >= 0 && pt.x() < lv->treeStepSize)
                sc = SC_Q3ListViewExpand;
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinbox = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            QRect r;
            uint ctrl = SC_SpinBoxUp;
            while (ctrl <= SC_SpinBoxSlider) {
                r = visualRect(opt->direction, opt->rect,
                               subControlRect(cc, spinbox, QStyle::SubControl(ctrl),
                                                      widget));
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
                r = visualRect(opt->direction, opt->rect,
                               subControlRect(cc, tb, QStyle::SubControl(ctrl), widget));
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
                r = visualRect(opt->direction, opt->rect,
                               subControlRect(cc, cmb, QStyle::SubControl(ctrl), widget));
                if (r.isValid() && r.contains(pt)) {
                    sc = QStyle::SubControl(ctrl);
                    break;
                }
                ctrl >>= 1;
            }
        }
        break;
    default:
        qWarning("QCommonStyle::hitTestComplexControl case not handled %d", cc);
    }
    return sc;
}

/*!
    Returns the rectangle occupied by the complex control \a cc, with
    style options \a opt, sub-control \a sc. The \a widget is optional
    and may contain a widget that is useful for drawing the
    sub-control.

    \sa hitTestComplexControl()
*/
QRect QCommonStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                                           SubControl sc, const QWidget *widget) const
{
    QRect ret;
    switch (cc) {
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            int tickOffset = pixelMetric(PM_SliderTickmarkOffset, slider, widget);
            int thickness = pixelMetric(PM_SliderControlThickness, slider, widget);

            switch (sc) {
            case SC_SliderHandle: {
                int sliderPos = 0;
                int len = pixelMetric(PM_SliderLength, slider, widget);
                bool horizontal = slider->orientation == Qt::Horizontal;
                sliderPos = sliderPositionFromValue(slider->minimum, slider->maximum,
                                                    slider->sliderPosition,
                                                    (horizontal ? slider->rect.width()
                                                                : slider->rect.height()) - len,
                                                    slider->upsideDown);
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
            int sbextent = pixelMetric(PM_ScrollBarExtent, scrollbar, widget);
            int maxlen = ((scrollbar->orientation == Qt::Horizontal) ?
                          scrollbar->rect.width() : scrollbar->rect.height()) - (sbextent * 2);
            int sliderlen;

            // calculate slider length
            if (scrollbar->maximum != scrollbar->minimum) {
                uint range = scrollbar->maximum - scrollbar->minimum;
                sliderlen = (scrollbar->pageStep * maxlen) / (range + scrollbar->pageStep);

                int slidermin = pixelMetric(PM_ScrollBarSliderMin, scrollbar, widget);
                if (sliderlen < slidermin || range > INT_MAX / 2)
                    sliderlen = slidermin;
                if (sliderlen > maxlen)
                    sliderlen = maxlen;
            } else {
                sliderlen = maxlen;
            }

            int sliderstart = sbextent + sliderPositionFromValue(scrollbar->minimum,
                                                                 scrollbar->maximum,
                                                                 scrollbar->sliderPosition,
                                                                 maxlen - sliderlen,
                                                                 scrollbar->upsideDown);
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
            default:
                break;
            }
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinbox = qt_cast<const QStyleOptionSpinBox *>(opt)) {

            int fw = spinbox->showFrame ? pixelMetric(PM_SpinBoxFrameWidth, spinbox, widget) : 0;
            int slider = spinbox->showSliderIndicator ? qMax(int(spinbox->rect.height() / 20),
                                             pixelMetric(PM_SpinBoxSliderHeight, spinbox, widget))
                                         : 0;
            QSize bs;

            bs.setHeight(qMax(8, spinbox->rect.height()/2 - fw));
            // 1.6 -approximate golden mean
            bs.setWidth(qMax(16, qMin(bs.height() * 8 / 5, spinbox->rect.width() / 4)));
            bs = bs.expandedTo(QApplication::globalStrut());
            int y = fw;
            int x, lx, rx;
            x = spinbox->rect.width() - y - bs.width();
            lx = fw;
            rx = x - fw;
            switch (sc) {
            case SC_SpinBoxUp:
                ret = QRect(x, y, bs.width(), bs.height());
                break;
            case SC_SpinBoxDown:
                ret = QRect(x, y + bs.height(), bs.width(), bs.height());
                break;
            case SC_SpinBoxButtonField:
                ret = QRect(x, y, bs.width(), spinbox->rect.height() - 2*fw);
                break;
            case SC_SpinBoxEditField:
                ret = QRect(lx, fw, rx, spinbox->rect.height() - 2*fw - slider);
                break;
            case SC_SpinBoxSlider:
                ret = (slider > 0 ? QRect(lx, spinbox->rect.height() - fw - slider, rx, slider)
                                  : QRect());
                break;
            case SC_SpinBoxFrame:
                ret = spinbox->showFrame ? spinbox->rect : QRect();
            default:
                break;
            }
        }
        break;
    case CC_ToolButton:
        if (const QStyleOptionToolButton *tb = qt_cast<const QStyleOptionToolButton *>(opt)) {
            int mbi = pixelMetric(PM_MenuButtonIndicator, tb, widget);
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
            const int controlMargin = 2;
            const int controlHeight = tb->rect.height() - controlMargin *2;
            const int delta = controlHeight + controlMargin;
            int offset = 0;

            switch (sc) {
            case SC_TitleBarLabel:
                ret = tb->rect;
                ret.addCoords(0, 0, -delta, 0); // Close button always there
                if (tb->titleBarFlags & Qt::WStyle_SysMenu)
                    ret.addCoords(delta, 0, -delta, 0);
                if (tb->titleBarFlags & Qt::WStyle_Minimize)
                    ret.addCoords(0, 0, -delta, 0);
                if (tb->titleBarFlags & Qt::WStyle_Maximize)
                    ret.addCoords(0, 0, -delta, 0);
                if (tb->titleBarFlags & Qt::WStyle_ContextHelp)
                    ret.addCoords(0, 0, -delta, 0);
                break;
            case SC_TitleBarContextHelpButton:
                if (tb->titleBarFlags & Qt::WStyle_ContextHelp)
                    offset += delta;
            case SC_TitleBarMinButton:
            case SC_TitleBarNormalButton:
                if (tb->titleBarFlags & Qt::WStyle_Minimize)
                    offset += delta;
            case SC_TitleBarMaxButton:
            case SC_TitleBarShadeButton:
            case SC_TitleBarUnshadeButton:
                if (tb->titleBarFlags & Qt::WStyle_Maximize)
                    offset += delta;
            case SC_TitleBarCloseButton:
                offset += delta;
                ret.setRect(tb->rect.right() - offset, tb->rect.top() + controlMargin,
                            controlHeight, controlHeight);
                break;
            case SC_TitleBarSysMenu:
                ret.setRect(tb->rect.left() + controlMargin, tb->rect.top() + controlMargin,
                            controlHeight, controlHeight);
                break;
            default:
                break;
            }
        }
        break;
    default:
        qWarning("QCommonStyle::subControlRect case not handled %d", cc);
    }
    return ret;
}

/*! \reimp */
int QCommonStyle::pixelMetric(PixelMetric m, const QStyleOption *opt, const QWidget *widget) const
{
    int ret;

    switch (m) {
    case PM_FocusFrameVMargin:
    case PM_FocusFrameHMargin:
        ret = 2;
        break;
    case PM_MenuBarVMargin:
    case PM_MenuBarHMargin:
        ret = 0;
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
        if (const QStyleOptionTitleBar *tb = qt_cast<const QStyleOptionTitleBar *>(opt)) {
            if (tb->titleBarFlags & Qt::WStyle_Tool) {
                ret = qMax(widget ? widget->fontMetrics().lineSpacing() : 0, 16);
#ifndef QT_NO_MAINWINDOW
            } else if (qt_cast<const QDockWindow*>(widget)) {
                ret = qMax(widget->fontMetrics().lineSpacing(), 13);
#endif
            } else {
                ret = qMax(widget ? widget->fontMetrics().lineSpacing() : 0, 18);
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
        if (!opt)
            ret = 12;
        else
            ret = qMax(12, (opt->rect.height() - 4) / 3);
        break;

    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:

    case PM_MenuPanelWidth:
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
        if (const QStyleOptionSlider *sb = qt_cast<const QStyleOptionSlider *>(opt)) {
            int s = sb->orientation == Qt::Horizontal ?
                    QApplication::globalStrut().height()
                    : QApplication::globalStrut().width();
            ret = qMax(16, s);
        } else {
            ret = 16;
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
        if (const QStyleOptionSlider *sl = qt_cast<const QStyleOptionSlider *>(opt)) {
            int space = (sl->orientation == Qt::Horizontal) ? sl->rect.height()
                                                            : sl->rect.width();
            int thickness = pixelMetric(PM_SliderControlThickness, sl, widget);
            int ticks = sl->tickPosition;

            if (ticks == QSlider::TicksBothSides)
                ret = (space - thickness) / 2;
            else if (ticks == QSlider::TicksAbove)
                ret = space - thickness;
            else
                ret = 0;
        } else {
            ret = 0;
        }
        break;

    case PM_SliderSpaceAvailable:
        if (const QStyleOptionSlider *sl = qt_cast<const QStyleOptionSlider *>(opt)) {
            if (sl->orientation == Qt::Horizontal)
                ret = sl->rect.width() - pixelMetric(PM_SliderLength, sl, widget);
            else
                ret = sl->rect.height() - pixelMetric(PM_SliderLength, sl, widget);
            break;
        } else {
            ret = 0;
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
    case PM_MenuBarPanelWidth:
        ret = 2;
        break;

    case PM_MenuBarItemSpacing:
        ret = 0;
        break;

    case PM_ToolBarFrameWidth:
        ret = 1;
        break;

    case PM_ToolBarItemMargin:
        ret = 0;
        break;

    case PM_ToolBarItemSpacing:
        ret = 4;
        break;

    case PM_ToolBarHandleExtent:
        ret = 8;
        break;

    case PM_ToolBarSeparatorExtent:
        ret = 6;
        break;

    case PM_ToolBarExtensionExtent:
        ret = 14;
        break;

    case PM_TabBarTabOverlap:
        ret = 3;
        break;

    case PM_TabBarBaseHeight:
        ret = 2;
        break;

    case PM_TabBarBaseOverlap:
        ret = 1;
        break;

    case PM_TabBarTabHSpace:
        ret = 24;
        break;

    case PM_TabBarTabShiftHorizontal:
        ret = 0;
    case PM_TabBarTabShiftVertical:
        ret = 2;
        break;

#ifndef QT_NO_TABBAR
    case PM_TabBarTabVSpace: {
        const QStyleOptionTab *tb = qt_cast<const QStyleOptionTab *>(opt);
        if (tb && (tb->shape == QTabBar::RoundedNorth || tb->shape == QTabBar::RoundedSouth
                   || tb->shape == QTabBar::RoundedWest || tb->shape == QTabBar::RoundedEast))
            ret = 10;
        else
            ret = 0;
        break; }
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

    case PM_ToolBarIconSize:
        ret = pixelMetric(PM_SmallIconSize);
        break;

    case PM_ListViewIconSize:
        ret = pixelMetric(PM_SmallIconSize);
        break;
    case PM_IconViewIconSize:
        ret = pixelMetric(PM_LargeIconSize);
        break;

    case PM_SmallIconSize:
        ret = 22;
        break;
    case PM_LargeIconSize:
        ret = 32;
        break;

    default:
        ret = 0;
        break;
    }

    return ret;
}

/*!
    Returns the size required by the contents of type \a ct, with
    style options \a opt, original size \a csz.  The \a widget is
    optional and may contain a widget that is useful for calculating
    the size.
*/
QSize QCommonStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt,
                                     const QSize &csz, const QWidget *widget) const
{
    QSize sz(csz);
    switch (ct) {
    case CT_PushButton:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            int w = csz.width(),
                h = csz.height(),
                bm = pixelMetric(PM_ButtonMargin, btn, widget),
            fw = pixelMetric(PM_DefaultFrameWidth, btn, widget) * 2;
            w += bm + fw;
            h += bm + fw;
            if (btn->features & QStyleOptionButton::AutoDefaultButton){
                int dbw = pixelMetric(PM_ButtonDefaultIndicator, btn, widget) * 2;
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
            QRect irect = subRect(isRadio ? SR_RadioButtonIndicator
                                          : SR_CheckBoxIndicator, btn, widget);
            int h = pixelMetric(isRadio ? PM_ExclusiveIndicatorHeight
                                        : PM_IndicatorHeight, btn, widget);
            int margins = (!btn->icon.isNull() && btn->text.isEmpty()) ? 0 : 10;
            sz += QSize(irect.right() + margins, 4);
            sz.setHeight(qMax(sz.height(), h));
        }
        break;
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            bool checkable = mi->checkType != QStyleOptionMenuItem::NotCheckable;
            int maxpmw = mi->maxIconWidth;
            int w = sz.width(), h = sz.height();
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                w = 10;
                h = 2;
            } else {
                h = qMax(h, mi->fontMetrics.height() + 8);
                if (!mi->icon.isNull())
                    h = qMax(h, mi->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal).height() + 4);
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
        int dfw = pixelMetric(PM_DefaultFrameWidth, opt, widget) * 2;
        sz = QSize(sz.width() + dfw + 21, sz.height() + dfw);
        break; }
    case CT_HeaderSection:
        if (const QStyleOptionHeader *hdr = qt_cast<const QStyleOptionHeader *>(opt)) {
            int margin = pixelMetric(QStyle::PM_HeaderMargin);
            QSize icn = hdr->icon.isNull() ? QSize(0,0) : QSize(22,22);
            QSize txt = hdr->fontMetrics.size(0, hdr->text);
            sz.setHeight(margin + qMax(icn.height(), txt.height()) + margin);
            sz.setWidth(margin + icn.width() + margin + txt.width() + margin);
        }
        break;
    case CT_MenuBar:
    case CT_Menu:
    case CT_MenuBarItem:
    case CT_LineEdit:
    case CT_Header:
    case CT_Slider:
    case CT_ProgressBar:
    case CT_TabBarTab:
        // just return the contentsSize for now
        // fall through intended
    default:
        break;
    }
    return sz;
}


/*! \reimp */
int QCommonStyle::styleHint(StyleHint sh, const QStyleOption *opt, const QWidget *widget,
                            QStyleHintReturn *hret) const
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
        ret = int(opt ? opt->palette.foreground().color().rgb() : 0);
        break;

    case SH_Q3ListViewExpand_SelectMouseType:
    case SH_TabBar_SelectMouseType:
        ret = QEvent::MouseButtonPress;
        break;

#ifdef QT3_SUPPORT
    case SH_GUIStyle:
        ret = Qt::WindowsStyle;
        break;
#endif

#ifndef QT_NO_PALETTE
    case SH_ScrollBar_BackgroundRole:
        ret = QPalette::Background;
        break;
#endif

    case SH_TabBar_Alignment:
    case SH_Header_ArrowAlignment:
        ret = Qt::AlignLeft;
        break;

    case SH_TitleBar_AutoRaise:
        ret = false;
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
        if (opt)
            ret = opt->palette.color(QPalette::Mid).rgb();
        else
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

    case SH_Button_FocusPolicy:
        ret = Qt::StrongFocus;
        break;

    case SH_MenuBar_DismissOnSecondClick:
        ret = 1;
        break;

    case SH_MessageBox_UseBorderForButtonSpacing:
        ret = 0;
        break;

    case SH_ToolButton_PopupDelay:
        ret = 600;
        break;

    case SH_RubberBand_Mask:
        ret = false;
        if (widget && (opt->state & State_Rectangle)) {
            ret = true;
            if(QStyleHintReturnMask *mask = qt_cast<QStyleHintReturnMask*>(hret)) {
                mask->region = widget->rect();
                int margin = pixelMetric(PM_DefaultFrameWidth) * 2;
                mask->region -= opt->rect.adjusted(margin, margin, -margin, -margin);
            }
        }
        break;

    default:
        ret = 0;
        break;
    }

    return ret;
}

/*! \reimp */
QPixmap QCommonStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *,
                                     const QWidget *) const
{
#ifndef QT_NO_IMAGEIO_XPM
    switch (standardPixmap) {
    case SP_ToolBarHorizontalExtensionButton:
        return QPixmap((const char **)tb_extension_arrow_h_xpm);
    case SP_ToolBarVerticalExtensionButton:
        return QPixmap((const char **)tb_extension_arrow_v_xpm);
    case SP_FileDialogStart:
        return QPixmap((const char **)filedialog_start_xpm);
    case SP_FileDialogEnd:
        return QPixmap((const char **)filedialog_end_xpm);
    case SP_FileDialogToParent:
        return QPixmap((const char **)filedialog_toparent_xpm);
    case SP_FileDialogNewFolder:
        return QPixmap((const char **)filedialog_newfolder_xpm);
    case SP_FileDialogDetailedView:
        return QPixmap((const char **)filedialog_detailedview_xpm);
    case SP_FileDialogInfoView:
        return QPixmap((const char **)filedialog_previewinfoview_xpm);
    case SP_FileDialogContentsView:
        return QPixmap((const char **)filedialog_previewcontentsview_xpm);
    case SP_FileDialogListView:
        return QPixmap((const char **)filedialog_mclistview_xpm);
    case SP_FileDialogBack:
        return QPixmap((const char **)filedialog_back_xpm);
    default:
        break;
    }
#endif // QT_NO_IMAGEIO_XPM
    return QPixmap();
}

static inline uint qt_intensity(uint r, uint g, uint b)
{
    // 30% red, 59% green, 11% blue
    return (77 * r + 150 * g + 28 * b) / 255;
}

/*! \reimp */
QPixmap QCommonStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap,
                                          const QStyleOption *opt) const
{
    switch (iconMode) {
    case QIcon::Disabled: {
        QImage im = pixmap.toImage();

        // Create a colortable based on the background (black -> bg -> white)
        QColor bg = opt->palette.color(QPalette::Disabled, QPalette::Background);
        int red = bg.red();
        int green = bg.green();
        int blue = bg.blue();
        uchar reds[256], greens[256], blues[256];
        for (int i=0; i<128; ++i) {
            reds[i]   = uchar((red   * (i<<1)) >> 8);
            greens[i] = uchar((green * (i<<1)) >> 8);
            blues[i]  = uchar((blue  * (i<<1)) >> 8);
        }
        for (int i=0; i<128; ++i) {
            reds[i+128]   = uchar(qMin(red   + (i << 1), 255));
            greens[i+128] = uchar(qMin(green + (i << 1), 255));
            blues[i+128]  = uchar(qMin(blue  + (i << 1), 255));
        }

        int intensity = qt_intensity(red, green, blue);
        const int factor = 191;

        // High intensity colors needs dark shifting in the color table, while
        // low intensity colors needs light shifting. This is to increase the
        // percieved contrast.
        if ((red - factor > green && red - factor > blue)
            || (green - factor > red && green - factor > blue)
            || (blue - factor > red && blue - factor > green))
            intensity = qMin(255, intensity + 91);
        else if (intensity <= 128)
            intensity -= 51;

        for (int y=0; y<im.height(); ++y) {
            QRgb *scanLine = (QRgb*)im.scanLine(y);
            for (int x=0; x<im.width(); ++x) {
                QRgb pixel = *scanLine;
                // Calculate color table index, taking intensity adjustment
                // and a magic offset into account.
                uint ci = uint(qGray(pixel)/3 + (130 - intensity / 3));
                *scanLine = qRgba(reds[ci], greens[ci], blues[ci], qAlpha(pixel));
                ++scanLine;
            }
        }

        return QPixmap(im);
    }
    case QIcon::Active:
        return pixmap;
    default:
        break;
    }
    return pixmap;
}

#endif // QT_NO_STYLE
