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

#include "qfont.h"
#include "qpaintdevice.h"
#include "qfontmetrics.h"

#include "qfontdata_p.h"
#include "qfontengine_p.h"
#include <private/qunicodetables_p.h>

#ifdef Q_WS_X11
#include "qx11info_x11.h"
extern const QX11Info *qt_x11Info(const QPaintDevice *pd);
#endif

extern void qt_format_text(const QFont& font, const QRectF &_r,
                           int tf, const QString& str, QRectF *brect,
                           int tabstops, int* tabarray, int tabarraylen,
                           QPainter* painter);
extern int qt_defaultDpi();

/*****************************************************************************
  QFontMetrics member functions
 *****************************************************************************/

/*!
    \class QFontMetrics qfontmetrics.h
    \brief The QFontMetrics class provides font metrics information.

    \ingroup multimedia
    \ingroup shared

    QFontMetrics functions calculate the size of characters and
    strings for a given font. There are three ways you can create a
    QFontMetrics object:

    \list 1
    \i Calling the QFontMetrics constructor with a QFont creates a
    font metrics object for a screen-compatible font, i.e. the font
    cannot be a printer font<sup>*</sup>. If the font is changed
    later, the font metrics object is \e not updated.

    \i QWidget::fontMetrics() returns the font metrics for a widget's
    font. This is equivalent to QFontMetrics(widget->font()). If the
    widget's font is changed later, the font metrics object is \e not
    updated.

    \i QPainter::fontMetrics() returns the font metrics for a
    painter's current font. If the painter's font is changed later, the
    font metrics object is \e not updated.
    \endlist

    <sup>*</sup> If you use a printer font the values returned may be
    inaccurate. Printer fonts are not always accessible so the nearest
    screen font is used if a printer font is supplied.

    Once created, the object provides functions to access the
    individual metrics of the font, its characters, and for strings
    rendered in the font.

    There are several functions that operate on the font: ascent(),
    descent(), height(), leading() and lineSpacing() return the basic
    size properties of the font. The underlinePos(), overlinePos(),
    strikeOutPos() and lineWidth() functions, return the properties of
    the line that underlines, overlines or strikes out the
    characters. These functions are all fast.

    There are also some functions that operate on the set of glyphs in
    the font: minLeftBearing(), minRightBearing() and maxWidth().
    These are by necessity slow, and we recommend avoiding them if
    possible.

    For each character, you can get its width(), leftBearing() and
    rightBearing() and find out whether it is in the font using
    inFont(). You can also treat the character as a string, and use
    the string functions on it.

    The string functions include width(), to return the width of a
    string in pixels (or points, for a printer), boundingRect(), to
    return a rectangle large enough to contain the rendered string,
    and size(), to return the size of that rectangle.

    Example:
    \code
    QFont font("times", 24);
    QFontMetrics fm(font);
    int pixelsWide = fm.width("What's the width of this text?");
    int pixelsHigh = fm.height();
    \endcode

    \sa QFont QFontInfo QFontDatabase
*/

/*!
    Constructs a font metrics object for \a font.

    The font metrics will be screen-compatible, i.e. the metrics you
    get if you use the font for drawing text on a \link QWidget
    widgets\endlink or \link QPixmap pixmaps\endlink, not on a
    QPicture or QPrinter.

    The font metrics object holds the information for the font that is
    passed in the constructor at the time it is created, and is not
    updated if the font's attributes are changed later.

    Use QFontMetrics(const QFont &, QPaintDevice *) to get the font
    metrics that are compatible with a certain paint device.
*/
QFontMetrics::QFontMetrics(const QFont &font)
    : d(font.d), fscript(QFont::NoScript)
{
    int dpi = qt_defaultDpi();
    if (font.d->dpi != dpi) {
        d = new QFontPrivate(*font.d);
        d->dpi = dpi;
        d->screen = 0;
    } else {
        ++d->ref;
    }
}

/*!
    \overload

    Constructs a font metrics object for \a font using the given \a
    script.
*/
QFontMetrics::QFontMetrics(const QFont &font, QFont::Script script)
    : d(font.d), fscript(script)
{
    int dpi = qt_defaultDpi();
    if (font.d->dpi != dpi) {
        d = new QFontPrivate(*font.d);
        d->dpi = dpi;
        d->screen = 0;
    } else {
        ++d->ref;
    }
}

/*!
    Constructs a font metrics object for \a font and \a paintdevice.

    The font metrics will be compatible with the paintdevice passed.
    If the \a paintdevice is 0, the metrics will be screen-compatible,
    ie. the metrics you get if you use the font for drawing text on a
    \link QWidget widgets\endlink or \link QPixmap pixmaps\endlink,
    not on a QPicture or QPrinter.

    The font metrics object holds the information for the font that is
    passed in the constructor at the time it is created, and is not
    updated if the font's attributes are changed later.
*/
QFontMetrics::QFontMetrics(const QFont &font, QPaintDevice *paintdevice)
    : fscript(QFont::NoScript)
{
    int dpi = paintdevice ? paintdevice->logicalDpiY() : qt_defaultDpi();
#ifdef Q_WS_X11
    const QX11Info *info = qt_x11Info(paintdevice);
    int screen = info ? info->screen() : 0;
#else
    const int screen = 0;
#endif
    if (font.d->dpi != dpi || font.d->screen != screen ) {
        d = new QFontPrivate(*font.d);
        d->dpi = dpi;
        d->screen = screen;
    } else {
        d = font.d;
        ++d->ref;
    }

}

