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

#ifndef QTEXTENGINE_P_H
#define QTEXTENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qglobal.h"
#include "qstring.h"
#include "qnamespace.h"
#include "qtextlayout.h"
#include "qtextformat_p.h"
#include "qfont_p.h"
#include <qvector.h>
#include <qpaintengine.h>
#include "qtextobject.h"
#include "qtextoption.h"
#include "qtextdocument_p.h"
#include "qset.h"

#include <stdlib.h>
#ifndef Q_OS_TEMP
#include <assert.h>
#endif // Q_OS_TEMP

class QFontPrivate;
class QFontEngine;

class QString;
class QOpenType;
class QPainter;

class QAbstractTextDocumentLayout;

class QTextItemInt : public QTextItem
{
public:
    qreal descent;
    qreal ascent;
    qreal width;

    RenderFlags flags;
    int num_chars;
    const QChar *chars;
    const QFont *f;

    QGlyphLayout *glyphs;
    int num_glyphs;
    QFontEngine *fontEngine;
};


// this uses the same coordinate system as Qt, but a different one to freetype.
// * y is usually negative, and is equal to the ascent.
// * negative yoff means the following stuff is drawn higher up.
// the characters bounding rect is given by QRect(x,y,width,height), it's advance by
// xoo and yoff
struct glyph_metrics_t
{
    inline glyph_metrics_t()
        : x(100000), y(100000),
          width(0), height(0), xoff(0), yoff(0)
        {}
    inline glyph_metrics_t(qreal _x, qreal _y, qreal _width, qreal _height, qreal _xoff, qreal _yoff)
        : x(_x),
          y(_y),
          width(_width),
          height(_height),
          xoff(_xoff),
          yoff(_yoff)
        {}
    qreal x;
    qreal y;
    qreal width;
    qreal height;
    qreal xoff;
    qreal yoff;
};
Q_DECLARE_TYPEINFO(glyph_metrics_t, Q_PRIMITIVE_TYPE);

typedef unsigned int glyph_t;

#if defined(Q_WS_X11) || defined (Q_WS_QWS) || defined (Q_WS_MAC)


struct QScriptAnalysis
{
    unsigned short script    : 7;
    unsigned short override  : 1;  // Set when in LRO/RLO embedding
    unsigned short bidiLevel : 6;  // Unicode Bidi algorithm embedding level (0-61)
    unsigned short reserved  : 2;
    bool operator == (const QScriptAnalysis &other) {
        return
            script == other.script &&
            bidiLevel == other.bidiLevel;
        // ###
//             && override == other.override;
    }

};
Q_DECLARE_TYPEINFO(QScriptAnalysis, Q_PRIMITIVE_TYPE);

#elif defined(Q_WS_WIN)

struct QScriptAnalysis {
    unsigned short script         :10;
    unsigned short rtl            :1;
    unsigned short layoutRTL      :1;
    unsigned short linkBefore     :1;
    unsigned short linkAfter      :1;
    unsigned short logicalOrder   :1;
    unsigned short noGlyphIndex   :1;
    unsigned short bidiLevel         :5;
    unsigned short override          :1;
    unsigned short inhibitSymSwap    :1;
    unsigned short charShape         :1;
    unsigned short digitSubstitute   :1;
    unsigned short inhibitLigate     :1;
    unsigned short fDisplayZWG        :1;
    unsigned short arabicNumContext  :1;
    unsigned short gcpClusters       :1;
    unsigned short reserved          :1;
    unsigned short engineReserved    :2;
};
Q_DECLARE_TYPEINFO(QScriptAnalysis, Q_PRIMITIVE_TYPE);

inline bool operator == (const QScriptAnalysis &sa1, const QScriptAnalysis &sa2)
{
    return
        sa1.script == sa2.script &&
        sa1.bidiLevel == sa2.bidiLevel;
        // ###
//             && override == other.override;
}

#endif

