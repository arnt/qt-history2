//#define NO_MENUS
//#define NO_FONT_SELECTION

#include <qapp.h>

#include <qdialog.h>
#include <qlined.h>
#include <qchkbox.h>
#include <qbttngrp.h>
#include <qpushbt.h>

#include <qpainter.h>
#include <qbitmap.h>
#include <qpmcache.h>
#include <qimage.h>
#include <qbuffer.h>
#include <qlabel.h>
#include <qprinter.h>

#if !defined(NO_MENUS)
#include <qmenubar.h>
#endif
#include "doom.cpp"

// --------------------------------------------------------------------------
//  menu mappings
//

static PenStyle menuPenStyles[4] = {		// menu pens
    NoPen,	SolidLine,	DotLine,	DashLine };

static int menuLWidths[3] = {			// menu line widths
     0,	     2,		  4 };

static BrushStyle menuBrushStyles[15] = {		// menu brushes
    NoBrush,      SolidPattern, Dense1Pattern,Dense2Pattern,Dense3Pattern,
    Dense4Pattern,Dense5Pattern,Dense6Pattern,Dense7Pattern,HorPattern,
    VerPattern,   CrossPattern, BDiagPattern, FDiagPattern,
    DiagCrossPattern };

static QColor menuColors[17] = {		// menu colors
    black,	white,	  darkGray,	gray,	    lightGray,
    red,	green,	  blue,		cyan,	    magenta,
    yellow,     darkRed,  darkGreen,	darkBlue,   darkCyan,
    darkMagenta,darkYellow };

 class ToggleIconButton : public QPushButton
 {
     Q_OBJECT
 public:
     ToggleIconButton( QWidget *parent=0, const char *name=0 )
	 : QPushButton(parent, name) { setToggleButton(TRUE); }
     ToggleIconButton( const QPixmap &pm, 
		       QWidget *parent=0, const char *name=0 )
	 : QPushButton( parent, name) { setPixmap(pm); setToggleButton(TRUE); }
 };

const char *shapeNames[] = {"Line", "Rectangle", "RoundRect", "Text", 
			    "Ellipse", "Arc", "Pie", "Chord", "PolyLine",
			    "Polygon", "Bezier", "Bitmap", "Pixmap", 0 };

const char *colorNames[] = { "Black", "White",
			     "Dark Gray", "Gray", "Light Gray", "Red", "Green",
			     "Blue", "Cyan", "Magenta", "Yellow", "Dark Red",
			     "Dark Green", "Dark Blue", "Dark Cyan",
			     "Dark Magenta", "Dark Yellow", 0 };

const char *penStyleNames[] = { "None", "Solid", "Dots", "Dashes", 0 };

const char *penWidthNames[] = { "Thin", "Medium", "Fat", 0 };
const char *brushStyleNames[] = {
    "None", "Solid", "Dense1", "Dense2",
    "Dense3", "Dense4", "Dense5", "Dense6", "Dense7",
    "Horizontal", "Vertical", "Cross", "Diagonal 1",
    "Diagonal 2", "Diagonal Cross", "User defined 1",
    "User defined 2",0 };

    const char *labelNames[] = {"Shape", "Pen color", "Pen style", "Pen width",
			   "Brush color", "Brush style", "Font family",
			   "Font size" };

    const char *fontNames[] = { "Helvetica", "Times", "Courier", "Symbol" };
    const int fontSizes[] = { 8, 10, 12, 14, 18, 24, 36, 48, 72, 96 };

class DrawView : public QWidget
{
    Q_OBJECT
public:
    DrawView( QWidget *parent=0, const char *name=0 );
   ~DrawView() {}

    enum Shape {Line, Rectangle, RoundRect, Text, Ellipse, Arc, Pie, Chord, 
		 PolyLine, Polygon, Bezier, Bitmap, Pixmap, MaxShape };

    void setShape( Shape s ) { currentShape = s; repaint(); }
    Shape shape()   { return currentShape; }
    QPen *pen()     { return &currentPen; } 
    QBrush *brush() { return &currentBrush; } 
    bool evenOdd()  { return evenOddOn; }

