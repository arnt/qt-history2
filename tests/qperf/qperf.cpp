#include "qperf.h"
#include <qapplication.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qdict.h>
#include <qdatetime.h>
#if defined(Q_OS_WIN32)
#include <conio.h>
#endif
#include <stdarg.h>


void output( const char *msg, ... )
{
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    vfprintf( stderr, msg, ap );
    va_end( ap );
    fprintf( stderr, "\n" );			// add newline
}


static int max_iter = 0;
static int max_time = 1000;

static bool        dblbuf = FALSE;
static QWidget    *widget;
static QPixmap    *pixmap;
static QPainter   *painter = 0;
static int	   max_colors = 16;
static QColor	  *colors = 0;
static const char *image_format = "BMP";


QWidget *qperf_widget()
{
    return widget;
}

QPaintDevice *qperf_paintDevice()
{
    if ( dblbuf )
	return pixmap;
    else
	return widget;
}

QPainter *qperf_painter()
{
    return painter;
}

int qperf_maxColors()
{
    return max_colors;
}

QColor *qperf_colors()
{
    return colors;
}

QString qperf_imageFormat()
{
    return image_format;
}

QString qperf_imageExt()
{
    QString s = ".";
    s += image_format;
    s = s.lower();
    return s;
}


#if QT_VERSION < 200
typedef GCI Item;
#endif

class QPerfList : public QPtrList<QPerfEntry>
{
public:
    QPerfList() {}
   ~QPerfList() { clear(); }
protected:
    int	  compareItems( Item s1, Item s2 )
	{ return strcmp(((QPerfEntry*)s1)->funcName,
			((QPerfEntry*)s2)->funcName); }
};

typedef QPtrListIterator<QPerfEntry> QPerfListIt;
typedef QDict<QPerfEntry>	     QPerfDict;
typedef QDictIterator<QPerfEntry>    QPerfDictIt;

static QPerfList *perf_list;
static QPerfDict *perf_dict;


QPerfTableInit::QPerfTableInit( QPerfEntry *t )
{
    if ( !perf_list )
	perf_list = new QPerfList;
    if ( !perf_dict )
	perf_dict = new QPerfDict(211);
    perf_list->inSort( t );
    while ( t->funcName ) {
	perf_dict->insert( t->funcName, t );
	t++;
    }
}


void run_test( const char *funcName )
{
    QPerfEntry *e = perf_dict->find(funcName);
    if ( !e ) {
	output( "qperf: No such test '%s'", funcName );
	return;
    }
    QPerfEntry *p = e;
    int numTests;
    if ( p->group ) {
	// This is a group test, e.g. "string", which includes several
	// sub tests.  The perf entries are stored in a table and comes
	// right after p.  We set numTests to a high number and can
	// safely assume that no group includes more than 9999 tests.
	e++;
	numTests = 9999;
    } else {
	// Run a single test.  We scan the table backwards to find the
	// first (group) node, since we need to call the init function.
	p = e;
	while ( !p->group )
	    --p;
	numTests = 1;
    }
    if ( p->funcPtr ) {				// run init function?
	(*p->funcPtr)();
	p->funcPtr = 0;				// run only once
    }
    while ( e->funcPtr && numTests > 0 ) {
	printf("%s...\n", e->funcName );
	QTime time;
	time.start();
	int i = 0;
	int t = 0;
	while ( TRUE ) {
	    i += (*e->funcPtr)();
	    QApplication::syncX();
	    t = time.elapsed();
	    if ( max_iter && i >= max_iter )
		break;
	    if ( max_time && t >= max_time )
		break;
	}
	printf( "  Iterations     = %8d\n", i );
	printf( "  Time (msec)    = %8d\n", t );
	int perf;
	if ( t == 0 )
	    perf = -999999999;
	else if ( i > 2000000 )			// avoid overflow
	    perf = (i/t)*1000;
	else
	    perf = (i*1000)/t;
	printf( "  Iterations/sec = %8d\n", perf );
	if ( dblbuf )
	    bitBlt( widget, 0, 0, pixmap );
	e++;
	numTests--;
    }
}


