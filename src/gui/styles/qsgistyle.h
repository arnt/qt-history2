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

#ifndef QSGISTYLE_H
#define QSGISTYLE_H

#include "QtGui/qmotifstyle.h"
#include "QtCore/qpointer.h"
#include "QtGui/qwidget.h"

#if !defined(QT_NO_STYLE_SGI) || defined(QT_PLUGIN)

#if defined(QT_PLUGIN)
#define Q_GUI_EXPORT_STYLE_SGI
#else
#define Q_GUI_EXPORT_STYLE_SGI Q_GUI_EXPORT
#endif

class QSGIStylePrivate;

class Q_GUI_EXPORT_STYLE_SGI QSGIStyle: public QMotifStyle
{
    Q_OBJECT
public:
    explicit QSGIStyle(bool useHighlightCols = false);
    virtual ~QSGIStyle();

#if !defined(Q_NO_USING_KEYWORD)
    using QMotifStyle::polish;
#endif
    void polish(QWidget*);
    void unpolish(QWidget*);
    void polish(QApplication*);
    void unpolish(QApplication*);

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                       const QWidget *w = 0) const;

    void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                     const QWidget *w = 0) const;

    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            const QWidget *w = 0) const;

    int pixelMetric(PixelMetric pm, const QStyleOption *opt = 0, const QWidget *widget = 0) const;

    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt,
                           const QSize &contentsSize, const QWidget *widget = 0) const;

    QRect subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const;

    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                                 const QWidget *w = 0) const;

protected:
    bool eventFilter(QObject*, QEvent*);

private:
    Q_DISABLE_COPY(QSGIStyle)

    QSGIStylePrivate *d;
    uint isApplicationStyle : 1;
};

#endif // QT_NO_STYLE_SGI

#endif // QSGISTYLE_H
