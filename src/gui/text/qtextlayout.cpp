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

#include "qtextlayout.h"
#include "qtextengine_p.h"

#include <qfont.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qvarlengtharray.h>
#include <qtextformat.h>
#include <qabstracttextdocumentlayout.h>
#include "qtextdocument_p.h"
#include "qtextformat_p.h"
#include <limits.h>

#include "qfontengine_p.h"

/*!
    \class QTextInlineObject
    \brief The QTextInlineObject class represents an inline object in
    a QTextLayout.

    \ingroup text

    This class is only used if the text layout is used to lay out
    parts of a QTextDocument.

    The inline object has various attributes that can be set, for
    example using, setWidth(), setAscent(), and setDescent(). The
    rectangle it occupies is given by rect(), and its direction by
    isRightToLeft(). Its position in the text layout is given by at(),
    and its format is given by format().
*/

/*!
    \fn QTextInlineObject::QTextInlineObject(int i, QTextEngine *e)

    Creates a new inline object for the item at position \a i in the
    text engine \a e.
*/

/*!
    \fn QTextInlineObject::QTextInlineObject()

    \internal
*/

/*!
    \fn bool QTextInlineObject::isValid() const

    Returns true if this inline object is valid; otherwise returns
    false.
*/

/*!
    \fn QTextEngine *QTextInlineObject::engine() const

    Returns the text engine set in the constructor.
*/

/*!
    \fn int QTextInlineObject::item() const

    Returns the inline object's position in the text engine as set in
    the constructor.
*/

/*!
    Returns the inline object's rectangle.

    \sa ascent() descent() width()
*/
QRect QTextInlineObject::rect() const
{
    QScriptItem& si = eng->items[itm];
    return QRect(0, -si.ascent.toInt(), si.width.toInt(), (si.ascent+si.descent).toInt());
}

/*!
    Returns the inline object's width.

    \sa ascent() descent() rect()
*/
int QTextInlineObject::width() const
{
    return eng->items[itm].width.toInt();
}

/*!
    Returns the inline object's ascent.

    \sa descent() width() rect()
*/
int QTextInlineObject::ascent() const
{
    return eng->items[itm].ascent.toInt();
}

/*!
    Returns the inline object's descent.

    \sa ascent() width() rect()
*/
int QTextInlineObject::descent() const
{
    return eng->items[itm].descent.toInt();
}

/*!
    Sets the inline object's width to \a w.

    \sa width() ascent() descent() rect()
*/
void QTextInlineObject::setWidth(int w)
{
    eng->items[itm].width = w;
}

/*!
    Sets the inline object's ascent to \a a.

    \sa ascent() setDescent() width() rect()
*/
void QTextInlineObject::setAscent(int a)
{
    eng->items[itm].ascent = a;
}

/*!
    Sets the inline object's decent to \a d.

    \sa descent() setAscent() width() rect()
*/
void QTextInlineObject::setDescent(int d)
{
    eng->items[itm].descent = d;
}

/*!
  The position of the inline object within the text layout.
*/
int QTextInlineObject::at() const
{
    return eng->items[itm].position;
}

/*!
  Returns an integer describing the format of the inline object
  within the text layout.
*/
int QTextInlineObject::formatIndex() const
{
    return eng->items[itm].format;
}

/*!
  Returns format of the inline object within the text layout.
*/
QTextFormat QTextInlineObject::format() const
{
    if (!eng->formats)
        return QTextFormat();
    return eng->formats->format(eng->items[itm].format);
}

/*!
  Returns if the object should be laid out right-to-left or left-to-right.
*/
bool QTextInlineObject::isRightToLeft() const
{
    return (eng->items[itm].analysis.bidiLevel % 2);
}

/*!
    \class QTextLayout
    \brief The QTextLayout class is used to lay out and paint a single
    paragraph of text.

    \ingroup text

    It offers most features expected from a modern text layout
    engine, including Unicode compliant rendering, line breaking and
    handling of cursor positioning. It can also produce and render
    device independent layout, something that is important for WYSIWYG
    applications.

    The class has a rather low level API and unless you intend to
    implement you own text rendering for some specialized widget, you
    probably won't need to use it directly.

    QTextLayout can currently deal with plain text and rich text
    paragraphs that are part of a QTextDocument.

    QTextLayout can be used to create a sequence of QTextLine's with
    given widths and can position them independently on the screen.
    Once the layout is done, these lines can be drawn on a paint
    device.

    Here's some pseudo code that presents the layout phase:
    \code
        int leading = fontMetrics.leading();
        int height = 0;
        int widthUsed = 0;
        textLayout.clearLines();
        while (1) {
            QTextLine line = textLayout.createLine();
            if (!line.isValid())
                break;

            line.layout(lineWidth);
            height += leading;
            line.setPosition(QPoint(0, height));
            height += line.ascent() + line.descent();
            widthUsed = qMax(widthUsed, line.textWidth());
        }
    \endcode

    And here's some pseudo code that presents the painting phase:
    \code
        for (int i = 0; i < textLayout.numLines(); ++i) {
            QTextLine line = textLayout.lineAt(i);
            line.draw(painter, rect.x() + xoffset + line.x(), rect.y() + yoffset);
        }
    \endcode

    The text layout's text is set in the constructor or with
    setText(). The layout can be seen as a sequence of QTextLine
    objects; use lineAt() or findLine() to get a QTextLine,
    createLine() to create one and clearLines() to remove them. For a
    given position in the text you can find a valid cursor position
    with validCursorPosition(), nextCursorPosition(), and
    previousCursorPosition(). The layout itself can be positioned with
    setPosition(); it has a boundingRect(), and a minimumWidth() and a
    maximumWidth(). A text layout can be drawn on a painter device
    using draw().

*/

