#include "qperf.h"
#include <qwidget.h>


static void widget_init()
{
}

static int widget_create()
{
    int i;
    QWidget *parent = qperf_widget();
    for ( i=0; i<1000; i++ ) {
	QWidget *child = new QWidget(parent);
	delete child;
    }
    return i;
}

static int widget_showhide()
{
    int i;
    QWidget *widget = qperf_widget();
    for ( i=0; i<10; i++ ) {
	widget->show();
	widget->hide();
    }
    return i;
}

QPERF_BEGIN(widget,"QWidget tests")
    QPERF(widget_create,"Create and delete widget")
    QPERF(widget_showhide,"Show and hide the top level widget")
QPERF_END(widget)