/*!
    Constructs a copy of \a fm.
*/
QFontMetrics::QFontMetrics(const QFontMetrics &fm)
    : d(fm.d),  fscript(fm.fscript)
{
    ++d->ref;
}

/*!
    Destroys the font metrics object and frees all allocated
    resources.
*/
QFontMetrics::~QFontMetrics()
{
    if (!--d->ref)
        delete d;
}

/*!
    Assigns the font metrics \a fm.
*/
QFontMetrics &QFontMetrics::operator=(const QFontMetrics &fm)
{
    qAtomicAssign(d, fm.d);
    return *this;
}

/*!
    Returns true if \a other is equal to this object; otherwise
    returns false.

    Two font metrics are considered equal if they were constructed
    from the same QFont and the paint devices they were constructed
    for are considered compatible.

    \sa operator!=()
*/
bool QFontMetrics::operator ==(const QFontMetrics &other)
{
    return d == other.d;
}

/*!
    \fn bool QFontMetrics::operator !=(const QFontMetrics &other)

    Returns true if \a other is not equal to this object; otherwise returns false.

    Two font metrics are considered equal if they were constructed
    from the same QFont and the paint devices they were constructed
    for are considered compatible.

    \sa operator==()
*/

/*!
    Returns the ascent of the font.

    The ascent of a font is the distance from the baseline to the
    highest position characters extend to. In practice, some font
    designers break this rule, e.g. when they put more than one accent
    on top of a character, or to accommodate an unusual character in
    an exotic language, so it is possible (though rare) that this
    value will be too small.

    \sa descent()
*/
int QFontMetrics::ascent() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *latin_engine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(latin_engine != 0);

    return qRound(qMax(engine->ascent(), latin_engine->ascent()));
}


/*!
    Returns the descent of the font.

    The descent is the distance from the base line to the lowest point
    characters extend to. (Note that this is different from X, which
    adds 1 pixel.) In practice, some font designers break this rule,
    e.g. to accommodate an unusual character in an exotic language, so
    it is possible (though rare) that this value will be too small.

    \sa ascent()
*/
int QFontMetrics::descent() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *latin_engine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(latin_engine != 0);

    return qRound(qMax(engine->descent(), latin_engine->descent()));
}

/*!
    Returns the height of the font.

    This is always equal to ascent()+descent()+1 (the 1 is for the
    base line).

    \sa leading(), lineSpacing()
*/
int QFontMetrics::height() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *latin_engine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(latin_engine != 0);

    return qRound(qMax(engine->ascent(), latin_engine->ascent()) +
            qMax(engine->descent(), latin_engine->descent())) + 1;
}

/*!
    Returns the leading of the font.

    This is the natural inter-line spacing.

    \sa height(), lineSpacing()
*/
int QFontMetrics::leading() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *latin_engine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(latin_engine != 0);

    return qRound(qMax(engine->leading(), latin_engine->leading()));
}

/*!
    Returns the distance from one base line to the next.

    This value is always equal to leading()+height().

    \sa height(), leading()
*/
int QFontMetrics::lineSpacing() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *latin_engine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(latin_engine != 0);

    return qRound(qMax(engine->leading(), latin_engine->leading()) +
            qMax(engine->ascent(), latin_engine->ascent()) +
            qMax(engine->descent(), latin_engine->descent())) + 1;
}

/*!
    Returns the minimum left bearing of the font.

    This is the smallest leftBearing(char) of all characters in the
    font.

    Note that this function can be very slow if the font is large.

    \sa minRightBearing(), leftBearing()
*/
int QFontMetrics::minLeftBearing() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *latin_engine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(latin_engine != 0);

    return qRound(qMin(engine->minLeftBearing(), latin_engine->minLeftBearing()));
}

/*!
    Returns the minimum right bearing of the font.

    This is the smallest rightBearing(char) of all characters in the
    font.

    Note that this function can be very slow if the font is large.

    \sa minLeftBearing(), rightBearing()
*/
int QFontMetrics::minRightBearing() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *latin_engine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(latin_engine != 0);

    return qRound(qMin(engine->minRightBearing(), latin_engine->minRightBearing()));
}

/*!
    Returns the width of the widest character in the font.
*/
int QFontMetrics::maxWidth() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *lengine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(lengine != 0);

    return qRound(qMax(engine->maxCharWidth(), lengine->maxCharWidth()));
}

/*!
    Returns true if character \a ch is a valid character in the font;
    otherwise returns false.
*/
bool QFontMetrics::inFont(QChar ch) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR(script, ch);

    QFontEngine *engine = d->engineForScript(script);
    Q_ASSERT(engine != 0);

    if (engine->type() == QFontEngine::Box) return false;
    return engine->canRender(&ch, 1);
}

/*! \fn int QFontMetrics::leftBearing(QChar ch) const
    Returns the left bearing of character \a ch in the font.

    The left bearing is the right-ward distance of the left-most pixel
    of the character from the logical origin of the character. This
    value is negative if the pixels of the character extend to the
    left of the logical origin.

    See width(QChar) for a graphical description of this metric.

    \sa rightBearing(), minLeftBearing(), width()
*/
int QFontMetrics::leftBearing(QChar ch) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR(script, ch);

    QFontEngine *engine = d->engineForScript(script);
    Q_ASSERT(engine != 0);

    if (engine->type() == QFontEngine::Box) return 0;

    QGlyphLayout glyphs[10];
    int nglyphs = 9;
    engine->stringToCMap(&ch, 1, glyphs, &nglyphs, 0);
    // ### can nglyphs != 1 happen at all? Not currently I think
    glyph_metrics_t gi = engine->boundingBox(glyphs[0].glyph);
    return qRound(gi.x);
}

