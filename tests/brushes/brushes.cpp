#include <qpainter.h>
#include <qprinter.h>
#include <qapplication.h>
#include <qpixmap.h>
#include <qmessagebox.h>

static Qt::BrushStyle brushes[] = { Qt::NoBrush, Qt::SolidPattern,
				    Qt::Dense1Pattern, Qt::Dense2Pattern,
				    Qt::Dense3Pattern, Qt::Dense4Pattern,
				    Qt::Dense5Pattern, Qt::Dense6Pattern,
				    Qt::Dense7Pattern, Qt::HorPattern,
				    Qt::VerPattern, Qt::CrossPattern,
				    Qt::BDiagPattern, Qt::FDiagPattern,
				    Qt::DiagCrossPattern, Qt::CustomPattern };

void drawBrushes( QPainter & p )
{
    int i = 0;
    int y = 0;
    bool done = FALSE;
    while ( !done ) {
	if ( brushes[i] == Qt::CustomPattern ) {
	    done = TRUE;
	    p.setBrush( QBrush( Qt::white, QMessageBox::standardIcon( QMessageBox::Information, qApp->style() ) ) );
	} else {
	    p.setBrush( QBrush( brushes[i] ) );
	}
	p.setPen( Qt::SolidLine );
	p.drawEllipse( 10, y+10, 20, 20 );
	p.setPen( Qt::NoPen );
	p.drawEllipse( 40, y+10, 15, 20 );
	p.setPen( Qt::DotLine );
	p.drawEllipse( 70, y+10, 20, 15 );
	y += 24;
	i++;
    }
}

main(int argc, char** argv)
{
    QApplication app( argc, argv );

    QPrinter printer;
    printer.setFullPage( FALSE );

    QPixmap pm( 100, 400 );
    pm.fill();
    QPainter p( &pm );
    drawBrushes( p );
    p.end();

    if ( argc > 0 && argv[1] ) {
	printer.setOutputFileName( argv[1] );
	printer.setOutputToFile( TRUE );
    }

    p.begin( &printer );
    p.drawPixmap( 100, 0, pm );
    drawBrushes( p );
    p.setPen( Qt::SolidLine );
    p.setBrush( Qt::NoBrush );
}
