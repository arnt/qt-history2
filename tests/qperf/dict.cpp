#include "qperf.h"
#include <qdict.h>
#include <qmap.h>
#include "words.inc"

QDict<int> d;

QDict<int>         wordDict(601);
QMap<QString,int*> wordMap;


static void fill_wordDict()
{
    wordDict.clear();
    for ( int i=0; i<num_words; i++ )
	wordDict.insert(words[i],(int*)i+1 );
}

static void fill_wordMap()
{
    wordMap.clear();
    for ( int i=0; i<num_words; i++ )
	wordMap.insert(words[i],(int*)i+1 );
}


static void dict_init()
{
    fill_wordDict();
    fill_wordMap();
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

static int dict_words_lookup()
{
    int i;
    for ( i=0; i<1000; i++ ) {
	wordDict.find("Troll Tech");	 // no match
	wordDict.find("sequences");	 // match
	wordDict.find("FSF");		 // match
	wordDict.find("POSIXLYCORRECT"); // match
    }
    return i*4;
}

static int dict_words_lookup_map()
{
    QString s1("Troll Tech");
    QString s2("sequences");
    QString s3("FSF");
    QString s4("POSIXLYCORRECT");
    int i;
    for ( i=0; i<1000; i++ ) {
	wordMap[s1];
	wordMap[s2];
	wordMap[s3];
	wordMap[s4];
    }
    return i*4;
}


QPERF_BEGIN(dict,"QDict tests")
    QPERF(dict_lookup,"Tests QDict lookup, using const char *")
    QPERF(dict_lookup_qstring,"Tests QDict lookup, using QString")
    QPERF(dict_words_lookup,"Realistic dict lookup using char*")
    QPERF(dict_words_lookup_map,"Dict lookup, but use a QMap and QString")
QPERF_END(dict)
