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

#ifndef QLOCALE_P_H
#define QLOCALE_P_H

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

#include <QString>
#include <QVarLengthArray>

struct QLocalePrivate
{
public:
    const QChar &decimal() const { return reinterpret_cast<const QChar&>(m_decimal); }
    const QChar &group() const { return reinterpret_cast<const QChar&>(m_group); }
    const QChar &list() const { return reinterpret_cast<const QChar&>(m_list); }
    const QChar &percent() const { return reinterpret_cast<const QChar&>(m_percent); }
    const QChar &zero() const { return reinterpret_cast<const QChar&>(m_zero); }
    QChar plus() const { return QLatin1Char('+'); }
    const QChar &minus() const { return reinterpret_cast<const QChar&>(m_minus); }
    const QChar &exponential() const { return reinterpret_cast<const QChar&>(m_exponential); }
    QString infinity() const;
    QString nan() const;

    Q_UINT32 languageId() const { return m_language_id; }
    Q_UINT32 countryId() const { return m_country_id; }

    enum DoubleForm {
        DFExponent = 0,
        DFDecimal,
        DFSignificantDigits,
        _DFMax = DFSignificantDigits
    };

    enum Flags {
        NoFlags             = 0,
        Alternate           = 0x01,
        ZeroPadded          = 0x02,
        LeftAdjusted        = 0x04,
        BlankBeforePositive = 0x08,
        AlwaysShowSign      = 0x10,
        ThousandsGroup      = 0x20,
        CapitalEorX         = 0x40
    };

    enum GroupSeparatorMode {
        FailOnGroupSeparators,
        ParseGroupSeparators
    };

    QString doubleToString(double d,
                           int precision = -1,
                           DoubleForm form = DFSignificantDigits,
                           int width = -1,
                           unsigned flags = NoFlags) const;
    QString longLongToString(Q_LONGLONG l, int precision = -1,
                             int base = 10,
                             int width = -1,
                             unsigned flags = NoFlags) const;
    QString unsLongLongToString(Q_ULONGLONG l, int precision = -1,
                                int base = 10,
                                int width = -1,
                                unsigned flags = NoFlags) const;
    double stringToDouble(const QString &num, bool *ok, GroupSeparatorMode group_sep_mode) const;
    Q_LONGLONG stringToLongLong(const QString &num, int base, bool *ok, GroupSeparatorMode group_sep_mode) const;
    Q_ULONGLONG stringToUnsLongLong(const QString &num, int base, bool *ok, GroupSeparatorMode group_sep_mode) const;


    static double bytearrayToDouble(const char *num, bool *ok);
    static Q_LONGLONG bytearrayToLongLong(const char *num, int base, bool *ok);
    static Q_ULONGLONG bytearrayToUnsLongLong(const char *num, int base, bool *ok);

    typedef QVarLengthArray<char, 256> CharBuff;
    bool numberToCLocale(const QString &num,
    	    	    	  GroupSeparatorMode group_sep_mode,
                          CharBuff *result) const;
                          
    Q_UINT32 m_language_id, m_country_id;

    Q_UINT16 m_decimal, m_group, m_list, m_percent,
        m_zero, m_minus, m_exponential;

    static const QString m_infinity;
    static const QString m_nan;
    static const QChar m_plus;

    static const char *systemLocaleName();
};

#endif
