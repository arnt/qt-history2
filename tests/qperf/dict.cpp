#include "qperf.h"
#include <qdict.h>
#if QT_VERSION >= 200
#include <qasciidict.h>
#include <qmap.h>
#endif
#include "words.inc"

QDict<int>  wordDict(601);

#if QT_VERSION < 200
#define wordDictAscii wordDict
#define wordMap       wordDict
#else
QAsciiDict<int>       wordDictAscii(601);
QMap<QString,int*>    wordMap;
#endif


static void fill_wordDict()
{
    wordDict.clear();
    wordDictAscii.clear();
    int i;
    for ( i=0; i<num_words; i++ ) {
	wordDict.insert(words[i],(int*)i+1 );
	wordDictAscii.insert(words[i],(int*)i+1 );
    }
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


static int dict_lookup_ascii()
{
    int i;
    for ( i=0; i<1000; i++ ) {
	wordDictAscii.find("Trolltech");	 // no match
	wordDictAscii.find("sequences");	 // match
	wordDictAscii.find("FSF");		 // match
	wordDictAscii.find("POSIXLYCORRECT"); // match
    }
    return i*4;
}

static int dict_lookup_string()
{
    QString s1("Trolltech");
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

static int dict_lookup_ascii_string()
{
    QString s1("Trolltech");
    QString s2("sequences");
    QString s3("FSF");
    QString s4("POSIXLYCORRECT");
    int i;
    for ( i=0; i<1000; i++ ) {
	wordDictAscii.find(s1);
	wordDictAscii.find(s2);
	wordDictAscii.find(s3);
	wordDictAscii.find(s4);
    }
    return i*4;
}

static int dict_lookup_string_ascii()
{
    int i;
    for ( i=0; i<1000; i++ ) {
	wordDict.find("Trolltech");	 // no match
	wordDict.find("sequences");	 // match
	wordDict.find("FSF");		 // match
	wordDict.find("POSIXLYCORRECT"); // match
    }
    return i*4;
}

static int dict_lookup_map()
{
    QString s1("Trolltech");
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
}

static int dict_insdel_ascii()
{
    const char *s1 = "Trolltech";
    int i;
    for ( i=0; i<1000; i++ ) {
	wordDictAscii.insert(s1,(int*)123);
	wordDictAscii.remove(s1);
    }
    return i*4;
}

static int dict_insdel_string()
{
    QString s1 = "Trolltech";
    int i;
    for ( i=0; i<1000; i++ ) {
	wordDict.insert(s1,(int*)123);
	wordDict.remove(s1);
    }
    return i*4;
}

static int dict_insdel_map()
{
    QString s1 = "Trolltech";
    int i;
    for ( i=0; i<1000; i++ ) {
	wordMap.insert(s1,(int*)123);
	wordMap.remove(s1);
    }
    return i*4;
}


QPERF_BEGIN(dict,"QDict tests")
    QPERF(dict_lookup_ascii,"QAsciiDict lookup, using const char *")
    QPERF(dict_lookup_string,"QDict lookup, using QString")
    QPERF(dict_lookup_ascii_string,"QAsciiDict lookup, using QString")
    QPERF(dict_lookup_string_ascii,"QDict lookup, using char *")
    QPERF(dict_lookup_map,"QMap lookup, using QString")
    QPERF(dict_insdel_ascii,"Insert and delete QAsciiDict/char*")
    QPERF(dict_insdel_string,"Insert and delete QDict/QString")
    QPERF(dict_insdel_map,"Insert and delete for QMap")
QPERF_END(dict)
