/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qtranslatordialog.cpp#10 $
**
** Implementation of QTranslatorDialog class
**
** Created : 990115
**
** Copyright (C)1998-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/
#include "qtranslatordialog.h"
#include "qlistview.h"
#include "qlayout.h"
#include "qtranslator.h"
#include "qpushbutton.h"
#include "qmultilineedit.h"
#include "qheader.h"
#include "qmenubar.h"
#include "qapplication.h"

// NOT REVISED

class QTranslatorSignaller : public QTranslator
{
    Q_OBJECT
public:
    QTranslatorSignaller( QObject *parent, const char *name = 0 );
    QString find( uint, const char*, const char* ) const;
signals:
    void triedToFind( const char*, const char* );
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTranslatorSignaller( const QTranslatorSignaller & );
    QTranslatorSignaller &operator=( const QTranslatorSignaller & );
#endif
};

#include "qtranslatordialog.moc"

/*
  \class QTranslatorSignaller qmessagefile.h
  \brief The QTranslatorSignaller class emits a signal for each attempted translation.

  When an instance of this class is installed with
  QApplication::installTranslator(), it will send out a triedToFind()
  signal for each attempted translation that is not handled by a
  message file that was installed later.

  To get a signal for every untranslated message, install a
  QTranslatorSignaller as the \e first message file. To get a signal
  for every attempted translation, install a QTranslatorSignaller the
  \e last message file.


*/


/*!
  Constructs a QTranslatorSignaller with parent \a parent and name \a name.
*/

QTranslatorSignaller::QTranslatorSignaller( QObject *parent, const char *name )
    :QTranslator( parent, name )
{
}



/*!
  Overrides QTranslator::find to send out a triedToFind() signal with the arguments
  \a scope and \a key. The hash argument is ignored.

  It returns a null string, to indicate that there was no translation found in this message file.
*/

QString QTranslatorSignaller::find( uint, const char *scope, const char *key ) const
{
    //avoid const warning, emit:
    QTranslatorSignaller *This = (QTranslatorSignaller*)this;
    This->triedToFind( scope, key );
    return QString::null;
}

/*! \fn QTranslatorSignaller::triedToFind( const char *scope, const char *key )

  This signal is emitted for every attempted translation.
*/




class QTranslatorEdit : public QMultiLineEdit
{
public:
    QTranslatorEdit( QWidget * parent, const char *name=0 );
};

QTranslatorEdit::QTranslatorEdit( QWidget * parent, const char *name )
    :QMultiLineEdit( parent, name )
{
        clearTableFlags( Tbl_autoScrollBars );
}

class QTranslatorItem : public QListViewItem
{
public:
    QTranslatorItem( QListViewItem *parent, QString key, QString trans = QString::null );
    QTranslatorItem( QListView *parent, QString scope );

    void setText( int, const QString& );
    int width( const QFontMetrics&, const QListView*, int ) const;

    void setup();
};


QTranslatorItem::QTranslatorItem( QListViewItem *parent, QString key, QString trans)
    :QListViewItem( parent, key, trans )
{
}

QTranslatorItem::QTranslatorItem( QListView *parent, QString scope )
    :QListViewItem( parent, scope, QString::fromLatin1("----") )
{
}

static int numLines( QString s )
{
    int n = 1;
    for ( int i = 0; i < (int)s.length(); i++ )
	if ( s[i] == '\n' )
	    n++;
    return n;
}

int QTranslatorItem::width( const QFontMetrics &fm, const QListView*lv, int col ) const
{
    QRect br = fm.boundingRect( 0, 0, 1000, 1000, AlignTop|AlignLeft, text( col ) );
    int w = -(fm.minLeftBearing()+fm.minRightBearing()) +
	    br.width() + lv->itemMargin() * 2;

    return w;
}

void QTranslatorItem::setup()
{
    widthChanged();
    QListView * v = listView();
    int lines = QMAX( numLines( text(0) ), numLines( text(1) ) );
    setHeight( lines * v->fontMetrics().lineSpacing() + 2*v->itemMargin() );

}

void QTranslatorItem::setText( int col, const QString &s )
{
    QListViewItem::setText( col, s );
    setup();
}

/*!  Constructs an empty translate dialog

*/

QTranslatorDialog::QTranslatorDialog( QWidget * parent, const char* name )
    :QDialog( parent, name )
{
    showing = FALSE;
    edited = FALSE;
    QVBoxLayout *vbox = new QVBoxLayout( this, 5 );

    QMenuBar *mb = new QMenuBar( this );
    QPopupMenu * file = new QPopupMenu();
    mb->insertItem( tr("&File"), file );


    //    file->insertItem( "New", this, SLOT(newDoc()), CTRL+Key_N );
    //    file->insertItem( openIcon, "Open", this, SLOT(load()), CTRL+Key_O );
    file->insertItem( /*saveIcon,*/ tr("Save"), this, SLOT(save()), CTRL+Key_S );
    file->insertItem( tr("Close"), this, SLOT(reject()), CTRL+Key_W );
    //    file->insertItem( "Quit", qApp, SLOT(quit()), CTRL+Key_Q );


    vbox->setMenuBar( mb );

    lv = new QListView( this );
    lv->addColumn( tr("Key"), 100 );
    lv->addColumn( tr("Translation") , 100 );
    lv->setColumnWidthMode( 0, QListView::Maximum );
    lv->setColumnWidthMode( 1, QListView::Maximum );
    vbox->addWidget( lv );
    /*
    QHBoxLayout *buttons = new QHBoxLayout;
    vbox->addLayout( buttons );
    QPushButton *but  = new QPushButton( "Save", this );
    connect( but, SIGNAL(clicked()), this, SLOT( save() ) );
    buttons->addWidget( but );

    but  = new QPushButton( "Cancel", this );
    connect( but, SIGNAL(clicked()), this, SLOT( reject() ) );
    buttons->addWidget( but );
    */
    currentItem = 0;

    ed = new QTranslatorEdit( lv->viewport() );
    connect( ed, SIGNAL(textChanged()),
	     this, SLOT(textChanged()) );

    ed->hide();
    connect( lv, SIGNAL(currentChanged(QListViewItem*)),
	     this, SLOT(currentItemSet(QListViewItem*)) );
    connect( lv->header(), SIGNAL(sizeChange(int,int,int)),
	     this, SLOT(updateEd()) );
    connect( lv->header(), SIGNAL(moved(int,int)),
	     this, SLOT(updateEd()) );

}


