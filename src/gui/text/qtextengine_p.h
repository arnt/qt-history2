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

#ifndef QTEXTENGINE_P_H
#define QTEXTENGINE_P_H

#ifndef QT_H
#include "qglobal.h"
#include "qstring.h"
#include "qnamespace.h"
#include "qtextlayout.h"
#include "qtextformat_p.h"
#include <private/qfontdata_p.h>
#include <qvector.h>
#include <qpaintengine.h>
#endif // QT_H

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

enum q26Dot6 { F26Dot6 };

class Q26Dot6 {
public:
    Q26Dot6() : val(0) {}
    Q26Dot6(int f26d6, q26Dot6) : val(f26d6) {}
    Q26Dot6(int i) : val(i<<6) {}
    Q26Dot6(unsigned int i) : val(i<<6) {}
    Q26Dot6(long i) : val(i<<6) {}
    Q26Dot6(double d) { val = (int)(d*64.); }
    Q26Dot6(const Q26Dot6 &other) : val(other.val) {}
    Q26Dot6 & operator=(const Q26Dot6 &other) { val = other.val; return *this; }

    inline int value() const { return val; }
    inline Q26Dot6 &setValue(int value) { val = value; return *this; }

    inline int toInt() const { return (((val)+32) & -64)>>6; }
    inline double toDouble() const { return ((double)val)/64.; }

    inline int truncate() const { return val>>6; }
    inline Q26Dot6 round() const { return Q26Dot6(((val)+32) & -64, F26Dot6); }
    inline Q26Dot6 floor() const { return Q26Dot6((val) & -64, F26Dot6); }
    inline Q26Dot6 ceil() const { return Q26Dot6((val+63) & -64, F26Dot6); }

    inline Q26Dot6 operator+(const Q26Dot6 &other) const { return Q26Dot6(val + other.val, F26Dot6); }
    inline Q26Dot6 &operator+=(const Q26Dot6 &other) { val += other.val; return *this; }
    inline Q26Dot6 operator-(const Q26Dot6 &other) const { return Q26Dot6(val - other.val, F26Dot6); }
    inline Q26Dot6 &operator-=(const Q26Dot6 &other) { val -= other.val; return *this; }
    inline Q26Dot6 operator-() const { return Q26Dot6(-val, F26Dot6); }

    inline bool operator==(const Q26Dot6 &other) const { return val == other.val; }
    inline bool operator!=(const Q26Dot6 &other) const { return val != other.val; }
    inline bool operator<(const Q26Dot6 &other) const { return val < other.val; }
    inline bool operator>(const Q26Dot6 &other) const { return val > other.val; }
    inline bool operator<=(const Q26Dot6 &other) const { return val <= other.val; }
    inline bool operator>=(const Q26Dot6 &other) const { return val >= other.val; }
    inline bool operator!() const { return !val; }

    inline Q26Dot6 &operator/=(int d) { val /= d; return *this; }
    inline Q26Dot6 &operator/=(double d) { val = (int)(val/d); return *this; }
    inline Q26Dot6 &operator/=(const Q26Dot6 &o) {
        if (o == Q26Dot6()) {
            val =0x7FFFFFFFL;
        } else {
            bool neg = false;
            Q_INT64 a = val;
            Q_INT64 b = o.val;
            if (a < 0) { a = -a; neg = true; }
            if (b < 0) { b = -b; neg = !neg; }

            int res = (int)(((a << 6) + (b >> 1)) / b);

            val = (neg ? -res : res);
        }
        return *this;
    }
    inline Q26Dot6 operator/(int d) const { return Q26Dot6(val/d, F26Dot6); }
    inline Q26Dot6 operator/(double d) const { return Q26Dot6((int)(val/d), F26Dot6); }
    inline Q26Dot6 operator/(const Q26Dot6 &b) const { Q26Dot6 v = *this; return (v /= b); }
    inline Q26Dot6 &operator*=(int i) { val *= i; return *this; }
    inline Q26Dot6 &operator*=(double d) { val = (int) (val*d); return *this; }
    inline Q26Dot6 &operator*=(const Q26Dot6 &o) {
        bool neg = false;
        Q_INT64 a = val;
        Q_INT64 b = o.val;
        if (a < 0) { a = -a; neg = true; }
        if (b < 0) { b = -b; neg = !neg; }

        int res = (int)((a * b + 0x20L) >> 6);
        val = neg ? -res : res;
        return *this;
    }
    inline Q26Dot6 operator*(int i) const { return Q26Dot6(val*i, F26Dot6); }
    inline Q26Dot6 operator*(double d) const { return Q26Dot6((int)(val*d), F26Dot6); }
    inline Q26Dot6 operator*(const Q26Dot6 &o) const { Q26Dot6 v = *this; return (v *= o); }

private:
    int val;
};
Q_DECLARE_TYPEINFO(Q26Dot6, Q_PRIMITIVE_TYPE);

inline Q26Dot6 operator*(int i, const Q26Dot6 &d) { return d*i; }
inline Q26Dot6 operator*(double d, const Q26Dot6 &d2) { return d2*d; }


struct Q26Dot6Offset {
    Q26Dot6 x;
    Q26Dot6 y;
};