    static void drawShape( QPainter *p, const QPoint&, Shape shape,
			   bool evenOddFill = FALSE );
    QRegion region;
    bool    evenOddOn;
    bool    regionOn;
public slots:
    void toggleEvenOdd();
    void toggleRegion();
private:

    void paintEvent( QPaintEvent * );
    void resizeEvent( QResizeEvent * );
    void closeEvent( QCloseEvent *e ) { e->ignore(); }

    Shape   currentShape;
    QPen    currentPen;
    QBrush  currentBrush;
};

class DrawControl : public QWidget
{
    Q_OBJECT
public:
    DrawControl( QWidget *parent=0, const char *name=0 );
   ~DrawControl() {}

    static void setBrushPattern( QBrush *, int index );
public slots:
    void raiseView() { view->raise(); }
    void printView();

private slots:
    void shapeSelected( int );
    void penColorSelected( int );
    void penStyleSelected( int );
    void penWidthSelected( int );
    void brushColorSelected( int );
    void brushStyleSelected( int );
    void fontNameSelected( int );
    void fontSizeSelected( int );
private:
    void createMenus();
    DrawView *view;
    bool brushColorSet;
};

DrawControl::DrawControl( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
    view = new DrawView( 0, "drawView" );
    view->setGeometry( 200, 200, 320, 200);
    createMenus();
    view->show();
    brushColorSet = FALSE;
}

DrawView::DrawView( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
    setFont( QFont( "Courier", 18, QFont::Bold ) );
    setBackgroundColor( white );
    currentShape = Line;
    currentBrush.setStyle( SolidPattern );
    currentBrush.setColor( white );
    evenOddOn = FALSE;
    regionOn  = FALSE;
}
    void printView();

void DrawControl::printView()
{
    QPrinter printer;
    QPainter p;

    if ( printer.setup() ) {
	p.begin( &printer );
	p.setWindow( 0, 0, 320, 200 ); // define coordinate system
	p.setViewport( 0, 0, 320, 200 ); // define coordinate system
	p.setPen( *view->pen() );
	p.setBrush( *view->brush() );
	p.setFont( view->font() );
	if ( view->regionOn )
	    p.setClipRegion( view->region );		     // set clip region
	if ( view->shape() == DrawView::Bitmap && view->evenOdd() )
	    p.setBackgroundMode( TransparentMode );
	DrawView::drawShape( &p, QPoint( 0, 0 ), view->shape(), !view->evenOdd() );
	p.end();
    }
}

void DrawControl::shapeSelected( int shapeID )
{
    view->setShape( (DrawView::Shape) shapeID );
}

void DrawControl::penColorSelected( int i )
{
    view->pen()->setColor( menuColors[i] );
    if ( view->shape() == DrawView::Pixmap )
	return;
    view->repaint( FALSE );
}

void DrawControl::penStyleSelected( int i )
{
    view->pen()->setStyle( menuPenStyles[i] );
    if ( view->shape() == DrawView::Pixmap || 
	 view->shape() == DrawView::Bitmap )
	return;
    view->repaint();
}

void DrawControl::penWidthSelected( int i )
{
    view->pen()->setWidth( menuLWidths[i] );
    if ( view->shape() == DrawView::Pixmap || 
	 view->shape() == DrawView::Bitmap )
	return;
    view->repaint();
}

void DrawControl::brushColorSelected( int i )
{
    brushColorSet = TRUE;
    view->brush()->setColor( menuColors[i] );
    if ( view->shape() == DrawView::Pixmap )
	return;
    view->repaint( FALSE );
}

void DrawControl::brushStyleSelected( int i )
{
    setBrushPattern( view->brush(), i );
    if ( !brushColorSet )
	view->brush()->setColor( black );

    if ( view->shape() == DrawView::Pixmap || 
	 view->shape() == DrawView::Bitmap )
	return;
    view->repaint();
}

void DrawControl::fontNameSelected( int i )
{
    QFont f = view->font();
    f.setFamily( fontNames[i] );
    view->setFont( f );
    if ( view->shape() == DrawView::Text )
	view->repaint();
}

