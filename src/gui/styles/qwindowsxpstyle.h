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

#include "qwindowsstyle.h"

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

    void unPolish(QApplication*);
    void polish(QApplication*);
    void polish(QWidget*);
    void unPolish(QWidget*);

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                       const QWidget *w = 0) const;

    void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                     const QWidget *widget = 0) const;

    void drawControlMask(ControlElement element, const QStyleOption *opt, QPainter *p,
                         const QWidget *w = 0) const;

    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter* p,
                            const QWidget* w = 0) const;


    int pixelMetric(PixelMetric metic, const QStyleOption *option = 0,
                    const QWidget *widget = 0) const;

    QRect querySubControlMetrics(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                                 const QWidget *w = 0) const;

    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt,
                           const QSize &contentsSize, const QWidget *widget = 0) const;

    int styleHint(StyleHint sh, const QStyleOption *opt = 0, const QWidget *w = 0,
                  QStyleHintReturn *shret = 0) const;

protected:
    bool eventFilter(QObject *o, QEvent *e);

    void updateRegion(QWidget *widget);

protected slots:
    void activeTabChanged();

private:
    Q_DISABLE_COPY(QWindowsXPStyle)

    QWindowsXPStylePrivate *d;

    friend class QStyleFactory;
    friend class QWindowsXPStylePrivate;
    static bool resolveSymbols();
};

#endif // QT_NO_STYLE_WINDOWSXP

#endif // QWINDOWSXPSTYLE_H
