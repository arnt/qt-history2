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

#include "qtextformat.h"
#include "qtextformat_p.h"
#include "qtextengine_p.h"
#include "qabstracttextdocumentlayout.h"
#include "qtextlayout.h"
#include "qvarlengtharray.h"
#include "qscriptengine_p.h"
#include "qfont.h"
#include "qfontdata_p.h"
#include "qfontengine_p.h"
#include "qstring.h"
#include <private/qunicodetables_p.h>
#include "qtextdocument_p.h"
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

struct BidiStatus {
    BidiStatus() {
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

struct BidiControl {
    inline BidiControl(bool rtl)
        : cCtx(0), base(rtl), singleLine(false), override(false), level(rtl) {}

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
    unsigned char singleLine : 1;
    unsigned char override : 1;
    unsigned char unused : 1;
    unsigned char level : 6;
};

static QChar::Direction basicDirection(const QString &str)
{
    int len = str.length();
    int pos = 0;
    const QChar *uc = str.unicode() + pos;
    while(pos < len) {
        switch(direction(*uc))
        {
        case QChar::DirL:
        case QChar::DirLRO:
        case QChar::DirLRE:
            return QChar::DirL;
        case QChar::DirR:
        case QChar::DirAL:
        case QChar::DirRLO:
        case QChar::DirRLE:
            return QChar::DirR;
        default:
            break;
        }
        ++pos;
        ++uc;
    }
    return QChar::DirL;
}


static void qAppendItems(QTextEngine *engine, int &start, int &stop, BidiControl &control, QChar::Direction dir)
{
    QScriptItemArray &items = engine->items;
    const QChar *text = engine->string.unicode();

    if (start > stop) {
        // #### the algorithm is currently not really safe against this. Still needs fixing.
//         qWarning("Bidi: appendItems() internal error");
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
    QFont::Script script = QFont::NoScript;
    QScriptItem item;
    item.position = start;
    item.analysis.script = script;
    item.analysis.bidiLevel = level;
    item.analysis.override = control.override;
    item.analysis.reserved = 0;

    if (control.singleLine) {
        for (int i = start; i <= stop; i++) {

            unsigned short uc = text[i].unicode();
            QFont::Script s = (QFont::Script)qt_scriptForChar(uc);
            if (s == QFont::UnknownScript || s == QFont::CombiningMarks)
                s = script;

            if (s != script) {
                item.analysis.script = s;
                item.analysis.bidiLevel = level;
                item.position = i;
                items.append(item);
                script = s;
            }
        }
    } else {
        for (int i = start; i <= stop; i++) {

            unsigned short uc = text[i].unicode();
            QFont::Script s = (QFont::Script)qt_scriptForChar(uc);
            if (s == QFont::UnknownScript || s == QFont::CombiningMarks)
                s = script;

            QChar::Category category = ::category(uc);
            if (uc == QChar::ObjectReplacementCharacter || uc == QChar::LineSeparator) {
                item.analysis.bidiLevel = level % 2 ? level-1 : level;
                item.analysis.script = QFont::Latin;
                item.isObject = true;
                s = QFont::NoScript;
#if 0
            } else if ((uc >= 9 && uc <=13) ||
                       (category >= QChar::Separator_Space && category <= QChar::Separator_Paragraph)) {
                item.analysis.script = QFont::Latin;
                item.isSpace = true;
                item.isTab = (uc == '\t');
                item.analysis.bidiLevel = item.isTab ? control.baseLevel() : level;
                s = QFont::NoScript;
#else
            } else if (uc == 9) {
                item.analysis.script = QFont::Latin;
                item.isSpace = true;
                item.isTab = true;
                item.analysis.bidiLevel = control.baseLevel();
                s = QFont::NoScript;
#endif
            } else if (s != script && (category != QChar::Mark_NonSpacing || script == QFont::NoScript)) {
                item.analysis.script = s;
                item.analysis.bidiLevel = level;
            } else {
                continue;
            }

            item.position = i;
            items.append(item);
            script = s;
            item.isSpace = item.isTab = item.isObject = false;
        }
    }
    ++stop;
    start = stop;
}

typedef void (* fAppendItems)(QTextEngine *, int &start, int &stop, BidiControl &control, QChar::Direction dir);
static fAppendItems appendItems = qAppendItems;

// creates the next QScript items.
static void bidiItemize(QTextEngine *engine, bool rightToLeft, int mode)
{
    BidiControl control(rightToLeft);
    if (mode & QTextLayout::SingleLine)
        control.singleLine = true;

    int sor = 0;
    int eor = -1;

    // ### should get rid of this!
    bool first = true;

    int length = engine->string.length();

    if (!length)
        return;

    const QChar *unicode = engine->string.unicode();
    int current = 0;

    QChar::Direction dir = QChar::DirON;
    BidiStatus status;
    QChar::Direction sdir = direction(*unicode);
    if (sdir != QChar::DirL && sdir != QChar::DirR && sdir != QChar::DirEN && sdir != QChar::DirAN)
        sdir = QChar::DirON;
    status.eor = sdir;
    status.lastStrong = rightToLeft ? QChar::DirR : QChar::DirL;
    status.last = status.lastStrong;
    status.dir = sdir;



    while (current <= length) {

        QChar::Direction dirCurrent;
        if (current == (int)length)
            dirCurrent = control.basicDirection();
        else
            dirCurrent = direction(unicode[current]);

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
                    if (!first) {
                        appendItems(engine, sor, eor, control, dir);
                        dir = eor < length ? direction(unicode[eor]) : control.basicDirection();
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
                            dir = eor < length ? direction(unicode[eor]) : control.basicDirection();
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
            if(dir == QChar::DirON) dir = QChar::DirR;
            switch(status.last)
                {
                case QChar::DirL:
                case QChar::DirEN:
                case QChar::DirAN:
                    if (!first) {
                        appendItems(engine, sor, eor, control, dir);
                        dir = QChar::DirON; status.eor = QChar::DirON;
                        break;
                    }
                case QChar::DirR:
                case QChar::DirAL:
                    eor = current; status.eor = QChar::DirR; break;
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
                            dir = QChar::DirON; status.eor = QChar::DirON;
                            dir = QChar::DirR;
                            eor = current;
                        } else {
                            eor = current - 1;
                            appendItems(engine, sor, eor, control, dir);
                            dir = QChar::DirON; status.eor = QChar::DirON;
                            dir = QChar::DirR;
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
                        if (!first)
                            appendItems(engine, sor, eor, control, dir);
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
                    if (!first)
                        appendItems(engine, sor, eor, control, dir);
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

        first = false;
        ++current;
    }

#if (BIDI_DEBUG >= 1)
    cout << "reached end of line current=" << current << ", eor=" << eor << endl;
#endif
    eor = current - 1; // remove dummy char

    if (sor <= eor)
        appendItems(engine, sor, eor, control, dir);


}

void QTextEngine::bidiReorder(int numItems, const Q_UINT8 *levels, int *visualOrder)
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


// -----------------------------------------------------------------------------------------------------
//
// The line break algorithm. See http://www.unicode.org/reports/tr14/tr14-13.html
//
// -----------------------------------------------------------------------------------------------------

/* The Unicode algorithm does in our opinion allow line breaks at some
   places they shouldn't be allowed. The following changes were thus
   made in comparison to the Unicode reference:

   CL->AL from Dbk to Ibk
   CL->PR from Dbk to Ibk
   EX->AL from Dbk to Ibk
   IS->AL from Dbk to Ibk
   PO->AL from Dbk to Ibk
   SY->AL from Dbk to Ibk
   SY->PO from Dbk to Ibk
   SY->PR from Dbk to Ibk
   SY->OP from Dbk to Ibk
   Al->OP from Dbk to Ibk
   AL->HY from Dbk to Ibk
   AL->PR from Dbk to Ibk
   AL->PO from Dbk to Ibk
   PR->PR from Dbk to Ibk
   PO->PO from Dbk to Ibk
   PR->PO from Dbk to Ibk
   PO->PR from Dbk to Ibk
   HY->PO from Dbk to Ibk
   HY->PR from Dbk to Ibk
   HY->OP from Dbk to Ibk
   PO->OP from Dbk to Ibk
   NU->EX from Dbk to Ibk
   NU->PR from Dbk to Ibk
   PO->NU from Dbk to Ibk
   EX->PO from Dbk to Ibk
*/

enum break_action {
    Dbk, // Direct break
    Ibk, // Indirect break; only allowed if space between the two chars
    Pbk // Prohibited break; no break allowed even if space between chars
};

// The following line break classes are not treated by the table:
// SA, BK, CR, LF, SG, CB, SP
static const Q_UINT8 breakTable[QUnicodeTables::LineBreak_CM+1][QUnicodeTables::LineBreak_CM+1] =
{
    // OP,  CL,  QU,  GL, NS,  EX,  SY,  IS,  PR,  PO,  NU,  AL,  ID,  IN,  HY,  BA,  BB,  B2,  ZW,  CM
    { Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk, Pbk }, // OP
    { Dbk, Pbk, Ibk, Pbk, Pbk, Pbk, Pbk, Pbk, Ibk, Ibk, Dbk, Ibk, Dbk, Dbk, Ibk, Ibk, Pbk, Pbk, Pbk, Pbk }, // CL
    { Pbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Pbk, Pbk }, // QU
    { Ibk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Pbk, Pbk }, // GL
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // NS
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Ibk, Ibk, Ibk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // EX
    { Ibk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // SY
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // IS
    { Ibk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Ibk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Pbk }, // PR
    { Ibk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // PO
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Dbk, Ibk, Ibk, Ibk, Dbk, Dbk, Pbk, Pbk }, // NU
    { Ibk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Dbk, Ibk, Ibk, Ibk, Dbk, Dbk, Pbk, Pbk }, // AL
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Ibk, Dbk, Dbk, Dbk, Ibk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // ID
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Dbk, Dbk, Dbk, Dbk, Ibk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // IN
    { Ibk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // HY
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Ibk, Ibk, Dbk, Dbk, Pbk, Ibk }, // BA
    { Ibk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Ibk, Pbk, Ibk }, // BB
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Ibk, Ibk, Dbk, Pbk, Pbk, Ibk }, // B2
    { Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Dbk, Pbk, Ibk }, // ZW
    { Dbk, Pbk, Ibk, Pbk, Ibk, Pbk, Pbk, Pbk, Dbk, Ibk, Dbk, Dbk, Dbk, Ibk, Ibk, Ibk, Dbk, Dbk, Pbk, Pbk }  // CM
};

// set the soft break flag at every possible line breaking point. This needs correct clustering information.
static void calcLineBreaks(const QString &str, QCharAttributes *charAttributes)
{
    int len = str.length();
    if (!len)
        return;

    const QChar *uc = str.unicode();
    int cls = lineBreakClass(*uc);
    if (cls >= QUnicodeTables::LineBreak_CM)
        cls = QUnicodeTables::LineBreak_ID;

    charAttributes[0].softBreak = false;

    for (int i = 1; i < len; ++i) {
        int ncls = lineBreakClass(uc[i]);

        if (ncls == QUnicodeTables::LineBreak_SP || ncls == QUnicodeTables::LineBreak_CM) {
            charAttributes[i].softBreak = false;
            continue;
        }
	if (cls == QUnicodeTables::LineBreak_SA && ncls == QUnicodeTables::LineBreak_SA)
            // two complex chars (thai or lao), thai_attributes has already set this correctly
            continue;
        int tcls = ncls;
        if (tcls >= QUnicodeTables::LineBreak_SA)
            tcls = QUnicodeTables::LineBreak_ID;

	int brk = charAttributes[i].charStop ? breakTable[cls][tcls] : (int)Pbk;
        if (brk == Ibk)
            charAttributes[i].softBreak = (lineBreakClass(uc[i-1]) == QUnicodeTables::LineBreak_SP);
        else
            charAttributes[i].softBreak = (brk == Dbk);
//        qDebug("char = %c %04x, cls=%d, ncls=%d, brk=%d soft=%d", uc[i].cell(), uc[i].unicode(), cls, ncls, brk, charAttributes[i].softBreak);
        cls = ncls;
    }
}

#if defined(Q_WS_X11) || defined (Q_WS_QWS)
# include "qtextengine_unix.cpp"
#elif defined(Q_WS_WIN)
# include "qtextengine_win.cpp"
#elif defined(Q_WS_MAC)
# include "qtextengine_mac.cpp"
#endif

static void init(QTextEngine *e)
{
    e->formats = 0;

    e->direction = QChar::DirON;
    e->haveCharAttributes = false;
    e->widthOnly = false;
    e->designMetrics = false;
    e->textColorFromPalette = false;
    e->itemization_mode = 0;
    e->textFlags = 0;

    e->pal = 0;
    e->docLayout = 0;

    e->allocated = 0;
    e->memory = 0;
    e->num_glyphs = 0;
    e->used = 0;
    e->minWidth = 0.;
    e->maxWidth = 0.;

    e->cursorPos = -1;
    e->underlinePositions = 0;
    e->invalid = true;
}

QTextEngine::QTextEngine()
    : fnt(0)
{
    init(this);
}

QTextEngine::QTextEngine(const QString &str, QFontPrivate *f)
    : fnt(f)
{
    init(this);
    setText(str);
    if (fnt)
        ++fnt->ref;
}


void QTextEngine::setText(const QString &str)
{
    pal = 0;
    textFlags = 0;
    invalidate();
    string = str;
}

QTextEngine::~QTextEngine()
{
    if (fnt && !--fnt->ref)
        delete fnt;
    delete pal;
    free(memory);
    allocated = 0;
}

void QTextEngine::reallocate(int totalGlyphs)
{
    int space_charAttributes = sizeof(QCharAttributes)*string.length()/sizeof(void*) + 1;
    int space_logClusters = sizeof(unsigned short)*string.length()/sizeof(void*) + 1;
    int space_glyphs = sizeof(QGlyphLayout)*totalGlyphs/sizeof(void*) + 1;

    int newAllocated = space_charAttributes + space_glyphs + space_logClusters;
    memory = (void **)::realloc(memory, newAllocated*sizeof(void *));

    void **m = memory;
    m += space_charAttributes;
    logClustersPtr = (unsigned short *) m;
    m += space_logClusters;
    glyphPtr = (QGlyphLayout *) m;

    memset(((char *)memory) + allocated*sizeof(void *), 0, (newAllocated-allocated)*sizeof(void *));

    allocated = newAllocated;
    num_glyphs = totalGlyphs;
}


const QCharAttributes *QTextEngine::attributes()
{
    if (haveCharAttributes)
        return (QCharAttributes *) memory;

    if (items.size() == 0)
        itemize();
    ensureSpace(string.length());

    for (int i = 0; i < items.size(); i++) {
        QScriptItem &si = items[i];
        int from = si.position;
        int len = length(i);
        int script = si.analysis.script;
        Q_ASSERT(script < QFont::NScripts);
        qt_scriptEngines[si.analysis.script].charAttributes(script, string, from, len, (QCharAttributes *) memory);
    }

    calcLineBreaks(string, (QCharAttributes *) memory);
    haveCharAttributes = true;
    return (QCharAttributes *) memory;
}

void QTextEngine::setFormat(int from, int length, int format)
{
    if (from >= string.length())
        return;

    setBoundary(from+length);
    setBoundary(from);

    int item = findItem(from);
    while (item < items.size() && items[item].position < from+length) {
        QScriptItem &si = items[item];
        si.format = format;
        ++item;
    }
}

void QTextEngine::setBoundary(int strPos)
{
    if (strPos <= 0 || strPos >= string.length())
        return;

    int itemToSplit = 0;
    while (itemToSplit < items.size() && items[itemToSplit].position <= strPos)
        itemToSplit++;
    itemToSplit--;
    if (items[itemToSplit].position == strPos) {
        // already a split at the requested position
        return;
    }
    splitItem(itemToSplit, strPos - items[itemToSplit].position);
}

void QTextEngine::shape(int item) const
{
    if (items[item].isObject) {
        if (docLayout && formats) {
            QTextFormat format = formats->format(items[item].format);
            // ##### const cast
            docLayout->setSize(QTextInlineObject(item, const_cast<QTextEngine *>(this)), format);
        }
    } else {
        shapeText(item);
    }
}

void QTextEngine::splitItem(int item, int pos)
{
    if (pos <= 0)
        return;

    items.insert(item+1, QScriptItem(items[item]));
    QScriptItem &oldItem = items[item];
    QScriptItem &newItem = items[item+1];
    newItem.position += pos;

    if (oldItem.num_glyphs) {
        // already shaped, break glyphs aswell
        int breakGlyph = logClusters(&oldItem)[pos];

        newItem.num_glyphs = oldItem.num_glyphs - breakGlyph;
        oldItem.num_glyphs = breakGlyph;
        newItem.glyph_data_offset = oldItem.glyph_data_offset + breakGlyph;

        for (int i = 0; i < newItem.num_glyphs; i++)
            logClusters(&newItem)[i] -= breakGlyph;

        float w = 0;
        const QGlyphLayout *g = glyphs(&oldItem);
        for(int j = 0; j < breakGlyph; ++j)
            w += (g++)->advance.x();

        newItem.width = oldItem.width - w;
        oldItem.width = w;
    }

//     qDebug("split at position %d itempos=%d", pos, item);
}

void QTextEngine::itemize()
{
    validate();
    items.clear();
    if (string.length() == 0)
        return;

    if (!(itemization_mode & QTextLayout::NoBidi)) {
        if (direction == QChar::DirON)
            direction = basicDirection(string);
        bidiItemize(this, direction == QChar::DirR, itemization_mode);
    } else {
        BidiControl control(false);
        if (itemization_mode & QTextLayout::SingleLine)
            control.singleLine = true;
        int start = 0;
        int stop = string.length() - 1;
        appendItems(this, start, stop, control, QChar::DirL);
    }
    if ((itemization_mode & QTextEngine::WidthOnly) == WidthOnly)
        widthOnly = true;

    if (docLayout)
        setFormatsFromDocument();
}

int QTextEngine::findItem(int strPos) const
{
    // ##### use binary search
    int item;
    for (item = items.size()-1; item > 0; --item) {
        if (items[item].position <= strPos)
            break;
    }
    return item;
}

float QTextEngine::width(int from, int len) const
{
    float w = 0;

//     qDebug("QTextEngine::width(from = %d, len = %d), numItems=%d, strleng=%d", from,  len, items.size(), string.length());
    for (int i = 0; i < items.size(); i++) {
        const QScriptItem *si = items.constData() + i;
        int pos = si->position;
        int ilen = length(i);
//          qDebug("item %d: from %d len %d", i, pos, ilen);
        if (pos >= from + len)
            break;
        if (pos + ilen > from) {
            if (!si->num_glyphs)
                shape(i);

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
                    w += glyphs[i].advance.x();
            }
        }
    }
//     qDebug("   --> w= %d ", w);
    return w;
}

glyph_metrics_t QTextEngine::boundingBox(int from,  int len) const
{
    glyph_metrics_t gm;

    for (int i = 0; i < items.size(); i++) {
        const QScriptItem *si = items.constData() + i;
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

QFont QTextEngine::font(const QScriptItem &si) const
{
    QFontPrivate *fp = fnt;
    if (formats) {
        Q_ASSERT(formats);
        QTextFormat f = formats->format(si.format);
        Q_ASSERT(f.isCharFormat());
        QTextCharFormat chf = f.toCharFormat();
        QFont fnt = chf.font();
        if (docLayout)
            fnt = fnt.resolve(docLayout->defaultFont());
        return fnt;
    }
    return QFont(fp);
}

QFont QTextEngine::font() const
{
    if (fnt)
        return QFont(fnt);
    return QFont();
}

QFontEngine *QTextEngine::fontEngine(const QScriptItem &si) const
{
    if (!fnt) {
        QFont font = this->font(si);
        return font.d->engineForScript((QFont::Script)si.analysis.script);
    }
    return fnt->engineForScript((QFont::Script)si.analysis.script);
}

struct JustificationPoint {
    int type;
    float kashidaWidth;
    QGlyphLayout *glyph;
    QFontEngine *fontEngine;
};

Q_DECLARE_TYPEINFO(JustificationPoint, Q_PRIMITIVE_TYPE);

static void set(JustificationPoint *point, int type, QGlyphLayout *glyph, QFontEngine *fe)
{
    point->type = type;
    point->glyph = glyph;
    point->fontEngine = fe;

    if (type >= QGlyphLayout::Arabic_Normal) {
        QChar ch(0x640); // Kashida character
        QGlyphLayout glyphs[8];
        int nglyphs = 7;
        fe->stringToCMap(&ch, 1, glyphs, &nglyphs, 0);
        if (glyphs[0].glyph && glyphs[0].advance.x()) {
            point->kashidaWidth = glyphs[0].advance.x();
        } else {
            point->type = QGlyphLayout::NoJustification;
            point->kashidaWidth = 0.;
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

    if ((textFlags & Qt::AlignHorizontal_Mask) != Qt::AlignJustify) {
        return;
    }

    // justify line
    int maxJustify = 0;

    // don't include trailing white spaces when doing justification
    int line_length = line.length;
    const QCharAttributes *a = attributes()+line.from;
    while (line_length && a[line_length-1].whiteSpace)
        --line_length;
    // subtract one char more, as we can't justfy after the last character
    --line_length;

    if (!line_length) {
        const_cast<QScriptLine &>(line).justified = true;
        return;
    }

    int firstItem = findItem(line.from);
    int nItems = findItem(line.from + line_length - 1) - firstItem + 1;

    QVarLengthArray<JustificationPoint> justificationPoints;
    int nPoints = 0;
//     qDebug("justifying from %d len %d, firstItem=%d, nItems=%d", line.from, line_length, firstItem, nItems);
    float minKashida = 0x100000;

    for (int i = 0; i < nItems; ++i) {
        QScriptItem &si = items[firstItem + i];
        if (!si.num_glyphs)
            shape(firstItem + i);

        int kashida_type = QGlyphLayout::Arabic_Normal;
        int kashida_pos = -1;

        int start = qMax(line.from - si.position, 0);
        int end = qMin(line.from + line_length - (int)si.position, length(firstItem+i));

        unsigned short *log_clusters = logClusters(&si);

        int gs = log_clusters[start];
        int ge = (end == length(firstItem+i) ? si.num_glyphs : log_clusters[end]);

        QGlyphLayout *g = glyphs(&si);

        for (int i = gs; i < ge; ++i) {
            g[i].justificationType = QGlyphLayout::JustifyNone;
            g[i].nKashidas = 0;
            g[i].space_18d6 = 0;

            justificationPoints.resize(nPoints+3);
            int justification = g[i].attributes.justification;

            switch(justification) {
            case QGlyphLayout::NoJustification:
                break;
            case QGlyphLayout::Space          :
                // fall through
            case QGlyphLayout::Arabic_Space   :
                if (kashida_pos >= 0) {
//                     qDebug("kashida position at %d in word", kashida_pos);
                    set(&justificationPoints[nPoints], kashida_type, g+kashida_pos, fontEngine(si));
                    minKashida = qMin(minKashida, justificationPoints[nPoints].kashidaWidth);
                    maxJustify = qMax(maxJustify, justificationPoints[nPoints].type);
                    ++nPoints;
                }
                kashida_pos = -1;
                kashida_type = QGlyphLayout::Arabic_Normal;
                // fall through
            case QGlyphLayout::Character      :
                set(&justificationPoints[nPoints++], justification, g+i, fontEngine(si));
                maxJustify = qMax(maxJustify, justification);
                break;
            case QGlyphLayout::Arabic_Normal  :
            case QGlyphLayout::Arabic_Waw     :
            case QGlyphLayout::Arabic_BaRa    :
            case QGlyphLayout::Arabic_Alef    :
            case QGlyphLayout::Arabic_HaaDal  :
            case QGlyphLayout::Arabic_Seen    :
            case QGlyphLayout::Arabic_Kashida :
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

    // don't justify last line. Return here, so we ensure justificationType etc. are reset.
    if (line.from + (int)line.length == string.length()) {
        const_cast<QScriptLine &>(line).justified = true;
        return;
    }


    float need = line.width - line.textWidth;
    if (need < 0) {
        // line overflows already!
        const_cast<QScriptLine &>(line).justified = true;
        return;
    }

//     qDebug("doing justification: textWidth=%x, requested=%x, maxJustify=%d", line.textWidth.value(), line.width.value(), maxJustify);
//     qDebug("     minKashida=%f, need=%f", minKashida, need);

    // distribute in priority order
    if (maxJustify >= QGlyphLayout::Arabic_Normal) {
        while (need >= minKashida) {
            for (int type = maxJustify; need >= minKashida && type >= QGlyphLayout::Arabic_Normal; --type) {
                for (int i = 0; need >= minKashida && i < nPoints; ++i) {
                    if (justificationPoints[i].type == type && justificationPoints[i].kashidaWidth <= need) {
                        justificationPoints[i].glyph->nKashidas++;
                        // ############
                        justificationPoints[i].glyph->space_18d6 += (int)justificationPoints[i].kashidaWidth*64;
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

    maxJustify = qMin(maxJustify, (int)QGlyphLayout::Space);
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
                float add = need/n;
//                  qDebug("adding %x to glyph %x", add.value(), justificationPoints[i].glyph->glyph);
                justificationPoints[i].glyph->space_18d6 = qRound(add*64);
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

    if (eng->fnt) {
        e = eng->fnt->engineForScript(QFont::Latin);
    } else {
        f = eng->block.charFormat().font();
        if (eng->docLayout)
            f = f.resolve(eng->docLayout->defaultFont());
        e = f.d->engineForScript(QFont::Latin);
    }

    ascent = e->ascent();
    descent = e->descent();
}

void QTextEngine::freeMemory()
{
    free(memory);
    memory = 0;
    haveCharAttributes = false;
    allocated = 0;
    num_glyphs = 0;
    used = 0;
    for (int i = 0; i < lines.size(); ++i) {
        lines[i].justified = 0;
        lines[i].gridfitted = 0;
    }
//     for (int i = 0; i < items.size(); ++i)
//         items[i].num_glyphs = 0;
    items.clear();
}

void QTextEngine::invalidate()
{
    freeMemory();
    lines.clear();
    boundingRect = QRect();
    minWidth = 0;
    maxWidth = 0;
    invalid = true;
}

void QTextEngine::validate()
{
    if (invalid && docLayout)
        string = block.text();
    invalid = false;
}

void QTextEngine::setFormatsFromDocument()
{
    int lastTextPosition = 0;
    int textLength = 0;

    const QTextDocumentPrivate *p = block.docHandle();
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
