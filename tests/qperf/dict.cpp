#include "qperf.h"
#include <qdict.h>
#if QT_VERSION >= 200
#include <qmap.h>
#endif
#include "words.inc"

QDict<int> wordDict(601);
#if QT_VERSION >= 200
QMap<QString,int*> wordMap;
#endif


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


static int dict_lookup_pchar()
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

static int dict_lookup()
{
    QString s1("Troll Tech");
    QString s2("sequences");
    QString s3("FSF");
    QString s4("POSIXLYCORRECT");
    int i;
    for ( i=0; i<1000; i++ ) {
	wordDict.find(s1);
	wordDict.find(s2);
	wordDict.find(s3);
	wordDict.find(s4);
    }
    return i*4;
}

static int dict_lookup_map()
{
#if QT_VERSION >= 200
    QString s1("Troll Tech");
    QString s2("sequences");
    QString s3("FSF");
    QString s4("POSIXLYCORRECT");
    int i;
    for ( i=0; i<1000; i++ ) {
	wordMap.find(s1);
	wordMap.find(s2);
	wordMap.find(s3);
	wordMap.find(s4);
    }
    return i*4;
#endif
}

static int dict_insdel()
{
    const char *s1 = "Troll Tech";
    int i;
    for ( i=0; i<1000; i++ ) {
	wordDict.insert(s1,(int*)123);
	wordDict.remove(s1);
    }
    return i*4;
}

static int dict_insdel_map()
{
#if QT_VERSION >= 200
    QString s1("Troll Tech");
    int i;
    for ( i=0; i<1000; i++ ) {
	wordMap.insert(s1,(int*)123);
	wordMap.remove(s1);
    }
    return i*4;
#endif
}


QPERF_BEGIN(dict,"QDict tests")
    QPERF(dict_lookup_pchar,"Tests QDict lookup, using const char *")
    QPERF(dict_lookup,"QDict lookup, using QString")
    QPERF(dict_lookup_map,"QMap lookup, using QString")
    QPERF(dict_insdel,"Insert and delete const char *")
    QPERF(dict_insdel_map,"Insert and delete for QMap")
QPERF_END(dict)
