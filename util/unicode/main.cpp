#include <qlist.h>
#include <qhash.h>
#include <qfile.h>
#include <qstring.h>
#include <qchar.h>
#include <private/qunicodetables_p.h>

#if 0
// accessing properties goes via:

int toUpper(int ucs4)
{
    const int BLOCKSIZE = 128;
    ushort offset = offsetMap[ucs4/BLOCKSIZE];
    ushort propertyIndex = trie[offset + ucs4%(BLOCKSIZE-1)];
    PropertyFlags *p = propertyFlags + propertyIndex;
    if (p->category == QChar::Letter_Lowercase)
        return ucs4 + p->caseDiff;
    return ucs4;
}


#endif

struct PropertyFlags {
    bool operator ==(const PropertyFlags &o) {
        return (combiningClass == o.combiningClass
                && category == o.category
                && direction == o.direction
                && titleCaseDiffersFromUpper == o.titleCaseDiffersFromUpper
                && joining == o.joining
                && age == o.age
                && mirrorDiff == o.mirrorDiff
                && line_break_class == o.line_break_class
                && caseDiff == o.caseDiff
                && digitValue == o.digitValue);
    }
    // from UnicodeData.txt
    uchar combiningClass : 8;
    QChar::Category category : 5;
    QChar::Direction direction : 5;
    // there are only very few codepoints (4 in Unicode 4.0) where titlecase is different from uppercase.
    // a bool is enought to mark them.
    bool titleCaseDiffersFromUpper : 1;
    // from ArabicShaping.txt
    QChar::Joining joining : 2;
    // from DerivedAge.txt
    QChar::UnicodeVersion age : 4;
    uint digitValue : 4;
    uint unused : 2;
    uint line_break_class : 5;

    int mirrorDiff : 16;
    int caseDiff : 16;
};


struct UnicodeData {
    UnicodeData(int codepoint) {
        p.category = QChar::NoCategory;
        p.combiningClass = 0;

        p.direction = QChar::DirL;
        // DirR for:  U+0590..U+05FF, U+07C0..U+08FF, U+FB1D..U+FB4F, U+10800..U+10FFF
        if ((codepoint >= 0x590 && codepoint <= 0x5ff)
            || (codepoint >= 0x7c0 && codepoint <= 0x8ff)
            || (codepoint >= 0xfb1d && codepoint <= 0xfb4f)
            || (codepoint >= 0x10800 && codepoint <= 0x10fff))
            p.direction = QChar::DirR;
        // DirAL for: U+0600..U+07BF, U+FB50..U+FDCF, U+FDF0..U+FDFF, U+FE70..U+FEFE
        if ((codepoint >= 0x600 && codepoint <= 0x7bf)
            || (codepoint >= 0xfb50 && codepoint <= 0xfdcf)
            || (codepoint >= 0xfdf0 && codepoint <= 0xfdff)
            || (codepoint >= 0xfe70 && codepoint <= 0xfefe))
            p.direction = QChar::DirAL;

        mirroredChar = 0;
        decompositionType = QChar::NoDecomposition;
        p.joining = QChar::OtherJoining;
        otherCase = 0;
        p.titleCaseDiffersFromUpper = false;
        p.age = QChar::Unassigned;
        p.unused = 0;
        p.mirrorDiff = 0;
        p.caseDiff = 0;
        p.digitValue = 0xf;
        p.line_break_class = QUnicodeTables::LineBreak_AL;
        propertyIndex = -1;
        excludedComposition = false;
    }
    PropertyFlags p;

    // from UnicodeData.txt
    QChar::Decomposition decompositionType;
    QList<int> decomposition;

    // from BidiMirroring.txt
    int mirroredChar;
    int otherCase;

    // CompositionExclusions.txt
    bool excludedComposition;

    // computed position of unicode property set
    int propertyIndex;
};

enum UniDataFields {
    UD_Value,
    UD_Name,
    UD_Category,
    UD_CombiningClass,
    UD_BidiCategory,
    UD_Decomposition,
    UD_DecimalDigitValue,
    UD_DigitValue,
    UD_NumericValue,
    UD_Mirrored,
    UD_OldName,
    UD_Comment,
    UD_UpperCase,
    UD_LowerCase,
    UD_TitleCase
};

QHash<QByteArray, QChar::Category> categoryMap;

static void initCategoryMap()
{
    struct Cat {
        QChar::Category cat;
        const char *name;
    } categories [] = {
        { QChar::Mark_NonSpacing,          "Mn" },
        { QChar::Mark_SpacingCombining,    "Mc" },
        { QChar::Mark_Enclosing,           "Me" },

        { QChar::Number_DecimalDigit,      "Nd" },
        { QChar::Number_Letter,            "Nl" },
        { QChar::Number_Other,             "No" },

        { QChar::Separator_Space,          "Zs" },
        { QChar::Separator_Line,           "Zl" },
        { QChar::Separator_Paragraph,      "Zp" },

        { QChar::Other_Control,            "Cc" },
        { QChar::Other_Format,             "Cf" },
        { QChar::Other_Surrogate,          "Cs" },
        { QChar::Other_PrivateUse,         "Co" },
        { QChar::Other_NotAssigned,        "Cn" },

        { QChar::Letter_Uppercase,         "Lu" },
        { QChar::Letter_Lowercase,         "Ll" },
        { QChar::Letter_Titlecase,         "Lt" },
        { QChar::Letter_Modifier,          "Lm" },
        { QChar::Letter_Other,             "Lo" },

        { QChar::Punctuation_Connector,    "Pc" },
        { QChar::Punctuation_Dash,         "Pd" },
        { QChar::Punctuation_Open,         "Ps" },
        { QChar::Punctuation_Close,        "Pe" },
        { QChar::Punctuation_InitialQuote, "Pi" },
        { QChar::Punctuation_FinalQuote,   "Pf" },
        { QChar::Punctuation_Other,        "Po" },

        { QChar::Symbol_Math,              "Sm" },
        { QChar::Symbol_Currency,          "Sc" },
        { QChar::Symbol_Modifier,          "Sk" },
        { QChar::Symbol_Other,             "So" },
        { QChar::NoCategory, 0 }
    };
    Cat *c = categories;
    while (c->cat != QChar::NoCategory) {
        categoryMap.insert(c->name, c->cat);
        ++c;
    }
}

