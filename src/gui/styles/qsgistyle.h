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

#ifndef QSGISTYLE_H
#define QSGISTYLE_H

#ifndef QT_H
#include "qmotifstyle.h"
#include "qpointer.h"
#include "qwidget.h"
#endif // QT_H

#if !defined(QT_NO_STYLE_SGI) || defined(QT_PLUGIN)

#if defined(QT_PLUGIN)
#define Q_GUI_EXPORT_STYLE_SGI
#else
#define Q_GUI_EXPORT_STYLE_SGI Q_GUI_EXPORT
#endif

class QSGIStylePrivate;

class Q_GUI_EXPORT_STYLE_SGI QSGIStyle: public QMotifStyle
{
    Q_OBJECT
public:
    QSGIStyle(bool useHighlightCols = false);
    virtual ~QSGIStyle();

#if !defined(Q_NO_USING_KEYWORD)
    using QMotifStyle::polish;
#endif
    void polish(QWidget*);
    void unPolish(QWidget*);
    void polish(QApplication*);
    void unPolish(QApplication*);

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

    int pixelMetric(PixelMetric metric, const QWidget *widget = 0) const;

    QSize sizeFromContents(ContentsType contents,
                            const QWidget *widget,
                            const QSize &contentsSize
                            /*const Q3StyleOption& = Q3StyleOption::Default*/) const;

    QRect subRect(SubRect r, const QWidget *widget) const;
    QRect querySubControlMetrics(ComplexControl control,
                                  const QWidget *widget,
                                  SubControl sc
                                  /*const Q3StyleOption& = Q3StyleOption::Default*/) const;

protected:
    bool eventFilter(QObject*, QEvent*);

private:
    QSGIStylePrivate *d;

    uint isApplicationStyle :1;
#if defined(Q_DISABLE_COPY)
    QSGIStyle(const QSGIStyle &);
    QSGIStyle& operator=(const QSGIStyle &);
#endif

};

#endif // QT_NO_STYLE_SGI

#endif // QSGISTYLE_H
