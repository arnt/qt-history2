/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qstylesheetstyle_p.h"
#include <qdebug.h>
#include <qapplication.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qpainter.h>
#include <qstyleoption.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qtextedit.h>
#include <qwindowsstyle.h>
#include <qcombobox.h>
#include <qwindowsstyle.h>
#include <qplastiquestyle.h>
#include "private/qcssparser_p.h"

using namespace QCss;

QHash<QWidget *, QVector<QCss::StyleRule> > QStyleSheetStyle::styleRulesCache;
QHash<QWidget *, QHash<int, QRenderRule> > QStyleSheetStyle::renderRulesCache;
///////////////////////////////////////////////////////////////////////////////////////////
#define ceil(x) ((int)(x) + ((x) > 0 && (x) != (int)(x)))

void QRenderRule::merge(const QVector<Declaration>& decls)
{
    bool hasBorderImage = false;
    for (int i = 0; i < decls.count(); i++) {
        const Declaration& decl = decls.at(i);
        switch (decl.propertyId) {
        case Color: palette()->foreground = decl.colorValue(); break;
        case BackgroundColor: palette()->background = decl.colorValue(); break;
        case SelectionForeground:palette()->selectionForeground = decl.brushValue(); break;
        case SelectionBackground: palette()->selectionBackground = decl.brushValue(); break;

        case PaddingLeft: decl.intValue(&box()->paddings[LeftEdge], "px"); break;
        case PaddingRight: decl.intValue(&box()->paddings[RightEdge], "px"); break;
        case PaddingTop: decl.intValue(&box()->paddings[TopEdge], "px"); break;
        case PaddingBottom: decl.intValue(&box()->paddings[BottomEdge], "px"); break;
        case Padding: decl.marginValues(box()->paddings, "px"); break;

        case MarginLeft: decl.intValue(&box()->margins[LeftEdge], "px"); break;
        case MarginRight: decl.intValue(&box()->margins[RightEdge], "px"); break;
        case MarginTop: decl.intValue(&box()->margins[TopEdge], "px"); break;
        case MarginBottom: decl.intValue(&box()->margins[BottomEdge], "px"); break;
        case Margin: decl.marginValues(box()->margins, "px"); break;

        case BorderLeftWidth: decl.intValue(&box()->borders[LeftEdge], "px"); break;
        case BorderRightWidth: decl.intValue(&box()->borders[RightEdge], "px"); break;
        case BorderTopWidth: decl.intValue(&box()->borders[TopEdge], "px"); break;
        case BorderBottomWidth: decl.intValue(&box()->borders[BottomEdge], "px"); break;
        case BorderWidth: decl.marginValues(box()->borders, "px"); break;

        case BorderLeftColor: box()->colors[LeftEdge] = decl.colorValue(); break;
        case BorderRightColor: box()->colors[RightEdge] = decl.colorValue(); break;
        case BorderTopColor: box()->colors[TopEdge] = decl.colorValue(); break;
        case BorderBottomColor: box()->colors[BottomEdge] = decl.colorValue(); break;
        case BorderColor: decl.colorValues(box()->colors); break;

        case BorderTopStyle: box()->styles[TopEdge] = decl.styleValue(); break;
        case BorderBottomStyle: box()->styles[BottomEdge] = decl.styleValue(); break;
        case BorderLeftStyle: box()->styles[LeftEdge] = decl.styleValue(); break;
        case BorderRightStyle: box()->styles[RightEdge] = decl.styleValue(); break;
        case BorderStyles:  decl.styleValues(box()->styles); break;

        case BorderTopLeftRadius: decl.radiusValue(&box()->radii[0], "px"); break;
        case BorderTopRightRadius: decl.radiusValue(&box()->radii[1], "px"); break;
        case BorderBottomLeftRadius: decl.radiusValue(&box()->radii[2], "px"); break;
        case BorderBottomRightRadius: decl.radiusValue(&box()->radii[3], "px"); break;
        case BorderRadius: decl.radiiValues(box()->radii, "px"); break;

        case BackgroundOrigin: background()->origin = decl.originValue(); break;
        case BackgroundRepeat: background()->repeat = decl.repeatValue(); break;
        case BackgroundPosition: background()->position = decl.alignmentValue(); break;
        case BackgroundImage: background()->pixmap = QPixmap(decl.uriValue()); break;

        case BorderImage: decl.borderImageValue(&box()->borderImage()->pixmap,
                                                box()->borderImage()->cuts,
                                                &box()->borderImage()->horizStretch,
                                                &box()->borderImage()->vertStretch);
                          hasBorderImage = true;
                          break;
        default:
            if (decl.property.compare("exclusive-indicator", Qt::CaseInsensitive) == 0) {
                pixmaps["exclusive-indicator"] = QPixmap(decl.uriValue());
            } else if (decl.property.compare("combobox-arrow", Qt::CaseInsensitive) == 0
                       || decl.property.compare("down-arrow", Qt::CaseInsensitive) == 0) {
                pixmaps["down-arrow"] = QPixmap(decl.uriValue());
            }
            break;
        }
    }

    if (hasBorderImage) {
        // inspect the border image
        QStyleSheetBorderImageData *borderImage = box()->borderImage();
        const QPixmap& pixmap = borderImage->pixmap;
        if (pixmap.isNull()) {
            box()->bi = 0; // delete it
            return;
        }

        if (borderImage->cuts[0] == -1) {
            for (int i = 0; i < 4; i++) // cut = border
                borderImage->cuts[i] = box()->borders[i];
        } else {
            for (int i = 0; i < 4; i++) // border = cut
                box()->borders[i] = borderImage->cuts[i];
        }
        borderImage->cutBorderImage();
    }
}

QRect QRenderRule::borderRect(const QRect& r) const
{
    const int* m = box()->margins;
    return r.adjusted(m[LeftEdge], m[TopEdge], -m[RightEdge], -m[BottomEdge]);
}

QRect QRenderRule::paddingRect(const QRect& r) const
{
    QRect br = borderRect(r);
    const int *b= box()->borders;
    return br.adjusted(b[LeftEdge], b[TopEdge], -b[RightEdge], -b[BottomEdge]);
}

QRect QRenderRule::contentsRect(const QRect& r) const
{
    QRect pr = paddingRect(r);
    const int *p = box()->paddings;
    return pr.adjusted(p[LeftEdge], p[TopEdge], -p[RightEdge], -p[BottomEdge]);
}

QRect QRenderRule::boxRect(const QRect& cr) const
{
    const int *m = box()->margins;
    const int *b = box()->borders;
    const int *p = box()->paddings;
    return cr.adjusted(-p[LeftEdge] - b[LeftEdge] - m[LeftEdge],
                       -p[TopEdge] - b[TopEdge] - m[TopEdge],
                        p[RightEdge] + b[RightEdge] + m[RightEdge],
                        p[BottomEdge] + b[BottomEdge] + m[BottomEdge]);
}

void QStyleSheetBorderImageData::cutBorderImage()
{
    const int w = pixmap.width();
    const int h = pixmap.height();
    const int &l = cuts[LeftEdge], &r = cuts[RightEdge],
              &t = cuts[TopEdge], &b = cuts[BottomEdge];

    // Perform step 1
    topEdgeRect = QRect(l, 0, w - r - l, t);
    bottomEdgeRect = QRect(l, h - b, w - l - r, b);
    if (horizStretch != TileMode_Stretch) {
        if (topEdgeRect.isValid())
            topEdge = pixmap.copy(topEdgeRect).scaledToHeight(t);
        if (bottomEdgeRect.isValid())
            bottomEdge = pixmap.copy(bottomEdgeRect).scaledToHeight(b);
    }

    leftEdgeRect = QRect(0, t, l, h - b - t);
    rightEdgeRect = QRect(w - r, t, r, h - t- b);
    if (vertStretch != TileMode_Stretch) {
        if (leftEdgeRect.isValid())
            leftEdge = pixmap.copy(leftEdgeRect).scaledToWidth(l);
        if (rightEdgeRect.isValid())
            rightEdge = pixmap.copy(rightEdgeRect).scaledToWidth(r);
    }

    middleRect = QRect(l, t, w - r -l, h - t - b);
    if (middleRect.isValid()
        && !(horizStretch == TileMode_Stretch && vertStretch == TileMode_Stretch)) {
        middle = pixmap.copy(middleRect);
    }
}

