/****************************************************************************
**
** Definition of Platinum-like style class.
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

#ifndef QPLATINUMSTYLE_H
#define QPLATINUMSTYLE_H

#ifndef QT_H
#include "qwindowsstyle.h"
#endif // QT_H

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

    // new Style Stuff
    void drawPrimitive(PrimitiveElement pe,
                        QPainter *p,
                        const QRect &r,
                        const QPalette &pal,
                        SFlags flags = Style_Default,
                        const Q3StyleOption& = Q3StyleOption::Default) const;

    void drawControl(ControlElement element,
                      QPainter *p,
                      const QWidget *widget,
                      const QRect &r,
                      const QPalette &pal,
                      SFlags how = Style_Default,
                      const Q3StyleOption& = Q3StyleOption::Default) const;

    void drawComplexControl(ComplexControl control,
                             QPainter *p,
                             const QWidget *widget,
                             const QRect &r,
                             const QPalette &pal,
                             SFlags how = Style_Default,
                             SCFlags sub = SC_All,
                             SCFlags subActive = SC_None,
                             const Q3StyleOption& = Q3StyleOption::Default) const;

    QRect querySubControlMetrics(ComplexControl control,
                                  const QWidget *widget,
                                  SubControl sc,
                                  const Q3StyleOption& = Q3StyleOption::Default) const;

    int pixelMetric(PixelMetric metric, const QWidget *widget = 0) const;

    QRect subRect(SubRect r, const QWidget *widget) const;

protected:
    QColor mixedColor(const QColor &, const QColor &) const;
    void drawRiffles(QPainter* p,  int x, int y, int w, int h,
                      const QPalette &pal, bool horizontal) const;
private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPlatinumStyle(const QPlatinumStyle &);
    QPlatinumStyle& operator=(const QPlatinumStyle &);
#endif
};

#endif // QT_NO_STYLE_PLATINUM

#endif // QPLATINUMSTYLE_H
