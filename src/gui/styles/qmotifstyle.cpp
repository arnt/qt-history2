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

#include "qmotifstyle.h"

#if !defined(QT_NO_STYLE_MOTIF) || defined(QT_PLUGIN)

#include "qmenu.h"
#ifdef QT_COMPAT
# include "q3popupmenu.h"
#endif
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
#include "qprogressbar.h"
#include "qimage.h"
#include <limits.h>



// old constants that might still be useful...
static const int motifItemFrame         = 2;    // menu item frame width
static const int motifSepHeight         = 2;    // separator item height
static const int motifItemHMargin       = 3;    // menu item hor text margin
static const int motifItemVMargin       = 2;    // menu item ver text margin
static const int motifArrowHMargin      = 6;    // arrow horizontal margin
static const int motifTabSpacing        = 12;   // space between text and tab
static const int motifCheckMarkHMargin  = 2;    // horiz. margins of check mark
static const int motifCheckMarkSpace    = 12;


/*!
    \class QMotifStyle qmotifstyle.h
    \brief The QMotifStyle class provides Motif look and feel.

    \ingroup appearance

    This class implements the Motif look and feel. It closely
    resembles the original Motif look as defined by the Open Group,
    but with some minor improvements. The Motif style is Qt's default
    GUI style on UNIX platforms.

    Most of the functions are documented in the base classes,
    \l{QCommonStyle} and \l{QStyle}, but the functions overloaded by
    QMotifStyle, drawComplexControl(), drawControl(), drawPrimitive(),
    querySubControlMetrics(), setUseHighlightColors(),
    sizeFromContents(), subRect(), and useHighlightColors(), are
    documented here.
*/

/*!
    Constructs a QMotifStyle.

    If \a useHighlightCols is false (the default), the style will
    polish the application's color palette to emulate the Motif way of
    highlighting, which is a simple inversion between the base and the
    text color.
*/
QMotifStyle::QMotifStyle(bool useHighlightCols) : QCommonStyle()
{
    highlightCols = useHighlightCols;
}

/*!
    \overload

    Destroys the style.
*/
QMotifStyle::~QMotifStyle()
{
}

/*!
    If \a arg is false, the style will polish the application's color
    palette to emulate the Motif way of highlighting, which is a
    simple inversion between the base and the text color.

    The effect will show up the next time an application palette is
    set via QApplication::setPalette(). The current color palette of
    the application remains unchanged.

    \sa QStyle::polish()
*/
void QMotifStyle::setUseHighlightColors(bool arg)
{
    highlightCols = arg;
}

/*!
    Returns true if the style treats the highlight colors of the
    palette in a Motif-like manner, which is a simple inversion
    between the base and the text color; otherwise returns false. The
    default is false.
*/
bool QMotifStyle::useHighlightColors() const
{
    return highlightCols;
}

/*! \reimp */

void QMotifStyle::polish(QPalette& pal)
{
    if (pal.brush(QPalette::Active, QPalette::Light) == pal.brush(QPalette::Active, QPalette::Base)) {
        QColor nlight = pal.color(QPalette::Active, QPalette::Light).dark(108);
        pal.setColor(QPalette::Active, QPalette::Light, nlight) ;
        pal.setColor(QPalette::Disabled, QPalette::Light, nlight) ;
        pal.setColor(QPalette::Inactive, QPalette::Light, nlight) ;
    }

    if (highlightCols)
        return;

    // force the ugly motif way of highlighting *sigh*
    pal.setColor(QPalette::Active, QPalette::Highlight,
                  pal.color(QPalette::Active, QPalette::Text));
    pal.setColor(QPalette::Active, QPalette::HighlightedText,
                  pal.color(QPalette::Active, QPalette::Base));
    pal.setColor(QPalette::Disabled, QPalette::Highlight,
                  pal.color(QPalette::Disabled, QPalette::Text));
    pal.setColor(QPalette::Disabled, QPalette::HighlightedText,
                  pal.color(QPalette::Disabled, QPalette::Base));
    pal.setColor(QPalette::Inactive, QPalette::Highlight,
                  pal.color(QPalette::Active, QPalette::Text));
    pal.setColor(QPalette::Inactive, QPalette::HighlightedText,
                  pal.color(QPalette::Active, QPalette::Base));
}

/*!
 \reimp
 \internal
 Keep QStyle::polish() visible.
*/
void QMotifStyle::polish(QWidget* w)
{
    QStyle::polish(w);
    if(QMenu *menu = qt_cast<QMenu*>(w))
        menu->setCheckable(false);
}

/*!
 \reimp
 \internal
 Keep QStyle::polish() visible.
*/
void QMotifStyle::polish(QApplication* a)
{
    QStyle::polish(a);
}

static void rot(QPointArray& a, int n)
{
    QPointArray r(a.size());
    for (int i = 0; i < (int)a.size(); i++) {
        switch (n) {
            case 1: r.setPoint(i,-a[i].y(),a[i].x()); break;
            case 2: r.setPoint(i,-a[i].x(),-a[i].y()); break;
            case 3: r.setPoint(i,a[i].y(),-a[i].x()); break;
        }
    }
    a = r;
}


