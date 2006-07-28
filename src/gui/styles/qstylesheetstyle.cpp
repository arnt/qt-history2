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

#ifndef QT_NO_STYLE_STYLESHEET

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
#include <qwindowsstyle.h>
#include <qcombobox.h>
#include <qwindowsstyle.h>
#include <qplastiquestyle.h>
#include <qframe.h>
#include "private/qcssparser_p.h"
#include "private/qmath_p.h"
#include <qabstractscrollarea.h>
#include <qfileinfo.h>
#include <qtooltip.h>

using namespace QCss;

QHash<const QWidget *, QVector<QCss::StyleRule> > QStyleSheetStyle::styleRulesCache;
QHash<const QWidget *, QRenderRules> QStyleSheetStyle::renderRulesCache;

///////////////////////////////////////////////////////////////////////////////////////////
#define ceil(x) ((int)(x) + ((x) > 0 && (x) != (int)(x)))

struct StdIcons {
    const char *suffix;
    int state;
};

void QRenderRules::init(const QVector<QCss::StyleRule>& styleRules)
{
   for (int i = 0; i < styleRules.count(); i++) {
        const QCss::StyleRule &styleRule = styleRules.at(i);
        int cssState = styleRule.selectors.at(0).pseudoState();
        const QVector<QCss::Declaration>& decls = styleRule.declarations;

        if ((cssState != PseudoState_Unspecified) && (cssState != PseudoState_Enabled))
            continue;

        for (int i = 0; i < decls.count(); i++) {
            const Declaration& decl = decls.at(i);
            if (decl.property.endsWith("-size", Qt::CaseInsensitive)) {
                sizes[decl.property.left(decl.property.length() - 5)] = decl.sizeValue();
            } else if (decl.propertyId == Width) {
                decl.realValue(&contentsWidth, "px"); 
            } else if (decl.propertyId == Height) {
                decl.realValue(&contentsHeight);
            }
        }
    }
}

void QRenderRule::merge(const QVector<Declaration>& decls)
{
    for (int i = 0; i < decls.count(); i++) {
        const Declaration& decl = decls.at(i);
        switch (decl.propertyId) {
        case Color: _palette()->foreground = decl.colorValue(); break;
        case BackgroundColor: _palette()->background = decl.colorValue(); break;
        case SelectionForeground:_palette()->selectionForeground = decl.brushValue(); break;
        case SelectionBackground: _palette()->selectionBackground = decl.brushValue(); break;
        case AlternateBackground: _palette()->alternateBackground = decl.brushValue(); break;

        case PaddingLeft: decl.realValue(&_box()->paddings[LeftEdge], "px"); break;
        case PaddingRight: decl.realValue(&_box()->paddings[RightEdge], "px"); break;
        case PaddingTop: decl.realValue(&_box()->paddings[TopEdge], "px"); break;
        case PaddingBottom: decl.realValue(&_box()->paddings[BottomEdge], "px"); break;
        case Padding: decl.realValues(_box()->paddings, "px"); break;

        case MarginLeft: decl.realValue(&_box()->margins[LeftEdge], "px"); break;
        case MarginRight: decl.realValue(&_box()->margins[RightEdge], "px"); break;
        case MarginTop: decl.realValue(&_box()->margins[TopEdge], "px"); break;
        case MarginBottom: decl.realValue(&_box()->margins[BottomEdge], "px"); break;
        case Margin: decl.realValues(_box()->margins, "px"); break;

        case BorderLeftWidth: decl.realValue(&_border()->borders[LeftEdge], "px"); break;
        case BorderRightWidth: decl.realValue(&_border()->borders[RightEdge], "px"); break;
        case BorderTopWidth: decl.realValue(&_border()->borders[TopEdge], "px"); break;
        case BorderBottomWidth: decl.realValue(&_border()->borders[BottomEdge], "px"); break;
        case BorderWidth: decl.realValues(_border()->borders, "px"); break;

        case BorderLeftColor: _border()->colors[LeftEdge] = decl.colorValue(); break;
        case BorderRightColor: _border()->colors[RightEdge] = decl.colorValue(); break;
        case BorderTopColor: _border()->colors[TopEdge] = decl.colorValue(); break;
        case BorderBottomColor: _border()->colors[BottomEdge] = decl.colorValue(); break;
        case BorderColor: decl.colorValues(_border()->colors); break;

        case BorderTopStyle: _border()->styles[TopEdge] = decl.styleValue(); break;
        case BorderBottomStyle: _border()->styles[BottomEdge] = decl.styleValue(); break;
        case BorderLeftStyle: _border()->styles[LeftEdge] = decl.styleValue(); break;
        case BorderRightStyle: _border()->styles[RightEdge] = decl.styleValue(); break;
        case BorderStyles:  decl.styleValues(_border()->styles); break;

        case BorderTopLeftRadius: _border()->radii[0] = decl.sizeValue(); break;
        case BorderTopRightRadius: _border()->radii[1] = decl.sizeValue(); break;
        case BorderBottomLeftRadius: _border()->radii[2] = decl.sizeValue(); break;
        case BorderBottomRightRadius: _border()->radii[3] = decl.sizeValue(); break;
        case BorderRadius: decl.radiiValues(_border()->radii, "px"); break;

        case BackgroundOrigin: _backgroundImage()->origin = decl.originValue(); break;
        case BackgroundRepeat: _backgroundImage()->repeat = decl.repeatValue(); break;
        case BackgroundPosition: _backgroundImage()->position = decl.alignmentValue(); break;
        case BackgroundImage: _backgroundImage()->pixmap = QPixmap(decl.uriValue()); break;

        case BorderImage: decl.borderImageValue(&_border()->_borderImage()->pixmap,
                                                _border()->_borderImage()->cuts,
                                                &_border()->_borderImage()->horizStretch,
                                                &_border()->_borderImage()->vertStretch);
                          break;

        case Spacing: decl.realValue(&_box()->spacing, "px"); break;

        default:
            if (decl.property.endsWith("-icon", Qt::CaseInsensitive)) {
                icons[decl.property.left(decl.property.length() - 5)] = decl.iconValue();
            }
            break;
        }
    }
}

QRectF QRenderRule::borderRect(const QRectF& r) const
{
    if (!hasBox())
        return r;
    const qreal* m = box()->margins;
    return r.adjusted(m[LeftEdge], m[TopEdge], -m[RightEdge], -m[BottomEdge]);
}

QRectF QRenderRule::paddingRect(const QRectF& r) const
{
    QRectF br = borderRect(r);
    if (!hasBorder())
        return br;
    const qreal *b = border()->borders;
    return br.adjusted(b[LeftEdge], b[TopEdge], -b[RightEdge], -b[BottomEdge]);
}

QRectF QRenderRule::contentsRect(const QRectF& r) const
{
    QRectF pr = paddingRect(r);
    if (!hasBox())
        return pr;
    const qreal *p = box()->paddings;
    return pr.adjusted(p[LeftEdge], p[TopEdge], -p[RightEdge], -p[BottomEdge]);
}

QRectF QRenderRule::boxRect(const QRectF& cr) const
{
    QRectF r = cr;
    if (hasBox()) {
        const qreal *m = box()->margins;
        const qreal *p = box()->paddings;
        r.adjust(-p[LeftEdge] - m[LeftEdge], -p[TopEdge] - m[TopEdge],
                 p[RightEdge] + m[RightEdge], p[BottomEdge] + m[BottomEdge]);
    }
    if (hasBorder()) {
        const qreal *b = border()->borders;
        r.adjust(-b[LeftEdge], -b[TopEdge], b[RightEdge], b[BottomEdge]);
    }
    return r;
}

QRect QRenderRule::borderRect(const QRect& r) const
{
    return borderRect(QRectF(r)).toRect();
}

QRect QRenderRule::paddingRect(const QRect& r) const
{
    return paddingRect(QRectF(r)).toRect();
}

