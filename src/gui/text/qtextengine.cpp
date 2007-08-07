/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdebug.h"
#include "qtextformat.h"
#include "qtextformat_p.h"
#include "qtextengine_p.h"
#include "qabstracttextdocumentlayout.h"
#include "qtextlayout.h"
#include "qvarlengtharray.h"
#include "qfont.h"
#include "qfont_p.h"
#include "qfontengine_p.h"
#include "qstring.h"
#include <private/qunicodetables_p.h>
#include "qtextdocument_p.h"
#include <qapplication.h>
#include <stdlib.h>


// -----------------------------------------------------------------------------------------------------
//
// The BiDi algorithm
//
// -----------------------------------------------------------------------------------------------------


#define BIDI_DEBUG 0//2
#if (BIDI_DEBUG >= 1)
#include <iostream>
using namespace std;

static const char *directions[] = {
    "DirL", "DirR", "DirEN", "DirES", "DirET", "DirAN", "DirCS", "DirB", "DirS", "DirWS", "DirON",
    "DirLRE", "DirLRO", "DirAL", "DirRLE", "DirRLO", "DirPDF", "DirNSM", "DirBN"
};

#endif

struct QBidiStatus {
    QBidiStatus() {
        eor = QChar::DirON;
        lastStrong = QChar::DirON;
        last = QChar:: DirON;
        dir = QChar::DirON;
    }
    QChar::Direction eor;
    QChar::Direction lastStrong;
    QChar::Direction last;
    QChar::Direction dir;
};

// The Unicode standard says this should be 61, setting it to 29 would save quite some space here.
enum { MaxBidiLevel = 61 };

struct QBidiControl {
    inline QBidiControl(bool rtl)
        : cCtx(0), base(rtl), override(false), level(rtl) {}

    inline void embed(bool rtl, bool o = false) {
        uchar plus2 = 0;
        if((level%2 != 0) == rtl ) {
            level++;
            plus2 = 2;
        }
        level++;
        if (level <= MaxBidiLevel) {
            override = o;
            unsigned char control = (plus2 + (override ? 1 : 0)) << (cCtx % 4)*2;
            unsigned char mask = ~(0x3 << (cCtx % 4)*2);
            ctx[cCtx>>2] &= mask;
            ctx[cCtx>>2] |= control;
            cCtx++;
        }
    }
    inline bool canPop() const { return cCtx != 0; }
    inline void pdf() {
        Q_ASSERT(cCtx);
        (void) --cCtx;
        unsigned char control = (ctx[cCtx>>2] >> ((cCtx % 4)*2)) & 0x3;
        override = control & 0x1;
        level--;
        if (control & 0x2)
            level--;
    }

    inline QChar::Direction basicDirection() const {
        return (base ? QChar::DirR : QChar:: DirL);
    }
    inline uchar baseLevel() const {
        return base;
    }
    inline QChar::Direction direction() const {
        return ((level%2) ? QChar::DirR : QChar:: DirL);
    }

    unsigned char ctx[(MaxBidiLevel+3)/4];
    unsigned char cCtx : 6;
    unsigned char base : 1;
    unsigned char override : 1;
    unsigned char unused : 2;
    unsigned char level : 6;
};

static void qAppendItems(QTextEngine *engine, int &start, int &stop, QBidiControl &control, QChar::Direction dir)
{
    QScriptItemArray &items = engine->layoutData->items;
    const QChar *text = engine->layoutData->string.unicode();

    if (start > stop) {
        // #### the algorithm is currently not really safe against this. Still needs fixing.
//         qWarning("QTextEngine: BiDi internal error in qAppendItems()");
        return;
    }

    int level = control.level;

    if(dir != QChar::DirON && !control.override) {
        // add level of run (cases I1 & I2)
        if(level % 2) {
            if(dir == QChar::DirL || dir == QChar::DirAN || dir == QChar::DirEN)
                level++;
        } else {
            if(dir == QChar::DirR)
                level++;
            else if(dir == QChar::DirAN || dir == QChar::DirEN)
                level += 2;
        }
    }

#if (BIDI_DEBUG >= 1)
    qDebug("new run: dir=%s from %d, to %d level = %d\n", directions[dir], start, stop, level);
#endif
    int script = -1;
    QScriptItem item;
    item.position = start;
    item.analysis.script = script;
    item.analysis.bidiLevel = level;
    item.analysis.override = control.override;
    item.analysis.reserved = 0;

    for (int i = start; i <= stop; i++) {
        unsigned short uc = text[i].unicode();
        int s = QUnicodeTables::script(text[i]);
        if (uc == QChar::ObjectReplacementCharacter || uc == QChar::LineSeparator) {
            item.analysis.bidiLevel = level % 2 ? level-1 : level;
            item.analysis.script = QUnicodeTables::Common;
            item.isObject = true;
            s = -1;
        } else if (uc == 9) {
            item.analysis.script = QUnicodeTables::Common;
            item.isSpace = true;
            item.isTab = true;
            item.analysis.bidiLevel = control.baseLevel();
            s = -1;
        } else if (s != script && (s != QUnicodeTables::Inherited || script == -1)) {
            item.analysis.script = s == QUnicodeTables::Inherited ? QUnicodeTables::Common : s;
            item.analysis.bidiLevel = level;
        } else {
            if (i - start < 32000)
                continue;
            start = i;
        }

        item.position = i;
        items.append(item);
        script = s;
        item.isSpace = item.isTab = item.isObject = false;
    }
    ++stop;
    start = stop;
}

typedef void (* fAppendItems)(QTextEngine *, int &start, int &stop, QBidiControl &control, QChar::Direction dir);
static fAppendItems appendItems = qAppendItems;

