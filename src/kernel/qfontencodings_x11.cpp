#include "qfontencodings_p.h"

#ifndef QT_NO_CODECS

#include <qjpunicode.h>


int QFontJis0208Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}

const QJpUnicodeConv * QFontJis0208Codec::convJP;

QFontJis0208Codec::QFontJis0208Codec()
{
    if ( !convJP )
	convJP = QJpUnicodeConv::newConverter(QJpUnicodeConv::Default);
}

const char* QFontJis0208Codec::name() const
{
    return "jisx0208.1983-0";
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
    QCString result(lenInOut * 2 + 1);
    uchar *rdata = (uchar *) result.data();
    const QChar *ucp = uc.unicode();

    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch(*ucp++);

#if 0
	// ### these fonts usually don't seem to have 0x3000 and the other ones
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
#endif
	
	ch = convJP->unicodeToJisx0208(ch.unicode());

	if ( ! ch.isNull() ) {
	    *rdata++ = ch.row();
	    *rdata++ = ch.cell();
	} else {
	    //white square
	    *rdata++ = 0x22;
	    *rdata++ = 0x22;
	}
    }
    
    lenInOut *= 2;

    return result;
}

bool QFontJis0208Codec::canEncode( QChar ch ) const
{
    return ( convJP->unicodeToJisx0208(ch.unicode()) != 0 );
}
// ----------------------------------------------------------

extern unsigned int qt_UnicodeToKsc5601(unsigned int unicode);

int QFontKsc5601Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}

QFontKsc5601Codec::QFontKsc5601Codec()
{
}

const char* QFontKsc5601Codec::name() const
{
    return "ksc5601.1987-0";
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
    QCString result(lenInOut * 2 + 1);
    uchar *rdata = (uchar *) result.data();
    const QChar *ucp = uc.unicode();
    
    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch(*ucp++);
	
#if 0
	if ( ch.row() == 0) {
	    if ( ch.cell() == ' ' )
		ch = QChar( 0x3000 );
	    else if ( ch.cell() > ' ' && ch.cell() < 127 )
		ch = QChar( ch.cell()-' ', 255 );
	}
#endif
	
	ch = qt_UnicodeToKsc5601(ch.unicode());

	if ( ! ch.isNull() ) {
	    *rdata++ = ch.row() & 0x7f ;
	    *rdata++ = ch.cell() & 0x7f;
	} else {
	    //white square
	    *rdata++ = 0x21;
	    *rdata++ = 0x60;
	}
    }
    
    lenInOut *= 2;
    
    return result;
}

bool QFontKsc5601Codec::canEncode( QChar ch ) const
{
    return (qt_UnicodeToKsc5601(ch) != 0);
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


int QFontGB2312Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}

QFontGB2312Codec::QFontGB2312Codec()
{
}

const char* QFontGB2312Codec::name() const
{
    return "gb2312.1980-0";
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
    QCString result(lenInOut * 2 + 1);
    uchar *rdata = (uchar *) result.data();
    const QChar *ucp = uc.unicode();
    
    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch(*ucp++);
	
#if 0
	if ( ch.row() == 0) {
	    if ( ch.cell() == ' ' )
		ch = QChar( 0x3000 );
	    else if ( ch.cell() > ' ' && ch.cell() < 127 )
		ch = QChar( ch.cell()-' ', 255 );
	}
#endif
	
	ch = qt_UnicodeToGBK(ch.unicode());
	
	if ( ch.row() > 0xa0 && ch.cell() > 0xa0  ) {
	    *rdata++ = ch.row() & 0x7f ;
	    *rdata++ = ch.cell() & 0x7f;
	} else {
	    //white square
	    *rdata++ = 0x21;
	    *rdata++ = 0x75;
	}
    }
    
    lenInOut *= 2;
    
    return result;
}

bool QFontGB2312Codec::canEncode( QChar ch ) const
{
    ch = qt_UnicodeToGBK( ch.unicode() );
    return ( ch.row() > 0xa0 && ch.cell() > 0xa0 );
}

// ----------------------------------------------------------------

extern unsigned int qt_UnicodeToBig5(unsigned int unicode);