QHash<QByteArray, QChar::Direction> directionMap;

static void initDirectionMap()
{
    struct Dir {
        QChar::Direction dir;
        const char *name;
    } directions[] = {
        { QChar::DirL, "L" },
        { QChar::DirR, "R" },
        { QChar::DirEN, "EN" },
        { QChar::DirES, "ES" },
        { QChar::DirET, "ET" },
        { QChar::DirAN, "AN" },
        { QChar::DirCS, "CS" },
        { QChar::DirB, "B" },
        { QChar::DirS, "S" },
        { QChar::DirWS, "WS" },
        { QChar::DirON, "ON" },
        { QChar::DirLRE, "LRE" },
        { QChar::DirLRO, "LRO" },
        { QChar::DirAL, "AL" },
        { QChar::DirRLE, "RLE" },
        { QChar::DirRLO, "RLO" },
        { QChar::DirPDF, "PDF" },
        { QChar::DirNSM, "NSM" },
        { QChar::DirBN, "BN" },
        { QChar::DirL, 0 }
    };
    Dir *d = directions;
    while (d->name) {
        directionMap.insert(d->name, d->dir);
        ++d;
    }
}


QHash<QByteArray, QChar::Decomposition> decompositionMap;

static void initDecompositionMap()
{
    struct Dec {
        QChar::Decomposition dec;
        const char *name;
    } decompositions[] = {
        { QChar::Canonical, "<canonical>" },
        { QChar::Font, "<font>" },
        { QChar::NoBreak, "<noBreak>" },
        { QChar::Initial, "<initial>" },
        { QChar::Medial, "<medial>" },
        { QChar::Final, "<final>" },
        { QChar::Isolated, "<isolated>" },
        { QChar::Circle, "<circle>" },
        { QChar::Super, "<super>" },
        { QChar::Sub, "<sub>" },
        { QChar::Vertical, "<vertical>" },
        { QChar::Wide, "<wide>" },
        { QChar::Narrow, "<narrow>" },
        { QChar::Small, "<small>" },
        { QChar::Square, "<square>" },
        { QChar::Compat, "<compat>" },
        { QChar::Fraction, "<fraction>" },
        { QChar::NoDecomposition,  0 }
    };
    Dec *d = decompositions;
    while (d->name) {
        decompositionMap.insert(d->name, d->dec);
        ++d;
    }
}


QHash<int, UnicodeData> unicodeData;
QList<PropertyFlags> uniqueProperties;


QHash<int, int> decompositionLength;
int highestComposedCharacter = 0;
int numLigatures = 0;
int highestLigature = 0;

struct Ligature {ushort u1; ushort u2; ushort ligature;};
// we need them sorted after the first component for fast lookup
bool operator < (const Ligature &l1, const Ligature &l2) {
    return l1.u1 < l2.u1;
}

QHash<ushort, QList<Ligature> > ligatureHashes;

QHash<int, int> combiningClassUsage;

int maxCaseDiff = 0;

static void readUnicodeData()
{
    QFile f("data/UnicodeData.txt");
    if (!f.exists())
        qFatal("Couldn't find UnicodeData.txt");

    f.open(QFile::IO_ReadOnly);

    while (!f.atEnd()) {
        QByteArray line;
        line.resize(1024);
        int len = f.readLine(line.data(), 1024);
        line.truncate(len-1);

        int comment = line.indexOf('#');
        if (comment >= 0)
            line = line.left(comment);
        if (line.isEmpty())
            continue;

        QList<QByteArray> properties = line.split(';');
        bool ok;
        int codepoint = properties[UD_Value].toInt(&ok, 16);
        int lastCodepoint = codepoint;

        QByteArray name = properties[UD_Name];
        if (name.startsWith('<') && name.contains("First")) {
            QByteArray nextLine;
            nextLine.resize(1024);
            f.readLine(nextLine.data(), 1024);
            QList<QByteArray> properties = line.split(';');
            lastCodepoint = properties[UD_Value].toInt(&ok, 16);
        }

        UnicodeData data(codepoint);
        data.p.category = categoryMap.value(properties[UD_Category], QChar::NoCategory);
        data.p.combiningClass = properties[UD_CombiningClass].toInt();

        if (!combiningClassUsage.contains(data.p.combiningClass))
            combiningClassUsage[data.p.combiningClass] = 1;
        else
            ++combiningClassUsage[data.p.combiningClass];

        data.p.direction = directionMap.value(properties[UD_BidiCategory], data.p.direction);

        if (data.p.category == QChar::Letter_Lowercase) {
            data.otherCase = properties[UD_UpperCase].toInt(&ok, 16);
            int titleCase = properties[UD_TitleCase].toInt(&ok, 16);
            if (titleCase != data.otherCase) {
                Q_ASSERT(titleCase-1 == data.otherCase);
                data.p.titleCaseDiffersFromUpper = true;
            }
        } else if (data.p.category == QChar::Letter_Uppercase
                   || data.p.category == QChar::Letter_Titlecase) {
            data.otherCase = properties[UD_LowerCase].toInt(&ok, 16);
        }
        if (data.otherCase == 0)
            data.otherCase = codepoint;
        if (QABS(codepoint - data.otherCase) > maxCaseDiff)
            maxCaseDiff = QABS(codepoint - data.otherCase);
        if (QABS(codepoint - data.otherCase) >= 0x8000)
            qFatal("case diff not in range at codepoint %x: othercase=%x", codepoint, data.otherCase);
        data.p.caseDiff = data.otherCase - codepoint;

        if (!properties[UD_DigitValue].isEmpty())
            data.p.digitValue = properties[UD_DigitValue].toInt();

        // decompositition
        QByteArray decomposition = properties[UD_Decomposition];
        if (!decomposition.isEmpty()) {
            highestComposedCharacter = qMax(highestComposedCharacter, codepoint);
            QList<QByteArray> d = decomposition.split(' ');
            if (d[0].contains('<')) {
                data.decompositionType = decompositionMap.value(d[0], QChar::Canonical);
                d.takeFirst();
            } else {
                data.decompositionType = QChar::Canonical;
            }
            for (int i = 0; i < d.size(); ++i)
                data.decomposition.append(d[i].toInt(&ok, 16));
            if (!decompositionLength.contains(data.decomposition.size()))
                decompositionLength[data.decomposition.size()] = 1;
            else
                ++decompositionLength[data.decomposition.size()];
        }

        for (int i = codepoint; i <= lastCodepoint; ++i)
            unicodeData.insert(i, data);
    }

}