struct QGlyphLayout
{
    inline QGlyphLayout()
        : glyph(0), justificationType(0), nKashidas(0), space_18d6(0)
        {}

    // highest value means highest priority for justification. Justification is done by first inserting kashidas
    // starting with the highest priority positions, then stretching spaces, afterwards extending inter char
    // spacing, and last spacing between arabic words.
    // NoJustification is for example set for arabic where no Kashida can be inserted or for diacritics.
    enum Justification {
        NoJustification= 0,   // Justification can't be applied after this glyph
        Arabic_Space   = 1,   // This glyph represents a space inside arabic text
        Character      = 2,   // Inter-character justification point follows this glyph
        Space          = 4,   // This glyph represents a blank outside an Arabic run
        Arabic_Normal  = 7,   // Normal Middle-Of-Word glyph that connects to the right (begin)
        Arabic_Waw     = 8,    // Next character is final form of Waw/Ain/Qaf/Fa
        Arabic_BaRa    = 9,   // Next two chars are Ba + Ra/Ya/AlefMaksura
        Arabic_Alef    = 10,  // Next character is final form of Alef/Tah/Lam/Kaf/Gaf
        Arabic_HaaDal  = 11,  // Next character is final form of Haa/Dal/Taa Marbutah
        Arabic_Seen    = 12,  // Initial or Medial form Of Seen/Sad
        Arabic_Kashida = 13   // Kashida(U+640) in middle of word
    };

    glyph_t glyph;
    struct Attributes {
        unsigned short justification   :4;  // Justification class
        unsigned short clusterStart    :1;  // First glyph of representation of cluster
        unsigned short mark            :1;  // needs to be positioned around base char
        unsigned short zeroWidth       :1;  // ZWJ, ZWNJ etc, with no width
        unsigned short dontPrint       :1;
        unsigned short combiningClass  :8;
    };
    Attributes attributes;
    QPointF advance;
    QPointF offset;

    enum JustificationType {
        JustifyNone,
        JustifySpace,
        JustifyKashida
    };
    uint justificationType :2;
    uint nKashidas : 6; // more do not make sense...
    uint space_18d6 : 24;
};
Q_DECLARE_TYPEINFO(QGlyphLayout, Q_PRIMITIVE_TYPE);

// also this is compatible to uniscribe. Do not change.
struct QCharAttributes {
    uchar softBreak      :1;     // Potential linebreak point _before_ this character
    uchar whiteSpace     :1;     // A unicode whitespace character, except NBSP, ZWNBSP
    uchar charStop       :1;     // Valid cursor position (for left/right arrow)
    uchar wordStop       :1;     // Valid cursor position (for ctrl + left/right arrow)
    uchar invalid        :1;
    uchar reserved       :3;
};
Q_DECLARE_TYPEINFO(QCharAttributes, Q_PRIMITIVE_TYPE);

struct QScriptItem
{
    inline QScriptItem() : position(0), isSpace(false), isTab(false),
                           isObject(false),
                           num_glyphs(0), descent(-1), ascent(-1), width(-1),
                           glyph_data_offset(0)
        { }

    int position;
    QScriptAnalysis analysis;
    unsigned short isSpace  : 1;
    unsigned short isTab    : 1;
    unsigned short isObject : 1;
    int num_glyphs;
    qreal descent;
    qreal ascent;
    qreal width;
    int glyph_data_offset;
    qreal height() const { return ascent + descent; }
};


Q_DECLARE_TYPEINFO(QScriptItem, Q_MOVABLE_TYPE);

typedef QVector<QScriptItem> QScriptItemArray;

struct QScriptLine
{
    QScriptLine()
        : descent(0), ascent(0), x(0), y(0), width(0), textWidth(0) {}
    qreal descent;
    qreal ascent;
    qreal x;
    qreal y;
    qreal width;
    qreal textWidth;
    int from;
    signed int length : 30;
    mutable uint justified : 1;
    mutable uint gridfitted : 1;
    qreal height() const { return ascent + descent + 1.; }
    void setDefaultHeight(QTextEngine *eng);
};
Q_DECLARE_TYPEINFO(QScriptLine, Q_PRIMITIVE_TYPE);

