#ifndef QLOCALE_P_H
#define QLOCALE_P_H

#include <qstring.h>

struct QLocalePrivate
{
    public:
    	const QChar &decimal() const { return (QChar&)m_decimal; }
	const QChar &group() const { return (QChar&)m_group; }
	const QChar &list() const { return (QChar&)m_list; }
	const QChar &percent() const { return (QChar&)m_percent; }
	const QChar &zero() const { return (QChar&)m_zero; }
	const QChar plus() const { return QChar('+'); }
	const QChar &minus() const { return (QChar&)m_minus; }
	const QChar &exponential() const { return (QChar&)m_exponential; }
	const QString &infinity() const;
	const QString &nan() const;
	
	Q_UINT32 languageId() const { return m_language_id; }
	Q_UINT32 countryId() const { return m_country_id; }

	bool isDigit(QChar d) const;

	enum DoubleForm {
	    DFExponent = 0,
	    DFDecimal,
	    DFSignificantDigits,
	    _DFMax = DFSignificantDigits
	};

	enum Flags { 
    	    NoFlags	    	= 0,
	    Alternate           = 0x01, 
	    ZeroPadded          = 0x02,
	    LeftAdjusted        = 0x04,
	    BlankBeforePositive = 0x08,
	    AlwaysShowSign      = 0x10,
	    ThousandsGroup      = 0x20,
	    CapitalEorX	    	= 0x40
	};
	
	QString doubleToString(double d,
    	 		 int precision = -1,
	 		 DoubleForm form = DFSignificantDigits,
	 		 int width = -1,
	 		 unsigned flags = NoFlags) const;
    	QString longLongToString(Q_LLONG l, int precision = -1,
	 		 int base = 10,
	 		 int width = -1,
	 		 unsigned flags = NoFlags) const;
    	QString unsLongLongToString(Q_ULLONG l, int precision = -1,
	 		 int base = 10,
	 		 int width = -1,
	 		 unsigned flags = NoFlags) const;
    	double stringToDouble(QString num, bool *ok) const;
	Q_LLONG stringToLongLong(QString num, int base, bool *ok) const;
	Q_ULLONG stringToUnsLongLong(QString num, int base, bool *ok) const;
				    
    	QString &numberToCLocale(QString &locale_num) const;

    	Q_UINT32 m_language_id, m_country_id;
    
    	Q_UINT16 m_decimal, m_group, m_list, m_percent,
	    	m_zero, m_minus, m_exponential;
	
	static const QString m_infinity;
	static const QString m_nan;
	static const QChar m_plus;
};

#endif