void usage()
{
    output( "qperf [options] <tests>" );
    // print options
    output( "\nOptions:" );
    output( "    -c  <numcolors>           Use colors for painter tests" );
    output( "    -db                       Set/toggle double buffering" );
    output( "    -i  <iterations>          Set max # iterations (approx)" );
    output( "    -if <image format>        Set image format for saving images" );
    output( "    -p                        Pause after program is finished" );
    output( "    -po <no|normal|best>      Set default pixmap optimization" );
    output( "    -r  <region type>         Set painter clipping region" );
    output( "                                none, rect, rects, polygon, ellipse" );
    output( "    -t  <milliseconds>        Set max time to run test (approx)");
    output( "    -w  <95|98|nt>            Set Windows version" );
    output( "    -xf <xform type>          Set painter transformation:" );
    output( "                                none, translate, scale, rotate" );
    output( "Tests:" );
    QPerfEntry *t;
    QPerfListIt it(*perf_list);
    while ( (t=it.current()) ) {
	++it;
	while ( t->funcName ) {
	    if ( t->group )
		output( "  %-27s %s", t->funcName, t->description );
	    else
		output( "    %-25s %s", t->funcName, t->description );
	    ++t;
	}
    }
    exit( 1 );
}


void qrnd_init()
{
    // No initialization
}

int qrnd_speed()
{
    int i;
    for ( i=0; i<100000; i++ ) {
	qrnd(640);
	qrnd(48);
    }
    return i*2;
}

QPERF_BEGIN(qrnd,"qrnd test (to manually correct qperf results)")
    QPERF(qrnd_speed,"Tests the speed of the qrnd function")
QPERF_END(qrnd)


#if defined(Q_WS_WIN)
extern Q_EXPORT Qt::WindowsVersion qt_winver;
#endif


const char *painter_xform = 0;
const char *painter_clip  = 0;

void setupPainter()
{
    if ( colors )
	delete [] colors;
    colors = new QColor[max_colors];
    colors[0] = Qt::black;
    int i;
    for ( i=1; i<max_colors; i++ ) {
	colors[i] = QColor(qrnd(255),qrnd(255),qrnd(255));
    }
    if ( painter )
	painter->end();
    else
	painter = new QPainter;
    painter->begin( qperf_paintDevice() );
    if ( painter_xform ) {
	if ( strcmp(painter_xform,"none") == 0 ) {
	    // nothing
	} else if ( strcmp(painter_xform,"translate") == 0 ) {
	    painter->translate( 8, 10 );
	} else if ( strcmp(painter_xform,"scale") == 0 ) {
	    painter->scale( 1.1, 1.2 );
	} else if ( strcmp(painter_xform,"rotate") == 0 ) {
	    painter->rotate( 90 );
	    painter->translate( 0, -480 );
	} else {
	    output( "qperf: Bad xform argument %s", painter_xform );
	}
    }
    if ( painter_clip ) {
	if ( strcmp(painter_clip,"none") == 0 ) {
	    // nothing
	} else if ( strcmp(painter_clip,"rect") == 0 ) {
	    painter->setClipRect( 20, 240, 600, 200 );
	} else if ( strcmp(painter_clip,"rects") == 0 ) {
	    QRegion r1(0,0,200,200);
	    QRegion r2(0,280,200,200);
	    QRegion r3(440,0,200,200);
	    QRegion r4(440,280,200,200);
	    QRegion r = r1.unite(r2).unite(r3).unite(r4);
	    painter->setClipRegion( r );
	} else if ( strcmp(painter_clip,"polygon") == 0 ) {
	    QPointArray a(6);
	    a[0] = QPoint(0,0);
	    a[1] = QPoint(318,240);
	    a[2] = QPoint(0,480);
	    a[3] = QPoint(640,480);
	    a[4] = QPoint(322,240);
	    a[5] = QPoint(640,0);
	    painter->setClipRegion( a );
	} else if ( strcmp(painter_clip,"ellipse") == 0 ) {
	    painter->setClipRegion( QRegion(0,0,640,480,QRegion::Ellipse).
				    subtract(QRegion(220,140,200,200,
						     QRegion::Ellipse)) );
	} else {
	    output( "qperf: Bad clipping argument %s", painter_clip );
	}
    }
}