/*! \fn int QFontMetrics::rightBearing(QChar ch) const
    Returns the right bearing of character \a ch in the font.

    The right bearing is the left-ward distance of the right-most
    pixel of the character from the logical origin of a subsequent
    character. This value is negative if the pixels of the character
    extend to the right of the width() of the character.

    See width() for a graphical description of this metric.

    \sa leftBearing(), minRightBearing(), width()
*/
int QFontMetrics::rightBearing(QChar ch) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR(script, ch);

    QFontEngine *engine = d->engineForScript(script);
    Q_ASSERT(engine != 0);

    if (engine->type() == QFontEngine::Box) return 0;

    QGlyphLayout glyphs[10];
    int nglyphs = 9;
    engine->stringToCMap(&ch, 1, glyphs, &nglyphs, 0);
    // ### can nglyphs != 1 happen at all? Not currently I think
    glyph_metrics_t gi = engine->boundingBox(glyphs[0].glyph);
    return qRound(gi.xoff - gi.x - gi.width);
}

/*!
    Returns the width in pixels of the first \a len characters of \a
    str. If \a len is negative (the default), the entire string is
    used.

    Note that this value is \e not equal to boundingRect().width();
    boundingRect() returns a rectangle describing the pixels this
    string will cover whereas width() returns the distance to where
    the next string should be drawn.

    \sa boundingRect()
*/
int QFontMetrics::width(const QString &str, int len) const
{
    if (len < 0)
        len = str.length();
    if (len == 0)
        return 0;

    QTextEngine layout(str, d);
    layout.setMode(QTextEngine::WidthOnly);
    layout.itemize();
    return qRound(layout.width(0, len));
}

/*! \fn int QFontMetrics::width(QChar ch) const

    \overload

    \img bearings.png Bearings

    Returns the logical width of character \a ch in pixels. This is a
    distance appropriate for drawing a subsequent character after \a
    ch.

    Some of the metrics are described in the image to the right. The
    central dark rectangles cover the logical width() of each
    character. The outer pale rectangles cover the leftBearing() and
    rightBearing() of each character. Notice that the bearings of "f"
    in this particular font are both negative, while the bearings of
    "o" are both positive.

    \warning This function will produce incorrect results for Arabic
    characters or non-spacing marks in the middle of a string, as the
    glyph shaping and positioning of marks that happens when
    processing strings cannot be taken into account. Use charWidth()
    instead if you aren't looking for the width of isolated
    characters.

    \sa boundingRect(), charWidth()
*/
int QFontMetrics::width(QChar ch) const
{
    if (::category(ch) == QChar::Mark_NonSpacing)
        return 0;

    QFont::Script script;
    SCRIPT_FOR_CHAR(script, ch);

    QFontEngine *engine = d->engineForScript(script);
    Q_ASSERT(engine != 0);

    QGlyphLayout glyphs[8];
    int nglyphs = 7;
    engine->stringToCMap(&ch, 1, glyphs, &nglyphs, 0);
    return qRound(glyphs[0].advance.x());
}

/*! \fn int QFontMetrics::charWidth(const QString &str, int pos) const
    Returns the width of the character at position \a pos in the
    string \a str.

    The whole string is needed, as the glyph drawn may change
    depending on the context (the letter before and after the current
    one) for some languages (e.g. Arabic).

    This function also takes non spacing marks and ligatures into
    account.
*/

/*!
    Returns the bounding rectangle of the first \a len characters of
    \a str, which is the set of pixels the text would cover if drawn
    at (0, 0).

    If \a len is negative (the default), the entire string is used.

    Note that the bounding rectangle may extend to the left of (0, 0),
    e.g. for italicized fonts, and that the text output may cover \e
    all pixels in the bounding rectangle.

    Newline characters are processed as normal characters, \e not as
    linebreaks.

    Due to the different actual character heights, the height of the
    bounding rectangle of e.g. "Yes" and "yes" may be different.

    \sa width(), QPainter::boundingRect()
*/
QRect QFontMetrics::boundingRect(const QString &str) const
{
    if (str.length() == 0)
        return QRect();

    QTextEngine layout(str, d);
    layout.setMode(QTextEngine::WidthOnly);
    layout.itemize();
    glyph_metrics_t gm = layout.boundingBox(0, str.length());
    return QRect(qRound(gm.x), qRound(gm.y), qRound(gm.width), qRound(gm.height));
}

/*!
    Returns the bounding rectangle of the character \a ch relative to
    the left-most point on the base line.

    Note that the bounding rectangle may extend to the left of (0, 0),
    e.g. for italicized fonts, and that the text output may cover \e
    all pixels in the bounding rectangle.

    Note that the rectangle usually extends both above and below the
    base line.

    \sa width()
*/
QRect QFontMetrics::boundingRect(QChar ch) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR(script, ch);

    QFontEngine *engine = d->engineForScript(script);
    Q_ASSERT(engine != 0);

    QGlyphLayout glyphs[10];
    int nglyphs = 9;
    engine->stringToCMap(&ch, 1, glyphs, &nglyphs, 0);
    glyph_metrics_t gm = engine->boundingBox(glyphs[0].glyph);
    return QRect(qRound(gm.x), qRound(gm.y), qRound(gm.width), qRound(gm.height));
}