///////////////////////////////////////////////////////////////////////////////////
static QPen qPenFromStyle(const QBrush& b, qreal width, BorderStyle s)
{
    Qt::PenStyle ps = Qt::NoPen;

    switch (s) {
    case BorderStyle_Dotted:
        ps  = Qt::DotLine;
        break;
    case BorderStyle_Dashed:
        ps = width == 1 ? Qt::DotLine : Qt::DashLine;
        break;
    case BorderStyle_DotDash:
        ps = Qt::DashDotLine;
        break;
    case BorderStyle_DotDotDash:
        ps = Qt::DashDotDotLine;
        break;
    case BorderStyle_Inset:
    case BorderStyle_Outset:
    case BorderStyle_Solid:
        ps = Qt::SolidLine;
        break;
    default:
        break;
    }

    return QPen(b, width, ps);
}

static void qDrawRoundedCorners(QPainter *p, qreal x1, qreal y1, qreal x2, qreal y2,
                                Edge edge, const QColor& b, BorderStyle s,
                                const QSizeF& r1, const QSizeF& r2)
{
    const qreal pw = (edge == TopEdge || edge == BottomEdge) ? y2-y1 : x2-x1;
    if (s == BorderStyle_Double) {
        qreal wby3 = pw/3;
        switch (edge) {
        case TopEdge:
        case BottomEdge:
            qDrawRoundedCorners(p, x1, y1, x2, y1+wby3, edge, 
                                b, BorderStyle_Solid, r1, r2);
            qDrawRoundedCorners(p, x1, y2-wby3, x2, y2, edge, 
                                b, BorderStyle_Solid, r1, r2);
            break;
        case LeftEdge:
            qDrawRoundedCorners(p, x1, y1+1, x1+wby3, y2, LeftEdge, 
                                b, BorderStyle_Solid, r1, r2);
            qDrawRoundedCorners(p, x2-wby3, y1+1, x2, y2, LeftEdge, 
                                b, BorderStyle_Solid, r1, r2);
            break;
        case RightEdge:
            qDrawRoundedCorners(p, x1, y1+1, x1+wby3, y2, RightEdge, 
                                b, BorderStyle_Solid, r1, r2);
            qDrawRoundedCorners(p, x2-wby3, y1+1, x2, y2, RightEdge, 
                                b, BorderStyle_Solid, r1, r2);
            break;
        default:
            break;
        }
        return;
    }

    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    qreal pwby2 = pw/2;
    p->setBrush(Qt::NoBrush);
    p->setPen(qPenFromStyle(b, pw, s));
    switch (edge) {
    case TopEdge:
        if (!r1.isEmpty())
            p->drawArc(QRectF(x1 - r1.width() + pwby2, y1 + pwby2,
                       2*r1.width() - pw, 2*r1.height() - pw), 90 * 16, 45 * 16);
        if (!r2.isEmpty())
            p->drawArc(QRectF(x2 - r2.width() + pwby2, y1 + pwby2,
                       2*r2.width() - pw, 2*r2.height() - pw), 45* 16, 45 * 16);
        break;
    case BottomEdge:
        if (!r1.isEmpty())
            p->drawArc(QRectF(x1 - r1.width() + pwby2, y2 - 2 * r1.height() + pwby2,
                              2*r1.width() - pw, 2*r1.height() - pw), -90 * 16, -45 * 16);
        if (!r2.isEmpty())
            p->drawArc(QRectF(x2 - r2.width() + pwby2, y2 - 2 * r2.height() + pwby2,
                       2*r2.width() - pw, 2*r2.height() - pw), -90 * 16, 45 * 16);
        break;
    case LeftEdge:
        if (!r1.isEmpty())
            p->drawArc(QRectF(x1 + pwby2, y1 - r1.height() + pwby2,
                       2*r1.width() - pw, 2*r1.height() - pw), 135 * 16, 45 * 16);
        if (!r2.isEmpty())
            p->drawArc(QRectF(x1 + pwby2, y2 - r2.height() + pwby2,
                       2*r2.width() - pw, 2*r2.height() - pw), 180 * 16, 45 * 16);
        break;
    case RightEdge:
        if (!r1.isEmpty())
            p->drawArc(QRectF(x2 - 2*r1.width() + pwby2, y1 - r1.height() + pwby2,
                       2*r1.width() - pw, 2*r1.height() - pw), 0 * 16, 45 * 16);
        if (!r2.isEmpty())
            p->drawArc(QRectF(x2 - 2*r2.width() + pwby2, y2 - r2.height() + pwby2,
                       2*r2.width() - pw, 2*r2.height() - pw), 0 * 16, -45 * 16);
        break;
    default:
        break;
    }
    p->restore();
}

