/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTEXTLAYOUT_P_H
#define QTEXTLAYOUT_P_H

#ifndef QT_H
#include "qstring.h"
#include "qnamespace.h"
#include "qrect.h"
#include "qvector.h"
#include "qcolor.h"
#include "qobject.h"
#endif // QT_H

class QTextEngine;
class QFont;
class QRect;
class QRegion;

class Q_GUI_EXPORT QTextInlineObject
{
public:
    QTextInlineObject(int i, QTextEngine *e) : itm(i), eng(e) {}
    inline QTextInlineObject() : itm(0), eng(0) {}
    inline bool isValid() const { return (bool)eng; }

    QRect rect() const;
    int width() const;
    int ascent() const;
    int descent() const;

    bool isRightToLeft() const;

    void setWidth(int w);
    void setAscent(int a);
    void setDescent(int d);

    int at() const;

    QTextEngine *engine() const { return eng; }
    int item() const { return itm; }

    int format() const;

private:
    friend class QTextLayout;
    int itm;
    QTextEngine *eng;
};

class QPainter;
class QTextFormatCollection;
class QTextFormat;
class QTextLine;
class QAbstractTextDocumentLayout;

class Q_GUI_EXPORT QTextLayout
{
public:
    // does itemization
    QTextLayout();
    QTextLayout(const QString& string);
    QTextLayout(const QString& string, QPainter *);
    QTextLayout(const QString& string, const QFont& fnt);
    // ######### implement me!
    // QTextLayout(QTextBlockIterator it);
    ~QTextLayout();

    void setText(const QString& string, const QFont& fnt);

    // ######### go away
    void setFormatCollection(const QTextFormatCollection *formats);
    void setText(const QString& string);
    QString text() const;
    void setDocumentLayout(QAbstractTextDocumentLayout *layout);

    enum LineBreakStrategy {
        AtWordBoundaries,
        AtCharBoundaries
    };

    void setTextFlags(int textFlags);
    // #### remove me
    void setFormat(int from, int length, int format);

    enum PaletteFlags {
        None,
        UseTextColor
    };
    void setPalette(const QPalette &, PaletteFlags f = None);

    void useDesignMetrics(bool);
    bool usesDesignMetrics() const;

    enum LayoutMode {
        NoBidi,
        SingleLine,
        MultiLine
    };

    void clearLines();
    QTextLine createLine();

    int numLines() const;
    QTextLine lineAt(int i) const;
    QTextLine findLine(int pos) const;

    // ### go away!
    void beginLayout(LayoutMode m = MultiLine, int textFlags = 0);


    enum CursorMode {
        SkipCharacters,
        SkipWords
    };
    bool validCursorPosition(int pos) const;
    int nextCursorPosition(int oldPos, CursorMode mode = SkipCharacters) const;
    int previousCursorPosition(int oldPos, CursorMode mode = SkipCharacters) const;

    enum SelectionType {
        NoSelection = 0,
        Highlight = -1,
        ImText = -2,
        ImSelection = -3
    };
    class Selection {
        int f;
        int l;
        int t;
    public:
        Selection() : f(-1), l(0), t(NoSelection) {}
        //Selection(int f, int l, int formatIndex) : from(f), length(l), selectionType(formatIndex) {}
        Selection(int from, int length, SelectionType type) : f(from), l(length), t(type) {}
        inline int from() const { return f; }
        inline int length() const { return l; }
        inline int type() const { return t; }
        inline void setRange(int from, int length) { f = from; l = length; }
        inline void setType(SelectionType type) { t = type; }
    };

    enum { NoCursor = -1 };

    inline void draw(QPainter *p, const QPoint &pos, int cursorPos, const QVector<Selection> &selections) const {
        draw(p, pos, cursorPos, selections.constData(), selections.size());
    }
    void draw(QPainter *p, const QPoint &pos, int cursorPos = NoCursor, const Selection *selections = 0, int nSelections = 0, const QRect &cr = QRect()) const;

    QPoint position() const;
    void setPosition(const QPoint &p);

    QRect boundingRect() const;
    QRect rect() const;

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

class QTextLine
{
public:
    QTextLine(int line, QTextEngine *e) : i(line), eng(e) {}
    inline QTextLine() : i(0), eng(0) {}
    inline bool isValid() const { return (bool)eng; }

    QRect rect() const;
    int x() const;
    int y() const;
    int width() const;
    int ascent() const;
    int descent() const;
    int textWidth() const;

    enum Edge {
        Leading,
        Trailing
    };
    enum CursorPosition {
        BetweenCharacters,
        OnCharacters
    };

    /* cPos gets set to the valid position */
    int cursorToX(int *cPos, Edge edge = Leading) const;
    inline int cursorToX(int cPos, Edge edge = Leading) const { return cursorToX(&cPos, edge); }
    int xToCursor(int x, CursorPosition = BetweenCharacters) const;

    void layout(int width);
    void setPosition(const QPoint &pos);

    int from() const;
    int length() const;

    QTextEngine *engine() const { return eng; }
    int line() const { return i; }

    void draw(QPainter *p, int x, int y, int selection = -1) const;

private:
    friend class QTextLayout;
    int i;
    QTextEngine *eng;
};

#endif