// this uses the same coordinate system as Qt, but a different one to freetype and Xft.
// * y is usually negative, and is equal to the ascent.
// * negative yoff means the following stuff is drawn higher up.
// the characters bounding rect is given by QRect(x,y,width,height), it's advance by
// xoo and yoff
struct glyph_metrics_t
{
    inline glyph_metrics_t()
        : x(100000),
          y(100000)
        {}
    inline glyph_metrics_t(Q26Dot6 _x, Q26Dot6 _y, Q26Dot6 _width, Q26Dot6 _height, Q26Dot6 _xoff, Q26Dot6 _yoff)
        : x(_x),
          y(_y),
          width(_width),
          height(_height),
          xoff(_xoff),
          yoff(_yoff)
        {}
    Q26Dot6 x;
    Q26Dot6 y;
    Q26Dot6 width;
    Q26Dot6 height;
    Q26Dot6 xoff;
    Q26Dot6 yoff;
};
Q_DECLARE_TYPEINFO(glyph_metrics_t, Q_PRIMITIVE_TYPE);

typedef unsigned short glyph_t;

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

    unsigned short glyph;
    struct Attributes {
        unsigned short justification   :4;  // Justification class
        unsigned short clusterStart    :1;  // First glyph of representation of cluster
        unsigned short mark            :1;  // needs to be positioned around base char
        unsigned short zeroWidth       :1;  // ZWJ, ZWNJ etc, with no width
        unsigned short dontPrint       :1;
        unsigned short combiningClass  :8;
    };
    Attributes attributes;
    Q26Dot6Offset advance;
    Q26Dot6Offset offset;

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
                           glyph_data_offset(0),
                           format(-1) { }

    int position;
    QScriptAnalysis analysis;
    unsigned short isSpace  : 1;
    unsigned short isTab    : 1;
    unsigned short isObject : 1;
    int num_glyphs;
    Q26Dot6 descent;
    Q26Dot6 ascent;
    Q26Dot6 width;
    int glyph_data_offset;
    int format;
};


Q_DECLARE_TYPEINFO(QScriptItem, Q_MOVABLE_TYPE);

typedef QVector<QScriptItem> QScriptItemArray;

struct QScriptLine
{
    Q26Dot6 descent;
    Q26Dot6 ascent;
    Q26Dot6 x;
    Q26Dot6 y;
    Q26Dot6 width;
    Q26Dot6 textWidth;
    int from;
    uint length : 30;
    mutable uint justified : 1;
    mutable uint gridfitted : 1;
};
Q_DECLARE_TYPEINFO(QScriptLine, Q_PRIMITIVE_TYPE);

typedef QVector<QScriptLine> QScriptLineArray;

class QFontPrivate;
class QTextFormatCollection;
class QPalette;

class QTextEngine {
public:
    QTextEngine();
    QTextEngine(const QString &str, QFontPrivate *f);
    ~QTextEngine();

    void setText(const QString &str);
    void setFormatCollection(const QTextFormatCollection *fmts) {
        // ##### atomic!
        if (fmts != formats) {
            if (fmts) ++fmts->ref;
            if (formats && !--formats->ref)
                    delete formats;
            formats = fmts;
        }
    }
    void setDocumentLayout(QAbstractTextDocumentLayout *layout) { docLayout = layout; }

    enum Mode {
        Full = 0x00,
        NoBidi = 0x01,
        SingleLine = 0x02,
        WidthOnly = 0x07
    };

    enum ShaperFlagsEnum {
        RightToLeft = 0x0001,
        Mirrored = 0x0001,
        DesignMetrics = 0x0002
    };
    Q_DECLARE_FLAGS(ShaperFlags, ShaperFlagsEnum);


    void itemize(int mode = Full);

    static void bidiReorder(int numRuns, const Q_UINT8 *levels, int *visualOrder);

    const QCharAttributes *attributes();

    void setFormat(int from, int length, int format);
    void setBoundary(int strPos);

    void shape(int item) const;

    void justify(const QScriptLine &si);

    enum Edge {
        Leading,
        Trailing
    };

    Q26Dot6 width(int charFrom, int numChars) const;
    glyph_metrics_t boundingBox(int from,  int len) const;

    int length(int item) const {
        const QScriptItem &si = items[item];
        int from = si.position;
        item++;
        return (item < items.size() ? items[item].position : string.length()) - from;
    }

    QFontEngine *fontEngine(const QScriptItem &si) const;
    QFont font(const QScriptItem &si) const;
    QFont font() const { if (fnt) return QFont(fnt, 0); return QFont(); }

    void splitItem(int item, int pos);

    unsigned short *logClustersPtr;
    QGlyphLayout *glyphPtr;

    inline unsigned short *logClusters(const QScriptItem *si) const
        { return logClustersPtr+si->position; }
    inline QGlyphLayout *glyphs(const QScriptItem *si) const
        { return glyphPtr + si->glyph_data_offset; }

    void reallocate(int totalGlyphs);
    inline void ensureSpace(int nGlyphs) const {
        if (num_glyphs - used < nGlyphs)
            ((QTextEngine *)this)->reallocate((((used + nGlyphs)*3/2 + 15) >> 4) << 4);
    }


    int findItem(int strPos) const;

    mutable QScriptItemArray items;
    mutable QScriptLineArray lines;

    QString string;
    QFontPrivate *fnt;
    const QTextFormatCollection *formats;
    QChar::Direction direction : 5;
    unsigned int haveCharAttributes : 1;
    unsigned int widthOnly : 1;
    unsigned int designMetrics : 1;
    unsigned int reserved : 24;
    unsigned int textFlags;
    QPalette *pal;
    QAbstractTextDocumentLayout *docLayout;

    int allocated;
    void **memory;
    int num_glyphs;
    mutable int used;
    Q26Dot6 minWidth;
    QRect boundingRect;
    QPoint position;

    int cursorPos;
    const QTextLayout::Selection *selections;
    int nSelections;
    int *underlinePositions;
private:
    void shapeText(int item) const;
};

#endif
