/****************************************************************************
** $Id: //depot/qt/main/etc/trial/qapp_trial.cpp#4 $
**
**		     ***   STRICTLY CONFIDENTIAL   ***
**
**	    This file is NOT a part of the official Qt source code!
**
** Implementation of Windows trial routines.
** This file is included by qapp_win.cpp if QT_TRIAL is defined.
**
** IMPORTANT: Change FINAL_TRIALDATE before March 15th 1997.
**
** Created : 970421
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/


#define FINAL_TRIALDATE 2450935	    // May 1st 1998


#if defined(UNIX)
#include <stdio.h>
#endif


/*****************************************************************************
  General encryption/decryption routines.
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
  Key string decryption.
 *****************************************************************************/

static const int keyLen = 7;

static QString simplifyUserString( const char* str )
{
    if ( !str ) str = "dummy";
    QString qsi( str );
    if ( qsi.isEmpty() )
	qsi = "dummy";
    qsi.simplifyWhiteSpace();
    qsi = qsi.lower();
    QString qso;
    for( int i = 0; i < (int)qsi.length(); i++ ) {
	char c = qsi[i];
	if( ((c >= 'a') && (c <='z')) || ((c >= '0') && (c <='9')) )
	    qso += c;	
    }
    return qso;
}


static bool checkTrialInfo( const char* name, const char* company,
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

    QString x = simplifyUserString( name );
    x = x + simplifyUserString( company );
    x = x + simplifyUserString( email );

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


static bool getTrialData( const char* keyString, int* y, int* m, int* d,
			  int* serialNo )
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
	    if( !j ) {
		keyCode[i] = (c << 3);
	    } else {
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


/*****************************************************************************
  Date routines, taken from qdatetm.cpp
 *****************************************************************************/

static uint julianDay( int y, int m, int d )
{
    uint c, ya;
    if ( y <= 99 )
	y += 1900;
    if ( m > 2 ) {
	m -= 3;
    } else {
	m += 9;
	y--;
    }
    c = y;
    c /= 100;
    ya = y - 100*c;
    return 1721119 + d + (146097*c)/4 + (1461*ya)/4 + (153*m+2)/5;
}


/*****************************************************************************
  Trial license data.
 *****************************************************************************/

#include <qlabel.h>

static const char trial_data[141] =
    "2502200483trial50021trial50023trial2502200483trial50021trial50023trial"
    "2502200483trial50021trial50023trial2502200483trial50021trial50023trial";

static int	 trial_timer;

static QWidget  *trial_info = 0;

static const int trial_key_len = 7;
static char	*trial_key = 0;
static char	*trial_name;
static char	*trial_company;
static char	*trial_email;

static int	 trial_julday;
static int	 today_julday;
static QDate	 trial_date;
static QDate	 today_date;
static int	 trial_messup_1 = 231;
static int	 trial_messup_2 = 0;

static int	 trial_julday_copy;

#if defined(_OS_WIN32_)
static SYSTEMTIME trial_t;
#else
static time_t	  trial_t;
#endif


/*****************************************************************************
  Encrypted messages.
 *****************************************************************************/

static uchar trial_data_header[] = {
    0x8c,0xb3,0x65,0xda,0x3e,0xf7,0xf0,0x99,0xdc,0xab,0x50,0xde,0xd9,0x5e,
    0x9a,0x81,0xfd,0x2b,0x0c,0x7c,0xa8,0x59,0xe6,0x2a,0x57,0x2d,0x91,0xf7,
    0x04,0xa4,0xf5,0xbb,0x6f,0xce,0xe4,0x13,0xad,0xa1,0x30,0xfd,0x0e,0x2e,
    0x0a,0x31,0xad,0x3a,0xd4,0xd6,0x1d,0xe8,0x0b,0x93,0x74,0x97,0xf7,0xaa,
    0x7d,0x92,0xb6,0xda,0x29,0xb7,0x73,0x40,0x4f,0xa7,0x40,0x09,0x4c,0x1b,
    0xa0,0xee,0xc9,0x0e,0xea,0x11,0x6d,0x47,0x60,0xdd,0xed,0x48,0xd6,0x3a,
    0xde,0xb9 };
static const int trial_len_header = 86;

static uchar trial_data_regto[] = {
    0x29,0x44,0x94,0x00,0x18,0x45,0x06,0x6b,0x3e,0x25,0xb3,0x5d,0x64,0x2b,
    0x89,0x13 };
static const int trial_len_regto = 16;

static uchar trial_data_expire[] = {
    0x7b,0xd5,0x82,0xe5,0x9a,0x7b,0x96,0x39,0x5a,0xfc,0x93,0x4c,0x10,0x23,
    0x17,0xb0,0x97,0x4d,0x8e,0xd7,0x3a,0x3e,0xd7,0x5c,0x21,0xfd,0x22,0xa1,
    0x7f,0x09,0x56,0xac,0x9c };
static const int trial_len_expire = 33;

static uchar trial_data_copyright[] = {
    0xb4,0x7a,0xe1,0x29,0x66,0x26,0x2b,0xad,0x44,0x94,0x43,0x38,0x94,0xe5,
    0x0b,0x7f,0xb8,0x79,0xe3,0xb3,0x49,0xaa,0xbf,0x8b,0x18,0x47,0x8f,0x4d,
    0x55,0x6c,0xbc,0x64,0x39,0xd2,0xcf,0x1c,0x29,0x21,0x3f,0x3d,0xe2,0xc7,
    0x6c,0xf5,0x46,0x16,0xd6,0xa3,0xd0,0xd5,0x86,0xb3,0x61,0x2a,0xcd,0x60,
    0x36,0xc1,0x06,0x72,0xc9,0x0b,0x35,0x9c,0x03,0x35,0xe6,0x6e,0xa7,0x06,
    0xd0,0xbd,0x8e,0xc7,0x26,0x7a,0x6f,0xd1,0x5f,0x25,0xb4,0x5c,0x0f,0x8c,
    0x3e,0x7c,0x8d,0xa0,0xc6,0x2c,0x76,0x0a,0x07,0x67,0xe6,0x6d,0xc3,0x07,
    0x80,0x71,0x8b,0x85,0x2b,0x48,0x54,0xdd,0x8f,0x3c,0xf4,0x6b,0x1f,0x2c,
    0x9b,0xb9,0x6f,0xef,0x4e,0xa2,0x77,0x39,0x64,0x95,0xfd,0xd8,0xd4,0xc0,
    0x2d,0xfb,0x92,0x31,0xc1 };
static const int trial_len_copyright = 131;


/*****************************************************************************
  Dummy messages. The original messages are not used. They are only stored
  to fool potential pseudo-hackers who think they can just change the texts.
 *****************************************************************************/

static char *dummy_1 =  "Qt %s Evaluation License\n\nThis trial version may"
			" only be used for evaluation purposes.";
static char *dummy_2 =	"Registered to:\n\n";
static char *dummy_3 =	"The evaluation expires in %d days";
static char *dummy_4 =	"Contact sales@troll.no for pricing and purchasing information.\n"
		        "Qt is copyright (C) 1992-1997 by Troll Tech AS. "
		    	"All rights reserved.";


/*****************************************************************************
  Trial information window.
 *****************************************************************************/

static void cleanup_trial_info()
{
    delete [] trial_key;
    delete [] trial_name;
    delete [] trial_company;
    delete [] trial_email;
}


class TrialInfo : public QLabel
{
public:
    TrialInfo( const char *message, const char *name, const char *company,
	       const char *email, const char *expire, const char *copyright );
private:
    bool event( QEvent * );
};

TrialInfo::TrialInfo( const char *message,
		      const char *name, const char *company,
		      const char *email, const char *expire,
		      const char *copyright )
    : QLabel( 0, 0, WStyle_Customize | WStyle_NoBorder | WStyle_Tool )
{
    QColorGroup ga( black,	// foreground
		    lightGray,	// background
		    white,	// light
		    black,	// dark
		    darkGray,	// mid
		    black,	// text
		    white );	// base
    QColorGroup gb( black,	// foreground
		    lightGray,	// background
		    white,	// light
		    black,	// dark
		    darkGray,	// mid
		    red,	// text
		    white );	// base

    setFrameStyle( WinPanel | Raised );
    setLineWidth( 2 );

    QLabel *l1, *l2, *l3, *l4;
    l1 = new QLabel( this );
    l2 = new QLabel( this );
    l3 = new QLabel( this );
    l4 = new QLabel( this );

    l1->setPalette( QPalette(ga,ga,ga) );
    l1->setFont( QFont("arial",14,QFont::Bold) );
    l1->setText( message );
    l1->setAlignment( AlignCenter );
    l1->setMargin( 10 );
    l1->adjustSize();

    l2->setPalette( QPalette(ga,ga,ga) );
    l2->setFont( QFont("arial",14) );
    QString s = unscrambleString( 57113,
				  trial_data_regto,
				  trial_len_regto );
    s += name; s += '\n'; s += company; s += '\n'; s += email;
    l2->setText( s );
    l2->setAlignment( AlignCenter );
    l2->setMargin( 10 );
    l2->adjustSize();

    l3->setPalette( QPalette(gb,gb,gb) );
    l3->setFont( QFont("arial",14,QFont::Bold) );
    l3->setText( expire );
    l3->setAlignment( AlignCenter );
    l3->setMargin( 10 );
    l3->adjustSize();

    l4->setPalette( QPalette(ga,ga,ga) );
    l4->setFont( QFont("arial",9) );
    l4->setText( copyright );
    l4->setAlignment( AlignCenter );
    l4->setMargin( 10 );
    l4->adjustSize();

    int w = l1->width();
    w = QMAX(w,l2->width());
    w = QMAX(w,l3->width());
    w = QMAX(w,l4->width());
    l1->resize( w, l1->height() );
    l2->resize( w, l2->height() );
    l3->resize( w, l3->height() );
    l4->resize( w, l4->height() );

    l1->move( 2, 2 );
    l2->move( 2, l1->y() + l1->height() );
    l3->move( 2, l2->y() + l2->height() );
    l4->move( 2, l3->y() + l3->height() );

    QRect r = childrenRect();
    resize( r.width()+2*r.x(), r.height()+2*r.y() );

#if defined(_OS_WIN32_)
    GetLocalTime( &trial_t );
#else
    time( &trial_t );
#endif

#if defined(_OS_WIN32_)
    int sw = GetSystemMetrics( SM_CXSCREEN );
    int sh = GetSystemMetrics( SM_CYSCREEN );
#else
    int sw = DisplayWidth( appDpy, appScreen );
    int sh = DisplayHeight( appDpy, appScreen );
#endif
    move( sw/2 - width()/2, sh/2 - height()/2 );
    show();
}

bool TrialInfo::event( QEvent *e )
{
    switch ( e->type() ) {
	case Event_Paint:
	    paintEvent( (QPaintEvent*)e );
	    break;
	case Event_Move:
	    moveEvent( (QMoveEvent*)e );
	    break;
	case Event_Resize:
	    resizeEvent( (QResizeEvent*)e );
	    break;
	case Event_Close: {
	    QCloseEvent *c = (QCloseEvent *)e;
	    closeEvent( c );
	    if ( !c->isAccepted() )
		return FALSE;
	    }
	    break;
	default:
	    return FALSE;
    }
    return TRUE;
}


/*****************************************************************************
  Decodes the data, shows the trial window and checks if the license has
  expired.
 *****************************************************************************/

static void extract_trial_info()
{
    if ( trial_key != 0 )
	return;
    qAddPostRoutine( cleanup_trial_info );

#if defined(_OS_WIN32_)
    trial_timer = SetTimer( 0, 0, 4000, 0 );
#else
    trial_timer = qApp->startTimer( 4000 );
#endif

    const char *p;
    int len;

    QByteArray td(140);
    memcpy( td.data(), trial_data, td.size() );
    setRandomSeed( 79153 );
    scramble( (uchar *)td.data(), td.size() );
    p = td.data();

    if ( strncmp(trial_data,"2502200483trial",15)== 0 ) {
#if defined(_OS_WIN32_)
	MessageBox( 0,
		    "The registration data is uninitialized.\n"
		    "Please report to qt-bugs@troll.no.",
		    "Qt Internal Error",
		    MB_OK | MB_ICONERROR );
	ExitProcess( 1 );
#else
	fprintf( stderr, "Qt Internal Error:\n  "
		 "The registration data is uninitialized. "
		 "Please report to qt-bugs@troll.no.\n" );
	exit( 1 );
#endif
	warning( dummy_1 );			// don't optimize them away
	warning( dummy_2 );
	warning( dummy_3 );
	warning( dummy_4 );
    }

    len = strlen(p)+1;
    trial_key = new char[len];
    memcpy( trial_key, p, len );
    p += len;

    len = strlen(p)+1;
    trial_name = new char[len];
    memcpy( trial_name, p, len );
    p += len;

    len = strlen(p)+1;
    trial_company = new char[len];
    memcpy( trial_company, p, len );
    p += len;

    len = strlen(p)+1;
    trial_email = new char[len];
    memcpy( trial_email, p, len );

    int year, month, day, serial;
    bool ok;
    ok = getTrialData( trial_key, &year, &month, &day, &serial );
    if ( ok )
	ok = checkTrialInfo( trial_name, trial_company, trial_email,trial_key);
    if ( !ok ) {
#if defined(_OS_WIN32_)
	MessageBox( 0,
		    "The registration data is invalid.\n"
		    "Please report to qt-bugs@troll.no.",
		    "Qt Internal Error",
		    MB_OK | MB_ICONERROR );
	ExitProcess( 1 );
#else
	fprintf( stderr, "Qt Internal Error:\n  "
		 "The registration data is invalid. "
		 "Please report to qt-bugs@troll.no.\n" );
	exit( 1 );
#endif
    }

    trial_date = QDate(year,month,day);;
    today_date = QDate::currentDate();

    int ndays = today_date.daysTo( trial_date );
    if ( ndays < 0 ) {
#if defined(_OS_WIN32_)
	MessageBox( 0,
		    "The 30 day trial period has ended, please contact\n"
		    "sales@troll.no for pricing and purchasing information.",
		    "Qt Trial Ended",
		    MB_OK | MB_ICONERROR );
	ExitProcess( 1 );
#else
	fprintf( stderr, "Qt Trial Ended:\n  "
		    "The 30 day trial period has ended, please contact\n  "
		    "sales@troll.no for pricing and purchasing information." );
	exit( 1 );
#endif
    }
    QString msg(1024);
    msg.sprintf( unscrambleString(37391,trial_data_header,trial_len_header),
		 qVersion() );
    QString expire;
    expire.sprintf( unscrambleString(91573,trial_data_expire,
				     trial_len_expire), ndays );
    trial_messup_2 = 1;
    trial_info = new TrialInfo( msg, trial_name, trial_company,
				trial_email, expire,
				unscrambleString(17517,trial_data_copyright,
				trial_len_copyright) );
    trial_julday = julianDay(year,month,day);
#if defined(_OS_WIN32_)
    today_julday = julianDay(trial_t.wYear,trial_t.wMonth,trial_t.wDay);
#else
    tm *t = localtime( &trial_t );
    today_julday = julianDay(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
#endif
    trial_messup_1 = trial_julday < today_julday;
    trial_messup_2 = trial_julday <= FINAL_TRIALDATE;
#if defined(UNIX)
    QApplication::flushX();
    for ( int i=0; i<10; i++ )
	qApp->processEvents();
#endif
}


int qt_init_usr()
{
    extract_trial_info();
    return 50033;
}
