//
// Qt Example Application: widgets
//
// Demonstrates Qt widgets.
//

#include <qdialog.h>
#include <qmsgbox.h>
#include <qpixmap.h>
#include <qapp.h>

// Standard Qt widgets

#include <qbttngrp.h>
#include <qchkbox.h>
#include <qcombo.h>
#include <qframe.h>
#include <qgrpbox.h>
#include <qlabel.h>
#include <qlcdnum.h>
#include <qlined.h>
#include <qlistview.h>
#include <qpushbt.h>
#include <qradiobt.h>
#include <qscrbar.h>
#include <qdrawutl.h>

class MyItem : public QListViewItem
{
public:
    enum Type { RadioButton, CheckBox, Pixmap };

    MyItem( MyItem *parent, const char *text, Type= Pixmap );
    MyItem( QListView *parent, const char *text );
    MyItem( QListViewItem *parent, const char *text, QPixmap );
    MyItem( QListView *parent, const char *text, QPixmap );

    void paintCell( QPainter *,  const QColorGroup & cg,
		    int column, int width, bool showFocus ) const;
    void setup();

protected:
    void paintBranches( QPainter * p, const QColorGroup & cg,
			int w, int y, int h, GUIStyle s ) const;

    void activate();
    void turnOffChild();
private:
    void init();
    Type t;
    bool on;
    MyItem *exclusive;

    QPixmap pix;
    QString txt;
};

//invariant: if (type != Pixmap) then parent is of type MyItem

/* XPM */
static const char * p1_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #000000000000",
"X	c #FFFFFFFF0000",
"                ",
"                ",
"         ....   ",
"        .XXXX.  ",
" .............. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .............. ",
"                "};

static    QPixmap * icon = 0; // ########### temporary!
static const int BoxSize = 16;



MyItem::MyItem( MyItem *parent, const char *text, Type type )
    : QListViewItem( parent )
{
    t = type;
    txt = text;
    init();
    if ( type == RadioButton ) {
	if ( parent->t != Pixmap )
	    warning( "MyItem::MyItem(), radio button cannot be child of button" );
	else 
	    exclusive = parent;
    }
}

MyItem::MyItem( QListView *parent, const char *text )
    : QListViewItem( parent, text, 0 )
{
    t = Pixmap;
    txt = text;
    init();
}

MyItem::MyItem( QListView *parent, const char *text, QPixmap p )
    : QListViewItem( parent, text, 0 )
{
    t = Pixmap;
    txt = text;
    init();
    pix = p;
}

MyItem::MyItem( QListViewItem *parent, const char *text, QPixmap p )
    : QListViewItem( parent, text, 0 )
{
    t = Pixmap;
    txt = text;
    init();
    pix = p;
}

void MyItem::init()
{
    on = FALSE;
    if ( !icon )
	icon = new QPixmap( p1_xpm );
    if ( t == Pixmap )
	pix = *icon; //#####
    exclusive = 0;
}

void MyItem::turnOffChild()
{
    if ( t == Pixmap && exclusive ) {
	exclusive->on = FALSE;
	exclusive->repaint();
    }
}

void MyItem::activate()
{
    if ( t == CheckBox ) {
	on = !on;
    } else if ( t == RadioButton ) {
	if ( exclusive && exclusive->exclusive != this )
	    exclusive->turnOffChild();
	on = TRUE;
	exclusive->exclusive = this;
    }
}

void MyItem::setup()
{
    QListViewItem::setup();
    int h = height();
    if ( t == Pixmap )
	 h = QMAX( pix.height(), h );
    h = QMAX( BoxSize, h );
    setHeight( h );
}