void DrawControl::fontSizeSelected( int i )
{
    QFont f = view->font();
    f.setPointSize( fontSizes[i] );
    view->setFont( f );
    if ( view->shape() == DrawView::Text )
	view->repaint();
}

void DrawView::toggleEvenOdd()
{
    evenOddOn = !evenOddOn;
    if ( currentShape == Polygon )
	repaint();
}

void DrawView::toggleRegion()
{
    static bool first = TRUE;
    regionOn = !regionOn;
    if ( first ) {
	QRegion r1( QRect(10,10,200,200),	// r1 = elliptic region
		    QRegion::Ellipse );
	QRegion r2( QRect(0,20,150,150) );	// r2 = rectangular region
	QRegion r3( QRect(240,100,40,40),	 // r3 = circular region
		    QRegion::Ellipse );
	QRegion r4( QRect(240,100,35,30) );	   // r3 = rectangular region
	QRegion r5 = r3.intersect( r4 );
	region = r1.intersect( r2 );
	region = region.unite( r5 );
	first = FALSE;
    }
    repaint();
}


static int maxWidth( const QFontMetrics &fm, const char * arr[] )
{
    int tmp, maxW = 0;
    for( int i = 0 ; arr[i] ; i++ ) {
	tmp = fm.width( arr[i] );
	if ( tmp > maxW )
	    maxW = tmp;
    }
    return maxW;
}

