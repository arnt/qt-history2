// qperf - Qt performance testing


#ifndef QPERF_H
#define QPERF_H

#include <qglobal.h>
#include <qpainter.h>
#include <stdio.h>
#include <stdlib.h>


#if QT_VERSION >= 0x040000
#define QT_4x
#elif QT_VERSION >= 0x030000
#define QT_3x
#elif QT_VERSION >= 200
#define QT_2x
#else
#define QT_1x
#define Qt
#endif

/*
  Outputs a message to stderr, compatible with printf.
*/
void output( const char *, ... );

/*
  This pseudo-random routine is far from perfect in giving random-like
  numbers, but it is pretty fast, which is more important for our tests.
*/
#if defined(Q_CC_MSVC) || defined(Q_OS_MAC)
static				// Can't inline, compiler bug
#else
inline
#endif
uint qrnd( uint maxVal )
{
    static uint qrnd_val = 1;
    qrnd_val = qrnd_val*214013 + 2531011;
    return qrnd_val % maxVal;
}


/*
  Returns the top level widget.
*/
QWidget *qperf_widget();

/*
  Returns the output paint device. This is normally a 640x480 widget,
  but it can be a 640x480 pixmap if double buffering is on.
*/
QPaintDevice *qperf_paintDevice();


/*
  Returns the current painter object, opened on the paint device.
*/
QPainter *qperf_painter();

/*
  Returns the max number of colors and array of colors.
*/
int     qperf_maxColors();
QColor *qperf_colors();

/*
  Return the image format and image extension.
*/
QString qperf_imageFormat();
QString qperf_imageExt();


/*
  Macros and internal stuff below.
*/

struct QPerfEntry {
    const char *funcName;
    int	      (*funcPtr)();
    const char *description;
    bool	group;
};

class QPerfTableInit {
public:
    QPerfTableInit(QPerfEntry*);
};

/*
  These macros are used to build tables of testing functions.
*/
#define QPERF_BEGIN(t,d) QPerfEntry perf__##t[] = { {#t,(int(*)())t##_init,d,1},
#define QPERF(f,d)	 {#f,f,d,0},
#define QPERF_END(t)     {0,0,0,0}};static QPerfTableInit init__##t(perf__##t);


#endif // QPERF_H