static int maxMirroredDiff = 0;

static void readBidiMirroring()
{
    QFile f("data/BidiMirroring.txt");
    if (!f.exists())
        qFatal("Couldn't find BidiMirroring.txt");

    f.open(QFile::IO_ReadOnly);

    while (!f.atEnd()) {
        QByteArray line;
        line.resize(1024);
        int len = f.readLine(line.data(), 1024);
        line.resize(len-1);

        int comment = line.indexOf('#');
        if (comment >= 0)
            line = line.left(comment);

        if (line.isEmpty())
            continue;
        line = line.replace(" ", "");

        QList<QByteArray> pair = line.split(';');
        Q_ASSERT(pair.size() == 2);

        bool ok;
        int codepoint = pair[0].toInt(&ok, 16);
        int mirror = pair[1].toInt(&ok, 16);

        UnicodeData d = unicodeData.value(codepoint, UnicodeData(codepoint));
        d.mirroredChar = mirror;
        if (QABS(codepoint-d.mirroredChar) > maxMirroredDiff) {
            maxMirroredDiff = QABS(codepoint - d.mirroredChar);
        }
        Q_ASSERT(QABS(codepoint - d.mirroredChar) < 0x8000);
        d.p.mirrorDiff = d.mirroredChar - codepoint;
        unicodeData.insert(codepoint, d);
    }
}

static void readArabicShaping()
{
    QFile f("data/ArabicShaping.txt");
    if (!f.exists())
        qFatal("Couldn't find ArabicShaping.txt");

    f.open(QFile::IO_ReadOnly);

    while (!f.atEnd()) {
        QByteArray line;
        line.resize(1024);
        int len = f.readLine(line.data(), 1024);
        line.resize(len-1);

        int comment = line.indexOf('#');
        if (comment >= 0)
            line = line.left(comment);
        line = line.trimmed();

        if (line.isEmpty())
            continue;

        QList<QByteArray> shaping = line.split(';');
        Q_ASSERT(shaping.size() == 4);

        bool ok;
        int codepoint = shaping[0].toInt(&ok, 16);
        QChar::Joining j = QChar::OtherJoining;
        QByteArray shape = shaping[2].trimmed();
        if (shape == "R")
            j = QChar::Right;
        else if (shape == "D")
            j = QChar::Dual;
        else if (shape == "C")
            j = QChar::Center;

        UnicodeData d = unicodeData.value(codepoint, UnicodeData(codepoint));
        d.p.joining = j;
        unicodeData.insert(codepoint, d);
    }
}

static void readDerivedAge()
{
    QFile f("data/DerivedAge.txt");
    if (!f.exists())
        qFatal("Couldn't find DerivedAge.txt");

    f.open(QFile::IO_ReadOnly);

    while (!f.atEnd()) {
        QByteArray line;
        line.resize(1024);
        int len = f.readLine(line.data(), 1024);
        line.resize(len-1);

        int comment = line.indexOf('#');
        if (comment >= 0)
            line = line.left(comment);
        line.replace(" ", "");

        if (line.isEmpty())
            continue;

        QList<QByteArray> l = line.split(';');
        Q_ASSERT(l.size() == 2);

        QByteArray codes = l[0];
        codes.replace("..", ".");
        QList<QByteArray> cl = codes.split('.');

        bool ok;
        int from = cl[0].toInt(&ok, 16);
        int to = from;
        if (cl.size() == 2)
            to = cl[1].toInt(&ok, 16);

        QChar::UnicodeVersion age = QChar::Unassigned;
        QByteArray ba = l[1];
        if (ba == "1.1")
            age = QChar::Unicode_1_1;
        else if (ba == "2.0")
            age = QChar::Unicode_2_0;
        else if (ba == "2.1")
            age = QChar::Unicode_2_1_2;
        else if (ba == "3.0")
            age = QChar::Unicode_3_0;
        else if (ba == "3.1")
            age = QChar::Unicode_3_1;
        else if (ba == "3.2")
            age = QChar::Unicode_3_2;
        else if (ba == "4.0")
            age = QChar::Unicode_4_0;
        Q_ASSERT(age != QChar::Unassigned);

        for (int codepoint = from; codepoint <= to; ++codepoint) {
            UnicodeData d = unicodeData.value(codepoint, UnicodeData(codepoint));
            d.p.age = age;
            unicodeData.insert(codepoint, d);
        }
    }
}


static void readCompositionExclusion()
{
    QFile f("data/CompositionExclusions.txt");
    if (!f.exists())
        qFatal("Couldn't find CompositionExclusions.txt");

    f.open(QFile::IO_ReadOnly);

    while (!f.atEnd()) {
        QByteArray line;
        line.resize(1024);
        int len = f.readLine(line.data(), 1024);
        line.resize(len-1);

        int comment = line.indexOf('#');
        if (comment >= 0)
            line = line.left(comment);
        line.replace(" ", "");

        if (line.isEmpty())
            continue;

        Q_ASSERT(!line.contains(".."));

        bool ok;
        int codepoint = line.toInt(&ok, 16);

        UnicodeData d = unicodeData.value(codepoint, UnicodeData(codepoint));
        d.excludedComposition = true;
        unicodeData.insert(codepoint, d);
    }

    for (int i = 0; i < 0x110000; ++i) {
        UnicodeData data = unicodeData.value(i, UnicodeData(i));
        if (!data.excludedComposition
            && data.decompositionType == QChar::Canonical && data.decomposition.size() > 1) {
            Q_ASSERT(data.decomposition.size() == 2);

            ++numLigatures;
            highestLigature = qMax(highestLigature, data.decomposition.at(0));
            Ligature l = {(ushort)data.decomposition.at(0), (ushort)data.decomposition.at(1), i};
            ligatureHashes[data.decomposition.at(1)].append(l);
        }
    }
}

