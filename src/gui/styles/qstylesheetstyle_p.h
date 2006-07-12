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

#include "QtGui/qcommonstyle.h"
#include "QtGui/qstyleoption.h"
#include "QtCore/qhash.h"
#include "QtGui/qevent.h"
#include "QtCore/qvector.h"
#include "QtCore/qshareddata.h"
#include "QtGui/qapplication.h"
#include "private/qcssparser_p.h"

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
    QPixmap topEdge, bottomEdge, leftEdge, rightEdge, middle;
    QRect topEdgeRect, bottomEdgeRect, leftEdgeRect, rightEdgeRect, middleRect;
    QRect topLeftCorner, topRightCorner, bottomRightCorner, bottomLeftCorner;
    int cuts[4];
    QPixmap pixmap;
    QImage image;
    QCss::TileMode horizStretch, vertStretch;

    void cutBorderImage();
};

struct Q_AUTOTEST_EXPORT QStyleSheetBackgroundData : public QSharedData
{
    QStyleSheetBackgroundData() : position(Qt::AlignTop | Qt::AlignLeft),
                        origin(QCss::Origin_Padding), repeat(QCss::Repeat_XY) { }
    QPixmap pixmap;
    Qt::Alignment position;
    QCss::Origin origin;
    QCss::Repeat repeat;
};

struct Q_AUTOTEST_EXPORT QStyleSheetBoxData : public QSharedData
{
    QStyleSheetBoxData() : bi(0)
    {
        for (int i = 0; i < 4; i++) {
            margins[i] = borders[i] = paddings[i] = 0;
            styles[i] = QCss::BorderStyle_None;
            colors[i] = Qt::transparent;
        }
    }

    qreal margins[4];
    qreal borders[4];
    qreal paddings[4];
    QCss::BorderStyle styles[4];
    QColor colors[4];

    QSize radii[4]; // topleft, topright, bottomleft, bottomright
    QStyleSheetBorderImageData *borderImage() 
    { if (!bi) bi = new QStyleSheetBorderImageData; return bi; }
    bool hasBorderImage() const { return bi!=0; }
    QSharedDataPointer<QStyleSheetBorderImageData> bi;
};

struct Q_AUTOTEST_EXPORT QStyleSheetFocusRectData : public QSharedData
{
    QColor color;
    int width;
    QCss::BorderStyle style;
};

struct Q_AUTOTEST_EXPORT QStyleSheetPalette : public QSharedData
{
    QBrush foreground;
    QBrush background;
    QBrush selectionForeground;
    QBrush selectionBackground;
    QBrush alternateBackground;
};

class Q_AUTOTEST_EXPORT QRenderRule
{
public:
    inline QRenderRule() : pal(0), b(0), fr(0), bg(0) { }
    inline ~QRenderRule() { }

    void merge(const QVector<QCss::Declaration>& declarations);
    bool isEmpty() const { return pal == 0 && b == 0 && fr == 0 && bg == 0; }

    QRect borderRect(const QRect& r) const;
    QRect paddingRect(const QRect& r) const;
    QRect contentsRect(const QRect& r) const;
    QRect boxRect(const QRect& r) const;

    QRectF borderRect(const QRectF& r) const;
    QRectF paddingRect(const QRectF& r) const;
    QRectF contentsRect(const QRectF& r) const;
    QRectF boxRect(const QRectF& r) const;

    inline QStyleSheetPalette *palette() const 
    { if (!pal) pal = new QStyleSheetPalette(); return pal; }
    inline QStyleSheetBoxData *box() const 
    { if (!b) b = new QStyleSheetBoxData(); return b; }
    inline QStyleSheetBackgroundData *background() const 
    { if (!bg) bg = new QStyleSheetBackgroundData(); return bg; }
    inline QStyleSheetFocusRectData *focusRect() const 
    { if (!fr) fr = new QStyleSheetFocusRectData(); return fr; }

    void cutBorderImage();
    void fixup();

    inline bool hasPalette() const { return pal != 0; }
    inline bool hasBackground() const { return bg != 0; }
    inline bool hasBox() const { return b != 0; }
    inline bool hasFocusRect() const { return fr != 0; }

    QHash<QString, QPixmap> pixmaps;

private:
    mutable QSharedDataPointer<QStyleSheetPalette> pal;
    mutable QSharedDataPointer<QStyleSheetBoxData> b;
    mutable QSharedDataPointer<QStyleSheetFocusRectData> fr;
    mutable QSharedDataPointer<QStyleSheetBackgroundData> bg;
};

class QFrame;

class QStyleSheetStyle : public QCommonStyle
{
    typedef QCommonStyle ParentStyle;

    Q_OBJECT
public:
    QStyleSheetStyle(QStyle *baseStyle, QObject *parent);

    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            const QWidget *w = 0) const;
    void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                     const QWidget *w = 0) const;
    void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const;
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
    void unpolish(QWidget *widget);
    void unpolish(QApplication *app);

    void setBaseStyle(QStyle *base) { bs = base; }
    QStyle *baseStyle() const { return bs ? bs : qApp->style(); }

    void repolish(QWidget *widget);
    void repolish(QApplication *app);

protected Q_SLOTS:
    QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt = 0,
                                     const QWidget *widget = 0) const;

private:
    QStyle *bs;

    enum WidgetType {
        PushButton, LineEdit, ComboBox, GroupBox, Frame
    };
    bool baseStyleCanRender(WidgetType, const QRenderRule&) const;
    void update(const QList<QWidget *>& widgets);

    void setPalette(QWidget *);
    void unsetPalette(QWidget *);
    QVector<QCss::StyleRule> computeStyleSheet(QWidget *widget);

    QRenderRule renderRule(const QWidget *w, const QStyleOption *opt) const;
    QRenderRule renderRule(QWidget *w, QStyle::State state) const;

    static QHash<QWidget *, QVector<QCss::StyleRule> > styleRulesCache;
    static QHash<QWidget *, QHash<int, QRenderRule> > renderRulesCache;
};

#endif // QSTYLESHEETSTYLE_P_H