// Draws edge for the rect (x1, y1) - (x2 - 1, y2 - 1)
void qDrawEdge(QPainter *p, qreal x1, qreal y1, qreal x2, qreal y2,
                Edge edge, QColor c, BorderStyle style, qreal dw1, qreal dw2)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    const qreal width = (edge == TopEdge || edge == BottomEdge) ? y2-y1 : x2-x1;

    switch (style) {
    case BorderStyle_Inset:
    case BorderStyle_Outset:
        if (style == BorderStyle_Inset && (edge == TopEdge || edge == LeftEdge)
            || (style == BorderStyle_Outset && (edge == BottomEdge || edge == RightEdge)))
            c = c.dark();
        // fall through!
    case BorderStyle_Solid: {
        p->setPen(Qt::NoPen);
        p->setBrush(c);
        if (dw1 == 0 && dw2 == 0) {
            p->drawRect(QRectF(x1, y1, x2 - x1, y2 - y1));
        } else { // draw trapezoids
            QPolygonF quad;
            switch (edge) {
            case TopEdge:
                quad << QPointF(x1, y1) << QPointF(x1 + dw1, y2)
                    << QPointF(x2 - dw2, y2) << QPointF(x2, y1);
                break;
            case BottomEdge:
                quad << QPointF(x1 + dw1, y1) << QPointF(x1, y2)
                    << QPointF(x2, y2) << QPointF(x2 - dw2, y1);
                break;
            case LeftEdge:
                quad << QPointF(x1, y1) << QPointF(x1, y2)
                    << QPointF(x2, y2 - dw2) << QPointF(x2, y1 + dw1);
                break;
            case RightEdge:
                quad << QPointF(x1, y1 + dw1) << QPointF(x1, y2 - dw2)
                    << QPointF(x2, y2) << QPointF(x2, y1);
                break;
            default:
                break;
            }
            p->drawConvexPolygon(quad);
        }
        break;
    }
    case BorderStyle_Dotted:
    case BorderStyle_Dashed:
    case BorderStyle_DotDash:
    case BorderStyle_DotDotDash:
        p->setPen(qPenFromStyle(c, width, style));
        if (edge == TopEdge || edge == BottomEdge)
            p->drawLine(QLineF(x1 + width/2, (y1 + y2)/2, x2 - width/2, (y1 + y2)/2));
        else
            p->drawLine(QLineF((x1+x2)/2, y1 + width/2, (x1+x2)/2, y2 - width/2));
        break;

    case BorderStyle_Double: {
        qreal wby3 = width/3;
        if (dw1 == 0 && dw2 == 0) { // not a trapezoid
            p->setBrush(c);
            p->setPen(Qt::NoPen);
            switch(edge) {
            case TopEdge:
            case BottomEdge:
                p->drawRect(QRectF(x1, y1, x2-x1, wby3));
                p->drawRect(QRectF(x1, y2-wby3, x2-x1, wby3));
                break;
            case LeftEdge:
                p->drawRect(QRectF(x1, y1+1, wby3, y2-y1-1));
                p->drawRect(QRectF(x2-wby3, y1+1, wby3, y2-y1-1));
                break;
            case RightEdge:
                p->drawRect(QRectF(x1, y1+1, wby3, y2-y1-1));
                p->drawRect(QRectF(x2-wby3, y1+1, wby3, y2-y1-1));
                break;
            default:
                break;
            }
        } else {
            qreal dw1by3 = dw1/3;
            qreal dw2by3 = dw2/3;
            switch (edge) {
            case TopEdge:
                qDrawEdge(p, x1, y1, x2, y1 + wby3, TopEdge, c, 
                          BorderStyle_Solid, dw1by3, dw2by3);
                qDrawEdge(p, x1 + dw1 - dw1by3, y2 - wby3, x2 - dw2 + dw1by3, y2, TopEdge, c,
                          BorderStyle_Solid, dw1by3, dw2by3);
                break;
            case LeftEdge:
                qDrawEdge(p, x1, y1, x1 + wby3, y2, LeftEdge, c, 
                          BorderStyle_Solid, dw1by3, dw2by3);
                qDrawEdge(p, x2 - wby3, y1 + dw1 - dw1by3, x2, y2 - dw2 + dw2by3, LeftEdge, c,
                          BorderStyle_Solid, dw1by3, dw2by3);
                break;
            case BottomEdge:
                qDrawEdge(p, x1 + dw1 - dw1by3, y1, x2 - dw2 + dw2by3, y1 + wby3, BottomEdge,
                          c, BorderStyle_Solid, dw1by3, dw2by3);
                qDrawEdge(p, x1, y2 - wby3, x2, y2 , BottomEdge, c, 
                          BorderStyle_Solid, dw1by3, dw2by3);
                break;
            case RightEdge:
                qDrawEdge(p, x2 - wby3, y1, x2, y2, RightEdge, c, 
                          BorderStyle_Solid, dw1by3, dw2by3);
                qDrawEdge(p, x1, y1 + dw1 - dw1by3, x1 + wby3, y2 - dw2 + dw2by3, RightEdge, 
                          c, BorderStyle_Solid, dw1by3, dw2by3);
                break;
            default:
                break;
            }
        }
        break;
    }
    case BorderStyle_Ridge:
    case BorderStyle_Groove: {
        BorderStyle s1, s2;
        if (style == BorderStyle_Groove) {
            s1 = BorderStyle_Inset;
            s2 = BorderStyle_Outset;
        } else {
            s1 = BorderStyle_Outset;
            s2 = BorderStyle_Inset;
        }
        qreal dw1by2 = dw1/2, dw2by2 = dw2/2;
        qreal wby2 = width/2;
        switch (edge) {
        case TopEdge:
            qDrawEdge(p, x1, y1, x2, y1 + wby2, 
                      TopEdge, c, s1, dw1by2, dw2by2);
            qDrawEdge(p, x1 + dw1by2, y1 + wby2, x2 - dw2by2, y2, 
                      TopEdge, c, s2, dw1by2, dw2by2);
            break;
        case BottomEdge:
            qDrawEdge(p, x1, y1 + wby2, x2, y2, 
                      BottomEdge, c, s1, dw1by2, dw2by2);
            qDrawEdge(p, x1 + dw1by2, y1, x2 - dw2by2, y1 + wby2, 
                      BottomEdge, c, s2, dw1by2, dw2by2);
            break;
        case LeftEdge:
            qDrawEdge(p, x1, y1, x1 + wby2, y2, 
                      LeftEdge, c, s1, dw1by2, dw2by2);
            qDrawEdge(p, x1 + wby2, y1 + dw1by2, x2, y2 - dw2by2, 
                      LeftEdge, c, s2, dw1by2, dw2by2);
            break;
        case RightEdge:
            qDrawEdge(p, x1 + wby2, y1, x2, y2, 
                      RightEdge, c, s1, dw1by2, dw2by2);
            qDrawEdge(p, x1, y1 + dw1by2, x1 + wby2, y2 - dw2by2, 
                      RightEdge, c, s2, dw1by2, dw2by2);
            break;
        default:
            break;
        }
    }
    case BorderStyle_None:
    case BorderStyle_Unknown:
    default:
        break;
    }
    p->restore();
}

// Determines if Edge e1 with Style s1 paints over Edge e2 with Style s2
static bool qPaintsOver(Edge e1, BorderStyle s1, Edge, BorderStyle)
{
    // Outset always wins
    return (s1 == BorderStyle_Outset && (e1 == TopEdge || e1 == LeftEdge))
            || (s1== BorderStyle_Inset && (e1 == BottomEdge || e1 == RightEdge));
}

// FIXME:
// 1. Middle image with Round
// 2. Should Repeat centerize the tile?
static void qDrawBorderImage(QPainter *p, const QRenderRule &rule, const QRect& rect)
{
    const QRect br = rule.borderRect(rect);
    const QRect pr = rule.paddingRect(rect);
    const QStyleSheetBorderImageData* bi = rule.box()->borderImage();
    const int *borders = rule.box()->borders;
    const int& l = borders[LeftEdge];
    const int& r = borders[RightEdge];
    const int& t = borders[TopEdge];
    const int& b = borders[BottomEdge];

    const QPixmap& pix = bi->pixmap;

    const int *c = bi->cuts;
    QRect tlc = QRect(0, 0, c[LeftEdge], c[TopEdge]);
    if (tlc.isValid())
        p->drawPixmap(QRect(br.topLeft(), QSize(l, t)), pix, tlc);
    QRect trc = QRect(pix.width() - c[RightEdge], 0, c[RightEdge], c[LeftEdge]);
    if (trc.isValid())
        p->drawPixmap(QRect(br.left() + br.width() - r, br.y(), l, t), pix, trc);
    QRect blc = QRect(0, pix.height() - c[BottomEdge], c[LeftEdge], c[BottomEdge]);
    if (blc.isValid())
        p->drawPixmap(QRect(br.x(), br.y() + br.height() - b, l, b), pix, blc);
    QRect brc = QRect(pix.width() - c[RightEdge], pix.height() - c[BottomEdge],
                      c[RightEdge], c[BottomEdge]);
    if (brc.isValid())
        p->drawPixmap(QRect(br.x() + br.width() - r, br.y() + br.height() - b, r, b), pix, brc);

    int rwh, blank;

    switch (bi->horizStretch) {
    case TileMode_Stretch:
        if (bi->topEdgeRect.isValid())
            p->drawPixmap(QRect(br.x() + l, br.y(), pr.width(), t), pix, bi->topEdgeRect);
        if (bi->bottomEdgeRect.isValid())
            p->drawPixmap(QRect(br.x() + l, br.y() + br.height() - b, pr.width(), b), 
                          pix, bi->bottomEdgeRect);
        if (bi->middleRect.isValid()) {
            if (bi->vertStretch == TileMode_Stretch)
                p->drawPixmap(pr, pix, bi->middleRect);
            else if (bi->vertStretch == TileMode_Repeat) {
                QPixmap scaled = bi->middle.scaled(pr.width(), bi->middle.height());
                p->drawTiledPixmap(pr, scaled);
            }
        }
        break;
    case TileMode_Repeat:
        if (!bi->topEdge.isNull())
            p->drawTiledPixmap(QRect(br.x() + l, br.y(), pr.width(), t), bi->topEdge);
        if (!bi->bottomEdge.isNull())
            p->drawTiledPixmap(QRect(br.x() + l, br.y() + br.height() -b, pr.width(), b),
                               bi->bottomEdge);
        if (bi->vertStretch == TileMode_Repeat && !bi->middle.isNull()) {
            p->drawTiledPixmap(pr, bi->middle);
        } else if (bi->vertStretch == TileMode_Stretch && bi->middleRect.isValid()) {
            QPixmap scaled = bi->middle.scaled(bi->middle.width(), pr.height());
            p->drawTiledPixmap(pr, scaled);
        }
        break;
    case TileMode_Round:
        if (!bi->topEdge.isNull()) {
            rwh = pr.width()/ceil(pr.width()/bi->topEdge.width());
            QPixmap scaled = bi->topEdge.scaled(rwh, bi->topEdge.height());
            blank = pr.width() % rwh;
            p->drawTiledPixmap(QRectF(br.x() + l + blank/2, br.y(), pr.width() - blank, t),
                               scaled);
        }
        if (!bi->bottomEdge.isNull()) {
            rwh = pr.width()/ceil(pr.width()/bi->bottomEdge.width());
            QPixmap scaled = bi->bottomEdge.scaled(rwh, bi->bottomEdge.height());
            blank = pr.width() % rwh;
            p->drawTiledPixmap(QRectF(br.x() + l+ blank/2, br.y()+br.height()-b, pr.width() - blank, b),
                               scaled);
        }
        break;
    default:
        break;
    }

    switch (bi->vertStretch) {
    case TileMode_Stretch:
        if (bi->leftEdgeRect.isValid())
             p->drawPixmap(QRect(br.x(), br.y() + t, l, pr.height()), pix, bi->leftEdgeRect);
        if (bi->rightEdgeRect.isValid())
            p->drawPixmap(QRect(br.x() + br.width()- r, br.y() + t, r, pr.height()), 
                          pix, bi->rightEdgeRect);
        break;
    case TileMode_Repeat:
        if (!bi->leftEdge.isNull())
            p->drawTiledPixmap(QRect(br.x(), br.y() + t, l, pr.height()), bi->leftEdge);
        if (!bi->rightEdge.isNull())
            p->drawTiledPixmap(QRect(br.x() + br.width() - r, br.y() + t, r, pr.height()),
                                bi->rightEdge);
        break;
    case TileMode_Round:
        if (!bi->leftEdge.isNull()) {
            rwh = pr.height()/ceil(pr.height()/bi->leftEdge.height());
            QPixmap scaled = bi->leftEdge.scaled(bi->leftEdge.width(), rwh);
            blank = pr.height() % rwh;
            p->drawTiledPixmap(QRectF(br.x(), br.y() + t + blank/2, l, pr.height() - blank),
                               scaled);
        }
        if (!bi->rightEdge.isNull()) {
            rwh = pr.height()/ceil(pr.height()/bi->rightEdge.height());
            QPixmap scaled = bi->rightEdge.scaled(bi->rightEdge.width(), rwh);
            blank = pr.height() % rwh;
            p->drawTiledPixmap(QRectF(br.x() + br.width() - r, br.y()+t+blank/2, r, pr.height() - blank),
                               scaled);
        }
        break;
    default:
        break;
    }
}