/*!
    \reimp
*/
void QMotifStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                                const QWidget *w) const
/*
void QMotifStyle::drawPrimitive(PrimitiveElement pe,
                                 QPainter *p,
                                 const QRect &r,
                                 const QPalette &pal,
                                 SFlags flags,
                                 const Q3StyleOption& opt) const
                                 */
{
    switch(pe) {
#ifndef QT_NO_LISTVIEW
    case PE_CheckListExclusiveIndicator: {
        QCheckListItem *item = opt.checkListItem();
        QListView *lv = item->listView();
        if(!item)
            return;

        if (item->isEnabled())
            p->setPen(QPen(pal.text()));
        else
            p->setPen(QPen(lv->palette().color(QPalette::Disabled, QPalette::Text)));
        QPointArray a;

        int cx = r.width()/2 - 1;
        int cy = r.height()/2;
        int e = r.width()/2 - 1;
        for (int i = 0; i < 3; i++) { //penWidth 2 doesn't quite work
            a.setPoints(4, cx-e, cy, cx, cy-e,  cx+e, cy,  cx, cy+e);
            p->drawPolygon(a);
            e--;
        }
        if (item->isOn()) {
            if (item->isEnabled())
                p->setPen(QPen(pal.text()));
            else
                p->setPen(QPen(item->listView()->palette().color(QPalette::Disabled,
                                                                    QPalette::Text)));
            QBrush saveBrush = p->brush();
            p->setBrush(pal.text());
            e = e - 2;
            a.setPoints(4, cx-e, cy, cx, cy-e,  cx+e, cy,  cx, cy+e);
            p->drawPolygon(a);
            p->setBrush(saveBrush);
        }
        break; }
#endif
    case PE_ButtonCommand:
    case PE_ButtonBevel:
    case PE_ButtonTool:
    case PE_HeaderSection: {
        QBrush fill;
        if (flags & Style_Down)
            fill = pal.brush(QPalette::Mid);
        else if (flags & Style_On)
            fill = QBrush(pal.mid(), Qt::Dense4Pattern);
        else
            fill = pal.brush(QPalette::Button);
        qDrawShadePanel(p, r, pal, bool(flags & (Style_Down | Style_On)),
                        pixelMetric(PM_DefaultFrameWidth), &fill);
        break;
    }

    case PE_Indicator: {
#ifndef QT_NO_BUTTON
        bool on = flags & Style_On;
        bool down = flags & Style_Down;
        bool showUp = !(down ^ on);
        QBrush fill = showUp || flags & Style_NoChange ? pal.brush(QPalette::Button) : pal.brush(QPalette::Mid);
        if (flags & Style_NoChange) {
            qDrawPlainRect(p, r, pal.text(),
                            1, &fill);
            p->drawLine(r.x() + r.width() - 1, r.y(),
                         r.x(), r.y() + r.height() - 1);
        } else
            qDrawShadePanel(p, r, pal, !showUp,
                             pixelMetric(PM_DefaultFrameWidth), &fill);
#endif
        break;
    }

    case PE_ExclusiveIndicator:
        {
#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)
            QCOORD inner_pts[] = { // used for filling diamond
                2,r.height()/2,
                r.width()/2,2,
                r.width()-3,r.height()/2,
                r.width()/2,r.height()-3
            };
            QCOORD top_pts[] = { // top (^) of diamond
                0,r.height()/2,
                r.width()/2,0,
                r.width()-2,r.height()/2-1,
                r.width()-3,r.height()/2-1,
                r.width()/2,1,
                1,r.height()/2,
                2,r.height()/2,
                r.width()/2,2,
                r.width()-4,r.height()/2-1
            };
            QCOORD bottom_pts[] = { // bottom (v) of diamond
                1,r.height()/2+1,
                r.width()/2,r.height()-1,
                r.width()-1,r.height()/2,
                r.width()-2,r.height()/2,
                r.width()/2,r.height()-2,
                2,r.height()/2+1,
                3,r.height()/2+1,
                r.width()/2,r.height()-3,
                r.width()-3,r.height()/2
            };
            bool on = flags & Style_On;
            bool down = flags & Style_Down;
            bool showUp = !(down ^ on);
            QPointArray a(QCOORDARRLEN(inner_pts), inner_pts);
            p->setPen(Qt::NoPen);
            p->setBrush(showUp ? pal.brush(QPalette::Button) :
                         pal.brush(QPalette::Mid));
            a.translate(r.x(), r.y());
            p->drawPolygon(a);
            p->setPen(showUp ? pal.light() : pal.dark());
            p->setBrush(Qt::NoBrush);
            a.setPoints(QCOORDARRLEN(top_pts), top_pts);
            a.translate(r.x(), r.y());
            p->drawPolyline(a);
            p->setPen(showUp ? pal.dark() : pal.light());
            a.setPoints(QCOORDARRLEN(bottom_pts), bottom_pts);
            a.translate(r.x(), r.y());
            p->drawPolyline(a);

            break;
        }

    case PE_ExclusiveIndicatorMask:
        {
            static QCOORD inner_pts[] = { // used for filling diamond
                0,r.height()/2,
                r.width()/2,0,
                r.width()-1,r.height()/2,
                r.width()/2,r.height()-1
            };
            QPointArray a(QCOORDARRLEN(inner_pts), inner_pts);
            p->setPen(Qt::color1);
            p->setBrush(Qt::color1);
            a.translate(r.x(), r.y());
            p->drawPolygon(a);
            break;
        }

    case PE_ArrowUp:
    case PE_ArrowDown:
    case PE_ArrowRight:
    case PE_ArrowLeft:
        {
            QRect rect = r;
            QPointArray bFill;
            QPointArray bTop;
            QPointArray bBot;
            QPointArray bLeft;
            bool vertical = pe == PE_ArrowUp || pe == PE_ArrowDown;
            bool horizontal = !vertical;
            int dim = rect.width() < rect.height() ? rect.width() : rect.height();
            int colspec = 0x0000;

            if (dim < 2)
                break;

            // adjust size and center (to fix rotation below)
            if (rect.width() > dim) {
                rect.setX(rect.x() + ((rect.width() - dim) / 2));
                rect.setWidth(dim);
            }
            if (rect.height() > dim) {
                rect.setY(rect.y() + ((rect.height() - dim) / 2));
                rect.setHeight(dim);
            }

            if (dim > 3) {
                if (dim > 6)
                    bFill.resize(dim & 1 ? 3 : 4);
                bTop.resize((dim/2)*2);
                bBot.resize(dim & 1 ? dim + 1 : dim);
                bLeft.resize(dim > 4 ? 4 : 2);
                bLeft.putPoints(0, 2, 0,0, 0,dim-1);
                if (dim > 4)
                    bLeft.putPoints(2, 2, 1,2, 1,dim-3);
                bTop.putPoints(0, 4, 1,0, 1,1, 2,1, 3,1);
                bBot.putPoints(0, 4, 1,dim-1, 1,dim-2, 2,dim-2, 3,dim-2);

                for(int i=0; i<dim/2-2 ; i++) {
                    bTop.putPoints(i*2+4, 2, 2+i*2,2+i, 5+i*2, 2+i);
                    bBot.putPoints(i*2+4, 2, 2+i*2,dim-3-i, 5+i*2,dim-3-i);
                }
                if (dim & 1)                          // odd number size: extra line
                    bBot.putPoints(dim-1, 2, dim-3,dim/2, dim-1,dim/2);
                if (dim > 6) {                        // dim>6: must fill interior
                    bFill.putPoints(0, 2, 1,dim-3, 1,2);
                    if (dim & 1)                      // if size is an odd number
                        bFill.setPoint(2, dim - 3, dim / 2);
                    else
                        bFill.putPoints(2, 2, dim-4,dim/2-1, dim-4,dim/2);
                }
            }
            else {
                if (dim == 3) {                       // 3x3 arrow pattern
                    bLeft.setPoints(4, 0,0, 0,2, 1,1, 1,1);
                    bTop .setPoints(2, 1,0, 1,0);
                    bBot .setPoints(2, 1,2, 2,1);
                }
                else {                                  // 2x2 arrow pattern
                    bLeft.setPoints(2, 0,0, 0,1);
                    bTop .setPoints(2, 1,0, 1,0);
                    bBot .setPoints(2, 1,1, 1,1);
                }
            }

            // We use rot() and translate() as it is more efficient that
            // matrix transformations on the painter, and because it still
            // works with QT_NO_TRANSFORMATIONS defined.

            if (pe == PE_ArrowUp || pe == PE_ArrowLeft) {
                if (vertical) {
                    rot(bFill,3);
                    rot(bLeft,3);
                    rot(bTop,3);
                    rot(bBot,3);
                    bFill.translate(0, rect.height() - 1);
                    bLeft.translate(0, rect.height() - 1);
                    bTop.translate(0, rect.height() - 1);
                    bBot.translate(0, rect.height() - 1);
                } else {
                    rot(bFill,2);
                    rot(bLeft,2);
                    rot(bTop,2);
                    rot(bBot,2);
                    bFill.translate(rect.width() - 1, rect.height() - 1);
                    bLeft.translate(rect.width() - 1, rect.height() - 1);
                    bTop.translate(rect.width() - 1, rect.height() - 1);
                    bBot.translate(rect.width() - 1, rect.height() - 1);
                }
                if (flags & Style_Down)
                    colspec = horizontal ? 0x2334 : 0x2343;
                else
                    colspec = horizontal ? 0x1443 : 0x1434;
            } else {
                if (vertical) {
                    rot(bFill,1);
                    rot(bLeft,1);
                    rot(bTop,1);
                    rot(bBot,1);
                    bFill.translate(rect.width() - 1, 0);
                    bLeft.translate(rect.width() - 1, 0);
                    bTop.translate(rect.width() - 1, 0);
                    bBot.translate(rect.width() - 1, 0);
                }
                if (flags & Style_Down)
                    colspec = horizontal ? 0x2443 : 0x2434;
                else
                    colspec = horizontal ? 0x1334 : 0x1343;
            }
            bFill.translate(rect.x(), rect.y());
            bLeft.translate(rect.x(), rect.y());
            bTop.translate(rect.x(), rect.y());
            bBot.translate(rect.x(), rect.y());

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

#define CMID *cols[(colspec>>12) & 0xf]
#define CLEFT *cols[(colspec>>8) & 0xf]
#define CTOP *cols[(colspec>>4) & 0xf]
#define CBOT *cols[colspec & 0xf]

            QPen savePen = p->pen();
            QBrush saveBrush = p->brush();
            QPen pen(Qt::NoPen);
            QBrush brush = pal.brush(flags & Style_Enabled ? QPalette::Button :
                                     QPalette::Mid);
            p->setPen(pen);
            p->setBrush(brush);
            p->drawPolygon(bFill);
            p->setBrush(Qt::NoBrush);

            p->setPen(CLEFT);
            p->drawLineSegments(bLeft);
            p->setPen(CTOP);
            p->drawLineSegments(bTop);
            p->setPen(CBOT);
            p->drawLineSegments(bBot);

            p->setBrush(saveBrush);
            p->setPen(savePen);
#undef CMID
#undef CLEFT
#undef CTOP
#undef CBOT
            break;
        }

    case PE_SpinBoxPlus:
    case PE_SpinBoxMinus:
        {
            p->save();
            int fw = pixelMetric(PM_DefaultFrameWidth);
            QRect br;
            br.setRect(r.x() + fw, r.y() + fw, r.width() - fw*2,
                        r.height() - fw*2);

            if (flags & Style_Sunken)
                p->fillRect(r, pal.brush(QPalette::Dark));
            else
                p->fillRect(r, pal.brush(QPalette::Button));

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
            if (pe == PE_SpinBoxPlus)
                p->drawLine((x+w / 2) - 1, y + ymarg,
                             (x+w / 2) - 1, y + ymarg + length - 1);
            p->restore();
            break;
        }

    case PE_SpinBoxUp:
    case PE_SpinBoxDown:
        {
            p->save();
            int fw = pixelMetric(PM_DefaultFrameWidth);
            QRect br;
            br.setRect(r.x() + fw, r.y() + fw, r.width() - fw*2,
                        r.height() - fw*2);
            if (flags & Style_Sunken)
                p->fillRect(br, pal.brush(QPalette::Mid));
            else
                p->fillRect(br, pal.brush(QPalette::Button));

            int x = r.x(), y = r.y(), w = r.width(), h = r.height();
            int sw = w-4;
            if (sw < 3)
                return;
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
            if (flags & Style_Sunken) {
                bsx = pixelMetric(PM_ButtonShiftHorizontal);
                bsy = pixelMetric(PM_ButtonShiftVertical);
            }
            p->translate(sx + bsx, sy + bsy);
            p->setPen(pal.buttonText());
            p->setBrush(pal.buttonText());
            p->drawPolygon(a);
            p->restore();
            break;
        }

    case PE_DockWindowHandle:
        {
            p->save();
            p->translate(r.x(), r.y());

            QColor dark(pal.dark());
            QColor light(pal.light());
            int i;
            if (flags & Style_Horizontal) {
                int h = r.height();
                if (h > 6) {
                    if (flags & Style_On)
                        p->fillRect(1, 1, 8, h - 2, pal.highlight());
                    QPointArray a(2 * ((h-6)/3));
                    int y = 3 + (h%3)/2;
                    p->setPen(dark);
                    p->drawLine(8, 1, 8, h-2);
                    for(i=0; 2*i < a.size(); i ++) {
                        a.setPoint(2*i, 5, y+1+3*i);
                        a.setPoint(2*i+1, 2, y+2+3*i);
                    }
                    p->drawPoints(a);
                    p->setPen(light);
                    p->drawLine(9, 1, 9, h-2);
                    for(i=0; 2*i < a.size(); i++) {
                        a.setPoint(2*i, 4, y+3*i);
                        a.setPoint(2*i+1, 1, y+1+3*i);
                    }
                    p->drawPoints(a);
                    // if (drawBorder) {
                    // p->setPen(QPen(Qt::darkGray));
                    // p->drawLine(0, r.height() - 1,
                    // tbExtent, r.height() - 1);
                    // }
                }
            } else {
                int w = r.width();
                if (w > 6) {
                    if (flags & Style_On)
                        p->fillRect(1, 1, w - 2, 9, pal.highlight());
                    QPointArray a(2 * ((w-6)/3));

                    int x = 3 + (w%3)/2;
                    p->setPen(dark);
                    p->drawLine(1, 8, w-2, 8);
                    for(i=0; 2*i < a.size(); i ++) {
                        a.setPoint(2*i, x+1+3*i, 6);
                        a.setPoint(2*i+1, x+2+3*i, 3);
                    }
                    p->drawPoints(a);
                    p->setPen(light);
                    p->drawLine(1, 9, w-2, 9);
                    for(i=0; 2*i < a.size(); i++) {
                        a.setPoint(2*i, x+3*i, 5);
                        a.setPoint(2*i+1, x+1+3*i, 2);
                    }
                    p->drawPoints(a);
                    // if (drawBorder) {
                    // p->setPen(QPen(Qt::darkGray));
                    // p->drawLine(r.width() - 1, 0,
                    // r.width() - 1, tbExtent);
                    // }
                }
            }
            p->restore();
            break;
        }

    case PE_Splitter:
        if (flags & Style_Horizontal)
            flags &= ~Style_Horizontal;
        else
            flags |= Style_Horizontal;
        // fall through intended

    case PE_DockWindowResizeHandle:
        {
            const int motifOffset = 10;
            int sw = pixelMetric(PM_SplitterWidth);
            if (flags & Style_Horizontal) {
                QCOORD yPos = r.y() + r.height() / 2;
                QCOORD kPos = r.width() - motifOffset - sw;
                QCOORD kSize = sw - 2;

                qDrawShadeLine(p, 0, yPos, kPos, yPos, pal);
                qDrawShadePanel(p, kPos, yPos - sw / 2 + 1, kSize, kSize,
                                 pal, false, 1, &pal.brush(QPalette::Button));
                qDrawShadeLine(p, kPos + kSize - 1, yPos, r.width(), yPos, pal);
            } else {
                QCOORD xPos = r.x() + r.width() / 2;
                QCOORD kPos = motifOffset;
                QCOORD kSize = sw - 2;

                qDrawShadeLine(p, xPos, kPos + kSize - 1, xPos, r.height(), pal);
                qDrawShadePanel(p, xPos - sw / 2 + 1, kPos, kSize, kSize, pal,
                                 false, 1, &pal.brush(QPalette::Button));
                qDrawShadeLine(p, xPos, 0, xPos, kPos, pal);
            }
            break;
        }

    case PE_CheckMark:
        {
            const int markW = 6;
            const int markH = 6;
            int posX = r.x() + (r.width()  - markW) / 2 - 1;
            int posY = r.y() + (r.height() - markH) / 2;
            int dfw = pixelMetric(PM_DefaultFrameWidth);

            if (dfw < 2) {
                // Could do with some optimizing/caching...
                QPointArray a(7*2);
                int i, xx, yy;
                xx = posX;
                yy = 3 + posY;
                for (i=0; i<3; i++) {
                    a.setPoint(2*i,   xx, yy);
                    a.setPoint(2*i+1, xx, yy+2);
                    xx++; yy++;
                }
                yy -= 2;
                for (i=3; i<7; i++) {
                    a.setPoint(2*i,   xx, yy);
                    a.setPoint(2*i+1, xx, yy+2);
                    xx++; yy--;
                }
                if (! (flags & Style_Enabled) && ! (flags & Style_On)) {
                    int pnt;
                    p->setPen(pal.highlightedText());
                    QPoint offset(1,1);
                    for (pnt = 0; pnt < (int)a.size(); pnt++)
                        a[pnt] += offset;
                    p->drawLineSegments(a);
                    for (pnt = 0; pnt < (int)a.size(); pnt++)
                        a[pnt] -= offset;
                }
                p->setPen(pal.text());
                p->drawLineSegments(a);

                qDrawShadePanel(p, posX-2, posY-2, markW+4, markH+6, pal, true, dfw);
            } else
                qDrawShadePanel(p, posX, posY, markW, markH, pal, true, dfw,
                                 &pal.brush(QPalette::Mid));

            break;
        }

    case PE_ScrollBarSubLine:
        drawPrimitive(((flags & Style_Horizontal) ? PE_ArrowLeft : PE_ArrowUp),
                      p, r, pal, Style_Enabled | flags);
        break;

    case PE_ScrollBarAddLine:
        drawPrimitive(((flags & Style_Horizontal) ? PE_ArrowRight : PE_ArrowDown),
                      p, r, pal, Style_Enabled | flags);
        break;

    case PE_ScrollBarSubPage:
    case PE_ScrollBarAddPage:
        p->fillRect(r, pal.brush(QPalette::Mid));
        break;

    case PE_ScrollBarSlider:
        drawPrimitive(PE_ButtonBevel, p, r, pal,
                      (flags | Style_Raised) & ~Style_Down);
        break;

    case PE_ProgressBarChunk:
        p->fillRect(r.x(), r.y() + 2, r.width() - 2,
                     r.height() - 4, pal.brush(QPalette::Highlight));
        break;

    default:
        QCommonStyle::drawPrimitive(pe, p, r, pal, flags, opt);
        break;
    }
}


