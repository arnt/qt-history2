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
//  UpdateMatchDialog class
//

UpdateMatchDialog::UpdateMatchDialog( QSqlRecord* buf, Mode mode, QWidget * parent,
				      const char * name )
    : QDialog( parent, name, TRUE ),
      matchRecord( buf ),
      mMode( mode )
{
    QWidget *     w = new QWidget( this );
    QVBoxLayout * g = new QVBoxLayout( this );
    QHBoxLayout * h = new QHBoxLayout;

    QString op;
    switch ( mMode ) {
    case Insert:
	setCaption( "Insert match results" );		
	op = "&Insert";
	break;	
    case Update:
	setCaption( "Update match results" );	
	op = "&Update";	
	break;
    case Delete:
	setCaption( "Delete match results" );	
	op = "&Delete";		
	break;
    }

    // Lay out the editor widgets manually
    QSqlEditorFactory * ef = QSqlEditorFactory::defaultFactory();
    QWidget * editor;
    QLabel * flabel;
    QGridLayout * formLayout = new QGridLayout( w );

    formLayout->setSpacing( 5 );
    formLayout->setMargin( 5 );

    form = new QSqlForm( this, "matchform" );

    QSqlPropertyMap* pm = new QSqlPropertyMap();
    pm->insert( "TeamPicker", "teamid" );
    form->installPropertyMap( pm );

    flabel = new QLabel( buf->field("winner")->displayLabel(), w );
    wteam = new TeamPicker( w );
    wteam->setTeamId( buf->value("winnerid").toInt() );
    formLayout->addWidget( flabel, 0, 0 );
    formLayout->addWidget( wteam, 0, 1 );
    form->associate( wteam, buf->field("winnerid") );

    flabel = new QLabel( buf->field("loser")->displayLabel(), w );
    lteam = new TeamPicker( w );
    lteam->setTeamId( buf->value("loserid").toInt() );
    formLayout->addWidget( flabel, 0, 2 );
    formLayout->addWidget( lteam, 0, 3 );
    form->associate( lteam, buf->field("loserid") );

    flabel = new QLabel( buf->field("wins")->displayLabel(), w );
    wins = new QSpinBox( w );
    formLayout->addWidget( flabel, 1, 0 );
    formLayout->addWidget( wins, 1, 1 );
    form->associate( wins, buf->field("wins") );
    connect( wins, SIGNAL( valueChanged(int) ),
	     SLOT( updateSets() ) );

    flabel = new QLabel( buf->field("losses")->displayLabel(), w );
    losses = new QSpinBox( w );
    formLayout->addWidget( flabel, 1, 2 );
    formLayout->addWidget( losses, 1, 3 );
    form->associate( losses, buf->field("losses") );
    connect( losses, SIGNAL( valueChanged(int) ),
	     SLOT( updateSets() ) );

    flabel = new QLabel( buf->field("date")->displayLabel(), w );
    editor = ef->createEditor( w, buf->value("date") );
    formLayout->addWidget( flabel, 2, 0 );
    formLayout->addWidget( editor, 2, 1 );
    form->associate( editor, buf->field("date") );

    flabel = new QLabel( buf->field("sets")->displayLabel(), w );
    sets = new QLineEdit( w );
    formLayout->addWidget( flabel, 2, 2 );
    formLayout->addWidget( sets, 2, 3 );
    sets->setEnabled( FALSE );
    form->readRecord();

    g->setMargin( 3 );

    QLabel * label = new QLabel( "Update match results", this );
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

    if ( mMode == Delete )
	w->setEnabled( FALSE );
    updateSets();
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

void UpdateMatchDialog::updateSets()
{
    matchRecord->setValue( "sets", wins->value() + losses->value() );
    sets->setText( matchRecord->value( "sets" ).toString() );
}