/*!
    \enum QTextLayout::LineBreakStrategy

    \value AtWordBoundaries
    \value AtCharBoundaries
*/

/*!
    \enum QTextLayout::PaletteFlags

    \internal

    \value None
    \value UseTextColor
*/

/*!
    \enum QTextLayout::LayoutMode

    \internal

    \value NoBidi
    \value SingleLine
    \value MultiLine
*/

/*!
    \enum QTextLayout::CursorMode

    \value SkipCharacters
    \value SkipWords
*/

/*!
    \enum QTextLayout::SelectionType

    \internal

    \value NoSelection
    \value Highlight
    \value ImText
    \value ImSelection
*/

/*!
    \fn QTextEngine *QTextLayout::engine() const

    \internal
*/

/*!
    Constructs an empty text layout.

    \sa setText()
*/
QTextLayout::QTextLayout()
{ d = new QTextEngine(); }

/*!
    Constructs a text layout to lay out the given \a string.
*/
QTextLayout::QTextLayout(const QString& string)
{
    d = new QTextEngine();
    d->setText(string);
}

/*!
    Constructs a text layout to lay out the given \a string.

    All the metric and layout calculations will be done in terms of
    the painter, \a p.
*/
QTextLayout::QTextLayout(const QString& string, QPainter *p)
{
    QFontPrivate *f = p ? (p->font().d) : QApplication::font().d;
    d = new QTextEngine((string.isNull() ? (const QString&)QString::fromLatin1("") : string), f);
}

/*!
    Constructs a text layout to lay out the given \a string using the
    font \a fnt.
*/
QTextLayout::QTextLayout(const QString& string, const QFont& fnt)
{
    d = new QTextEngine((string.isNull() ? (const QString&)QString::fromLatin1("") : string), fnt.d);
}

/*!
    Constructs a text layout to lay out the given \a block.
*/
QTextLayout::QTextLayout(const QTextBlock &block)
{
    d = new QTextEngine();
    d->block = block;

    const QTextDocumentPrivate *p = block.docHandle();

    d->formats = p->formatCollection();
    d->docLayout = p->layout();

    QString txt = block.text();
    setText(txt);
    if (txt.isEmpty())
        return;

    int lastTextPosition = 0;
    int textLength = 0;

    QTextDocumentPrivate::FragmentIterator it = p->find(block.position());
    QTextDocumentPrivate::FragmentIterator end = p->find(block.position() + block.length() - 1); // -1 to omit the block separator char
    int lastFormatIdx = it.value()->format;

    for (; it != end; ++it) {
        const QTextFragmentData * const frag = it.value();

        const int formatIndex = frag->format;
        if (formatIndex != lastFormatIdx) {
            Q_ASSERT(lastFormatIdx != -1);
            setFormat(lastTextPosition, textLength, lastFormatIdx);

            lastFormatIdx = formatIndex;
            lastTextPosition += textLength;
            textLength = 0;
        }

        textLength += frag->size;
    }

    Q_ASSERT(lastFormatIdx != -1);
    setFormat(lastTextPosition, textLength, lastFormatIdx);
}

/*!
    Destructs the layout.
*/
QTextLayout::~QTextLayout()
{
    delete d;
}

// ####### go away!
/*!
  \internal
*/
void QTextLayout::setText(const QString& string, const QFont& fnt)
{
    delete d;
    d = new QTextEngine((string.isNull() ? (const QString&)QString::fromLatin1("") : string), fnt.d);
}

/*!
  \internal
*/
void QTextLayout::setFormatCollection(const QTextFormatCollection *formats)
{
    d->setFormatCollection(formats);
}

// ### DOC: How is it laid out again? Do they call draw() or what?
// Same as initially, see the code snipplet in the class overview.
/*!
    Sets the layout's text to the given \a string. The layout is
    invalidated and must be laid out again.

    \sa text()
*/
void QTextLayout::setText(const QString& string)
{
    d->setText(string);
}

/*!
    Returns the layout's text.

    \sa setText()
*/
QString QTextLayout::text() const
{
    return d->string;
}

/*!
  \internal
*/
void QTextLayout::setDocumentLayout(QAbstractTextDocumentLayout *layout)
{
    d->setDocumentLayout(layout);
}

/*!
  \internal
*/
void QTextLayout::setFormat(int from, int length, int format)
{
    if (d->items.size() == 0)
        d->itemize(QTextEngine::Full);
    d->setFormat(from, length, format);
}

/*!
  \internal
*/
void QTextLayout::setTextFlags(int textFlags)
{
    d->textFlags = textFlags;
}

/*!
    If \a b is true then the layout will use design metrics for its
    layout; otherwise it will use the metrics of the paint device
    (which is the default behavior).

    \sa usesDesignMetrics()
*/
void QTextLayout::useDesignMetrics(bool b)
{
    d->designMetrics = b;
}

/*!
    Returns true if this layout uses design rather than device
    metrics; otherwise returns false.

    \sa useDesignMetrics()
*/
bool QTextLayout::usesDesignMetrics() const
{
    return d->designMetrics;
}

/*!
  \internal
*/
void QTextLayout::setPalette(const QPalette &p, PaletteFlags f)
{
    if (!d->pal)
        d->pal = new QPalette(p);
    else
        *d->pal = p;
    d->textColorFromPalette = (f & UseTextColor);
}

