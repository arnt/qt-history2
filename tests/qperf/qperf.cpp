#include "qperf.h"
#include <qapplication.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qlist.h>
#include <qdict.h>
#include <qdatetime.h>
#if defined(_OS_WIN32_)
#include <conio.h>
#endif


#if QT_VERSION < 200
#define qDebug debug
#endif


int qrnd_val = 1;	// qrnd seed

int max_iter = 0;
int max_time = 1000;

bool        dblbuf = FALSE;
QWidget    *widget;
QPixmap    *pixmap;
QPainter   *painter = 0;
const char *image_format = "BMP";


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


typedef QList<QPerfEntry>	  QPerfList;
typedef QListIterator<QPerfEntry> QPerfListIt;
typedef QDict<QPerfEntry>	  QPerfDict;
typedef QDictIterator<QPerfEntry> QPerfDictIt;

static QPerfList *perf_list;
static QPerfDict *perf_dict;


QPerfTableInit::QPerfTableInit( QPerfEntry *t )
{
    if ( !perf_list )
	perf_list = new QPerfList;
    if ( !perf_dict )
	perf_dict = new QPerfDict(211);
    perf_list->append( t );
    while ( t->funcName ) {
	perf_dict->insert( t->funcName, t );
	t++;
    }
}


void run_test( const char *funcName )
{
    QPerfEntry *e = perf_dict->find(funcName);
    if ( !e ) {
	qDebug( "qperf: No such test '%s'", funcName );
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
	int t;
	while ( TRUE ) {
	    i += (*e->funcPtr)();
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
    qDebug( "qperf [options] <tests>" );
    // print options
    qDebug( "  Options:" );
    qDebug( "    -db                       Set/toggle double buffering" );
    qDebug( "    -i  <iterations>          Set max # iterations (approx)" );
    qDebug( "    -if <image format>        Set image format for saving images" );
    qDebug( "    -p                        Pause after program is finished" );
    qDebug( "    -po <no|normal|best>      Set default pixmap optimization" );
    qDebug( "    -r  <region type>         Set painter clipping region" );
    qDebug( "                                none, rect, rects, polygon, ellipse" );
    qDebug( "    -t  <milliseconds>        Set max time to run test (approx)");
    qDebug( "    -w  <95|98|nt>            Set Windows version" );
    qDebug( "    -xf <xform type>          Set painter transformation:" );
    qDebug( "                                none, translate, scale, rotate" );
    qDebug( "  Tests:" );
    QPerfEntry *t;
    QPerfListIt it(*perf_list);
    while ( (t=it.current()) ) {
	++it;
	while ( t->funcName ) {
	    qDebug( "    %-25s %s", t->funcName, t->description );
	    ++t;
	}
    }
    exit( 1 );
}
	

#if defined(_WS_WIN_)
extern Q_EXPORT Qt::WindowsVersion qt_winver;
#endif


const char *painter_xform = 0;
const char *painter_clip  = 0;

void setupPainter()
{
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
	    qDebug( "qperf: Bad xform argument %s", painter_xform );
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
	    qDebug( "qperf: Bad clipping argument %s", painter_clip );
	}
    }
}


int main( int argc, char **argv )
{
    if ( argc < 2 )
	usage();

    QApplication app(argc,argv);
    widget = new QWidget;
    app.setMainWidget( widget );
    widget->setBackgroundColor( Qt::white );
    widget->setGeometry( 0, 0, 640, 480 );
    pixmap = new QPixmap( 640, 480 );
    pixmap->fill( Qt::white );
    widget->show();

    bool pause = FALSE;
    bool dirtyPainter = TRUE;

    int i = 1;
    while ( i < argc ) {
	char *a = argv[i++];
	if ( a[0] == '-' ) {			// parse options
	    a = &a[1];
	    if ( strcmp(a,"db") == 0 ) {	// toggle double buffering
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
		    qDebug("Invalid pixmap optimization setting");
		}
	    } else if ( strcmp(a,"r") == 0 ) {	// set clip region
		dirtyPainter = TRUE;
	        painter_clip = argv[i++];
	    } else if ( strcmp(a,"t") == 0 ) {	// set max time
		max_time = atoi(argv[i++]);
		max_iter = 0;
	    } else if ( strcmp(a,"w") == 0 ) {	// set Windows version
		a = argv[i++];
#if defined(_WS_WIN_)
		if ( strcmp(a,"95") == 0 ) {
		    qt_winver = Qt::WV_95;
		} else if ( strcmp(a,"98") == 0 ) {
		    qt_winver = Qt::WV_98;
		} else if ( strcmp(a,"nt") == 0 ) {
		    qt_winver = Qt::WV_NT;
		} else {
		    qDebug("Invalid Windows version setting");
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
    delete painter;
    delete widget;
    delete pixmap;
    delete perf_dict;
    delete perf_list;
    if ( pause ) {
	qDebug("Press a key to continue...");
	getch();
    }
    return 0;
}


