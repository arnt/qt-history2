#include <qpainter.h>
#include <qprinter.h>
#include <qapplication.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpaintdevicemetrics.h>
#include <qpicture.h>


void drawPoint( QPainter & p, const QRect &r )
{
    p.setPen( Qt::black );
    p.drawPoint( r.center() );
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawPoint( r.topLeft() );
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawPoint( r.bottomRight() );
    p.setPen( Qt::white );
    p.drawPoint( r.bottomRight() );
}


void drawPoints( QPainter & p, const QRect &r )
{
    QPointArray a( 3 );
    a[0] = r.topLeft();
    a[1] = r.center();
    a[2] = r.bottomRight();
    p.setPen( Qt::black );
    p.drawPoints( a );
}


void drawLine( QPainter & p, const QRect &r )
{
    p.setPen( Qt::black );
    p.drawLine( r.topLeft(), r.bottomRight() );
    p.setPen( Qt::red );
    p.drawLine( r.topLeft(), QPoint( r.right(), r.center().y() ) );
    p.setPen( Qt::green );
    p.drawLine( r.topLeft(), QPoint( r.center().x(), r.bottom() ) );
}


void drawRect( QPainter & p, const QRect &r )
{
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawRect( r );
    p.setPen( Qt::white );
    p.drawRect( r );
}


void drawWinFocusRect( QPainter & p, const QRect &r )
{
    p.drawWinFocusRect( r );
}


void drawEllipse( QPainter & p, const QRect &r )
{
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawEllipse( r.left(), r.top(), r.width()/2, r.height() );
    p.setPen( Qt::black );
    p.drawEllipse( r.right()-r.width()/2, r.top(), r.width()/2, r.height() );
    p.setPen( Qt::white );
    p.drawEllipse( r.left(), r.top(), r.width()/2, r.height() );
}


void drawArc( QPainter & p, const QRect &r )
{
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawArc( r, 1600, 2560 );
    p.setPen( Qt::white );
    p.drawArc( r, 1600, 2560 );
}


void drawChord( QPainter & p, const QRect &r )
{
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawChord( r, 1600, 2560 );
    p.setPen( Qt::white );
    p.drawChord( r, 1600, 2560 );
}


void drawLineSegments( QPainter & p, const QRect &r )
{
    QPointArray a( 6 );
    a[0] = r.topLeft();
    a[1] = r.bottomLeft();
    a[2] = QPoint( r.center().x(), r.top() );
    a[3] = QPoint( r.center().x(), r.bottom() );
    a[4] = r.topRight();
    a[5] = r.bottomRight();
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawLineSegments( a );
    p.setPen( Qt::white );
    p.drawLineSegments( a );
}


void drawPolyline( QPainter & p, const QRect &r )
{
    QPointArray a( 6 );
    a[0] = r.topLeft();
    a[1] = r.bottomLeft();
    a[2] = QPoint( r.center().x(), r.top() );
    a[3] = QPoint( r.center().x(), r.bottom() );
    a[4] = r.topRight();
    a[5] = r.bottomRight();
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawPolyline( a );
    p.setPen( Qt::white );
    p.drawPolyline( a );
}


void drawPolygon( QPainter & p, const QRect &r )
{
    double magic = .57735026918962576451;

    QPointArray a( 3 );
    a[0] = QPoint( r.left(), r.center().y() + (int)(r.height()/2*magic) );
    a[1] = QPoint( r.right(), r.center().y() + (int)(r.height()/2*magic) );
    a[2] = QPoint( r.center().x(), r.top() );
    QPointArray b( 3 );
    b[0] = QPoint( r.left(), r.center().y() - (int)(r.height()/2*magic) );
    b[1] = QPoint( r.right(), r.center().y() - (int)(r.height()/2*magic) );
    b[2] = QPoint( r.center().x(), r.bottom() );
    p.setPen( QPen( Qt::black, 3 ) );
    p.setBrush( QBrush( Qt::Dense4Pattern ) );
    p.drawPolygon( a );
    p.setBrush( Qt::NoBrush );
    p.drawPolygon( b );
    p.setPen( Qt::white );
    p.drawPolygon( a );
    p.drawPolygon( b );
}


