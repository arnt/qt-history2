/****************************************************************************
** $Id: //depot/qt/main/examples/widgets/main.cpp#3 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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
	switch (s%4){
	case 0:
	    qApp->setStyle(new QMotifStyle);
	    break;
	case 1:
	    qApp->setStyle(new QCDEStyle);
	    break;
	case 2:
	    qApp->setStyle(new QWindowsStyle);
	    break;
	case 3:
	    qApp->setStyle(new QPlatinumStyle);
	    break;
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

    MyWidgetView w;
    a.setMainWidget( &w );

     w.show();
     return a.exec();
}
