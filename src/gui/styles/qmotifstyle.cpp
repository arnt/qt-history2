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
  subControlRect(), setUseHighlightColors(),
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
    focus = 0;
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

static void rot(QPolygon& a, int n)
{
    QPolygon r(a.size());
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
{
    switch(pe) {
    case PE_Q3CheckListExclusiveIndicator:
        if (const QStyleOptionQ3ListView *lv = qt_cast<const QStyleOptionQ3ListView *>(opt)) {
            if (lv->items.isEmpty())
                return;

            if (lv->state & State_Enabled)
                p->setPen(QPen(opt->palette.text().color()));
            else
                p->setPen(QPen(lv->palette.color(QPalette::Disabled, QPalette::Text)));
            QPolygon a;

            int cx = opt->rect.width()/2 - 1;
            int cy = opt->rect.height()/2;
            int e = opt->rect.width()/2 - 1;
            for (int i = 0; i < 3; i++) { //penWidth 2 doesn't quite work
                a.setPoints(4, cx-e, cy, cx, cy-e,  cx+e, cy,  cx, cy+e);
                p->drawPolygon(a);
                e--;
            }
            if (opt->state & State_On) {
                if (lv->state & State_Enabled)
                    p->setPen(QPen(opt->palette.text().color()));
                else
                    p->setPen(QPen(lv->palette.color(QPalette::Disabled,
                                                     QPalette::Text)));
                QBrush saveBrush = p->brush();
                p->setBrush(opt->palette.text());
                e = e - 2;
                a.setPoints(4, cx-e, cy, cx, cy-e,  cx+e, cy,  cx, cy+e);
                p->drawPolygon(a);
                p->setBrush(saveBrush);
            }
        }
        break;

    case PE_IndicatorToolBarHandle: {
        p->save();
        p->translate(opt->rect.x(), opt->rect.y());

        QColor dark(opt->palette.dark().color());
        QColor light(opt->palette.light().color());
        int i;
        if (opt->state & State_Horizontal) {
            int h = opt->rect.height();
            if (h > 6) {
                if (opt->state & State_On)
                    p->fillRect(1, 1, 8, h - 2, opt->palette.highlight());
                QPolygon a(2 * ((h-6)/3));
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
                // p->drawLine(0, opt->rect.height() - 1,
                // tbExtent, opt->rect.height() - 1);
                // }
            }
        } else {
            int w = opt->rect.width();
            if (w > 6) {
                if (opt->state & State_On)
                    p->fillRect(1, 1, w - 2, 9, opt->palette.highlight());
                QPolygon a(2 * ((w-6)/3));

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
                // p->drawLine(opt->rect.width() - 1, 0,
                // opt->rect.width() - 1, tbExtent);
                // }
            }
        }
        p->restore();
        break; }

    case PE_PanelButtonCommand:
    case PE_PanelButtonBevel:
    case PE_PanelButtonTool: {
        QBrush fill;
        if (opt->state & State_Down)
            fill = opt->palette.brush(QPalette::Mid);
        else if (opt->state & State_On)
            fill = QBrush(opt->palette.mid().color(), Qt::Dense4Pattern);
        else
            fill = opt->palette.brush(QPalette::Button);
        qDrawShadePanel(p, opt->rect, opt->palette, bool(opt->state & (State_Down | State_On)),
                        pixelMetric(PM_DefaultFrameWidth), &fill);
        break; }

    case PE_IndicatorCheckBox: {
        bool on = opt->state & State_On;
        bool down = opt->state & State_Down;
        bool showUp = !(down ^ on);
        QBrush fill = showUp || opt->state & State_NoChange ?
                      opt->palette.brush(QPalette::Button) : opt->palette.brush(QPalette::Mid);
        if (opt->state & State_NoChange) {
            qDrawPlainRect(p, opt->rect, opt->palette.text().color(),
                           1, &fill);
            p->drawLine(opt->rect.x() + opt->rect.width() - 1, opt->rect.y(),
                        opt->rect.x(), opt->rect.y() + opt->rect.height() - 1);
        } else {
            qDrawShadePanel(p, opt->rect, opt->palette, !showUp,
                            pixelMetric(PM_DefaultFrameWidth), &fill);
        }
        if (!(opt->state & State_Enabled) && styleHint(SH_DitherDisabledText))
            p->fillRect(opt->rect, QBrush(p->background().color(), Qt::Dense5Pattern));
        break; }

    case PE_IndicatorRadioButton: {
#define INTARRLEN(x) sizeof(x)/(sizeof(int)*2)
        int inner_pts[] = { // used for filling diamond
            2,opt->rect.height()/2,
            opt->rect.width()/2,2,
            opt->rect.width()-3,opt->rect.height()/2,
            opt->rect.width()/2,opt->rect.height()-3
        };
        int top_pts[] = { // top (^) of diamond
            0,opt->rect.height()/2,
            opt->rect.width()/2,0,
            opt->rect.width()-2,opt->rect.height()/2-1,
            opt->rect.width()-3,opt->rect.height()/2-1,
            opt->rect.width()/2,1,
            1,opt->rect.height()/2,
            2,opt->rect.height()/2,
            opt->rect.width()/2,2,
            opt->rect.width()-4,opt->rect.height()/2-1
        };
        int bottom_pts[] = { // bottom (v) of diamond
            1,opt->rect.height()/2+1,
            opt->rect.width()/2,opt->rect.height()-1,
            opt->rect.width()-1,opt->rect.height()/2,
            opt->rect.width()-2,opt->rect.height()/2,
            opt->rect.width()/2,opt->rect.height()-2,
            2,opt->rect.height()/2+1,
            3,opt->rect.height()/2+1,
            opt->rect.width()/2,opt->rect.height()-3,
            opt->rect.width()-3,opt->rect.height()/2
        };
        bool on = opt->state & State_On;
        bool down = opt->state & State_Down;
        bool showUp = !(down ^ on);
        QPolygon a(INTARRLEN(inner_pts), inner_pts);
        p->setPen(Qt::NoPen);
        p->setBrush(showUp ? opt->palette.brush(QPalette::Button) :
                    opt->palette.brush(QPalette::Mid));
        a.translate(opt->rect.x(), opt->rect.y());
        p->drawPolygon(a);
        p->setPen(showUp ? opt->palette.light().color() : opt->palette.dark().color());
        p->setBrush(Qt::NoBrush);
        a.setPoints(INTARRLEN(top_pts), top_pts);
        a.translate(opt->rect.x(), opt->rect.y());
        p->drawPolyline(a);
        p->setPen(showUp ? opt->palette.dark().color() : opt->palette.light().color());
        a.setPoints(INTARRLEN(bottom_pts), bottom_pts);
        a.translate(opt->rect.x(), opt->rect.y());
        p->drawPolyline(a);
        if (!(opt->state & State_Enabled) && styleHint(SH_DitherDisabledText))
            p->fillRect(opt->rect, QBrush(p->background().color(), Qt::Dense5Pattern));
        break; }

    case PE_IndicatorSpinUp:
    case PE_IndicatorSpinPlus:
    case PE_IndicatorSpinDown:
    case PE_IndicatorSpinMinus:
    case PE_IndicatorArrowUp:
    case PE_IndicatorArrowDown:
    case PE_IndicatorArrowRight:
    case PE_IndicatorArrowLeft: {
        QRect rect = opt->rect;
        QPolygon bFill;
        QPolygon bTop;
        QPolygon bBot;
        QPolygon bLeft;
        if (pe == PE_IndicatorSpinPlus || pe == PE_IndicatorSpinUp)
            pe = PE_IndicatorArrowUp;
        else if (pe == PE_IndicatorSpinMinus || pe == PE_IndicatorSpinDown)
            pe = PE_IndicatorArrowDown;
        bool vertical = pe == PE_IndicatorArrowUp || pe == PE_IndicatorArrowDown;
        bool horizontal = !vertical;
        int dim = rect.width() < rect.height() ? rect.width() : rect.height();
        int colspec = 0x0000;

        if (!(opt->state & State_Enabled))
            dim -= 2;

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
            if (pixelMetric(PM_DefaultFrameWidth) < 2) { // thin style
                bFill.resize( dim & 1 ? 3 : 4 );
                bTop.resize( 2 );
                bBot.resize( 2 );
                bLeft.resize( 2 );
                bLeft.putPoints( 0, 2, 0, 0, 0, dim-1 );
                bTop.putPoints( 0, 2, 1, 0, dim-1, dim/2 );
                bBot.putPoints( 0, 2, 1, dim-1, dim-1, dim/2 );

                if ( dim > 6 ) {                        // dim>6: must fill interior
                    bFill.putPoints( 0, 2, 0, dim-1, 0, 0 );
                    if ( dim & 1 )                      // if size is an odd number
                        bFill.setPoint( 2, dim - 1, dim / 2 );
                    else
                        bFill.putPoints( 2, 2, dim-1, dim/2-1, dim-1, dim/2 );
                }
            } else {
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
        } else {
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

        if (pe == PE_IndicatorArrowUp || pe == PE_IndicatorArrowLeft) {
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
            if (opt->state & State_Down)
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
            if (opt->state & State_Down)
                colspec = horizontal ? 0x2443 : 0x2434;
            else
                colspec = horizontal ? 0x1334 : 0x1343;
        }
        bFill.translate(rect.x(), rect.y());
        bLeft.translate(rect.x(), rect.y());
        bTop.translate(rect.x(), rect.y());
        bBot.translate(rect.x(), rect.y());

        const QColor *cols[5];
        if (opt->state & State_Enabled) {
            cols[0] = 0;
            cols[1] = &opt->palette.button().color();
            cols[2] = &opt->palette.mid().color();
            cols[3] = &opt->palette.light().color();
            cols[4] = &opt->palette.dark().color();
        } else {
            cols[0] = 0;
            cols[1] = &opt->palette.mid().color();
            cols[2] = &opt->palette.mid().color();
            cols[3] = &opt->palette.mid().color();
            cols[4] = &opt->palette.mid().color();
        }

#define CMID *cols[(colspec>>12) & 0xf]
#define CLEFT *cols[(colspec>>8) & 0xf]
#define CTOP *cols[(colspec>>4) & 0xf]
#define CBOT *cols[colspec & 0xf]

        QPen savePen = p->pen();
        QBrush saveBrush = p->brush();
        QPen pen(Qt::NoPen);
        QBrush brush = opt->palette.brush(opt->state & State_Enabled ? QPalette::Button :
                                          QPalette::Mid);
        p->setPen(pen);
        p->setBrush(brush);
        p->drawPolygon(bFill);
        p->setBrush(Qt::NoBrush);

        p->setPen(CLEFT);
        p->drawPolyline(bLeft);
        p->setPen(CTOP);
        p->drawPolyline(bTop);
        p->setPen(CBOT);
        p->drawPolyline(bBot);

        p->setBrush(saveBrush);
        p->setPen(savePen);
#undef CMID
#undef CLEFT
#undef CTOP
#undef CBOT
        if (!(opt->state & State_Enabled) && styleHint(SH_DitherDisabledText))
            p->fillRect(opt->rect, QBrush(p->background().color(), Qt::Dense5Pattern));
        break; }

    case PE_IndicatorDockWindowResizeHandle: {
        const int motifOffset = 10;
        int sw = pixelMetric(PM_SplitterWidth);
        if (opt->state & State_Horizontal) {
            int yPos = opt->rect.y() + opt->rect.height() / 2;
            int kPos = opt->rect.width() - motifOffset - sw;
            int kSize = sw - 2;

            qDrawShadeLine(p, 0, yPos, kPos, yPos, opt->palette);
            qDrawShadePanel(p, kPos, yPos - sw / 2 + 1, kSize, kSize,
                            opt->palette, false, 1, &opt->palette.brush(QPalette::Button));
            qDrawShadeLine(p, kPos + kSize - 1, yPos, opt->rect.width(), yPos, opt->palette);
        } else {
            int xPos = opt->rect.x() + opt->rect.width() / 2;
            int kPos = motifOffset;
            int kSize = sw - 2;

            qDrawShadeLine(p, xPos, kPos + kSize - 1, xPos, opt->rect.height(), opt->palette);
            qDrawShadePanel(p, xPos - sw / 2 + 1, kPos, kSize, kSize, opt->palette,
                            false, 1, &opt->palette.brush(QPalette::Button));
            qDrawShadeLine(p, xPos, 0, xPos, kPos, opt->palette);
        }
        break; }

    case PE_IndicatorMenuCheckMark: {
        const int markW = 6;
        const int markH = 6;
        int posX = opt->rect.x() + (opt->rect.width()  - markW) / 2 - 1;
        int posY = opt->rect.y() + (opt->rect.height() - markH) / 2;
        int dfw = pixelMetric(PM_DefaultFrameWidth);

        if (dfw < 2) {
            // Could do with some optimizing/caching...
            QPolygon a(7*2);
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
            if (! (opt->state & State_Enabled) && ! (opt->state & State_On)) {
                int pnt;
                p->setPen(opt->palette.highlightedText().color());
                QPoint offset(1,1);
                for (pnt = 0; pnt < (int)a.size(); pnt++)
                    a[pnt] += offset;
                p->drawPolyline(a);
                for (pnt = 0; pnt < (int)a.size(); pnt++)
                    a[pnt] -= offset;
            }
            p->setPen(opt->palette.text().color());
            p->drawPolyline(a);

            qDrawShadePanel(p, posX-2, posY-2, markW+4, markH+6, opt->palette, true, dfw);
        } else
            qDrawShadePanel(p, posX, posY, markW, markH, opt->palette, true, dfw,
                            &opt->palette.brush(QPalette::Mid));

        break; }

    case PE_IndicatorProgressChunk:
        p->fillRect(opt->rect.x(), opt->rect.y() + 2, opt->rect.width() - 2,
                    opt->rect.height() - 4, opt->palette.brush(QPalette::Highlight));
        break;

    default:
        QCommonStyle::drawPrimitive(pe, opt, p, w);
        break;
    }
}


/*!
  \reimp
*/
void QMotifStyle::drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                              const QWidget *widget) const
{
    switch(element) {
    case CE_Splitter: {
        QStyleOption handleOpt = *opt;
        if (handleOpt.state & State_Horizontal)
            handleOpt.state &= ~State_Horizontal;
        else
            handleOpt.state |= State_Horizontal;
        drawPrimitive(PE_IndicatorDockWindowResizeHandle, &handleOpt, p, widget);
        break; }

    case CE_ScrollBarSubLine: {
        QStyleOption arrowOpt = *opt;
        arrowOpt.state |= State_Enabled;
        drawPrimitive(((opt->state & State_Horizontal) ? PE_IndicatorArrowLeft : PE_IndicatorArrowUp),
                      &arrowOpt, p, widget);
        break; }

    case CE_ScrollBarAddLine:{
        QStyleOption arrowOpt = *opt;
        arrowOpt.state |= State_Enabled;
        drawPrimitive(((opt->state & State_Horizontal) ? PE_IndicatorArrowRight : PE_IndicatorArrowDown),
                      &arrowOpt, p, widget);
        break; }

    case CE_ScrollBarSubPage:
    case CE_ScrollBarAddPage:
        p->fillRect(opt->rect, opt->palette.brush(QPalette::Mid));
        break;

    case CE_ScrollBarSlider: {
        QStyleOption bevelOpt = *opt;
        bevelOpt.state = (opt->state | State_Raised) & ~State_Down;
        drawPrimitive(PE_PanelButtonBevel, &bevelOpt, p, widget);
        break; }

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
            if ((btn->state & State_HasFocus) && (!focus || !focus->isVisible())) {
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
    case CE_PushButton:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            drawControl(CE_PushButtonBevel, btn, p, widget);
            QStyleOptionButton subopt = *btn;
            subopt.rect = QStyle::visualRect(btn->direction, btn->rect,
                                             subRect(SR_PushButtonContents, btn, widget));
            drawControl(CE_PushButtonLabel, &subopt, p, widget);
            if ((btn->state & State_HasFocus) && (!focus || !focus->isVisible())) {
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
            int diw, x1, y1, x2, y2;
            p->setPen(opt->palette.foreground().color());
            p->setBrush(QBrush(opt->palette.button().color(), Qt::NoBrush));
            diw = pixelMetric(PM_ButtonDefaultIndicator);
            opt->rect.getCoords(&x1, &y1, &x2, &y2);
            if (btn->features & (QStyleOptionButton::AutoDefaultButton|QStyleOptionButton::DefaultButton)) {
                x1 += diw;
                y1 += diw;
                x2 -= diw;
                y2 -= diw;
            }
            if (btn->features & QStyleOptionButton::DefaultButton) {
                if (diw == 0) {
                    QPolygon a;
                    a.setPoints(9,
                                x1, y1, x2, y1, x2, y2, x1, y2, x1, y1+1,
                                x2-1, y1+1, x2-1, y2-1, x1+1, y2-1, x1+1, y1+1);
                    p->setPen(opt->palette.shadow().color());
                    p->drawPolygon(a);
                    x1 += 2;
                    y1 += 2;
                    x2 -= 2;
                    y2 -= 2;
                } else {
                    qDrawShadePanel(p, opt->rect.adjusted(1, 1, -1, -1), opt->palette, true);
                }
            }
            if (!(btn->features & QStyleOptionButton::Flat) ||
                (btn->state & (State_Down|State_On))) {
                QStyleOption newOpt = *opt;
                newOpt.rect = QRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
                p->setBrushOrigin(p->brushOrigin());
                drawPrimitive(PE_PanelButtonCommand, &newOpt, p, widget);
            }
            break;
        }

    case CE_TabBarTabShape:
        if (const QStyleOptionTab *tab = qt_cast<const QStyleOptionTab *>(opt)) {
            int dfw = pixelMetric(PM_DefaultFrameWidth, tab, widget);
            bool selected = opt->state & State_Selected;
            int o =  dfw > 1 ? 1 : 0;
            bool lastTab = false;

            QRect r2(opt->rect);
            if (tab->shape == QTabBar::RoundedAbove) {
                if (styleHint(SH_TabBar_Alignment, tab, widget) == Qt::AlignRight &&
                    tab->position == QStyleOptionTab::End)
                    lastTab = true;

                if (o) {
                    p->setPen(opt->palette.light().color());
                    p->drawLine(r2.left(), r2.bottom(), r2.right(), r2.bottom());
                    p->setPen(opt->palette.light().color());
                    p->drawLine(r2.left(), r2.bottom()-1, r2.right(), r2.bottom()-1);
                    if (r2.left() == 0)
                        p->drawPoint(opt->rect.bottomLeft());
                }
                else {
                    p->setPen(opt->palette.light().color());
                    p->drawLine(r2.left(), r2.bottom(), r2.right(), r2.bottom());
                }

                if (selected) {
                    p->fillRect(QRect(r2.left()+1, r2.bottom()-o, r2.width()-3, 2),
                                tab->palette.brush(QPalette::Active, QPalette::Background));
                    p->setPen(tab->palette.background().color());
                    // p->drawLine(r2.left()+1, r2.bottom(), r2.right()-2, r2.bottom());
                    // if (o)
                    // p->drawLine(r2.left()+1, r2.bottom()-1, r2.right()-2, r2.bottom()-1);
                    p->drawLine(r2.left()+1, r2.bottom(), r2.left()+1, r2.top()+2);
                    p->setPen(tab->palette.light().color());
                } else {
                    p->setPen(tab->palette.light().color());
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

                p->setPen(tab->palette.dark().color());
                p->drawLine(r2.right() - 1, r2.top() + 2,
                            r2.right() - 1, r2.bottom() - 1 + (selected ? o : -o));
                if (o) {
                    p->drawPoint(r2.right() - 1, r2.top() + 1);
                    p->drawLine(r2.right(), r2.top() + 2, r2.right(),
                                r2.bottom() -
                                (selected ? (lastTab ? 0:1):1+o));
                    p->drawPoint(r2.right() - 1, r2.top() + 1);
                }
            } else if (tab->shape  == QTabBar::RoundedBelow) {
                if (styleHint(SH_TabBar_Alignment, tab, widget) == Qt::AlignLeft &&
                    tab->position == QStyleOptionTab::End)
                    lastTab = true;
                if (selected) {
                    p->fillRect(QRect(r2.left()+1, r2.top(), r2.width()-3, 1),
                                tab->palette.brush(QPalette::Active, QPalette::Background));
                    p->setPen(tab->palette.background().color());
                    // p->drawLine(r2.left()+1, r2.top(), r2.right()-2, r2.top());
                    p->drawLine(r2.left()+1, r2.top(), r2.left()+1, r2.bottom()-2);
                    p->setPen(tab->palette.dark().color());
                } else {
                    p->setPen(tab->palette.dark().color());
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

                p->setPen(tab->palette.light().color());
                p->drawLine(r2.left(), r2.top() + (selected ? 0 : 2),
                            r2.left(), r2.bottom() - 2);
                p->drawLine(r2.left() + 1, r2.top() + (selected ? 0 : 2),
                            r2.left() + 1, r2.bottom() - 3);

            } else {
                QCommonStyle::drawControl(element, opt, p, widget);
            }
            break; }
    case CE_ProgressBarGroove:
        qDrawShadePanel(p, opt->rect, opt->palette, true, 2);
        break;

    case CE_ProgressBarLabel:
        if (const QStyleOptionProgressBar *pb = qt_cast<const QStyleOptionProgressBar *>(opt)) {
            const int unit_width = pixelMetric(PM_ProgressBarChunkWidth, opt, widget);
            int u = opt->rect.width() / unit_width;
            int p_v = pb->progress;
            int t_s = pb->minimum - pb->maximum;
            if (u > 0 && pb->progress >= INT_MAX / u && t_s >= u) {
                // scale down to something usable.
                p_v /= u;
                t_s /= u;
            }
            if (pb->textVisible && pb->minimum - pb->maximum) {
                int nu = (u * p_v + t_s/2) / t_s;
                int x = unit_width * nu;
                if (pb->textAlignment == Qt::AlignAuto || pb->textAlignment == Qt::AlignCenter) {
                    p->setPen(opt->palette.highlightedText().color());
                    p->setClipRect(opt->rect.x(), opt->rect.y(), x, opt->rect.height());
                    p->drawText(opt->rect, Qt::AlignCenter | Qt::TextSingleLine, pb->text);

                    if (pb->progress != pb->maximum) {
                        p->setClipRect(opt->rect.x() + x, opt->rect.y(), opt->rect.width() - x, opt->rect.height());
                        p->setPen(opt->palette.highlight().color());
                        p->drawText(opt->rect, Qt::AlignCenter | Qt::TextSingleLine, pb->text);
                    }
                } else {
                    p->setPen(opt->palette.text().color());
                    p->drawText(opt->rect, Qt::AlignCenter | Qt::TextSingleLine, pb->text);
                }
            }
            break;
        }

    case CE_MenuTearoff: {
        if(opt->state & State_Selected) {
            if(pixelMetric(PM_MenuPanelWidth, opt, widget) > 1)
                qDrawShadePanel(p, opt->rect.x(), opt->rect.y(), opt->rect.width(),
                                opt->rect.height(), opt->palette, false, motifItemFrame,
                                &opt->palette.brush(QPalette::Button));
            else
                qDrawShadePanel(p, opt->rect.x()+1, opt->rect.y()+1, opt->rect.width()-2,
                                opt->rect.height()-2, opt->palette, true, 1, &opt->palette.brush(QPalette::Button));
        } else {
            p->fillRect(opt->rect, opt->palette.brush(QPalette::Button));
        }
        p->setPen(QPen(opt->palette.dark().color(), 1, Qt::DashLine));
        p->drawLine(opt->rect.x()+2, opt->rect.y()+opt->rect.height()/2-1, opt->rect.x()+opt->rect.width()-4,
                    opt->rect.y()+opt->rect.height()/2-1);
        p->setPen(QPen(opt->palette.light().color(), 1, Qt::DashLine));
        p->drawLine(opt->rect.x()+2, opt->rect.y()+opt->rect.height()/2, opt->rect.x()+opt->rect.width()-4,
                    opt->rect.y()+opt->rect.height()/2);
        break; }

    case CE_MenuItem:
        if (const QStyleOptionMenuItem *menuitem = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            int maxpmw = menuitem->maxIconWidth;
            if(menuitem->checkType != QStyleOptionMenuItem::NotCheckable)
                maxpmw = qMax(maxpmw, motifCheckMarkSpace);

            int x, y, w, h;
            opt->rect.getRect(&x, &y, &w, &h);

            if(menuitem->menuItemType == QStyleOptionMenuItem::Separator) {  // draw separator
                p->setPen(opt->palette.dark().color());
                p->drawLine(x, y, x+w, y);
                p->setPen(opt->palette.light().color());
                p->drawLine(x, y+1, x+w, y+1);
                return;
            }

            int pw = motifItemFrame;
            if((opt->state & State_Selected) && (opt->state & State_Enabled)) {  // active item frame
                if(pixelMetric(PM_MenuPanelWidth, opt) > 1)
                    qDrawShadePanel(p, x, y, w, h, opt->palette, false, pw,
                                    &opt->palette.brush(QPalette::Button));
                else
                    qDrawShadePanel(p, x+1, y+1, w-2, h-2, opt->palette, true, 1,
                                    &opt->palette.brush(QPalette::Button));
            } else  {                               // incognito frame
                p->fillRect(x, y, w, h, opt->palette.brush(QPalette::Button));
            }

            QRect vrect = visualRect(opt->direction, opt->rect,
                                     QRect(x+motifItemFrame, y+motifItemFrame, maxpmw, h-2*motifItemFrame));
            int xvis = vrect.x();
            if (menuitem->checked) {
                if(!menuitem->icon.isNull())
                    qDrawShadePanel(p, xvis, y+motifItemFrame, maxpmw, h-2*motifItemFrame,
                                    opt->palette, true, 1, &opt->palette.brush(QPalette::Midlight));
            } else if (!(opt->state & State_Selected)) {
                p->fillRect(xvis, y+motifItemFrame, maxpmw, h-2*motifItemFrame,
                            opt->palette.brush(QPalette::Button));
            }

            if(!menuitem->icon.isNull()) {              // draw icon
                QIcon::Mode mode = QIcon::Normal; // no disabled icons in Motif
                if ((opt->state & State_Selected) && !!(opt->state & State_Enabled))
                    mode = QIcon::Active;
                QPixmap pixmap;
                if (menuitem->checkType != QStyleOptionMenuItem::NotCheckable && menuitem->checked)
                    pixmap = menuitem->icon.pixmap(Qt::SmallIconSize, mode, QIcon::On);
                else
                    pixmap = menuitem->icon.pixmap(Qt::SmallIconSize, mode);

                int pixw = pixmap.width();
                int pixh = pixmap.height();
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(vrect.center());
                p->setPen(opt->palette.text().color());
                p->drawPixmap(pmr.topLeft(), pixmap);

            } else  if (menuitem->checkType != QStyleOptionMenuItem::NotCheckable) {  // just "checking"...
                int mw = maxpmw;
                int mh = h - 2*motifItemFrame;
                if (menuitem->checked) {
                    QStyleOptionMenuItem newMenuItem = *menuitem;
                    newMenuItem.state = State_Default;
                    if ((opt->state & State_Enabled))
                        newMenuItem.state |= State_Enabled;
                    if ((opt->state & State_Selected))
                        newMenuItem.state |= State_On;
                    newMenuItem.rect = QRect(xvis, y+motifItemFrame, mw, mh);
                    drawPrimitive(PE_IndicatorMenuCheckMark, &newMenuItem, p, widget);
                }
            }

            p->setPen(opt->palette.buttonText().color());

            QColor discol;
            if (!(opt->state & State_Enabled)) {
                discol = opt->palette.text().color();
                p->setPen(discol);
            }

            int xm = motifItemFrame + maxpmw + motifItemHMargin;

            vrect = visualRect(opt->direction, opt->rect,
                               QRect(x+xm, y+motifItemVMargin, w-xm-menuitem->tabWidth, h-2*motifItemVMargin));
            xvis = vrect.x();

            QString s = menuitem->text;
            if (!s.isNull()) {                        // draw text
                int t = s.indexOf('\t');
                int m = motifItemVMargin;
                int text_flags = Qt::AlignVCenter|Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                text_flags |= Qt::AlignLeft;
                if (t >= 0) {                         // draw tab text
                    QRect vr = visualRect(opt->direction, opt->rect,
                                          QRect(x+w-menuitem->tabWidth-motifItemHMargin-motifItemFrame,
                                                y+motifItemVMargin, menuitem->tabWidth, h-2*motifItemVMargin));
                    int xv = vr.x();
                    QRect tr(xv, y+m, menuitem->tabWidth, h-2*m);
                    p->drawText(tr, text_flags, s.mid(t+1));
                    if (!(opt->state & State_Enabled) && styleHint(SH_DitherDisabledText))
                        p->fillRect(tr, QBrush(p->background().color(), Qt::Dense5Pattern));
                    s = s.left(t);
                }
                QRect tr(xvis, y+m, w - xm - menuitem->tabWidth + 1, h-2*m);
                p->drawText(tr, text_flags, s.left(t));
                if (!(opt->state & State_Enabled) && styleHint(SH_DitherDisabledText))
                    p->fillRect(tr, QBrush(p->background().color(), Qt::Dense5Pattern));
            }
            if (menuitem->menuItemType == QStyleOptionMenuItem::SubMenu) {           // draw sub menu arrow
                int dim = (h-2*motifItemFrame) / 2;
                QStyle::PrimitiveElement arrow = (QApplication::isRightToLeft() ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight);
                QStyleOption arrowOpt = *opt;
                arrowOpt.rect = visualRect(opt->direction, opt->rect,
                                           QRect(x+w - motifArrowHMargin - motifItemFrame - dim,
                                                 y+h/2-dim/2, dim, dim));
                if ((opt->state & State_Selected))
                    arrowOpt.state = (State_Down | ((opt->state & State_Enabled) ? State_Enabled : State_Default));
                else
                    arrowOpt.state = ((opt->state & State_Enabled) ? State_Enabled : State_Default);
                drawPrimitive(arrow, &arrowOpt, p, widget);
            }
            break; }

    case CE_MenuBarItem:
        if (opt->state & State_Selected)  // active item
            qDrawShadePanel(p, opt->rect, opt->palette, false, motifItemFrame,
                            &opt->palette.brush(QPalette::Button));
        else  // other item
            p->fillRect(opt->rect, opt->palette.brush(QPalette::Button));
        QCommonStyle::drawControl(element, opt, p, widget);
        break;

    case CE_HeaderSection: {
        QBrush fill;
        if (opt->state & State_Sunken)
            fill = opt->palette.brush(QPalette::Mid);
        else if (opt->state & State_On)
            fill = QBrush(opt->palette.mid().color(), Qt::Dense4Pattern);
        else
            fill = opt->palette.brush(QPalette::Button);
        qDrawShadePanel(p, opt->rect, opt->palette, bool(opt->state & State_On),
                        pixelMetric(PM_DefaultFrameWidth), &fill);
        break; }
    case CE_RubberBand:
        p->fillRect(opt->rect, opt->palette.base());
        p->fillRect(opt->rect, QBrush(opt->palette.foreground().color(), Qt::Dense4Pattern));
        break;
    default:
        QCommonStyle::drawControl(element, opt, p, widget);
        break; }
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
                                     const QWidget *widget) const
{
    switch (cc) {
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

            if ((toolbutton->state & State_HasFocus) && (!focus || !focus->isVisible())) {
                QStyleOptionFocusRect fr;
                fr.rect = toolbutton->rect;
                fr.rect.addCoords(3, 3, -3, -3);
                fr.palette = toolbutton->palette;
                fr.state = State_None;
                drawPrimitive(PE_FrameFocusRect, &fr, p, widget);
            }
            drawControl(CE_ToolButtonLabel, toolbutton, p, widget);
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            QStyleOptionSpinBox copy = *sb;
            PrimitiveElement pe;

            if (sb->subControls & SC_SpinBoxFrame) {
                QRect r = visualRect(opt->direction, opt->rect,
                                     subControlRect(CC_SpinBox, sb, SC_SpinBoxFrame, widget));
                qDrawShadePanel(p, r, opt->palette, false, pixelMetric(PM_SpinBoxFrameWidth));
            }

            if (sb->subControls & SC_SpinBoxEditField) {
                int fw = pixelMetric(QStyle::PM_SpinBoxFrameWidth);
                QRect r = visualRect(opt->direction, opt->rect,
                                     subControlRect(CC_SpinBox, sb, SC_SpinBoxEditField, widget)).adjusted(-fw,-fw,fw,fw);
                qDrawShadePanel(p, r, opt->palette, true, fw);
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
                    copy.state |= State_Down;
                } else {
                    copy.state |= State_Raised;
                }
                pe = (sb->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_IndicatorSpinPlus
                                                                       : PE_IndicatorSpinUp);

                copy.rect = visualRect(opt->direction, opt->rect,
                                       subControlRect(CC_SpinBox, sb, SC_SpinBoxUp,
                                                              widget));
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
                    copy.state |= State_Down;
                } else {
                    copy.state |= State_Raised;
                }
                pe = (sb->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_IndicatorSpinMinus
                                                                       : PE_IndicatorSpinDown);

                copy.rect = visualRect(opt->direction, opt->rect,
                                       subControlRect(CC_SpinBox, sb, SC_SpinBoxDown,
                                                              widget));
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

    case CC_Slider:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            QRect groove = subControlRect(CC_Slider, opt, SC_SliderGroove, widget),
                  handle = subControlRect(CC_Slider, opt, SC_SliderHandle, widget);

            if ((opt->subControls & SC_SliderGroove) && groove.isValid()) {
                qDrawShadePanel(p, groove, opt->palette, true, 2, &opt->palette.brush(QPalette::Mid));
                if ((opt->state & State_HasFocus) && (!focus || !focus->isVisible())) {
                    QStyleOption focusOpt = *opt;
                    focusOpt.rect = subRect(SR_SliderFocusRect, opt, widget);
                    drawPrimitive(PE_FrameFocusRect, &focusOpt, p, widget);
                }
            }

            if ((opt->subControls & SC_SliderHandle) && handle.isValid()) {
                QStyleOption bevelOpt = *opt;
                bevelOpt.rect = handle;
                drawPrimitive(PE_PanelButtonBevel, &bevelOpt, p, widget);

                if (slider->orientation == Qt::Horizontal) {
                    int mid = handle.x() + handle.width() / 2;
                    qDrawShadeLine(p, mid, handle.y(), mid, handle.y() + handle.height() - 2,
                                   opt->palette, true, 1);
                } else {
                    int mid = handle.y() + handle.height() / 2;
                    qDrawShadeLine(p, handle.x(), mid, handle.x() + handle.width() - 2, mid, opt->palette,
                                   true, 1);
                }
            }

            if (opt->subControls & SC_SliderTickmarks) {
                QStyleOptionComplex tickOpt = *opt;
                tickOpt.subControls = SC_SliderTickmarks;
                QCommonStyle::drawComplexControl(cc, &tickOpt, p, widget);
            }
        }
        break;

    case CC_ComboBox:
        if (const QStyleOptionComboBox *cb = qt_cast<const QStyleOptionComboBox *>(opt)) {
            if (opt->subControls & SC_ComboBoxArrow) {
                int awh, ax, ay, sh, sy, dh, ew;
                int fw = pixelMetric(PM_DefaultFrameWidth, opt, widget);

                drawPrimitive(PE_PanelButtonCommand, opt, p, widget);

                QRect tr = opt->rect;
                tr.addCoords(fw, fw, -fw, -fw);
                get_combo_parameters(tr, ew, awh, ax, ay, sh, dh, sy);

                QRect ar = QStyle::visualRect(opt->direction, opt->rect, QRect(ax,ay,awh,awh));

                QStyleOption arrowOpt = *opt;
                arrowOpt.rect = ar;
                arrowOpt.state |= State_Enabled;
                drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, p, widget);


                // draws the shaded line under the arrow
                p->setPen(opt->palette.light().color());
                p->drawLine(ar.x(), sy, ar.x()+awh-1, sy);
                p->drawLine(ar.x(), sy, ar.x(), sy+sh-1);
                p->setPen(opt->palette.dark().color());
                p->drawLine(ar.x()+1, sy+sh-1, ar.x()+awh-1, sy+sh-1);
                p->drawLine(ar.x()+awh-1, sy+1, ar.x()+awh-1, sy+sh-1);

                if ((cb->state & State_HasFocus) && (!focus || !focus->isVisible())) {
                    QStyleOptionFocusRect focus;
                    focus.rect = QStyle::visualRect(opt->direction, opt->rect, subRect(SR_ComboBoxFocusRect, opt, widget));
                    focus.palette = opt->palette;
                    focus.backgroundColor = opt->palette.button().color();
                    drawPrimitive(PE_FrameFocusRect, &focus, p, widget);
                }
            }

            if (opt->subControls & SC_ComboBoxEditField) {
                if (cb->editable) {
                    QRect er = QStyle::visualRect(opt->direction, opt->rect,
                                                  subControlRect(CC_ComboBox, opt, SC_ComboBoxEditField, widget));
                    er.addCoords(-1, -1, 1, 1);
                    qDrawShadePanel(p, er, opt->palette, true, 1,
                                    &opt->palette.brush(QPalette::Button));
                }
            }
            p->setPen(opt->palette.buttonText().color());
            }
        break;

    case CC_ScrollBar: {
        if (opt->subControls & SC_ScrollBarGroove)
            qDrawShadePanel(p, opt->rect, opt->palette, true,
                            pixelMetric(PM_DefaultFrameWidth, opt, widget),
                            &opt->palette.brush(QPalette::Mid));
        QCommonStyle::drawComplexControl(cc, opt, p, widget);
        break; }


    case CC_Q3ListView:
        if (opt->subControls & (SC_Q3ListViewBranch | SC_Q3ListViewExpand)) {
            int i;
            if (opt->subControls & SC_Q3ListView)
                QCommonStyle::drawComplexControl(cc, opt, p, widget);
            if (const QStyleOptionQ3ListView *lv = qt_cast<const QStyleOptionQ3ListView *>(lv)) {
                QStyleOptionQ3ListViewItem item = lv->items.at(0);
                int y = opt->rect.y();
                int c;
                QPolygon dotlines;
                if ((opt->activeSubControls & SC_All) && (opt->subControls & SC_Q3ListViewExpand)) {
                    c = 2;
                    dotlines.resize(2);
                    dotlines[0] = QPoint(opt->rect.right(), opt->rect.top());
                    dotlines[1] = QPoint(opt->rect.right(), opt->rect.bottom());
                } else {
                    int linetop = 0, linebot = 0;
                    // each branch needs at most two lines, ie. four end points
                    dotlines.resize(item.childCount * 4);
                    c = 0;

                    // skip the stuff above the exposed rectangle
                    for (i = 1; i < lv->items.size(); ++i) {
                        QStyleOptionQ3ListViewItem child = lv->items.at(i);
                        if (child.height + y > 0)
                            break;
                        y += child.totalHeight;
                    }

                    int bx = opt->rect.width() / 2;

                    // paint stuff in the magical area
                    while (i < lv->items.size() && y < lv->rect.height()) {
                        QStyleOptionQ3ListViewItem child = lv->items.at(i);
                        if (child.features & QStyleOptionQ3ListViewItem::Visible) {
                            int lh;
                            if (!(item.features & QStyleOptionQ3ListViewItem::MultiLine))
                                lh = child.height;
                            else
                                lh = p->fontMetrics().height() + 2 * lv->itemMargin;
                            lh = qMax(lh, QApplication::globalStrut().height());
                            if (lh % 2 > 0)
                                lh++;
                            linebot = y + lh/2;
                            if ((child.features & QStyleOptionQ3ListViewItem::Expandable || child.childCount > 0) &&
                                child.height > 0) {
                                // needs a box
                                p->setPen(opt->palette.text().color());
                                p->drawRect(bx-4, linebot-4, 9, 9);
                                QPolygon a;
                                if ((child.state & State_Open))
                                    a.setPoints(3, bx-2, linebot-2,
                                                bx, linebot+2,
                                                bx+2, linebot-2); //Qt::RightArrow
                                else
                                    a.setPoints(3, bx-2, linebot-2,
                                                bx+2, linebot,
                                                bx-2, linebot+2); //Qt::DownArrow
                                p->setBrush(opt->palette.text());
                                p->drawPolygon(a);
                                p->setBrush(Qt::NoBrush);
                                // dotlinery
                                dotlines[c++] = QPoint(bx, linetop);
                                dotlines[c++] = QPoint(bx, linebot - 5);
                                dotlines[c++] = QPoint(bx + 5, linebot);
                                dotlines[c++] = QPoint(opt->rect.width(), linebot);
                                linetop = linebot + 5;
                            } else {
                                // just dotlinery
                                dotlines[c++] = QPoint(bx+1, linebot);
                                dotlines[c++] = QPoint(opt->rect.width(), linebot);
                            }
                            y += child.totalHeight;
                        }
                        ++i;
                    }

                    // Expand line height to edge of rectangle if there's any
                    // visible child below
                    while (i < lv->items.size() && lv->items.at(i).height <= 0)
                        ++i;
                    if (i < lv->items.size())
                        linebot = opt->rect.height();

                    if (linetop < linebot) {
                        dotlines[c++] = QPoint(bx, linetop);
                        dotlines[c++] = QPoint(bx, linebot);
                    }
                }

                int line; // index into dotlines
                p->setPen(opt->palette.text().color());
                if (opt->subControls & SC_Q3ListViewBranch) for(line = 0; line < c; line += 2) {
                    p->drawLine(dotlines[line].x(), dotlines[line].y(),
                                dotlines[line+1].x(), dotlines[line+1].y());
                }
            }
            break; }

    default:
        QCommonStyle::drawComplexControl(cc, opt, p, widget);
        break;
    }
}


/*! \reimp */
int QMotifStyle::pixelMetric(PixelMetric pm, const QStyleOption *opt,
                             const QWidget *widget) const
{
    int ret;

    switch(pm) {
    case PM_ButtonDefaultIndicator:
        ret = 5;
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
        if (const QStyleOptionSlider *sl = qt_cast<const QStyleOptionSlider *>(opt)) {
            int space = (sl->orientation == Qt::Horizontal) ? sl->rect.height() : sl->rect.width();
            int ticks = sl->tickPosition;
            int n = 0;
            if (ticks & QSlider::Above)
                n++;
            if (ticks & QSlider::Below)
                n++;
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
        }
        break;

    case PM_SliderSpaceAvailable:
        if (const QStyleOptionSlider *sl = qt_cast<const QStyleOptionSlider *>(opt)) {
            if (sl->orientation == Qt::Horizontal)
                ret = sl->rect.width() - pixelMetric(PM_SliderLength, opt, widget) - 6;
            else
                ret = sl->rect.height() - pixelMetric(PM_SliderLength, opt, widget) - 6;
        }
        break;

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
        ret =  QCommonStyle::pixelMetric(pm, opt, widget);
        break;
    }
    return ret;
}


/*!
  \reimp
*/
QRect
QMotifStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                            SubControl sc, const QWidget *widget) const
{
    switch (cc) {
    case CC_SpinBox: {
        int fw = pixelMetric(PM_SpinBoxFrameWidth, opt, widget);
        QSize bs;
        bs.setHeight(opt->rect.height()/2 - fw);
        bs.setWidth(qMin(bs.height() * 8 / 5, opt->rect.width() / 4)); // 1.6 -approximate golden mean
        bs = bs.expandedTo(QApplication::globalStrut());
        int y = fw;
        int x, lx, rx;
        x = opt->rect.width() - y - bs.width();
        lx = fw;
        rx = x - fw * 2;
        const int margin = 4;
        switch (sc) {
        case SC_SpinBoxUp:
            return QRect(x, y, bs.width(), bs.height());
        case SC_SpinBoxDown:
            return QRect(x, y + bs.height(), bs.width(), bs.height());
        case SC_SpinBoxButtonField:
            return QRect(x, y, bs.width(), opt->rect.height() - 2*fw);
        case SC_SpinBoxEditField:
            return QRect(lx + margin, fw + margin, rx - margin, opt->rect.height() - 2*fw - 2 * margin);
        case SC_SpinBoxFrame:
            return opt->rect;
        default:
            break;
        }
        break; }

    case CC_Slider:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            if (sc == SC_SliderHandle) {
                int tickOffset = pixelMetric(PM_SliderTickmarkOffset, opt, widget);
                int thickness = pixelMetric(PM_SliderControlThickness, opt, widget);
                bool horizontal = slider->orientation == Qt::Horizontal;
                int len = pixelMetric(PM_SliderLength, opt, widget);
                int motifBorder = 3;
                int sliderPos = sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderPosition,
                                                        horizontal ? slider->rect.width() - len - 2 * motifBorder
                                                        : slider->rect.height() - len - 2 * motifBorder,
                                                        (!horizontal == !slider->upsideDown));
                if (horizontal)
                    return QRect(sliderPos + motifBorder, tickOffset + motifBorder, len,
                                 thickness - 2 * motifBorder);
                return QRect(tickOffset + motifBorder, sliderPos + motifBorder,
                             thickness - 2 * motifBorder, len);
            }
        }
        break;

    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollbar = qt_cast<const QStyleOptionSlider *>(opt)) {
            int dfw = pixelMetric(PM_DefaultFrameWidth);
            QRect rect =  QCommonStyle::subControlRect(cc, scrollbar, sc, widget);
            if (sc == SC_ScrollBarSlider) {
                if (scrollbar->orientation == Qt::Horizontal)
                    rect.addCoords(-dfw, dfw, dfw, -dfw);
                else
                    rect.addCoords(dfw, -dfw, -dfw, dfw);
            } else if (sc != SC_ScrollBarGroove)
                if (scrollbar->orientation == Qt::Horizontal)
                    rect.addCoords(0, dfw, 0, -dfw);
                else
                    rect.addCoords(dfw, 0, -dfw, 0);
            return rect;
        }
        break;

    case CC_ComboBox:
        switch (sc) {
        case SC_ComboBoxArrow: {
            int ew, awh, sh, dh, ax, ay, sy;
            int fw = pixelMetric(PM_DefaultFrameWidth, opt, widget);
            QRect cr = opt->rect;
            cr.addCoords(fw, fw, -fw, -fw);
            get_combo_parameters(cr, ew, awh, ax, ay, sh, dh, sy);
            return QRect(QPoint(ax, ay), cr.bottomRight()); }

        case SC_ComboBoxEditField: {
            int fw = pixelMetric(PM_DefaultFrameWidth, opt, widget);
            QRect rect = opt->rect;
            rect.addCoords(fw, fw, -fw, -fw);
            int ew = get_combo_extra_width(rect.height(), rect.width());
            rect.addCoords(1, 1, -1-ew, -1);
            return rect; }

        default:
            break;
        }
        break;
    default:
        break;
    }
    return QCommonStyle::subControlRect(cc, opt, sc, widget);
}

