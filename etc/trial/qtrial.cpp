/****************************************************************************
** $Id: //depot/qt/main/etc/trial/qtrial.cpp#3 $
**
**		     ***   STRICTLY CONFIDENTIAL   ***
**
**	    This file is NOT a part of the official Qt source code!
**
** Inserts trial user registration data in qt.lib.
**
** Created : 970421
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include <qfile.h>
#include <qdatetime.h>
#include <stdio.h>
#include <stdlib.h>


/*****************************************************************************
  General encryption/decryption routines.
  The code below has been copied from qapplication_trial.cpp
 *****************************************************************************/

static long randomSeed = 13579;

static void setRandomSeed( int seed )
{
    if ( seed < 0 )
        seed = - seed;
    randomSeed = seed - (seed / 100000)*100000;
    if ( randomSeed == 0 )
        randomSeed = 15397;
}

static inline uchar randomValue()
{
    randomSeed = randomSeed*147;
    randomSeed = randomSeed - (randomSeed / 100000)*100000;
    if ( randomSeed == 0 )
        randomSeed = 15397;
    return randomSeed % 256;
}

static inline void scramble( uchar *data, int length )
{
    while ( length-- ) {
	*data ^= randomValue();
	data++;
    }
}

static QString unscrambleString( int seed, const uchar *data, int len )
{
    QString s( len+1 );
    memcpy( s.data(), data, len );
    setRandomSeed( seed );
    scramble( (uchar *)s.data(), len );
    return s;
}


/*****************************************************************************
  Key string encryption.
 *****************************************************************************/

/*
  This function has been copied from qapplication_trial.cpp
*/

static QString simplifyUserString( const char* str )
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

static const int keyLen = 7;

/*
  Converts a 7 byte array of key bytes to a 14 character string.
*/

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


/*
  Generates a 14 character key string via a 7 byte key code array.
  Encoding:
    16 bit info checksum.
    16 bit key checksum.
    12 bit expiry date.
    12 bit serial number.
*/