QRect QRenderRule::contentsRect(const QRect& r) const
{
    return contentsRect(QRectF(r)).toRect();
}

QRect QRenderRule::boxRect(const QRect& r) const
{
    return boxRect(QRectF(r)).toRect();
}

QSize QRenderRule::sizeWithPadding(const QSize& sz) const
{
    if (!hasBox())
        return sz;
    const qreal *p = box()->paddings;
    return sz + QSize(qRound(p[LeftEdge] + p[RightEdge]),
                      qRound(p[TopEdge] + p[BottomEdge]));
}

QSize QRenderRule::sizeWithBorder(const QSize& sz) const
{
    if (!hasBorder())
        return sz;
    const qreal *b = border()->borders;
    return sz + QSize(qRound(b[LeftEdge] + b[RightEdge]),
                      qRound(b[TopEdge] + b[BottomEdge]));
}

QSize QRenderRule::sizeWithMargin(const QSize& sz) const
{
    if (!hasBox())
        return sz;
    const qreal *m = box()->margins;
    return sz + QSize(qRound(m[LeftEdge] + m[RightEdge]),
                      qRound(m[TopEdge] + m[BottomEdge]));
}

void QRenderRule::fixupBorder()
{
    if (bd == 0)
        return;

    if (!bd->hasBorderImage()) {
        // ignore the color, border of edges that have none border-style
        for (int i = 0; i < 4; i++) {
            if (bd->styles[i] != BorderStyle_None)
                continue;
            bd->colors[i] = QColor();
            bd->borders[i] = 0;
        }

        return;
    }

    // inspect the border image
    QStyleSheetBorderImageData *borderImage = bd->_borderImage();
    const QPixmap& pixmap = borderImage->pixmap;
    if (pixmap.isNull()) {
        bd->bi = 0; // delete it
        return;
    }

    if (borderImage->cuts[0] == -1) {
        for (int i = 0; i < 4; i++) // assume, cut = border
            borderImage->cuts[i] = int(border()->borders[i]);
    }
    borderImage->cutBorderImage();
}

void QStyleSheetBorderImageData::cutBorderImage()
{
    const int w = pixmap.width();
    const int h = pixmap.height();
    const int &l = cuts[LeftEdge], &r = cuts[RightEdge],
              &t = cuts[TopEdge], &b = cuts[BottomEdge];

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

    return QPen(b, width, ps, Qt::FlatCap);
}

static void qDrawRoundedCorners(QPainter *p, qreal x1, qreal y1, qreal x2, qreal y2,
                                const QSizeF& r1, const QSizeF& r2,
                                Edge edge, BorderStyle s, QColor c)
{
    const qreal pw = (edge == TopEdge || edge == BottomEdge) ? y2-y1 : x2-x1;
    if (s == BorderStyle_Double) {
        qreal wby3 = pw/3;
        switch (edge) {
        case TopEdge:
        case BottomEdge:
            qDrawRoundedCorners(p, x1, y1, x2, y1+wby3, r1, r2, edge, BorderStyle_Solid, c);
            qDrawRoundedCorners(p, x1, y2-wby3, x2, y2, r1, r2, edge, BorderStyle_Solid, c);
            break;
        case LeftEdge:
            qDrawRoundedCorners(p, x1, y1+1, x1+wby3, y2, r1, r2, LeftEdge, BorderStyle_Solid, c);
            qDrawRoundedCorners(p, x2-wby3, y1+1, x2, y2, r1, r2, LeftEdge, BorderStyle_Solid, c);
            break;
        case RightEdge:
            qDrawRoundedCorners(p, x1, y1+1, x1+wby3, y2, r1, r2, RightEdge, BorderStyle_Solid, c);
            qDrawRoundedCorners(p, x2-wby3, y1+1, x2, y2, r1, r2, RightEdge, BorderStyle_Solid, c);
            break;
        default:
            break;
        }
        return;
    } else if (s == BorderStyle_Ridge || s == BorderStyle_Groove) {
        BorderStyle s1, s2;
        if (s == BorderStyle_Groove) {
            s1 = BorderStyle_Inset;
            s2 = BorderStyle_Outset;
        } else {
            s1 = BorderStyle_Outset;
            s2 = BorderStyle_Inset;
        }
        int pwby2 = qRound(pw/2);
        switch (edge) {
        case TopEdge:
            qDrawRoundedCorners(p, x1, y1, x2, y1 + pwby2, r1, r2, TopEdge, s1, c);
            qDrawRoundedCorners(p, x1, y1 + pwby2, x2, y2, r1, r2, TopEdge, s2, c);
            break;
        case BottomEdge:
            qDrawRoundedCorners(p, x1, y1 + pwby2, x2, y2, r1, r2, BottomEdge, s1, c);
            qDrawRoundedCorners(p, x1, y1, x2, y2-pwby2, r1, r2, BottomEdge, s2, c);
            break;
        case LeftEdge:
            qDrawRoundedCorners(p, x1, y1, x1 + pwby2, y2, r1, r2, LeftEdge, s1, c);
            qDrawRoundedCorners(p, x1 + pwby2, y1, x2, y2, r1, r2, LeftEdge, s2, c);
            break;
        case RightEdge:
            qDrawRoundedCorners(p, x1 + pwby2, y1, x2, y2, r1, r2, RightEdge, s1, c);
            qDrawRoundedCorners(p, x1, y1, x2 - pwby2, y2, r1, r2, RightEdge, s2, c);
            break;
        default:
            break;
        }
    } else if ((s == BorderStyle_Outset && (edge == TopEdge || edge == LeftEdge))
            || (s == BorderStyle_Inset && (edge == BottomEdge || edge == RightEdge)))
            c = c.light();

    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    qreal pwby2 = pw/2;
    p->setBrush(Qt::NoBrush);
    QPen pen = qPenFromStyle(c, pw, s);
    pen.setCapStyle(Qt::SquareCap); // this eliminates the offby1 errors that we might hit below
    p->setPen(pen);
    switch (edge) {
    case TopEdge:
        if (!r1.isEmpty())
            p->drawArc(QRectF(x1 - r1.width() + pwby2, y1 + pwby2,
                              2*r1.width() - pw, 2*r1.height() - pw), 135*16, -45*16);
        if (!r2.isEmpty())
            p->drawArc(QRectF(x2 - r2.width() + pwby2, y1 + pwby2,
                       2*r2.width() - pw, 2*r2.height() - pw), 45*16, 45*16);
        break;
    case BottomEdge:
        if (!r1.isEmpty())
            p->drawArc(QRectF(x1 - r1.width() + pwby2, y2 - 2*r1.height() + pwby2,
                              2*r1.width() - pw, 2*r1.height() - pw), -90 * 16, -45 * 16);
        if (!r2.isEmpty())
            p->drawArc(QRectF(x2 - r2.width() + pwby2, y2 - 2*r2.height() + pwby2,
                       2*r2.width() - pw, 2*r2.height() - pw), -90 * 16, 45 * 16);
        break;
    case LeftEdge:
        if (!r1.isEmpty())
            p->drawArc(QRectF(x1 + pwby2, y1 - r1.height() + pwby2,
                       2*r1.width() - pw, 2*r1.height() - pw), 135*16, 45*16);
        if (!r2.isEmpty())
            p->drawArc(QRectF(x1 + pwby2, y2 - r2.height() + pwby2,
                       2*r2.width() - pw, 2*r2.height() - pw), 180*16, 45*16);
        break;
    case RightEdge:
        if (!r1.isEmpty())
            p->drawArc(QRectF(x2 - 2*r1.width() + pwby2, y1 - r1.height() + pwby2,
                       2*r1.width() - pw, 2*r1.height() - pw), 45*16, -45*16);
        if (!r2.isEmpty())
            p->drawArc(QRectF(x2 - 2*r2.width() + pwby2, y2 - r2.height() + pwby2,
                       2*r2.width() - pw, 2*r2.height() - pw), 315*16, 45*16);
        break;
    default:
        break;
    }
    p->restore();
}