void DrawControl::createMenus()
{
#if !defined(NO_MENUS)
    QMenuBar *menu = new QMenuBar( this, "mainMenu" );	// create menubar

    QPopupMenu *shapeM = new QPopupMenu;		// create menu popups
    QPopupMenu *brushM = new QPopupMenu;
    QPopupMenu *penM	= new QPopupMenu;
#if !defined(NO_FONT_SELECTION)
    QPopupMenu *fontM	= new QPopupMenu;
#endif
    menu->insertItem( "Shape", shapeM );
    menu->insertItem( "Pen"  , penM );
    menu->insertItem( "Brush", brushM );

#if !defined(NO_FONT_SELECTION)
    menu->insertItem( "Font" , fontM );
#endif
    QPopupMenu *brushStyleM = new QPopupMenu;
    QPopupMenu *brushColorM = new QPopupMenu;
    brushM->insertItem( "Color", brushColorM );
    brushM->insertItem( "Style", brushStyleM );

    QPopupMenu *penStyleM   = new QPopupMenu;
    QPopupMenu *penColorM   = new QPopupMenu;
    QPopupMenu *penWidthM   = new QPopupMenu;
    penM->insertItem( "Color", penColorM );
    penM->insertItem( "Style", penStyleM );
    penM->insertItem( "Width", penWidthM );

#if !defined(NO_FONT_SELECTION)
    QPopupMenu *fontNameM   = new QPopupMenu;
    QPopupMenu *fontSizeM   = new QPopupMenu;
    fontM->insertItem( "Family", fontNameM );
    fontM->insertItem( "Size", fontSizeM );

    connect( fontSizeM	 ,SIGNAL(activated(int)),SLOT(fontSizeSelected(int)));
    connect( fontNameM	 ,SIGNAL(activated(int)),SLOT(fontNameSelected(int)));
#endif
    connect( shapeM	 ,SIGNAL(activated(int)),SLOT(shapeSelected(int)));
    connect( brushStyleM,SIGNAL(activated(int)),SLOT(brushStyleSelected(int)));
    connect( brushColorM,SIGNAL(activated(int)),SLOT(brushColorSelected(int)));
    connect( penStyleM	 ,SIGNAL(activated(int)),SLOT(penStyleSelected(int)));
    connect( penColorM	 ,SIGNAL(activated(int)),SLOT(penColorSelected(int)));
    connect( penWidthM	 ,SIGNAL(activated(int)),SLOT(penWidthSelected(int)));

#endif

    QButtonGroup *shapeG      = new QButtonGroup;
    QButtonGroup *brushStyleG = new QButtonGroup;
    QButtonGroup *brushColorG = new QButtonGroup;
    QButtonGroup *penStyleG  = new QButtonGroup;
    QButtonGroup *penColorG  = new QButtonGroup;
    QButtonGroup *penWidthG  = new QButtonGroup;

#if !defined(NO_FONT_SELECTION)
    QButtonGroup *fontNameG  = new QButtonGroup;
    QButtonGroup *fontSizeG  = new QButtonGroup;
    connect( fontSizeG	 ,SIGNAL(clicked(int)),SLOT(fontSizeSelected(int)));
    connect( fontNameG	 ,SIGNAL(clicked(int)),SLOT(fontNameSelected(int)));
#endif
    connect( shapeG	 ,SIGNAL(clicked(int)),SLOT(shapeSelected(int)));
    connect( brushStyleG,SIGNAL(clicked(int)),SLOT(brushStyleSelected(int)));
    connect( brushColorG,SIGNAL(clicked(int)),SLOT(brushColorSelected(int)));
    connect( penStyleG	 ,SIGNAL(clicked(int)),SLOT(penStyleSelected(int)));
    connect( penColorG	 ,SIGNAL(clicked(int)),SLOT(penColorSelected(int)));
    connect( penWidthG	 ,SIGNAL(clicked(int)),SLOT(penWidthSelected(int)));

    ToggleIconButton *tmp, *tmp2;
    QPixmap pix;
    QPainter p;
    QFontMetrics fm = fontMetrics();

    QWMatrix mtx;
    mtx.scale( 1.0*fm.height()/200, 1.0*fm.height()/200 ); 
    pix.resize( 120, fm.height()+4 );

    int i;

    QLabel *tmpLabel;
#if !defined(NO_FONT_SELECTION)
    const int nLabels = 8;
#else
    const int nLabels = 6;
#endif
    for( i = 0 ; i < nLabels ; i++ ) {
	tmpLabel = new QLabel( labelNames[i], this );
	if ( i < 6 )
	    tmpLabel->setGeometry( 10 + i*130, 40,  130, 20 );
	else
	    tmpLabel->setGeometry( 270 + (i - 6)*130, 250,  130, 20 );
	tmpLabel->setAlignment( AlignHCenter );
	tmpLabel->setFrameStyle( QFrame::Box | QFrame::Sunken );
    }

    for ( i = 0 ; i < 4 ; i++ ) {
	pix.resize( maxWidth( fm, penStyleNames ) + 50, fm.height() + 4 );
	pix.fill( backgroundColor() );
	p.begin( &pix );
	p.setPen( QPen(black,2,menuPenStyles[i]) );
	p.drawLine( 0, fm.height()/2+2, 40, fm.height()/2+2 );
	p.drawText( 50, fm.ascent()+2, penStyleNames[i] );
	p.end();
	tmp = new ToggleIconButton( pix, this );
	tmp->setGeometry(270, i*30 + 60, 130, 30 );
	penStyleG->insert( tmp, i );
#if !defined(NO_MENUS)
	penStyleM->insertItem( pix, i );
#endif
    }

    for ( i = 0 ; i < 3 ; i++ ) {
	pix.resize( maxWidth( fm, penWidthNames ) + 50, fm.height() + 4 );
	pix.fill( backgroundColor() );
	p.begin( &pix );
	p.setPen( QPen(black,menuLWidths[i]) );
	p.drawLine( 0, fm.height()/2+2, 40, fm.height()/2+2 );
	p.drawText( 50, fm.ascent()+2, penWidthNames[i] );
	p.end();
	tmp = new ToggleIconButton( pix, this );
	tmp->setGeometry(400, i*30 + 60, 130, 30 );
	penWidthG->insert( tmp, i );
#if !defined(NO_MENUS)
	penWidthM->insertItem( pix, i );
#endif
    }

    QBrush br( black );
    for ( i = 0 ; i < 17 ; i++ ) {
	pix.resize( maxWidth( fm, brushStyleNames ) + 30, fm.height() + 4 );
	pix.fill( backgroundColor() );
	p.begin( &pix );
	setBrushPattern( &br, i );
	p.setBrush( br );
	p.drawRect( 0, 2, 20, fm.height() );
	p.drawText( 30, fm.ascent()+2, brushStyleNames[i] );
	p.end();
	tmp = new ToggleIconButton( pix, this );
	tmp->setGeometry(660, i*30 + 60, 130, 30 );
	brushStyleG->insert( tmp, i );
#if !defined(NO_MENUS)
	brushStyleM->insertItem( pix, i );
#endif
    }

    for ( i = 0 ; i < 17 ; i++ ) {
	pix.resize( maxWidth( fm, colorNames ) + 30, fm.height() + 4 );
	pix.fill( backgroundColor() );
	p.begin( &pix );
	p.setPen( black );
	p.setBrush( menuColors[i] );
	p.drawRect( 0, 2, 20, fm.height() );
	p.drawText( 30, fm.ascent()+2, colorNames[i] );
	p.end();
	tmp = new ToggleIconButton( pix, this );
	tmp->setGeometry(140, i*30 + 60, 130, 30 );
	tmp2 = new ToggleIconButton( pix, this );
	tmp2->setGeometry(530, i*30 + 60, 130, 30 );
	penColorG->insert( tmp, i );
	brushColorG->insert( tmp2, i );
#if !defined(NO_MENUS)
	penColorM->insertItem( pix );
	brushColorM->insertItem( pix, i );
#endif
    }

    for ( i = 0 ; i < DrawView::MaxShape ; i++ ) {
	pix.resize( maxWidth( fm, shapeNames ) + 50, fm.height() + 4 );
	pix.fill( backgroundColor());
	p.begin( &pix );
	p.setBackgroundColor( backgroundColor());
	p.setWorldMatrix( mtx );
	if ( i != DrawView::Text )
	   DrawView::drawShape(&p,p.xFormDev(QPoint(2,2)),(DrawView::Shape)i);
	p.setWorldMatrix( QWMatrix() );
	p.drawText( 40, fm.ascent() + 2, shapeNames[i] );
	if ( i == DrawView::Text ) {
	    p.setFont( view->font() );
	    p.drawText(2, -p.fontMetrics().boundingRect("Yo!").top()+2, "Yo!");
	}
	p.end();
	tmp = new ToggleIconButton( pix, this );
	tmp->setGeometry(10, i*30 + 60, 130, 30 );
	shapeG->insert( tmp, i );
#if !defined(NO_MENUS)
	shapeM->insertItem(pix, i);
#endif
    }

#if !defined(NO_FONT_SELECTION)
    for ( i = 0 ; i < 4 ; i++ ) {
	QFont f( fontNames[i], 18 );
	pix.resize( 100, 26 );
	pix.fill( backgroundColor() );
	p.begin( &pix );
	p.setPen( black );
	p.setFont( f );
	QFontMetrics fm = p.fontMetrics();
	p.drawText( 2, fm.ascent()+2, fontNames[i] );
	p.end();
	tmp = new ToggleIconButton( pix, this );
	tmp->setGeometry(270, i*30 + 270, 130, 30 );
	fontNameG->insert( tmp, i );
#if !defined(NO_MENUS)
	fontNameM->insertItem( pix );
#endif
    }

    QString s;
    for ( i = 0 ; i < 10 ; i++ ) {
	s.sprintf( "%i",fontSizes[i] );
	QPushButton *tmp = new QPushButton( s, this );
	tmp->setFont( QFont( "Times", 18 ) );
	tmp->setGeometry(400, i*30 + 270, 130, 30 );
	fontSizeG->insert( tmp, i );
#if !defined(NO_MENUS)
	fontSizeM->insertItem( s, i );
#endif
    }
#if !defined(NO_MENUS)
    fontSizeM->setFont( QFont( "Times", 18 ) );
#endif

#endif // if !defined(NO_FONT_SELECTION)

    QCheckBox *evenOdd = new QCheckBox( "Even/odd fill", this );
    evenOdd->setGeometry( 30, 455, 100, 25 );
    connect( evenOdd, SIGNAL(clicked()), view, SLOT(toggleEvenOdd()) );

    QCheckBox *check = new QCheckBox( "Clip region", this );
    check->setGeometry( 30, 480, 90, 25 );
    connect( check, SIGNAL(clicked()), view, SLOT(toggleRegion()) );

    QPushButton *push = new QPushButton( "Raise view!", this );
    push->setGeometry( 25, 510, 100, 25 );
    connect( push, SIGNAL(clicked()), SLOT(raiseView()) );

    QPushButton *print = new QPushButton( "Print", this );
    print->setGeometry( 25, 540, 100, 25 );
    connect( print, SIGNAL(clicked()), SLOT(printView()) );
}

