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

#ifdef Q_CC_GNU
#define FASTCALL __attribute__((regparm(3)))
#else
#define FASTCALL
#endif

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

    Q_CORE_EXPORT QChar::Category category(uint ucs4) FASTCALL;
    Q_CORE_EXPORT unsigned char combiningClass(uint ucs4) FASTCALL;
    Q_CORE_EXPORT QChar::Direction direction(uint ucs4) FASTCALL;
    Q_CORE_EXPORT QChar::UnicodeVersion unicodeVersion(uint ucs4) FASTCALL;
    Q_CORE_EXPORT QChar::Joining joining(uint ucs4) FASTCALL;
    Q_CORE_EXPORT bool mirrored(uint ucs4) FASTCALL;
    Q_CORE_EXPORT int mirroredChar(uint ucs4) FASTCALL;
    Q_CORE_EXPORT int upper(uint ucs4) FASTCALL;
    Q_CORE_EXPORT int lower(uint ucs4) FASTCALL;
    Q_CORE_EXPORT int QUnicodeTables::digitValue(uint ucs4) FASTCALL;
    Q_CORE_EXPORT QString decomposition(uint ucs4) FASTCALL;
    Q_CORE_EXPORT QChar::Decomposition decompositionTag(uint ucs4) FASTCALL;
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

Q_CORE_EXPORT int lineBreakClass(const QChar &ch);

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