void drawCubicBezier( QPainter & p, const QRect &r )
{
    QPointArray a( 4 );
    a[0] = r.topLeft();
    a[1] = r.bottomLeft();
    a[2] = r.bottomRight();
    a[3] = r.topRight();
    p.setPen( QPen( Qt::black, 3 ) );
    p.drawCubicBezier( a );
    p.setPen( Qt::white );
    p.drawCubicBezier( a );
    a[1] = r.bottomRight();
    a[2] = r.bottomLeft();
    p.setPen( QPen( Qt::blue, 3 ) );
    p.drawCubicBezier( a );
    p.setPen( Qt::white );
    p.drawCubicBezier( a );
}


const char *fileopen_xpm[] = {
    "    16    13        5            1",
    ". c #040404",
    "# c #808304",
    "a c None",
    "b c #f3f704",
    "c c #f3f7f3",
    "aaaaaaaaa...aaaa",
    "aaaaaaaa.aaa.a.a",
    "aaaaaaaaaaaaa..a",
    "a...aaaaaaaa...a",
    ".bcb.......aaaaa",
    ".cbcbcbcbc.aaaaa",
    ".bcbcbcbcb.aaaaa",
    ".cbcb...........",
    ".bcb.#########.a",
    ".cb.#########.aa",
    ".b.#########.aaa",
    "..#########.aaaa",
    "...........aaaaa"
};


void drawPixmap( QPainter & p, const QRect &r )
{
    double s;
    s = r.width() / 16.0;
    if ( s > r.height() / 13.0 )
	s = r.height() / 13.0;
    p.translate( r.left(), r.top() );
    p.scale( s, s );
    p.drawPixmap( 0, 0, QPixmap( fileopen_xpm ) );
}


void drawImage( QPainter & p, const QRect &r )
{
    double s;
    s =  r.width() / 16.0;
    if ( s > r.height() / 13.0 )
	s = r.height() / 13.0;
    p.translate( r.left(), r.top() );
    p.scale( s, s );
    p.drawImage( 0, 0, QImage( fileopen_xpm ) );
}


void drawTiledPixmap( QPainter & p, const QRect &r )
{
    p.drawTiledPixmap( r, QPixmap( fileopen_xpm ) );
}


static const char * futility_of_love =
"The day starts out all right.  Barbara Dare, porn star, is "
"appearing at a local video store.  I arrive just a few hours early "
"and help the guy open the store.  After lunch, Ms. Dare arrived. "
"Her skin was very clear and her hair smelled really good.  I was in "
"love and felt queasy when I looked at all the lechers ogling her. "
"But worse still, at the end of the day, she retreated to a back "
"room and emerged with her panties in her hand.  They were to be "
"auctioned off."
"\n\n"
"Oh the humanity."
"\n\n"
"I got the panties and headed back home.  On TV Johnny "
"Cougar's (Yeah, I don't give a fuck what he calls himself, he'll "
"always be Johnny Cougar to me) new video (\"Get a leg up\") is on and "
"in it, he's dancing with this model (Elaine Irwin?  Nancy Irwin? "
"Ashley Montana?) and there's something about the way she smiles as "
"she dances that makes the whole Barbara Dare thing seem cheap, and "
"leave a bad taste in my mouth."
"\n\n"
"So I take the panties from my mouth and later that evening "
"head out to a local comedy club.  The manager is a guy I went to "
"high school with and after the show I go out with him for coffee. "
"Joining us is the woman who had just performed (to a standing "
"ovation), mousey, deadpan comedienne Margaret Smith.  She and the "
"manager are buzzed because the show went so great and I'm buzzed "
"because as we drink our coffee, smoke our cigarettes down to the "
"filter and talk, I come to realize that her onstage persona (the "
"first lady of angst) is no sham--she *is* just like that.  Her "
"despair is voluptuous; her entropy goes straight to the bone."
"\n\n"
"Mine."
"\n\n"
"Her skin is pasty; she is flat-chested; her ass (as she puts "
"it) is two Saltines; she smells like stale cigarettes and I want "
"her desperately.  Our calves touch under the table and she doesn't "
"pull hers back.  I lose my breath and when my manager-friend goes "
"to the bathroom I ask her out."
"\n\n"
"A few hours later, as I'm whacking off into Barbara Dare's "
"panties, I laugh so hard I burst a blood vessel in my neck and I "
"die."
"\n\n"
"Oh, I die.";


