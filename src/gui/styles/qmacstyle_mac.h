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

#ifndef QMACSTYLE_MAC_H
#define QMACSTYLE_MAC_H

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

class Q_GUI_EXPORT_STYLE_MAC QMacStyle : public QWindowsStyle
{
    Q_OBJECT
public:
    QMacStyle();
    virtual ~QMacStyle();

    void polish(QWidget * w);
    void unPolish(QWidget * w);
    void polish(QApplication*);

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                       const QWidget *w = 0) const;
    void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                     const QWidget *w = 0) const;
    QRect subRect(SubRect r, const QStyleOption *opt, const QWidget *widget = 0) const;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            const QWidget *w = 0) const;
    SubControl querySubControl(ComplexControl cc, const QStyleOptionComplex *opt,
                               const QPoint &pt, const QWidget *w = 0) const;
    QRect querySubControlMetrics(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                                 const QWidget *w = 0) const;
    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize,
                           const QFontMetrics &fm, const QWidget *w = 0) const;

    int pixelMetric(PixelMetric metric,
                     const QWidget *widget = 0) const;



    virtual int styleHint(StyleHint sh, const QWidget *, const Q3StyleOption &,
                          QStyleHintReturn *) const;

    enum FocusRectPolicy { FocusEnabled, FocusDisabled, FocusDefault };
    static void setFocusRectPolicy(QWidget *w, FocusRectPolicy policy);
    static FocusRectPolicy focusRectPolicy(const QWidget *w);

    enum WidgetSizePolicy { SizeSmall, SizeLarge, SizeMini, SizeDefault
#ifdef QT_COMPAT
                            , SizeNone = SizeDefault
#endif
    };
    static void setWidgetSizePolicy(const QWidget *w, WidgetSizePolicy policy);
    static WidgetSizePolicy widgetSizePolicy(const QWidget *w);

    QPixmap stylePixmap(StylePixmap sp,
                         const QWidget *widget = 0,
                         const Q3StyleOption& = Q3StyleOption::Default) const;

    QPixmap stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap,
                         const QPalette &pal, const Q3StyleOption& = Q3StyleOption::Default) const;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMacStyle(const QMacStyle &);
    QMacStyle& operator=(const QMacStyle &);
#endif

private:
    QStyle *correctStyle(const QPainter *) const;
    QStyle *correctStyle(const QPaintDevice *) const;
    mutable QStyle *qd_style, *cg_style;
};

#endif // Q_WS_MAC

#endif // QMACSTYLE_H