static void qDrawBackground(QPainter *p, const QRenderRule &rule, const QRect& rect,
                             Qt::LayoutDirection dir)
{
    if (rule.palette()->background.style() != Qt::NoBrush)
        p->fillRect(rule.borderRect(rect), rule.palette()->background);
    if (!rule.hasBackground())
        return;
    QRect r;
    QStyleSheetBackgroundData *background = rule.background();
    switch (background->origin) {
        case Origin_Padding:
            r = rule.paddingRect(rect);
            break;
        case Origin_Border:
            r = rule.borderRect(rect);
            break;
        case Origin_Content:
            r = rule.contentsRect(rect);
            break;
        default:
            break;
    }
    const QPixmap& bgp = background->pixmap;
    if (bgp.isNull())
        return;
    QRect aligned = QStyle::alignedRect(dir, (Qt::Alignment)background->position,
                                        bgp.size(), r);
    QRect inter = aligned.intersect(r);

    switch (background->repeat) {
    case Repeat_None:
        p->drawPixmap(inter.x(), inter.y(), bgp, inter.x() - aligned.x(),
                      inter.y() - aligned.y(), inter.width(), inter.height());
        break;
    case Repeat_Y:
        p->drawTiledPixmap(inter.x(), r.y(), inter.width(), r.height(), bgp,
                           0, qMax(((inter.y() - r.y()) % bgp.height()) - 1, 0));
        break;
    case Repeat_X:
        p->drawTiledPixmap(r.x(), inter.y(), r.width(), inter.height(), bgp,
                           qMax(((inter.x() - r.x()) % bgp.width()) - 1, 0), 0);
        break;
    case Repeat_XY:
        p->drawTiledPixmap(r, bgp, QPoint(qMax(((inter.x() - r.x()) % bgp.width()) - 1, 0),
                                          qMax(((inter.y() - r.y()) % bgp.height()) - 1, 0)));
        break;
    default:
        break;
    }
}

static void qDrawBorder(QPainter *p, const QRenderRule& rule, const QRect& rect)
{
    const QRect br = rule.borderRect(rect);
    QStyleSheetBoxData *box = rule.box();
    if (box->hasBorderImage()) {
        qDrawBorderImage(p, rule, rect);
        return;
    }

    const BorderStyle *styles = box->styles;
    const QColor *colors = box->colors;
    const QRect pr = rule.paddingRect(rect);
    bool rounded_corners = false;

    QSize tlr(0, 0), trr(0, 0), brr(0, 0), blr(0, 0);
    if (box->radii[0].isValid()) {
        tlr = box->radii[0];
        trr = box->radii[1];
        blr = box->radii[2];
        brr = box->radii[3];
        int rw = qMax(tlr.width() + trr.width(), blr.width() + brr.width());
        int rh = qMax(tlr.height() + trr.height(), blr.height() + brr.height());
        if (br.height() >= rh && br.width() >= rw)
            rounded_corners = true;
    }

    // Drawn in order of precendence
    if (styles[BottomEdge] != BorderStyle_None) {
        bool blc = rounded_corners && blr.width() != 0;
        bool brc = rounded_corners && brr.width() != 0;
        int dw1 = (blc || qPaintsOver(BottomEdge, styles[BottomEdge], LeftEdge,
                                      styles[LeftEdge])) ? 0 : pr.x() - br.x();
        int dw2 = (brc || qPaintsOver(BottomEdge, styles[BottomEdge], RightEdge,
                                      styles[RightEdge])) ? 0 : br.right() - pr.right();

        int x1 = br.x() + (blc ? blr.width() : 0);
        int x2 = br.x() + br.width() - (brc ? brr.width() : 0);

        qDrawEdge(p, x1, pr.y() + pr.height(), x2, br.y() + br.height(), BottomEdge,
                  colors[BottomEdge], styles[BottomEdge], dw1, dw2);
        if (blc || brc)
            qDrawRoundedCorners(p, x1, pr.y() + pr.height(), x2, br.y() + br.height(), BottomEdge,
                                 colors[BottomEdge], styles[BottomEdge], blr, brr);
    }
    if (styles[RightEdge] != BorderStyle_None) {
        bool trc = rounded_corners && trr.height() != 0;
        bool brc = rounded_corners && brr.height() != 0;
        int dw1 = (trc || qPaintsOver(RightEdge, styles[RightEdge], TopEdge, 
                                      styles[TopEdge])) ? 0 : pr.y() - br.y();
        int dw2 = (brc || qPaintsOver(RightEdge, styles[RightEdge], BottomEdge,
                                      styles[BottomEdge])) ? 0 : br.bottom() - pr.bottom();
        int y1 = br.y() + (trc ? trr.height() : 0);
        int y2 = br.y() + br.height() - (brc ? brr.height() : 0);

        qDrawEdge(p, pr.x() + pr.width(), y1, br.x() + br.width(), y2, RightEdge, colors[RightEdge], styles[RightEdge], dw1, dw2);
        if (trc || brc)
            qDrawRoundedCorners(p, pr.right(), y1, br.right(), y2, RightEdge, colors[RightEdge],
                                 styles[RightEdge], trr, brr);
    }
    if (styles[LeftEdge] != BorderStyle_None) {
        bool tlc = rounded_corners && tlr.height() != 0;
        bool blc = rounded_corners && blr.height() != 0;
        int dw1 = (tlc || qPaintsOver(LeftEdge, styles[LeftEdge], TopEdge, styles[TopEdge])) ? 0 : pr.y() - br.y();
        int dw2 = (blc || qPaintsOver(LeftEdge, styles[LeftEdge], BottomEdge, styles[BottomEdge])) ? 0 : br.bottom() - pr.bottom();
        int y1 = br.y() + (tlc ? tlr.height() : 0);
        int y2 = br.y() + br.height() - (blc ? blr.height() : 0);

        qDrawEdge(p, br.x(), y1, pr.x(), y2, LeftEdge, colors[LeftEdge], styles[LeftEdge], dw1, dw2);
        if (tlc || blc)
            qDrawRoundedCorners(p, br.x(), y1, pr.x(), y2, LeftEdge, colors[LeftEdge], styles[LeftEdge], tlr, blr);
    }
    if (styles[TopEdge] != BorderStyle_None) {
        bool tlc = rounded_corners && tlr.width() != 0;
        bool trc = rounded_corners && trr.width() != 0;
        int dw1 = (tlc || qPaintsOver(TopEdge, styles[TopEdge], LeftEdge, styles[LeftEdge])) ? 0 : pr.x() - br.x();
        int dw2 = (trc || qPaintsOver(TopEdge, styles[TopEdge], RightEdge, styles[RightEdge])) ? 0 : br.right() - pr.right();
        int x1 = br.x() + (tlc ? tlr.width() : 0);
        int x2 = br.x() + br.width() - (trc ? trr.width() : 0);

        qDrawEdge(p, x1, br.y(), x2, pr.y(), TopEdge, colors[TopEdge], styles[TopEdge], dw1, dw2);
        if (tlc || trc)
            qDrawRoundedCorners(p, x1, br.y(), x2, pr.y(), TopEdge, colors[TopEdge], styles[TopEdge], tlr, trr);
    }
}

