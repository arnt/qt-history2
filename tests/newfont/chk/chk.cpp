#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>

#include <sys/time.h>

enum CharSet {
    // ISO 8859 encodings
    ISO_8859_1,
    ISO_8859_2,
    ISO_8859_3,
    ISO_8859_4,
    ISO_8859_5,
    ISO_8859_6,
    ISO_8859_7,
    ISO_8859_8,
    ISO_8859_9,
    ISO_8859_10,
    ISO_8859_11,
    ISO_8859_12,
    ISO_8859_13,
    ISO_8859_14,
    ISO_8859_15,

    // Russian and Ukranian
    KOI8_R,
    KOI8_U,

    // multibyte encodings
    // EUC_JP,
    // EUC_KR,
    // EUC_ZH, // does this exist?
    // EUC_TW, // does this exist?
    SJIS,
    // UJIS,   // does this exist? where did i get it from?
    GBK,
    BIG5,
    KSC,       // is this the right name?  X has the ksc5601 registry

    // Tamil
    TSCII,

    // Unicode
    ISO_10646_1,

    // End
    AnyCharSet,
    Unknown = AnyCharSet,

    // Latin[1-9] encodings map to ISO encodings
    Latin1   = ISO_8859_1,
    Latin2   = ISO_8859_2,
    Latin3   = ISO_8859_3,
    Latin4   = ISO_8859_4,
    Latin5   = ISO_8859_9,
    Latin6   = ISO_8859_10,
    Latin7   = ISO_8859_13,
    Latin8   = ISO_8859_14,
    Latin9   = ISO_8859_15,

    // Language names (?) that map to ISO encodings
    Cyrillic = ISO_8859_5,
    Arabic   = ISO_8859_6,
    Greek    = ISO_8859_7,
    Hebrew   = ISO_8859_8,
    Turkish  = ISO_8859_9,
    Thai     = ISO_8859_11,

    // Alternate encoding names (?) that map to ISO encodings
    TIS620   = ISO_8859_11,
};


char *X11EncodingNames[] = {
    "iso8859-1",
    "iso8859-2",
    "iso8859-3",
    "iso8859-4",
    "iso8859-5",
    "iso8859-6",
    "iso8859-7",
    "iso8859-8",
    "iso8859-9",
    "iso8859-10",
    "tis620-*", // "iso8859-11", // this should probably be tis620-1 (Thai)
    "iso8859-12",
    "iso8859-13",
    "iso8859-14",
    "iso8859-15",
    "koi8-r",
    "koi8-ru",
    //    "*-*",             // EUC_JP ???
    //    "*-*",             // EUC_KR ???
    //    "jisx0201.1976-0", // JIS aka JIS7 - an 8bit japanese encoding
    "jisx0208.1983-0", // SJIS aka Shift JIS - a 16bit japanese encoding
    "gb2312.1980-0",   // GBK
    "big5-0",          // Big5
    "ksc5601.1987-*",
    "tscii-*",         // Tamil... i've seen tscii-0 and tscii-1
    "iso10646-1",
    "*-*"
};


CharSet encodingForChar( const QChar &c )
{
    uchar row = c.row();
    if ( !row )
	return ISO_8859_1;

    if ( row >= 0x30 && row <= 0xd7 ) {
	// asian fonts, need to split up
	return Unknown;
    }

    switch ( row ) {
    case 0x01:
    case 0x02:
	// ### might be something else too
	return ISO_8859_2;

    case 0x03:
	if ( c.cell() >= 0x60 ) // greek
	    return ISO_8859_7;
	break;

    case 0x04:
	return KOI8_R; // russian

    case 0x05:
	if( c.cell() >= 0x90 )
	    return ISO_8859_8; // hebrew
	break;
	// return Armenian; // no support for armenian

    case 0x06:
	// probably won't work like this because of shaping...
	return ISO_8859_6; // arabic

    case 0x0b:
	if ( c.cell() >= 0x80 )
	    return TSCII; // tamil
	break;

    case 0x0e:
	// ### lao is also in this region
	return ISO_8859_11; // thai
    }

    return Unknown;
}


int main(int, char **)
{
    QFile file("../newfont/x-utf8.html");

    if (! file.open(IO_ReadOnly)) {
	qDebug("failed to load file");
	return 1;
    }

    QTextStream stream(&file);
    stream.setEncoding(QTextStream::UnicodeUTF8);
    QString contents = stream.read();

    if (contents.isNull() || contents.length() < 1) {
	qDebug("failed to read file");
	return 1;
    }

    CharSet current = AnyCharSet, tmp;
    const QChar *uc = contents.unicode();
    int i = 0, len = contents.length();
    
    // get charset for first character
    //    current = encodingForChar(*uc);
    //    uc++;
    //    i++;

    struct timeval tv, tv2;
    gettimeofday(&tv, 0);

    for (; i < len; i++) {
	tmp = encodingForChar(*(uc++));
	if (tmp != current) {
	    qDebug("new charset %s at %d", X11EncodingNames[tmp], i);
	    current = tmp;
	}
    }

    gettimeofday(&tv2, 0);

    qDebug("search completed, %ld s %02ld us", tv2.tv_sec - tv.tv_sec,
	   tv2.tv_usec - tv.tv_usec);

    return 0;
}