/*!
    \overload

    Returns the bounding rectangle of the first \a len characters of
    \a str, which is the set of pixels the text would cover if drawn
    at (0, 0). The drawing, and hence the bounding rectangle, is
    constrained to the rectangle \a r.

    If \a len is negative (which is the default), the entire string is
    used.

    The \a flgs argument is the bitwise OR of the following flags:
    \list
    \i \c Qt::AlignAuto aligns to the left border for all languages except
          Arabic and Hebrew where it aligns to the right.
    \i \c Qt::AlignLeft aligns to the left border.
    \i \c Qt::AlignRight aligns to the right border.
    \i \c Qt::AlignJustify produces justified text.
    \i \c Qt::AlignHCenter aligns horizontally centered.
    \i \c Qt::AlignTop aligns to the top border.
    \i \c Qt::AlignBottom aligns to the bottom border.
    \i \c Qt::AlignVCenter aligns vertically centered
    \i \c Qt::AlignCenter (== \c{Qt::AlignHCenter | Qt::AlignVCenter})
    \i \c Qt::TextSingleLine ignores newline characters in the text.
    \i \c Qt::TextExpandTabs expands tabs (see below)
    \i \c Qt::TextShowMnemonic interprets "&amp;x" as "<u>x</u>", i.e. underlined.
    \i \c Qt::TextWordBreak breaks the text to fit the rectangle.
    \endlist

    Qt::Horizontal alignment defaults to \c Qt::AlignAuto and vertical
    alignment defaults to \c Qt::AlignTop.

    If several of the horizontal or several of the vertical alignment
    flags are set, the resulting alignment is undefined.

    These flags are defined in \c qnamespace.h.

    If \c Qt::TextExpandTabs is set in \a flgs, then: if \a tabarray is
    non-null, it specifies a 0-terminated sequence of pixel-positions
    for tabs; otherwise if \a tabstops is non-zero, it is used as the
    tab spacing (in pixels).

    Note that the bounding rectangle may extend to the left of (0, 0),
    e.g. for italicized fonts, and that the text output may cover \e
    all pixels in the bounding rectangle.

    Newline characters are processed as linebreaks.

    Despite the different actual character heights, the heights of the
    bounding rectangles of "Yes" and "yes" are the same.

    The bounding rectangle returned by this function is somewhat larger
    than that calculated by the simpler boundingRect() function. This
    function uses the \link minLeftBearing() maximum left \endlink and
    \link minRightBearing() right \endlink font bearings as is
    necessary for multi-line text to align correctly. Also,
    fontHeight() and lineSpacing() are used to calculate the height,
    rather than individual character heights.

    \sa width(), QPainter::boundingRect(), Qt::Alignment
*/
QRect QFontMetrics::boundingRect(const QRect &r, int flgs, const QString& str, int tabstops, int *tabarray) const
{
    int tabarraylen=0;
    if (tabarray)
        while (tabarray[tabarraylen])
            tabarraylen++;

    QRectF rb;
    QRectF rr(r);
    qt_format_text(QFont(d), rr, flgs|Qt::TextDontPrint, str, &rb, tabstops, tabarray, tabarraylen, 0);

    return rb.toRect();
}

/*!
    Returns the size in pixels of \a text.

    The \a flgs argument is the bitwise OR of the following flags:
    \list
    \i \c Qt::TextSingleLine ignores newline characters.
    \i \c Qt::TextExpandTabs expands tabs (see below)
    \i \c Qt::TextShowMnemonic interprets "&amp;x" as "<u>x</u>", i.e. underlined.
    \i \c Qt::TextWordBreak breaks the text to fit the rectangle.
    \endlist

    These flags are defined in \c qnamespace.h.

    If \c Qt::TextExpandTabs is set in \a flgs, then: if \a tabarray is
    non-null, it specifies a 0-terminated sequence of pixel-positions
    for tabs; otherwise if \a tabstops is non-zero, it is used as the
    tab spacing (in pixels).

    Newline characters are processed as linebreaks.

    Despite the different actual character heights, the heights of the
    bounding rectangles of "Yes" and "yes" are the same.

    \sa boundingRect()
*/
QSize QFontMetrics::size(int flgs, const QString &text, int tabstops, int *tabarray) const
{
    return boundingRect(QRect(0,0,0,0), flgs, text, tabstops, tabarray).size();
}

/*!
    Returns the distance from the base line to where an underscore
    should be drawn.

    \sa overlinePos(), strikeOutPos(), lineWidth()
*/
int QFontMetrics::underlinePos() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    Q_ASSERT(engine != 0);

    return qRound(engine->underlinePosition());
}

/*!
    Returns the distance from the base line to where an overline
    should be drawn.

    \sa underlinePos(), strikeOutPos(), lineWidth()
*/
int QFontMetrics::overlinePos() const
{
    return ascent() + 1;
}

/*!
    Returns the distance from the base line to where the strikeout
    line should be drawn.

    \sa underlinePos(), overlinePos(), lineWidth()
*/
int QFontMetrics::strikeOutPos() const
{
    int pos = ascent() / 3;
    return pos > 0 ? pos : 1;
}

/*!
    Returns the width of the underline and strikeout lines, adjusted
    for the point size of the font.

    \sa underlinePos(), overlinePos(), strikeOutPos()
*/
int QFontMetrics::lineWidth() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    Q_ASSERT(engine != 0);

    return qRound(engine->lineThickness());
}




