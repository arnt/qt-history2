/*
  painting.cpp
*/

#include <qapplication.h>
#include <qhbox.h>
#include <qpainter.h>
#include <qwidget.h>

class BarGraph : public QWidget
{
public:
    BarGraph( QWidget *parent ) : QWidget( parent ) { }

protected:
    virtual void paintEvent( QPaintEvent * );

private:
    void draw_bar( QPainter *painter, int x, int barHeight,
		   BrushStyle pattern );
};

// quote
void BarGraph::paintEvent( QPaintEvent * )
{
    QPainter painter( this );

    draw_bar( &painter, 10, 39, Qt::DiagCrossPattern );
    draw_bar( &painter, 40, 31, Qt::BDiagPattern );
    draw_bar( &painter, 70, 44, Qt::FDiagPattern );
    draw_bar( &painter, 100, 68, Qt::SolidPattern );

    painter.setPen( black );
    painter.drawLine( 0, 0, 0, height() - 1 );
    painter.drawLine( 0, height() - 1, width() - 1, height() - 1 );

    painter.setFont( QFont("Helvetica", 18) );
    painter.drawText( rect(), AlignHCenter | AlignTop, "Sales" );
}

void BarGraph::draw_bar( QPainter *painter, int x, int barHeight,
			 BrushStyle pattern )
{
    painter->setPen( blue );
    painter->setBrush( QBrush(darkGreen, pattern) );
    painter->drawRect( x, height() - barHeight, 20, barHeight );
}
// endquote

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    QHBox parent;
    BarGraph w( &parent );
    app.setMainWidget( &parent );
#if 0
    parent.setBackgroundColor( QColor(0xc0, 0xc0, 0xc0) );
    w.setBackgroundColor( QColor(0xb0, 0xb0, 0xb0) );
#endif
    parent.resize( 150, 125 );
    parent.setMargin( 11 );
    parent.show();
    return app.exec();
}
