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

#ifndef QFONTMETRICS_H
#define QFONTMETRICS_H

#include "qfont.h"
#ifndef QT_INCLUDE_COMPAT
#include "qrect.h"
#endif

#ifdef Q_WS_QWS
class QFontEngine;
#endif

class QTextCodec;
class QRect;

class Q_GUI_EXPORT QFontMetrics
{
public:
    QFontMetrics(const QFont &);
    QFontMetrics(const QFont &, QFont::Script);
    QFontMetrics(const QFontMetrics &);
    ~QFontMetrics();

    QFontMetrics &operator=(const QFontMetrics &);

    int                ascent()        const;
    int                descent()        const;
    int                height()        const;
    int                leading()        const;
    int                lineSpacing()        const;
    int                minLeftBearing() const;
    int                minRightBearing() const;
    int                maxWidth()        const;

    bool        inFont(QChar)        const;

    int                leftBearing(QChar) const;
    int                rightBearing(QChar) const;
    int                width(const QString &, int len = -1) const;

    int                width(QChar) const;

    int                 charWidth(const QString &str, int pos) const;
    QRect        boundingRect(const QString &, int len = -1) const;
    QRect        boundingRect(QChar) const;
    QRect        boundingRect(int x, int y, int w, int h, int flags,
                              const QString& str, int len=-1, int tabstops=0,
                              int *tabarray=0) const;
    QSize        size(int flags,
                      const QString& str, int len=-1, int tabstops=0,
                      int *tabarray=0) const;

    int                underlinePos()        const;
    int         overlinePos()   const;
    int                strikeOutPos()        const;
    int                lineWidth()        const;

private:
    QFontMetrics(const QPainter *);

    friend class QWidget;
    friend class QPainter;
    friend class Q3TextFormat;
#if defined(Q_WS_MAC)
    friend class QFontPrivate;
#endif

    QFontPrivate  *d;
    QPainter      *painter;
    int                   fscript;
};


#endif // QFONTMETRICS_H