/*****************************************************************************
  QFontMetricsF member functions
 *****************************************************************************/

/*!
    \class QFontMetricsF qfontmetrics.h
    \brief The QFontMetricsF class provides font metrics information.

    \ingroup multimedia
    \ingroup shared

    QFontMetricsF functions calculate the size of characters and
    strings for a given font. There are three ways you can create a
    QFontMetricsF object:

    \list 1
    \i Calling the QFontMetricsF constructor with a QFont creates a
    font metrics object for a screen-compatible font, i.e. the font
    cannot be a printer font<sup>*</sup>. If the font is changed
    later, the font metrics object is \e not updated.

    \i QWidget::fontMetrics() returns the font metrics for a widget's
    font. This is equivalent to QFontMetricsF(widget->font()). If the
    widget's font is changed later, the font metrics object is \e not
    updated.

    \i QPainter::fontMetrics() returns the font metrics for a
    painter's current font. If the painter's font is changed later, the
    font metrics object is \e not updated.
    \endlist

    <sup>*</sup> If you use a printer font the values returned may be
    inaccurate. Printer fonts are not always accessible so the nearest
    screen font is used if a printer font is supplied.

    Once created, the object provides functions to access the
    individual metrics of the font, its characters, and for strings
    rendered in the font.

    There are several functions that operate on the font: ascent(),
    descent(), height(), leading() and lineSpacing() return the basic
    size properties of the font. The underlinePos(), overlinePos(),
    strikeOutPos() and lineWidth() functions, return the properties of
    the line that underlines, overlines or strikes out the
    characters. These functions are all fast.

    There are also some functions that operate on the set of glyphs in
    the font: minLeftBearing(), minRightBearing() and maxWidth().
    These are by necessity slow, and we recommend avoiding them if
    possible.

    For each character, you can get its width(), leftBearing() and
    rightBearing() and find out whether it is in the font using
    inFont(). You can also treat the character as a string, and use
    the string functions on it.

    The string functions include width(), to return the width of a
    string in pixels (or points, for a printer), boundingRect(), to
    return a rectangle large enough to contain the rendered string,
    and size(), to return the size of that rectangle.

    Example:
    \code
    QFont font("times", 24);
    QFontMetricsF fm(font);
    int pixelsWide = fm.width("What's the width of this text?");
    int pixelsHigh = fm.height();
    \endcode

    \sa QFont QFontInfo QFontDatabase
*/

/*!
    \fn QFontMetricsF::QFontMetricsF(const QFontMetrics &fontMetrics)

    Constructs a font metrics object with floating point precision
    from the given \a fontMetrics object.
*/

/*!
    \fn QFontMetricsF &QFontMetricsF::operator=(const QFontMetrics &fontMetrics)

    Assigns \a fontMetrics to this font metrics object.
*/

/*!
    Constructs a font metrics object for \a font.

    The font metrics will be screen-compatible, i.e. the metrics you
    get if you use the font for drawing text on a \link QWidget
    widgets\endlink or \link QPixmap pixmaps\endlink, not on a
    QPicture or QPrinter.

    The font metrics object holds the information for the font that is
    passed in the constructor at the time it is created, and is not
    updated if the font's attributes are changed later.

    Use QFontMetricsF(const QFont &, QPaintDevice *) to get the font
    metrics that are compatible with a certain paint device.
*/
QFontMetricsF::QFontMetricsF(const QFont &font)
    : d(font.d), fscript(QFont::NoScript)
{
    int dpi = qt_defaultDpi();
    if (font.d->dpi != dpi) {
        d = new QFontPrivate(*font.d);
        d->dpi = dpi;
        d->screen = 0;
    } else {
        ++d->ref;
    }
}

/*!
    \overload

    Constructs a font metrics object for \a font using the given \a
    script.
*/
QFontMetricsF::QFontMetricsF(const QFont &font, QFont::Script script)
    : d(font.d), fscript(script)
{
    int dpi = qt_defaultDpi();
    if (font.d->dpi != dpi) {
        d = new QFontPrivate(*font.d);
        d->dpi = dpi;
        d->screen = 0;
    } else {
        ++d->ref;
    }
}

/*!
    Constructs a font metrics object for \a font and \a paintdevice.

    The font metrics will be compatible with the paintdevice passed.
    If the \a paintdevice is 0, the metrics will be screen-compatible,
    ie. the metrics you get if you use the font for drawing text on a
    \link QWidget widgets\endlink or \link QPixmap pixmaps\endlink,
    not on a QPicture or QPrinter.

    The font metrics object holds the information for the font that is
    passed in the constructor at the time it is created, and is not
    updated if the font's attributes are changed later.
*/
QFontMetricsF::QFontMetricsF(const QFont &font, QPaintDevice *paintdevice)
    : fscript(QFont::NoScript)
{
    int dpi = paintdevice ? paintdevice->logicalDpiY() : qt_defaultDpi();
#ifdef Q_WS_X11
    int screen = paintdevice ? qt_x11Info(paintdevice)->screen() : 0;
#else
    const int screen = 0;
#endif
    if (font.d->dpi != dpi || font.d->screen != screen ) {
        d = new QFontPrivate(*font.d);
        d->dpi = dpi;
        d->screen = screen;
    } else {
        d = font.d;
        ++d->ref;
    }

}