void qDrawEdge(QPainter *p, qreal x1, qreal y1, qreal x2, qreal y2, qreal dw1, qreal dw2,
               Edge edge, BorderStyle style, QColor c)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    const qreal width = (edge == TopEdge || edge == BottomEdge) ? (y2-y1) : (x2-x1);

    if (width <= 2 && style == BorderStyle_Double)
        style = BorderStyle_Solid;

    switch (style) {
    case BorderStyle_Inset:
    case BorderStyle_Outset:
        if (style == BorderStyle_Outset && (edge == TopEdge || edge == LeftEdge)
            || (style == BorderStyle_Inset && (edge == BottomEdge || edge == RightEdge)))
            c = c.light();
        // fall through!
    case BorderStyle_Solid: {
        p->setPen(Qt::NoPen);
        p->setBrush(c);
        if (width == 1 || (dw1 == 0 && dw2 == 0)) {
            p->drawRect(QRectF(x1, y1, x2-x1, y2-y1));
        } else { // draw trapezoid
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
        if (width == 1)
            p->drawLine(QLineF(x1, y1, x2 - 1, y2 - 1));
        else if (edge == TopEdge || edge == BottomEdge)
            p->drawLine(QLineF(x1 + width/2, (y1 + y2)/2, x2 - width/2, (y1 + y2)/2));
        else
            p->drawLine(QLineF((x1+x2)/2, y1 + width/2, (x1+x2)/2, y2 - width/2));
        break;

    case BorderStyle_Double: {
        int wby3 = qRound(width/3);
        int dw1by3 = qRound(dw1/3);
        int dw2by3 = qRound(dw2/3);
        switch (edge) {
        case TopEdge:
            qDrawEdge(p, x1, y1, x2, y1 + wby3, dw1by3, dw2by3, TopEdge, BorderStyle_Solid, c);
            qDrawEdge(p, x1 + dw1 - dw1by3, y2 - wby3, x2 - dw2 + dw1by3, y2,
                      dw1by3, dw2by3, TopEdge, BorderStyle_Solid, c);
            break;
        case LeftEdge:
            qDrawEdge(p, x1, y1, x1 + wby3, y2, dw1by3, dw2by3, LeftEdge, BorderStyle_Solid, c);
            qDrawEdge(p, x2 - wby3, y1 + dw1 - dw1by3, x2, y2 - dw2 + dw2by3, dw1by3, dw2by3,
                      LeftEdge, BorderStyle_Solid, c);
            break;
        case BottomEdge:
            qDrawEdge(p, x1 + dw1 - dw1by3, y1, x2 - dw2 + dw2by3, y1 + wby3, dw1by3, dw2by3,
                      BottomEdge, BorderStyle_Solid, c);
            qDrawEdge(p, x1, y2 - wby3, x2, y2, dw1by3, dw2by3, BottomEdge, BorderStyle_Solid, c);
            break;
        case RightEdge:
            qDrawEdge(p, x2 - wby3, y1, x2, y2, dw1by3, dw2by3, RightEdge, BorderStyle_Solid, c);
            qDrawEdge(p, x1, y1 + dw1 - dw1by3, x1 + wby3, y2 - dw2 + dw2by3, dw1by3, dw2by3,
                      RightEdge, BorderStyle_Solid, c);
            break;
        default:
            break;
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
        int dw1by2 = qFloor(dw1/2), dw2by2 = qFloor(dw2/2);
        int wby2 = qRound(width/2);
        switch (edge) {
        case TopEdge:
            qDrawEdge(p, x1, y1, x2, y1 + wby2, dw1by2, dw2by2, TopEdge, s1, c);
            qDrawEdge(p, x1 + dw1by2, y1 + wby2, x2 - dw2by2, y2, dw1by2, dw2by2, TopEdge, s2, c);
            break;
        case BottomEdge:
            qDrawEdge(p, x1, y1 + wby2, x2, y2, dw1by2, dw2by2, BottomEdge, s1, c);
            qDrawEdge(p, x1 + dw1by2, y1, x2 - dw2by2, y1 + wby2, dw1by2, dw2by2, BottomEdge, s2, c);
            break;
        case LeftEdge:
            qDrawEdge(p, x1, y1, x1 + wby2, y2, dw1by2, dw2by2, LeftEdge, s1, c);
            qDrawEdge(p, x1 + wby2, y1 + dw1by2, x2, y2 - dw2by2, dw1by2, dw2by2, LeftEdge, s2, c);
            break;
        case RightEdge:
            qDrawEdge(p, x1 + wby2, y1, x2, y2, dw1by2, dw2by2, RightEdge, s1, c);
            qDrawEdge(p, x1, y1 + dw1by2, x1 + wby2, y2 - dw2by2, dw1by2, dw2by2, RightEdge, s2, c);
            break;
        default:
            break;
        }
    }
    default:
        break;
    }
    p->restore();
}

// Determines if Edge e1 draws over Edge e2. Depending on this trapezoids or rectanges are drawn
static bool qPaintsOver(const QRenderRule &rule, Edge e1, Edge e2)
{
    const QStyleSheetBorderData *border = rule.border();
    const BorderStyle& s1 = border->styles[e1];
    const BorderStyle& s2 = border->styles[e2];

    if (s2 == BorderStyle_None)
        return true;

    if (s1 == BorderStyle_Solid && s2 == BorderStyle_Solid && border->colors[e1] == border->colors[e2])
        return true;

    return false;
}

static void qDrawCenterTiledPixmap(QPainter *p, const QRectF& r, const QPixmap& pix)
{
    p->drawTiledPixmap(r, pix, QPoint(pix.width() - int(r.width())%pix.width(),
                                      pix.height() - int(r.height())%pix.height()));
}

// Note: Round is not supported
static void qDrawBorderImage(QPainter *p, const QRenderRule &rule, const QRect& rect)
{
    const QRectF br(rect);

    const QStyleSheetBorderImageData* bi = rule.border()->borderImage();
    const qreal *borders = rule.border()->borders;
    const qreal &l = borders[LeftEdge], &r = borders[RightEdge],
                &t = borders[TopEdge],  &b = borders[BottomEdge];
    QRectF pr = br.adjusted(l, t, -r, -b);

    const QPixmap& pix = bi->pixmap;
    const int *c = bi->cuts;
    QRectF tlc(0, 0, c[LeftEdge], c[TopEdge]);
    if (tlc.isValid())
        p->drawPixmap(QRectF(br.topLeft(), QSizeF(l, t)), pix, tlc);
    QRectF trc(pix.width() - c[RightEdge], 0, c[RightEdge], c[LeftEdge]);
    if (trc.isValid())
        p->drawPixmap(QRectF(br.left() + br.width() - r, br.y(), l, t), pix, trc);
    QRectF blc(0, pix.height() - c[BottomEdge], c[LeftEdge], c[BottomEdge]);
    if (blc.isValid())
        p->drawPixmap(QRectF(br.x(), br.y() + br.height() - b, l, b), pix, blc);
    QRectF brc(pix.width() - c[RightEdge], pix.height() - c[BottomEdge],
               c[RightEdge], c[BottomEdge]);
    if (brc.isValid())
        p->drawPixmap(QRectF(br.x() + br.width() - r, br.y() + br.height() - b, r, b),
                      pix, brc);

    QRectF topEdgeRect(br.x() + l, br.y(), pr.width(), t);
    QRectF bottomEdgeRect(br.x() + l, br.y() + br.height() - b, pr.width(), b);

    switch (bi->horizStretch) {
    case TileMode_Stretch:
        if (bi->topEdgeRect.isValid())
            p->drawPixmap(topEdgeRect, pix, bi->topEdgeRect);
        if (bi->bottomEdgeRect.isValid())
            p->drawPixmap(bottomEdgeRect, pix, bi->bottomEdgeRect);
        if (bi->middleRect.isValid()) {
            if (bi->vertStretch == TileMode_Stretch)
                p->drawPixmap(pr, pix, bi->middleRect);
            else if (bi->vertStretch == TileMode_Repeat) {
                QPixmap scaled = bi->middle.scaled(int(pr.width()), bi->middle.height());
                qDrawCenterTiledPixmap(p, pr, scaled);
            }
        }
        break;
    case TileMode_Repeat:
        if (!bi->topEdge.isNull())
            qDrawCenterTiledPixmap(p, topEdgeRect, bi->topEdge);
        if (!bi->bottomEdge.isNull())
            qDrawCenterTiledPixmap(p, bottomEdgeRect, bi->bottomEdge);
        if (bi->middleRect.isValid()) {
            if (bi->vertStretch == TileMode_Repeat) {
                qDrawCenterTiledPixmap(p, pr, bi->middle);
            } else if (bi->vertStretch == TileMode_Stretch) {
                QPixmap scaled = bi->middle.scaled(bi->middle.width(), int(pr.height()));
                qDrawCenterTiledPixmap(p, pr, scaled);
            }
        }
        break;
    case TileMode_Round:
        if (!bi->topEdge.isNull()) {
            int rwh = (int)pr.width()/ceil(pr.width()/bi->topEdge.width());
            QPixmap scaled = bi->topEdge.scaled(rwh, bi->topEdge.height());
            int blank = int(pr.width()) % rwh;
            p->drawTiledPixmap(QRectF(br.x() + l + blank/2, br.y(), pr.width() - blank, t),
                               scaled);
        }
        if (!bi->bottomEdge.isNull()) {
            int rwh = (int) pr.width()/ceil(pr.width()/bi->bottomEdge.width());
            QPixmap scaled = bi->bottomEdge.scaled(rwh, bi->bottomEdge.height());
            int blank = int(pr.width()) % rwh;
            p->drawTiledPixmap(QRectF(br.x() + l+ blank/2, br.y()+br.height()-b,
                                      pr.width() - blank, b), scaled);
        }
        break;
    default:
        break;
    }

    QRectF leftEdgeRect(br.x(), br.y() + t, l, pr.height());
    QRectF rightEdgeRect(br.x() + br.width()- r, br.y() + t, r, pr.height());

    switch (bi->vertStretch) {
    case TileMode_Stretch:
         if (bi->leftEdgeRect.isValid())
              p->drawPixmap(leftEdgeRect, pix, bi->leftEdgeRect);
        if (bi->rightEdgeRect.isValid())
            p->drawPixmap(rightEdgeRect, pix, bi->rightEdgeRect);
        break;
    case TileMode_Repeat:
        if (!bi->leftEdge.isNull())
            qDrawCenterTiledPixmap(p, leftEdgeRect, bi->leftEdge);
        if (!bi->rightEdge.isNull())
            qDrawCenterTiledPixmap(p, rightEdgeRect, bi->rightEdge);
        break;
    case TileMode_Round:
        if (!bi->leftEdge.isNull()) {
            int rwh = (int) pr.height()/ceil(pr.height()/bi->leftEdge.height());
            QPixmap scaled = bi->leftEdge.scaled(bi->leftEdge.width(), rwh);
            int blank = int(pr.height()) % rwh;
            p->drawTiledPixmap(QRectF(br.x(), br.y() + t + blank/2, l, pr.height() - blank),
                               scaled);
        }
        if (!bi->rightEdge.isNull()) {
            int rwh = (int) pr.height()/ceil(pr.height()/bi->rightEdge.height());
            QPixmap scaled = bi->rightEdge.scaled(bi->rightEdge.width(), rwh);
            int blank = int(pr.height()) % rwh;
            p->drawTiledPixmap(QRectF(br.x() + br.width() - r, br.y()+t+blank/2, r,
                                      pr.height() - blank), scaled);
        }
        break;
    default:
        break;
    }
}

static QRect qBackgroundImageRect(const QRenderRule &rule, const QRect &rect)
{
    Q_ASSERT(rule.hasBackgroundImage());
    QRect r;
    const QStyleSheetBackgroundImageData *background = rule.backgroundImage();
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
    return r;
}

static void qDrawBackgroundImage(QPainter *p, const QRenderRule &rule, const QRect &rect,
                                 Qt::LayoutDirection dir)
{
    Q_ASSERT(rule.hasBackgroundImage());
    const QStyleSheetBackgroundImageData *background = rule.backgroundImage();
    const QPixmap& bgp = background->pixmap;
    if (bgp.isNull())
        return;
    QRect r = qBackgroundImageRect(rule, rect);
    QRect aligned = QStyle::alignedRect(dir, background->position, bgp.size(), r);
    QRect inter = aligned.intersected(r);

    switch (background->repeat) {
    case Repeat_Y:
        p->drawTiledPixmap(inter.x(), r.y(), inter.width(), r.height(), bgp,
                           inter.x() - aligned.x(),
                           bgp.height() - int(aligned.y() - r.y()) % bgp.height());
        break;
    case Repeat_X:
        p->drawTiledPixmap(r.x(), inter.y(), r.width(), inter.height(), bgp,
                           bgp.width() - int(aligned.x() - r.x())%bgp.width(),
                           inter.y() - aligned.y());
        break;
    case Repeat_XY:
        p->drawTiledPixmap(r, bgp,
                           QPoint(bgp.width() - int(aligned.x() - r.x())% bgp.width(),
                                  bgp.height() - int(aligned.y() - r.y())%bgp.height()));
        break;
    case Repeat_None:
    default:
        p->drawPixmap(inter.x(), inter.y(), bgp, inter.x() - aligned.x(),
                      inter.y() - aligned.y(), inter.width(), inter.height());
        break;
    }
}

static void qDrawBorder(QPainter *p, const QRenderRule& rule, const QRect& rect)
{
    Q_ASSERT(rule.hasBorder());
    const QStyleSheetBorderData *border = rule.border();
    if (border->hasBorderImage()) {
        qDrawBorderImage(p, rule, rect);
        return;
    }

    const BorderStyle *styles = border->styles;
    const qreal *borders = border->borders;
    const QColor *colors = border->colors;
    const QRectF br(rect);

    QSize tlr(0, 0), trr(0, 0), brr(0, 0), blr(0, 0);
    if (border->radii[0].isValid()) {
        tlr = border->radii[0];
        trr = border->radii[1];
        blr = border->radii[2];
        brr = border->radii[3];
        if (tlr.width() + trr.width() > br.width()
            || blr.width() + brr.width() > br.width())
            tlr = blr = QSize(0, 0);
        if (tlr.height() + trr.height() > br.height()
            || blr.height() + brr.height() > br.height())
            trr = brr = QSize(0, 0);
    }

    // Drawn in increasing order of precendence
    if (styles[BottomEdge] != BorderStyle_None) {
        qreal dw1 = (blr.width() || qPaintsOver(rule, BottomEdge, LeftEdge)) ? 0 : borders[LeftEdge];
        qreal dw2 = (brr.width() || qPaintsOver(rule, BottomEdge, RightEdge)) ? 0 : borders[RightEdge];
        qreal x1 = br.x() + blr.width();
        qreal y1 = br.y() + br.height() - borders[BottomEdge];
        qreal x2 = br.x() + br.width() - brr.width();
        qreal y2 = br.y() + br.height() ;

        qDrawEdge(p, x1, y1, x2, y2, dw1, dw2, BottomEdge, styles[BottomEdge], colors[BottomEdge]);
        if (blr.width() || brr.width())
            qDrawRoundedCorners(p, x1, y1, x2, y2, blr, brr, BottomEdge, styles[BottomEdge], colors[BottomEdge]);
    }
    if (styles[RightEdge] != BorderStyle_None) {
        qreal dw1 = (trr.height() || qPaintsOver(rule, RightEdge, TopEdge)) ? 0 : borders[TopEdge];
        qreal dw2 = (brr.height() || qPaintsOver(rule, RightEdge, BottomEdge)) ? 0 : borders[BottomEdge];
        qreal x1 = br.x() + br.width() - borders[RightEdge];
        qreal y1 = br.y() + trr.height();
        qreal x2 = br.x() + br.width();
        qreal y2 = br.y() + br.height() - brr.height();

        qDrawEdge(p, x1, y1, x2, y2, dw1, dw2, RightEdge, styles[RightEdge], colors[RightEdge]);
        if (trr.height() || brr.height())
            qDrawRoundedCorners(p, x1, y1, x2, y2, trr, brr, RightEdge, styles[RightEdge], colors[RightEdge]);
    }
    if (styles[LeftEdge] != BorderStyle_None) {
        qreal dw1 = (tlr.height() || qPaintsOver(rule, LeftEdge, TopEdge)) ? 0 : borders[TopEdge];
        qreal dw2 = (blr.height() || qPaintsOver(rule, LeftEdge, BottomEdge)) ? 0 : borders[BottomEdge];
        qreal x1 = br.x();
        qreal y1 = br.y() + tlr.height();
        qreal x2 = br.x() + borders[LeftEdge];
        qreal y2 = br.y() + br.height() - blr.height();

        qDrawEdge(p, x1, y1, x2, y2, dw1, dw2, LeftEdge, styles[LeftEdge], colors[LeftEdge]);
        if (tlr.height() || blr.height())
            qDrawRoundedCorners(p, x1, y1, x2, y2, tlr, blr, LeftEdge, styles[LeftEdge], colors[LeftEdge]);
    }
    if (styles[TopEdge] != BorderStyle_None) {
        qreal dw1 = (tlr.width() || qPaintsOver(rule, TopEdge, LeftEdge)) ? 0 : borders[LeftEdge];
        qreal dw2 = (trr.width() || qPaintsOver(rule, TopEdge, RightEdge)) ? 0 : borders[RightEdge];
        qreal x1 = br.x() + tlr.width();
        qreal y1 = br.y();
        qreal x2 = br.left() + br.width() - trr.width();
        qreal y2 = br.y() + borders[TopEdge];

        qDrawEdge(p, x1, y1, x2, y2, dw1, dw2, TopEdge, styles[TopEdge], colors[TopEdge]);
        if (tlr.width() || trr.width())
            qDrawRoundedCorners(p, x1, y1, x2, y2, tlr, trr, TopEdge, styles[TopEdge], colors[TopEdge]);
    }
}

static void qFillBackground(QPainter *p, const QRenderRule &rule,
                            const QRect& rect, Qt::LayoutDirection)
{
    if (rule.hasPalette() && rule.palette()->background.style() != Qt::NoBrush)
        p->fillRect(rule.borderRect(rect), rule.palette()->background);
}

static void qDrawBackground(QPainter *p, const QRenderRule &rule,
                            const QRect& rect, Qt::LayoutDirection dir)
{
    qFillBackground(p, rule, rect, dir);
    if (rule.hasBackgroundImage())
        qDrawBackgroundImage(p, rule, rect, dir);
}

static void qDrawFrame(QPainter *p, const QRenderRule &rule,
                       const QRect& rect, Qt::LayoutDirection dir)
{
    qDrawBackground(p, rule, rect, dir);
    if (rule.hasBorder())
        qDrawBorder(p, rule, rule.borderRect(rect));
}

static void qConfigurePalette(QPalette *p, const QRenderRule &rule,
                              QPalette::ColorRole fg, QPalette::ColorRole bg)
{
    if (!rule.hasPalette())
        return;
    const QStyleSheetPaletteData *rp = rule.palette();
    if (rp->foreground.style() != Qt::NoBrush) {
        p->setBrush(fg, rp->foreground);
        p->setBrush(QPalette::WindowText, rp->foreground);
        p->setBrush(QPalette::Text, rp->foreground);
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
#define PARENT_WIDGET(x) (WIDGET(x) ? WIDGET(x)->parentWidget() : 0)
#define ISA(type, ptr) (qobject_cast<type *>(ptr) != 0)

class QStyleSheetStyleSelector : public StyleSelector
{
public:
    QStyleSheetStyleSelector() { }

    bool hasNodeName(NodePtr node, const QString& name) const
    { 
        if (WIDGET(node)->inherits(name.toLatin1()))
            return true;
        if (name == QLatin1String("QToolTip")
            && QLatin1String("QTipLabel")== QLatin1String(WIDGET(node)->metaObject()->className()))
            return true;
        return false;
    }
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
QRenderRules QStyleSheetStyle::renderRules(const QWidget *w) const
{
    return renderRulesCache.value(w);
}

QRenderRule QStyleSheetStyle::renderRule(const QWidget *w, QStyle::State state) const
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

    Q_ASSERT(styleRulesCache.contains(w)); // style sheet rules must have been computed!
    const QVector<StyleRule>& styleRules = styleRulesCache.value(w);

    if (!renderRulesCache.contains(w)) // virgin
        renderRulesCache[w].init(styleRules);

    QHash<int, QRenderRule> &computedRulesCache = renderRulesCache[w].computedRulesCache;

    if (computedRulesCache.contains(pseudoState))
        return computedRulesCache.value(pseudoState); // already computed before

    QRenderRule newRule;
    for (int i = 0; i < styleRules.count(); i++) {
        int cssState = styleRules.at(i).selectors.at(0).pseudoState();
        if ((cssState == PseudoState_Unspecified) || (cssState & pseudoState) == cssState)
            newRule.merge(styleRules.at(i).declarations);
    }

    newRule.fixupBorder();
    computedRulesCache[pseudoState] = newRule;
    return newRule;
}

QRenderRule QStyleSheetStyle::renderRule(const QWidget *w, const QStyleOption *opt) const
{
    if (!w || !styleRulesCache.contains(w))
        return QRenderRule();

    QStyle::State state = opt ? opt->state : QStyle::State(QStyle::State_Enabled);

    // Add hacks for <w, opt> here
#ifndef QT_NO_LINEEDIT
    if (qobject_cast<const QLineEdit *>(w))
        state &= ~QStyle::State_Sunken;
#endif

    return renderRule(w, state);
}

// remember to revert changes in unsetPalette
void QStyleSheetStyle::setPalette(QWidget *w)
{
    const QRenderRule &hoverRule = renderRule(w, QStyle::State_MouseOver);
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

#ifndef QT_NO_TOOLTIP
    const bool isToolTip = QLatin1String(w->metaObject()->className()) == "QTipLabel";
    QPalette p = isToolTip ? QToolTip::palette() : qApp->palette();
#else
    QPalette p = qApp->palette();
#endif
    
    for (int i = 0; i < 2; i++) {
        const QRenderRule &rule = renderRule(w, map[i].state);
        p.setCurrentColorGroup(map[i].group);

#ifndef QT_NO_COMBOBOX
        if (QComboBox *cmb = qobject_cast<QComboBox *>(w)) {
            if (!cmb->isEditable())
                qConfigurePalette(&p, rule, QPalette::ButtonText, QPalette::Button);
            else
                qConfigurePalette(&p, rule, QPalette::Text, QPalette::Base);
        } else
#endif
        if (qobject_cast<QFrame *>(w)) {
            qConfigurePalette(&p, rule, QPalette::Text, QPalette::Base);
            if (qobject_cast<QAbstractScrollArea *>(w)) {
                if (rule.hasBackground())
                    p.setColor(QPalette::Base, Qt::transparent);
            }
        } else {
            qConfigurePalette(&p, rule, w->foregroundRole(), w->backgroundRole());
        }
    }

    // sync the viewport and the abstract scroll area
    if (QAbstractScrollArea *sa = qobject_cast<QAbstractScrollArea *>(w))
        sa->viewport()->setPalette(p);

#ifndef QT_NO_TOOLTIP
    isToolTip ? QToolTip::setPalette(p) : w->setPalette(p);
#else
    w->setPalette(p);
#endif
}

void QStyleSheetStyle::unsetPalette(QWidget *w)
{
    w->setPalette(qApp->palette(w));
}

void QStyleSheetStyle::repolish(QWidget *w)
{
    QList<const QWidget *> children = qFindChildren<const QWidget *>(w, QString());
    children.append(w);
    update(children);
}

/*!\reimp*/
QIcon QStyleSheetStyle::standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt,
                                                   const QWidget *widget) const
{
    return QCommonStyle::standardIconImplementation(standardIcon, opt, widget);
}

void QStyleSheetStyle::repolish(QApplication *)
{
    update(styleRulesCache.keys());
}

void QStyleSheetStyle::update(const QList<const QWidget *>& widgets)
{
    for (int i = 0; i < widgets.size(); ++i) {
        QWidget *widget = const_cast<QWidget *>(widgets.at(i));
        styleRulesCache.remove(widget);
        renderRulesCache.remove(widget);
        polish(widget);
        widget->update();
        widget->updateGeometry();
    }
}

void QStyleSheetStyle::widgetDestroyed(QObject *o)
{
    renderRulesCache.remove(static_cast<const QWidget *>(o));
    renderRulesCache.remove(static_cast<const QWidget *>(o));
}

static bool unstylable(QWidget *w)
{
    if (QAbstractScrollArea *sa = qobject_cast<QAbstractScrollArea *>(w->parentWidget())) {
        if (sa->viewport() == w)
            return true;
    } else if (qobject_cast<QLineEdit *>(w)) {
        QWidget *parent = w->parentWidget();
        if (qobject_cast<QComboBox *>(parent))
            return true;
    }

    return false;
}

// remember to revert changes in unpolish
void QStyleSheetStyle::polish(QWidget *w)
{
    baseStyle()->polish(w);
    if (unstylable(w))
        return;

    QObject::connect(w, SIGNAL(destroyed(QObject *)), this, SLOT(widgetDestroyed(QObject *)));
    renderRulesCache.remove(w);
    QVector<QCss::StyleRule> rules = computeStyleSheet(w);
    styleRulesCache[w] = rules;
    if (rules.isEmpty()) {
        unsetPalette(w);
    } else {
        setPalette(w);
    }

    QEvent e(QEvent::StyleChange);
    QApplication::sendEvent(w, &e);
}

QVector<QCss::StyleRule> QStyleSheetStyle::computeStyleSheet(QWidget *w)
{
    QStyleSheetStyleSelector styleSelector;
    StyleSheet appSs;
    Parser parser1(qApp->styleSheet());
    if (!parser1.parse(&appSs))
        qWarning("Could not parse application stylesheet");
    styleSelector.styleSheets += appSs;

    QList<QCss::StyleSheet> inheritedSs;
    for (QWidget *wid = w->parentWidget(); wid; wid = wid->parentWidget()) {
        if (wid->styleSheet().isEmpty())
            continue;
        StyleSheet ss;
        Parser parser(wid->styleSheet());
        if (!parser.parse(&ss))
            qWarning("Could not parse stylesheet");
        inheritedSs.prepend(ss);
    }
    styleSelector.styleSheets += inheritedSs;

    StyleSheet widgetSs;
    Parser parser2(w->styleSheet());
    if (!parser2.parse(&widgetSs))
        qWarning("Could not parse widget stylesheet");

    styleSelector.styleSheets += widgetSs;

    StyleSelector::NodePtr n;
    n.ptr = w;
    return styleSelector.styleRulesForNode(n);
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

void QStyleSheetStyle::unpolish(QWidget *w)
{
    styleRulesCache.remove(w);
    renderRulesCache.remove(w);
    baseStyle()->unpolish(w);
    QObject::disconnect(w, SIGNAL(destroyed(QObject *)),
                       this, SLOT(widgetDestroyed(QObject *)));
}

void QStyleSheetStyle::unpolish(QApplication *app)
{
    styleRulesCache.clear();
    renderRulesCache.clear();
    baseStyle()->unpolish(app);
}

bool QStyleSheetStyle::baseStyleCanRender(QStyleSheetStyle::WidgetType wt, const QRenderRule &rule) const
{
    switch (wt) {
    case PushButton:
        if (rule.hasBackgroundImage() || rule.hasBorder() || rule.hasBox())
            return false;
        if (!rule.hasPalette())
            return true;
        return (rule.palette()->background.style() == Qt::NoBrush);

    case LineEdit:
        return !rule.hasBorder();

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

/////////////////////////////////////////////////////////////////////////////////////////
QStyleSheetStyle::QStyleSheetStyle(QStyle *baseStyle, QObject *parent)
: bs(baseStyle)
{
    setParent(parent);
}

void QStyleSheetStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                 const QWidget *w) const
{
    const QRenderRule &rule = renderRule(w, opt);
    if (rule.isEmpty()) {
        baseStyle()->drawComplexControl(cc, opt, p, w);
        return;
    }

    switch (cc) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            if (!rule.hasBorder()) {
                QStyleOptionComboBox cmbOpt(*cb);
                qConfigurePalette(&cmbOpt.palette, rule, QPalette::Text, QPalette::Base);
                baseStyle()->drawComplexControl(cc, &cmbOpt, p, w);
            } else {
                qDrawFrame(p, rule, opt->rect, opt->direction);
            }
            return;
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
        break;
    }

    baseStyle()->drawComplexControl(cc, opt, p, w);
}

void QStyleSheetStyle::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                          const QWidget *w) const
{
    const QRenderRules &rules = renderRules(w);
    const QRenderRule &rule = renderRule(w, opt);
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
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QStyleOptionButton btnOpt(*btn);
            if (rule.hasBorder() || rule.hasBackground()
                || ((btn->features & QStyleOptionButton::HasMenu) && rule.hasIcon("down-arrow"))) {
                QStyleOptionButton btnOpt(*btn);
                if (rule.hasBorder() || rule.hasBackground()) {
                    qDrawFrame(p, rule, opt->rect, opt->direction);
                } else {
                    btnOpt.features &= ~QStyleOptionButton::HasMenu;
                    baseStyle()->drawControl(ce, &btnOpt, p, w);
                }

                if (btn->features & QStyleOptionButton::HasMenu) {
                    int mbi = pixelMetric(PM_MenuButtonIndicator, btn, w);
                    btnOpt.rect = rule.contentsRect(btnOpt.rect);
                    QRect &ir = btnOpt.rect;
                    const int spacing = rule.hasBox() ? qRound(rule.box()->spacing) : 0;
                    ir = QRect(ir.left() + ir.width() - mbi + spacing,
                                (ir.height() - mbi)/2, mbi - spacing, mbi);
                    drawPrimitive(PE_IndicatorArrowDown, &btnOpt, p, w);
                }
            } else {
                baseStyle()->drawControl(ce, opt, p, w);
            }
        }
        return;

    case CE_PushButtonLabel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QStyleOptionButton butOpt(*btn);
            qConfigurePalette(&butOpt.palette, rule, QPalette::ButtonText, QPalette::Button);
            if (rule.hasBox()) // shift+padding
                ParentStyle::drawControl(ce, &butOpt, p, w);
            else
                baseStyle()->drawControl(ce, &butOpt, p, w);
        }
        return;

    case CE_RadioButton:
    case CE_CheckBox:
        if (rule.hasBorder() || rule.hasBackgroundImage())
            qDrawFrame(p, rule, opt->rect, opt->direction);
        else
            qFillBackground(p, rule, opt->rect, opt->direction);
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

    case CE_Splitter:
        if (rule.hasBorder() || rule.hasBackground()) {
            qDrawFrame(p, rule, opt->rect, opt->direction);
            return;
        }
        break;

    case CE_MenuEmptyArea:
    case CE_MenuBarEmptyArea:
        if (const QStyleOptionMenuItem *m = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            if (rule.hasBackground()) {
                qDrawBackground(p, rule, opt->rect, opt->direction);
            } else {
                QStyleOptionMenuItem mi(*m);
                qConfigurePalette(&mi.palette, rule, QPalette::ButtonText, QPalette::Button);
                baseStyle()->drawControl(ce, &mi, p, w);
            }
        }
        return;

    case CE_MenuBarItem:
    case CE_MenuItem:
        if (const QStyleOptionMenuItem *m = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            QStyleOptionMenuItem mi(*m);
            qConfigurePalette(&mi.palette, rule, QPalette::ButtonText, QPalette::Button);
            if (rule.hasBackgroundImage()) {
                qDrawBackgroundImage(p, rule, opt->rect, opt->direction);
                mi.palette.setBrush(QPalette::Button, rule.backgroundImage()->pixmap); // FIXME: Set origin
                ParentStyle::drawControl(ce, &mi, p, w);
            } else {
                baseStyle()->drawControl(ce, &mi, p, w);
            }
        }
        return;

    case CE_MenuScroller:
    case CE_MenuTearoff:
        baseStyle()->drawControl(ce, opt, p, w);
        return;

    case CE_ComboBoxLabel:
        if (!rule.hasBorder()) {
            baseStyle()->drawControl(ce, opt, p, w);
        } else {
            QPen savedPen = p->pen();
            if (rule.hasPalette() && rule.palette()->foreground.style() != Qt::NoBrush)
                p->setPen(rule.palette()->foreground.color());
            ParentStyle::drawControl(ce, opt, p, w);
            p->setPen(savedPen);
        }
        return;

    case CE_SizeGrip:
        if (!rule.hasIcon("grip"))
            break;
        if (const QStyleOptionSizeGrip *sgOpt = qstyleoption_cast<const QStyleOptionSizeGrip *>(opt)) {
            p->save();
            char *icon = "grip";
            switch (sgOpt->corner) {
            case Qt::BottomRightCorner: break;
            case Qt::BottomLeftCorner: p->rotate(90); break;
            case Qt::TopLeftCorner: p->rotate(180); break;
            case Qt::TopRightCorner: p->rotate(270); break;
            default: icon = 0; break;
            }
            if (icon == 0)
                break;
            drawIcon(p, opt->rect, Qt::AlignCenter, rules, rule, icon);
            p->restore();
            return;
        }
        break;

    case CE_ProgressBar:
        ParentStyle::drawControl(ce, opt, p, w);
        break;

    case CE_ProgressBarGroove:
        if (rule.hasBorder()) {
            qDrawFrame(p, rule, opt->rect, opt->direction);
            return;
        }
        if (!rule.hasBox())
            break;
        if (const QStyleOptionProgressBarV2 *pb2
                                    = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(opt)) {
            QStyleOptionProgressBarV2 newPb2(*pb2);
            newPb2.rect = rule.borderRect(opt->rect);
            baseStyle()->drawControl(ce, &newPb2, p, w);
            return;
        }
        break;

    case CE_ProgressBarLabel:
        break;

    case CE_ProgressBarContents:
        if (!rule.hasIcon("chunk"))
            break;
        break;

    default:
        qDebug() << "controlElement: Panic " << ce;
        break;
    }

    baseStyle()->drawControl(ce, opt, p, w);
}

