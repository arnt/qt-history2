
/* Test program for QRegExp to try to make sure that the 2.0 QRegExp
   behaves sensibly and like the 1.x QRegExp.

   Not a full regexp tester, only a brief exercising of each of the
   case statements in QRegExp::compile() and ::match().
*/

// TODO: 16bit chars testing

#include <stdio.h>
#include "qstring.h"
#include "qregexp.h"

int err=0;
int res = 0;
#define TEST(A,E) \
 if ( (res=(int)(A))!=(E) ) { err++; printf("TEST(%s,%s) failed, line %d. Left side result was %i\n",#A,#E,__LINE__,res);}

// define this to exercise QCString in Qt 2.x
//#define USE_QCSTRING

#if QT_VERSION < 190
#define TESTING_1XSTRING
#else
#ifdef USE_QCSTRING
#define QString QCString
#define TESTING_1XSTRING
#endif
#endif

// Some bugs in 1.x have been fixed in 2.0:
#if QT_VERSION > 190
#define EMPTYRXWORKS
#else
#endif

void test_cclass()
{
  QString string = "[]";
  QString restr1 = "[!,\\.=:\\*()\\-\\[\\]/\\${}a-zA-Z0-9_]";
  QString restr3 = "[\\[\\]]";
  QString restr2 = "[\\]\\[]";
  QString restr4 = "[\\]\\[!,\\.=:\\*()\\-/\\${}a-zA-Z0-9_]";
  QString restr5 = "[\\[\\]!,\\.=:\\*()\\-/\\${}a-zA-Z0-9_]";

  QRegExp re1( restr1 );
  TEST( string.contains( re1 ), 2 );
  QRegExp re2( restr2 );
  TEST( string.contains( re2 ), 2 );
  QRegExp re3( restr3 );
  TEST( string.contains( re3 ), 2 );
  QRegExp re4( restr4 );
  TEST( string.contains( re4 ), 2 );
  QRegExp re5( restr5 );
  TEST( string.contains( re5 ), 2 );
}

void test_16bit()
{
#if QT_VERSION > 190
    QChar ucstr[3];
    ucstr[0] = QChar( 5, 5 );
    ucstr[1] = QChar( 6, 5 );
    ucstr[2] = QChar( 6, 6 );
    QString str( ucstr, 3 );
    
    QChar ucrx[2];
    ucrx[0] = QChar( '^' );
    ucrx[1] = QChar( 5, 5 );
    QString rx( ucrx, 2 );

    QRegExp r( rx );

    TEST( str.find( r ), 0 );
#endif
}

void test_bruteforce( int argc, const char** argv );

int main( int argc, const char** argv )
{

    if ( argc > 1 && !strcmp( argv[1], "-brute" ) ) {
	test_bruteforce( argc, argv );
	return 0;
    }

#ifndef EMPTYRXWORKS
    warning("Not testing Empty regexp; in 1.x it crashes.");
#endif

    test_16bit();

    test_cclass();
    
    int len = -1;

    QRegExp r0;
    QString s2("dill");
    TEST( r0.isEmpty(), TRUE );
    TEST( r0.isValid(), TRUE );		// true in 1.x, shouldn't it be false?
    TEST( r0.wildcard(), FALSE );
    TEST( r0.caseSensitive(), TRUE );
#ifdef EMPTYRXWORKS
    TEST( r0.match( s2 ), -1 );
#endif

    r0 = "[a-z";
    TEST( r0.isEmpty(), TRUE );
    TEST( r0.isValid(), FALSE );
    TEST( r0.wildcard(), FALSE );
    TEST( r0.caseSensitive(), TRUE );
    TEST( r0.match( s2 ), -1 );

    QString s;
    TEST( s.isNull(), TRUE );
    TEST( s.isEmpty(), TRUE );
    TEST( s.find(r0), -1 );
    TEST( s.findRev(r0), -1 );
    TEST( s.contains(r0), 0 );

    s = "en streng";
    TEST( s.find(r0), -1 );
    TEST( s.findRev(r0), -1 );
    TEST( s.contains(r0), 0 );

    r0 = "[a-z]";
    QString s0;
    TEST( s0.find(r0), -1 );
    TEST( s0.findRev(r0), -1 );
    TEST( s0.contains(r0), 0 );
    TEST( r0.match( s0 ), -1 );

    s0 = "";
    TEST( s0.isNull(), FALSE );
    TEST( s0.isEmpty(), TRUE );
    TEST( s0.find(r0), -1 );
    TEST( s0.findRev(r0), -1 );
    TEST( s0.contains(r0), 0 );
    TEST( r0.match( s0 ), -1 );

    // These strings are never changed:
    //                   111111111122222222223
    //         0123456789012345678901234567890
    QString a("Heisan Hoppsan");
    QString b("Heisan Heisan");
    QString c("Heisan heisan");
    QString d("xHeisan heisan");
    QString e("Heisan heisanx");
    QString f("dill  og dall");
    QString g(" Heeei og Hooooopp");
    QString h(" hEeEi og Heeei og hEEEi");
    QString k("A1");
    QString l("1");
    QString ns;
    QString es( "" );
    QRegExp r;

    QRegExp r21( "[a-zA-Z]*" );
    r = r21;
    TEST( r.match( k ), 0 );
    TEST( r.match( l ), 0 );


    QRegExp r20( ".?" );
    r = r20;
    TEST( a.find(r), 0 );
    TEST( a.findRev(r), 13 );
    TEST( a.contains(r), 14 );
    TEST( r.match( ns ), 0 );
    TEST( r.match( es ), 0 );
    TEST( es.find(r), 0 );
    TEST( es.findRev(r), 0 );
    TEST( es.contains(r), 1 );
    TEST( ns.find(r), 0 );
    TEST( ns.findRev(r), 0 );
    TEST( ns.contains(r), 1 );

    QRegExp r19( ".*a" );
    r = r19;
    TEST( r.match( ns ), -1 );
    TEST( r.match( es ), -1 );
    TEST( es.find(r), -1 );
    TEST( es.findRev(r), -1 );
    TEST( es.contains(r), 0 );
    TEST( ns.find(r), -1 );
    TEST( ns.findRev(r), -1 );
    TEST( ns.contains(r), 0 );

    QRegExp r18( ".*" );
    r = r18;
    TEST( a.find(r), 0 );
    TEST( a.findRev(r), 13 );
    TEST( a.contains(r), 14 );
    TEST( r.match( a, 0, &len ), 0 );
    TEST( len, 14 );
    TEST( r.match( ns ), 0 );
    TEST( r.match( es ), 0 );
    TEST( es.find(r), 0 );
    TEST( es.findRev(r), 0 );
    TEST( es.contains(r), 1 );
    TEST( ns.find(r), 0 );
    TEST( ns.findRev(r), 0 );
    TEST( ns.contains(r), 1 );

    QRegExp r17( "^.*$" );
    r = r17;
    TEST( a.find(r), 0 );
    TEST( a.findRev(r), 13 );
    TEST( a.contains(r), 14 );
    TEST( r.match( a, 0, &len ), 0 );
    TEST( len, 14 );
    TEST( r.match( ns ), 0 );
    TEST( r.match( es ), 0 );
    TEST( es.find(r), 0 );
    TEST( es.findRev(r), 0 );
    TEST( es.contains(r), 1 );
    TEST( ns.find(r), 0 );
    TEST( ns.findRev(r), 0 );
    TEST( ns.contains(r), 1 );

    QRegExp r16( "^$" );
    r = r16;
    TEST( a.find(r), -1 );
    TEST( a.findRev(r), -1 );
    TEST( a.contains(r), 0 );
    TEST( r.match( ns ), 0 );
    TEST( r.match( es ), 0 );
    TEST( es.find(r), 0 );
    TEST( es.findRev(r), 0 );
    TEST( es.contains(r), 1 );
    TEST( ns.find(r), 0 );
    TEST( ns.findRev(r), 0 );
    TEST( ns.contains(r), 1 );

    QRegExp r15( "$" );
    r = r15;
    TEST( a.find(r), 14 );
    TEST( r.match( a, 0, &len ), 14 );
    TEST( len, 0 );
    TEST( a.contains(r), 1 );
    TEST( r.match( ns ), 0 );
    TEST( r.match( es ), 0 );
    TEST( es.find(r), 0 );
    TEST( es.findRev(r), 0 );
    TEST( es.contains(r), 1 );
    TEST( ns.find(r), 0 );
    TEST( ns.findRev(r), 0 );
    TEST( ns.contains(r), 1 );

    QRegExp r14( "He?", FALSE );
    r = r14;
    
    TEST( a.find(r), 0 );
    TEST( a.findRev(r), 7 );
    TEST( a.contains(r), 2 );

    TEST( c.find(r), 0 );
    TEST( c.findRev(r), 7 );
    TEST( c.contains(r), 2 );

    TEST( d.find(r), 1 );
    TEST( d.findRev(r), 8 );
    TEST( d.contains(r), 2 );

    TEST( f.find(r), -1 );
    TEST( f.findRev(r), -1 );
    TEST( f.contains(r), 0 );
    
    TEST( r.match( g, 0, &len ), 1 );
    TEST( len, 2 );
    TEST( r.match( a, 0, &len ), 0 );
    TEST( len, 2 );
    TEST( r.match( h, 0, &len ), 1 );
    TEST( len, 2 );
    TEST( r.match( h, 6, &len ), 10 );
    TEST( len, 2 );
    TEST( r.match( h, 15, &len ), 19 );
    TEST( len, 2 );
    TEST( r.match( a, 0, &len ), 0 );
    TEST( len, 2 );
    


    QRegExp r12( "He?" );
    r = r12;
    
    TEST( a.find(r), 0 );
    TEST( a.findRev(r), 7 );
    TEST( a.contains(r), 2 );

    TEST( d.find(r), 1 );
    TEST( d.findRev(r), 1 );
    TEST( d.contains(r), 1 );

    TEST( f.find(r), -1 );
    TEST( f.findRev(r), -1 );
    TEST( f.contains(r), 0 );
    
    TEST( r.match( g, 0, &len ), 1 );
    TEST( len, 2 );
    TEST( r.match( a, 0, &len ), 0 );
    TEST( len, 2 );

    
    QRegExp r11( "He*", FALSE );
    r = r11;
    
    TEST( a.find(r), 0 );
    TEST( a.findRev(r), 7 );
    TEST( a.contains(r), 2 );

    TEST( c.find(r), 0 );
    TEST( c.findRev(r), 7 );
    TEST( c.contains(r), 2 );

    TEST( d.find(r), 1 );
    TEST( d.findRev(r), 8 );
    TEST( d.contains(r), 2 );

    TEST( f.find(r), -1 );
    TEST( f.findRev(r), -1 );
    TEST( f.contains(r), 0 );
    
    TEST( r.match( g, 0, &len ), 1 );
    TEST( len, 4 );
    TEST( r.match( a, 0, &len ), 0 );
    TEST( len, 2 );
    TEST( r.match( h, 0, &len ), 1 );
    TEST( len, 4 );
    TEST( r.match( h, 6, &len ), 10 );
    TEST( len, 4 );
    TEST( r.match( h, 15, &len ), 19 );
    TEST( len, 4 );
    TEST( r.match( a, 0, &len ), 0 );
    TEST( len, 2 );
    

    QRegExp r10( "He*" );
    r = r10;
    
    TEST( a.find(r), 0 );
    TEST( a.findRev(r), 7 );
    TEST( a.contains(r), 2 );

    TEST( d.find(r), 1 );
    TEST( d.findRev(r), 1 );
    TEST( d.contains(r), 1 );

    TEST( f.find(r), -1 );
    TEST( f.findRev(r), -1 );
    TEST( f.contains(r), 0 );
    
    TEST( r.match( g, 0, &len ), 1 );
    TEST( len, 4 );
    TEST( r.match( a, 0, &len ), 0 );
    TEST( len, 2 );


    QRegExp r9( "[xZ-Wnp1-9][\\sa-z]" );
    r = r9;
    
    TEST( a.find(r), 5 );
    TEST( a.findRev(r), 10 );
    TEST( a.contains(r), 3 );

    TEST( b.find(r), 5 );
    TEST( b.findRev(r), 5 );
    TEST( b.contains(r), 1 );

    TEST( f.find(r), -1 );
    TEST( f.findRev(r), -1 );
    TEST( f.contains(r), 0 );

    QRegExp r8( "[H][^A-Z][a-z]s" );
    r = r8;

    TEST( a.find(r), 0 );
    TEST( a.findRev(r), 0 );
    TEST( a.contains(r), 1 );

    TEST( b.find(r), 0 );
    TEST( b.findRev(r), 7 );
    TEST( b.contains(r), 2 );

    TEST( c.find(r), 0 );
    TEST( c.findRev(r), 0 );
    TEST( c.contains(r), 1 );

    TEST( f.find(r), -1 );
    TEST( f.findRev(r), -1 );
    TEST( f.contains(r), 0 );


    QRegExp r7( "H[a-z][a-z]s" );
    r = r7;
    
    TEST( a.find(r), 0 );
    TEST( a.findRev(r), 0 );
    TEST( a.contains(r), 1 );

    TEST( b.find(r), 0 );
    TEST( b.findRev(r), 7 );
    TEST( b.contains(r), 2 );

    TEST( c.find(r), 0 );
    TEST( c.findRev(r), 0 );
    TEST( c.contains(r), 1 );

    TEST( f.find(r), -1 );
    TEST( f.findRev(r), -1 );
    TEST( f.contains(r), 0 );

    QRegExp r6( "san$" );
    r = r6;

    TEST( a.find(r), 11 );
    TEST( a.findRev(r), 11 );
    TEST( a.contains(r), 1 );

    TEST( f.find(r), -1 );
    TEST( f.findRev(r), -1 );
    TEST( f.contains(r), 0 );

    TEST( e.find(r), -1 );
    TEST( e.findRev(r), -1 );
    TEST( e.contains(r), 0 );

    TEST( f.find(r), -1 );
    TEST( f.findRev(r), -1 );
    TEST( f.contains(r), 0 );
    

    // Note different treatment of '^' and '$' in 1.x; '$' means always
    // end-of-string, but '^' is not start-of-string in findRev. 1.x bug?

    QRegExp r5("^Hei");
    TEST( a.find(r5), 0 );
    TEST( a.findRev(r5), 0 );
    TEST( a.contains(r5), 1 );

    TEST( b.find(r5), 0 );
    TEST( b.findRev(r5), 7 );		// 7 in 1.x; should be 0?
    TEST( b.contains(r5), 1 );

    TEST( c.find(r5), 0 );
    TEST( c.findRev(r5), 0 );
    TEST( c.contains(r5), 1 );

    TEST( d.find(r5), -1 );
    TEST( d.findRev(r5), 1 );		// 1 in 1.x, should be -1?
    TEST( d.contains(r5), 0 );
    

    // The '.' special char
    QRegExp r4( "H..s", FALSE );
    
    TEST( a.find(r4), 0 );
    TEST( a.findRev(r4), 0 );
    TEST( a.contains(r4), 1 );

    TEST( b.find(r4), 0 );
    TEST( b.findRev(r4), 7 );
    TEST( b.contains(r4), 2 );

    TEST( c.find(r4), 0 );
    TEST( c.findRev(r4), 7 );
    TEST( c.contains(r4), 2 );


    QRegExp r3( "H..s" );
    
    TEST( a.find(r3), 0 );
    TEST( a.findRev(r3), 0 );
    TEST( a.contains(r3), 1 );

    TEST( b.find(r3), 0 );
    TEST( b.findRev(r3), 7 );
    TEST( b.contains(r3), 2 );

    TEST( c.find(r3), 0 );
    TEST( c.findRev(r3), 0 );
    TEST( c.contains(r3), 1 );


    // Basic character matching, no special chars
    QRegExp r1("Hei");
    TEST( a.find(r1), 0 );
    TEST( a.findRev(r1), 0 );
    TEST( a.contains(r1), 1 );

    TEST( b.find(r1), 0 );
    TEST( b.findRev(r1), 7 );
    TEST( b.contains(r1), 2 );


    QRegExp r2 = r1;
    TEST( r1 == r2, TRUE );
    r2.setCaseSensitive( FALSE );
    TEST( r1 != r2, TRUE );

    TEST( c.find(r1), 0 );
    TEST( c.findRev(r1), 0 );
    TEST( c.contains(r1), 1 );

    TEST( c.find(r2), 0 );
    TEST( c.findRev(r2), 7 );
    TEST( c.contains(r2), 2 );
    


    warning("\n%d error%s\n",err,"s"+(err==1));

    return 0;
}




void test_bruteforce( int argc, const char** argv )
{
    const char* chars = ".^$[]*+?\\rnts";
    const int maxIdx = strlen( chars ) - 1;
    const int l = argc - 2;
    if ( !l ) {
	warning( "Cannot test 0 length regexps" );
	return;
    }
    int* idx = new int[l];
    for ( int i = 0; i < l; i++ ) {
	QString arg( argv[i+2] );
	bool ok = TRUE;
	idx[i] = arg.toInt( &ok );
	if ( !ok ) {
	    warning( "Argument %i not a number", i+1 );
	    return;
	}
    }

    QString lmsg = "BF: ";
    QString nums;
    QString msg;
    QString ns;
    QString es( "" );
    QString fs( "A normal string." );
    int maxCnt = 10000;
    int cnt = maxCnt;
    bool done = FALSE;
    while( !done ) {

	// build RE
	QString rxs;
	for ( int i = 0; i < l; i++ )
	    rxs += chars[idx[i]];

	// output
	if ( !cnt-- ) {
	    cnt = maxCnt;
	    msg = lmsg;
	    for ( int i = 0; i < l; i++ ) {
		nums.setNum( idx[i] );
		msg += nums;
		msg += " ";
	    }
	    msg += "RE: ";
	    msg += rxs;
	    warning( "%s", (const char*)msg );
	}
	// do test
	
	QRegExp rx( rxs );
	if ( rx.isValid() ) {
	    int res = rx.match( ns );
	    res = rx.match( es );
	    res = rx.match( fs );
	    res = rx.match( rxs );	// hehe...
	}

	// Increment
	for ( int i = l-1; i >= 0; i-- ) {
	    if ( idx[i] == maxIdx ) {
		if ( !i )
		    done = TRUE;
		idx[i] = 0;
		continue;
	    }
	    idx[i]++;
	    break;
	}
    }

    warning( "Brute Force test of length %i completed", l );

}
    