int QFontBig5Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}

QFontBig5Codec::QFontBig5Codec()
{
}

const char* QFontBig5Codec::name() const
{
    return "big5-0";
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
    QCString result(lenInOut * 2 + 1);
    uchar *rdata = (uchar *) result.data();
    const QChar *ucp = uc.unicode();
    
    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch(*ucp++);
	
#if 0
	if ( ch.row() == 0) {
	    if ( ch.cell() == ' ' )
		ch = QChar( 0x3000 );
	    else if ( ch.cell() > ' ' && ch.cell() < 127 )
		ch = QChar( ch.cell()-' ', 255 );
	}
#endif
	ch = QChar( qt_UnicodeToBig5(ch.unicode()) );

	if ( ch.row() > 0xa0 && ch.cell() >= 0x40  ) {
	    *rdata++ = ch.row();
	    *rdata++ = ch.cell();
	} else {
	    //white square
	    *rdata++ = 0xa1;
	    *rdata++ = 0xbc;
	}
    }
    lenInOut *=2;
    return result;
}

bool QFontBig5Codec::canEncode( QChar ch ) const
{
    ch = qt_UnicodeToBig5( ch.unicode() );
    return ( ch.row() > 0xa0 && ch.cell() > 0xa0 );
}

// ---------------------------------------------------------------
#if 0
// this table covers basic arabic letter, not the extensions used in various other languages.
static const uchar arabic68Mapping[112][4] = {
    // Isolated, left, right, medial forms or:
    // Isolated, Initial, Final, Medial
    { 0xff, 0xff, 0xff, 0xff }, // 0x600
    { 0xff, 0xff, 0xff, 0xff }, // 0x601
    { 0xff, 0xff, 0xff, 0xff }, // 0x602
    { 0xff, 0xff, 0xff, 0xff }, // 0x603
    { 0xff, 0xff, 0xff, 0xff }, // 0x604
    { 0xff, 0xff, 0xff, 0xff }, // 0x605
    { 0xff, 0xff, 0xff, 0xff }, // 0x606
    { 0xff, 0xff, 0xff, 0xff }, // 0x607
    { 0xff, 0xff, 0xff, 0xff }, // 0x608
    { 0xff, 0xff, 0xff, 0xff }, // 0x609
    { 0xff, 0xff, 0xff, 0xff }, // 0x60a
    { 0xff, 0xff, 0xff, 0xff }, // 0x60b
    { 0xac, 0xff, 0xff, 0xff }, // 0x60c 		Arabic comma
    { 0xff, 0xff, 0xff, 0xff }, // 0x60d
    { 0xff, 0xff, 0xff, 0xff }, // 0x60e
    { 0xff, 0xff, 0xff, 0xff }, // 0x60f

    { 0xff, 0xff, 0xff, 0xff }, // 0x610
    { 0xff, 0xff, 0xff, 0xff }, // 0x611
    { 0xff, 0xff, 0xff, 0xff }, // 0x612
    { 0xff, 0xff, 0xff, 0xff }, // 0x613
    { 0xff, 0xff, 0xff, 0xff }, // 0x614
    { 0xff, 0xff, 0xff, 0xff }, // 0x615
    { 0xff, 0xff, 0xff, 0xff }, // 0x616
    { 0xff, 0xff, 0xff, 0xff }, // 0x617
    { 0xff, 0xff, 0xff, 0xff }, // 0x618
    { 0xff, 0xff, 0xff, 0xff }, // 0x619
    { 0xff, 0xff, 0xff, 0xff }, // 0x61a
    { 0xbb, 0xff, 0xff, 0xff }, // 0x61b  		Arabic semicolon
    { 0xff, 0xff, 0xff, 0xff }, // 0x61c
    { 0xff, 0xff, 0xff, 0xff }, // 0x61d
    { 0xff, 0xff, 0xff, 0xff }, // 0x61e
    { 0xbf, 0xff, 0xff, 0xff }, // 0x61f  		Arabic question mark

    { 0xff, 0xff, 0xff, 0xff }, // 0x620
    { 0xc1, 0xff, 0xff, 0xff }, // 0x621 	 	Hamza
    { 0xc2, 0xff, 0xc2, 0xff }, // 0x622 	R 	Alef with Madda above
    { 0xc3, 0xff, 0xc3, 0xff }, // 0x623 	R	Alef with Hamza above
    { 0xc4, 0xff, 0xc4, 0xff }, // 0x624 	R	Waw with Hamza above
    { 0xc5, 0xff, 0xc5, 0xff }, // 0x625 	R	Alef with Hamza below
    { 0xc6, 0xc0, 0xc6, 0xc0 }, // 0x626 	D	Yeh with Hamza above
    { 0xc7, 0xff, 0xc7, 0xff }, // 0x627 	R	Alef
    { 0xc8, 0xeb, 0xc8, 0xeb }, // 0x628 	D	Beh
    { 0xc9, 0xff, 0x8e, 0xff }, // 0x629 	R	Teh Marbuta
    { 0xca, 0xec, 0xc1, 0xec }, // 0x62a 	D	Teh
    { 0xcb, 0xed, 0xcb, 0xed }, // 0x62b 	D 	Theh
    { 0xcc, 0xee, 0xcc, 0xee }, // 0x62c 	D 	Jeem
    { 0xcd, 0xef, 0xcd, 0xef }, // 0x62d 	D 	Hah
    { 0xce, 0xf0, 0xce, 0xf0 }, // 0x62e 	D 	Khah
    { 0xcf, 0xff, 0xcf, 0xff }, // 0x62f 	R 	Dal

    { 0xd0, 0xff, 0xd0, 0xff }, // 0x630 	R 	Thal
    { 0xd1, 0xff, 0xd1, 0xff }, // 0x631 	R 	Reh
    { 0xd2, 0xff, 0xd2, 0xff }, // 0x632 	R 	Zain
    { 0xd3, 0xf1, 0x8f, 0xf1 }, // 0x633 	D 	Seen
    { 0xd4, 0xf2, 0x90, 0xf2 }, // 0x634 	D 	Sheen
    { 0xd5, 0xf3, 0x91, 0xf3 }, // 0x635 	D 	Sad
    { 0xd6, 0xf4, 0x92, 0xf4 }, // 0x636 	D 	Dad
    { 0xd7, 0xd7, 0x93, 0x93 }, // 0x637 	D 	Tah
    { 0xd8, 0xd8, 0x94, 0x94 }, // 0x638 	D 	Zah
    { 0xd9, 0xf5, 0x96, 0x95 }, // 0x639 	D 	Ain
    { 0xda, 0xf6, 0x98, 0x97 }, // 0x63a 	D	Ghain
    { 0xff, 0xff, 0xff, 0xff }, // 0x63b
    { 0xff, 0xff, 0xff, 0xff }, // 0x63c
    { 0xff, 0xff, 0xff, 0xff }, // 0x63d
    { 0xff, 0xff, 0xff, 0xff }, // 0x63e
    { 0xff, 0xff, 0xff, 0xff }, // 0x63f

    { 0xe0, 0xff, 0xff, 0xff }, // 0x640 		Tatweel
    { 0xe1, 0xf7, 0xe1, 0x99 }, // 0x641 	D 	Feh
    { 0xe2, 0xf8, 0xe2, 0x9a }, // 0x642 	D 	Qaf
    { 0xe3, 0xf9, 0xe3, 0x9b }, // 0x643 	D 	Kaf
    { 0xe4, 0xfa, 0xe4, 0xfa }, // 0x644 	D 	Lam
    { 0xe5, 0xfb, 0xe5, 0xfb }, // 0x645 	D 	Meem
    { 0xe6, 0xfc, 0xe6, 0xfc }, // 0x646 	D 	Noon
    { 0xe7, 0xfd, 0xe7, 0xfd }, // 0x647 	D 	Heh
    { 0xe8, 0xff, 0xe8, 0xff }, // 0x648 	R 	Waw
    { 0x8d, 0x9f, 0x9f, 0x9f }, // 0x649 	D 	Alef Maksura 	### this looks strange, the font only has isolated and final forms, still dual joining?
    { 0xea, 0xfe, 0xea, 0xfe }, // 0x64a 	D 	Yeh
    { 0xeb, 0xff, 0xff, 0xff }, // 0x64b 	Mark Fathatan
    { 0xec, 0xff, 0xff, 0xff }, // 0x64c 	Mark Dammatan
    { 0xed, 0xff, 0xff, 0xff }, // 0x64d 	Mark Kasratan
    { 0xee, 0xff, 0xff, 0xff }, // 0x64e 	Mark Fatha
    { 0xef, 0xff, 0xff, 0xff }, // 0x64f 	Mark Damma

    { 0xf0, 0xff, 0xff, 0xff }, // 0x650 	Mark Kasra
    { 0xf1, 0xff, 0xff, 0xff }, // 0x651 	Mark Shadda
    { 0xf2, 0xff, 0xff, 0xff }, // 0x652 	Mark Sukan
    // these do not exist in latin6 anymore:
    { 0xff, 0xff, 0xff, 0xff }, // 0x653 	Mark Maddah above
    { 0xff, 0xff, 0xff, 0xff }, // 0x654 	Mark Hamza above
    { 0xff, 0xff, 0xff, 0xff }, // 0x655 	Mark Hamza below
    { 0xff, 0xff, 0xff, 0xff }, // 0x656
    { 0xff, 0xff, 0xff, 0xff }, // 0x657
    { 0xff, 0xff, 0xff, 0xff }, // 0x658
    { 0xff, 0xff, 0xff, 0xff }, // 0x659
    { 0xff, 0xff, 0xff, 0xff }, // 0x65a
    { 0xff, 0xff, 0xff, 0xff }, // 0x65b
    { 0xff, 0xff, 0xff, 0xff }, // 0x65c
    { 0xff, 0xff, 0xff, 0xff }, // 0x65d
    { 0xff, 0xff, 0xff, 0xff }, // 0x65e
    { 0xff, 0xff, 0xff, 0xff }, // 0x65f

    { 0xb0, 0xff, 0xff, 0xff }, // 0x660 	Arabic 0
    { 0xb1, 0xff, 0xff, 0xff }, // 0x661 	Arabic 1
    { 0xb2, 0xff, 0xff, 0xff }, // 0x662 	Arabic 2
    { 0xb3, 0xff, 0xff, 0xff }, // 0x663 	Arabic 3
    { 0xb4, 0xff, 0xff, 0xff }, // 0x664 	Arabic 4
    { 0xb5, 0xff, 0xff, 0xff }, // 0x665 	Arabic 5
    { 0xb6, 0xff, 0xff, 0xff }, // 0x666 	Arabic 6
    { 0xb7, 0xff, 0xff, 0xff }, // 0x667 	Arabic 7
    { 0xb8, 0xff, 0xff, 0xff }, // 0x668 	Arabic 8
    { 0xb9, 0xff, 0xff, 0xff }, // 0x669 	Arabic 9
    { 0x25, 0xff, 0xff, 0xff }, // 0x66a 	Arabic % sign
    { 0x2c, 0xff, 0xff, 0xff }, // 0x66b 	Arabic decimal separator
    { 0x2c, 0xff, 0xff, 0xff }, // 0x66c 	Arabic thousands separator
    { 0x2a, 0xff, 0xff, 0xff }, // 0x66d 	Arabic five pointed star
    { 0xff, 0xff, 0xff, 0xff }, // 0x66e
    { 0xff, 0xff, 0xff, 0xff } // 0x66f
};

