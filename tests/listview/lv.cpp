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

    MyItem( QListViewItem *parent, const char *text, Type= Pixmap );
    MyItem( QListView *parent, const char *text, Type = Pixmap );
    MyItem( QListViewItem *parent, const char *text, QPixmap );
    MyItem( QListView *parent, const char *text, QPixmap );

    void paintCell( QPainter *,  const QColorGroup & cg,
		    int column, int width, bool showFocus ) const;
protected:
    bool decorateChildren() const;

private:
    Type t;
    QPixmap pix;
    QString txt;
};

MyItem::MyItem( QListViewItem *parent, const char *text, Type type )
    : QListViewItem( parent )
{
    t = type;
    txt = text;
}

MyItem::MyItem( QListView *parent, const char *text, Type type )
    : QListViewItem( parent, text, 0 )
{
    t = type;
    txt = text;
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

    //    QPixmap * icon = 0; // ### temporary! to be replaced with an array

    bool winStyle = lv->style() == WindowsStyle;

    if ( t == Pixmap ) {
#if 0
	p->drawPixmap( 0, (height()-icon->height())/2, pix );
	r += pix.width();
#endif
    } else {	
	ASSERT( lv ); //###
	//	QFontMetrics fm( lv->font() );
	//	int d = fm.height();
	int d = height() - 4;
	p->setPen( QPen( cg.text(), winStyle ? 2 : 1 ) );
	if ( t == CheckBox ) {
	    p->drawRect( 2, 2, d, d );
	} else {
	    if ( winStyle ) {
		p->drawEllipse( 2, 2, d, d );  //####### UGLY!!!!
	    } else {
		QPointArray a;
		int z = height()/2;
		int e = d/2;
		a.setPoints( 4, z-e, z,  z, z-e,  z+e, z,  z, z+e ); 
		QPen     savePen   = p->pen();
                QBrush   saveBrush = p->brush();

		p->drawPolygon( a );

	    }
	}
	r += d + 4;
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





bool MyItem::decorateChildren() const
{
    return FALSE;
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
#if 1

    MyItem *it = new MyItem( lb, "One" );
    it->setOpen( TRUE );
    new MyItem( it, "Two", MyItem::RadioButton );
    new MyItem( it, "Three", MyItem::RadioButton );
    QListViewItem *lit = new QListViewItem( it, "Dutt", 0 );
    new QListViewItem( lit, "Futt", 0 );
    it = new MyItem( it, "Four" );
    new MyItem( it, "Five", MyItem::CheckBox );
    new MyItem( it, "Six", MyItem::CheckBox );


#endif
    a.setMainWidget( lb );
    lb->show();

    return a.exec();
}