void DrawView::paintEvent( QPaintEvent * )
{
    QPainter p;
    p.begin( this );
    p.setWindow( 0, 0, 320, 200 ); // define coordinate system
    p.setPen( currentPen );
    p.setBrush( currentBrush );
    if ( regionOn )
	p.setClipRegion( region );			// set clip region
    drawShape( &p, QPoint( 0, 0 ), currentShape, !evenOddOn );
    p.end();
}

QRect operator+(const QPoint&p, const QRect r)
{
    QRect tmp = r;
    tmp.moveTopLeft( p );
    return tmp;
}

// bm1_bits and bm2_bits were generated by the X bitmap program.

#define bm1_width  32
#define bm1_height 32

static unsigned char bm1_bits[] = {	// "smiling" face
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x0f, 0x00,
   0x00, 0x06, 0x30, 0x00, 0x80, 0x01, 0xc0, 0x00, 0x40, 0x00, 0x00, 0x01,
   0x20, 0x00, 0x00, 0x02, 0x10, 0x00, 0x00, 0x04, 0x08, 0x3e, 0x3e, 0x08,
   0x08, 0x03, 0xe0, 0x08, 0xc4, 0x00, 0x00, 0x11, 0x04, 0x1e, 0x78, 0x10,
   0x02, 0x0c, 0x30, 0x20, 0x02, 0x40, 0x00, 0x20, 0x02, 0x40, 0x00, 0x20,
   0x02, 0x40, 0x00, 0x20, 0x02, 0x20, 0x04, 0x20, 0x02, 0x20, 0x04, 0x20,
   0x02, 0x10, 0x08, 0x20, 0x02, 0x08, 0x08, 0x20, 0x02, 0xf0, 0x07, 0x20,
   0x04, 0x00, 0x00, 0x10, 0x04, 0x00, 0x00, 0x10, 0x08, 0x00, 0xc0, 0x08,
   0x08, 0x3c, 0x30, 0x08, 0x10, 0xe6, 0x19, 0x04, 0x20, 0x00, 0x0f, 0x02,
   0x40, 0x00, 0x00, 0x01, 0x80, 0x01, 0xc0, 0x00, 0x00, 0x06, 0x30, 0x00,
   0x00, 0xf8, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00};