bool pause = FALSE;

void run_main( int argc, char **argv )
{
    bool dirtyPainter = TRUE;

    int i = 1;
    while ( i < argc ) {
	char *a = argv[i++];
	if ( a[0] == '-' ) {			// parse options
	    a = &a[1];
	    if ( strcmp(a,"c") == 0 ) {		// use random colors
		max_colors = atoi(argv[i++]);
	    } else if ( strcmp(a,"db") == 0 ) {	// toggle double buffering
		dblbuf = !dblbuf;
		dirtyPainter = TRUE;
	    } else if ( strcmp(a,"i") == 0 ) {	// set max iterations
		max_iter = atoi(argv[i++]);
		max_time = 0;
	    } else if ( strcmp(a,"if") == 0 ) {	// set image format
		image_format = argv[i++];
	    } else if ( strcmp(a,"p") == 0 ) {	// pause
		pause = !pause;
	    } else if ( strcmp(a,"po") == 0 ) {	// set pixmap optimization
		a = argv[i++];
		if ( strcmp(a,"no") == 0 ) {
		    QPixmap::setDefaultOptimization( QPixmap::NoOptim );
		} else if ( strcmp(a,"normal") == 0 ) {
		    QPixmap::setDefaultOptimization( QPixmap::NormalOptim );
		} else if ( strcmp(a,"best") == 0 ) {
		    QPixmap::setDefaultOptimization( QPixmap::BestOptim );
		} else {
		    output("Invalid pixmap optimization setting");
		}
	    } else if ( strcmp(a,"r") == 0 ) {	// set clip region
		dirtyPainter = TRUE;
	        painter_clip = argv[i++];
	    } else if ( strcmp(a,"t") == 0 ) {	// set max time
		max_time = atoi(argv[i++]);
		max_iter = 0;
	    } else if ( strcmp(a,"w") == 0 ) {	// set Windows version
		a = argv[i++];
#if defined(Q_WS_WIN)
		if ( strcmp(a,"95") == 0 ) {
		    qt_winver = Qt::WV_95;
		} else if ( strcmp(a,"98") == 0 ) {
		    qt_winver = Qt::WV_98;
		} else if ( strcmp(a,"nt") == 0 ) {
		    qt_winver = Qt::WV_NT;
		} else {
		    output("Invalid Windows version setting");
		}
#endif
	    } else if ( strcmp(a,"xf") == 0 ) {	// set painter xform
		painter_xform = argv[i++];
		dirtyPainter = TRUE;
	    } else {
		usage();
	    }
	} else {
	    if ( dirtyPainter ) {
		setupPainter();
		dirtyPainter = FALSE;
	    }
	    run_test(a);
	}
    }
    if ( painter ) {
	painter->end();
	delete painter;
    }
    if ( colors )
	delete [] colors;
    delete widget;
    delete pixmap;
    delete perf_dict;
    delete perf_list;
}

class Runner : public QObject
{
public:
    Runner()
    {
	startTimer(10);
    }
protected:
    void timerEvent( QTimerEvent * )
    {
	killTimers();
	run_main(qApp->argc(),qApp->argv());
    }
};


int main( int argc, char **argv )
{
    if ( argc < 2 )
	usage();
    QApplication a(argc,argv);
    widget = new QWidget;
    qApp->setMainWidget( widget );
    widget->setBackgroundColor( Qt::white );
    widget->setGeometry( 20, 20, 640, 480 );
    pixmap = new QPixmap( 640, 480 );
    pixmap->fill( Qt::white );
    widget->show();

    Runner r;
    a.exec();

    if ( pause ) {
	output("Press a key to continue...");
#if defined(Q_OS_WIN32)
	getch();
#else
	getc(stdin);
#endif
    }
    return 0;
}