static const uchar arabic68AlefMapping[6][5] = {
    { 0xc2, 0xff, 0xc2, 0xff, 0xa2 }, // 0x622 	R 	Alef with Madda above
    { 0xc3, 0xff, 0xc3, 0xff, 0xa3 }, // 0x623 	R	Alef with Hamza above
    { 0xc4, 0xff, 0xc4, 0xff, 0xff}, // 0x624 	R	Waw with Hamza above
    { 0xc5, 0xff, 0xc5, 0xff, 0xa4 }, // 0x625 	R	Alef with Hamza below
    { 0xc6, 0xc0, 0xc6, 0xc0, 0xff }, // 0x626 	D	Yeh with Hamza above
    { 0xc7, 0xff, 0xc7, 0xff, 0xa1 } // 0x627 	R	Alef
};

static const uchar arabic68LamLigature[8] = {
    0xff, 0xa5, 0xff, 0xa6
};

int QFontArabic68Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}

QFontArabic68Codec::QFontArabic68Codec()
{
}

const char* QFontArabic68Codec::name() const
{
    return "iso8859-6.8x";
}

int QFontArabic68Codec::mibEnum() const
{
    return -1;
}

QString QFontArabic68Codec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return QString(); //###
}

QCString QFontArabic68Codec::fromUnicode(const QString& , int&  ) const
{
    return QCString();
}