static void qDrawFrame(QPainter *p, const QRenderRule &rule,
                       const QRect& rect, Qt::LayoutDirection dir)
{
    qDrawBackground(p, rule, rect, dir);
    if (rule.hasBox())
        qDrawBorder(p, rule, rect);
}

static void qConfigurePalette(QPalette *p, const QRenderRule &rule,
                              QPalette::ColorRole fg, QPalette::ColorRole bg)
{
    QStyleSheetPalette *rp = rule.palette();
    if (rp->foreground.style() != Qt::NoBrush) {
        p->setBrush(fg, rp->foreground);
        p->setBrush(QPalette::WindowText, rp->foreground);
    }
    if (rp->background.style() != Qt::NoBrush) {
        p->setBrush(bg, rp->background);
        p->setBrush(QPalette::Window, rp->background);
    }
    if (rp->selectionBackground.style() != Qt::NoBrush)
        p->setBrush(QPalette::Highlight, rp->selectionBackground);
    if (rp->selectionForeground.style() != Qt::NoBrush)
        p->setBrush(QPalette::HighlightedText, rp->selectionForeground);
    if (rp->alternateBackground.style() != Qt::NoBrush)
        p->setBrush(QPalette::AlternateBase, rp->alternateBackground);
}

#define WIDGET(x) (static_cast<QWidget *>(x.ptr))

class QStyleSheetStyleSelector : public StyleSelector
{
public:
    QStyleSheetStyleSelector() { }

    bool hasNodeName(NodePtr node, const QString& name) const
    { return WIDGET(node)->inherits(name.toLatin1()); }
    QString attribute(NodePtr node, const QString& name) const
    {
        return (name == "class")
            ? WIDGET(node)->metaObject()->className()
            : WIDGET(node)->property(name.toLatin1()).toString();
    }
    bool hasAttribute(NodePtr node, const QString& name) const
    { return name == "class" 
             || WIDGET(node)->metaObject()->indexOfProperty(name.toLatin1()) != -1; }
    bool hasAttributes(NodePtr) const
    { return true; }
    QStringList nodeIds(NodePtr node) const
    { return QStringList(WIDGET(node)->objectName()); }
    bool isNullNode(NodePtr node) const
    { return node.ptr == 0; }
    NodePtr parentNode(NodePtr node)
    { NodePtr n; n.ptr = WIDGET(node)->parentWidget(); return n; }
    NodePtr previousSiblingNode(NodePtr)
    { NodePtr n; n.ptr = 0; return n; }
    NodePtr duplicateNode(NodePtr node)
    { return node; }
    void freeNode(NodePtr)
    { }
};

/////////////////////////////////////////////////////////////////////////////////////////
QRenderRule QStyleSheetStyle::renderRule(QWidget *w, QStyle::State state)
{
    int pseudoState = (state & QStyle::State_Enabled)
                                ? PseudoState_Enabled : PseudoState_Disabled;
    if (state & QStyle::State_Sunken)
        pseudoState |= PseudoState_Pressed;
    if (state & QStyle::State_MouseOver)
        pseudoState |= PseudoState_Hover;
    if (state & QStyle::State_HasFocus)
        pseudoState |= PseudoState_Focus;
    if (state & QStyle::State_On)
        pseudoState |= PseudoState_Checked;
    if (state & QStyle::State_Off)
        pseudoState |= PseudoState_Unchecked;
    if (state & QStyle::State_NoChange)
        pseudoState |= PseudoState_Indeterminate;

    QHash<int, QRenderRule>& renderingRules = renderRulesCache[w];

    if (renderingRules.contains(pseudoState))
        return renderingRules.value(pseudoState);

    Q_ASSERT(styleRulesCache.contains(w)); // style sheet rules must have been computed!
    const QVector<StyleRule>& rules = styleRulesCache.value(w);
    QRenderRule newRule;
    for (int i = 0; i < rules.count(); i++) {
        int cssState = rules.at(i).selectors.at(0).pseudoState();
        if ((cssState == PseudoState_Unspecified) || (cssState & pseudoState) == cssState)
            newRule.merge(rules.at(i).declarations);
    }

    renderingRules[pseudoState] = newRule;
    return newRule;
}

QRenderRule QStyleSheetStyle::renderRule(const QWidget *w, const QStyleOption *opt)
{
    if (!w)
        return QRenderRule();

    if (!styleRulesCache.contains(const_cast<QWidget *>(w)))
        computeStyleSheet(const_cast<QWidget *>(w));

    QStyle::State state = opt ? opt->state : QStyle::State(QStyle::State_Enabled);

    // Add hacks for <w, opt> here
    if (qobject_cast<const QLineEdit *>(w))
        state &= ~QStyle::State_Sunken;

    return renderRule(const_cast<QWidget *>(w), state);
}

void QStyleSheetStyle::setPalette(QWidget *w)
{
    const QRenderRule& hoverRule = renderRule(w, QStyle::State_MouseOver);
    if (!hoverRule.isEmpty())
        w->setAttribute(Qt::WA_Hover);

    struct ruleRoleMap {
        QStyle::StateFlag state;
        QPalette::ColorGroup group;
    };
    ruleRoleMap map[2] = {
        { QStyle::State_Enabled, QPalette::Active },
        { QStyle::State_None, QPalette::Disabled }
    };

    QPalette p = qApp->palette(w);
    for (int i = 0; i < 2; i++) {
        const QRenderRule& rule = renderRule(w, map[i].state);
        p.setCurrentColorGroup(map[i].group);

        if (qobject_cast<QMenu *>(w)) {
            qConfigurePalette(&p, rule, QPalette::ButtonText, QPalette::Button);
        } else if (qobject_cast<QComboBox *>(w)
                   || qobject_cast<QMenuBar *>(w)) {
                    // FIXME adjust the palette here.
        } else if (qobject_cast<QLineEdit *>(w)) {
            qConfigurePalette(&p, rule, QPalette::Text, QPalette::Base);
        } else if (QFrame *frame = qobject_cast<QFrame *>(w)) {
            if (rule.hasBox())
                frame->setFrameStyle(QFrame::StyledPanel);
            qConfigurePalette(&p, rule, QPalette::Text, QPalette::Base);
        }
    }
    w->setPalette(p);
}


void QStyleSheetStyle::repolish(QWidget *w)
{
    QList<QWidget *> children = qFindChildren<QWidget *>(w, QString());
    children.append(w);
    update(children);
}

void QStyleSheetStyle::repolish(QApplication *)
{
    update(styleRulesCache.keys());
}

void QStyleSheetStyle::update(const QList<QWidget *>& widgets)
{
    QEvent e(QEvent::StyleChange);
    for (int i = 0; i < widgets.size(); ++i) {
        QWidget *widget = widgets.at(i);
        widget->setPalette(qApp->palette(widget));
        computeStyleSheet(widget);
        QApplication::sendEvent(widget, &e);
    }
}

void QStyleSheetStyle::polish(QWidget *w)
{
    baseStyle()->polish(w);
    computeStyleSheet(w);
}

