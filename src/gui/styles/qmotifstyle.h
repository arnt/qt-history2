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

#include "qcommonstyle.h"

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

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                        const QWidget *w = 0) const;

    void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                      const QWidget *w = 0) const;

    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            const QWidget *w = 0) const;

    QRect querySubControlMetrics(ComplexControl cc, const QStyleOptionComplex *opt,
                                 SubControl sc, const QWidget *widget = 0) const;

    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0,
                     const QWidget *widget = 0) const;

    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt,
                           const QSize &contentsSize, const QWidget *widget = 0) const;

    QRect subRect(SubRect r, const QStyleOption *opt, const QWidget *widget = 0) const;

    QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                           const QWidget *widget = 0) const;

    int styleHint(StyleHint hint, const QStyleOption *opt = 0, const QWidget *widget = 0,
                  QStyleHintReturn *returnData = 0) const;

private:
    Q_DISABLE_COPY(QMotifStyle)

    bool highlightCols;
};

#endif // QT_NO_STYLE_MOTIF

#endif // QMOTIFSTYLE_H