QByteArray QFontArabic68Codec::fromUnicode(const QString& uc, int from, int len ) const
{
    if( len < 0 )
	len = uc.length() - from;
    if( len == 0 )
	return QByteArray();

    QByteArray result( len );
    uchar *data = (uchar *)result.data();
    const QChar *ch = uc.unicode() + from;
    for ( int i = 0; i < len; i++ ) {
	uchar r = ch->row();
	uchar c = ch->cell();
	if ( r == 0 && c < 0x80 ) {
	    *data = c;
	} else if ( r != 0x06 || c > 0x6f )
	    *data = 0xff; // undefined char in iso8859-6.8x
	else {
	    int shape = QArabicShaping::glyphVariant( uc, i+from );
	    //qDebug("mapping U+%x to shape %d glyph=0x%x", ch->unicode(), shape, arabic68Mapping[ch->cell()][shape]);

	    // take care of lam-alef ligatures (lam right of alef)
	    switch ( c ) {
		case 0x22: // alef with madda
		case 0x23: // alef with hamza above
		case 0x25: // alef with hamza below
		case 0x27: // alef
		    if ( nextChar( uc, i+from )->unicode() == 0x0644 ) {
			// have a lam alef ligature
			shape = 4; // ligating shape
			//qDebug(" alef of lam-alef ligature");
		    }
		    *data = arabic68AlefMapping[c - 0x22][shape];
		    break;
		case 0x44: { // lam
		    const QChar *pch = prevChar( uc, i+from );
		    if ( pch->row() == 0x06 ) {
			switch ( pch->cell() ) {
			    case 0x22:
			    case 0x23:
			    case 0x25:
			    case 0x27:
				//qDebug(" lam of lam-alef ligature");
				*data = arabic68LamLigature[shape];
				goto next;
			    default:
				break;
			}
		    }
		}
		default:
		    *data = arabic68Mapping[c][shape];
	    }
	}
    next:
	ch++;
	data++;
    }
    return result;
}

