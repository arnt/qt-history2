#include "qperf.h"
#include <qcstring.h>


#if QT_VERSION < 200
#define QCString QString
#endif


static void cstring_init()
{
}

static int cstring_def_ctor()
{
    int i;
    for ( i=0; i<10000; i++ ) {
	QCString dummy;
    }
    return i;
}

static int cstring_assign_charptr()
{
    int i;
    QCString s;
    const char *ascii = "Sample String To Assign";
    for ( i=0; i<1000; i++ ) {
	s = ascii;
    }
    return i;    
}

static int dummy_getptr( const char *s )
{
    static int x = 0;
    return (x = s[0] + 1);
}

static int cstring_pass_charptr()
{
    int i;
    int x = 0;
    QCString s = "Sample String To Pass";
    for ( i=0; i<1000; i++ ) {
	x += dummy_getptr(s);
    }
    return i;    
}

static int cstring_length()
{
    int i;
    QCString s = "Measure length of me";
    int dummy = 0;
    for ( i=0; i<1000; i++ ) {
	dummy += s.length();
    }
    return i;    
}

static int cstring_lower()
{
    int i;
    QCString s = "CONVERT me To lowerCASE";
    QCString s2;
    for ( i=0; i<1000; i++ ) {
	s2 = s.lower();
    }
    return i;    
}

static int cstring_upper()
{
    int i;
    QCString s = "convert ME tO UPPERcase";
    QCString s2;
    for ( i=0; i<1000; i++ ) {
	s2 = s.upper();
    }
    return i;    
}

static int cstring_simplify_ws()
{
    int i;
    QCString s = "  convert M\t E tO UPPERcase   ";
    QCString s2;
    for ( i=0; i<1000; i++ ) {
	s2 = s.simplifyWhiteSpace();
    }
    return i;    
}

static int cstring_find()
{
    int i;
    QCString s = "This quite long string contains magic here";
    QCString f = "magic";
    for ( i=0; i<1000; i++ ) {
#ifdef QT_4x
	(void)s.indexOf(f);
#else
	(void)s.find(f);
#endif
    }
    return i;    
}

static int cstring_contains()
{
    int i;
    QCString s = "This quite long string contains magic here";
    QCString f = "ng";
    for ( i=0; i<1000; i++ ) {
	(void)s.contains(f);
    }
    return i;    
}

static int cstring_to_int()
{
    int i;
    QCString s = "31456";
    bool ok;
    int x = 0;
    for ( i=0; i<1000; i++ ) {
	x += s.toInt(&ok);
    }
    return i;    
}

static int cstring_set_int()
{
    int i;
    QCString s;
    for ( i=0; i<1000; i++ ) {
	s.setNum(33246);
    }
    return i;    
}

static int cstring_set_int_sprintf()
{
    int i;
    QCString s;
    for ( i=0; i<1000; i++ ) {
	s.sprintf("%d",33246);
    }
    return i;    
}

static int cstring_set_double()
{
    int i;
    QCString s;
    double v = 3.141593;
    for ( i=0; i<1000; i++ ) {
	s.setNum(v);
    }
    return i;    
}

static int cstring_append()
{
    int i;
    QCString s = "This quite long string will become";
    QCString f = " much longer when this string is appended";
    for ( i=0; i<1000; i++ ) {
	s += f;
    }
    return i;
}

QPERF_BEGIN(cstring,"QCString tests")
    QPERF(cstring_def_ctor,"Test default QCString constructor")
    QPERF(cstring_assign_charptr,"Assign const char * to QCString")
    QPERF(cstring_pass_charptr,"Pass const char * to C function")
    QPERF(cstring_length,"Find length() of string")
    QPERF(cstring_lower,"Convert string to lower case")
    QPERF(cstring_upper,"Convert string to upper case")
    QPERF(cstring_simplify_ws,"Simplify whitespace")
    QPERF(cstring_find,"Find substring")
    QPERF(cstring_contains,"Count number of times substring occurrs")
    QPERF(cstring_to_int,"Convert string to integer")
    QPERF(cstring_set_int,"Convert integer to string")
    QPERF(cstring_set_int_sprintf,"Convert integer to string using sprintf")
    QPERF(cstring_set_double,"Convert double to string")
    QPERF(cstring_append,"Append a string to a string")
QPERF_END(cstring)
