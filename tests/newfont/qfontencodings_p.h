#include <qfeatures.h>
#include <qtextcodec.h>

#ifndef QT_NO_CODECS

#include <qjpunicode.h>

class QFontJis0208Codec : public QTextCodec
{
public:
    QFontJis0208Codec();

    const char* name() const ;
    //       Return the official name for the encoding.
    int mibEnum() const ;
    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.

    QString toUnicode(const char* chars, int len) const ;
    // Converts len characters from chars to Unicode.
    QCString fromUnicode(const QString& uc, int& lenInOut ) const;
    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QCString result, and also returning
    // the length of the result in lenInOut.

    int heuristicContentMatch(const char *, int) const;
private:
    static const QJpUnicodeConv * convJP;
};


int QFontJis0208Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}

const QJpUnicodeConv * QFontJis0208Codec::convJP;

QFontJis0208Codec::QFontJis0208Codec()
{
    if ( !convJP )
	convJP = QJpUnicodeConv::newConverter(JU_Default);
}

const char* QFontJis0208Codec::name() const
{
    return "JIS_X0208";
}

int QFontJis0208Codec::mibEnum() const
{
    return 63;
}

QString QFontJis0208Codec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return QString(); //###
}

QCString QFontJis0208Codec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QCString result;
    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch = uc[i];
	if ( ch.row() == 0) {
	    if ( ch.cell() == ' ' )
		ch = QChar( 0x3000 );
	    else if ( ch.cell() == '"' )
		ch = QChar( 0x2033 );
	    else if ( ch.cell() == '\'' )
		ch = QChar( 0x2032 );
	    else if ( ch.cell() == '-' )
		ch = QChar( 0x2212 );
	    else if ( ch.cell() == '~' )
		ch = QChar( 0x301c );
	    else if ( ch.cell() > ' ' && ch.cell() < 127 )
		ch = QChar( ch.cell()-' ', 255 );
	}
	ch = convJP->UnicodeToJisx0208( ch.unicode());
	if ( !ch.isNull() ) {
	    result += ch.row();
	    result += ch.cell();
	} else {
	    //black square
	    result += 0x22;
	    result += 0x23;
	}
    }
    lenInOut *=2;
    return result;
}



extern unsigned int qt_UnicodeToKsc5601(unsigned int unicode);


class QFontKsc5601Codec : public QTextCodec
{
public:
    QFontKsc5601Codec();

    const char* name() const ;
    //       Return the official name for the encoding.
    int mibEnum() const ;
    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.

    QString toUnicode(const char* chars, int len) const ;
    // Converts len characters from chars to Unicode.
    QCString fromUnicode(const QString& uc, int& lenInOut ) const;
    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QCString result, and also returning
    // the length of the result in lenInOut.

    int heuristicContentMatch(const char *, int) const;
};


int QFontKsc5601Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}

QFontKsc5601Codec::QFontKsc5601Codec()
{
}

const char* QFontKsc5601Codec::name() const
{
    return "KSC_5601";
}

int QFontKsc5601Codec::mibEnum() const
{
    return 63;
}

QString QFontKsc5601Codec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return QString(); //###
}

QCString QFontKsc5601Codec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QCString result;
    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch = uc[i];
	if ( ch.row() == 0) {
	    if ( ch.cell() == ' ' )
		ch = QChar( 0x3000 );
	    else if ( ch.cell() > ' ' && ch.cell() < 127 )
		ch = QChar( ch.cell()-' ', 255 );
	}
	ch = QChar( qt_UnicodeToKsc5601(ch.unicode()) );

	if ( ch.row() > 0 && ch.cell() > 0  ) {
	    result += ch.row() & 0x7f ;
	    result += ch.cell() & 0x7f;
	} else {
	    //black square
	    result += 0x21;
	    result += 0x61;
	}
    }
    lenInOut *=2;
    return result;
}


/********


Name: GB_2312-80                                        [RFC1345,KXS2]
MIBenum: 57
Source: ECMA registry
Alias: iso-ir-58
Alias: chinese
Alias: csISO58GB231280

*/


extern unsigned int qt_UnicodeToGBK(unsigned int code);



class QFontGB2312Codec : public QTextCodec
{
public:
    QFontGB2312Codec();

    const char* name() const ;
    //       Return the official name for the encoding.
    int mibEnum() const ;
    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.

    QString toUnicode(const char* chars, int len) const ;
    // Converts len characters from chars to Unicode.
    QCString fromUnicode(const QString& uc, int& lenInOut ) const;
    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QCString result, and also returning
    // the length of the result in lenInOut.

    int heuristicContentMatch(const char *, int) const;
};


int QFontGB2312Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}

QFontGB2312Codec::QFontGB2312Codec()
{
}

const char* QFontGB2312Codec::name() const
{
    return "GB_2312";
}

int QFontGB2312Codec::mibEnum() const
{
    return 57;
}

QString QFontGB2312Codec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return QString(); //###
}

QCString QFontGB2312Codec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QCString result;
    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch = uc[i];
	if ( ch.row() == 0) {
	    if ( ch.cell() == ' ' )
		ch = QChar( 0x3000 );
	    else if ( ch.cell() > ' ' && ch.cell() < 127 )
		ch = QChar( ch.cell()-' ', 255 );
	}
	ch = QChar( qt_UnicodeToGBK(ch.unicode()) );

	if ( ch.row() > 0xa0 && ch.cell() > 0xa0  ) {
	    result += ch.row() & 0x7f ;
	    result += ch.cell() & 0x7f;
	} else {
	    //black square
	    result += 0x21;
	    result += 0x76;
	}
    }
    lenInOut *=2;
    return result;
}



extern unsigned int qt_UnicodeToBig5(unsigned int unicode);

class QFontBig5Codec : public QTextCodec
{
public:
    QFontBig5Codec();

    const char* name() const ;
    //       Return the official name for the encoding.
    int mibEnum() const ;
    // Return the MIB enum for the encoding if it is listed in the
    // IANA character-sets encoding file.

    QString toUnicode(const char* chars, int len) const ;
    // Converts len characters from chars to Unicode.
    QCString fromUnicode(const QString& uc, int& lenInOut ) const;
    // Converts lenInOut characters (of type QChar) from the start of
    // the string uc, returning a QCString result, and also returning
    // the length of the result in lenInOut.

    int heuristicContentMatch(const char *, int) const;
};


int QFontBig5Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}

QFontBig5Codec::QFontBig5Codec()
{
}

const char* QFontBig5Codec::name() const
{
    return "Big5";
}

int QFontBig5Codec::mibEnum() const
{
    return -1;
}

QString QFontBig5Codec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return QString(); //###
}

QCString QFontBig5Codec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QCString result;
    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch = uc[i];
	if ( ch.row() == 0) {
	    if ( ch.cell() == ' ' )
		ch = QChar( 0x3000 );
	    else if ( ch.cell() > ' ' && ch.cell() < 127 )
		ch = QChar( ch.cell()-' ', 255 );
	}
	ch = QChar( qt_UnicodeToBig5(ch.unicode()) );

	if ( ch.row() > 0xa0 && ch.cell() >= 0x40  ) {
	    result += ch.row();
	    result += ch.cell();
	} else {
	    //black square
	    result += 0xa1;
	    result += 0xbd;
	}
    }
    lenInOut *=2;
    return result;
}


#endif //QT_NO_CODECS