/*!
    Constructs a copy of \a fm.
*/
QFontMetricsF::QFontMetricsF(const QFontMetricsF &fm)
    : d(fm.d),  fscript(fm.fscript)
{
    ++d->ref;
}

/*!
    Destroys the font metrics object and frees all allocated
    resources.
*/
QFontMetricsF::~QFontMetricsF()
{
    if (!--d->ref)
        delete d;
}

/*!
    Assigns the font metrics \a fm to this font metrics object.
*/
QFontMetricsF &QFontMetricsF::operator=(const QFontMetricsF &fm)
{
    qAtomicAssign(d, fm.d);
    return *this;
}

/*!
  Returns true if the font metrics are equal to the \a other font
  metrics; otherwise returns false.

  Two font metrics are considered equal if they were constructed from the
  same QFont and the paint devices they were constructed for are
  considered to be compatible.
*/
bool QFontMetricsF::operator ==(const QFontMetricsF &other)
{
    return d == other.d;
}

/*!
  \fn bool QFontMetricsF::operator !=(const QFontMetricsF &other);

  Returns true if the font metrics are not equal to the \a other font
  metrics; otherwise returns false.

  \sa operator==()
*/

/*!
    Returns the ascent of the font.

    The ascent of a font is the distance from the baseline to the
    highest position characters extend to. In practice, some font
    designers break this rule, e.g. when they put more than one accent
    on top of a character, or to accommodate an unusual character in
    an exotic language, so it is possible (though rare) that this
    value will be too small.

    \sa descent()
*/
qReal QFontMetricsF::ascent() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *latin_engine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(latin_engine != 0);

    return qMax(engine->ascent(), latin_engine->ascent());
}


/*!
    Returns the descent of the font.

    The descent is the distance from the base line to the lowest point
    characters extend to. (Note that this is different from X, which
    adds 1 pixel.) In practice, some font designers break this rule,
    e.g. to accommodate an unusual character in an exotic language, so
    it is possible (though rare) that this value will be too small.

    \sa ascent()
*/
qReal QFontMetricsF::descent() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *latin_engine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(latin_engine != 0);

    return qMax(engine->descent(), latin_engine->descent());
}

/*!
    Returns the height of the font.

    This is always equal to ascent()+descent()+1 (the 1 is for the
    base line).

    \sa leading(), lineSpacing()
*/
qReal QFontMetricsF::height() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *latin_engine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(latin_engine != 0);

    return qMax(engine->ascent(), latin_engine->ascent()) +
        qMax(engine->descent(), latin_engine->descent()) + 1;
}

/*!
    Returns the leading of the font.

    This is the natural inter-line spacing.

    \sa height(), lineSpacing()
*/
qReal QFontMetricsF::leading() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *latin_engine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(latin_engine != 0);

    return qMax(engine->leading(), latin_engine->leading());
}

/*!
    Returns the distance from one base line to the next.

    This value is always equal to leading()+height().

    \sa height(), leading()
*/
qReal QFontMetricsF::lineSpacing() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *latin_engine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(latin_engine != 0);

    return qMax(engine->leading(), latin_engine->leading()) +
            qMax(engine->ascent(), latin_engine->ascent()) +
            qMax(engine->descent(), latin_engine->descent()) + 1;
}

/*!
    Returns the minimum left bearing of the font.

    This is the smallest leftBearing(char) of all characters in the
    font.

    Note that this function can be very slow if the font is large.

    \sa minRightBearing(), leftBearing()
*/
qReal QFontMetricsF::minLeftBearing() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *latin_engine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(latin_engine != 0);

    return qMin(engine->minLeftBearing(), latin_engine->minLeftBearing());
}

/*!
    Returns the minimum right bearing of the font.

    This is the smallest rightBearing(char) of all characters in the
    font.

    Note that this function can be very slow if the font is large.

    \sa minLeftBearing(), rightBearing()
*/
qReal QFontMetricsF::minRightBearing() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *latin_engine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(latin_engine != 0);

    return qMin(engine->minRightBearing(), latin_engine->minRightBearing());
}

/*!
    Returns the width of the widest character in the font.
*/
qReal QFontMetricsF::maxWidth() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    QFontEngine *lengine = d->engineForScript(QFont::Latin);
    Q_ASSERT(engine != 0);
    Q_ASSERT(lengine != 0);

    return qMax(engine->maxCharWidth(), lengine->maxCharWidth());
}

/*!
    Returns true if character \a ch is a valid character in the font;
    otherwise returns false.
*/
bool QFontMetricsF::inFont(QChar ch) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR(script, ch);

    QFontEngine *engine = d->engineForScript(script);
    Q_ASSERT(engine != 0);

    if (engine->type() == QFontEngine::Box) return false;
    return engine->canRender(&ch, 1);
}

/*! \fn int QFontMetricsF::leftBearing(QChar ch) const
    Returns the left bearing of character \a ch in the font.

    The left bearing is the right-ward distance of the left-most pixel
    of the character from the logical origin of the character. This
    value is negative if the pixels of the character extend to the
    left of the logical origin.

    See width(QChar) for a graphical description of this metric.

    \sa rightBearing(), minLeftBearing(), width()
*/
qReal QFontMetricsF::leftBearing(QChar ch) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR(script, ch);

    QFontEngine *engine = d->engineForScript(script);
    Q_ASSERT(engine != 0);

    if (engine->type() == QFontEngine::Box) return 0;

    QGlyphLayout glyphs[10];
    int nglyphs = 9;
    engine->stringToCMap(&ch, 1, glyphs, &nglyphs, 0);
    // ### can nglyphs != 1 happen at all? Not currently I think
    glyph_metrics_t gi = engine->boundingBox(glyphs[0].glyph);
    return gi.x;
}