/*!
  \reimp
*/
QSize
QMotifStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt,
                              const QSize &contentsSize, const QWidget *widget) const
{
    QSize sz(contentsSize);

    switch(ct) {
    case CT_Splitter:
        sz = QSize(10, 10);
        break;

    case CT_RadioButton:
    case CT_CheckBox:
        sz = QCommonStyle::sizeFromContents(ct, opt, contentsSize, widget);
        sz.rwidth() += motifItemFrame;
        break;

    case CT_PushButton:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            sz = QCommonStyle::sizeFromContents(ct, opt, contentsSize, widget);
            if (!btn->text.isEmpty() && (btn->features & (QStyleOptionButton::AutoDefaultButton|QStyleOptionButton::DefaultButton)))
                sz.setWidth(qMax(75, sz.width()));
            sz += QSize(0, 1); // magical extra pixel
        }
        break;

    case CT_MenuBarItem: {
        if(!sz.isEmpty())
            sz += QSize(2*motifItemHMargin, 2*motifItemVMargin + motifItemFrame);
        break; }

    case CT_MenuItem:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            sz = QCommonStyle::sizeFromContents(ct, opt, sz, widget);
            int w = sz.width(), h = sz.height();

            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                w = 10;
                h = motifSepHeight;
            }

            // a little bit of border can never harm
            w += 2*motifItemHMargin + 2*motifItemFrame;

            if (!mi->text.isNull() && mi->text.indexOf('\t') >= 0)
                // string contains tab
                w += motifTabSpacing;
            else if (mi->menuItemType == QStyleOptionMenuItem::SubMenu)
                // submenu indicator needs some room if we don't have a tab column
                w += motifArrowHMargin + 4*motifItemFrame;

            if (mi->checkType != QStyleOptionMenuItem::NotCheckable && mi->maxIconWidth <= 0)
                // if we are checkable and have no icons, add space for a checkmark
                w += motifCheckMarkSpace;
            else if (mi->checkType != QStyleOptionMenuItem::NotCheckable && mi->maxIconWidth < motifCheckMarkSpace)
                // make sure the check-column is wide enough if we have icons
                w += (motifCheckMarkSpace - mi->maxIconWidth);

            // if we have a check-column (icons of checkmarks), add space
            // to separate the columns
            if (mi->maxIconWidth > 0 || mi->checkType != QStyleOptionMenuItem::NotCheckable)
                w += motifCheckMarkHMargin;

            sz = QSize(w, h);
        }
        break;


    default:
        sz = QCommonStyle::sizeFromContents(ct, opt, contentsSize, widget);
        break;
    }

    return sz;
}