static void computeUniqueProperties()
{
    qDebug("computeUniqueProperties:");
    for (int uc = 0; uc < 0x110000; ++uc) {
        UnicodeData d = unicodeData.value(uc, UnicodeData(uc));

        int index = uniqueProperties.indexOf(d.p);
        if (index == -1) {
            index = uniqueProperties.size();
            uniqueProperties.append(d.p);
        }
        d.propertyIndex = index;
        unicodeData.insert(uc, d);
    }
    qDebug("    %d unicode properties found", uniqueProperties.size());
}


static void readLineBreak()
{
    QFile f("data/LineBreak.txt");
    if (!f.exists())
        qFatal("Couldn't find LineBreak.txt");

    f.open(QFile::IO_ReadOnly);

    while (!f.atEnd()) {
        QByteArray line;
        line.resize(1024);
        int len = f.readLine(line.data(), 1024);
        line.resize(len-1);

        int comment = line.indexOf('#');
        if (comment >= 0)
            line = line.left(comment);
        line.replace(" ", "");

        if (line.isEmpty())
            continue;

        QList<QByteArray> l = line.split(';');
        Q_ASSERT(l.size() == 2);

        QByteArray codes = l[0];
        codes.replace("..", ".");
        QList<QByteArray> cl = codes.split('.');

        bool ok;
        int from = cl[0].toInt(&ok, 16);
        int to = from;
        if (cl.size() == 2)
            to = cl[1].toInt(&ok, 16);

        // ### Classes XX and AI are left out and mapped to AL for now
        QUnicodeTables::LineBreakClass lb = QUnicodeTables::LineBreak_AL;
        QByteArray ba = l[1];

        if (ba == "OP") lb = QUnicodeTables::LineBreak_OP;
        else if (ba == "CL") lb = QUnicodeTables::LineBreak_CL;
        else if (ba == "QU") lb = QUnicodeTables::LineBreak_QU;
        else if (ba == "GL") lb = QUnicodeTables::LineBreak_GL;
        else if (ba == "NS") lb = QUnicodeTables::LineBreak_NS;
        else if (ba == "EX") lb = QUnicodeTables::LineBreak_EX;
        else if (ba == "SY") lb = QUnicodeTables::LineBreak_SY;
        else if (ba == "IS") lb = QUnicodeTables::LineBreak_IS;
        else if (ba == "PR") lb = QUnicodeTables::LineBreak_PR;
        else if (ba == "PO") lb = QUnicodeTables::LineBreak_PO;
        else if (ba == "NU") lb = QUnicodeTables::LineBreak_NU;
        else if (ba == "AL") lb = QUnicodeTables::LineBreak_AL;
        else if (ba == "ID") lb = QUnicodeTables::LineBreak_ID;
        else if (ba == "IN") lb = QUnicodeTables::LineBreak_IN;
        else if (ba == "HY") lb = QUnicodeTables::LineBreak_HY;
        else if (ba == "BA") lb = QUnicodeTables::LineBreak_BA;
        else if (ba == "BB") lb = QUnicodeTables::LineBreak_BB;
        else if (ba == "B2") lb = QUnicodeTables::LineBreak_B2;
        else if (ba == "ZW") lb = QUnicodeTables::LineBreak_ZW;
        else if (ba == "CM") lb = QUnicodeTables::LineBreak_CM;
        else if (ba == "SA") lb = QUnicodeTables::LineBreak_SA;
        else if (ba == "BK") lb = QUnicodeTables::LineBreak_BK;
        else if (ba == "CR") lb = QUnicodeTables::LineBreak_CR;
        else if (ba == "LF") lb = QUnicodeTables::LineBreak_LF;
        else if (ba == "SG") lb = QUnicodeTables::LineBreak_SG;
        else if (ba == "CB") lb = QUnicodeTables::LineBreak_CB;
        else if (ba == "SP") lb = QUnicodeTables::LineBreak_SP;

        for (int codepoint = from; codepoint <= to; ++codepoint) {
            UnicodeData d = unicodeData.value(codepoint, UnicodeData(codepoint));
            d.p.line_break_class = lb;
            unicodeData.insert(codepoint, d);
        }
    }
}


static void dump(int from, int to)
{
    for (int i = from; i <= to; ++i) {
        UnicodeData d = unicodeData.value(i, UnicodeData(i));
        qDebug("0x%04x: cat=%d combining=%d dir=%d case=%x mirror=%x joining=%d age=%d",
               i, d.p.category, d.p.combiningClass, d.p.direction, d.otherCase, d.mirroredChar, d.p.joining, d.p.age);
        if (d.decompositionType != QChar::NoDecomposition) {
            qDebug("    decomposition: type=%d, length=%d, first=%x", d.decompositionType, d.decomposition.size(),
                   d.decomposition[0]);
        }
    }
    qDebug(" ");
}

struct PropertyBlock {
    PropertyBlock() { index = -1; }
    int index;
    QList<int> properties;
    bool operator ==(const PropertyBlock &other) { return properties == other.properties; }
};

