#include "dialogs.h"

#include <qlayout.h>
#include <qsqlform.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>

//
//  ProductEditor custom editor class
//


ProductEditor::ProductEditor( QWidget * parent, const char * name )
    : QFrame( parent, name )
{
    QHBoxLayout* hb = new QHBoxLayout( this );
    
    cb = new QComboBox( this, "productComboBox" );
    cb->setFocusPolicy( StrongFocus );
    setFocusProxy( cb );
    QSqlQuery products;
    products.exec( "select name, id from product;" );
    while ( products.next() )
	cb->insertItem( products.value(0).toString(), -1 );

    connect( cb, SIGNAL( activated( int ) ),
	     SLOT( slotSetProductId( int ) ) );
    hb->addWidget( cb );
}

int ProductEditor::productId() const
{
    return cb->currentItem();
}

void ProductEditor::setProductId( int productId )
{
    cb->setCurrentItem( productId );
}

void ProductEditor::slotSetProductId( int productId )
{
    cb->setCurrentItem( productId );
}

//
//  InvoiceEditorFactory class
//

InvoiceEditorFactory::InvoiceEditorFactory ( QObject * parent, const char * name )
    : QSqlEditorFactory( parent, name )
{
}

QWidget * InvoiceEditorFactory::createEditor( QWidget * parent, const QSqlField* f )
{
    if ( f->name() == "productid" ) 
	return new ProductEditor( parent );
    else
	return QSqlEditorFactory::createEditor( parent, f );
}



//
//  GenericDialog class
//

GenericDialog::GenericDialog( QSqlRecord* buf, Mode mode, QWidget * parent,
			      const char * name )
    : QDialog( parent, name, TRUE ),
      mMode( mode )
{
    QWidget *     w = new QWidget( this );
    QVBoxLayout * g = new QVBoxLayout( this );
    QHBoxLayout * h = new QHBoxLayout;

    QString op, caption;
    if( mMode == Insert ){
	op      = "&Insert";
	caption = "Insert record";
    } else if( mMode == Update ){
	op      = "&Update";
	caption = "Update record";
    } else if( mMode == Delete ){
	op      = "&Delete";
	caption = "Delete record";
	w->setEnabled( FALSE );
    }
    setCaption( caption );

    form = new QSqlForm( w, buf, 2, this);
    g->setMargin( 3 );

    QLabel * label = new QLabel( caption, this );
    QFont f = font();
    f.setBold( TRUE );
    label->setFont( f );
    g->addWidget( label );

    h->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding,
				 QSizePolicy::Minimum ) );

    QPushButton * button = new QPushButton( op, this );
    button->setDefault( TRUE );
    connect( button, SIGNAL( clicked() ), SLOT( execute() ) );
    h->addWidget( button );

    button = new QPushButton( "&Close", this );
    connect( button, SIGNAL( clicked() ), SLOT( close() ) );
    h->addWidget( button );

    g->addWidget( w );
    g->addLayout( h );
}

void GenericDialog::close()
{
    reject();
}

void GenericDialog::execute()
{
    form->writeRecord();
    accept();
}
