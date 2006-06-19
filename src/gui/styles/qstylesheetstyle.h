#ifndef QSTYLESHEETSTYLE_H
#define QSTYLESHEETSTYLE_H

#include <QtGui/QCommonStyle>
#include <QtGui/QStyleOption>
#include <QtCore/QHash>
#include <QtCore/QEvent>
#include <QtCore/QVector>

#include "private/qcssparser_p.h"

class RenderRule;

class QStyleSheetStyle : public QCommonStyle
{
    typedef QCommonStyle ParentStyle;

    Q_OBJECT
public:
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            const QWidget *w = 0) const;
    void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                     const QWidget *w = 0) const;
    void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const;
    void drawItemText(QPainter *painter, const QRect& rect, int alignment, const QPalette &pal, 
              bool enabled, const QString& text, QPalette::ColorRole textRole  = QPalette::NoRole) const;
    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                       const QWidget *w = 0) const;
    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap& pixmap, 
                                const QStyleOption *option) const;
    SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                     const QPoint &pt, const QWidget *w = 0) const;
    QRect itemPixmapRect(const QRect &rect, int alignment, const QPixmap &pixmap) const;
    QRect itemTextRect(const QFontMetrics &metrics, const QRect& rect, int alignment, bool enabled, 
                       const QString& text) const;
    int pixelMetric(PixelMetric metric, const QStyleOption* option = 0, const QWidget *widget = 0) const;
    void polish(QWidget *widget);
    void polish(QApplication *app);
    void polish(QPalette &pal);
    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt,
                           const QSize &contentsSize, const QWidget *widget = 0) const;
    QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget* w = 0) const;
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

    static QStyle *styleSheetStyle();

private:
    QStyleSheetStyle(QStyle *baseStyle);
    QStyle *baseStyle;

    enum WidgetType {
        PushButton, LineEdit, ComboBox, GroupBox, Frame
    };
    bool baseStyleCanRender(WidgetType, RenderRule *) const;
    void setPalette(QWidget *, RenderRule *);
    RenderRule *renderRule(const QWidget *w, const QStyleOption *opt) const;
    mutable QHash<const QWidget *, RenderRule *> cachedRules;
};

#endif // QSTYLESHEETSTYLE_H