void MyItem::paintCell( QPainter * p, const QColorGroup & cg,
			       int column, int width, bool showFocus ) const
{
    if ( !p )
	return;

    QListView *lv = listView();
    if ( !lv )
	return;
    int r = 2;

    p->fillRect( 0, 0, width, height(), cg.base() );

    if ( column != 0 )
	return; //### simplified...



    bool winStyle = lv->style() == WindowsStyle;

    if ( t == Pixmap ) {
	p->drawPixmap( 0, (height()-pix.height())/2, pix );
	r += pix.width();
    } else {	
	ASSERT( lv ); //###
	//	QFontMetrics fm( lv->font() );
	//	int d = fm.height();
	int x = 0;
	int y = (height() - BoxSize) / 2;
	//	p->setPen( QPen( cg.text(), winStyle ? 2 : 1 ) );
	if ( t == CheckBox ) {
	    p->setPen( QPen( cg.text(), 2 ) );
	    p->drawRect( x+2, y+2, BoxSize-4, BoxSize-4 );
	    /////////////////////
	    x++;
	    y++;
	    if ( on ) {
		QPointArray a( 7*2 );
		int i, xx, yy;
		xx = x+3;
		yy = y+5;
		for ( i=0; i<3; i++ ) {
		    a.setPoint( 2*i,   xx, yy );
		    a.setPoint( 2*i+1, xx, yy+2 );
		    xx++; yy++;
		}
		yy -= 2;
		for ( i=3; i<7; i++ ) {
		    a.setPoint( 2*i,   xx, yy );
		    a.setPoint( 2*i+1, xx, yy+2 );
		    xx++; yy--;
		}
		p->setPen( black );
		p->drawLineSegments( a );
	    }
	    ////////////////////////
	} else { //radiobutton look
	    if ( winStyle ) {
#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

		static QCOORD pts1[] = {		// dark lines
		    1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
		static QCOORD pts2[] = {		// black lines
		    2,8, 1,7, 1,4, 2,3, 2,2, 3,2, 4,1, 7,1, 8,2, 9,2 };
		static QCOORD pts3[] = {		// background lines
		    2,9, 3,9, 4,10, 7,10, 8,9, 9,9, 9,8, 10,7, 10,4, 9,3 };
		static QCOORD pts4[] = {		// white lines
		    2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
		    11,4, 10,3, 10,2 };
		// static QCOORD pts5[] = {		// inner fill
		//    4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };
		//QPointArray a;
		//	p->eraseRect( x, y, w, h );

		p->setPen( cg.text() );
		QPointArray a( QCOORDARRLEN(pts1), pts1 );
		a.translate( x, y );
		//p->setPen( cg.dark() );
		p->drawPolyline( a );
		a.setPoints( QCOORDARRLEN(pts2), pts2 );
		a.translate( x, y );
		p->drawPolyline( a );
		a.setPoints( QCOORDARRLEN(pts3), pts3 );
		a.translate( x, y );
		//		p->setPen( black );
		p->drawPolyline( a );
		a.setPoints( QCOORDARRLEN(pts4), pts4 );
		a.translate( x, y );
		//			p->setPen( blue );
		p->drawPolyline( a );
		//		a.setPoints( QCOORDARRLEN(pts5), pts5 );
		//		a.translate( x, y );
		//	QColor fillColor = isDown() ? g.background() : g.base();
		//	p->setPen( fillColor );
		//	p->setBrush( fillColor );
		//	p->drawPolygon( a );
		if ( on     ) {
		    p->setPen( NoPen );
		    p->setBrush( cg.text() );
		    p->drawRect( x+5, y+4, 2, 4 );
		    p->drawRect( x+4, y+5, 4, 2 );
		}

	    } else { //motif
		p->setPen( QPen( cg.text() ) );
		QPointArray a;
		int cx = BoxSize/2 - 1;
		int cy = height()/2;
		int e = BoxSize/2 - 1;
		for ( int i = 0; i < 3; i++ ) { //penWidth 2 doesn't quite work
		    a.setPoints( 4, cx-e, cy, cx, cy-e,  cx+e, cy,  cx, cy+e ); 
		    p->drawPolygon( a );
		    e--;
		}
		if ( on ) { 
		    p->setPen( QPen( cg.text()) );
		    QBrush   saveBrush = p->brush();
		    p->setBrush( cg.text() );
		    e = e - 2;
		    a.setPoints( 4, cx-e, cy, cx, cy-e,  cx+e, cy,  cx, cy+e ); 
		    p->drawPolygon( a );
		    p->setBrush( saveBrush );
		}
	    }
	}
	r += BoxSize + 4;
    }

    const char * t = txt;
    if ( t ) {
	if ( lv )
	    p->setFont( lv->font() );

	if ( isSelected() && column==0 ) {
	    p->fillRect( r-2, 0, width - r + 2, height(),
			 QApplication::winStyleHighlightColor() );
	    p->setPen( white ); // ###
	} else {
	    p->setPen( cg.text() );
	}

	// should do the ellipsis thing here
	p->drawText( r, 0, width-2-r, height(), AlignLeft + AlignVCenter, t );
    }

    if ( showFocus && !column ) {
	if ( lv->style() == WindowsStyle ) {
	    p->drawWinFocusRect( r-2, 0, width-r+2, height() );
	} else {
	    p->setPen( black );
	    p->drawRect( r-2, 0, width-r+2, height() );
	}
    }
}


void MyItem::paintBranches( QPainter * p, const QColorGroup & cg,
			    int w, int, int h, GUIStyle) const
{
    p->fillRect( 0, 0, w, h, cg.base() );

}

#include "lv.moc"

int main( int argc, char **argv )
{
    QApplication  a( argc, argv );

    QListView *lb = new QListView( 0, "Hepp" );
    //lb->setFont( QFont( "Times", 24, 75 ) );
    lb->setColumn( "Name", 100 );
    lb->setColumn( "Size", 50 );
    lb->setColumn( "Type", 50 );
    lb->setColumn( "Date", 100 );
    //    connect( files, SIGNAL(sizeChanged()), SLOT(updateGeometry()) );


    lb->setTreeStepSize( 20 );

    //    QListViewItem * roo= new QListViewItem( lb );

    QListView * roo = lb;
    

    for ( int j = 0; j < 2; j++ ) {
	QListViewItem * root= new QListViewItem( roo, "a", "b", "c", "d" );

	root->setOpen(TRUE);

	QListViewItem *hurfu = 0;

	for ( int i=45; i>35; i-- ) {
	    QString str;
	    str.sprintf( "%d item", i );
	    QListViewItem* h= new QListViewItem( root, str, "test", "hei", "hopp", 0 );
	    //	    h->setOpen( TRUE );
	    if ( i == 42 )
		hurfu = h;
	}

	QListViewItem *gruff = 0;

	for ( int i=4; i>0; i-- ) {
	    QString str;
	    str.sprintf( "Haba %d", i );
	    QListViewItem *h = new QListViewItem( hurfu, "SUBB", (const char*)str, 
						  "urk", "nei", 0 );
	    if ( i == 4 )
		gruff = h;
	}

	new QListViewItem(  gruff, "Dutt", "hepp", "glurk", "niks", 0 );

    }


    MyItem *lit = new MyItem( lb, "One" );
    lit->setOpen( TRUE );
    new MyItem( lit, "Two", MyItem::CheckBox );
    new MyItem( lit, "Three", MyItem::CheckBox );
    MyItem *it = new MyItem( lit, "Four" );
    it->setOpen( TRUE );
    new MyItem( it, "Five", MyItem::RadioButton);
    new MyItem( it, "Six", MyItem::RadioButton);
    new MyItem( it, "Six and a half", MyItem::RadioButton );

    it = new MyItem( lit, "Seven" );
    it->setOpen( TRUE );
    new MyItem( it, "Eight", MyItem::CheckBox );
    new MyItem( it, "Nine", MyItem::CheckBox );
    new MyItem( it, "Ten", MyItem::CheckBox );
    it = new MyItem( it, "Eleven" );
    new MyItem( it, "Twelve", MyItem::RadioButton );
    new MyItem( it, "Thirteen", MyItem::RadioButton );
    new MyItem( it, "Fourteen", MyItem::RadioButton );



#if 0
    QFont f("Helvetica", 24, QFont::Bold);
    lb->setFont( f );              
#endif
    a.setMainWidget( lb );
    lb->show();

    return a.exec();
}
