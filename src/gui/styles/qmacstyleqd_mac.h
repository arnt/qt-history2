/****************************************************************************
**
** Definition of ...
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

#ifndef QMACSTYLEQD_MAC_H
#define QMACSTYLEQD_MAC_H

#ifndef QT_H
#include "qwindowsstyle.h"
#endif // QT_H

#if defined(Q_WS_MAC) && !defined(QT_NO_STYLE_MAC)

class QPalette;

#if defined(QT_PLUGIN)
#define Q_GUI_EXPORT_STYLE_MAC
#else
#define Q_GUI_EXPORT_STYLE_MAC Q_GUI_EXPORT
#endif

class QMacStyleQDPrivate;

class Q_GUI_EXPORT_STYLE_MAC QMacStyleQD : public QWindowsStyle
{
    Q_OBJECT
public:
    QMacStyleQD();
    ~QMacStyleQD();

    void polish(QWidget * w);
    void unPolish(QWidget * w);
    void polish(QApplication*);

    void drawPrimitive(PrimitiveElement pe, const Q4StyleOption *opt, QPainter *p,
                       const QWidget *w = 0) const;
    void drawControl(ControlElement element, const Q4StyleOption *opt, QPainter *p,
                     const QWidget *w = 0) const;
    QRect subRect(SubRect r, const Q4StyleOption *opt, const QWidget *widget = 0) const;
    void drawComplexControl(ComplexControl cc, const Q4StyleOptionComplex *opt, QPainter *p,
                            const QWidget *w = 0) const;
    SubControl querySubControl(ComplexControl cc, const Q4StyleOptionComplex *opt,
                               const QPoint &pt, const QWidget *w = 0) const;
    QRect querySubControlMetrics(ComplexControl cc, const Q4StyleOptionComplex *opt, SubControl sc,
                                 const QWidget *w) const;
    QSize sizeFromContents(ContentsType ct, const Q4StyleOption *opt, const QSize &contentsSize,
                           const QFontMetrics &fm, const QWidget *widget = 0) const;

    int pixelMetric(PixelMetric metric, const QWidget *widget) const;

    int styleHint(StyleHint sh, const QWidget *, const QStyleOption &, QStyleHintReturn *) const;

    QPixmap stylePixmap(StylePixmap sp, const QWidget *widget,
                        const QStyleOption& = QStyleOption::Default) const;

    QPixmap stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap,
                        const QPalette &pal, const QStyleOption& = QStyleOption::Default) const;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMacStyleQD(const QMacStyleQD &);
    QMacStyleQD& operator=(const QMacStyleQD &);
#endif

protected:
    QMacStyleQDPrivate *d;
};

#endif // Q_WS_MAC

#endif // QMACSTYLEQD_H
