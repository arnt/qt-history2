/****************************************************************************
**
** Definition of Windows-like style class.
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

#ifndef QWINDOWSSTYLE_H
#define QWINDOWSSTYLE_H

#ifndef QT_H
#include "qcommonstyle.h"
#endif // QT_H

#if !defined(QT_NO_STYLE_WINDOWS) || defined(QT_PLUGIN)

#if defined(QT_PLUGIN)
#define Q_GUI_EXPORT_STYLE_WINDOWS
#else
#define Q_GUI_EXPORT_STYLE_WINDOWS Q_GUI_EXPORT
#endif


class Q_GUI_EXPORT_STYLE_WINDOWS QWindowsStyle : public QCommonStyle
{
    Q_OBJECT
public:
    QWindowsStyle();
    ~QWindowsStyle();

    void polish(QApplication*);
    void unPolish(QApplication*);

    void polish(QWidget*);
    void unPolish(QWidget*);

    void polish(QPalette &);

    void drawPrimitive(PrimitiveElement pe, const Q4StyleOption *opt, QPainter *p,
                               const QWidget *w = 0) const;
    void drawControl(ControlElement element, const Q4StyleOption *opt, QPainter *p,
                             const QWidget *w = 0) const;
    QRect subRect(SubRect r, const Q4StyleOption *opt, const QWidget *widget = 0) const;
    void drawComplexControl(ComplexControl cc, const Q4StyleOptionComplex *opt, QPainter *p,
                                    const QWidget *w = 0) const;
    QSize sizeFromContents(ContentsType ct, const Q4StyleOption *opt, const QSize &contentsSize,
                           const QFontMetrics &fm, const QWidget *widget = 0) const;

    void drawPrimitive(PrimitiveElement pe,
                        QPainter *p,
                        const QRect &r,
                        const QPalette &pal,
                        SFlags flags = Style_Default,
                        const QStyleOption& = QStyleOption::Default) const;

    void drawControl(ControlElement element,
                      QPainter *p,
                      const QWidget *widget,
                      const QRect &r,
                      const QPalette &pal,
                      SFlags flags = Style_Default,
                      const QStyleOption& = QStyleOption::Default) const;

    void drawComplexControl(ComplexControl control,
                             QPainter* p,
                             const QWidget* widget,
                             const QRect& r,
                             const QPalette &pal,
                             SFlags flags = Style_Default,
                             SCFlags sub = SC_All,
                             SCFlags subActive = SC_None,
                             const QStyleOption& = QStyleOption::Default) const;

    int pixelMetric(PixelMetric metric,
                     const QWidget *widget = 0) const;

    QSize sizeFromContents(ContentsType contents,
                            const QWidget *widget,
                            const QSize &contentsSize,
                            const QStyleOption& = QStyleOption::Default) const;

    int styleHint(StyleHint sh, const QWidget *, const QStyleOption & = QStyleOption::Default,
                  QStyleHintReturn* = 0) const;

    QPixmap stylePixmap(StylePixmap stylepixmap,
                         const QWidget *widget = 0,
                         const QStyleOption& = QStyleOption::Default) const;

    QRect subRect(SubRect r, const QWidget *widget) const;


private:
    class Private;
    Private *d;

    // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWindowsStyle(const QWindowsStyle &);
    QWindowsStyle& operator=(const QWindowsStyle &);
#endif
};

#endif // QT_NO_STYLE_WINDOWS

#endif // QWINDOWSSTYLE_H