/*!
    \reimp
*/
void QMotifStyle::drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                              const QWidget *w) const
/*
void QMotifStyle::drawControl(ControlElement element,
                               QPainter *p,
                               const QWidget *widget,
                               const QRect &r,
                               const QPalette &pal,
                               SFlags flags,
                               const Q3StyleOption& opt) const
                               */
{
    switch(element) {
    case CE_PushButton:
        {
#ifndef QT_NO_PUSHBUTTON
            int diw, x1, y1, x2, y2;
            const QPushButton *btn;
            btn = (const QPushButton *)widget;
            p->setPen(pal.foreground());
            p->setBrush(QBrush(pal.button(), Qt::NoBrush));
            diw = pixelMetric(PM_ButtonDefaultIndicator);
            r.coords(&x1, &y1, &x2, &y2);
            if (btn->isDefault() || btn->autoDefault()) {
                x1 += diw;
                y1 += diw;
                x2 -= diw;
                y2 -= diw;
            }
            if (btn->isDefault()) {
                if (diw == 0) {
                    QPointArray a;
                    a.setPoints(9,
                                 x1, y1, x2, y1, x2, y2, x1, y2, x1, y1+1,
                                 x2-1, y1+1, x2-1, y2-1, x1+1, y2-1, x1+1, y1+1);
                    p->setPen(pal.shadow());
                    p->drawPolygon(a);
                    x1 += 2;
                    y1 += 2;
                    x2 -= 2;
                    y2 -= 2;
                } else {
                    qDrawShadePanel(p, r, pal, true);
                }
            }
            if (!btn->isFlat() || btn->isOn() || btn->isDown()) {
                QRect tmp(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
                SFlags flags = Style_Default;
                if (btn->isOn())
                    flags |= Style_On;
                if (btn->isDown())
                    flags |= Style_Down;
                p->setBrushOrigin(p->brushOrigin());
                drawPrimitive(PE_ButtonCommand, p, tmp, pal, flags);
            }
#endif
            break;
        }

    case CE_TabBarTab:
        {
#ifndef QT_NO_TABBAR
            if (!widget || !opt.tab())
                break;

            const QTabBar * tb = (const QTabBar *) widget;
            const QTab * t = opt.tab();

            int dfw = pixelMetric(PM_DefaultFrameWidth, tb);
            bool selected = flags & Style_Selected;
            int o =  dfw > 1 ? 1 : 0;
            bool lastTab = false;

            QRect r2(r);
            if (tb->shape() == QTabBar::RoundedAbove) {
                if (styleHint(SH_TabBar_Alignment, tb) == Qt::AlignRight &&
                     tb->indexOf(t->identifier()) == tb->count()-1)
                    lastTab = true;

                if (o) {
                    p->setPen(tb->palette().light());
                    p->drawLine(r2.left(), r2.bottom(), r2.right(), r2.bottom());
                    p->setPen(tb->palette().light());
                    p->drawLine(r2.left(), r2.bottom()-1, r2.right(), r2.bottom()-1);
                    if (r2.left() == 0)
                        p->drawPoint(tb->rect().bottomLeft());
                }
                else {
                    p->setPen(tb->palette().light());
                    p->drawLine(r2.left(), r2.bottom(), r2.right(), r2.bottom());
                }

                if (selected) {
                    p->fillRect(QRect(r2.left()+1, r2.bottom()-o, r2.width()-3, 2),
                                 tb->palette().brush(QPalette::Active, QPalette::Background));
                    p->setPen(tb->palette().background());
                    // p->drawLine(r2.left()+1, r2.bottom(), r2.right()-2, r2.bottom());
                    // if (o)
                    // p->drawLine(r2.left()+1, r2.bottom()-1, r2.right()-2, r2.bottom()-1);
                    p->drawLine(r2.left()+1, r2.bottom(), r2.left()+1, r2.top()+2);
                    p->setPen(tb->palette().light());
                } else {
                    p->setPen(tb->palette().light());
                    r2.setRect(r2.left() + 2, r2.top() + 2,
                                r2.width() - 4, r2.height() - 2);
                }

                p->drawLine(r2.left(), r2.bottom()-1, r2.left(), r2.top() + 2);
                p->drawPoint(r2.left()+1, r2.top() + 1);
                p->drawLine(r2.left()+2, r2.top(),
                             r2.right() - 2, r2.top());
                p->drawPoint(r2.left(), r2.bottom());

                if (o) {
                    p->drawLine(r2.left()+1, r2.bottom(), r2.left()+1, r2.top() + 2);
                    p->drawLine(r2.left()+2, r2.top()+1,
                                 r2.right() - 2, r2.top()+1);
                }

                p->setPen(tb->palette().dark());
                p->drawLine(r2.right() - 1, r2.top() + 2,
                             r2.right() - 1, r2.bottom() - 1 + (selected ? o : -o));
                if (o) {
                    p->drawPoint(r2.right() - 1, r2.top() + 1);
                    p->drawLine(r2.right(), r2.top() + 2, r2.right(),
                                 r2.bottom() -
                                 (selected ? (lastTab ? 0:1):1+o));
                    p->drawPoint(r2.right() - 1, r2.top() + 1);
                }
            } else if (tb->shape()  == QTabBar::RoundedBelow) {
                if (styleHint(SH_TabBar_Alignment, tb) == Qt::AlignLeft &&
                     tb->indexOf(t->identifier()) == tb->count()-1)
                    lastTab = true;
                if (selected) {
                    p->fillRect(QRect(r2.left()+1, r2.top(), r2.width()-3, 1),
                                 tb->palette().brush(QPalette::Active, QPalette::Background));
                    p->setPen(tb->palette().background());
                    // p->drawLine(r2.left()+1, r2.top(), r2.right()-2, r2.top());
                    p->drawLine(r2.left()+1, r2.top(), r2.left()+1, r2.bottom()-2);
                    p->setPen(tb->palette().dark());
                } else {
                    p->setPen(tb->palette().dark());
                    p->drawLine(r2.left(), r2.top(), r2.right(), r2.top());
                    p->drawLine(r2.left() + 1, r2.top() + 1,
                                 r2.right() - (lastTab ? 0 : 2),
                                 r2.top() + 1);
                    r2.setRect(r2.left() + 2, r2.top(),
                                r2.width() - 4, r2.height() - 2);
                }

                p->drawLine(r2.right() - 1, r2.top(),
                             r2.right() - 1, r2.bottom() - 2);
                p->drawPoint(r2.right() - 2, r2.bottom() - 2);
                p->drawLine(r2.right() - 2, r2.bottom() - 1,
                             r2.left() + 1, r2.bottom() - 1);
                p->drawPoint(r2.left() + 1, r2.bottom() - 2);

                if (dfw > 1) {
                    p->drawLine(r2.right(), r2.top(),
                                 r2.right(), r2.bottom() - 1);
                    p->drawPoint(r2.right() - 1, r2.bottom() - 1);
                    p->drawLine(r2.right() - 1, r2.bottom(),
                                 r2.left() + 2, r2.bottom());
                }

                p->setPen(tb->palette().light());
                p->drawLine(r2.left(), r2.top() + (selected ? 0 : 2),
                             r2.left(), r2.bottom() - 2);
                p->drawLine(r2.left() + 1, r2.top() + (selected ? 0 : 2),
                             r2.left() + 1, r2.bottom() - 3);

            } else {
                QCommonStyle::drawControl(element, p, widget, r, pal, flags, opt);
            }
#endif
            break;
        }

    case CE_ProgressBarGroove:
        qDrawShadePanel(p, r, pal, true, 2);
        break;

    case CE_ProgressBarLabel:
        {
#ifndef QT_NO_PROGRESSBAR
            const QProgressBar * pb = (const QProgressBar *) widget;
            const int unit_width = pixelMetric(PM_ProgressBarChunkWidth, pb);
            int u = r.width() / unit_width;
            int p_v = pb->progress();
            int t_s = pb->totalSteps();
            if (u > 0 && pb->progress() >= INT_MAX / u && t_s >= u) {
                // scale down to something usable.
                p_v /= u;
                t_s /= u;
            }
            if (pb->percentageVisible() && pb->totalSteps()) {
                int nu = (u * p_v + t_s/2) / t_s;
                int x = unit_width * nu;
                if (pb->indicatorFollowsStyle() || pb->centerIndicator()) {
                    p->setPen(pal.highlightedText());
                    p->setClipRect(r.x(), r.y(), x, r.height());
                    p->drawText(r, Qt::AlignCenter | Qt::TextSingleLine, pb->progressString());

                    if (pb->progress() != pb->totalSteps()) {
                        p->setClipRect(r.x() + x, r.y(), r.width() - x, r.height());
                        p->setPen(pal.highlight());
                        p->drawText(r, Qt::AlignCenter | Qt::TextSingleLine, pb->progressString());
                    }
                } else {
                    p->setPen(pal.text());
                    p->drawText(r, Qt::AlignCenter | Qt::TextSingleLine, pb->progressString());
                }
            }
#endif
            break;
        }

#ifndef QT_NO_MENU
    case CE_MenuTearoff: {
        if(flags & Style_Active) {
            if(pixelMetric(PM_MenuFrameWidth) > 1)
                qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(), pal, false, motifItemFrame,
                                 &pal.brush(QPalette::Button));
            else
                qDrawShadePanel(p, r.x()+1, r.y()+1, r.width()-2, r.height()-2, pal, true, 1, &pal.brush(QPalette::Button));
        } else {
            p->fillRect(r, pal.brush(QPalette::Button));
        }
        p->setPen(QPen(pal.dark(), 1, Qt::DashLine));
        p->drawLine(r.x()+2, r.y()+r.height()/2-1, r.x()+r.width()-4, r.y()+r.height()/2-1);
        p->setPen(QPen(pal.light(), 1, Qt::DashLine));
        p->drawLine(r.x()+2, r.y()+r.height()/2, r.x()+r.width()-4, r.y()+r.height()/2);
        break; }

    case CE_MenuItem: {
        if (!widget || opt.isDefault())
            break;

        const QMenu *menu = (const QMenu *) widget;
        QAction *mi = opt.action();
        if(!mi)
            break;

        int tab = opt.tabWidth();
        int maxpmw = opt.maxIconWidth();
        bool dis = ! (flags & Style_Enabled);
        bool checkable = menu->isCheckable();
        bool act = flags & Style_Active;

        int x, y, w, h;
        r.rect(&x, &y, &w, &h);

        if(checkable)
            maxpmw = qMax(maxpmw, motifCheckMarkSpace);

        int checkcol = maxpmw;
        if(mi && mi->isSeparator()) {                    // draw separator
            p->setPen(pal.dark());
            p->drawLine(x, y, x+w, y);
            p->setPen(pal.light());
            p->drawLine(x, y+1, x+w, y+1);
            return;
        }

        int pw = motifItemFrame;
        if(act && !dis) {                        // active item frame
            if(pixelMetric(PM_MenuFrameWidth) > 1)
                qDrawShadePanel(p, x, y, w, h, pal, false, pw,
                                 &pal.brush(QPalette::Button));
            else
                qDrawShadePanel(p, x+1, y+1, w-2, h-2, pal, true, 1,
                                 &pal.brush(QPalette::Button));
        } else  {                               // incognito frame
            p->fillRect(x, y, w, h, pal.brush(QPalette::Button));
        }

        if (!mi)
            return;

        QIcon is = mi->icon();
        QRect vrect = visualRect(QRect(x+motifItemFrame, y+motifItemFrame, checkcol, h-2*motifItemFrame), r);
        int xvis = vrect.x();
        if (mi->isChecked()) {
            if(!is.isNull())
                qDrawShadePanel(p, xvis, y+motifItemFrame, checkcol, h-2*motifItemFrame,
                                 pal, true, 1, &pal.brush(QPalette::Midlight));
        } else if (!act) {
            p->fillRect(xvis, y+motifItemFrame, checkcol, h-2*motifItemFrame,
                        pal.brush(QPalette::Button));
        }

        if(!is.isNull()) {              // draw icon
            QIcon::Mode mode = QIcon::Normal; // no disabled icons in Motif
            if (act && !dis)
                mode = QIcon::Active;
            QPixmap pixmap;
            if (checkable && mi->isChecked())
                pixmap = is.pixmap(Qt::SmallIconSize, mode, QIcon::On);
            else
                pixmap = is.pixmap(Qt::SmallIconSize, mode);

            int pixw = pixmap.width();
            int pixh = pixmap.height();
            QRect pmr(0, 0, pixw, pixh);
            pmr.moveCenter(vrect.center());
            p->setPen(pal.text());
            p->drawPixmap(pmr.topLeft(), pixmap);

        } else  if (checkable) {  // just "checking"...
            int mw = checkcol;
            int mh = h - 2*motifItemFrame;
            if (mi->isChecked()) {
                SFlags cflags = Style_Default;
                if (! dis)
                    cflags |= Style_Enabled;
                if (act)
                    cflags |= Style_On;

                drawPrimitive(PE_CheckMark, p,
                              QRect(xvis, y+motifItemFrame, mw, mh),
                              pal, cflags);
            }
        }


        p->setPen(pal.buttonText());

        QColor discol;
        if (dis) {
            discol = pal.text();
            p->setPen(discol);
        }

        int xm = motifItemFrame + checkcol + motifItemHMargin;

        vrect = visualRect(QRect(x+xm, y+motifItemVMargin, w-xm-tab, h-2*motifItemVMargin), r);
        xvis = vrect.x();

        QString s = mi->text();
        if (!s.isNull()) {                        // draw text
            int t = s.indexOf('\t');
            int m = motifItemVMargin;
            int text_flags = Qt::AlignVCenter|Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
            text_flags |= (QApplication::isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft);
            if (t >= 0) {                         // draw tab text
                QRect vr = visualRect(QRect(x+w-tab-motifItemHMargin-motifItemFrame,
                                              y+motifItemVMargin, tab, h-2*motifItemVMargin), r);
                int xv = vr.x();
                p->drawText(xv, y+m, tab, h-2*m, text_flags, s.mid(t+1));
                s = s.left(t);
            }
            p->drawText(xvis, y+m, w-xm-tab+1, h-2*m, text_flags, s, t);
        }
        if (mi->menu()) {                        // draw sub menu arrow
            int dim = (h-2*motifItemFrame) / 2;
            QStyle::PrimitiveElement arrow = (QApplication::isRightToLeft() ? PE_ArrowLeft : PE_ArrowRight);
            QRect vr = visualRect(QRect(x+w - motifArrowHMargin - motifItemFrame - dim,
                                         y+h/2-dim/2, dim, dim), r);
            if (act)
                drawPrimitive(arrow, p, vr, pal, (Style_Down | (dis ? Style_Default : Style_Enabled)));
            else
                drawPrimitive(arrow, p, vr, pal, (dis ? Style_Default : Style_Enabled));
        }

        break;  }
#endif // QT_NO_MENU


#ifdef QT_COMPAT
#ifndef QT_NO_POPUPMENU
    case CE_Q3PopupMenuItem:
        {
            if (! widget || opt.isDefault())
                break;

            const Q3PopupMenu *popupmenu = (const Q3PopupMenu *) widget;
            Q3MenuItem *mi = opt.menuItem();
            if (!mi)
                break;

            int tab = opt.tabWidth();
            int maxpmw = opt.maxIconWidth();
            bool dis = ! (flags & Style_Enabled);
            bool checkable = popupmenu->isCheckable();
            bool act = flags & Style_Active;
            int x, y, w, h;

            r.rect(&x, &y, &w, &h);

            if (checkable)
                maxpmw = qMax(maxpmw, motifCheckMarkSpace);

            int checkcol = maxpmw;

            if (mi && mi->isSeparator()) {                    // draw separator
                p->setPen(pal.dark());
                p->drawLine(x, y, x+w, y);
                p->setPen(pal.light());
                p->drawLine(x, y+1, x+w, y+1);
                return;
            }

            int pw = motifItemFrame;

            if (act && !dis) {                        // active item frame
                if (pixelMetric(PM_DefaultFrameWidth) > 1)
                    qDrawShadePanel(p, x, y, w, h, pal, false, pw,
                                     &pal.brush(QPalette::Button));
                else
                    qDrawShadePanel(p, x+1, y+1, w-2, h-2, pal, true, 1,
                                     &pal.brush(QPalette::Button));
            }
            else                                // incognito frame
                p->fillRect(x, y, w, h, pal.brush(QPalette::Button));

            if (!mi)
                return;

            QRect vrect = visualRect(QRect(x+motifItemFrame, y+motifItemFrame, checkcol, h-2*motifItemFrame), r);
            int xvis = vrect.x();
            if (mi->isChecked()) {
                if (mi->iconSet()) {
                    qDrawShadePanel(p, xvis, y+motifItemFrame, checkcol, h-2*motifItemFrame,
                                     pal, true, 1, &pal.brush(QPalette::Midlight));
                }
            } else if (!act) {
                p->fillRect(xvis, y+motifItemFrame, checkcol, h-2*motifItemFrame,
                            pal.brush(QPalette::Button));
            }

            if (mi->iconSet()) {              // draw icon
                QIcon::Mode mode = QIcon::Normal; // no disabled icons in Motif
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap;
                if (checkable && mi->isChecked())
                    pixmap = mi->iconSet()->pixmap(Qt::SmallIconSize, mode, QIcon::On);
                else
                    pixmap = mi->iconSet()->pixmap(Qt::SmallIconSize, mode);

                int pixw = pixmap.width();
                int pixh = pixmap.height();
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(vrect.center());
                p->setPen(pal.text());
                p->drawPixmap(pmr.topLeft(), pixmap);

            } else  if (checkable) {  // just "checking"...
                int mw = checkcol;
                int mh = h - 2*motifItemFrame;
                if (mi->isChecked()) {
                    SFlags cflags = Style_Default;
                    if (! dis)
                        cflags |= Style_Enabled;
                    if (act)
                        cflags |= Style_On;

                    drawPrimitive(PE_CheckMark, p,
                                  QRect(xvis, y+motifItemFrame, mw, mh),
                                  pal, cflags);
                }
            }


            p->setPen(pal.buttonText());

            QColor discol;
            if (dis) {
                discol = pal.text();
                p->setPen(discol);
            }

            int xm = motifItemFrame + checkcol + motifItemHMargin;

            vrect = visualRect(QRect(x+xm, y+motifItemVMargin, w-xm-tab, h-2*motifItemVMargin), r);
            xvis = vrect.x();
            if (mi->custom()) {
                int m = motifItemVMargin;
                p->save();
                mi->custom()->paint(p, pal, act, !dis,
                                     xvis, y+m, w-xm-tab+1, h-2*m);
                p->restore();
            }
            QString s = mi->text();
            if (!s.isNull()) {                        // draw text
                int t = s.indexOf('\t');
                int m = motifItemVMargin;
                int text_flags = Qt::AlignVCenter|Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                text_flags |= (QApplication::isRightToLeft() ? Qt::AlignRight : Qt::AlignLeft);
                if (t >= 0) {                         // draw tab text
                    QRect vr = visualRect(QRect(x+w-tab-motifItemHMargin-motifItemFrame,
                                                  y+motifItemVMargin, tab, h-2*motifItemVMargin), r);
                    int xv = vr.x();
                    p->drawText(xv, y+m, tab, h-2*m, text_flags, s.mid(t+1));
                    s = s.left(t);
                }
                p->drawText(xvis, y+m, w-xm-tab+1, h-2*m, text_flags, s, t);
            } else if (mi->pixmap()) {                        // draw pixmap
                QPixmap *pixmap = mi->pixmap();
                if (pixmap->depth() == 1)
                    p->setBackgroundMode(Qt::OpaqueMode);
                p->drawPixmap(xvis, y+motifItemFrame, *pixmap);
                if (pixmap->depth() == 1)
                    p->setBackgroundMode(Qt::TransparentMode);
            }
            if (mi->popup()) {                        // draw sub menu arrow
                int dim = (h-2*motifItemFrame) / 2;
                QStyle::PrimitiveElement arrow = (QApplication::isRightToLeft() ? PE_ArrowLeft : PE_ArrowRight);
                QRect vr = visualRect(QRect(x+w - motifArrowHMargin - motifItemFrame - dim,
                                        y+h/2-dim/2, dim, dim), r);
                if (act)
                    drawPrimitive(arrow, p, vr, pal,
                                  (Style_Down |
                                  (dis ? Style_Default : Style_Enabled)));
                else
                    drawPrimitive(arrow, p, vr, pal,
                                  (dis ? Style_Default : Style_Enabled));
            }

            break;
        }
#endif // QT_NO_POPUPMENU
#endif

    case CE_MenuBarItem:
        {
            if (flags & Style_Active)  // active item
                qDrawShadePanel(p, r, pal, false, motifItemFrame,
                                 &pal.brush(QPalette::Button));
            else  // other item
                p->fillRect(r, pal.brush(QPalette::Button));
            QCommonStyle::drawControl(element, p, widget, r, pal, flags, opt);
            break;
        }

#ifdef QT_COMPAT
    case CE_Q3MenuBarItem:
        {
            if (flags & Style_Active)  // active item
                qDrawShadePanel(p, r, pal, false, motifItemFrame,
                                 &pal.brush(QPalette::Button));
            else  // other item
                p->fillRect(r, pal.brush(QPalette::Button));
            QCommonStyle::drawControl(element, p, widget, r, pal, flags, opt);
            break;
        }
#endif

    default:
        QCommonStyle::drawControl(element, p, widget, r, pal, flags, opt);
        break;
    }
}