/*!
  \internal
*/
void QTextLayout::beginLayout(LayoutMode m, int textFlags)
{
    d->items.clear();
    QTextEngine::Mode mode = QTextEngine::Full;
    if (m == NoBidi)
        mode = QTextEngine::NoBidi;
    else if (m == SingleLine)
        mode = QTextEngine::SingleLine;
    d->itemize(mode);
    d->textFlags = textFlags;
}

/*!
    Returns the next valid cursor position after \a oldPos that
    respects the given cursor \a mode.

    \sa validCursorPosition() previousCursorPosition()
*/
int QTextLayout::nextCursorPosition(int oldPos, CursorMode mode) const
{
//     qDebug("looking for next cursor pos for %d", oldPos);
    const QCharAttributes *attributes = d->attributes();
    int len = d->string.length();
    if (oldPos >= len)
        return oldPos;
    oldPos++;
    if (mode == SkipCharacters) {
        while (oldPos < len && !attributes[oldPos].charStop)
            oldPos++;
    } else {
        while (oldPos < len && !attributes[oldPos].wordStop && !attributes[oldPos-1].whiteSpace)
            oldPos++;
    }
//     qDebug("  -> %d",  oldPos);
    return oldPos;
}

/*!
    Returns the first valid cursor position before \a oldPos that
    respects the given cursor \a mode.

    \sa validCursorPosition() nextCursorPosition()
*/
int QTextLayout::previousCursorPosition(int oldPos, CursorMode mode) const
{
//     qDebug("looking for previous cursor pos for %d", oldPos);
    const QCharAttributes *attributes = d->attributes();
    if (oldPos <= 0)
        return 0;
    oldPos--;
    if (mode == SkipCharacters) {
        while (oldPos && !attributes[oldPos].charStop)
            oldPos--;
    } else {
        while (oldPos && !attributes[oldPos].wordStop && !attributes[oldPos-1].whiteSpace)
            oldPos--;
    }
//     qDebug("  -> %d",  oldPos);
    return oldPos;
}

/*!
    Returns true if position \a pos is a valid cursor position.

    In a Unicode context some positions in the text are not valid
    cursor positions, because the position is inside a Unicode
    surrogate or a grapheme cluster.

    A grapheme cluster is a sequence of two or more Unicode characters
    that form one indivisible entity on the screen. For example the
    latin character `Ä' can be represented in Unicode by two
    characters, `A' (0x41), and the combining diaresis (0x308). A text
    cursor can only validly be positioned before or after these two
    characters, never between them since that wouldn't make sense. In
    indic languages every syllable forms a grapheme cluster.
*/
bool QTextLayout::validCursorPosition(int pos) const
{
    const QCharAttributes *attributes = d->attributes();
    if (pos < 0 || pos > (int)d->string.length())
        return false;
    return attributes[pos].charStop;
}


/*!
    Clears the layout information stored in the layout, and begins a
    new layout process.
*/
void QTextLayout::clearLines()
{
    d->lines.clear();
    // invalidate bounding rect
    d->boundingRect = QRect();
}

// ### DOC: Don't know what this really does.
// added a bit more description
/*!
    Creates a new text line to be laid out.

    The text layout creates a new line object that starts after the
    last layouted line (or at the beginning of the contained text if
    no line has been layouted up to now).

    The line is still empty and you need to call layout() on the line
    to fill it with text. After the layout() call a new line can be
    created and filled again. Repeating this process will layout the
    whole block of text contained in the QTextLayout. If there is no
    text left to be layouted, the retuned QTextLine will not be valid
    (ie. isValid() will return false).
*/
QTextLine QTextLayout::createLine()
{
    int l = d->lines.size();
    int from = l > 0 ? d->lines.at(l-1).from + d->lines.at(l-1).length : 0;
    if (l && from >= d->string.length())
        return QTextLine();

    QScriptLine line;
    line.from = from;
    line.length = 0;
    line.justified = false;
    line.gridfitted = false;

    d->lines.append(line);
    return QTextLine(l, d);
}

/*!
    Returns the number of lines in this text layout.

    \sa lineAt()
*/
int QTextLayout::numLines() const
{
    return d->lines.size();
}

/*!
    Returns the \a{i}-th line of text in this text layout.

    \sa numLines() findLine()
*/
QTextLine QTextLayout::lineAt(int i) const
{
    return QTextLine(i, d);
}

/*!
    Returns the line that contains the cursor position \a pos.

    \sa validCursorPosition() lineAt()
*/
QTextLine QTextLayout::findLine(int pos) const
{
    for (int i = 0; i < d->lines.size(); ++i) {
        const QScriptLine& line = d->lines[i];
        if (line.from + (int)line.length > pos)
            return QTextLine(i, d);
    }
    if (pos == d->string.length() && d->lines.size())
        return QTextLine(d->lines.size()-1, d);
    return QTextLine();
}

/*!
    The global position of the layout. This is independent of the
    bounding rectangle and of the layout process.

    \sa setPosition()
*/
QPoint QTextLayout::position() const
{
    return d->position;
}

/*!
    Moves the text layout to point \a p.

    \sa position()
*/
void QTextLayout::setPosition(const QPoint &p)
{
    d->position = p;
}