static QByteArray createPropertyInfo()
{
    qDebug("createPropertyInfo:");

    const int BMP_BLOCKSIZE=32;
    const int BMP_SHIFT = 5;
    const int BMP_END = 0x11000;
    const int SMP_END = 0x110000;
    const int SMP_BLOCKSIZE = 256;
    const int SMP_SHIFT = 8;

    QList<PropertyBlock> blocks;
    QList<int> blockMap;

    int used = 0;

    for (int block = 0; block < BMP_END/BMP_BLOCKSIZE; ++block) {
        PropertyBlock b;
        for (int i = 0; i < BMP_BLOCKSIZE; ++i) {
            int uc = block*BMP_BLOCKSIZE + i;
            UnicodeData d = unicodeData.value(uc, UnicodeData(uc));
            b.properties.append(d.propertyIndex);
        }
        int index = blocks.indexOf(b);
        if (index == -1) {
            index = blocks.size();
            b.index = used;
            used += BMP_BLOCKSIZE;
            blocks.append(b);
        }
        blockMap.append(blocks.at(index).index);
    }

    int bmp_blocks = blocks.size();
    Q_ASSERT(blockMap.size() == BMP_END/BMP_BLOCKSIZE);

    for (int block = BMP_END/SMP_BLOCKSIZE; block < SMP_END/SMP_BLOCKSIZE; ++block) {
        PropertyBlock b;
        for (int i = 0; i < SMP_BLOCKSIZE; ++i) {
            int uc = block*SMP_BLOCKSIZE + i;
            UnicodeData d = unicodeData.value(uc, UnicodeData(uc));
            b.properties.append(d.propertyIndex);
        }
        int index = blocks.indexOf(b);
        if (index == -1) {
            index = blocks.size();
            b.index = used;
            used += SMP_BLOCKSIZE;
            blocks.append(b);
        }
        blockMap.append(blocks.at(index).index);
    }

    int bmp_block_data = bmp_blocks*BMP_BLOCKSIZE*2;
    int bmp_trie = BMP_END/BMP_BLOCKSIZE*2;
    int bmp_mem = bmp_block_data + bmp_trie;
    qDebug("    %d unique blocks in BMP.",blocks.size());
    qDebug("        block data uses: %d bytes", bmp_block_data);
    qDebug("        trie data uses : %d bytes", bmp_trie);
    qDebug("        memory usage: %d bytes", bmp_mem);

    int smp_block_data = (blocks.size()- bmp_blocks)*SMP_BLOCKSIZE*2;
    int smp_trie = (SMP_END-BMP_END)/SMP_BLOCKSIZE*2;
    int smp_mem = smp_block_data + smp_trie;
    qDebug("    %d unique blocks in SMP.",blocks.size()-bmp_blocks);
    qDebug("        block data uses: %d bytes", smp_block_data);
    qDebug("        trie data uses : %d bytes", smp_trie);

    qDebug("\n        properties use : %d bytes", uniqueProperties.size()*8);
    qDebug("    memory usage: %d bytes", bmp_mem+smp_mem + uniqueProperties.size()*8);

    QByteArray out;
    out += "static const unsigned short uc_property_trie[] = {\n";

    // first write the map
    out += "    // 0x" + QByteArray::number(BMP_END, 16);
    for (int i = 0; i < BMP_END/BMP_BLOCKSIZE; ++i) {
        if (!(i % 8)) {
            if (!((i*BMP_BLOCKSIZE) % 0x1000))
                out += "\n";
            out += "\n    ";
        }
        out += QByteArray::number(blockMap.at(i) + blockMap.size());
        out += ", ";
    }
    out += "\n\n    // 0x" + QByteArray::number(BMP_END, 16) + " - 0x" + QByteArray::number(SMP_END, 16) + "\n";;
    for (int i = BMP_END/BMP_BLOCKSIZE; i < blockMap.size(); ++i) {
        if (!(i % 8)) {
            if (!(i % (0x10000/SMP_BLOCKSIZE)))
                out += "\n";
            out += "\n    ";
        }
        out += QByteArray::number(blockMap.at(i) + blockMap.size());
        out += ", ";
    }
    out += "\n";
    // write the data
    for (int i = 0; i < blocks.size(); ++i) {
        out += "\n";
        const PropertyBlock &b = blocks.at(i);
        for (int j = 0; j < b.properties.size(); ++j) {
            if (!(j % 8))
                out += "\n    ";
            out += QByteArray::number(b.properties.at(j));
            out += ", ";
        }
    }

    // we reserve one bit more than in the assert below for the sign
    Q_ASSERT(maxMirroredDiff < (1<<12));
    Q_ASSERT(maxCaseDiff < (1<<14));

    out += "\n};\n\n"

           "#define GET_PROP_INDEX(ucs4) \\\n"
           "       (ucs4 < 0x" + QByteArray::number(BMP_END, 16) + " \\\n"
           "        ? (uc_property_trie[uc_property_trie[ucs4>>" + QByteArray::number(BMP_SHIFT) +
           "] + (ucs4 & 0x" + QByteArray::number(BMP_BLOCKSIZE-1, 16)+ ")]) \\\n"
           "        : (uc_property_trie[uc_property_trie[((ucs4 - 0x" + QByteArray::number(BMP_END, 16) +
           ")>>" + QByteArray::number(SMP_SHIFT) + ") + 0x" + QByteArray::number(BMP_END/BMP_BLOCKSIZE, 16) + "]"
           " + (ucs4 & 0x" + QByteArray::number(SMP_BLOCKSIZE-1, 16) + ")]))\n\n"

           "struct UC_Properties {\n"
           "    uint category : 5;\n"
           "    uint line_break_class : 5;"
           "    uint direction : 5;\n"
           "    uint titleCaseDiffersFromUpper : 1;\n"
           "    uint combiningClass :8;\n"
           "    uint unicode_version : 4;\n"
           "    uint digit_value : 4;\n"
           "    \n"
           "    signed short mirrorDiff : 14 /* 13 needed */;\n"
           "    uint joining : 2;\n"
           "    signed short caseDiff /* 14 needed */;\n"
           "};\n\n"

           "static const UC_Properties uc_properties [] = {\n";

    for (int i = 0; i < uniqueProperties.size(); ++i) {
        PropertyFlags p = uniqueProperties.at(i);
        out += "    { ";
        out += QByteArray::number( p.category );
        out += ", ";
        out += QByteArray::number( p.line_break_class );
        out += ", ";
        out += QByteArray::number( p.direction );
        out += ", ";
        out += p.titleCaseDiffersFromUpper ? "1" : "0";
        out += ", ";
        out += QByteArray::number( p.combiningClass );
        out += ", ";
        out += QByteArray::number( p.age );
        out += ", ";
        out += QByteArray::number( p.digitValue );
        out += ", ";
        out += QByteArray::number( p.mirrorDiff );
        out += ", ";
        out += QByteArray::number( p.joining );
        out += ", ";
        out += QByteArray::number( p.caseDiff );
        out += "},\n";
    }
    out += "};\n\n"

           "#define GET_PROP(ucs4) (uc_properties + GET_PROP_INDEX(ucs4))\n\n"

           "QChar::Category QUnicodeTables::category(uint ucs4)\n"
           "{\n"
           "    return (QChar::Category) GET_PROP(ucs4)->category;\n"
           "}\n\n"

           "unsigned char QUnicodeTables::combiningClass(uint ucs4)\n"
           "{\n"
           "    return (unsigned char) GET_PROP(ucs4)->combiningClass;\n"
           "}\n\n"

           "QChar::Direction QUnicodeTables::direction(uint ucs4)\n"
           "{\n"
           "    return (QChar::Direction) GET_PROP(ucs4)->direction;\n"
           "}\n\n"

           "QChar::Joining QUnicodeTables::joining(uint ucs4)\n"
           "{\n"
           "    return (QChar::Joining) GET_PROP(ucs4)->joining;\n"
           "}\n\n"

           "QChar::UnicodeVersion QUnicodeTables::unicodeVersion(uint ucs4)\n"
           "{\n"
           "    return (QChar::UnicodeVersion) GET_PROP(ucs4)->unicode_version;\n"
           "}\n\n"

           "int QUnicodeTables::digitValue(uint ucs4)\n"
           "{\n"
           "    int val = GET_PROP(ucs4)->digit_value;\n"
           "    return val == 0xf ? -1 : val;\n"
           "}\n\n"

           "bool QUnicodeTables::mirrored(uint ucs4)\n"
           "{\n"
           "    return GET_PROP(ucs4)->mirrorDiff != 0;\n"
           "}\n\n"

           "int QUnicodeTables::mirroredChar(uint ucs4)\n"
           "{\n"
           "    return ucs4 + GET_PROP(ucs4)->mirrorDiff;\n"
           "}\n\n"

           "QUnicodeTables::LineBreakClass QUnicodeTables::lineBreakClass(uint ucs4)\n"
           "{\n"
           "    return (QUnicodeTables::LineBreakClass) GET_PROP(ucs4)->line_break_class;\n"
           "}\n\n"

           "int QUnicodeTables::upper(uint ucs4)\n"
           "{\n"
           "    const UC_Properties *p = GET_PROP(ucs4);\n"
           "    if (p->category == QChar::Letter_Lowercase)\n"
           "        return ucs4 + p->caseDiff;\n"
           "    return ucs4;\n"
           "}\n\n"

           "int QUnicodeTables::lower(uint ucs4)\n"
           "{\n"
           "    const UC_Properties *p = GET_PROP(ucs4);\n"
           "    if (p->category == QChar::Letter_Uppercase || p->category == QChar::Letter_Titlecase)\n"
           "        return ucs4 + p->caseDiff;\n"
           "    return ucs4;\n"
           "}\n\n";


    return out;
}