static int get_combo_extra_width(int h, int w, int *return_awh=0)
{
    int awh,
        tmp;
    if (h < 8) {
        awh = 6;
    } else if (h < 14) {
        awh = h - 2;
    } else {
        awh = h/2;
    }
    tmp = (awh * 3) / 2;
    if (tmp > w / 2) {
        awh = w / 2 - 3;
        tmp = w / 2 + 3;
    }

    if (return_awh)
        *return_awh = awh;

    return tmp;
}

static void get_combo_parameters(const QRect &r,
                                  int &ew, int &awh, int &ax,
                                  int &ay, int &sh, int &dh,
                                  int &sy)
{
    ew = get_combo_extra_width(r.height(), r.width(), &awh);

    sh = (awh+3)/4;
    if (sh < 3)
        sh = 3;
    dh = sh/2 + 1;

    ay = r.y() + (r.height()-awh-sh-dh)/2;
    if (ay < 0) {
        //panic mode
        ay = 0;
        sy = r.height();
    } else {
        sy = ay+awh+dh;
    }
    ax = r.x() + r.width() - ew;
    ax  += (ew-awh)/2;
}

/*!
    \reimp
*/
void QMotifStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                    const QWidget *w) const
        /*
void QMotifStyle::drawComplexControl(ComplexControl control,
                                     QPainter *p,
                                     const QWidget *widget,
                                     const QRect &r,
                                     const QPalette &pal,
                                     SFlags flags,
                                     SCFlags sub,
                                     SCFlags subActive,
                                     const Q3StyleOption& opt) const
                                     */
{
    switch (control) {
    case CC_SpinBox: {
        SCFlags drawSub = SC_None;
        if (sub & SC_SpinBoxFrame)
            qDrawShadePanel(p, r, pal, true,
                             pixelMetric(PM_DefaultFrameWidth));

        if (sub & SC_SpinBoxUp || sub & SC_SpinBoxDown) {
            if (sub & SC_SpinBoxUp)
                drawSub |= SC_SpinBoxUp;
            if (sub & SC_SpinBoxDown)
                drawSub |= SC_SpinBoxDown;

            QCommonStyle::drawComplexControl(control, p, widget, r, pal, flags,
                                              drawSub, subActive, opt);
        }
        break; }

    case CC_Slider:
        {
#ifndef QT_NO_SLIDER
            const QSlider * slider = static_cast<const QSlider *>(widget);

            QRect groove = querySubControlMetrics(CC_Slider, widget, SC_SliderGroove, opt),
                  handle = querySubControlMetrics(CC_Slider, widget, SC_SliderHandle, opt);

            if ((sub & SC_SliderGroove) && groove.isValid()) {
                qDrawShadePanel(p, groove, pal, true, 2, &pal.brush(QPalette::Mid));
                if (flags & Style_HasFocus) {
                    QRect fr = subRect(SR_SliderFocusRect, widget);
                    drawPrimitive(PE_FocusRect, p, fr, pal);
                }
            }

            if ((sub & SC_SliderHandle) && handle.isValid()) {
                drawPrimitive(PE_ButtonBevel, p, handle, pal);

                if (slider->orientation() == Qt::Horizontal) {
                    QCOORD mid = handle.x() + handle.width() / 2;
                    qDrawShadeLine(p, mid, handle.y(), mid, handle.y() + handle.height() - 2,
                                   pal, true, 1);
                } else {
                    QCOORD mid = handle.y() + handle.height() / 2;
                    qDrawShadeLine(p, handle.x(), mid, handle.x() + handle.width() - 2, mid, pal,
                                   true, 1);
                }
            }

            if (sub & SC_SliderTickmarks)
                QCommonStyle::drawComplexControl(control, p, widget, r, pal, flags,
                                                 SC_SliderTickmarks, subActive,
                                                 opt);
#endif
            break;
        }

    case CC_ComboBox:
#ifndef QT_NO_COMBOBOX
        if (sub & SC_ComboBoxArrow) {
            const QComboBox * cb = (const QComboBox *) widget;
            int awh, ax, ay, sh, sy, dh, ew;
            int fw = pixelMetric(PM_DefaultFrameWidth, cb);

            drawPrimitive(PE_ButtonCommand, p, r, pal, flags);
            QRect ar = QStyle::visualRect(querySubControlMetrics(CC_ComboBox, cb, SC_ComboBoxArrow,
                                                                   opt), cb);
            drawPrimitive(PE_ArrowDown, p, ar, pal, flags | Style_Enabled);

            QRect tr = r;
            tr.addCoords(fw, fw, -fw, -fw);
            get_combo_parameters(tr, ew, awh, ax, ay, sh, dh, sy);

            // draws the shaded line under the arrow
            p->setPen(pal.light());
            p->drawLine(ar.x(), sy, ar.x()+awh-1, sy);
            p->drawLine(ar.x(), sy, ar.x(), sy+sh-1);
            p->setPen(pal.dark());
            p->drawLine(ar.x()+1, sy+sh-1, ar.x()+awh-1, sy+sh-1);
            p->drawLine(ar.x()+awh-1, sy+1, ar.x()+awh-1, sy+sh-1);

            if (cb->hasFocus()) {
                QRect re = QStyle::visualRect(subRect(SR_ComboBoxFocusRect, cb), cb);
                drawPrimitive(PE_FocusRect, p, re, pal);
            }
        }

        if (sub & SC_ComboBoxEditField) {
            QComboBox * cb = (QComboBox *) widget;
            if (cb->editable()) {
                QRect er = QStyle::visualRect(querySubControlMetrics(CC_ComboBox, cb,
                                                                       SC_ComboBoxEditField), cb);
                er.addCoords(-1, -1, 1, 1);
                qDrawShadePanel(p, er, pal, true, 1,
                                 &pal.brush(QPalette::Button));
            }
        }
#endif
        p->setPen(pal.buttonText());
        break;

    case CC_ScrollBar:
        {
            if (sub & SC_ScrollBarGroove)
                qDrawShadePanel(p, widget->rect(), pal, true,
                               pixelMetric(PM_DefaultFrameWidth, widget),
                               &pal.brush(QPalette::Mid));
            QCommonStyle::drawComplexControl(control, p, widget, r, pal, flags, sub,
                                             subActive, opt);
            break;
        }

#ifndef QT_NO_LISTVIEW
    case CC_ListView:
        {
            if (sub & SC_ListView) {
                QCommonStyle::drawComplexControl(control, p, widget, r, pal, flags, sub, subActive, opt);
            }
            if (sub & (SC_ListViewBranch | SC_ListViewExpand)) {
                if (opt.isDefault())
                    break;

                QListViewItem *item = opt.listViewItem();
                QListViewItem *child = item->firstChild();

                int y = r.y();
                int c;
                QPointArray dotlines;
                if (subActive == SC_All && sub == SC_ListViewExpand) {
                    c = 2;
                    dotlines.resize(2);
                    dotlines[0] = QPoint(r.right(), r.top());
                    dotlines[1] = QPoint(r.right(), r.bottom());
                } else {
                    int linetop = 0, linebot = 0;
                    // each branch needs at most two lines, ie. four end points
                    dotlines.resize(item->childCount() * 4);
                    c = 0;

                    // skip the stuff above the exposed rectangle
                    while (child && y + child->height() <= 0) {
                        y += child->totalHeight();
                        child = child->nextSibling();
                    }

                    int bx = r.width() / 2;

                    // paint stuff in the magical area
                    QListView* v = item->listView();
                    while (child && y < r.height()) {
                        if (child->isVisible()) {
                            int lh;
                            if (!item->multiLinesEnabled())
                                lh = child->height();
                            else
                                lh = p->fontMetrics().height() + 2 * v->itemMargin();
                            lh = qMax(lh, QApplication::globalStrut().height());
                            if (lh % 2 > 0)
                                lh++;
                            linebot = y + lh/2;
                            if ((child->isExpandable() || child->childCount()) &&
                                 (child->height() > 0)) {
                                // needs a box
                                p->setPen(pal.text());
                                p->drawRect(bx-4, linebot-4, 9, 9);
                                QPointArray a;
                                if (child->isOpen())
                                    a.setPoints(3, bx-2, linebot-2,
                                                 bx, linebot+2,
                                                 bx+2, linebot-2); //Qt::RightArrow
                                else
                                    a.setPoints(3, bx-2, linebot-2,
                                                 bx+2, linebot,
                                                 bx-2, linebot+2); //Qt::DownArrow
                                p->setBrush(pal.text());
                                p->drawPolygon(a);
                                p->setBrush(Qt::NoBrush);
                                // dotlinery
                                dotlines[c++] = QPoint(bx, linetop);
                                dotlines[c++] = QPoint(bx, linebot - 5);
                                dotlines[c++] = QPoint(bx + 5, linebot);
                                dotlines[c++] = QPoint(r.width(), linebot);
                                linetop = linebot + 5;
                            } else {
                                // just dotlinery
                                dotlines[c++] = QPoint(bx+1, linebot);
                                dotlines[c++] = QPoint(r.width(), linebot);
                            }
                            y += child->totalHeight();
                        }
                        child = child->nextSibling();
                    }

                    // Expand line height to edge of rectangle if there's any
                    // visible child below
                    while (child && child->height() <= 0)
                        child = child->nextSibling();
                    if (child)
                        linebot = r.height();

                    if (linetop < linebot) {
                        dotlines[c++] = QPoint(bx, linetop);
                        dotlines[c++] = QPoint(bx, linebot);
                    }
                }

                int line; // index into dotlines
                p->setPen(pal.text());
                if (sub & SC_ListViewBranch) for(line = 0; line < c; line += 2) {
                    p->drawLine(dotlines[line].x(), dotlines[line].y(),
                                 dotlines[line+1].x(), dotlines[line+1].y());
                }
            }

            break;
        }
#endif // QT_NO_LISTVIEW

    default:
        QCommonStyle::drawComplexControl(control, p, widget, r, pal, flags,
                                          sub, subActive, opt);
    }
}


