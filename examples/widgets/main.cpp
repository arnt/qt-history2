/****************************************************************************
** $Id: //depot/qt/main/examples/widgets/main.cpp#4 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include "widgets.h"
#include <qmotifstyle.h>
#include <qcdestyle.h>
#include <qwindowsstyle.h>
#include <qplatinumstyle.h>
#include <qsgistyle.h>

class MyWidgetView : public WidgetView
{
    int s;
public:
    MyWidgetView( QWidget *parent=0, const char *name=0 )
	:WidgetView(parent, name), s(0)
	{
	}

    void button1Clicked() {
	s++;
	switch (s%5){
#if QT_FEATURE_STYLE_MOTIF
	case 0:
	    qApp->setStyle(new QMotifStyle);
	    break;
#endif
#if QT_FEATURE_STYLE_CDE
	case 1:
	    qApp->setStyle(new QCDEStyle);
	    break;
#endif
#if QT_FEATURE_STYLE_WINDOWS
	case 2:
	    qApp->setStyle(new QWindowsStyle);
	    break;
#endif
#if QT_FEATURE_STYLE_PLATINUM
	case 3:
	    qApp->setStyle(new QPlatinumStyle);
	    break;
#endif
#if QT_FEATURE_STYLE_SGI
	case 4:
	    qApp->setStyle(new QSGIStyle);
#endif
	}
	WidgetView::button1Clicked();
    }
};


//
// Create and display our WidgetView.
//

int main( int argc, char **argv )
{
    QApplication::setColorSpec( QApplication::CustomColor );
    QApplication a( argc, argv );
    QApplication::setStyle( new QWindowsStyle() );

    MyWidgetView* w = new MyWidgetView;
    a.setMainWidget( w );

    w->show();
    int res = a.exec();
    delete w;
    return res;
}