/*! \fn int QFontMetricsF::rightBearing(QChar ch) const
    Returns the right bearing of character \a ch in the font.

    The right bearing is the left-ward distance of the right-most
    pixel of the character from the logical origin of a subsequent
    character. This value is negative if the pixels of the character
    extend to the right of the width() of the character.

    See width() for a graphical description of this metric.

    \sa leftBearing(), minRightBearing(), width()
*/
qReal QFontMetricsF::rightBearing(QChar ch) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR(script, ch);

    QFontEngine *engine = d->engineForScript(script);
    Q_ASSERT(engine != 0);

    if (engine->type() == QFontEngine::Box) return 0;

    QGlyphLayout glyphs[10];
    int nglyphs = 9;
    engine->stringToCMap(&ch, 1, glyphs, &nglyphs, 0);
    // ### can nglyphs != 1 happen at all? Not currently I think
    glyph_metrics_t gi = engine->boundingBox(glyphs[0].glyph);
    return gi.xoff - gi.x - gi.width;
}

/*!
    \fn qReal QFontMetricsF::width(const QString &text) const

    Returns the width in pixels of the characters in the given \a text.

    Note that this value is \e not equal to the width returned by
    boundingRect().width() because boundingRect() returns a rectangle
    describing the pixels this string will cover whereas width()
    returns the distance to where the next string should be drawn.

    \sa boundingRect()
*/
qReal QFontMetricsF::width(const QString &str) const
{
    QTextEngine layout(str, d);
    layout.setMode(QTextEngine::WidthOnly);
    layout.itemize();
    return layout.width(0, str.length());
}

/*! \fn int QFontMetricsF::width(QChar ch) const

    \overload

    \img bearings.png Bearings

    Returns the logical width of character \a ch in pixels. This is a
    distance appropriate for drawing a subsequent character after \a
    ch.

    Some of the metrics are described in the image to the right. The
    central dark rectangles cover the logical width() of each
    character. The outer pale rectangles cover the leftBearing() and
    rightBearing() of each character. Notice that the bearings of "f"
    in this particular font are both negative, while the bearings of
    "o" are both positive.

    \warning This function will produce incorrect results for Arabic
    characters or non-spacing marks in the middle of a string, as the
    glyph shaping and positioning of marks that happens when
    processing strings cannot be taken into account. Use charWidth()
    instead if you aren't looking for the width of isolated
    characters.

    \sa boundingRect(), charWidth()
*/
qReal QFontMetricsF::width(QChar ch) const
{
    if (::category(ch) == QChar::Mark_NonSpacing)
        return 0.;

    QFont::Script script;
    SCRIPT_FOR_CHAR(script, ch);

    QFontEngine *engine = d->engineForScript(script);
    Q_ASSERT(engine != 0);

    QGlyphLayout glyphs[8];
    int nglyphs = 7;
    engine->stringToCMap(&ch, 1, glyphs, &nglyphs, 0);
    return glyphs[0].advance.x();
}

/*!
    \fn QRectF QFontMetricsF::boundingRect(const QString &text) const

    Returns the bounding rectangle of the characters in the given
    \a text. This is the set of pixels the text would cover if drawn
    at (0, 0).

    Note that the bounding rectangle may extend to the left of (0, 0),
    e.g. for italicized fonts, and that the text output may cover \e
    all pixels in the bounding rectangle.

    Newline characters are processed as normal characters, \e not as
    line breaks.

    Due to the different actual character heights, the height of the
    bounding rectangle of e.g. "Yes" and "yes" may be different.

    \sa width(), QPainter::boundingRect()
*/
QRectF QFontMetricsF::boundingRect(const QString &str) const
{
    int len = str.length();
    if (len == 0)
        return QRectF();

    QTextEngine layout(str, d);
    layout.setMode(QTextLayout::NoBidi|QTextLayout::SingleLine);
    layout.itemize();
    glyph_metrics_t gm = layout.boundingBox(0, len);
    return QRectF(gm.x, gm.y, gm.width, gm.height);
}

/*!
    Returns the bounding rectangle of the character \a ch relative to
    the left-most point on the base line.

    Note that the bounding rectangle may extend to the left of (0, 0),
    e.g. for italicized fonts, and that the text output may cover \e
    all pixels in the bounding rectangle.

    Note that the rectangle usually extends both above and below the
    base line.

    \sa width()
*/
QRectF QFontMetricsF::boundingRect(QChar ch) const
{
    QFont::Script script;
    SCRIPT_FOR_CHAR(script, ch);

    QFontEngine *engine = d->engineForScript(script);
    Q_ASSERT(engine != 0);

    QGlyphLayout glyphs[10];
    int nglyphs = 9;
    engine->stringToCMap(&ch, 1, glyphs, &nglyphs, 0);
    glyph_metrics_t gm = engine->boundingBox(glyphs[0].glyph);
    return QRectF(gm.x, gm.y, gm.width, gm.height);
}