struct DecompositionBlock {
    DecompositionBlock() { index = -1; }
    int index;
    QList<int> decompositionPositions;
    bool operator ==(const DecompositionBlock &other)
        { return decompositionPositions == other.decompositionPositions; }
};

static QByteArray createCompositionInfo()
{
    qDebug("createCompositionInfo:");

    const int BMP_BLOCKSIZE=16;
    const int BMP_SHIFT = 4;
    const int BMP_END = 0x3400; // start of Han
    const int SMP_END = 0x30000;
    const int SMP_BLOCKSIZE = 256;
    const int SMP_SHIFT = 8;

    if(SMP_END <= highestComposedCharacter)
        qFatal("end of table smaller than highest composed character at %x", highestComposedCharacter);

    QList<DecompositionBlock> blocks;
    QList<int> blockMap;
    QList<unsigned short> decompositions;

    int used = 0;
    int tableIndex = 0;

    for (int block = 0; block < BMP_END/BMP_BLOCKSIZE; ++block) {
        DecompositionBlock b;
        for (int i = 0; i < BMP_BLOCKSIZE; ++i) {
            int uc = block*BMP_BLOCKSIZE + i;
            UnicodeData d = unicodeData.value(uc, UnicodeData(uc));
            if (!d.decomposition.isEmpty()) {
                int utf16Chars = 0;
                for (int j = 0; j < d.decomposition.size(); ++j)
                    utf16Chars += d.decomposition.at(j) > 0x10000 ? 2 : 1;
                decompositions.append(d.decompositionType + (utf16Chars<<8));
                for (int j = 0; j < d.decomposition.size(); ++j) {
                    int code = d.decomposition.at(j);
                    if (code > 0x10000) {
                        // save as surrogate pair
                        code -= 0x10000;
                        ushort high = code/0x400 + 0xd800;
                        ushort low = code%0x400 + 0xdc00;
                        decompositions.append(high);
                        decompositions.append(low);
                    } else {
                        decompositions.append(code);
                    }
                }
                b.decompositionPositions.append(tableIndex);
                tableIndex += utf16Chars + 1;
            } else {
                b.decompositionPositions.append(0xffff);
            }
        }
        int index = blocks.indexOf(b);
        if (index == -1) {
            index = blocks.size();
            b.index = used;
            used += BMP_BLOCKSIZE;
            blocks.append(b);
        }
        blockMap.append(blocks.at(index).index);
    }

    int bmp_blocks = blocks.size();
    Q_ASSERT(blockMap.size() == BMP_END/BMP_BLOCKSIZE);

    for (int block = BMP_END/SMP_BLOCKSIZE; block < SMP_END/SMP_BLOCKSIZE; ++block) {
        DecompositionBlock b;
        for (int i = 0; i < SMP_BLOCKSIZE; ++i) {
            int uc = block*SMP_BLOCKSIZE + i;
            UnicodeData d = unicodeData.value(uc, UnicodeData(uc));
            if (!d.decomposition.isEmpty()) {
                int utf16Chars = 0;
                for (int j = 0; j < d.decomposition.size(); ++j)
                    utf16Chars += d.decomposition.at(j) > 0x10000 ? 2 : 1;
                decompositions.append(d.decompositionType + (utf16Chars<<8));
                for (int j = 0; j < d.decomposition.size(); ++j) {
                    int code = d.decomposition.at(j);
                    if (code > 0x10000) {
                        // save as surrogate pair
                        code -= 0x10000;
                        ushort high = code/0x400 + 0xd800;
                        ushort low = code%0x400 + 0xdc00;
                        decompositions.append(high);
                        decompositions.append(low);
                    } else {
                        decompositions.append(code);
                    }
                }
                b.decompositionPositions.append(tableIndex);
                tableIndex += utf16Chars + 1;
            } else {
                b.decompositionPositions.append(0xffff);
            }
        }
        int index = blocks.indexOf(b);
        if (index == -1) {
            index = blocks.size();
            b.index = used;
            used += SMP_BLOCKSIZE;
            blocks.append(b);
        }
        blockMap.append(blocks.at(index).index);
    }

    int bmp_block_data = bmp_blocks*BMP_BLOCKSIZE*2;
    int bmp_trie = BMP_END/BMP_BLOCKSIZE*2;
    int bmp_mem = bmp_block_data + bmp_trie;
    qDebug("    %d unique blocks in BMP.",blocks.size());
    qDebug("        block data uses: %d bytes", bmp_block_data);
    qDebug("        trie data uses : %d bytes", bmp_trie);
    qDebug("        memory usage: %d bytes", bmp_mem);

    int smp_block_data = (blocks.size()- bmp_blocks)*SMP_BLOCKSIZE*2;
    int smp_trie = (SMP_END-BMP_END)/SMP_BLOCKSIZE*2;
    int smp_mem = smp_block_data + smp_trie;
    qDebug("    %d unique blocks in SMP.",blocks.size()-bmp_blocks);
    qDebug("        block data uses: %d bytes", smp_block_data);
    qDebug("        trie data uses : %d bytes", smp_trie);

    qDebug("\n        decomposition table use : %d bytes", decompositions.size()*2);
    qDebug("    memory usage: %d bytes", bmp_mem+smp_mem + decompositions.size()*2);

    QByteArray out;

    out += "static const unsigned short uc_decomposition_trie[] = {\n";

    // first write the map
    out += "    // 0 - 0x" + QByteArray::number(BMP_END, 16);
    for (int i = 0; i < BMP_END/BMP_BLOCKSIZE; ++i) {
        if (!(i % 8)) {
            if (!((i*BMP_BLOCKSIZE) % 0x1000))
                out += "\n";
            out += "\n    ";
        }
        out += QByteArray::number(blockMap.at(i) + blockMap.size());
        out += ", ";
    }
    out += "\n\n    // 0x" + QByteArray::number(BMP_END, 16) + " - 0x" + QByteArray::number(SMP_END, 16) + "\n";;
    for (int i = BMP_END/BMP_BLOCKSIZE; i < blockMap.size(); ++i) {
        if (!(i % 8)) {
            if (!(i % (0x10000/SMP_BLOCKSIZE)))
                out += "\n";
            out += "\n    ";
        }
        out += QByteArray::number(blockMap.at(i) + blockMap.size());
        out += ", ";
    }
    out += "\n";
    // write the data
    for (int i = 0; i < blocks.size(); ++i) {
        out += "\n";
        const DecompositionBlock &b = blocks.at(i);
        for (int j = 0; j < b.decompositionPositions.size(); ++j) {
            if (!(j % 8))
                out += "\n    ";
            out += "0x" + QByteArray::number(b.decompositionPositions.at(j), 16);
            out += ", ";
        }
    }

    out += "\n};\n\n"

           "#define GET_DECOMPOSITION_INDEX(ucs4) \\\n"
           "       (ucs4 < 0x" + QByteArray::number(BMP_END, 16) + " \\\n"
           "        ? (uc_decomposition_trie[uc_decomposition_trie[ucs4>>" + QByteArray::number(BMP_SHIFT) +
           "] + (ucs4 & 0x" + QByteArray::number(BMP_BLOCKSIZE-1, 16)+ ")]) \\\n"
           "        : (ucs4 < 0x" + QByteArray::number(SMP_END, 16) + "\\\n"
           "           ? uc_decomposition_trie[uc_decomposition_trie[((ucs4 - 0x" + QByteArray::number(BMP_END, 16) +
           ")>>" + QByteArray::number(SMP_SHIFT) + ") + 0x" + QByteArray::number(BMP_END/BMP_BLOCKSIZE, 16) + "]"
           " + (ucs4 & 0x" + QByteArray::number(SMP_BLOCKSIZE-1, 16) + ")]\\\n"
           "           : 0xffff))\n\n"

           "static const unsigned short uc_decomposition_map[] = {\n";

    for (int i = 0; i < decompositions.size(); ++i) {
        if (!(i % 8)) {
            out += "\n    ";
        }
        out += "0x" + QByteArray::number(decompositions.at(i), 16);
        out += ", ";
    }

    out += "\n};\n\n"

           "QString QUnicodeTables::decomposition(uint ucs4)\n"
           "{\n"
           "    const unsigned short index = GET_DECOMPOSITION_INDEX(ucs4);\n"
           "    if (index == 0xffff)\n"
           "        return QString();\n"
           "    const unsigned short *decomposition = uc_decomposition_map+index;\n"
           "    uint length = (*decomposition) >> 8;\n"
           "    QString str;\n"
           "    str.resize(length);\n"
           "    QChar *c = str.data();\n"
           "    for (uint i = 0; i < length; ++i)\n"
           "        *(c++) = *(++decomposition);\n"
           "    return str;\n"
           "}\n\n"

           "QChar::Decomposition QUnicodeTables::decompositionTag(uint ucs4)\n"
           "{\n"
           "    const unsigned short index = GET_DECOMPOSITION_INDEX(ucs4);\n"
           "    if (index == 0xffff)\n"
           "        return QChar::NoDecomposition;\n"
           "    return (QChar::Decomposition)(uc_decomposition_map[index] & 0xff);\n"
           "}\n\n";

    return out;
}