// creates the next QScript items.
static bool bidiItemize(QTextEngine *engine, bool rightToLeft)
{
#if BIDI_DEBUG >= 2
    cout << "bidiItemize: rightToLeft=" << rightToLeft << endl;
#endif
    QBidiControl control(rightToLeft);

    bool hasBidi = rightToLeft;

    int sor = 0;
    int eor = -1;


    int length = engine->layoutData->string.length();

    if (!length)
        return hasBidi;

    const ushort *unicode = (const ushort *)engine->layoutData->string.unicode();
    int current = 0;

    QChar::Direction dir = rightToLeft ? QChar::DirR : QChar::DirL;
    QBidiStatus status;
    QChar::Direction sdir = QChar::direction(*unicode);
    if (sdir != QChar::DirL && sdir != QChar::DirR && sdir != QChar::DirEN && sdir != QChar::DirAN)
	sdir = QChar::DirON;
    else
        dir = QChar::DirON;
    status.eor = sdir;
    status.lastStrong = rightToLeft ? QChar::DirR : QChar::DirL;
    status.last = status.lastStrong;
    status.dir = sdir;



    while (current <= length) {

        QChar::Direction dirCurrent;
        if (current == (int)length)
            dirCurrent = control.basicDirection();
        else
            dirCurrent = QChar::direction(unicode[current]);

#if (BIDI_DEBUG >= 2)
        cout << "pos=" << current << " dir=" << directions[dir]
             << " current=" << directions[dirCurrent] << " last=" << directions[status.last]
             << " eor=" << eor << "/" << directions[status.eor]
             << " sor=" << sor << " lastStrong="
             << directions[status.lastStrong]
             << " level=" << (int)control.level << " override=" << (bool)control.override << endl;
#endif

        switch(dirCurrent) {

            // embedding and overrides (X1-X9 in the BiDi specs)
        case QChar::DirRLE:
        case QChar::DirRLO:
        case QChar::DirLRE:
        case QChar::DirLRO:
            {
                bool rtl = (dirCurrent == QChar::DirRLE || dirCurrent == QChar::DirRLO);
                hasBidi |= rtl;
                bool override = (dirCurrent == QChar::DirLRO || dirCurrent == QChar::DirRLO);

                uchar level = control.level+1;
                if ((level%2 != 0) == rtl) ++level;
                if(level < MaxBidiLevel) {
                    eor = current-1;
                    appendItems(engine, sor, eor, control, dir);
                    eor = current;
                    control.embed(rtl, override);
                    QChar::Direction edir = (rtl ? QChar::DirR : QChar::DirL);
                    dir = status.eor = edir;
                    status.lastStrong = edir;
                }
                break;
            }
        case QChar::DirPDF:
            {
                if (control.canPop()) {
                    if (dir != control.direction()) {
                        eor = current-1;
                        appendItems(engine, sor, eor, control, dir);
                        dir = control.direction();
                    }
                    eor = current;
                    appendItems(engine, sor, eor, control, dir);
                    control.pdf();
                    dir = QChar::DirON; status.eor = QChar::DirON;
                    status.last = control.direction();
                    if (control.override)
                        dir = control.direction();
                    else
                        dir = QChar::DirON;
                    status.lastStrong = control.direction();
                }
                break;
            }

            // strong types
        case QChar::DirL:
            if(dir == QChar::DirON)
                dir = QChar::DirL;
            switch(status.last)
                {
                case QChar::DirL:
                    eor = current; status.eor = QChar::DirL; break;
                case QChar::DirR:
                case QChar::DirAL:
                case QChar::DirEN:
                case QChar::DirAN:
                    if (eor >= 0) {
                        appendItems(engine, sor, eor, control, dir);
                        dir = eor < length ? QChar::direction(unicode[eor]) : control.basicDirection();
                        status.eor = dir;
                    } else {
                        eor = current; status.eor = dir;
                    }
                    break;
                case QChar::DirES:
                case QChar::DirET:
                case QChar::DirCS:
                case QChar::DirBN:
                case QChar::DirB:
                case QChar::DirS:
                case QChar::DirWS:
                case QChar::DirON:
                    if(dir != QChar::DirL) {
                        //last stuff takes embedding dir
                        if(control.direction() == QChar::DirR) {
                            if(status.eor != QChar::DirR) {
                                // AN or EN
                                appendItems(engine, sor, eor, control, dir);
                                status.eor = QChar::DirON;
                                dir = QChar::DirR;
                            }
                            eor = current - 1;
                            appendItems(engine, sor, eor, control, dir);
                            dir = eor < length ? QChar::direction(unicode[eor]) : control.basicDirection();
                            status.eor = dir;
                        } else {
                            if(status.eor != QChar::DirL) {
                                appendItems(engine, sor, eor, control, dir);
                                status.eor = QChar::DirON;
                                dir = QChar::DirL;
                            } else {
                                eor = current; status.eor = QChar::DirL; break;
                            }
                        }
                    } else {
                        eor = current; status.eor = QChar::DirL;
                    }
                default:
                    break;
                }
            status.lastStrong = QChar::DirL;
            break;
        case QChar::DirAL:
        case QChar::DirR:
            hasBidi = true;
            if(dir == QChar::DirON) dir = QChar::DirR;
            switch(status.last)
                {
                case QChar::DirL:
                case QChar::DirEN:
                case QChar::DirAN:
                    if (eor >= 0)
                        appendItems(engine, sor, eor, control, dir);
                    // fall through
                case QChar::DirR:
                case QChar::DirAL:
                    dir = QChar::DirR; eor = current; status.eor = QChar::DirR; break;
                case QChar::DirES:
                case QChar::DirET:
                case QChar::DirCS:
                case QChar::DirBN:
                case QChar::DirB:
                case QChar::DirS:
                case QChar::DirWS:
                case QChar::DirON:
                    if(status.eor != QChar::DirR && status.eor != QChar::DirAL) {
                        //last stuff takes embedding dir
                        if(control.direction() == QChar::DirR
                           || status.lastStrong == QChar::DirR || status.lastStrong == QChar::DirAL) {
                            appendItems(engine, sor, eor, control, dir);
                            dir = QChar::DirR; status.eor = QChar::DirON;
                            eor = current;
                        } else {
                            eor = current - 1;
                            appendItems(engine, sor, eor, control, dir);
                            dir = QChar::DirR; status.eor = QChar::DirON;
                        }
                    } else {
                        eor = current; status.eor = QChar::DirR;
                    }
                default:
                    break;
                }
            status.lastStrong = dirCurrent;
            break;

            // weak types:

        case QChar::DirNSM:
            if (eor == current-1)
                eor = current;
            break;
        case QChar::DirEN:
            // if last strong was AL change EN to AN
            if(status.lastStrong != QChar::DirAL) {
                if(dir == QChar::DirON) {
                    if(status.lastStrong == QChar::DirL)
                        dir = QChar::DirL;
                    else
                        dir = QChar::DirEN;
                }
                switch(status.last)
                    {
                    case QChar::DirET:
                        if (status.lastStrong == QChar::DirR || status.lastStrong == QChar::DirAL) {
                            appendItems(engine, sor, eor, control, dir);
                            status.eor = QChar::DirON;
                            dir = QChar::DirAN;
                        }
                        // fall through
                    case QChar::DirEN:
                    case QChar::DirL:
                        eor = current;
                        status.eor = dirCurrent;
                        break;
                    case QChar::DirR:
                    case QChar::DirAL:
                    case QChar::DirAN:
                        if (eor >= 0)
                            appendItems(engine, sor, eor, control, dir);
                        else
                            eor = current;
                        status.eor = QChar::DirEN;
                        dir = QChar::DirAN; break;
                    case QChar::DirES:
                    case QChar::DirCS:
                        if(status.eor == QChar::DirEN || dir == QChar::DirAN) {
                            eor = current; break;
                        }
                    case QChar::DirBN:
                    case QChar::DirB:
                    case QChar::DirS:
                    case QChar::DirWS:
                    case QChar::DirON:
                        if(status.eor == QChar::DirR) {
                            // neutrals go to R
                            eor = current - 1;
                            appendItems(engine, sor, eor, control, dir);
                            dir = QChar::DirON; status.eor = QChar::DirEN;
                            dir = QChar::DirAN;
                        }
                        else if(status.eor == QChar::DirL ||
                                 (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
                            eor = current; status.eor = dirCurrent;
                        } else {
                            // numbers on both sides, neutrals get right to left direction
                            if(dir != QChar::DirL) {
                                appendItems(engine, sor, eor, control, dir);
                                dir = QChar::DirON; status.eor = QChar::DirON;
                                eor = current - 1;
                                dir = QChar::DirR;
                                appendItems(engine, sor, eor, control, dir);
                                dir = QChar::DirON; status.eor = QChar::DirON;
                                dir = QChar::DirAN;
                            } else {
                                eor = current; status.eor = dirCurrent;
                            }
                        }
                    default:
                        break;
                    }
                break;
            }
        case QChar::DirAN:
            hasBidi = true;
            dirCurrent = QChar::DirAN;
            if(dir == QChar::DirON) dir = QChar::DirAN;
            switch(status.last)
                {
                case QChar::DirL:
                case QChar::DirAN:
                    eor = current; status.eor = QChar::DirAN; break;
                case QChar::DirR:
                case QChar::DirAL:
                case QChar::DirEN:
                    if (eor >= 0){
                        appendItems(engine, sor, eor, control, dir);
                    } else {
                        eor = current;
                    }
                    dir = QChar::DirON; status.eor = QChar::DirAN;
                    break;
                case QChar::DirCS:
                    if(status.eor == QChar::DirAN) {
                        eor = current; break;
                    }
                case QChar::DirES:
                case QChar::DirET:
                case QChar::DirBN:
                case QChar::DirB:
                case QChar::DirS:
                case QChar::DirWS:
                case QChar::DirON:
                    if(status.eor == QChar::DirR) {
                        // neutrals go to R
                        eor = current - 1;
                        appendItems(engine, sor, eor, control, dir);
                        status.eor = QChar::DirAN;
                        dir = QChar::DirAN;
                    } else if(status.eor == QChar::DirL ||
                               (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
                        eor = current; status.eor = dirCurrent;
                    } else {
                        // numbers on both sides, neutrals get right to left direction
                        if(dir != QChar::DirL) {
                            appendItems(engine, sor, eor, control, dir);
                            status.eor = QChar::DirON;
                            eor = current - 1;
                            dir = QChar::DirR;
                            appendItems(engine, sor, eor, control, dir);
                            status.eor = QChar::DirAN;
                            dir = QChar::DirAN;
                        } else {
                            eor = current; status.eor = dirCurrent;
                        }
                    }
                default:
                    break;
                }
            break;
        case QChar::DirES:
        case QChar::DirCS:
            break;
        case QChar::DirET:
            if(status.last == QChar::DirEN) {
                dirCurrent = QChar::DirEN;
                eor = current; status.eor = dirCurrent;
            }
            break;

            // boundary neutrals should be ignored
        case QChar::DirBN:
            break;
            // neutrals
        case QChar::DirB:
            // ### what do we do with newline and paragraph separators that come to here?
            break;
        case QChar::DirS:
            // ### implement rule L1
            break;
        case QChar::DirWS:
        case QChar::DirON:
            break;
        default:
            break;
        }

        //cout << "     after: dir=" << //        dir << " current=" << dirCurrent << " last=" << status.last << " eor=" << status.eor << " lastStrong=" << status.lastStrong << " embedding=" << control.direction() << endl;

        if(current >= (int)length) break;

        // set status.last as needed.
        switch(dirCurrent) {
        case QChar::DirET:
        case QChar::DirES:
        case QChar::DirCS:
        case QChar::DirS:
        case QChar::DirWS:
        case QChar::DirON:
            switch(status.last)
            {
            case QChar::DirL:
            case QChar::DirR:
            case QChar::DirAL:
            case QChar::DirEN:
            case QChar::DirAN:
                status.last = dirCurrent;
                break;
            default:
                status.last = QChar::DirON;
            }
            break;
        case QChar::DirNSM:
        case QChar::DirBN:
            // ignore these
            break;
        case QChar::DirLRO:
        case QChar::DirLRE:
            status.last = QChar::DirL;
            break;
        case QChar::DirRLO:
        case QChar::DirRLE:
            status.last = QChar::DirR;
            break;
        case QChar::DirEN:
            if (status.last == QChar::DirL) {
                status.last = QChar::DirL;
                break;
            }
            // fall through
        default:
            status.last = dirCurrent;
        }

        ++current;
    }

#if (BIDI_DEBUG >= 1)
    cout << "reached end of line current=" << current << ", eor=" << eor << endl;
#endif
    eor = current - 1; // remove dummy char

    if (sor <= eor)
        appendItems(engine, sor, eor, control, dir);

    return hasBidi;
}

void QTextEngine::bidiReorder(int numItems, const quint8 *levels, int *visualOrder)
{

    // first find highest and lowest levels
    uchar levelLow = 128;
    uchar levelHigh = 0;
    int i = 0;
    while (i < numItems) {
        //printf("level = %d\n", r->level);
        if (levels[i] > levelHigh)
            levelHigh = levels[i];
        if (levels[i] < levelLow)
            levelLow = levels[i];
        i++;
    }

    // implements reordering of the line (L2 according to BiDi spec):
    // L2. From the highest level found in the text to the lowest odd level on each line,
    // reverse any contiguous sequence of characters that are at that level or higher.

    // reversing is only done up to the lowest odd level
    if(!(levelLow%2)) levelLow++;

#if (BIDI_DEBUG >= 1)
    cout << "reorderLine: lineLow = " << (uint)levelLow << ", lineHigh = " << (uint)levelHigh << endl;
#endif

    int count = numItems - 1;
    for (i = 0; i < numItems; i++)
        visualOrder[i] = i;

    while(levelHigh >= levelLow) {
        int i = 0;
        while (i < count) {
            while(i < count && levels[i] < levelHigh) i++;
            int start = i;
            while(i <= count && levels[i] >= levelHigh) i++;
            int end = i-1;

            if(start != end) {
                //cout << "reversing from " << start << " to " << end << endl;
                for(int j = 0; j < (end-start+1)/2; j++) {
                    int tmp = visualOrder[start+j];
                    visualOrder[start+j] = visualOrder[end-j];
                    visualOrder[end-j] = tmp;
                }
            }
            i++;
        }
        levelHigh--;
    }

#if (BIDI_DEBUG >= 1)
    cout << "visual order is:" << endl;
    for (i = 0; i < numItems; i++)
        cout << visualOrder[i] << endl;
#endif
}

#if defined(Q_WS_X11) || defined (Q_WS_QWS)

#include "qfontengine_ft_p.h"

void QTextEngine::shapeText(int item) const
{
    shapeTextWithHarfbuzz(item);
}

#elif defined(Q_WS_WIN)
# include "qtextengine_win.cpp"
#elif defined(Q_WS_MAC)
# include "qtextengine_mac.cpp"
#endif

#include <private/qharfbuzz_p.h>

static HB_Error hb_getSFntTable(void *font, HB_Tag tableTag, HB_Byte *buffer, HB_UInt *length)
{
    QFontEngine *fe = (QFontEngine *)font;
    if (!fe->getSfntTableData(tableTag, buffer, length))
        return HB_Err_Invalid_Argument;
    return HB_Err_Ok;
}

static HB_Bool hb_stringToGlyphs(HB_Font font, const HB_UChar16 *string, hb_uint32 length, HB_Glyph *glyphs, hb_uint32 *numGlyphs, HB_Bool rightToLeft)
{
    QFontEngine *fe = (QFontEngine *)font->userData;

    QVarLengthArray<QGlyphLayout> qglyphs(*numGlyphs);

    QTextEngine::ShaperFlags shaperFlags(QTextEngine::GlyphIndicesOnly);
    if (rightToLeft)
        shaperFlags |= QTextEngine::RightToLeft;

    int nGlyphs = *numGlyphs;
    bool result = fe->stringToCMap(reinterpret_cast<const QChar *>(string), length, qglyphs.data(), &nGlyphs, shaperFlags);
    *numGlyphs = nGlyphs;
    if (!result)
        return false;

    for (hb_uint32 i = 0; i < *numGlyphs; ++i)
        glyphs[i] = qglyphs[i].glyph;

    return true;
}

static void hb_getAdvances(HB_Font font, const HB_Glyph *glyphs, hb_uint32 numGlyphs, HB_Fixed *advances, int flags)
{
    QFontEngine *fe = (QFontEngine *)font->userData;

    QVarLengthArray<QGlyphLayout> qglyphs(numGlyphs);
    for (hb_uint32 i = 0; i < numGlyphs; ++i)
        qglyphs[i].glyph = glyphs[i];

    fe->recalcAdvances(numGlyphs, qglyphs.data(), flags & HB_ShaperFlag_UseDesignMetrics ? QTextEngine::DesignMetrics : QFlag(0));

    for (hb_uint32 i = 0; i < numGlyphs; ++i)
        advances[i] = qglyphs[i].advance.x.value();
}

static HB_Bool hb_canRender(HB_Font font, const HB_UChar16 *string, hb_uint32 length)
{
    QFontEngine *fe = (QFontEngine *)font->userData;
    return fe->canRender(reinterpret_cast<const QChar *>(string), length);
}

static void hb_getGlyphMetrics(HB_Font font, HB_Glyph glyph, HB_GlyphMetrics *metrics)
{
    QFontEngine *fe = (QFontEngine *)font->userData;
    glyph_metrics_t m = fe->boundingBox(glyph);
    metrics->x = m.x.value();
    metrics->y = m.y.value();
    metrics->width = m.width.value();
    metrics->height = m.height.value();
    metrics->xOffset = m.xoff.value();
    metrics->yOffset = m.yoff.value();
}

static HB_Fixed hb_getAscent(HB_Font font)
{
    QFontEngine *fe = (QFontEngine *)font->userData;
    return fe->ascent().value();
}

const HB_FontClass hb_fontClass = {
    hb_stringToGlyphs, hb_getAdvances, hb_canRender, /*getPointInOutline*/0,
    hb_getGlyphMetrics, hb_getAscent
};

static bool stringToGlyphs(HB_ShaperItem *item, HB_Glyph *itemGlyphs, QFontEngine *fontEngine)
{
    int nGlyphs = item->num_glyphs;
    QVarLengthArray<QGlyphLayout> qglyphs(nGlyphs);

    QTextEngine::ShaperFlags shaperFlags(QTextEngine::GlyphIndicesOnly);
    if (item->item.bidiLevel % 2)
        shaperFlags |= QTextEngine::RightToLeft;

    bool result = fontEngine->stringToCMap(reinterpret_cast<const QChar *>(item->string + item->item.pos),
                                           item->item.length,
                                           qglyphs.data(), &nGlyphs,
                                           shaperFlags);
    item->num_glyphs = nGlyphs;
    if (!result)
        return false;

    for (hb_uint32 i = 0; i < item->num_glyphs; ++i)
        itemGlyphs[i] = qglyphs[i].glyph;

    return true;
}

void QTextEngine::shapeTextWithHarfbuzz(int item) const
{
    Q_ASSERT(item < layoutData->items.size());
    QScriptItem &si = layoutData->items[item];

    if (si.num_glyphs)
        return;

    si.glyph_data_offset = layoutData->used;

    QFontEngine *font = fontEngine(si, &si.ascent, &si.descent);

    bool kerningEnabled = this->font(si).d->kerning;

    HB_ShaperItem entire_shaper_item;
    entire_shaper_item.kerning_applied = false;
    entire_shaper_item.string = reinterpret_cast<const HB_UChar16 *>(layoutData->string.constData());
    entire_shaper_item.stringLength = layoutData->string.length();
    entire_shaper_item.item.script = (HB_Script)si.analysis.script;
    entire_shaper_item.item.pos = si.position;
    entire_shaper_item.item.length = length(item);
    entire_shaper_item.item.bidiLevel = si.analysis.bidiLevel;
    entire_shaper_item.glyphIndicesPresent = false;

    entire_shaper_item.shaperFlags = 0;
    if (!kerningEnabled)
        entire_shaper_item.shaperFlags |= HB_ShaperFlag_NoKerning;
    if (option.useDesignMetrics())
        entire_shaper_item.shaperFlags |= HB_ShaperFlag_UseDesignMetrics;

    entire_shaper_item.num_glyphs = qMax(layoutData->num_glyphs - layoutData->used, int(entire_shaper_item.item.length));

    QVarLengthArray<HB_Glyph> hb_initial_glyphs(entire_shaper_item.num_glyphs);

    if (!stringToGlyphs(&entire_shaper_item, hb_initial_glyphs.data(), font)) {
        hb_initial_glyphs.resize(entire_shaper_item.num_glyphs);
        if (!stringToGlyphs(&entire_shaper_item, hb_initial_glyphs.data(), font)) {
            // ############ if this happens there's a bug in the fontengine
            return;
        }
    }

    QVarLengthArray<int> itemBoundaries(2);
    // k * 2 entries, array[k] == index in string, array[k + 1] == index in glyphs
    itemBoundaries[0] = entire_shaper_item.item.pos;
    itemBoundaries[1] = 0;

    if (font->type() == QFontEngine::Multi) {
        uint lastEngine = 0;
        int charIdx = entire_shaper_item.item.pos;
        const int stringEnd = charIdx + entire_shaper_item.item.length;
        for (quint32 i = 0; i < entire_shaper_item.num_glyphs; ++i, ++charIdx) {
            uint engineIdx = hb_initial_glyphs[i] >> 24;
            if (engineIdx != lastEngine && i > 0) {
                itemBoundaries.append(charIdx);
                itemBoundaries.append(i);
            }
            lastEngine = engineIdx;
            if (HB_IsHighSurrogate(entire_shaper_item.string[charIdx])
                && charIdx < stringEnd - 1
                && HB_IsLowSurrogate(entire_shaper_item.string[charIdx + 1]))
                ++charIdx;
        }
    }

    int initial_glyph_pos = 0;
    int glyph_pos = 0;
    for (int k = 0; k < itemBoundaries.size(); k += 2) {

        HB_ShaperItem shaper_item = entire_shaper_item;

        shaper_item.item.pos = itemBoundaries[k];
        if (k < itemBoundaries.size() - 3) {
            shaper_item.item.length = itemBoundaries[k + 2] - shaper_item.item.pos;
            shaper_item.num_glyphs = itemBoundaries[k + 3] - itemBoundaries[k + 1];
        } else {
            shaper_item.item.length -= shaper_item.item.pos - entire_shaper_item.item.pos;
            shaper_item.num_glyphs -= itemBoundaries[k + 1];
        }
        shaper_item.initialGlyphCount = shaper_item.num_glyphs;

        QVarLengthArray<HB_Glyph> hb_glyphs(shaper_item.num_glyphs);
        QVarLengthArray<HB_GlyphAttributes> hb_attributes(shaper_item.num_glyphs);
        QVarLengthArray<HB_Fixed> hb_advances(shaper_item.num_glyphs);
        QVarLengthArray<HB_FixedPoint> hb_offsets(shaper_item.num_glyphs);

        QFontEngine *actualFontEngine = font;
#if defined(Q_WS_X11) || defined(Q_WS_QWS)
        QFontEngineFT *ftEngine = 0;
#endif
        uint engineIdx = 0;
        if (font->type() == QFontEngine::Multi) {
            engineIdx = uint(hb_initial_glyphs[itemBoundaries[k + 1]] >> 24);

            actualFontEngine = static_cast<QFontEngineMulti *>(font)->engine(engineIdx);
        }

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
        if (actualFontEngine->type() == QFontEngine::Freetype) {
            ftEngine = static_cast<QFontEngineFT *>(actualFontEngine);
        }
#endif

        HB_FontClass fontClass = hb_fontClass;
        HB_FontRec hbFont;
        memset(&hbFont, 0, sizeof(hbFont));
        hbFont.klass = &fontClass;
        hbFont.userData = actualFontEngine;

        shaper_item.font = &hbFont;

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
        if (ftEngine) {
            Q_ASSERT(ftEngine);
            ftEngine->lockFace();

            ftEngine->setupHarfbuzzFont(&hbFont, &fontClass);

            shaper_item.face = ftEngine->harfbuzzFace();
        } else
#endif
       	{
            shaper_item.face = qHBNewFace(actualFontEngine, hb_getSFntTable);
        }

        {
            QFixed emSquare = actualFontEngine->properties().emSquare;
            hbFont.x_ppem = actualFontEngine->fontDef.pixelSize;
            hbFont.y_ppem = actualFontEngine->fontDef.pixelSize * actualFontEngine->fontDef.stretch / 100;
            hbFont.x_scale = (QFixed(hbFont.x_ppem * (1 << 16)) / emSquare).value();
            hbFont.y_scale = (QFixed(hbFont.y_ppem * (1 << 16)) / emSquare).value();
        }

        shaper_item.glyphIndicesPresent = true;

        while (1) {
            ensureSpace(glyph_pos + shaper_item.num_glyphs);
            shaper_item.num_glyphs = layoutData->num_glyphs - layoutData->used - glyph_pos;

            hb_glyphs.resize(shaper_item.num_glyphs);
            hb_attributes.resize(shaper_item.num_glyphs);
            hb_advances.resize(shaper_item.num_glyphs);
            hb_offsets.resize(shaper_item.num_glyphs);

            memset(hb_glyphs.data(), 0, hb_glyphs.size() * sizeof(HB_Glyph));
            memset(hb_attributes.data(), 0, hb_attributes.size() * sizeof(HB_GlyphAttributes));
            memset(hb_advances.data(), 0, hb_advances.size() * sizeof(HB_Fixed));
            memset(hb_offsets.data(), 0, hb_offsets.size() * sizeof(HB_FixedPoint));

            if (shaper_item.glyphIndicesPresent) {
                for (hb_uint32 i = 0; i < shaper_item.initialGlyphCount; ++i)
                    hb_glyphs[i] = hb_initial_glyphs[initial_glyph_pos + i] & 0x00ffffff;
            }

            shaper_item.glyphs = hb_glyphs.data();
            shaper_item.attributes = hb_attributes.data();
            shaper_item.advances = hb_advances.data();
            shaper_item.offsets = hb_offsets.data();
            shaper_item.log_clusters = logClusters(&si) + shaper_item.item.pos - entire_shaper_item.item.pos;

//          qDebug("    .. num_glyphs=%d, used=%d, item.num_glyphs=%d", num_glyphs, used, shaper_item.num_glyphs);

            if (!qShapeItem(&shaper_item))
                continue;

            QGlyphLayout *g = glyphs(&si) + glyph_pos;

            for (hb_uint32 i = 0; i < shaper_item.num_glyphs; ++i) {
                g[i].glyph = shaper_item.glyphs[i] | (engineIdx << 24);
                g[i].advance.x = QFixed::fromFixed(shaper_item.advances[i]);
                g[i].advance.y = QFixed();
                g[i].offset.x = QFixed::fromFixed(shaper_item.offsets[i].x);
                g[i].offset.y = QFixed::fromFixed(shaper_item.offsets[i].y);

                g[i].attributes.justification = shaper_item.attributes[i].justification;
                g[i].attributes.clusterStart = shaper_item.attributes[i].clusterStart;
                g[i].attributes.mark = shaper_item.attributes[i].mark;
                g[i].attributes.zeroWidth = shaper_item.attributes[i].zeroWidth;
                g[i].attributes.dontPrint = shaper_item.attributes[i].dontPrint;
                g[i].attributes.combiningClass = shaper_item.attributes[i].combiningClass;
            }

            for (hb_uint32 i = 0; i < shaper_item.item.length; ++i)
                shaper_item.log_clusters[i] += glyph_pos;

            if (kerningEnabled && !shaper_item.kerning_applied)
                font->doKerning(shaper_item.num_glyphs, g, option.useDesignMetrics() ? QFlag(QTextEngine::DesignMetrics) : QFlag(0));

            glyph_pos += shaper_item.num_glyphs;
#if defined(Q_WS_X11) || defined(Q_WS_QWS)
            if (ftEngine)
                ftEngine->unlockFace();
            else
#endif
                qHBFreeFace(shaper_item.face);
            break;
        }

        initial_glyph_pos += shaper_item.initialGlyphCount;
    }

//     qDebug("    -> item: script=%d num_glyphs=%d", shaper_item.script, shaper_item.num_glyphs);
    si.num_glyphs = glyph_pos;

    layoutData->used += si.num_glyphs;

    QGlyphLayout *g = glyphs(&si);

    si.width = 0;
    QGlyphLayout *end = g + si.num_glyphs;
    while (g < end)
        si.width += (g++)->advance.x;

    return;
}

static void init(QTextEngine *e)
{
#ifdef Q_WS_WIN
    if(!resolvedUsp10)
        resolveUsp10();
#endif
    e->ignoreBidi = false;
    e->cacheGlyphs = false;
    e->forceJustification = false;

    e->layoutData = 0;

    e->minWidth = 0;
    e->maxWidth = 0;

    e->underlinePositions = 0;
    e->specialData = 0;
    e->stackEngine = false;
}

QTextEngine::QTextEngine()
{
    init(this);
}

QTextEngine::QTextEngine(const QString &str, const QFont &f)
    : fnt(f)
{
    init(this);
    text = str;
}

QTextEngine::~QTextEngine()
{
    if (!stackEngine)
        delete layoutData;
    delete specialData;
}

const HB_CharAttributes *QTextEngine::attributes() const
{
    if (layoutData && layoutData->haveCharAttributes)
        return (HB_CharAttributes *) layoutData->memory;

    itemize();
    ensureSpace(layoutData->string.length());

    QVarLengthArray<HB_ScriptItem> hbScriptItems(layoutData->items.size());

    for (int i = 0; i < layoutData->items.size(); ++i) {
        const QScriptItem &si = layoutData->items[i];
        hbScriptItems[i].pos = si.position;
        hbScriptItems[i].length = length(i);
        hbScriptItems[i].bidiLevel = si.analysis.bidiLevel;
        hbScriptItems[i].script = (HB_Script)si.analysis.script;
#ifdef Q_WS_WIN
        if(hasUsp10) {
            hbScriptItems[i].script = (HB_Script)QUnicodeTables::script(layoutData->string.at(si.position));
        }
#endif
    }

    qGetCharAttributes(reinterpret_cast<const HB_UChar16 *>(layoutData->string.constData()),
                       layoutData->string.length(),
                       hbScriptItems.data(), hbScriptItems.size(),
                       (HB_CharAttributes *)layoutData->memory);


    layoutData->haveCharAttributes = true;
    return (HB_CharAttributes *) layoutData->memory;
}

void QTextEngine::shape(int item) const
{
    if (layoutData->items[item].isObject) {
        ensureSpace(1);
        if (block.docHandle()) {
            QTextFormat format = formats()->format(formatIndex(&layoutData->items[item]));
            docLayout()->resizeInlineObject(QTextInlineObject(item, const_cast<QTextEngine *>(this)),
                                            layoutData->items[item].position + block.position(), format);
        }
    } else {
        shapeText(item);
    }
}

void QTextEngine::invalidate()
{
    freeMemory();
    lines.clear();
    minWidth = 0;
    maxWidth = 0;
    if (specialData)
        specialData->resolvedFormatIndices.clear();
}

void QTextEngine::validate() const
{
    if (layoutData)
        return;
    layoutData = new LayoutData();
    if (block.docHandle())
        layoutData->string = block.text();
    else
        layoutData->string = text;
    if (specialData && specialData->preeditPosition != -1)
        layoutData->string.insert(specialData->preeditPosition, specialData->preeditText);
}


void QTextEngine::itemize() const
{
    validate();
    if (layoutData->items.size())
        return;

    if (layoutData->string.length() == 0)
        return;

    bool ignore = ignoreBidi;
    if (!ignore && option.textDirection() == Qt::LeftToRight) {
        ignore = true;
        const QChar *start = layoutData->string.unicode();
        const QChar * const end = start + layoutData->string.length();
        while (start < end) {
            if (start->unicode() >= 0x590) {
                ignore = false;
                break;
            }
            ++start;
        }
    }
    
    if (!ignore) {
        layoutData->hasBidi = bidiItemize(const_cast<QTextEngine *>(this), (option.textDirection() == Qt::RightToLeft));
    } else {
        QBidiControl control(false);
        int start = 0;
        int stop = layoutData->string.length() - 1;
        appendItems(const_cast<QTextEngine *>(this), start, stop, control, QChar::DirL);
        layoutData->hasBidi = false;
    }

    addRequiredBoundaries();
    resolveAdditionalFormats();
}

int QTextEngine::findItem(int strPos) const
{
    itemize();

    // ##### use binary search
    int item;
    for (item = layoutData->items.size()-1; item > 0; --item) {
        if (layoutData->items[item].position <= strPos)
            break;
    }
    return item;
}

QFixed QTextEngine::width(int from, int len) const
{
    itemize();

    QFixed w = 0;

//     qDebug("QTextEngine::width(from = %d, len = %d), numItems=%d, strleng=%d", from,  len, items.size(), string.length());
    for (int i = 0; i < layoutData->items.size(); i++) {
        const QScriptItem *si = layoutData->items.constData() + i;
        int pos = si->position;
        int ilen = length(i);
//          qDebug("item %d: from %d len %d", i, pos, ilen);
        if (pos >= from + len)
            break;
        if (pos + ilen > from) {
            if (!si->num_glyphs)
                shape(i);

            if (si->isObject) {
                w += si->width;
                continue;
            } else if (si->isTab) {
                w = nextTab(si, w);
                continue;
            }


            QGlyphLayout *glyphs = this->glyphs(si);
            unsigned short *logClusters = this->logClusters(si);

//             fprintf(stderr, "  logclusters:");
//             for (int k = 0; k < ilen; k++)
//                 fprintf(stderr, " %d", logClusters[k]);
//             fprintf(stderr, "\n");
            // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
            int charFrom = from - pos;
            if (charFrom < 0)
                charFrom = 0;
            int glyphStart = logClusters[charFrom];
            if (charFrom > 0 && logClusters[charFrom-1] == glyphStart)
                while (charFrom < ilen && logClusters[charFrom] == glyphStart)
                    charFrom++;
            if (charFrom < ilen) {
                glyphStart = logClusters[charFrom];
                int charEnd = from + len - 1 - pos;
                if (charEnd >= ilen)
                    charEnd = ilen-1;
                int glyphEnd = logClusters[charEnd];
                while (charEnd < ilen && logClusters[charEnd] == glyphEnd)
                    charEnd++;
                glyphEnd = (charEnd == ilen) ? si->num_glyphs : logClusters[charEnd];

//                 qDebug("char: start=%d end=%d / glyph: start = %d, end = %d", charFrom, charEnd, glyphStart, glyphEnd);
                for (int i = glyphStart; i < glyphEnd; i++)
                    w += glyphs[i].advance.x * !glyphs[i].attributes.dontPrint;
            }
        }
    }
//     qDebug("   --> w= %d ", w);
    return w;
}

glyph_metrics_t QTextEngine::boundingBox(int from,  int len) const
{
    itemize();

    glyph_metrics_t gm;

    for (int i = 0; i < layoutData->items.size(); i++) {
        const QScriptItem *si = layoutData->items.constData() + i;
        int pos = si->position;
        int ilen = length(i);
        if (pos > from + len)
            break;
        if (pos + len > from) {
            if (!si->num_glyphs)
                shape(i);
            unsigned short *logClusters = this->logClusters(si);
            QGlyphLayout *glyphs = this->glyphs(si);

            // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
            int charFrom = from - pos;
            if (charFrom < 0)
                charFrom = 0;
            int glyphStart = logClusters[charFrom];
            if (charFrom > 0 && logClusters[charFrom-1] == glyphStart)
                while (charFrom < ilen && logClusters[charFrom] == glyphStart)
                    charFrom++;
            if (charFrom < ilen) {
                glyphStart = logClusters[charFrom];
                int charEnd = from + len - 1 - pos;
                if (charEnd >= ilen)
                    charEnd = ilen-1;
                int glyphEnd = logClusters[charEnd];
                while (charEnd < ilen && logClusters[charEnd] == glyphEnd)
                    charEnd++;
                glyphEnd = (charEnd == ilen) ? si->num_glyphs : logClusters[charEnd];
                if (glyphStart <= glyphEnd ) {
                    QFontEngine *fe = fontEngine(*si);
                    glyph_metrics_t m = fe->boundingBox(glyphs+glyphStart, glyphEnd-glyphStart);
                    gm.x = qMin(gm.x, m.x + gm.xoff);
                    gm.y = qMin(gm.y, m.y + gm.yoff);
                    gm.width = qMax(gm.width, m.width+gm.xoff);
                    gm.height = qMax(gm.height, m.height+gm.yoff);
                    gm.xoff += m.xoff;
                    gm.yoff += m.yoff;
                }
            }
        }
    }
    return gm;
}

glyph_metrics_t QTextEngine::tightBoundingBox(int from,  int len) const
{
    itemize();

    glyph_metrics_t gm;

    for (int i = 0; i < layoutData->items.size(); i++) {
        const QScriptItem *si = layoutData->items.constData() + i;
        int pos = si->position;
        int ilen = length(i);
        if (pos > from + len)
            break;
        if (pos + len > from) {
            if (!si->num_glyphs)
                shape(i);
            unsigned short *logClusters = this->logClusters(si);
            QGlyphLayout *glyphs = this->glyphs(si);

            // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
            int charFrom = from - pos;
            if (charFrom < 0)
                charFrom = 0;
            int glyphStart = logClusters[charFrom];
            if (charFrom > 0 && logClusters[charFrom-1] == glyphStart)
                while (charFrom < ilen && logClusters[charFrom] == glyphStart)
                    charFrom++;
            if (charFrom < ilen) {
                glyphStart = logClusters[charFrom];
                int charEnd = from + len - 1 - pos;
                if (charEnd >= ilen)
                    charEnd = ilen-1;
                int glyphEnd = logClusters[charEnd];
                while (charEnd < ilen && logClusters[charEnd] == glyphEnd)
                    charEnd++;
                glyphEnd = (charEnd == ilen) ? si->num_glyphs : logClusters[charEnd];
                if (glyphStart <= glyphEnd ) {
                    QFontEngine *fe = fontEngine(*si);
                    glyph_metrics_t m = fe->tightBoundingBox(glyphs+glyphStart, glyphEnd-glyphStart);
                    gm.x = qMin(gm.x, m.x + gm.xoff);
                    gm.y = qMin(gm.y, m.y + gm.yoff);
                    gm.width = qMax(gm.width, m.width+gm.xoff);
                    gm.height = qMax(gm.height, m.height+gm.yoff);
                    gm.xoff += m.xoff;
                    gm.yoff += m.yoff;
                }
            }
        }
    }
    return gm;
}

QFont QTextEngine::font(const QScriptItem &si) const
{
    if (!hasFormats())
        return fnt;
    QTextCharFormat f = format(&si);
    QFont font = f.font();

    if (block.docHandle() && block.docHandle()->layout()) {
        // Make sure we get the right dpi on printers
        QPaintDevice *pdev = block.docHandle()->layout()->paintDevice();
        if (pdev)
            font = QFont(font, pdev);
    } else {
        font = font.resolve(fnt);
    }

    QTextCharFormat::VerticalAlignment valign = f.verticalAlignment();
    if (valign == QTextCharFormat::AlignSuperScript || valign == QTextCharFormat::AlignSubScript) {
        if (font.pointSize() != -1)
            font.setPointSize((font.pointSize() * 2) / 3);
        else
            font.setPixelSize((font.pixelSize() * 2) / 3);
    }

    return font;
}

QFontEngine *QTextEngine::fontEngine(const QScriptItem &si, QFixed *ascent, QFixed *descent) const
{
    QFontEngine *engine;
    QFontEngine *scaledEngine = 0;
    int script = si.analysis.script;
#if defined(Q_WS_WIN)
    if (hasUsp10) {
        const SCRIPT_PROPERTIES *script_prop = script_properties[si.analysis.script];
        script = scriptForWinLanguage(script_prop->langid);
    }
#endif

    if (!hasFormats()) {
        engine = fnt.d->engineForScript(script);
#if defined(Q_WS_WIN)
        if (engine->type() == QFontEngine::Box)
            engine = fnt.d->engineForScript(QUnicodeTables::Common);
#endif
    } else {
        QTextCharFormat f = format(&si);
        QFont font = f.font();

        if (block.docHandle() && block.docHandle()->layout()) {
            // Make sure we get the right dpi on printers
            QPaintDevice *pdev = block.docHandle()->layout()->paintDevice();
            if (pdev)
                font = QFont(font, pdev);
        } else {
            font = font.resolve(fnt);
        }
        engine = font.d->engineForScript(script);
#if defined(Q_WS_WIN)
        if (engine->type() == QFontEngine::Box)
            engine = font.d->engineForScript(QUnicodeTables::Common);
#endif
        QTextCharFormat::VerticalAlignment valign = f.verticalAlignment();
        if (valign == QTextCharFormat::AlignSuperScript || valign == QTextCharFormat::AlignSubScript) {
            if (font.pointSize() != -1)
                font.setPointSize((font.pointSize() * 2) / 3);
            else
                font.setPixelSize((font.pixelSize() * 2) / 3);
            scaledEngine = font.d->engineForScript(script);
#if defined(Q_WS_WIN)
            if (scaledEngine->type() == QFontEngine::Box)
                scaledEngine = font.d->engineForScript(QUnicodeTables::Common);
#endif
        }
    }

    if (ascent) {
        *ascent = engine->ascent();
        *descent = engine->descent();
    }

    if (scaledEngine)
        return scaledEngine;
    return engine;
}

struct QJustificationPoint {
    int type;
    QFixed kashidaWidth;
    QGlyphLayout *glyph;
    QFontEngine *fontEngine;
};

Q_DECLARE_TYPEINFO(QJustificationPoint, Q_PRIMITIVE_TYPE);

static void set(QJustificationPoint *point, int type, QGlyphLayout *glyph, QFontEngine *fe)
{
    point->type = type;
    point->glyph = glyph;
    point->fontEngine = fe;

    if (type >= HB_Arabic_Normal) {
        QChar ch(0x640); // Kashida character
        QGlyphLayout glyphs[8];
        int nglyphs = 7;
        fe->stringToCMap(&ch, 1, glyphs, &nglyphs, 0);
        if (glyphs[0].glyph && glyphs[0].advance.x != 0) {
            point->kashidaWidth = glyphs[0].advance.x;
        } else {
            point->type = HB_NoJustification;
            point->kashidaWidth = 0;
        }
    }
}


void QTextEngine::justify(const QScriptLine &line)
{
//     qDebug("justify: line.gridfitted = %d, line.justified=%d", line.gridfitted, line.justified);
    if (line.gridfitted && line.justified)
        return;

    if (!line.gridfitted) {
        // redo layout in device metrics, then adjust
        const_cast<QScriptLine &>(line).gridfitted = true;
    }

    if ((option.alignment() & Qt::AlignHorizontal_Mask) != Qt::AlignJustify)
        return;

    itemize();

    if (!forceJustification && (line.from + (int)line.length == layoutData->string.length()
        || layoutData->string.at(line.from + line.length - 1) == QChar::LineSeparator))
        return;

    // justify line
    int maxJustify = 0;

    // don't include trailing white spaces when doing justification
    int line_length = line.length;
    const HB_CharAttributes *a = attributes()+line.from;
    while (line_length && a[line_length-1].whiteSpace)
        --line_length;
    // subtract one char more, as we can't justfy after the last character
    --line_length;

    if (!line_length)
        return;

    int firstItem = findItem(line.from);
    int nItems = findItem(line.from + line_length - 1) - firstItem + 1;

    QVarLengthArray<QJustificationPoint> justificationPoints;
    int nPoints = 0;
//     qDebug("justifying from %d len %d, firstItem=%d, nItems=%d", line.from, line_length, firstItem, nItems);
    QFixed minKashida = 0x100000;

    // we need to do all shaping before we go into the next loop, as we there
    // store pointers to the glyph data that could get reallocated by the shaping
    // process.
    for (int i = 0; i < nItems; ++i) {
        QScriptItem &si = layoutData->items[firstItem + i];
        if (!si.num_glyphs)
            shape(firstItem + i);
    }

    for (int i = 0; i < nItems; ++i) {
        QScriptItem &si = layoutData->items[firstItem + i];

        int kashida_type = HB_Arabic_Normal;
        int kashida_pos = -1;

        int start = qMax(line.from - si.position, 0);
        int end = qMin(line.from + line_length - (int)si.position, length(firstItem+i));

        unsigned short *log_clusters = logClusters(&si);

        int gs = log_clusters[start];
        int ge = (end == length(firstItem+i) ? si.num_glyphs : log_clusters[end]);

        QGlyphLayout *g = glyphs(&si);

        for (int i = gs; i < ge; ++i) {
            g[i].justification.type = QGlyphJustification::JustifyNone;
            g[i].justification.nKashidas = 0;
            g[i].justification.space_18d6 = 0;

            justificationPoints.resize(nPoints+3);
            int justification = g[i].attributes.justification;

            switch(justification) {
            case HB_NoJustification:
                break;
            case HB_Space          :
                // fall through
            case HB_Arabic_Space   :
                if (kashida_pos >= 0) {
//                     qDebug("kashida position at %d in word", kashida_pos);
                    set(&justificationPoints[nPoints], kashida_type, g+kashida_pos, fontEngine(si));
                    minKashida = qMin(minKashida, justificationPoints[nPoints].kashidaWidth);
                    maxJustify = qMax(maxJustify, justificationPoints[nPoints].type);
                    ++nPoints;
                }
                kashida_pos = -1;
                kashida_type = HB_Arabic_Normal;
                // fall through
            case HB_Character      :
                set(&justificationPoints[nPoints++], justification, g+i, fontEngine(si));
                maxJustify = qMax(maxJustify, justification);
                break;
            case HB_Arabic_Normal  :
            case HB_Arabic_Waw     :
            case HB_Arabic_BaRa    :
            case HB_Arabic_Alef    :
            case HB_Arabic_HaaDal  :
            case HB_Arabic_Seen    :
            case HB_Arabic_Kashida :
                if (justification >= kashida_type) {
                    kashida_pos = i;
                    kashida_type = justification;
                }
            }
        }
        if (kashida_pos >= 0) {
            set(&justificationPoints[nPoints], kashida_type, g+kashida_pos, fontEngine(si));
            minKashida = qMin(minKashida, justificationPoints[nPoints].kashidaWidth);
            maxJustify = qMax(maxJustify, justificationPoints[nPoints].type);
            ++nPoints;
        }
    }

    QFixed need = line.width - line.textWidth;
    if (need < 0) {
        // line overflows already!
        const_cast<QScriptLine &>(line).justified = true;
        return;
    }

//     qDebug("doing justification: textWidth=%x, requested=%x, maxJustify=%d", line.textWidth.value(), line.width.value(), maxJustify);
//     qDebug("     minKashida=%f, need=%f", minKashida, need);

    // distribute in priority order
    if (maxJustify >= HB_Arabic_Normal) {
        while (need >= minKashida) {
            for (int type = maxJustify; need >= minKashida && type >= HB_Arabic_Normal; --type) {
                for (int i = 0; need >= minKashida && i < nPoints; ++i) {
                    if (justificationPoints[i].type == type && justificationPoints[i].kashidaWidth <= need) {
                        justificationPoints[i].glyph->justification.nKashidas++;
                        // ############
                        justificationPoints[i].glyph->justification.space_18d6 += justificationPoints[i].kashidaWidth.value();
                        need -= justificationPoints[i].kashidaWidth;
//                         qDebug("adding kashida type %d with width %x, neednow %x", type, justificationPoints[i].kashidaWidth, need.value());
                    }
                }
            }
        }
    }
    Q_ASSERT(need >= 0);
    if (!need)
        goto end;

    maxJustify = qMin(maxJustify, (int)HB_Space);
    for (int type = maxJustify; need != 0 && type > 0; --type) {
        int n = 0;
        for (int i = 0; i < nPoints; ++i) {
            if (justificationPoints[i].type == type)
                ++n;
        }
//          qDebug("number of points for justification type %d: %d", type, n);


        if (!n)
            continue;

        for (int i = 0; i < nPoints; ++i) {
            if (justificationPoints[i].type == type) {
                QFixed add = need/n;
//                  qDebug("adding %x to glyph %x", add.value(), justificationPoints[i].glyph->glyph);
                justificationPoints[i].glyph->justification.space_18d6 = add.value();
                need -= add;
                --n;
            }
        }

        Q_ASSERT(!need);
    }
 end:
    const_cast<QScriptLine &>(line).justified = true;
}

void QScriptLine::setDefaultHeight(QTextEngine *eng)
{
    QFont f;
    QFontEngine *e;

    if (eng->block.docHandle() && eng->block.docHandle()->layout()) {
        f = eng->block.charFormat().font();
        // Make sure we get the right dpi on printers
        QPaintDevice *pdev = eng->block.docHandle()->layout()->paintDevice();
        if (pdev)
            f = QFont(f, pdev);
        e = f.d->engineForScript(QUnicodeTables::Common);
    } else {
        e = eng->fnt.d->engineForScript(QUnicodeTables::Common);
    }

    ascent = qMax(ascent, e->ascent());
    descent = qMax(descent, e->descent());
}

QTextEngine::LayoutData::LayoutData()
{
    memory = 0;
    allocated = 0;
    num_glyphs = 0;
    memory_on_stack = false;
    used = 0;
    hasBidi = false;
    inLayout = false;
    haveCharAttributes = false;
    logClustersPtr = 0;
    glyphPtr = 0;
}

QTextEngine::LayoutData::LayoutData(const QString &str, void **stack_memory, int _allocated)
    : string(str)
{
    allocated = _allocated;

    int space_charAttributes = sizeof(HB_CharAttributes)*string.length()/sizeof(void*) + 1;
    int space_logClusters = sizeof(unsigned short)*string.length()/sizeof(void*) + 1;
    available_glyphs = ((int)allocated - space_charAttributes - space_logClusters)*(int)sizeof(void*)/(int)sizeof(QGlyphLayout);

    if (available_glyphs < str.length()) {
        // need to allocate on the heap
        num_glyphs = 0;
        allocated = 0;

        memory_on_stack = false;
        memory = 0;
        logClustersPtr = 0;
        glyphPtr = 0;
    } else {
        num_glyphs = str.length();

        memory_on_stack = true;
        memory = stack_memory;
        logClustersPtr = (unsigned short *)(memory + space_charAttributes);
        glyphPtr = (QGlyphLayout *)(memory + space_charAttributes + space_logClusters);
        memset(memory, 0, space_charAttributes*sizeof(void *));
        memset(glyphPtr, 0, num_glyphs*sizeof(QGlyphLayout));
    }
    used = 0;
    hasBidi = false;
    inLayout = false;
    haveCharAttributes = false;
}

QTextEngine::LayoutData::~LayoutData()
{
    if (!memory_on_stack)
        free(memory);
    memory = 0;
}

void QTextEngine::LayoutData::reallocate(int totalGlyphs)
{
    Q_ASSERT(totalGlyphs >= num_glyphs);
    if (memory_on_stack && available_glyphs >= totalGlyphs) {
        memset(glyphPtr + num_glyphs, 0, (totalGlyphs - num_glyphs)*sizeof(QGlyphLayout));
        num_glyphs = totalGlyphs;
        return;
    }

    int space_charAttributes = sizeof(HB_CharAttributes)*string.length()/sizeof(void*) + 1;
    int space_logClusters = sizeof(unsigned short)*string.length()/sizeof(void*) + 1;
    int space_glyphs = sizeof(QGlyphLayout)*totalGlyphs/sizeof(void*) + 2;

    int newAllocated = space_charAttributes + space_glyphs + space_logClusters;
    Q_ASSERT(newAllocated >= allocated);
    void **old_mem = memory;
    memory = (void **)::realloc(memory_on_stack ? 0 : old_mem, newAllocated*sizeof(void *));
    if (memory_on_stack && memory)
        memcpy(memory, old_mem, allocated*sizeof(void *));
    memory_on_stack = false;

    void **m = memory;
    m += space_charAttributes;
    logClustersPtr = (unsigned short *) m;
    m += space_logClusters;

    glyphPtr = (QGlyphLayout *) m;

    memset(((char *)memory) + allocated*sizeof(void *), 0,
           (newAllocated - allocated)*sizeof(void *));

    allocated = newAllocated;
    num_glyphs = totalGlyphs;
}

void QTextEngine::freeMemory()
{
    if (!stackEngine) {
        delete layoutData;
        layoutData = 0;
    } else {
        layoutData->used = 0;
        layoutData->hasBidi = false;
        layoutData->inLayout = false;
        layoutData->haveCharAttributes = false;
    }
    for (int i = 0; i < lines.size(); ++i) {
        lines[i].justified = 0;
        lines[i].gridfitted = 0;
    }
}

int QTextEngine::formatIndex(const QScriptItem *si) const
{
    if (specialData && !specialData->resolvedFormatIndices.isEmpty())
        return specialData->resolvedFormatIndices.at(si - &layoutData->items[0]);
    QTextDocumentPrivate *p = block.docHandle();
    if (!p)
        return -1;
    int pos = si->position;
    if (specialData && si->position >= specialData->preeditPosition) {
        if (si->position < specialData->preeditPosition + specialData->preeditText.length())
            pos = qMax(specialData->preeditPosition - 1, 0);
        else
            pos -= specialData->preeditText.length();
    }
    QTextDocumentPrivate::FragmentIterator it = p->find(block.position() + pos);
    return it.value()->format;
}


QTextCharFormat QTextEngine::format(const QScriptItem *si) const
{
    QTextCharFormat format;
    const QTextFormatCollection *formats = 0;
    if (block.docHandle()) {
        formats = this->formats();
        format = formats->charFormat(formatIndex(si));
    }
    if (specialData && specialData->resolvedFormatIndices.isEmpty()) {
        int end = si->position + length(si);
        for (int i = 0; i < specialData->addFormats.size(); ++i) {
            const QTextLayout::FormatRange &r = specialData->addFormats.at(i);
            if (r.start <= si->position && r.start + r.length >= end) {
                if (!specialData->addFormatIndices.isEmpty())
                    format.merge(formats->format(specialData->addFormatIndices.at(i)));
                else
                    format.merge(r.format);
            }
        }
    }
    return format;
}

void QTextEngine::addRequiredBoundaries() const
{
    int position = 0;
    SpecialData *s = specialData;

    const QTextDocumentPrivate *p = block.docHandle();
    if (p) {
        QTextDocumentPrivate::FragmentIterator it = p->find(block.position());
        QTextDocumentPrivate::FragmentIterator end = p->find(block.position() + block.length() - 1); // -1 to omit the block separator char
        int format = it.value()->format;

        for (; it != end; ++it) {
            if (s && position >= s->preeditPosition) {
                position += s->preeditText.length();
                s = 0;
            }
            const QTextFragmentData * const frag = it.value();
            if (format != frag->format)
                setBoundary(position);
            format = frag->format;
            position += frag->size;
        }
    }
    if (specialData) {
        for (int i = 0; i < specialData->addFormats.size(); ++i) {
            const QTextLayout::FormatRange &r = specialData->addFormats.at(i);
            setBoundary(r.start);
            setBoundary(r.start + r.length);
            //qDebug("adding boundaries %d %d", r.start, r.start+r.length);
        }
    }
}

bool QTextEngine::atWordSeparator(int position) const
{
    const QChar c = layoutData->string.at(position);
    return c == QLatin1Char('.')
        || c == QLatin1Char(',')
        || c == QLatin1Char('?')
        || c == QLatin1Char('!')
        || c == QLatin1Char(':')
        || c == QLatin1Char(';')
        || c == QLatin1Char('-')
        || c == QLatin1Char('<')
        || c == QLatin1Char('>')
        || c == QLatin1Char('[')
        || c == QLatin1Char(']')
        || c == QLatin1Char('(')
        || c == QLatin1Char(')')
        || c == QLatin1Char('{')
        || c == QLatin1Char('}')
        || c == QLatin1Char('=')
        || c == QLatin1Char('\t')
        || c == QChar::Nbsp
        ;
}

void QTextEngine::indexAdditionalFormats()
{
    if (!block.docHandle())
        return;

    specialData->addFormatIndices.resize(specialData->addFormats.count());
    QTextFormatCollection * const formats = this->formats();

    for (int i = 0; i < specialData->addFormats.count(); ++i) {
        specialData->addFormatIndices[i] = formats->indexForFormat(specialData->addFormats.at(i).format);
        specialData->addFormats[i].format = QTextCharFormat();
    }
}

QString QTextEngine::elidedText(Qt::TextElideMode mode, const QFixed &width, int flags) const
{
//    qDebug() << "elidedText; available width" << width.toReal() << "text width:" << this->width(0, layoutData->string.length()).toReal();

    if (flags & Qt::TextShowMnemonic) {
        itemize();
        for (int i = 0; i < layoutData->items.size(); ++i) {
            QScriptItem &si = layoutData->items[i];
            if (!si.num_glyphs)
                shape(i);

            unsigned short *logClusters = this->logClusters(&si);
            QGlyphLayout *glyphs = this->glyphs(&si);

            const int end = si.position + length(&si);
            for (int i = si.position; i < end - 1; ++i)
                if (layoutData->string.at(i) == QLatin1Char('&')) {
                    const int gp = logClusters[i - si.position];
                    glyphs[gp].attributes.dontPrint = true;
                    HB_CharAttributes *attributes = const_cast<HB_CharAttributes *>(this->attributes());
                    attributes[i + 1].charStop = false;
                    attributes[i + 1].whiteSpace = false;
                    attributes[i + 1].lineBreakType = HB_NoBreak;
                    if (i < end - 1
                            && layoutData->string.at(i + 1) == QLatin1Char('&'))
                        ++i;
                }
        }
    }

    validate();

    if (mode == Qt::ElideNone
        || this->width(0, layoutData->string.length()) <= width
        || layoutData->string.length() <= 1)
        return layoutData->string;

    QFixed ellipsisWidth;
    QString ellipsisText;
    {
        QChar ellipsisChar(0x2026);

        QFontEngine *fe = fnt.d->engineForScript(QUnicodeTables::Common);
        // the lookup can be really slow when we use XLFD fonts
        bool isXlfdEngine = (fe->type() == QFontEngine::XLFD)
                            || (fe->type() == QFontEngine::Multi
                                && static_cast<QFontEngineMulti*>(fe)->engine(0)->type() == QFontEngine::XLFD);
        if (!isXlfdEngine && fe->canRender(&ellipsisChar, 1)) {
            QGlyphLayout glyph;
            int nGlyphs = 1;
            fe->stringToCMap(&ellipsisChar, 1, &glyph, &nGlyphs, 0);
            ellipsisWidth = glyph.advance.x;
            ellipsisText = ellipsisChar;
        } else {
            QString dotDotDot(QLatin1String("..."));

            QGlyphLayout glyphs[3];
            int nGlyphs = 3;
            if (!fe->stringToCMap(dotDotDot.constData(), 3, glyphs, &nGlyphs, 0))
                // should never happen...
                return layoutData->string;
            for (int i = 0; i < nGlyphs; ++i)
                ellipsisWidth += glyphs[i].advance.x;
            ellipsisText = dotDotDot;
        }
    }

    const QFixed availableWidth = width - ellipsisWidth;

    const HB_CharAttributes *attributes = this->attributes();

    if (mode == Qt::ElideRight) {
        QFixed currentWidth;
        int pos = 0;
        int nextBreak = 0;

        do {
            pos = nextBreak;

            ++nextBreak;
            while (nextBreak < layoutData->string.length() && !attributes[nextBreak].charStop)
                ++nextBreak;

            currentWidth += this->width(pos, nextBreak - pos);
        } while (nextBreak < layoutData->string.length()
                 && currentWidth < availableWidth);

        return layoutData->string.left(pos) + ellipsisText;
    } else if (mode == Qt::ElideLeft) {
        QFixed currentWidth;
        int pos = layoutData->string.length();
        int nextBreak = layoutData->string.length();

        do {
            pos = nextBreak;

            --nextBreak;
            while (nextBreak > 0 && !attributes[nextBreak].charStop)
                --nextBreak;

            currentWidth += this->width(nextBreak, pos - nextBreak);
        } while (nextBreak > 0
                 && currentWidth < availableWidth);

        return ellipsisText + layoutData->string.mid(pos);
    } else if (mode == Qt::ElideMiddle) {
        QFixed leftWidth;
        QFixed rightWidth;

        int leftPos = 0;
        int nextLeftBreak = 0;

        int rightPos = layoutData->string.length();
        int nextRightBreak = layoutData->string.length();

        do {
            leftPos = nextLeftBreak;
            rightPos = nextRightBreak;

            ++nextLeftBreak;
            while (nextLeftBreak < layoutData->string.length() && !attributes[nextLeftBreak].charStop)
                ++nextLeftBreak;

            --nextRightBreak;
            while (nextRightBreak > 0 && !attributes[nextRightBreak].charStop)
                --nextRightBreak;

            leftWidth += this->width(leftPos, nextLeftBreak - leftPos);
            rightWidth += this->width(nextRightBreak, rightPos - nextRightBreak);
        } while (nextLeftBreak < layoutData->string.length()
                 && nextRightBreak > 0
                 && leftWidth + rightWidth < availableWidth);

        return layoutData->string.left(leftPos) + ellipsisText + layoutData->string.mid(rightPos);
    }

    return layoutData->string;
}

void QTextEngine::setBoundary(int strPos) const
{
    if (strPos <= 0 || strPos >= layoutData->string.length())
        return;

    int itemToSplit = 0;
    while (itemToSplit < layoutData->items.size() && layoutData->items[itemToSplit].position <= strPos)
        itemToSplit++;
    itemToSplit--;
    if (layoutData->items[itemToSplit].position == strPos) {
        // already a split at the requested position
        return;
    }
    splitItem(itemToSplit, strPos - layoutData->items[itemToSplit].position);
}

void QTextEngine::splitItem(int item, int pos) const
{
    if (pos <= 0)
        return;

    layoutData->items.insert(item + 1, QScriptItem(layoutData->items[item]));
    QScriptItem &oldItem = layoutData->items[item];
    QScriptItem &newItem = layoutData->items[item+1];
    newItem.position += pos;

    if (oldItem.num_glyphs) {
        // already shaped, break glyphs aswell
        int breakGlyph = logClusters(&oldItem)[pos];

        newItem.num_glyphs = oldItem.num_glyphs - breakGlyph;
        oldItem.num_glyphs = breakGlyph;
        newItem.glyph_data_offset = oldItem.glyph_data_offset + breakGlyph;

        for (int i = 0; i < newItem.num_glyphs; i++)
            logClusters(&newItem)[i] -= breakGlyph;

        QFixed w = 0;
        const QGlyphLayout *g = glyphs(&oldItem);
        for(int j = 0; j < breakGlyph; ++j)
            w += (g++)->advance.x;

        newItem.width = oldItem.width - w;
        oldItem.width = w;
    }

//     qDebug("split at position %d itempos=%d", pos, item);
}

QFixed QTextEngine::nextTab(const QScriptItem *si, QFixed x) const
{
    // #### should work for alignright and righttoleft
    if (!(option.alignment() & Qt::AlignLeft) ||
        option.textDirection() != Qt::LeftToRight)
        return x + si->width;

    QList<qreal> tabArray = option.tabArray();
    if (!tabArray.isEmpty()) {
        for (int i = 0; i < tabArray.size(); ++i) {
            if (tabArray.at(i) > x.toReal())
                return QFixed::fromReal(tabArray.at(i));
        }
    }
    QFixed tab = QFixed::fromReal(option.tabStop());
    if (tab <= 0)
        tab = 80; // default
    return ((x / tab).truncate() + 1) * tab;
}

void QTextEngine::resolveAdditionalFormats() const
{
    if (!specialData || specialData->addFormats.isEmpty()
        || !block.docHandle()
        || !specialData->resolvedFormatIndices.isEmpty())
        return;

    QTextFormatCollection *collection = this->formats();

    specialData->resolvedFormatIndices.clear();
    QVector<int> indices(layoutData->items.count());
    for (int i = 0; i < layoutData->items.count(); ++i) {
        QTextCharFormat f = format(&layoutData->items.at(i));
        indices[i] = collection->indexForFormat(f);
    }
    specialData->resolvedFormatIndices = indices;
}

QStackTextEngine::QStackTextEngine(const QString &string, const QFont &f)
    : _layoutData(string, _memory, MemSize)
{
    fnt = f;
    text = string;
    stackEngine = true;
    layoutData = &_layoutData;
}

QTextItemInt::QTextItemInt(const QScriptItem &si, QFont *font, const QTextCharFormat &format)
    : justified(false), underlineStyle(QTextCharFormat::NoUnderline), num_chars(0), chars(0),
      logClusters(0), f(0), glyphs(0), num_glyphs(0), fontEngine(0)
{
    // explicitly initialize flags so that initFontAttributes can be called
    // multiple times on the same TextItem
    flags = 0;
    if (si.analysis.bidiLevel %2)
        flags |= QTextItem::RightToLeft;
    ascent = si.ascent;
    descent = si.descent;
    f = font;
    fontEngine = f->d->engineForScript(si.analysis.script);
    Q_ASSERT(fontEngine);

    underlineColor = format.underlineColor();
    underlineStyle = QTextCharFormat::NoUnderline;

    if (format.hasProperty(QTextFormat::TextUnderlineStyle)) {
        underlineStyle = format.underlineStyle();
    } else if (format.boolProperty(QTextFormat::FontUnderline)
               || f->d->underline) {
        underlineStyle = QTextCharFormat::SingleUnderline;
    }

    // compat
    if (underlineStyle == QTextCharFormat::SingleUnderline)
        flags |= QTextItem::Underline;

    if (f->d->overline || format.fontOverline())
        flags |= QTextItem::Overline;
    if (f->d->strikeOut || format.fontStrikeOut())
        flags |= QTextItem::StrikeOut;
}
