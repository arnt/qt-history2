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

#include "QtGui/qmotifstyle.h"

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

    explicit QCDEStyle(bool useHighlightCols = false);
    virtual ~QCDEStyle();

    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0,
                    const QWidget *widget = 0) const;

    void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                             const QWidget *w = 0) const;

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                        const QWidget *w = 0) const;

    QPalette standardPalette() const;

};

#endif // QT_NO_STYLE_CDE

#endif // QCDESTYLE_H