QString genKeyString( const char* name, const char* company,
		      const char* email, QDate endDate, int serialNo )
{
    Q_UINT8 keyCode[keyLen];
    for (int i = 0; i < keyLen; i++ )
	keyCode[i] = 169;

    QString x = simplifyUserString( name );
    x = x + simplifyUserString( company );
    x = x + simplifyUserString( email );

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


/*****************************************************************************
  Misc. stuff
 *****************************************************************************/

static const char trial_data[141] =
    "2502200483trial50021trial50023trial2502200483trial50021trial50023trial"
    "2502200483trial50021trial50023trial2502200483trial50021trial50023trial";

int     searchFile( const QString &file );
QString patchFile( const QString &file, const QString &name,
	           const QString &company, const QString &email,
	 	   const QDate &expiry, int serial, int offset );


void usage()
{
    fprintf( stderr, "qtrial -file    <qt.lib>\n"
		     "       -name    <user name>\n"
		     "       -company <company>\n"
		     "       -email   <email-address>\n"
		     "       -serial  <serial number>\n"
		     "       -offset  <position in file> (optional)\n"
		     "       -expiry  <yymmdd> (optional)\n"
		     "   or\n"
		     "qtrial -file    <qt.lib>\n"
		     "       -findoffset\n\n"
		     "All arguments are mandatory, except -company\n" );
    exit( 1 );
}


void error( const char *msg )
{
    fprintf( stderr, "qtrial: %s\n", msg );
    exit( 1 );
}



int main( int argc, char **argv )
{
    if ( argc < 2 )
	usage();

    bool     findOffset = FALSE;
    QString  file;
    QString  name;
    QString  company;
    QString  email;
    QString  serial;
    QString  offset;
    QString  expiry;
    QString *val = 0;				// expected value
    int	     i;

    for ( i=1; i<argc; i++ ) {
	if ( val ) {
	    *val = argv[i];
	    val = 0;
	    continue;
	}
	QString opt = argv[i];
	opt = opt.lower();
	if ( opt == "-findoffset" )
	    findOffset = TRUE;
	else if ( opt == "-file" )
	    val = &file;
	else if ( opt == "-name" )
	    val = &name;
	else if ( opt == "-company" )
	    val = &company;
	else if ( opt == "-email" )
	    val = &email;
	else if ( opt == "-serial" )
	    val = &serial;
	else if ( opt == "-offset" )
	    val = &offset;
	else if ( opt == "-expiry" )
	    val = &expiry;
	else
	    usage();
    }
    if ( val )					// value not set
	usage();
    if ( file.isEmpty() )
	error( "Missing file argument" );

    if ( findOffset ) {
	printf( "%d\n", searchFile(file) );
    } else {
	if ( name.isEmpty() )
	    error( "Missing name argument" );
	if ( email.isEmpty() )
	    error( "Missing email argument" );
	if ( offset.isEmpty() )
	    offset.setNum( searchFile(file) );
	if ( company.isEmpty() )
	    company = "";
	bool ok;
	int offset_num = offset.toInt( &ok );
	if ( !ok || offset_num <= 0 )
	    error( "Trial data pattern not found - file probably patched." );
	int serial_num = serial.toInt( &ok );
	if ( !ok || serial_num <= 0 || serial_num > 65535)
	    error( "Invalid serial number" );
	QDate exp;
	if ( expiry.isEmpty() ) {
	    exp = QDate::currentDate();
	    exp = exp.addDays( 30 );
	} else {
	    int d = expiry.toInt(&ok);
	    if ( !ok )
		error( "Invalid expiry date" );
	    exp = QDate( d / 10000 + 1900, (d / 100) % 100, d % 100 );
	}
	QString key;
	key = patchFile( file, name, company, email, exp, serial_num,
			 offset_num );
	printf( "Registered information:\n\t"
	        "name    : %s\n\t"
		"company : %s\n\t"
		"email   : %s\n\t"
		"serial  : %s\n\t"
		"EXPIRY  : %s\n",
		name.data(), company.data(), email.data(), key.data(),
		exp.toString().data() );
    }
    return 0;
}


int searchFile( const QString &file )
{
    QFile f(file);
    if ( !f.open(IO_ReadOnly) ) {
	error( "Cannot open file" );
	return -1;
    }
    QByteArray buf(f.size());
    f.readBlock(buf.data(),buf.size());
    f.close();
    int   s = 140;
    char *p = buf.data();
    char *end = p + buf.size()-s-1;
    while ( p < end ) {
	if ( *p == trial_data[0] && memcmp(p,trial_data,s) == 0 ) {
	    return (int)p - (int)buf.data();
	}
	p++;
    }
    return -1;
}


QString patchFile( const QString &file, const QString &name,
	           const QString &company, const QString &email,
		   const QDate &expiry, int serial, int offset )
{
    QString key = genKeyString( name, company, email, expiry, serial );
    QByteArray buf(1024);
    buf.fill( (uchar)251 );
    char *p = buf.data();

    memcpy( p, key.data(), key.size() );
    p += key.size();
    memcpy( p, name.data(), name.size() );
    p += name.size();
    memcpy( p, company.data(), company.size() );
    p += company.size();
    memcpy( p, email.data(), email.size() );
    p += email.size();
    if ( (int)p - (int)buf.data() >= 140 )
	error( "Name + company + email too long" );

    setRandomSeed( 79153 );
    scramble( (uchar *)buf.data(), 140 );

    QFile f(file);
    if ( !f.open(IO_WriteOnly|IO_Raw) )
	error( "Cannot open file" );
    f.at( offset );
    if ( f.at() != offset )
	error( "Cannot jump to the offset in the file" );
    f.writeBlock(buf.data(),140);
    f.close();

    return key;
}