void drawText( QPainter & p, const QRect &r )
{
    p.setFont( QFont( "Helvetica", 4 ) );
    p.setPen( Qt::black );
    p.drawText( r, Qt::AlignLeft + Qt::AlignTop + Qt::WordBreak,
		futility_of_love );
}


void drawTextMetrics( QPainter & p, const QRect &r )
{
    p.setPen( Qt::lightGray );
    p.drawRect( r );

    int s = 4;
    QString t = QString::fromLatin1( futility_of_love ).simplifyWhiteSpace();

    p.setFont( QFont( "times", 9 ) );
    QFontMetrics fm = p.fontMetrics();

    int x = r.left();
    int y = r.top() + fm.ascent() + fm.leading();
    int b = 0;
    while( b >= 0 ) {
	int e = t.find( ' ', b+1 );
	if ( e > b ) {
	    QString w = t.mid( b, e-b ).simplifyWhiteSpace();
	    QRect wr( x, y - fm.ascent(), fm.width( w ), fm.height() );
	    if ( wr.right() >= r.right() ) {
		p.setPen( Qt::lightGray );
		p.drawRect( wr );
		y += fm.lineSpacing()+2;
		x = r.left();
		wr.setRect( x, y - fm.ascent(), fm.width( w ), fm.height() );
	    }
	    p.setPen( Qt::darkGray );
	    p.drawRect( wr );
	    p.setPen( Qt::black );
	    p.drawText( x, y, w );
	    p.setPen( Qt::lightGray );
	    x = x + wr.width() + fm.width( ' ' );
	}
	b = e;
    }
}


void drawPicture( QPainter & p, const QRect &r )
{
    QPicture picture;
    QPainter p2( &picture );
    p2.save();
    drawImage( p2, QRect( r.topLeft(), r.center() ) );
    p2.restore();
    p2.save();
    drawText( p2, QRect( r.topRight(), r.center() ).normalize() );
    p2.restore();
    p2.save();
    drawPolyline( p2, QRect( r.bottomLeft(), r.center() ).normalize() );
    p2.restore();
    p2.save();
    drawCubicBezier( p2, QRect( r.bottomRight(), r.center() ).normalize() );
    p2.restore();
    p2.end();
    p.drawPicture( picture );

    p.setPen( Qt::darkGray );
    p.drawLine( r.left(), r.center().y(), r.right(), r.center().y() );
    p.drawLine( r.center().x(), r.top(), r.center().x(), r.bottom() );
}


void lineCapJoinHack(  QPainter & p, int x, int y, int w, int h,
		       Qt::PenCapStyle s, Qt::PenJoinStyle j )
{
    QPointArray a( 4 );
    a[0] = QPoint( x + w/4, y + h/4 );
    a[1] = QPoint( x + w/4, y + h - h/4 );
    a[2] = QPoint( x + w/2, y + h - h/4 );
    a[3] = QPoint( x + w - w/4, y + h/4 );
    p.setPen( QPen( Qt::black, 3, Qt::SolidLine, s, j ) );
    p.drawPolyline( a );
    p.setPen( Qt::white );
    p.drawPolyline( a );
}

