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
    QFontMetrics(const QFont &, QPaintDevice *pd);
    QFontMetrics(const QFont &, QFont::Script);
    QFontMetrics(const QFontMetrics &);
    ~QFontMetrics();

    QFontMetrics &operator=(const QFontMetrics &);

    int ascent() const;
    int descent() const;
    int height() const;
    int leading() const;
    int lineSpacing() const;
    int minLeftBearing() const;
    int minRightBearing() const;
    int maxWidth() const;

    bool inFont(QChar) const;

    int leftBearing(QChar) const;
    int rightBearing(QChar) const;
    int width(const QString &, int len = -1) const;

    int width(QChar) const;
    int charWidth(const QString &str, int pos) const;

    QRect boundingRect(QChar) const;

    QRect boundingRect(const QString &text) const;
    QRect boundingRect(const QRect &r, int flags, const QString &text, int tabstops=0, int *tabarray=0) const;
    inline QRect boundingRect(int x, int y, int w, int h, int flags, const QString &text,
                              int tabstops=0, int *tabarray=0) const
        { return boundingRect(QRect(x, y, w, h), flags, text, tabstops, tabarray); }
    QSize size(int flags, const QString& str, int tabstops=0, int *tabarray=0) const;

    int underlinePos() const;
    int overlinePos() const;
    int strikeOutPos() const;
    int lineWidth() const;

    bool operator==(const QFontMetrics &other);
    inline bool operator !=(const QFontMetrics &other) { return !operator==(other); }

#ifdef QT_COMPAT
    inline QRect boundingRect(const QString &text, int len) const
        { return boundingRect(text.left(len)); }
    inline QRect boundingRect(int x, int y, int w, int h, int flags, const QString& str, int len,
                              int tabstops=0, int *tabarray=0) const
        { return boundingRect(QRect(x, y, w, h), flags, str.left(len), tabstops, tabarray); }
    inline QSize size(int flags, const QString& str, int len, int tabstops=0, int *tabarray=0) const
        { return size(flags, str.left(len), tabstops, tabarray); }
#endif
private:
#if defined(Q_WS_MAC)
    friend class QFontPrivate;
#endif

    QFontPrivate  *d;
    int fscript;
};


class Q_GUI_EXPORT QFontMetricsF
{
public:
    QFontMetricsF(const QFont &);
    QFontMetricsF(const QFont &, QPaintDevice *pd);
    QFontMetricsF(const QFont &, QFont::Script);
    QFontMetricsF(const QFontMetrics &);
    QFontMetricsF(const QFontMetricsF &);
    ~QFontMetricsF();

    QFontMetricsF &operator=(const QFontMetricsF &);
    QFontMetricsF &operator=(const QFontMetrics &);

    qReal ascent() const;
    qReal descent() const;
    qReal height() const;
    qReal leading() const;
    qReal lineSpacing() const;
    qReal minLeftBearing() const;
    qReal minRightBearing() const;
    qReal maxWidth() const;

    bool inFont(QChar) const;

    qReal leftBearing(QChar) const;
    qReal rightBearing(QChar) const;
    qReal width(const QString &string) const;

    qReal width(QChar) const;

    QRectF boundingRect(const QString &string) const;
    QRectF boundingRect(QChar) const;
    QRectF boundingRect(const QRectF &r, int flags, const QString& string, int tabstops=0, int *tabarray=0) const;
    QSizeF size(int flags, const QString& str, int tabstops=0, int *tabarray=0) const;

    qReal underlinePos() const;
    qReal overlinePos() const;
    qReal strikeOutPos() const;
    qReal lineWidth() const;

    bool operator==(const QFontMetricsF &other);
    inline bool operator !=(const QFontMetricsF &other) { return !operator==(other); }

private:
    QFontPrivate  *d;
    int fscript;
};

#endif // QFONTMETRICS_H
