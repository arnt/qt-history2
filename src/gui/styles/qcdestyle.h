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


#ifndef QCDESTYLE_H
#define QCDESTYLE_H

#include "qmotifstyle.h"

#if !defined(QT_NO_STYLE_CDE) || defined(QT_PLUGIN)

#if defined(QT_PLUGIN)
#define Q_GUI_EXPORT_STYLE_CDE
#else
#define Q_GUI_EXPORT_STYLE_CDE Q_GUI_EXPORT
#endif

class Q_GUI_EXPORT_STYLE_CDE QCDEStyle : public QMotifStyle
{
    Q_OBJECT
public:

    QCDEStyle(bool useHighlightCols = false);
    virtual ~QCDEStyle();

    int pixelMetric(PixelMetric metric, const QWidget *widget = 0) const;

    void drawControl(ControlElement element,
                      QPainter *p,
                      const QWidget *widget,
                      const QRect &r,
                      const QPalette &pal,
                      SFlags how = Style_Default
                      /*const Q3StyleOption& = Q3StyleOption::Default*/) const;

    void drawPrimitive(PrimitiveElement pe,
                        QPainter *p,
                        const QRect &r,
                        const QPalette &pal,
                        SFlags flags = Style_Default
                        /*const Q3StyleOption& = Q3StyleOption::Default*/) const;

};

#endif // QT_NO_STYLE_CDE

#endif // QCDESTYLE_H
