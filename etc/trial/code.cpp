

#include <qstring.h>
#include <qdatetime.h>
#include <stdio.h>

static const int keyLen = 7;

QString keyString( Q_UINT8* keyCode )
{
    QString key;
    Q_UINT8 s = keyCode[4] & 0x7f;

    for( int i = 0; i < keyLen; i++) {
	for( int j = 0; j < 2; j++) {
	    int n = j ? ( keyCode[i] & 0x1f ) : ( (keyCode[i] & 0xf8) >> 3 );
	    if (j) {
		n &= 0x0f;
		n |= (( s >> i ) & 0x01) << 4;
	    }
	    char c = 'A' + n;
	    if ( c > 'Z' )
		c = '0' + ( c - 'Z' );
	    key = key + c;
	}
    }
    return key;
}


QString simplifyString( const char* str )
{
    if ( !str ) str = "dummy";
    QString qsi( str );
    if ( qsi.isEmpty() )
	qsi = "dummy";
    qsi.simplifyWhiteSpace();
    qsi = qsi.lower();
    QString qso;
    for( int i = 0; i < qsi.length(); i++ ) {
	char c = qsi[i];
	if( ((c >= 'a') && (c <='z')) || ((c >= '1') && (c <='9')) )
	    qso = qso + qsi[i];	
    }

    return qso;
}



QString genKeyString( const char* name, const char* company,
		      const char* email, QDate endDate, int serialNo )
{
    Q_UINT8 keyCode[keyLen];
    for (int i = 0; i < keyLen; i++ )
	keyCode[i] = 169;

    QString x = simplifyString( name );
    x = x + simplifyString( company );
    x = x + simplifyString( email );

    Q_UINT16 n = qChecksum( (const char*)x, x.length() );

    keyCode[2] = (Q_UINT8)(n & 0x00ff);
    keyCode[0] = (Q_UINT8)((n & 0xff00) >> 8);
    
    Q_UINT8 y = ((Q_UINT8)(endDate.year() - 1997 )) & 0x07;
    Q_UINT8 m = ((Q_UINT8)endDate.month()) & 0x0f;
    Q_UINT8 d = ((Q_UINT8)endDate.day()) & 0x1f;
    
    Q_UINT16 s = ((Q_UINT16)serialNo) & 0x0fff;
    Q_UINT8 s1 = (Q_UINT8)( s & 0x00ff );
    Q_UINT8 s2 = (Q_UINT8)( (s & 0xff00) >> 8  );

    keyCode[1] = (m << 4) | s2;
    keyCode[3] = s1;
    keyCode[5] = ( d << 3 ) | y;

    Q_UINT16 crc = qChecksum( (const char*)keyCode, keyLen );
    keyCode[6] = (Q_UINT8)(crc & 0x00ff);
    keyCode[4] = (Q_UINT8)((crc & 0xff00) >> 8);

    return keyString( keyCode );

}



void main(int argc, char* argv[])
{
    QDate d = QDate::currentDate().addDays(40);
    if (argc < 4 ) {
	printf("Usage: %s name company email serialnumber\n", argv[0] );
	return 1;
    }
    QString numStr( argv[4] );
    bool ok = TRUE;
    ushort num = numStr.toUShort( &ok );
    if (!ok) {
	printf("%s: serialnumber must be a short unsigned integer\n", argv[0] );
	return 1;
    }

    printf("The key string is '%s'\n", (const char*) genKeyString( argv[1], argv[2], argv[3], d, num ));
}
