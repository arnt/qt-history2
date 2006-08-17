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
#include <qlineedit.h>
#include <qwindowsstyle.h>
#include <qcombobox.h>
#include <qwindowsstyle.h>
#include <qplastiquestyle.h>
#include "private/qcssparser_p.h"
#include "private/qmath_p.h"
#include <qabstractscrollarea.h>
#include <qtooltip.h>

using namespace QCss;

QHash<const QWidget *, QVector<QCss::StyleRule> > QStyleSheetStyle::styleRulesCache;
QHash<const QWidget *, QRenderRules> QStyleSheetStyle::renderRulesCache;

#define ceil(x) ((int)(x) + ((x) > 0 && (x) != (int)(x)))

struct QStyleSheetBorderImageData
{
    QStyleSheetBorderImageData()
    {
        for (int i = 0; i < 4; i++) 
            cuts[i] = -1;
    }
    QPixmap topEdge, bottomEdge, leftEdge, rightEdge, middle;
    QRect topEdgeRect, bottomEdgeRect, leftEdgeRect, rightEdgeRect, middleRect;
    QRect topLeftCorner, topRightCorner, bottomRightCorner, bottomLeftCorner;
    int cuts[4];
    QPixmap pixmap;
    QImage image;
    QCss::TileMode horizStretch, vertStretch;

    void cutBorderImage();
};

struct QStyleSheetBackgroundData
{
    QStyleSheetBackgroundData(const QColor& c, const QPixmap& p, QCss::Repeat r, 
                              Qt::Alignment a, QCss::Origin o) 
        : color(c), pixmap(p), repeat(r), position(a), origin(o) { }

    QColor color;
    QPixmap pixmap;
    QCss::Repeat repeat;
    Qt::Alignment position;
    QCss::Origin origin;
};

struct QStyleSheetBorderData
{
    QStyleSheetBorderData() : bi(0) 
    {
        for (int i = 0; i < 4; i++) {
            borders[i] = 0;
            styles[i] = QCss::BorderStyle_None;
        }
    }

    QStyleSheetBorderData(int *b, QColor *c, QCss::BorderStyle *s, QSize *r) : bi(0)
    {
        for (int i = 0; i < 4; i++) {
            borders[i] = b[i];
            styles[i] = s[i];
            colors[i] = c[i];
            radii[i] = r[i];
        }
    }

    int borders[4];
    QColor colors[4];
    QCss::BorderStyle styles[4];
    QSize radii[4]; // topleft, topright, bottomleft, bottomright

    const QStyleSheetBorderImageData *borderImage() const
    { return bi; }
    bool hasBorderImage() const { return bi!=0; }

    QStyleSheetBorderImageData *bi;
};

struct QStyleSheetBoxData
{
    QStyleSheetBoxData(int *m, int *p, int s) : spacing(s)
    {
        for (int i = 0; i < 4; i++) {
            margins[i] = m[i];
            paddings[i] = p[i];
        }
    }

    int margins[4];
    int paddings[4];

    int spacing;

    QSizeF marginSizeF() const {
        return QSizeF(margins[QCss::LeftEdge] + margins[QCss::RightEdge],
                      margins[QCss::TopEdge] + margins[QCss::BottomEdge]);
    }
    QSize marginSize() const {
        return marginSizeF().toSize();
    }
    QSizeF paddingSizeF() const {
        return QSizeF(paddings[QCss::LeftEdge] + paddings[QCss::RightEdge],
                      paddings[QCss::TopEdge] + paddings[QCss::BottomEdge]);
    }
    QSize paddingSize() const {
        return paddingSizeF().toSize();
    }
};

struct QStyleSheetPaletteData
{
    QStyleSheetPaletteData(const QColor &fg, const QColor &sfg, const QBrush &sbg, 
                           const QBrush &abg)
        : foreground(fg), selectionForeground(sfg), selectionBackground(sbg),
          alternateBackground(abg) { }

    QColor foreground;
    QColor selectionForeground;
    QBrush selectionBackground;
    QBrush alternateBackground;
};

struct QStyleSheetGeometryData
{
    QStyleSheetGeometryData(int w, int h, int minw, int minh) 
        : minWidth(minw), minHeight(minh), width(w), height(h) { }

    int minWidth, minHeight, width, height;
};

struct QStyleSheetPositionData
{
    QStyleSheetPositionData(int l, int t, Origin o, Qt::Alignment p) 
        : left(l), top(t), origin(o), position(p) { }

    int left, top;
    Origin origin;
    Qt::Alignment position;
};

class QRenderRule
{
public:
    QRenderRule() : pal(0), b(0), bg(0), bd(0), geo(0), p(0), isCopy(false) { }
    QRenderRule(const QRenderRule &other) : image(other.image), imageRect(other.imageRect),
       font(other.font), styleHints(other.styleHints), pal(other.pal), b(other.b), bg(other.bg), 
       bd(other.bd), geo(other.geo), p(other.p), isCopy(true) { }
    QRenderRule(const QVector<QCss::Declaration> &);
    ~QRenderRule() 
    { 
        if (isCopy) 
            return;
        //delete pal; delete b; delete bg; delete bd; delete geo; delete p;
    }

    bool isEmpty() const
    { return pal == 0 && b == 0 && bg == 0 && bd == 0 && geo == 0 && image.isNull(); }

    QRect borderRect(const QRect &r) const;
    QRect paddingRect(const QRect &r) const;
    QRect contentsRect(const QRect &r) const;
    QRect boxRect(const QRect &r) const;
    QSize boxSize(const QSize &s) const;
    QRect originRect(const QRect &rect, Origin origin) const;
    QRect positionRect(const QRenderRule &rule, Qt::LayoutDirection dir, const QRect &rect,
                       const QSize &defSize, Origin defOrigin, Qt::Alignment defPosition) const;
    QRect positionRect(const QRect &originRect, Qt::LayoutDirection dir,
                       const QSize &defSize, Qt::Alignment defPosition) const;

