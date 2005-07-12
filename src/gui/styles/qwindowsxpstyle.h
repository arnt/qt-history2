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

#ifndef QWINDOWSXPSTYLE_H
#define QWINDOWSXPSTYLE_H

#include "QtGui/qwindowsstyle.h"

#if !defined(QT_NO_STYLE_WINDOWSXP) || defined(QT_PLUGIN)

#if defined(QT_PLUGIN)
#define Q_GUI_EXPORT_STYLE_WINDOWSXP
#else
#define Q_GUI_EXPORT_STYLE_WINDOWSXP Q_GUI_EXPORT
#endif

class QWindowsXPStylePrivate;
class Q_GUI_EXPORT_STYLE_WINDOWSXP QWindowsXPStyle : public QWindowsStyle
{
    Q_OBJECT
public:
    QWindowsXPStyle();
    ~QWindowsXPStyle();

    void unpolish(QApplication*);
    void polish(QApplication*);
    void polish(QWidget*);
    void polish(QPalette&);
    void unpolish(QWidget*);

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *option, QPainter *p,
                       const QWidget *widget = 0) const;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *p,
                     const QWidget *wwidget = 0) const;
    QRect subElementRect(SubElement r, const QStyleOption *option, const QWidget *widget = 0) const;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *option, SubControl sc,
                         const QWidget *widget = 0) const;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *option, QPainter *p,
                            const QWidget *widget = 0) const;
    QSize sizeFromContents(ContentsType ct, const QStyleOption *option, const QSize &contentsSize,
                           const QWidget *widget = 0) const;
    int pixelMetric(PixelMetric pm, const QStyleOption *option = 0,
                    const QWidget *widget = 0) const;
    int styleHint(StyleHint hint, const QStyleOption *option = 0, const QWidget *widget = 0,
                  QStyleHintReturn *returnData = 0) const;

    QPalette standardPalette() const;
    QPixmap standardPixmap(StandardPixmap standardIcon, const QStyleOption *option,
                           const QWidget *widget = 0) const;

protected slots:
    QIcon standardIconSlot(StandardPixmap standardIcon, const QStyleOption *option,
                           const QWidget *widget = 0) const;

private:
    Q_DISABLE_COPY(QWindowsXPStyle)

    friend class QStyleFactory;
    friend class QWindowsXPStylePrivate;
    QWindowsXPStylePrivate *dd;
};

#endif // QT_NO_STYLE_WINDOWSXP

#endif // QWINDOWSXPSTYLE_H