/*!
    \fn QRectF QFontMetricsF::boundingRect(const QRectF &rect, int flags, const QString &text, int tabstops, int *tabarray) const
    \overload

    Returns the bounding rectangle of the characters in the given \a text.
    This is the set of pixels the text would cover if drawn when constrained
    to the bounding rectangle specified by \a rect.

    The \a flags argument is the bitwise OR of the following flags:
    \list
    \i \c Qt::AlignAuto aligns to the left border for all languages except
          Arabic and Hebrew where it aligns to the right.
    \i \c Qt::AlignLeft aligns to the left border.
    \i \c Qt::AlignRight aligns to the right border.
    \i \c Qt::AlignJustify produces justified text.
    \i \c Qt::AlignHCenter aligns horizontally centered.
    \i \c Qt::AlignTop aligns to the top border.
    \i \c Qt::AlignBottom aligns to the bottom border.
    \i \c Qt::AlignVCenter aligns vertically centered
    \i \c Qt::AlignCenter (== \c{Qt::AlignHCenter | Qt::AlignVCenter})
    \i \c Qt::TextSingleLine ignores newline characters in the text.
    \i \c Qt::TextExpandTabs expands tabs (see below)
    \i \c Qt::TextShowMnemonic interprets "&amp;x" as "<u>x</u>", i.e. underlined.
    \i \c Qt::TextWordBreak breaks the text to fit the rectangle.
    \endlist

    Qt::Horizontal alignment defaults to \c Qt::AlignAuto and vertical
    alignment defaults to \c Qt::AlignTop.

    If several of the horizontal or several of the vertical alignment
    flags are set, the resulting alignment is undefined.

    These flags are defined in \l{Qt::AlignmentFlag}.

    If \c Qt::TextExpandTabs is set in \a flags, the following behavior is
    used to interpret tab characters in the text:
    \list
    \i If \a tabarray is non-null, it specifies a 0-terminated sequence of
       pixel-positions for tabs in the text.
    \i If \a tabstops is non-zero, it is used as the tab spacing (in pixels).
    \endlist

    Note that the bounding rectangle may extend to the left of (0, 0),
    e.g. for italicized fonts.

    Newline characters are processed as line breaks.

    Despite the different actual character heights, the heights of the
    bounding rectangles of "Yes" and "yes" are the same.

    The bounding rectangle returned by this function is somewhat larger
    than that calculated by the simpler boundingRect() function. This
    function uses the \link minLeftBearing() maximum left \endlink and
    \link minRightBearing() right \endlink font bearings as is
    necessary for multi-line text to align correctly. Also,
    fontHeight() and lineSpacing() are used to calculate the height,
    rather than individual character heights.

    \sa width(), QPainter::boundingRect(), Qt::Alignment
*/
QRectF QFontMetricsF::boundingRect(const QRectF &r, int flgs, const QString& str,
                                   int tabstops, int *tabarray) const
{
    int tabarraylen=0;
    if (tabarray)
        while (tabarray[tabarraylen])
            tabarraylen++;

    QRectF rb;
    qt_format_text(QFont(d), r, flgs|Qt::TextDontPrint, str, &rb, tabstops, tabarray, tabarraylen, 0);

    return rb;
}

/*!
    \fn QSizeF QFontMetricsF::size(int flags, const QString &text, int tabstops, int *tabarray) const

    Returns the size in pixels of the characters in the given \a text.

    The \a flags argument is the bitwise OR of the following flags:
    \list
    \i \c Qt::TextSingleLine ignores newline characters.
    \i \c Qt::TextExpandTabs expands tabs (see below)
    \i \c Qt::TextShowMnemonic interprets "&amp;x" as "<u>x</u>", i.e. underlined.
    \i \c Qt::TextWordBreak breaks the text to fit the rectangle.
    \endlist

    These flags are defined in \l{Qt::TextFlags}.

    If \c Qt::TextExpandTabs is set in \a flags, the following behavior is
    used to interpret tab characters in the text:
    \list
    \i If \a tabarray is non-null, it specifies a 0-terminated sequence of
       pixel-positions for tabs in the text.
    \i If \a tabstops is non-zero, it is used as the tab spacing (in pixels).
    \endlist

    Newline characters are processed as line breaks.

    Note: Despite the different actual character heights, the heights of the
    bounding rectangles of "Yes" and "yes" are the same.

    \sa boundingRect()
*/
QSizeF QFontMetricsF::size(int flgs, const QString &str, int tabstops, int *tabarray) const
{
    return boundingRect(QRectF(), flgs, str, tabstops, tabarray).size();
}

/*!
    Returns the distance from the base line to where an underscore
    should be drawn.

    \sa overlinePos(), strikeOutPos(), lineWidth()
*/
qReal QFontMetricsF::underlinePos() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    Q_ASSERT(engine != 0);

    return engine->underlinePosition();
}

/*!
    Returns the distance from the base line to where an overline
    should be drawn.

    \sa underlinePos(), strikeOutPos(), lineWidth()
*/
qReal QFontMetricsF::overlinePos() const
{
    return ascent() + 1;
}

/*!
    Returns the distance from the base line to where the strikeout
    line should be drawn.

    \sa underlinePos(), overlinePos(), lineWidth()
*/
qReal QFontMetricsF::strikeOutPos() const
{
    return ascent() / 3.;
}

/*!
    Returns the width of the underline and strikeout lines, adjusted
    for the point size of the font.

    \sa underlinePos(), overlinePos(), strikeOutPos()
*/
qReal QFontMetricsF::lineWidth() const
{
    QFontEngine *engine = d->engineForScript((QFont::Script) fscript);
    Q_ASSERT(engine != 0);

    return engine->lineThickness();
}


