#include "labelled.h"
#include <qlabelled.h>
#include <qlabel.h>
#include <qapp.h>
#include <qgrid.h>
#include <qvbox.h>
#include <qchkbox.h>
#include <qpushbt.h>



#include <qlayout.h>
#include <qobjcoll.h>
/*!
  Constructs an buttonrow widget with parent \a parent and name \a name
 */
QButtonRow::QButtonRow( QWidget *parent, const char *name )
    :QWidget( parent, name )
{
    lay = new QHBoxLayout( this, parent?0:5, 5, name ); //### border
    first = TRUE;
    prefSize = QSize(0,0);
}

void QButtonRow::recalc()
{
    //    const int border = 5;
    int maxw=0;
    int miny=0;

    debug( "QButtonRow::recalc" );

    QObjectListIt it(*children());
    QObject *o;
    while ( (o=it.current()) ) {
	++it;
	if ( o-> isWidgetType() ) {
	    QWidget *w = (QWidget*) o;
	    maxw = QMAX( maxw, w->minimumSize().width() );
	    miny = QMAX( miny, w->minimumSize().height() );
	}
    }

    if ( maxw == prefSize.width() && miny == prefSize.height() )
	return;
    prefSize = QSize( maxw, miny );
    it.toFirst();
    while ( (o=it.current()) ) {
	++it;
	if ( o->isWidgetType() ) {
	    QWidget *w = (QWidget*) o;
	    w->setMaximumSize( prefSize );
	    debug( "recalc setting %p to (%d,%d)", w, prefSize.width(), 
		   prefSize.height() );
	}
    }
}

/*!
  This function is called when a child changes min/max size.
 */
void QButtonRow::layoutEvent( QEvent * )
{
    recalc();
}

/*!
  This function is called when the widget gets a new child or loses an old one.
 */
void QButtonRow::childEvent( QChildEvent *c )
{
    QWidget *w = c->child();

    if ( !c->inserted() ) {
	///recalc();  //############# removed event comes while child is still there!
	return;
    }

    QSize sh = w->sizeHint();
    w->setAutoMinimumSize( TRUE );
    w->setMinimumSize( sh );

    if ( first )
	first = FALSE;
    else
	lay->addStretch( 0 );
    lay->addWidget( w, 1 );

    if ( sh.width() > prefSize.width() || sh.height() > prefSize.height() ) 
	recalc();
    else {
	w->setMaximumSize( prefSize );
	debug( "childEvent setting %p to (%d,%d)", w, prefSize.width(), prefSize.height() );
    }
    if ( isVisible() )
	lay->activate();
}


void QButtonRow::dump()
{
    QObjectListIt it(*children());
    QObject *o;
    while ( (o=it.current()) ) {
	++it;
	if ( o-> isWidgetType() ) {
	    QWidget *w = (QWidget*) o;
	    debug( "%s/%s, max (%d,%d), min (%d,%d)", w->className(), w->name(),
		   w->maximumSize().width(), w->maximumSize().height(),
		   w->minimumSize().width(), w->minimumSize().height() );
	}
    }
    QWidget *w = this;
    debug( "%s/%s, max (%d,%d), min (%d,%d)", w->className(), w->name(),
	   w->maximumSize().width(), w->maximumSize().height(),
	   w->minimumSize().width(), w->minimumSize().height() );

}


/*!
  Provides childEvent() while waiting for Qt 2.0.
 */
bool QButtonRow::event( QEvent *e ) {
    switch ( e->type() ) {
    case Event_ChildInserted:
    case Event_ChildRemoved:
	childEvent( (QChildEvent*)e );
	return TRUE;
    case Event_LayoutHint:
	layoutEvent( e );
	return TRUE;
    default:
	return QWidget::event( e );
    }
}

main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    QVBox b;

    QLabelled m("Labelled widget", &b);
    //m.setAlignment(AlignLeft);
    QGrid g( 2, &m);
    for ( int i = 1; i < 10; i++ ) 
	new QCheckBox( "Press Me", &g );


    QButtonRow h( &b );
    QPushButton p1( "OK", &h );
    QObject::connect( &p1, SIGNAL(clicked()), qApp, SLOT(quit()));
    QPushButton pp( "I'll think about it", &h );
    QPushButton p2( "Cancel", &h );
    QObject::connect( &p2, SIGNAL(clicked()), qApp, SLOT(quit()));
    //    b.resize(0,0); //############

    QObject::connect( &pp, SIGNAL(clicked()), &h, SLOT(dump()));

    b.show();


    //    QApplication::setFont( QFont("Helvetica",32),TRUE );

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