void QStyleSheetStyle::drawIcon(QPainter *p, const QRect &rect, int alignment,
                                const QRenderRules& rules, const QRenderRule& rule,
                                const char *icon) const
{
    QSize sz = rules.hasIconSize(icon) ? rules.iconSize(icon) : rect.size();
    drawItemPixmap(p, rect, alignment, rule.pixmap(icon, sz));
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
    const QRenderRules &rules = renderRules(w);
    const QRenderRule &rule = renderRule(w, opt);
    if (rule.isEmpty()) {
        baseStyle()->drawPrimitive(pe, opt, p, w);
        return;
    }

    switch (pe) {
    case PE_IndicatorArrowDown:
        if (rule.hasIcon("down-arrow")) {
            drawIcon(p, opt->rect, Qt::AlignCenter, rules, rule, "down-arrow");
            return;
        }
        break;

    case PE_PanelButtonCommand:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QStyleOptionButton butOpt(*btn);
            qConfigurePalette(&butOpt.palette, rule, QPalette::ButtonText, QPalette::Button);
            if (!rule.hasBorder())
                baseStyle()->drawPrimitive(pe, &butOpt, p, w);
            else
                qDrawBorder(p, rule, opt->rect);
        }
        return;

    // how do we specify these in the css?
    case PE_FrameDefaultButton:
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

    case PE_Frame:
    case PE_PanelLineEdit:
        if (const QStyleOptionFrame *frm = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            if (!rule.hasBorder()) {
                if (pe == PE_Frame)
                    qFillBackground(p, rule, opt->rect, opt->direction);
                if (rule.hasBackgroundImage())
                    qDrawBackgroundImage(p, rule, opt->rect, opt->direction);
                QStyleOptionFrame frmOpt(*frm);
                qConfigurePalette(&frmOpt.palette, rule, QPalette::Text, QPalette::Base);
                frmOpt.rect = rule.borderRect(frmOpt.rect); // apply padding
                baseStyle()->drawPrimitive(pe, &frmOpt, p, w);
            } else {
                qDrawFrame(p, rule, opt->rect, opt->direction);
            }
        }
        return;

    // RadioButton and CheckBox
    case PE_IndicatorRadioButton:
    case PE_IndicatorCheckBox:
        if (rule.hasIcon("indicator")) {
            drawIcon(p, opt->rect, Qt::AlignCenter, rules, rule, "indicator");
            return;
        }
        break;

    case PE_Widget:
        qDrawFrame(p, rule, opt->rect, opt->direction);
        return;

    case PE_FrameMenu:
    case PE_PanelMenuBar:
        if (rule.hasBorder()) {
            qDrawBorder(p, rule, rule.borderRect(opt->rect));
            return;
        }
        break;

        // Menu stuff that would be nice to customize
    case PE_IndicatorMenuCheckMark:
    case PE_IndicatorArrowLeft:
    case PE_IndicatorArrowRight:
        baseStyle()->drawPrimitive(pe, opt, p, w);
        return;

    case PE_PanelTipLabel: 
        if (rule.hasFrame()) {
            qDrawFrame(p, rule, opt->rect, opt->direction);
            return;
        }
        if (rule.hasBackground())
            qDrawBackground(p, rule, opt->rect, opt->direction);
        break;

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
    const QRenderRules& rules = renderRules(w);
    const QRenderRule& rule = renderRule(w, opt);
    if (rule.isEmpty())
        return baseStyle()->pixelMetric(m, opt, w);

    switch (m) {
    case PM_MenuButtonIndicator:
        if (rules.hasIconSize("down-arrow"))
            return rules.iconSize("down-arrow").width();
        break;

    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
    case PM_ButtonMargin:
        // do it with the padding, if you want a shift
        if (rule.hasBox())
            return 0;

    case PM_DefaultFrameWidth:
    case PM_ButtonDefaultIndicator:
        break;

    case PM_ExclusiveIndicatorWidth:
    case PM_IndicatorWidth:
        if (rules.hasIconSize("indicator"))
            return rules.iconSize("indicator").width();
        break;

    case PM_ExclusiveIndicatorHeight:
    case PM_IndicatorHeight:
        if (rules.hasIconSize("indicator"))
            return rules.iconSize("indicator").height();
        break;

    case PM_ToolTipLabelFrameWidth:
        if (rule.hasBorder())
            return rule.border()->borders[LeftEdge];
        break;

    case PM_MenuPanelWidth:
    case PM_MenuBarPanelWidth: // border + margin (support only one width)
        if (rule.hasBorder() || rule.hasBox())
            return rule.sizeWithMargin(rule.sizeWithBorder(QSize(0, 0))).width()/2;
        break;

    case PM_MenuHMargin:
    case PM_MenuBarHMargin:
        if (rule.hasBox())
            return qRound(rule.box()->paddings[LeftEdge]);
        break;

    case PM_MenuVMargin:
    case PM_MenuBarVMargin:
        if (rule.hasBox())
            return qRound(rule.box()->paddings[TopEdge]);
        break;

    case PM_MenuBarItemSpacing:
        if (rule.hasBox())
            return qRound(rule.box()->spacing);
        break;

    case PM_ProgressBarChunkWidth:
        if (rules.hasIconSize("chunk"))
            return rules.iconSize("chunk").width();
        break;

    case PM_SmallIconSize:
    case PM_MenuDesktopFrameWidth:
    case PM_MenuTearoffHeight:
    case PM_MenuScrollerHeight: 
        break;
    case PM_ToolBarExtensionExtent:
        break;

    case PM_SplitterWidth:
        if (rules.contentsWidth != -1)
            return rules.contentsWidth;
        break;

    case PM_SizeGripSize:
        break;

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
    const QRenderRules &rules = renderRules(w);
    const QRenderRule &rule = renderRule(w, opt);
    if (rule.isEmpty())
        return baseStyle()->sizeFromContents(ct, opt, csz, w);

    QSize sz = csz.expandedTo(rules.contentsSize());
    
    switch (ct) {
    case CT_PushButton:
        if (rule.hasBorder() || rule.hasBox()) {
            sz = rule.boxRect(QRect(0, 0, sz.width(), sz.height())).size();
            if (!rule.hasBox()) { // some spacing when no padding
                int bm = baseStyle()->pixelMetric(QStyle::PM_ButtonMargin, opt, w);
                sz += QSize(bm, bm);
            } else {
                sz += QSize(qRound(rule.box()->spacing), 0);
            }
            return sz;
        } else {
            return baseStyle()->sizeFromContents(ct, opt, sz, w);
        }
        break;

    case CT_LineEdit: // does not contains fw
        if (rule.hasBorder()) {
            return rule.boxRect(QRect(0, 0, sz.width(), sz.height())).size();
        } else {
            QSize baseSize = baseStyle()->sizeFromContents(ct, opt, sz, w);
            if (!rule.hasBox())
                return baseSize;
            QSize parentSize = ParentStyle::sizeFromContents(ct, opt, sz, w);
            if (parentSize != baseSize)
                return baseSize;
            return baseSize + rule.box()->paddingSize() + rule.box()->marginSize();
        }
        break;

    case CT_CheckBox:
    case CT_RadioButton:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            if (rule.hasBox() || rule.hasBorder() || rules.hasIconSize("indicator")) {
                bool isRadio = (ct == CT_RadioButton);
                int iw = pixelMetric(isRadio ? PM_ExclusiveIndicatorWidth
                                             : PM_IndicatorWidth, btn, w);
                int ih = pixelMetric(isRadio ? PM_ExclusiveIndicatorHeight
                                             : PM_IndicatorHeight, btn, w);

                QSize margins(0, 0);
                if (!rule.hasBox() && !rule.hasBorder())
                    margins = QSize((!btn->icon.isNull() && btn->text.isEmpty()) ? 0 : 10, 4);
                int spacing = rule.hasBox() ? qRound(rule.box()->spacing) : 6;
                sz = sz + QSize(iw+qMax(spacing, 0), 0) + margins;
                sz.setHeight(qMax(sz.height(), ih));
                return rule.boxRect(QRect(0, 0, sz.width(), sz.height())).size();
            }
        }
        break;

    case CT_Menu:
    case CT_MenuItem:

    case CT_MenuBar:
        if (rule.hasBox())
            return sz;
        break;

    case CT_MenuBarItem:
        break;

    case CT_ComboBox:
        if (rule.hasBorder())
            return rule.boxRect(QRect(0, 0, sz.width(), sz.height())).size();
        break;

    case CT_SizeGrip:
        if (rules.hasIconSize("grip"))
            return rules.iconSize("grip");
        break;

    case CT_Splitter:
    case CT_ProgressBar:
        if (rule.hasBorder() || rule.hasBox()) {
            return rule.boxRect(QRect(0, 0,  sz.width(), sz.height())).size();
        }
        break;

    default:
        static QList<int> panics;
        if (!panics.contains(ct)) {
            qDebug() << "sizeFromContents: Panic " << ct;
            panics.append(ct);
        }
        break;
    }

    return baseStyle()->sizeFromContents(ct, opt, sz, w);
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
    const QRenderRule &rule = renderRule(w, opt);
    if (rule.isEmpty())
        return baseStyle()->subControlRect(cc, opt, sc, w);

    QRect r = opt->rect;
    switch (cc) {
    case CC_ComboBox:
        if (rule.hasBorder()) {
            int rightBorder = qRound(rule.border()->borders[RightEdge]);
            switch (sc) {
            case SC_ComboBoxArrow:
                r = rule.borderRect(r);
                r = QRect(r.left() + r.width() - rightBorder, r.top(),
                        rightBorder, r.height());
                break;
            case SC_ComboBoxEditField:
                r = rule.contentsRect(r);
                break;
            case SC_ComboBoxListBoxPopup:
                r.setBottom(r.top() + r.height());
                break;
            case SC_ComboBoxFrame:
                //r = rule.borderRect(r);
                break;
            default:
                break;
            }
            return visualRect(opt->direction, opt->rect, r);
        }
        break;

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
        break;
    }

    return baseStyle()->subControlRect(cc, opt, sc, w);;
}