void QStyleSheetStyle::computeStyleSheet(QWidget *w)
{
    QStyleSheetStyleSelector styleSelector;
    StyleSheet appSs;
    Parser parser1(qApp->styleSheet());
    if (!parser1.parse(&appSs))
        qWarning("Could not parse application stylesheet");
    styleSelector.styleSheets += appSs;

    for (QWidget *wid = w->parentWidget(); wid; wid = wid->parentWidget()) {
        if (wid->styleSheet().isEmpty())
            continue;
        StyleSheet ss;
        Parser parser(wid->styleSheet());
        if (!parser.parse(&ss))
            qWarning("Could not parse window stylesheet");
        styleSelector.styleSheets += ss;
    }

    StyleSheet widgetSs;
    Parser parser2(w->styleSheet());
    if (parser2.parse(&widgetSs)) {
        // Give the widgetSs the highest priority by adding object names
        QVector<StyleRule>& rules = widgetSs.styleRules;
        for (int i = 0; i < rules.count(); i++) {
            QVector<Selector>& selectors = rules[i].selectors;
            for (int j = 0; j < selectors.count(); j++) {
                QVector<BasicSelector>& basicSelectors = selectors[j].basicSelectors;
                if (!basicSelectors.isEmpty())
                    basicSelectors.first().ids = QStringList(w->objectName());
            }
        }
    } else
        qWarning("Could not parse widget stylesheet");

    styleSelector.styleSheets += widgetSs;

    StyleSelector::NodePtr n;
    n.ptr = w;
    const QVector<StyleRule>& rules = styleSelector.styleRulesForNode(n);
    styleRulesCache[w] = rules;
    renderRulesCache.remove(w);
    setPalette(w);
}

void QStyleSheetStyle::polish(QApplication *app)
{
    styleRulesCache.clear();
    renderRulesCache.clear();
    baseStyle()->polish(app);
}

void QStyleSheetStyle::polish(QPalette &pal)
{
    baseStyle()->polish(pal);
}

void QStyleSheetStyle::unpolish(QWidget *widget)
{
    styleRulesCache.remove(widget);
    renderRulesCache.remove(widget);
    baseStyle()->unpolish(widget);
}

void QStyleSheetStyle::unpolish(QApplication *app)
{
    styleRulesCache.clear();
    renderRulesCache.clear();
    baseStyle()->unpolish(app);
}

bool QStyleSheetStyle::baseStyleCanRender(QStyleSheetStyle::WidgetType wt, const QRenderRule& rule) const
{
    switch (wt) {
    case PushButton:
        if (rule.hasBackground() || rule.hasFocusRect() || rule.hasBox())
            return false;
        if (!rule.hasPalette())
            return true;
        return (rule.palette()->background.style() == Qt::NoBrush);

    case LineEdit:
        return !rule.hasBox();

    case Frame:
    case ComboBox:
        return rule.isEmpty();

    //    case GroupBox:
    //        return (!rule.isPropertySet(PropertyBorder)
    //                && !rule.isPropertySet(PropertyBorderBrush)
    //                && !rule.isPropertySet(PropertyBorderImage)
    //                && !rule.isPropertySet(PropertyBorderStyle)
    //                && !rule.isPropertySet(PropertyBackgroundPixmap)
    //                && !rule.isPropertySet(PropertyBackground));
    default:
         return false;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
QStyleSheetStyle::QStyleSheetStyle(QStyle *baseStyle, QObject *parent)
: bs(baseStyle)
{
    setParent(parent);
}

void QStyleSheetStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                 const QWidget *w) const
{
    QRenderRule rule = renderRule(w, opt);
    if (rule.isEmpty()) {
        baseStyle()->drawComplexControl(cc, opt, p, w);
        return;
    }

    switch (cc) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            QStyleOptionComboBox cmbOpt(*cb);
            if (baseStyleCanRender(ComboBox, rule)) {
                qConfigurePalette(&cmbOpt.palette, rule, QPalette::Text, QPalette::Base);
                baseStyle()->drawComplexControl(cc, &cmbOpt, p, w);
                return;
            }
            if ((cb->subControls & SC_ComboBoxFrame) && cb->frame)
                qDrawFrame(p, rule, opt->rect, opt->direction);

            if (cb->subControls & SC_ComboBoxArrow) {
                const QPixmap& downArrow = rule.pixmaps.value("down-arrow");
                if (!downArrow.isNull()) {
                    QRect rect = subControlRect(CC_ComboBox, cb, SC_ComboBoxArrow, w);
                    p->drawPixmap(rect, downArrow);
                } else {
                    State flags = State_None;

                    QRect ar = subControlRect(CC_ComboBox, cb, SC_ComboBoxArrow, w);
                    if (cb->activeSubControls == SC_ComboBoxArrow) {
                        p->setPen(cb->palette.dark().color());
                        p->setBrush(cb->palette.brush(QPalette::Button));
                        p->drawRect(ar.adjusted(0,0,-1,-1));
                    } else {
                        // Make qDrawWinButton use the right colors for drawing the shade of the button
                        QPalette pal(cb->palette);
                        pal.setColor(QPalette::Button, cb->palette.light().color());
                        pal.setColor(QPalette::Light, cb->palette.button().color());
                        qDrawWinButton(p, ar, pal, false,
                                       &cb->palette.brush(QPalette::Button));
                    }

                    ar.adjust(2, 2, -2, -2);
                    if (opt->state & State_Enabled)
                        flags |= State_Enabled;

                    if (cb->activeSubControls == SC_ComboBoxArrow)
                        flags |= State_Sunken;
                    QStyleOption arrowOpt(0);
                    arrowOpt.rect = ar;
                    arrowOpt.palette = cb->palette;
                    arrowOpt.state = flags;
                    drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, p, w);
                }
            }
        }
        break;

    case CC_GroupBox:
//         if (const QStyleOptionGroupBox *gb = qstyleoption_cast<const QStyleOptionGroupBox *>(opt)) {
//             QStyleOptionGroupBox gbOpt(*gb);
//             if (rule.foreground.style() != Qt::NoBrush)
//                 gbOpt.textColor = rule.foreground.color();
//             if (baseStyleCanRender(GroupBox, rule))
//                 baseStyle()->drawComplexControl(cc, &gbOpt, p, w);
//             else
//                 ParentStyle::drawComplexControl(cc, &gbOpt, p, w);
//         }
//         break;

    default:
        qDebug() << "drawComplexControl: panic " << cc;
        baseStyle()->drawComplexControl(cc, opt, p, w);
        break;
    }

}

void QStyleSheetStyle::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                          const QWidget *w) const
{
    QRenderRule rule = renderRule(w, opt);
    if (rule.isEmpty()) {
        baseStyle()->drawControl(ce, opt, p, w);
        return;
    }

    switch (ce) {
        // Push button
        case CE_PushButton:
            ParentStyle::drawControl(ce, opt, p, w);
            return;

        case CE_PushButtonBevel:
        case CE_PushButtonLabel:
            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
                QStyleOptionButton butOpt(*btn);
                qConfigurePalette(&butOpt.palette, rule, QPalette::ButtonText, QPalette::Button);
                if (baseStyleCanRender(PushButton, rule))
                    baseStyle()->drawControl(ce, &butOpt, p, w);
                else
                    ParentStyle::drawControl(ce, &butOpt, p, w);
            }
            return;

        case CE_RadioButton:
        case CE_CheckBox:
            if (rule.palette()->background.style() != Qt::NoBrush)
                p->fillRect(opt->rect, rule.palette()->background);
            // any funky border stuff, when this is fixed, fix the CT_RadioButton too
            if (rule.hasBox())
                qDrawFrame(p, rule, opt->rect, opt->direction);
            ParentStyle::drawControl(ce, opt, p, w);
            return;

        case CE_RadioButtonLabel:
        case CE_CheckBoxLabel:
            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
                QStyleOptionButton butOpt(*btn);
                qConfigurePalette(&butOpt.palette, rule, QPalette::ButtonText, QPalette::Button);
                baseStyle()->drawControl(ce, &butOpt, p, w);
            }
            return;

        case CE_MenuBarItem:
        case CE_MenuItem:
            if (const QStyleOptionMenuItem *mb = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
                QStyleOptionMenuItem mi(*mb);
                qConfigurePalette(&mi.palette, rule, QPalette::ButtonText, QPalette::Button);
                baseStyle()->drawControl(ce, &mi, p, w);
            }
            return;

        case CE_MenuEmptyArea:
        case CE_MenuBarEmptyArea:
            if (const QStyleOptionMenuItem *m = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
                QStyleOptionMenuItem menuOpt(*m);
                qConfigurePalette(&menuOpt.palette, rule, QPalette::ButtonText, QPalette::Button);
                baseStyle()->drawControl(ce, &menuOpt, p, w);
            }
            return;

        case CE_MenuScroller:
        case QStyle::CE_MenuTearoff:
            baseStyle()->drawControl(ce, opt, p, w);
            return;

        case CE_ComboBoxLabel:
            if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
                if (!cb->editable) {
                    if (rule.palette()->selectionBackground.style() != Qt::NoBrush)
                        p->fillRect(subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, w),
                                    rule.palette()->selectionBackground);
                    QPen savedPen = p->pen();
                    if (rule.palette()->selectionForeground.style() != Qt::NoBrush)
                        p->setPen(rule.palette()->selectionForeground.color());
                    ParentStyle::drawControl(ce, opt, p, w);
                    p->setPen(savedPen);
                } else {
                    qDebug() << "Fix QComboBox using line edit";
                }
            }
            return;

        default:
            qDebug() << "controlElement: Panic " << ce;
            break;
    }

    baseStyle()->drawControl(ce, opt, p, w);
}