void lineCapAndJoin( QPainter & p, const QRect &r )
{
    int w = r.width()/3;
    int h = r.height()/3;
    int x = r.left();
    int y = r.top();

    lineCapJoinHack( p, x,y, w,h, Qt::FlatCap, Qt::MiterJoin );
    lineCapJoinHack( p, x+w,y, w,h, Qt::FlatCap, Qt::BevelJoin );
    lineCapJoinHack( p, x+w+w,y, w,h, Qt::FlatCap, Qt::RoundJoin );

    y += h;
    lineCapJoinHack( p, x,y, w,h, Qt::SquareCap, Qt::MiterJoin );
    lineCapJoinHack( p, x+w,y, w,h, Qt::SquareCap, Qt::BevelJoin );
    lineCapJoinHack( p, x+w+w,y, w,h, Qt::SquareCap, Qt::RoundJoin );

    y += h;
    lineCapJoinHack( p, x,y, w,h, Qt::RoundCap, Qt::MiterJoin );
    lineCapJoinHack( p, x+w,y, w,h, Qt::RoundCap, Qt::BevelJoin );
    lineCapJoinHack( p, x+w+w,y, w,h, Qt::RoundCap, Qt::RoundJoin );
}


typedef void (*TestFunction)(QPainter &, const QRect &);


TestFunction f[19] = {
    drawPoint, drawPoints, drawLine, drawRect, drawWinFocusRect,
    drawEllipse, drawArc, drawChord, drawLineSegments, drawPolyline,
    drawPolygon, drawCubicBezier, drawPixmap, drawImage,
    drawTiledPixmap, drawText, drawPicture, drawTextMetrics, lineCapAndJoin
};


const char * n[19] = {
    "drawPoint", "drawPoints", "drawLine", "drawRect", "drawWinFocusRect",
    "drawEllipse", "drawArc", "drawChord", "drawLineSegments", "drawPolyline",
    "drawPolygon", "drawCubicBezier", "drawPixmap", "drawImage",
    "drawTiledPixmap", "drawText", "drawPicture", "fontMetrics", "lineCap/Join"
};



void test( const char * output,
	   QPrinter::Orientation o, QPrinter::ColorMode cm,
	   int copies, bool multipage, const char * creator )
{
    QPrinter printer;

    if ( output ) {
	printer.setOutputFileName( output );
	printer.setOutputToFile( TRUE );
    }

    printer.setFullPage( TRUE );
    printer.setOrientation( o );
    printer.setColorMode( cm );
    printer.setNumCopies( copies );
    if ( creator )
	printer.setCreator( creator );

    QPainter p( &printer );
    p.setFont( QFont( "Helvetica", 8 ) );

    p.setPen( Qt::lightGray );
    int i = multipage ? 240 : 127;
    QRect cr( 28, 28, i, i );
    QRect pr( 14, 14, cr.width()-28, cr.width()-28 );
    int paperWidth = QPaintDeviceMetrics( &printer ).width();
    int paperHeight = QPaintDeviceMetrics( &printer ).height();

    p.setPen( Qt::lightGray );

    for( i=0; i<19; i++ ) {
	p.save();
	p.drawRect( cr );
	p.setPen( Qt::darkGray );
	p.drawText( cr.left()+5, cr.top()+12, n[i] );
	p.setClipRect( cr );
	p.translate( cr.left(), cr.top() );
	f[i]( p, pr );
	p.restore();

	// two 28-point margins
	cr.setRect( cr.right()+1, cr.top(), cr.width(), cr.height() );
	if ( cr.right() >= paperWidth-2*28 ) {
	    cr.setRect( 28, cr.bottom()+1, cr.width(), cr.height() );
	    if ( multipage &&
		 cr.bottom() >= paperHeight - 2*28 ) {
		printer.newPage();
		cr.setRect( 28, 28, cr.width(), cr.height() );
	    }
	}
    }
}

main(int argc, char** argv)
{
    QApplication app(argc, argv);

    test( "/tmp/portrait-1.ps", QPrinter::Portrait, QPrinter::Color,
	  1, FALSE, 0 );
    test( "/tmp/landscape-1.ps", QPrinter::Landscape, QPrinter::Color,
	  1, FALSE, 0 );

    test( "/tmp/portrait-2.ps", QPrinter::Portrait, QPrinter::Color,
	  1, TRUE, 0 );
    test( "/tmp/landscape-2.ps", QPrinter::Landscape, QPrinter::Color,
	  1, TRUE, 0 );

    test( "/tmp/portrait-mono.ps", QPrinter::Portrait, QPrinter::GrayScale,
	  1, TRUE, 0 );
}
