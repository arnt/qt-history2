/****************************************************************************
**
** Definition of QCommonStyle class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCOMMONSTYLE_H
#define QCOMMONSTYLE_H

#ifndef QT_H
#include "qstyle.h"
#endif // QT_H

#ifndef QT_NO_STYLE

class Q_GUI_EXPORT QCommonStyle: public QStyle
{
    Q_OBJECT

public:
    QCommonStyle();
    ~QCommonStyle();

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                       const QWidget *w = 0) const;
    void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                     const QWidget *w = 0) const;
    void drawControlMask(ControlElement element, const QStyleOption *opt, QPainter *p,
                         const QWidget *w) const;
    QRect subRect(SubRect r, const QStyleOption *opt, const QWidget *widget = 0) const;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            const QWidget *w = 0) const;
    void drawComplexControlMask(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                const QWidget *w = 0) const;
    SubControl querySubControl(ComplexControl cc, const QStyleOptionComplex *opt,
                               const QPoint &pt, const QWidget *w = 0) const;
    QRect querySubControlMetrics(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                                 const QWidget *w) const;
    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize,
                           const QFontMetrics &fm, const QWidget *widget = 0) const;

    int pixelMetric(PixelMetric m, const QWidget *widget = 0) const;

    int styleHint(StyleHint sh, const QWidget *, const Q3StyleOption &, QStyleHintReturn *) const;

    QPixmap stylePixmap(StylePixmap sp,
                         const QWidget *widget = 0,
                         const Q3StyleOption& = Q3StyleOption::Default) const;

    QPixmap stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap,
                         const QPalette &pal, const Q3StyleOption& = Q3StyleOption::Default) const;

private:
    // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QCommonStyle(const QCommonStyle &);
    QCommonStyle &operator=(const QCommonStyle &);
#endif
};



#endif // QT_NO_STYLE

#endif // QCOMMONSTYLE_H