static QByteArray createLigatureInfo()
{
    qDebug("createCompositionInfo:");

    QList<DecompositionBlock> blocks;
    QList<int> blockMap;
    QList<unsigned short> ligatures;

    const int BMP_BLOCKSIZE = 32;
    const int BMP_SHIFT = 5;
    const int BMP_END = 0x3100;
    Q_ASSERT(highestLigature < BMP_END);

    int used = 0;
    int tableIndex = 0;

    for (int block = 0; block < BMP_END/BMP_BLOCKSIZE; ++block) {
        DecompositionBlock b;
        for (int i = 0; i < BMP_BLOCKSIZE; ++i) {
            int uc = block*BMP_BLOCKSIZE + i;
            QList<Ligature> l = ligatureHashes.value(uc);
            if (!l.isEmpty()) {
                b.decompositionPositions.append(tableIndex);
                qBubbleSort(l);

                ligatures.append(l.size());
                for (int i = 0; i < l.size(); ++i) {
                    ligatures.append(l.at(i).u1);
                    ligatures.append(l.at(i).ligature);
                }
                tableIndex += 2*l.size() + 1;
            } else {
                b.decompositionPositions.append(0xffff);
            }
        }
        int index = blocks.indexOf(b);
        if (index == -1) {
            index = blocks.size();
            b.index = used;
            used += BMP_BLOCKSIZE;
            blocks.append(b);
        }
        blockMap.append(blocks.at(index).index);
    }

    int bmp_blocks = blocks.size();
    Q_ASSERT(blockMap.size() == BMP_END/BMP_BLOCKSIZE);

    int bmp_block_data = bmp_blocks*BMP_BLOCKSIZE*2;
    int bmp_trie = BMP_END/BMP_BLOCKSIZE*2;
    int bmp_mem = bmp_block_data + bmp_trie;
    qDebug("    %d unique blocks in BMP.",blocks.size());
    qDebug("        block data uses: %d bytes", bmp_block_data);
    qDebug("        trie data uses : %d bytes", bmp_trie);
    qDebug("        memory usage: %d bytes", bmp_mem);

    QByteArray out;


    out += "static const unsigned short uc_ligature_trie[] = {\n";

    // first write the map
    out += "    // 0 - 0x" + QByteArray::number(BMP_END, 16);
    for (int i = 0; i < BMP_END/BMP_BLOCKSIZE; ++i) {
        if (!(i % 8)) {
            if (!((i*BMP_BLOCKSIZE) % 0x1000))
                out += "\n";
            out += "\n    ";
        }
        out += QByteArray::number(blockMap.at(i) + blockMap.size());
        out += ", ";
    }
    out += "\n";
    // write the data
    for (int i = 0; i < blocks.size(); ++i) {
        out += "\n";
        const DecompositionBlock &b = blocks.at(i);
        for (int j = 0; j < b.decompositionPositions.size(); ++j) {
            if (!(j % 8))
                out += "\n    ";
            out += "0x" + QByteArray::number(b.decompositionPositions.at(j), 16);
            out += ", ";
        }
    }
    out += "\n};\n\n"

           "#define GET_LIGATURE_INDEX(u2) "
           "(u2 < 0x" + QByteArray::number(BMP_END, 16) + " ? "
           "uc_ligature_trie[uc_ligature_trie[u2>>" + QByteArray::number(BMP_SHIFT) +
           "] + (u2 & 0x" + QByteArray::number(BMP_BLOCKSIZE-1, 16)+ ")] : 0xffff);\n\n"

           "static const unsigned short uc_ligature_map [] = {\n";

    for (int i = 0; i < ligatures.size(); ++i) {
        if (!(i % 8)) {
            out += "\n    ";
        }
        out += "0x" + QByteArray::number(ligatures.at(i), 16);
        out += ", ";
    }

    out += "\n};\n\n"

           "ushort QUnicodeTables::ligature(ushort u1, ushort u2)\n"
           "{\n"
           "    const unsigned short index = GET_LIGATURE_INDEX(u2);\n"
           "    if (index == 0xffff)\n"
           "        return 0;\n"
           "    const unsigned short *ligatures = uc_ligature_map+index;\n"
           "    ushort length = *ligatures;\n"
           "    ++ligatures;\n"
           "    // ### use bsearch\n"
           "    for (uint i = 0; i < length; ++i)\n"
           "        if (ligatures[2*i] == u1)\n"
           "            return ligatures[2*i+1];\n"
           "    return 0;\n"
           "}\n\n";

    return out;
}