#define bm2_width 16
#define bm2_height 16
static unsigned char bm2_bits[] = {    // pattern with "Qt" embedded
   0x8e, 0xf0, 0x99, 0x92, 0x15, 0x91, 0x73, 0xf8, 0xee, 0x1f, 0x99, 0xc9,
   0x5a, 0x29, 0x32, 0x2f, 0xf2, 0xae, 0x91, 0xd9, 0x92, 0x15, 0xf8, 0x73,
   0x1f, 0xee, 0x89, 0x98, 0xc9, 0x59, 0x8f, 0x30};

void DrawControl::setBrushPattern( QBrush *b, int index )
{
    if ( index < 15 )
	b->setStyle( menuBrushStyles[index] );
    else {
	QString key;
	key.sprintf( "mybrush %d", index );
	QPixmap *pm = QPixmapCache::find( key );
	if ( !pm ) {				// not in pixmap cache
	    if ( index == 15 )
		pm = new QBitmap( bm1_width, bm1_height, bm1_bits, TRUE );
	    else {
		pm = new QPixmap;
		if ( !pm->load( "colorful.bmp" ) ) {
                    static const char *txt = "\"colorful.bmp\" not found!";
                    QRect r = qApp->fontMetrics().boundingRect( txt );
                    pm->resize( r.width() + 10, r.height() + 10 );
                    pm->fill( black );
                    QPainter p;
                    p.begin( pm );
                    p.setPen( white );
                    p.drawText( -r.x() + 5, -r.y() + 5, txt );
                    p.end();
		}                
	    }
	    QPixmapCache::insert( key, pm );	// save in pixmap cache
	}
	b->setPixmap( *pm );
    }
}