/*!
    The smallest rectangle that contains all the lines in the layout.
*/
QRect QTextLayout::boundingRect() const
{
    if (!d->boundingRect.isValid()) {
        Q26Dot6 xmin, xmax, ymin, ymax;
        for (int i = 0; i < d->lines.size(); ++i) {
            const QScriptLine &si = d->lines[i];
            xmin = qMin(xmin, si.x);
            ymin = qMin(ymin, si.y);
            xmax = qMax(xmax, si.x+si.width);
            // ### shouldn't the ascent be used in ymin???
            ymax = qMax(ymax, si.y+si.ascent+si.descent);
        }
        d->boundingRect = QRect(xmin.toInt(), ymin.toInt(), (xmax-xmin).toInt(), (ymax-ymin).toInt());
    }
    return d->boundingRect;
}

/*!
  \internal
*/
QRect QTextLayout::rect() const
{
    QRect r = boundingRect();
    r.moveBy(d->position);
    return r;
}

/*!
    The minimum width the layout needs. This is the width of the
    layout's smallest non-breakable sub-string.

    \warning This function only returns a valid value after the layout
    has been done.

    \sa maximumWidth()
*/
int QTextLayout::minimumWidth() const
{
    return d->minWidth.toInt();
}

/*!
    The maximum width the layout could expand to; this is essentially
    the width of the entire text.

    \warning This function only returns a valid value after the layout
    has been done.

    \sa minimumWidth()
*/
int QTextLayout::maximumWidth() const
{
    return d->maxWidth.toInt();
}

static void drawSelection(QPainter *p, QPalette *pal, QTextLayout::SelectionType type,
                          const QRect &rect, const QTextLine &line, const QPoint &pos, int selectionIdx)
{
    p->save();
    p->setClipRect(rect);
    QColor bg;
    QColor text;
    switch(type) {
    case QTextLayout::Highlight:
        bg = pal->highlight();
        text = pal->highlightedText();
        break;
    case QTextLayout::ImText:
        int h1, s1, v1, h2, s2, v2;
        pal->color(QPalette::Base).getHsv(&h1, &s1, &v1);
        pal->color(QPalette::Background).getHsv(&h2, &s2, &v2);
        bg.setHsv(h1, s1, (v1 + v2) / 2);
        break;
    case QTextLayout::ImSelection:
        bg = pal->text();
        text = pal->background();
        break;
    case QTextLayout::NoSelection:
        Q_ASSERT(false); // should never happen.
        return;
    }
    p->fillRect(rect, bg);
    if (text.isValid())
        p->setPen(text);
    line.draw(p, pos.x(), pos.y(), selectionIdx);
    p->restore();
    return;
}

/*!
    \fn void QTextLayout::draw(QPainter *p, const QPoint &pos, int cursorPos, const QVector<Selection> &selections) const

    Draws the whole layout on painter \a p at point \a pos with the
    given \a cursorPos with the given \a selections (which may be
    empty).
*/

/*!
    \internal

    The number of selections (which may be 0) is given by \a
    nSelections, and the selections themselves are passed in \a
    selections.
*/
void QTextLayout::draw(QPainter *p, const QPoint &pos, int cursorPos, const Selection *selections, int nSelections, const QRect &cr) const
{
    Q_ASSERT(numLines() != 0);

    d->cursorPos = cursorPos;
    d->selections = selections;
    d->nSelections = nSelections;

    QPoint position = pos + d->position;

    int clipy = INT_MIN;
    int clipe = INT_MAX;
    if (cr.isValid()) {
        clipy = cr.y() - position.y();
        clipe = clipy + cr.height();
    }

    for (int i = 0; i < d->lines.size(); i++) {
        QTextLine l(i, d);
        const QScriptLine &sl = d->lines[i];

        if (sl.y.toInt() > clipe || (sl.y + sl.ascent + sl.descent).toInt() < clipy)
            continue;

        int from = sl.from;
        int length = sl.length;

        l.draw(p, position.x(), position.y());
        if (selections) {
            for (int j = 0; j < nSelections; ++j) {
                const Selection &s = selections[j];
                if (s.type() != Highlight)
                    continue;
                if (!d->pal)
                    continue;

                if (s.from() + s.length() > from && s.from() < from+length) {
                    QRect highlight = QRect(QPoint(position.x() + l.cursorToX(qMax(s.from(), from)),
                                                   position.y() + sl.y.toInt()),
                                            QPoint(position.x() + l.cursorToX(qMin(s.from() + s.length(), from+length)) - 1,
                                                   position.y() + (sl.y + sl.ascent + sl.descent).toInt())).normalize();
                    drawSelection(p, d->pal, (QTextLayout::SelectionType)s.type(), highlight, l, position, j);
                }
            }
        }
        if (sl.from <= cursorPos && sl.from + (int)sl.length >= cursorPos) {
            int x = l.cursorToX(cursorPos);
            p->drawLine(position.x() + x, position.y() + sl.y.toInt(), position.x() + x, position.y() + (sl.y + sl.ascent + sl.descent).toInt());
        }
    }

    d->cursorPos = -1;
    d->selections = 0;
    d->nSelections = 0;
}


/*!
    \class QTextLine
    \brief The QTextLine class represents a line of text inside a QTextLayout.

    \ingroup text

    A text line is usually created by QTextLayout::createLine().

    After being created, the line() can be filled using the layout()
    function. A line has a number of attributes including the
    rectangle it occupies, rect(), its coordinates, x() and y(), its
    length(), width() and textWidth(), and its ascent() and decent()
    relative to the text. The position of the cursor in terms of the
    line is available from cursorToX() and its inverse from
    xToCursor(). A line can be moved with setPosition().
*/

/*!
    \enum QTextLine::Edge

    \value Leading
    \value Trailing
*/