int main(int, char **)
{
    initCategoryMap();
    initDirectionMap();
    initDecompositionMap();

    readUnicodeData();
    readBidiMirroring();
    readArabicShaping();
    readDerivedAge();
    readCompositionExclusion();
    readLineBreak();

    computeUniqueProperties();
    QByteArray properties = createPropertyInfo();
    QByteArray compositions = createCompositionInfo();
    QByteArray ligatures = createLigatureInfo();

    QFile f("qunicodetables.cpp");
    f.open(IO_WriteOnly|IO_Truncate);

    QByteArray header =
        "/****************************************************************************\n"
        "**\n"
        "** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.\n"
        "**\n"
        "** This file is part of the kernel module of the Qt GUI Toolkit.\n"
        "** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE\n"
        "**\n"
        "** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE\n"
        "** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.\n"
        "**\n"
        "****************************************************************************/\n\n"

        "/* This file is autogenerated from the Unicode 4.0.1 database. Do not edit */\n\n"

        "#include \"qunicodetables_p.h\"\n\n";

    f.writeBlock(header.data(), header.length());
    f.writeBlock(properties.data(), properties.length());
    f.writeBlock(compositions.data(), compositions.length());
    f.writeBlock(ligatures.data(), ligatures.size());

    qDebug("maxMirroredDiff = %x, maxCaseDiff = %x", maxMirroredDiff, maxCaseDiff);
#if 0
//     dump(0, 0x7f);
//     dump(0x620, 0x640);
//     dump(0x10000, 0x10020);
//     dump(0x10800, 0x10820);

    qDebug("decompositionLength used:");
    int totalcompositions = 0;
    int sum = 0;
    for (int i = 1; i < 20; ++i) {
        qDebug("    length %d used %d times", i, decompositionLength.value(i, 0));
        totalcompositions += i*decompositionLength.value(i, 0);
        sum += decompositionLength.value(i, 0);
    }
    qDebug("    len decomposition map %d, average length %f, num composed chars %d",
           totalcompositions, (float)totalcompositions/(float)sum,  sum);
    qDebug("highest composed character %x", highestComposedCharacter);
    qDebug("num ligatures = %d highest=%x, maxLength=%d", numLigatures, highestLigature, longestLigature);

    qBubbleSort(ligatures);
    for (int i = 0; i < ligatures.size(); ++i)
        qDebug("%s", ligatures.at(i).data());

//     qDebug("combiningClass usage:");
//     int numClasses = 0;
//     for (int i = 0; i < 255; ++i) {
//         int num = combiningClassUsage.value(i, 0);
//         if (num) {
//             ++numClasses;
//             qDebug("    combiningClass %d used %d times", i, num);
//         }
//     }
//     qDebug("total of %d combining classes used", numClasses);

#endif
}

