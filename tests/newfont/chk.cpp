#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>

#include <sys/time.h>

enum CharSet {
    Latin1,
    Latin2,
    Latin6,
    Latin7,
    Latin8,
    Russian,
    Armenian,
    Tamil,
    Thai,
    Unknown,
    AnyCharSet = Unknown
};

char *CharSetNames[] = {
    "Latin1",
    "Latin2",
    "Latin6",
    "Latin7",
    "Latin8",
    "Russian",
    "Armenian",
    "Tamil",
    "Thai",
    "Unknown"
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
    "iso8859-11",
    "iso8859-12",
    "iso8859-13",
    "iso8859-14",
    "iso8859-15",
    "koi8-r",          // russian
    "koi8-ru",         // ukrainian
    "jisx0208.1983-0", // japanses JIS
    "gb2313.1980-0"    // gbk
    "big5-0",          // big5
    "ksc5601.1987-0",  // korean
    "*-*"              // unknown / any
};

CharSet encodingForChar( const QChar &c )
{
    uchar row = c.row();
    if ( !row )
	return Latin1;

    if ( row >= 0x30 && row <= 0xd7 ) {
	// asian fonts, need to split up
	return AnyCharSet;
    }

    switch ( row ) {
    case 0x01:
    case 0x02:
	// ### might be something else too
	return Latin2;

    case 0x03:
	if ( c.cell() >= 0x60 ) // greek
	    return Latin7;
	break;

    case 0x04:
	return Russian; // russian

    case 0x05:
	if( c.cell() >= 0x90 )
	    return Latin8; // hebrew
	else
	    return Armenian;
	break;

    case 0x06:
	// probably won't work like this because of shaping...
	return Latin6; // arabic

    case 0x0b:
	if ( c.cell() >= 0x80 )
	    return Tamil; // tamil
	break;

    case 0x0e:
	// ### lao is also in this region
	return Thai; // thai
    }

    return AnyCharSet;
}


QRect boundingRect_new(const QString &string, int len = -1)
{
    if (len < 0)
	len = str.length();
    
    int ascent,
	descent,
   	lb, // left bearing
	rb, // right bearing
	width;
    
    bool underline = underlineFlag(),
	 strikeOut = strikeOutFlag();
    
    // const QTextCodec *m = mapper();
    for (int i = 0; i < 
    
    
    const QChar *uc = string.unicode();
    
    
    
    
    
}


int main(int, char **)
{
    QFile file("x-utf8.html");

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

    CharSet current, tmp;
    const QChar *uc = contents.unicode();
    int i = 0, len = contents.length();

    // get charset for first character
    current = encodingForChar(*uc);
    uc++;
    i++;

    struct timeval tv, tv2;
    gettimeofday(&tv, 0);

    for (; i < len; i++) {
	tmp = encodingForChar(*(uc++));
	if (tmp != current && tmp != Latin1) {
	    qDebug("new charset %s at %d", CharSetNames[tmp], i);
	    current = tmp;
	}
    }

    gettimeofday(&tv2, 0);

    qDebug("search completed, %ld s %02ld us", tv2.tv_sec - tv.tv_sec,
	   tv2.tv_usec - tv.tv_usec);

    return 0;
}