/*! \reimp */
int QMotifStyle::pixelMetric(PixelMetric pm, const QStyleOption *option,
                             const QWidget *widget) const
/*
int QMotifStyle::pixelMetric(PixelMetric metric, const QStyleOption *option,
                             const QWidget *widget) const
                             */
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

    case PM_SplitterWidth:
        ret = qMax(10, QApplication::globalStrut().width());
        break;

    case PM_SliderLength:
        ret = 30;
        break;

    case PM_SliderThickness:
        ret = 24;
        break;

    case PM_SliderControlThickness:
        {
#ifndef QT_NO_SLIDER
            const QSlider * sl = (const QSlider *) widget;
            int space = (sl->orientation() == Qt::Horizontal) ? sl->height()
                        : sl->width();
            int ticks = sl->tickmarks();
            int n = 0;
            if (ticks & QSlider::Above) n++;
            if (ticks & QSlider::Below) n++;
            if (!n) {
                ret = space;
                break;
            }

            int thick = 6;        // Magic constant to get 5 + 16 + 5

            space -= thick;
            //### the two sides may be unequal in size
            if (space > 0)
                thick += (space * 2) / (n + 2);
            ret = thick;
#endif
            break;
        }

    case PM_SliderSpaceAvailable:
        {
#ifndef QT_NO_SLIDER
            const QSlider * sl = (const QSlider *) widget;
            if (sl->orientation() == Qt::Horizontal)
                ret = sl->width() - pixelMetric(PM_SliderLength, sl) - 6;
            else
                ret = sl->height() - pixelMetric(PM_SliderLength, sl) - 6;
#endif
            break;
        }

    case PM_DockWindowHandleExtent:
        ret = 9;
        break;

    case PM_ProgressBarChunkWidth:
        ret = 1;
        break;

    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
        ret = 13;
        break;

    default:
        ret =  QCommonStyle::pixelMetric(metric, option, widget);
        break;
    }
    return ret;
}


