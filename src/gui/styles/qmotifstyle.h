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

#ifndef QMOTIFSTYLE_H
#define QMOTIFSTYLE_H

#ifndef QT_H
#include "qcommonstyle.h"
#endif // QT_H

#if !defined(QT_NO_STYLE_MOTIF) || defined(QT_PLUGIN)

class QPalette;

#if defined(QT_PLUGIN)
#define Q_GUI_EXPORT_STYLE_MOTIF
#else
#define Q_GUI_EXPORT_STYLE_MOTIF Q_GUI_EXPORT
#endif


class Q_GUI_EXPORT_STYLE_MOTIF QMotifStyle : public QCommonStyle
{
    Q_OBJECT
public:
    QMotifStyle(bool useHighlightCols=false);
    virtual ~QMotifStyle();

    void setUseHighlightColors(bool);
    bool useHighlightColors() const;

    void polish(QPalette&);
    void polish(QWidget*);
    void polish(QApplication*);

    // new style API
    void drawPrimitive(PrimitiveElement pe,
                        QPainter *p,
                        const QRect &r,
                        const QPalette &pal,
                        SFlags flags = Style_Default
                        /*const Q3StyleOption& = Q3StyleOption::Default*/) const;

    void drawControl(ControlElement element,
                      QPainter *p,
                      const QWidget *widget,
                      const QRect &r,
                      const QPalette &pal,
                      SFlags how = Style_Default
                      /*const Q3StyleOption& = Q3StyleOption::Default*/) const;

    void drawComplexControl(ComplexControl control,
                             QPainter *p,
                             const QWidget* widget,
                             const QRect& r,
                             const QPalette& pal,
                             SFlags how = Style_Default,
                             SCFlags sub = SC_All,
                             SCFlags subActive = SC_None
                             /*const Q3StyleOption& = Q3StyleOption::Default*/) const;

    QRect querySubControlMetrics(ComplexControl control,
                                  const QWidget *widget,
                                  SubControl sc
                                  /*const Q3StyleOption& = Q3StyleOption::Default*/) const;

    int pixelMetric(PixelMetric metric, const QWidget *widget = 0) const;

    QSize sizeFromContents(ContentsType contents,
                            const QWidget *widget,
                            const QSize &contentsSize
                            /*const Q3StyleOption& = Q3StyleOption::Default*/) const;

    QRect subRect(SubRect r, const QWidget *widget) const;

    QPixmap stylePixmap(StylePixmap, const QWidget * = 0/*, const Q3StyleOption& = Q3StyleOption::Default*/) const;

    int styleHint(StyleHint sh, const QWidget * /*, const Q3StyleOption & = Q3StyleOption::Default,
                  QStyleHintReturn* = 0*/) const;

private:
    bool highlightCols;

    // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMotifStyle(const QMotifStyle &);
    QMotifStyle& operator=(const QMotifStyle &);
#endif
};

#endif // QT_NO_STYLE_MOTIF

#endif // QMOTIFSTYLE_H
