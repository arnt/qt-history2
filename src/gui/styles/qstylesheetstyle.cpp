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
#include "private/qabstractscrollarea_p.h"
#include <qtooltip.h>
#include <qshareddata.h>
#include <qradiobutton.h>
#include <qtoolbutton.h>
#include <qscrollbar.h>
#include <qstring.h>
#include <qfile.h>
#include <qcheckbox.h>
#include <qstatusbar.h>

#include <limits.h>

using namespace QCss;

static QHash<const QWidget *, QVector<StyleRule> > *styleRulesCache = 0;
typedef QHash<QString, QHash<int, QRenderRule> > QRenderRules;
static QHash<const QWidget *, QRenderRules> *renderRulesCache = 0;
static QHash<const QWidget *, int> *customPaletteWidgets = 0; // widgets whose palette we tampered
static QHash<const QWidget *, int> *customFontWidgets = 0; // widgets whose font we tampered
static QHash<void *, StyleSheet> *styleSheetCache = 0; // parsed style sheets
static QSet<const QWidget *> *autoFillDisabledWidgets = 0;

#define ceil(x) ((int)(x) + ((x) > 0 && (x) != (int)(x)))

struct QStyleSheetBorderImageData : public QSharedData
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

struct QStyleSheetBackgroundData : public QSharedData
{
    QStyleSheetBackgroundData(const QBrush& b, const QPixmap& p, QCss::Repeat r,
                              Qt::Alignment a, QCss::Origin o, Attachment t)
        : brush(b), pixmap(p), repeat(r), position(a), origin(o), attachment(t) { }

    bool isTransparent() const {
        if (brush.style() != Qt::NoBrush)
            return !brush.isOpaque();
        return pixmap.isNull() ? false : pixmap.hasAlpha();
    }
    QBrush brush;
    QPixmap pixmap;
    QCss::Repeat repeat;
    Qt::Alignment position;
    QCss::Origin origin;
    QCss::Attachment attachment;
};

struct QStyleSheetBorderData : public QSharedData
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
            radii[i] = r[i].expandedTo(QSize(0, 0));
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

struct QStyleSheetBoxData : public QSharedData
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
};

struct QStyleSheetPaletteData : public QSharedData
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

struct QStyleSheetGeometryData : public QSharedData
{
    QStyleSheetGeometryData(int w, int h, int minw, int minh)
        : minWidth(minw), minHeight(minh), width(w), height(h) { }

    int minWidth, minHeight, width, height;
};

struct QStyleSheetPositionData : public QSharedData
{
    QStyleSheetPositionData(int l, int t, int r, int b, Origin o, Qt::Alignment p, QCss::PositionMode m)
        : left(l), top(t), bottom(b), right(r), origin(o), position(p), mode(m) { }

    int left, top, bottom, right;
    Origin origin;
    Qt::Alignment position;
    QCss::PositionMode mode;
};

class QRenderRule
{
public:
    QRenderRule() : features(0), pal(0), b(0), bg(0), bd(0), geo(0), p(0) { }
    QRenderRule(const QVector<QCss::Declaration> &, const QWidget *);
    ~QRenderRule() { }

    QRect borderRect(const QRect &r) const;
    QRect paddingRect(const QRect &r) const;
    QRect contentsRect(const QRect &r) const;

    enum { Margin = 1, Border = 2, Padding = 4, All=Margin|Border|Padding };
    QRect boxRect(const QRect &r, int flags = All) const;
    QSize boxSize(const QSize &s, int flags = All) const;
    QRect originRect(const QRect &rect, Origin origin) const;

    bool paintsOver(Edge e1, Edge e2);
    void drawBorder(QPainter *, const QRect&);
    void drawBorderImage(QPainter *, const QRect&);
    void drawBackground(QPainter *, const QRect&, const QPoint& = QPoint(0, 0));
    void drawBackgroundImage(QPainter *, const QRect&, QPoint = QPoint(0, 0));
    void drawFrame(QPainter *, const QRect&);
    void drawImage(QPainter *p, const QRect &rect);
    void drawRule(QPainter *, const QRect&);
    void configurePalette(QPalette *, QPalette::ColorGroup, const QWidget *, bool);
    void configurePalette(QPalette *p, QPalette::ColorRole fr, QPalette::ColorRole br);

    void getRadii(const QRect &br, QSize *tlr, QSize *trr, QSize *blr, QSize *brr) const;

    const QStyleSheetPaletteData *palette() const { return pal; }
    const QStyleSheetBoxData *box() const { return b; }
    const QStyleSheetBackgroundData *background() const { return bg; }
    const QStyleSheetBorderData *border() const { return bd; }
    const QStyleSheetGeometryData *geometry() const { return geo; }
    const QStyleSheetPositionData *position() const { return p; }

    bool hasPalette() const { return pal != 0; }
    bool hasBackground() const { return bg != 0; }
    bool hasGradientBackground() const { return bg && bg->brush.style() >= Qt::LinearGradientPattern
                                                   && bg->brush.style() <= Qt::ConicalGradientPattern; }

    bool hasNativeBorder() const { 
        return bd == 0
               || (!bd->hasBorderImage() && bd->styles[0] == BorderStyle_Native); 
    }

    bool nativeBorder() const {
        return (bd == 0
                || (!bd->hasBorderImage() && bd->styles[0] == BorderStyle_Native));
    }

    bool baseStyleCanDraw() const {
        if (!hasBackground() || background()->brush.style() == Qt::NoBrush)
            return true;
        if (hasGradientBackground())
            return features & StyleFeature_BackgroundGradient;
        return features & StyleFeature_BackgroundColor;
    }

    bool hasBox() const { return b != 0; }
    bool hasBorder() const { return bd != 0; }
    bool hasPosition() const { return p != 0; }
    bool hasGeometry() const { return geo != 0; }
    bool hasDrawable() const { return hasBorder() || hasBackground() || hasImage(); }

    QSize minimumContentsSize() const
    { return geo ? QSize(geo->minWidth, geo->minHeight) : QSize(0, 0); }
    QSize minimumSize() const
    { return boxSize(minimumContentsSize()); }

    QSize contentsSize() const
    { return geo ? QSize(geo->width, geo->height) : (imageRect.isValid() ? imageRect.size() : QSize()); }
    bool hasContentsSize() const
    { return (geo && (geo->width != -1 || geo->height != -1)) || imageRect.isValid(); }

    QSize size() const { return boxSize(contentsSize()); }

    bool hasImage() const { return !image.isNull(); }

    int features;
    QBrush defaultBackground;
    QPixmap image;
    QRect imageRect;
    QFont font;

    QHash<QString, int> styleHints;
    bool hasStyleHint(const QString& sh) const { return styleHints.contains(sh); }
    int styleHint(const QString& sh) const { return styleHints.value(sh); }

    void fixupBorder(int);

    QSharedDataPointer<QStyleSheetPaletteData> pal;
    QSharedDataPointer<QStyleSheetBoxData> b;
    QSharedDataPointer<QStyleSheetBackgroundData> bg;
    QSharedDataPointer<QStyleSheetBorderData> bd;
    QSharedDataPointer<QStyleSheetGeometryData> geo;
    QSharedDataPointer<QStyleSheetPositionData> p;
};

///////////////////////////////////////////////////////////////////////////////////////////
static const char *knownStyleHints[] = {
    "activate-on-singleclick",
    "button-layout",
    "combobox-list-mousetracking",
    "combobox-popup",
    "dither-disable-text",
    "etch-disabled-text",
    "gridline-color",
    "lineedit-password-character",
    "menubar-altkey-navigation",
    "menubar-separator",
    "messagebox-text-interaction-flags",
    "mouse-tracking",
    "opacity",
    "scrollbar-contextmenu",
    "scrollbar-leftclick-absolute-position",
    "scrollbar-middleclick-absolute-position",
    "scrollbar-roll-between-buttons",
    "scrollbar-scroll-when-pointer-leaves-control",
    "scrollview-frame-around-contents",
    "show-decoration-selected",
    "spinbox-click-autorepeat-rate",
    "spincontrol-disable-on-bounds",
    "toolbox-selected-page-title-bold",
    "toolbutton-popup-delay"
};

static const int numKnownStyleHints = sizeof(knownStyleHints)/sizeof(knownStyleHints[0]);

