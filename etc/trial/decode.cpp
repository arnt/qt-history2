

#include <qstring.h>
#include <qdatetime.h>
#include <stdio.h>

static const int keyLen = 7;


/* Only the 2 functions below need to be included in the INSTALL program! */
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
	if( ((c >= 'a') && (c <='z')) || ((c >= '0') && (c <='9')) )
	    qso += c;	
    }

    return qso;
}


bool checkInfo( const char* name, const char* company,
		const char* email, char* keyString )
{
    static Q_UINT16 g = 0;

    Q_UINT8 keyCode[keyLen];
    char c;
    for( int i = 0; i < keyLen; i++) {
	for( int j = 0; j < 2; j++) {
	    c = keyString[2*i + j]; 
	    if ( c < '9' )
		c = 'Z' + (c - '0');
	    c -= 'A';
	    if( !j )
		keyCode[i] = (c << 3);
	    else
		keyCode[i] |= ( c & 0x0f );
	}
    }	    

    Q_UINT16 kc = ((Q_UINT16)keyCode[4] << 8) | (Q_UINT16)keyCode[6];
    kc += ( g & 0xfff8);
    keyCode[4] = 169;
    keyCode[6] = 169;
    Q_UINT16 crc = qChecksum( (const char*)keyCode, keyLen );
    
    bool res = TRUE;
    if ( kc != crc )
	res = FALSE;

    QString x = simplifyString( name );
    x = x + simplifyString( company );
    x = x + simplifyString( email );

    Q_UINT16 n = qChecksum( (const char*)x, x.length() );
 
    Q_UINT8 k = (Q_UINT8)((n & 0xff00) >> 8);
    if ( res )
	res = ( k == keyCode[0] );

    k = (Q_UINT8)(n & 0x00ff);
    if ( res )
	res = ( k == keyCode[2] );

    g++; //### Don't do if this function is going to be called repeatedly
    return res;

}


bool getData( char* keyString, int* y, int* m, int* d, int* serialNo )
{
    static Q_UINT16 g = 0;
    Q_UINT8 keyCode[keyLen];
    int s = 0;
    char c;
    bool res = TRUE;
    for( int i = 0; i < keyLen; i++) {
	for( int j = 0; j < 2; j++) {
	    c = keyString[2*i + j];
	    if ( c < '9' )
		c = 'Z' + (c - '0');
	    c -= 'A';
	    if( !j )
		keyCode[i] = (c << 3);
	    else {
		s |= ((( c & 0x10 ) >> 4) << i);
		keyCode[i] |= ( c & 0x0f );
	    }
	}
    }	    

    if ( s != (keyCode[4] & 0x7f) )
	res = FALSE;

    Q_UINT16 kc = ((Q_UINT16)keyCode[4] << 8) | (Q_UINT16)keyCode[6];
    keyCode[4] = 169;
    keyCode[6] = 169;
    Q_UINT16 crc = qChecksum( (const char*)keyCode, keyLen );
    kc += ( g & 0xfff8);
    
    if ( kc != crc )
	res = FALSE;
    
    if (res) {
	*y = (int)1997+(keyCode[5] & 0x07);
	*m = (int)(keyCode[1]&0xf0) >> 4;
	*d = (int)(keyCode[5]&0xf8) >> 3;
    
	Q_UINT16 sn = (Q_UINT16)(keyCode[1]&0x0f) << 8;
	sn |= (Q_UINT16) keyCode[3];

	*serialNo = (int)sn;
    }

    g++; //### Don't do if this function is going to be called repeatedly
    return res;
}

void main(int argc, char* argv[])
{
    if (argc < 4 ) {
	printf("Usage: %s keystring name company email\n", argv[0] );
	return 1;
    }

    //    for( int h=0; h<10; h++ ) {
    if ( checkInfo( argv[2], argv[3], argv[4], argv[1] ) )
	printf("Name, company & email OK\n");
    else
	printf("Name, company & email MISMATCH!\n");

    QDate end(1990, 12, 1);
    int serialNo, y, m, d;

    if ( getData( argv[1], &y, &m, &d, &serialNo ) ) {
	end.setYMD( y, m, d );
	printf("keystring OK, serialNo=%i, expires %s\n", serialNo, (const char*)end.toString());
    }
    else
	printf("keyString MISMATCH!\n");
    
    //    }
}
