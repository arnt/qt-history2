#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>

#include <sys/time.h>





enum CharSet { Latin1, Latin2, Latin6, Latin7, Latin8, Russian,
	       Armenian, Tamil, Thai, Unknown, AnyCharSet = Unknown };

char *CharSetNames[] = { "Latin1", "Latin2", "Latin6", "Latin7", "Latin8", "Russian",
			 "Armenian", "Tamil", "Thai", "Unknown" };


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
	if (tmp != current) {
	    // qDebug("new charset %s at %d", CharSetNames[tmp], i);
	    current = tmp;
	}
    }
    
    gettimeofday(&tv2, 0);
    
    qDebug("search completed, %ld s %02ld us", tv2.tv_sec - tv.tv_sec,
	   tv2.tv_usec - tv.tv_usec);
    
    return 0;
}
