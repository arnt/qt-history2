#include "qperf.h"
#include <qstring.h>


static void string_init()
{
}

static int string_def_ctor()
{
    int i;
    for ( i=0; i<10000; i++ ) {
	QString dummy;
    }
    return i;
}

static int string_assign_charptr()
{
    int i;
    QString s;
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

static int string_pass_charptr()
{
    int i;
    int x = 0;
    QString s = "Sample String To Pass";
    for ( i=0; i<1000; i++ ) {
	x += dummy_getptr(s);
    }
    return i;    
}

static int string_length()
{
    int i;
    QString s = "Measure length of me";
    int dummy = 0;
    for ( i=0; i<1000; i++ ) {
	dummy += s.length();
    }
    return i;    
}

static int string_lower()
{
    int i;
    QString s = "CONVERT me To lowerCASE";
    QString s2;
    for ( i=0; i<1000; i++ ) {
#ifdef QT_4x
	s2 = s.toLower();
#else
	s2 = s.lower();
#endif
    }
    return i;    
}

static int string_upper()
{
    int i;
    QString s = "convert ME tO UPPERcase";
    QString s2;
    for ( i=0; i<1000; i++ ) {
#ifdef QT_4x
	s2 = s.toUpper();
#else
	s2 = s.upper();
#endif
    }
    return i;    
}

static int string_simplify_ws()
{
    int i;
    QString s = "  convert M\t E tO UPPERcase   ";
    QString s2;
    for ( i=0; i<1000; i++ ) {
#ifdef QT_4x
	s2 = s.simplified();
#else
	s2 = s.simplifyWhiteSpace();
#endif
    }
    return i;    
}

static int string_find()
{
    int i;
    QString s = "This quite long string contains magic here";
    QString f = "magic";
    for ( i=0; i<1000; i++ ) {
#ifdef QT_4x
	(void)s.indexOf(f);
#else
	(void)s.find(f);
#endif
    }
    return i;    
}

static int string_contains()
{
    int i;
    QString s = "This quite long string contains magic here";
    QString f = "ng";
    for ( i=0; i<1000; i++ ) {
	(void)s.contains(f);
    }
    return i;    
}

static int string_to_int()
{
    int i;
    QString s = "31456";
    bool ok;
    int x = 0;
    for ( i=0; i<1000; i++ ) {
	x += s.toInt(&ok);
    }
    return i;    
}

static int string_set_int()
{
    int i;
    QString s;
    for ( i=0; i<1000; i++ ) {
	s.setNum(33246);
    }
    return i;    
}

static int string_set_int_sprintf()
{
    int i;
    QString s;
    for ( i=0; i<1000; i++ ) {
	s.sprintf("%d",33246);
    }
    return i;    
}

static int string_set_double()
{
    int i;
    QString s;
    double v = 3.141593;
    for ( i=0; i<1000; i++ ) {
	s.setNum(v);
    }
    return i;    
}

static int string_append()
{
    int i;
    QString s = "This quite long string will become";
    QString f = " much longer when this string is appended";
    for ( i=0; i<1000; i++ ) {
	s += f;
    }
    return i;
}
   

QPERF_BEGIN(string,"QString tests")
    QPERF(string_def_ctor,"Test default QString constructor")
    QPERF(string_assign_charptr,"Assign const char * to QString")
    QPERF(string_pass_charptr,"Pass const char * to C function")
    QPERF(string_length,"Find length() of string")
    QPERF(string_lower,"Convert string to lower case")
    QPERF(string_upper,"Convert string to upper case")
    QPERF(string_simplify_ws,"Simplify whitespace")
    QPERF(string_find,"Find substring")
    QPERF(string_contains,"Count number of times substring occurrs")
    QPERF(string_to_int,"Convert string to integer")
    QPERF(string_set_int,"Convert integer to string")
    QPERF(string_set_int_sprintf,"Convert integer to string using sprintf")
    QPERF(string_set_double,"Convert double to string")
    QPERF(string_append,"Append a string to a string")
QPERF_END(string)
