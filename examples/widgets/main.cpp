/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qstylefactory.h>
#include "widgets.h"

class MyWidgetView : public WidgetView
{
    int s;
public:
    MyWidgetView( QWidget *parent=0, const char *name=0 )
	:WidgetView(parent, name), s(0)
    {
	setToolBarsMovable( TRUE );
    }

    void button1Clicked()
    {
	QStringList styles = QStyleFactory::keys();

	s = (++s)%styles.count();
	qApp->setStyle( styles[ s] );
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

    MyWidgetView* w = new MyWidgetView;
    a.setMainWidget( w );

    w->show();
    int res = a.exec();
    delete w;
    return res;
}
