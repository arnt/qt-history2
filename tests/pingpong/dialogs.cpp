#include "dialogs.h"
#include "widgets.h"

#include <qlayout.h>
#include <qsqlform.h>
#include <qlabel.h>
#include <qfont.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlineedit.h>

//
//  MatchDialog class
//
MatchDialog::MatchDialog( QSqlRecord* buf, Mode mode, QWidget * parent,
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
	setCaption( "Insert Match Result" );
	op = "&Insert";
	break;
    case Update:
	setCaption( "Update Match Result" );
	op = "&Update";
	break;
    case Delete:
	setCaption( "Delete Match Result" );
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

    flabel = new QLabel( buf->displayLabel( "winner" ), w );
    wteam = new TeamPicker( w );
    wteam->setTeamId( buf->value("winnerid").toInt() );
    formLayout->addWidget( flabel, 0, 0 );
    formLayout->addWidget( wteam, 0, 1 );
    form->associate( wteam, buf->field("winnerid") );

    flabel = new QLabel( buf->displayLabel("loser"), w );
    lteam = new TeamPicker( w );
    lteam->setTeamId( buf->value("loserid").toInt() );
    formLayout->addWidget( flabel, 0, 2 );
    formLayout->addWidget( lteam, 0, 3 );
    form->associate( lteam, buf->field("loserid") );

    flabel = new QLabel( buf->displayLabel("winnerwins"), w );
    wins = new QSpinBox( w );
    formLayout->addWidget( flabel, 1, 0 );
    formLayout->addWidget( wins, 1, 1 );
    form->associate( wins, buf->field("winnerwins") );
    connect( wins, SIGNAL( valueChanged(int) ), SLOT( updateSets() ) );

    flabel = new QLabel( buf->displayLabel("loserwins"), w );
    losses = new QSpinBox( w );
    formLayout->addWidget( flabel, 1, 2 );
    formLayout->addWidget( losses, 1, 3 );
    form->associate( losses, buf->field("loserwins") );
    connect( losses, SIGNAL( valueChanged(int) ), SLOT( updateSets() ) );

    flabel = new QLabel( buf->displayLabel("date"), w );
    editor = ef->createEditor( w, buf->value("date") );
    formLayout->addWidget( flabel, 2, 0 );
    formLayout->addWidget( editor, 2, 1 );
    form->associate( editor, buf->field("date") );

    flabel = new QLabel( buf->displayLabel("sets"), w );
    sets = new QLineEdit( w );
    formLayout->addWidget( flabel, 2, 2 );
    formLayout->addWidget( sets, 2, 3 );
    sets->setEnabled( FALSE );
    form->readRecord();

    g->setMargin( 3 );

    QLabel * label = new QLabel( caption(), this );
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

void MatchDialog::close()
{
    reject();
}

void MatchDialog::execute()
{
    form->writeRecord();
    accept();
}

void MatchDialog::updateSets()
{
    matchRecord->setValue( "sets", wins->value() + losses->value() );
    sets->setText( matchRecord->value( "sets" ).toString() );
}