    bool paintsOver(Edge e1, Edge e2);
    void drawBorder(QPainter *, const QRect&, Qt::LayoutDirection);
    void drawBorderImage(QPainter *, const QRect&, Qt::LayoutDirection);
    void drawBackground(QPainter *, const QRect&, Qt::LayoutDirection);
    void drawBackgroundImage(QPainter *, const QRect&, Qt::LayoutDirection);
    void drawFrame(QPainter *, const QRect&, Qt::LayoutDirection);
    void drawImage(QPainter *p, const QRect &rect);
    void drawRule(QPainter *, const QRect&, Qt::LayoutDirection);
    void configurePalette(QPalette *, QPalette::ColorRole, QPalette::ColorRole);

    const QStyleSheetPaletteData *palette() const { return pal; }
    const QStyleSheetBoxData *box() const { return b; }
        const QStyleSheetBackgroundData *background() const { return bg; }
    const QStyleSheetBorderData *border() const { return bd; }
    const QStyleSheetGeometryData *geometry() const { return geo; }
    const QStyleSheetPositionData *position() const { return p; }

    bool hasPalette() const { return pal != 0; }
    bool hasBackground() const { return bg != 0; }
    bool hasBox() const { return b != 0; }
    bool hasBorder() const { return bd != 0; }
    bool hasPosition() const { return p != 0; }
    bool hasDrawable() const { return hasBorder() || hasBackground() || hasImage(); }

    QSize minimumContentsSize() const 
    { return geo ? QSize(geo->minWidth, geo->minHeight) : QSize(0, 0); }

    QSize contentsSize() const 
    { return geo ? QSize(geo->width, geo->height) : imageRect.size(); }
    bool hasContentsSize() const 
    { return (geo && (geo->width != -1 || geo->height != -1)) || imageRect.isValid(); }

    QSize size() const 
    { return boxSize(geo ? QSizeF(geo->width, geo->height).toSize() : imageRect.size()); }

    bool hasImage() const { return !image.isNull(); }
    QPixmap image;
    QRect imageRect;
    QFont font;

    QHash<QString, int> styleHints;
    bool hasStyleHint(const QString& sh) const { return styleHints.contains(sh); }
    int styleHint(const QString& sh) const { return styleHints.value(sh); }

private:
    void fixupBorder();

    // note that copy constructor will just copy pointers. this is intentional
    // since a copy of a rule is never maintained outside of painting code
    QStyleSheetPaletteData *pal;
    QStyleSheetBoxData *b;
    QStyleSheetBackgroundData *bg;
    QStyleSheetBorderData *bd;
    QStyleSheetGeometryData *geo;
    QStyleSheetPositionData *p;

    bool isCopy;
};

///////////////////////////////////////////////////////////////////////////////////////////
static const char *knownStyleHints[] = {
    "activate-on-singleclick",
    "button-layout",
    "etch-disabled-text",
    "gridline-color",
    "lineedit-password-character",
    "opacity",
    "show-decoration-selected"
};

static const char **endKnownStyleHints = knownStyleHints + sizeof(knownStyleHints)/sizeof(char *);

QRenderRule::QRenderRule(const QVector<Declaration> &declarations)
: pal(0), b(0), bg(0), bd(0), geo(0), p(0)
{
    ValueExtractor v(declarations);
    int w = -1, h = -1, minw = -1, minh = -1;
    if (v.extractGeometry(&w, &h, &minw, &minh))
        geo = new QStyleSheetGeometryData(w, h, minw, minh);

    int left = 0, top = 0;
    Origin origin = Origin_Unknown;
    Qt::Alignment position = 0;
    if (v.extractPosition(&left, &top, &origin, &position))
        p = new QStyleSheetPositionData(left, top, origin, position);

    int margins[4], paddings[4], spacing = -1;
    for (int i = 0; i < 4; i++) 
        margins[i] = paddings[i] = 0;
    if (v.extractBox(margins, paddings, &spacing))
        b = new QStyleSheetBoxData(margins, paddings, spacing);

    int borders[4];
    QColor colors[4];
    QCss::BorderStyle styles[4];
    QSize radii[4];
    for (int i = 0; i < 4; i++) {
        borders[i] = 0;
        styles[i] = BorderStyle_None;
    }
    if (v.extractBorder(borders, colors, styles, radii))
        bd = new QStyleSheetBorderData(borders, colors, styles, radii);

    QColor color;
    QString uri;
    Repeat repeat = Repeat_XY;
    Qt::Alignment alignment = Qt::AlignTop | Qt::AlignLeft;
    origin = Origin_Padding;
    if (v.extractBackground(&color, &uri, &repeat, &alignment, &origin))
        bg = new QStyleSheetBackgroundData(color, QPixmap(uri), repeat, alignment, origin);

    QColor sfg, fg;
    QBrush sbg, abg;
    if (v.extractPalette(&fg, &sfg, &sbg, &abg))
        pal = new QStyleSheetPaletteData(fg, sfg, sbg, abg);

    // FIXME: Need to take into account the widget fonts
    int adj = -255;
    v.extractFont(&font, &adj);

    for (int i = 0; i < declarations.count(); i++) {
        const Declaration& decl = declarations.at(i);
        if (decl.propertyId == BorderImage) {
            if (!bd)
                bd = new QStyleSheetBorderData;
            if (!bd->bi)
                bd->bi = new QStyleSheetBorderImageData;

            QStyleSheetBorderImageData *bi = bd->bi;
            QString uri;
            decl.borderImageValue(&uri, bi->cuts, &bi->horizStretch, &bi->vertStretch);
            if (!uri.isEmpty())
                bi->pixmap = QPixmap(uri);
        } else if (decl.propertyId == Image) {
            image = QPixmap(decl.uriValue());
            if (!imageRect.isValid())
                imageRect = QRect(0, 0, image.width(), image.height());
        } else if (qBinaryFind(knownStyleHints, endKnownStyleHints, decl.property) != endKnownStyleHints) {
            int hint;
            decl.intValue(&hint);
            styleHints[decl.property] = hint;
        } else if (decl.property.compare(QLatin1String("image-region"), Qt::CaseInsensitive) == 0) {
            imageRect = decl.rectValue();
        } else if (decl.propertyId == UnknownProperty) {
            qWarning() <<  "Unknown property " << decl.property;
        }
    }

    fixupBorder();
}

