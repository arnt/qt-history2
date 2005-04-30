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
#include "qstyleoption.h"
#include <limits.h>

#include <qdebug.h>

#include "qfontengine_p.h"

static qreal alignLine(QTextEngine *eng, const QScriptLine &line)
{
    qreal x = 0;
    eng->justify(line);
    if (!line.justified) {
        int align = eng->option.alignment();
        if (align & Qt::AlignJustify && eng->option.textDirection() == Qt::RightToLeft)
            align = Qt::AlignRight;
        if (align & Qt::AlignRight)
            x = line.width - line.textWidth;
        else if (align & Qt::AlignHCenter)
            x = (line.width - line.textWidth)/2;
    }
    return x;
}

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
    Returns the inline object's rectangle.

    \sa ascent() descent() width()
*/
QRect QTextInlineObject::rect() const
{
    QScriptItem& si = eng->layoutData->items[itm];
    return QRect(0, qRound(-si.ascent), qRound(si.width), qRound(si.height()));
}

/*!
    Returns the inline object's width.

    \sa ascent() descent() rect()
*/
qreal QTextInlineObject::width() const
{
    return eng->layoutData->items[itm].width;
}

/*!
    Returns the inline object's ascent.

    \sa descent() width() rect()
*/
qreal QTextInlineObject::ascent() const
{
    return eng->layoutData->items[itm].ascent;
}

/*!
    Returns the inline object's descent.

    \sa ascent() width() rect()
*/
qreal QTextInlineObject::descent() const
{
    return eng->layoutData->items[itm].descent;
}

/*!
    Returns the inline object's total height. This is equal to
    ascent() + descent() + 1.

    \sa ascent() descent() width() rect()
*/
qreal QTextInlineObject::height() const
{
    return eng->layoutData->items[itm].height();
}


/*!
    Sets the inline object's width to \a w.

    \sa width() ascent() descent() rect()
*/
void QTextInlineObject::setWidth(qreal w)
{
    eng->layoutData->items[itm].width = w;
}

/*!
    Sets the inline object's ascent to \a a.

    \sa ascent() setDescent() width() rect()
*/
void QTextInlineObject::setAscent(qreal a)
{
    eng->layoutData->items[itm].ascent = a;
}

/*!
    Sets the inline object's decent to \a d.

    \sa descent() setAscent() width() rect()
*/
void QTextInlineObject::setDescent(qreal d)
{
    eng->layoutData->items[itm].descent = d;
}

/*!
  The position of the inline object within the text layout.
*/
int QTextInlineObject::textPosition() const
{
    return eng->layoutData->items[itm].position;
}

/*!
  Returns an integer describing the format of the inline object
  within the text layout.
*/
int QTextInlineObject::formatIndex() const
{
    return eng->formatIndex(&eng->layoutData->items[itm]);
}

/*!
  Returns format of the inline object within the text layout.
*/
QTextFormat QTextInlineObject::format() const
{
    if (!eng->block.docHandle())
        return QTextFormat();
    return eng->formats()->format(eng->formatIndex(&eng->layoutData->items[itm]));
}