/*!
    \reimp
*/
QRect QMotifStyle::querySubControlMetrics(ComplexControl cc, const QStyleOptionComplex *opt,
                                 SubControl sc, const QWidget *widget) const
/*
QRect QMotifStyle::querySubControlMetrics(ComplexControl control,
                                           const QWidget *widget,
                                           SubControl sc,
                                           const Q3StyleOption& opt) const
                                           */
{
    switch (control) {
    case CC_SpinBox: {
        if (!widget)
            return QRect();
        int fw = pixelMetric(PM_SpinBoxFrameWidth, 0);
        QSize bs;
        bs.setHeight(widget->height()/2);
        if (bs.height() < 8)
            bs.setHeight(8);
        bs.setWidth(qMin(bs.height() * 8 / 5, widget->width() / 4)); // 1.6 -approximate golden mean
        bs = bs.expandedTo(QApplication::globalStrut());
        int y = 0;
        int x, lx, rx;
        x = widget->width() - y - bs.width();
        lx = fw;
        rx = x - fw * 2;
        switch (sc) {
        case SC_SpinBoxUp:
            return QRect(x, y, bs.width(), bs.height());
        case SC_SpinBoxDown:
            return QRect(x, y + bs.height(), bs.width(), bs.height());
        case SC_SpinBoxButtonField:
            return QRect(x, y, bs.width(), widget->height() - 2*fw);
        case SC_SpinBoxEditField:
            return QRect(lx, fw, rx, widget->height() - 2*fw);
        case SC_SpinBoxFrame:
            return QRect(0, 0,
                          widget->width() - bs.width(), widget->height());
        default:
            break;
        }
        break; }

#ifndef QT_NO_SLIDER
    case CC_Slider: {
        if (sc == SC_SliderHandle) {
            const QSlider *sl = static_cast<const QSlider *>(widget);
            int tickOffset = pixelMetric(PM_SliderTickmarkOffset, sl);
            int thickness = pixelMetric(PM_SliderControlThickness, sl);
            bool horizontal = sl->orientation() == Qt::Horizontal;
            int len = pixelMetric(PM_SliderLength, sl);
            int motifBorder = 3;
            int sliderPos = positionFromValue(sl->minimum(), sl->maximum(), sl->sliderPosition(),
                                              horizontal ? sl->width() - len - 2 * motifBorder
                                                         : sl->height() - len - 2 * motifBorder,
                                              (!horizontal == !sl->invertedAppearance()));
            if (horizontal)
                return QRect(sliderPos + motifBorder, tickOffset + motifBorder, len,
                             thickness - 2 * motifBorder);
            return QRect(tickOffset + motifBorder, sliderPos + motifBorder,
                         thickness - 2 * motifBorder, len);
        }
        break; }
#endif

#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar: {
        if (! widget)
            return QRect();

        const QScrollBar *scrollbar = (const QScrollBar *) widget;
        int sbextent = pixelMetric(PM_ScrollBarExtent, widget);
        int fw = pixelMetric(PM_DefaultFrameWidth, widget);
        int buttonw = sbextent - (fw * 2);
        int buttonh = sbextent - (fw * 2);
        int maxlen = ((scrollbar->orientation() == Qt::Horizontal) ?
                      scrollbar->width() : scrollbar->height()) -
                     (buttonw * 2) - (fw * 2);
        int sliderlen;

        // calculate slider length
        if (scrollbar->maximum() != scrollbar->minimum()) {
            uint range = scrollbar->maximum() - scrollbar->minimum();
            sliderlen = (scrollbar->pageStep() * maxlen) /
                        (range + scrollbar->pageStep());

            if (sliderlen < 9 || range > INT_MAX/2)
                sliderlen = 9;
            if (sliderlen > maxlen)
                sliderlen = maxlen;
        } else
            sliderlen = maxlen;

        int sliderstart = sbextent + positionFromValue(scrollbar->minimum(), scrollbar->maximum(),
                                                       scrollbar->sliderPosition(),
                                                       maxlen - sliderlen,
                                                       scrollbar->invertedAppearance());

        switch (sc) {
        case SC_ScrollBarSubLine:
            // top/left button
            if (scrollbar->orientation() == Qt::Horizontal) {
                if (scrollbar->width()/2 < sbextent)
                    buttonw = scrollbar->width()/2 - (fw*2);
                return QRect(fw, fw, buttonw, buttonh);
            } else {
                if (scrollbar->height()/2 < sbextent)
                    buttonh = scrollbar->height()/2 - (fw*2);
                return QRect(fw, fw, buttonw, buttonh);
            }
        case SC_ScrollBarAddLine:
            // bottom/right button
            if (scrollbar->orientation() == Qt::Horizontal) {
                if (scrollbar->width()/2 < sbextent)
                    buttonw = scrollbar->width()/2 - (fw*2);
                return QRect(scrollbar->width() - buttonw - fw, fw,
                             buttonw, buttonh);
            } else {
                if (scrollbar->height()/2 < sbextent)
                    buttonh = scrollbar->height()/2 - (fw*2);
                return QRect(fw, scrollbar->height() - buttonh - fw,
                             buttonw, buttonh);
            }
        case SC_ScrollBarSubPage:
            if (scrollbar->orientation() == Qt::Horizontal)
                return QRect(buttonw + fw, fw, sliderstart - buttonw - fw, buttonw);
            return QRect(fw, buttonw + fw, buttonw, sliderstart - buttonw - fw);

        case SC_ScrollBarAddPage:
            if (scrollbar->orientation() == Qt::Horizontal)
                return QRect(sliderstart + sliderlen, fw,
                             maxlen - sliderstart - sliderlen + buttonw + fw, buttonw);
            return QRect(fw, sliderstart + sliderlen, buttonw,
                         maxlen - sliderstart - sliderlen + buttonw + fw);

        case SC_ScrollBarGroove:
            if (scrollbar->orientation() == Qt::Horizontal)
                return QRect(buttonw + fw, fw, maxlen, buttonw);
            return QRect(fw, buttonw + fw, buttonw, maxlen);

        case SC_ScrollBarSlider:
            if (scrollbar->orientation() == Qt::Horizontal)
                return QRect(sliderstart, fw, sliderlen, buttonw);
            return QRect(fw, sliderstart, buttonw, sliderlen);

        default:
            break;
        }
        break; }
#endif

#ifndef QT_NO_COMBOBOX
    case CC_ComboBox:

        switch (sc) {
        case SC_ComboBoxArrow: {
            const QComboBox * cb = (const QComboBox *) widget;
            int ew, awh, sh, dh, ax, ay, sy;
            int fw = pixelMetric(PM_DefaultFrameWidth, cb);
            QRect cr = cb->rect();
            cr.addCoords(fw, fw, -fw, -fw);
            get_combo_parameters(cr, ew, awh, ax, ay, sh, dh, sy);
            return QRect(ax, ay, awh, awh); }

        case SC_ComboBoxEditField: {
            const QComboBox * cb = (const QComboBox *) widget;
            int fw = pixelMetric(PM_DefaultFrameWidth, cb);
            QRect rect = cb->rect();
            rect.addCoords(fw, fw, -fw, -fw);
            int ew = get_combo_extra_width(rect.height(), rect.width());
            rect.addCoords(1, 1, -1-ew, -1);
            return rect; }

        default:
            break;
        }
        break;
#endif
    default: break;
    }
    return QCommonStyle::querySubControlMetrics(control, widget, sc, opt);
}