ushort QFontArabic68Codec::characterFromUnicode( const QString &str, int pos ) const
{
    const QChar *ch = str.unicode() + pos;
    uchar r = ch->row();
    uchar c = ch->cell();
    if ( r == 0 && c < 0x80 ) {
	return c;
    }
    if ( r != 0x06 || c > 0x6f )
	return 0xff; // undefined char in iso8859-6.8x
    else {
	int shape = QArabicShaping::glyphVariantLogical( str, pos );
	//qDebug("mapping U+%x to shape %d glyph=0x%x", ch->unicode(), shape, arabic68Mapping[ch->cell()][shape]);

	// lam aleph ligatures
	switch ( c ) {
	    case 0x22: // alef with madda
	    case 0x23: // alef with hamza above
	    case 0x25: // alef with hamza below
	    case 0x27: // alef
		if ( prevChar( str, pos )->unicode() == 0x0644 )
		    // have a lam alef ligature
		    shape = 4; // ligating shape
		return arabic68AlefMapping[c - 0x22][shape];
	    case 0x44: { // lam
		const QChar *nch = nextChar( str, pos );
		if ( nch->row() == 0x06 ) {
		    switch ( nch->cell() ) {
			case 0x22:
			case 0x23:
			case 0x25:
			case 0x27:
			    return arabic68LamLigature[shape];
			default:
			    break;
		    }
		}
	    }
	    default:
		break;
	}
	return arabic68Mapping[c][shape];
    }
}
#endif

#endif //QT_NO_CODECS