/*!
    \enum QTextLine::LineWidthUnit

    \value UnitIsPixels
    \value UnitIsGlyphs
*/

/*!
    \enum QTextLine::CursorPosition

    \value CursorBetweenCharacters
    \value CursorOnCharacter
*/

/*!
    \fn QTextLine::QTextLine(int line, QTextEngine *e)
    \internal

    Constructs a new text line using the line at position \a line in
    the text engine \a e.
*/

/*!
    \fn QTextLine::QTextLine()

    Creates an invalid line.
*/

/*!
    \fn bool QTextLine::isValid() const

    Returns true if this text line is valid; otherwise returns false.
*/

/*!
    \fn QTextEngine *QTextLine::engine() const
  \internal

    Returns the text line's text engine.
*/

/*!
    \fn int QTextLine::line() const

    Returns the position of the line in the text engine.
*/


/*!
    Returns the line's bounding rectangle.

    \sa x() y() length() width()
*/
QRect QTextLine::rect() const
{
    const QScriptLine& si = eng->lines[i];
    return QRect(si.x.toInt(), si.y.toInt(), si.width.toInt(), (si.ascent + si.descent).toInt());
}

/*!
    Returns the line's x position.

    \sa rect() y() length() width()
*/
int QTextLine::x() const
{
    return eng->lines[i].x.toInt();
}

/*!
    Returns the line's y position.

    \sa x() rect() length() width()
*/
int QTextLine::y() const
{
    return eng->lines[i].y.toInt();
}

/*!
    Returns the line's width as specified by the layout() function.

    \sa textWidth() x() y() length() rect()
*/
int QTextLine::width() const
{
    return eng->lines[i].width.toInt();
}


/*!
    Returns the line's ascent.

    \sa descent()
*/
int QTextLine::ascent() const
{
    return eng->lines[i].ascent.toInt();
}

/*!
    Returns the line's descent.

    \sa ascent()
*/
int QTextLine::descent() const
{
    return eng->lines[i].descent.toInt();
}

/*!
    Returns the width of the line that is occupied by text. This is
    always \<= to width(), and is the minimum width that could be used
    by layout() without changing the line break position.
*/
int QTextLine::textWidth() const
{
    return eng->lines[i].textWidth.toInt();
}

/*!
    Lays out the line with the given \a width. The line is filled from
    it's starting position with as many characters as will fit into
    the line. Depending on the specified \a unit the width is either
    interpreted in pixels (default) or in number of visible glyphs.
*/
void QTextLine::layout(int width, LineWidthUnit unit)
{
    int maxGlyphs = INT_MAX;
    if (unit == UnitIsGlyphs) {
        maxGlyphs = width;
        width = INT_MAX >> 6;
    }

    eng->boundingRect = QRect();

    QScriptLine &line = eng->lines[i];
    line.width = width;
    line.length = 0;
    line.textWidth = 0;

    if (!eng->items.size()) {
        QFont f;
        QFontEngine *e;

        if (eng->fnt) {
            e = eng->fnt->engineForScript(QFont::Latin);
        } else {
            f = eng->block.charFormat().font();
            e = f.d->engineForScript(QFont::Latin);
        }

        line.ascent = e->ascent();
        line.descent = e->descent();
        return;
    }

    Q_ASSERT(line.from < eng->string.length());

    bool breakany = eng->textFlags & Qt::TextWrapAnywhere;

    // #### binary search!
    int item;
    for (item = eng->items.size()-1; item > 0; --item) {
        if (eng->items[item].position <= line.from)
            break;
    }

    Q26Dot6 minw, spacew;
    int glyphCount = 0;

//     qDebug("from: %d:   item=%d, total %d width available %d/%d", line.from, item, eng->items.size(), line.width.value(), line.width.toInt());

    while (item < eng->items.size()) {
        eng->shape(item);
        const QCharAttributes *attributes = eng->attributes();
        const QScriptItem &current = eng->items[item];
        line.ascent = qMax(line.ascent, current.ascent);
        line.descent = qMax(line.descent, current.descent);

        if (current.isObject) {
            QTextFormat format = eng->formats->format(eng->items[item].format);
            if (eng->docLayout)
                eng->docLayout->layoutObject(QTextInlineObject(item, eng), format);
            if (line.length && !(eng->textFlags & Qt::TextSingleLine)) {
                if (line.textWidth + current.width > line.width || glyphCount > maxGlyphs)
                    goto found;
            }

            line.length++;
            // the width of the linesep doesn't count into the textwidth
            if (eng->string[current.position] == QChar::LineSeparator)
                goto found;
            line.textWidth += current.width;

            ++item;
            ++glyphCount;
            continue;
        }

        int length = eng->length(item);

        const QCharAttributes *itemAttrs = attributes + current.position;
        QGlyphLayout *glyphs = eng->glyphs(&current);
        unsigned short *logClusters = eng->logClusters(&current);

        int pos = qMax(0, line.from - current.position);

        do {
            int next = pos;

            Q26Dot6 tmpw;
            if (!itemAttrs[next].whiteSpace) {
                tmpw = spacew;
                spacew = 0;
                do {
                    int gp = logClusters[next];
                    do {
                        ++next;
                    } while (next < length && logClusters[next] == gp);
                    do {
                        tmpw += glyphs[gp].advance.x;
                        ++gp;
                    } while (gp < current.num_glyphs && !glyphs[gp].attributes.clusterStart);

                    Q_ASSERT((next == length && gp == current.num_glyphs) || logClusters[next] == gp);

                    ++glyphCount;
                } while (next < length && !itemAttrs[next].whiteSpace && !itemAttrs[next].softBreak && !(breakany && itemAttrs[next].charStop));
                minw = qMax(tmpw, minw);
            }

            if (itemAttrs[next].softBreak)
                breakany = false;

            while (next < length && itemAttrs[next].whiteSpace) {
                int gp = logClusters[next];
                do {
                    ++next;
                } while (next < length && logClusters[next] == gp);
                do {
                    spacew += glyphs[gp].advance.x;
                    ++gp;
                } while (gp < current.num_glyphs && !glyphs[gp].attributes.clusterStart);

                ++glyphCount;
                Q_ASSERT((next == length && gp == current.num_glyphs) || logClusters[next] == gp);
            }

//             qDebug("possible break at %d, chars (%d-%d) / glyphs (%d-%d): width %d, spacew=%d",
//                    current.position + next, pos, next, logClusters[pos], logClusters[next], tmpw.value(), spacew.value());

            if (line.length && tmpw.value()
                && (line.textWidth + tmpw > line.width || glyphCount > maxGlyphs)
                && !(eng->textFlags & Qt::TextSingleLine))
                goto found;

            line.textWidth += tmpw;
            line.length += next - pos;

            pos = next;
        } while (pos < length);
        ++item;
    }
 found:
//     qDebug("line length = %d, ascent=%d, descent=%d, textWidth=%d (%d)", line.length, line.ascent.value(),
//            line.descent.value(), line.textWidth.value(), line.textWidth.toInt());
//     qDebug("        : '%s'", eng->string.mid(line.from, line.length).utf8());

    eng->minWidth = qMax(eng->minWidth, minw);
    eng->maxWidth += line.textWidth + spacew;
    // ##########################
}

