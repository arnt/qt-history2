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

#ifndef QTEXTLAYOUT_H
#define QTEXTLAYOUT_H

#include "QtCore/qstring.h"
#include "QtCore/qnamespace.h"
#include "QtCore/qrect.h"
#include "QtCore/qvector.h"
#include "QtGui/qcolor.h"
#include "QtCore/qobject.h"
#include "QtGui/qevent.h"
#include "QtGui/qtextformat.h"

class QTextEngine;
class QFont;
class QRect;
class QRegion;
class QTextFormat;
class QPalette;
class QPainter;

class Q_GUI_EXPORT QTextInlineObject
{
public:
    QTextInlineObject(int i, QTextEngine *e) : itm(i), eng(e) {}
    inline QTextInlineObject() : itm(0), eng(0) {}
    inline bool isValid() const { return eng; }

    QRect rect() const;
    qreal width() const;
    qreal ascent() const;
    qreal descent() const;
    qreal height() const;

    bool isRightToLeft() const;

    void setWidth(qreal w);
    void setAscent(qreal a);
    void setDescent(qreal d);

    int at() const;

    QTextEngine *engine() const { return eng; }
    int item() const { return itm; }

    int formatIndex() const;
    QTextFormat format() const;

private:
    friend class QTextLayout;
    int itm;
    QTextEngine *eng;
};

class QPaintDevice;
class QTextFormat;
class QTextLine;
class QTextBlock;
class QTextOption;

class Q_GUI_EXPORT QTextLayout
{
public:
    // does itemization
    QTextLayout();
    QTextLayout(const QString& string);
    QTextLayout(const QString& string, const QFont &font, QPaintDevice *paintdevice = 0);
    QTextLayout(const QTextBlock &b);
    ~QTextLayout();

    void setFont(const QFont &f);
    QFont font() const;

    void setText(const QString& string);
    QString text() const;

    enum LineBreakStrategy {
        AtWordBoundaries,
        AtCharBoundaries
    };

    void setTextOption(const QTextOption &option);
    QTextOption textOption() const;

    void setPalette(const QPalette &);

    void setPreeditArea(int position, const QString &text);
    int preeditAreaPosition() const;
    QString preeditAreaText() const;

    struct FormatOverride {
        int from;
        int length;
        QTextCharFormat format;
    };
    void setFormatOverrides(const QList<FormatOverride> &overrides);
    QList<FormatOverride> formatOverrides() const;
    void clearFormatOverrides();

    enum LayoutModeFlags {
        MultiLine = 0,
        NoBidi = 0x1,
        NoGlyphCache = 0x2000
    };
    Q_DECLARE_FLAGS(LayoutMode, LayoutModeFlags)

    void setLayoutMode(LayoutMode m);
    void beginLayout();
    void endLayout();

    QTextLine createLine();

    int numLines() const;
    QTextLine lineAt(int i) const;
    QTextLine findLine(int pos) const;


    enum CursorMode {
        SkipCharacters,
        SkipWords
    };
    bool validCursorPosition(int pos) const;
    int nextCursorPosition(int oldPos, CursorMode mode = SkipCharacters) const;
    int previousCursorPosition(int oldPos, CursorMode mode = SkipCharacters) const;

    void draw(QPainter *p, const QPointF &pos, const QRect &clip = QRect()) const;
    void drawCursor(QPainter *p, const QPointF &pos, int cursorPosition) const;

    QPointF position() const;
    void setPosition(const QPointF &p);

    QRectF boundingRect() const;
    QRectF rect() const;

    QTextEngine *engine() const { return d; }

    int minimumWidth() const;
    int maximumWidth() const;

private:
    QTextLayout(QTextEngine *e) : d(e) {}
    /* disable copy and assignment */
    QTextLayout(const QTextLayout &) {}
    void operator = (const QTextLayout &) {}

    friend class QPainter;
    friend class QPSPrinter;
    QTextEngine *d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QTextLayout::LayoutMode);


class Q_GUI_EXPORT QTextLine
{
public:
    inline QTextLine() : i(0), eng(0) {}
    inline bool isValid() const { return eng; }

    QRect rect() const;
    qreal x() const;
    qreal y() const;
    qreal width() const;
    qreal ascent() const;
    qreal descent() const;
    qreal height() const;
    qreal textWidth() const;
    QRectF textRect() const;

    enum Edge {
        Leading,
        Trailing
    };
    enum CursorPosition {
        CursorBetweenCharacters,
        CursorOnCharacter
    };

    /* cursorPos gets set to the valid position */
    qreal cursorToX(int *cursorPos, Edge edge = Leading) const;
    inline qreal cursorToX(int cursorPos, Edge edge = Leading) const { return cursorToX(&cursorPos, edge); }
    int xToCursor(qreal x, CursorPosition = CursorBetweenCharacters) const;

    void layout(qreal width);
    void layoutFixedColumnWidth(int numColumns);
    void setPosition(const QPointF &pos);

    int from() const;
    int length() const;

    QTextEngine *engine() const { return eng; }
    int line() const { return i; }

    void draw(QPainter *p, const QPointF &point) const;

private:
    QTextLine(int line, QTextEngine *e) : i(line), eng(e) {}
    void layout_helper(int numGlyphs);
    friend class QTextLayout;
    int i;
    QTextEngine *eng;
};

#endif // QTEXTLAYOUT_H
