/****************************************************************************
**
** Definition of QCommonStyle class.
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

#ifndef QCOMMONSTYLE_H
#define QCOMMONSTYLE_H

#ifndef QT_H
#include "qstyle.h"
#endif // QT_H

#ifndef QT_NO_STYLE

class Q_GUI_EXPORT QCommonStyle: public QStyle
{
    Q_OBJECT

public:
    QCommonStyle();
    ~QCommonStyle();

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
                      SFlags how = Style_Default,
                      const QStyleOption& = QStyleOption::Default) const;

    void drawControlMask(ControlElement element,
                          QPainter *p,
                          const QWidget *widget,
                          const QRect &r,
                          const QStyleOption& = QStyleOption::Default) const;

    QRect subRect(SubRect r, const QWidget *widget) const;

    void drawComplexControl(ComplexControl control,
                             QPainter *p,
                             const QWidget *widget,
                             const QRect &r,
                             const QPalette &pal,
                             SFlags how = Style_Default,
                             SCFlags sub = SC_All,
                             SCFlags subActive = SC_None,
                             const QStyleOption& = QStyleOption::Default) const;

    void drawComplexControlMask(ComplexControl control,
                                 QPainter *p,
                                 const QWidget *widget,
                                 const QRect &r,
                                 const QStyleOption& = QStyleOption::Default) const;

    QRect querySubControlMetrics(ComplexControl control,
                                  const QWidget *widget,
                                  SubControl sc,
                                  const QStyleOption& = QStyleOption::Default) const;

    SubControl querySubControl(ComplexControl control,
                                const QWidget *widget,
                                const QPoint &pos,
                                const QStyleOption& = QStyleOption::Default) const;

    int pixelMetric(PixelMetric m, const QWidget *widget = 0) const;

    QSize sizeFromContents(ContentsType s,
                            const QWidget *widget,
                            const QSize &contentsSize,
                            const QStyleOption& = QStyleOption::Default) const;

    int styleHint(StyleHint sh, const QWidget *, const QStyleOption &, QStyleHintReturn *) const;

    QPixmap stylePixmap(StylePixmap sp,
                         const QWidget *widget = 0,
                         const QStyleOption& = QStyleOption::Default) const;

    QPixmap stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap,
                         const QPalette &pal, const QStyleOption& = QStyleOption::Default) const;

private:
    // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QCommonStyle(const QCommonStyle &);
    QCommonStyle &operator=(const QCommonStyle &);
#endif
};



#endif // QT_NO_STYLE

#endif // QCOMMONSTYLE_H
