#include "dialogs.h"

#include <qlayout.h>
#include <qsqlform.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>

//
// TeamPicker editor widget
//
TeamPicker::TeamPicker( QWidget * parent = 0, const char * name = 0 )
    : QComboBox( parent, name )
{
    //    setFocusPolicy( StrongFocus );
    QSqlCursor team( "team" );
    team.select( team.index("name") );
    int idx = 0;
    while( team.next() ) {
	insertItem( team.value("name").toString(), idx );
	index2Id[idx] = team.value("id").toInt();
	idx++;
    }
}

int TeamPicker::teamId() const
{
    return index2Id[ currentItem() ];
}

void TeamPicker::setTeamId( int id )
{
    QMap<int,int>::Iterator it;
    for( it = index2Id.begin(); it != index2Id.end(); ++it ) {
	if ( it.data() == id ) {
	    setCurrentItem( it.key() );
	    break;
	}
    }
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

    // Lay out the editor widgets manually
    QSqlEditorFactory * ef = QSqlEditorFactory::defaultFactory();
    QWidget * editor;
    QLabel * flabel;
    QGridLayout * formLayout = new QGridLayout( w );
    
    formLayout->setSpacing( 5 );
    formLayout->setMargin( 5 );

    form = new QSqlForm( this, "updatematchform" );
    
    flabel = new QLabel( buf->field("winner")->displayLabel(), w );
    TeamPicker * wteam = new TeamPicker( w );
    wteam->setTeamId( buf->value("winnerid").toInt() ); 
    formLayout->addWidget( flabel, 0, 0 );
    formLayout->addWidget( wteam, 0, 1 );
    
    flabel = new QLabel( buf->field("loser")->displayLabel(), w );
    TeamPicker * lteam = new TeamPicker( w );
    lteam->setTeamId( buf->value("loserid").toInt() ); 
    formLayout->addWidget( flabel, 0, 2 );
    formLayout->addWidget( lteam, 0, 3 );

    flabel = new QLabel( buf->field("wins")->displayLabel(), w );
    editor = ef->createEditor( w, buf->value("wins") );
    formLayout->addWidget( flabel, 1, 0 );
    formLayout->addWidget( editor, 1, 1 );
    form->associate( editor, buf->field("wins") );

    flabel = new QLabel( buf->field("losses")->displayLabel(), w );
    editor = ef->createEditor( w, buf->value("losses") );
    formLayout->addWidget( flabel, 1, 2 );
    formLayout->addWidget( editor, 1, 3 );
    form->associate( editor, buf->field("losses") );

    flabel = new QLabel( buf->field("date")->displayLabel(), w );
    editor = ef->createEditor( w, buf->value("date") );
    formLayout->addWidget( flabel, 2, 0 );
    formLayout->addWidget( editor, 2, 1 );
    form->associate( editor, buf->field("date") );

    flabel = new QLabel( buf->field("sets")->displayLabel(), w );
    editor = ef->createEditor( w, buf->value("sets") );
    formLayout->addWidget( flabel, 2, 2 );
    formLayout->addWidget( editor, 2, 3 );
    form->associate( editor, buf->field("sets") );
    form->readRecord();
    
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
