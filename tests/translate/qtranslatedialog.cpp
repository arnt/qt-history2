/****************************************************************************
** $Id: //depot/qt/main/tests/translate/qtranslatedialog.cpp#1 $
**
**
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtranslatedialog.h"
#include "qlistview.h"
#include "qlayout.h"
#include "qmessagefile.h"
#include "qpushbutton.h"
#include "qlineedit.h"
#include "qheader.h"


/*!  Constructs an empty translate dialog

*/

QTranslateDialog::QTranslateDialog( QWidget * parent, const char* name )
    :QDialog( parent, name )
{
    QVBoxLayout *vbox = new QVBoxLayout( this, 5 );
    lv = new QListView( this );
    lv->addColumn( "Key", 100 );
    lv->addColumn( "Translation" , 100 );
    vbox->addWidget( lv );
    QHBoxLayout *buttons = new QHBoxLayout;
    vbox->addLayout( buttons );
    QPushButton *but  = new QPushButton( "Save", this );
    connect( but, SIGNAL(clicked()), this, SLOT( save() ) );
    buttons->addWidget( but );

    but  = new QPushButton( "Cancel", this );
    connect( but, SIGNAL(clicked()), this, SLOT( reject() ) );
    buttons->addWidget( but );

    currentItem = 0;
    
    ed = new QLineEdit( lv->viewport() );
    ed->hide();
    connect( lv, SIGNAL(currentChanged(QListViewItem*)),
	     this, SLOT(currentItemSet(QListViewItem*)) );
}


/*! Destroys the object and frees any allocated resources.

*/

QTranslateDialog::~QTranslateDialog()
{
    
}


/*!
  Adds the text \a key inside \a scope to the list of untranslated messages.
*/

void QTranslateDialog::add( const char* scope, const char* key )
{
    addTranslation( scope, key, 0 );
}



/*!
  Sets the translation for the text \a key inside \a scope to \a translation.
*/

void QTranslateDialog::addTranslation( const char* scope, const char* key,
				       const char* translation  )
{
    debug( "QTranslateDialog %s, %s", scope, key);
    
    QListViewItem *it = lv->firstChild();
    while ( it && it->text(0) != scope )
	it = it->nextSibling();
    if ( it == 0 ) {
	it = new QListViewItem( lv, scope, "----" );
	it->setOpen(TRUE);
    }
    (void) new QListViewItem( it, key, translation );
    
    if ( !isVisible() )
	show();
    
}

/*!
  Saves the message file.
*/

void QTranslateDialog::save()
{
    //#### should be a member function or slot or something.
    if ( currentItem )
	currentItem->setText( 1, ed->text() );

    QString filename = "test.tr";
    QMessageFile mf( 0 ) ;
    
    QListViewItem *it = lv->firstChild();
    while ( it  ) {
	QString scope = it->text(0);
	QListViewItem *sub = it->firstChild();
	while ( sub ) {
	    if ( sub->text(1) ) {
		int hash = mf.hash(scope, sub->text(0));
		mf.insert( hash, sub->text(1) );
		debug( "QTranslateDialog::save %d, %s, %s, %s", 
		       hash,
		       (const char*) scope,
		       (const char*) sub->text(0),
		       (const char*) sub->text(1) );
	    }
	    sub = sub->nextSibling();
	}
	it = it->nextSibling();
    }

    int hash = mf.hash("Main","Quit");
    QString s = mf.find( hash, "Main", "Quit" );
    debug( "mf: %d -> %s", hash, (const char*)s );
    
    
    mf.save( filename );




}




/*!
  Responds to changes in the selection. Moving the edit widget around
  if we are in edit mode.
*/

void QTranslateDialog::currentItemSet( QListViewItem *it )
{
    if ( !ed )
	return;
    if ( currentItem )
	currentItem->setText( 1, ed->text() );

    QString s = it->text(1);
    if ( s == "----" ) { //#### need a better way of distinguishing keys
	currentItem = 0;
	ed->hide();
    } else {
	currentItem = it;
	QRect r = lv->itemRect( it );
	int idx = lv->header()->mapToActual( 1 );
	r.setLeft( lv->header()->cellPos( idx ) ); 
	r.setWidth( lv->header()->cellSize( idx ) ); 
	ed->setGeometry( r );
	ed->setText( it->text(1) );
	ed->show();
	ed->setFocus();
    }
}