void QStyleSheetStyle::drawItemPixmap(QPainter *p, const QRect &rect, int alignment, const
                                  QPixmap &pixmap) const
{
    baseStyle()->drawItemPixmap(p, rect, alignment, pixmap);
}

void QStyleSheetStyle::drawItemText(QPainter *painter, const QRect& rect, int alignment, const QPalette &pal,
                                bool enabled, const QString& text, QPalette::ColorRole textRole) const
{
    baseStyle()->drawItemText(painter, rect, alignment, pal, enabled, text, textRole);
}

void QStyleSheetStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                            const QWidget *w) const
{
    QRenderRule rule = renderRule(w, opt);
    if (rule.isEmpty()) {
        baseStyle()->drawPrimitive(pe, opt, p, w);
        return;
    }

    switch (pe) {
    case PE_IndicatorArrowDown:
        if (rule.pixmaps.contains("down-arrow"))
            baseStyle()->drawPrimitive(pe, opt, p, w);
        return;

    case PE_PanelButtonCommand:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QStyleOptionButton butOpt(*btn);
            qConfigurePalette(&butOpt.palette, rule, QPalette::ButtonText, QPalette::Button);
            if (baseStyleCanRender(PushButton, rule))
                baseStyle()->drawPrimitive(pe, &butOpt, p, w);
            else
                qDrawFrame(p, rule, opt->rect, opt->direction);
        }
        return;

    // how do we specify these in the css?
    case PE_FrameDefaultButton:
        baseStyle()->drawPrimitive(pe, opt, p, w);
        return;

    case PE_FrameFocusRect:
        if (const QStyleOptionFocusRect *fropt = qstyleoption_cast<const QStyleOptionFocusRect *>(opt)) {
            Q_UNUSED(fropt);
            //if (rule.focusRectColor.isValid()) {
                //QRenderRule focusRule = rule;
                //focusRule.borderStyle = rule.focusRectStyle;
                //focusRule.border = Margin(rule.focusRectWidth, rule.focusRectWidth,
                //                          rule.focusRectWidth, rule.focusRectWidth);
                //focusRule.borderBrushes = rule.focusRectColor;
                //qDrawFrame(p, focusRule, opt->rect, opt->direction);
            //} else
                baseStyle()->drawPrimitive(pe, opt, p, w);
        }
        return;

    case PE_PanelLineEdit:
        if (const QStyleOptionFrame *frm = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            QStyleOptionFrame frmOpt(*frm);
            qConfigurePalette(&frmOpt.palette, rule, QPalette::Text, QPalette::Base);
            if (baseStyleCanRender(LineEdit, rule))
                baseStyle()->drawPrimitive(pe, &frmOpt, p, w);
            else {
                qDrawFrame(p, rule, opt->rect, opt->direction);
            }
        }
        return;

    case PE_Frame:
        if (rule.hasBox())
            qDrawFrame(p, rule, opt->rect, opt->direction);
        else
            baseStyle()->drawPrimitive(pe, opt, p, w);
        return;

    // RadioButton and CheckBox
    case PE_IndicatorRadioButton:
    case PE_IndicatorCheckBox:
        if (rule.pixmaps.contains("exclusive-indicator")) {
            p->drawPixmap(opt->rect, rule.pixmaps["exclusive-indicator"]);
        } else {
            QStyleOption optCopy(*opt);
            qConfigurePalette(&optCopy.palette, rule, QPalette::ButtonText, QPalette::Button);
            baseStyle()->drawPrimitive(pe, &optCopy, p, w);
        }
        return;

    case PE_FrameMenu:
    case PE_PanelMenuBar:
        if (rule.hasBox())
            qDrawFrame(p, rule, opt->rect, opt->direction);
        else
            baseStyle()->drawPrimitive(pe, opt, p, w);
        return;

        // Menu stuff that would be nice to customize
    case PE_IndicatorMenuCheckMark:
    case PE_IndicatorArrowLeft:
    case PE_IndicatorArrowRight:
        baseStyle()->drawPrimitive(pe, opt, p, w);
        return;

    case PE_PanelTipLabel: // the frame is drawn in PE_Frame
        return;

    case PE_FrameGroupBox:
        if (rule.hasBox()) {
            qDrawFrame(p, rule, opt->rect, opt->direction);
        } else
            baseStyle()->drawPrimitive(pe, opt, p, w);
        return;

    default:
        break;
    }

    baseStyle()->drawPrimitive(pe, opt, p, w);
}

QPixmap QStyleSheetStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap& pixmap,
                                          const QStyleOption *option) const
{
    return baseStyle()->generatedIconPixmap(iconMode, pixmap, option);
}

QStyle::SubControl QStyleSheetStyle::hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                 const QPoint &pt, const QWidget *w) const
{
    return ParentStyle::hitTestComplexControl(cc, opt, pt, w);
}

QRect QStyleSheetStyle::itemPixmapRect(const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    return baseStyle()->itemPixmapRect(rect, alignment, pixmap);
}

QRect QStyleSheetStyle::itemTextRect(const QFontMetrics &metrics, const QRect& rect, int alignment,
                                 bool enabled, const QString& text) const
{
    return baseStyle()->itemTextRect(metrics, rect, alignment, enabled, text);
}

int QStyleSheetStyle::pixelMetric(PixelMetric m, const QStyleOption *opt, const QWidget *w) const
{
    QRenderRule rule = renderRule(w, opt);
    if (rule.isEmpty())
        return baseStyle()->pixelMetric(m, opt, w);

    switch (m) {
    case PM_MenuButtonIndicator:
        //if (rule.hasPixmaps() && !rule.pixmaps()->downArrow.isNull())
            return baseStyle()->pixelMetric(m, opt, w);
        break;

    case PM_ButtonMargin:
    case PM_DefaultFrameWidth:
    case PM_ButtonDefaultIndicator:
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        break;

    case PM_ExclusiveIndicatorWidth:
    case PM_IndicatorWidth:
        if (rule.pixmaps.contains("exclusive-indicator"))
            return rule.pixmaps["exclusive-indicator"].width();
        break;

    case PM_ExclusiveIndicatorHeight:
    case PM_IndicatorHeight:
        if (rule.pixmaps.contains("exclusive-indicator"))
            return rule.pixmaps["exclusive-indicator"].height();
        break;


    case PM_ToolTipLabelFrameWidth:
    case PM_ComboBoxFrameWidth:
        break;

    case PM_MenuPanelWidth:
    case PM_MenuBarPanelWidth: // support only one border
        if (rule.box())
            return rule.box()->borders[LeftEdge];
        break;
    case PM_MenuHMargin:
    case PM_MenuVMargin:
    case PM_MenuBarHMargin:
    case PM_MenuBarVMargin:
    case PM_SmallIconSize:
    case PM_MenuDesktopFrameWidth:
    case PM_MenuTearoffHeight:
    case PM_MenuScrollerHeight:
        break;
    case PM_MenuBarItemSpacing:
        break;
    case PM_ToolBarExtensionExtent:
        break;

    case PM_SizeGripSize:

        // group box
    case PM_CheckBoxLabelSpacing:
        break;

    default: {
        static QList<int> panics;
        if (!panics.contains(m)) {
            qDebug() << "pixelMetric: Panic " << m;
            panics.append(m);
        }
             }
    }

    return baseStyle()->pixelMetric(m, opt, w);
}