/*! Destructs the object and frees any allocated resources.

*/

QTranslatorDialog::~QTranslatorDialog()
{

}


/*!
  Adds the text \a key inside \a scope to the list of untranslated messages.
*/

void QTranslatorDialog::add( const char* scope, const char* key )
{
    addTranslation( scope, key, 0 );
}



/*!
  Sets the translation for the text \a key inside \a scope to \a translation.
*/

void QTranslatorDialog::addTranslation( const char* scope, const char* key,
				       const char* translation  )
{
    QListViewItem *it = lv->firstChild();
    while ( it && it->text(0) != QString::fromLatin1(scope) )
	it = it->nextSibling();
    if ( it == 0 ) {
	it = new QTranslatorItem( lv, QString::fromLatin1(scope) );
	it->setOpen(TRUE);
    } else {
	QListViewItem *t  = it->firstChild();
	while ( t ) {
	    if ( t->text(0) == QString::fromLatin1(key) )
		return;
	    t = t->nextSibling();
	}

    }
    (void) new QTranslatorItem( it, QString::fromLatin1(key),
	QString::fromLatin1(translation) ); // #### huh? why latin1?

    if ( !showing )
	show();

}

/*!
  Saves the message file.
*/

void QTranslatorDialog::save()
{

    if ( currentItem )
	currentItem->setText( 1, ed->text() );     //#### should be a member function or slot or something.

    QString filename = QString::fromLatin1("test.tr");
    QTranslator mf( 0 ) ;

    QListViewItem *it = lv->firstChild();
    while ( it  ) {
	QCString scope = it->text(0).latin1();
	QListViewItem *sub = it->firstChild();
	while ( sub ) {
	    if ( !!sub->text(1) ) {
		mf.insert( scope, sub->text(0).latin1(), sub->text(1) );
		qDebug( "QTranslatorDialog::save %d, %s, %s, %s",
		       mf.hash( scope, sub->text(0).latin1() ),
		       scope.data(),
		       sub->text(0).latin1(),
		       sub->text(1).latin1() );
	    }
	    sub = sub->nextSibling();
	}
	it = it->nextSibling();
    }

    int hash = mf.hash("Main","Quit");
    QString s = mf.find( hash, "Main", "Quit" );
    qDebug( "mf: %d -> %s", hash, s.latin1() );


    mf.save( filename );




}




/*!
  Responds to changes in the selection. Moving the edit widget around
  if we are in edit mode.
*/

void QTranslatorDialog::currentItemSet( QListViewItem *it )
{
    if ( !ed )
	return;
    if ( currentItem && edited ) {
	currentItem->setText( 1, ed->text() );
    }
    QString s = it->text(1);
    if ( s == QString::fromLatin1("----") ) { //#### need a better way of distinguishing keys
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
    edited = FALSE;
}


/*!
  Sets the geometry of the edit widget according to the header state.
*/

void QTranslatorDialog::updateEd()
{
    if ( ed->isVisible() ) {
	QRect r = ed->geometry();
	int idx = lv->header()->mapToActual( 1 );
	r.setLeft( lv->header()->cellPos( idx ) );
	r.setWidth( lv->header()->cellSize( idx ) );
	ed->setGeometry( r );
    }
}


/*!
  Reimplement QWidget::hide() to avoid infinite recursion.
*/

void QTranslatorDialog::hide()
{
    showing = FALSE;
    QDialog::hide();
}


/*!
  Reimplement QWidget::show() to avoid infinite recursion.
*/

void QTranslatorDialog::show()
{
    showing = TRUE;
    QDialog::show();
}



/*!
  Sets the editor-is-dirty flag.
*/

void QTranslatorDialog::textChanged()
{
    edited = TRUE;
}



/*!  Constructs an empty

*/

QAppTranslator::QAppTranslator( QWidget * parent, const char* name )
    :QTranslatorDialog( parent, name )
{
    QTranslatorSignaller *trs = new QTranslatorSignaller( this );
    qApp->installTranslator( trs );

    connect( trs, SIGNAL(triedToFind( const char*, const char* )),
	     this, SLOT(add( const char*, const char* )));
}


/*! Destructs the object and frees any allocated resources.

*/

QAppTranslator::~QAppTranslator()
{
}
