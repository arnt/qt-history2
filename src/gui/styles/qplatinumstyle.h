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

#ifndef QPLATINUMSTYLE_H
#define QPLATINUMSTYLE_H

#include "QtGui/qwindowsstyle.h"

#if !defined(QT_NO_STYLE_PLATINUM) || defined(QT_PLUGIN)

class QPalette;

#if defined(QT_PLUGIN)
#define Q_GUI_EXPORT_STYLE_PLATINUM
#else
#define Q_GUI_EXPORT_STYLE_PLATINUM Q_GUI_EXPORT
#endif

class Q_GUI_EXPORT_STYLE_PLATINUM QPlatinumStyle : public QWindowsStyle
{
    Q_OBJECT
public:
    QPlatinumStyle();
    virtual ~QPlatinumStyle();

     void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                        const QWidget *w = 0) const;

     void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                      const QWidget *w = 0) const;

     void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                             const QWidget *widget = 0) const;

     QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                          SubControl sc, const QWidget *widget = 0) const;

     int pixelMetric(PixelMetric metric, const QStyleOption *option = 0,
                     const QWidget *widget = 0) const;

     QRect subElementRect(SubElement ce, const QStyleOption *opt, const QWidget *widget = 0) const;

protected:
    QColor mixedColor(const QColor &, const QColor &) const;
    void drawRiffles(QPainter* p,  int x, int y, int w, int h,
                      const QPalette &pal, bool horizontal) const;

private:
    Q_DISABLE_COPY(QPlatinumStyle)
};

#endif // QT_NO_STYLE_PLATINUM

#endif // QPLATINUMSTYLE_H
