#include "dialogs.h"

#include <qlayout.h>
#include <qsqlform.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>

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



//
//  UpdateMatchDialog class
//

UpdateMatchDialog::UpdateMatchDialog( QSqlRecord* buf, QWidget * parent,
				      const char * name )
    : QDialog( parent, name, TRUE )
{
    QWidget *     w = new QWidget( this );
    QVBoxLayout * g = new QVBoxLayout( this );
    QHBoxLayout * h = new QHBoxLayout;

    setCaption( "Update match results" );

    form = new QSqlForm( w, buf, 2, this);
    g->setMargin( 3 );

    QLabel * label = new QLabel( "Update match results", this );
    QFont f = font();
    f.setBold( TRUE );
    label->setFont( f );
    g->addWidget( label );

    h->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding,
				 QSizePolicy::Minimum ) );

    QPushButton * button = new QPushButton( "&Update", this );
    button->setDefault( TRUE );
    connect( button, SIGNAL( clicked() ), SLOT( execute() ) );
    h->addWidget( button );

    button = new QPushButton( "&Close", this );
    connect( button, SIGNAL( clicked() ), SLOT( close() ) );
    h->addWidget( button );

    g->addWidget( w );
    g->addLayout( h );
}

void UpdateMatchDialog::close()
{
    reject();
}

void UpdateMatchDialog::execute()
{
    form->writeRecord();
    accept();
}