/*!
    Moves the line to position \a pos.
*/
void QTextLine::setPosition(const QPoint &pos)
{
    eng->lines[i].x = pos.x();
    eng->lines[i].y = pos.y();
}

// ### DOC: I have no idea what this means/does.
// You create a text layout with a string of text. Once you layouted
// it, it contains a number of QTextLines. from() returns the position
// inside the text string where this line starts. If you e.g. has a
// text of "This is a string", layouted into two lines (the second
// starting at the word 'a'), layout.lineAt(0).from() == 0 and
// layout.lineAt(1).from() == 8.
/*!
    Returns the start of the line from the beginning of the string
    passed to the QTextLayout.
*/
int QTextLine::from() const
{
    return eng->lines[i].from;
}

/*!
    Returns the length of the text in the line.

    \sa textWidth()
*/
int QTextLine::length() const
{
    return eng->lines[i].length;
}

// ### DOC: You can't draw a line with only one point, so how does this work?
// You don't draw the line _with_ one point, but _at_ one point. It's
// similar to drawText(int x, int y, const QString &str).
/*!
    \internal

    Draws a line on painter \a p at position \a xpos, \a ypos. \a
    selection is reserved for internal use.
*/
void QTextLine::draw(QPainter *p, int xpos, int ypos, int selection) const
{
    const QScriptLine &line = eng->lines[i];

    if (!line.length)
        return;

    int lineEnd = line.from + line.length;
    // don't draw trailing spaces or take them into the layout.
    const QCharAttributes *attributes = eng->attributes();
    while (lineEnd > line.from && attributes[lineEnd-1].whiteSpace)
        --lineEnd;

    int firstItem = eng->findItem(line.from);
    int lastItem = eng->findItem(lineEnd - 1);
    int nItems = lastItem-firstItem+1;

    Q26Dot6 x(xpos);
    Q26Dot6 y(ypos);
    x += line.x;
    y += line.y + line.ascent;

    eng->justify(line);
    if (eng->textFlags & Qt::AlignRight)
        x += line.width - line.textWidth;
    else if (eng->textFlags & Qt::AlignHCenter)
        x += (line.width - line.textWidth)/2;

    QVarLengthArray<int> visualOrder(nItems);
    QVarLengthArray<uchar> levels(nItems);
    for (int i = 0; i < nItems; ++i)
        levels[i] = eng->items[i+firstItem].analysis.bidiLevel;
    QTextEngine::bidiReorder(nItems, levels.data(), visualOrder.data());

    QFont f = eng->font();
    for (int i = 0; i < nItems; ++i) {
        int item = visualOrder[i]+firstItem;
        QScriptItem &si = eng->items[item];

        if (si.isObject && eng->docLayout && eng->formats) {
            QTextFormat format = eng->formats->format(si.format);

            QTextLayout::SelectionType selType = QTextLayout::NoSelection;
            if (selection >= 0 && eng->selections && eng->nSelections > 0)
                // ###
                selType = static_cast<QTextLayout::SelectionType>(eng->selections[selection].type());

            eng->docLayout->drawObject(p, QRect(x.toInt(), (y-si.ascent).toInt(), si.width.toInt(), (si.ascent+si.descent).toInt()),
                                       QTextInlineObject(item, eng), format, selType);
        }

        if (si.isTab || si.isObject) {
            x += si.width;
            continue;
        }
        unsigned short *logClusters = eng->logClusters(&si);
        QGlyphLayout *glyphs = eng->glyphs(&si);

        int start = qMax(line.from, si.position);
        int gs = logClusters[start-si.position];
        int end;
        int ge;
        if (lineEnd < si.position + eng->length(item)) {
            end = lineEnd;
            ge = logClusters[end-si.position];
        } else {
            end = si.position + eng->length(item);
            ge = si.num_glyphs;
        }

        if (eng->formats) {
            QTextFormat fmt = eng->formats->format(si.format);
            Q_ASSERT(fmt.isCharFormat());
            QTextCharFormat chf = fmt.toCharFormat();
            if (selection == -1) {
                QColor c = chf.color();
                if (!c.isValid() && eng->textColorFromPalette) {
                    c = eng->pal->color(QPalette::Text);
                }
                p->setPen(c);
            }
            f = chf.font();
        }
        QFontEngine *fe = f.d->engineForScript((QFont::Script)si.analysis.script);
        Q_ASSERT(fe);

        QTextItem gf;
        gf.right_to_left = (si.analysis.bidiLevel % 2);
        gf.ascent = si.ascent.toInt();
        gf.descent = si.descent.toInt();
        gf.num_glyphs = ge - gs + 1;
        gf.glyphs = glyphs + gs;
        gf.fontEngine = fe;
        gf.chars = eng->string.unicode() + start;
        gf.num_chars = end - start;
        int textFlags = 0;
        if (f.d->underline) textFlags |= Qt::TextUnderline;
        if (f.d->overline) textFlags |= Qt::TextOverline;
        if (f.d->strikeOut) textFlags |= Qt::TextStrikeOut;

        int *ul = eng->underlinePositions;
        if (ul)
            while (*ul != -1 && *ul < start)
                ++ul;
        do {
            int gtmp = ge;
            int stmp = end;
            if (ul && *ul != -1 && *ul < end) {
                stmp = *ul;
                gtmp = logClusters[*ul-si.position];
            }

            gf.num_glyphs = gtmp - gs;
            gf.glyphs = glyphs + gs;
            gf.num_chars = stmp - start;
            gf.chars = eng->string.unicode() + start;
            Q26Dot6 w;
            while (gs < gtmp) {
                w += glyphs[gs].advance.x + Q26Dot6(glyphs[gs].space_18d6, F26Dot6);
                ++gs;
            }
            start = stmp;
            gf.width = w.toInt();
            if (gf.num_chars)
                p->drawTextItem(QPoint(x.toInt(), y.toInt()), gf, textFlags);
            x += w;
            if (ul && *ul != -1 && *ul < end) {
                // draw underline
                gtmp = (*ul == end-1) ? ge : logClusters[*ul+1-si.position];
                ++stmp;
                gf.num_glyphs = gtmp - gs;
                gf.glyphs = glyphs + gs;
                gf.num_chars = stmp - start;
                gf.chars = eng->string.unicode() + start;
                w = 0;
                while (gs < gtmp) {
                    w += glyphs[gs].advance.x + Q26Dot6(glyphs[gs].space_18d6, F26Dot6);
                    ++gs;
                }
                ++start;
                gf.width = w.toInt();
                p->drawTextItem(QPoint(x.toInt(), y.toInt()), gf, (textFlags ^ Qt::TextUnderline));
                ++gf.chars;
                x += w;
                ++ul;
            }
        } while (gs < ge);

    }
}