typedef QVector<QScriptLine> QScriptLineArray;

class QFontPrivate;
class QTextFormatCollection;

class Q_GUI_EXPORT QTextEngine {
public:
    QTextEngine();
    QTextEngine(const QString &str, QFontPrivate *f);
    ~QTextEngine();

    void setText(const QString &str);

    enum Mode {
        WidthOnly = 0x07
    };

    enum ShaperFlag {
        RightToLeft = 0x0001,
        Mirrored = 0x0001,
        DesignMetrics = 0x0002
    };
    Q_DECLARE_FLAGS(ShaperFlags, ShaperFlag)

    void invalidate();

    void validate() const;
    void itemize() const;

    static void bidiReorder(int numRuns, const quint8 *levels, int *visualOrder);

    const QCharAttributes *attributes();

    void shape(int item) const;

    void justify(const QScriptLine &si);

    qreal width(int charFrom, int numChars) const;
    glyph_metrics_t boundingBox(int from,  int len) const;

    int length(int item) const {
        const QScriptItem &si = layoutData->items[item];
        int from = si.position;
        item++;
        return (item < layoutData->items.size() ? layoutData->items[item].position : layoutData->string.length()) - from;
    }
    int length(const QScriptItem *si) const {
        int end;
        if (si + 1 < layoutData->items.constData()+ layoutData->items.size())
            end = (si+1)->position;
        else
            end = layoutData->string.length();
        return end - si->position;
    }

    QFontEngine *fontEngine(const QScriptItem &si) const;
    QFont font(const QScriptItem &si) const;
    QFont font() const;

    unsigned short *logClustersPtr;
    QGlyphLayout *glyphPtr;

    inline unsigned short *logClusters(const QScriptItem *si) const
        { return logClustersPtr+si->position; }
    inline QGlyphLayout *glyphs(const QScriptItem *si) const
        { return glyphPtr + si->glyph_data_offset; }

    void reallocate(int totalGlyphs);
    inline void ensureSpace(int nGlyphs) const {
        if (layoutData->num_glyphs - layoutData->used < nGlyphs)
            const_cast<QTextEngine *>(this)->reallocate((((layoutData->used + nGlyphs)*3/2 + 15) >> 4) << 4);
    }

    void freeMemory();

    int findItem(int strPos) const;
    inline QTextFormatCollection *formats() const {
        return block.docHandle()->formatCollection();
    }
    QTextFormat format(const QScriptItem *si) const;
    inline QAbstractTextDocumentLayout *docLayout() const {
        return block.docHandle()->document()->documentLayout();
    }
    int formatIndex(const QScriptItem *si) const;

    qreal nextTab(const QScriptItem *si, qreal x);

    mutable QScriptLineArray lines;

    QString text;
    QFontPrivate *fnt;
    QTextBlock block;

    QTextOption option;

    qreal minWidth;
    qreal maxWidth;
    QPointF position;
    uint ignoreBidi : 1;
    uint cacheGlyphs : 1;

    int *underlinePositions;

    struct LayoutData {
        LayoutData();
        ~LayoutData();
        mutable QScriptItemArray items;
        int allocated;
        void **memory;
        int num_glyphs;
        mutable int used;
        uint hasBidi : 1;
        uint inLayout : 1;
        bool haveCharAttributes;
        QString string;
    };
    mutable LayoutData *layoutData;

    struct SpecialData {
        int preeditPosition;
        QString preeditText;
        QList<QTextLayout::FormatRange> addFormats;
    };
    SpecialData *specialData;

    bool atWordSeparator(int position) const;

private:
    void setBoundary(int strPos) const;
    void addRequiredBoundaries() const;
    void shapeText(int item) const;
    void splitItem(int item, int pos) const;

};

#endif // QTEXTENGINE_P_H
