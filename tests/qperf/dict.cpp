#include "qperf.h"
#include <qdict.h>


QDict<int> d;

static void dict_init()
{
}

static int dict_lookup()
{
    int i;
    for ( i=0; i<10000; i++ ) {
	d.find("Troll Tech");
    }
    return i;
}

static int dict_lookup_qstring()
{
    int i;
    QString s("Troll Tech");
    for ( i=0; i<10000; i++ ) {
	d.find(s);
    }
    return i;
}


QPERF_BEGIN(dict,"QDict tests")
    QPERF(dict_lookup,"Tests QDict lookup, using const char *")
    QPERF(dict_lookup_qstring,"Tests QDict lookup, using QString")
QPERF_END(dict)