/*!
    \fn int QTextLine::cursorToX(int cursorPos, Edge edge) const

    \overload
*/


/*!
    Converts the cursor position \a cursorPos to the corresponding x position
    inside the line, taking account of the \a edge.

    If \a cursorPos is not a valid cursor position, the nearest valid
    cursor position will be used instead, and cpos will be modified to
    point to this valid cursor position.

    \sa xToCursor()
*/
int QTextLine::cursorToX(int *cursorPos, Edge edge) const
{
    if (!i && !eng->items.size()) {
        *cursorPos = 0;
        return eng->lines[0].x.toInt();
    }

    int pos = *cursorPos;

    int itm = eng->findItem(pos);

    const QScriptLine &line = eng->lines[i];
    if (pos == line.from + (int)line.length) {
        // end of line ensure we have the last item on the line
        itm = eng->findItem(pos-1);
    }

    const QScriptItem *si = &eng->items[itm];
    pos -= si->position;

    eng->shape(itm);
    QGlyphLayout *glyphs = eng->glyphs(si);
    unsigned short *logClusters = eng->logClusters(si);

    int l = eng->length(itm);
    if (pos > l)
        pos = l;
    if (pos < 0)
        pos = 0;

    int glyph_pos = pos == l ? si->num_glyphs : logClusters[pos];
    if (edge == Trailing) {
        // trailing edge is leading edge of next cluster
        while (glyph_pos < si->num_glyphs && !glyphs[glyph_pos].attributes.clusterStart)
            glyph_pos++;
    }

    Q26Dot6 x;
    bool reverse = eng->items[itm].analysis.bidiLevel % 2;

    if (reverse) {
        for (int i = si->num_glyphs-1; i >= glyph_pos; i--)
            x += glyphs[i].advance.x + Q26Dot6(glyphs[i].space_18d6, F26Dot6);
    } else {
        for (int i = 0; i < glyph_pos; i++)
            x += glyphs[i].advance.x + Q26Dot6(glyphs[i].space_18d6, F26Dot6);
    }

    // add the items left of the cursor

    int lineEnd = line.from + line.length;
//     // don't draw trailing spaces or take them into the layout.
//     const QCharAttributes *attributes = eng->attributes();
//     while (lineEnd > line.from && attributes[lineEnd-1].whiteSpace)
//         --lineEnd;

    int firstItem = eng->findItem(line.from);
    int lastItem = eng->findItem(lineEnd - 1);
    int nItems = lastItem-firstItem+1;

    x += line.x;

    eng->justify(line);
    if (eng->textFlags & Qt::AlignRight)
        x += line.width - line.textWidth;
    else if (eng->textFlags & Qt::AlignHCenter)
        x += (line.width - line.textWidth)/2;

    QVarLengthArray<int> visualOrder(nItems);
    QVarLengthArray<uchar> levels(nItems);
    for (int i = 0; i < nItems; ++i)
        levels[i] = eng->items[i+firstItem].analysis.bidiLevel;
    QTextEngine::bidiReorder(nItems, levels.data(), visualOrder.data());

    for (int i = 0; i < nItems; ++i) {
        int item = visualOrder[i]+firstItem;
        if (item == itm)
            break;
        QScriptItem &si = eng->items[item];

        if (si.isTab || si.isObject) {
            x += si.width;
            continue;
        }
        int start = qMax(line.from, si.position);
        int end = qMin(lineEnd, si.position + eng->length(item));

        unsigned short *logClusters = eng->logClusters(&si);

        int gs = logClusters[start-si.position];
        int ge = logClusters[end-si.position-1];

        QGlyphLayout *glyphs = eng->glyphs(&si);

        while (gs <= ge) {
            x += glyphs[gs].advance.x + Q26Dot6(glyphs[gs].space_18d6, F26Dot6);
            ++gs;
        }
    }

    *cursorPos = pos + si->position;
    return x.toInt();
}

