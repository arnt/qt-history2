/****************************************************************************
**
** Definition of compact style class, good for small displays.
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

#ifndef QCOMPACTSTYLE_H
#define QCOMPACTSTYLE_H

#ifndef QT_H
#include "qwindowsstyle.h"
#endif // QT_H

#if !defined(QT_NO_STYLE_COMPACT) || defined(QT_PLUGIN)

#if defined(QT_PLUGIN)
#define Q_GUI_EXPORT_STYLE_COMPACT
#else
#define Q_GUI_EXPORT_STYLE_COMPACT Q_GUI_EXPORT
#endif

class Q_GUI_EXPORT_STYLE_COMPACT QCompactStyle : public QWindowsStyle
{
public:
    QCompactStyle();

    int pixelMetric(PixelMetric metric, const QWidget *widget = 0) const;

    void drawControl(ControlElement element, QPainter *p, const QWidget *w, const QRect &r,
                      const QPalette &pal, SFlags how = Style_Default, const Q3StyleOption& = Q3StyleOption::Default);

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QCompactStyle(const QCompactStyle &);
    QCompactStyle& operator=(const QCompactStyle &);
#endif
};

#endif // QT_NO_STYLE_WINDOWS

#endif // QCOMPACTSTYLE_H
