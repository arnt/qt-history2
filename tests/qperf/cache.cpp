#include "qperf.h"
#include <qcache.h>
#if QT_VERSION >= 200
#include <qasciicache.h>
#endif
#include "words.inc"

QCache<int>  wordCache(400,601);

#if QT_VERSION < 200
#define wordCacheAscii wordCache
#else
QAsciiCache<int>       wordCacheAscii(400,601);
#endif

static void fill_wordCache()
{
    int i;
    wordCache.clear();
    wordCacheAscii.clear();
    for ( i=0; i<num_words; i++ ) {
	wordCache.insert(words[i],(int*)i+1 );
	wordCacheAscii.insert(words[i],(int*)i+1 );
    }
}


static void cache_init()
{
    fill_wordCache();
}


static int cache_lookup_ascii()
{
    int i;
    for ( i=0; i<1000; i++ ) {
	wordCacheAscii.find("Trolltech");	 // no match
	wordCacheAscii.find("sequences");	 // match
	wordCacheAscii.find("FSF");		 // match
	wordCacheAscii.find("POSIXLYCORRECT"); // match
    }
    return i*4;
}

static int cache_lookup_string()
{
    QString s1("Trolltech");
    QString s2("sequences");
    QString s3("FSF");
    QString s4("POSIXLYCORRECT");
    int i;
    for ( i=0; i<1000; i++ ) {
	wordCache.find(s1);
	wordCache.find(s2);
	wordCache.find(s3);
	wordCache.find(s4);
    }
    return i*4;
}

static int cache_lookup_ascii_string()
{
    QString s1("Trolltech");
    QString s2("sequences");
    QString s3("FSF");
    QString s4("POSIXLYCORRECT");
    int i;
    for ( i=0; i<1000; i++ ) {
	wordCacheAscii.find(s1);
	wordCacheAscii.find(s2);
	wordCacheAscii.find(s3);
	wordCacheAscii.find(s4);
    }
    return i*4;
}

static int cache_lookup_string_ascii()
{
    int i;
    for ( i=0; i<1000; i++ ) {
	wordCache.find("Trolltech");	 // no match
	wordCache.find("sequences");	 // match
	wordCache.find("FSF");		 // match
	wordCache.find("POSIXLYCORRECT"); // match
    }
    return i*4;
}

static int cache_insdel_ascii()
{
    const char *s1 = "Trolltech";
    int i;
    for ( i=0; i<1000; i++ ) {
	wordCacheAscii.insert(s1,(int*)123);
	wordCacheAscii.remove(s1);
    }
    return i*4;
}

static int cache_insdel_string()
{
    QString s1 = "Trolltech";
    int i;
    for ( i=0; i<1000; i++ ) {
	wordCache.insert(s1,(int*)123);
	wordCache.remove(s1);
    }
    return i*4;
}


QPERF_BEGIN(cache,"QCache tests")
    QPERF(cache_lookup_ascii,"QAsciiCache lookup, using const char *")
    QPERF(cache_lookup_string,"QCache lookup, using QString")
    QPERF(cache_lookup_ascii_string,"QAsciiCache lookup, using QString")
    QPERF(cache_lookup_string_ascii,"QCache lookup, using char *")
    QPERF(cache_insdel_ascii,"QCache lookup, using char *")
    QPERF(cache_insdel_string,"QCache lookup, using char *")
QPERF_END(cache)