/*!
  \reimp
*/
QRect
QMotifStyle::subRect(SubRect sr, const QStyleOption *opt, const QWidget *widget) const
{
    QRect rect;

    switch (sr) {
    case SR_SliderFocusRect:
        rect = QCommonStyle::subRect(sr, opt, widget);
        rect.addCoords(2, 2, -2, -2);
        break;

    case SR_CheckBoxIndicator:
    case SR_RadioButtonIndicator:
        {
            rect = QCommonStyle::subRect(sr, opt, widget);
            rect.addCoords(motifItemFrame,0, motifItemFrame,0);
        }
        break;

    case SR_ComboBoxFocusRect:
    {
        int awh, ax, ay, sh, sy, dh, ew;
        int fw = pixelMetric(PM_DefaultFrameWidth, opt, widget);
        QRect tr = opt->rect;

        tr.addCoords(fw, fw, -fw, -fw);
        get_combo_parameters(tr, ew, awh, ax, ay, sh, dh, sy);
        rect.setRect(ax-2, ay-2, awh+4, awh+sh+dh+4);
        break;
    }

    case SR_Q3DockWindowHandleRect:
        if (const QStyleOptionQ3DockWindow *dw = qt_cast<const QStyleOptionQ3DockWindow *>(opt)) {
            if (!dw->docked || !dw->closeEnabled)
                rect.setRect(0, 0, opt->rect.width(), opt->rect.height());
            else {
                if (dw->state == State_Horizontal)
                    rect.setRect(2, 15, opt->rect.width()-2, opt->rect.height() - 15);
                else
                    rect.setRect(0, 2, opt->rect.width() - 15, opt->rect.height() - 2);
            }
        }
        break;

    case SR_ProgressBarLabel:
    case SR_ProgressBarGroove:
    case SR_ProgressBarContents:
        if (const QStyleOptionProgressBar *pb = qt_cast<const QStyleOptionProgressBar *>(opt)) {
            int textw = 0;
            if (pb->textVisible)
                textw = pb->fontMetrics.width("100%") + 6;

            if (pb->textAlignment == Qt::AlignAuto || pb->textAlignment == Qt::AlignCenter) {
                rect = opt->rect;
            } else {
                if(sr == SR_ProgressBarLabel)
                    rect.setCoords(opt->rect.right() - textw, opt->rect.top(),
                                   opt->rect.right(), opt->rect.bottom());
                else
                    rect.setCoords(opt->rect.left(), opt->rect.top(),
                                   opt->rect.right() - textw, opt->rect.bottom());
            }
        }
        break;
    case SR_CheckBoxContents: {
        QRect ir = subRect(SR_CheckBoxIndicator, opt, widget);
        rect.setRect(ir.right() + 10, opt->rect.y(),
                     opt->rect.width() - ir.width() - 10, opt->rect.height());
        break; }

    case SR_RadioButtonContents: {
        QRect ir = subRect(SR_RadioButtonIndicator, opt, widget);
        rect.setRect(ir.right() + 10, opt->rect.y(),
                     opt->rect.width() - ir.width() - 10, opt->rect.height());
        break; }

    case SR_CheckBoxClickRect:
    case SR_RadioButtonClickRect:
        rect = opt->rect;
        break;

    default:
        rect = QCommonStyle::subRect(sr, opt, widget);
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
QPixmap
QMotifStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                            const QWidget *widget) const
{
#ifndef QT_NO_IMAGEIO_XPM
    switch (standardPixmap) {
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
        switch (standardPixmap) {
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
            switch (standardPixmap) {
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
            image = pm.toImage();
        }
        return pm;
    }

    default:
        break;
    }
#endif

    return QCommonStyle::standardPixmap(standardPixmap, opt, widget);
}

bool
QMotifStyle::event(QEvent *e)
{
    if(e->type() == QEvent::FocusIn) {
        if (QApplication::focusWidget()) {
            if(focus)
                focus->setWidget(QApplication::focusWidget());
            else
                focus = new QFocusFrame(QApplication::focusWidget());
        } else {
            if(focus)
                focus->setWidget(0);
        }
    } else if(e->type() == QEvent::FocusOut) {
        if(focus)
            focus->setWidget(0);
    }
    return false;
}


/*! \reimp */
int
QMotifStyle::styleHint(StyleHint hint, const QStyleOption *opt, const QWidget *widget,
                           QStyleHintReturn *returnData) const
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

    case SH_ScrollBar_MiddleClickAbsolutePosition:
    case SH_Slider_SloppyKeyEvents:
    case SH_ProgressDialog_CenterCancelButton:
    case SH_Menu_SpaceActivatesItem:
    case SH_ScrollView_FrameOnlyAroundContents:
    case SH_DitherDisabledText:
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

    case SH_MenuBar_DismissOnSecondClick:
        ret = 0;
        break;

    case SH_MessageBox_UseBorderForButtonSpacing:
        ret = 1;
        break;

    default:
        ret = QCommonStyle::styleHint(hint, opt, widget, returnData);
        break;
    }

    return ret;
}


#endif


