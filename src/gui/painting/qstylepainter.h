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

#ifndef QSTYLEPAINTER_H
#define QSTYLEPAINTER_H

#include "qpainter.h"
#include "qstyle.h"
#include "qwidget.h"

class QStylePainter : public QPainter
{
public:
    inline QStylePainter() : QPainter(), widget(0), wstyle(0) {}
    inline QStylePainter(QWidget *w) { begin(w, w); }
    inline QStylePainter(QPaintDevice *pd, QWidget *w) { begin(pd, w); }
    inline bool begin(QWidget *w) { return begin(w, w); }
    inline bool begin(QPaintDevice *pd, QWidget *w) {
        Q_ASSERT_X(w, "QStylePainter::QStylePainter", "Widget must be non-zero");
        widget = w;
        wstyle = w->style();
        return QPainter::begin(pd);
    };
    inline void drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption &opt);
    inline void drawControl(QStyle::ControlElement ce, const QStyleOption &opt);
    inline void drawControlMask(QStyle::ControlElement element, const QStyleOption &opt);
    inline void drawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex &opt);
    inline void drawComplexControlMask(QStyle::ComplexControl cc, const QStyleOptionComplex &opt);
    inline void drawItem(const QRect &r, int flags, const QPalette &pal, bool enabled,
                  const QString &text, int len = -1, const QColor *penColor = 0)
    {
        wstyle->drawItem(this, r, flags, pal, enabled, text, len, penColor);
    }

    inline void drawItem(const QRect &r, int flags, const QPalette &pal, bool enabled,
                          const QPixmap &pixmap, const QColor *penColor = 0)
    {
        wstyle->drawItem(this, r, flags, pal, enabled, pixmap, penColor);
    }

    inline void drawItem(const QRect &r, int flags, const QPalette &pal, bool enabled,
                         const QPixmap &pixmap, const QString &text, int len = -1,
                         const QColor *penColor = 0)
    {
        if (!isActive()) {
            qWarning("QPainter is not active");
            return;
        }
        if (!pixmap.isNull())
            drawItem(r, flags, pal, enabled, pixmap, penColor);
        else
            drawItem(r, flags, pal, enabled, text, len, penColor);
    }

    inline QStyle *style() const { return wstyle; }

private:
    QWidget *widget;
    QStyle *wstyle;
};

void QStylePainter::drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption &opt)
{
    if (!isActive()) {
        qWarning("QPainter is not active");
        return;
    }
    wstyle->drawPrimitive(pe, &opt, this, widget);
}

void QStylePainter::drawControl(QStyle::ControlElement ce, const QStyleOption &opt)
{
    if (!isActive()) {
        qWarning("QPainter is not active");
        return;
    }
    wstyle->drawControl(ce, &opt, this, widget);
}

void QStylePainter::drawControlMask(QStyle::ControlElement ce, const QStyleOption &opt)
{
    if (!isActive()) {
        qWarning("QPainter is not active");
        return;
    }
    wstyle->drawControlMask(ce, &opt, this, widget);
}

void QStylePainter::drawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex &opt)
{
    if (!isActive()) {
        qWarning("QPainter is not active");
        return;
    }
    wstyle->drawComplexControl(cc, &opt, this, widget);
}

void QStylePainter::drawComplexControlMask(QStyle::ComplexControl cc,
                                           const QStyleOptionComplex &opt)
{
    if (!isActive()) {
        qWarning("QPainter is not active");
        return;
    }
    wstyle->drawComplexControlMask(cc, &opt, this, widget);
}


#endif