void DrawView::drawShape( QPainter *p, const QPoint &pnt, Shape shape,
			  bool evenOddFill )
{
    static QPoint pos1( 10, 10 );
    static QPoint textPos( 10, 100 );
    static QPoint pos2( 310, 190 );
    static QRect rct( pos1, pos2 );
    static const char *text = "Yo! Yo! Set yourself free!";
    static int angle1 = 120*16;
    static int angle2 = 230*16;
    static QCOORD pnts[] = { 10, 190, 160, 10, 310, 190, 10, 100, 310, 100 };
    static QPointArray points( 5, pnts );
    static QCOORD bpnts[] = { 10, 10, 310, 300, 310, -200, 10, 190 };
    static QPointArray bezPoints( 4, bpnts );
    static QPixmap pixmap;
    static QBitmap bitmap;
    static bool pixLoaded = FALSE;

    if ( !pixLoaded && (shape == Pixmap || shape == Bitmap) ) {
	QImageIO io;
	QString tmp; 
	tmp.resize( doomSize + 1);     // doomSize and doomStr are defined
	qmemmove( tmp.data(), doomStr, doomSize + 1 ); // in doom.cpp
	QBuffer b( tmp );	      
	b.open( IO_ReadOnly );
	io.setIODevice( &b );	      // read image from string via buffer
	io.read();
	b.close();
	pixmap.convertFromImage( io.image() );
	bitmap = pixmap;	     // will automatically dither image	       
	pixLoaded = TRUE;
    }

    switch( shape ) {
	case Line:
	    p->drawLine( pnt+pos1, pnt+pos2 );
	    break;
	case Rectangle:
	    p->drawRect( pnt+rct );
	    break;
	case RoundRect:
	    p->drawRoundRect( pnt+rct, 20, 40 );
	    break;
	case Text:
	    p->drawText(pnt+p->xFormDev(QPoint(5,p->fontMetrics().ascent()+2)),
			text );
	    break;
	case Ellipse:
	    p->drawEllipse( pnt+rct );
	    break;
	case Arc:
	    p->drawArc( pnt+rct, angle1, angle2 );
	    break;
	case Pie:
	    p->drawPie( pnt+rct, angle1, angle2);
	    break;
	case Chord:
	    p->drawChord( pnt+rct, angle1, angle2);
	    break;
	case PolyLine: {
	    QPointArray tmpArr = points.copy();
	    tmpArr.translate( pnt.x(), pnt.y() );
	    p->drawPolyline( tmpArr );
	    break;
	}
	case Polygon: {
	    QPointArray tmpArr = points.copy();
	    tmpArr.translate( pnt.x(), pnt.y() );
	    p->drawPolygon( tmpArr, evenOddFill );
	    break;
	}
	case Bezier: {
	    QPointArray tmpArr = bezPoints.copy();
	    tmpArr.translate( pnt.x(), pnt.y() );
	    p->drawQuadBezier( tmpArr );
	    break;
	}
	case Bitmap:
//	    p->setBackgroundMode( OpaqueMode );
	    p->drawPixmap( pnt, bitmap );
	    break;
	case Pixmap:
	    p->drawPixmap( pnt, pixmap );
	    break;
	default:
	    break;
    }


}

void DrawView::resizeEvent( QResizeEvent * )
{
//    eraseRect = QRect( width()/2, height()/2, 0, 0 );
}

void f(char*){}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
//    installDebugHandler(f);
    DrawControl *tmp = new DrawControl;
    tmp->setGeometry( 0, 0, 800, 575 );
    tmp->show();
    tmp->raiseView();

    a.setMainWidget( tmp );
    return a.exec();
}

#include "draw.moc"		     // include metadata generated by the moc