/*!
  Returns if the object should be laid out right-to-left or left-to-right.
*/
Qt::LayoutDirection QTextInlineObject::textDirection() const
{
    return (eng->layoutData->items[itm].analysis.bidiLevel % 2 ? Qt::RightToLeft : Qt::LeftToRight);
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
    implement your own text rendering for some specialized widget, you
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
        textLayout.beginLayout();
        while (1) {
            QTextLine line = textLayout.createLine();
            if (!line.isValid())
                break;

            line.layout(lineWidth);
            height += leading;
            line.setPosition(QPoint(0, height));
            height += line.height();
            widthUsed = qMax(widthUsed, line.naturalTextWidth());
        }
        textLayout.endLayout();
    \endcode

    And here's some pseudo code that presents the painting phase:
    \code
        for (int i = 0; i < textLayout.lineCount(); ++i) {
            QTextLine line = textLayout.lineAt(i);
            line.draw(painter, rect.x() + xoffset + line.x(), rect.y() + yoffset);
        }
    \endcode

    The text layout's text is set in the constructor or with
    setText(). The layout can be seen as a sequence of QTextLine
    objects; use lineAt() or lineForTextPosition() to get a QTextLine,
    createLine() to create one. For a given position in the text you
    can find a valid cursor position with isValidCursorPosition(),
    nextCursorPosition(), and previousCursorPosition(). The layout
    itself can be positioned with setPosition(); it has a
    boundingRect(), and a minimumWidth() and a maximumWidth(). A text
    layout can be drawn on a painter device using draw().

*/

/*!
    \enum QTextLayout::CursorMode

    \value SkipCharacters
    \value SkipWords
*/

/*!
    \fn QTextEngine *QTextLayout::engine() const
    \internal

    Returns the text engine used to render the text layout.
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
    Constructs a text layout to lay out the given \a string with the specified
    \a font.

    All the metric and layout calculations will be done in terms of
    the paint device, \a paintdevice. If \a paintdevice is 0 the
    calculations will be done in screen metrics.
*/
QTextLayout::QTextLayout(const QString& string, const QFont &font, QPaintDevice *paintdevice)
{
    QFontPrivate *f = paintdevice ? QFont(font, paintdevice).d : font.d;
    d = new QTextEngine((string.isNull() ? (const QString&)QString::fromLatin1("") : string), f);
}

/*!
    Constructs a text layout to lay out the given \a block.
*/
QTextLayout::QTextLayout(const QTextBlock &block)
{
    d = new QTextEngine();
    d->block = block;
}

/*!
    Destructs the layout.
*/
QTextLayout::~QTextLayout()
{
    delete d;
}

/*!
    Sets the layout's font to the given \a font. The layout is
    invalidated and must be laid out again.

    \sa text()
*/
void QTextLayout::setFont(const QFont &font)
{
    if (d->fnt && !d->fnt->ref.deref())
        delete d->fnt;
    d->fnt = font.d;
    d->fnt->ref.ref();
}

/*!
    Returns the current font that is used for the layout, or a default
    font if none is set.
*/
QFont QTextLayout::font() const
{
    return d->fnt ? QFont(d->fnt) : QFont();
}

/*!
    Sets the layout's text to the given \a string. The layout is
    invalidated and must be laid out again.

    \sa text()
*/
void QTextLayout::setText(const QString& string)
{
    d->invalidate();
    d->text = string;
}

/*!
    Returns the layout's text.

    \sa setText()
*/
QString QTextLayout::text() const
{
    return d->text;
}

/*!
  Sets the text option structure that controls the layout process to the
  given \a option.

  \sa textOption() QTextOption
*/
void QTextLayout::setTextOption(const QTextOption &option)
{
    d->option = option;
}

/*!
  Returns the current text option used to control the layout process.

  \sa setTextOption() QTextOption
*/
QTextOption QTextLayout::textOption() const
{
    return d->option;
}

/*!
    Sets the \a position and \a text of the area in the layout that is
    processed before editing occurs.
*/
void QTextLayout::setPreeditArea(int position, const QString &text)
{
    if (text.isEmpty()) {
        if (!d->specialData)
            return;
        if (d->specialData->addFormats.isEmpty()) {
            delete d->specialData;
            d->specialData = 0;
        } else {
            d->specialData->preeditText = QString();
            d->specialData->preeditPosition = -1;
        }
    } else {
        if (!d->specialData)
            d->specialData = new QTextEngine::SpecialData;
        d->specialData->preeditPosition = position;
        d->specialData->preeditText = text;
    }
    d->invalidate();
    if (d->block.docHandle())
        d->block.docHandle()->documentChange(d->block.position(), d->block.length());
}

/*!
    Returns the position of the area in the text layout that will be
    processed before editing occurs.
*/
int QTextLayout::preeditAreaPosition() const
{
    return d->specialData ? d->specialData->preeditPosition : -1;
}

/*!
    Returns the text that is inserted in the layout before editing occurs.
*/
QString QTextLayout::preeditAreaText() const
{
    return d->specialData ? d->specialData->preeditText : QString();
}


/*!
    Sets the additional formats supported by the text layout.

    \sa additionalFormats(), clearAdditionalFormats()
*/
void QTextLayout::setAdditionalFormats(const QList<FormatRange> &formatList)
{
    if (formatList.isEmpty()) {
        if (!d->specialData)
            return;
        if (d->specialData->preeditText.isEmpty()) {
            delete d->specialData;
            d->specialData = 0;
        } else {
            d->specialData->addFormats = formatList;
        }
        return;
    }
    if (!d->specialData) {
        d->specialData = new QTextEngine::SpecialData;
        d->specialData->preeditPosition = -1;
    }
    d->specialData->addFormats = formatList;
    if (d->block.docHandle())
        d->block.docHandle()->documentChange(d->block.position(), d->block.length());
}

/*!
    Returns the list of additional formats supported by the text layout.

    \sa setAdditionalFormats(), clearAdditionalFormats()
*/
QList<QTextLayout::FormatRange> QTextLayout::additionalFormats() const
{
    return d->specialData ? d->specialData->addFormats : QList<FormatRange>();
}

/*!
    Clears the list of additional formats supported by the text layout.

    \sa additionalFormats(), setAdditionalFormats()
*/
void QTextLayout::clearAdditionalFormats()
{
    setAdditionalFormats(QList<FormatRange>());
}

/*!
  Enables caching of the complete layout information if \a enable is
  true; otherwise disables layout caching. Usually
  QTextLayout throws most of the layouting information away after a
  call to endLayout() to reduce memory consumption. If you however
  want to draw the layouted text directly afterwards enabling caching
  might speed up drawing significantly.

  \sa cacheEnabled
*/
void QTextLayout::setCacheEnabled(bool enable)
{
    d->cacheGlyphs = enable;
}

/*!
  Returns true if the complete layout information is cached; otherwise
  returns false.

  \sa setCacheEnabled
*/
bool QTextLayout::cacheEnabled() const
{
    return d->cacheGlyphs;
}

/*!
    Begins the layout process.
*/
void QTextLayout::beginLayout()
{
#ifndef QT_NO_DEBUG
    if (d->layoutData && d->layoutData->inLayout) {
        qWarning("QTextLayout::beginLayout() called while doing layout");
        return;
    }
#endif
    d->invalidate();
    d->itemize();
    d->layoutData->inLayout = true;
}

/*!
    Ends the layout process.
*/
void QTextLayout::endLayout()
{
#ifndef QT_NO_DEBUG
    if (!d->layoutData || !d->layoutData->inLayout) {
        qWarning("QTextLayout::endLayout() called without beginLayout()");
        return;
    }
#endif
    int l = d->lines.size();
    if (l && d->lines.at(l-1).length < 0) {
        QTextLine(l-1, d).setNumColumns(INT_MAX);
    }
    d->layoutData->inLayout = false;
    if (!d->cacheGlyphs)
        d->freeMemory();
}

/*!
    Returns the next valid cursor position after \a oldPos that
    respects the given cursor \a mode.

    \sa isValidCursorPosition() previousCursorPosition()
*/
int QTextLayout::nextCursorPosition(int oldPos, CursorMode mode) const
{
//      qDebug("looking for next cursor pos for %d", oldPos);
    const QCharAttributes *attributes = d->attributes();
    if (!attributes)
        return 0;
    int len = d->layoutData->string.length();
    if (oldPos >= len)
        return oldPos;
    oldPos++;
    if (mode == SkipCharacters) {
        while (oldPos < len && !attributes[oldPos].charStop)
            oldPos++;
    } else {
        while (oldPos < len && attributes[oldPos].whiteSpace)
            oldPos++;

        while (oldPos < len && !attributes[oldPos].wordStop && !attributes[oldPos-1].whiteSpace
               && !d->atWordSeparator(oldPos))
            oldPos++;
    }
//      qDebug("  -> %d", oldPos);
    return oldPos;
}

/*!
    Returns the first valid cursor position before \a oldPos that
    respects the given cursor \a mode.

    \sa isValidCursorPosition() nextCursorPosition()
*/
int QTextLayout::previousCursorPosition(int oldPos, CursorMode mode) const
{
//     qDebug("looking for previous cursor pos for %d", oldPos);
    const QCharAttributes *attributes = d->attributes();
    if (!attributes || oldPos <= 0)
        return 0;
    oldPos--;
    if (mode == SkipCharacters) {
        while (oldPos && !attributes[oldPos].charStop)
            oldPos--;
    } else {
        while (oldPos && attributes[oldPos].whiteSpace)
            oldPos--;

        while (oldPos && !attributes[oldPos].wordStop && !attributes[oldPos-1].whiteSpace
               && !d->atWordSeparator(oldPos - 1))
            oldPos--;
    }
//     qDebug("  -> %d", oldPos);
    return oldPos;
}

/*!
    Returns true if position \a pos is a valid cursor position.

    In a Unicode context some positions in the text are not valid
    cursor positions, because the position is inside a Unicode
    surrogate or a grapheme cluster.

    A grapheme cluster is a sequence of two or more Unicode characters
    that form one indivisible entity on the screen. For example the
    latin character `�' can be represented in Unicode by two
    characters, `A' (0x41), and the combining diaresis (0x308). A text
    cursor can only validly be positioned before or after these two
    characters, never between them since that wouldn't make sense. In
    indic languages every syllable forms a grapheme cluster.
*/
bool QTextLayout::isValidCursorPosition(int pos) const
{
    const QCharAttributes *attributes = d->attributes();
    if (!attributes || pos < 0 || pos > (int)d->layoutData->string.length())
        return false;
    return attributes[pos].charStop;
}


// ### DOC: Don't know what this really does.
// added a bit more description
/*!
    Returns a new text line to be laid out if there is text to be
    inserted into the layout; otherwise returns an invalid text line.

    The text layout creates a new line object that starts after the
    last line in the layout, or at the beginning if the layout is empty.
    The layout maintains an internal cursor, and each line is filled
    with text from the cursor position onwards when the
    QTextLine::setLineWidth() function is called.

    Once QTextLine::setLineWidth() is called, a new line can be created and
    filled with text. Repeating this process will lay out the whole block
    of text contained in the QTextLayout. If there is no text left to be
    inserted into the layout, the QTextLine returned will not be valid
    (isValid() will return false).
*/
QTextLine QTextLayout::createLine()
{
#ifndef QT_NO_DEBUG
    if (!d->layoutData || !d->layoutData->inLayout) {
        qWarning("QTextLayout::createLine() called without layouting");
        return QTextLine();
    }
#endif
    int l = d->lines.size();
    if (l && d->lines.at(l-1).length < 0) {
        QTextLine(l-1, d).setNumColumns(INT_MAX);
    }
    int from = l > 0 ? d->lines.at(l-1).from + d->lines.at(l-1).length : 0;
    if (l && from >= d->layoutData->string.length())
        return QTextLine();

    QScriptLine line;
    line.from = from;
    line.length = -1;
    line.justified = false;
    line.gridfitted = false;

    d->lines.append(line);
    return QTextLine(l, d);
}

/*!
    Returns the number of lines in this text layout.

    \sa lineAt()
*/
int QTextLayout::lineCount() const
{
    return d->lines.size();
}

/*!
    Returns the \a{i}-th line of text in this text layout.

    \sa lineCount() lineForTextPosition()
*/
QTextLine QTextLayout::lineAt(int i) const
{
    return QTextLine(i, d);
}

/*!
    Returns the line that contains the cursor position specified by \a pos.

    \sa isValidCursorPosition() lineAt()
*/
QTextLine QTextLayout::lineForTextPosition(int pos) const
{
    for (int i = 0; i < d->lines.size(); ++i) {
        const QScriptLine& line = d->lines[i];
        if (line.from + (int)line.length > pos)
            return QTextLine(i, d);
    }
    if (!d->layoutData)
        d->validate();
    if (pos == d->layoutData->string.length() && d->lines.size())
        return QTextLine(d->lines.size()-1, d);
    return QTextLine();
}

/*!
    The global position of the layout. This is independent of the
    bounding rectangle and of the layout process.

    \sa setPosition()
*/
QPointF QTextLayout::position() const
{
    return d->position;
}

/*!
    Moves the text layout to point \a p.

    \sa position()
*/
void QTextLayout::setPosition(const QPointF &p)
{
    d->position = p;
}

/*!
    The smallest rectangle that contains all the lines in the layout.
*/
QRectF QTextLayout::boundingRect() const
{
    qreal xmin = 0, xmax = 0, ymin = 0, ymax = 0;
    for (int i = 0; i < d->lines.size(); ++i) {
        const QScriptLine &si = d->lines[i];
        xmin = qMin(xmin, si.x);
        ymin = qMin(ymin, si.y);
        xmax = qMax(xmax, si.x+si.width);
        // ### shouldn't the ascent be used in ymin???
        ymax = qMax(ymax, si.y+si.ascent+si.descent+1);
    }
    return QRectF(xmin, ymin, xmax-xmin, ymax-ymin);
}

/*!
    The minimum width the layout needs. This is the width of the
    layout's smallest non-breakable sub-string.

    \warning This function only returns a valid value after the layout
    has been done.

    \sa maximumWidth()
*/
qreal QTextLayout::minimumWidth() const
{
    return d->minWidth;
}

/*!
    The maximum width the layout could expand to; this is essentially
    the width of the entire text.

    \warning This function only returns a valid value after the layout
    has been done.

    \sa minimumWidth()
*/
qreal QTextLayout::maximumWidth() const
{
    return d->maxWidth;
}

/*!
    \omit
    \fn void QTextLayout::draw(QPainter *painter, const QPointF &position, const QVector &selections, const QRectF &clipRect) const
    \endomit

    Draws the whole layout on the painter \a p at the position specified by
    \a pos.
    The rendered layout includes the given \a selections and is clipped within
    the rectangle specified by \a clip.
*/
void QTextLayout::draw(QPainter *p, const QPointF &pos, const QVector<QTextLayout::FormatRange> &selections, const QRectF &clip) const
{
    Q_ASSERT(lineCount() != 0);

    if (!d->layoutData)
        d->itemize();

    QPointF position = pos + d->position;

    qreal clipy = qreal(INT_MIN/256);
    qreal clipe = qreal(INT_MAX/256);
    if (clip.isValid()) {
        clipy = clip.y() - position.y();
        clipe = clipy + clip.height();
    }

    for (int i = 0; i < d->lines.size(); i++) {
        QTextLine l(i, d);
        const QScriptLine &sl = d->lines[i];

        if (sl.y > clipe || (sl.y + sl.height()) < clipy)
            continue;

        l.draw(p, position);
        for (int i = 0; i < selections.size(); ++i)
            l.draw(p, position, selections.constData()+i);
    }

    if (!d->cacheGlyphs)
        d->freeMemory();
}

/*!
  \fn void QTextLayout::drawCursor(QPainter *painter, const QPointF &position, int cursorPosition) const

  Draws a text cursor with the current pen at the given \a position using the
  \a painter specified.
  The corresponding position within the text is specified by \a cursorPosition.
*/
void QTextLayout::drawCursor(QPainter *p, const QPointF &pos, int cursorPosition) const
{
    if (!d->layoutData)
        d->itemize();

    QPointF position = pos + d->position;

    for (int i = 0; i < d->lines.size(); i++) {
        QTextLine l(i, d);
        const QScriptLine &sl = d->lines[i];

        if ((sl.from <= cursorPosition && sl.from + (int)sl.length > cursorPosition)
            || (sl.from + (int)sl.length == cursorPosition && cursorPosition == d->layoutData->string.length())) {

            const qreal x = position.x() + l.cursorToX(cursorPosition);

            int itm = d->findItem(cursorPosition - 1);
            qreal ascent = sl.ascent;
            qreal descent = sl.descent;
            bool rightToLeft = (d->option.textDirection() == Qt::RightToLeft);
            if (itm >= 0) {
                const QScriptItem &si = d->layoutData->items.at(itm);
                if (si.ascent > 0.0)
                    ascent = si.ascent;
                if (si.descent > 0.0)
                    descent = si.descent;
                rightToLeft = si.analysis.bidiLevel % 2;
            }
            qreal y = position.y() + sl.y + sl.ascent - ascent;
            p->drawLine(QLineF(x, y, x, y + ascent + descent));
            if (d->layoutData->hasBidi) {
                const int arrow_extent = 4;
                int sign = rightToLeft ? -1 : 1;
                p->drawLine(QLineF(x, y, x + (sign * arrow_extent/2), y + arrow_extent/2));
                p->drawLine(QLineF(x, y+arrow_extent, x + (sign * arrow_extent/2), y + arrow_extent/2));
            }
            return;
        }
    }

}

/*!
    \class QTextLine
    \brief The QTextLine class represents a line of text inside a QTextLayout.

    \ingroup text

    A text line is usually created by QTextLayout::createLine().

    After being created, the line can be filled using the layout()
    function. A line has a number of attributes including the
    rectangle it occupies, rect(), its coordinates, x() and y(), its
    textLength(), width() and naturalTextWidth(), and its ascent() and decent()
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
    \fn int QTextLine::lineNumber() const

    Returns the position of the line in the text engine.
*/


/*!
    Returns the line's bounding rectangle.

    \sa x() y() textLength() width()
*/
QRect QTextLine::rect() const
{
    const QScriptLine& sl = eng->lines[i];
    return QRect(qRound(sl.x), qRound(sl.y), qRound(sl.width), qRound(sl.height()));
}

/*!
    Returns the rectangle covered by the line.
*/
QRectF QTextLine::naturalTextRect() const
{
    const QScriptLine& sl = eng->lines[i];
    qreal x = sl.x + alignLine(eng, sl);

    return QRectF(x, sl.y, sl.textWidth, sl.height());
}

/*!
    Returns the line's x position.

    \sa rect() y() textLength() width()
*/
qreal QTextLine::x() const
{
    return eng->lines[i].x;
}

/*!
    Returns the line's y position.

    \sa x() rect() textLength() width()
*/
qreal QTextLine::y() const
{
    return eng->lines[i].y;
}

/*!
    Returns the line's width as specified by the layout() function.

    \sa naturalTextWidth() x() y() textLength() rect()
*/
qreal QTextLine::width() const
{
    return eng->lines[i].width;
}


/*!
    Returns the line's ascent.

    \sa descent() height()
*/
qreal QTextLine::ascent() const
{
    return eng->lines[i].ascent;
}

/*!
    Returns the line's descent.

    \sa ascent() height()
*/
qreal QTextLine::descent() const
{
    return eng->lines[i].descent;
}

/*!
    Returns the line's height. This is equal to ascent() + descent() + 1.

    \sa ascent() descent()
*/
qreal QTextLine::height() const
{
    return eng->lines[i].height();
}

/*!
    Returns the width of the line that is occupied by text. This is
    always \<= to width(), and is the minimum width that could be used
    by layout() without changing the line break position.
*/
qreal QTextLine::naturalTextWidth() const
{
    return eng->lines[i].textWidth;
}

/*!
    Lays out the line with the given \a width. The line is filled from
    it's starting position with as many characters as will fit into
    the line.
*/
void QTextLine::setLineWidth(qreal width)
{
    QScriptLine &line = eng->lines[i];
    line.width = width;
    line.length = 0;
    line.textWidth = 0;
    layout_helper(INT_MAX);
}

/*!
    Lays out the line. The line is filled from it's starting position
    with as many characters as are specified by \a numColumns.
*/
void QTextLine::setNumColumns(int numColumns)
{
    QScriptLine &line = eng->lines[i];
    line.width = qreal(INT_MAX/256);
    line.length = 0;
    line.textWidth = 0;
    layout_helper(numColumns);
}

void QTextLine::layout_helper(int maxGlyphs)
{
    QScriptLine &line = eng->lines[i];

    if (!eng->layoutData->items.size()) {
        line.setDefaultHeight(eng);
        return;
    }

    Q_ASSERT(line.from < eng->layoutData->string.length());

    bool breakany = (eng->option.wrapMode() == QTextOption::WrapAnywhere);

    // #### binary search!
    int item;
    for (item = eng->layoutData->items.size()-1; item > 0; --item) {
        if (eng->layoutData->items[item].position <= line.from)
            break;
    }

    qreal minw = 0, spacew = 0;
    int glyphCount = 0;

//     qDebug("from: %d:   item=%d, total %d width available %f", line.from, item, eng->layoutData->items.size(), line.width);

    while (item < eng->layoutData->items.size()) {
        const QCharAttributes *attributes = eng->attributes();
        const QScriptItem &current = eng->layoutData->items[item];
        if (!current.num_glyphs)
            eng->shape(item);

        if (current.isObject) {
            QTextFormat format = eng->formats()->format(eng->formatIndex(&eng->layoutData->items[item]));
            if (eng->block.docHandle())
                eng->docLayout()->positionInlineObject(QTextInlineObject(item, eng), format);
            if (line.length && eng->option.wrapMode() != QTextOption::ManualWrap) {
                if (line.textWidth + current.width > line.width || glyphCount > maxGlyphs)
                    goto found;
            }

            line.length++;
            // the width of the linesep doesn't count into the textwidth
            if (eng->layoutData->string.at(current.position) == QChar::LineSeparator) {
                // if the line consists only of the line separator make sure
                // we have a sane height
                if (line.length == 1)
                    line.setDefaultHeight(eng);
                goto found;
            }
            line.textWidth += current.width;

            ++item;
            ++glyphCount;
            line.ascent = qMax(line.ascent, current.ascent);
            line.descent = qMax(line.descent, current.descent);
            continue;
        } else if (current.isTab &&
                   (eng->option.alignment() & Qt::AlignLeft)) {
            qreal x = line.x + line.textWidth;
            qreal nx = eng->nextTab(&current, x);
            line.textWidth += nx - x;
            line.length++;
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

            qreal tmpw = 0;
            if (!itemAttrs[next].whiteSpace) {
                tmpw = spacew;
                spacew = 0;
                do {
                    int gp = logClusters[next];
                    do {
                        ++next;
                    } while (next < length && logClusters[next] == gp);
                    do {
                        tmpw += glyphs[gp].advance.x();
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
                    spacew += glyphs[gp].advance.x();
                    ++gp;
                } while (gp < current.num_glyphs && !glyphs[gp].attributes.clusterStart);

                ++glyphCount;
                Q_ASSERT((next == length && gp == current.num_glyphs) || logClusters[next] == gp);
            }

//             qDebug("possible break at %d, chars (%d-%d) / glyphs (%d-%d): width %f, spacew=%f",
//                    current.position + next, pos, next, logClusters[pos], logClusters[next], tmpw, spacew);

            if (line.length && tmpw != qreal(0) && (line.textWidth + tmpw > line.width || glyphCount > maxGlyphs)
                && eng->option.wrapMode() != QTextOption::ManualWrap)
                goto found;

            line.textWidth += tmpw;
            line.length += next - pos;
            line.ascent = qMax(line.ascent, current.ascent);
            line.descent = qMax(line.descent, current.descent);

            pos = next;
        } while (pos < length);
        ++item;
    }
 found:
//     qDebug("line length = %d, ascent=%f, descent=%f, textWidth=%f (spacew=%f)", line.length, line.ascent,
//            line.descent, line.textWidth, spacew);
//     qDebug("        : '%s'", eng->layoutData->string.mid(line.from, line.length).toUtf8().data());

    eng->minWidth = qMax(eng->minWidth, minw);
    eng->maxWidth += line.textWidth;
    if (line.textWidth > 0 && item < eng->layoutData->items.size())
        eng->maxWidth += spacew;
    if (eng->option.flags() & QTextOption::IncludeTrailingSpaces)
        line.textWidth += spacew;

    line.justified = false;
    line.gridfitted = false;
}

/*!
    Moves the line to position \a pos.
*/
void QTextLine::setPosition(const QPointF &pos)
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
int QTextLine::textStart() const
{
    return eng->lines[i].from;
}

/*!
    Returns the length of the text in the line.

    \sa naturalTextWidth()
*/
int QTextLine::textLength() const
{
    return eng->lines[i].length;
}

static void drawMenuText(QPainter *p, qreal x, qreal y, const QScriptItem &si, QTextItemInt &gf, QTextEngine *eng,
                         int start, int glyph_start)
{
    int ge = glyph_start + gf.num_glyphs;
    int gs = glyph_start;
    int end = start + gf.num_chars;
    unsigned short *logClusters = eng->logClusters(&si);
    QGlyphLayout *glyphs = eng->glyphs(&si);
    qreal orig_width = gf.width;

    int *ul = eng->underlinePositions;
    if (ul)
        while (*ul != -1 && *ul < start)
            ++ul;
    bool rtl = si.analysis.bidiLevel % 2;
    if (rtl)
        x += si.width;

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
        gf.chars = eng->layoutData->string.unicode() + start;
        qreal w = 0;
        while (gs < gtmp) {
            w += glyphs[gs].advance.x() + qreal(glyphs[gs].space_18d6)/qreal(64);
            ++gs;
        }
        start = stmp;
        gf.width = w;
        if (rtl)
            x -= w;
        if (gf.num_chars)
            p->drawTextItem(QPointF(x, y), gf);
        if (!rtl)
            x += w;
        if (ul && *ul != -1 && *ul < end) {
            // draw underline
            gtmp = (*ul == end-1) ? ge : logClusters[*ul+1-si.position];
            ++stmp;
            gf.num_glyphs = gtmp - gs;
            gf.glyphs = glyphs + gs;
            gf.num_chars = stmp - start;
            gf.chars = eng->layoutData->string.unicode() + start;
            w = 0;
            while (gs < gtmp) {
                w += glyphs[gs].advance.x() + qreal(glyphs[gs].space_18d6)/qreal(64);
                ++gs;
            }
            ++start;
            gf.width = w;
            gf.flags |= QTextItem::Underline;
            if (rtl)
                x -= w;
            p->drawTextItem(QPointF(x, y), gf);
            if (!rtl)
                x += w;
            gf.flags &= ~QTextItem::Underline;
            ++gf.chars;
            ++ul;
        }
    } while (gs < ge);

    gf.width = orig_width;
}


static void setPenAndDrawBackground(QPainter *p, const QPen &defaultPen, const QTextCharFormat &chf, const QRectF &r)
{
    QBrush c = chf.foreground();
    if (c == Qt::NoBrush)
        p->setPen(defaultPen);

    QBrush bg = chf.background();
    if (bg.style() != Qt::NoBrush)
        p->fillRect(r, bg);
    if (c != Qt::NoBrush)
        p->setPen(QPen(c, 0));
}

/*!
    \fn void QTextLine::draw(QPainter *painter, const QPointF &position, const QTextLayout::FormatRange *selection) const

    Draws a line on the given \a painter at the specified \a position.
    The \a selection is reserved for internal use.
*/
void QTextLine::draw(QPainter *p, const QPointF &pos, const QTextLayout::FormatRange *selection) const
{
    const QScriptLine &line = eng->lines[i];

    if (!line.length)
        return;

    QPen pen = p->pen();

    int lineEnd = line.from + line.length;
    // don't draw trailing spaces or take them into the layout.
    if (!(eng->option.flags() & QTextOption::IncludeTrailingSpaces)) {
        const QCharAttributes *attributes = eng->attributes();
        while (lineEnd > line.from && attributes[lineEnd-1].whiteSpace)
            --lineEnd;
    }

    int firstItem = eng->findItem(line.from);
    int lastItem = eng->findItem(lineEnd - 1);
    int nItems = lastItem-firstItem+1;

    qreal x = pos.x();
    qreal y = pos.y();
    x += line.x;
    y += line.y + line.ascent;

    x += alignLine(eng, line);

    QVarLengthArray<int> visualOrder(nItems);
    QVarLengthArray<uchar> levels(nItems);
    for (int i = 0; i < nItems; ++i)
        levels[i] = eng->layoutData->items[i+firstItem].analysis.bidiLevel;
    QTextEngine::bidiReorder(nItems, levels.data(), visualOrder.data());

    QRectF outlineRect;
    QPen outlinePen(Qt::NoPen);
    if (selection) {
        QVariant outline = selection->format.property(QTextFormat::OutlinePen);
        if (outline.type() == QVariant::Pen)
            outlinePen = qVariantValue<QPen>(outline);
    }

    QFont f = eng->font();
    for (int i = 0; i < nItems; ++i) {
        int item = visualOrder[i]+firstItem;
        QScriptItem &si = eng->layoutData->items[item];
        int si_len = eng->length(item);
        if (!si.num_glyphs)
            eng->shape(item);

        if (si.isObject || si.isTab) {
            if (eng->block.docHandle() &&
                (!selection || (si.position < selection->start + selection->length
                                && si.position + si_len > selection->start))) {
                p->save();
                QTextCharFormat format = eng->format(&si).toCharFormat();
                if (selection)
                    format.merge(selection->format);
                qreal width = si.width;
                if (si.isTab) {
                    width = eng->nextTab(&si, x - pos.x()) - (x - pos.x());
                }
                setPenAndDrawBackground(p, pen, format, QRectF(x, y - line.ascent, width, line.height()));
                if (si.isObject) {
                    QRectF itemRect(x, y-si.ascent, width, si.height());
                    eng->docLayout()->drawInlineObject(p, itemRect,
                                                       QTextInlineObject(item, eng), format);
                    if (selection) {
                        QBrush bg = format.background();
                        if (bg.style() != Qt::NoBrush) {
                            QColor c = bg.color();
                            c.setAlpha(128);
                            p->fillRect(itemRect, c);
                        }
                        if (outlinePen.style() != Qt::NoPen)
                            outlineRect = outlineRect.unite(itemRect);
                    }
                }
                p->restore();
            }

            if (si.isTab)
                x = eng->nextTab(&si, x - pos.x()) + pos.x();
            else
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
            end = si.position + si_len;
            ge = si.num_glyphs;
        }

        qreal itemBaseLine = y;

        QTextItemInt gf;
        if (si.analysis.bidiLevel %2)
            gf.flags |= QTextItem::RightToLeft;
        gf.ascent = si.ascent;
        gf.descent = si.descent;
        gf.num_glyphs = ge - gs;
        gf.glyphs = glyphs + gs;
        gf.chars = eng->layoutData->string.unicode() + start;
        gf.num_chars = end - start;
        gf.width = 0;
        int g = gs;
        while (g < ge) {
            gf.width += glyphs[g].advance.x() + qreal(glyphs[g].space_18d6)/qreal(64);
            ++g;
        }

        if (selection) {
            int from = qMax(start, selection->start) - si.position;
            int to = qMin(end, selection->start + selection->length) - si.position;
            if (from >= to) {
                x += gf.width;
                continue;
            }
            int start_glyph = logClusters[from];
            int end_glyph = (to == eng->length(item)) ? si.num_glyphs : logClusters[to];
            qreal soff = 0;
            qreal swidth = 0;
            if (si.analysis.bidiLevel %2) {
                for (int g = ge - 1; g >= end_glyph; --g)
                    soff += glyphs[g].advance.x() + qreal(glyphs[g].space_18d6)/qreal(64);
                for (int g = end_glyph - 1; g >= start_glyph; --g)
                    swidth += glyphs[g].advance.x() + qreal(glyphs[g].space_18d6)/qreal(64);
            } else {
                for (int g = gs; g < start_glyph; ++g)
                    soff += glyphs[g].advance.x() + qreal(glyphs[g].space_18d6)/qreal(64);
                for (int g = start_glyph; g < end_glyph; ++g)
                    swidth += glyphs[g].advance.x() + qreal(glyphs[g].space_18d6)/qreal(64);
            }

            QRectF rect(x + soff, y - line.ascent, swidth, line.height());
            if (outlinePen.style() != Qt::NoPen)
                outlineRect = outlineRect.unite(rect);
            p->save();
            p->setClipRect(rect);
        }


        if (eng->block.docHandle() || selection) {
            QTextCharFormat chf;
            if (eng->block.docHandle())
                chf = eng->format(&si).toCharFormat();
            if (selection)
                chf.merge(selection->format);

            setPenAndDrawBackground(p, pen, chf, QRectF(x, y - line.ascent, gf.width, line.height()));

            QTextCharFormat::VerticalAlignment valign = chf.verticalAlignment();
            if (valign == QTextCharFormat::AlignSubScript)
                itemBaseLine += (si.ascent + si.descent + 1) / 6;
            else if (valign == QTextCharFormat::AlignSuperScript)
                itemBaseLine -= (si.ascent + si.descent + 1) / 2;

            f = eng->font(si);
        }

        gf.fontEngine = f.d->engineForScript(si.analysis.script);
        gf.f = &f;
        if (f.d->underline)
            gf.flags |= QTextItem::Underline;
        if (f.d->overline)
            gf.flags |= QTextItem::Overline;
        if (f.d->strikeOut)
            gf.flags |= QTextItem::StrikeOut;
        Q_ASSERT(gf.fontEngine);

        if (eng->underlinePositions) {
            // can't have selections in this case
            drawMenuText(p, x, itemBaseLine, si, gf, eng, start, gs);
        } else {
            p->drawTextItem(QPointF(x, itemBaseLine), gf);
        }
        if (selection)
            p->restore();

        x += gf.width;
    }

    if (outlineRect.isValid()) {
        p->setPen(outlinePen);
        p->drawRect(outlineRect);
    }

    p->setPen(pen);
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
qreal QTextLine::cursorToX(int *cursorPos, Edge edge) const
{
    eng->itemize();
    const QScriptLine &line = eng->lines[i];

    qreal x = line.x;
    x += alignLine(eng, line);

    if (!i && !eng->layoutData->items.size()) {
        *cursorPos = 0;
        return x;
    }

    int pos = *cursorPos;
    int itm = eng->findItem(pos);
    if (pos == line.from + (int)line.length) {
        // end of line ensure we have the last item on the line
        itm = eng->findItem(pos-1);
    }

    const QScriptItem *si = &eng->layoutData->items[itm];
    if (!si->num_glyphs)
        eng->shape(itm);
    pos -= si->position;

    if (!si->num_glyphs)
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

    bool reverse = eng->layoutData->items[itm].analysis.bidiLevel % 2;

    int lineEnd = line.from + line.length;
    // don't draw trailing spaces or take them into the layout.
    if (!(eng->option.flags() & QTextOption::IncludeTrailingSpaces)) {
        const QCharAttributes *attributes = eng->attributes();
        while (lineEnd > line.from && attributes[lineEnd-1].whiteSpace)
            --lineEnd;
    }

    // add the items left of the cursor

    int firstItem = eng->findItem(line.from);
    int lastItem = eng->findItem(lineEnd - 1);
    int nItems = lastItem-firstItem+1;

    QVarLengthArray<int> visualOrder(nItems);
    QVarLengthArray<uchar> levels(nItems);
    for (int i = 0; i < nItems; ++i)
        levels[i] = eng->layoutData->items[i+firstItem].analysis.bidiLevel;
    QTextEngine::bidiReorder(nItems, levels.data(), visualOrder.data());

    for (int i = 0; i < nItems; ++i) {
        int item = visualOrder[i]+firstItem;
        if (item == itm)
            break;
        QScriptItem &si = eng->layoutData->items[item];
        if (!si.num_glyphs)
            eng->shape(item);

        if (si.isTab) {
            x = eng->nextTab(&si, x);
            continue;
        } else if (si.isObject) {
            x += si.width;
            continue;
        }
        int start = qMax(line.from, si.position);
        int end = qMin(lineEnd, si.position + eng->length(item));

        logClusters = eng->logClusters(&si);

        int gs = logClusters[start-si.position];
        int ge = (end == si.position + eng->length(item)) ? si.num_glyphs-1 : logClusters[end-si.position-1];

        QGlyphLayout *glyphs = eng->glyphs(&si);

        while (gs <= ge) {
            x += glyphs[gs].advance.x() + qreal(glyphs[gs].space_18d6)/qreal(64);
            ++gs;
        }
    }

    logClusters = eng->logClusters(si);
    glyphs = eng->glyphs(si);
    if (si->isTab) {
        if(pos == l)
            x = eng->nextTab(si, x);
    } else if (si->isObject) {
        if(pos == l)
            x += si->width;
    } else {
        if (reverse) {
            int end = qMin(lineEnd, si->position + l) - si->position;
            int glyph_end = end == l ? si->num_glyphs : logClusters[end];
            for (int i = glyph_end - 1; i >= glyph_pos; i--)
                x += glyphs[i].advance.x() + qreal(glyphs[i].space_18d6)/qreal(64);
        } else {
            int start = qMax(line.from - si->position, 0);
            int glyph_start = logClusters[start];
            for (int i = glyph_start; i < glyph_pos; i++)
                x += glyphs[i].advance.x() + qreal(glyphs[i].space_18d6)/qreal(64);
        }
    }

    *cursorPos = pos + si->position;
    return x;
}

/*!
  Converts the x-coordinate \a x, to the nearest matching cursor
  position, depending on the cursor position type, \a cpos.

  \sa cursorToX()
*/
int QTextLine::xToCursor(qreal x, CursorPosition cpos) const
{
    const QScriptLine &line = eng->lines[i];

    int line_length = line.length;
    // don't draw trailing spaces or take them into the layout.
    const QCharAttributes *a = eng->attributes() + line.from;
    while (line_length && a[line_length-1].whiteSpace)
        --line_length;

    if (!line_length)
        return line.from;


    int firstItem = eng->findItem(line.from);
    int lastItem = eng->findItem(line.from + line_length - 1);
    int nItems = lastItem-firstItem+1;

    x -= line.x;
    x -= alignLine(eng, line);
//     qDebug("xToCursor: x=%f, cpos=%d", x, cpos);

    QVarLengthArray<int> visualOrder(nItems);
    QVarLengthArray<unsigned char> levels(nItems);
    for (int i = 0; i < nItems; ++i)
        levels[i] = eng->layoutData->items[i+firstItem].analysis.bidiLevel;
    QTextEngine::bidiReorder(nItems, levels.data(), visualOrder.data());

    if (x <= 0) {
        // left of first item
        int item = visualOrder[0]+firstItem;
        QScriptItem &si = eng->layoutData->items[item];
        if (!si.num_glyphs)
            eng->shape(item);
        int pos = si.position;
        if (si.analysis.bidiLevel % 2)
            pos += eng->length(item);
        pos = qMax(line.from, pos);
        pos = qMin(line.from + line_length, pos);
        return pos;
    } else if (x < line.textWidth) {
        // has to be in one of the runs
        qreal pos = 0;

        for (int i = 0; i < nItems; ++i) {
            int item = visualOrder[i]+firstItem;
            QScriptItem &si = eng->layoutData->items[item];
            if (!si.num_glyphs)
                eng->shape(item);
            int item_length = eng->length(item);
//             qDebug("    item %d, visual %d x_remain=%f", i, item, x);

            int start = qMax(line.from - si.position, 0);
            int end = qMin(line.from + line_length - si.position, item_length);

            unsigned short *logClusters = eng->logClusters(&si);

            int gs = logClusters[start];
            int ge = (end == item_length ? si.num_glyphs : logClusters[end]) - 1;
            QGlyphLayout *glyphs = eng->glyphs(&si);

            qreal item_width = 0;
            if (si.isTab) {
                item_width = eng->nextTab(&si, pos) - pos;
            } else if (si.isObject) {
                item_width = si.width;
            } else {
                int g = gs;
                while (g <= ge) {
                    item_width += glyphs[g].advance.x() + qreal(glyphs[g].space_18d6)/qreal(64);
                    ++g;
                }
            }
//             qDebug("      start=%d, end=%d, gs=%d, ge=%d item_width=%f", start, end, gs, ge, item_width);

            if (pos + item_width < x) {
                pos += item_width;
                continue;
            }
//             qDebug("      inside run");
            if (si.isTab || si.isObject) {
                if (cpos == QTextLine::CursorOnCharacter)
                    return si.position;
                bool left_half = (x - pos) < item_width/2.;

                if (bool(si.analysis.bidiLevel % 2) ^ left_half)
                    return si.position;
                return si.position + 1;
            }

            int glyph_pos = -1;
            // has to be inside run
            if (cpos == QTextLine::CursorOnCharacter) {
                if (si.analysis.bidiLevel % 2) {
                    pos += item_width;
                    int last_glyph = gs;
                    while (gs <= ge) {
                        if (glyphs[gs].attributes.clusterStart && pos < x) {
                            glyph_pos = last_glyph;
                            break;
                        }
                        pos -= glyphs[gs].advance.x() + qreal(glyphs[gs].space_18d6)/qreal(64);
                        ++gs;
                    }
                } else {
                    glyph_pos = gs;
                    while (gs <= ge) {
                        if (glyphs[gs].attributes.clusterStart) {
                            if (pos > x)
                                break;
                            glyph_pos = gs;
                        }
                        pos += glyphs[gs].advance.x() + qreal(glyphs[gs].space_18d6)/qreal(64);
                        ++gs;
                    }
                }
            } else {
                qreal dist = qreal(INT_MAX/256);
                if (si.analysis.bidiLevel % 2) {
                    pos += item_width;
                    while (gs <= ge) {
                        if (glyphs[gs].attributes.clusterStart && qAbs(x-pos) < dist) {
                            glyph_pos = gs;
                            dist = qAbs(x-pos);
                        }
                        pos -= glyphs[gs].advance.x() + qreal(glyphs[gs].space_18d6)/qreal(64);
                        ++gs;
                    }
                } else {
                    while (gs <= ge) {
                        if (glyphs[gs].attributes.clusterStart && qAbs(x-pos) < dist) {
                            glyph_pos = gs;
                            dist = qAbs(x-pos);
                        }
                        pos += glyphs[gs].advance.x() + qreal(glyphs[gs].space_18d6)/qreal(64);
                        ++gs;
                    }
                }
                if (qAbs(x-pos) < dist)
                    return si.position + end;
            }
            Q_ASSERT(glyph_pos != -1);
            int j;
            for (j = 0; j < eng->length(item); ++j)
                if (logClusters[j] == glyph_pos)
                    break;
//             qDebug("at pos %d (in run: %d)", si.position + j, j);
            return si.position + j;
        }
    }
    // right of last item
    int item = visualOrder[nItems-1]+firstItem;
    QScriptItem &si = eng->layoutData->items[item];
    if (!si.num_glyphs)
        eng->shape(item);
    int pos = si.position;
    if (!(si.analysis.bidiLevel % 2))
        pos += eng->length(item);
    pos = qMax(line.from, pos);
    pos = qMin(line.from + line_length, pos);
    return pos;
}
