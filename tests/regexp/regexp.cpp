
/* Test program for QRegExp to try to make sure that the 2.0 QRegExp
   behaves sensibly and like the 1.x QRegExp.

   Not a full regexp tester, only a brief excercising of each of the
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


main()
{
    
#ifndef EMPTYRXWORKS
    warning("Not testing Empty regexp; in 1.x it crashes.");
#endif

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
    QRegExp r;

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


    // Basic charcter matching, no special chars
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
    



    printf("\n%d error%s\n",err,"s"+(err==1));

}