QRect QRenderRule::positionRect(const QRect &originRect, Qt::LayoutDirection dir, 
                                const QSize &defSize, Qt::Alignment defPosition) const
{
    Qt::Alignment position = (p && p->position != 0) ? p->position : defPosition;
    QSize sz = hasContentsSize() ? boxSize(contentsSize()) : defSize;

    QRect r = QStyle::alignedRect(dir, position, sz, originRect);
    if (p)
        r.translate(p->left, p->top);
    return r;
}

QRect QRenderRule::positionRect(const QRenderRule &rule, Qt::LayoutDirection dir, 
                                const QRect &rect, const QSize &defSize,
                                Origin defOrigin, Qt::Alignment defPosition) const
{
    Origin origin = (p && p->origin != Origin_Unknown) ? p->origin : defOrigin;
    return positionRect(rule.originRect(rect, origin), dir, defSize, defPosition);
}

QRect QRenderRule::borderRect(const QRect& r) const
{
    if (!hasBox())
        return r;
    const int* m = box()->margins;
    return r.adjusted(m[LeftEdge], m[TopEdge], -m[RightEdge], -m[BottomEdge]);
}

QRect QRenderRule::paddingRect(const QRect& r) const
{
    QRect br = borderRect(r);
    if (!hasBorder())
        return br;
    const int *b = border()->borders;
    return br.adjusted(b[LeftEdge], b[TopEdge], -b[RightEdge], -b[BottomEdge]);
}

QRect QRenderRule::contentsRect(const QRect& r) const
{
    QRect pr = paddingRect(r);
    if (!hasBox())
        return pr;
    const int *p = box()->paddings;
    return pr.adjusted(p[LeftEdge], p[TopEdge], -p[RightEdge], -p[BottomEdge]);
}

QRect QRenderRule::boxRect(const QRect& cr) const
{
    QRect r = cr;
    if (hasBox()) {
        const int *m = box()->margins;
        const int *p = box()->paddings;
        r.adjust(-p[LeftEdge] - m[LeftEdge], -p[TopEdge] - m[TopEdge],
                 p[RightEdge] + m[RightEdge], p[BottomEdge] + m[BottomEdge]);
    }
    if (hasBorder()) {
        const int *b = border()->borders;
        r.adjust(-b[LeftEdge], -b[TopEdge], b[RightEdge], b[BottomEdge]);
    }
    return r;
}

