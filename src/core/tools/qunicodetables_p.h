/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QUNICODETABLES_P_H
#define QUNICODETABLES_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of internal files.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

namespace QUnicodeTables {

    // see http://www.unicode.org/reports/tr14/tr14-13.html
    // we don't use the XX and AI properties and map them to AL instead.
    enum LineBreakClass {
        LineBreak_OP, LineBreak_CL, LineBreak_QU, LineBreak_GL, LineBreak_NS,
        LineBreak_EX, LineBreak_SY, LineBreak_IS, LineBreak_PR, LineBreak_PO,
        LineBreak_NU, LineBreak_AL, LineBreak_ID, LineBreak_IN, LineBreak_HY,
        LineBreak_BA, LineBreak_BB, LineBreak_B2, LineBreak_ZW, LineBreak_CM,
        LineBreak_SA, LineBreak_BK, LineBreak_CR, LineBreak_LF, LineBreak_SG,
        LineBreak_CB, LineBreak_SP
    };

    inline uint surrogateToUcs4(ushort high, ushort low) {
        return (high<<10) + low - 0x35fdc00;
    }
    inline ushort highSurrogate(uint ucs4) {
        return (ucs4>>10) + 0xd7c0;
    }
    inline ushort lowSurrogate(uint ucs4) {
        return ucs4%0x400 + 0xdc00;
    }
    inline bool isHighSurrogate(uint utf16) {
        return (utf16 >= 0xd800 && utf16 < 0xdc00);
    }
    inline bool isLowSurrogate(uint utf16) {
        return (utf16 >= 0xdc00 && utf16 < 0xe000);
    }


    Q_CORE_EXPORT QChar::Category  QT_FASTCALL category(uint ucs4);
    Q_CORE_EXPORT unsigned char  QT_FASTCALL combiningClass(uint ucs4);
    Q_CORE_EXPORT QChar::Direction QT_FASTCALL direction(uint ucs4);
    Q_CORE_EXPORT QChar::UnicodeVersion QT_FASTCALL unicodeVersion(uint ucs4);
    Q_CORE_EXPORT QChar::Joining QT_FASTCALL joining(uint ucs4);
    Q_CORE_EXPORT bool QT_FASTCALL mirrored(uint ucs4);
    Q_CORE_EXPORT int QT_FASTCALL mirroredChar(uint ucs4);
    Q_CORE_EXPORT LineBreakClass QT_FASTCALL lineBreakClass(uint ucs4);
    Q_CORE_EXPORT int QT_FASTCALL upper(uint ucs4);
    Q_CORE_EXPORT int QT_FASTCALL lower(uint ucs4);
    Q_CORE_EXPORT int QT_FASTCALL digitValue(uint ucs4);
    Q_CORE_EXPORT QString QT_FASTCALL decomposition(uint ucs4);
    Q_CORE_EXPORT QChar::Decomposition QT_FASTCALL decompositionTag(uint ucs4);
    Q_CORE_EXPORT ushort QT_FASTCALL QUnicodeTables::ligature(ushort u1, ushort u2);

    enum NormalizationMode {
        NormalizationMode_D,
        NormalizationMode_C,
        NormalizationMode_KD,
        NormalizationMode_KC
    };
    QString normalize(const QString &str, QUnicodeTables::NormalizationMode mode);
    QString normalize(const QString &str, QUnicodeTables::NormalizationMode mode, QChar::UnicodeVersion version);
};


inline QChar::Category category(const QChar &c)
{
    return QUnicodeTables::category(c.unicode());
}

inline QChar lower(const QChar &c)
{
    return QChar(QUnicodeTables::lower(c.unicode()));
}

inline QChar upper(const QChar &c)
{
    return QChar(QUnicodeTables::upper(c.unicode()));
}

inline QChar::Direction direction(const QChar &c)
{
    return QUnicodeTables::direction(c.unicode());
}

inline bool mirrored(const QChar &c)
{
    return QUnicodeTables::mirrored(c.unicode());
}


inline QChar mirroredChar(const QChar &c)
{
    return QUnicodeTables::mirroredChar(c.unicode());
}

inline QChar::Joining joining(const QChar &c)
{
    return QUnicodeTables::joining(c.unicode());
}

inline bool isMark(const QChar &ch)
{
    QChar::Category c = QUnicodeTables::category(ch.unicode());
    return c >= QChar::Mark_NonSpacing && c <= QChar::Mark_Enclosing;
}

inline unsigned char combiningClass(const QChar &ch)
{
    return QUnicodeTables::combiningClass(ch.unicode());
}

inline bool isSpace(const QChar &ch)
{
    if(ch.unicode() >= 9 && ch.unicode() <=13) return true;
    QChar::Category c = QUnicodeTables::category(ch.unicode());
    return c >= QChar::Separator_Space && c <= QChar::Separator_Paragraph;
}

inline int lineBreakClass(const QChar &ch)
{
    return QUnicodeTables::lineBreakClass(ch.unicode());
}

Q_CORE_EXPORT int scriptForChar(ushort uc);

#ifdef Q_WS_X11
#define SCRIPT_FOR_CHAR(script, c)         \
do {                                                 \
    unsigned short _uc = (c).unicode();                 \
    if (_uc < 0x100) {                                \
        script = QFont::Latin;                \
    } else {                                         \
        script = (QFont::Script)scriptForChar(_uc);         \
    }                                                 \
} while(false)
#else
#define SCRIPT_FOR_CHAR(script, c) \
    script = (QFont::Script)scriptForChar((c).unicode())
#endif

#endif