/*!
    \reimp
*/
QSize QMotifStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt,
                           const QSize &contentsSize, const QWidget *widget) const
/*
QSize QMotifStyle::sizeFromContents(ContentsType contents,
                                     const QWidget *widget,
                                     const QSize &contentsSize,
                                     const Q3StyleOption& opt) const
                                     */
{
    QSize sz(contentsSize);

    switch(contents) {
    case CT_PushButton:
        {
#ifndef QT_NO_PUSHBUTTON
            const QPushButton *button = (const QPushButton *) widget;
            sz = QCommonStyle::sizeFromContents(contents, widget, contentsSize, opt);
            if ((button->isDefault() || button->autoDefault()) &&
                sz.width() < 80 && ! button->pixmap())
                sz.setWidth(80);
#endif
            break;
        }

    case CT_MenuBarItem: {
        if(!sz.isEmpty())
            sz = QSize(sz.width()+(motifItemVMargin*2), sz.height()+(motifItemHMargin*2));
        break; }

    case CT_MenuItem:
        {
#ifndef QT_NO_MENU
            if(!widget || opt.isDefault())
                break;

            const QMenu *menu = (QMenu *) widget;
            bool checkable = menu->isCheckable();
            QAction *mi = opt.action();
            int maxpmw = opt.maxIconWidth();
            int w = sz.width(), h = sz.height();

            if (mi->isSeparator()) {
                w = 10;
                h = motifSepHeight;
            } else if (!mi->icon().isNull() || !mi->text().isNull()) {
                h += 2*motifItemVMargin + 2*motifItemFrame;
            }

            // a little bit of border can never harm
            w += 2*motifItemHMargin + 2*motifItemFrame;

            if (!mi->text().isNull() && mi->text().indexOf('\t') >= 0)
                // string contains tab
                w += motifTabSpacing;
            else if (mi->menu())
                // submenu indicator needs some room if we don't have a tab column
                w += motifArrowHMargin + 4*motifItemFrame;

            if (checkable && maxpmw <= 0)
                // if we are checkable and have no icons, add space for a checkmark
                w += motifCheckMarkSpace;
            else if (checkable && maxpmw < motifCheckMarkSpace)
                // make sure the check-column is wide enough if we have icons
                w += (motifCheckMarkSpace - maxpmw);

            // if we have a check-column (icons of checkmarks), add space
            // to separate the columns
            if (maxpmw > 0 || checkable)
                w += motifCheckMarkHMargin;

            sz = QSize(w, h);
#endif
            break;
        }

#ifdef QT_COMPAT
    case CT_Q3PopupMenuItem:
        {
#ifndef QT_NO_POPUPMENU
            if (! widget || opt.isDefault())
                break;

            const Q3PopupMenu *popup = (Q3PopupMenu *) widget;
            bool checkable = popup->isCheckable();
            Q3MenuItem *mi = opt.menuItem();
            int maxpmw = opt.maxIconWidth();
            int w = sz.width(), h = sz.height();

            if (mi->custom()) {
                w = mi->custom()->sizeHint().width();
                h = mi->custom()->sizeHint().height();
                if (! mi->custom()->fullSpan())
                    h += 2*motifItemVMargin + 2*motifItemFrame;
            } else if (mi->widget()) {
            } else if (mi->isSeparator()) {
                w = 10;
                h = motifSepHeight;
            } else if (mi->pixmap() || ! mi->text().isNull())
                h += 2*motifItemVMargin + 2*motifItemFrame;

            // a little bit of border can never harm
            w += 2*motifItemHMargin + 2*motifItemFrame;

            if (!mi->text().isNull() && mi->text().indexOf('\t') >= 0)
                // string contains tab
                w += motifTabSpacing;
            else if (mi->popup())
                // submenu indicator needs some room if we don't have a tab column
                w += motifArrowHMargin + 4*motifItemFrame;

            if (checkable && maxpmw <= 0)
                // if we are checkable and have no icons, add space for a checkmark
                w += motifCheckMarkSpace;
            else if (checkable && maxpmw < motifCheckMarkSpace)
                // make sure the check-column is wide enough if we have icons
                w += (motifCheckMarkSpace - maxpmw);

            // if we have a check-column (icons of checkmarks), add space
            // to separate the columns
            if (maxpmw > 0 || checkable)
                w += motifCheckMarkHMargin;

            sz = QSize(w, h);
#endif
            break;
        }
#endif

    default:
        sz = QCommonStyle::sizeFromContents(contents, widget, contentsSize, opt);
        break;
    }

    return sz;
}

/*!
    \reimp
*/
QRect QMotifStyle::subRect(SubRect r, const QStyleOption *opt, const QWidget *widget) const
//QRect QMotifStyle::subRect(SubRect r, const QWidget *widget) const
{
    QRect rect;
    QRect wrect = widget->rect();

    switch (r) {
    case SR_SliderFocusRect:
        rect = QCommonStyle::subRect(r, widget);
        rect.addCoords(2, 2, -2, -2);
        break;

    case SR_ComboBoxFocusRect:
        {
            int awh, ax, ay, sh, sy, dh, ew;
            int fw = pixelMetric(PM_DefaultFrameWidth, widget);
            QRect tr = wrect;

            tr.addCoords(fw, fw, -fw, -fw);
            get_combo_parameters(tr, ew, awh, ax, ay, sh, dh, sy);
            rect.setRect(ax-2, ay-2, awh+4, awh+sh+dh+4);
            break;
        }

    case SR_DockWindowHandleRect:
        {
#ifndef QT_NO_MAINWINDOW
            if (!widget || !widget->parent())
                break;

            const QDockWindow * dw = (const QDockWindow *) widget->parent();
            if (!dw->area() || !dw->isCloseEnabled())
                rect.setRect(0, 0, widget->width(), widget->height());
            else {
                if (dw->area()->orientation() == Qt::Horizontal)
                    rect.setRect(2, 15, widget->width()-2, widget->height() - 15);
                else
                    rect.setRect(0, 2, widget->width() - 15, widget->height() - 2);
            }
#endif
            break;
        }

    case SR_ProgressBarGroove:
    case SR_ProgressBarContents:
        {
#ifndef QT_NO_PROGRESSBAR
            QFontMetrics fm((widget ? widget->fontMetrics() :
                               QApplication::fontMetrics()));
            const QProgressBar *progressbar = (const QProgressBar *) widget;
            int textw = 0;
            if (progressbar->percentageVisible())
                textw = fm.width("100%") + 6;

            if (progressbar->indicatorFollowsStyle() ||
                progressbar->centerIndicator())
                rect = wrect;
            else
                rect.setCoords(wrect.left(), wrect.top(),
                               wrect.right() - textw, wrect.bottom());
#endif
            break;
        }

    case SR_ProgressBarLabel:
        {
#ifndef QT_NO_PROGRESSBAR
            QFontMetrics fm((widget ? widget->fontMetrics() :
                               QApplication::fontMetrics()));
            const QProgressBar *progressbar = (const QProgressBar *) widget;
            int textw = 0;
            if (progressbar->percentageVisible())
                textw = fm.width("100%") + 6;

            if (progressbar->indicatorFollowsStyle() ||
                progressbar->centerIndicator())
                rect = wrect;
            else
                rect.setCoords(wrect.right() - textw, wrect.top(),
                               wrect.right(), wrect.bottom());
#endif
            break;
        }

    case SR_CheckBoxContents:
        {
#ifndef QT_NO_CHECKBOX
            QRect ir = subRect(SR_CheckBoxIndicator, widget);
            rect.setRect(ir.right() + 10, wrect.y(),
                         wrect.width() - ir.width() - 10, wrect.height());
#endif
            break;
        }

    case SR_RadioButtonContents:
        {
            QRect ir = subRect(SR_RadioButtonIndicator, widget);
            rect.setRect(ir.right() + 10, wrect.y(),
                         wrect.width() - ir.width() - 10, wrect.height());
            break;
        }

    default:
        rect = QCommonStyle::subRect(r, widget);
    }

    return rect;
}

#ifndef QT_NO_IMAGEIO_XPM
static const char * const qt_close_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"   .    .   ",
"  ...  ...  ",
"   ......   ",
"    ....    ",
"    ....    ",
"   ......   ",
"  ...  ...  ",
"   .    .   ",
"            ",
"            "};

static const char * const qt_maximize_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"            ",
"     .      ",
"    ...     ",
"   .....    ",
"  .......   ",
" .........  ",
"            ",
"            ",
"            ",
"            "};

static const char * const qt_minimize_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"            ",
"            ",
" .........  ",
"  .......   ",
"   .....    ",
"    ...     ",
"     .      ",
"            ",
"            ",
"            "};

#if 0 // ### not used???
static const char * const qt_normalize_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"  .         ",
"  ..        ",
"  ...       ",
"  ....      ",
"  .....     ",
"  ......    ",
"  .......   ",
"            ",
"            ",
"            "};
#endif

static const char * const qt_normalizeup_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"            ",
"  .......   ",
"   ......   ",
"    .....   ",
"     ....   ",
"      ...   ",
"       ..   ",
"        .   ",
"            ",
"            "};

static const char * const qt_shade_xpm[] = {
"12 12 2 1", "# c #000000",
". c None",
"............",
"............",
".#########..",
".#########..",
"............",
"............",
"............",
"............",
"............",
"............",
"............",
"............"};


static const char * const qt_unshade_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
".#########..",
".#########..",
".#.......#..",
".#.......#..",
".#.......#..",
".#.......#..",
".#.......#..",
".#########..",
"............",
"............"};


static const char * dock_window_close_xpm[] = {
"8 8 2 1",
"# c #000000",
". c None",
"##....##",
".##..##.",
"..####..",
"...##...",
"..####..",
".##..##.",
"##....##",
"........"};

// Message box icons, from page 210 of the Windows style guide.

