#include "dialogs.h"
#include "widgets.h"

#include <qlayout.h>
#include <qsqlform.h>
#include <qsqleditorfactory.h>
#include <qsqlpropertymap.h>
#include <qlabel.h>
#include <qfont.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qdatetimeedit.h>

//
//  MatchDialog class
//
MatchDialog::MatchDialog( QSqlRecord* buf, Mode mode, QWidget * parent,
			  const char * name )
    : MatchDialogBase( parent, name, TRUE ),
      matchRecord( buf ),
      mMode( mode )
{
    QString op, title;
    switch ( mMode ) {
    case Insert:
	title = "Insert match result";
	op = "&Insert";
	break;
    case Update:
	title = "Update match result";
	op = "&Update";
	break;
    case Delete:
	title = "Delete match result";
	op = "&Delete";
	break;
    }

    setCaption( title );
    titleLabel->setText( title );
    actionButton->setText( op );
    
    form = new QSqlForm( this, "matchform" );
    QSqlPropertyMap * pm = new QSqlPropertyMap();
    
    pm->insert( "TeamPicker", "teamid" );
    form->installPropertyMap( pm );

    form->insert( winnerTeam, buf->field("winnerid") );
    form->insert( loserTeam, buf->field("loserid") );
    form->insert( winnerWins, buf->field("winnerwins") );
    form->insert( loserWins, buf->field("loserwins") );
    form->insert( date, buf->field("date") );
    
    form->readFields();
    updateSets();

    // If we're in delete mode - disable editing
    if( mode == Delete ){
	uint i = 0;
	QWidget * w;

	while( (w = form->widget( i++ )) != 0 )
	    w->setEnabled( FALSE );
    }
    
    connect( winnerWins, SIGNAL( valueChanged(int) ), SLOT( updateSets() ) );
    connect( loserWins, SIGNAL( valueChanged(int) ), SLOT( updateSets() ) );
    connect( actionButton, SIGNAL( clicked() ), SLOT( execute() ) );
    connect( closeButton, SIGNAL( clicked() ), SLOT( close() ) );
}

void MatchDialog::close()
{
    reject();
}

void MatchDialog::execute()
{
    form->writeFields();
    accept();
}

void MatchDialog::updateSets()
{
    sets->setText( QString::number(winnerWins->value() + loserWins->value()) );
}