QRenderRule::QRenderRule(const QVector<Declaration> &declarations, const QWidget *widget)
: features(0), pal(0), b(0), bg(0), bd(0), geo(0), p(0)
{
    Q_ASSERT(widget);
    QPalette palette = qApp->palette(); // ###: ideally widget's palette
    ValueExtractor v(declarations, palette);
    features = v.extractStyleFeatures();

    int w = -1, h = -1, minw = -1, minh = -1;
    if (v.extractGeometry(&w, &h, &minw, &minh))
        geo = new QStyleSheetGeometryData(w, h, minw, minh);

    int left = 0, top = 0, right = 0, bottom = 0;
    Origin origin = Origin_Unknown;
    Qt::Alignment position = 0;
    QCss::PositionMode mode = PositionMode_Static;
    if (v.extractPosition(&left, &top, &right, &bottom, &origin, &position, &mode))
        p = new QStyleSheetPositionData(left, top, right, bottom, origin, position, mode);

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

    QBrush brush;
    QString uri;
    Repeat repeat = Repeat_XY;
    Qt::Alignment alignment = Qt::AlignTop | Qt::AlignLeft;
    Attachment attachment = Attachment_Scroll;
    origin = Origin_Padding;
    if (v.extractBackground(&brush, &uri, &repeat, &alignment, &origin, &attachment))
        bg = new QStyleSheetBackgroundData(brush, QPixmap(uri), repeat, alignment, origin, attachment);

    QColor sfg, fg;
    QBrush sbg, abg;
    if (v.extractPalette(&fg, &sfg, &sbg, &abg))
        pal = new QStyleSheetPaletteData(fg, sfg, sbg, abg);

    int adj = -255;
    v.extractFont(&font, &adj);

#ifndef QT_NO_TOOLTIP
    if (QString::fromLatin1(widget->metaObject()->className()) == QLatin1String("QTipLabel"))
        palette = QToolTip::palette();
#endif

    for (int i = 0; i < declarations.count(); i++) {
        const Declaration& decl = declarations.at(i);
        if (decl.propertyId == BorderImage) {
            QString uri;
            QCss::TileMode horizStretch, vertStretch;
            int cuts[4];

            decl.borderImageValue(&uri, cuts, &horizStretch, &vertStretch);
            if (uri.isEmpty() || uri == QLatin1String("none")) {
                if (bd && bd->bi)
                    bd->bi->pixmap = QPixmap();
            } else {
                if (!bd)
                    bd = new QStyleSheetBorderData;
                if (!bd->bi)
                    bd->bi = new QStyleSheetBorderImageData;

                QStyleSheetBorderImageData *bi = bd->bi;
                bi->pixmap = QPixmap(uri);
                for (int i = 0; i < 4; i++)
                    bi->cuts[i] = cuts[i];
                bi->horizStretch = horizStretch;
                bi->vertStretch = vertStretch;
            }
        } else if (decl.propertyId == QtImage) {
            image = QPixmap(decl.uriValue()); // ###: support none
            if (!imageRect.isValid())
                imageRect = QRect(0, 0, image.width(), image.height());
        } else if (decl.propertyId == QtBackgroundRole) {
            if (bg && bg->brush.style() != Qt::NoBrush)
                continue;
            int role = decl.values.first().variant.toInt();
            if (role >= Value_FirstColorRole && role <= Value_LastColorRole)
                defaultBackground = palette.color((QPalette::ColorRole)(role-Value_FirstColorRole));
        } else if (decl.property.compare(QLatin1String("image-region"), Qt::CaseInsensitive) == 0) {
            imageRect = decl.rectValue();
        } else if (decl.property.startsWith(QLatin1String("qproperty-"), Qt::CaseInsensitive)) {
        } else if (decl.propertyId == UnknownProperty) {
            bool knownStyleHint = false;
            for (int i = 0; i < numKnownStyleHints; i++) {
                const char *styleHint = knownStyleHints[i];
                if (decl.property.compare(QLatin1String(styleHint)) == 0) {
                   int hint;
                   decl.intValue(&hint);
                   styleHints[decl.property] = hint;
                   knownStyleHint = true;
                   break;
                }
            }
            if (!knownStyleHint)
                qWarning("Unknown property %s", qPrintable(decl.property));
        }
    }

    QStyleSheetStyle *style = qobject_cast<QStyleSheetStyle *>(widget->style());
    Q_ASSERT(style);
    fixupBorder(style->nativeFrameWidth(widget));
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

QRect QRenderRule::boxRect(const QRect& cr, int flags) const
{
    QRect r = cr;
    if (hasBox()) {
        if (flags & Margin) {
            const int *m = box()->margins;
            r.adjust(-m[LeftEdge], -m[TopEdge], m[RightEdge], m[BottomEdge]);
        }
        if (flags & Padding) {
            const int *p = box()->paddings;
            r.adjust(-p[LeftEdge], -p[TopEdge], p[RightEdge], p[BottomEdge]);
        }
    }
    if (hasBorder() && (flags & Border)) {
        const int *b = border()->borders;
        r.adjust(-b[LeftEdge], -b[TopEdge], b[RightEdge], b[BottomEdge]);
    }
    return r;
}

QSize QRenderRule::boxSize(const QSize &cs, int flags) const
{
    QSize bs = boxRect(QRect(QPoint(0, 0), cs), flags).size();
    if (cs.width() < 0) bs.setWidth(-1);
    if (cs.height() < 0) bs.setHeight(-1);
    return bs;
}

void QRenderRule::fixupBorder(int nativeWidth)
{
    if (bd == 0)
        return;

    if (!bd->hasBorderImage() || bd->bi->pixmap.isNull()) {
        delete bd->bi;
        bd->bi = 0;
        // ignore the color, border of edges that have none border-style
        QColor color = pal ? pal->foreground : QColor();
        for (int i = 0; i < 4; i++) {
            switch (bd->styles[i]) {
            case BorderStyle_None:
                // border-style: none forces width to be 0
                bd->colors[i] = QColor();
                bd->borders[i] = 0;
                break;
            case BorderStyle_Native:
                if (bd->borders[i] == 0)
                    bd->borders[i] = nativeWidth;
                // intentional fall through
            default:
                if (!bd->colors[i].isValid()) // auto-acquire 'color'
                    bd->colors[i] = color;
                break;
            }
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
            c = c.lighter();

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
            c = c.lighter();
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
    BorderStyle s1 = bd->styles[e1];
    BorderStyle s2 = bd->styles[e2];

    if (s2 == BorderStyle_None || bd->colors[e2] == Qt::transparent)
        return true;

    if ((s1 == BorderStyle_Solid && s2 == BorderStyle_Solid) && (bd->colors[e1] == bd->colors[e2]))
        return true;

    return false;
}

static void qDrawCenterTiledPixmap(QPainter *p, const QRectF& r, const QPixmap& pix)
{
    p->drawTiledPixmap(r, pix, QPoint(pix.width() - int(r.width())%pix.width(),
                                      pix.height() - int(r.height())%pix.height()));
}

// Note: Round is not supported
void QRenderRule::drawBorderImage(QPainter *p, const QRect& rect)
{
    const QRectF br(rect);
    const int *borders = border()->borders;
    const int &l = borders[LeftEdge], &r = borders[RightEdge],
              &t = borders[TopEdge],  &b = borders[BottomEdge];
    QRectF pr = br.adjusted(l, t, -r, -b);

    QPainter::RenderHints oldHints = p->renderHints();
    p->setRenderHint(QPainter::SmoothPixmapTransform);

    const QStyleSheetBorderImageData *bi = border()->borderImage();
    const QPixmap& pix = bi->pixmap;
    const int *c = bi->cuts;
    QRectF tlc(0, 0, c[LeftEdge], c[TopEdge]);
    if (tlc.isValid())
        p->drawPixmap(QRectF(br.topLeft(), QSizeF(l, t)), pix, tlc);
    QRectF trc(pix.width() - c[RightEdge], 0, c[RightEdge], c[TopEdge]);
    if (trc.isValid())
        p->drawPixmap(QRectF(br.left() + br.width() - r, br.y(), r, t), pix, trc);
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

    p->setRenderHints(oldHints);
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

void QRenderRule::drawBackgroundImage(QPainter *p, const QRect &rect, QPoint off)
{
    if (!hasBackground())
        return;

    const QPixmap& bgp = background()->pixmap;
    if (bgp.isNull())
        return;

    if (background()->attachment == Attachment_Fixed)
        off = QPoint(0, 0);

    QRect r = originRect(rect, background()->origin);
    QRect aligned = QStyle::alignedRect(Qt::LeftToRight, background()->position, bgp.size(), r);
    QRect inter = aligned.intersected(r);

    switch (background()->repeat) {
    case Repeat_Y:
        p->drawTiledPixmap(inter.x(), r.y(), inter.width(), r.height(), bgp,
                           inter.x() - aligned.x() + off.x(),
                           bgp.height() - int(aligned.y() - r.y()) % bgp.height() + off.y());
        break;
    case Repeat_X:
        p->drawTiledPixmap(r.x(), inter.y(), r.width(), inter.height(), bgp,
                           bgp.width() - int(aligned.x() - r.x())%bgp.width() + off.x(),
                           inter.y() - aligned.y() + off.y());
        break;
    case Repeat_XY:
        p->drawTiledPixmap(r, bgp,
                           QPoint(bgp.width() - int(aligned.x() - r.x())% bgp.width() + off.x(),
                                  bgp.height() - int(aligned.y() - r.y())%bgp.height() + off.y()));
        break;
    case Repeat_None:
    default:
        p->drawPixmap(inter.x(), inter.y(), bgp, inter.x() - aligned.x() + off.x(),
                      inter.y() - aligned.y() + off.y(), inter.width(), inter.height());
        break;
    }
}

void QRenderRule::getRadii(const QRect &br, QSize *tlr, QSize *trr, QSize *blr, QSize *brr) const
{
    Q_ASSERT(hasBorder());
    const QSize *radii = border()->radii;
    *tlr = radii[0];
    *trr = radii[1];
    *blr = radii[2];
    *brr = radii[3];
    if (tlr->width() + trr->width() > br.width()
        || blr->width() + brr->width() > br.width())
        *tlr = *blr = QSize(0, 0);
    if (tlr->height() + trr->height() > br.height()
        || blr->height() + brr->height() > br.height())
        *trr = *brr = QSize(0, 0);
}

void QRenderRule::drawBorder(QPainter *p, const QRect& rect)
{
    Q_ASSERT(hasBorder());
    const QRectF br(rect);
    if (border()->hasBorderImage()) {
        drawBorderImage(p, rect);
        return;
    }

    const BorderStyle *styles = border()->styles;
    const int *borders = border()->borders;
    const QColor *colors = border()->colors;

    QSize tlr, trr, blr, brr;
    getRadii(rect, &tlr, &trr, &blr, &brr);

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

void QRenderRule::drawBackground(QPainter *p, const QRect& rect, const QPoint& off)
{
    QBrush brush = hasBackground() ? background()->brush : QBrush();
    if (brush.style() == Qt::NoBrush)
        brush = defaultBackground;

    if (brush.style() != Qt::NoBrush) {
        QRect fillRect = borderRect(rect);
        if (brush.style() >= Qt::LinearGradientPattern
            && brush.style() <= Qt::ConicalGradientPattern) {
            QTransform m;
            m.translate(fillRect.left(), fillRect.top());
            m.scale(fillRect.width(), fillRect.height());
            brush.setTransform(m);
        }
        p->fillRect(fillRect, brush);
    }
    drawBackgroundImage(p, rect, off);
}

void QRenderRule::drawFrame(QPainter *p, const QRect& rect)
{
    drawBackground(p, rect);
    if (hasBorder())
        drawBorder(p, borderRect(rect));
}

void QRenderRule::drawImage(QPainter *p, const QRect &rect)
{
    if (!hasImage())
        return;
    // draw center aligned
    QRect aligned = QStyle::alignedRect(Qt::LeftToRight, QFlag(Qt::AlignCenter), imageRect.size(), rect);
    QRect inter = aligned.intersected(rect);
    p->drawPixmap(inter.x(), inter.y(), image, imageRect.x() + inter.x() - aligned.x(),
                  imageRect.y() + inter.y() - aligned.y(), inter.width(), inter.height());
}

void QRenderRule::drawRule(QPainter *p, const QRect& rect)
{
    drawFrame(p, rect);
    drawImage(p, contentsRect(rect));
}

// *shudder* , *horror*, *whoa* <-- what you might feel when you see the functions below
void QRenderRule::configurePalette(QPalette *p, QPalette::ColorRole fr, QPalette::ColorRole br)
{
    if (bg && bg->brush.style() != Qt::NoBrush) {
        if (br != QPalette::NoRole)
            p->setBrush(br, bg->brush);
        p->setBrush(QPalette::Window, bg->brush);
    }

    if (!hasPalette())
        return;

    if (pal->foreground.isValid()) {
        if (fr != QPalette::NoRole)
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

void QRenderRule::configurePalette(QPalette *p, QPalette::ColorGroup cg, const QWidget *w, bool embedded)
{
#ifdef QT_NO_COMBOBOX
    const bool isReadOnlyCombo = false;
#else
    const bool isReadOnlyCombo = qobject_cast<const QComboBox *>(w) != 0;
#endif

    if (bg && bg->brush.style() == Qt::SolidPattern) {
        if (isReadOnlyCombo) {
            p->setBrush(cg, QPalette::Base, bg->brush); // for windows, windowxp
            p->setBrush(cg, QPalette::Button, bg->brush); // for plastique
        } else {
            p->setBrush(cg, w->backgroundRole(), bg->brush);
            //p->setBrush(cg, QPalette::Window, bg->brush);
        }
    }

    if (embedded) {
        /* For embedded widgets (ComboBox, SpinBox and ScrollArea) we want the embedded widget
         * to be transparent when we have a transparent background or border image */
        if ((hasBackground() && background()->isTransparent())
            || (hasBorder() && border()->hasBorderImage() && border()->borderImage()->middleRect.isValid()))
            p->setBrush(cg, w->backgroundRole(), Qt::NoBrush);
    }

    if (!hasPalette())
        return;

    if (pal->foreground.isValid()) {
        if (isReadOnlyCombo) {
            p->setBrush(cg, QPalette::ButtonText, pal->foreground);
        } else {
            p->setBrush(cg, w->foregroundRole(), pal->foreground);
            p->setBrush(cg, QPalette::WindowText, pal->foreground);
        }
        p->setBrush(cg, QPalette::Text, pal->foreground);
    }
    if (pal->selectionBackground.style() != Qt::NoBrush)
        p->setBrush(cg, QPalette::Highlight, pal->selectionBackground);
    if (pal->selectionForeground.isValid())
        p->setBrush(cg, QPalette::HighlightedText, pal->selectionForeground);
    if (pal->alternateBackground.style() != Qt::NoBrush)
        p->setBrush(cg, QPalette::AlternateBase, pal->alternateBackground);
}

///////////////////////////////////////////////////////////////////////////////
// Style rules
#define WIDGET(x) (static_cast<QWidget *>(x.ptr))

class QStyleSheetStyleSelector : public StyleSelector
{
public:
    QStyleSheetStyleSelector() { }

    bool nodeNameEquals(NodePtr node, const QString& name) const
    {
        if (WIDGET(node)->inherits(name.toLatin1()))
            return true;
#ifndef QT_NO_TOOLTIP
        if (name == QLatin1String("QToolTip")
            && QString::fromLatin1("QTipLabel")== QString::fromLatin1(WIDGET(node)->metaObject()->className()))
            return true;
#endif
        return false;
    }
    QString attribute(NodePtr node, const QString& name) const
    {
        QVariant value = WIDGET(node)->property(name.toLatin1());
        if (!value.isValid()) {
            if (name == QLatin1String("class")) {
                return QString::fromLatin1(WIDGET(node)->metaObject()->className());
            } else if (name == QLatin1String("style")) {
                QStyleSheetStyle *proxy = qobject_cast<QStyleSheetStyle *>(WIDGET(node)->style());
                if (proxy)
                    return QString::fromLatin1(proxy->baseStyle()->metaObject()->className());
            }
        }
        return value.toString();
    }
    bool hasAttribute(NodePtr node, const QString& name) const
    { return name == QLatin1String("class")
             || name == QLatin1String("style")
             || WIDGET(node)->metaObject()->indexOfProperty(name.toLatin1()) != -1
             || WIDGET(node)->dynamicPropertyNames().contains(name.toLatin1()); }
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

static QVector<QCss::StyleRule> styleRules(QWidget *w)
{
    QStyleSheetStyleSelector styleSelector;

    StyleSheet defaultSs;
    if (!styleSheetCache->contains(0)) {
        QFile defaultQss(QLatin1String(":/trolltech/stylesheet/default.qss"));
        if (!defaultQss.open(QFile::ReadOnly))
            qWarning("could not open defaut stylesheet");
        QString data = QString::fromLatin1(defaultQss.readAll());
        Parser parser(data);
        if (!parser.parse(&defaultSs))
            qWarning("Could not parse default stylesheet");
        defaultSs.origin = StyleSheetOrigin_UserAgent;
        styleSheetCache->insert(0, defaultSs);
    } else {
        defaultSs = styleSheetCache->value(0);
    }
    styleSelector.styleSheets += defaultSs;

    if (!qApp->styleSheet().isEmpty()) {
        StyleSheet appSs;
        if (!styleSheetCache->contains(qApp)) {
            Parser parser1(qApp->styleSheet());
            if (!parser1.parse(&appSs))
                qWarning("Could not parse application stylesheet");
            appSs.origin = StyleSheetOrigin_Inline;
            appSs.depth = 1;
            styleSheetCache->insert(qApp, appSs);
        } else {
            appSs = styleSheetCache->value(qApp);
        }
        styleSelector.styleSheets += appSs;
    }

    QVector<QCss::StyleSheet> widgetSs;
    for (QWidget *wid = w; wid; wid = wid->parentWidget()) {
        if (wid->styleSheet().isEmpty())
            continue;
        StyleSheet ss;
        if (!styleSheetCache->contains(wid)) {
            Parser parser(wid->styleSheet());
            if (!parser.parse(&ss) && wid == w) {
                Parser parser2(QLatin1String("* {") + wid->styleSheet() + QLatin1String("}"));
                if (!parser2.parse(&ss))
                   qWarning("Could not parse stylesheet of widget %p", wid);
            }
            ss.origin = StyleSheetOrigin_Inline;
            styleSheetCache->insert(wid, ss);
        } else {
            ss = styleSheetCache->value(wid);
        }
        widgetSs.prepend(ss);
    }

    for (int i = 0; i < widgetSs.count(); i++)
        widgetSs[i].depth = i + 2;

    styleSelector.styleSheets += widgetSs;

    StyleSelector::NodePtr n;
    n.ptr = w;
    return styleSelector.styleRulesForNode(n);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Rendering rules
enum PseudoElement {
    PseudoElement_None,
    PseudoElement_DownArrow,
    PseudoElement_UpArrow,
    PseudoElement_LeftArrow,
    PseudoElement_RightArrow,
    PseudoElement_Indicator,
    PseudoElement_ExclusiveIndicator,
    PseudoElement_MenuIndicator,
    PseudoElement_ComboBoxDropDown,
    PseudoElement_ComboBoxArrow,
    PseudoElement_Item,
    PseudoElement_SpinBoxUpButton,
    PseudoElement_SpinBoxUpArrow,
    PseudoElement_SpinBoxDownButton,
    PseudoElement_SpinBoxDownArrow,
    PseudoElement_GroupBoxTitle,
    PseudoElement_GroupBoxIndicator,
    PseudoElement_ToolButtonMenu,
    PseudoElement_ToolButtonMenuArrow,
    PseudoElement_ToolButtonDownArrow,
    PseudoElement_ToolBoxTab,
    PseudoElement_ScrollBarSlider,
    PseudoElement_ScrollBarAddPage,
    PseudoElement_ScrollBarSubPage,
    PseudoElement_ScrollBarAddLine,
    PseudoElement_ScrollBarSubLine,
    PseudoElement_ScrollBarFirst,
    PseudoElement_ScrollBarLast,
    PseudoElement_ToolBarHandle,
    PseudoElement_ToolBarSeparator,
    NumPseudoElements
};

struct PseudoElementInfo {
    QStyle::SubControl subControl;
    const char *name;
};

static PseudoElementInfo knownPseudoElements[NumPseudoElements] = {
    { QStyle::SC_None, "", },
    { QStyle::SC_None, "down-arrow" },
    { QStyle::SC_None, "up-arrow" },
    { QStyle::SC_None, "left-arrow" },
    { QStyle::SC_None, "right-arrow" },
    { QStyle::SC_None, "indicator" },
    { QStyle::SC_None, "indicator" },
    { QStyle::SC_None, "menu-indicator" },
    { QStyle::SC_ComboBoxArrow, "drop-down" },
    { QStyle::SC_ComboBoxArrow, "down-arrow" },
    { QStyle::SC_None, "item" },
    { QStyle::SC_SpinBoxUp, "up-button" },
    { QStyle::SC_SpinBoxUp, "up-arrow" },
    { QStyle::SC_SpinBoxDown, "down-button" },
    { QStyle::SC_SpinBoxDown, "down-arrow" },
    { QStyle::SC_GroupBoxLabel, "title" },
    { QStyle::SC_GroupBoxCheckBox, "indicator" },
    { QStyle::SC_ToolButtonMenu, "drop-down" },
    { QStyle::SC_None, "down-arrow" },
    { QStyle::SC_None, "down-arrow" },
    { QStyle::SC_None, "tab" },
    { QStyle::SC_ScrollBarSlider, "slider" },
    { QStyle::SC_ScrollBarAddPage, "add-page" },
    { QStyle::SC_ScrollBarSubPage, "sub-page" },
    { QStyle::SC_ScrollBarAddLine, "add-line" },
    { QStyle::SC_ScrollBarSubLine, "sub-line" },
    { QStyle::SC_ScrollBarFirst, "first" },
    { QStyle::SC_ScrollBarLast, "last" },
    { QStyle::SC_None, "handle" },
    { QStyle::SC_None, "separator" }
};

QVector<Declaration> declarations(const QVector<StyleRule> &styleRules, const QString &part, int pseudoState = PseudoState_Unspecified)
{
    QVector<Declaration> decls;
    for (int i = 0; i < styleRules.count(); i++) {
        const Selector& selector = styleRules.at(i).selectors.at(0);
        // Rules with pseudo elements don't cascade. This is an intentional
        // diversion for CSS
        if (part.compare(selector.pseudoElement(), Qt::CaseInsensitive) != 0)
            continue;
        int cssState = selector.pseudoState();
        if (!(pseudoState & PseudoState_Horizontal))
            cssState &= ~PseudoState_Vertical;
        if ((cssState == PseudoState_Unspecified) || ((cssState & pseudoState) == cssState))
            decls += styleRules.at(i).declarations;
    }
    return decls;
}

int QStyleSheetStyle::nativeFrameWidth(const QWidget *w)
{
    QStyle *base = baseStyle();

#ifndef QT_NO_SPINBOX
    if (qobject_cast<const QAbstractSpinBox *>(w))
        return base->pixelMetric(QStyle::PM_SpinBoxFrameWidth);
#endif

#ifndef QT_NO_COMBOBOX
    if (qobject_cast<const QComboBox *>(w))
        return base->pixelMetric(QStyle::PM_ComboBoxFrameWidth);
#endif

#ifndef QT_NO_MENU
    if (qobject_cast<const QMenu *>(w))
        return base->pixelMetric(QStyle::PM_MenuPanelWidth);
#endif

#ifndef QT_NO_MENUBAR
    if (qobject_cast<const QMenuBar *>(w))
        return base->pixelMetric(QStyle::PM_MenuBarPanelWidth);
#endif

    if (QString::fromLatin1(w->metaObject()->className()) == QLatin1String("QTipLabel"))
        return base->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth);

    return base->pixelMetric(QStyle::PM_DefaultFrameWidth);
}

QRenderRule QStyleSheetStyle::renderRule(const QWidget *w, const QString &part, QStyle::State state) const
{
    Q_ASSERT(w);
    Q_ASSERT(styleRulesCache->contains(w)); // style sheet rules must have been computed!
    QHash<int, QRenderRule> &renderRules = (*renderRulesCache)[w][part];

    int pseudoState = (state & QStyle::State_Enabled)
                                 ? PseudoState_Enabled : PseudoState_Disabled;
    if (state & QStyle::State_Sunken)
        pseudoState |= PseudoState_Pressed;
    if (state & (QStyle::State_MouseOver /*| QStyle::State_Selected*/))
        pseudoState |= PseudoState_Hover;
    if (state & QStyle::State_HasFocus)
        pseudoState |= PseudoState_Focus;
    if (state & QStyle::State_On)
        pseudoState |= PseudoState_On;
    if (state & QStyle::State_Off)
        pseudoState |= PseudoState_Off;
    if (state & QStyle::State_NoChange)
        pseudoState |= PseudoState_Indeterminate;
    if (state & QStyle::State_Selected)
        pseudoState |= PseudoState_Selected;
    if (state & QStyle::State_Horizontal)
        pseudoState |= PseudoState_Horizontal;

    if (renderRules.contains(pseudoState))
        return renderRules[pseudoState]; // already computed before

    QVector<Declaration> decls = declarations((*styleRulesCache)[w], part, pseudoState);
    QRenderRule newRule(decls, w);
    renderRules[pseudoState] = newRule;
    return newRule;
}

QRenderRule QStyleSheetStyle::renderRule(const QWidget *w, int pseudoElement, QStyle::State state) const
{
    return renderRule(w, QLatin1String(knownPseudoElements[pseudoElement].name), state);
}

QRenderRule QStyleSheetStyle::renderRule(const QWidget *w, const QStyleOption *opt, int pseudoElement) const
{
    Q_ASSERT(w && !styleRulesCache->value(w).isEmpty());
    QStyle::State state = opt ? opt->state : QStyle::State(QStyle::State_None);

    if (const QStyleOptionComplex *complex = qstyleoption_cast<const QStyleOptionComplex *>(opt)) {
        if (pseudoElement != PseudoElement_None) {
            // if not an active subcontrol, just pass enabled/disabled
            QStyle::SubControl subControl = knownPseudoElements[pseudoElement].subControl;

            if (!(complex->activeSubControls & subControl))
                state = QStyle::State(state & QStyle::State_Enabled);
        }

        switch (pseudoElement) {
        case PseudoElement_ComboBoxDropDown:
        case PseudoElement_ComboBoxArrow:
            state |= (state & QStyle::State_On); // propagate popup state as on/off
            break;
        case PseudoElement_SpinBoxUpButton:
        case PseudoElement_SpinBoxDownButton:
        case PseudoElement_SpinBoxUpArrow:
        case PseudoElement_SpinBoxDownArrow:
#ifndef QT_NO_SPINBOX
            if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
                bool on = false;
                bool up = pseudoElement == PseudoElement_SpinBoxUpButton
                          || pseudoElement == PseudoElement_SpinBoxUpArrow;
                if ((sb->stepEnabled & QAbstractSpinBox::StepUpEnabled) && up)
                    on = true;
                else if ((sb->stepEnabled & QAbstractSpinBox::StepDownEnabled) && !up)
                    on = true;
                state |= (on ? QStyle::State_On : QStyle::State_Off);
            }
#endif // QT_NO_SPINBOX
            break;
        case PseudoElement_GroupBoxTitle:
            state |= (state & (QStyle::State_MouseOver | QStyle::State_Sunken));
            break;
        case PseudoElement_ToolButtonMenu:
        case PseudoElement_ToolButtonMenuArrow:
        case PseudoElement_ToolButtonDownArrow:
            state |= complex->state & QStyle::State_MouseOver;
            if (complex->state & QStyle::State_Sunken ||
                complex->activeSubControls & QStyle::SC_ToolButtonMenu)
                state |= QStyle::State_Sunken;
            break;
        case PseudoElement_None:
            // QStyle::State_On is set when the popup is being shown
            // Propagate EditField Pressed state
            if (qstyleoption_cast<const QStyleOptionComboBox *>(opt)
                && (complex->activeSubControls & QStyle::SC_ComboBoxEditField)
                && (!(state & QStyle::State_MouseOver))) {
                state |= QStyle::State_Sunken;
            }
            break;
        default:
            break;
        }
    } else {
        // Add hacks for simple controls here
#ifndef QT_NO_LINEEDIT
        // LineEdit sets Sunken flag to indicate Sunken frame (argh)
        if (qobject_cast<const QLineEdit *>(w)) {
            state &= ~QStyle::State_Sunken;
        } else
#endif
#ifndef QT_NO_MENUBAR
        if (qobject_cast<const QMenuBar *>(w)) {
            if (state & QStyle::State_Sunken) {
                state &= ~QStyle::State_Sunken;
                state |= QStyle::State_On;
            }
            if (state & QStyle::State_Selected) {
                state &= ~QStyle::State_Selected;
                state |= QStyle::State_MouseOver;
            }
        } else
#endif
        { } // required for the above ifdef'ery
    }

    return renderRule(w, QLatin1String(knownPseudoElements[pseudoElement].name), state);
}

static bool hasStyleRule(const QWidget *w, int part = PseudoElement_None)
{
    const QVector<StyleRule> &styleRules = styleRulesCache->value(w);
    if (part == PseudoElement_None)
        return w && !styleRules.isEmpty();

    // ### cache the result
    QString pseudoElement = QLatin1String(knownPseudoElements[part].name);
    QVector<Declaration> declarations;
    for (int i = 0; i < styleRules.count(); i++) {
        const Selector& selector = styleRules.at(i).selectors.at(0);
        if (pseudoElement.compare(selector.pseudoElement(), Qt::CaseInsensitive) == 0)
            return true;
    }

    return false;
}

static Origin defaultOrigin(int pe)
{
    switch (pe) {
    case PseudoElement_GroupBoxTitle:
    case PseudoElement_GroupBoxIndicator: // never used
    case PseudoElement_ToolButtonMenu:
        return Origin_Border;

    case PseudoElement_SpinBoxUpButton:
    case PseudoElement_SpinBoxDownButton:
    case PseudoElement_MenuIndicator:
    case PseudoElement_ComboBoxDropDown:
    case PseudoElement_ToolButtonDownArrow:
        return Origin_Padding;

    case PseudoElement_Indicator:
    case PseudoElement_ExclusiveIndicator:
    case PseudoElement_ComboBoxArrow:
    case PseudoElement_SpinBoxUpArrow:
    case PseudoElement_SpinBoxDownArrow:
    case PseudoElement_ToolButtonMenuArrow:
    default:
        return Origin_Content;
    }
}

static Qt::Alignment defaultPosition(int pe)
{
    switch (pe) {
    case PseudoElement_Indicator:
    case PseudoElement_ExclusiveIndicator:
        return Qt::AlignLeft | Qt::AlignVCenter;

    case PseudoElement_SpinBoxDownButton:
    case PseudoElement_MenuIndicator:
    case PseudoElement_ToolButtonDownArrow:
        return Qt::AlignRight | Qt::AlignBottom;

    case PseudoElement_SpinBoxUpButton:
    case PseudoElement_ComboBoxDropDown:
    case PseudoElement_ToolButtonMenu:
        return Qt::AlignRight | Qt::AlignTop;

    case PseudoElement_SpinBoxUpArrow:
    case PseudoElement_SpinBoxDownArrow:
    case PseudoElement_ComboBoxArrow:
    case PseudoElement_DownArrow:
        case PseudoElement_ToolButtonMenuArrow:
        return Qt::AlignCenter;

    case PseudoElement_GroupBoxTitle:
    case PseudoElement_GroupBoxIndicator: // never used
        return Qt::AlignLeft | Qt::AlignTop;

    default:
        return 0;
    }
}

QSize QStyleSheetStyle::defaultSize(const QWidget *w, QSize sz, const QRect& rect, int pe) const
{
    QStyle *base = baseStyle();

    switch (pe) {
    case PseudoElement_Indicator:
        if (sz.width() == -1)
            sz.setWidth(base->pixelMetric(PM_IndicatorWidth, 0, w));
        if (sz.height() == -1)
            sz.setHeight(base->pixelMetric(PM_IndicatorHeight, 0, w));
        break;

    case PseudoElement_ExclusiveIndicator:
    case PseudoElement_GroupBoxIndicator:
        if (sz.width() == -1)
            sz.setWidth(base->pixelMetric(PM_ExclusiveIndicatorWidth, 0, w));
        if (sz.height() == -1)
            sz.setHeight(base->pixelMetric(PM_ExclusiveIndicatorHeight, 0, w));
        break;

    case PseudoElement_MenuIndicator: {
        int pm = base->pixelMetric(PM_MenuButtonIndicator, 0, w);
        if (sz.width() == -1)
            sz.setWidth(pm);
        if (sz.height() == -1)
            sz.setHeight(pm);
                                      }
        break;

    case PseudoElement_ComboBoxDropDown:
        if (sz.width() == -1)
            sz.setWidth(16);
        break;

    case PseudoElement_ComboBoxArrow:
    case PseudoElement_DownArrow:
    case PseudoElement_ToolButtonMenuArrow:
    case PseudoElement_ToolButtonDownArrow:
        if (sz.width() == -1)
            sz.setWidth(13);
        if (sz.height() == -1)
            sz.setHeight(13);
        break;

    case PseudoElement_SpinBoxUpButton:
    case PseudoElement_SpinBoxDownButton:
        if (sz.width() == -1)
            sz.setWidth(16);
        if (sz.height() == -1)
            sz.setHeight(rect.height()/2);
        break;

    case PseudoElement_ToolButtonMenu:
        if (sz.width() == -1)
            sz.setWidth(base->pixelMetric(PM_MenuButtonIndicator, 0, w));
        break;

    default:
        break;
    }

    // expand to rectangle
    if (sz.height() == -1)
        sz.setHeight(rect.height());
    if (sz.width() == -1)
        sz.setWidth(rect.width());

    return sz;
}

QRect QStyleSheetStyle::positionRect(const QWidget *w, const QRenderRule& rule1, const QRenderRule& rule2, int pe,
                                     const QRect& rect, Qt::LayoutDirection dir) const
{
    const QStyleSheetPositionData *p = rule2.position();
    Origin origin = (p && p->origin != Origin_Unknown) ? p->origin : defaultOrigin(pe);
    QRect originRect = rule1.originRect(rect, origin);
    PositionMode mode = p ? p->mode : PositionMode_Static;
    Qt::Alignment position = (p && p->position != 0) ? p->position : defaultPosition(pe);
    QRect r;

    if (mode != PositionMode_Absolute) {
        QSize sz = defaultSize(w, rule2.size(), originRect, pe);
        sz = rule2.boxSize(sz.expandedTo(rule2.minimumContentsSize()));
        r = QStyle::alignedRect(dir, position, sz, originRect);
        if (p) {
            int left = p->left ? p->left : -p->right;
            int top = p->top ? p->top : -p->bottom;
            r.translate(dir == Qt::LeftToRight ? left : -left, top);
        }
    } else {
        if (p)
            r = originRect.adjusted(dir == Qt::LeftToRight ? p->left : p->right, p->top,
                                    dir == Qt::LeftToRight ? -p->right : -p->left, -p->bottom);
        if (rule2.hasContentsSize()) {
            QSize sz = defaultSize(w, rule2.size(), r, pe);
            sz = rule2.boxSize(sz.expandedTo(rule2.minimumContentsSize()));
            r = QStyle::alignedRect(dir, position, sz, r);
        }
    }
    return r;
}

static QWidget *embeddedWidget(QWidget *w)
{
#ifndef QT_NO_COMBOBOX
    if (QComboBox *cmb = qobject_cast<QComboBox *>(w))
        if (cmb->isEditable())
            return cmb->lineEdit();
        else
            return cmb;
#endif

#ifndef QT_NO_SPINBOX
    if (QAbstractSpinBox *sb = qobject_cast<QAbstractSpinBox *>(w))
        return qFindChild<QLineEdit *>(sb);
#endif

#ifndef QT_NO_SCROLLAREA
    if (QAbstractScrollArea *sa = qobject_cast<QAbstractScrollArea *>(w))
        return sa->viewport();
#endif

    return w;
}

static void setProperties(QWidget *w)
{
    QHash<QString, QVariant> propertyHash;
    QVector<Declaration> decls = declarations((*styleRulesCache)[w], QString());
    // run through the declarations in order
    for (int i = 0; i < decls.count(); i++) {
        Declaration decl = decls.at(i);
        QString property = decl.property;
        if (!property.startsWith(QLatin1String("qproperty-"), Qt::CaseInsensitive))
            continue;
        property = property.mid(10); // strip "qproperty-"
        const QVariant value = w->property(property.toLatin1()); // takes care of dynamic properties too
        if (!value.isValid()) {
            qWarning() << w << " does not have a property named " << property;
            continue;
        }
        QVariant v;
        switch (value.type()) {
        case QVariant::Icon: v = QIcon(decl.uriValue()); break;
        case QVariant::Image: v = QImage(decl.uriValue()); break;
        case QVariant::Pixmap: v = QPixmap(decl.uriValue()); break;
        case QVariant::Rect: v = decl.rectValue(); break;
        case QVariant::Size: v = decl.sizeValue(); break;
        case QVariant::Color: v = decl.colorValue(); break;
        case QVariant::Brush: v = decl.brushValue(); break;
        case QVariant::KeySequence: v = QKeySequence(decl.values.first().variant.toString());
        default: v = decl.values.first().variant; break;
        }
        propertyHash[property] = v;
    }
    // apply the values
    const QList<QString> properties = propertyHash.keys();
    for (int i = 0; i < properties.count(); i++) {
        const QString property = properties.at(i);
        w->setProperty(property.toLatin1(), propertyHash[property]);
    }
}

void QStyleSheetStyle::setPalette(QWidget *w)
{
    const QRenderRule &hoverRule = renderRule(w, PseudoElement_None, QStyle::State_MouseOver);
    if (hoverRule.hasDrawable())
        w->setAttribute(Qt::WA_Hover);

    struct RuleRoleMap {
        QStyle::StateFlag state;
        QPalette::ColorGroup group;
    } map[3] = {
        { QStyle::State_Enabled, QPalette::Active },
        { QStyle::State_None, QPalette::Disabled },
        { QStyle::State_Enabled, QPalette::Inactive }
    };

    QPalette p = w->palette();
    QWidget *ew = embeddedWidget(w);

    for (int i = 0; i < 3; i++) {
        QRenderRule rule = renderRule(w, PseudoElement_None, map[i].state);
        if (i == 0) {
            customFontWidgets->insert(w, rule.font.resolve() & ~w->font().resolve());
            w->setFont(rule.font);

            if (ew->autoFillBackground() &&
                (rule.hasBackground()
                 || (rule.hasBorder() && rule.border()->hasBorderImage()
                     && rule.border()->borderImage()->middleRect.isValid()))) {
                ew->setAutoFillBackground(false);
                autoFillDisabledWidgets->insert(w);
            }
        }

        rule.configurePalette(&p, map[i].group, ew, ew != w);
    }

    if (w->palette() != p) {
        customPaletteWidgets->insert(w, p.resolve() & ~w->palette().resolve());
        w->setPalette(p);
    }
}

void QStyleSheetStyle::unsetPalette(QWidget *w)
{
    if (customPaletteWidgets->contains(w)) {
        QPalette p = QApplication::palette(w);
        p.resolve(customPaletteWidgets->value(w));
        p.resolve(w->palette());
        w->setPalette(p);
        customPaletteWidgets->remove(w);
    }
    if (customFontWidgets->contains(w)) {
        QFont f = QApplication::font();
        f.resolve(customFontWidgets->value(w));
        f.resolve(w->font());
        w->setFont(f);
        customFontWidgets->remove(w);
    }
    if (autoFillDisabledWidgets->contains(w)) {
        embeddedWidget(w)->setAutoFillBackground(true);
        autoFillDisabledWidgets->remove(w);
    }
}

static void updateWidget(QWidget *widget)
{
    setProperties(widget);
    QEvent e(QEvent::StyleChange);
    QApplication::sendEvent(widget, &e);
    widget->update();
    widget->updateGeometry();
}

static void updateWidgets(const QList<const QWidget *>& widgets)
{
    for (int i = 0; i < widgets.size(); ++i) {
        QWidget *widget = const_cast<QWidget *>(widgets.at(i));
        Q_ASSERT(widget);
        styleRulesCache->remove(widget);
        renderRulesCache->remove(widget);
        widget->style()->polish(widget);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// The stylesheet style
int QStyleSheetStyle::numinstances = 0;

QStyleSheetStyle::QStyleSheetStyle(QStyle *base)
: base(base), refcount(1)
{
    ++numinstances;
    if (numinstances == 1) {
        styleRulesCache = new QHash<const QWidget *, QVector<StyleRule> >;
        renderRulesCache = new QHash<const QWidget *, QRenderRules>;
        customPaletteWidgets = new QHash<const QWidget *, int>;
        customFontWidgets = new QHash<const QWidget *, int>;
        styleSheetCache = new QHash<void *, StyleSheet>;
        autoFillDisabledWidgets = new QSet<const QWidget *>;
    }
}

QStyleSheetStyle::~QStyleSheetStyle()
{
    --numinstances;
    if (numinstances == 0) {
        delete styleRulesCache;
        styleRulesCache = 0;
        delete renderRulesCache;
        renderRulesCache = 0;
        delete customPaletteWidgets;
        customPaletteWidgets = 0;
        delete customFontWidgets;
        customFontWidgets = 0;
        delete styleSheetCache;
        styleSheetCache = 0;
        delete autoFillDisabledWidgets;
        autoFillDisabledWidgets = 0;
    }
}
QStyle *QStyleSheetStyle::baseStyle() const
{
    if (base)
        return base;
    if (QStyleSheetStyle *me = qobject_cast<QStyleSheetStyle *>(qApp->style()))
        return me->base;
    return qApp->style();
}

void QStyleSheetStyle::widgetDestroyed(QObject *o)
{
    styleRulesCache->remove((const QWidget *)o);
    renderRulesCache->remove((const QWidget *)o);
    customPaletteWidgets->remove((const QWidget *)o);
}

static bool unstylable(QWidget *w)
{
    if (w->windowType() == Qt::Desktop)
        return true;

#ifndef QT_NO_SCROLLAREA
    if (QAbstractScrollArea *sa = qobject_cast<QAbstractScrollArea *>(w->parentWidget())) {
        if (sa->viewport() == w)
            return true;
    } else
#endif
#ifndef QT_NO_LINEEDIT
    if (qobject_cast<QLineEdit *>(w)) {
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

void QStyleSheetStyle::polish(QWidget *w)
{
    if (styleRulesCache->contains(w))
        return;

    baseStyle()->polish(w);
    if (unstylable(w))
        return;

    QObject::connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(widgetDestroyed(QObject*)));
    renderRulesCache->remove(w);
    QVector<QCss::StyleRule> rules = styleRules(w);
    styleRulesCache->insert(w, rules);
    unsetPalette(w);
    if (!rules.isEmpty()) {
        setPalette(w);
    }

#ifndef QT_NO_SCROLLAREA
    if (QAbstractScrollArea *sa = qobject_cast<QAbstractScrollArea *>(w)) {
        QRenderRule rule = renderRule(sa, 0);
        if ((rule.hasBorder() && rule.border()->hasBorderImage())
            || (rule.hasBackground() && !rule.background()->pixmap.isNull())) {
            QObject::connect(sa->horizontalScrollBar(), SIGNAL(valueChanged(int)),
                             sa, SLOT(update()));
            QObject::connect(sa->verticalScrollBar(), SIGNAL(valueChanged(int)),
                             sa, SLOT(update()));
        }
    }
#endif

    const QMetaObject *me = w->metaObject();
    const QMetaObject *super = w->metaObject()->superClass();
    bool on = QString::fromLocal8Bit(me->className()) == QLatin1String("QWidget")
#ifndef QT_NO_MENUBAR
              || qobject_cast<QMenuBar *>(w)
#endif
#ifndef QT_NO_STATUSBAR
              || qobject_cast<QStatusBar *>(w)
#endif
#ifndef QT_NO_MENU
              || qobject_cast<QMenu *>(w)
#endif
              || QString::fromLocal8Bit(me->className()) == QLatin1String("QDialog")
              || QString::fromLocal8Bit(super->className()) == QLatin1String("QDialog");

    w->setAttribute(Qt::WA_StyledBackground, on);
    updateWidget(w);
}

void QStyleSheetStyle::polish(QApplication *app)
{
    styleRulesCache->clear();
    renderRulesCache->clear();
    baseStyle()->polish(app);
}

void QStyleSheetStyle::polish(QPalette &pal)
{
    baseStyle()->polish(pal);
}

void QStyleSheetStyle::repolish(QWidget *w)
{
    QList<const QWidget *> children = qFindChildren<const QWidget *>(w, QString());
    children.append(w);
    styleSheetCache->remove(w);
    updateWidgets(children);
}

void QStyleSheetStyle::repolish(QApplication *)
{
    styleSheetCache->remove(qApp);
    updateWidgets(styleRulesCache->keys());
}

void QStyleSheetStyle::unpolish(QWidget *w)
{
    styleRulesCache->remove(w);
    renderRulesCache->remove(w);
    styleSheetCache->remove(w);
    baseStyle()->unpolish(w);
    unsetPalette(w);
    QObject::disconnect(w, SIGNAL(destroyed(QObject*)),
                      this, SLOT(widgetDestroyed(QObject*)));
    if (QAbstractScrollArea *sa = qobject_cast<QAbstractScrollArea *>(w)) {
        QObject::disconnect(sa->horizontalScrollBar(), SIGNAL(valueChanged(int)),
                            sa, SLOT(update()));
        QObject::disconnect(sa->verticalScrollBar(), SIGNAL(valueChanged(int)),
                            sa, SLOT(update()));
    }
}

void QStyleSheetStyle::unpolish(QApplication *app)
{
    styleRulesCache->clear();
    renderRulesCache->clear();
    styleSheetCache->remove(qApp);
    baseStyle()->unpolish(app);
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
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            QStyleOptionComboBox cmbOpt(*cmb);
            cmbOpt.rect = rule.borderRect(opt->rect);
            if (rule.nativeBorder()) {
                rule.drawBackgroundImage(p, cmbOpt.rect);
                bool customDropDown = (opt->subControls & QStyle::SC_ComboBoxArrow)
                                      && hasStyleRule(w, PseudoElement_ComboBoxDropDown);
                if (customDropDown)
                    cmbOpt.subControls &= ~QStyle::SC_ComboBoxArrow;
                if (rule.baseStyleCanDraw()) {
                    baseStyle()->drawComplexControl(cc, &cmbOpt, p, w);
                } else {
                    QWindowsStyle::drawComplexControl(cc, &cmbOpt, p, w);
                }
                if (!customDropDown)
                    return;
            } else {
                rule.drawFrame(p, opt->rect);
            }

            if (opt->subControls & QStyle::SC_ComboBoxArrow) {
                QRenderRule subRule = renderRule(w, opt, PseudoElement_ComboBoxDropDown);
                if (subRule.hasDrawable()) {
                        QRect r = subControlRect(CC_ComboBox, opt, SC_ComboBoxArrow, w);
                        subRule.drawRule(p, r);
                        QRenderRule subRule2 = renderRule(w, opt, PseudoElement_ComboBoxArrow);
                        r = positionRect(w, subRule, subRule2, PseudoElement_ComboBoxArrow, r, opt->direction);
                        subRule2.drawRule(p, r);
                } else {
                        cmbOpt.subControls = QStyle::SC_ComboBoxArrow;
                        QWindowsStyle::drawComplexControl(cc, &cmbOpt, p, w);
                }
            }

            return;
        }
        break;

    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spin = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            QStyleOptionSpinBox spinOpt(*spin);
            spinOpt.rect = rule.borderRect(opt->rect);
            bool customUp = true, customDown = true;
            if (rule.hasNativeBorder()) {
                rule.drawBackgroundImage(p, spinOpt.rect);
                customUp = (opt->subControls & QStyle::SC_SpinBoxUp)
                           && hasStyleRule(w, PseudoElement_SpinBoxUpButton);
                if (customUp)
                        spinOpt.subControls &= ~QStyle::SC_SpinBoxUp;
                customDown = (opt->subControls & QStyle::SC_SpinBoxDown)
                                         && hasStyleRule(w, PseudoElement_SpinBoxDownButton);
                if (customDown)
                        spinOpt.subControls &= ~QStyle::SC_SpinBoxDown;
                if (rule.baseStyleCanDraw()) {
                        baseStyle()->drawComplexControl(cc, &spinOpt, p, w);
                } else {
                        QWindowsStyle::drawComplexControl(cc, &spinOpt, p, w);
                }
                if (!customUp && !customDown)
                        return;
            } else {
                rule.drawFrame(p, opt->rect);
            }

            if ((opt->subControls & QStyle::SC_SpinBoxUp) && customUp) {
                QRenderRule subRule = renderRule(w, opt, PseudoElement_SpinBoxUpButton);
                if (subRule.hasDrawable()) {
                    QRect r = subControlRect(CC_SpinBox, opt, SC_SpinBoxUp, w);
                    subRule.drawRule(p, r);
                    QRenderRule subRule2 = renderRule(w, opt, PseudoElement_SpinBoxUpArrow);
                    r = positionRect(w, subRule, subRule2, PseudoElement_SpinBoxUpArrow, r, opt->direction);
                    subRule2.drawRule(p, r);
                } else {
                    spinOpt.subControls = QStyle::SC_SpinBoxUp;
                    QWindowsStyle::drawComplexControl(cc, &spinOpt, p, w);
                }
            }

            if ((opt->subControls & QStyle::SC_SpinBoxDown) && customDown) {
                QRenderRule subRule = renderRule(w, opt, PseudoElement_SpinBoxDownButton);
                if (subRule.hasDrawable()) {
                    QRect r = subControlRect(CC_SpinBox, opt, SC_SpinBoxDown, w);
                    subRule.drawRule(p, r);
                    QRenderRule subRule2 = renderRule(w, opt, PseudoElement_SpinBoxDownArrow);
                    r = positionRect(w, subRule, subRule2, PseudoElement_SpinBoxDownArrow, r, opt->direction);
                    subRule2.drawRule(p, r);
                } else {
                    spinOpt.subControls = QStyle::SC_SpinBoxDown;
                    QWindowsStyle::drawComplexControl(cc, &spinOpt, p, w);
                }
            }
            return;
        }
        break;

    case CC_GroupBox:
        if (const QStyleOptionGroupBox *gb = qstyleoption_cast<const QStyleOptionGroupBox *>(opt)) {
            rule.drawBackground(p, opt->rect);

            QRect labelRect, checkBoxRect, titleRect, frameRect;
            bool hasTitle = (gb->subControls & QStyle::SC_GroupBoxCheckBox) || !gb->text.isEmpty();
            QRenderRule titleRule = renderRule(w, opt, PseudoElement_GroupBoxTitle);

            bool clipSet = false;

            if (hasTitle) {
                labelRect = subControlRect(CC_GroupBox, opt, SC_GroupBoxLabel, w);
                if (gb->subControls & QStyle::SC_GroupBoxCheckBox) {
                    checkBoxRect = subControlRect(CC_GroupBox, opt, SC_GroupBoxCheckBox, w);
                    titleRect = titleRule.boxRect(checkBoxRect.united(labelRect));
                } else {
                    titleRect = titleRule.boxRect(labelRect);
                }
                if (!titleRule.hasBackground() || !titleRule.background()->isTransparent()) {
                    clipSet = true;
                    p->save();
                    p->setClipRegion(QRegion(opt->rect) - titleRect);
                }
            }

            frameRect = subControlRect(CC_GroupBox, opt, SC_GroupBoxFrame, w);
            QStyleOptionFrameV2 frame;
            frame.QStyleOption::operator=(*gb);
            frame.features = gb->features;
            frame.lineWidth = gb->lineWidth;
            frame.midLineWidth = gb->midLineWidth;
            frame.rect = frameRect;
            drawPrimitive(PE_FrameGroupBox, &frame, p, w);

            if (clipSet)
                p->restore();

            // draw background and frame of the title
            if (hasTitle)
                titleRule.drawRule(p, titleRect);

            // draw the indicator
            if (gb->subControls & QStyle::SC_GroupBoxCheckBox) {
                QStyleOptionButton box;
                box.QStyleOption::operator=(*gb);
                box.rect = checkBoxRect;
                drawPrimitive(PE_IndicatorCheckBox, &box, p, w);
            }

            // draw the text
            if (!gb->text.isEmpty()) {
                int alignment = int(Qt::AlignCenter);
                if (!styleHint(QStyle::SH_UnderlineShortcut, opt, w))
                    alignment |= Qt::TextHideMnemonic;

                QPalette pal = gb->palette;
                if (gb->textColor.isValid())
                    pal.setColor(QPalette::WindowText, gb->textColor);
                titleRule.configurePalette(&pal, QPalette::WindowText, QPalette::Window);
                drawItemText(p, labelRect,  alignment, pal, gb->state & State_Enabled,
                             gb->text, QPalette::WindowText);
            }

                        return;
        }
        break;

    case CC_ToolButton:
        if (const QStyleOptionToolButton *tool = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            QStyleOptionToolButton toolOpt(*tool);
            toolOpt.rect = rule.borderRect(opt->rect);
            bool customArrow = (tool->features & QStyleOptionToolButton::HasMenu), 
                     customDropDown = (tool->subControls & QStyle::SC_ToolButtonMenu);
            if (rule.hasNativeBorder()) {
                rule.drawBackground(p, toolOpt.rect);
                customArrow = customArrow && hasStyleRule(w, PseudoElement_ToolButtonDownArrow);
                if (customArrow)
                        toolOpt.features &= ~QStyleOptionToolButton::HasMenu;
                customDropDown = customDropDown && hasStyleRule(w, PseudoElement_ToolButtonMenu);
                if (customDropDown)
                    toolOpt.subControls &= ~QStyle::SC_ToolButtonMenu;

                if (rule.baseStyleCanDraw()) {
                    baseStyle()->drawComplexControl(cc, &toolOpt, p, w);
                } else {
                    QWindowsStyle::drawComplexControl(cc, &toolOpt, p, w);
                }
                if (!customArrow && !customDropDown)
                    return;
            } else {
                rule.drawFrame(p, opt->rect);
                toolOpt.rect = rule.contentsRect(opt->rect);
                drawControl(CE_ToolButtonLabel, &toolOpt, p, w);
            }

            QRenderRule subRule = renderRule(w, opt, PseudoElement_ToolButtonMenu);
            QRect r = subControlRect(CC_ToolButton, opt, QStyle::SC_ToolButtonMenu, w);
            if (customDropDown) {
                if (opt->subControls & QStyle::SC_ToolButtonMenu) {
                    if (subRule.hasDrawable()) {
                        subRule.drawRule(p, r);
                    } else {
                        toolOpt.rect = r;
                        baseStyle()->drawPrimitive(PE_IndicatorButtonDropDown, &toolOpt, p, w);
                    }
                }
            }

            if (customArrow) {
                QRenderRule subRule2 = customDropDown ? renderRule(w, opt, PseudoElement_ToolButtonMenuArrow)
                                                      : renderRule(w, opt, PseudoElement_ToolButtonDownArrow);
                QRect r2 = customDropDown
                          ? positionRect(w, subRule, subRule2, PseudoElement_ToolButtonMenuArrow, r, opt->direction)
                          : positionRect(w, rule, subRule2, PseudoElement_ToolButtonDownArrow, opt->rect, opt->direction);
                if (subRule2.hasDrawable()) {
                        subRule2.drawRule(p, r2);
                } else {
                    toolOpt.rect = r2;
                    baseStyle()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &toolOpt, p, w);
                }
            }

            return;
        }
        break;

    case CC_ScrollBar:
        if (rule.hasDrawable() || rule.hasBox()) {
            rule.drawRule(p, opt->rect);
            ParentStyle::drawComplexControl(cc, opt, p, w);
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
    int pe1 = PseudoElement_None, pe2 = PseudoElement_None;

    switch (ce) {
    case CE_ToolButtonLabel:
        if (const QStyleOptionToolButton *btn = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            QStyleOptionToolButton butOpt(*btn);
            rule.configurePalette(&butOpt.palette, QPalette::ButtonText, QPalette::Button);
            baseStyle()->drawControl(ce, &butOpt, p, w);
            return;
        }
        break;

    // Push button
    case CE_PushButton:
        ParentStyle::drawControl(ce, opt, p, w);
        return;

    case CE_PushButtonBevel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QStyleOptionButton btnOpt(*btn);
            btnOpt.rect = rule.borderRect(opt->rect);
            if (rule.nativeBorder()) {
                rule.drawBackgroundImage(p, btnOpt.rect);
                rule.configurePalette(&btnOpt.palette, QPalette::ButtonText, QPalette::Button);
                bool customMenu = (btn->features & QStyleOptionButton::HasMenu
                                   && hasStyleRule(w, PseudoElement_MenuIndicator));
                if (customMenu)
                    btnOpt.features &= ~QStyleOptionButton::HasMenu;
                if (rule.baseStyleCanDraw()) {
                    baseStyle()->drawControl(ce, &btnOpt, p, w);
                } else {
                    QWindowsStyle::drawControl(ce, &btnOpt, p, w);
                }
                if (!customMenu)
                    return;
            } else {
                rule.drawFrame(p, opt->rect);
            }

            if (btn->features & QStyleOptionButton::HasMenu) {
                QRenderRule subRule = renderRule(w, opt, PseudoElement_MenuIndicator);
                QRect ir = positionRect(w, rule, subRule, PseudoElement_MenuIndicator, opt->rect, opt->direction);
                if (subRule.hasDrawable()) {
                    subRule.drawRule(p, ir);
                } else {
                    btnOpt.rect = ir;
                    baseStyle()->drawPrimitive(PE_IndicatorArrowDown, &btnOpt, p, w);
                }
            }
        }
        return;

    case CE_PushButtonLabel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QStyleOptionButton butOpt(*btn);
            rule.configurePalette(&butOpt.palette, QPalette::ButtonText, QPalette::Button);
            ParentStyle::drawControl(ce, &butOpt, p, w);
        }
        return;

    case CE_RadioButton:
    case CE_CheckBox:
        rule.drawFrame(p, opt->rect);
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
            rule.drawRule(p, opt->rect);
            return;
        }
        break;

    case CE_ToolBar:
        if (rule.hasBackground()) {
            rule.drawBackground(p, opt->rect);
        }
        if (rule.hasBorder()) {
            rule.drawBorder(p, rule.borderRect(opt->rect));
        } else {
#ifndef QT_NO_TOOLBAR
            if (const QStyleOptionToolBar *tb = qstyleoption_cast<const QStyleOptionToolBar *>(opt)) {
                QStyleOptionToolBar newTb(*tb);
                newTb.rect = rule.borderRect(opt->rect);
                baseStyle()->drawControl(ce, &newTb, p, w);
            }
#endif // QT_NO_TOOLBAR
        }
        return;

    case CE_MenuEmptyArea:
    case CE_MenuBarEmptyArea:
        if (rule.hasBackground()) {
            return;
        }
        break;

    case CE_MenuTearoff:
    case CE_MenuScroller:
        break;

    case CE_MenuBarItem:
    case CE_MenuItem:
        if (const QStyleOptionMenuItem *m = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            QStyleOptionMenuItem mi(*m);
            QRenderRule subRule = renderRule(w, opt, PseudoElement_Item);
            mi.rect = subRule.contentsRect(opt->rect);
            rule.configurePalette(&mi.palette, QPalette::ButtonText, QPalette::Button);
            subRule.configurePalette(&mi.palette, QPalette::ButtonText, QPalette::Button);

            if (subRule.hasDrawable()) {
                subRule.drawRule(p, opt->rect);
                QCommonStyle::drawControl(ce, &mi, p, w);
            } else {
                baseStyle()->drawControl(ce, &mi, p, w);
            }
        }
        return;

#ifndef QT_NO_COMBOBOX
    case CE_ComboBoxLabel:
        if (!rule.hasBox())
                        break;
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            QRect editRect = subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, w);
            p->save();
            p->setClipRect(editRect);
            if (!cb->currentIcon.isNull()) {
                int spacing = rule.hasBox() ? rule.box()->spacing : -1;
                if (spacing == -1)
                        spacing = 6;
                QIcon::Mode mode = cb->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
                QPixmap pixmap = cb->currentIcon.pixmap(cb->iconSize, mode);
                QRect iconRect(editRect);
                iconRect.setWidth(cb->iconSize.width());
                iconRect = alignedRect(QApplication::layoutDirection(),
                                                           Qt::AlignLeft | Qt::AlignVCenter,
                                                           iconRect.size(), editRect);
                drawItemPixmap(p, iconRect, Qt::AlignCenter, pixmap);

                if (cb->direction == Qt::RightToLeft)
                        editRect.translate(-spacing - cb->iconSize.width(), 0);
                else
                        editRect.translate(cb->iconSize.width() + spacing, 0);
            }
            if (!cb->currentText.isEmpty() && !cb->editable) {
                drawItemText(p, editRect.adjusted(0, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, cb->palette,
                             cb->state & State_Enabled, cb->currentText);
            }
            p->restore();
            return;
        }
    break;
#endif // QT_NO_COMBOBOX

    case CE_SizeGrip:
        if (!rule.hasDrawable())
            break;
        if (const QStyleOptionSizeGrip *sgOpt = qstyleoption_cast<const QStyleOptionSizeGrip *>(opt)) {
            if (rule.hasDrawable()) {
                rule.drawFrame(p, opt->rect);
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
            } else {
                QStyleOptionSizeGrip sg(*sgOpt);
                sg.rect = rule.contentsRect(opt->rect);
                baseStyle()->drawControl(CE_SizeGrip, &sg, p, w);
            }
            return;
        }
        break;

    case CE_ToolBoxTabShape:
        if (hasStyleRule(w, PseudoElement_ToolBoxTab)) {
            QRenderRule subRule = renderRule(w, opt, PseudoElement_ToolBoxTab);
            subRule.drawRule(p, opt->rect);
            return;
        }
        break;

    case CE_ScrollBarAddPage:
        pe1 = PseudoElement_ScrollBarAddPage;
        break;

    case CE_ScrollBarSubPage:
        pe1 = PseudoElement_ScrollBarSubPage;
        break;

    case CE_ScrollBarAddLine:
        pe1 = PseudoElement_ScrollBarAddLine;
        pe2 = PseudoElement_DownArrow;
        break;

    case CE_ScrollBarSubLine:
        pe1 = PseudoElement_ScrollBarSubLine;
        pe2 = PseudoElement_UpArrow;
        break;

    case CE_ScrollBarFirst:
        pe1 = PseudoElement_ScrollBarFirst;
        break;

    case CE_ScrollBarLast:
        pe1 = PseudoElement_ScrollBarLast;
        break;

    case CE_ScrollBarSlider:
        pe1 = PseudoElement_ScrollBarSlider;
        break;

    default:
        break;
    }

    if (pe1 != PseudoElement_None) {
        if (!hasStyleRule(w, pe1))
            return;
        QRenderRule subRule = renderRule(w, opt, pe1);
        subRule.drawRule(p, opt->rect);
        if (pe2 != PseudoElement_None) {
            QRenderRule subSubRule = renderRule(w, opt, pe2);
            QRect r = positionRect(w, subRule, subSubRule, pe2, opt->rect, opt->direction);
            subSubRule.drawRule(p, r);
        }
        return;
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
    if (pe == PE_FrameStatusBar) {
        if (hasStyleRule(w->parentWidget())) {
            QRenderRule subRule = renderRule(w->parentWidget(), opt, PseudoElement_Item);
            if (subRule.hasDrawable()) {
                subRule.drawRule(p, opt->rect);
                return;
            }
        }
        baseStyle()->drawPrimitive(pe, opt, p, w);
        return;
    }

    if (!hasStyleRule(w)) {
        baseStyle()->drawPrimitive(pe, opt, p, w);
        return;
    }

    int pseudoElement = PseudoElement_None;

    QRenderRule rule = renderRule(w, opt);

    switch (pe) {
    case PE_IndicatorArrowDown:
        pseudoElement = PseudoElement_DownArrow;
        break;

    case PE_IndicatorRadioButton:
        pseudoElement = PseudoElement_ExclusiveIndicator;
        break;

    case PE_IndicatorCheckBox:
    case PE_IndicatorViewItemCheck:
        pseudoElement = PseudoElement_Indicator;
        break;

    case PE_PanelButtonTool:
    case PE_PanelButtonCommand:
        if (!rule.nativeBorder()) {
            rule.drawRule(p, rule.boxRect(opt->rect, QRenderRule::Margin));
                        return;
        }
        break;

    case PE_IndicatorButtonDropDown: {
        QRenderRule subRule = renderRule(w, opt, PseudoElement_ToolButtonMenu);
        if (!subRule.nativeBorder()) {
            rule.drawBorder(p, opt->rect);
            return;
        }
        break;
                                     }

    case PE_FrameDefaultButton:
        if (rule.nativeBorder()) {
            if (rule.baseStyleCanDraw())
                break;
            QWindowsStyle::drawPrimitive(pe, opt, p, w);
        }
        return;

    case PE_Frame:
        if (const QStyleOptionFrame *frm = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            if (const QAbstractScrollArea *sa = qobject_cast<const QAbstractScrollArea *>(w)) {
                const QAbstractScrollAreaPrivate *sap = sa->d_func();
                rule.drawBackground(p, opt->rect, sap->contentsOffset());
            } else {
                rule.drawBackground(p, opt->rect);
            }

            if (rule.hasNativeBorder()) {
                QStyleOptionFrame frmOpt(*frm);
                rule.configurePalette(&frmOpt.palette, QPalette::Text, QPalette::Base);
                frmOpt.rect = rule.borderRect(frmOpt.rect);
                baseStyle()->drawPrimitive(pe, &frmOpt, p, w);
            } else {
                rule.drawBorder(p, rule.borderRect(opt->rect));
            }
        }
        return;

    case PE_PanelLineEdit:
        if (const QStyleOptionFrame *frm = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            if (rule.hasNativeBorder()) {
                QStyleOptionFrame frmOpt(*frm);
                rule.configurePalette(&frmOpt.palette, QPalette::Text, QPalette::Base);
                frmOpt.rect = rule.borderRect(frmOpt.rect);

                if (rule.baseStyleCanDraw()) {
                    rule.drawBackgroundImage(p, opt->rect);
                    baseStyle()->drawPrimitive(pe, &frmOpt, p, w);
                } else {
                    rule.drawBackground(p, opt->rect);
                    baseStyle()->drawPrimitive(PE_FrameLineEdit, &frmOpt, p, w);
                }
            } else {
                rule.drawRule(p, opt->rect);
            }
        }
        return;

    case PE_Widget:
        rule.drawBackground(p, opt->rect);
        return;

    case PE_FrameMenu:
    case PE_PanelMenuBar:
        if (rule.hasBorder()) {
            rule.drawBorder(p, rule.borderRect(opt->rect));
            return;
        }
        break;

        case PE_IndicatorToolBarSeparator:
    case PE_IndicatorToolBarHandle: {
        PseudoElement ps = pe == PE_IndicatorToolBarHandle ? PseudoElement_ToolBarHandle : PseudoElement_ToolBarSeparator;
        QRenderRule subRule = renderRule(w, opt, ps);
                if (subRule.hasDrawable()) {
                        subRule.drawRule(p, opt->rect);
                        return;
                }
                                    }
        break;

    // Menu stuff that would be nice to customize
    case PE_IndicatorMenuCheckMark:
    case PE_IndicatorArrowLeft:
    case PE_IndicatorArrowRight:
        baseStyle()->drawPrimitive(pe, opt, p, w);
        return;

    case PE_PanelTipLabel:
        if (!rule.hasDrawable())
            break;

        if (const QStyleOptionFrame *frmOpt = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            if (rule.hasNativeBorder()) {
                rule.drawBackground(p, opt->rect);
                QStyleOptionFrame optCopy(*frmOpt);
                optCopy.rect = rule.borderRect(opt->rect);
                optCopy.palette.setBrush(QPalette::Window, Qt::NoBrush); // oh dear
                baseStyle()->drawPrimitive(pe, &optCopy, p, w);
            } else {
                rule.drawRule(p, opt->rect);
            }
        }
        return;

    case PE_FrameGroupBox:
        if (rule.nativeBorder())
            break;
        rule.drawBorder(p, rule.borderRect(opt->rect));
        return;

    default:
        break;
    }

    if (pseudoElement != PseudoElement_None) {
        QRenderRule subRule = renderRule(w, opt, pseudoElement);
        if (subRule.hasDrawable()) {
            subRule.drawRule(p, opt->rect);
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
    switch (cc) {
    case CC_ScrollBar:
        if (hasStyleRule(w)) {
            QRenderRule rule = renderRule(w, opt);
            if (!rule.hasBorder() && !rule.hasBox() && !rule.hasDrawable())
                break;
        }
        // intentionally falls through
    case CC_GroupBox:
    case CC_ComboBox:
    case CC_Slider:
    case CC_ToolButton:
        return ParentStyle::hitTestComplexControl(cc, opt, pt, w);
    default:
        break;
    }

    return baseStyle()->hitTestComplexControl(cc, opt, pt, w);
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
#ifndef QT_NO_TOOLBUTTON
        // QToolButton adds this directly to the width
        if (qobject_cast<const QToolButton *>(w) && (rule.hasBox() || !rule.hasNativeBorder()))
            return 0;
#endif
        subRule = renderRule(w, opt, PseudoElement_MenuIndicator);
        if (subRule.hasContentsSize())
            return subRule.size().width();
        break;

    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
    case PM_ButtonMargin:
    case PM_DefaultFrameWidth:
    case PM_ButtonDefaultIndicator:
        break;

    case PM_ExclusiveIndicatorWidth:
    case PM_IndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
    case PM_IndicatorHeight:
        subRule = renderRule(w, PseudoElement_Indicator);
        if (subRule.hasContentsSize()) {
            return (m == PM_ExclusiveIndicatorWidth) || (m == PM_IndicatorWidth)
                        ? subRule.size().width() : subRule.size().height();
        }
        break;

    case PM_ToolTipLabelFrameWidth: // border + margin + padding (support only one width)
        if (!rule.hasDrawable())
            break;

        return (rule.border() ? rule.border()->borders[LeftEdge] : 0)
                + (rule.hasBox() ? rule.box()->margins[LeftEdge] + rule.box()->paddings[LeftEdge]: 0);

    case PM_ToolBarFrameWidth:
        if (rule.hasBorder() || rule.hasBox())
            return (rule.border() ? rule.border()->borders[LeftEdge] : 0)
                   + (rule.hasBox() ? rule.box()->paddings[LeftEdge]: 0);
        break;

    case PM_MenuPanelWidth:
    case PM_MenuBarPanelWidth:
        if (rule.hasBorder() || rule.hasBox())
            return (rule.border() ? rule.border()->borders[LeftEdge] : 0)
                   + (rule.hasBox() ? rule.box()->margins[LeftEdge]: 0);
        break;


    case PM_MenuHMargin:
    case PM_MenuBarHMargin:
        if (rule.hasBox())
            return rule.box()->paddings[LeftEdge];
        break;

    case PM_ToolBarItemMargin:
        if (rule.hasBox())
            return rule.box()->margins[TopEdge];
        break;

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

    case PM_MenuTearoffHeight:
    case PM_MenuScrollerHeight:
        break;
    case PM_ToolBarExtensionExtent:
        break;

    case PM_ToolBarSeparatorExtent:
    case PM_ToolBarHandleExtent: {
        PseudoElement ps = m == PM_ToolBarHandleExtent ? PseudoElement_ToolBarHandle : PseudoElement_ToolBarSeparator;
        subRule = renderRule(w, opt, ps);
                if (subRule.hasContentsSize()) {
                        QSize sz = subRule.size();
                        return (opt && opt->state & QStyle::State_Horizontal) ? sz.width() : sz.height();
                }
                                 }
                break;

    case PM_SplitterWidth:
        if (rule.hasContentsSize())
            return rule.size().width();
        break;

    case PM_RadioButtonLabelSpacing:
        if (rule.hasBox() && rule.box()->spacing != -1)
            return rule.box()->spacing;

    case PM_CheckBoxLabelSpacing:
        if (qobject_cast<const QCheckBox *>(w)) {
            if (rule.hasBox() && rule.box()->spacing != -1)
                return rule.box()->spacing;
        }
        // assume group box
        subRule = renderRule(w, PseudoElement_GroupBoxTitle);
        if (subRule.hasBox() && subRule.box()->spacing != -1)
            return subRule.box()->spacing;
        break;

    case PM_ScrollBarExtent:
        if (rule.hasContentsSize()) {
            QSize sz = rule.size();
            if (const QStyleOptionSlider *sb = qstyleoption_cast<const QStyleOptionSlider *>(opt))
                return sb->orientation == Qt::Horizontal ? sz.height() : sz.width();
        }
        break;

    case PM_ScrollBarSliderMin:
        if (hasStyleRule(w, PseudoElement_ScrollBarSlider)) {
            subRule = renderRule(w, PseudoElement_ScrollBarSlider);
            QSize msz = subRule.minimumSize();
            if (const QStyleOptionSlider *sb = qstyleoption_cast<const QStyleOptionSlider *>(opt))
                return sb->orientation == Qt::Horizontal ? msz.width() : msz.height();
        }
        break;

    default:
        break;
    }

    return baseStyle()->pixelMetric(m, opt, w);
}

QSize QStyleSheetStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt,
                                         const QSize &csz, const QWidget *w) const
{
    if (!hasStyleRule(w)) {
#ifndef QT_NO_SPINBOX
        // don't touch the size of a embedded lineedit in a styled spin box
        if (ct == CT_LineEdit && w && qobject_cast<QAbstractSpinBox *>(w->parentWidget())
            && hasStyleRule(w->parentWidget())) {
            QRenderRule rule = renderRule(w->parentWidget(), opt);
            if (rule.hasBox() || rule.hasBorder())
                return csz;
        }
#endif
        return baseStyle()->sizeFromContents(ct, opt, csz, w);
    }

    QRenderRule rule = renderRule(w, opt);
    QSize sz = csz.expandedTo(rule.minimumContentsSize());

    switch (ct) {
    case CT_ToolButton:
    case CT_SpinBox:
    case CT_ComboBox:
    case CT_PushButton:
        if (rule.hasBox() || !rule.nativeBorder())
            return rule.boxSize(sz);
        sz = rule.baseStyleCanDraw() ? baseStyle()->sizeFromContents(ct, opt, sz, w)
                                     : QWindowsStyle::sizeFromContents(ct, opt, sz, w);
        return rule.boxSize(sz, Margin);

    case CT_GroupBox:
    case CT_LineEdit:
        if (rule.hasBox() || !rule.hasNativeBorder()) {
            return rule.boxSize(sz);
        }
        break;

    case CT_CheckBox:
    case CT_RadioButton:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            if (rule.hasBox() || rule.hasBorder() || hasStyleRule(w, PseudoElement_Indicator)) {
                bool isRadio = (ct == CT_RadioButton);
                int iw = pixelMetric(isRadio ? PM_ExclusiveIndicatorWidth
                                             : PM_IndicatorWidth, btn, w);
                int ih = pixelMetric(isRadio ? PM_ExclusiveIndicatorHeight
                                             : PM_IndicatorHeight, btn, w);

                int spacing = pixelMetric(isRadio ? PM_RadioButtonLabelSpacing
                                                  : PM_CheckBoxLabelSpacing, btn, w);
                sz.setWidth(sz.width() + iw + spacing);
                sz.setHeight(qMax(sz.height(), ih));
                return rule.boxSize(sz);
            }
        }
        break;

    case CT_Menu:
    case CT_MenuBar: // already has everything!
    case CT_ScrollBar:
        if (rule.hasBox() || rule.hasBorder())
            return sz;
        break;

    case CT_MenuItem:
    case CT_MenuBarItem: {
        QRenderRule subRule = renderRule(w, opt, PseudoElement_Item);
        if (subRule.hasBox() || subRule.hasBorder())
            return subRule.boxSize(sz);
        break;
                        }

    case CT_SizeGrip:
        return (rule.hasContentsSize())
            ? rule.size()
            : rule.boxSize(baseStyle()->sizeFromContents(ct, opt, sz, w));
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

/*!
    \internal
*/
QIcon QStyleSheetStyle::standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt,
                                                   const QWidget *widget) const
{
    return QCommonStyle::standardIconImplementation(standardIcon, opt, widget);
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

int QStyleSheetStyle::layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
                          Qt::Orientation orientation, const QStyleOption *option,
                          const QWidget *widget) const
{
    return baseStyle()->layoutSpacing(control1, control2, orientation, option, widget);
}

int QStyleSheetStyle::layoutSpacingImplementation(QSizePolicy::ControlType  control1 ,
                                        QSizePolicy::ControlType  control2,
                                        Qt::Orientation orientation, 
                                        const QStyleOption *  option ,
                                        const QWidget *  widget) const
{
    return baseStyle()->layoutSpacing(control1, control2, orientation, option, widget);
}

int QStyleSheetStyle::styleHint(StyleHint sh, const QStyleOption *opt, const QWidget *widget,
                           QStyleHintReturn *shret) const
{
    const QWidget *w = widget;
    if (sh == SH_ToolBox_SelectedPageTitleBold)
       w = widget->parentWidget();

    if (!hasStyleRule(w))
        return baseStyle()->styleHint(sh, opt, widget, shret);

    QRenderRule rule = renderRule(w, opt);
    QString s;
    switch (sh) {
        case SH_LineEdit_PasswordCharacter: s = QLatin1String("lineedit-password-character"); break;
        case SH_DitherDisabledText: s = QLatin1String("dither-disabled-text"); break;
        case SH_EtchDisabledText: s = QLatin1String("etch-disabled-text"); break;
        case SH_ItemView_ActivateItemOnSingleClick: s = QLatin1String("activate-on-singleclick"); break;
        case SH_ItemView_ShowDecorationSelected: s = QLatin1String("show-decoration-selected"); break;
        case SH_Table_GridLineColor: s = QLatin1String("gridline-color"); break;
        case SH_DialogButtonLayout: s = QLatin1String("button-layout"); break;
        case SH_ToolTipLabel_Opacity: s = QLatin1String("opacity"); break;
        case SH_ComboBox_Popup: s = QLatin1String("combobox-popup"); break;
        case SH_ComboBox_ListMouseTracking: s = QLatin1String("combobox-list-mousetracking"); break;
        case SH_MenuBar_AltKeyNavigation: s = QLatin1String("menubar-altkey-navigation"); break;
        case SH_DrawMenuBarSeparator: s = QLatin1String("menubar-separator"); break;
        case SH_MenuBar_MouseTracking: s = QLatin1String("mouse-tracking"); break;
        case SH_SpinBox_ClickAutoRepeatRate: s = QLatin1String("spinbox-click-autorepeat-rate"); break;
        case SH_SpinControls_DisableOnBounds: s = QLatin1String("spincontrol-disable-on-bounds"); break;
        case SH_MessageBox_TextInteractionFlags: s = QLatin1String("messagebox-text-interaction-flags"); break;
        case SH_ToolButton_PopupDelay: s = QLatin1String("toolbutton-popup-delay"); break;
        case SH_ToolBox_SelectedPageTitleBold: s= QLatin1String("toolbox-selected-page-title-bold"); break;
        case SH_GroupBox_TextLabelColor:
            if (rule.hasPalette() && rule.palette()->foreground.isValid())
                return rule.palette()->foreground.rgba();
            break;
        case SH_ScrollView_FrameOnlyAroundContents: s = QLatin1String("scrollview-frame-around-contents"); break;
        case SH_ScrollBar_ContextMenu: s = QLatin1String("scrollbar-contextmenu"); break;
        case SH_ScrollBar_LeftClickAbsolutePosition: s = QLatin1String("scrollbar-leftclick-absolute-position"); break;
        case SH_ScrollBar_MiddleClickAbsolutePosition: s = QLatin1String("scrollbar-middleclick-absolute-position"); break;
        case SH_ScrollBar_RollBetweenButtons: s = QLatin1String("scrollbar-roll-between-buttons"); break;
        case SH_ScrollBar_ScrollWhenPointerLeavesControl: s = QLatin1String("scrollbar-scroll-when-pointer-leaves-control"); break;
        default: break;
    }
    if (!s.isEmpty() && rule.hasStyleHint(s)) {
        return rule.styleHint(s);
    }

    return baseStyle()->styleHint(sh, opt, w, shret);
}

QRect QStyleSheetStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                              const QWidget *w) const
{
    if (!hasStyleRule(w))
        return baseStyle()->subControlRect(cc, opt, sc, w);

    QRenderRule rule = renderRule(w, opt);
    switch (cc) {
   case CC_ComboBox:
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            if (rule.hasBox() || !rule.hasNativeBorder()) {
                switch (sc) {
                case SC_ComboBoxFrame: return rule.borderRect(opt->rect);
                case SC_ComboBoxEditField: return rule.contentsRect(opt->rect);
                case SC_ComboBoxArrow: {
                    QRenderRule subRule = renderRule(w, opt, PseudoElement_ComboBoxDropDown);
                    return positionRect(w, rule, subRule, PseudoElement_ComboBoxDropDown, opt->rect, opt->direction);
                                                                           }
                case SC_ComboBoxListBoxPopup:
                default:
                    return baseStyle()->subControlRect(cc, opt, sc, w);
                }
            }

            QStyleOptionComboBox comboBox(*cb);
            comboBox.rect = rule.borderRect(opt->rect);
            return rule.baseStyleCanDraw() ? baseStyle()->subControlRect(cc, &comboBox, sc, w)
                                           : QWindowsStyle::subControlRect(cc, &comboBox, sc, w);
        }
        break;

    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spin = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            if (rule.hasBox() || !rule.hasNativeBorder()) {
                switch (sc) {
                case SC_SpinBoxFrame: return rule.borderRect(opt->rect);
                case SC_SpinBoxEditField: return rule.contentsRect(opt->rect);
                case SC_SpinBoxDown:
                case SC_SpinBoxUp: {
                    int pe = (sc == SC_SpinBoxUp ? PseudoElement_SpinBoxUpButton : PseudoElement_SpinBoxDownButton);
                    QRenderRule subRule = renderRule(w, opt, pe);
                    return positionRect(w, rule, subRule, pe, opt->rect, opt->direction);
                                                           }
                    default:
                        return baseStyle()->subControlRect(cc, opt, sc, w);
                }
            }

            QStyleOptionSpinBox spinBox(*spin);
            spinBox.rect = rule.borderRect(opt->rect);
            return rule.baseStyleCanDraw() ? baseStyle()->subControlRect(cc, &spinBox, sc, w)
                                           : QWindowsStyle::subControlRect(cc, &spinBox, sc, w);
        }
        break;

    case CC_GroupBox:
        if (const QStyleOptionGroupBox *gb = qstyleoption_cast<const QStyleOptionGroupBox *>(opt)) {
            switch (sc) {
            case SC_GroupBoxFrame:
            case SC_GroupBoxContents: {
                if (rule.hasBox() || !rule.hasNativeBorder()) {
                    return sc == SC_GroupBoxFrame ? rule.borderRect(opt->rect)
                                                  : rule.contentsRect(opt->rect);
                }
                QStyleOptionGroupBox groupBox(*gb);
                groupBox.rect = rule.borderRect(opt->rect);
                return baseStyle()->subControlRect(cc, &groupBox, sc, w);
            }
            default:
            case SC_GroupBoxLabel:
            case SC_GroupBoxCheckBox: {
                QRenderRule indRule = renderRule(w, opt, PseudoElement_GroupBoxIndicator);
                QRenderRule labelRule = renderRule(w, opt, PseudoElement_GroupBoxTitle);
                if (!labelRule.hasPosition() && !labelRule.hasGeometry() && !labelRule.hasBox()
                    && !labelRule.hasBorder() && !indRule.hasContentsSize()) {
                    QStyleOptionGroupBox groupBox(*gb);
                    groupBox.rect = rule.borderRect(opt->rect);
                    return baseStyle()->subControlRect(cc, &groupBox, sc, w);
                }
                int tw = opt->fontMetrics.width(gb->text);
                int th = opt->fontMetrics.height();
                int spacing = pixelMetric(QStyle::PM_CheckBoxLabelSpacing, opt, w);
                int iw = pixelMetric(QStyle::PM_IndicatorWidth, opt, w);
                int ih = pixelMetric(QStyle::PM_IndicatorHeight, opt, w);

                if (gb->subControls & QStyle::SC_GroupBoxCheckBox) {
                    tw = tw + iw + spacing;
                    th = qMax(th, ih);
                }
                if (!labelRule.hasGeometry()) {
                    labelRule.geo = new QStyleSheetGeometryData(tw, th, tw, th);
                } else {
                    labelRule.geo->width = tw;
                    labelRule.geo->height = th;
                }
                if (!labelRule.hasPosition()) {
                    labelRule.p = new QStyleSheetPositionData(0, 0, 0, 0, defaultOrigin(PseudoElement_GroupBoxTitle),
                                                              gb->textAlignment, PositionMode_Static);
                }
                QRect r = positionRect(w, rule, labelRule, PseudoElement_GroupBoxTitle,
                                      opt->rect, opt->direction);
                if (gb->subControls & SC_GroupBoxCheckBox) {
                    r = labelRule.contentsRect(r);
                    if (sc == SC_GroupBoxLabel) {
                        r.setLeft(r.left() + iw + spacing);
                        r.setTop(r.center().y() - th/2);
                    } else {
                        r = QRect(r.left(), r.center().y() - ih/2, iw, ih);
                    }
                    return r;
                } else {
                    return labelRule.contentsRect(r);
                }
            }
            } // switch
        }
        break;

    case CC_ToolButton:
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            if (rule.hasBox() || !rule.hasNativeBorder()) {
                switch (sc) {
                case SC_ToolButton: return rule.borderRect(opt->rect);
                case SC_ToolButtonMenu: {
                    QRenderRule subRule = renderRule(w, opt, PseudoElement_ToolButtonMenu);
                    return positionRect(w, rule, subRule, PseudoElement_ToolButtonMenu, opt->rect, opt->direction);
                                                                            }
                default:
                    break;
                }
            }

            QStyleOptionToolButton tool(*tb);
            tool.rect = rule.borderRect(opt->rect);
            return rule.baseStyleCanDraw() ? baseStyle()->subControlRect(cc, &tool, sc, w)
                                           : QWindowsStyle::subControlRect(cc, &tool, sc, w);
            }
            break;

    case CC_ScrollBar:
        if (const QStyleOptionSlider *sb = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            if (rule.hasDrawable() || rule.hasBox()) {
                QRenderRule subRule;
                PseudoElement pe = PseudoElement_None;
                switch (sc) {
                case SC_ScrollBarGroove:
                    return rule.contentsRect(opt->rect);
                case SC_ScrollBarAddPage:
                case SC_ScrollBarSubPage:
                case SC_ScrollBarSlider: {
                    QRect cr = rule.contentsRect(opt->rect);
                    int maxlen = (sb->orientation == Qt::Horizontal) ? cr.width() : cr.height();
                    int sliderlen;

                    if (sb->maximum != sb->minimum) {
                        uint range = sb->maximum - sb->minimum;
                        sliderlen = (qint64(sb->pageStep) * maxlen) / (range + sb->pageStep);

                        int slidermin = pixelMetric(PM_ScrollBarSliderMin, sb, w);
                        if (sliderlen < slidermin || range > INT_MAX / 2)
                            sliderlen = slidermin;
                        if (sliderlen > maxlen)
                            sliderlen = maxlen;
                    } else {
                        sliderlen = maxlen;
                    }

                    int sliderstart = (sb->orientation == Qt::Horizontal ? cr.left() : cr.top())
                        + sliderPositionFromValue(sb->minimum, sb->maximum, sb->sliderPosition,
                                                  maxlen - sliderlen, sb->upsideDown);

                    QRect sr = (sb->orientation == Qt::Horizontal)
                               ? QRect(sliderstart, cr.top(), sliderlen, cr.height())
                               : QRect(cr.left(), sliderstart, cr.width(), sliderlen);
                    if (sc == SC_ScrollBarSlider) {
                        return sr;
                    } else if (sc == SC_ScrollBarSubPage) {
                        return QRect(cr.topLeft(), sb->orientation == Qt::Horizontal ? sr.bottomLeft() : sr.topRight());
                    } else { // SC_ScrollBarAddPage
                        return QRect(sb->orientation == Qt::Horizontal ? sr.topRight() : sr.bottomLeft(), cr.bottomRight());
                    }
                    break;
                }
                case SC_ScrollBarAddLine: pe = PseudoElement_ScrollBarAddLine; break;
                case SC_ScrollBarSubLine: pe = PseudoElement_ScrollBarSubLine; break;
                case SC_ScrollBarFirst: pe = PseudoElement_ScrollBarFirst; break;
                case SC_ScrollBarLast: pe = PseudoElement_ScrollBarLast; break;
                default: break;
                }
                if (!hasStyleRule(w, pe))
                    return QRect();
                subRule = renderRule(w, opt, pe);
                return positionRect(w, rule, subRule, pe, opt->rect, opt->direction);
            }
        }
        break;
    default:
        break;
    }

    return baseStyle()->subControlRect(cc, opt, sc, w);
}

QRect QStyleSheetStyle::subElementRect(SubElement se, const QStyleOption *opt, const QWidget *w) const
{
    if ((se == SE_ToolBoxTabContents && !hasStyleRule(w->parentWidget())) || !hasStyleRule(w))
        return baseStyle()->subElementRect(se, opt, w);

    QRenderRule rule = renderRule(w, opt);
    QRenderRule subRule;

    switch (se) {
    case SE_PushButtonContents:
    case SE_PushButtonFocusRect:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QStyleOptionButton btnOpt(*btn);
            if (rule.hasBox() || !rule.nativeBorder())
                return visualRect(opt->direction, opt->rect, rule.contentsRect(opt->rect));
            return rule.baseStyleCanDraw() ? baseStyle()->subElementRect(se, &btnOpt, w)
                                           : QWindowsStyle::subElementRect(se, &btnOpt, w);
        }
        break;

    case SE_LineEditContents:
    case SE_FrameContents:
        if (rule.hasBox() || !rule.nativeBorder()) {
            return visualRect(opt->direction, opt->rect, rule.contentsRect(opt->rect));
        }
        break;

    case SE_CheckBoxIndicator:
    case SE_RadioButtonIndicator:
        if (rule.hasBox() || rule.hasBorder() || hasStyleRule(w, PseudoElement_Indicator)) {
            PseudoElement pe = se == SE_CheckBoxIndicator ? PseudoElement_Indicator : PseudoElement_ExclusiveIndicator;
            QRenderRule subRule = renderRule(w, opt, pe);
            return positionRect(w, rule, subRule, pe, opt->rect, opt->direction);
        }
        break;

    case SE_CheckBoxContents:
    case SE_RadioButtonContents:
        if (rule.hasBox() || rule.hasBorder() || hasStyleRule(w, PseudoElement_Indicator)) {
            bool isRadio = se == SE_RadioButtonContents;
            QRect ir = subElementRect(isRadio ? SE_RadioButtonIndicator : SE_CheckBoxIndicator,
                                      opt, w);
            ir = visualRect(opt->direction, opt->rect, ir);
            int spacing = pixelMetric(isRadio ? PM_RadioButtonLabelSpacing : PM_CheckBoxLabelSpacing, 0, w);
            QRect cr = rule.contentsRect(opt->rect);
            ir.setRect(ir.left() + ir.width() + spacing, cr.y(),
                       cr.width() - ir.width() - spacing, cr.height());
            return visualRect(opt->direction, opt->rect, ir);
        }
        break;

    case SE_ToolBoxTabContents:
        if (hasStyleRule(w->parentWidget(), PseudoElement_ToolBoxTab)) {
            QRenderRule subRule = renderRule(w->parentWidget(), opt, PseudoElement_ToolBoxTab);
            return visualRect(opt->direction, opt->rect, subRule.contentsRect(opt->rect));
        }
        break;

    case SE_RadioButtonFocusRect:
    case SE_RadioButtonClickRect: // focusrect | indicator
    case SE_CheckBoxFocusRect:
    case SE_CheckBoxClickRect: // relies on indicator and contents
        return ParentStyle::subElementRect(se, opt, w);

    case SE_ViewItemCheckIndicator:
        if (hasStyleRule(w, PseudoElement_Indicator)) {
            QRenderRule subRule = renderRule(w, opt, PseudoElement_Indicator);
            if (subRule.hasContentsSize()) {
                return alignedRect(opt->direction, Qt::AlignLeft|Qt::AlignCenter, subRule.contentsSize(), opt->rect);
            }
        }
        break;

    default:
        break;
    }

    return baseStyle()->subElementRect(se, opt, w);
}

#include "moc_qstylesheetstyle_p.cpp"

#endif // QT_NO_STYLE_STYLESHEET