/*!
    Converts the x-coordinate \a xpos, to the nearest matching cursor
    position, depending on the cursor position type, \a cpos.

    \sa cursorToX()
*/
int QTextLine::xToCursor(int xpos, CursorPosition cpos) const
{
    const QScriptLine &line = eng->lines[i];

    if (!line.length)
        return line.from;

    Q26Dot6 x(xpos);

    int line_length = line.length;
    // don't draw trailing spaces or take them into the layout.
    const QCharAttributes *a = eng->attributes() + line.from;
    while (line_length && a[line_length-1].whiteSpace)
        --line_length;

    int firstItem = eng->findItem(line.from);
    int lastItem = eng->findItem(line.from + line_length - 1);
    int nItems = lastItem-firstItem+1;

    x -= line.x;

    eng->justify(line);
    if (eng->textFlags & Qt::AlignRight)
        x -= line.width - line.textWidth;
    else if (eng->textFlags & Qt::AlignHCenter)
        x -= (line.width - line.textWidth)/2;

    QVarLengthArray<int> visualOrder(nItems);
    QVarLengthArray<unsigned char> levels(nItems);
    for (int i = 0; i < nItems; ++i)
        levels[i] = eng->items[i+firstItem].analysis.bidiLevel;
    QTextEngine::bidiReorder(nItems, levels.data(), visualOrder.data());

    int gl_before = 0;
    int gl_after = 0;
    int it_before = 0;
    int it_after = 0;
    Q26Dot6 x_before(0xffffff);
    Q26Dot6 x_after(0xffffff);


    for (int i = 0; i < nItems; ++i) {
        int item = visualOrder[i]+firstItem;
        QScriptItem &si = eng->items[item];
        int item_length = eng->length(item);

        if (si.isTab || si.isObject) {
            x -= si.width;
            continue;
        }
        int start = qMax(line.from - si.position, 0);
        int end = qMin(line.from + line_length - si.position, item_length);

        unsigned short *logClusters = eng->logClusters(&si);

        int gs = logClusters[start];
        int ge = (end == item_length ? si.num_glyphs : logClusters[end]) - 1;
        QGlyphLayout *glyphs = eng->glyphs(&si);

        if (si.analysis.bidiLevel %2) {
            Q26Dot6 item_width;
            int g = gs;
            while (g <= ge) {
                item_width += glyphs[g].advance.x + Q26Dot6(glyphs[g].space_18d6, F26Dot6);
                ++g;
            }

            x -= item_width;
            if (x > 0) {
                gl_before = gs;
                it_before = item;
                x_before = x;
                continue;
            }

            while (1) {
                if (glyphs[gs].attributes.clusterStart) {
                    if (x < 0) {
                        gl_after = gs;
                        it_after = item;
                        x_after = -x;
                    } else {
                        gl_before = gs;
                        it_before = item;
                        x_before = x;
                        goto end;
                    }
                }
                if (gs > ge)
                    Q_ASSERT(false);
                x += glyphs[gs].advance.x + Q26Dot6(glyphs[gs].space_18d6, F26Dot6);
                ++gs;
            }
        } else {
            while (1) {
                if (glyphs[gs].attributes.clusterStart) {
                    if (x > 0) {
                        gl_before = gs;
                        it_before = item;
                        x_before = x;
                    } else {
                        gl_after = gs;
                        it_after = item;
                        x_after = -x;
                        goto end;
                    }
                }
                if (gs > ge) {
                    gl_before = gs;
                    it_before = item;
                    x_before = x;
                    break;
                }
                x -= glyphs[gs].advance.x + Q26Dot6(glyphs[gs].space_18d6, F26Dot6);
                ++gs;
            }
        }
    }

 end:

    int item;
    int glyph;
    if (cpos == CursorOnCharacter || x_before < x_after) {
        item = it_before;
        glyph = gl_before;
    } else {
        item = it_after;
        glyph = gl_after;
    }

    // find the corresponding cursor position
    const QScriptItem &si = eng->items[item];
    unsigned short *logClusters = eng->logClusters(&si);
    int j;
    for (j = 0; j < eng->length(item); ++j)
        if (logClusters[j] == glyph)
            break;
    return si.position + j;
}