QSize QRenderRule::boxSize(const QSize &s) const
{
    return boxRect(QRect(QPoint(0, 0), s)).size();
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
    QStyleSheetBorderImageData *bi = bd->bi;
    if (bi->cuts[0] == -1) {
        for (int i = 0; i < 4; i++) // assume, cut = border
            bi->cuts[i] = int(border()->borders[i]);
    }
    bi->cutBorderImage();
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
bool QRenderRule::paintsOver(Edge e1, Edge e2)
{
    const BorderStyle& s1 = border()->styles[e1];
    const BorderStyle& s2 = border()->styles[e2];

    if (s2 == BorderStyle_None)
        return true;

    if (s1 == BorderStyle_Solid && s2 == BorderStyle_Solid 
        && border()->colors[e1] == border()->colors[e2])
        return true;

    return false;
}

static void qDrawCenterTiledPixmap(QPainter *p, const QRectF& r, const QPixmap& pix)
{
    p->drawTiledPixmap(r, pix, QPoint(pix.width() - int(r.width())%pix.width(),
                                      pix.height() - int(r.height())%pix.height()));
}

// Note: Round is not supported
void QRenderRule::drawBorderImage(QPainter *p, const QRect& rect, Qt::LayoutDirection)
{
    const QRectF br(rect);
    const int *borders = border()->borders;
    const int &l = borders[LeftEdge], &r = borders[RightEdge],
              &t = borders[TopEdge],  &b = borders[BottomEdge];
    QRectF pr = br.adjusted(l, t, -r, -b);

    const QStyleSheetBorderImageData *bi = border()->borderImage();
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

QRect QRenderRule::originRect(const QRect &rect, Origin origin) const
{
    switch (origin) {
    case Origin_Padding:
        return paddingRect(rect);
    case Origin_Border:
        return borderRect(rect);
    case Origin_Content:
        return contentsRect(rect);
    case Origin_Margin:
    default:
        return rect;
    }
}

void QRenderRule::drawBackgroundImage(QPainter *p, const QRect &rect, Qt::LayoutDirection dir)
{
    Q_ASSERT(hasBackground());
    const QPixmap& bgp = background()->pixmap;
    if (bgp.isNull())
        return;
    QRect r = originRect(rect, background()->origin);
    QRect aligned = QStyle::alignedRect(dir, background()->position, bgp.size(), r);
    QRect inter = aligned.intersected(r);

    switch (background()->repeat) {
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

void QRenderRule::drawBorder(QPainter *p, const QRect& rect, Qt::LayoutDirection dir)
{
    Q_ASSERT(hasBorder());
    if (border()->hasBorderImage()) {
        drawBorderImage(p, rect, dir);
        return;
    }

    const BorderStyle *styles = border()->styles;
    const int *borders = border()->borders;
    const QColor *colors = border()->colors;
    const QRectF br(rect);

    QSize tlr(0, 0), trr(0, 0), brr(0, 0), blr(0, 0);
    if (border()->radii[0].isValid()) {
        tlr = border()->radii[0];
        trr = border()->radii[1];
        blr = border()->radii[2];
        brr = border()->radii[3];
        if (tlr.width() + trr.width() > br.width()
            || blr.width() + brr.width() > br.width())
            tlr = blr = QSize(0, 0);
        if (tlr.height() + trr.height() > br.height()
            || blr.height() + brr.height() > br.height())
            trr = brr = QSize(0, 0);
    }

    // Drawn in increasing order of precendence
    if (styles[BottomEdge] != BorderStyle_None) {
        qreal dw1 = (blr.width() || paintsOver(BottomEdge, LeftEdge)) ? 0 : borders[LeftEdge];
        qreal dw2 = (brr.width() || paintsOver(BottomEdge, RightEdge)) ? 0 : borders[RightEdge];
        qreal x1 = br.x() + blr.width();
        qreal y1 = br.y() + br.height() - borders[BottomEdge];
        qreal x2 = br.x() + br.width() - brr.width();
        qreal y2 = br.y() + br.height() ;

        qDrawEdge(p, x1, y1, x2, y2, dw1, dw2, BottomEdge, styles[BottomEdge], colors[BottomEdge]);
        if (blr.width() || brr.width())
            qDrawRoundedCorners(p, x1, y1, x2, y2, blr, brr, BottomEdge, styles[BottomEdge], colors[BottomEdge]);
    }
    if (styles[RightEdge] != BorderStyle_None) {
        qreal dw1 = (trr.height() || paintsOver(RightEdge, TopEdge)) ? 0 : borders[TopEdge];
        qreal dw2 = (brr.height() || paintsOver(RightEdge, BottomEdge)) ? 0 : borders[BottomEdge];
        qreal x1 = br.x() + br.width() - borders[RightEdge];
        qreal y1 = br.y() + trr.height();
        qreal x2 = br.x() + br.width();
        qreal y2 = br.y() + br.height() - brr.height();

        qDrawEdge(p, x1, y1, x2, y2, dw1, dw2, RightEdge, styles[RightEdge], colors[RightEdge]);
        if (trr.height() || brr.height())
            qDrawRoundedCorners(p, x1, y1, x2, y2, trr, brr, RightEdge, styles[RightEdge], colors[RightEdge]);
    }
    if (styles[LeftEdge] != BorderStyle_None) {
        qreal dw1 = (tlr.height() || paintsOver(LeftEdge, TopEdge)) ? 0 : borders[TopEdge];
        qreal dw2 = (blr.height() || paintsOver(LeftEdge, BottomEdge)) ? 0 : borders[BottomEdge];
        qreal x1 = br.x();
        qreal y1 = br.y() + tlr.height();
        qreal x2 = br.x() + borders[LeftEdge];
        qreal y2 = br.y() + br.height() - blr.height();

        qDrawEdge(p, x1, y1, x2, y2, dw1, dw2, LeftEdge, styles[LeftEdge], colors[LeftEdge]);
        if (tlr.height() || blr.height())
            qDrawRoundedCorners(p, x1, y1, x2, y2, tlr, blr, LeftEdge, styles[LeftEdge], colors[LeftEdge]);
    }
    if (styles[TopEdge] != BorderStyle_None) {
        qreal dw1 = (tlr.width() || paintsOver(TopEdge, LeftEdge)) ? 0 : borders[LeftEdge];
        qreal dw2 = (trr.width() || paintsOver(TopEdge, RightEdge)) ? 0 : borders[RightEdge];
        qreal x1 = br.x() + tlr.width();
        qreal y1 = br.y();
        qreal x2 = br.left() + br.width() - trr.width();
        qreal y2 = br.y() + borders[TopEdge];

        qDrawEdge(p, x1, y1, x2, y2, dw1, dw2, TopEdge, styles[TopEdge], colors[TopEdge]);
        if (tlr.width() || trr.width())
            qDrawRoundedCorners(p, x1, y1, x2, y2, tlr, trr, TopEdge, styles[TopEdge], colors[TopEdge]);
    }
}

void QRenderRule::drawBackground(QPainter *p, const QRect& rect, Qt::LayoutDirection dir)
{
    if (!hasBackground())
        return;
    if (background()->color.isValid())
        p->fillRect(borderRect(rect), background()->color);
    drawBackgroundImage(p, rect, dir);
}

void QRenderRule::drawFrame(QPainter *p, const QRect& rect, Qt::LayoutDirection dir)
{
    drawBackground(p, rect, dir);
    if (hasBorder())
        drawBorder(p, borderRect(rect), dir);
}

void QRenderRule::drawImage(QPainter *p, const QRect &rect)
{
    if (!hasImage())
        return;
    if (geo && QSize(geo->width, geo->height).isValid()) {
        // align center if we had a width, height set
        QRect aligned = QStyle::alignedRect(Qt::LeftToRight, QFlag(Qt::AlignCenter), imageRect.size(), rect);
        QRect inter = aligned.intersected(rect);
        p->drawPixmap(inter.x(), inter.y(), image, imageRect.x() + inter.x() - aligned.x(), 
                      imageRect.y() + inter.y() - aligned.y(), inter.width(), inter.height());
    } else {
        p->drawPixmap(rect, image, imageRect);
    }
}

void QRenderRule::drawRule(QPainter *p, const QRect& rect, Qt::LayoutDirection dir)
{
    drawFrame(p, rect, dir);
    drawImage(p, contentsRect(rect));
}

void QRenderRule::configurePalette(QPalette *p, QPalette::ColorRole fr, QPalette::ColorRole br)
{
    if (bg && bg->color.isValid()) {
        p->setColor(br, bg->color);
        p->setBrush(QPalette::Window, bg->color);
    }

    if (!hasPalette())
        return;
    if (pal->foreground.isValid()) {
        p->setBrush(fr, pal->foreground);
        p->setBrush(QPalette::WindowText, pal->foreground);
        p->setBrush(QPalette::Text, pal->foreground);
    }
    if (pal->selectionBackground.style() != Qt::NoBrush)
        p->setBrush(QPalette::Highlight, pal->selectionBackground);
    if (pal->selectionForeground.isValid())
        p->setBrush(QPalette::HighlightedText, pal->selectionForeground);
    if (pal->alternateBackground.style() != Qt::NoBrush)
        p->setBrush(QPalette::AlternateBase, pal->alternateBackground);
}

///////////////////////////////////////////////////////////////////////////////
#define WIDGET(x) (static_cast<QWidget *>(x.ptr))

class QStyleSheetStyleSelector : public StyleSelector
{
public:
    QStyleSheetStyleSelector() { }

    bool nodeNameEquals(NodePtr node, const QString& name) const
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
QStyle *QStyleSheetStyle::baseStyle() const
{ 
    if (base)
        return base;
    if (QStyleSheetStyle *me = qobject_cast<QStyleSheetStyle *>(qApp->style()))
        return me->base;
    return qApp->style();
}

QRenderRule QStyleSheetStyle::renderRule(const QWidget *w, const QString &part,
                                         QStyle::State state) const
{
    Q_ASSERT(w);
    Q_ASSERT(styleRulesCache.contains(w)); // style sheet rules must have been computed!
    const QVector<StyleRule> &styleRules = styleRulesCache[w];
    QHash<int, QRenderRule> &renderRules = renderRulesCache[w][part];

    int pseudoState;
    if (state & QStyle::State_Enabled)
        pseudoState = PseudoState_Enabled;
    else if (state != QStyle::State_None)
        pseudoState = PseudoState_Disabled;

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

    if (renderRules.contains(pseudoState))
        return renderRules[pseudoState]; // already computed before

    QVector<Declaration> declarations;
    for (int i = 0; i < styleRules.count(); i++) {
        const Selector& selector = styleRules.at(i).selectors.at(0);
        // Rules with pseudo elements dont cascade.This is an intentional
        // diversion for CSS
        if (part.compare(selector.pseudoElement(), Qt::CaseInsensitive) != 0)
            continue;
        const int cssState = selector.pseudoState();
        if ((cssState == PseudoState_Unspecified) || ((cssState & pseudoState) == cssState))
            declarations += styleRules.at(i).declarations;
    }

    QRenderRule newRule(declarations);
    renderRules[pseudoState] = newRule;
    return newRule;
}

enum PseudoElement {
    PseudoElement_None,
    ArrowDown,
    Indicator,
    MenuIndicator,
};

static struct PseudoElementInfo {
    PseudoElement pseudoElement;
    QStyle::SubControl subControl;
    const char *name;
} knownPseudoElements[] = {
    { PseudoElement_None, QStyle::SC_None, "", },
    { ArrowDown, QStyle::SC_None, "down-arrow" },
    { Indicator, QStyle::SC_None, "indicator" },
    { MenuIndicator, QStyle::SC_None, "menu-indicator" }
};

QRenderRule QStyleSheetStyle::renderRule(const QWidget *w, int pseudoElement, 
                                         QStyle::State state) const
{
    return renderRule(w, knownPseudoElements[pseudoElement].name, state);
}

bool QStyleSheetStyle::hasStyleRule(const QWidget *w) const
{
    return w && !styleRulesCache.value(w).isEmpty();
}

bool QStyleSheetStyle::hasStyleRule(const QWidget *w, int part) const
{
    QRenderRule rule = renderRule(w, knownPseudoElements[part].name);
    return !rule.isEmpty();
}

QRenderRule QStyleSheetStyle::renderRule(const QWidget *w, const QStyleOption *opt, 
                                         int pseudoElement) const
{
    Q_ASSERT(hasStyleRule(w));
    QStyle::State state = opt ? opt->state : QStyle::State(QStyle::State_None);

    // Add hacks for <w, opt> here
#ifndef QT_NO_LINEEDIT
    if (qobject_cast<const QLineEdit *>(w))
        state &= ~QStyle::State_Sunken;
#endif

    QStyle::SubControl subControl = knownPseudoElements[pseudoElement].subControl;
    if (subControl != QStyle::SC_None) {
        // if opt is a complex control then adjust state
        if (const QStyleOptionComplex *complex = qstyleoption_cast<const QStyleOptionComplex *>(opt)) {
            if (!(complex->activeSubControls & subControl))
                state = QStyle::State(opt->state & QStyle::State_Enabled);
        }
    }

    return renderRule(w, knownPseudoElements[pseudoElement].name, state);
}

// remember to revert changes in unsetPalette
void QStyleSheetStyle::setPalette(QWidget *w)
{
    const QRenderRule &hoverRule = renderRule(w, PseudoElement_None, QStyle::State_MouseOver);
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
        QRenderRule rule = renderRule(w, PseudoElement_None, map[i].state);
        if (i == 0)
            w->setFont(rule.font);

        p.setCurrentColorGroup(map[i].group);

#ifndef QT_NO_COMBOBOX
        if (QComboBox *cmb = qobject_cast<QComboBox *>(w)) {
            if (!cmb->isEditable())
                rule.configurePalette(&p, QPalette::ButtonText, QPalette::Button);
            else
                rule.configurePalette(&p, QPalette::Text, QPalette::Base);
        } else
#endif
        if (QAbstractScrollArea *sa = qobject_cast<QAbstractScrollArea *>(w)) {
            rule.configurePalette(&p, sa->viewport()->foregroundRole(),
                                  sa->viewport()->backgroundRole());
        } else {
            rule.configurePalette(&p, w->foregroundRole(), w->backgroundRole());
        }
    }

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

/*!
    \internal
*/
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
        QEvent e(QEvent::StyleChange);
        QApplication::sendEvent(widget, &e);
        widget->update();
        widget->updateGeometry();
    }
}

void QStyleSheetStyle::widgetDestroyed(QObject *o)
{
    renderRulesCache.remove((const QWidget *)o);
    renderRulesCache.remove((const QWidget *)o);
}

bool unstylable(QWidget *w)
{
    if (QAbstractScrollArea *sa = qobject_cast<QAbstractScrollArea *>(w->parentWidget())) {
        if (sa->viewport() == w)
            return true;
    } 
#ifndef QT_NO_LINEEDIT
    else if (qobject_cast<QLineEdit *>(w)) {
        if (0
#ifndef QT_NO_COMBOBOX
            || qobject_cast<QComboBox *>(w->parentWidget())
#endif
#ifndef QT_NO_SPINBOX            
            || qobject_cast<QAbstractSpinBox *>(w->parentWidget())
#endif
            )
            return true;
    }
#endif
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
    QVector<QCss::StyleRule> rules = styleRules(w);
    styleRulesCache[w] = rules;
    if (rules.isEmpty()) {
        unsetPalette(w);
    } else {
        setPalette(w);
    }
}

QVector<QCss::StyleRule> QStyleSheetStyle::styleRules(QWidget *w)
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

/////////////////////////////////////////////////////////////////////////////////////////
QStyleSheetStyle::QStyleSheetStyle(QStyle *base)
: base(base), refcount(1)
{
}

void QStyleSheetStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                 const QWidget *w) const
{
    if (!hasStyleRule(w)) {
        baseStyle()->drawComplexControl(cc, opt, p, w);
        return;
    }

    QRenderRule rule = renderRule(w, opt);

    switch (cc) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            if (rule.hasBorder() || hasStyleRule(w, ArrowDown)) {
                rule.drawFrame(p, opt->rect, opt->direction);
                // ###
            } else {
                QStyleOptionComboBox cmbOpt(*cb);
                rule.configurePalette(&cmbOpt.palette, QPalette::Text, QPalette::Base);
                baseStyle()->drawComplexControl(cc, &cmbOpt, p, w);
            }
            return;
        }
        break;

    default:
        break;
    }

    baseStyle()->drawComplexControl(cc, opt, p, w);
}

