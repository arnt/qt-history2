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

#ifndef QSTYLESHEETSTYLE_P_H
#define QSTYLESHEETSTYLE_P_H

#include "QtGui/qwindowsstyle.h"

#ifndef QT_NO_STYLE_STYLESHEET

#include "QtGui/qstyleoption.h"
#include "QtCore/qhash.h"
#include "QtGui/qevent.h"
#include "QtCore/qvector.h"
#include "QtCore/qshareddata.h"
#include "QtGui/qapplication.h"
#include "private/qcssparser_p.h"
#include "QtGui/qbrush.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

struct Q_AUTOTEST_EXPORT QStyleSheetBorderImageData : public QSharedData
{
    QStyleSheetBorderImageData()
    {
        for (int i = 0; i < 4; i++) cuts[i] = -1;
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

struct Q_AUTOTEST_EXPORT QStyleSheetBackgroundImageData : public QSharedData
{
    QStyleSheetBackgroundImageData() : position(Qt::AlignTop | Qt::AlignLeft),
                        origin(QCss::Origin_Padding), repeat(QCss::Repeat_XY) { }
    QPixmap pixmap;
    Qt::Alignment position;
    QCss::Origin origin;
    QCss::Repeat repeat;
};

struct Q_AUTOTEST_EXPORT QStyleSheetBorderData : public QSharedData
{
    QStyleSheetBorderData() : bi(0)
    {
        for (int i = 0; i < 4; i++) {
            borders[i] = 0;
            styles[i] = QCss::BorderStyle_None;
            colors[i] = Qt::transparent;
        }
    }

    qreal borders[4];
    QColor colors[4];
    QCss::BorderStyle styles[4];
    QSize radii[4]; // topleft, topright, bottomleft, bottomright

    const QStyleSheetBorderImageData *borderImage() const
    { return bi; }
    QStyleSheetBorderImageData *_borderImage()
    { if (!bi) bi = new QStyleSheetBorderImageData; return bi; }
    bool hasBorderImage() const { return bi!=0; }

    QSharedDataPointer<QStyleSheetBorderImageData> bi;
};

struct Q_AUTOTEST_EXPORT QStyleSheetBoxData : public QSharedData
{
    QStyleSheetBoxData() : spacing(0)
    {
        for (int i = 0; i < 4; i++)
            margins[i] = paddings[i] = 0;
    }

    qreal margins[4];
    qreal paddings[4];

    qreal spacing;

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

struct Q_AUTOTEST_EXPORT QStyleSheetPaletteData : public QSharedData
{
    QBrush foreground;
    QBrush background;
    QBrush selectionForeground;
    QBrush selectionBackground;
    QBrush alternateBackground;
};

struct Q_AUTOTEST_EXPORT QStyleSheetGeometryData : public QSharedData
{
    QStyleSheetGeometryData() : minWidth(-1), minHeight(-1), width(-1), height(-1) { }

    qreal minWidth, minHeight, width, height;
};

struct Q_AUTOTEST_EXPORT QStyleSheetPositionData : public QSharedData
{
    QStyleSheetPositionData() : left(0), right(0), top(0), bottom(0) { }

    qreal left, right, top, bottom;
};

class Q_AUTOTEST_EXPORT QRenderRule
{
public:
    inline QRenderRule() : pal(0), b(0), bg(0), bd(0), geo(0), p(0) { }
    inline ~QRenderRule() { }

    void merge(const QVector<QCss::Declaration>& declarations);
    bool isEmpty() const
    { return pal == 0 && b == 0 && bg == 0 && bd == 0 && geo == 0 && image.isNull(); }

    QRect borderRect(const QRect &r) const;
    QRect paddingRect(const QRect &r) const;
    QRect contentsRect(const QRect &r) const;
    QRect boxRect(const QRect &r) const;
    QSize boxSize(const QSize &s) const;
    QRect positionRect(const QRect &r) const;

    QRectF borderRect(const QRectF& r) const;
    QRectF paddingRect(const QRectF& r) const;
    QRectF contentsRect(const QRectF& r) const;
    QRectF boxRect(const QRectF& r) const;

    const QStyleSheetPaletteData *palette() const { return pal; }
    const QStyleSheetBoxData *box() const { return b; }
    const QStyleSheetBackgroundImageData *backgroundImage() const { return bg; }
    const QStyleSheetBorderData *border() const { return bd; }
    const QStyleSheetGeometryData *geometry() const { return geo; }
    const QStyleSheetPositionData *position() const { return p; }

    QStyleSheetPaletteData *_palette()
    { if (!pal) pal = new QStyleSheetPaletteData(); return pal; }
    QStyleSheetBoxData *_box()
    { if (!b) b = new QStyleSheetBoxData(); return b; }
    QStyleSheetBackgroundImageData *_backgroundImage()
    { if (!bg) bg = new QStyleSheetBackgroundImageData(); return bg; }
    QStyleSheetBorderData *_border()
    { if (!bd) bd = new QStyleSheetBorderData(); return bd; }
    QStyleSheetGeometryData *_geometry()
    { if (!geo) geo = new QStyleSheetGeometryData(); return geo; }
    QStyleSheetPositionData *_position()
    { if (!p) p = new QStyleSheetPositionData(); return p; }

    void cutBorderImage();
    void fixupBorder();

    bool hasPalette() const { return pal != 0; }
    bool hasBackgroundImage() const { return bg != 0; }
    bool hasBox() const { return b != 0; }
    bool hasBorder() const { return bd != 0; }
    bool hasGeometry() const { return geo != 0 || !image.isNull(); }
    bool hasPosition() const { return p != 0; }
    bool hasBackground() const
    { return bg != 0 || (pal != 0 && pal->background.style() != Qt::NoBrush); }
    bool hasFrame() const { return hasBorder() || hasBackground(); }

    QSize minimumContentsSize() const 
    { return geo ? QSizeF(geo->minWidth, geo->minHeight).toSize() : QSize(0, 0); }

    QSize size() const 
    { return boxSize(geo ? QSizeF(geo->width, geo->height).toSize() : image.size()); }

    QPixmap image;

private:
    QSharedDataPointer<QStyleSheetPaletteData> pal;
    QSharedDataPointer<QStyleSheetBoxData> b;
    QSharedDataPointer<QStyleSheetBackgroundImageData> bg;
    QSharedDataPointer<QStyleSheetBorderData> bd;
    QSharedDataPointer<QStyleSheetGeometryData> geo;
    QSharedDataPointer<QStyleSheetPositionData> p;
};

class QFrame;

typedef QHash<QString, QHash<int, QRenderRule> > QRenderRules;

class Q_AUTOTEST_EXPORT QStyleSheetStyle : public QWindowsStyle
{
    typedef QWindowsStyle ParentStyle;

    Q_OBJECT
public:
    QStyleSheetStyle(QStyle *baseStyle);

    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            const QWidget *w = 0) const;
    void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                     const QWidget *w = 0) const;
    void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const;
    void drawIcon(QPainter *p, const QRect &rect, int alignment, const QRenderRules& rules,
                  const QRenderRule& rule, const char *icon) const;
    void drawItemText(QPainter *painter, const QRect& rect, int alignment, const QPalette &pal,
              bool enabled, const QString& text, QPalette::ColorRole textRole  = QPalette::NoRole) const;
    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                       const QWidget *w = 0) const;
    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap,
                                const QStyleOption *option) const;
    SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                     const QPoint &pt, const QWidget *w = 0) const;
    QRect itemPixmapRect(const QRect &rect, int alignment, const QPixmap &pixmap) const;
    QRect itemTextRect(const QFontMetrics &metrics, const QRect &rect, int alignment, bool enabled,
                       const QString &text) const;
    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;
    void polish(QWidget *widget);
    void polish(QApplication *app);
    void polish(QPalette &pal);
    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt,
                           const QSize &contentsSize, const QWidget *widget = 0) const;
    QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *w = 0) const;
    QPalette standardPalette() const;
    QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option = 0,
                           const QWidget *w = 0 ) const;
    int styleHint(StyleHint sh, const QStyleOption *opt = 0, const QWidget *w = 0,
                  QStyleHintReturn *shret = 0) const;
    QRect subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                         const QWidget *w = 0) const;

    // These functions are called from QApplication/QWidget. Be careful.
    void setBaseStyle(QStyle *style) { base = style; }
    QStyle *baseStyle() const;
    void repolish(QWidget *widget);
    void repolish(QApplication *app);

    void unpolish(QWidget *widget);
    void unpolish(QApplication *app);

    QStyle *base;
    void ref() { ++refcount; }
    void deref() { Q_ASSERT(--refcount >= 0); if (!refcount) delete this; }

protected Q_SLOTS:
    QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt = 0,
                                     const QWidget *widget = 0) const;

private Q_SLOTS:
    void widgetDestroyed(QObject *);

private:
    int refcount;

    void update(const QList<const QWidget *>& widgets);

    void setPalette(QWidget *);
    void unsetPalette(QWidget *);
    QVector<QCss::StyleRule> styleRules(QWidget *widget);

    bool hasStyleRule(const QWidget *w) const;
    bool hasStyleRule(const QWidget *w, int part) const;

    QRenderRule renderRule(const QWidget *w, const QStyleOption *opt, int part = 0) const;
    QRenderRule renderRule(const QWidget *w, QStyle::State state, const QString &part = QString()) const;
    QRenderRule renderRule(const QWidget *w, QStyle::State state, int pseudoElement) const;

    static QHash<const QWidget *, QVector<QCss::StyleRule> > styleRulesCache;
    static QHash<const QWidget *, QRenderRules> renderRulesCache;
};

#endif // QT_NO_STYLE_STYLESHEET
#endif // QSTYLESHEETSTYLE_P_H