QSize QStyleSheetStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt,
                                const QSize &csz, const QWidget *w) const
{
    QRenderRule rule = renderRule(w, opt);
    if (rule.isEmpty())
        return baseStyle()->sizeFromContents(ct, opt, csz, w);

    switch (ct) {
    case CT_PushButton:
        if (baseStyleCanRender(PushButton, rule))
            return baseStyle()->sizeFromContents(ct, opt, csz, w);
        else
            return rule.boxRect(QRect(0, 0, csz.width(), csz.height())).size();
        break;

    case CT_LineEdit: // does not contains fw
        return baseStyleCanRender(LineEdit, rule)
               ? baseStyle()->sizeFromContents(ct, opt, csz, w)
               : rule.boxRect(QRect(0, 0, csz.width(), csz.height())).size();

    case CT_CheckBox:
    case CT_RadioButton:
        // any funky border stuff, when this is fixed, fix the CE_RadioButton too
        if (!rule.hasBox())
            return baseStyle()->sizeFromContents(ct, opt, csz, w);
        else {
            QSize sz = ParentStyle::sizeFromContents(ct, opt, csz, w);
            return rule.boxRect(QRect(0, 0, sz.width(), sz.height())).size();
        }
        break;

    case CT_Menu:
    case CT_MenuItem:
    case CT_MenuBarItem:
        return baseStyle()->sizeFromContents(ct, opt, csz, w);

    case CT_ComboBox: {
        if (baseStyleCanRender(ComboBox, rule))
            return baseStyle()->sizeFromContents(ct, opt, csz, w);
        // FIXME
        QPixmap downArrow = rule.pixmaps.value("down-arrow");
        QRect br = rule.boxRect(QRect(0, 0, csz.width() + downArrow.width() + 23,
                                       qMax(csz.height(), downArrow.height())));
        return br.size();
                      }
    default:
        static QList<int> panics;
        if (!panics.contains(ct)) {
            qDebug() << "sizeFromContents: Panic " << ct;
            panics.append(ct);
        }
        break;
    }

    return baseStyle()->sizeFromContents(ct, opt, csz, w);
}

QIcon QStyleSheetStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption *option,
                                 const QWidget* w) const
{
    return baseStyle()->standardIcon(standardIcon, option, w);
}

QPalette QStyleSheetStyle::standardPalette() const
{
    return baseStyle()->standardPalette();
}

QPixmap QStyleSheetStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option,
                                     const QWidget *w) const
{
    return baseStyle()->standardPixmap(standardPixmap, option, w);
}

int QStyleSheetStyle::styleHint(StyleHint sh, const QStyleOption *opt, const QWidget *w,
                           QStyleHintReturn *shret) const
{
    return baseStyle()->styleHint(sh, opt, w, shret);
}

QRect QStyleSheetStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                              const QWidget *w) const
{
    QRenderRule rule = renderRule(w, opt);
    if (rule.isEmpty())
        return baseStyle()->subControlRect(cc, opt, sc, w);

    QRect r = opt->rect;
    switch (cc) {
    case CC_ComboBox:
    {
        if (baseStyleCanRender(ComboBox, rule))
            return baseStyle()->subControlRect(cc, opt, sc, w);
        QPixmap downArrow = rule.pixmaps.value("down-arrow");
        switch (sc) {
        case SC_ComboBoxArrow:
            r = rule.contentsRect(r);
            if (!downArrow.isNull()) {
                r.setLeft(r.right() - downArrow.width());
            } else {
                r.setLeft(r.right() - 18);
            }
            break;
        case SC_ComboBoxEditField:
            r = rule.contentsRect(r);
            if (!downArrow.isNull()) {
                r.setRight(r.right() - downArrow.width());
            } else {
                r.setRight(r.right() - 18);
            }
            break;
        case SC_ComboBoxListBoxPopup:
            r.setBottom(r.bottom() + 1);
            break;
        case SC_ComboBoxFrame:
            break;
        default:
            break;
        }
        r = visualRect(opt->direction, opt->rect, r);
        break;
    }
    case CC_GroupBox:
        if (baseStyleCanRender(GroupBox, rule))
            return baseStyle()->subControlRect(cc, opt, sc, w);
        switch (sc) {
        case SC_GroupBoxFrame:
            r = visualRect(opt->direction, opt->rect, rule.borderRect(opt->rect));
            break;
        case SC_GroupBoxContents:
            r = visualRect(opt->direction, opt->rect, rule.contentsRect(opt->rect));
            break;
        case SC_GroupBoxLabel:
        case SC_GroupBoxCheckBox:
//             if (const QStyleOptionGroupBox *gb = qstyleoption_cast<const QStyleOptionGroupBox *>(opt)) {
//                 int h = opt->fontMetrics.height();
//                 if (rule.padding.valid) {
//                     Margin p = rule.padding;
//                     r = opt->rect.adjusted(p.left, p.top, 0, 0);
//                 } else
//                     r = opt->rect.adjusted(8, 0, -8, 0);
//                 bool hasCheckBox = gb->subControls & QStyle::SC_GroupBoxCheckBox;
//                 r.setHeight(qMax(h, pixelMetric(PM_IndicatorHeight, opt, w)));
//
//                 int indicatorWidth = 0;
//                 if (hasCheckBox) {
//                     indicatorWidth = pixelMetric(PM_IndicatorWidth, opt, w)
//                                      + pixelMetric(PM_CheckBoxLabelSpacing, opt, w);
//                 }
//                 if (sc == SC_GroupBoxLabel) {
//                     r.setLeft(r.left() + indicatorWidth);
//                 } else if (sc == SC_GroupBoxCheckBox) {
//                     r.setRight(r.left() + indicatorWidth);
//                 }
//
//                 r = alignedRect(opt->direction, gb->textAlignment, r.size(), r);
//             }
             break;
        default:
            break;
        }

        break;
    default:
        r = baseStyle()->subControlRect(cc, opt, sc, w);
        break;
    }
    return r;
}

QRect QStyleSheetStyle::subElementRect(SubElement se, const QStyleOption *opt, const QWidget *w) const
{
    QRenderRule rule = renderRule(w, opt);
    if (rule.isEmpty())
        return baseStyle()->subElementRect(se, opt, w);

    switch (se) {
    // PushButton
    case SE_PushButtonContents:
    case SE_PushButtonFocusRect:
        return (baseStyleCanRender(PushButton, rule))
               ? baseStyle()->subElementRect(se, opt, w)
               : visualRect(opt->direction, opt->rect, rule.contentsRect(opt->rect));

    case SE_LineEditContents:
        return (baseStyleCanRender(LineEdit, rule))
               ? baseStyle()->subElementRect(se, opt, w)
               : visualRect(opt->direction, opt->rect, rule.contentsRect(opt->rect));

    case SE_FrameContents:
        return (baseStyleCanRender(Frame, rule))
               ? baseStyle()->subElementRect(se, opt, w)
               : visualRect(opt->direction, opt->rect, rule.contentsRect(opt->rect));

        // RadioButton and CheckBox
    case SE_CheckBoxIndicator:
    case SE_RadioButtonIndicator: {
        // any funky border stuff, when this is fixed, fix the CT_RadioButton too
        QStyleOption optCopy(*opt);
        if (rule.hasBox()) {
            optCopy.rect = rule.contentsRect(opt->rect);
        }
        return ParentStyle::subElementRect(se, &optCopy, w);
                               }

    // relies on indicator
    case SE_CheckBoxContents:
    case SE_RadioButtonContents:
    case SE_RadioButtonFocusRect:
    case SE_RadioButtonClickRect: // focusrect | indicator
    case SE_CheckBoxFocusRect:
    case SE_CheckBoxClickRect: // relies on indicator and contents
        return ParentStyle::subElementRect(se, opt, w);

    default:
        break;
    }

    return baseStyle()->subElementRect(se, opt, w);
}

#include "moc_qstylesheetstyle_p.cpp"