void QStyleSheetStyle::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                          const QWidget *w) const
{
    if (!hasStyleRule(w)) {
        baseStyle()->drawControl(ce, opt, p, w);
        return;
    }

    QRenderRule rule = renderRule(w, opt);

    switch (ce) {
    // Push button
    case CE_PushButton:
        ParentStyle::drawControl(ce, opt, p, w);
        return;

    case CE_PushButtonBevel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QStyleOptionButton btnOpt(*btn);
            btnOpt.rect = rule.borderRect(opt->rect);
            if (rule.hasBox() || rule.hasDrawable()
                || ((btn->features & QStyleOptionButton::HasMenu) && hasStyleRule(w, MenuIndicator))) {
                if (rule.hasBox() || rule.hasDrawable()) {
                    rule.drawFrame(p, opt->rect, opt->direction);
                } else {
                    btnOpt.features &= ~QStyleOptionButton::HasMenu;
                    baseStyle()->drawControl(ce, &btnOpt, p, w);
                }

                if (btn->features & QStyleOptionButton::HasMenu) {
                    QRenderRule subRule = renderRule(w, opt, MenuIndicator);
                    QRect ir = btnOpt.rect;
                    int mbi = pixelMetric(PM_MenuButtonIndicator, btn, w);
                    ir = subRule.positionRect(rule, opt->direction, opt->rect, QSize(mbi, mbi),
                                              Origin_Content, Qt::AlignBottom | Qt::AlignRight);
                    if (subRule.hasImage()) {
                        subRule.drawImage(p, ir);
                    } else {
                        btnOpt.rect = ir;
                        baseStyle()->drawPrimitive(PE_IndicatorArrowDown, &btnOpt, p, w);
                    }
                }
            } else {
                baseStyle()->drawControl(ce, &btnOpt, p, w);
            }
        }
        return;

    case CE_PushButtonLabel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QStyleOptionButton butOpt(*btn);
            rule.configurePalette(&butOpt.palette, QPalette::ButtonText, QPalette::Button);
            if (rule.hasBox()) // text can be shifted with padding
                ParentStyle::drawControl(ce, &butOpt, p, w);
            else
                baseStyle()->drawControl(ce, &butOpt, p, w);
        }
        return;

    case CE_RadioButton:
    case CE_CheckBox:
        rule.drawFrame(p, opt->rect, opt->direction);
        ParentStyle::drawControl(ce, opt, p, w);
        return;

    case CE_RadioButtonLabel:
    case CE_CheckBoxLabel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QStyleOptionButton butOpt(*btn);
            rule.configurePalette(&butOpt.palette, QPalette::ButtonText, QPalette::Button);
            ParentStyle::drawControl(ce, &butOpt, p, w);
        }
        return;

    case CE_Splitter:
        if (rule.hasDrawable()) {
            rule.drawRule(p, opt->rect, opt->direction);
            return;
        }
        break;

    case CE_ToolBar:
        if (rule.hasBackground()) {
            rule.drawBackground(p, opt->rect, opt->direction);
        }
        if (rule.hasBorder()) {
            rule.drawBorder(p, rule.borderRect(opt->rect), opt->direction);
        } else {
            if (const QStyleOptionToolBar *tb = qstyleoption_cast<const QStyleOptionToolBar *>(opt)) {
                QStyleOptionToolBar newTb(*tb);
                newTb.rect = rule.borderRect(opt->rect);
                baseStyle()->drawControl(ce, &newTb, p, w);
            }
        }
        return;

    case CE_MenuEmptyArea:
    case CE_MenuBarEmptyArea:
        if (const QStyleOptionMenuItem *m = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            if (rule.hasBackground()) {
                rule.drawBackground(p, opt->rect, opt->direction);
            } else {
                QStyleOptionMenuItem mi(*m);
                rule.configurePalette(&mi.palette, QPalette::ButtonText, QPalette::Button);
                baseStyle()->drawControl(ce, &mi, p, w);
            }
        }
        return;

    case CE_MenuBarItem:
    case CE_MenuItem:
        if (const QStyleOptionMenuItem *m = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            QStyleOptionMenuItem mi(*m);
            rule.configurePalette(&mi.palette, QPalette::ButtonText, QPalette::Button);
            if (rule.hasBackground()) {
                rule.drawBackgroundImage(p, opt->rect, opt->direction);
                mi.palette.setBrush(QPalette::Button, rule.background()->pixmap); // FIXME: Set origin
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
            if (rule.hasPalette() && rule.palette()->foreground.isValid())
                p->setPen(rule.palette()->foreground);
            ParentStyle::drawControl(ce, opt, p, w);
            p->setPen(savedPen);
        }
        return;

    case CE_SizeGrip:
        if (!rule.hasImage())
            break;
        if (const QStyleOptionSizeGrip *sgOpt = qstyleoption_cast<const QStyleOptionSizeGrip *>(opt)) {
            p->save();
            switch (sgOpt->corner) {
            case Qt::BottomRightCorner: break;
            case Qt::BottomLeftCorner: p->rotate(90); break;
            case Qt::TopLeftCorner: p->rotate(180); break;
            case Qt::TopRightCorner: p->rotate(270); break;
            default: break;
            }
            rule.drawImage(p, opt->rect);
            p->restore();
            return;
        }
        break;

    case CE_ProgressBar:
        ParentStyle::drawControl(ce, opt, p, w);
        break;

    case CE_ProgressBarGroove:
        if (rule.hasBorder()) {
            rule.drawFrame(p, opt->rect, opt->direction);
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
        //if (!rule.hasIcon("chunk"))
        //    break;
        break;

    default:
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
    if (!hasStyleRule(w)) {
        baseStyle()->drawPrimitive(pe, opt, p, w);
        return;
    }

    int pseudoElement = PseudoElement_None;

    QRenderRule rule = renderRule(w, opt);

    switch (pe) {
    case PE_IndicatorArrowDown:
        pseudoElement = ArrowDown;
        break;

    case PE_IndicatorRadioButton:
    case PE_IndicatorCheckBox:
        pseudoElement = Indicator;
        break;

    case PE_PanelButtonCommand:
        if (!rule.hasBorder()) {
            baseStyle()->drawPrimitive(pe, opt, p, w);
        } else {
            rule.drawBorder(p, opt->rect, opt->direction);
        }
        return;

    // how do we specify these in the css?
    case PE_FrameDefaultButton:
        return;

    case PE_Frame:
    case PE_PanelLineEdit:
        if (const QStyleOptionFrame *frm = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            if (!rule.hasBorder()) {
                // Only background brush is supported on a LineEdit
                if (pe == PE_Frame)
                    rule.drawBackground(p, opt->rect, opt->direction);
                QStyleOptionFrame frmOpt(*frm);
                rule.configurePalette(&frmOpt.palette, QPalette::Text, QPalette::Base);
                frmOpt.rect = rule.borderRect(frmOpt.rect); // apply margin
                baseStyle()->drawPrimitive(pe, &frmOpt, p, w);
            } else {
                rule.drawFrame(p, opt->rect, opt->direction);
            }
        }
        return;

    case PE_Widget:
        rule.drawBackground(p, opt->rect, opt->direction);
        return;

    case PE_FrameMenu:
    case PE_PanelMenuBar:
        if (rule.hasBorder()) {
            rule.drawBorder(p, rule.borderRect(opt->rect), opt->direction);
            return;
        }
        break;

    case PE_IndicatorToolBarHandle:
        if (rule.hasDrawable()) {
            rule.drawRule(p, opt->rect, opt->direction);
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
        if (rule.hasDrawable()) {
            rule.drawFrame(p, opt->rect, opt->direction);
            return;
        }
        break;

    case PE_FrameGroupBox:
        if (rule.hasBox()) {
            rule.drawFrame(p, opt->rect, opt->direction);
        } else
            baseStyle()->drawPrimitive(pe, opt, p, w);
        return;

    default:
        break;
    }

    if (pseudoElement != PseudoElement_None) {
        QRenderRule subRule = renderRule(w, opt, pseudoElement);
        if (!subRule.isEmpty()) {
            subRule.drawImage(p, opt->rect);
        } else {
            baseStyle()->drawPrimitive(pe, opt, p, w);
        }
    } else {
        baseStyle()->drawPrimitive(pe, opt, p, w);
    }
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
    if (!hasStyleRule(w))
        return baseStyle()->pixelMetric(m, opt, w);

    QRenderRule rule = renderRule(w, opt);
    QRenderRule subRule;

    switch (m) {
    case PM_MenuButtonIndicator:
        subRule = renderRule(w, opt, MenuIndicator);
        if (subRule.hasContentsSize())
            return subRule.contentsSize().width();
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
    case PM_ExclusiveIndicatorHeight:
    case PM_IndicatorHeight:
        subRule = renderRule(w, Indicator);
        if (subRule.hasContentsSize()) {
            return (m == PM_ExclusiveIndicatorWidth) || (m == PM_IndicatorWidth)
                        ? subRule.contentsSize().width() 
                        : subRule.contentsSize().height();
        }
        break;

    case PM_ToolTipLabelFrameWidth: // border + margin (support only one width)
    case PM_MenuPanelWidth:
    case PM_MenuBarPanelWidth: 
    case PM_ToolBarFrameWidth:
        if (rule.hasBorder() || rule.hasBox())
            return (rule.border() ? rule.border()->borders[LeftEdge] : 0)
                   + (rule.hasBox() ? rule.box()->margins[LeftEdge] + rule.box()->paddings[LeftEdge]: 0);
        break;

    case PM_MenuHMargin:
    case PM_MenuBarHMargin:
        if (rule.hasBox())
            return rule.box()->paddings[LeftEdge];
        break;

    case PM_ToolBarItemMargin:
    case PM_MenuVMargin:
    case PM_MenuBarVMargin:
        if (rule.hasBox())
            return rule.box()->paddings[TopEdge];
        break;

    case PM_ToolBarItemSpacing:
    case PM_MenuBarItemSpacing:
        if (rule.hasBox() && rule.box()->spacing != -1)
            return rule.box()->spacing;
        break;

    case PM_ProgressBarChunkWidth:
        //if (rules.hasIconSize("chunk"))
        //    return rules.iconSize("chunk").width();
        break;

    case PM_SmallIconSize:
    case PM_MenuDesktopFrameWidth:
    case PM_MenuTearoffHeight:
    case PM_MenuScrollerHeight: 
        break;
    case PM_ToolBarExtensionExtent:
        break;

    case PM_ToolBarHandleExtent:
        if (rule.hasBox() || rule.hasBorder() || rule.hasContentsSize())
            return rule.boxSize(rule.contentsSize()).expandedTo(rule.minimumContentsSize()).width();

    case PM_SplitterWidth:
        if (rule.hasContentsSize())
            return rule.contentsSize().width();
        break;

    case PM_SizeGripSize: // not used by QSizeGrip!
        break;

        // group box
    case PM_CheckBoxLabelSpacing:
        break;

    default:
        break;
    }

    return baseStyle()->pixelMetric(m, opt, w);
}

QSize QStyleSheetStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt,
                                         const QSize &csz, const QWidget *w) const
{
    if (!hasStyleRule(w))
        return baseStyle()->sizeFromContents(ct, opt, csz, w);

    QRenderRule rule = renderRule(w, opt);
    QSize sz = csz.expandedTo(rule.minimumContentsSize());

    switch (ct) {
    case CT_PushButton:
        if (rule.hasBorder() || rule.hasBox())
            return rule.boxSize(sz);
        break;

    case CT_LineEdit: // does not contains fw
        if (rule.hasBorder()) {
            return rule.boxSize(sz);
        } else {
            // What follows is a really ugly hack to support padding with native frames
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
            if (rule.hasBox() || rule.hasBorder() || hasStyleRule(w, Indicator)) {
                bool isRadio = (ct == CT_RadioButton);
                int iw = pixelMetric(isRadio ? PM_ExclusiveIndicatorWidth
                                             : PM_IndicatorWidth, btn, w);
                int ih = pixelMetric(isRadio ? PM_ExclusiveIndicatorHeight
                                             : PM_IndicatorHeight, btn, w);

                QSize margins(0, 0);
                if (!rule.hasBox() && !rule.hasBorder())
                    margins = QSize((!btn->icon.isNull() && btn->text.isEmpty()) ? 0 : 10, 4);
                int spacing = rule.hasBox() ? rule.box()->spacing : -1;
                if (spacing == -1)
                    spacing = 6;
                sz = sz + QSize(iw + spacing, 0) + margins;
                sz.setHeight(qMax(sz.height(), ih));
                return rule.boxSize(sz);
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
            return rule.boxSize(sz);;
        break;

    case CT_SizeGrip:
        if (rule.hasContentsSize())
            return rule.contentsSize();
        break;

    case CT_Splitter:
    case CT_ProgressBar:
        if (rule.hasBorder() || rule.hasBox())
            return rule.boxSize(sz);
        break;

    default:
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
    if (!hasStyleRule(w))
        return baseStyle()->styleHint(sh, opt, w, shret);

    QRenderRule rule = renderRule(w, opt);
    QString s;
    switch (sh) {
        case SH_LineEdit_PasswordCharacter: s = QLatin1String("lineedit-password-character"); break;
        case SH_EtchDisabledText: s = "etch-disabled-text"; break;
        case SH_ItemView_ActivateItemOnSingleClick: s = "activate-on-singleclick"; break;
        case SH_ItemView_ShowDecorationSelected: s = "show-decoration-selected"; break;
        case SH_Table_GridLineColor: s = "gridline-color"; break;
        case SH_DialogButtonLayout: s = "button-layout"; break;
        case SH_ToolTipLabel_Opacity: s = "opacity"; break;
        default: break;
    }
    if (!s.isEmpty() && rule.hasStyleHint(s))
        return rule.styleHint(s);

    return baseStyle()->styleHint(sh, opt, w, shret);
}

QRect QStyleSheetStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                              const QWidget *w) const
{
    if (!hasStyleRule(w))
        return baseStyle()->subControlRect(cc, opt, sc, w);

    QRenderRule rule = renderRule(w, opt);
    QRect r = opt->rect;
    switch (cc) {
    case CC_ComboBox:
        if (rule.hasBorder()) {
            int rightBorder = rule.border()->borders[RightEdge];
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

    default:
            break;
    }

    return baseStyle()->subControlRect(cc, opt, sc, w);;
}

QRect QStyleSheetStyle::subElementRect(SubElement se, const QStyleOption *opt, const QWidget *w) const
{
    if (!hasStyleRule(w))
        return baseStyle()->subElementRect(se, opt, w);

    QRenderRule rule = renderRule(w, opt);
    QRenderRule subRule;

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
            // ugly hack to support native frames with padding
            QRect baseRect = baseStyle()->subElementRect(se, opt, w);
            if (!rule.hasBox())
                return baseRect;
            QRect parentRect = ParentStyle::subElementRect(se, opt, w);
            if (baseRect != parentRect)
                return baseRect;
            return rule.contentsRect(baseRect);
        }
        break;

    case SE_CheckBoxIndicator:
    case SE_RadioButtonIndicator:
        if (rule.hasBox() || rule.hasBorder() || hasStyleRule(w, Indicator)) {
            QRenderRule subRule = renderRule(w, opt, Indicator);
            const bool cb = se == SE_CheckBoxIndicator;
            int wid = pixelMetric(cb ? PM_ExclusiveIndicatorWidth : PM_IndicatorWidth, opt, w);
            int hei = pixelMetric(cb ? PM_ExclusiveIndicatorHeight : PM_IndicatorHeight, opt, w);
            return subRule.positionRect(rule, opt->direction, opt->rect, QSize(wid, hei),
                                        Origin_Content, Qt::AlignLeft | Qt::AlignVCenter);
        }
        break;

    case SE_CheckBoxContents:
    case SE_RadioButtonContents:
        if (rule.hasBox() || rule.hasBorder() || hasStyleRule(w, Indicator)) {
            bool isRadio = se == SE_RadioButtonContents;
            QRect ir = subElementRect(isRadio ? SE_RadioButtonIndicator : SE_CheckBoxIndicator,
                                      opt, w);
            ir = visualRect(opt->direction, opt->rect, ir);
            int spacing = rule.hasBox() ? rule.box()->spacing : -1;
            if (spacing == -1)
                spacing = 6;
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