// Hand-drawn to resemble Microsoft's icons, but in the Mac/Netscape palette.
// Thanks to TrueColor displays, it is slightly more efficient to have
// them duplicated.
/* XPM */
static const char * const information_xpm[]={
"32 32 5 1",
". c None",
"c c #000000",
"* c #999999",
"a c #ffffff",
"b c #0000ff",
"...........********.............",
"........***aaaaaaaa***..........",
"......**aaaaaaaaaaaaaa**........",
".....*aaaaaaaaaaaaaaaaaa*.......",
"....*aaaaaaaabbbbaaaaaaaac......",
"...*aaaaaaaabbbbbbaaaaaaaac.....",
"..*aaaaaaaaabbbbbbaaaaaaaaac....",
".*aaaaaaaaaaabbbbaaaaaaaaaaac...",
".*aaaaaaaaaaaaaaaaaaaaaaaaaac*..",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaaac*.",
"*aaaaaaaaaabbbbbbbaaaaaaaaaaac*.",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
".*aaaaaaaaaaabbbbbaaaaaaaaaac***",
".*aaaaaaaaaaabbbbbaaaaaaaaaac***",
"..*aaaaaaaaaabbbbbaaaaaaaaac***.",
"...caaaaaaabbbbbbbbbaaaaaac****.",
"....caaaaaaaaaaaaaaaaaaaac****..",
".....caaaaaaaaaaaaaaaaaac****...",
"......ccaaaaaaaaaaaaaacc****....",
".......*cccaaaaaaaaccc*****.....",
"........***cccaaaac*******......",
"..........****caaac*****........",
".............*caaac**...........",
"...............caac**...........",
"................cac**...........",
".................cc**...........",
"..................***...........",
"...................**..........."};
/* XPM */
static const char* const warning_xpm[]={
"32 32 4 1",
". c None",
"a c #ffff00",
"* c #000000",
"b c #999999",
".............***................",
"............*aaa*...............",
"...........*aaaaa*b.............",
"...........*aaaaa*bb............",
"..........*aaaaaaa*bb...........",
"..........*aaaaaaa*bb...........",
".........*aaaaaaaaa*bb..........",
".........*aaaaaaaaa*bb..........",
"........*aaaaaaaaaaa*bb.........",
"........*aaaa***aaaa*bb.........",
".......*aaaa*****aaaa*bb........",
".......*aaaa*****aaaa*bb........",
"......*aaaaa*****aaaaa*bb.......",
"......*aaaaa*****aaaaa*bb.......",
".....*aaaaaa*****aaaaaa*bb......",
".....*aaaaaa*****aaaaaa*bb......",
"....*aaaaaaaa***aaaaaaaa*bb.....",
"....*aaaaaaaa***aaaaaaaa*bb.....",
"...*aaaaaaaaa***aaaaaaaaa*bb....",
"...*aaaaaaaaaa*aaaaaaaaaa*bb....",
"..*aaaaaaaaaaa*aaaaaaaaaaa*bb...",
"..*aaaaaaaaaaaaaaaaaaaaaaa*bb...",
".*aaaaaaaaaaaa**aaaaaaaaaaa*bb..",
".*aaaaaaaaaaa****aaaaaaaaaa*bb..",
"*aaaaaaaaaaaa****aaaaaaaaaaa*bb.",
"*aaaaaaaaaaaaa**aaaaaaaaaaaa*bb.",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaa*bbb",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaa*bbb",
".*aaaaaaaaaaaaaaaaaaaaaaaaa*bbbb",
"..*************************bbbbb",
"....bbbbbbbbbbbbbbbbbbbbbbbbbbb.",
".....bbbbbbbbbbbbbbbbbbbbbbbbb.."};
/* XPM */
static const char* const critical_xpm[]={
"32 32 4 1",
". c None",
"a c #999999",
"* c #ff0000",
"b c #ffffff",
"...........********.............",
".........************...........",
".......****************.........",
"......******************........",
".....********************a......",
"....**********************a.....",
"...************************a....",
"..*******b**********b*******a...",
"..******bbb********bbb******a...",
".******bbbbb******bbbbb******a..",
".*******bbbbb****bbbbb*******a..",
"*********bbbbb**bbbbb*********a.",
"**********bbbbbbbbbb**********a.",
"***********bbbbbbbb***********aa",
"************bbbbbb************aa",
"************bbbbbb************aa",
"***********bbbbbbbb***********aa",
"**********bbbbbbbbbb**********aa",
"*********bbbbb**bbbbb*********aa",
".*******bbbbb****bbbbb*******aa.",
".******bbbbb******bbbbb******aa.",
"..******bbb********bbb******aaa.",
"..*******b**********b*******aa..",
"...************************aaa..",
"....**********************aaa...",
"....a********************aaa....",
".....a******************aaa.....",
"......a****************aaa......",
".......aa************aaaa.......",
".........aa********aaaaa........",
"...........aaaaaaaaaaa..........",
".............aaaaaaa............"};
/* XPM */
static const char *const question_xpm[] = {
"32 32 5 1",
". c None",
"c c #000000",
"* c #999999",
"a c #ffffff",
"b c #0000ff",
"...........********.............",
"........***aaaaaaaa***..........",
"......**aaaaaaaaaaaaaa**........",
".....*aaaaaaaaaaaaaaaaaa*.......",
"....*aaaaaaaaaaaaaaaaaaaac......",
"...*aaaaaaaabbbbbbaaaaaaaac.....",
"..*aaaaaaaabaaabbbbaaaaaaaac....",
".*aaaaaaaabbaaaabbbbaaaaaaaac...",
".*aaaaaaaabbbbaabbbbaaaaaaaac*..",
"*aaaaaaaaabbbbaabbbbaaaaaaaaac*.",
"*aaaaaaaaaabbaabbbbaaaaaaaaaac*.",
"*aaaaaaaaaaaaabbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaaabbbaaaaaaaaaaaac**",
"*aaaaaaaaaaaaabbaaaaaaaaaaaaac**",
"*aaaaaaaaaaaaabbaaaaaaaaaaaaac**",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaaac**",
".*aaaaaaaaaaaabbaaaaaaaaaaaac***",
".*aaaaaaaaaaabbbbaaaaaaaaaaac***",
"..*aaaaaaaaaabbbbaaaaaaaaaac***.",
"...caaaaaaaaaabbaaaaaaaaaac****.",
"....caaaaaaaaaaaaaaaaaaaac****..",
".....caaaaaaaaaaaaaaaaaac****...",
"......ccaaaaaaaaaaaaaacc****....",
".......*cccaaaaaaaaccc*****.....",
"........***cccaaaac*******......",
"..........****caaac*****........",
".............*caaac**...........",
"...............caac**...........",
"................cac**...........",
".................cc**...........",
"..................***...........",
"...................**...........",
};
#endif

/*!
 \reimp
 */
QPixmap QMotifStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                           const QWidget *widget) const
/*
QPixmap QMotifStyle::stylePixmap(StylePixmap sp,
                                 const QWidget *widget,
                                 const Q3StyleOption& opt) const
                                 */
{
#ifndef QT_NO_IMAGEIO_XPM
    switch (sp) {
    case SP_TitleBarShadeButton:
        return QPixmap((const char **)qt_shade_xpm);
    case SP_TitleBarUnshadeButton:
        return QPixmap((const char **)qt_unshade_xpm);
    case SP_TitleBarNormalButton:
        return QPixmap((const char **)qt_normalizeup_xpm);
    case SP_TitleBarMinButton:
        return QPixmap((const char **)qt_minimize_xpm);
    case SP_TitleBarMaxButton:
        return QPixmap((const char **)qt_maximize_xpm);
    case SP_TitleBarCloseButton:
        return QPixmap((const char **)qt_close_xpm);
    case SP_DockWindowCloseButton:
        return QPixmap((const char **)dock_window_close_xpm);

    case SP_MessageBoxInformation:
    case SP_MessageBoxWarning:
    case SP_MessageBoxCritical:
    case SP_MessageBoxQuestion:
        {
            const char * const * xpm_data;
            switch (sp) {
            case SP_MessageBoxInformation:
                xpm_data = information_xpm;
                break;
            case SP_MessageBoxWarning:
                xpm_data = warning_xpm;
                break;
            case SP_MessageBoxCritical:
                xpm_data = critical_xpm;
                break;
            case SP_MessageBoxQuestion:
                xpm_data = question_xpm;
                break;
            default:
                xpm_data = 0;
                break;
            }
            QPixmap pm;
            if (xpm_data) {
                QImage image((const char **) xpm_data);
                // All that color looks ugly in Motif
                const QPalette &pal = QApplication::palette();
                switch (sp) {
                case SP_MessageBoxInformation:
                case SP_MessageBoxQuestion:
                    image.setColor(2, 0xff000000 |
                                    pal.color(QPalette::Active, QPalette::Dark).rgb());
                    image.setColor(3, 0xff000000 |
                                    pal.color(QPalette::Active, QPalette::Base).rgb());
                    image.setColor(4, 0xff000000 |
                                    pal.color(QPalette::Active, QPalette::Text).rgb());
                    break;
                case SP_MessageBoxWarning:
                    image.setColor(1, 0xff000000 |
                                    pal.color(QPalette::Active, QPalette::Base).rgb());
                    image.setColor(2, 0xff000000 |
                                    pal.color(QPalette::Active, QPalette::Text).rgb());
                    image.setColor(3, 0xff000000 |
                                    pal.color(QPalette::Active, QPalette::Dark).rgb());
                    break;
                case SP_MessageBoxCritical:
                    image.setColor(1, 0xff000000 |
                                    pal.color(QPalette::Active, QPalette::Dark).rgb());
                    image.setColor(2, 0xff000000 |
                                    pal.color(QPalette::Active, QPalette::Text).rgb());
                    image.setColor(3, 0xff000000 |
                                    pal.color(QPalette::Active, QPalette::Base).rgb());
                    break;
                default:
                    break;
                }
                pm.convertFromImage(image);
            }
            return pm;
        }

    default:
        break;
    }
#endif

    return QCommonStyle::stylePixmap(sp, widget, opt);
}


/*! \reimp */
int QMotifStyle::styleHint(StyleHint hint, const QStyleOption *opt, const QWidget *widget,
                  QStyleHintReturn *returnData) const
/*
int QMotifStyle::styleHint(StyleHint hint,
                           const QWidget *widget,
                           const Q3StyleOption &opt,
                           QStyleHintReturn *returnData) const
                           */
{
    int ret;

    switch (hint) {
#ifdef QT_COMPAT
    case SH_GUIStyle:
        ret = Qt::MotifStyle;
        break;
#endif
    case SH_DrawMenuBarSeparator:
        ret = true;
        break;

    // #ifndef QT_NO_PALETTE
    // case SH_ScrollBar_BackgroundRole:
    //     ret = QPalette::Mid;
    //     break;
    // #endif

    case SH_ScrollBar_MiddleClickAbsolutePosition:
    case SH_Slider_SloppyKeyEvents:
    case SH_ProgressDialog_CenterCancelButton:
    case SH_Menu_SpaceActivatesItem:
    case SH_ScrollView_FrameOnlyAroundContents:
        ret = 1;
        break;

    case SH_Menu_SubMenuPopupDelay:
        ret = 96;
        break;

    case SH_ProgressDialog_TextLabelAlignment:
        ret = Qt::AlignAuto | Qt::AlignVCenter;
        break;

    case SH_ItemView_ChangeHighlightOnFocus:
        ret = 0;
        break;

    case SH_ColorDialog_SelectedColorBorder:
        ret = 2;
        break;

    case SH_MenuBar_DismissOnSecondClick:
        ret = 0;
        break;

    case SH_MessageBox_UseBorderForButtonSpacing:
        ret = 1;
        break;

    default:
        ret = QCommonStyle::styleHint(hint, widget, opt, returnData);
        break;
    }

    return ret;
}


#endif