QRect QStyleSheetStyle::subElementRect(SubElement se, const QStyleOption *opt, const QWidget *w) const
{
    const QRenderRules &rules = renderRules(w);
    const QRenderRule &rule = renderRule(w, opt);
    if (rule.isEmpty())
        return baseStyle()->subElementRect(se, opt, w);

    switch (se) {
    // PushButton
    case SE_PushButtonContents:
    case SE_PushButtonFocusRect:
        if (rule.hasBorder() || rule.hasBox())
            return visualRect(opt->direction, opt->rect, rule.contentsRect(opt->rect));
        break;

    case SE_LineEditContents:
    case SE_FrameContents:
        if (rule.hasBorder()) {
            return visualRect(opt->direction, opt->rect, rule.contentsRect(opt->rect));
        } else {
            QRect baseRect = baseStyle()->subElementRect(se, opt, w);
            if (!rule.hasBox())
                return baseRect;
            QRect parentRect = ParentStyle::subElementRect(se, opt, w);
            if (baseRect != parentRect)
                return baseRect;
            return rule.borderRect(rule.contentsRect(baseRect));
        }
        break;

    case SE_CheckBoxIndicator:
    case SE_RadioButtonIndicator:
        if (rule.hasBox() || rule.hasBorder() || rules.hasIconSize("indicator")) {
            QStyleOption optCopy(*opt);
            optCopy.rect = rule.contentsRect(opt->rect);
            return ParentStyle::subElementRect(se, &optCopy, w);
        }
        break;

    case SE_CheckBoxContents:
    case SE_RadioButtonContents:
        if (rule.hasBox() || rule.hasBorder() || !rules.hasIconSize("indicator")) {
            bool isRadio = se == SE_RadioButtonContents;
            QRect ir = subElementRect(isRadio ? SE_RadioButtonIndicator : SE_CheckBoxIndicator,
                                      opt, w);
            ir = visualRect(opt->direction, opt->rect, ir);
            const int spacing = rule.hasBox() ? qRound(rule.box()->spacing) : 6;
            QRect cr = rule.contentsRect(opt->rect);
            ir.setRect(ir.left() + ir.width() + spacing, cr.y(),
                       cr.width() - ir.width() - spacing, cr.height());
            return visualRect(opt->direction, opt->rect, ir);
        }
        break;

    case SE_ProgressBarGroove:
        break;
    case SE_ProgressBarContents:
    case SE_ProgressBarLabel:
        if (rule.hasBox() || rule.hasBorder())
            return visualRect(opt->direction, rule.contentsRect(opt->rect), opt->rect);
        break;

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

#endif // QT_NO_STYLE_STYLESHEET
