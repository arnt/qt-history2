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
#include <qgridview.h>
#include <qpainter.h>

using namespace Qt;

// Grid size
const int numRows = 100;
const int numCols = 100;

class MyGridView : public QGridView
{
public:
    MyGridView() {
	setNumRows( ::numRows );
	setNumCols( ::numCols );
	setCellWidth( fontMetrics().width( QString("%1 / %2").arg(numRows()).arg(numCols()))); 
	setCellHeight( 2*fontMetrics().lineSpacing() );
	setWindowTitle( tr( "Qt Example - This is a grid with 100 x 100 cells" ) );
    }

protected:
    void paintCell( QPainter *p, int row, int col ) {
	p->drawLine( cellWidth()-1, 0, cellWidth()-1, cellHeight()-1 );
	p->drawLine( 0, cellHeight()-1, cellWidth()-1, cellHeight()-1 );
	p->drawText( cellRect(), AlignCenter, QString("%1 / %2").arg(row).arg(col) );
    }
};

// The program starts here.
int main( int argc, char **argv )
{
    QApplication app( argc, argv );			

    MyGridView gridview;
    app.setMainWidget( &gridview );
    gridview.show();
    return app.exec();
}
